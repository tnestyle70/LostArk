SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1100(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t2.xzwy, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).xyz;
    // 20: mad r4.xyz, cb0[3].yyyy, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[3].yyyy)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 21: mad r4.xyz, r3.zzzz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((r3.zzzz)*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 22: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, v4.xyxx, t0.zwxy, s1, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 24: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 25: dp2 r1.w, r3.zwzz, r3.zwzz
    r1.w = (dot((r3.zwzz).xy,(r3.zwzz).xy).xxxx).w;
    // 26: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 28: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 29: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 30: mul r4.xy, r3.zwzz, cb0[3].xxxx
    r4.xy = ((r3.zwzz)*(source[3].xxxx)).xy;
    // 31: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 32: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 33: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 34: dp3 r1.w, r4.xyzx, r1.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 35: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mul r5.xyz, r0.xyzx, r2.wwww
    r5.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 38: mul r6.xyz, cb0[2].xyzx, cb0[4].xxxx
    r6.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // 39: mul r3.yzw, r3.yyyy, r6.xxyz
    r3.yzw = ((r3.yyyy)*(r6.xxyz)).yzw;
    // 40: mad r6.xyz, r3.yzwy, r0.xyzx, -r5.xyzx
    r6.xyz = ((r3.yzwy)*(r0.xyzx)+(-(r5.xyzx))).xyz;
    // 41: mad r3.yzw, r3.yyzw, r6.xxyz, r5.xxyz
    r3.yzw = ((r3.yyzw)*(r6.xxyz)+(r5.xxyz)).yzw;
    // 42: mul r5.xyz, r3.xxxx, cb0[1].xyzx
    r5.xyz = ((r3.xxxx)*(source[1].xyzx)).xyz;
    // 43: mul r5.xyz, r5.xyzx, cb0[3].zzzz
    r5.xyz = ((r5.xyzx)*(source[3].zzzz)).xyz;
    // 44: mad r1.xyz, v7.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((v7.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 45: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 46: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 47: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 48: dp3 r0.w, r1.xyzx, r4.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 49: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 50: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 51: mul r0.w, r0.w, cb0[3].w
    r0.w = ((r0.wwww)*(source[3].wwww)).w;
    // 52: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 53: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 55: mul r1.xyz, r5.xyzx, r0.wwww
    r1.xyz = ((r5.xyzx)*(r0.wwww)).xyz;
    // 56: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 57: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 58: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 59: mad r0.xyz, r2.xyzx, r3.yzwy, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r3.yzwy)+(r0.xyzx)).xyz;
    // 60: mul r1.xyz, r1.wwww, cb2[3].xyzx
    r1.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 61: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 62: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 63: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 64: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 65: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 66: ret
    return output;
}

// source.character.static-map-native-1101.v1 / source program 2009235c6dcf2d4b9c2626bc5f8deabc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1101(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterLightConstants[0];
    source[4]=g_SourceCharacterLightConstants[2];
    source[5]=g_SourceCharacterLightConstants[3];
    source[6]=g_SourceCharacterLightConstants[4];
    source[7]=g_SourceCharacterLightConstants[5];
    source[8]=g_SourceCharacterLightConstants[6];
    source[9]=g_SourceCharacterLightConstants[7];
    source[10]=g_SourceCharacterLightConstants[8];
    source[11]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t2.yzxw, s2, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 2: mul r0.x, r0.x, cb0[9].x
    r0.x = ((r0.xxxx)*(source[9].xxxx)).x;
    // 3: mul r0.y, r0.y, cb0[7].x
    r0.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 4: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 5: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 6: mul r0.z, r0.z, cb0[9].y
    r0.z = ((r0.zzzz)*(source[9].yyyy)).z;
    // 7: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 8: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 9: max r0.x, r0.x, cb0[0].x
    r0.x = (max(r0.xxxx,source[0].xxxx)).x;
    // 10: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 11: add r0.z, -r0.x, l(1.000000)
    r0.z = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 12: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 13: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 14: mul r0.w, r0.w, cb0[7].y
    r0.w = ((r0.wwww)*(source[7].yyyy)).w;
    // 15: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 16: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 17: min r0.w, r0.y, l(1.000000)
    r0.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: mul r1.xyz, cb0[4].xyzx, cb0[6].yyyy
    r1.xyz = ((source[4].xyzx)*(source[6].yyyy)).xyz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 20: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 21: add r2.xy, r2.wwww, cb0[8].zxzz
    r2.xy = ((r2.wwww)+(source[8].zxzz)).xy;
    // 22: mul r3.xyz, r1.xyzx, cb0[6].zzzz
    r3.xyz = ((r1.xyzx)*(source[6].zzzz)).xyz;
    // 23: mad r1.xyz, cb0[6].wwww, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[6].wwww)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 24: mad r1.xyz, r0.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 25: add r3.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 26: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 27: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 28: mov_sat r0.w, cb0[7].z
    r0.w = (saturate(source[7].zzzz)).w;
    // 29: mad r3.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r3.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 30: mul r0.w, r0.w, l(0.080000)
    r0.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r1.w, v4.xyxx, t3.yzwx, s3, l(0.000000)
    r1.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 32: mad r2.x, r1.w, -r2.x, r2.x
    r2.x = ((r1.wwww)*(-(r2.xxxx))+(r2.xxxx)).x;
    // 33: add_sat r0.y, r0.y, r2.x
    r0.y = (saturate((r0.yyyy)+(r2.xxxx))).y;
    // 34: mul_sat r0.y, r0.y, cb2[3].w
    r0.y = (saturate((r0.yyyy)*(passValues[3].wwww))).y;
    // 35: mad r2.xzw, r0.yyyy, r3.xxyz, r0.wwww
    r2.xzw = ((r0.yyyy)*(r3.xxyz)+(r0.wwww)).xzw;
    // 36: max r3.xyz, r0.zzzz, r2.xzwx
    r3.xyz = (max(r0.zzzz,r2.xzwx)).xyz;
    // 37: add r3.xyz, -r2.xzwx, r3.xyzx
    r3.xyz = ((-(r2.xzwx))+(r3.xyzx)).xyz;
    // 38: dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 39: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 40: mul r4.xyz, r0.zzzz, v7.xyzx
    r4.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // 41: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 42: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 43: mad r5.xyz, v5.xyzx, r0.zzzz, r4.xyzx
    r5.xyz = ((v5.xyzx)*(r0.zzzz)+(r4.xyzx)).xyz;
    // 44: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 45: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 46: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 47: dp3_sat r0.w, r4.xyzx, r5.xyzx
    r0.w = (saturate(dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 48: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: mad r3.w, v5.z, r0.z, l(1.000000)
    r3.w = ((v5.zzzz)*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mul r6.xyz, r0.zzzz, v5.xyzx
    r6.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 51: min r0.z, r3.w, l(1.000000)
    r0.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: add r0.z, -r0.z, r0.w
    r0.z = ((-(r0.zzzz))+(r0.wwww)).z;
    // 53: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 54: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 55: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 56: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 57: mul r3.w, r0.z, r0.w
    r3.w = ((r0.zzzz)*(r0.wwww)).w;
    // 58: mad r0.z, -r0.w, r0.z, l(1.000000)
    r0.z = ((-(r0.wwww))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 59: mad r7.xyz, -r3.wwww, r2.xzwx, r2.xzwx
    r7.xyz = ((-(r3.wwww))*(r2.xzwx)+(r2.xzwx)).xyz;
    // 60: mul_sat r0.w, r2.z, l(50.000000)
    r0.w = (saturate((r2.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 61: mul r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)*(r0.wwww)).w;
    // 62: mad r3.xyz, r0.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 63: mad r2.xzw, r0.zzzz, r2.xxzw, r0.wwww
    r2.xzw = ((r0.zzzz)*(r2.xxzw)+(r0.wwww)).xzw;
    // 64: add r2.xzw, -r2.xxzw, l(1.000000, 0.000000, 1.000000, 1.000000)
    r2.xzw = ((-(r2.xxzw))+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 65: mul r2.xzw, r2.xxzw, r2.xxzw
    r2.xzw = ((r2.xxzw)*(r2.xxzw)).xzw;
    // 66: add r7.xyz, -r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r3.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 67: add r0.z, -r2.y, l(1.000000)
    r0.z = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 68: mad_sat r0.z, r1.w, r0.z, r2.y
    r0.z = (saturate((r1.wwww)*(r0.zzzz)+(r2.yyyy))).z;
    // 69: mad r0.z, -r0.z, cb0[2].x, l(1.000000)
    r0.z = ((-(r0.zzzz))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: mul r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)*(r0.xxxx)).w;
    // 71: mad r0.x, -r0.x, r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 72: mad r1.w, r0.w, l(0.350000), l(1.000000)
    r1.w = ((r0.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: div_sat r0.z, r0.z, r1.w
    r0.z = (saturate((r0.zzzz)/(r1.wwww))).z;
    // 74: mad r2.xyz, -r0.zzzz, r2.xzwx, r7.xyzx
    r2.xyz = ((-(r0.zzzz))*(r2.xzwx)+(r7.xyzx)).xyz;
    // 75: mad r7.xyz, -r1.xyzx, r0.yyyy, r1.xyzx
    r7.xyz = ((-(r1.xyzx))*(r0.yyyy)+(r1.xyzx)).xyz;
    // 76: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 77: mul r7.xyz, r7.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 78: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 79: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 80: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 81: dp2 r0.z, r7.xyxx, r7.xyxx
    r0.z = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).z;
    // 82: mul r7.xy, r7.xyxx, cb0[6].xxxx
    r7.xy = ((r7.xyxx)*(source[6].xxxx)).xy;
    // 83: mul r7.xy, r7.xyxx, v2.wwww
    r7.xy = ((r7.xyxx)*(v2.wwww)).xy;
    // 84: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 85: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 86: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 87: add r7.z, r0.z, l(0.000010)
    r7.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 88: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 89: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 90: div r7.xyz, r7.xyzx, r0.zzzz
    r7.xyz = ((r7.xyzx)/(r0.zzzz)).xyz;
    // 91: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 92: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 93: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 94: dp3 r0.z, r7.xyzx, r4.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 95: add r0.z, |r0.z|, l(0.000010)
    r0.z = ((abs(r0.zzzz))+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 96: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 97: mad r1.w, r0.z, r0.x, r0.w
    r1.w = ((r0.zzzz)*(r0.xxxx)+(r0.wwww)).w;
    // 98: dp3_sat r2.w, r7.xyzx, r6.xyzx
    r2.w = (saturate(dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 99: mad r6.xyz, r7.xyzx, cb0[1].xxxx, r6.xyzx
    r6.xyz = ((r7.xyzx)*(source[1].xxxx)+(r6.xyzx)).xyz;
    // 100: dp3_sat r3.w, r7.xyzx, r5.xyzx
    r3.w = (saturate(dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 101: mad r0.x, r2.w, r0.x, r0.w
    r0.x = ((r2.wwww)*(r0.xxxx)+(r0.wwww)).x;
    // 102: mul r0.xw, r0.xxxw, r0.zzzw
    r0.xw = ((r0.xxxw)*(r0.zzzw)).xw;
    // 103: mad r0.x, r2.w, r1.w, r0.x
    r0.x = ((r2.wwww)*(r1.wwww)+(r0.xxxx)).x;
    // 104: rcp r0.x, r0.x
    r0.x = (1.0/(r0.xxxx)).x;
    // 105: mad r0.z, r3.w, r0.w, -r3.w
    r0.z = ((r3.wwww)*(r0.wwww)+(-(r3.wwww))).z;
    // 106: mad r0.z, r0.z, r3.w, l(1.000000)
    r0.z = ((r0.zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 107: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 108: mul r0.z, r0.z, l(3.141593)
    r0.z = ((r0.zzzz)*(float4(3.141593,3.141593,3.141593,3.141593))).z;
    // 109: div r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)/(r0.zzzz)).z;
    // 110: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 111: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 112: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 113: add r0.z, r0.z, l(0.000100)
    r0.z = ((r0.zzzz)+(float4(0.000100,0.000100,0.000100,0.000100))).z;
    // 114: div r0.z, l(3.000000), r0.z
    r0.z = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.zzzz)).z;
    // 115: min r0.x, r0.z, r0.x
    r0.x = (min(r0.zzzz,r0.xxxx)).x;
    // 116: mad r0.xzw, r0.xxxx, r3.xxyz, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r3.xxyz)+(r2.xxyz)).xzw;
    // 117: mul r0.xzw, r2.wwww, r0.xxzw
    r0.xzw = ((r2.wwww)*(r0.xxzw)).xzw;
    // 118: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 119: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 120: mul r2.xyz, r1.wwww, r6.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 121: dp3_sat r1.w, r4.xyzx, -r2.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(-(r2.xyzx)).xyz).xxxx)).w;
    // 122: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 123: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 124: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 125: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 126: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 127: add_sat r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (saturate((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 128: add_sat r2.xyz, r2.xyzx, cb0[10].yyyy
    r2.xyz = (saturate((r2.xyzx)+(source[10].yyyy))).xyz;
    // 129: mul r2.w, r2.x, cb0[10].z
    r2.w = ((r2.xxxx)*(source[10].zzzz)).w;
    // 130: mul_sat r2.xyz, r2.xyzx, cb0[5].xyzx
    r2.xyz = (saturate((r2.xyzx)*(source[5].xyzx))).xyz;
    // 131: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 132: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 133: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 134: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 135: mad r0.xyz, r0.xzwx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xzwx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 136: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 137: mov o0.w, l(1.000000)
    output.targets[0].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 138: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 139: ret
    return output;
}

// source.character.static-map-native-1102.v1 / source program 1dff528b246dd64891246ab212ae6de1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1102(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterLightConstants[0];
    source[4]=g_SourceCharacterLightConstants[2];
    source[5]=g_SourceCharacterLightConstants[3];
    source[6]=g_SourceCharacterLightConstants[4];
    source[7]=g_SourceCharacterLightConstants[5];
    source[8]=g_SourceCharacterLightConstants[6];
    source[9]=g_SourceCharacterLightConstants[7];
    source[10]=g_SourceCharacterLightConstants[8];
    source[11]=g_SourceCharacterLightConstants[9];
    source[12]=g_SourceCharacterLightConstants[10];
    source[13]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
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
    // 9: mad r1.xyz, cb0[7].zzzz, r1.xyzx, r0.xyzx
    r1.xyz = ((source[7].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 10: mul r2.xyz, cb0[4].xyzx, cb0[7].wwww
    r2.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 11: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 12: mul r2.xyz, cb0[5].xyzx, cb0[8].xxxx
    r2.xyz = ((source[5].xyzx)*(source[8].xxxx)).xyz;
    // 13: mad r0.xyz, r2.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t2.yzxw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 15: mul r1.w, r2.y, cb0[8].y
    r1.w = ((r2.yyyy)*(source[8].yyyy)).w;
    // 16: mul r2.x, r2.x, cb0[10].w
    r2.x = ((r2.xxxx)*(source[10].wwww)).x;
    // 17: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 18: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 19: mul r2.y, r2.y, cb0[8].z
    r2.y = ((r2.yyyy)*(source[8].zzzz)).y;
    // 20: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 21: movc r1.w, r1.w, l(0), r2.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).w;
    // 22: min r2.y, r1.w, l(1.000000)
    r2.y = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 23: mad r0.xyz, r2.yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 24: mul r1.xyz, r0.xyzx, cb0[8].wwww
    r1.xyz = ((r0.xyzx)*(source[8].wwww)).xyz;
    // 25: mad r0.xyz, cb0[9].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[9].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 26: mad r0.xyz, r2.yyyy, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.yyyy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 27: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 28: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 29: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 30: add r1.x, r0.w, cb0[10].y
    r1.x = ((r0.wwww)+(source[10].yyyy)).x;
    // 31: add r0.w, r0.w, cb0[9].w
    r0.w = ((r0.wwww)+(source[9].wwww)).w;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.y, v4.xyxx, t3.yxzw, s3, l(0.000000)
    r1.y = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 33: mad r1.x, r1.y, -r1.x, r1.x
    r1.x = ((r1.yyyy)*(-(r1.xxxx))+(r1.xxxx)).x;
    // 34: add_sat r1.x, r1.x, r1.w
    r1.x = (saturate((r1.xxxx)+(r1.wwww))).x;
    // 35: mul_sat r1.x, r1.x, cb2[3].w
    r1.x = (saturate((r1.xxxx)*(passValues[3].wwww))).x;
    // 36: mad r2.yzw, -r0.xxyz, r1.xxxx, r0.xxyz
    r2.yzw = ((-(r0.xxyz))*(r1.xxxx)+(r0.xxyz)).yzw;
    // 37: mul r2.yzw, r2.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r2.yzw = ((r2.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 38: add r1.z, -r0.w, l(1.000000)
    r1.z = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 39: mad_sat r0.w, r1.y, r1.z, r0.w
    r0.w = (saturate((r1.yyyy)*(r1.zzzz)+(r0.wwww))).w;
    // 40: mad r0.w, -r0.w, cb0[2].x, l(1.000000)
    r0.w = ((-(r0.wwww))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: log r1.y, |r2.x|
    r1.y = (log2(abs(r2.xxxx))).y;
    // 42: lt r1.z, |r2.x|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 43: mul r1.y, r1.y, cb0[11].x
    r1.y = ((r1.yyyy)*(source[11].xxxx)).y;
    // 44: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 45: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 46: max r1.y, r1.y, cb0[0].x
    r1.y = (max(r1.yyyy,source[0].xxxx)).y;
    // 47: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 48: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 49: mad r1.w, r1.z, l(0.350000), l(1.000000)
    r1.w = ((r1.zzzz)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: div_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)/(r1.wwww))).w;
    // 51: add r1.w, -r1.y, l(1.000000)
    r1.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mad r1.y, -r1.y, r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: mov_sat r2.x, cb0[9].y
    r2.x = (saturate(source[9].yyyy)).x;
    // 54: mad r3.xyz, -r2.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r3.xyz = ((-(r2.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 55: mul r2.x, r2.x, l(0.080000)
    r2.x = ((r2.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 56: mad r3.xyz, r1.xxxx, r3.xyzx, r2.xxxx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)+(r2.xxxx)).xyz;
    // 57: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 58: max r4.xyz, r1.wwww, r3.xyzx
    r4.xyz = (max(r1.wwww,r3.xyzx)).xyz;
    // 59: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 60: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 61: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 62: mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 63: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 64: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 65: mad r6.xyz, v5.xyzx, r1.wwww, r5.xyzx
    r6.xyz = ((v5.xyzx)*(r1.wwww)+(r5.xyzx)).xyz;
    // 66: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 67: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 68: mul r6.xyz, r2.xxxx, r6.xyzx
    r6.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 69: dp3_sat r2.x, r5.xyzx, r6.xyzx
    r2.x = (saturate(dot((r5.xyzx).xyz,(r6.xyzx).xyz).xxxx)).x;
    // 70: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 71: mad r3.w, v5.z, r1.w, l(1.000000)
    r3.w = ((v5.zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: mul r7.xyz, r1.wwww, v5.xyzx
    r7.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 73: min r1.w, r3.w, l(1.000000)
    r1.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 74: add r1.w, -r1.w, r2.x
    r1.w = ((-(r1.wwww))+(r2.xxxx)).w;
    // 75: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul r2.x, r1.w, r1.w
    r2.x = ((r1.wwww)*(r1.wwww)).x;
    // 78: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 79: mul r3.w, r1.w, r2.x
    r3.w = ((r1.wwww)*(r2.xxxx)).w;
    // 80: mad r1.w, -r2.x, r1.w, l(1.000000)
    r1.w = ((-(r2.xxxx))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mad r8.xyz, -r3.wwww, r3.xyzx, r3.xyzx
    r8.xyz = ((-(r3.wwww))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 82: mul_sat r2.x, r3.y, l(50.000000)
    r2.x = (saturate((r3.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).x;
    // 83: mul r2.x, r3.w, r2.x
    r2.x = ((r3.wwww)*(r2.xxxx)).x;
    // 84: mad r4.xyz, r2.xxxx, r4.xyzx, r8.xyzx
    r4.xyz = ((r2.xxxx)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 85: mad r3.xyz, r1.wwww, r3.xyzx, r2.xxxx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r2.xxxx)).xyz;
    // 86: add r3.xyz, -r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r3.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 87: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 88: add r8.xyz, -r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r4.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 89: mad r3.xyz, -r0.wwww, r3.xyzx, r8.xyzx
    r3.xyz = ((-(r0.wwww))*(r3.xyzx)+(r8.xyzx)).xyz;
    // 90: mul r2.xyz, r2.yzwy, r3.xyzx
    r2.xyz = ((r2.yzwy)*(r3.xyzx)).xyz;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 92: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 93: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 94: mul r3.xy, r3.xyxx, cb0[7].xxxx
    r3.xy = ((r3.xyxx)*(source[7].xxxx)).xy;
    // 95: mul r3.xy, r3.xyxx, v2.wwww
    r3.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 96: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 98: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 99: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 100: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 101: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 102: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 103: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 104: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 105: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 106: dp3 r0.w, r3.xyzx, r5.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 107: add r0.w, |r0.w|, l(0.000010)
    r0.w = ((abs(r0.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 108: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mad r1.w, r0.w, r1.y, r1.z
    r1.w = ((r0.wwww)*(r1.yyyy)+(r1.zzzz)).w;
    // 110: dp3_sat r2.w, r3.xyzx, r7.xyzx
    r2.w = (saturate(dot((r3.xyzx).xyz,(r7.xyzx).xyz).xxxx)).w;
    // 111: mad r7.xyz, r3.xyzx, cb0[1].xxxx, r7.xyzx
    r7.xyz = ((r3.xyzx)*(source[1].xxxx)+(r7.xyzx)).xyz;
    // 112: dp3_sat r3.x, r3.xyzx, r6.xyzx
    r3.x = (saturate(dot((r3.xyzx).xyz,(r6.xyzx).xyz).xxxx)).x;
    // 113: mad r1.y, r2.w, r1.y, r1.z
    r1.y = ((r2.wwww)*(r1.yyyy)+(r1.zzzz)).y;
    // 114: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 115: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 116: mad r0.w, r2.w, r1.w, r0.w
    r0.w = ((r2.wwww)*(r1.wwww)+(r0.wwww)).w;
    // 117: rcp r0.w, r0.w
    r0.w = (1.0/(r0.wwww)).w;
    // 118: mad r1.y, r3.x, r1.z, -r3.x
    r1.y = ((r3.xxxx)*(r1.zzzz)+(-(r3.xxxx))).y;
    // 119: mad r1.y, r1.y, r3.x, l(1.000000)
    r1.y = ((r1.yyyy)*(r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 120: mul r1.y, r1.y, r1.y
    r1.y = ((r1.yyyy)*(r1.yyyy)).y;
    // 121: mul r1.y, r1.y, l(3.141593)
    r1.y = ((r1.yyyy)*(float4(3.141593,3.141593,3.141593,3.141593))).y;
    // 122: div r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)/(r1.yyyy)).y;
    // 123: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 124: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 125: dp3 r1.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 126: add r1.y, r1.y, l(0.000100)
    r1.y = ((r1.yyyy)+(float4(0.000100,0.000100,0.000100,0.000100))).y;
    // 127: div r1.y, l(3.000000), r1.y
    r1.y = ((float4(3.000000,3.000000,3.000000,3.000000))/(r1.yyyy)).y;
    // 128: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 129: mad r1.yzw, r0.wwww, r4.xxyz, r2.xxyz
    r1.yzw = ((r0.wwww)*(r4.xxyz)+(r2.xxyz)).yzw;
    // 130: mul r1.yzw, r2.wwww, r1.yyzw
    r1.yzw = ((r2.wwww)*(r1.yyzw)).yzw;
    // 131: dp3 r0.w, r7.xyzx, r7.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 132: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 133: mul r2.xyz, r0.wwww, r7.xyzx
    r2.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 134: dp3_sat r0.w, r5.xyzx, -r2.xyzx
    r0.w = (saturate(dot((r5.xyzx).xyz,(-(r2.xyzx)).xyz).xxxx)).w;
    // 135: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 136: mul r0.w, r0.w, cb0[1].y
    r0.w = ((r0.wwww)*(source[1].yyyy)).w;
    // 137: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 138: mad_sat r0.w, r0.w, cb0[1].w, cb0[1].z
    r0.w = (saturate((r0.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 139: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 140: add_sat r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (saturate((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 141: add_sat r2.xyz, r2.xyzx, cb0[12].xxxx
    r2.xyz = (saturate((r2.xyzx)+(source[12].xxxx))).xyz;
    // 142: mul r2.w, r2.x, cb0[12].y
    r2.w = ((r2.xxxx)*(source[12].yyyy)).w;
    // 143: mul_sat r2.xyz, r2.xyzx, cb0[6].xyzx
    r2.xyz = (saturate((r2.xyzx)*(source[6].xyzx))).xyz;
    // 144: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 145: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 146: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 147: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 148: mad r0.xyz, r1.yzwy, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r1.yzwy)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 149: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 150: mov o0.w, l(1.000000)
    output.targets[0].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 151: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 152: ret
    return output;
}

// source.character.static-map-native-1103.v1 / source program 92cd313389d4184e9009f0de3c840fa9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1103(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[6];
    source[3]=g_SourceCharacterLightConstants[7];
    source[4]=g_SourceCharacterLightConstants[8];
    source[5]=g_SourceCharacterLightConstants[9];
    source[6]=g_SourceCharacterLightConstants[13];
    source[7]=g_SourceCharacterLightConstants[14];
    source[8]=g_SourceCharacterLightConstants[15];
    source[9]=g_SourceCharacterLightConstants[16];
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
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 51: mad r0.xy, cb0[6].xxxx, r1.xyxx, r0.xyxx
    r0.xy = ((source[6].xxxx)*(r1.xyxx)+(r0.xyxx)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 53: mul r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[2].xyzx)).xyz;
    // 54: mad r0.xyz, cb0[6].yyyy, r0.xyzx, r0.xyzx
    r0.xyz = ((source[6].yyyy)*(r0.xyzx)+(r0.xyzx)).xyz;
    // 55: add r0.xyz, r0.xyzx, -cb0[6].yyyy
    r0.xyz = ((r0.xyzx)+(-(source[6].yyyy))).xyz;
    // 56: mov_sat r1.xyz, r0.xyzx
    r1.xyz = (saturate(r0.xyzx)).xyz;
    // 57: mul r2.w, r4.z, cb0[6].z
    r2.w = ((r4.zzzz)*(source[6].zzzz)).w;
    // 58: mul r5.xyz, cb0[3].xyzx, cb0[6].wwww
    r5.xyz = ((source[3].xyzx)*(source[6].wwww)).xyz;
    // 59: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 60: mul r9.xyz, cb0[4].xyzx, cb0[7].xxxx
    r9.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t3.yzxw, s4, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 62: mul r3.w, r10.y, cb0[7].y
    r3.w = ((r10.yyyy)*(source[7].yyyy)).w;
    // 63: lt r4.z, |r3.w|, l(0.000001)
    r4.z = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 64: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 65: mul r3.w, r3.w, cb0[7].z
    r3.w = ((r3.wwww)*(source[7].zzzz)).w;
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
    // 77: mul r1.xyz, r0.xyzx, cb0[7].wwww
    r1.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 78: mad r0.xyz, cb0[8].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[8].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 79: mad r0.xyz, r4.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r4.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 80: mul r0.xyz, r8.xyzx, r0.xyzx
    r0.xyz = ((r8.xyzx)*(r0.xyzx)).xyz;
    // 81: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 82: mov_sat r1.x, cb0[8].y
    r1.x = (saturate(source[8].yyyy)).x;
    // 83: mul_sat r1.y, r3.w, cb2[3].w
    r1.y = (saturate((r3.wwww)*(passValues[3].wwww))).y;
    // 84: mul r1.z, r10.x, cb0[8].w
    r1.z = ((r10.xxxx)*(source[8].wwww)).z;
    // 85: lt r2.w, |r1.z|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 86: log r1.z, |r1.z|
    r1.z = (log2(abs(r1.zzzz))).z;
    // 87: mul r1.z, r1.z, cb0[9].x
    r1.z = ((r1.zzzz)*(source[9].xxxx)).z;
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

// source.character.static-map-native-1104.v1 / source program 6fc4bf0926956b489c2819855b435769
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1104(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[5];
    source[4]=g_SourceCharacterLightConstants[6];
    source[5]=g_SourceCharacterLightConstants[7];
    source[6]=g_SourceCharacterLightConstants[8];
    source[7]=g_SourceCharacterLightConstants[9];
    source[8]=g_SourceCharacterLightConstants[10];
    source[9]=g_SourceCharacterLightConstants[11];
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
    // 16: mul r4.xy, v4.xyxx, cb0[2].xyxx
    r4.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t0.xywz, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 18: mad r4.zw, r5.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r4.zw = ((r5.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 19: dp2 r1.w, r4.zwzz, r4.zwzz
    r1.w = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).w;
    // 20: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 24: mul r4.zw, r4.zzzw, cb0[5].wwww
    r4.zw = ((r4.zzzw)*(source[5].wwww)).zw;
    // 25: mul r6.xy, r4.zwzz, v2.wwww
    r6.xy = ((r4.zwzz)*(v2.wwww)).xy;
    // 26: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 27: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 28: div r5.xyw, r6.xyxz, r1.wwww
    r5.xyw = ((r6.xyxz)/(r1.wwww)).xyw;
    // 29: dp3 r1.w, r5.xywx, r5.xywx
    r1.w = (dot((r5.xywx).xyz,(r5.xywx).xyz).xxxx).w;
    // 30: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 31: mul r5.xyw, r1.wwww, r5.xyxw
    r5.xyw = ((r1.wwww)*(r5.xyxw)).xyw;
    // 32: dp3 r1.w, r5.xywx, r2.xyzx
    r1.w = (dot((r5.xywx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 33: mul r6.xyz, r1.wwww, r5.xywx
    r6.xyz = ((r1.wwww)*(r5.xywx)).xyz;
    // 34: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 35: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].xxxx)) * 0xffffffffu)).w;
    // 36: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 37: div r4.zw, v8.xxxy, v8.wwww
    r4.zw = ((v8.xxxy)/(v8.wwww)).zw;
    // 38: mad r4.zw, r4.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r4.zw = ((r4.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 39: sample_indexable(texture2d)(float,float,float,float) r7.xyz, r4.zwzz, t4.xyzw, s0
    r7.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 40: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 41: else
    } else {
    // 42: mov r7.xyz, l(1.000000,1.000000,1.000000,0)
    r7.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 43: endif
    }
    // 44: add r8.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, r4.xyxx, t1.xyzw, s2, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 46: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 47: add r10.xyz, -r9.xyzx, r2.wwww
    r10.xyz = ((-(r9.xyzx))+(r2.wwww)).xyz;
    // 48: mad r9.xyz, cb0[6].zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((source[6].zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 49: mul r10.xyz, cb0[3].xyzx, cb0[6].wwww
    r10.xyz = ((source[3].xyzx)*(source[6].wwww)).xyz;
    // 50: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 51: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 52: dp3 r1.y, r0.xyzx, r6.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 53: mul r0.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 54: mad r0.xy, cb0[7].xxxx, r1.xyxx, r0.xyxx
    r0.xy = ((source[7].xxxx)*(r1.xyxx)+(r0.xyxx)).xy;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 56: mul r1.x, r5.z, cb0[7].y
    r1.x = ((r5.zzzz)*(source[7].yyyy)).x;
    // 57: mad r0.xyz, r0.xyzx, cb0[4].xyzx, -r9.xyzx
    r0.xyz = ((r0.xyzx)*(source[4].xyzx)+(-(r9.xyzx))).xyz;
    // 58: mad r0.xyz, r1.xxxx, r0.xyzx, r9.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r9.xyzx)).xyz;
    // 59: mul r1.xyz, r0.xyzx, cb0[7].zzzz
    r1.xyz = ((r0.xyzx)*(source[7].zzzz)).xyz;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r4.xyxx, t3.yzxw, s4, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 61: mul r2.w, r4.y, cb0[8].x
    r2.w = ((r4.yyyy)*(source[8].xxxx)).w;
    // 62: lt r3.w, |r2.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 63: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 64: mul r2.w, r2.w, cb0[8].y
    r2.w = ((r2.wwww)*(source[8].yyyy)).w;
    // 65: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 66: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 67: min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 68: mad r0.xyz, cb0[7].wwww, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[7].wwww)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 69: mad r0.xyz, r3.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 70: mul r0.xyz, r8.xyzx, r0.xyzx
    r0.xyz = ((r8.xyzx)*(r0.xyzx)).xyz;
    // 71: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 72: mov_sat r1.x, cb0[8].z
    r1.x = (saturate(source[8].zzzz)).x;
    // 73: mul_sat r1.y, r2.w, cb2[3].w
    r1.y = (saturate((r2.wwww)*(passValues[3].wwww))).y;
    // 74: mul r1.z, r4.x, cb0[9].x
    r1.z = ((r4.xxxx)*(source[9].xxxx)).z;
    // 75: lt r2.w, |r1.z|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 76: log r1.z, |r1.z|
    r1.z = (log2(abs(r1.zzzz))).z;
    // 77: mul r1.z, r1.z, cb0[9].y
    r1.z = ((r1.zzzz)*(source[9].yyyy)).z;
    // 78: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 79: movc r1.z, r2.w, l(0), r1.z
    r1.z = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 80: max r1.z, r1.z, cb0[0].w
    r1.z = (max(r1.zzzz,source[0].wwww)).z;
    // 81: mad r4.xyz, v5.xyzx, r0.wwww, r2.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 82: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 83: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 84: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 85: dp3_sat r2.w, r5.xywx, r4.xyzx
    r2.w = (saturate(dot((r5.xywx).xyz,(r4.xyzx).xyz).xxxx)).w;
    // 86: add r1.w, |r1.w|, l(0.000010)
    r1.w = ((abs(r1.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 87: min r1.zw, r1.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = (min(r1.zzzw,float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 88: dp3_sat r3.x, r5.xywx, r3.xyzx
    r3.x = (saturate(dot((r5.xywx).xyz,(r3.xyzx).xyz).xxxx)).x;
    // 89: dp3_sat r2.x, r2.xyzx, r4.xyzx
    r2.x = (saturate(dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 90: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 91: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 93: add r0.w, -r0.w, r2.x
    r0.w = ((-(r0.wwww))+(r2.xxxx)).w;
    // 94: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mad r2.xyz, -r0.xyzx, r1.yyyy, r0.xyzx
    r2.xyz = ((-(r0.xyzx))*(r1.yyyy)+(r0.xyzx)).xyz;
    // 96: mul r3.y, r1.z, r1.z
    r3.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 97: mul r3.z, r3.y, r3.y
    r3.z = ((r3.yyyy)*(r3.yyyy)).z;
    // 98: mad r3.w, r2.w, r3.z, -r2.w
    r3.w = ((r2.wwww)*(r3.zzzz)+(-(r2.wwww))).w;
    // 99: mad r2.w, r3.w, r2.w, l(1.000000)
    r2.w = ((r3.wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 101: mul r2.xyzw, r2.xyzw, l(0.318310, 0.318310, 0.318310, 3.141593)
    r2.xyzw = ((r2.xyzw)*(float4(0.318310,0.318310,0.318310,3.141593))).xyzw;
    // 102: div r2.w, r3.z, r2.w
    r2.w = ((r3.zzzz)/(r2.wwww)).w;
    // 103: mad r3.z, -r1.z, r1.z, l(1.000000)
    r3.z = ((-(r1.zzzz))*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 104: mad r3.w, r1.w, r3.z, r3.y
    r3.w = ((r1.wwww)*(r3.zzzz)+(r3.yyyy)).w;
    // 105: mad r3.y, r3.x, r3.z, r3.y
    r3.y = ((r3.xxxx)*(r3.zzzz)+(r3.yyyy)).y;
    // 106: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 107: mad r1.w, r3.x, r3.w, r1.w
    r1.w = ((r3.xxxx)*(r3.wwww)+(r1.wwww)).w;
    // 108: rcp r1.w, r1.w
    r1.w = (1.0/(r1.wwww)).w;
    // 109: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 110: mul r2.w, r1.x, l(0.080000)
    r2.w = ((r1.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 111: mad r0.xyz, -r1.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r1.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 112: mad r0.xyz, r1.yyyy, r0.xyzx, r2.wwww
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(r2.wwww)).xyz;
    // 113: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 115: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 116: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 117: mul_sat r1.x, r0.y, l(50.000000)
    r1.x = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).x;
    // 118: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 119: add r1.y, -r1.z, l(1.000000)
    r1.y = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 120: max r3.yzw, r0.xxyz, r1.yyyy
    r3.yzw = (max(r0.xxyz,r1.yyyy)).yzw;
    // 121: add r3.yzw, -r0.xxyz, r3.yyzw
    r3.yzw = ((-(r0.xxyz))+(r3.yyzw)).yzw;
    // 122: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 123: mad r0.xyz, r1.xxxx, r3.yzwy, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r3.yzwy)+(r0.xyzx)).xyz;
    // 124: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 126: mul r1.x, r1.w, l(0.500000)
    r1.x = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 127: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 128: min r0.w, r0.w, r1.x
    r0.w = (min(r0.wwww,r1.xxxx)).w;
    // 129: mul r1.xyz, r0.xyzx, r0.wwww
    r1.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 130: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 131: mad r0.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 132: mul r0.xyz, r3.xxxx, r0.xyzx
    r0.xyz = ((r3.xxxx)*(r0.xyzx)).xyz;
    // 133: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 134: mul r0.xyz, r7.xyzx, r0.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)).xyz;
    // 135: mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 136: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 137: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 138: ret
    return output;
}

// source.character.static-map-native-1105.v1 / source program 7892933bb13ed2498de0d1421f33032b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1105(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
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
    // 34: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).w;
    // 35: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 36: div r6.xy, v8.xyxx, v8.wwww
    r6.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 37: mad r6.xy, r6.xyxx, cb2[0].xyxx, cb2[0].wzww
    r6.xy = ((r6.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: sample_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s0
    r6.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 39: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 40: else
    } else {
    // 41: mov r6.xyz, l(1.000000,1.000000,1.000000,0)
    r6.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 42: endif
    }
    // 43: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 44: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 45: dp3 r1.y, r0.xyzx, r5.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 46: mul r0.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 47: mad r0.xy, cb0[5].yyyy, r1.xyxx, r0.xyxx
    r0.xy = ((source[5].yyyy)*(r1.xyxx)+(r0.xyxx)).xy;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 49: mul r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[2].xyzx)).xyz;
    // 50: mad r0.xyz, cb0[5].zzzz, r0.xyzx, r0.xyzx
    r0.xyz = ((source[5].zzzz)*(r0.xyzx)+(r0.xyzx)).xyz;
    // 51: add r0.xyz, r0.xyzx, -cb0[5].zzzz
    r0.xyz = ((r0.xyzx)+(-(source[5].zzzz))).xyz;
    // 52: mov_sat r1.xyz, r0.xyzx
    r1.xyz = (saturate(r0.xyzx)).xyz;
    // 53: mul r2.w, r4.z, cb0[5].w
    r2.w = ((r4.zzzz)*(source[5].wwww)).w;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 55: mul r8.xyz, cb0[3].xyzx, cb0[6].xxxx
    r8.xyz = ((source[3].xyzx)*(source[6].xxxx)).xyz;
    // 56: mul r8.xyz, r5.xyzx, r8.xyzx
    r8.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 57: mul r9.xyz, cb0[4].xyzx, cb0[6].yyyy
    r9.xyz = ((source[4].xyzx)*(source[6].yyyy)).xyz;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t3.yzxw, s4, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 59: mul r3.w, r10.y, cb0[6].z
    r3.w = ((r10.yyyy)*(source[6].zzzz)).w;
    // 60: lt r4.z, |r3.w|, l(0.000001)
    r4.z = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 61: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 62: mul r3.w, r3.w, cb0[6].w
    r3.w = ((r3.wwww)*(source[6].wwww)).w;
    // 63: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 64: movc r3.w, r4.z, l(0), r3.w
    r3.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 65: min r4.z, r3.w, l(1.000000)
    r4.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 66: mad r5.xyz, r9.xyzx, r5.xyzx, -r8.xyzx
    r5.xyz = ((r9.xyzx)*(r5.xyzx)+(-(r8.xyzx))).xyz;
    // 67: mad r5.xyz, r4.zzzz, r5.xyzx, r8.xyzx
    r5.xyz = ((r4.zzzz)*(r5.xyzx)+(r8.xyzx)).xyz;
    // 68: mad r1.xyz, r2.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 69: mov_sat r0.xyz, -r0.xyzx
    r0.xyz = (saturate(-(r0.xyzx))).xyz;
    // 70: mad r0.xyz, -r2.wwww, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r2.wwww))*(r0.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 71: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 72: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 73: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 74: mul r1.xyz, r0.xyzx, cb0[7].xxxx
    r1.xyz = ((r0.xyzx)*(source[7].xxxx)).xyz;
    // 75: mad r0.xyz, cb0[7].yyyy, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[7].yyyy)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 76: mad r0.xyz, r4.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r4.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 77: mul r0.xyz, r7.xyzx, r0.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)).xyz;
    // 78: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 79: mov_sat r1.x, cb0[7].z
    r1.x = (saturate(source[7].zzzz)).x;
    // 80: mul_sat r1.y, r3.w, cb2[3].w
    r1.y = (saturate((r3.wwww)*(passValues[3].wwww))).y;
    // 81: mul r1.z, r10.x, cb0[8].x
    r1.z = ((r10.xxxx)*(source[8].xxxx)).z;
    // 82: lt r2.w, |r1.z|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 83: log r1.z, |r1.z|
    r1.z = (log2(abs(r1.zzzz))).z;
    // 84: mul r1.z, r1.z, cb0[8].y
    r1.z = ((r1.zzzz)*(source[8].yyyy)).z;
    // 85: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 86: movc r1.z, r2.w, l(0), r1.z
    r1.z = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 87: max r1.z, r1.z, cb0[0].w
    r1.z = (max(r1.zzzz,source[0].wwww)).z;
    // 88: mad r5.xyz, v5.xyzx, r0.wwww, r2.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 89: dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 90: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 91: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 92: dp3_sat r2.w, r4.xywx, r5.xyzx
    r2.w = (saturate(dot((r4.xywx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 93: add r1.w, |r1.w|, l(0.000010)
    r1.w = ((abs(r1.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 94: min r1.zw, r1.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = (min(r1.zzzw,float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 95: dp3_sat r3.x, r4.xywx, r3.xyzx
    r3.x = (saturate(dot((r4.xywx).xyz,(r3.xyzx).xyz).xxxx)).x;
    // 96: dp3_sat r2.x, r2.xyzx, r5.xyzx
    r2.x = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 97: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 100: add r0.w, -r0.w, r2.x
    r0.w = ((-(r0.wwww))+(r2.xxxx)).w;
    // 101: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r2.xyz, -r0.xyzx, r1.yyyy, r0.xyzx
    r2.xyz = ((-(r0.xyzx))*(r1.yyyy)+(r0.xyzx)).xyz;
    // 103: mul r3.y, r1.z, r1.z
    r3.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 104: mul r3.z, r3.y, r3.y
    r3.z = ((r3.yyyy)*(r3.yyyy)).z;
    // 105: mad r3.w, r2.w, r3.z, -r2.w
    r3.w = ((r2.wwww)*(r3.zzzz)+(-(r2.wwww))).w;
    // 106: mad r2.w, r3.w, r2.w, l(1.000000)
    r2.w = ((r3.wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 108: mul r2.xyzw, r2.xyzw, l(0.318310, 0.318310, 0.318310, 3.141593)
    r2.xyzw = ((r2.xyzw)*(float4(0.318310,0.318310,0.318310,3.141593))).xyzw;
    // 109: div r2.w, r3.z, r2.w
    r2.w = ((r3.zzzz)/(r2.wwww)).w;
    // 110: mad r3.z, -r1.z, r1.z, l(1.000000)
    r3.z = ((-(r1.zzzz))*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 111: mad r3.w, r1.w, r3.z, r3.y
    r3.w = ((r1.wwww)*(r3.zzzz)+(r3.yyyy)).w;
    // 112: mad r3.y, r3.x, r3.z, r3.y
    r3.y = ((r3.xxxx)*(r3.zzzz)+(r3.yyyy)).y;
    // 113: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 114: mad r1.w, r3.x, r3.w, r1.w
    r1.w = ((r3.xxxx)*(r3.wwww)+(r1.wwww)).w;
    // 115: rcp r1.w, r1.w
    r1.w = (1.0/(r1.wwww)).w;
    // 116: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 117: mul r2.w, r1.x, l(0.080000)
    r2.w = ((r1.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 118: mad r0.xyz, -r1.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r1.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 119: mad r0.xyz, r1.yyyy, r0.xyzx, r2.wwww
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(r2.wwww)).xyz;
    // 120: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 122: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 123: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 124: mul_sat r1.x, r0.y, l(50.000000)
    r1.x = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).x;
    // 125: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 126: add r1.y, -r1.z, l(1.000000)
    r1.y = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 127: max r3.yzw, r0.xxyz, r1.yyyy
    r3.yzw = (max(r0.xxyz,r1.yyyy)).yzw;
    // 128: add r3.yzw, -r0.xxyz, r3.yyzw
    r3.yzw = ((-(r0.xxyz))+(r3.yyzw)).yzw;
    // 129: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 130: mad r0.xyz, r1.xxxx, r3.yzwy, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r3.yzwy)+(r0.xyzx)).xyz;
    // 131: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 132: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 133: mul r1.x, r1.w, l(0.500000)
    r1.x = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 134: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 135: min r0.w, r0.w, r1.x
    r0.w = (min(r0.wwww,r1.xxxx)).w;
    // 136: mul r1.xyz, r0.xyzx, r0.wwww
    r1.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 137: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 138: mad r0.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 139: mul r0.xyz, r3.xxxx, r0.xyzx
    r0.xyz = ((r3.xxxx)*(r0.xyzx)).xyz;
    // 140: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 141: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 142: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 143: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 144: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 145: ret
    return output;
}

// source.character.static-map-native-1106.v1 / source program 2bfdccc125b4084f860064bf82e997c7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1106(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterLightConstants[0];
    source[4]=g_SourceCharacterLightConstants[2];
    source[5]=g_SourceCharacterLightConstants[3];
    source[6]=g_SourceCharacterLightConstants[4];
    source[7]=g_SourceCharacterLightConstants[5];
    source[8]=g_SourceCharacterLightConstants[6];
    source[9]=g_SourceCharacterLightConstants[7];
    source[10]=g_SourceCharacterLightConstants[8];
    source[11]=g_SourceCharacterLightConstants[9];
    source[12]=g_SourceCharacterLightConstants[10];
    source[13]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t2.yzxw, s2, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 2: mul r0.y, r0.y, cb0[8].y
    r0.y = ((r0.yyyy)*(source[8].yyyy)).y;
    // 3: mul r0.x, r0.x, cb0[10].w
    r0.x = ((r0.xxxx)*(source[10].wwww)).x;
    // 4: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 5: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 6: mul r0.z, r0.z, cb0[8].z
    r0.z = ((r0.zzzz)*(source[8].zzzz)).z;
    // 7: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 8: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 9: min r0.z, r0.y, l(1.000000)
    r0.z = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 10: mul r1.xyz, cb0[5].xyzx, cb0[8].xxxx
    r1.xyz = ((source[5].xyzx)*(source[8].xxxx)).xyz;
    // 11: mul r2.xyz, cb0[4].xyzx, cb0[7].wwww
    r2.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 13: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 14: add r4.xyz, -r3.xyzx, r0.wwww
    r4.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 15: mad r4.xyz, cb0[7].zzzz, r4.xyzx, r3.xyzx
    r4.xyz = ((source[7].zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 16: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 17: mad r1.xyz, r1.xyzx, r3.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(-(r2.xyzx))).xyz;
    // 18: mad r1.xyz, r0.zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 19: mul r2.xyz, r1.xyzx, cb0[8].wwww
    r2.xyz = ((r1.xyzx)*(source[8].wwww)).xyz;
    // 20: mad r1.xyz, cb0[9].xxxx, r1.xyzx, -r2.xyzx
    r1.xyz = ((source[9].xxxx)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 21: mad r1.xyz, r0.zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 22: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 23: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 24: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 25: mov_sat r0.z, cb0[9].y
    r0.z = (saturate(source[9].yyyy)).z;
    // 26: mad r2.xyz, -r0.zzzz, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r2.xyz = ((-(r0.zzzz))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 27: mul r0.z, r0.z, l(0.080000)
    r0.z = ((r0.zzzz)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 28: add r0.w, r3.w, cb0[10].y
    r0.w = ((r3.wwww)+(source[10].yyyy)).w;
    // 29: add r1.w, r3.w, cb0[9].w
    r1.w = ((r3.wwww)+(source[9].wwww)).w;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r2.w, v4.xyxx, t3.yzwx, s3, l(0.000000)
    r2.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 31: mad r0.w, r2.w, -r0.w, r0.w
    r0.w = ((r2.wwww)*(-(r0.wwww))+(r0.wwww)).w;
    // 32: add_sat r0.y, r0.w, r0.y
    r0.y = (saturate((r0.wwww)+(r0.yyyy))).y;
    // 33: mul_sat r0.y, r0.y, cb2[3].w
    r0.y = (saturate((r0.yyyy)*(passValues[3].wwww))).y;
    // 34: mad r2.xyz, r0.yyyy, r2.xyzx, r0.zzzz
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r0.zzzz)).xyz;
    // 35: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 36: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 37: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 38: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 39: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 40: max r0.x, r0.x, cb0[0].x
    r0.x = (max(r0.xxxx,source[0].xxxx)).x;
    // 41: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 42: add r0.z, -r0.x, l(1.000000)
    r0.z = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 43: max r3.xyz, r2.xyzx, r0.zzzz
    r3.xyz = (max(r2.xyzx,r0.zzzz)).xyz;
    // 44: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 45: dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 46: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 47: mul r4.xyz, r0.zzzz, v7.xyzx
    r4.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // 48: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 49: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 50: mad r5.xyz, v5.xyzx, r0.zzzz, r4.xyzx
    r5.xyz = ((v5.xyzx)*(r0.zzzz)+(r4.xyzx)).xyz;
    // 51: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 52: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 53: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 54: dp3_sat r0.w, r4.xyzx, r5.xyzx
    r0.w = (saturate(dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 55: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: mad r3.w, v5.z, r0.z, l(1.000000)
    r3.w = ((v5.zzzz)*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: mul r6.xyz, r0.zzzz, v5.xyzx
    r6.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 58: min r0.z, r3.w, l(1.000000)
    r0.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 59: add r0.z, -r0.z, r0.w
    r0.z = ((-(r0.zzzz))+(r0.wwww)).z;
    // 60: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 61: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 62: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 63: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 64: mul r3.w, r0.z, r0.w
    r3.w = ((r0.zzzz)*(r0.wwww)).w;
    // 65: mad r0.z, -r0.w, r0.z, l(1.000000)
    r0.z = ((-(r0.wwww))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 66: mad r7.xyz, -r3.wwww, r2.xyzx, r2.xyzx
    r7.xyz = ((-(r3.wwww))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 67: mul_sat r0.w, r2.y, l(50.000000)
    r0.w = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 68: mul r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)*(r0.wwww)).w;
    // 69: mad r3.xyz, r0.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 70: mad r2.xyz, r0.zzzz, r2.xyzx, r0.wwww
    r2.xyz = ((r0.zzzz)*(r2.xyzx)+(r0.wwww)).xyz;
    // 71: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 72: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 73: add r7.xyz, -r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r3.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: add r0.z, -r1.w, l(1.000000)
    r0.z = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 75: mad_sat r0.z, r2.w, r0.z, r1.w
    r0.z = (saturate((r2.wwww)*(r0.zzzz)+(r1.wwww))).z;
    // 76: mad r0.z, -r0.z, cb0[2].x, l(1.000000)
    r0.z = ((-(r0.zzzz))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 77: mul r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)*(r0.xxxx)).w;
    // 78: mad r0.x, -r0.x, r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 79: mad r1.w, r0.w, l(0.350000), l(1.000000)
    r1.w = ((r0.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: div_sat r0.z, r0.z, r1.w
    r0.z = (saturate((r0.zzzz)/(r1.wwww))).z;
    // 81: mad r2.xyz, -r0.zzzz, r2.xyzx, r7.xyzx
    r2.xyz = ((-(r0.zzzz))*(r2.xyzx)+(r7.xyzx)).xyz;
    // 82: mad r7.xyz, -r1.xyzx, r0.yyyy, r1.xyzx
    r7.xyz = ((-(r1.xyzx))*(r0.yyyy)+(r1.xyzx)).xyz;
    // 83: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 84: mul r7.xyz, r7.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 85: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 87: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 88: dp2 r0.z, r7.xyxx, r7.xyxx
    r0.z = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).z;
    // 89: mul r7.xy, r7.xyxx, cb0[7].xxxx
    r7.xy = ((r7.xyxx)*(source[7].xxxx)).xy;
    // 90: mul r7.xy, r7.xyxx, v2.wwww
    r7.xy = ((r7.xyxx)*(v2.wwww)).xy;
    // 91: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 92: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 93: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 94: add r7.z, r0.z, l(0.000010)
    r7.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 95: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 96: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 97: div r7.xyz, r7.xyzx, r0.zzzz
    r7.xyz = ((r7.xyzx)/(r0.zzzz)).xyz;
    // 98: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 99: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 100: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 101: dp3 r0.z, r7.xyzx, r4.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 102: add r0.z, |r0.z|, l(0.000010)
    r0.z = ((abs(r0.zzzz))+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 103: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 104: mad r1.w, r0.z, r0.x, r0.w
    r1.w = ((r0.zzzz)*(r0.xxxx)+(r0.wwww)).w;
    // 105: dp3_sat r2.w, r7.xyzx, r6.xyzx
    r2.w = (saturate(dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 106: mad r6.xyz, r7.xyzx, cb0[1].xxxx, r6.xyzx
    r6.xyz = ((r7.xyzx)*(source[1].xxxx)+(r6.xyzx)).xyz;
    // 107: dp3_sat r3.w, r7.xyzx, r5.xyzx
    r3.w = (saturate(dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 108: mad r0.x, r2.w, r0.x, r0.w
    r0.x = ((r2.wwww)*(r0.xxxx)+(r0.wwww)).x;
    // 109: mul r0.xw, r0.xxxw, r0.zzzw
    r0.xw = ((r0.xxxw)*(r0.zzzw)).xw;
    // 110: mad r0.x, r2.w, r1.w, r0.x
    r0.x = ((r2.wwww)*(r1.wwww)+(r0.xxxx)).x;
    // 111: rcp r0.x, r0.x
    r0.x = (1.0/(r0.xxxx)).x;
    // 112: mad r0.z, r3.w, r0.w, -r3.w
    r0.z = ((r3.wwww)*(r0.wwww)+(-(r3.wwww))).z;
    // 113: mad r0.z, r0.z, r3.w, l(1.000000)
    r0.z = ((r0.zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 114: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 115: mul r0.z, r0.z, l(3.141593)
    r0.z = ((r0.zzzz)*(float4(3.141593,3.141593,3.141593,3.141593))).z;
    // 116: div r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)/(r0.zzzz)).z;
    // 117: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 118: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 119: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 120: add r0.z, r0.z, l(0.000100)
    r0.z = ((r0.zzzz)+(float4(0.000100,0.000100,0.000100,0.000100))).z;
    // 121: div r0.z, l(3.000000), r0.z
    r0.z = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.zzzz)).z;
    // 122: min r0.x, r0.z, r0.x
    r0.x = (min(r0.zzzz,r0.xxxx)).x;
    // 123: mad r0.xzw, r0.xxxx, r3.xxyz, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r3.xxyz)+(r2.xxyz)).xzw;
    // 124: mul r0.xzw, r2.wwww, r0.xxzw
    r0.xzw = ((r2.wwww)*(r0.xxzw)).xzw;
    // 125: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 126: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 127: mul r2.xyz, r1.wwww, r6.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 128: dp3_sat r1.w, r4.xyzx, -r2.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(-(r2.xyzx)).xyz).xxxx)).w;
    // 129: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 130: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 131: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 132: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 133: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 134: add_sat r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (saturate((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 135: add_sat r2.xyz, r2.xyzx, cb0[12].xxxx
    r2.xyz = (saturate((r2.xyzx)+(source[12].xxxx))).xyz;
    // 136: mul r2.w, r2.x, cb0[12].y
    r2.w = ((r2.xxxx)*(source[12].yyyy)).w;
    // 137: mul_sat r2.xyz, r2.xyzx, cb0[6].xyzx
    r2.xyz = (saturate((r2.xyzx)*(source[6].xyzx))).xyz;
    // 138: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 139: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 140: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 141: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 142: mad r0.xyz, r0.xzwx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xzwx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 143: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 144: mov o0.w, l(1.000000)
    output.targets[0].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 145: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 146: ret
    return output;
}

// source.character.static-map-native-1107.v1 / source program 7b3d7696d2aa3c46b3304ce3482c3dc2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1107(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterLightConstants[0];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=g_SourceCharacterLightConstants[3];
    source[5]=g_SourceCharacterLightConstants[4];
    source[6]=g_SourceCharacterLightConstants[5];
    source[7]=g_SourceCharacterLightConstants[6];
    source[8]=g_SourceCharacterLightConstants[7];
    source[9]=g_SourceCharacterLightConstants[8];
    source[10]=float4(input.lightColor,1.f);
    source[16].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
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
    // 10: dp3 r0.w, -v8.xyzx, -v8.xyzx
    r0.w = (dot((-(v8.xyzx)).xyz,(-(v8.xyzx)).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, -v8.xyzx
    r3.xyz = ((r0.wwww)*(-(v8.xyzx))).xyz;
    // 13: mul r4.xyz, r2.xyzx, r3.yyyy
    r4.xyz = ((r2.xyzx)*(r3.yyyy)).xyz;
    // 14: mad r3.xyw, r3.xxxx, r1.xyxz, r4.xyxz
    r3.xyw = ((r3.xxxx)*(r1.xyxz)+(r4.xyxz)).xyw;
    // 15: mad r0.xyz, r3.zzzz, r0.xyzx, r3.xywx
    r0.xyz = ((r3.zzzz)*(r0.xyzx)+(r3.xywx)).xyz;
    // 16: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
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
    // 27: mul r5.xy, r4.xyxx, v7.wwww
    r5.xy = ((r4.xyxx)*(v7.wwww)).xy;
    // 28: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 29: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 30: div r4.xyw, r5.xyxz, r1.wwww
    r4.xyw = ((r5.xyxz)/(r1.wwww)).xyw;
    // 31: dp3 r1.w, r4.xywx, r4.xywx
    r1.w = (dot((r4.xywx).xyz,(r4.xywx).xyz).xxxx).w;
    // 32: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 33: mul r4.xyw, r1.wwww, r4.xyxw
    r4.xyw = ((r1.wwww)*(r4.xyxw)).xyw;
    // 34: ge r1.w, r4.w, l(0.999850)
    r1.w = (asfloat((uint4)((r4.wwww)>=(float4(0.999850,0.999850,0.999850,0.999850))) * 0xffffffffu)).w;
    // 35: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 36: mov r5.xyz, l(0,0,1.000000,0)
    r5.xyz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xyz;
    // 37: endif
    }
    // 38: if_z r1.w
    if ((asuint(r1.wwww)).x == 0u) {
    // 39: mov r5.xyz, r4.xywx
    r5.xyz = (r4.xywx).xyz;
    // 40: endif
    }
    // 41: dp3 r1.w, r5.xyzx, r0.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 42: mul r4.xyw, r1.wwww, r5.xyxz
    r4.xyw = ((r1.wwww)*(r5.xyxz)).xyw;
    // 43: mad r4.xyw, r4.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r0.xyxz
    r4.xyw = ((r4.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r0.xyxz))).xyw;
    // 44: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[16].y
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[16].yyyy)) * 0xffffffffu)).w;
    // 45: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 46: mul r6.xyzw, v8.yyyy, cb1[1].xyzw
    r6.xyzw = ((v8.yyyy)*(projection[1].xyzw)).xyzw;
    // 47: mad r6.xyzw, cb1[0].xyzw, v8.xxxx, r6.xyzw
    r6.xyzw = ((projection[0].xyzw)*(v8.xxxx)+(r6.xyzw)).xyzw;
    // 48: mad r6.xyzw, cb1[2].xyzw, v8.zzzz, r6.xyzw
    r6.xyzw = ((projection[2].xyzw)*(v8.zzzz)+(r6.xyzw)).xyzw;
    // 49: mad r6.xyzw, cb1[3].xyzw, v8.wwww, r6.xyzw
    r6.xyzw = ((projection[3].xyzw)*(v8.wwww)+(r6.xyzw)).xyzw;
    // 50: mul r7.xyzw, r6.yyyy, cb0[12].xyzw
    r7.xyzw = ((r6.yyyy)*(source[12].xyzw)).xyzw;
    // 51: mad r7.xyzw, cb0[11].xyzw, r6.xxxx, r7.xyzw
    r7.xyzw = ((source[11].xyzw)*(r6.xxxx)+(r7.xyzw)).xyzw;
    // 52: mad r7.xyzw, cb0[13].xyzw, r6.zzzz, r7.xyzw
    r7.xyzw = ((source[13].xyzw)*(r6.zzzz)+(r7.xyzw)).xyzw;
    // 53: mad r6.xyzw, cb0[14].xyzw, r6.wwww, r7.xyzw
    r6.xyzw = ((source[14].xyzw)*(r6.wwww)+(r7.xyzw)).xyzw;
    // 54: div r6.xy, r6.xyxx, r6.wwww
    r6.xy = ((r6.xyxx)/(r6.wwww)).xy;
    // 55: sample_indexable(texture2d)(float,float,float,float) r7.x, r6.xyxx, t1.xyzw, s4
    r7.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 56: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 57: mov r8.yz, cb0[15].wwzw
    r8.yz = (source[15].wwzw).yz;
    // 58: add r8.xyzw, r6.xyxy, r8.xyzw
    r8.xyzw = ((r6.xyxy)+(r8.xyzw)).xyzw;
    // 59: sample_indexable(texture2d)(float,float,float,float) r7.y, r8.xyxx, t1.yxzw, s4
    r7.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 60: sample_indexable(texture2d)(float,float,float,float) r7.z, r8.zwzz, t1.yzxw, s4
    r7.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 61: add r8.xy, r6.xyxx, cb0[15].zwzz
    r8.xy = ((r6.xyxx)+(source[15].zwzz)).xy;
    // 62: sample_indexable(texture2d)(float,float,float,float) r7.w, r8.xyxx, t1.yzwx, s4
    r7.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 63: lt r7.xyzw, r6.zzzz, r7.xyzw
    r7.xyzw = (asfloat((uint4)((r6.zzzz)<(r7.xyzw)) * 0xffffffffu)).xyzw;
    // 64: and r8.xyzw, r7.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r8.xyzw = (asfloat(asuint(r7.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 65: mul r6.xy, r6.xyxx, cb0[15].xyxx
    r6.xy = ((r6.xyxx)*(source[15].xyxx)).xy;
    // 66: frc r6.xy, r6.xyxx
    r6.xy = (frac(r6.xyxx)).xy;
    // 67: movc r6.zw, r7.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r6.zw = ((asuint(r7.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 68: add r6.zw, r6.zzzw, r8.zzzw
    r6.zw = ((r6.zzzw)+(r8.zzzw)).zw;
    // 69: mad r6.xz, r6.xxxx, r6.zzwz, r8.xxyx
    r6.xz = ((r6.xxxx)*(r6.zzwz)+(r8.xxyx)).xz;
    // 70: add r2.w, -r6.x, r6.z
    r2.w = ((-(r6.xxxx))+(r6.zzzz)).w;
    // 71: mad r2.w, r6.y, r2.w, r6.x
    r2.w = ((r6.yyyy)*(r2.wwww)+(r6.xxxx)).w;
    // 72: mul r6.xyz, r2.wwww, cb0[16].xxxx
    r6.xyz = ((r2.wwww)*(source[16].xxxx)).xyz;
    // 73: else
    } else {
    // 74: mov r6.xyz, l(1.000000,1.000000,1.000000,0)
    r6.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 75: endif
    }
    // 76: add r7.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: dp3 r1.x, r1.xyzx, r4.xywx
    r1.x = (dot((r1.xyzx).xyz,(r4.xywx).xyz).xxxx).x;
    // 78: dp3 r1.y, r2.xyzx, r4.xywx
    r1.y = (dot((r2.xyzx).xyz,(r4.xywx).xyz).xxxx).y;
    // 79: mul r2.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r2.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 80: mad r1.xy, cb0[6].yyyy, r1.xyxx, r2.xyxx
    r1.xy = ((source[6].yyyy)*(r1.xyxx)+(r2.xyxx)).xy;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 82: mul r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)*(source[3].xyzx)).xyz;
    // 83: mad r1.xyz, cb0[6].zzzz, r1.xyzx, r1.xyzx
    r1.xyz = ((source[6].zzzz)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 84: add r1.xyz, r1.xyzx, -cb0[6].zzzz
    r1.xyz = ((r1.xyzx)+(-(source[6].zzzz))).xyz;
    // 85: mov_sat r2.xyz, r1.xyzx
    r2.xyz = (saturate(r1.xyzx)).xyz;
    // 86: mul r2.w, r4.z, cb0[6].w
    r2.w = ((r4.zzzz)*(source[6].wwww)).w;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 88: mul r8.xyz, cb0[4].xyzx, cb0[7].xxxx
    r8.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // 89: mul r8.xyz, r4.xyzx, r8.xyzx
    r8.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 90: mul r9.xyz, cb0[5].xyzx, cb0[7].yyyy
    r9.xyz = ((source[5].xyzx)*(source[7].yyyy)).xyz;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t4.yzxw, s3, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 92: mul r3.w, r10.y, cb0[7].z
    r3.w = ((r10.yyyy)*(source[7].zzzz)).w;
    // 93: lt r5.w, |r3.w|, l(0.000001)
    r5.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 94: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 95: mul r3.w, r3.w, cb0[7].w
    r3.w = ((r3.wwww)*(source[7].wwww)).w;
    // 96: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 97: movc r3.w, r5.w, l(0), r3.w
    r3.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 98: min r5.w, r3.w, l(1.000000)
    r5.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mad r4.xyz, r9.xyzx, r4.xyzx, -r8.xyzx
    r4.xyz = ((r9.xyzx)*(r4.xyzx)+(-(r8.xyzx))).xyz;
    // 100: mad r4.xyz, r5.wwww, r4.xyzx, r8.xyzx
    r4.xyz = ((r5.wwww)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 101: mad r2.xyz, r2.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 102: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 103: mad r1.xyz, -r2.wwww, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r2.wwww))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 104: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 105: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 106: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 107: mul r2.xyz, r1.xyzx, cb0[8].xxxx
    r2.xyz = ((r1.xyzx)*(source[8].xxxx)).xyz;
    // 108: mad r1.xyz, cb0[8].yyyy, r1.xyzx, -r2.xyzx
    r1.xyz = ((source[8].yyyy)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 109: mad r1.xyz, r5.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r5.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 110: mul r1.xyz, r7.xyzx, r1.xyzx
    r1.xyz = ((r7.xyzx)*(r1.xyzx)).xyz;
    // 111: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 112: mov_sat r2.x, cb0[8].z
    r2.x = (saturate(source[8].zzzz)).x;
    // 113: mul_sat r2.y, r3.w, cb2[3].w
    r2.y = (saturate((r3.wwww)*(passValues[3].wwww))).y;
    // 114: mul r2.z, r10.x, cb0[9].y
    r2.z = ((r10.xxxx)*(source[9].yyyy)).z;
    // 115: lt r2.w, |r2.z|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 116: log r2.z, |r2.z|
    r2.z = (log2(abs(r2.zzzz))).z;
    // 117: mul r2.z, r2.z, cb0[9].z
    r2.z = ((r2.zzzz)*(source[9].zzzz)).z;
    // 118: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 119: movc r2.z, r2.w, l(0), r2.z
    r2.z = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).z;
    // 120: max r2.z, r2.z, cb0[0].w
    r2.z = (max(r2.zzzz,source[0].wwww)).z;
    // 121: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 122: mad r4.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 123: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 124: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 125: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 126: dp3_sat r2.w, r5.xyzx, r4.xyzx
    r2.w = (saturate(dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx)).w;
    // 127: add r1.w, |r1.w|, l(0.000010)
    r1.w = ((abs(r1.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 128: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: dp3_sat r3.x, r5.xyzx, r3.xyzx
    r3.x = (saturate(dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx)).x;
    // 130: dp3_sat r0.x, r0.xyzx, r4.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 131: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 132: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 133: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 134: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 135: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 136: mad r0.yzw, -r1.xxyz, r2.yyyy, r1.xxyz
    r0.yzw = ((-(r1.xxyz))*(r2.yyyy)+(r1.xxyz)).yzw;
    // 137: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 138: mul r3.y, r2.z, r2.z
    r3.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 139: mul r3.z, r3.y, r3.y
    r3.z = ((r3.yyyy)*(r3.yyyy)).z;
    // 140: mad r3.w, r2.w, r3.z, -r2.w
    r3.w = ((r2.wwww)*(r3.zzzz)+(-(r2.wwww))).w;
    // 141: mad r2.w, r3.w, r2.w, l(1.000000)
    r2.w = ((r3.wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 143: mul r2.w, r2.w, l(3.141593)
    r2.w = ((r2.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 144: div r2.w, r3.z, r2.w
    r2.w = ((r3.zzzz)/(r2.wwww)).w;
    // 145: mad r3.z, -r2.z, r2.z, l(1.000000)
    r3.z = ((-(r2.zzzz))*(r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: mad r3.w, r1.w, r3.z, r3.y
    r3.w = ((r1.wwww)*(r3.zzzz)+(r3.yyyy)).w;
    // 147: mad r3.y, r3.x, r3.z, r3.y
    r3.y = ((r3.xxxx)*(r3.zzzz)+(r3.yyyy)).y;
    // 148: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 149: mad r1.w, r3.x, r3.w, r1.w
    r1.w = ((r3.xxxx)*(r3.wwww)+(r1.wwww)).w;
    // 150: rcp r1.w, r1.w
    r1.w = (1.0/(r1.wwww)).w;
    // 151: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 152: mul r2.w, r2.x, l(0.080000)
    r2.w = ((r2.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 153: mad r1.xyz, -r2.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r1.xyz = ((-(r2.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 154: mad r1.xyz, r2.yyyy, r1.xyzx, r2.wwww
    r1.xyz = ((r2.yyyy)*(r1.xyzx)+(r2.wwww)).xyz;
    // 155: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 156: mul r2.x, r0.x, r0.x
    r2.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 157: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 158: mul r0.x, r0.x, r2.x
    r0.x = ((r0.xxxx)*(r2.xxxx)).x;
    // 159: mul_sat r2.x, r1.y, l(50.000000)
    r2.x = (saturate((r1.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).x;
    // 160: mul r2.x, r0.x, r2.x
    r2.x = ((r0.xxxx)*(r2.xxxx)).x;
    // 161: add r2.y, -r2.z, l(1.000000)
    r2.y = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 162: max r2.yzw, r1.xxyz, r2.yyyy
    r2.yzw = (max(r1.xxyz,r2.yyyy)).yzw;
    // 163: add r2.yzw, -r1.xxyz, r2.yyzw
    r2.yzw = ((-(r1.xxyz))+(r2.yyzw)).yzw;
    // 164: mad r1.xyz, -r0.xxxx, r1.xyzx, r1.xyzx
    r1.xyz = ((-(r0.xxxx))*(r1.xyzx)+(r1.xyzx)).xyz;
    // 165: mad r1.xyz, r2.xxxx, r2.yzwy, r1.xyzx
    r1.xyz = ((r2.xxxx)*(r2.yzwy)+(r1.xyzx)).xyz;
    // 166: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 167: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 168: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 169: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 170: min r0.x, r0.x, r1.w
    r0.x = (min(r0.xxxx,r1.wwww)).x;
    // 171: mul r2.xyz, r1.xyzx, r0.xxxx
    r2.xyz = ((r1.xyzx)*(r0.xxxx)).xyz;
    // 172: add r1.xyz, -r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 173: mad r0.xyz, r0.yzwy, r1.xyzx, r2.xyzx
    r0.xyz = ((r0.yzwy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 174: mul r0.xyz, r3.xxxx, r0.xyzx
    r0.xyz = ((r3.xxxx)*(r0.xyzx)).xyz;
    // 175: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 176: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 177: mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 178: mul_sat r0.x, r4.w, cb0[8].w
    r0.x = (saturate((r4.wwww)*(source[8].wwww))).x;
    // 179: mul o0.w, r0.x, cb0[1].x
    output.targets[0].w = ((r0.xxxx)*(source[1].xxxx)).w;
    // 180: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 181: ret
    return output;
}

// source.character.static-map-native-1108.v1 / source program e54cb64661df354ca6459ec281d19369
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1108(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[4];
    source[3]=g_SourceCharacterLightConstants[5];
    source[4]=g_SourceCharacterLightConstants[6];
    source[5]=g_SourceCharacterLightConstants[7];
    source[6]=g_SourceCharacterLightConstants[8];
    source[7]=g_SourceCharacterLightConstants[9];
    source[8]=g_SourceCharacterLightConstants[10];
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
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
    // 34: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).w;
    // 35: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 36: div r6.xy, v8.xyxx, v8.wwww
    r6.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 37: mad r6.xy, r6.xyxx, cb2[0].xyxx, cb2[0].wzww
    r6.xy = ((r6.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: sample_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s0
    r6.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 39: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 40: else
    } else {
    // 41: mov r6.xyz, l(1.000000,1.000000,1.000000,0)
    r6.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 42: endif
    }
    // 43: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 44: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 45: dp3 r1.y, r0.xyzx, r5.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 46: mul r0.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 47: mad r0.xy, cb0[5].zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((source[5].zzzz)*(r1.xyxx)+(r0.xyxx)).xy;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 49: mul r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[2].xyzx)).xyz;
    // 50: mad r0.xyz, cb0[5].wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((source[5].wwww)*(r0.xyzx)+(r0.xyzx)).xyz;
    // 51: add r0.xyz, r0.xyzx, -cb0[5].wwww
    r0.xyz = ((r0.xyzx)+(-(source[5].wwww))).xyz;
    // 52: mov_sat r1.xyz, r0.xyzx
    r1.xyz = (saturate(r0.xyzx)).xyz;
    // 53: mul r2.w, r4.z, cb0[6].x
    r2.w = ((r4.zzzz)*(source[6].xxxx)).w;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 55: mul r8.xyz, cb0[3].xyzx, cb0[6].yyyy
    r8.xyz = ((source[3].xyzx)*(source[6].yyyy)).xyz;
    // 56: mul r8.xyz, r5.xyzx, r8.xyzx
    r8.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 57: mul r9.xyz, cb0[4].xyzx, cb0[6].zzzz
    r9.xyz = ((source[4].xyzx)*(source[6].zzzz)).xyz;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t3.yzxw, s4, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 59: mul r3.w, r10.y, cb0[6].w
    r3.w = ((r10.yyyy)*(source[6].wwww)).w;
    // 60: lt r4.z, |r3.w|, l(0.000001)
    r4.z = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 61: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 62: mul r3.w, r3.w, cb0[7].x
    r3.w = ((r3.wwww)*(source[7].xxxx)).w;
    // 63: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 64: movc r3.w, r4.z, l(0), r3.w
    r3.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 65: min r4.z, r3.w, l(1.000000)
    r4.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 66: mad r5.xyz, r9.xyzx, r5.xyzx, -r8.xyzx
    r5.xyz = ((r9.xyzx)*(r5.xyzx)+(-(r8.xyzx))).xyz;
    // 67: mad r5.xyz, r4.zzzz, r5.xyzx, r8.xyzx
    r5.xyz = ((r4.zzzz)*(r5.xyzx)+(r8.xyzx)).xyz;
    // 68: mad r1.xyz, r2.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 69: mov_sat r0.xyz, -r0.xyzx
    r0.xyz = (saturate(-(r0.xyzx))).xyz;
    // 70: mad r0.xyz, -r2.wwww, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r2.wwww))*(r0.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 71: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 72: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 73: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 74: mul r1.xyz, r0.xyzx, cb0[7].yyyy
    r1.xyz = ((r0.xyzx)*(source[7].yyyy)).xyz;
    // 75: mad r0.xyz, cb0[7].zzzz, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[7].zzzz)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 76: mad r0.xyz, r4.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r4.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 77: mul r0.xyz, r7.xyzx, r0.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)).xyz;
    // 78: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 79: mov_sat r1.x, cb0[7].w
    r1.x = (saturate(source[7].wwww)).x;
    // 80: mul_sat r1.y, r3.w, cb2[3].w
    r1.y = (saturate((r3.wwww)*(passValues[3].wwww))).y;
    // 81: mul r1.z, r10.x, cb0[8].y
    r1.z = ((r10.xxxx)*(source[8].yyyy)).z;
    // 82: lt r2.w, |r1.z|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 83: log r1.z, |r1.z|
    r1.z = (log2(abs(r1.zzzz))).z;
    // 84: mul r1.z, r1.z, cb0[8].z
    r1.z = ((r1.zzzz)*(source[8].zzzz)).z;
    // 85: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 86: movc r1.z, r2.w, l(0), r1.z
    r1.z = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 87: max r1.z, r1.z, cb0[0].w
    r1.z = (max(r1.zzzz,source[0].wwww)).z;
    // 88: mad r5.xyz, v5.xyzx, r0.wwww, r2.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 89: dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 90: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 91: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 92: dp3_sat r2.w, r4.xywx, r5.xyzx
    r2.w = (saturate(dot((r4.xywx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 93: add r1.w, |r1.w|, l(0.000010)
    r1.w = ((abs(r1.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 94: min r1.zw, r1.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = (min(r1.zzzw,float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 95: dp3_sat r3.x, r4.xywx, r3.xyzx
    r3.x = (saturate(dot((r4.xywx).xyz,(r3.xyzx).xyz).xxxx)).x;
    // 96: dp3_sat r2.x, r2.xyzx, r5.xyzx
    r2.x = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 97: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 100: add r0.w, -r0.w, r2.x
    r0.w = ((-(r0.wwww))+(r2.xxxx)).w;
    // 101: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r2.xyz, -r0.xyzx, r1.yyyy, r0.xyzx
    r2.xyz = ((-(r0.xyzx))*(r1.yyyy)+(r0.xyzx)).xyz;
    // 103: mul r3.y, r1.z, r1.z
    r3.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 104: mul r3.z, r3.y, r3.y
    r3.z = ((r3.yyyy)*(r3.yyyy)).z;
    // 105: mad r3.w, r2.w, r3.z, -r2.w
    r3.w = ((r2.wwww)*(r3.zzzz)+(-(r2.wwww))).w;
    // 106: mad r2.w, r3.w, r2.w, l(1.000000)
    r2.w = ((r3.wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 108: mul r2.xyzw, r2.xyzw, l(0.318310, 0.318310, 0.318310, 3.141593)
    r2.xyzw = ((r2.xyzw)*(float4(0.318310,0.318310,0.318310,3.141593))).xyzw;
    // 109: div r2.w, r3.z, r2.w
    r2.w = ((r3.zzzz)/(r2.wwww)).w;
    // 110: mad r3.z, -r1.z, r1.z, l(1.000000)
    r3.z = ((-(r1.zzzz))*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 111: mad r3.w, r1.w, r3.z, r3.y
    r3.w = ((r1.wwww)*(r3.zzzz)+(r3.yyyy)).w;
    // 112: mad r3.y, r3.x, r3.z, r3.y
    r3.y = ((r3.xxxx)*(r3.zzzz)+(r3.yyyy)).y;
    // 113: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 114: mad r1.w, r3.x, r3.w, r1.w
    r1.w = ((r3.xxxx)*(r3.wwww)+(r1.wwww)).w;
    // 115: rcp r1.w, r1.w
    r1.w = (1.0/(r1.wwww)).w;
    // 116: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 117: mul r2.w, r1.x, l(0.080000)
    r2.w = ((r1.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 118: mad r0.xyz, -r1.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r1.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 119: mad r0.xyz, r1.yyyy, r0.xyzx, r2.wwww
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(r2.wwww)).xyz;
    // 120: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 122: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 123: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 124: mul_sat r1.x, r0.y, l(50.000000)
    r1.x = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).x;
    // 125: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 126: add r1.y, -r1.z, l(1.000000)
    r1.y = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 127: max r3.yzw, r0.xxyz, r1.yyyy
    r3.yzw = (max(r0.xxyz,r1.yyyy)).yzw;
    // 128: add r3.yzw, -r0.xxyz, r3.yyzw
    r3.yzw = ((-(r0.xxyz))+(r3.yyzw)).yzw;
    // 129: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 130: mad r0.xyz, r1.xxxx, r3.yzwy, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r3.yzwy)+(r0.xyzx)).xyz;
    // 131: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 132: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 133: mul r1.x, r1.w, l(0.500000)
    r1.x = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 134: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 135: min r0.w, r0.w, r1.x
    r0.w = (min(r0.wwww,r1.xxxx)).w;
    // 136: mul r1.xyz, r0.xyzx, r0.wwww
    r1.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 137: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 138: mad r0.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 139: mul r0.xyz, r3.xxxx, r0.xyzx
    r0.xyz = ((r3.xxxx)*(r0.xyzx)).xyz;
    // 140: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 141: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 142: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 143: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 144: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 145: ret
    return output;
}

// source.character.static-map-native-1109.v1 / source program aa78d01f49ba7a4a80404c8e36d9cdba
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1109(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterLightConstants[0];
    source[4]=g_SourceCharacterLightConstants[2];
    source[5]=g_SourceCharacterLightConstants[3];
    source[6]=g_SourceCharacterLightConstants[4];
    source[7]=g_SourceCharacterLightConstants[5];
    source[8]=g_SourceCharacterLightConstants[6];
    source[9]=g_SourceCharacterLightConstants[7];
    source[10]=g_SourceCharacterLightConstants[8];
    source[11]=g_SourceCharacterLightConstants[9];
    source[12]=g_SourceCharacterLightConstants[10];
    source[13]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t2.yzxw, s2, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 2: mul r0.x, r0.x, cb0[11].y
    r0.x = ((r0.xxxx)*(source[11].yyyy)).x;
    // 3: mul r0.y, r0.y, cb0[8].w
    r0.y = ((r0.yyyy)*(source[8].wwww)).y;
    // 4: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 5: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 6: mul r0.z, r0.z, cb0[11].z
    r0.z = ((r0.zzzz)*(source[11].zzzz)).z;
    // 7: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 8: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 9: max r0.x, r0.x, cb0[0].x
    r0.x = (max(r0.xxxx,source[0].xxxx)).x;
    // 10: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 11: add r0.z, -r0.x, l(1.000000)
    r0.z = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 12: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 13: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 14: mul r0.w, r0.w, cb0[9].x
    r0.w = ((r0.wwww)*(source[9].xxxx)).w;
    // 15: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 16: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 17: min r0.w, r0.y, l(1.000000)
    r0.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: mul r1.xyz, cb0[5].xyzx, cb0[8].xxxx
    r1.xyz = ((source[5].xyzx)*(source[8].xxxx)).xyz;
    // 19: mul r2.xyz, cb0[4].xyzx, cb0[7].wwww
    r2.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 21: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 22: add r4.xyz, -r3.xyzx, r1.wwww
    r4.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 23: mad r5.xyz, cb0[7].zzzz, r4.xyzx, r3.xyzx
    r5.xyz = ((source[7].zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 24: mad r3.xyz, cb0[8].zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((source[8].zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 25: add r4.xy, r3.wwww, cb0[10].wyww
    r4.xy = ((r3.wwww)+(source[10].wyww)).xy;
    // 26: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 27: mad r1.xyz, r1.xyzx, r3.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(-(r2.xyzx))).xyz;
    // 28: mad r1.xyz, r0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 29: mul r2.xyz, r1.xyzx, cb0[9].yyyy
    r2.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 30: mad r1.xyz, cb0[9].zzzz, r1.xyzx, -r2.xyzx
    r1.xyz = ((source[9].zzzz)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 31: mad r1.xyz, r0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 32: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 33: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 34: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 35: mov_sat r0.w, cb0[9].w
    r0.w = (saturate(source[9].wwww)).w;
    // 36: mad r2.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r2.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 37: mul r0.w, r0.w, l(0.080000)
    r0.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.w, v4.xyxx, t3.yzwx, s3, l(0.000000)
    r1.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 39: mad r2.w, r1.w, -r4.x, r4.x
    r2.w = ((r1.wwww)*(-(r4.xxxx))+(r4.xxxx)).w;
    // 40: add_sat r0.y, r0.y, r2.w
    r0.y = (saturate((r0.yyyy)+(r2.wwww))).y;
    // 41: mul_sat r0.y, r0.y, cb2[3].w
    r0.y = (saturate((r0.yyyy)*(passValues[3].wwww))).y;
    // 42: mad r2.xyz, r0.yyyy, r2.xyzx, r0.wwww
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r0.wwww)).xyz;
    // 43: max r3.xyz, r0.zzzz, r2.xyzx
    r3.xyz = (max(r0.zzzz,r2.xyzx)).xyz;
    // 44: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 45: dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 46: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 47: mul r4.xzw, r0.zzzz, v7.xxyz
    r4.xzw = ((r0.zzzz)*(v7.xxyz)).xzw;
    // 48: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 49: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 50: mad r5.xyz, v5.xyzx, r0.zzzz, r4.xzwx
    r5.xyz = ((v5.xyzx)*(r0.zzzz)+(r4.xzwx)).xyz;
    // 51: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 52: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 53: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 54: dp3_sat r0.w, r4.xzwx, r5.xyzx
    r0.w = (saturate(dot((r4.xzwx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 55: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: mad r2.w, v5.z, r0.z, l(1.000000)
    r2.w = ((v5.zzzz)*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: mul r6.xyz, r0.zzzz, v5.xyzx
    r6.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 58: min r0.z, r2.w, l(1.000000)
    r0.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 59: add r0.z, -r0.z, r0.w
    r0.z = ((-(r0.zzzz))+(r0.wwww)).z;
    // 60: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 61: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 62: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 63: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 64: mul r2.w, r0.z, r0.w
    r2.w = ((r0.zzzz)*(r0.wwww)).w;
    // 65: mad r0.z, -r0.w, r0.z, l(1.000000)
    r0.z = ((-(r0.wwww))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 66: mad r7.xyz, -r2.wwww, r2.xyzx, r2.xyzx
    r7.xyz = ((-(r2.wwww))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 67: mul_sat r0.w, r2.y, l(50.000000)
    r0.w = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 68: mul r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)*(r0.wwww)).w;
    // 69: mad r3.xyz, r0.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 70: mad r2.xyz, r0.zzzz, r2.xyzx, r0.wwww
    r2.xyz = ((r0.zzzz)*(r2.xyzx)+(r0.wwww)).xyz;
    // 71: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 72: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 73: add r7.xyz, -r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r3.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: add r0.z, -r4.y, l(1.000000)
    r0.z = ((-(r4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 75: mad_sat r0.z, r1.w, r0.z, r4.y
    r0.z = (saturate((r1.wwww)*(r0.zzzz)+(r4.yyyy))).z;
    // 76: mad r0.z, -r0.z, cb0[2].x, l(1.000000)
    r0.z = ((-(r0.zzzz))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 77: mul r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)*(r0.xxxx)).w;
    // 78: mad r0.x, -r0.x, r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 79: mad r1.w, r0.w, l(0.350000), l(1.000000)
    r1.w = ((r0.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: div_sat r0.z, r0.z, r1.w
    r0.z = (saturate((r0.zzzz)/(r1.wwww))).z;
    // 81: mad r2.xyz, -r0.zzzz, r2.xyzx, r7.xyzx
    r2.xyz = ((-(r0.zzzz))*(r2.xyzx)+(r7.xyzx)).xyz;
    // 82: mad r7.xyz, -r1.xyzx, r0.yyyy, r1.xyzx
    r7.xyz = ((-(r1.xyzx))*(r0.yyyy)+(r1.xyzx)).xyz;
    // 83: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 84: mul r7.xyz, r7.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 85: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 87: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 88: dp2 r0.z, r7.xyxx, r7.xyxx
    r0.z = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).z;
    // 89: mul r7.xy, r7.xyxx, cb0[7].xxxx
    r7.xy = ((r7.xyxx)*(source[7].xxxx)).xy;
    // 90: mul r7.xy, r7.xyxx, v2.wwww
    r7.xy = ((r7.xyxx)*(v2.wwww)).xy;
    // 91: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 92: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 93: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 94: add r7.z, r0.z, l(0.000010)
    r7.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 95: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 96: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 97: div r7.xyz, r7.xyzx, r0.zzzz
    r7.xyz = ((r7.xyzx)/(r0.zzzz)).xyz;
    // 98: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 99: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 100: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 101: dp3 r0.z, r7.xyzx, r4.xzwx
    r0.z = (dot((r7.xyzx).xyz,(r4.xzwx).xyz).xxxx).z;
    // 102: add r0.z, |r0.z|, l(0.000010)
    r0.z = ((abs(r0.zzzz))+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 103: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 104: mad r1.w, r0.z, r0.x, r0.w
    r1.w = ((r0.zzzz)*(r0.xxxx)+(r0.wwww)).w;
    // 105: dp3_sat r2.w, r7.xyzx, r6.xyzx
    r2.w = (saturate(dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 106: mad r6.xyz, r7.xyzx, cb0[1].xxxx, r6.xyzx
    r6.xyz = ((r7.xyzx)*(source[1].xxxx)+(r6.xyzx)).xyz;
    // 107: dp3_sat r3.w, r7.xyzx, r5.xyzx
    r3.w = (saturate(dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 108: mad r0.x, r2.w, r0.x, r0.w
    r0.x = ((r2.wwww)*(r0.xxxx)+(r0.wwww)).x;
    // 109: mul r0.xw, r0.xxxw, r0.zzzw
    r0.xw = ((r0.xxxw)*(r0.zzzw)).xw;
    // 110: mad r0.x, r2.w, r1.w, r0.x
    r0.x = ((r2.wwww)*(r1.wwww)+(r0.xxxx)).x;
    // 111: rcp r0.x, r0.x
    r0.x = (1.0/(r0.xxxx)).x;
    // 112: mad r0.z, r3.w, r0.w, -r3.w
    r0.z = ((r3.wwww)*(r0.wwww)+(-(r3.wwww))).z;
    // 113: mad r0.z, r0.z, r3.w, l(1.000000)
    r0.z = ((r0.zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 114: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 115: mul r0.z, r0.z, l(3.141593)
    r0.z = ((r0.zzzz)*(float4(3.141593,3.141593,3.141593,3.141593))).z;
    // 116: div r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)/(r0.zzzz)).z;
    // 117: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 118: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 119: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 120: add r0.z, r0.z, l(0.000100)
    r0.z = ((r0.zzzz)+(float4(0.000100,0.000100,0.000100,0.000100))).z;
    // 121: div r0.z, l(3.000000), r0.z
    r0.z = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.zzzz)).z;
    // 122: min r0.x, r0.z, r0.x
    r0.x = (min(r0.zzzz,r0.xxxx)).x;
    // 123: mad r0.xzw, r0.xxxx, r3.xxyz, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r3.xxyz)+(r2.xxyz)).xzw;
    // 124: mul r0.xzw, r2.wwww, r0.xxzw
    r0.xzw = ((r2.wwww)*(r0.xxzw)).xzw;
    // 125: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 126: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 127: mul r2.xyz, r1.wwww, r6.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 128: dp3_sat r1.w, r4.xzwx, -r2.xyzx
    r1.w = (saturate(dot((r4.xzwx).xyz,(-(r2.xyzx)).xyz).xxxx)).w;
    // 129: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 130: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 131: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 132: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 133: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 134: add_sat r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (saturate((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 135: add_sat r2.xyz, r2.xyzx, cb0[12].zzzz
    r2.xyz = (saturate((r2.xyzx)+(source[12].zzzz))).xyz;
    // 136: mul r2.w, r2.x, cb0[12].w
    r2.w = ((r2.xxxx)*(source[12].wwww)).w;
    // 137: mul_sat r2.xyz, r2.xyzx, cb0[6].xyzx
    r2.xyz = (saturate((r2.xyzx)*(source[6].xyzx))).xyz;
    // 138: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 139: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 140: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 141: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 142: mad r0.xyz, r0.xzwx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xzwx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 143: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 144: mov o0.w, l(1.000000)
    output.targets[0].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 145: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 146: ret
    return output;
}

// source.character.static-map-native-1110.v1 / source program 2ec24d5ae5c7b646be9e1570c25c70dd
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1110(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 16: mul r3.xyz, cb0[0].xyzx, cb0[2].wwww
    r3.xyz = ((source[0].xyzx)*(source[2].wwww)).xyz;
    // 17: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 19: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 20: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 21: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 22: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 23: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 24: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 25: mul r4.zw, v4.xxxy, cb0[2].yyyy
    r4.zw = ((v4.xxxy)*(source[2].yyyy)).zw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r4.zw, r4.zwzz, t1.zwxy, s2, l(0.000000)
    r4.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 27: mad r4.zw, r4.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r4.zw = ((r4.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 28: mul r4.zw, r4.zzzw, cb0[2].zzzz
    r4.zw = ((r4.zzzw)*(source[2].zzzz)).zw;
    // 29: mad r4.xy, cb0[2].xxxx, r4.xyxx, r4.zwzz
    r4.xy = ((source[2].xxxx)*(r4.xyxx)+(r4.zwzz)).xy;
    // 30: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 31: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 32: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 33: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 34: dp3 r1.w, r4.xyzx, r0.yzwy
    r1.w = (dot((r4.xyzx).xyz,(r0.yzwy).xyz).xxxx).w;
    // 35: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mul r5.xyz, cb0[1].xyzx, cb0[3].xxxx
    r5.xyz = ((source[1].xyzx)*(source[3].xxxx)).xyz;
    // 38: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 39: mad r0.xyz, v7.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v7.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 40: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 41: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 42: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 43: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
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
    // 50: mul r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 51: mad r0.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
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

// source.character.static-map-native-1111.v1 / source program 0f61b2e3924d0848ba13c484269451bc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1111(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 12: mul r1.xyz, r0.wwww, v7.xyzx
    r1.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r2.xyz, r0.wwww, v5.xyzx
    r2.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 17: add r0.w, r3.w, l(-0.333300)
    r0.w = ((r3.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 18: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 19: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 20: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t2.zwxy, s3, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).xy;
    // 22: mul r1.w, r4.y, cb0[3].z
    r1.w = ((r4.yyyy)*(source[3].zzzz)).w;
    // 23: add r5.xyz, -r3.xyzx, r0.wwww
    r5.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 24: mad r3.xyz, r1.wwww, r5.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 25: mad r5.xyz, cb0[3].wwww, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((source[3].wwww)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 26: mad r4.yzw, r4.yyyy, r5.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r4.yzw = ((r4.yyyy)*(r5.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 27: mul r3.xyz, r3.xyzx, r4.yzwy
    r3.xyz = ((r3.xyzx)*(r4.yzwy)).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r4.yz, v4.xyxx, t0.zxyw, s1, l(0.000000)
    r4.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 29: mad r4.yz, r4.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r4.yz = ((r4.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 30: dp2 r0.w, r4.yzyy, r4.yzyy
    r0.w = (dot((r4.yzyy).xy,(r4.yzyy).xy).xxxx).w;
    // 31: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 34: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 35: mul r5.xy, r4.yzyy, cb0[3].xxxx
    r5.xy = ((r4.yzyy)*(source[3].xxxx)).xy;
    // 36: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 37: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 38: div r4.yzw, r5.xxyz, r0.wwww
    r4.yzw = ((r5.xxyz)/(r0.wwww)).yzw;
    // 39: dp3 r0.w, r4.yzwy, r2.xyzx
    r0.w = (dot((r4.yzwy).xyz,(r2.xyzx).xyz).xxxx).w;
    // 40: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: mul r2.xyz, r0.xyzx, r1.wwww
    r2.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 43: mul r5.xyz, cb0[1].xyzx, cb0[4].xxxx
    r5.xyz = ((source[1].xyzx)*(source[4].xxxx)).xyz;
    // 44: mul r5.xyz, r4.xxxx, r5.xyzx
    r5.xyz = ((r4.xxxx)*(r5.xyzx)).xyz;
    // 45: mad r0.xyz, r5.xyzx, r0.xyzx, -r2.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 46: mad r0.xyz, r5.xyzx, r0.xyzx, r2.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 47: mul r2.xyz, cb0[2].xyzx, cb0[4].yyyy
    r2.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // 48: dp3 r1.x, r4.yzwy, r1.xyzx
    r1.x = (dot((r4.yzwy).xyz,(r1.xyzx).xyz).xxxx).x;
    // 49: add r1.xw, -|r1.xxxz|, l(1.000000, 0.000000, 0.000000, 1.000000)
    r1.xw = ((-(abs(r1.xxxz)))+(float4(1.000000,0.000000,0.000000,1.000000))).xw;
    // 50: mul r1.x, r1.x, r1.w
    r1.x = ((r1.xxxx)*(r1.wwww)).x;
    // 51: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 52: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 53: mul r1.x, r1.x, cb0[4].z
    r1.x = ((r1.xxxx)*(source[4].zzzz)).x;
    // 54: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 55: mul r1.xzw, r2.xxyz, r1.xxxx
    r1.xzw = ((r2.xxyz)*(r1.xxxx)).xzw;
    // 56: movc r1.xyz, r1.yyyy, l(0,0,0,0), r1.xzwx
    r1.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xzwx)).xyz;
    // 57: mad r0.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 58: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 59: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 60: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 61: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 62: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 63: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 64: ret
    return output;
}

// source.character.static-map-native-1112.v1 / source program cd993bbcb1d322469e313d7f20ff8fe4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1112(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[1];
    source[1]=g_SourceCharacterLightConstants[2];
    source[2]=float4(input.lightColor,1.f);
    source[3].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: dp3 r0.x, v3.xyzx, v3.xyzx
    r0.x = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v3.z
    r0.x = ((r0.xxxx)*(v3.zzzz)).x;
    // 4: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[3].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[3].xxxx)) * 0xffffffffu)).y;
    // 5: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 6: div r0.yz, v6.xxyx, v6.wwww
    r0.yz = ((v6.xxyx)/(v6.wwww)).yz;
    // 7: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 8: sample_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t1.wxyz, s0
    r0.yzw = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).wxyz).yzw;
    // 9: mul r0.yzw, r0.yyzw, r0.yyzw
    r0.yzw = ((r0.yyzw)*(r0.yyzw)).yzw;
    // 10: else
    } else {
    // 11: mov r0.yzw, l(0,1.000000,1.000000,1.000000)
    r0.yzw = (float4(asfloat(0u),1.000000,1.000000,1.000000)).yzw;
    // 12: endif
    }
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 14: mul r2.xyz, r1.xyzx, cb0[0].xyzx
    r2.xyz = ((r1.xyzx)*(source[0].xyzx)).xyz;
    // 15: mad r1.xyz, -r1.xyzx, cb0[0].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[0].xyzx)+(r1.xyzx)).xyz;
    // 16: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 17: mul r1.xyz, r1.xyzx, cb0[1].xxxx
    r1.xyz = ((r1.xyzx)*(source[1].xxxx)).xyz;
    // 18: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 19: mul r2.xyz, r0.xxxx, cb2[3].xyzx
    r2.xyz = ((r0.xxxx)*(passValues[3].xyzx)).xyz;
    // 20: mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 21: mul r0.xyz, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 22: mul o0.xyz, r0.xyzx, cb0[2].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[2].xyzx)).xyz;
    // 23: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 24: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 25: ret
    return output;
}

// source.character.static-map-native-1113.v1 / source program 36e8eb53f5f0f1428aefb794851d27d5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1113(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=g_SourceCharacterLightConstants[5];
    source[5]=g_SourceCharacterLightConstants[6];
    source[6]=g_SourceCharacterLightConstants[7];
    source[7]=g_SourceCharacterLightConstants[8];
    source[8]=g_SourceCharacterLightConstants[9];
    source[9]=g_SourceCharacterLightConstants[10];
    source[9].x=(g_SourceCharacterTime.xxxx).x;
    source[10]=g_SourceCharacterLightConstants[11];
    source[11]=g_SourceCharacterLightConstants[12];
    source[12]=g_SourceCharacterLightConstants[13];
    source[13]=g_SourceCharacterLightConstants[14];
    source[14]=g_SourceCharacterLightConstants[15];
    source[15]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: mul r0.xy, cb0[2].zwzz, cb0[9].xxxx
    r0.xy = ((source[2].zwzz)*(source[9].xxxx)).xy;
    // 2: add r1.xyz, v6.xyzx, cb0[0].xyzx
    r1.xyz = ((v6.xyzx)+(source[0].xyzx)).xyz;
    // 3: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.010000, 0.010000), r0.xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,0.010000,0.010000))+(r0.xxxy)).zw;
    // 4: mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: dp2 r0.z, r2.xyxx, r2.xyxx
    r0.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 8: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 9: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 10: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 11: add r0.z, r0.z, l(0.000010)
    r0.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 12: mul r3.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000)
    r3.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))).xy;
    // 13: mad r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000), r3.xyxx
    r0.xy = ((r0.xyxx)*(float4(-0.500000,-0.500000,0.000000,0.000000))+(r3.xyxx)).xy;
    // 14: mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // 15: mul r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 17: mad r4.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r0.x, r4.xyxx, r4.xyxx
    r0.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 19: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 21: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 22: add r4.z, r0.z, r0.x
    r4.z = ((r0.zzzz)+(r0.xxxx)).z;
    // 23: mov r2.z, l(0.000010)
    r2.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // 24: add r0.xyz, r2.xyzx, r4.xyzx
    r0.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 25: mul r2.xyzw, r0.xyxy, cb0[9].yyzz
    r2.xyzw = ((r0.xyxy)*(source[9].yyzz)).xyzw;
    // 26: mul r3.zw, cb0[3].zzzw, cb0[9].xxxx
    r3.zw = ((source[3].zzzw)*(source[9].xxxx)).zw;
    // 27: mad r4.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000), r3.zwzz
    r4.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))+(r3.zwzz)).xy;
    // 28: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000), r3.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,-0.500000,-0.500000))+(r3.xxxy)).zw;
    // 29: mad r3.xy, cb0[9].xxxx, cb0[8].zwzz, r3.xyxx
    r3.xy = ((source[9].xxxx)*(source[8].zwzz)+(r3.xyxx)).xy;
    // 30: mul r3.zw, r3.zzzw, cb0[3].xxxy
    r3.zw = ((r3.zzzw)*(source[3].xxxy)).zw;
    // 31: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), r2.zzzw
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(r2.zzzw)).zw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, r3.zwzz, t1.zwxy, s2, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 33: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 34: mad r2.zw, r4.xxxy, cb0[3].xxxy, r2.zzzw
    r2.zw = ((r4.xxxy)*(source[3].xxxy)+(r2.zzzw)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.zwzz, t1.zwxy, s2, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 36: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 37: mad r2.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), r2.zzzw
    r2.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(r2.zzzw)).zw;
    // 38: add r2.zw, r2.zzzw, l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 39: mad r2.xy, cb0[9].wwww, r2.zwzz, r2.xyxx
    r2.xy = ((source[9].wwww)*(r2.zwzz)+(r2.xyxx)).xy;
    // 40: mov r2.z, r0.z
    r2.z = (r0.zzzz).z;
    // 41: mad r0.xy, cb0[12].yyyy, r0.xyxx, v2.xyxx
    r0.xy = ((source[12].yyyy)*(r0.xyxx)+(v2.xyxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s6, l(0.000000)
    r0.x = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 43: add r0.yzw, -r2.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r0.yzw = ((-(r2.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // 44: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 45: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 46: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 47: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 48: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 49: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 50: mul r1.x, r1.x, cb0[10].x
    r1.x = ((r1.xxxx)*(source[10].xxxx)).x;
    // 51: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 52: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 53: mad r0.yzw, r1.xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 54: mul_sat r1.x, r1.x, cb0[12].x
    r1.x = (saturate((r1.xxxx)*(source[12].xxxx))).x;
    // 55: dp3 r1.y, r0.yzwy, r0.yzwy
    r1.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 56: rsq r1.z, r1.y
    r1.z = (rsqrt(r1.yyyy)).z;
    // 57: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 58: div r2.xyz, r0.yzwy, r1.yyyy
    r2.xyz = ((r0.yzwy)/(r1.yyyy)).xyz;
    // 59: mul r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)*(r1.zzzz)).yzw;
    // 60: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 61: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 62: mul r1.yzw, r1.yyyy, v5.xxyz
    r1.yzw = ((r1.yyyy)*(v5.xxyz)).yzw;
    // 63: dp3 r2.w, r0.yzwy, r1.yzwy
    r2.w = (dot((r0.yzwy).xyz,(r1.yzwy).xyz).xxxx).w;
    // 64: mul r4.xyz, r0.yzwy, r2.wwww
    r4.xyz = ((r0.yzwy)*(r2.wwww)).xyz;
    // 65: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.yzwy
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.yzwy))).xyz;
    // 66: add r3.zw, r4.xxxy, -v2.xxxy
    r3.zw = ((r4.xxxy)+(-(v2.xxxy))).zw;
    // 67: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 0.050000, 0.050000), v2.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,0.050000,0.050000))+(v2.xxxy)).zw;
    // 68: mul r3.zw, r3.zzzw, cb0[11].wwww
    r3.zw = ((r3.zzzw)*(source[11].wwww)).zw;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r3.zwzz, t3.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 70: mul r5.xyz, r5.xyzx, cb0[6].xyzx
    r5.xyz = ((r5.xyzx)*(source[6].xyzx)).xyz;
    // 71: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 72: add r6.xyz, r2.xyzx, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r6.xyz = ((r2.xyzx)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // 73: mad r6.xyz, cb0[11].xxxx, r6.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((source[11].xxxx)*(r6.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 74: dp3 r1.x, r6.xyzx, r1.yzwy
    r1.x = (dot((r6.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // 75: mul r3.zw, r6.xxxy, r1.xxxx
    r3.zw = ((r6.xxxy)*(r1.xxxx)).zw;
    // 76: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), -r1.yyyz
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(-(r1.yyyz))).zw;
    // 77: dp3 r1.x, r2.xyzx, r1.yzwy
    r1.x = (dot((r2.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // 78: mul r1.yz, r2.xxyx, cb0[10].yyyy
    r1.yz = ((r2.xxyx)*(source[10].yyyy)).yz;
    // 79: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 80: mad r1.x, -r1.x, l(0.500000), l(1.000000)
    r1.x = ((-(r1.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 81: mul r2.xy, r3.zwzz, cb0[5].xyxx
    r2.xy = ((r3.zwzz)*(source[5].xyxx)).xy;
    // 82: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 83: mul r2.xyz, r2.xyzx, cb0[4].xyzx
    r2.xyz = ((r2.xyzx)*(source[4].xyzx)).xyz;
    // 84: log r1.w, |r1.x|
    r1.w = (log2(abs(r1.xxxx))).w;
    // 85: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 86: mul r1.w, r1.w, cb0[11].y
    r1.w = ((r1.wwww)*(source[11].yyyy)).w;
    // 87: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 88: mul r1.w, r1.w, cb0[11].z
    r1.w = ((r1.wwww)*(source[11].zzzz)).w;
    // 89: movc r1.x, r1.x, l(0), r1.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).x;
    // 90: mad r2.xyz, r1.xxxx, r2.xyzx, r5.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 91: add r5.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mul r1.xw, r3.xxxy, cb0[8].xxxy
    r1.xw = ((r3.xxxy)*(source[8].xxxy)).xw;
    // 93: mad r3.xy, r3.xyxx, cb0[8].xyxx, r1.yzyy
    r3.xy = ((r3.xyxx)*(source[8].xyxx)+(r1.yzyy)).xy;
    // 94: mad r1.xy, r1.xwxx, l(2.000000, 2.000000, 0.000000, 0.000000), r1.yzyy
    r1.xy = ((r1.xwxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r1.yzyy)).xy;
    // 95: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t4.xyzw, s5, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 97: mad r1.xyz, -r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), r1.xyzx
    r1.xyz = ((-(r3.xyzx))*(float4(2.000000,2.000000,2.000000,0.000000))+(r1.xyzx)).xyz;
    // 98: add r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)+(r3.xyzx)).xyz;
    // 99: mad r1.xyz, r0.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 100: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: add r3.xyz, -r1.xyzx, r1.wwww
    r3.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 102: mad r1.xyz, cb0[12].wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((source[12].wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 103: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 104: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 105: mad r1.xyz, r1.xyzx, r5.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 106: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 107: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 108: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 109: dp3 r1.w, v3.xyzx, v3.xyzx
    r1.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 110: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 111: mul r2.xyz, r1.wwww, v3.xyzx
    r2.xyz = ((r1.wwww)*(v3.xyzx)).xyz;
    // 112: dp3_sat r0.y, r0.yzwy, r2.xyzx
    r0.y = (saturate(dot((r0.yzwy).xyz,(r2.xyzx).xyz).xxxx)).y;
    // 113: dp3_sat r0.z, r4.xyzx, r2.xyzx
    r0.z = (saturate(dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx)).z;
    // 114: lt r0.w, r0.y, l(0.000001)
    r0.w = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 115: movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 116: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 117: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 118: mul r0.w, r0.w, cb0[13].y
    r0.w = ((r0.wwww)*(source[13].yyyy)).w;
    // 119: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 120: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 121: mad r2.xyz, cb0[13].xxxx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((source[13].xxxx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 122: mul r2.xyz, r0.zzzz, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)).xyz;
    // 123: mad r0.yzw, r1.xxyz, r0.yyyy, r2.xxyz
    r0.yzw = ((r1.xxyz)*(r0.yyyy)+(r2.xxyz)).yzw;
    // 124: mul o0.xyz, r0.yzwy, cb0[15].xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[15].xyzx)).xyz;
    // 125: mul r0.yzw, v6.yyyy, cb1[1].xxyw
    r0.yzw = ((v6.yyyy)*(projection[1].xxyw)).yzw;
    // 126: mad r0.yzw, cb1[0].xxyw, v6.xxxx, r0.yyzw
    r0.yzw = ((projection[0].xxyw)*(v6.xxxx)+(r0.yyzw)).yzw;
    // 127: mad r0.yzw, cb1[2].xxyw, v6.zzzz, r0.yyzw
    r0.yzw = ((projection[2].xxyw)*(v6.zzzz)+(r0.yyzw)).yzw;
    // 128: mad r0.yzw, cb1[3].xxyw, v6.wwww, r0.yyzw
    r0.yzw = ((projection[3].xxyw)*(v6.wwww)+(r0.yyzw)).yzw;
    // 129: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 130: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 131: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t6.yxzw, s0, l(0.000000)
    r0.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 132: min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // 133: mad r0.z, r0.y, cb2[1].z, -cb2[1].w
    r0.z = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).z;
    // 134: mad r0.y, r0.y, cb2[1].x, cb2[1].y
    r0.y = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // 135: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 136: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 137: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 138: add r0.z, -cb0[13].z, l(1.000000)
    r0.z = ((-(source[13].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 139: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 140: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 141: mul r0.y, r0.y, cb0[13].w
    r0.y = ((r0.yyyy)*(source[13].wwww)).y;
    // 142: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 143: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 144: mul r0.z, r0.z, cb0[14].x
    r0.z = ((r0.zzzz)*(source[14].xxxx)).z;
    // 145: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 146: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 147: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 148: add r0.z, -r0.y, l(1.000000)
    r0.z = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 149: mad r0.x, r0.x, r0.z, r0.y
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).x;
    // 150: mul o0.w, r0.x, cb0[0].w
    output.targets[0].w = ((r0.xxxx)*(source[0].wwww)).w;
    // 151: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 152: ret
    return output;
}

// source.character.static-map-native-1114.v1 / source program ac19caace03d84468552f54a3e5dcd24
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1114(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xyz, v6.xyzx, cb0[0].xyzx
    r0.xyz = ((v6.xyzx)+(source[0].xyzx)).xyz;
    // 2: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 3: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 4: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 5: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 6: dp3 r0.w, cb0[4].xyzx, cb0[4].xyzx
    r0.w = (dot((source[4].xyzx).xyz,(source[4].xyzx).xyz).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: div r1.xyz, cb0[4].xyzx, r0.wwww
    r1.xyz = ((source[4].xyzx)/(r0.wwww)).xyz;
    // 9: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 10: mad r0.x, -r0.x, l(0.499000), l(0.500000)
    r0.x = ((-(r0.xxxx))*(float4(0.499000,0.499000,0.499000,0.499000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 11: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 12: mul r0.y, cb0[7].x, l(0.017453)
    r0.y = ((source[7].xxxx)*(float4(0.017453,0.017453,0.017453,0.017453))).y;
    // 13: sincos null, r0.y, r0.y
    r0.y = (cos(r0.yyyy)).y;
    // 14: max r0.y, r0.y, l(-0.999990)
    r0.y = (max(r0.yyyy,float4(-0.999990,-0.999990,-0.999990,-0.999990))).y;
    // 15: min r0.y, r0.y, l(0.999990)
    r0.y = (min(r0.yyyy,float4(0.999990,0.999990,0.999990,0.999990))).y;
    // 16: mad r0.y, -r0.y, l(0.500000), l(0.500000)
    r0.y = ((-(r0.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 17: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 18: mul r0.y, r0.y, l(0.693147)
    r0.y = ((r0.yyyy)*(float4(0.693147,0.693147,0.693147,0.693147))).y;
    // 19: div r0.y, l(-0.301030), r0.y
    r0.y = ((float4(-0.301030,-0.301030,-0.301030,-0.301030))/(r0.yyyy)).y;
    // 20: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 21: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 22: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mul r0.y, r0.x, cb0[7].z
    r0.y = ((r0.xxxx)*(source[7].zzzz)).y;
    // 24: mov r1.x, v2.y
    r1.x = (v2.yyyy).x;
    // 25: mov r1.y, cb0[5].z
    r1.y = (source[5].zzzz).y;
    // 26: add r0.zw, -r1.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r1.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 27: ge r1.x, v2.y, cb0[5].z
    r1.x = (asfloat((uint4)((v2.yyyy)>=(source[5].zzzz)) * 0xffffffffu)).x;
    // 28: movc r0.z, r1.x, r0.z, v2.y
    r0.z = ((asuint(r1.xxxx) != 0u) ? (r0.zzzz) : (v2.yyyy)).z;
    // 29: movc r0.w, r1.x, r0.w, cb0[5].z
    r0.w = ((asuint(r1.xxxx) != 0u) ? (r0.wwww) : (source[5].zzzz)).w;
    // 30: movc_sat r1.x, r1.x, cb0[5].w, cb0[6].x
    r1.x = (saturate((asuint(r1.xxxx) != 0u) ? (source[5].wwww) : (source[6].xxxx))).x;
    // 31: div r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)/(r0.wwww)).z;
    // 32: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 34: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 35: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 36: mul_sat r0.z, r0.z, cb0[6].y
    r0.z = (saturate((r0.zzzz)*(source[6].yyyy))).z;
    // 37: mad r0.w, -cb0[7].z, r0.x, r0.z
    r0.w = ((-(source[7].zzzz))*(r0.xxxx)+(r0.zzzz)).w;
    // 38: mad r0.y, cb0[7].z, r0.w, r0.y
    r0.y = ((source[7].zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 39: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 40: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 41: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 42: mad r2.xyz, r2.xyzx, r0.yyyy, -r1.xyzx
    r2.xyz = ((r2.xyzx)*(r0.yyyy)+(-(r1.xyzx))).xyz;
    // 43: mad r0.xyw, r0.xxxx, r2.xyxz, r1.xyxz
    r0.xyw = ((r0.xxxx)*(r2.xyxz)+(r1.xyxz)).xyw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 45: mul r2.xyz, r1.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(source[1].xyzx)).xyz;
    // 46: mad r1.xyz, -r1.xyzx, cb0[1].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[1].xyzx)+(r1.xyzx)).xyz;
    // 47: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 48: add r0.xyw, r0.xyxw, -r1.xyxz
    r0.xyw = ((r0.xyxw)+(-(r1.xyxz))).xyw;
    // 49: add r2.x, -r0.z, l(1.000000)
    r2.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: mad_sat r0.z, -r2.x, r1.w, r0.z
    r0.z = (saturate((-(r2.xxxx))*(r1.wwww)+(r0.zzzz))).z;
    // 51: mad r0.xyz, r0.zzzz, r0.xywx, r1.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r1.xyzx)).xyz;
    // 52: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 53: dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 54: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 55: mul r0.w, r0.w, v3.z
    r0.w = ((r0.wwww)*(v3.zzzz)).w;
    // 56: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 57: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 58: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 59: mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // 60: mov o0.w, cb0[0].w
    output.targets[0].w = (source[0].wwww).w;
    // 61: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 62: ret
    return output;
}

// source.character.static-map-native-1115.v1 / source program 542dce5dc946974c93c7945db2d30b25
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1115(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=float4(input.lightColor,1.f);
    source[6].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xyz, v6.xyzx, cb0[0].xyzx
    r0.xyz = ((v6.xyzx)+(source[0].xyzx)).xyz;
    // 2: dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 3: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 4: mul r0.w, r0.w, v3.z
    r0.w = ((r0.wwww)*(v3.zzzz)).w;
    // 5: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[6].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[6].xxxx)) * 0xffffffffu)).x;
    // 6: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 7: mul r1.xyz, v6.yyyy, cb1[1].xywx
    r1.xyz = ((v6.yyyy)*(projection[1].xywx)).xyz;
    // 8: mad r1.xyz, cb1[0].xywx, v6.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v6.xxxx)+(r1.xyzx)).xyz;
    // 9: mad r1.xyz, cb1[2].xywx, v6.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v6.zzzz)+(r1.xyzx)).xyz;
    // 10: mad r1.xyz, cb1[3].xywx, v6.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v6.wwww)+(r1.xyzx)).xyz;
    // 11: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 12: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 13: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s0
    r1.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 14: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 15: else
    } else {
    // 16: mov r1.xyz, l(1.000000,1.000000,1.000000,0)
    r1.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 17: endif
    }
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 19: mul r3.xyz, r2.xyzx, cb0[1].xyzx
    r3.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 20: mad r2.xyz, -r2.xyzx, cb0[1].xyzx, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(source[1].xyzx)+(r2.xyzx)).xyz;
    // 21: mad r2.xyz, r2.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 22: mul r3.xyz, cb0[2].xyzx, cb0[2].wwww
    r3.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 23: dp3 r1.w, cb0[3].xyzx, cb0[3].xyzx
    r1.w = (dot((source[3].xyzx).xyz,(source[3].xyzx).xyz).xxxx).w;
    // 24: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 25: div r4.xyz, cb0[3].xyzx, r1.wwww
    r4.xyz = ((source[3].xyzx)/(r1.wwww)).xyz;
    // 26: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 27: dp3 r1.w, r0.xyzx, r0.xyzx
    r1.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 28: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 29: div r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)/(r1.wwww)).xyz;
    // 30: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 31: add r0.x, -r0.x, l(-0.999500)
    r0.x = ((-(r0.xxxx))+(float4(-0.999500,-0.999500,-0.999500,-0.999500))).x;
    // 32: mul_sat r0.x, r0.x, l(2000.000000)
    r0.x = (saturate((r0.xxxx)*(float4(2000.000000,2000.000000,2000.000000,2000.000000)))).x;
    // 33: mad r0.xyz, r0.xxxx, r3.xyzx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 34: mul r0.xyz, r0.xyzx, cb0[4].xxxx
    r0.xyz = ((r0.xyzx)*(source[4].xxxx)).xyz;
    // 35: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: mul r2.xyz, r0.wwww, cb2[3].xyzx
    r2.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 37: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 38: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 39: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 40: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 41: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 42: ret
    return output;
}

// source.character.static-map-native-1116.v1 / source program 8f0b0bf00e44a643a998502e54259b89
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1116(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[10]=g_SourceCharacterLightConstants[10];
    source[10].y=(g_SourceCharacterTime.xxxx).x;
    source[10].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[11]=g_SourceCharacterLightConstants[13];
    source[11].x=((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))))).x;
    source[12]=float4(input.lightColor,1.f);
    source[13].x=1.f;
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
    // 8: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].xxxx)) * 0xffffffffu)).w;
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
    // 33: mov_sat r0.y, -r0.x
    r0.y = (saturate(-(r0.xxxx))).y;
    // 34: lt r0.z, r0.y, l(0.000001)
    r0.z = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 35: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 36: mul r0.y, r0.y, cb0[10].x
    r0.y = ((r0.yyyy)*(source[10].xxxx)).y;
    // 37: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 38: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 39: mov r5.xw, l(1.000000,0,0,2.000000)
    r5.xw = (float4(1.000000,asfloat(0u),asfloat(0u),2.000000)).xw;
    // 40: mov r5.yz, cb0[3].yyxy
    r5.yz = (source[3].yyxy).yz;
    // 41: mul r0.zw, r5.xxxy, v2.xxxy
    r0.zw = ((r5.xxxy)*(v2.xxxy)).zw;
    // 42: mad r5.xy, r5.zwzz, r0.zwzz, cb0[4].xyxx
    r5.xy = ((r5.zwzz)*(r0.zwzz)+(source[4].xyxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r5.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 44: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: dp2 r1.w, r6.xyxx, r6.xyxx
    r1.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 46: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 48: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 49: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 50: mul r7.xy, v2.xyxx, cb0[3].xyxx
    r7.xy = ((v2.xyxx)*(source[3].xyxx)).xy;
    // 51: mad r7.xy, r7.xyxx, l(-1.000000, 2.000000, 0.000000, 0.000000), cb0[5].xyxx
    r7.xy = ((r7.xyxx)*(float4(-1.000000,2.000000,0.000000,0.000000))+(source[5].xyxx)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r7.zw, r7.xyxx, t1.zwxy, s2, l(0.000000)
    r7.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 53: mad r8.xy, r7.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r7.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 54: dp2 r1.w, r8.xyxx, r8.xyxx
    r1.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 55: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 57: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 58: add r8.z, r1.w, l(0.000010)
    r8.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 59: add r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)+(r8.xyzx)).xyz;
    // 60: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 61: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 62: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 63: dp3 r1.x, r1.xyzx, r9.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 64: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: mad r1.x, -r1.x, l(0.500000), l(1.000000)
    r1.x = ((-(r1.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: mul r1.xyz, r1.xxxx, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(source[6].xyzx)).xyz;
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
    // 72: mad r0.zw, r5.zzzw, r0.zzzw, cb0[8].xxxy
    r0.zw = ((r5.zzzw)*(r0.zzzw)+(source[8].xxxy)).zw;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.zwzz, t2.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 74: mad_sat r5.xyz, r5.xyzx, r7.yzwy, r7.yzwy
    r5.xyz = (saturate((r5.xyzx)*(r7.yzwy)+(r7.yzwy))).xyz;
    // 75: add r7.yzw, -r6.xxyz, r8.xxyz
    r7.yzw = ((-(r6.xxyz))+(r8.xxyz)).yzw;
    // 76: mad r6.xyz, r7.xxxx, r7.yzwy, r6.xyzx
    r6.xyz = ((r7.xxxx)*(r7.yzwy)+(r6.xyzx)).xyz;
    // 77: dp3 r0.z, r2.xyzx, r6.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 78: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 79: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 80: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 81: mul r2.xyw, r5.xyxz, r0.zzzz
    r2.xyw = ((r5.xyxz)*(r0.zzzz)).xyw;
    // 82: add r0.z, cb0[7].w, l(-0.030000)
    r0.z = ((source[7].wwww)+(float4(-0.030000,-0.030000,-0.030000,-0.030000))).z;
    // 83: mad r2.xyw, r2.xyxw, r0.zzzz, l(0.030000, 0.030000, 0.000000, 0.030000)
    r2.xyw = ((r2.xyxw)*(r0.zzzz)+(float4(0.030000,0.030000,0.000000,0.030000))).xyw;
    // 84: mul r2.xyw, r2.xyxw, cb0[7].xyxz
    r2.xyw = ((r2.xyxw)*(source[7].xyxz)).xyw;
    // 85: mad r0.yzw, r0.yyyy, r1.xxyz, r2.xxyw
    r0.yzw = ((r0.yyyy)*(r1.xxyz)+(r2.xxyw)).yzw;
    // 86: mul r1.x, r5.x, cb0[11].y
    r1.x = ((r5.xxxx)*(source[11].yyyy)).x;
    // 87: add r1.y, -r4.w, l(0.200000)
    r1.y = ((-(r4.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 88: mul_sat r1.y, r1.y, l(5.000000)
    r1.y = (saturate((r1.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000)))).y;
    // 89: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 90: add r0.yzw, -r4.xxyz, r0.yyzw
    r0.yzw = ((-(r4.xxyz))+(r0.yyzw)).yzw;
    // 91: mad r0.yzw, r1.xxxx, r0.yyzw, r4.xxyz
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 92: add r1.x, -r7.x, l(1.000000)
    r1.x = ((-(r7.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 93: mul r1.yzw, cb0[9].xxyz, cb0[9].wwww
    r1.yzw = ((source[9].xxyz)*(source[9].wwww)).yzw;
    // 94: add r0.x, -r0.x, l(-0.999500)
    r0.x = ((-(r0.xxxx))+(float4(-0.999500,-0.999500,-0.999500,-0.999500))).x;
    // 95: mul_sat r0.x, r0.x, l(2000.000000)
    r0.x = (saturate((r0.xxxx)*(float4(2000.000000,2000.000000,2000.000000,2000.000000)))).x;
    // 96: mul r1.yzw, r1.yyzw, r0.xxxx
    r1.yzw = ((r1.yyzw)*(r0.xxxx)).yzw;
    // 97: mad r0.xyz, r1.xxxx, r1.yzwy, r0.yzwy
    r0.xyz = ((r1.xxxx)*(r1.yzwy)+(r0.yzwy)).xyz;
    // 98: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 99: max r0.w, r2.z, l(0.000000)
    r0.w = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 100: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 101: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 102: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 103: mul o0.xyz, r0.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 104: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 105: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 106: ret
    return output;
}

// source.character.static-map-native-1117.v1 / source program 4e8401e314243845a75ad21e6f1e7f41
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1117(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 6: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].y
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].yyyy)) * 0xffffffffu)).x;
    // 7: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 8: mul r1.xyzw, v8.yyyy, cb0[7].xyzw
    r1.xyzw = ((v8.yyyy)*(source[7].xyzw)).xyzw;
    // 9: mad r1.xyzw, cb0[6].xyzw, v8.xxxx, r1.xyzw
    r1.xyzw = ((source[6].xyzw)*(v8.xxxx)+(r1.xyzw)).xyzw;
    // 10: mad r1.xyzw, cb0[8].xyzw, v8.zzzz, r1.xyzw
    r1.xyzw = ((source[8].xyzw)*(v8.zzzz)+(r1.xyzw)).xyzw;
    // 11: mad r1.xyzw, cb0[9].xyzw, v8.wwww, r1.xyzw
    r1.xyzw = ((source[9].xyzw)*(v8.wwww)+(r1.xyzw)).xyzw;
    // 12: div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // 13: sample_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t1.xyzw, s2
    r2.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 14: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 15: mov r3.yz, cb0[10].wwzw
    r3.yz = (source[10].wwzw).yz;
    // 16: add r3.xyzw, r1.xyxy, r3.xyzw
    r3.xyzw = ((r1.xyxy)+(r3.xyzw)).xyzw;
    // 17: sample_indexable(texture2d)(float,float,float,float) r2.y, r3.xyxx, t1.yxzw, s2
    r2.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 18: sample_indexable(texture2d)(float,float,float,float) r2.z, r3.zwzz, t1.yzxw, s2
    r2.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 19: add r3.xy, r1.xyxx, cb0[10].zwzz
    r3.xy = ((r1.xyxx)+(source[10].zwzz)).xy;
    // 20: sample_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t1.yzwx, s2
    r2.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 21: lt r2.xyzw, r1.zzzz, r2.xyzw
    r2.xyzw = (asfloat((uint4)((r1.zzzz)<(r2.xyzw)) * 0xffffffffu)).xyzw;
    // 22: and r3.xyzw, r2.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r3.xyzw = (asfloat(asuint(r2.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 23: mul r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)*(source[10].xyxx)).xy;
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
    // 30: mul r1.xyz, r1.xxxx, cb0[11].xxxx
    r1.xyz = ((r1.xxxx)*(source[11].xxxx)).xyz;
    // 31: else
    } else {
    // 32: mov r1.xyz, l(1.000000,1.000000,1.000000,0)
    r1.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 33: endif
    }
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 35: mul r3.xyz, cb0[1].xyzx, cb0[3].yyyy
    r3.xyz = ((source[1].xyzx)*(source[3].yyyy)).xyz;
    // 36: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 38: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 39: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 40: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 42: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 43: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: mul r4.xy, r4.xyxx, cb0[3].xxxx
    r4.xy = ((r4.xyxx)*(source[3].xxxx)).xy;
    // 45: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 46: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 47: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 48: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 49: dp3 r1.w, r4.xyzx, r0.yzwy
    r1.w = (dot((r4.xyzx).xyz,(r0.yzwy).xyz).xxxx).w;
    // 50: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 51: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mul r5.xyz, cb0[2].xyzx, cb0[3].zzzz
    r5.xyz = ((source[2].xyzx)*(source[3].zzzz)).xyz;
    // 53: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 54: mad r0.xyz, v7.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v7.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 55: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 56: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 57: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 58: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 59: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 60: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 61: mul r0.x, r0.x, cb0[3].w
    r0.x = ((r0.xxxx)*(source[3].wwww)).x;
    // 62: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 63: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 64: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 65: mul r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 66: mad r0.xyz, r3.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 67: mul r2.xyz, r1.wwww, cb2[3].xyzx
    r2.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 68: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 69: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 70: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 71: mul_sat r0.x, r2.w, cb0[4].x
    r0.x = (saturate((r2.wwww)*(source[4].xxxx))).x;
    // 72: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 73: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 74: ret
    return output;
}

// source.character.static-map-native-1118.v1 / source program a7c63572a41d744db7fb27ff82e905d5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1118(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterLightConstants[0];
    source[3]=g_SourceCharacterLightConstants[1];
    source[4]=g_SourceCharacterLightConstants[3];
    source[5]=g_SourceCharacterLightConstants[4];
    source[6]=g_SourceCharacterLightConstants[5];
    source[7]=g_SourceCharacterLightConstants[6];
    source[8]=g_SourceCharacterLightConstants[7];
    source[9]=g_SourceCharacterLightConstants[8];
    source[10]=g_SourceCharacterLightConstants[9];
    source[11]=g_SourceCharacterLightConstants[10];
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
    // 23: mul r5.zw, v4.xxxy, cb0[3].xxxy
    r5.zw = ((v4.xxxy)*(source[3].xxxy)).zw;
    // 24: mul r7.xy, r5.zwzz, cb0[7].xxxx
    r7.xy = ((r5.zwzz)*(source[7].xxxx)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r7.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 26: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 27: mul r7.xy, r7.xyxx, cb0[7].yyyy
    r7.xy = ((r7.xyxx)*(source[7].yyyy)).xy;
    // 28: mad r5.xy, cb0[6].xxxx, r5.xyxx, r7.xyxx
    r5.xy = ((source[6].xxxx)*(r5.xyxx)+(r7.xyxx)).xy;
    // 29: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
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
    // 40: mul r0.xy, cb0[0].xyxx, cb0[7].wwww
    r0.xy = ((source[0].xyxx)*(source[7].wwww)).xy;
    // 41: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 42: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 43: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 44: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 45: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: mad r0.x, r0.x, l(0.500000), cb0[8].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).x;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r5.zwzz, t2.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 48: mul r0.y, r6.z, r6.z
    r0.y = ((r6.zzzz)*(r6.zzzz)).y;
    // 49: mul_sat r0.y, r0.y, r7.w
    r0.y = (saturate((r0.yyyy)*(r7.wwww))).y;
    // 50: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: mul r1.xy, v4.xyxx, cb0[8].wwww
    r1.xy = ((v4.xyxx)*(source[8].wwww)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 53: mul r0.z, r8.w, r8.w
    r0.z = ((r8.wwww)*(r8.wwww)).z;
    // 54: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 55: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 56: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 57: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 58: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 59: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 60: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 61: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 62: add r1.xyz, -r6.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r6.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 63: mad r1.xyz, r0.yyyy, r1.xyzx, r6.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 64: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 65: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 66: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 67: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].xxxx)) * 0xffffffffu)).y;
    // 68: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 69: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 70: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 71: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 72: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 73: else
    } else {
    // 74: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 75: endif
    }
    // 76: add r6.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: mul r9.xyz, cb0[4].xyzx, cb0[9].xxxx
    r9.xyz = ((source[4].xyzx)*(source[9].xxxx)).xyz;
    // 78: mul r10.xyz, r7.xyzx, r9.xyzx
    r10.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 79: mul r11.xyz, cb0[5].xyzx, cb0[9].yyyy
    r11.xyz = ((source[5].xyzx)*(source[9].yyyy)).xyz;
    // 80: mul r12.xyz, r8.xyzx, r11.xyzx
    r12.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 81: dp3 r0.y, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 82: mad r8.xyz, -r11.xyzx, r8.xyzx, r0.yyyy
    r8.xyz = ((-(r11.xyzx))*(r8.xyzx)+(r0.yyyy)).xyz;
    // 83: mad r8.xyz, cb0[9].wwww, r8.xyzx, r12.xyzx
    r8.xyz = ((source[9].wwww)*(r8.xyzx)+(r12.xyzx)).xyz;
    // 84: mad r7.xyz, -r7.xyzx, r9.xyzx, r8.xyzx
    r7.xyz = ((-(r7.xyzx))*(r9.xyzx)+(r8.xyzx)).xyz;
    // 85: mad r0.xyz, r0.xxxx, r7.xyzx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r7.xyzx)+(r10.xyzx)).xyz;
    // 86: mul r7.xyz, r0.xyzx, cb0[10].xxxx
    r7.xyz = ((r0.xyzx)*(source[10].xxxx)).xyz;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.zwzz, t4.yzxw, s5, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 88: mul r1.w, r5.y, cb0[10].z
    r1.w = ((r5.yyyy)*(source[10].zzzz)).w;
    // 89: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 90: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 91: mul r1.w, r1.w, cb0[10].w
    r1.w = ((r1.wwww)*(source[10].wwww)).w;
    // 92: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 93: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 94: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mad r0.xyz, cb0[10].yyyy, r0.xyzx, -r7.xyzx
    r0.xyz = ((source[10].yyyy)*(r0.xyzx)+(-(r7.xyzx))).xyz;
    // 96: mad r0.xyz, r2.wwww, r0.xyzx, r7.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r7.xyzx)).xyz;
    // 97: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 98: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 99: mov_sat r2.w, cb0[11].x
    r2.w = (saturate(source[11].xxxx)).w;
    // 100: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 101: mul r3.w, r5.x, cb0[11].z
    r3.w = ((r5.xxxx)*(source[11].zzzz)).w;
    // 102: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 103: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 104: mul r3.w, r3.w, cb0[11].w
    r3.w = ((r3.wwww)*(source[11].wwww)).w;
    // 105: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 106: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 107: max r3.w, r3.w, cb0[1].x
    r3.w = (max(r3.wwww,source[1].xxxx)).w;
    // 108: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 110: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 111: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 112: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 113: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 114: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 115: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 116: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 118: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 119: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 122: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 123: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 125: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 126: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 127: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 128: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 129: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 130: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 131: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 132: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 133: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 134: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 135: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 136: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 137: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 138: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 139: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 140: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 141: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 142: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 143: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 145: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 146: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 147: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 148: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 149: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 151: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 152: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 153: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 154: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 156: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 157: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 158: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 159: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 160: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 161: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 162: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 163: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 164: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 165: mul o0.xyz, r0.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 166: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 167: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 168: ret
    return output;
}

// source.character.static-map-native-1119.v1 / source program a7354cbe370dd7439e8bd6c3cbc58d5f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1119(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[6];
    source[3]=g_SourceCharacterLightConstants[7];
    source[4]=g_SourceCharacterLightConstants[11];
    source[5]=g_SourceCharacterLightConstants[12];
    source[6]=g_SourceCharacterLightConstants[13];
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r2.xy, r2.xyxx, cb0[3].xxxx
    r2.xy = ((r2.xyxx)*(source[3].xxxx)).xy;
    // 15: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 16: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 17: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 18: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 19: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 21: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.w, r3.w, l(-0.333300)
    r1.w = ((r3.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 24: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 26: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[8].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[8].xxxx)) * 0xffffffffu)).w;
    // 27: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 28: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 29: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 30: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t3.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 31: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 32: else
    } else {
    // 33: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 34: endif
    }
    // 35: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 36: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r6.xyz, -r3.xyzx, r1.wwww
    r6.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 38: mad r3.xyz, cb0[4].yyyy, r6.xyzx, r3.xyzx
    r3.xyz = ((source[4].yyyy)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 39: mul r6.xyz, cb0[2].xyzx, cb0[4].zzzz
    r6.xyz = ((source[2].xyzx)*(source[4].zzzz)).xyz;
    // 40: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 41: mul r6.xyz, r3.xyzx, cb0[4].wwww
    r6.xyz = ((r3.xyzx)*(source[4].wwww)).xyz;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t2.yzxw, s3, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 43: mul r1.w, r7.y, cb0[5].y
    r1.w = ((r7.yyyy)*(source[5].yyyy)).w;
    // 44: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 45: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 46: mul r1.w, r1.w, cb0[5].z
    r1.w = ((r1.wwww)*(source[5].zzzz)).w;
    // 47: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 48: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 49: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mad r3.xyz, cb0[5].xxxx, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[5].xxxx)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 51: mad r3.xyz, r2.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 52: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 53: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 54: mov_sat r2.w, cb0[5].w
    r2.w = (saturate(source[5].wwww)).w;
    // 55: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 56: mul r3.w, r7.x, cb0[6].y
    r3.w = ((r7.xxxx)*(source[6].yyyy)).w;
    // 57: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 58: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 59: mul r3.w, r3.w, cb0[6].z
    r3.w = ((r3.wwww)*(source[6].zzzz)).w;
    // 60: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 61: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 62: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 63: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 65: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 66: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 67: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 68: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 69: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 70: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 71: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: dp3_sat r1.x, r2.xyzx, r1.xyzx
    r1.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 73: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 74: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 75: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 76: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 77: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 78: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 79: mad r0.yzw, -r3.xxyz, r1.wwww, r3.xxyz
    r0.yzw = ((-(r3.xxyz))*(r1.wwww)+(r3.xxyz)).yzw;
    // 80: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 81: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 82: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 83: mad r2.x, r4.w, r1.z, -r4.w
    r2.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 84: mad r2.x, r2.x, r4.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 85: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 86: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 87: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 88: mad r2.x, -r3.w, r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 89: mad r2.y, r5.w, r2.x, r1.y
    r2.y = ((r5.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 90: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 91: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 92: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 93: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 94: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 95: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 96: mad r2.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r2.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 97: mad r2.xyz, r1.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 98: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 100: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 101: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 102: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 103: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 104: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: max r3.xyz, r2.xyzx, r1.wwww
    r3.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 106: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 107: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 108: mad r2.xyz, r1.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 109: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 110: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 111: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 112: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 113: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 114: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 115: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 116: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 117: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 119: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 120: mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // 121: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 122: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 123: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 124: ret
    return output;
}

// source.character.static-map-native-1120.v1 / source program 5436af6726b07f4cbbd575adede5982f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1120(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r4.xyzw, v4.xyxy, cb0[4].yyww
    r4.xyzw = ((v4.xyxy)*(source[4].yyww)).xyzw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r4.xyxx, t1.zwxy, s2, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 16: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 17: mul r2.zw, r2.zzzw, cb0[4].zzzz
    r2.zw = ((r2.zzzw)*(source[4].zzzz)).zw;
    // 18: mad r2.xy, cb0[4].xxxx, r2.xyxx, r2.zwzz
    r2.xy = ((source[4].xxxx)*(r2.xyxx)+(r2.zwzz)).xy;
    // 19: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 20: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r4.zwzz, t2.xyzw, s3, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 24: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 25: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 26: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 28: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 29: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 30: mul r5.xy, r3.xyxx, cb0[5].xxxx
    r5.xy = ((r3.xyxx)*(source[5].xxxx)).xy;
    // 31: max r1.w, cb0[5].y, l(0.000000)
    r1.w = (max(source[5].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 33: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 35: add r3.x, -v2.x, l(1.000000)
    r3.x = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul r3.y, r2.z, r2.z
    r3.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 38: mul_sat r3.y, r3.y, r6.w
    r3.y = (saturate((r3.yyyy)*(r6.wwww))).y;
    // 39: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.zwzz, t4.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: mul r3.z, r4.w, r4.w
    r3.z = ((r4.wwww)*(r4.wwww)).z;
    // 42: mul r3.y, r3.z, r3.y
    r3.y = ((r3.zzzz)*(r3.yyyy)).y;
    // 43: mul r3.z, r1.w, r3.y
    r3.z = ((r1.wwww)*(r3.yyyy)).z;
    // 44: mad r3.x, r3.x, r3.z, r3.x
    r3.x = ((r3.xxxx)*(r3.zzzz)+(r3.xxxx)).x;
    // 45: add r1.w, -r1.w, r3.x
    r1.w = ((-(r1.wwww))+(r3.xxxx)).w;
    // 46: mul r3.z, r1.w, r2.w
    r3.z = ((r1.wwww)*(r2.wwww)).z;
    // 47: mad r1.w, -r2.w, r1.w, r3.x
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.xxxx)).w;
    // 48: mad_sat r1.w, r3.y, r1.w, r3.z
    r1.w = (saturate((r3.yyyy)*(r1.wwww)+(r3.zzzz))).w;
    // 49: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 50: add r3.xyz, -r2.xyzx, r5.xyzx
    r3.xyz = ((-(r2.xyzx))+(r5.xyzx)).xyz;
    // 51: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 52: dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 53: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 54: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 55: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).w;
    // 56: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 57: div r3.xy, v8.xyxx, v8.wwww
    r3.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 58: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 59: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t6.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 60: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 61: else
    } else {
    // 62: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 63: endif
    }
    // 64: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 65: mul r7.xyz, cb0[2].xyzx, cb0[5].zzzz
    r7.xyz = ((source[2].xyzx)*(source[5].zzzz)).xyz;
    // 66: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 67: mul r9.xyz, cb0[3].xyzx, cb0[5].wwww
    r9.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // 68: mul r10.xyz, r4.xyzx, r9.xyzx
    r10.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 69: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: mad r4.xyz, -r9.xyzx, r4.xyzx, r2.wwww
    r4.xyz = ((-(r9.xyzx))*(r4.xyzx)+(r2.wwww)).xyz;
    // 71: mad r4.xyz, cb0[6].yyyy, r4.xyzx, r10.xyzx
    r4.xyz = ((source[6].yyyy)*(r4.xyzx)+(r10.xyzx)).xyz;
    // 72: mad r4.xyz, -r6.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r6.xyzx))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 73: mad r4.xyz, r1.wwww, r4.xyzx, r8.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 74: mul r6.xyz, r4.xyzx, cb0[6].zzzz
    r6.xyz = ((r4.xyzx)*(source[6].zzzz)).xyz;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t5.yzxw, s6, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 76: mul r1.w, r7.y, cb0[7].x
    r1.w = ((r7.yyyy)*(source[7].xxxx)).w;
    // 77: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 78: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 79: mul r1.w, r1.w, cb0[7].y
    r1.w = ((r1.wwww)*(source[7].yyyy)).w;
    // 80: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 81: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 82: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: mad r4.xyz, cb0[6].wwww, r4.xyzx, -r6.xyzx
    r4.xyz = ((source[6].wwww)*(r4.xyzx)+(-(r6.xyzx))).xyz;
    // 84: mad r4.xyz, r2.wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 85: mul r4.xyz, r5.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)*(r4.xyzx)).xyz;
    // 86: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 87: mov_sat r2.w, cb0[7].z
    r2.w = (saturate(source[7].zzzz)).w;
    // 88: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 89: mul r3.w, r7.x, cb0[8].x
    r3.w = ((r7.xxxx)*(source[8].xxxx)).w;
    // 90: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 91: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 92: mul r3.w, r3.w, cb0[8].y
    r3.w = ((r3.wwww)*(source[8].yyyy)).w;
    // 93: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 94: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 95: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 96: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 98: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 99: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 100: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 101: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 102: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 103: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 104: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: dp3_sat r1.x, r2.xyzx, r1.xyzx
    r1.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 106: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 107: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 108: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 109: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 110: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 111: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: mad r0.yzw, -r4.xxyz, r1.wwww, r4.xxyz
    r0.yzw = ((-(r4.xxyz))*(r1.wwww)+(r4.xxyz)).yzw;
    // 113: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 114: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 115: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 116: mad r2.x, r4.w, r1.z, -r4.w
    r2.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 117: mad r2.x, r2.x, r4.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 118: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 119: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 120: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 121: mad r2.x, -r3.w, r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 122: mad r2.y, r5.w, r2.x, r1.y
    r2.y = ((r5.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 123: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 124: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 125: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 126: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 127: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 128: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 129: mad r2.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r2.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 130: mad r2.xyz, r1.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 131: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 132: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 133: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 134: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 135: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 136: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 137: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: max r4.xyz, r2.xyzx, r1.wwww
    r4.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 139: add r4.xyz, -r2.xyzx, r4.xyzx
    r4.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 140: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 141: mad r2.xyz, r1.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 142: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 143: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 144: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 145: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 146: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 147: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 148: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 149: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 150: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 151: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 152: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 153: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 154: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 155: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 156: ret
    return output;
}

// source.character.static-map-native-1121.v1 / source program 95eb92236c17784dab2616b732a13384
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1121(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[10]=g_SourceCharacterLightConstants[10];
    source[11]=float4(input.lightColor,1.f);
    source[12].x=1.f;
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
    // 23: mul r7.xyzw, v4.xyxy, cb0[5].yyww
    r7.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r7.xyxx, t1.zwxy, s2, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 25: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 26: mul r5.zw, r5.zzzw, cb0[5].zzzz
    r5.zw = ((r5.zzzw)*(source[5].zzzz)).zw;
    // 27: mad r5.xy, cb0[5].xxxx, r5.xyxx, r5.zwzz
    r5.xy = ((source[5].xxxx)*(r5.xyxx)+(r5.zwzz)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
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
    // 39: mul r8.xy, r6.xyxx, cb0[6].xxxx
    r8.xy = ((r6.xyxx)*(source[6].xxxx)).xy;
    // 40: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 44: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 45: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 46: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 47: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 48: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 49: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 50: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mad r0.x, r0.x, l(0.500000), cb0[7].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].xxxx)).x;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 53: mul r0.y, r5.z, r5.z
    r0.y = ((r5.zzzz)*(r5.zzzz)).y;
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
    // 62: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 63: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 64: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 65: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 66: add r1.xyz, -r5.xyzx, r8.xyzx
    r1.xyz = ((-(r5.xyzx))+(r8.xyzx)).xyz;
    // 67: mad r1.xyz, r0.yyyy, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 68: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 69: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 70: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 71: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
    // 72: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 73: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 74: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 75: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t6.xyzw, s0
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
    // 81: mul r8.xyz, cb0[3].xyzx, cb0[7].yyyy
    r8.xyz = ((source[3].xyzx)*(source[7].yyyy)).xyz;
    // 82: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 83: mul r10.xyz, cb0[4].xyzx, cb0[7].zzzz
    r10.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // 84: mul r11.xyz, r7.xyzx, r10.xyzx
    r11.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 85: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 86: mad r7.xyz, -r10.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r10.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 87: mad r7.xyz, cb0[8].xxxx, r7.xyzx, r11.xyzx
    r7.xyz = ((source[8].xxxx)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 88: mad r6.xyz, -r6.xyzx, r8.xyzx, r7.xyzx
    r6.xyz = ((-(r6.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 89: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 90: mul r6.xyz, r0.xyzx, cb0[8].yyyy
    r6.xyz = ((r0.xyzx)*(source[8].yyyy)).xyz;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t5.yzxw, s6, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 92: mul r1.w, r7.y, cb0[8].w
    r1.w = ((r7.yyyy)*(source[8].wwww)).w;
    // 93: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 94: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 95: mul r1.w, r1.w, cb0[9].x
    r1.w = ((r1.wwww)*(source[9].xxxx)).w;
    // 96: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 97: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 98: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mad r0.xyz, cb0[8].zzzz, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[8].zzzz)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 100: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 101: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 102: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 103: mov_sat r2.w, cb0[9].y
    r2.w = (saturate(source[9].yyyy)).w;
    // 104: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 105: mul r3.w, r7.x, cb0[9].w
    r3.w = ((r7.xxxx)*(source[9].wwww)).w;
    // 106: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 107: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 108: mul r3.w, r3.w, cb0[10].x
    r3.w = ((r3.wwww)*(source[10].xxxx)).w;
    // 109: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 110: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 111: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 112: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 114: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 115: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 116: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 117: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 118: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 119: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 120: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 122: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 123: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 126: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 127: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 129: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 130: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 131: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 132: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 133: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 134: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 135: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 136: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 137: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 138: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 139: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 140: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 141: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 142: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 143: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 144: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 145: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 146: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 147: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 149: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 150: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 151: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 152: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 153: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 155: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 156: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 157: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 158: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 159: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 160: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 161: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 162: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 163: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 164: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 166: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 167: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 168: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 169: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 170: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 171: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 172: ret
    return output;
}

// source.character.static-map-native-1122.v1 / source program 01d9950665dd27449ee76f84eda7738e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1122(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[7]=g_SourceCharacterLightConstants[14];
    source[7].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[8]=g_SourceCharacterLightConstants[15];
    source[9]=g_SourceCharacterLightConstants[16];
    source[10]=g_SourceCharacterLightConstants[17];
    source[11]=float4(input.lightColor,1.f);
    source[12].x=1.f;
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
    // 68: add r0.y, r8.w, l(-0.333300)
    r0.y = ((r8.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).y;
    // 69: lt r0.y, r0.y, l(0.000000)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 70: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 71: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
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
    // 83: mad r7.xyz, cb0[7].wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((source[7].wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 84: mul r8.xyz, cb0[3].xyzx, cb0[8].xxxx
    r8.xyz = ((source[3].xyzx)*(source[8].xxxx)).xyz;
    // 85: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 86: mul r10.xyz, cb0[4].xyzx, cb0[8].yyyy
    r10.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 87: mul r11.xyz, r6.xyzx, r10.xyzx
    r11.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 88: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 89: mad r6.xyz, -r10.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r10.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 90: mad r6.xyz, cb0[8].wwww, r6.xyzx, r11.xyzx
    r6.xyz = ((source[8].wwww)*(r6.xyzx)+(r11.xyzx)).xyz;
    // 91: mad r6.xyz, -r7.xyzx, r8.xyzx, r6.xyzx
    r6.xyz = ((-(r7.xyzx))*(r8.xyzx)+(r6.xyzx)).xyz;
    // 92: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 93: mul r6.xyz, r0.xyzx, cb0[9].xxxx
    r6.xyz = ((r0.xyzx)*(source[9].xxxx)).xyz;
    // 94: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t4.yzxw, s5, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 95: mul r1.w, r7.y, cb0[9].z
    r1.w = ((r7.yyyy)*(source[9].zzzz)).w;
    // 96: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 97: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 98: mul r1.w, r1.w, cb0[9].w
    r1.w = ((r1.wwww)*(source[9].wwww)).w;
    // 99: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 100: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 101: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r0.xyz, cb0[9].yyyy, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[9].yyyy)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 103: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 104: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 105: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 106: mov_sat r2.w, cb0[10].x
    r2.w = (saturate(source[10].xxxx)).w;
    // 107: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 108: mul r3.w, r7.x, cb0[10].z
    r3.w = ((r7.xxxx)*(source[10].zzzz)).w;
    // 109: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 110: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 111: mul r3.w, r3.w, cb0[10].w
    r3.w = ((r3.wwww)*(source[10].wwww)).w;
    // 112: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 113: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 114: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 115: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 117: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 118: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 119: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 120: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 121: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 122: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 123: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 125: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 126: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 129: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 130: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 132: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 133: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 134: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 135: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 136: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 137: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 138: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 139: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 140: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 141: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 142: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 143: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 144: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 145: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 146: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 147: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 148: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 149: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 150: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 152: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 153: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 154: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 155: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 156: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 158: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 159: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 160: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 161: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 162: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 163: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 164: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 165: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 166: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 167: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 169: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 170: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 171: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 172: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 173: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 174: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 175: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 176: ret
    return output;
}

// source.character.static-map-native-1123.v1 / source program a708061a0d54f448a8c9122cbe24d17d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1123(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[9]=g_SourceCharacterLightConstants[16];
    source[10]=g_SourceCharacterLightConstants[17];
    source[11]=g_SourceCharacterLightConstants[18];
    source[12]=float4(input.lightColor,1.f);
    source[13].x=1.f;
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
    // 23: mul r7.xyzw, v4.xyxy, cb0[5].yyww
    r7.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r7.xyxx, t1.zwxy, s2, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 25: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 26: mul r5.zw, r5.zzzw, cb0[5].zzzz
    r5.zw = ((r5.zzzw)*(source[5].zzzz)).zw;
    // 27: mad r5.xy, cb0[5].xxxx, r5.xyxx, r5.zwzz
    r5.xy = ((source[5].xxxx)*(r5.xyxx)+(r5.zwzz)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
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
    // 39: mul r8.xy, r6.xyxx, cb0[6].xxxx
    r8.xy = ((r6.xyxx)*(source[6].xxxx)).xy;
    // 40: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 44: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 45: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 46: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 47: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 48: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 49: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 50: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mad r0.x, r0.x, l(0.500000), cb0[7].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].xxxx)).x;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 53: mul r0.y, r5.z, r5.z
    r0.y = ((r5.zzzz)*(r5.zzzz)).y;
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
    // 62: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 63: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 64: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 65: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 66: add r1.xyz, -r5.xyzx, r8.xyzx
    r1.xyz = ((-(r5.xyzx))+(r8.xyzx)).xyz;
    // 67: mad r1.xyz, r0.yyyy, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 68: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 69: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 70: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 71: add r0.y, r6.w, l(-0.333300)
    r0.y = ((r6.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).y;
    // 72: lt r0.y, r0.y, l(0.000000)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 73: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 74: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].xxxx)) * 0xffffffffu)).y;
    // 75: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 76: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 77: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 78: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t6.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 79: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 80: else
    } else {
    // 81: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 82: endif
    }
    // 83: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: dp3 r0.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 85: add r8.xyz, -r6.xyzx, r0.yyyy
    r8.xyz = ((-(r6.xyzx))+(r0.yyyy)).xyz;
    // 86: mad r6.xyz, cb0[8].yyyy, r8.xyzx, r6.xyzx
    r6.xyz = ((source[8].yyyy)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 87: mul r8.xyz, cb0[3].xyzx, cb0[8].zzzz
    r8.xyz = ((source[3].xyzx)*(source[8].zzzz)).xyz;
    // 88: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 89: mul r10.xyz, cb0[4].xyzx, cb0[8].wwww
    r10.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 90: mul r11.xyz, r7.xyzx, r10.xyzx
    r11.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 91: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 92: mad r7.xyz, -r10.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r10.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 93: mad r7.xyz, cb0[9].yyyy, r7.xyzx, r11.xyzx
    r7.xyz = ((source[9].yyyy)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 94: mad r6.xyz, -r6.xyzx, r8.xyzx, r7.xyzx
    r6.xyz = ((-(r6.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 95: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 96: mul r6.xyz, r0.xyzx, cb0[9].zzzz
    r6.xyz = ((r0.xyzx)*(source[9].zzzz)).xyz;
    // 97: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t5.yzxw, s6, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 98: mul r1.w, r7.y, cb0[10].x
    r1.w = ((r7.yyyy)*(source[10].xxxx)).w;
    // 99: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 100: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 101: mul r1.w, r1.w, cb0[10].y
    r1.w = ((r1.wwww)*(source[10].yyyy)).w;
    // 102: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 103: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 104: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: mad r0.xyz, cb0[9].wwww, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[9].wwww)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 106: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 107: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 108: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 109: mov_sat r2.w, cb0[10].z
    r2.w = (saturate(source[10].zzzz)).w;
    // 110: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 111: mul r3.w, r7.x, cb0[11].x
    r3.w = ((r7.xxxx)*(source[11].xxxx)).w;
    // 112: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 113: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 114: mul r3.w, r3.w, cb0[11].y
    r3.w = ((r3.wwww)*(source[11].yyyy)).w;
    // 115: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 116: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 117: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 118: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 120: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 121: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 122: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 123: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 124: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 125: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 126: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 128: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 129: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 132: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 133: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 135: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 136: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 137: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 138: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 139: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 140: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 141: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 142: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 143: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 144: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 145: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 146: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 147: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 148: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 149: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 150: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 151: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 152: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 153: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 155: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 156: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 157: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 158: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 159: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 161: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 162: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 163: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 164: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 165: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 166: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 167: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 168: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 169: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 170: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 171: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 172: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 173: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 174: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 175: mul o0.xyz, r0.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 176: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 177: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 178: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 179: ret
    return output;
}

// source.character.static-map-native-1124.v1 / source program 917acbd14573884290c22ac3f83930ee
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1124(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r4.xyzw, v4.xyxy, cb0[4].yyww
    r4.xyzw = ((v4.xyxy)*(source[4].yyww)).xyzw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r4.xyxx, t1.zwxy, s2, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 16: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 17: mul r2.zw, r2.zzzw, cb0[4].zzzz
    r2.zw = ((r2.zzzw)*(source[4].zzzz)).zw;
    // 18: mad r2.xy, cb0[4].xxxx, r2.xyxx, r2.zwzz
    r2.xy = ((source[4].xxxx)*(r2.xyxx)+(r2.zwzz)).xy;
    // 19: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 20: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r4.zwzz, t2.xyzw, s3, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 24: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 25: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 26: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 28: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 29: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 30: mul r5.xy, r3.xyxx, cb0[5].xxxx
    r5.xy = ((r3.xyxx)*(source[5].xxxx)).xy;
    // 31: max r1.w, cb0[5].y, l(0.000000)
    r1.w = (max(source[5].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 33: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 35: add r3.x, -v2.x, l(1.000000)
    r3.x = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul r3.y, r2.z, r2.z
    r3.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 38: mul_sat r3.y, r3.y, r6.w
    r3.y = (saturate((r3.yyyy)*(r6.wwww))).y;
    // 39: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.zwzz, t4.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: mul r3.z, r4.w, r4.w
    r3.z = ((r4.wwww)*(r4.wwww)).z;
    // 42: mul r3.y, r3.z, r3.y
    r3.y = ((r3.zzzz)*(r3.yyyy)).y;
    // 43: mul r3.z, r1.w, r3.y
    r3.z = ((r1.wwww)*(r3.yyyy)).z;
    // 44: mad r3.x, r3.x, r3.z, r3.x
    r3.x = ((r3.xxxx)*(r3.zzzz)+(r3.xxxx)).x;
    // 45: add r1.w, -r1.w, r3.x
    r1.w = ((-(r1.wwww))+(r3.xxxx)).w;
    // 46: mul r3.z, r1.w, r2.w
    r3.z = ((r1.wwww)*(r2.wwww)).z;
    // 47: mad r1.w, -r2.w, r1.w, r3.x
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.xxxx)).w;
    // 48: mad_sat r1.w, r3.y, r1.w, r3.z
    r1.w = (saturate((r3.yyyy)*(r1.wwww)+(r3.zzzz))).w;
    // 49: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 50: add r3.xyz, -r2.xyzx, r5.xyzx
    r3.xyz = ((-(r2.xyzx))+(r5.xyzx)).xyz;
    // 51: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 52: dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 53: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 54: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 55: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).w;
    // 56: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 57: div r3.xy, v8.xyxx, v8.wwww
    r3.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 58: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 59: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t6.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 60: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 61: else
    } else {
    // 62: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 63: endif
    }
    // 64: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 65: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 66: add r7.xyz, -r6.xyzx, r2.wwww
    r7.xyz = ((-(r6.xyzx))+(r2.wwww)).xyz;
    // 67: mad r6.xyz, cb0[5].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[5].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 68: mul r7.xyz, cb0[2].xyzx, cb0[6].xxxx
    r7.xyz = ((source[2].xyzx)*(source[6].xxxx)).xyz;
    // 69: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 70: mul r9.xyz, cb0[3].xyzx, cb0[6].yyyy
    r9.xyz = ((source[3].xyzx)*(source[6].yyyy)).xyz;
    // 71: mul r10.xyz, r4.xyzx, r9.xyzx
    r10.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 72: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: mad r4.xyz, -r9.xyzx, r4.xyzx, r2.wwww
    r4.xyz = ((-(r9.xyzx))*(r4.xyzx)+(r2.wwww)).xyz;
    // 74: mad r4.xyz, cb0[6].wwww, r4.xyzx, r10.xyzx
    r4.xyz = ((source[6].wwww)*(r4.xyzx)+(r10.xyzx)).xyz;
    // 75: mad r4.xyz, -r6.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r6.xyzx))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 76: mad r4.xyz, r1.wwww, r4.xyzx, r8.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 77: mul r6.xyz, r4.xyzx, cb0[7].xxxx
    r6.xyz = ((r4.xyzx)*(source[7].xxxx)).xyz;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t5.yzxw, s6, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 79: mul r1.w, r7.y, cb0[7].z
    r1.w = ((r7.yyyy)*(source[7].zzzz)).w;
    // 80: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 81: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 82: mul r1.w, r1.w, cb0[7].w
    r1.w = ((r1.wwww)*(source[7].wwww)).w;
    // 83: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 84: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 85: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 86: mad r4.xyz, cb0[7].yyyy, r4.xyzx, -r6.xyzx
    r4.xyz = ((source[7].yyyy)*(r4.xyzx)+(-(r6.xyzx))).xyz;
    // 87: mad r4.xyz, r2.wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 88: mul r4.xyz, r5.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)*(r4.xyzx)).xyz;
    // 89: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 90: mov_sat r2.w, cb0[8].x
    r2.w = (saturate(source[8].xxxx)).w;
    // 91: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 92: mul r3.w, r7.x, cb0[8].z
    r3.w = ((r7.xxxx)*(source[8].zzzz)).w;
    // 93: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 94: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 95: mul r3.w, r3.w, cb0[8].w
    r3.w = ((r3.wwww)*(source[8].wwww)).w;
    // 96: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 97: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 98: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 99: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 101: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 102: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 103: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 104: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 105: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 106: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 107: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: dp3_sat r1.x, r2.xyzx, r1.xyzx
    r1.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 109: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 110: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 111: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 112: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 113: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 114: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: mad r0.yzw, -r4.xxyz, r1.wwww, r4.xxyz
    r0.yzw = ((-(r4.xxyz))*(r1.wwww)+(r4.xxyz)).yzw;
    // 116: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 117: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 118: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 119: mad r2.x, r4.w, r1.z, -r4.w
    r2.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 120: mad r2.x, r2.x, r4.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 121: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 122: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 123: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 124: mad r2.x, -r3.w, r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 125: mad r2.y, r5.w, r2.x, r1.y
    r2.y = ((r5.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 126: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 127: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 128: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 129: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 130: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 131: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 132: mad r2.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r2.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 133: mad r2.xyz, r1.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 134: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 136: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 137: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 138: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 139: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 140: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: max r4.xyz, r2.xyzx, r1.wwww
    r4.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 142: add r4.xyz, -r2.xyzx, r4.xyzx
    r4.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 143: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 144: mad r2.xyz, r1.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 145: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 146: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 147: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 148: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 149: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 150: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 151: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 152: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 153: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 154: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 155: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 156: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 157: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 158: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 159: ret
    return output;
}

// source.character.static-map-native-1125.v1 / source program 41b5b5fe26391943a0d015b392100bfb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1125(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[10]=g_SourceCharacterLightConstants[10];
    source[11]=float4(input.lightColor,1.f);
    source[12].x=1.f;
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
    // 23: mul r7.xyzw, v4.xyxy, cb0[5].yyww
    r7.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r7.xyxx, t1.zwxy, s2, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 25: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 26: mul r5.zw, r5.zzzw, cb0[5].zzzz
    r5.zw = ((r5.zzzw)*(source[5].zzzz)).zw;
    // 27: mad r5.xy, cb0[5].xxxx, r5.xyxx, r5.zwzz
    r5.xy = ((source[5].xxxx)*(r5.xyxx)+(r5.zwzz)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
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
    // 39: mul r8.xy, r6.xyxx, cb0[6].xxxx
    r8.xy = ((r6.xyxx)*(source[6].xxxx)).xy;
    // 40: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 44: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 45: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 46: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 47: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 48: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 49: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 50: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mad r0.x, r0.x, l(0.500000), cb0[7].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].xxxx)).x;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 53: mul r0.y, r5.z, r5.z
    r0.y = ((r5.zzzz)*(r5.zzzz)).y;
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
    // 62: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 63: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 64: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 65: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 66: add r1.xyz, -r5.xyzx, r8.xyzx
    r1.xyz = ((-(r5.xyzx))+(r8.xyzx)).xyz;
    // 67: mad r1.xyz, r0.yyyy, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 68: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 69: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 70: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 71: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
    // 72: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 73: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 74: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 75: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t6.xyzw, s0
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
    // 81: dp3 r0.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 82: add r8.xyz, -r6.xyzx, r0.yyyy
    r8.xyz = ((-(r6.xyzx))+(r0.yyyy)).xyz;
    // 83: mad r6.xyz, cb0[7].zzzz, r8.xyzx, r6.xyzx
    r6.xyz = ((source[7].zzzz)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 84: mul r8.xyz, cb0[3].xyzx, cb0[7].wwww
    r8.xyz = ((source[3].xyzx)*(source[7].wwww)).xyz;
    // 85: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 86: mul r10.xyz, cb0[4].xyzx, cb0[8].xxxx
    r10.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // 87: mul r11.xyz, r7.xyzx, r10.xyzx
    r11.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 88: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 89: mad r7.xyz, -r10.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r10.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 90: mad r7.xyz, cb0[8].zzzz, r7.xyzx, r11.xyzx
    r7.xyz = ((source[8].zzzz)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 91: mad r6.xyz, -r6.xyzx, r8.xyzx, r7.xyzx
    r6.xyz = ((-(r6.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 92: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 93: mul r6.xyz, r0.xyzx, cb0[8].wwww
    r6.xyz = ((r0.xyzx)*(source[8].wwww)).xyz;
    // 94: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t5.yzxw, s6, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 95: mul r1.w, r7.y, cb0[9].y
    r1.w = ((r7.yyyy)*(source[9].yyyy)).w;
    // 96: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 97: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 98: mul r1.w, r1.w, cb0[9].z
    r1.w = ((r1.wwww)*(source[9].zzzz)).w;
    // 99: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 100: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 101: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: mad r0.xyz, cb0[9].xxxx, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[9].xxxx)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 103: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 104: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 105: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 106: mov_sat r2.w, cb0[9].w
    r2.w = (saturate(source[9].wwww)).w;
    // 107: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 108: mul r3.w, r7.x, cb0[10].y
    r3.w = ((r7.xxxx)*(source[10].yyyy)).w;
    // 109: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 110: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 111: mul r3.w, r3.w, cb0[10].z
    r3.w = ((r3.wwww)*(source[10].zzzz)).w;
    // 112: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 113: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 114: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 115: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 117: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 118: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 119: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 120: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 121: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 122: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 123: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 125: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 126: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 129: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 130: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 132: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 133: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 134: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 135: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 136: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 137: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 138: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 139: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 140: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 141: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 142: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 143: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 144: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 145: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 146: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 147: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 148: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 149: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 150: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 152: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 153: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 154: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 155: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 156: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 158: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 159: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 160: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 161: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 162: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 163: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 164: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 165: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 166: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 167: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 169: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 170: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 171: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 172: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 173: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 174: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 175: ret
    return output;
}

// source.character.static-map-native-1126.v1 / source program ec57cf38c63c4e4cb731d531a98843e6
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1126(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[6];
    source[4]=g_SourceCharacterLightConstants[7];
    source[5]=g_SourceCharacterLightConstants[8];
    source[6]=g_SourceCharacterLightConstants[10];
    source[7]=g_SourceCharacterLightConstants[11];
    source[8]=g_SourceCharacterLightConstants[12];
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
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
    // 7: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.xyxx, t0.zwxy, s1, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 9: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 10: dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: mul r2.zw, r2.zzzw, cb0[5].wwww
    r2.zw = ((r2.zzzw)*(source[5].wwww)).zw;
    // 16: mul r3.xy, r2.zwzz, v2.wwww
    r3.xy = ((r2.zwzz)*(v2.wwww)).xy;
    // 17: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 18: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 19: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 20: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 21: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 22: mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 23: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).w;
    // 24: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 25: div r2.zw, v8.xxxy, v8.wwww
    r2.zw = ((v8.xxxy)/(v8.wwww)).zw;
    // 26: mad r2.zw, r2.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r2.zw = ((r2.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 27: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r2.zwzz, t3.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 28: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 29: else
    } else {
    // 30: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 31: endif
    }
    // 32: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 34: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 36: mad r7.xyz, cb0[6].xxxx, r7.xyzx, r6.xyzx
    r7.xyz = ((source[6].xxxx)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 37: mul r8.xyz, cb0[3].xyzx, cb0[6].yyyy
    r8.xyz = ((source[3].xyzx)*(source[6].yyyy)).xyz;
    // 38: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 39: mul r8.xyz, cb0[4].xyzx, cb0[6].zzzz
    r8.xyz = ((source[4].xyzx)*(source[6].zzzz)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t2.yzxw, s3, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 41: mul r1.w, r2.y, cb0[6].w
    r1.w = ((r2.yyyy)*(source[6].wwww)).w;
    // 42: lt r2.y, |r1.w|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 43: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 44: mul r1.w, r1.w, cb0[7].x
    r1.w = ((r1.wwww)*(source[7].xxxx)).w;
    // 45: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 46: movc r1.w, r2.y, l(0), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: min r2.y, r1.w, l(1.000000)
    r2.y = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 48: mad r6.xyz, r8.xyzx, r6.xyzx, -r7.xyzx
    r6.xyz = ((r8.xyzx)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 49: mad r6.xyz, r2.yyyy, r6.xyzx, r7.xyzx
    r6.xyz = ((r2.yyyy)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 50: mul r7.xyz, r6.xyzx, cb0[7].yyyy
    r7.xyz = ((r6.xyzx)*(source[7].yyyy)).xyz;
    // 51: mad r6.xyz, cb0[7].zzzz, r6.xyzx, -r7.xyzx
    r6.xyz = ((source[7].zzzz)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 52: mad r2.yzw, r2.yyyy, r6.xxyz, r7.xxyz
    r2.yzw = ((r2.yyyy)*(r6.xxyz)+(r7.xxyz)).yzw;
    // 53: mul r2.yzw, r5.xxyz, r2.yyzw
    r2.yzw = ((r5.xxyz)*(r2.yyzw)).yzw;
    // 54: mad_sat r2.yzw, r2.yyzw, cb2[3].wwww, cb2[3].xxyz
    r2.yzw = (saturate((r2.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz))).yzw;
    // 55: mov_sat r3.w, cb0[7].w
    r3.w = (saturate(source[7].wwww)).w;
    // 56: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 57: mul r2.x, r2.x, cb0[8].y
    r2.x = ((r2.xxxx)*(source[8].yyyy)).x;
    // 58: lt r4.w, |r2.x|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 59: log r2.x, |r2.x|
    r2.x = (log2(abs(r2.xxxx))).x;
    // 60: mul r2.x, r2.x, cb0[8].z
    r2.x = ((r2.xxxx)*(source[8].zzzz)).x;
    // 61: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 62: movc r2.x, r4.w, l(0), r2.x
    r2.x = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 63: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 64: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 66: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 67: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 68: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 69: dp3_sat r4.w, r3.xyzx, r5.xyzx
    r4.w = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 70: dp3 r5.w, r3.xyzx, r0.xyzx
    r5.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 71: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 72: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: dp3_sat r1.x, r3.xyzx, r1.xyzx
    r1.x = (saturate(dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 74: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 75: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 76: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 77: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 78: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 79: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 80: mad r0.yzw, -r2.yyzw, r1.wwww, r2.yyzw
    r0.yzw = ((-(r2.yyzw))*(r1.wwww)+(r2.yyzw)).yzw;
    // 81: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 82: mul r1.y, r2.x, r2.x
    r1.y = ((r2.xxxx)*(r2.xxxx)).y;
    // 83: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 84: mad r3.x, r4.w, r1.z, -r4.w
    r3.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 85: mad r3.x, r3.x, r4.w, l(1.000000)
    r3.x = ((r3.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 86: mul r3.x, r3.x, r3.x
    r3.x = ((r3.xxxx)*(r3.xxxx)).x;
    // 87: mul r3.x, r3.x, l(3.141593)
    r3.x = ((r3.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 88: div r1.z, r1.z, r3.x
    r1.z = ((r1.zzzz)/(r3.xxxx)).z;
    // 89: mad r3.x, -r2.x, r2.x, l(1.000000)
    r3.x = ((-(r2.xxxx))*(r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 90: mad r3.y, r5.w, r3.x, r1.y
    r3.y = ((r5.wwww)*(r3.xxxx)+(r1.yyyy)).y;
    // 91: mad r1.y, r1.x, r3.x, r1.y
    r1.y = ((r1.xxxx)*(r3.xxxx)+(r1.yyyy)).y;
    // 92: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 93: mad r1.y, r1.x, r3.y, r1.y
    r1.y = ((r1.xxxx)*(r3.yyyy)+(r1.yyyy)).y;
    // 94: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 95: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 96: mul r1.z, r3.w, l(0.080000)
    r1.z = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 97: mad r2.yzw, -r3.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r2.yyzw
    r2.yzw = ((-(r3.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r2.yyzw)).yzw;
    // 98: mad r2.yzw, r1.wwww, r2.yyzw, r1.zzzz
    r2.yzw = ((r1.wwww)*(r2.yyzw)+(r1.zzzz)).yzw;
    // 99: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 100: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 101: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 102: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 103: mul_sat r1.z, r2.z, l(50.000000)
    r1.z = (saturate((r2.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 104: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 105: add r1.w, -r2.x, l(1.000000)
    r1.w = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: max r3.xyz, r2.yzwy, r1.wwww
    r3.xyz = (max(r2.yzwy,r1.wwww)).xyz;
    // 107: add r3.xyz, -r2.yzwy, r3.xyzx
    r3.xyz = ((-(r2.yzwy))+(r3.xyzx)).xyz;
    // 108: mad r2.xyz, -r0.xxxx, r2.yzwy, r2.yzwy
    r2.xyz = ((-(r0.xxxx))*(r2.yzwy)+(r2.yzwy)).xyz;
    // 109: mad r2.xyz, r1.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 110: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 111: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 112: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 113: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 114: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 115: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 116: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 117: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 118: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 119: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 120: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 121: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 122: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 123: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 124: ret
    return output;
}

// source.character.static-map-native-1127.v1 / source program b6281a77c23a874693761588e5c386b5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1127(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[4];
    source[4]=g_SourceCharacterLightConstants[5];
    source[5]=g_SourceCharacterLightConstants[6];
    source[6]=g_SourceCharacterLightConstants[7];
    source[7]=g_SourceCharacterLightConstants[8];
    source[8]=g_SourceCharacterLightConstants[9];
    source[9]=g_SourceCharacterLightConstants[10];
    source[10]=g_SourceCharacterLightConstants[11];
    source[11]=float4(input.lightColor,1.f);
    source[12].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
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
    // 7: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t0.xywz, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 9: mad r2.zw, r3.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r3.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 10: dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: mul r2.zw, r2.zzzw, cb0[6].wwww
    r2.zw = ((r2.zzzw)*(source[6].wwww)).zw;
    // 16: mul r4.xy, r2.zwzz, v2.wwww
    r4.xy = ((r2.zwzz)*(v2.wwww)).xy;
    // 17: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 18: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 19: div r3.xyw, r4.xyxz, r1.wwww
    r3.xyw = ((r4.xyxz)/(r1.wwww)).xyw;
    // 20: dp3 r1.w, r3.xywx, r3.xywx
    r1.w = (dot((r3.xywx).xyz,(r3.xywx).xyz).xxxx).w;
    // 21: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 22: mul r3.xyw, r1.wwww, r3.xyxw
    r3.xyw = ((r1.wwww)*(r3.xyxw)).xyw;
    // 23: dp3 r1.w, r3.xywx, r0.xyzx
    r1.w = (dot((r3.xywx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 24: mul r2.zw, r1.wwww, r3.xxxy
    r2.zw = ((r1.wwww)*(r3.xxxy)).zw;
    // 25: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), -r0.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(-(r0.xxxy))).zw;
    // 26: ne r4.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r4.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).x;
    // 27: if_nz r4.x
    if ((asuint(r4.xxxx)).x != 0u) {
    // 28: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 29: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 30: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t4.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 31: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 32: else
    } else {
    // 33: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 34: endif
    }
    // 35: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 36: add r2.zw, r2.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r2.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 37: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), -v4.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(-(v4.xxxy))).zw;
    // 38: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 0.750000, 0.750000), v4.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,0.750000,0.750000))+(v4.xxxy)).zw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.zwzz, t1.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mul r6.xyz, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r6.xyzx)*(source[3].xyzx)).xyz;
    // 41: mad r6.xyz, cb0[7].zzzz, r6.xyzx, r6.xyzx
    r6.xyz = ((source[7].zzzz)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 42: add r6.xyz, r6.xyzx, -cb0[7].zzzz
    r6.xyz = ((r6.xyzx)+(-(source[7].zzzz))).xyz;
    // 43: mov_sat r7.xyz, r6.xyzx
    r7.xyz = (saturate(r6.xyzx)).xyz;
    // 44: mul r2.z, r3.z, cb0[7].w
    r2.z = ((r3.zzzz)*(source[7].wwww)).z;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 46: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 47: add r9.xyz, -r8.xyzx, r2.wwww
    r9.xyz = ((-(r8.xyzx))+(r2.wwww)).xyz;
    // 48: mad r9.xyz, cb0[8].yyyy, r9.xyzx, r8.xyzx
    r9.xyz = ((source[8].yyyy)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 49: mul r10.xyz, cb0[4].xyzx, cb0[8].zzzz
    r10.xyz = ((source[4].xyzx)*(source[8].zzzz)).xyz;
    // 50: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 51: mul r10.xyz, cb0[5].xyzx, cb0[8].wwww
    r10.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t3.yzxw, s4, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 53: mul r2.y, r2.y, cb0[9].x
    r2.y = ((r2.yyyy)*(source[9].xxxx)).y;
    // 54: lt r2.w, |r2.y|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: log r2.y, |r2.y|
    r2.y = (log2(abs(r2.yyyy))).y;
    // 56: mul r2.y, r2.y, cb0[9].y
    r2.y = ((r2.yyyy)*(source[9].yyyy)).y;
    // 57: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 58: movc r2.y, r2.w, l(0), r2.y
    r2.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).y;
    // 59: min r2.w, r2.y, l(1.000000)
    r2.w = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: mad r8.xyz, r10.xyzx, r8.xyzx, -r9.xyzx
    r8.xyz = ((r10.xyzx)*(r8.xyzx)+(-(r9.xyzx))).xyz;
    // 61: mad r8.xyz, r2.wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 62: mad r7.xyz, r2.zzzz, r7.xyzx, r8.xyzx
    r7.xyz = ((r2.zzzz)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 63: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 64: mad r6.xyz, -r2.zzzz, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r2.zzzz))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 65: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 66: max r6.xyz, r6.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r6.xyz = (max(r6.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 67: min r6.xyz, r6.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 68: mul r7.xyz, r6.xyzx, cb0[9].zzzz
    r7.xyz = ((r6.xyzx)*(source[9].zzzz)).xyz;
    // 69: mad r6.xyz, cb0[9].wwww, r6.xyzx, -r7.xyzx
    r6.xyz = ((source[9].wwww)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 70: mad r6.xyz, r2.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 71: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 72: mad_sat r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = (saturate((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 73: mov_sat r2.z, cb0[10].x
    r2.z = (saturate(source[10].xxxx)).z;
    // 74: mul_sat r2.y, r2.y, cb2[3].w
    r2.y = (saturate((r2.yyyy)*(passValues[3].wwww))).y;
    // 75: mul r2.x, r2.x, cb0[10].z
    r2.x = ((r2.xxxx)*(source[10].zzzz)).x;
    // 76: lt r2.w, |r2.x|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 77: log r2.x, |r2.x|
    r2.x = (log2(abs(r2.xxxx))).x;
    // 78: mul r2.x, r2.x, cb0[10].w
    r2.x = ((r2.xxxx)*(source[10].wwww)).x;
    // 79: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 80: movc r2.x, r2.w, l(0), r2.x
    r2.x = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 81: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 82: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: mad r6.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r6.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 84: dp3 r2.w, r6.xyzx, r6.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 85: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 86: mul r6.xyz, r2.wwww, r6.xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 87: dp3_sat r2.w, r3.xywx, r6.xyzx
    r2.w = (saturate(dot((r3.xywx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 88: add r1.w, |r1.w|, l(0.000010)
    r1.w = ((abs(r1.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 89: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: dp3_sat r1.x, r3.xywx, r1.xyzx
    r1.x = (saturate(dot((r3.xywx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 91: dp3_sat r0.x, r0.xyzx, r6.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx)).x;
    // 92: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 93: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 94: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 95: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 96: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 97: mad r0.yzw, -r5.xxyz, r2.yyyy, r5.xxyz
    r0.yzw = ((-(r5.xxyz))*(r2.yyyy)+(r5.xxyz)).yzw;
    // 98: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 99: mul r1.y, r2.x, r2.x
    r1.y = ((r2.xxxx)*(r2.xxxx)).y;
    // 100: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 101: mad r3.x, r2.w, r1.z, -r2.w
    r3.x = ((r2.wwww)*(r1.zzzz)+(-(r2.wwww))).x;
    // 102: mad r2.w, r3.x, r2.w, l(1.000000)
    r2.w = ((r3.xxxx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 104: mul r2.w, r2.w, l(3.141593)
    r2.w = ((r2.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 105: div r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)/(r2.wwww)).z;
    // 106: mad r2.w, -r2.x, r2.x, l(1.000000)
    r2.w = ((-(r2.xxxx))*(r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: mad r3.x, r1.w, r2.w, r1.y
    r3.x = ((r1.wwww)*(r2.wwww)+(r1.yyyy)).x;
    // 108: mad r1.y, r1.x, r2.w, r1.y
    r1.y = ((r1.xxxx)*(r2.wwww)+(r1.yyyy)).y;
    // 109: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 110: mad r1.y, r1.x, r3.x, r1.y
    r1.y = ((r1.xxxx)*(r3.xxxx)+(r1.yyyy)).y;
    // 111: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 112: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 113: mul r1.z, r2.z, l(0.080000)
    r1.z = ((r2.zzzz)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 114: mad r3.xyz, -r2.zzzz, l(0.080000, 0.080000, 0.080000, 0.000000), r5.xyzx
    r3.xyz = ((-(r2.zzzz))*(float4(0.080000,0.080000,0.080000,0.000000))+(r5.xyzx)).xyz;
    // 115: mad r2.yzw, r2.yyyy, r3.xxyz, r1.zzzz
    r2.yzw = ((r2.yyyy)*(r3.xxyz)+(r1.zzzz)).yzw;
    // 116: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 117: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 118: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 119: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 120: mul_sat r1.z, r2.z, l(50.000000)
    r1.z = (saturate((r2.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 121: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 122: add r1.w, -r2.x, l(1.000000)
    r1.w = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: max r3.xyz, r2.yzwy, r1.wwww
    r3.xyz = (max(r2.yzwy,r1.wwww)).xyz;
    // 124: add r3.xyz, -r2.yzwy, r3.xyzx
    r3.xyz = ((-(r2.yzwy))+(r3.xyzx)).xyz;
    // 125: mad r2.xyz, -r0.xxxx, r2.yzwy, r2.yzwy
    r2.xyz = ((-(r0.xxxx))*(r2.yzwy)+(r2.yzwy)).xyz;
    // 126: mad r2.xyz, r1.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 127: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 128: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 129: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 130: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 131: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 132: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 133: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 134: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 135: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 136: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 137: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 138: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 139: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 140: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 141: ret
    return output;
}

// source.character.static-map-native-1128.v1 / source program 67759d2cec87044a898477b35c81817a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1128(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[10]=g_SourceCharacterLightConstants[10];
    source[11]=float4(input.lightColor,1.f);
    source[12].x=1.f;
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
    // 68: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
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
    // 78: mul r7.xyz, cb0[3].xyzx, cb0[7].wwww
    r7.xyz = ((source[3].xyzx)*(source[7].wwww)).xyz;
    // 79: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 80: mul r9.xyz, cb0[4].xyzx, cb0[8].xxxx
    r9.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).yz;
    // 82: mul r0.z, r0.z, cb0[8].y
    r0.z = ((r0.zzzz)*(source[8].yyyy)).z;
    // 83: lt r1.w, |r0.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 84: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 85: mul r0.z, r0.z, cb0[8].z
    r0.z = ((r0.zzzz)*(source[8].zzzz)).z;
    // 86: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 87: movc r0.z, r1.w, l(0), r0.z
    r0.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 88: min r1.w, r0.z, l(1.000000)
    r1.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 89: mad r8.xyz, r9.xyzx, r8.xyzx, -r7.xyzx
    r8.xyz = ((r9.xyzx)*(r8.xyzx)+(-(r7.xyzx))).xyz;
    // 90: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 91: mul r8.xyz, cb0[5].xyzx, cb0[8].wwww
    r8.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 92: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 93: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 94: mad r6.xyz, -r8.xyzx, r6.xyzx, r2.wwww
    r6.xyz = ((-(r8.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 95: mad r6.xyz, cb0[9].yyyy, r6.xyzx, r9.xyzx
    r6.xyz = ((source[9].yyyy)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 96: add r6.xyz, -r7.xyzx, r6.xyzx
    r6.xyz = ((-(r7.xyzx))+(r6.xyzx)).xyz;
    // 97: mad r6.xyz, r0.xxxx, r6.xyzx, r7.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 98: mul r7.xyz, r6.xyzx, cb0[9].zzzz
    r7.xyz = ((r6.xyzx)*(source[9].zzzz)).xyz;
    // 99: mad r6.xyz, cb0[9].wwww, r6.xyzx, -r7.xyzx
    r6.xyz = ((source[9].wwww)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 100: mad r6.xyz, r1.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 101: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 102: mad_sat r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = (saturate((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 103: mov_sat r0.x, cb0[10].x
    r0.x = (saturate(source[10].xxxx)).x;
    // 104: mul_sat r0.z, r0.z, cb2[3].w
    r0.z = (saturate((r0.zzzz)*(passValues[3].wwww))).z;
    // 105: mul r0.y, r0.y, cb0[10].z
    r0.y = ((r0.yyyy)*(source[10].zzzz)).y;
    // 106: lt r1.w, |r0.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 107: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 108: mul r0.y, r0.y, cb0[10].w
    r0.y = ((r0.yyyy)*(source[10].wwww)).y;
    // 109: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 110: movc r0.y, r1.w, l(0), r0.y
    r0.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 111: max r0.y, r0.y, cb0[0].x
    r0.y = (max(r0.yyyy,source[0].xxxx)).y;
    // 112: mad r6.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r6.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 113: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 114: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 115: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 116: dp3_sat r1.w, r1.xyzx, r6.xyzx
    r1.w = (saturate(dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 117: dp3 r2.w, r1.xyzx, r3.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 118: add r2.w, |r2.w|, l(0.000010)
    r2.w = ((abs(r2.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 119: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 121: dp3_sat r1.y, r3.xyzx, r6.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r6.xyzx).xyz).xxxx)).y;
    // 122: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: min r0.yw, r0.yyyw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = (min(r0.yyyw,float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 124: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 125: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 126: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: mad r3.xyz, -r5.xyzx, r0.zzzz, r5.xyzx
    r3.xyz = ((-(r5.xyzx))*(r0.zzzz)+(r5.xyzx)).xyz;
    // 128: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 129: mul r1.y, r0.y, r0.y
    r1.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 130: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 131: mad r3.w, r1.w, r1.z, -r1.w
    r3.w = ((r1.wwww)*(r1.zzzz)+(-(r1.wwww))).w;
    // 132: mad r1.w, r3.w, r1.w, l(1.000000)
    r1.w = ((r3.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 134: mul r1.w, r1.w, l(3.141593)
    r1.w = ((r1.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 135: div r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)/(r1.wwww)).z;
    // 136: mad r1.w, -r0.y, r0.y, l(1.000000)
    r1.w = ((-(r0.yyyy))*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: mad r3.w, r2.w, r1.w, r1.y
    r3.w = ((r2.wwww)*(r1.wwww)+(r1.yyyy)).w;
    // 138: mad r1.y, r1.x, r1.w, r1.y
    r1.y = ((r1.xxxx)*(r1.wwww)+(r1.yyyy)).y;
    // 139: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 140: mad r1.y, r1.x, r3.w, r1.y
    r1.y = ((r1.xxxx)*(r3.wwww)+(r1.yyyy)).y;
    // 141: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 142: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 143: mul r1.z, r0.x, l(0.080000)
    r1.z = ((r0.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 144: mad r4.xyz, -r0.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r5.xyzx
    r4.xyz = ((-(r0.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r5.xyzx)).xyz;
    // 145: mad r4.xyz, r0.zzzz, r4.xyzx, r1.zzzz
    r4.xyz = ((r0.zzzz)*(r4.xyzx)+(r1.zzzz)).xyz;
    // 146: add r0.xy, -r0.wyww, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.wyww))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 147: mul r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 148: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 149: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 150: mul_sat r0.z, r4.y, l(50.000000)
    r0.z = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 151: mul r0.z, r0.x, r0.z
    r0.z = ((r0.xxxx)*(r0.zzzz)).z;
    // 152: max r5.xyz, r4.xyzx, r0.yyyy
    r5.xyz = (max(r4.xyzx,r0.yyyy)).xyz;
    // 153: add r5.xyz, -r4.xyzx, r5.xyzx
    r5.xyz = ((-(r4.xyzx))+(r5.xyzx)).xyz;
    // 154: mad r0.xyw, -r0.xxxx, r4.xyxz, r4.xyxz
    r0.xyw = ((-(r0.xxxx))*(r4.xyxz)+(r4.xyxz)).xyw;
    // 155: mad r0.xyz, r0.zzzz, r5.xyzx, r0.xywx
    r0.xyz = ((r0.zzzz)*(r5.xyzx)+(r0.xywx)).xyz;
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
    // 167: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 168: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 169: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 170: ret
    return output;
}

// source.character.static-map-native-1129.v1 / source program 697161908188c44886f31f3287467cf9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1129(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[2];
    source[1]=g_SourceCharacterLightConstants[3];
    source[2]=g_SourceCharacterLightConstants[4];
    source[3]=g_SourceCharacterLightConstants[5];
    source[4]=g_SourceCharacterLightConstants[6];
    source[5]=g_SourceCharacterLightConstants[7];
    source[6]=g_SourceCharacterLightConstants[8];
    source[7]=float4(input.lightColor,1.f);
    source[8].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[8].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[8].xxxx)) * 0xffffffffu)).x;
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
    // 12: mul r1.xyz, r0.wwww, v7.xyzx
    r1.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r2.xyz, r0.wwww, v5.xyzx
    r2.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 17: add r1.w, r3.w, l(-0.333300)
    r1.w = ((r3.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 18: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 19: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 20: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t2.xzwy, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).xyz;
    // 22: mul r2.w, r4.z, cb0[4].w
    r2.w = ((r4.zzzz)*(source[4].wwww)).w;
    // 23: add r5.xyz, -r3.xyzx, r1.wwww
    r5.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 24: mad r3.xyz, r2.wwww, r5.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 25: mad r5.xyz, cb0[5].xxxx, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((source[5].xxxx)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 26: mad r5.xyz, r4.zzzz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((r4.zzzz)*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 27: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r4.zw, v4.xyxx, t0.zwxy, s1, l(0.000000)
    r4.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 29: mad r4.zw, r4.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r4.zw = ((r4.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 30: dp2 r1.w, r4.zwzz, r4.zwzz
    r1.w = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).w;
    // 31: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 34: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 35: mul r5.xy, r4.zwzz, cb0[4].xxxx
    r5.xy = ((r4.zwzz)*(source[4].xxxx)).xy;
    // 36: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 37: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 38: div r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 39: dp3 r1.w, r5.xyzx, r2.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 40: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r2.x, r1.w, l(1.000000)
    r2.x = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 42: mul r2.xyz, r0.xyzx, r2.xxxx
    r2.xyz = ((r0.xyzx)*(r2.xxxx)).xyz;
    // 43: mul r6.xyz, cb0[2].xyzx, cb0[5].wwww
    r6.xyz = ((source[2].xyzx)*(source[5].wwww)).xyz;
    // 44: mul r4.yzw, r4.yyyy, r6.xxyz
    r4.yzw = ((r4.yyyy)*(r6.xxyz)).yzw;
    // 45: mad r6.xyz, r4.yzwy, r0.xyzx, -r2.xyzx
    r6.xyz = ((r4.yzwy)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 46: mad r2.xyz, r4.yzwy, r6.xyzx, r2.xyzx
    r2.xyz = ((r4.yzwy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 47: mul r4.xyz, r4.xxxx, cb0[1].xyzx
    r4.xyz = ((r4.xxxx)*(source[1].xyzx)).xyz;
    // 48: mul r4.xyz, r4.xyzx, cb0[5].yyyy
    r4.xyz = ((r4.xyzx)*(source[5].yyyy)).xyz;
    // 49: mad r6.xyz, v5.xyzx, r0.wwww, r1.xyzx
    r6.xyz = ((v5.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 50: dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 51: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 52: div r6.xyz, r6.xyzx, r0.wwww
    r6.xyz = ((r6.xyzx)/(r0.wwww)).xyz;
    // 53: dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 54: lt r2.w, |r0.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 56: mul r0.w, r0.w, cb0[5].z
    r0.w = ((r0.wwww)*(source[5].zzzz)).w;
    // 57: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 58: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: movc r0.w, r2.w, l(0), r0.w
    r0.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 60: mul r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 61: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 62: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 63: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 64: mad r0.xyz, r3.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 65: mul r2.xyz, cb0[3].xyzx, cb0[6].xxxx
    r2.xyz = ((source[3].xyzx)*(source[6].xxxx)).xyz;
    // 66: add r0.w, -|r1.z|, l(1.000000)
    r0.w = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 67: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 68: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 69: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 70: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 71: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 72: mul r0.w, r0.w, cb0[6].y
    r0.w = ((r0.wwww)*(source[6].yyyy)).w;
    // 73: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 74: mul r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // 75: movc r1.xyz, r1.xxxx, l(0,0,0,0), r2.xyzx
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 76: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 77: mul r1.xyz, r1.wwww, cb2[3].xyzx
    r1.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 78: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 79: mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // 80: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 81: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 82: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 83: ret
    return output;
}

// source.character.static-map-native-1130.v1 / source program 42b9dce91ebed14abf7755d56fe692f3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1130(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[2];
    source[1]=g_SourceCharacterLightConstants[3];
    source[2]=g_SourceCharacterLightConstants[4];
    source[3]=g_SourceCharacterLightConstants[5];
    source[4]=g_SourceCharacterLightConstants[6];
    source[5]=g_SourceCharacterLightConstants[7];
    source[6]=g_SourceCharacterLightConstants[8];
    source[7]=float4(input.lightColor,1.f);
    source[8].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[8].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[8].xxxx)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: div r0.xy, v8.xyxx, v8.wwww
    r0.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 4: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 5: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s0
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
    // 12: mul r1.xyz, r0.wwww, v7.xyzx
    r1.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r2.xyz, r0.wwww, v5.xyzx
    r2.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 17: add r1.w, r3.w, l(-0.333300)
    r1.w = ((r3.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 18: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 19: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 20: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xzwy, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).xyz;
    // 22: mul r2.w, r4.z, cb0[5].y
    r2.w = ((r4.zzzz)*(source[5].yyyy)).w;
    // 23: add r5.xyz, -r3.xyzx, r1.wwww
    r5.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 24: mad r3.xyz, r2.wwww, r5.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 25: mad r5.xyz, cb0[5].zzzz, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((source[5].zzzz)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 26: mad r5.xyz, r4.zzzz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((r4.zzzz)*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 27: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r4.zw, v4.xyxx, t0.zwxy, s1, l(0.000000)
    r4.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 29: mad r4.zw, r4.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r4.zw = ((r4.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 30: dp2 r1.w, r4.zwzz, r4.zwzz
    r1.w = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).w;
    // 31: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 34: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 35: mul r6.xy, v4.xyxx, cb0[4].yyyy
    r6.xy = ((v4.xyxx)*(source[4].yyyy)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r6.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 37: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 38: mul r6.xy, r6.xyxx, cb0[4].zzzz
    r6.xy = ((r6.xyxx)*(source[4].zzzz)).xy;
    // 39: mad r5.xy, cb0[4].xxxx, r4.zwzz, r6.xyxx
    r5.xy = ((source[4].xxxx)*(r4.zwzz)+(r6.xyxx)).xy;
    // 40: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 41: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 42: div r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 43: dp3 r1.w, r5.xyzx, r2.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 44: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: min r2.x, r1.w, l(1.000000)
    r2.x = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: mul r2.xyz, r0.xyzx, r2.xxxx
    r2.xyz = ((r0.xyzx)*(r2.xxxx)).xyz;
    // 47: mul r6.xyz, cb0[2].xyzx, cb0[6].yyyy
    r6.xyz = ((source[2].xyzx)*(source[6].yyyy)).xyz;
    // 48: mul r4.yzw, r4.yyyy, r6.xxyz
    r4.yzw = ((r4.yyyy)*(r6.xxyz)).yzw;
    // 49: mad r6.xyz, r4.yzwy, r0.xyzx, -r2.xyzx
    r6.xyz = ((r4.yzwy)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 50: mad r2.xyz, r4.yzwy, r6.xyzx, r2.xyzx
    r2.xyz = ((r4.yzwy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 51: mul r4.xyz, r4.xxxx, cb0[1].xyzx
    r4.xyz = ((r4.xxxx)*(source[1].xyzx)).xyz;
    // 52: mul r4.xyz, r4.xyzx, cb0[5].wwww
    r4.xyz = ((r4.xyzx)*(source[5].wwww)).xyz;
    // 53: mad r6.xyz, v5.xyzx, r0.wwww, r1.xyzx
    r6.xyz = ((v5.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 54: dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 55: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 56: div r6.xyz, r6.xyzx, r0.wwww
    r6.xyz = ((r6.xyzx)/(r0.wwww)).xyz;
    // 57: dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 58: lt r2.w, |r0.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 59: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 60: mul r0.w, r0.w, cb0[6].x
    r0.w = ((r0.wwww)*(source[6].xxxx)).w;
    // 61: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 62: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 63: movc r0.w, r2.w, l(0), r0.w
    r0.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 64: mul r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 65: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 66: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 67: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 68: mad r0.xyz, r3.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 69: mul r2.xyz, cb0[3].xyzx, cb0[6].zzzz
    r2.xyz = ((source[3].xyzx)*(source[6].zzzz)).xyz;
    // 70: add r0.w, -|r1.z|, l(1.000000)
    r0.w = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 72: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 73: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 74: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 75: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 76: mul r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)*(source[6].wwww)).w;
    // 77: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 78: mul r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // 79: movc r1.xyz, r1.xxxx, l(0,0,0,0), r2.xyzx
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 80: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 81: mul r1.xyz, r1.wwww, cb2[3].xyzx
    r1.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 82: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 83: mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // 84: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 85: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 86: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 87: ret
    return output;
}

// source.character.static-map-native-1131.v1 / source program fc8ec87914e3a3488c032c1c4da308f6
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1131(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[5];
    source[4]=g_SourceCharacterLightConstants[6];
    source[5]=g_SourceCharacterLightConstants[7];
    source[6]=g_SourceCharacterLightConstants[8];
    source[7]=g_SourceCharacterLightConstants[9];
    source[8]=g_SourceCharacterLightConstants[10];
    source[9]=g_SourceCharacterLightConstants[11];
    source[10]=g_SourceCharacterLightConstants[12];
    source[11]=g_SourceCharacterLightConstants[13];
    source[12]=float4(input.lightColor,1.f);
    source[13].x=1.f;
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
    // 68: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].xxxx)) * 0xffffffffu)).y;
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
    // 78: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 79: add r7.xyz, -r8.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))+(r0.yyyy)).xyz;
    // 80: mad r7.xyz, cb0[8].yyyy, r7.xyzx, r8.xyzx
    r7.xyz = ((source[8].yyyy)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 81: mul r9.xyz, cb0[3].xyzx, cb0[8].zzzz
    r9.xyz = ((source[3].xyzx)*(source[8].zzzz)).xyz;
    // 82: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 83: mul r9.xyz, cb0[4].xyzx, cb0[8].wwww
    r9.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 84: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).yz;
    // 85: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 86: lt r1.w, |r0.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 87: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 88: mul r0.z, r0.z, cb0[9].y
    r0.z = ((r0.zzzz)*(source[9].yyyy)).z;
    // 89: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 90: movc r0.z, r1.w, l(0), r0.z
    r0.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 91: min r1.w, r0.z, l(1.000000)
    r1.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: mad r8.xyz, r9.xyzx, r8.xyzx, -r7.xyzx
    r8.xyz = ((r9.xyzx)*(r8.xyzx)+(-(r7.xyzx))).xyz;
    // 93: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 94: mul r8.xyz, cb0[5].xyzx, cb0[9].zzzz
    r8.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 95: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 96: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 97: mad r6.xyz, -r8.xyzx, r6.xyzx, r2.wwww
    r6.xyz = ((-(r8.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 98: mad r6.xyz, cb0[10].xxxx, r6.xyzx, r9.xyzx
    r6.xyz = ((source[10].xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 99: add r6.xyz, -r7.xyzx, r6.xyzx
    r6.xyz = ((-(r7.xyzx))+(r6.xyzx)).xyz;
    // 100: mad r6.xyz, r0.xxxx, r6.xyzx, r7.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 101: mul r7.xyz, r6.xyzx, cb0[10].yyyy
    r7.xyz = ((r6.xyzx)*(source[10].yyyy)).xyz;
    // 102: mad r6.xyz, cb0[10].zzzz, r6.xyzx, -r7.xyzx
    r6.xyz = ((source[10].zzzz)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 103: mad r6.xyz, r1.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 104: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 105: mad_sat r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = (saturate((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 106: mov_sat r0.x, cb0[10].w
    r0.x = (saturate(source[10].wwww)).x;
    // 107: mul_sat r0.z, r0.z, cb2[3].w
    r0.z = (saturate((r0.zzzz)*(passValues[3].wwww))).z;
    // 108: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 109: lt r1.w, |r0.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 110: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 111: mul r0.y, r0.y, cb0[11].z
    r0.y = ((r0.yyyy)*(source[11].zzzz)).y;
    // 112: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 113: movc r0.y, r1.w, l(0), r0.y
    r0.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 114: max r0.y, r0.y, cb0[0].x
    r0.y = (max(r0.yyyy,source[0].xxxx)).y;
    // 115: mad r6.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r6.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 116: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 117: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 118: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 119: dp3_sat r1.w, r1.xyzx, r6.xyzx
    r1.w = (saturate(dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 120: dp3 r2.w, r1.xyzx, r3.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 121: add r2.w, |r2.w|, l(0.000010)
    r2.w = ((abs(r2.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 122: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 124: dp3_sat r1.y, r3.xyzx, r6.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r6.xyzx).xyz).xxxx)).y;
    // 125: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: min r0.yw, r0.yyyw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = (min(r0.yyyw,float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 127: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 128: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 129: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mad r3.xyz, -r5.xyzx, r0.zzzz, r5.xyzx
    r3.xyz = ((-(r5.xyzx))*(r0.zzzz)+(r5.xyzx)).xyz;
    // 131: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 132: mul r1.y, r0.y, r0.y
    r1.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 133: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 134: mad r3.w, r1.w, r1.z, -r1.w
    r3.w = ((r1.wwww)*(r1.zzzz)+(-(r1.wwww))).w;
    // 135: mad r1.w, r3.w, r1.w, l(1.000000)
    r1.w = ((r3.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 137: mul r1.w, r1.w, l(3.141593)
    r1.w = ((r1.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 138: div r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)/(r1.wwww)).z;
    // 139: mad r1.w, -r0.y, r0.y, l(1.000000)
    r1.w = ((-(r0.yyyy))*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: mad r3.w, r2.w, r1.w, r1.y
    r3.w = ((r2.wwww)*(r1.wwww)+(r1.yyyy)).w;
    // 141: mad r1.y, r1.x, r1.w, r1.y
    r1.y = ((r1.xxxx)*(r1.wwww)+(r1.yyyy)).y;
    // 142: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 143: mad r1.y, r1.x, r3.w, r1.y
    r1.y = ((r1.xxxx)*(r3.wwww)+(r1.yyyy)).y;
    // 144: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 145: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 146: mul r1.z, r0.x, l(0.080000)
    r1.z = ((r0.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 147: mad r4.xyz, -r0.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r5.xyzx
    r4.xyz = ((-(r0.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r5.xyzx)).xyz;
    // 148: mad r4.xyz, r0.zzzz, r4.xyzx, r1.zzzz
    r4.xyz = ((r0.zzzz)*(r4.xyzx)+(r1.zzzz)).xyz;
    // 149: add r0.xy, -r0.wyww, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.wyww))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 150: mul r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 151: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 152: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 153: mul_sat r0.z, r4.y, l(50.000000)
    r0.z = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 154: mul r0.z, r0.x, r0.z
    r0.z = ((r0.xxxx)*(r0.zzzz)).z;
    // 155: max r5.xyz, r4.xyzx, r0.yyyy
    r5.xyz = (max(r4.xyzx,r0.yyyy)).xyz;
    // 156: add r5.xyz, -r4.xyzx, r5.xyzx
    r5.xyz = ((-(r4.xyzx))+(r5.xyzx)).xyz;
    // 157: mad r0.xyw, -r0.xxxx, r4.xyxz, r4.xyxz
    r0.xyw = ((-(r0.xxxx))*(r4.xyxz)+(r4.xyxz)).xyw;
    // 158: mad r0.xyz, r0.zzzz, r5.xyzx, r0.xywx
    r0.xyz = ((r0.zzzz)*(r5.xyzx)+(r0.xywx)).xyz;
    // 159: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 160: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 161: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 162: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 163: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 164: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 165: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 166: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 167: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 168: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 169: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 170: mul o0.xyz, r0.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 171: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 172: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 173: ret
    return output;
}

// source.character.static-map-native-1132.v1 / source program 988561597eac1c4db03de94ea824a383
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1132(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[9];
    source[4]=g_SourceCharacterLightConstants[10];
    source[5]=g_SourceCharacterLightConstants[11];
    source[6]=g_SourceCharacterLightConstants[12];
    source[7]=g_SourceCharacterLightConstants[16];
    source[7].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[7].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[8]=g_SourceCharacterLightConstants[17];
    source[9]=g_SourceCharacterLightConstants[18];
    source[10]=g_SourceCharacterLightConstants[19];
    source[11]=float4(input.lightColor,1.f);
    source[12].x=1.f;
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
    // 68: add r0.y, r8.w, l(-0.333300)
    r0.y = ((r8.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).y;
    // 69: lt r0.y, r0.y, l(0.000000)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 70: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 71: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
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
    // 81: mul r7.xyz, cb0[3].xyzx, cb0[7].wwww
    r7.xyz = ((source[3].xyzx)*(source[7].wwww)).xyz;
    // 82: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 83: mul r10.xyz, cb0[4].xyzx, cb0[8].xxxx
    r10.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // 84: mul r11.xyz, r6.xyzx, r10.xyzx
    r11.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 85: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 86: mad r6.xyz, -r10.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r10.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 87: mad r6.xyz, cb0[8].zzzz, r6.xyzx, r11.xyzx
    r6.xyz = ((source[8].zzzz)*(r6.xyzx)+(r11.xyzx)).xyz;
    // 88: mad r6.xyz, -r8.xyzx, r7.xyzx, r6.xyzx
    r6.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r6.xyzx)).xyz;
    // 89: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 90: mul r6.xyz, r0.xyzx, cb0[8].wwww
    r6.xyz = ((r0.xyzx)*(source[8].wwww)).xyz;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t4.yzxw, s5, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 92: mul r1.w, r7.y, cb0[9].y
    r1.w = ((r7.yyyy)*(source[9].yyyy)).w;
    // 93: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 94: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 95: mul r1.w, r1.w, cb0[9].z
    r1.w = ((r1.wwww)*(source[9].zzzz)).w;
    // 96: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 97: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 98: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mad r0.xyz, cb0[9].xxxx, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[9].xxxx)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 100: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 101: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 102: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 103: mov_sat r2.w, cb0[9].w
    r2.w = (saturate(source[9].wwww)).w;
    // 104: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 105: mul r3.w, r7.x, cb0[10].y
    r3.w = ((r7.xxxx)*(source[10].yyyy)).w;
    // 106: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 107: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 108: mul r3.w, r3.w, cb0[10].z
    r3.w = ((r3.wwww)*(source[10].zzzz)).w;
    // 109: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 110: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 111: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 112: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 114: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 115: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 116: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 117: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 118: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 119: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 120: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 122: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 123: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 126: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 127: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 129: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 130: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 131: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 132: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 133: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 134: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 135: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 136: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 137: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 138: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 139: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 140: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 141: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 142: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 143: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 144: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 145: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 146: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 147: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 149: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 150: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 151: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 152: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 153: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 155: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 156: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 157: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 158: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 159: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 160: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 161: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 162: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 163: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 164: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 166: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 167: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 168: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 169: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 170: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 171: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 172: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 173: ret
    return output;
}

// source.character.static-map-native-1133.v1 / source program 7a1e5a7db87ebf4e9d691a7310e4ff70
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1133(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[2];
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r4.xyzw, v4.xyxy, cb0[5].yyww
    r4.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r4.xyxx, t1.zwxy, s2, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 16: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 17: mul r2.zw, r2.zzzw, cb0[5].zzzz
    r2.zw = ((r2.zzzw)*(source[5].zzzz)).zw;
    // 18: mad r2.xy, cb0[5].xxxx, r2.xyxx, r2.zwzz
    r2.xy = ((source[5].xxxx)*(r2.xyxx)+(r2.zwzz)).xy;
    // 19: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 20: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r4.zwzz, t2.xyzw, s3, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 24: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 25: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 26: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 28: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 29: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 30: mul r5.xy, r3.xyxx, cb0[6].xxxx
    r5.xy = ((r3.xyxx)*(source[6].xxxx)).xy;
    // 31: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 33: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 35: add r3.x, -v2.x, l(1.000000)
    r3.x = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul r3.y, r2.z, r2.z
    r3.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 38: mul_sat r3.y, r3.y, r6.w
    r3.y = (saturate((r3.yyyy)*(r6.wwww))).y;
    // 39: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.zwzz, t4.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: mul r3.z, r4.w, r4.w
    r3.z = ((r4.wwww)*(r4.wwww)).z;
    // 42: mul r3.y, r3.z, r3.y
    r3.y = ((r3.zzzz)*(r3.yyyy)).y;
    // 43: mul r3.z, r1.w, r3.y
    r3.z = ((r1.wwww)*(r3.yyyy)).z;
    // 44: mad r3.x, r3.x, r3.z, r3.x
    r3.x = ((r3.xxxx)*(r3.zzzz)+(r3.xxxx)).x;
    // 45: add r1.w, -r1.w, r3.x
    r1.w = ((-(r1.wwww))+(r3.xxxx)).w;
    // 46: mul r3.z, r1.w, r2.w
    r3.z = ((r1.wwww)*(r2.wwww)).z;
    // 47: mad r1.w, -r2.w, r1.w, r3.x
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.xxxx)).w;
    // 48: mad_sat r1.w, r3.y, r1.w, r3.z
    r1.w = (saturate((r3.yyyy)*(r1.wwww)+(r3.zzzz))).w;
    // 49: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 50: add r3.xyz, -r2.xyzx, r5.xyzx
    r3.xyz = ((-(r2.xyzx))+(r5.xyzx)).xyz;
    // 51: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 52: dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 53: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 54: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 55: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].xxxx)) * 0xffffffffu)).w;
    // 56: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 57: div r3.xy, v8.xyxx, v8.wwww
    r3.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 58: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 59: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t6.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 60: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 61: else
    } else {
    // 62: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 63: endif
    }
    // 64: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 65: mul r7.xyz, cb0[2].xyzx, cb0[6].zzzz
    r7.xyz = ((source[2].xyzx)*(source[6].zzzz)).xyz;
    // 66: mul r7.xyz, r6.xyzx, r7.xyzx
    r7.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 67: mul r8.xyz, cb0[3].xyzx, cb0[6].wwww
    r8.xyz = ((source[3].xyzx)*(source[6].wwww)).xyz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t5.yzxw, s6, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 69: mul r2.w, r9.y, cb0[7].x
    r2.w = ((r9.yyyy)*(source[7].xxxx)).w;
    // 70: lt r3.w, |r2.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 71: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 72: mul r2.w, r2.w, cb0[7].y
    r2.w = ((r2.wwww)*(source[7].yyyy)).w;
    // 73: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 74: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 75: min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mad r6.xyz, r8.xyzx, r6.xyzx, -r7.xyzx
    r6.xyz = ((r8.xyzx)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 77: mad r6.xyz, r3.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 78: mul r7.xyz, cb0[4].xyzx, cb0[7].zzzz
    r7.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // 79: mul r8.xyz, r4.xyzx, r7.xyzx
    r8.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 80: dp3 r4.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: mad r4.xyz, -r7.xyzx, r4.xyzx, r4.wwww
    r4.xyz = ((-(r7.xyzx))*(r4.xyzx)+(r4.wwww)).xyz;
    // 82: mad r4.xyz, cb0[8].xxxx, r4.xyzx, r8.xyzx
    r4.xyz = ((source[8].xxxx)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 83: add r4.xyz, -r6.xyzx, r4.xyzx
    r4.xyz = ((-(r6.xyzx))+(r4.xyzx)).xyz;
    // 84: mad r4.xyz, r1.wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 85: mul r6.xyz, r4.xyzx, cb0[8].yyyy
    r6.xyz = ((r4.xyzx)*(source[8].yyyy)).xyz;
    // 86: mad r4.xyz, cb0[8].zzzz, r4.xyzx, -r6.xyzx
    r4.xyz = ((source[8].zzzz)*(r4.xyzx)+(-(r6.xyzx))).xyz;
    // 87: mad r4.xyz, r3.wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 88: mul r4.xyz, r5.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)*(r4.xyzx)).xyz;
    // 89: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 90: mov_sat r1.w, cb0[8].w
    r1.w = (saturate(source[8].wwww)).w;
    // 91: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 92: mul r3.w, r9.x, cb0[9].y
    r3.w = ((r9.xxxx)*(source[9].yyyy)).w;
    // 93: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 94: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 95: mul r3.w, r3.w, cb0[9].z
    r3.w = ((r3.wwww)*(source[9].zzzz)).w;
    // 96: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 97: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 98: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 99: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 101: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 102: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 103: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 104: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 105: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 106: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 107: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: dp3_sat r1.x, r2.xyzx, r1.xyzx
    r1.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 109: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 110: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 111: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 112: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 113: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 114: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: mad r0.yzw, -r4.xxyz, r2.wwww, r4.xxyz
    r0.yzw = ((-(r4.xxyz))*(r2.wwww)+(r4.xxyz)).yzw;
    // 116: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 117: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 118: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 119: mad r2.x, r4.w, r1.z, -r4.w
    r2.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 120: mad r2.x, r2.x, r4.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 121: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 122: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 123: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 124: mad r2.x, -r3.w, r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 125: mad r2.y, r5.w, r2.x, r1.y
    r2.y = ((r5.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 126: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 127: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 128: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 129: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 130: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 131: mul r1.z, r1.w, l(0.080000)
    r1.z = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 132: mad r2.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r2.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 133: mad r2.xyz, r2.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 134: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 136: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 137: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 138: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 139: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 140: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: max r4.xyz, r2.xyzx, r1.wwww
    r4.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 142: add r4.xyz, -r2.xyzx, r4.xyzx
    r4.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 143: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 144: mad r2.xyz, r1.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 145: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 146: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 147: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 148: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 149: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 150: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 151: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 152: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 153: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 154: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 155: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 156: mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 157: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 158: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 159: ret
    return output;
}

// source.character.static-map-native-1134.v1 / source program 7e91634fe16bbc44b1337931eb8a6e49
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1134(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r2.xyz, r2.xyzx, cb0[5].xxzx
    r2.xyz = ((r2.xyzx)*(source[5].xxzx)).xyz;
    // 15: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 16: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 17: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 18: div r2.xyw, r3.xyxz, r1.wwww
    r2.xyw = ((r3.xyxz)/(r1.wwww)).xyw;
    // 19: dp3 r1.w, r2.xywx, r2.xywx
    r1.w = (dot((r2.xywx).xyz,(r2.xywx).xyz).xxxx).w;
    // 20: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 21: mul r2.xyw, r1.wwww, r2.xyxw
    r2.xyw = ((r1.wwww)*(r2.xyxw)).xyw;
    // 22: dp3 r1.w, r2.xywx, r0.xyzx
    r1.w = (dot((r2.xywx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 23: mul r3.xy, r1.wwww, r2.xyxx
    r3.xy = ((r1.wwww)*(r2.xyxx)).xy;
    // 24: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r0.xyxx
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r0.xyxx))).xy;
    // 25: ne r3.z, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r3.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).z;
    // 26: if_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) {
    // 27: div r3.zw, v8.xxxy, v8.wwww
    r3.zw = ((v8.xxxy)/(v8.wwww)).zw;
    // 28: mad r3.zw, r3.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r3.zw = ((r3.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 29: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r3.zwzz, t4.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 30: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 31: else
    } else {
    // 32: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 33: endif
    }
    // 34: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 35: add r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 36: mad r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 37: mad r3.xy, r3.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 39: mul r3.xyz, r3.xyzx, cb0[2].xyzx
    r3.xyz = ((r3.xyzx)*(source[2].xyzx)).xyz;
    // 40: mad r3.xyz, cb0[5].yyyy, r3.xyzx, r3.xyzx
    r3.xyz = ((source[5].yyyy)*(r3.xyzx)+(r3.xyzx)).xyz;
    // 41: add r3.xyz, r3.xyzx, -cb0[5].yyyy
    r3.xyz = ((r3.xyzx)+(-(source[5].yyyy))).xyz;
    // 42: mov_sat r6.xyz, r3.xyzx
    r6.xyz = (saturate(r3.xyzx)).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 44: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 45: add r8.xyz, -r7.xyzx, r3.wwww
    r8.xyz = ((-(r7.xyzx))+(r3.wwww)).xyz;
    // 46: mad r8.xyz, cb0[6].xxxx, r8.xyzx, r7.xyzx
    r8.xyz = ((source[6].xxxx)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 47: mul r9.xyz, cb0[3].xyzx, cb0[6].yyyy
    r9.xyz = ((source[3].xyzx)*(source[6].yyyy)).xyz;
    // 48: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 49: mul r9.xyz, cb0[4].xyzx, cb0[6].zzzz
    r9.xyz = ((source[4].xyzx)*(source[6].zzzz)).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t3.yzxw, s4, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 51: mul r3.w, r10.y, cb0[6].w
    r3.w = ((r10.yyyy)*(source[6].wwww)).w;
    // 52: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 53: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 54: mul r3.w, r3.w, cb0[7].x
    r3.w = ((r3.wwww)*(source[7].xxxx)).w;
    // 55: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 56: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 57: min r4.w, r3.w, l(1.000000)
    r4.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: mad r7.xyz, r9.xyzx, r7.xyzx, -r8.xyzx
    r7.xyz = ((r9.xyzx)*(r7.xyzx)+(-(r8.xyzx))).xyz;
    // 59: mad r7.xyz, r4.wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((r4.wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 60: mad r6.xyz, r2.zzzz, r6.xyzx, r7.xyzx
    r6.xyz = ((r2.zzzz)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 61: mov_sat r3.xyz, -r3.xyzx
    r3.xyz = (saturate(-(r3.xyzx))).xyz;
    // 62: mad r3.xyz, -r2.zzzz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r2.zzzz))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 63: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 64: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 65: min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 66: mul r6.xyz, r3.xyzx, cb0[7].yyyy
    r6.xyz = ((r3.xyzx)*(source[7].yyyy)).xyz;
    // 67: mad r3.xyz, cb0[7].zzzz, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[7].zzzz)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 68: mad r3.xyz, r4.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r4.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 69: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 70: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 71: mov_sat r2.z, cb0[7].w
    r2.z = (saturate(source[7].wwww)).z;
    // 72: mul_sat r3.w, r3.w, cb2[3].w
    r3.w = (saturate((r3.wwww)*(passValues[3].wwww))).w;
    // 73: mul r4.w, r10.x, cb0[8].y
    r4.w = ((r10.xxxx)*(source[8].yyyy)).w;
    // 74: lt r5.x, |r4.w|, l(0.000001)
    r5.x = (asfloat((uint4)((abs(r4.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 75: log r4.w, |r4.w|
    r4.w = (log2(abs(r4.wwww))).w;
    // 76: mul r4.w, r4.w, cb0[8].z
    r4.w = ((r4.wwww)*(source[8].zzzz)).w;
    // 77: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 78: movc r4.w, r5.x, l(0), r4.w
    r4.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 79: max r4.w, r4.w, cb0[0].x
    r4.w = (max(r4.wwww,source[0].xxxx)).w;
    // 80: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 82: dp3 r5.w, r5.xyzx, r5.xyzx
    r5.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 83: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 84: mul r5.xyz, r5.wwww, r5.xyzx
    r5.xyz = ((r5.wwww)*(r5.xyzx)).xyz;
    // 85: dp3_sat r5.w, r2.xywx, r5.xyzx
    r5.w = (saturate(dot((r2.xywx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 86: add r1.w, |r1.w|, l(0.000010)
    r1.w = ((abs(r1.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 87: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: dp3_sat r1.x, r2.xywx, r1.xyzx
    r1.x = (saturate(dot((r2.xywx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 89: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 90: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 91: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 92: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 93: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 94: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 95: mad r0.yzw, -r3.xxyz, r3.wwww, r3.xxyz
    r0.yzw = ((-(r3.xxyz))*(r3.wwww)+(r3.xxyz)).yzw;
    // 96: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 97: mul r1.y, r4.w, r4.w
    r1.y = ((r4.wwww)*(r4.wwww)).y;
    // 98: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 99: mad r2.x, r5.w, r1.z, -r5.w
    r2.x = ((r5.wwww)*(r1.zzzz)+(-(r5.wwww))).x;
    // 100: mad r2.x, r2.x, r5.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 101: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 102: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 103: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 104: mad r2.x, -r4.w, r4.w, l(1.000000)
    r2.x = ((-(r4.wwww))*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 105: mad r2.y, r1.w, r2.x, r1.y
    r2.y = ((r1.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 106: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 107: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 108: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 109: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 110: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 111: mul r1.z, r2.z, l(0.080000)
    r1.z = ((r2.zzzz)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 112: mad r2.xyz, -r2.zzzz, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r2.xyz = ((-(r2.zzzz))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 113: mad r2.xyz, r3.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r3.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 114: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 116: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 117: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 118: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 119: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 120: add r1.w, -r4.w, l(1.000000)
    r1.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: max r3.xyz, r2.xyzx, r1.wwww
    r3.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 122: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 123: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 124: mad r2.xyz, r1.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 125: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 126: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 127: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 128: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 129: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 130: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 131: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 132: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 133: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 134: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 135: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 136: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 137: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 138: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 139: ret
    return output;
}

// source.character.static-map-native-1135.v1 / source program 94552f3b8deaaa498b1b992d25021605
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1135(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r2.xy, r2.xyxx, cb0[4].xxxx
    r2.xy = ((r2.xyxx)*(source[4].xxxx)).xy;
    // 15: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 16: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 17: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 18: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 19: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 21: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 22: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[9].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[9].xxxx)) * 0xffffffffu)).w;
    // 23: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 24: div r3.xy, v8.xyxx, v8.wwww
    r3.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 25: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 26: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 27: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 28: else
    } else {
    // 29: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 30: endif
    }
    // 31: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 33: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 34: add r6.xyz, -r5.xyzx, r1.wwww
    r6.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 35: mad r6.xyz, cb0[4].zzzz, r6.xyzx, r5.xyzx
    r6.xyz = ((source[4].zzzz)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 36: mul r7.xyz, cb0[2].xyzx, cb0[4].wwww
    r7.xyz = ((source[2].xyzx)*(source[4].wwww)).xyz;
    // 37: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 38: mul r7.xyz, cb0[3].xyzx, cb0[5].xxxx
    r7.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, v4.xyxx, t2.yzxw, s3, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 40: mul r1.w, r8.y, cb0[5].y
    r1.w = ((r8.yyyy)*(source[5].yyyy)).w;
    // 41: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 42: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 43: mul r1.w, r1.w, cb0[5].z
    r1.w = ((r1.wwww)*(source[5].zzzz)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 46: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: mad r5.xyz, r7.xyzx, r5.xyzx, -r6.xyzx
    r5.xyz = ((r7.xyzx)*(r5.xyzx)+(-(r6.xyzx))).xyz;
    // 48: mad r5.xyz, r2.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 49: mul r6.xyz, r5.xyzx, cb0[5].wwww
    r6.xyz = ((r5.xyzx)*(source[5].wwww)).xyz;
    // 50: mad r5.xyz, cb0[6].xxxx, r5.xyzx, -r6.xyzx
    r5.xyz = ((source[6].xxxx)*(r5.xyzx)+(-(r6.xyzx))).xyz;
    // 51: mad r5.xyz, r2.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 52: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 53: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 54: mov_sat r2.w, cb0[6].y
    r2.w = (saturate(source[6].yyyy)).w;
    // 55: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 56: mul r3.w, r8.x, cb0[6].w
    r3.w = ((r8.xxxx)*(source[6].wwww)).w;
    // 57: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 58: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 59: mul r3.w, r3.w, cb0[7].x
    r3.w = ((r3.wwww)*(source[7].xxxx)).w;
    // 60: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 61: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 62: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 63: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 65: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 66: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 67: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 68: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 69: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 70: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 71: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: dp3_sat r1.x, r2.xyzx, r1.xyzx
    r1.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 73: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 74: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 75: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 76: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 77: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 78: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 79: mad r0.yzw, -r4.xxyz, r1.wwww, r4.xxyz
    r0.yzw = ((-(r4.xxyz))*(r1.wwww)+(r4.xxyz)).yzw;
    // 80: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 81: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 82: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 83: mad r2.x, r4.w, r1.z, -r4.w
    r2.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 84: mad r2.x, r2.x, r4.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 85: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 86: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 87: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 88: mad r2.x, -r3.w, r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 89: mad r2.y, r5.w, r2.x, r1.y
    r2.y = ((r5.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 90: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 91: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 92: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 93: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 94: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 95: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 96: mad r2.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r2.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 97: mad r2.xyz, r1.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 98: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 100: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 101: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 102: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 103: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 104: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: max r4.xyz, r2.xyzx, r1.wwww
    r4.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 106: add r4.xyz, -r2.xyzx, r4.xyzx
    r4.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 107: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 108: mad r2.xyz, r1.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 109: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 110: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 111: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 112: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 113: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 114: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 115: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 116: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 117: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 119: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 120: mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // 121: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 122: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 123: ret
    return output;
}

// source.character.static-map-native-1136.v1 / source program 66453c1069579a4d9592a3b559da11c5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1136(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[10]=g_SourceCharacterLightConstants[10];
    source[11]=float4(input.lightColor,1.f);
    source[12].x=1.f;
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
    // 68: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
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
    // 78: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 79: add r7.xyz, -r8.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))+(r0.yyyy)).xyz;
    // 80: mad r7.xyz, cb0[7].xxxx, r7.xyzx, r8.xyzx
    r7.xyz = ((source[7].xxxx)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 81: mul r8.xyz, cb0[3].xyzx, cb0[7].yyyy
    r8.xyz = ((source[3].xyzx)*(source[7].yyyy)).xyz;
    // 82: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 83: mul r10.xyz, cb0[4].xyzx, cb0[7].zzzz
    r10.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // 84: mul r11.xyz, r6.xyzx, r10.xyzx
    r11.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 85: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 86: mad r6.xyz, -r10.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r10.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 87: mad r6.xyz, cb0[8].xxxx, r6.xyzx, r11.xyzx
    r6.xyz = ((source[8].xxxx)*(r6.xyzx)+(r11.xyzx)).xyz;
    // 88: mad r6.xyz, -r7.xyzx, r8.xyzx, r6.xyzx
    r6.xyz = ((-(r7.xyzx))*(r8.xyzx)+(r6.xyzx)).xyz;
    // 89: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 90: mul r6.xyz, r0.xyzx, cb0[8].yyyy
    r6.xyz = ((r0.xyzx)*(source[8].yyyy)).xyz;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t4.yzxw, s5, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 92: mul r1.w, r7.y, cb0[8].w
    r1.w = ((r7.yyyy)*(source[8].wwww)).w;
    // 93: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 94: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 95: mul r1.w, r1.w, cb0[9].x
    r1.w = ((r1.wwww)*(source[9].xxxx)).w;
    // 96: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 97: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 98: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mad r0.xyz, cb0[8].zzzz, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[8].zzzz)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 100: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 101: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 102: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 103: mov_sat r2.w, cb0[9].y
    r2.w = (saturate(source[9].yyyy)).w;
    // 104: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 105: mul r3.w, r7.x, cb0[9].w
    r3.w = ((r7.xxxx)*(source[9].wwww)).w;
    // 106: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 107: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 108: mul r3.w, r3.w, cb0[10].x
    r3.w = ((r3.wwww)*(source[10].xxxx)).w;
    // 109: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 110: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 111: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 112: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 114: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 115: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 116: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 117: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 118: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 119: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 120: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 122: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 123: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 126: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 127: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 129: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 130: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 131: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 132: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 133: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 134: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 135: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 136: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 137: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 138: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 139: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 140: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 141: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 142: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 143: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 144: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 145: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 146: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 147: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 149: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 150: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 151: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 152: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 153: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 155: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 156: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 157: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 158: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 159: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 160: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 161: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 162: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 163: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 164: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 166: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 167: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 168: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 169: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 170: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 171: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 172: ret
    return output;
}

// source.character.static-map-native-1137.v1 / source program 4be99e8068cccb45a3e48e8a20363d48
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1137(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 2: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 3: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 4: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 5: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r0.x, r1.xyxx, r1.xyxx
    r0.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 10: mul r1.xy, r1.xyxx, cb0[4].xxxx
    r1.xy = ((r1.xyxx)*(source[4].xxxx)).xy;
    // 11: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 13: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 14: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 16: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 17: div r1.xyz, r1.xyzx, r0.xxxx
    r1.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 18: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 19: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 20: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 21: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 22: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 23: mul r2.xyz, r0.xxxx, v7.xyzx
    r2.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 24: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 25: add r0.x, |r0.x|, l(0.000010)
    r0.x = ((abs(r0.xxxx))+(float4(0.000010,0.000010,0.000010,0.000010))).x;
    // 26: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t2.yzxw, s2, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 28: mul r1.w, r3.x, cb0[6].x
    r1.w = ((r3.xxxx)*(source[6].xxxx)).w;
    // 29: max r2.w, r3.y, l(0.000000)
    r2.w = (max(r3.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 30: min r2.w, r2.w, l(100.000000)
    r2.w = (min(r2.wwww,float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 31: mul r2.w, r2.w, cb0[5].x
    r2.w = ((r2.wwww)*(source[5].xxxx)).w;
    // 32: log r3.x, |r1.w|
    r3.x = (log2(abs(r1.wwww))).x;
    // 33: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 34: mul r3.x, r3.x, cb0[6].y
    r3.x = ((r3.xxxx)*(source[6].yyyy)).x;
    // 35: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 36: movc r1.w, r1.w, l(0), r3.x
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 37: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 38: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mad r3.x, -r1.w, r1.w, l(1.000000)
    r3.x = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: mul r3.y, r1.w, r1.w
    r3.y = ((r1.wwww)*(r1.wwww)).y;
    // 41: mad r3.z, r0.x, r3.x, r3.y
    r3.z = ((r0.xxxx)*(r3.xxxx)+(r3.yyyy)).z;
    // 42: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 43: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 44: mul r4.xyz, r3.wwww, v5.xyzx
    r4.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 45: dp3_sat r4.x, r1.xyzx, r4.xyzx
    r4.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 46: mad r3.x, r4.x, r3.x, r3.y
    r3.x = ((r4.xxxx)*(r3.xxxx)+(r3.yyyy)).x;
    // 47: mul r3.y, r3.y, r3.y
    r3.y = ((r3.yyyy)*(r3.yyyy)).y;
    // 48: mul r0.x, r0.x, r3.x
    r0.x = ((r0.xxxx)*(r3.xxxx)).x;
    // 49: mad r0.x, r4.x, r3.z, r0.x
    r0.x = ((r4.xxxx)*(r3.zzzz)+(r0.xxxx)).x;
    // 50: rcp r0.x, r0.x
    r0.x = (1.0/(r0.xxxx)).x;
    // 51: mad r4.yzw, v5.xxyz, r3.wwww, r2.xxyz
    r4.yzw = ((v5.xxyz)*(r3.wwww)+(r2.xxyz)).yzw;
    // 52: mad r3.x, v5.z, r3.w, l(1.000000)
    r3.x = ((v5.zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: dp3 r3.z, r4.yzwy, r4.yzwy
    r3.z = (dot((r4.yzwy).xyz,(r4.yzwy).xyz).xxxx).z;
    // 55: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 56: mul r4.yzw, r3.zzzz, r4.yyzw
    r4.yzw = ((r3.zzzz)*(r4.yyzw)).yzw;
    // 57: dp3_sat r1.x, r1.xyzx, r4.yzwy
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.yzwy).xyz).xxxx)).x;
    // 58: dp3_sat r1.y, r2.xyzx, r4.yzwy
    r1.y = (saturate(dot((r2.xyzx).xyz,(r4.yzwy).xyz).xxxx)).y;
    // 59: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 60: add r1.y, -r3.x, r1.y
    r1.y = ((-(r3.xxxx))+(r1.yyyy)).y;
    // 61: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 62: add r1.yw, -r1.yyyw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r1.yw = ((-(r1.yyyw))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 63: mad r1.z, r1.x, r3.y, -r1.x
    r1.z = ((r1.xxxx)*(r3.yyyy)+(-(r1.xxxx))).z;
    // 64: mad r1.x, r1.z, r1.x, l(1.000000)
    r1.x = ((r1.zzzz)*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 66: mul r1.x, r1.x, l(3.141593)
    r1.x = ((r1.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 67: div r1.x, r3.y, r1.x
    r1.x = ((r3.yyyy)/(r1.xxxx)).x;
    // 68: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 69: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 70: log r1.x, |r2.w|
    r1.x = (log2(abs(r2.wwww))).x;
    // 71: lt r1.z, |r2.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 72: mul r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)*(source[5].yyyy)).x;
    // 73: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 74: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 75: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 76: mul_sat r1.z, r1.x, cb2[3].w
    r1.z = (saturate((r1.xxxx)*(passValues[3].wwww))).z;
    // 77: mul r2.xyz, cb0[2].xyzx, cb0[4].zzzz
    r2.xyz = ((source[2].xyzx)*(source[4].zzzz)).xyz;
    // 78: mul r2.xyz, r0.yzwy, r2.xyzx
    r2.xyz = ((r0.yzwy)*(r2.xyzx)).xyz;
    // 79: mul r3.xyz, cb0[3].xyzx, cb0[4].wwww
    r3.xyz = ((source[3].xyzx)*(source[4].wwww)).xyz;
    // 80: mad r0.yzw, r3.xxyz, r0.yyzw, -r2.xxyz
    r0.yzw = ((r3.xxyz)*(r0.yyzw)+(-(r2.xxyz))).yzw;
    // 81: mad r0.yzw, r1.xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 82: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 83: mul r2.xyz, r0.yzwy, r2.xyzx
    r2.xyz = ((r0.yzwy)*(r2.xyzx)).xyz;
    // 84: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 85: mov_sat r1.x, cb0[5].z
    r1.x = (saturate(source[5].zzzz)).x;
    // 86: mad r3.xyz, -r1.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r3.xyz = ((-(r1.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 87: mad r2.xyz, -r2.xyzx, r1.zzzz, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(r1.zzzz)+(r2.xyzx)).xyz;
    // 88: mul r1.x, r1.x, l(0.080000)
    r1.x = ((r1.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 89: mad r3.xyz, r1.zzzz, r3.xyzx, r1.xxxx
    r3.xyz = ((r1.zzzz)*(r3.xyzx)+(r1.xxxx)).xyz;
    // 90: max r1.xzw, r1.wwww, r3.xxyz
    r1.xzw = (max(r1.wwww,r3.xxyz)).xzw;
    // 91: add r1.xzw, -r3.xxyz, r1.xxzw
    r1.xzw = ((-(r3.xxyz))+(r1.xxzw)).xzw;
    // 92: mul r2.w, r1.y, r1.y
    r2.w = ((r1.yyyy)*(r1.yyyy)).w;
    // 93: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 94: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 95: mul_sat r2.w, r3.y, l(50.000000)
    r2.w = (saturate((r3.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 96: mad r3.xyz, -r1.yyyy, r3.xyzx, r3.xyzx
    r3.xyz = ((-(r1.yyyy))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 97: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 98: mad r1.xyz, r1.yyyy, r1.xzwx, r3.xyzx
    r1.xyz = ((r1.yyyy)*(r1.xzwx)+(r3.xyzx)).xyz;
    // 99: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: add r1.w, r1.w, l(0.000100)
    r1.w = ((r1.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 101: div r1.w, l(3.000000), r1.w
    r1.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r1.wwww)).w;
    // 102: min r0.x, r0.x, r1.w
    r0.x = (min(r0.xxxx,r1.wwww)).x;
    // 103: mul r3.xyz, r1.xyzx, r0.xxxx
    r3.xyz = ((r1.xyzx)*(r0.xxxx)).xyz;
    // 104: add r1.xyz, -r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 105: add r4.yzw, r0.yyzw, l(0.000000, -1.000000, -1.000000, -1.000000)
    r4.yzw = ((r0.yyzw)+(float4(0.000000,-1.000000,-1.000000,-1.000000))).yzw;
    // 106: mad r0.xyz, r0.yzwy, r4.yzwy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((r0.yzwy)*(r4.yzwy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 107: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 108: mul r0.xyz, r0.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 109: mad r0.xyz, r0.xyzx, r1.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 110: mul r0.xyz, r4.xxxx, r0.xyzx
    r0.xyz = ((r4.xxxx)*(r0.xyzx)).xyz;
    // 111: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 112: mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // 113: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 114: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 115: ret
    return output;
}

// source.character.static-map-native-1138.v1 / source program 7427ae507511d2419990f6554e85c4c5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1138(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r2.xyz, r2.xyzx, cb0[5].xxzx
    r2.xyz = ((r2.xyzx)*(source[5].xxzx)).xyz;
    // 15: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 16: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 17: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 18: div r2.xyw, r3.xyxz, r1.wwww
    r2.xyw = ((r3.xyxz)/(r1.wwww)).xyw;
    // 19: dp3 r1.w, r2.xywx, r2.xywx
    r1.w = (dot((r2.xywx).xyz,(r2.xywx).xyz).xxxx).w;
    // 20: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 21: mul r2.xyw, r1.wwww, r2.xyxw
    r2.xyw = ((r1.wwww)*(r2.xyxw)).xyw;
    // 22: dp3 r1.w, r2.xywx, r0.xyzx
    r1.w = (dot((r2.xywx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 23: mul r3.xy, r1.wwww, r2.xyxx
    r3.xy = ((r1.wwww)*(r2.xyxx)).xy;
    // 24: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r0.xyxx
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r0.xyxx))).xy;
    // 25: ne r3.z, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r3.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).z;
    // 26: if_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) {
    // 27: div r3.zw, v8.xxxy, v8.wwww
    r3.zw = ((v8.xxxy)/(v8.wwww)).zw;
    // 28: mad r3.zw, r3.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r3.zw = ((r3.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 29: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r3.zwzz, t4.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 30: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 31: else
    } else {
    // 32: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 33: endif
    }
    // 34: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 35: add r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 36: mad r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 37: mad r3.xy, r3.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 39: mul r3.xyz, r3.xyzx, cb0[2].xyzx
    r3.xyz = ((r3.xyzx)*(source[2].xyzx)).xyz;
    // 40: mad r3.xyz, cb0[5].yyyy, r3.xyzx, r3.xyzx
    r3.xyz = ((source[5].yyyy)*(r3.xyzx)+(r3.xyzx)).xyz;
    // 41: add r3.xyz, r3.xyzx, -cb0[5].yyyy
    r3.xyz = ((r3.xyzx)+(-(source[5].yyyy))).xyz;
    // 42: mov_sat r6.xyz, r3.xyzx
    r6.xyz = (saturate(r3.xyzx)).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 44: mul r8.xyz, cb0[3].xyzx, cb0[5].wwww
    r8.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // 45: mul r8.xyz, r7.xyzx, r8.xyzx
    r8.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 46: mul r9.xyz, cb0[4].xyzx, cb0[6].xxxx
    r9.xyz = ((source[4].xyzx)*(source[6].xxxx)).xyz;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t3.yzxw, s4, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 48: mul r3.w, r10.y, cb0[6].y
    r3.w = ((r10.yyyy)*(source[6].yyyy)).w;
    // 49: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 50: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 51: mul r3.w, r3.w, cb0[6].z
    r3.w = ((r3.wwww)*(source[6].zzzz)).w;
    // 52: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 53: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 54: min r4.w, r3.w, l(1.000000)
    r4.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: mad r7.xyz, r9.xyzx, r7.xyzx, -r8.xyzx
    r7.xyz = ((r9.xyzx)*(r7.xyzx)+(-(r8.xyzx))).xyz;
    // 56: mad r7.xyz, r4.wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((r4.wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 57: mad r6.xyz, r2.zzzz, r6.xyzx, r7.xyzx
    r6.xyz = ((r2.zzzz)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 58: mov_sat r3.xyz, -r3.xyzx
    r3.xyz = (saturate(-(r3.xyzx))).xyz;
    // 59: mad r3.xyz, -r2.zzzz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r2.zzzz))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 60: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 61: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 62: min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 63: mul r6.xyz, r3.xyzx, cb0[6].wwww
    r6.xyz = ((r3.xyzx)*(source[6].wwww)).xyz;
    // 64: mad r3.xyz, cb0[7].xxxx, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[7].xxxx)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 65: mad r3.xyz, r4.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r4.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 66: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 67: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 68: mov_sat r2.z, cb0[7].y
    r2.z = (saturate(source[7].yyyy)).z;
    // 69: mul_sat r3.w, r3.w, cb2[3].w
    r3.w = (saturate((r3.wwww)*(passValues[3].wwww))).w;
    // 70: mul r4.w, r10.x, cb0[7].w
    r4.w = ((r10.xxxx)*(source[7].wwww)).w;
    // 71: lt r5.x, |r4.w|, l(0.000001)
    r5.x = (asfloat((uint4)((abs(r4.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 72: log r4.w, |r4.w|
    r4.w = (log2(abs(r4.wwww))).w;
    // 73: mul r4.w, r4.w, cb0[8].x
    r4.w = ((r4.wwww)*(source[8].xxxx)).w;
    // 74: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 75: movc r4.w, r5.x, l(0), r4.w
    r4.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 76: max r4.w, r4.w, cb0[0].x
    r4.w = (max(r4.wwww,source[0].xxxx)).w;
    // 77: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 78: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 79: dp3 r5.w, r5.xyzx, r5.xyzx
    r5.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 80: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 81: mul r5.xyz, r5.wwww, r5.xyzx
    r5.xyz = ((r5.wwww)*(r5.xyzx)).xyz;
    // 82: dp3_sat r5.w, r2.xywx, r5.xyzx
    r5.w = (saturate(dot((r2.xywx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 83: add r1.w, |r1.w|, l(0.000010)
    r1.w = ((abs(r1.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 84: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: dp3_sat r1.x, r2.xywx, r1.xyzx
    r1.x = (saturate(dot((r2.xywx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 86: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 87: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 88: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 90: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 91: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 92: mad r0.yzw, -r3.xxyz, r3.wwww, r3.xxyz
    r0.yzw = ((-(r3.xxyz))*(r3.wwww)+(r3.xxyz)).yzw;
    // 93: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 94: mul r1.y, r4.w, r4.w
    r1.y = ((r4.wwww)*(r4.wwww)).y;
    // 95: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 96: mad r2.x, r5.w, r1.z, -r5.w
    r2.x = ((r5.wwww)*(r1.zzzz)+(-(r5.wwww))).x;
    // 97: mad r2.x, r2.x, r5.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 98: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 99: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 100: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 101: mad r2.x, -r4.w, r4.w, l(1.000000)
    r2.x = ((-(r4.wwww))*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 102: mad r2.y, r1.w, r2.x, r1.y
    r2.y = ((r1.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 103: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 104: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 105: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 106: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 107: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 108: mul r1.z, r2.z, l(0.080000)
    r1.z = ((r2.zzzz)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 109: mad r2.xyz, -r2.zzzz, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r2.xyz = ((-(r2.zzzz))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 110: mad r2.xyz, r3.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r3.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 111: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 113: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 114: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 115: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 116: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 117: add r1.w, -r4.w, l(1.000000)
    r1.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: max r3.xyz, r2.xyzx, r1.wwww
    r3.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 119: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 120: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 121: mad r2.xyz, r1.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 122: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 123: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 124: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 125: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 126: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 127: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 128: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 129: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 130: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 131: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 132: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 133: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 134: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 135: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 136: ret
    return output;
}

// source.character.static-map-native-1139.v1 / source program 20dc64a59508984e8ca0989480914dc2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1139(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r2.xy, r2.xyxx, cb0[4].xxxx
    r2.xy = ((r2.xyxx)*(source[4].xxxx)).xy;
    // 15: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 16: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 17: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 18: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 19: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 21: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 22: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[9].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[9].xxxx)) * 0xffffffffu)).w;
    // 23: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 24: div r3.xy, v8.xyxx, v8.wwww
    r3.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 25: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 26: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 27: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 28: else
    } else {
    // 29: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 30: endif
    }
    // 31: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 33: mul r6.xyz, cb0[2].xyzx, cb0[4].yyyy
    r6.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // 34: mul r6.xyz, r5.xyzx, r6.xyzx
    r6.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 35: mul r7.xyz, cb0[3].xyzx, cb0[4].zzzz
    r7.xyz = ((source[3].xyzx)*(source[4].zzzz)).xyz;
    // 36: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r8.xyz, -r5.xyzx, r1.wwww
    r8.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 38: mad r5.xyz, cb0[5].xxxx, r8.xyzx, r5.xyzx
    r5.xyz = ((source[5].xxxx)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, v4.xyxx, t2.yzxw, s3, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 40: mul r1.w, r8.y, cb0[5].y
    r1.w = ((r8.yyyy)*(source[5].yyyy)).w;
    // 41: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 42: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 43: mul r1.w, r1.w, cb0[5].z
    r1.w = ((r1.wwww)*(source[5].zzzz)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 46: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: mad r5.xyz, r7.xyzx, r5.xyzx, -r6.xyzx
    r5.xyz = ((r7.xyzx)*(r5.xyzx)+(-(r6.xyzx))).xyz;
    // 48: mad r5.xyz, r2.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 49: mul r6.xyz, r5.xyzx, cb0[5].wwww
    r6.xyz = ((r5.xyzx)*(source[5].wwww)).xyz;
    // 50: mad r5.xyz, cb0[6].xxxx, r5.xyzx, -r6.xyzx
    r5.xyz = ((source[6].xxxx)*(r5.xyzx)+(-(r6.xyzx))).xyz;
    // 51: mad r5.xyz, r2.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 52: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 53: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 54: mov_sat r2.w, cb0[6].y
    r2.w = (saturate(source[6].yyyy)).w;
    // 55: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 56: mul r3.w, r8.x, cb0[6].w
    r3.w = ((r8.xxxx)*(source[6].wwww)).w;
    // 57: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 58: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 59: mul r3.w, r3.w, cb0[7].x
    r3.w = ((r3.wwww)*(source[7].xxxx)).w;
    // 60: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 61: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 62: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 63: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 65: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 66: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 67: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 68: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 69: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 70: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 71: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: dp3_sat r1.x, r2.xyzx, r1.xyzx
    r1.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 73: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 74: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 75: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 76: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 77: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 78: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 79: mad r0.yzw, -r4.xxyz, r1.wwww, r4.xxyz
    r0.yzw = ((-(r4.xxyz))*(r1.wwww)+(r4.xxyz)).yzw;
    // 80: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 81: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 82: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 83: mad r2.x, r4.w, r1.z, -r4.w
    r2.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 84: mad r2.x, r2.x, r4.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 85: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 86: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 87: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 88: mad r2.x, -r3.w, r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 89: mad r2.y, r5.w, r2.x, r1.y
    r2.y = ((r5.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 90: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 91: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 92: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 93: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 94: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 95: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 96: mad r2.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r2.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 97: mad r2.xyz, r1.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 98: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 100: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 101: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 102: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 103: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 104: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: max r4.xyz, r2.xyzx, r1.wwww
    r4.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 106: add r4.xyz, -r2.xyzx, r4.xyzx
    r4.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 107: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 108: mad r2.xyz, r1.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 109: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 110: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 111: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 112: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 113: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 114: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 115: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 116: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 117: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 119: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 120: mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // 121: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 122: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 123: ret
    return output;
}

// source.character.static-map-native-1140.v1 / source program 64eec2a52c4226479aaf280b13ba91b3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1140(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterLightConstants[0];
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
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t2.yzxw, s2, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 2: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 3: mul r0.y, r0.y, cb0[8].z
    r0.y = ((r0.yyyy)*(source[8].zzzz)).y;
    // 4: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 5: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 6: mul r0.z, r0.z, cb0[11].y
    r0.z = ((r0.zzzz)*(source[11].yyyy)).z;
    // 7: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 8: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 9: max r0.x, r0.x, cb0[0].x
    r0.x = (max(r0.xxxx,source[0].xxxx)).x;
    // 10: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 11: add r0.z, -r0.x, l(1.000000)
    r0.z = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 12: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 13: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 14: mul r0.w, r0.w, cb0[8].w
    r0.w = ((r0.wwww)*(source[8].wwww)).w;
    // 15: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 16: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 17: min r0.w, r0.y, l(1.000000)
    r0.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: mul r1.xyz, cb0[5].xyzx, cb0[8].yyyy
    r1.xyz = ((source[5].xyzx)*(source[8].yyyy)).xyz;
    // 19: mul r2.xyz, cb0[4].xyzx, cb0[8].xxxx
    r2.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 21: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 22: add r4.xyz, -r3.xyzx, r1.wwww
    r4.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 23: mad r4.xyz, cb0[7].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[7].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 24: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 25: mad r1.xyz, r1.xyzx, r3.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(-(r2.xyzx))).xyz;
    // 26: mad r1.xyz, r0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 27: add r2.xy, r3.wwww, cb0[10].zxzz
    r2.xy = ((r3.wwww)+(source[10].zxzz)).xy;
    // 28: mul r3.xyz, r1.xyzx, cb0[9].xxxx
    r3.xyz = ((r1.xyzx)*(source[9].xxxx)).xyz;
    // 29: mad r1.xyz, cb0[9].yyyy, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[9].yyyy)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 30: mad r1.xyz, r0.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 31: add r3.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 32: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 33: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 34: mov_sat r0.w, cb0[9].z
    r0.w = (saturate(source[9].zzzz)).w;
    // 35: mad r3.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r3.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 36: mul r0.w, r0.w, l(0.080000)
    r0.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.w, v4.xyxx, t3.yzwx, s3, l(0.000000)
    r1.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 38: mad r2.x, r1.w, -r2.x, r2.x
    r2.x = ((r1.wwww)*(-(r2.xxxx))+(r2.xxxx)).x;
    // 39: add_sat r0.y, r0.y, r2.x
    r0.y = (saturate((r0.yyyy)+(r2.xxxx))).y;
    // 40: mul_sat r0.y, r0.y, cb2[3].w
    r0.y = (saturate((r0.yyyy)*(passValues[3].wwww))).y;
    // 41: mad r2.xzw, r0.yyyy, r3.xxyz, r0.wwww
    r2.xzw = ((r0.yyyy)*(r3.xxyz)+(r0.wwww)).xzw;
    // 42: max r3.xyz, r0.zzzz, r2.xzwx
    r3.xyz = (max(r0.zzzz,r2.xzwx)).xyz;
    // 43: add r3.xyz, -r2.xzwx, r3.xyzx
    r3.xyz = ((-(r2.xzwx))+(r3.xyzx)).xyz;
    // 44: dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 45: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 46: mul r4.xyz, r0.zzzz, v7.xyzx
    r4.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // 47: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 48: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 49: mad r5.xyz, v5.xyzx, r0.zzzz, r4.xyzx
    r5.xyz = ((v5.xyzx)*(r0.zzzz)+(r4.xyzx)).xyz;
    // 50: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 51: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 52: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 53: dp3_sat r0.w, r4.xyzx, r5.xyzx
    r0.w = (saturate(dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 54: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: mad r3.w, v5.z, r0.z, l(1.000000)
    r3.w = ((v5.zzzz)*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: mul r6.xyz, r0.zzzz, v5.xyzx
    r6.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 57: min r0.z, r3.w, l(1.000000)
    r0.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 58: add r0.z, -r0.z, r0.w
    r0.z = ((-(r0.zzzz))+(r0.wwww)).z;
    // 59: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 61: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 62: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 63: mul r3.w, r0.z, r0.w
    r3.w = ((r0.zzzz)*(r0.wwww)).w;
    // 64: mad r0.z, -r0.w, r0.z, l(1.000000)
    r0.z = ((-(r0.wwww))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: mad r7.xyz, -r3.wwww, r2.xzwx, r2.xzwx
    r7.xyz = ((-(r3.wwww))*(r2.xzwx)+(r2.xzwx)).xyz;
    // 66: mul_sat r0.w, r2.z, l(50.000000)
    r0.w = (saturate((r2.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 67: mul r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)*(r0.wwww)).w;
    // 68: mad r3.xyz, r0.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 69: mad r2.xzw, r0.zzzz, r2.xxzw, r0.wwww
    r2.xzw = ((r0.zzzz)*(r2.xxzw)+(r0.wwww)).xzw;
    // 70: add r2.xzw, -r2.xxzw, l(1.000000, 0.000000, 1.000000, 1.000000)
    r2.xzw = ((-(r2.xxzw))+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 71: mul r2.xzw, r2.xxzw, r2.xxzw
    r2.xzw = ((r2.xxzw)*(r2.xxzw)).xzw;
    // 72: add r7.xyz, -r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r3.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 73: add r0.z, -r2.y, l(1.000000)
    r0.z = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 74: mad_sat r0.z, r1.w, r0.z, r2.y
    r0.z = (saturate((r1.wwww)*(r0.zzzz)+(r2.yyyy))).z;
    // 75: mad r0.z, -r0.z, cb0[2].x, l(1.000000)
    r0.z = ((-(r0.zzzz))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 76: mul r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)*(r0.xxxx)).w;
    // 77: mad r0.x, -r0.x, r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 78: mad r1.w, r0.w, l(0.350000), l(1.000000)
    r1.w = ((r0.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: div_sat r0.z, r0.z, r1.w
    r0.z = (saturate((r0.zzzz)/(r1.wwww))).z;
    // 80: mad r2.xyz, -r0.zzzz, r2.xzwx, r7.xyzx
    r2.xyz = ((-(r0.zzzz))*(r2.xzwx)+(r7.xyzx)).xyz;
    // 81: mad r7.xyz, -r1.xyzx, r0.yyyy, r1.xyzx
    r7.xyz = ((-(r1.xyzx))*(r0.yyyy)+(r1.xyzx)).xyz;
    // 82: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 83: mul r7.xyz, r7.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 84: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 85: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 86: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 87: dp2 r0.z, r7.xyxx, r7.xyxx
    r0.z = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).z;
    // 88: mul r7.xy, r7.xyxx, cb0[7].xxxx
    r7.xy = ((r7.xyxx)*(source[7].xxxx)).xy;
    // 89: mul r7.xy, r7.xyxx, v2.wwww
    r7.xy = ((r7.xyxx)*(v2.wwww)).xy;
    // 90: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 91: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 92: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 93: add r7.z, r0.z, l(0.000010)
    r7.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 94: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 95: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 96: div r7.xyz, r7.xyzx, r0.zzzz
    r7.xyz = ((r7.xyzx)/(r0.zzzz)).xyz;
    // 97: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 98: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 99: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 100: dp3 r0.z, r7.xyzx, r4.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 101: add r0.z, |r0.z|, l(0.000010)
    r0.z = ((abs(r0.zzzz))+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 102: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 103: mad r1.w, r0.z, r0.x, r0.w
    r1.w = ((r0.zzzz)*(r0.xxxx)+(r0.wwww)).w;
    // 104: dp3_sat r2.w, r7.xyzx, r6.xyzx
    r2.w = (saturate(dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 105: mad r6.xyz, r7.xyzx, cb0[1].xxxx, r6.xyzx
    r6.xyz = ((r7.xyzx)*(source[1].xxxx)+(r6.xyzx)).xyz;
    // 106: dp3_sat r3.w, r7.xyzx, r5.xyzx
    r3.w = (saturate(dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 107: mad r0.x, r2.w, r0.x, r0.w
    r0.x = ((r2.wwww)*(r0.xxxx)+(r0.wwww)).x;
    // 108: mul r0.xw, r0.xxxw, r0.zzzw
    r0.xw = ((r0.xxxw)*(r0.zzzw)).xw;
    // 109: mad r0.x, r2.w, r1.w, r0.x
    r0.x = ((r2.wwww)*(r1.wwww)+(r0.xxxx)).x;
    // 110: rcp r0.x, r0.x
    r0.x = (1.0/(r0.xxxx)).x;
    // 111: mad r0.z, r3.w, r0.w, -r3.w
    r0.z = ((r3.wwww)*(r0.wwww)+(-(r3.wwww))).z;
    // 112: mad r0.z, r0.z, r3.w, l(1.000000)
    r0.z = ((r0.zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 113: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 114: mul r0.z, r0.z, l(3.141593)
    r0.z = ((r0.zzzz)*(float4(3.141593,3.141593,3.141593,3.141593))).z;
    // 115: div r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)/(r0.zzzz)).z;
    // 116: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 117: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 118: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 119: add r0.z, r0.z, l(0.000100)
    r0.z = ((r0.zzzz)+(float4(0.000100,0.000100,0.000100,0.000100))).z;
    // 120: div r0.z, l(3.000000), r0.z
    r0.z = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.zzzz)).z;
    // 121: min r0.x, r0.z, r0.x
    r0.x = (min(r0.zzzz,r0.xxxx)).x;
    // 122: mad r0.xzw, r0.xxxx, r3.xxyz, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r3.xxyz)+(r2.xxyz)).xzw;
    // 123: mul r0.xzw, r2.wwww, r0.xxzw
    r0.xzw = ((r2.wwww)*(r0.xxzw)).xzw;
    // 124: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 125: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 126: mul r2.xyz, r1.wwww, r6.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 127: dp3_sat r1.w, r4.xyzx, -r2.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(-(r2.xyzx)).xyz).xxxx)).w;
    // 128: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 129: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 130: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 131: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 132: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 133: add_sat r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (saturate((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 134: add_sat r2.xyz, r2.xyzx, cb0[12].yyyy
    r2.xyz = (saturate((r2.xyzx)+(source[12].yyyy))).xyz;
    // 135: mul r2.w, r2.x, cb0[12].z
    r2.w = ((r2.xxxx)*(source[12].zzzz)).w;
    // 136: mul_sat r2.xyz, r2.xyzx, cb0[6].xyzx
    r2.xyz = (saturate((r2.xyzx)*(source[6].xyzx))).xyz;
    // 137: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 138: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 139: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 140: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 141: mad r0.xyz, r0.xzwx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xzwx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 142: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 143: mov o0.w, l(1.000000)
    output.targets[0].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 144: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 145: ret
    return output;
}

// source.character.static-map-native-1141.v1 / source program 9061e7855464144db5553cb9b4e03adf
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1141(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterLightConstants[0];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=g_SourceCharacterLightConstants[9];
    source[10]=g_SourceCharacterLightConstants[10];
    source[11]=g_SourceCharacterLightConstants[11];
    source[12]=g_SourceCharacterLightConstants[12];
    source[13]=g_SourceCharacterLightConstants[13];
    source[14]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: mul r0.xyz, cb0[5].xyzx, cb0[8].yyyy
    r0.xyz = ((source[5].xyzx)*(source[8].yyyy)).xyz;
    // 2: mul r1.xyz, cb0[4].xyzx, cb0[8].xxxx
    r1.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r4.xyz, cb0[7].wwww, r3.xyzx, r2.xyzx
    r4.xyz = ((source[7].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mad r2.xyz, cb0[8].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[8].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 8: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 9: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t2.yzxw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 11: mul r0.w, r2.y, cb0[9].x
    r0.w = ((r2.yyyy)*(source[9].xxxx)).w;
    // 12: mul r1.w, r2.x, cb0[11].z
    r1.w = ((r2.xxxx)*(source[11].zzzz)).w;
    // 13: log r2.x, |r0.w|
    r2.x = (log2(abs(r0.wwww))).x;
    // 14: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 15: mul r2.x, r2.x, cb0[9].y
    r2.x = ((r2.xxxx)*(source[9].yyyy)).x;
    // 16: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 17: movc r0.w, r0.w, l(0), r2.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 18: min r2.x, r0.w, l(1.000000)
    r2.x = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 19: mad r0.xyz, r2.xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 20: mul r1.xyz, r0.xyzx, cb0[9].zzzz
    r1.xyz = ((r0.xyzx)*(source[9].zzzz)).xyz;
    // 21: mad r0.xyz, cb0[9].wwww, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[9].wwww)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 22: mad r0.xyz, r2.xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 24: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 25: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 26: mov_sat r1.x, cb0[10].x
    r1.x = (saturate(source[10].xxxx)).x;
    // 27: mad r2.xyz, -r1.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r2.xyz = ((-(r1.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 28: mul r1.x, r1.x, l(0.080000)
    r1.x = ((r1.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 29: add r1.y, r2.w, cb0[11].x
    r1.y = ((r2.wwww)+(source[11].xxxx)).y;
    // 30: add r1.z, r2.w, cb0[10].z
    r1.z = ((r2.wwww)+(source[10].zzzz)).z;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.w, v4.xyxx, t3.yzwx, s3, l(0.000000)
    r2.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 32: mad r1.y, r2.w, -r1.y, r1.y
    r1.y = ((r2.wwww)*(-(r1.yyyy))+(r1.yyyy)).y;
    // 33: add_sat r0.w, r0.w, r1.y
    r0.w = (saturate((r0.wwww)+(r1.yyyy))).w;
    // 34: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 35: mad r2.xyz, r0.wwww, r2.xyzx, r1.xxxx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xxxx)).xyz;
    // 36: log r1.x, |r1.w|
    r1.x = (log2(abs(r1.wwww))).x;
    // 37: lt r1.y, |r1.w|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 38: mul r1.x, r1.x, cb0[11].w
    r1.x = ((r1.xxxx)*(source[11].wwww)).x;
    // 39: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 40: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 41: max r1.x, r1.x, cb0[0].x
    r1.x = (max(r1.xxxx,source[0].xxxx)).x;
    // 42: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: add r1.y, -r1.x, l(1.000000)
    r1.y = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 44: max r3.xyz, r2.xyzx, r1.yyyy
    r3.xyz = (max(r2.xyzx,r1.yyyy)).xyz;
    // 45: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 46: dp3 r1.y, v7.xyzx, v7.xyzx
    r1.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 47: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 48: mul r4.xyz, r1.yyyy, v7.xyzx
    r4.xyz = ((r1.yyyy)*(v7.xyzx)).xyz;
    // 49: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 50: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 51: mad r5.xyz, v5.xyzx, r1.yyyy, r4.xyzx
    r5.xyz = ((v5.xyzx)*(r1.yyyy)+(r4.xyzx)).xyz;
    // 52: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 53: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 54: mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 55: dp3_sat r1.w, r4.xyzx, r5.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 56: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: mad r3.w, v5.z, r1.y, l(1.000000)
    r3.w = ((v5.zzzz)*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: mul r6.xyz, r1.yyyy, v5.xyzx
    r6.xyz = ((r1.yyyy)*(v5.xyzx)).xyz;
    // 59: min r1.y, r3.w, l(1.000000)
    r1.y = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 60: add r1.y, -r1.y, r1.w
    r1.y = ((-(r1.yyyy))+(r1.wwww)).y;
    // 61: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 62: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 63: mul r1.w, r1.y, r1.y
    r1.w = ((r1.yyyy)*(r1.yyyy)).w;
    // 64: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 65: mul r3.w, r1.y, r1.w
    r3.w = ((r1.yyyy)*(r1.wwww)).w;
    // 66: mad r1.y, -r1.w, r1.y, l(1.000000)
    r1.y = ((-(r1.wwww))*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 67: mad r7.xyz, -r3.wwww, r2.xyzx, r2.xyzx
    r7.xyz = ((-(r3.wwww))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 68: mul_sat r1.w, r2.y, l(50.000000)
    r1.w = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 69: mul r1.w, r3.w, r1.w
    r1.w = ((r3.wwww)*(r1.wwww)).w;
    // 70: mad r3.xyz, r1.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 71: mad r2.xyz, r1.yyyy, r2.xyzx, r1.wwww
    r2.xyz = ((r1.yyyy)*(r2.xyzx)+(r1.wwww)).xyz;
    // 72: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 73: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 74: add r7.xyz, -r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r3.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 75: add r1.y, -r1.z, l(1.000000)
    r1.y = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 76: mad_sat r1.y, r2.w, r1.y, r1.z
    r1.y = (saturate((r2.wwww)*(r1.yyyy)+(r1.zzzz))).y;
    // 77: mad r1.y, -r1.y, cb0[2].x, l(1.000000)
    r1.y = ((-(r1.yyyy))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 78: mul r1.z, r1.x, r1.x
    r1.z = ((r1.xxxx)*(r1.xxxx)).z;
    // 79: mad r1.x, -r1.x, r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 80: mad r1.w, r1.z, l(0.350000), l(1.000000)
    r1.w = ((r1.zzzz)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: div_sat r1.y, r1.y, r1.w
    r1.y = (saturate((r1.yyyy)/(r1.wwww))).y;
    // 82: mad r2.xyz, -r1.yyyy, r2.xyzx, r7.xyzx
    r2.xyz = ((-(r1.yyyy))*(r2.xyzx)+(r7.xyzx)).xyz;
    // 83: mad r7.xyz, -r0.xyzx, r0.wwww, r0.xyzx
    r7.xyz = ((-(r0.xyzx))*(r0.wwww)+(r0.xyzx)).xyz;
    // 84: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r7.xyz, r7.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 86: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r1.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r1.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 88: mad r1.yw, r1.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 89: dp2 r2.w, r1.ywyy, r1.ywyy
    r2.w = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).w;
    // 90: mul r1.yw, r1.yyyw, cb0[7].xxxx
    r1.yw = ((r1.yyyw)*(source[7].xxxx)).yw;
    // 91: mul r7.xy, r1.ywyy, v2.wwww
    r7.xy = ((r1.ywyy)*(v2.wwww)).xy;
    // 92: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 93: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 94: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 95: add r7.z, r1.y, l(0.000010)
    r7.z = ((r1.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 96: dp3 r1.y, r7.xyzx, r7.xyzx
    r1.y = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 97: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 98: div r7.xyz, r7.xyzx, r1.yyyy
    r7.xyz = ((r7.xyzx)/(r1.yyyy)).xyz;
    // 99: dp3 r1.y, r7.xyzx, r7.xyzx
    r1.y = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 100: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 101: mul r7.xyz, r1.yyyy, r7.xyzx
    r7.xyz = ((r1.yyyy)*(r7.xyzx)).xyz;
    // 102: dp3 r1.y, r7.xyzx, r4.xyzx
    r1.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 103: add r1.y, |r1.y|, l(0.000010)
    r1.y = ((abs(r1.yyyy))+(float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 104: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 105: mad r1.w, r1.y, r1.x, r1.z
    r1.w = ((r1.yyyy)*(r1.xxxx)+(r1.zzzz)).w;
    // 106: dp3_sat r2.w, r7.xyzx, r6.xyzx
    r2.w = (saturate(dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 107: mad r6.xyz, r7.xyzx, cb0[1].xxxx, r6.xyzx
    r6.xyz = ((r7.xyzx)*(source[1].xxxx)+(r6.xyzx)).xyz;
    // 108: dp3_sat r3.w, r7.xyzx, r5.xyzx
    r3.w = (saturate(dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 109: mad r1.x, r2.w, r1.x, r1.z
    r1.x = ((r2.wwww)*(r1.xxxx)+(r1.zzzz)).x;
    // 110: mul r1.xz, r1.xxzx, r1.yyzy
    r1.xz = ((r1.xxzx)*(r1.yyzy)).xz;
    // 111: mad r1.x, r2.w, r1.w, r1.x
    r1.x = ((r2.wwww)*(r1.wwww)+(r1.xxxx)).x;
    // 112: rcp r1.x, r1.x
    r1.x = (1.0/(r1.xxxx)).x;
    // 113: mad r1.y, r3.w, r1.z, -r3.w
    r1.y = ((r3.wwww)*(r1.zzzz)+(-(r3.wwww))).y;
    // 114: mad r1.y, r1.y, r3.w, l(1.000000)
    r1.y = ((r1.yyyy)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 115: mul r1.y, r1.y, r1.y
    r1.y = ((r1.yyyy)*(r1.yyyy)).y;
    // 116: mul r1.y, r1.y, l(3.141593)
    r1.y = ((r1.yyyy)*(float4(3.141593,3.141593,3.141593,3.141593))).y;
    // 117: div r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)/(r1.yyyy)).y;
    // 118: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 119: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 120: dp3 r1.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 121: add r1.y, r1.y, l(0.000100)
    r1.y = ((r1.yyyy)+(float4(0.000100,0.000100,0.000100,0.000100))).y;
    // 122: div r1.y, l(3.000000), r1.y
    r1.y = ((float4(3.000000,3.000000,3.000000,3.000000))/(r1.yyyy)).y;
    // 123: min r1.x, r1.y, r1.x
    r1.x = (min(r1.yyyy,r1.xxxx)).x;
    // 124: mad r1.xyz, r1.xxxx, r3.xyzx, r2.xyzx
    r1.xyz = ((r1.xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 125: mul r1.xyz, r2.wwww, r1.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)).xyz;
    // 126: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 127: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 128: mul r2.xyz, r1.wwww, r6.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 129: dp3_sat r1.w, r4.xyzx, -r2.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(-(r2.xyzx)).xyz).xxxx)).w;
    // 130: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 131: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 132: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 133: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 135: add_sat r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (saturate((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 136: add_sat r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = (saturate((r2.xyzx)+(source[12].wwww))).xyz;
    // 137: mul r2.w, r2.x, cb0[13].x
    r2.w = ((r2.xxxx)*(source[13].xxxx)).w;
    // 138: mul_sat r2.xyz, r2.xyzx, cb0[6].xyzx
    r2.xyz = (saturate((r2.xyzx)*(source[6].xyzx))).xyz;
    // 139: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 140: mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 141: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 142: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 143: mad r0.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 144: mul o0.xyz, r0.xyzx, cb0[14].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[14].xyzx)).xyz;
    // 145: mov o0.w, l(1.000000)
    output.targets[0].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 146: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 147: ret
    return output;
}

// source.character.static-map-native-1142.v1 / source program 56b1728240ff7646bf9f3a258d624645
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1142(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterLightConstants[0];
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
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t2.yzxw, s2, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 2: mul r0.x, r0.x, cb0[10].z
    r0.x = ((r0.xxxx)*(source[10].zzzz)).x;
    // 3: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 4: log r0.z, |r0.x|
    r0.z = (log2(abs(r0.xxxx))).z;
    // 5: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 6: mul r0.z, r0.z, cb0[10].w
    r0.z = ((r0.zzzz)*(source[10].wwww)).z;
    // 7: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 8: movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // 9: max r0.x, r0.x, cb0[0].x
    r0.x = (max(r0.xxxx,source[0].xxxx)).x;
    // 10: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 11: add r0.z, -r0.x, l(1.000000)
    r0.z = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 12: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 13: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 14: mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // 15: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 16: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 17: min r0.w, r0.y, l(1.000000)
    r0.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: mul r1.xyz, cb0[5].xyzx, cb0[7].wwww
    r1.xyz = ((source[5].xyzx)*(source[7].wwww)).xyz;
    // 19: mul r2.xyz, cb0[4].xyzx, cb0[7].zzzz
    r2.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 21: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 22: mad r1.xyz, r1.xyzx, r3.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(-(r2.xyzx))).xyz;
    // 23: mad r1.xyz, r0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 24: mul r2.xyz, r1.xyzx, cb0[8].zzzz
    r2.xyz = ((r1.xyzx)*(source[8].zzzz)).xyz;
    // 25: mad r1.xyz, cb0[8].wwww, r1.xyzx, -r2.xyzx
    r1.xyz = ((source[8].wwww)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 26: mad r1.xyz, r0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 27: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 28: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 29: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 30: mov_sat r0.w, cb0[9].x
    r0.w = (saturate(source[9].xxxx)).w;
    // 31: mad r2.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r2.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 32: mul r0.w, r0.w, l(0.080000)
    r0.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 33: add r1.w, r3.w, cb0[10].x
    r1.w = ((r3.wwww)+(source[10].xxxx)).w;
    // 34: add r2.w, r3.w, cb0[9].z
    r2.w = ((r3.wwww)+(source[9].zzzz)).w;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.x, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r3.x = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 36: mad r1.w, r3.x, -r1.w, r1.w
    r1.w = ((r3.xxxx)*(-(r1.wwww))+(r1.wwww)).w;
    // 37: add_sat r0.y, r0.y, r1.w
    r0.y = (saturate((r0.yyyy)+(r1.wwww))).y;
    // 38: mul_sat r0.y, r0.y, cb2[3].w
    r0.y = (saturate((r0.yyyy)*(passValues[3].wwww))).y;
    // 39: mad r2.xyz, r0.yyyy, r2.xyzx, r0.wwww
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r0.wwww)).xyz;
    // 40: max r3.yzw, r0.zzzz, r2.xxyz
    r3.yzw = (max(r0.zzzz,r2.xxyz)).yzw;
    // 41: add r3.yzw, -r2.xxyz, r3.yyzw
    r3.yzw = ((-(r2.xxyz))+(r3.yyzw)).yzw;
    // 42: dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 43: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 44: mul r4.xyz, r0.zzzz, v7.xyzx
    r4.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // 45: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 46: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 47: mad r5.xyz, v5.xyzx, r0.zzzz, r4.xyzx
    r5.xyz = ((v5.xyzx)*(r0.zzzz)+(r4.xyzx)).xyz;
    // 48: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 49: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 50: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 51: dp3_sat r0.w, r4.xyzx, r5.xyzx
    r0.w = (saturate(dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 52: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mad r1.w, v5.z, r0.z, l(1.000000)
    r1.w = ((v5.zzzz)*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: mul r6.xyz, r0.zzzz, v5.xyzx
    r6.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 55: min r0.z, r1.w, l(1.000000)
    r0.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 56: add r0.z, -r0.z, r0.w
    r0.z = ((-(r0.zzzz))+(r0.wwww)).z;
    // 57: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 58: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 59: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 60: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 61: mul r1.w, r0.z, r0.w
    r1.w = ((r0.zzzz)*(r0.wwww)).w;
    // 62: mad r0.z, -r0.w, r0.z, l(1.000000)
    r0.z = ((-(r0.wwww))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 63: mad r7.xyz, -r1.wwww, r2.xyzx, r2.xyzx
    r7.xyz = ((-(r1.wwww))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 64: mul_sat r0.w, r2.y, l(50.000000)
    r0.w = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 65: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 66: mad r3.yzw, r0.wwww, r3.yyzw, r7.xxyz
    r3.yzw = ((r0.wwww)*(r3.yyzw)+(r7.xxyz)).yzw;
    // 67: mad r2.xyz, r0.zzzz, r2.xyzx, r0.wwww
    r2.xyz = ((r0.zzzz)*(r2.xyzx)+(r0.wwww)).xyz;
    // 68: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 70: add r7.xyz, -r3.yzwy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r3.yzwy))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 71: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 72: mad_sat r0.z, r3.x, r0.z, r2.w
    r0.z = (saturate((r3.xxxx)*(r0.zzzz)+(r2.wwww))).z;
    // 73: mad r0.z, -r0.z, cb0[2].x, l(1.000000)
    r0.z = ((-(r0.zzzz))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 74: mul r0.w, r0.x, r0.x
    r0.w = ((r0.xxxx)*(r0.xxxx)).w;
    // 75: mad r0.x, -r0.x, r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 76: mad r1.w, r0.w, l(0.350000), l(1.000000)
    r1.w = ((r0.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: div_sat r0.z, r0.z, r1.w
    r0.z = (saturate((r0.zzzz)/(r1.wwww))).z;
    // 78: mad r2.xyz, -r0.zzzz, r2.xyzx, r7.xyzx
    r2.xyz = ((-(r0.zzzz))*(r2.xyzx)+(r7.xyzx)).xyz;
    // 79: mad r7.xyz, -r1.xyzx, r0.yyyy, r1.xyzx
    r7.xyz = ((-(r1.xyzx))*(r0.yyyy)+(r1.xyzx)).xyz;
    // 80: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 81: mul r7.xyz, r7.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 82: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 83: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 84: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 85: dp2 r0.z, r7.xyxx, r7.xyxx
    r0.z = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).z;
    // 86: mul r7.xy, r7.xyxx, cb0[7].xxxx
    r7.xy = ((r7.xyxx)*(source[7].xxxx)).xy;
    // 87: mul r7.xy, r7.xyxx, v2.wwww
    r7.xy = ((r7.xyxx)*(v2.wwww)).xy;
    // 88: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 89: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 90: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 91: add r7.z, r0.z, l(0.000010)
    r7.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 92: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 93: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 94: div r7.xyz, r7.xyzx, r0.zzzz
    r7.xyz = ((r7.xyzx)/(r0.zzzz)).xyz;
    // 95: dp3 r0.z, r7.xyzx, r7.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 96: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 97: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 98: dp3 r0.z, r7.xyzx, r4.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 99: add r0.z, |r0.z|, l(0.000010)
    r0.z = ((abs(r0.zzzz))+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 100: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 101: mad r1.w, r0.z, r0.x, r0.w
    r1.w = ((r0.zzzz)*(r0.xxxx)+(r0.wwww)).w;
    // 102: dp3_sat r2.w, r7.xyzx, r6.xyzx
    r2.w = (saturate(dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 103: mad r6.xyz, r7.xyzx, cb0[1].xxxx, r6.xyzx
    r6.xyz = ((r7.xyzx)*(source[1].xxxx)+(r6.xyzx)).xyz;
    // 104: dp3_sat r3.x, r7.xyzx, r5.xyzx
    r3.x = (saturate(dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 105: mad r0.x, r2.w, r0.x, r0.w
    r0.x = ((r2.wwww)*(r0.xxxx)+(r0.wwww)).x;
    // 106: mul r0.xw, r0.xxxw, r0.zzzw
    r0.xw = ((r0.xxxw)*(r0.zzzw)).xw;
    // 107: mad r0.x, r2.w, r1.w, r0.x
    r0.x = ((r2.wwww)*(r1.wwww)+(r0.xxxx)).x;
    // 108: rcp r0.x, r0.x
    r0.x = (1.0/(r0.xxxx)).x;
    // 109: mad r0.z, r3.x, r0.w, -r3.x
    r0.z = ((r3.xxxx)*(r0.wwww)+(-(r3.xxxx))).z;
    // 110: mad r0.z, r0.z, r3.x, l(1.000000)
    r0.z = ((r0.zzzz)*(r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 111: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 112: mul r0.z, r0.z, l(3.141593)
    r0.z = ((r0.zzzz)*(float4(3.141593,3.141593,3.141593,3.141593))).z;
    // 113: div r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)/(r0.zzzz)).z;
    // 114: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 115: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 116: dp3 r0.z, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 117: add r0.z, r0.z, l(0.000100)
    r0.z = ((r0.zzzz)+(float4(0.000100,0.000100,0.000100,0.000100))).z;
    // 118: div r0.z, l(3.000000), r0.z
    r0.z = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.zzzz)).z;
    // 119: min r0.x, r0.z, r0.x
    r0.x = (min(r0.zzzz,r0.xxxx)).x;
    // 120: mad r0.xzw, r0.xxxx, r3.yyzw, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r3.yyzw)+(r2.xxyz)).xzw;
    // 121: mul r0.xzw, r2.wwww, r0.xxzw
    r0.xzw = ((r2.wwww)*(r0.xxzw)).xzw;
    // 122: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 123: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 124: mul r2.xyz, r1.wwww, r6.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 125: dp3_sat r1.w, r4.xyzx, -r2.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(-(r2.xyzx)).xyz).xxxx)).w;
    // 126: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 127: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 128: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 129: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 130: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 131: add_sat r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (saturate((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 132: add_sat r2.xyz, r2.xyzx, cb0[11].wwww
    r2.xyz = (saturate((r2.xyzx)+(source[11].wwww))).xyz;
    // 133: mul r2.w, r2.x, cb0[12].x
    r2.w = ((r2.xxxx)*(source[12].xxxx)).w;
    // 134: mul_sat r2.xyz, r2.xyzx, cb0[6].xyzx
    r2.xyz = (saturate((r2.xyzx)*(source[6].xyzx))).xyz;
    // 135: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 136: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 137: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 138: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 139: mad r0.xyz, r0.xzwx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xzwx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 140: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 141: mov o0.w, l(1.000000)
    output.targets[0].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 142: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 143: ret
    return output;
}

// source.character.static-map-native-1143.v1 / source program a10f991a683ee34d885f299c7b9b1756
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1143(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterLightConstants[0];
    source[4]=g_SourceCharacterLightConstants[2];
    source[5]=g_SourceCharacterLightConstants[3];
    source[6]=g_SourceCharacterLightConstants[4];
    source[7]=g_SourceCharacterLightConstants[5];
    source[8]=g_SourceCharacterLightConstants[6];
    source[9]=g_SourceCharacterLightConstants[7];
    source[10]=g_SourceCharacterLightConstants[8];
    source[11]=g_SourceCharacterLightConstants[9];
    source[12]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
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
    // 7: mul r1.xyz, cb0[5].xyzx, cb0[7].zzzz
    r1.xyz = ((source[5].xyzx)*(source[7].zzzz)).xyz;
    // 8: mul r2.xyz, cb0[4].xyzx, cb0[7].yyyy
    r2.xyz = ((source[4].xyzx)*(source[7].yyyy)).xyz;
    // 9: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 10: mad r0.xyz, r1.xyzx, r0.xyzx, -r2.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 11: add r1.xy, r0.wwww, cb0[9].wyww
    r1.xy = ((r0.wwww)+(source[9].wyww)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t2.xwyz, s2, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).zw;
    // 13: mul r0.w, r1.w, cb0[7].w
    r0.w = ((r1.wwww)*(source[7].wwww)).w;
    // 14: mul r1.z, r1.z, cb0[10].y
    r1.z = ((r1.zzzz)*(source[10].yyyy)).z;
    // 15: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 16: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 17: mul r1.w, r1.w, cb0[8].x
    r1.w = ((r1.wwww)*(source[8].xxxx)).w;
    // 18: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 19: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 20: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: mad r0.xyz, r1.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 22: mul r2.xyz, r0.xyzx, cb0[8].yyyy
    r2.xyz = ((r0.xyzx)*(source[8].yyyy)).xyz;
    // 23: mad r0.xyz, cb0[8].zzzz, r0.xyzx, -r2.xyzx
    r0.xyz = ((source[8].zzzz)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 24: mad r0.xyz, r1.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 25: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 26: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 27: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 28: mov_sat r1.w, cb0[8].w
    r1.w = (saturate(source[8].wwww)).w;
    // 29: mad r2.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r2.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 30: mul r1.w, r1.w, l(0.080000)
    r1.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.w, v4.xyxx, t3.yzwx, s3, l(0.000000)
    r2.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 32: mad r1.x, r2.w, -r1.x, r1.x
    r1.x = ((r2.wwww)*(-(r1.xxxx))+(r1.xxxx)).x;
    // 33: add_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)+(r1.xxxx))).w;
    // 34: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 35: mad r2.xyz, r0.wwww, r2.xyzx, r1.wwww
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r1.wwww)).xyz;
    // 36: log r1.x, |r1.z|
    r1.x = (log2(abs(r1.zzzz))).x;
    // 37: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 38: mul r1.x, r1.x, cb0[10].z
    r1.x = ((r1.xxxx)*(source[10].zzzz)).x;
    // 39: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 40: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 41: max r1.x, r1.x, cb0[0].x
    r1.x = (max(r1.xxxx,source[0].xxxx)).x;
    // 42: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 44: max r3.xyz, r2.xyzx, r1.zzzz
    r3.xyz = (max(r2.xyzx,r1.zzzz)).xyz;
    // 45: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 46: dp3 r1.z, v7.xyzx, v7.xyzx
    r1.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 47: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 48: mul r4.xyz, r1.zzzz, v7.xyzx
    r4.xyz = ((r1.zzzz)*(v7.xyzx)).xyz;
    // 49: dp3 r1.z, v5.xyzx, v5.xyzx
    r1.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 50: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 51: mad r5.xyz, v5.xyzx, r1.zzzz, r4.xyzx
    r5.xyz = ((v5.xyzx)*(r1.zzzz)+(r4.xyzx)).xyz;
    // 52: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 53: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 54: mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 55: dp3_sat r1.w, r4.xyzx, r5.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 56: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: mad r3.w, v5.z, r1.z, l(1.000000)
    r3.w = ((v5.zzzz)*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: mul r6.xyz, r1.zzzz, v5.xyzx
    r6.xyz = ((r1.zzzz)*(v5.xyzx)).xyz;
    // 59: min r1.z, r3.w, l(1.000000)
    r1.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: add r1.z, -r1.z, r1.w
    r1.z = ((-(r1.zzzz))+(r1.wwww)).z;
    // 61: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 62: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 63: mul r1.w, r1.z, r1.z
    r1.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 64: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 65: mul r3.w, r1.z, r1.w
    r3.w = ((r1.zzzz)*(r1.wwww)).w;
    // 66: mad r1.z, -r1.w, r1.z, l(1.000000)
    r1.z = ((-(r1.wwww))*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 67: mad r7.xyz, -r3.wwww, r2.xyzx, r2.xyzx
    r7.xyz = ((-(r3.wwww))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 68: mul_sat r1.w, r2.y, l(50.000000)
    r1.w = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 69: mul r1.w, r3.w, r1.w
    r1.w = ((r3.wwww)*(r1.wwww)).w;
    // 70: mad r3.xyz, r1.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 71: mad r2.xyz, r1.zzzz, r2.xyzx, r1.wwww
    r2.xyz = ((r1.zzzz)*(r2.xyzx)+(r1.wwww)).xyz;
    // 72: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 73: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 74: add r7.xyz, -r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r3.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 75: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 76: mad_sat r1.y, r2.w, r1.z, r1.y
    r1.y = (saturate((r2.wwww)*(r1.zzzz)+(r1.yyyy))).y;
    // 77: mad r1.y, -r1.y, cb0[2].x, l(1.000000)
    r1.y = ((-(r1.yyyy))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 78: mul r1.z, r1.x, r1.x
    r1.z = ((r1.xxxx)*(r1.xxxx)).z;
    // 79: mad r1.x, -r1.x, r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 80: mad r1.w, r1.z, l(0.350000), l(1.000000)
    r1.w = ((r1.zzzz)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: div_sat r1.y, r1.y, r1.w
    r1.y = (saturate((r1.yyyy)/(r1.wwww))).y;
    // 82: mad r2.xyz, -r1.yyyy, r2.xyzx, r7.xyzx
    r2.xyz = ((-(r1.yyyy))*(r2.xyzx)+(r7.xyzx)).xyz;
    // 83: mad r7.xyz, -r0.xyzx, r0.wwww, r0.xyzx
    r7.xyz = ((-(r0.xyzx))*(r0.wwww)+(r0.xyzx)).xyz;
    // 84: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r7.xyz, r7.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 86: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r1.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r1.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 88: mad r1.yw, r1.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 89: dp2 r2.w, r1.ywyy, r1.ywyy
    r2.w = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).w;
    // 90: mul r1.yw, r1.yyyw, cb0[7].xxxx
    r1.yw = ((r1.yyyw)*(source[7].xxxx)).yw;
    // 91: mul r7.xy, r1.ywyy, v2.wwww
    r7.xy = ((r1.ywyy)*(v2.wwww)).xy;
    // 92: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 93: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 94: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 95: add r7.z, r1.y, l(0.000010)
    r7.z = ((r1.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 96: dp3 r1.y, r7.xyzx, r7.xyzx
    r1.y = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 97: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 98: div r7.xyz, r7.xyzx, r1.yyyy
    r7.xyz = ((r7.xyzx)/(r1.yyyy)).xyz;
    // 99: dp3 r1.y, r7.xyzx, r7.xyzx
    r1.y = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 100: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 101: mul r7.xyz, r1.yyyy, r7.xyzx
    r7.xyz = ((r1.yyyy)*(r7.xyzx)).xyz;
    // 102: dp3 r1.y, r7.xyzx, r4.xyzx
    r1.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 103: add r1.y, |r1.y|, l(0.000010)
    r1.y = ((abs(r1.yyyy))+(float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 104: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 105: mad r1.w, r1.y, r1.x, r1.z
    r1.w = ((r1.yyyy)*(r1.xxxx)+(r1.zzzz)).w;
    // 106: dp3_sat r2.w, r7.xyzx, r6.xyzx
    r2.w = (saturate(dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 107: mad r6.xyz, r7.xyzx, cb0[1].xxxx, r6.xyzx
    r6.xyz = ((r7.xyzx)*(source[1].xxxx)+(r6.xyzx)).xyz;
    // 108: dp3_sat r3.w, r7.xyzx, r5.xyzx
    r3.w = (saturate(dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 109: mad r1.x, r2.w, r1.x, r1.z
    r1.x = ((r2.wwww)*(r1.xxxx)+(r1.zzzz)).x;
    // 110: mul r1.xz, r1.xxzx, r1.yyzy
    r1.xz = ((r1.xxzx)*(r1.yyzy)).xz;
    // 111: mad r1.x, r2.w, r1.w, r1.x
    r1.x = ((r2.wwww)*(r1.wwww)+(r1.xxxx)).x;
    // 112: rcp r1.x, r1.x
    r1.x = (1.0/(r1.xxxx)).x;
    // 113: mad r1.y, r3.w, r1.z, -r3.w
    r1.y = ((r3.wwww)*(r1.zzzz)+(-(r3.wwww))).y;
    // 114: mad r1.y, r1.y, r3.w, l(1.000000)
    r1.y = ((r1.yyyy)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 115: mul r1.y, r1.y, r1.y
    r1.y = ((r1.yyyy)*(r1.yyyy)).y;
    // 116: mul r1.y, r1.y, l(3.141593)
    r1.y = ((r1.yyyy)*(float4(3.141593,3.141593,3.141593,3.141593))).y;
    // 117: div r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)/(r1.yyyy)).y;
    // 118: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 119: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 120: dp3 r1.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 121: add r1.y, r1.y, l(0.000100)
    r1.y = ((r1.yyyy)+(float4(0.000100,0.000100,0.000100,0.000100))).y;
    // 122: div r1.y, l(3.000000), r1.y
    r1.y = ((float4(3.000000,3.000000,3.000000,3.000000))/(r1.yyyy)).y;
    // 123: min r1.x, r1.y, r1.x
    r1.x = (min(r1.yyyy,r1.xxxx)).x;
    // 124: mad r1.xyz, r1.xxxx, r3.xyzx, r2.xyzx
    r1.xyz = ((r1.xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 125: mul r1.xyz, r2.wwww, r1.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)).xyz;
    // 126: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 127: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 128: mul r2.xyz, r1.wwww, r6.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 129: dp3_sat r1.w, r4.xyzx, -r2.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(-(r2.xyzx)).xyz).xxxx)).w;
    // 130: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 131: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 132: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 133: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 135: add_sat r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (saturate((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 136: add_sat r2.xyz, r2.xyzx, cb0[11].zzzz
    r2.xyz = (saturate((r2.xyzx)+(source[11].zzzz))).xyz;
    // 137: mul r2.w, r2.x, cb0[11].w
    r2.w = ((r2.xxxx)*(source[11].wwww)).w;
    // 138: mul_sat r2.xyz, r2.xyzx, cb0[6].xyzx
    r2.xyz = (saturate((r2.xyzx)*(source[6].xyzx))).xyz;
    // 139: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 140: mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 141: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 142: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 143: mad r0.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 144: mul o0.xyz, r0.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 145: mov o0.w, l(1.000000)
    output.targets[0].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 146: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 147: ret
    return output;
}

// source.character.static-map-native-1144.v1 / source program 6f451626836d6948b9962527388ea85b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1144(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[6];
    source[3]=g_SourceCharacterLightConstants[7];
    source[4]=g_SourceCharacterLightConstants[8];
    source[5]=g_SourceCharacterLightConstants[12];
    source[6]=g_SourceCharacterLightConstants[13];
    source[7]=g_SourceCharacterLightConstants[14];
    source[8]=float4(input.lightColor,1.f);
    source[9].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r2.xy, r2.xyxx, cb0[4].xxxx
    r2.xy = ((r2.xyxx)*(source[4].xxxx)).xy;
    // 15: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 16: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 17: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 18: div r2.xyw, r3.xyxz, r1.wwww
    r2.xyw = ((r3.xyxz)/(r1.wwww)).xyw;
    // 19: dp3 r1.w, r2.xywx, r2.xywx
    r1.w = (dot((r2.xywx).xyz,(r2.xywx).xyz).xxxx).w;
    // 20: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 21: mul r2.xyw, r1.wwww, r2.xyxw
    r2.xyw = ((r1.wwww)*(r2.xyxw)).xyw;
    // 22: dp3 r1.w, r2.xywx, r0.xyzx
    r1.w = (dot((r2.xywx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 23: mul r3.xy, r1.wwww, r2.xyxx
    r3.xy = ((r1.wwww)*(r2.xyxx)).xy;
    // 24: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r0.xyxx
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r0.xyxx))).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: add r3.z, r4.w, l(-0.333300)
    r3.z = ((r4.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).z;
    // 27: lt r3.z, r3.z, l(0.000000)
    r3.z = (asfloat((uint4)((r3.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 28: discard_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 29: ne r3.z, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[9].x
    r3.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[9].xxxx)) * 0xffffffffu)).z;
    // 30: if_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) {
    // 31: div r3.zw, v8.xxxy, v8.wwww
    r3.zw = ((v8.xxxy)/(v8.wwww)).zw;
    // 32: mad r3.zw, r3.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r3.zw = ((r3.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 33: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r3.zwzz, t4.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 34: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 35: else
    } else {
    // 36: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 37: endif
    }
    // 38: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 39: add r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 40: mad r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 41: mad r3.xy, r3.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 43: mul r3.xyz, r3.xyzx, cb0[2].xyzx
    r3.xyz = ((r3.xyzx)*(source[2].xyzx)).xyz;
    // 44: mad r3.xyz, cb0[5].xxxx, r3.xyzx, r3.xyzx
    r3.xyz = ((source[5].xxxx)*(r3.xyzx)+(r3.xyzx)).xyz;
    // 45: add r3.xyz, r3.xyzx, -cb0[5].xxxx
    r3.xyz = ((r3.xyzx)+(-(source[5].xxxx))).xyz;
    // 46: mov_sat r7.xyz, r3.xyzx
    r7.xyz = (saturate(r3.xyzx)).xyz;
    // 47: mul r2.z, r2.z, cb0[5].y
    r2.z = ((r2.zzzz)*(source[5].yyyy)).z;
    // 48: mul r8.xyz, cb0[3].xyzx, cb0[5].zzzz
    r8.xyz = ((source[3].xyzx)*(source[5].zzzz)).xyz;
    // 49: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 50: mad r4.xyz, r2.zzzz, r7.xyzx, r4.xyzx
    r4.xyz = ((r2.zzzz)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 51: mov_sat r3.xyz, -r3.xyzx
    r3.xyz = (saturate(-(r3.xyzx))).xyz;
    // 52: mad r3.xyz, -r2.zzzz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r2.zzzz))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 53: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 54: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 55: min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 56: mul r4.xyz, r3.xyzx, cb0[5].wwww
    r4.xyz = ((r3.xyzx)*(source[5].wwww)).xyz;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t3.yzxw, s4, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 58: mul r2.z, r7.y, cb0[6].y
    r2.z = ((r7.yyyy)*(source[6].yyyy)).z;
    // 59: lt r3.w, |r2.z|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 60: log r2.z, |r2.z|
    r2.z = (log2(abs(r2.zzzz))).z;
    // 61: mul r2.z, r2.z, cb0[6].z
    r2.z = ((r2.zzzz)*(source[6].zzzz)).z;
    // 62: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 63: movc r2.z, r3.w, l(0), r2.z
    r2.z = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).z;
    // 64: min r3.w, r2.z, l(1.000000)
    r3.w = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 65: mad r3.xyz, cb0[6].xxxx, r3.xyzx, -r4.xyzx
    r3.xyz = ((source[6].xxxx)*(r3.xyzx)+(-(r4.xyzx))).xyz;
    // 66: mad r3.xyz, r3.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 67: mul r3.xyz, r6.xyzx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 68: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 69: mov_sat r3.w, cb0[6].w
    r3.w = (saturate(source[6].wwww)).w;
    // 70: mul_sat r2.z, r2.z, cb2[3].w
    r2.z = (saturate((r2.zzzz)*(passValues[3].wwww))).z;
    // 71: mul r4.x, r7.x, cb0[7].y
    r4.x = ((r7.xxxx)*(source[7].yyyy)).x;
    // 72: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 73: log r4.x, |r4.x|
    r4.x = (log2(abs(r4.xxxx))).x;
    // 74: mul r4.x, r4.x, cb0[7].z
    r4.x = ((r4.xxxx)*(source[7].zzzz)).x;
    // 75: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 76: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 77: max r4.x, r4.x, cb0[0].x
    r4.x = (max(r4.xxxx,source[0].xxxx)).x;
    // 78: min r4.x, r4.x, l(1.000000)
    r4.x = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 79: mad r4.yzw, v5.xxyz, r0.wwww, r0.xxyz
    r4.yzw = ((v5.xxyz)*(r0.wwww)+(r0.xxyz)).yzw;
    // 80: dp3 r5.w, r4.yzwy, r4.yzwy
    r5.w = (dot((r4.yzwy).xyz,(r4.yzwy).xyz).xxxx).w;
    // 81: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 82: mul r4.yzw, r4.yyzw, r5.wwww
    r4.yzw = ((r4.yyzw)*(r5.wwww)).yzw;
    // 83: dp3_sat r5.w, r2.xywx, r4.yzwy
    r5.w = (saturate(dot((r2.xywx).xyz,(r4.yzwy).xyz).xxxx)).w;
    // 84: add r1.w, |r1.w|, l(0.000010)
    r1.w = ((abs(r1.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 85: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 86: dp3_sat r1.x, r2.xywx, r1.xyzx
    r1.x = (saturate(dot((r2.xywx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 87: dp3_sat r0.x, r0.xyzx, r4.yzwy
    r0.x = (saturate(dot((r0.xyzx).xyz,(r4.yzwy).xyz).xxxx)).x;
    // 88: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 90: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 91: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 92: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 93: mad r0.yzw, -r3.xxyz, r2.zzzz, r3.xxyz
    r0.yzw = ((-(r3.xxyz))*(r2.zzzz)+(r3.xxyz)).yzw;
    // 94: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 95: mul r1.y, r4.x, r4.x
    r1.y = ((r4.xxxx)*(r4.xxxx)).y;
    // 96: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 97: mad r2.x, r5.w, r1.z, -r5.w
    r2.x = ((r5.wwww)*(r1.zzzz)+(-(r5.wwww))).x;
    // 98: mad r2.x, r2.x, r5.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 100: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 101: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 102: mad r2.x, -r4.x, r4.x, l(1.000000)
    r2.x = ((-(r4.xxxx))*(r4.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: mad r2.y, r1.w, r2.x, r1.y
    r2.y = ((r1.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 104: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 105: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 106: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 107: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 108: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 109: mul r1.z, r3.w, l(0.080000)
    r1.z = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 110: mad r2.xyw, -r3.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r3.xyxz
    r2.xyw = ((-(r3.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r3.xyxz)).xyw;
    // 111: mad r2.xyz, r2.zzzz, r2.xywx, r1.zzzz
    r2.xyz = ((r2.zzzz)*(r2.xywx)+(r1.zzzz)).xyz;
    // 112: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 113: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 114: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 115: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 116: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 117: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 118: add r1.w, -r4.x, l(1.000000)
    r1.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: max r3.xyz, r2.xyzx, r1.wwww
    r3.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 120: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 121: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 122: mad r2.xyz, r1.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 123: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 124: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 125: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 126: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 127: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 128: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 129: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 130: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 131: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 132: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 133: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 134: mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // 135: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 136: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 137: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 138: ret
    return output;
}

// source.character.static-map-native-1145.v1 / source program c6db0a1eb0687643ac94b106b20f21b4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1145(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[10]=g_SourceCharacterLightConstants[10];
    source[11]=float4(input.lightColor,1.f);
    source[12].x=1.f;
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
    // 23: mul r5.zw, v4.xxxy, cb0[5].yyyy
    r5.zw = ((v4.xxxy)*(source[5].yyyy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.zwzz, t1.zwxy, s2, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 25: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 26: mul r5.zw, r5.zzzw, cb0[5].zzzz
    r5.zw = ((r5.zzzw)*(source[5].zzzz)).zw;
    // 27: mad r5.xy, cb0[5].xxxx, r5.xyxx, r5.zwzz
    r5.xy = ((source[5].xxxx)*(r5.xyxx)+(r5.zwzz)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 32: max r1.w, cb0[5].w, l(0.000000)
    r1.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 34: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 36: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 37: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 38: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 39: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 40: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 41: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 42: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: mad r0.x, r0.x, l(0.500000), cb0[6].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).x;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 45: mul r0.y, r5.z, r5.z
    r0.y = ((r5.zzzz)*(r5.zzzz)).y;
    // 46: mul_sat r0.y, r0.y, r6.w
    r0.y = (saturate((r0.yyyy)*(r6.wwww))).y;
    // 47: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 48: mul r1.xy, v4.xyxx, cb0[6].wwww
    r1.xy = ((v4.xyxx)*(source[6].wwww)).xy;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 50: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 51: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 52: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 53: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 54: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 55: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 56: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 57: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 58: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 59: add r1.xyz, -r5.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r5.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 60: mad r1.xyz, r0.yyyy, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 61: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 64: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
    // 65: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 66: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 67: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 68: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 69: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 70: else
    } else {
    // 71: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 72: endif
    }
    // 73: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: dp3 r0.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 75: add r8.xyz, -r6.xyzx, r0.yyyy
    r8.xyz = ((-(r6.xyzx))+(r0.yyyy)).xyz;
    // 76: mad r6.xyz, cb0[7].yyyy, r8.xyzx, r6.xyzx
    r6.xyz = ((source[7].yyyy)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 77: mul r8.xyz, cb0[3].xyzx, cb0[7].zzzz
    r8.xyz = ((source[3].xyzx)*(source[7].zzzz)).xyz;
    // 78: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 79: mul r10.xyz, cb0[4].xyzx, cb0[7].wwww
    r10.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 80: mul r11.xyz, r7.xyzx, r10.xyzx
    r11.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 81: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 82: mad r7.xyz, -r10.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r10.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 83: mad r7.xyz, cb0[8].yyyy, r7.xyzx, r11.xyzx
    r7.xyz = ((source[8].yyyy)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 84: mad r6.xyz, -r6.xyzx, r8.xyzx, r7.xyzx
    r6.xyz = ((-(r6.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 85: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 86: mul r6.xyz, r0.xyzx, cb0[8].zzzz
    r6.xyz = ((r0.xyzx)*(source[8].zzzz)).xyz;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t4.yzxw, s5, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 88: mul r1.w, r7.y, cb0[9].x
    r1.w = ((r7.yyyy)*(source[9].xxxx)).w;
    // 89: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 90: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 91: mul r1.w, r1.w, cb0[9].y
    r1.w = ((r1.wwww)*(source[9].yyyy)).w;
    // 92: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 93: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 94: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mad r0.xyz, cb0[8].wwww, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[8].wwww)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 96: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 97: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 98: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 99: mov_sat r2.w, cb0[9].z
    r2.w = (saturate(source[9].zzzz)).w;
    // 100: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 101: mul r3.w, r7.x, cb0[10].x
    r3.w = ((r7.xxxx)*(source[10].xxxx)).w;
    // 102: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 103: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 104: mul r3.w, r3.w, cb0[10].y
    r3.w = ((r3.wwww)*(source[10].yyyy)).w;
    // 105: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 106: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 107: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 108: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 110: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 111: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 112: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 113: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 114: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 115: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 116: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 118: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 119: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 122: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 123: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 125: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 126: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 127: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 128: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 129: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 130: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 131: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 132: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 133: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 134: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 135: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 136: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 137: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 138: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 139: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 140: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 141: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 142: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 143: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 145: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 146: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 147: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 148: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 149: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 151: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 152: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 153: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 154: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 156: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 157: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 158: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 159: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 160: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 161: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 162: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 163: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 164: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 165: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 166: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 167: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 168: ret
    return output;
}

// source.character.static-map-native-1146.v1 / source program 2517a4f29a5d67489e62e2f08bc0f3de
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1146(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 5: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s0
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
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t3.zwxy, s3, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).xy;
    // 21: mul r2.w, r3.y, cb0[3].z
    r2.w = ((r3.yyyy)*(source[3].zzzz)).w;
    // 22: add r4.xyz, -r2.xyzx, r1.wwww
    r4.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 23: mad r2.xyz, r2.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 24: mad r4.xyz, cb0[3].wwww, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[3].wwww)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 25: mad r3.yzw, r3.yyyy, r4.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r3.yzw = ((r3.yyyy)*(r4.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 26: mul r2.xyz, r2.xyzx, r3.yzwy
    r2.xyz = ((r2.xyzx)*(r3.yzwy)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.yz, v4.xyxx, t0.zxyw, s1, l(0.000000)
    r3.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 28: mad r3.yz, r3.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r3.yz = ((r3.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 29: dp2 r1.w, r3.yzyy, r3.yzyy
    r1.w = (dot((r3.yzyy).xy,(r3.yzyy).xy).xxxx).w;
    // 30: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 33: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 34: mul r4.xy, r3.yzyy, cb0[3].xxxx
    r4.xy = ((r3.yzyy)*(source[3].xxxx)).xy;
    // 35: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 36: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 37: div r3.yzw, r4.xxyz, r1.wwww
    r3.yzw = ((r4.xxyz)/(r1.wwww)).yzw;
    // 38: dp3 r1.w, r3.yzwy, r1.xyzx
    r1.w = (dot((r3.yzwy).xyz,(r1.xyzx).xyz).xxxx).w;
    // 39: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: mul r4.xyz, r0.xyzx, r2.wwww
    r4.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 42: mul r5.xyz, cb0[2].xyzx, cb0[4].zzzz
    r5.xyz = ((source[2].xyzx)*(source[4].zzzz)).xyz;
    // 43: mul r5.xyz, r3.xxxx, r5.xyzx
    r5.xyz = ((r3.xxxx)*(r5.xyzx)).xyz;
    // 44: mad r6.xyz, r5.xyzx, r0.xyzx, -r4.xyzx
    r6.xyz = ((r5.xyzx)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 45: mad r4.xyz, r5.xyzx, r6.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 47: mul r5.xyz, r5.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(source[1].xyzx)).xyz;
    // 48: mul r5.xyz, r5.xyzx, cb0[4].xxxx
    r5.xyz = ((r5.xyzx)*(source[4].xxxx)).xyz;
    // 49: mad r1.xyz, v7.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((v7.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 50: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 51: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 52: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 53: dp3 r0.w, r1.xyzx, r3.yzwy
    r0.w = (dot((r1.xyzx).xyz,(r3.yzwy).xyz).xxxx).w;
    // 54: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 55: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 56: mul r0.w, r0.w, cb0[4].y
    r0.w = ((r0.wwww)*(source[4].yyyy)).w;
    // 57: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 58: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 60: mul r1.xyz, r5.xyzx, r0.wwww
    r1.xyz = ((r5.xyzx)*(r0.wwww)).xyz;
    // 61: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 62: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 63: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 64: mad r0.xyz, r2.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 65: mul r1.xyz, r1.wwww, cb2[3].xyzx
    r1.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 66: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 67: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 68: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 69: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 70: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 71: ret
    return output;
}

// source.character.static-map-native-1147.v1 / source program 17daa4713e8f744f99f7ccc32bbc4382
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1147(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 73: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].xxxx)) * 0xffffffffu)).y;
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
    // 83: mul r8.xyz, cb0[4].xyzx, cb0[9].xxxx
    r8.xyz = ((source[4].xyzx)*(source[9].xxxx)).xyz;
    // 84: mul r10.xyz, r7.xyzx, r8.xyzx
    r10.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 85: mul r11.xyz, cb0[5].xyzx, cb0[9].yyyy
    r11.xyz = ((source[5].xyzx)*(source[9].yyyy)).xyz;
    // 86: mul r12.xyz, r9.xyzx, r11.xyzx
    r12.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 87: dp3 r0.y, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 88: mad r9.xyz, -r11.xyzx, r9.xyzx, r0.yyyy
    r9.xyz = ((-(r11.xyzx))*(r9.xyzx)+(r0.yyyy)).xyz;
    // 89: mad r9.xyz, cb0[9].wwww, r9.xyzx, r12.xyzx
    r9.xyz = ((source[9].wwww)*(r9.xyzx)+(r12.xyzx)).xyz;
    // 90: mad r7.xyz, -r7.xyzx, r8.xyzx, r9.xyzx
    r7.xyz = ((-(r7.xyzx))*(r8.xyzx)+(r9.xyzx)).xyz;
    // 91: mad r0.xyz, r0.xxxx, r7.xyzx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r7.xyzx)+(r10.xyzx)).xyz;
    // 92: mul r7.xyz, r0.xyzx, cb0[10].xxxx
    r7.xyz = ((r0.xyzx)*(source[10].xxxx)).xyz;
    // 93: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.xyxx, t5.yzxw, s6, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 94: mul r1.w, r5.y, cb0[10].z
    r1.w = ((r5.yyyy)*(source[10].zzzz)).w;
    // 95: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 96: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 97: mul r1.w, r1.w, cb0[10].w
    r1.w = ((r1.wwww)*(source[10].wwww)).w;
    // 98: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 99: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 100: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 101: mad r0.xyz, cb0[10].yyyy, r0.xyzx, -r7.xyzx
    r0.xyz = ((source[10].yyyy)*(r0.xyzx)+(-(r7.xyzx))).xyz;
    // 102: mad r0.xyz, r2.wwww, r0.xyzx, r7.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r7.xyzx)).xyz;
    // 103: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 104: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 105: mov_sat r2.w, cb0[11].x
    r2.w = (saturate(source[11].xxxx)).w;
    // 106: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 107: mul r3.w, r5.x, cb0[11].z
    r3.w = ((r5.xxxx)*(source[11].zzzz)).w;
    // 108: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 109: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 110: mul r3.w, r3.w, cb0[11].w
    r3.w = ((r3.wwww)*(source[11].wwww)).w;
    // 111: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 112: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 113: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 114: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 116: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 117: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 118: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 119: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 120: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 121: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 122: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 124: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 125: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 128: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 129: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 131: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 132: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 133: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 134: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 135: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 136: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 137: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 138: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 139: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 140: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 141: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 142: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 143: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 144: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 145: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 146: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 147: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 148: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 149: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 151: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 152: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 153: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 154: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 155: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 157: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 158: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 159: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 160: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 162: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 163: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 164: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 165: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 166: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 168: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 169: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 170: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 171: mul o0.xyz, r0.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 172: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 173: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 174: ret
    return output;
}

// source.character.static-map-native-1148.v1 / source program 80e06e7b72ff264b9ba7c781bfba5894
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1148(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[6];
    source[3]=g_SourceCharacterLightConstants[7];
    source[4]=g_SourceCharacterLightConstants[8];
    source[5]=g_SourceCharacterLightConstants[12];
    source[5].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[5].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[6]=g_SourceCharacterLightConstants[13];
    source[7]=g_SourceCharacterLightConstants[14];
    source[8]=g_SourceCharacterLightConstants[15];
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 10: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 13: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r2.xy, r2.xyxx, cb0[4].xxxx
    r2.xy = ((r2.xyxx)*(source[4].xxxx)).xy;
    // 15: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 16: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 17: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 18: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 19: mul r3.xy, v4.xyxx, cb0[4].yyyy
    r3.xy = ((v4.xyxx)*(source[4].yyyy)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, r3.xyxx, t1.zwxy, s2, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 21: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 22: dp2 r1.w, r3.zwzz, r3.zwzz
    r1.w = (dot((r3.zwzz).xy,(r3.zwzz).xy).xxxx).w;
    // 23: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 25: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 26: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: mul r4.xy, r3.zwzz, cb0[4].zzzz
    r4.xy = ((r3.zwzz)*(source[4].zzzz)).xy;
    // 28: max r1.w, cb0[4].w, l(0.000000)
    r1.w = (max(source[4].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 29: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 30: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 32: add r3.z, -v2.x, l(1.000000)
    r3.z = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 34: mul r3.w, r2.z, r2.z
    r3.w = ((r2.zzzz)*(r2.zzzz)).w;
    // 35: mul_sat r3.w, r3.w, r5.w
    r3.w = (saturate((r3.wwww)*(r5.wwww))).w;
    // 36: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r3.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: mul r3.x, r6.w, r6.w
    r3.x = ((r6.wwww)*(r6.wwww)).x;
    // 39: mul r3.x, r3.x, r3.w
    r3.x = ((r3.xxxx)*(r3.wwww)).x;
    // 40: mul r3.y, r1.w, r3.x
    r3.y = ((r1.wwww)*(r3.xxxx)).y;
    // 41: mad r3.y, r3.z, r3.y, r3.z
    r3.y = ((r3.zzzz)*(r3.yyyy)+(r3.zzzz)).y;
    // 42: add r1.w, -r1.w, r3.y
    r1.w = ((-(r1.wwww))+(r3.yyyy)).w;
    // 43: mul r3.z, r1.w, r2.w
    r3.z = ((r1.wwww)*(r2.wwww)).z;
    // 44: mad r1.w, -r2.w, r1.w, r3.y
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.yyyy)).w;
    // 45: mad_sat r1.w, r3.x, r1.w, r3.z
    r1.w = (saturate((r3.xxxx)*(r1.wwww)+(r3.zzzz))).w;
    // 46: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 47: add r3.xyz, -r2.xyzx, r4.xyzx
    r3.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 48: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 49: dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 50: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 51: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 52: add r2.w, r5.w, l(-0.333300)
    r2.w = ((r5.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 53: lt r2.w, r2.w, l(0.000000)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 54: discard_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) { output.discarded = true; return output; }
    // 55: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).w;
    // 56: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 57: div r3.xy, v8.xyxx, v8.wwww
    r3.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 58: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 59: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t5.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 60: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 61: else
    } else {
    // 62: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 63: endif
    }
    // 64: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 65: mul r7.xyz, cb0[2].xyzx, cb0[5].wwww
    r7.xyz = ((source[2].xyzx)*(source[5].wwww)).xyz;
    // 66: mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 67: mul r9.xyz, cb0[3].xyzx, cb0[6].xxxx
    r9.xyz = ((source[3].xyzx)*(source[6].xxxx)).xyz;
    // 68: mul r10.xyz, r6.xyzx, r9.xyzx
    r10.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 69: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: mad r6.xyz, -r9.xyzx, r6.xyzx, r2.wwww
    r6.xyz = ((-(r9.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 71: mad r6.xyz, cb0[6].zzzz, r6.xyzx, r10.xyzx
    r6.xyz = ((source[6].zzzz)*(r6.xyzx)+(r10.xyzx)).xyz;
    // 72: mad r5.xyz, -r5.xyzx, r7.xyzx, r6.xyzx
    r5.xyz = ((-(r5.xyzx))*(r7.xyzx)+(r6.xyzx)).xyz;
    // 73: mad r5.xyz, r1.wwww, r5.xyzx, r8.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r8.xyzx)).xyz;
    // 74: mul r6.xyz, r5.xyzx, cb0[6].wwww
    r6.xyz = ((r5.xyzx)*(source[6].wwww)).xyz;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t4.yzxw, s5, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 76: mul r1.w, r7.y, cb0[7].y
    r1.w = ((r7.yyyy)*(source[7].yyyy)).w;
    // 77: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 78: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 79: mul r1.w, r1.w, cb0[7].z
    r1.w = ((r1.wwww)*(source[7].zzzz)).w;
    // 80: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 81: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 82: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: mad r5.xyz, cb0[7].xxxx, r5.xyzx, -r6.xyzx
    r5.xyz = ((source[7].xxxx)*(r5.xyzx)+(-(r6.xyzx))).xyz;
    // 84: mad r5.xyz, r2.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 85: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 86: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 87: mov_sat r2.w, cb0[7].w
    r2.w = (saturate(source[7].wwww)).w;
    // 88: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 89: mul r3.w, r7.x, cb0[8].y
    r3.w = ((r7.xxxx)*(source[8].yyyy)).w;
    // 90: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 91: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 92: mul r3.w, r3.w, cb0[8].z
    r3.w = ((r3.wwww)*(source[8].zzzz)).w;
    // 93: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 94: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 95: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 96: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 98: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 99: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 100: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 101: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 102: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 103: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 104: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: dp3_sat r1.x, r2.xyzx, r1.xyzx
    r1.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 106: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 107: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 108: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 109: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 110: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 111: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: mad r0.yzw, -r4.xxyz, r1.wwww, r4.xxyz
    r0.yzw = ((-(r4.xxyz))*(r1.wwww)+(r4.xxyz)).yzw;
    // 113: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 114: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 115: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 116: mad r2.x, r4.w, r1.z, -r4.w
    r2.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 117: mad r2.x, r2.x, r4.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 118: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 119: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 120: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 121: mad r2.x, -r3.w, r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 122: mad r2.y, r5.w, r2.x, r1.y
    r2.y = ((r5.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 123: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 124: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 125: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 126: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 127: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 128: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 129: mad r2.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r2.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 130: mad r2.xyz, r1.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 131: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 132: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 133: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 134: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 135: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 136: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 137: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: max r4.xyz, r2.xyzx, r1.wwww
    r4.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 139: add r4.xyz, -r2.xyzx, r4.xyzx
    r4.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 140: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 141: mad r2.xyz, r1.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 142: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 143: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 144: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 145: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 146: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 147: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 148: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 149: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 150: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 151: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 152: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 153: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 154: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 155: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 156: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 157: ret
    return output;
}

// source.character.static-map-native-1149.v1 / source program 7842f993d749a74d828edd1f776eeb7e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1149(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[7];
    source[4]=g_SourceCharacterLightConstants[8];
    source[5]=g_SourceCharacterLightConstants[12];
    source[5].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[5].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[6]=g_SourceCharacterLightConstants[13];
    source[7]=g_SourceCharacterLightConstants[14];
    source[8]=float4(input.lightColor,1.f);
    source[9].x=1.f;
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
    // 7: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.xyxx, t0.zwxy, s1, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 9: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 10: dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: mul r2.zw, r2.zzzw, cb0[4].wwww
    r2.zw = ((r2.zzzw)*(source[4].wwww)).zw;
    // 16: mul r3.xy, r2.zwzz, v2.wwww
    r3.xy = ((r2.zwzz)*(v2.wwww)).xy;
    // 17: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 18: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 19: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 20: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 21: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 22: mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 24: add r1.w, r4.w, l(-0.333300)
    r1.w = ((r4.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 25: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 26: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 27: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[9].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[9].xxxx)) * 0xffffffffu)).w;
    // 28: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 29: div r2.zw, v8.xxxy, v8.wwww
    r2.zw = ((v8.xxxy)/(v8.wwww)).zw;
    // 30: mad r2.zw, r2.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r2.zw = ((r2.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 31: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r2.zwzz, t3.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 32: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 33: else
    } else {
    // 34: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 35: endif
    }
    // 36: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 37: mul r7.xyz, cb0[3].xyzx, cb0[5].wwww
    r7.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // 38: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 39: mul r7.xyz, r4.xyzx, cb0[6].xxxx
    r7.xyz = ((r4.xyzx)*(source[6].xxxx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t2.yzxw, s3, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 41: mul r1.w, r2.y, cb0[6].z
    r1.w = ((r2.yyyy)*(source[6].zzzz)).w;
    // 42: lt r2.y, |r1.w|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 43: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 44: mul r1.w, r1.w, cb0[6].w
    r1.w = ((r1.wwww)*(source[6].wwww)).w;
    // 45: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 46: movc r1.w, r2.y, l(0), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: min r2.y, r1.w, l(1.000000)
    r2.y = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 48: mad r4.xyz, cb0[6].yyyy, r4.xyzx, -r7.xyzx
    r4.xyz = ((source[6].yyyy)*(r4.xyzx)+(-(r7.xyzx))).xyz;
    // 49: mad r2.yzw, r2.yyyy, r4.xxyz, r7.xxyz
    r2.yzw = ((r2.yyyy)*(r4.xxyz)+(r7.xxyz)).yzw;
    // 50: mul r2.yzw, r6.xxyz, r2.yyzw
    r2.yzw = ((r6.xxyz)*(r2.yyzw)).yzw;
    // 51: mad_sat r2.yzw, r2.yyzw, cb2[3].wwww, cb2[3].xxyz
    r2.yzw = (saturate((r2.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz))).yzw;
    // 52: mov_sat r3.w, cb0[7].x
    r3.w = (saturate(source[7].xxxx)).w;
    // 53: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 54: mul r2.x, r2.x, cb0[7].z
    r2.x = ((r2.xxxx)*(source[7].zzzz)).x;
    // 55: lt r4.x, |r2.x|, l(0.000001)
    r4.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 56: log r2.x, |r2.x|
    r2.x = (log2(abs(r2.xxxx))).x;
    // 57: mul r2.x, r2.x, cb0[7].w
    r2.x = ((r2.xxxx)*(source[7].wwww)).x;
    // 58: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 59: movc r2.x, r4.x, l(0), r2.x
    r2.x = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 60: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 61: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 62: mad r4.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 63: dp3 r4.w, r4.xyzx, r4.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 64: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 65: mul r4.xyz, r4.wwww, r4.xyzx
    r4.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 66: dp3_sat r4.w, r3.xyzx, r4.xyzx
    r4.w = (saturate(dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx)).w;
    // 67: dp3 r5.w, r3.xyzx, r0.xyzx
    r5.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 68: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 69: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 70: dp3_sat r1.x, r3.xyzx, r1.xyzx
    r1.x = (saturate(dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 71: dp3_sat r0.x, r0.xyzx, r4.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 72: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 73: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 74: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 75: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 76: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 77: mad r0.yzw, -r2.yyzw, r1.wwww, r2.yyzw
    r0.yzw = ((-(r2.yyzw))*(r1.wwww)+(r2.yyzw)).yzw;
    // 78: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 79: mul r1.y, r2.x, r2.x
    r1.y = ((r2.xxxx)*(r2.xxxx)).y;
    // 80: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 81: mad r3.x, r4.w, r1.z, -r4.w
    r3.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 82: mad r3.x, r3.x, r4.w, l(1.000000)
    r3.x = ((r3.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: mul r3.x, r3.x, r3.x
    r3.x = ((r3.xxxx)*(r3.xxxx)).x;
    // 84: mul r3.x, r3.x, l(3.141593)
    r3.x = ((r3.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 85: div r1.z, r1.z, r3.x
    r1.z = ((r1.zzzz)/(r3.xxxx)).z;
    // 86: mad r3.x, -r2.x, r2.x, l(1.000000)
    r3.x = ((-(r2.xxxx))*(r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 87: mad r3.y, r5.w, r3.x, r1.y
    r3.y = ((r5.wwww)*(r3.xxxx)+(r1.yyyy)).y;
    // 88: mad r1.y, r1.x, r3.x, r1.y
    r1.y = ((r1.xxxx)*(r3.xxxx)+(r1.yyyy)).y;
    // 89: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 90: mad r1.y, r1.x, r3.y, r1.y
    r1.y = ((r1.xxxx)*(r3.yyyy)+(r1.yyyy)).y;
    // 91: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 92: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 93: mul r1.z, r3.w, l(0.080000)
    r1.z = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 94: mad r2.yzw, -r3.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r2.yyzw
    r2.yzw = ((-(r3.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r2.yyzw)).yzw;
    // 95: mad r2.yzw, r1.wwww, r2.yyzw, r1.zzzz
    r2.yzw = ((r1.wwww)*(r2.yyzw)+(r1.zzzz)).yzw;
    // 96: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 97: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 98: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 99: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 100: mul_sat r1.z, r2.z, l(50.000000)
    r1.z = (saturate((r2.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 101: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 102: add r1.w, -r2.x, l(1.000000)
    r1.w = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: max r3.xyz, r2.yzwy, r1.wwww
    r3.xyz = (max(r2.yzwy,r1.wwww)).xyz;
    // 104: add r3.xyz, -r2.yzwy, r3.xyzx
    r3.xyz = ((-(r2.yzwy))+(r3.xyzx)).xyz;
    // 105: mad r2.xyz, -r0.xxxx, r2.yzwy, r2.yzwy
    r2.xyz = ((-(r0.xxxx))*(r2.yzwy)+(r2.yzwy)).xyz;
    // 106: mad r2.xyz, r1.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 107: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 108: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 109: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 110: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 111: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 112: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 113: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 114: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 115: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 116: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 117: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 118: mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // 119: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 120: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 121: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 122: ret
    return output;
}

// source.character.static-map-native-1150.v1 / source program e4f30a099c13d740bd43b3575f7202cb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1150(SOURCE_CHARACTER_NATIVE_INPUT input)
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

// source.character.static-map-native-1151.v1 / source program 58bb4dc165c9f94198415d93d444bf6a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1151(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[1];
    source[1]=g_SourceCharacterLightConstants[2];
    source[2]=g_SourceCharacterLightConstants[3];
    source[3]=g_SourceCharacterLightConstants[4];
    source[4]=g_SourceCharacterLightConstants[5];
    source[5]=g_SourceCharacterLightConstants[6];
    source[6]=g_SourceCharacterLightConstants[7];
    source[7]=g_SourceCharacterLightConstants[8];
    source[8]=g_SourceCharacterLightConstants[9];
    source[9]=g_SourceCharacterLightConstants[10];
    source[10]=g_SourceCharacterLightConstants[11];
    source[11]=g_SourceCharacterLightConstants[12];
    source[12]=g_SourceCharacterLightConstants[13];
    source[13]=g_SourceCharacterLightConstants[15];
    source[14]=g_SourceCharacterLightConstants[16];
    source[15]=g_SourceCharacterLightConstants[17];
    source[16]=g_SourceCharacterLightConstants[18];
    source[17]=g_SourceCharacterLightConstants[19];
    source[18]=g_SourceCharacterLightConstants[20];
    source[19]=g_SourceCharacterLightConstants[21];
    source[20]=float4(input.lightColor,1.f);
    source[21].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 4: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 5: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t10.xyzw, s0
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
    // 12: dp3 r1.x, v3.xyzx, v3.xyzx
    r1.x = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).x;
    // 13: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 14: mul r1.xyz, r1.xxxx, v3.xyzx
    r1.xyz = ((r1.xxxx)*(v3.xyzx)).xyz;
    // 15: mul r2.xy, v2.xyxx, cb0[1].xyxx
    r2.xy = ((v2.xyxx)*(source[1].xyxx)).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t9.xyzw, s10, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture9.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 17: mul r2.zw, v2.xxxy, cb0[2].xxxy
    r2.zw = ((v2.xxxy)*(source[2].xxxy)).zw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.zwzz, t2.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 19: mul r5.xy, v2.xyxx, cb0[3].xyxx
    r5.xy = ((v2.xyxx)*(source[3].xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r5.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 21: mul r5.zw, v2.xxxy, cb0[4].xxxy
    r5.zw = ((v2.xxxy)*(source[4].xxxy)).zw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r5.zwzz, t4.xyzw, s5, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: mov r8.x, r4.w
    r8.x = (r4.wwww).x;
    // 24: mov r8.y, r6.w
    r8.y = (r6.wwww).y;
    // 25: mov r8.z, r7.w
    r8.z = (r7.wwww).z;
    // 26: mad_sat r9.xyz, r8.xyzx, v0.xyzx, v0.xyzx
    r9.xyz = (saturate((r8.xyzx)*(v0.xyzx)+(v0.xyzx))).xyz;
    // 27: add r10.xyz, r9.xyzx, -cb0[14].xxxx
    r10.xyz = ((r9.xyzx)+(-(source[14].xxxx))).xyz;
    // 28: mul_sat r10.xyz, r10.xyzx, cb0[15].xxxx
    r10.xyz = (saturate((r10.xyzx)*(source[15].xxxx))).xyz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r11.xyz, v2.xyxx, t5.xywz, s6, l(0.000000)
    r11.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 30: mul r8.xyz, r8.xyzx, r11.zzzz
    r8.xyz = ((r8.xyzx)*(r11.zzzz)).xyz;
    // 31: add r9.xyz, r9.xyzx, -r10.xyzx
    r9.xyz = ((r9.xyzx)+(-(r10.xyzx))).xyz;
    // 32: mad r8.xyz, r8.xyzx, r9.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 33: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 34: mad r3.xyz, r8.xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((r8.xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 35: add r4.xyz, -r3.xyzx, r6.xyzx
    r4.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 36: mad r3.xyz, r8.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r8.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 37: add r4.xyz, -r3.xyzx, r7.xyzx
    r4.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 38: mad r3.xyz, r8.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r8.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 39: mul r4.xyz, cb0[5].xyzx, cb0[16].zzzz
    r4.xyz = ((source[5].xyzx)*(source[16].zzzz)).xyz;
    // 40: mad r6.xyz, cb0[16].wwww, cb0[6].xyzx, -r4.xyzx
    r6.xyz = ((source[16].wwww)*(source[6].xyzx)+(-(r4.xyzx))).xyz;
    // 41: mad r4.xyz, r8.xxxx, r6.xyzx, r4.xyzx
    r4.xyz = ((r8.xxxx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 42: mad r6.xyz, cb0[17].xxxx, cb0[7].xyzx, -r4.xyzx
    r6.xyz = ((source[17].xxxx)*(source[7].xyzx)+(-(r4.xyzx))).xyz;
    // 43: mad r4.xyz, r8.yyyy, r6.xyzx, r4.xyzx
    r4.xyz = ((r8.yyyy)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 44: mad r6.xyz, cb0[17].yyyy, cb0[8].xyzx, -r4.xyzx
    r6.xyz = ((source[17].yyyy)*(source[8].xyzx)+(-(r4.xyzx))).xyz;
    // 45: mad r4.xyz, r8.zzzz, r6.xyzx, r4.xyzx
    r4.xyz = ((r8.zzzz)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 46: mul r4.xyz, r3.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 48: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 49: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 50: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 52: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 53: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 54: mul r6.xy, r2.xyxx, cb0[13].xxxx
    r6.xy = ((r2.xyxx)*(source[13].xxxx)).xy;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.zwzz, t1.xyzw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 56: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 58: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 61: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mul r7.xy, r2.xyxx, cb0[13].zzzz
    r7.xy = ((r2.xyxx)*(source[13].zzzz)).xy;
    // 63: add r2.xyz, -r6.xyzx, r7.xyzx
    r2.xyz = ((-(r6.xyzx))+(r7.xyzx)).xyz;
    // 64: mad r2.xyz, r8.xxxx, r2.xyzx, r6.xyzx
    r2.xyz = ((r8.xxxx)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.xyxx, t6.xyzw, s7, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 66: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 67: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 68: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 70: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 71: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 72: mul r6.xy, r5.xyxx, cb0[15].yyyy
    r6.xy = ((r5.xyxx)*(source[15].yyyy)).xy;
    // 73: add r6.xyz, -r2.xyzx, r6.xyzx
    r6.xyz = ((-(r2.xyzx))+(r6.xyzx)).xyz;
    // 74: mad r2.xyz, r8.yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((r8.yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.zwzz, t7.xyzw, s8, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 76: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 77: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 78: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 80: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 81: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 82: mul r6.xy, r5.xyxx, cb0[15].zzzz
    r6.xy = ((r5.xyxx)*(source[15].zzzz)).xy;
    // 83: add r5.xyz, -r2.xyzx, r6.xyzx
    r5.xyz = ((-(r2.xyzx))+(r6.xyzx)).xyz;
    // 84: mad r2.xyz, r8.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r8.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 85: mad r5.xy, r11.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r11.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 86: mul r5.xy, r5.xyxx, cb0[15].wwww
    r5.xy = ((r5.xyxx)*(source[15].wwww)).xy;
    // 87: mov r5.z, l(0)
    r5.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 88: add r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)+(r5.xyzx)).xyz;
    // 89: mul r5.xy, v2.xyxx, cb0[0].xyxx
    r5.xy = ((v2.xyxx)*(source[0].xyxx)).xy;
    // 90: mul r5.xy, r5.xyxx, cb0[16].xxxx
    r5.xy = ((r5.xyxx)*(source[16].xxxx)).xy;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.xyxx, t8.xyzw, s9, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 92: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 93: mad r2.xy, cb0[16].yyyy, r5.xyxx, r2.xyxx
    r2.xy = ((source[16].yyyy)*(r5.xyxx)+(r2.xyxx)).xy;
    // 94: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 95: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 96: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 97: dp3 r1.w, r2.xyzx, r1.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 98: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 99: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: mul r5.xyz, r0.xyzx, r2.wwww
    r5.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 101: add r6.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r6.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 102: mad r6.xyz, r8.xxxx, r6.xyzx, cb0[9].xyzx
    r6.xyz = ((r8.xxxx)*(r6.xyzx)+(source[9].xyzx)).xyz;
    // 103: add r7.xyz, -r6.xyzx, cb0[11].xyzx
    r7.xyz = ((-(r6.xyzx))+(source[11].xyzx)).xyz;
    // 104: mad r6.xyz, r8.yyyy, r7.xyzx, r6.xyzx
    r6.xyz = ((r8.yyyy)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 105: add r7.xyz, -r6.xyzx, cb0[12].xyzx
    r7.xyz = ((-(r6.xyzx))+(source[12].xyzx)).xyz;
    // 106: mad r6.xyz, r8.zzzz, r7.xyzx, r6.xyzx
    r6.xyz = ((r8.zzzz)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 107: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 108: add r2.w, -cb0[17].w, cb0[17].z
    r2.w = ((-(source[17].wwww))+(source[17].zzzz)).w;
    // 109: mad r2.w, r8.x, r2.w, cb0[17].w
    r2.w = ((r8.xxxx)*(r2.wwww)+(source[17].wwww)).w;
    // 110: add r3.w, -r2.w, cb0[18].x
    r3.w = ((-(r2.wwww))+(source[18].xxxx)).w;
    // 111: mad r2.w, r8.y, r3.w, r2.w
    r2.w = ((r8.yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 112: add r3.w, -r2.w, cb0[18].y
    r3.w = ((-(r2.wwww))+(source[18].yyyy)).w;
    // 113: mad r2.w, r8.z, r3.w, r2.w
    r2.w = ((r8.zzzz)*(r3.wwww)+(r2.wwww)).w;
    // 114: mul r3.xyz, r3.xyzx, r2.wwww
    r3.xyz = ((r3.xyzx)*(r2.wwww)).xyz;
    // 115: mad r1.xyz, v5.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((v5.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 116: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 117: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 118: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 119: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 120: add r1.x, -cb0[18].w, cb0[18].z
    r1.x = ((-(source[18].wwww))+(source[18].zzzz)).x;
    // 121: mad r1.x, r8.x, r1.x, cb0[18].w
    r1.x = ((r8.xxxx)*(r1.xxxx)+(source[18].wwww)).x;
    // 122: add r1.y, -r1.x, cb0[19].x
    r1.y = ((-(r1.xxxx))+(source[19].xxxx)).y;
    // 123: mad r1.x, r8.y, r1.y, r1.x
    r1.x = ((r8.yyyy)*(r1.yyyy)+(r1.xxxx)).x;
    // 124: add r1.y, -r1.x, cb0[19].y
    r1.y = ((-(r1.xxxx))+(source[19].yyyy)).y;
    // 125: mad r1.x, r8.z, r1.y, r1.x
    r1.x = ((r8.zzzz)*(r1.yyyy)+(r1.xxxx)).x;
    // 126: lt r1.y, |r0.w|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 127: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 128: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 129: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 130: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: movc r0.w, r1.y, l(0), r0.w
    r0.w = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 132: mul r1.xyz, r3.xyzx, r0.wwww
    r1.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 133: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 134: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 135: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 136: mad r0.xyz, r4.xyzx, r5.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 137: mul r1.xyz, r1.wwww, cb2[3].xyzx
    r1.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 138: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 139: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 140: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 141: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 142: ret
    return output;
}

// source.character.static-map-native-1152.v1 / source program 112ee16dc87b284791b46482e0b4fc7d
