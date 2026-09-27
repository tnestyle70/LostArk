#ifndef SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
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
#else // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
    case 1100u: return SourceCharacterLight1100(input);
    case 1101u: return SourceCharacterLight1101(input);
    case 1102u: return SourceCharacterLight1102(input);
    case 1103u: return SourceCharacterLight1103(input);
#endif // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
