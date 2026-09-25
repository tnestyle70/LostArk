SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1408(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[1];
    source[1]=g_SourceCharacterLightConstants[2];
    source[2]=g_SourceCharacterLightConstants[3];
    source[3]=float4(input.lightColor,1.f);
    source[4].x=1.f;
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
    // 6: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[4].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[4].xxxx)) * 0xffffffffu)).x;
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
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 16: mul r3.xyz, cb0[0].xyzx, cb0[2].yyyy
    r3.xyz = ((source[0].xyzx)*(source[2].yyyy)).xyz;
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
    // 25: mul r4.xy, r4.xyxx, cb0[2].xxxx
    r4.xy = ((r4.xyxx)*(source[2].xxxx)).xy;
    // 26: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 27: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 28: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 29: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 30: dp3 r1.w, r4.xyzx, r0.yzwy
    r1.w = (dot((r4.xyzx).xyz,(r0.yzwy).xyz).xxxx).w;
    // 31: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mul r5.xyz, cb0[1].xyzx, cb0[2].zzzz
    r5.xyz = ((source[1].xyzx)*(source[2].zzzz)).xyz;
    // 34: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 35: mad r0.xyz, v7.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v7.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 36: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 37: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 38: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 39: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 40: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 41: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 42: mul r0.x, r0.x, cb0[2].w
    r0.x = ((r0.xxxx)*(source[2].wwww)).x;
    // 43: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 44: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 46: mul r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 47: mad r0.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 48: mul r2.xyz, r1.wwww, cb2[3].xyzx
    r2.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 49: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 50: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 51: mul o0.xyz, r0.xyzx, cb0[3].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[3].xyzx)).xyz;
    // 52: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 53: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 54: ret
    return output;
}

// source.character.static-map-native-1409.v1 / source program 3d9609ca2dbfbc4f8726e9231656afea
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1409(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 25: mul r6.xyz, cb0[1].xyzx, cb0[5].wwww
    r6.xyz = ((source[1].xyzx)*(source[5].wwww)).xyz;
    // 26: mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 27: mul r8.xyz, cb0[2].xyzx, cb0[6].xxxx
    r8.xyz = ((source[2].xyzx)*(source[6].xxxx)).xyz;
    // 28: mul r9.xy, v4.xyxx, cb0[5].zzzz
    r9.xy = ((v4.xyxx)*(source[5].zzzz)).xy;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t2.xyzw, s3, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 30: mul r10.xyz, r8.xyzx, r9.xyzx
    r10.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 31: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: mad r8.xyz, -r8.xyzx, r9.xyzx, r1.wwww
    r8.xyz = ((-(r8.xyzx))*(r9.xyzx)+(r1.wwww)).xyz;
    // 33: mad r8.xyz, cb0[6].zzzz, r8.xyzx, r10.xyzx
    r8.xyz = ((source[6].zzzz)*(r8.xyzx)+(r10.xyzx)).xyz;
    // 34: max r1.w, cb0[4].y, l(0.000000)
    r1.w = (max(source[4].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 35: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 36: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 39: mad r10.xy, r10.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((r10.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: dp2 r3.w, r10.xyxx, r10.xyxx
    r3.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 41: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 43: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 44: add r11.z, r3.w, l(0.000010)
    r11.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 45: mul r10.xy, r10.xyxx, cb0[4].xxxx
    r10.xy = ((r10.xyxx)*(source[4].xxxx)).xy;
    // 46: mul r11.xy, r10.xyxx, v2.wwww
    r11.xy = ((r10.xyxx)*(v2.wwww)).xy;
    // 47: dp3 r3.w, r11.xyzx, r11.xyzx
    r3.w = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 48: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 49: div r10.xyz, r11.xyzx, r3.wwww
    r10.xyz = ((r11.xyzx)/(r3.wwww)).xyz;
    // 50: dp3 r1.x, r1.xyzx, r10.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 51: dp3 r1.y, r2.xyzx, r10.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 52: dp3 r1.z, r0.xyzx, r10.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 53: mul r0.xy, cb0[0].xyxx, cb0[4].zzzz
    r0.xy = ((source[0].xyxx)*(source[4].zzzz)).xy;
    // 54: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 55: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 56: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 57: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 58: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: mad r0.x, r0.x, l(0.500000), cb0[5].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].yyyy)).x;
    // 60: mul r0.y, r10.z, r10.z
    r0.y = ((r10.zzzz)*(r10.zzzz)).y;
    // 61: mul_sat r0.y, r0.y, r5.w
    r0.y = (saturate((r0.yyyy)*(r5.wwww))).y;
    // 62: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 63: mul r0.z, r9.w, r9.w
    r0.z = ((r9.wwww)*(r9.wwww)).z;
    // 64: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 65: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 66: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 67: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 68: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 69: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 70: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 71: mad r1.xyz, -r5.xyzx, r6.xyzx, r8.xyzx
    r1.xyz = ((-(r5.xyzx))*(r6.xyzx)+(r8.xyzx)).xyz;
    // 72: mad r1.xyz, r0.xxxx, r1.xyzx, r7.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r7.xyzx)).xyz;
    // 73: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 74: add r2.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 75: mad r2.xyz, r0.yyyy, r2.xyzx, r10.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r10.xyzx)).xyz;
    // 76: dp3 r0.y, r2.xyzx, r3.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 77: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 78: mul r2.xyz, cb0[3].xyzx, cb0[6].wwww
    r2.xyz = ((source[3].xyzx)*(source[6].wwww)).xyz;
    // 79: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 80: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: add r5.xyz, -r9.xyzx, r1.wwww
    r5.xyz = ((-(r9.xyzx))+(r1.wwww)).xyz;
    // 82: mad r5.xyz, cb0[6].zzzz, r5.xyzx, r9.xyzx
    r5.xyz = ((source[6].zzzz)*(r5.xyzx)+(r9.xyzx)).xyz;
    // 83: mad r5.xyz, cb0[7].xxxx, r5.xyzx, -r2.xyzx
    r5.xyz = ((source[7].xxxx)*(r5.xyzx)+(-(r2.xyzx))).xyz;
    // 84: mad r2.xyz, r0.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 85: mad r3.xyz, v7.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v7.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 86: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 87: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 88: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 89: dp3 r0.x, r3.xyzx, r10.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 90: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 91: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 92: mul r0.x, r0.x, cb0[7].y
    r0.x = ((r0.xxxx)*(source[7].yyyy)).x;
    // 93: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 94: min r0.xz, r0.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = (min(r0.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 95: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 96: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 97: mad r0.xzw, r0.zzzz, r1.xxyz, r2.xxyz
    r0.xzw = ((r0.zzzz)*(r1.xxyz)+(r2.xxyz)).xzw;
    // 98: mul r1.xyz, r0.yyyy, cb2[3].xyzx
    r1.xyz = ((r0.yyyy)*(passValues[3].xyzx)).xyz;
    // 99: mad r0.xyz, r0.xzwx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xzwx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 100: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 101: mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // 102: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 103: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 104: ret
    return output;
}

// source.character.static-map-native-1410.v1 / source program ff2acd28d7493144a91308f03eb4e12d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1410(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
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
    // 7: mul r2.xyzw, v2.xyxy, cb0[5].xxzz
    r2.xyzw = ((v2.xyxy)*(source[5].xxzz)).xyzw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 9: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: mul r2.xy, r2.xyxx, cb0[5].yyyy
    r2.xy = ((r2.xyxx)*(source[5].yyyy)).xy;
    // 16: mul r3.xy, r2.xyxx, v0.wwww
    r3.xy = ((r2.xyxx)*(v0.wwww)).xy;
    // 17: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 18: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 19: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.zwzz, t1.xyzw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 21: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 23: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 25: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 26: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: mul r4.xy, r2.xyxx, cb0[5].wwww
    r4.xy = ((r2.xyxx)*(source[5].wwww)).xy;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.zwzz, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 29: add r5.xy, v0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r5.xy = ((v0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 30: mad r1.w, r5.y, l(2.000000), r2.w
    r1.w = ((r5.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(r2.wwww)).w;
    // 31: add r2.w, r1.w, -cb0[6].y
    r2.w = ((r1.wwww)+(-(source[6].yyyy))).w;
    // 32: mul r3.w, r2.w, cb0[6].w
    r3.w = ((r2.wwww)*(source[6].wwww)).w;
    // 33: mul r6.xyzw, v2.xyxy, cb0[7].xxyy
    r6.xyzw = ((v2.xyxy)*(source[7].xxyy)).xyzw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r4.w, r6.zwzz, t3.yzwx, s4, l(0.000000)
    r4.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 35: mad r4.w, r5.x, l(2.000000), r4.w
    r4.w = ((r5.xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(r4.wwww)).w;
    // 36: add r5.x, -r4.w, l(1.000000)
    r5.x = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 37: mul r5.x, r5.x, cb0[7].z
    r5.x = ((r5.xxxx)*(source[7].zzzz)).x;
    // 38: mad r5.x, r5.x, l(0.050000), l(-0.025000)
    r5.x = ((r5.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).x;
    // 39: mad r5.xy, r5.xxxx, r0.xyxx, r6.xyxx
    r5.xy = ((r5.xxxx)*(r0.xyxx)+(r6.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r5.xyxx, t4.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: mul r5.z, r3.z, r3.z
    r5.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 42: mul_sat r5.z, r5.z, r6.w
    r5.z = (saturate((r5.zzzz)*(r6.wwww))).z;
    // 43: mad r1.w, -r2.w, cb0[6].w, r1.w
    r1.w = ((-(r2.wwww))*(source[6].wwww)+(r1.wwww)).w;
    // 44: mad_sat r1.w, r5.z, r1.w, r3.w
    r1.w = (saturate((r5.zzzz)*(r1.wwww)+(r3.wwww))).w;
    // 45: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 46: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 47: dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 48: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 49: mul r4.xyz, r2.wwww, r3.xyzx
    r4.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 50: dp3 r2.w, r4.xyzx, r0.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 51: mul r4.xy, r2.wwww, r4.xyxx
    r4.xy = ((r2.wwww)*(r4.xyxx)).xy;
    // 52: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r0.xyxx
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r0.xyxx))).xy;
    // 53: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).w;
    // 54: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 55: div r5.zw, v6.xxxy, v6.wwww
    r5.zw = ((v6.xxxy)/(v6.wwww)).zw;
    // 56: mad r5.zw, r5.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r5.zw = ((r5.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 57: sample_indexable(texture2d)(float,float,float,float) r7.xyz, r5.zwzz, t7.xyzw, s0
    r7.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 58: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 59: else
    } else {
    // 60: mov r7.xyz, l(1.000000,1.000000,1.000000,0)
    r7.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 61: endif
    }
    // 62: mul r6.xyz, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r6.xyzx)*(source[3].xyzx)).xyz;
    // 63: dp3_sat r2.w, r0.xyzx, r3.xyzx
    r2.w = (saturate(dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx)).w;
    // 64: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 65: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 66: add r8.xyz, -cb0[0].xyzx, cb0[1].xyzx
    r8.xyz = ((-(source[0].xyzx))+(source[1].xyzx)).xyz;
    // 67: mad r8.xyz, r2.wwww, r8.xyzx, cb0[0].xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)+(source[0].xyzx)).xyz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s6, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 69: mul r9.xyz, r4.xyzx, cb0[2].xyzx
    r9.xyz = ((r4.xyzx)*(source[2].xyzx)).xyz;
    // 70: mad r4.xyz, r4.xyzx, cb0[2].xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(source[2].xyzx)+(r8.xyzx)).xyz;
    // 71: mad r4.xyz, r4.xyzx, r9.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)+(r4.xyzx)).xyz;
    // 72: add r2.w, r4.w, -cb0[6].y
    r2.w = ((r4.wwww)+(-(source[6].yyyy))).w;
    // 73: mul_sat r2.w, r2.w, cb0[6].w
    r2.w = (saturate((r2.wwww)*(source[6].wwww))).w;
    // 74: mad r4.xyz, r4.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), -r6.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(-(r6.xyzx))).xyz;
    // 75: mad r4.xyz, r2.wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 76: mad r6.xyz, r2.xyzx, cb0[4].xyzx, -r4.xyzx
    r6.xyz = ((r2.xyzx)*(source[4].xyzx)+(-(r4.xyzx))).xyz;
    // 77: mad r4.xyz, r1.wwww, r6.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 78: dp3 r1.x, r3.xyzx, r1.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 79: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 80: min r1.y, r1.x, l(1.000000)
    r1.y = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t5.xyzw, s7, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 82: mul r5.xyz, r5.xyzx, cb0[8].xxxx
    r5.xyz = ((r5.xyzx)*(source[8].xxxx)).xyz;
    // 83: mad r2.xyz, cb0[8].yyyy, r2.xyzx, -r5.xyzx
    r2.xyz = ((source[8].yyyy)*(r2.xyzx)+(-(r5.xyzx))).xyz;
    // 84: mad r2.xyz, r1.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 85: mad r0.xyz, v3.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((v3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 86: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 87: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 88: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 89: dp3 r0.x, r0.xyzx, r3.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 90: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 91: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 92: mul r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)*(source[8].zzzz)).x;
    // 93: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 94: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 95: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 96: mul r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 97: mad r0.xyz, r1.yyyy, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.yyyy)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 98: mul r1.xyz, r1.xxxx, cb2[3].xyzx
    r1.xyz = ((r1.xxxx)*(passValues[3].xyzx)).xyz;
    // 99: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 100: mul r0.xyz, r7.xyzx, r0.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)).xyz;
    // 101: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 102: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 103: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 104: ret
    return output;
}

// source.character.static-map-native-1411.v1 / source program 00a5b58b963b6046b662bbc992537056
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1411(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=g_SourceCharacterLightConstants[3];
    source[5]=g_SourceCharacterLightConstants[4];
    source[6]=g_SourceCharacterLightConstants[5];
    source[7]=g_SourceCharacterLightConstants[6];
    source[8]=g_SourceCharacterLightConstants[7];
    source[9]=float4(input.lightColor,1.f);
    source[15].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[15].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[15].yyyy)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: mul r0.xyzw, v8.yyyy, cb0[11].xyzw
    r0.xyzw = ((v8.yyyy)*(source[11].xyzw)).xyzw;
    // 4: mad r0.xyzw, cb0[10].xyzw, v8.xxxx, r0.xyzw
    r0.xyzw = ((source[10].xyzw)*(v8.xxxx)+(r0.xyzw)).xyzw;
    // 5: mad r0.xyzw, cb0[12].xyzw, v8.zzzz, r0.xyzw
    r0.xyzw = ((source[12].xyzw)*(v8.zzzz)+(r0.xyzw)).xyzw;
    // 6: mad r0.xyzw, cb0[13].xyzw, v8.wwww, r0.xyzw
    r0.xyzw = ((source[13].xyzw)*(v8.wwww)+(r0.xyzw)).xyzw;
    // 7: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 8: sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t3.xyzw, s3
    r1.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 9: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 10: mov r2.yz, cb0[14].wwzw
    r2.yz = (source[14].wwzw).yz;
    // 11: add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // 12: sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t3.yxzw, s3
    r1.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 13: sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t3.yzxw, s3
    r1.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 14: add r2.xy, r0.xyxx, cb0[14].zwzz
    r2.xy = ((r0.xyxx)+(source[14].zwzz)).xy;
    // 15: sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t3.yzwx, s3
    r1.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 16: lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // 17: and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 18: mul r0.xy, r0.xyxx, cb0[14].xyxx
    r0.xy = ((r0.xyxx)*(source[14].xyxx)).xy;
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
    // 25: mul r0.xyz, r0.xxxx, cb0[15].xxxx
    r0.xyz = ((r0.xxxx)*(source[15].xxxx)).xyz;
    // 26: else
    } else {
    // 27: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 28: endif
    }
    // 29: add r0.w, -v2.w, cb0[8].w
    r0.w = ((-(v2.wwww))+(source[8].wwww)).w;
    // 30: add r0.w, r0.w, cb0[8].z
    r0.w = ((r0.wwww)+(source[8].zzzz)).w;
    // 31: add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 32: mul r0.w, r0.w, cb0[1].x
    r0.w = ((r0.wwww)*(source[1].xxxx)).w;
    // 33: lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // 34: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 35: discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // 36: endif
    }
    // 37: dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 38: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 39: mul r1.xyz, r1.xxxx, v1.xyzx
    r1.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // 40: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 41: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 42: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 43: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 44: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 45: mul r3.xyz, r3.xyzx, v1.wwww
    r3.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 46: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 47: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 48: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 49: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 50: mul r4.xyz, r2.wwww, v5.xyzx
    r4.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mul r6.xyz, cb0[2].xyzx, cb0[6].wwww
    r6.xyz = ((source[2].xyzx)*(source[6].wwww)).xyz;
    // 53: mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 54: mul r8.xyz, cb0[3].xyzx, cb0[7].xxxx
    r8.xyz = ((source[3].xyzx)*(source[7].xxxx)).xyz;
    // 55: mul r9.xy, v4.xyxx, cb0[6].zzzz
    r9.xy = ((v4.xyxx)*(source[6].zzzz)).xy;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 57: mul r10.xyz, r8.xyzx, r9.xyzx
    r10.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 58: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 59: mad r8.xyz, -r8.xyzx, r9.xyzx, r2.wwww
    r8.xyz = ((-(r8.xyzx))*(r9.xyzx)+(r2.wwww)).xyz;
    // 60: mad r8.xyz, cb0[7].zzzz, r8.xyzx, r10.xyzx
    r8.xyz = ((source[7].zzzz)*(r8.xyzx)+(r10.xyzx)).xyz;
    // 61: max r2.w, cb0[5].y, l(0.000000)
    r2.w = (max(source[5].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 62: min r2.w, r2.w, l(0.990000)
    r2.w = (min(r2.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 63: add r3.w, -r2.w, l(1.000000)
    r3.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r3.w
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r3.wwww)).w;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 66: mad r10.xy, r10.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((r10.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 67: dp2 r4.w, r10.xyxx, r10.xyxx
    r4.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 68: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: max r4.w, r4.w, l(0.000000)
    r4.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 70: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 71: add r11.z, r4.w, l(0.000010)
    r11.z = ((r4.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 72: mul r10.xy, r10.xyxx, cb0[5].xxxx
    r10.xy = ((r10.xyxx)*(source[5].xxxx)).xy;
    // 73: mul r11.xy, r10.xyxx, v2.wwww
    r11.xy = ((r10.xyxx)*(v2.wwww)).xy;
    // 74: dp3 r4.w, r11.xyzx, r11.xyzx
    r4.w = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 75: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 76: div r10.xyz, r11.xyzx, r4.wwww
    r10.xyz = ((r11.xyzx)/(r4.wwww)).xyz;
    // 77: dp3 r2.x, r2.xyzx, r10.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 78: dp3 r2.y, r3.xyzx, r10.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 79: dp3 r2.z, r1.xyzx, r10.xyzx
    r2.z = (dot((r1.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 80: mul r1.xy, cb0[0].xyxx, cb0[5].zzzz
    r1.xy = ((source[0].xyxx)*(source[5].zzzz)).xy;
    // 81: max r1.xy, -r1.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = (max(-(r1.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 82: min r1.xy, r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = (min(r1.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 83: mov r1.z, l(1.000000)
    r1.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 84: dp3 r1.x, r2.xyzx, r1.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 85: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 86: mad r1.x, r1.x, l(0.500000), cb0[6].y
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].yyyy)).x;
    // 87: mul r1.y, r10.z, r10.z
    r1.y = ((r10.zzzz)*(r10.zzzz)).y;
    // 88: mul_sat r1.y, r1.y, r5.w
    r1.y = (saturate((r1.yyyy)*(r5.wwww))).y;
    // 89: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 90: mul r1.z, r9.w, r9.w
    r1.z = ((r9.wwww)*(r9.wwww)).z;
    // 91: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 92: mul r1.z, r1.y, r2.w
    r1.z = ((r1.yyyy)*(r2.wwww)).z;
    // 93: mad r1.x, r1.x, r1.z, r1.x
    r1.x = ((r1.xxxx)*(r1.zzzz)+(r1.xxxx)).x;
    // 94: add r1.z, -r2.w, r1.x
    r1.z = ((-(r2.wwww))+(r1.xxxx)).z;
    // 95: mul r2.x, r1.z, r3.w
    r2.x = ((r1.zzzz)*(r3.wwww)).x;
    // 96: mad r1.x, -r3.w, r1.z, r1.x
    r1.x = ((-(r3.wwww))*(r1.zzzz)+(r1.xxxx)).x;
    // 97: mad_sat r1.x, r1.y, r1.x, r2.x
    r1.x = (saturate((r1.yyyy)*(r1.xxxx)+(r2.xxxx))).x;
    // 98: mad r2.xyz, -r5.xyzx, r6.xyzx, r8.xyzx
    r2.xyz = ((-(r5.xyzx))*(r6.xyzx)+(r8.xyzx)).xyz;
    // 99: mad r2.xyz, r1.xxxx, r2.xyzx, r7.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 100: mul r1.y, r1.x, l(0.650000)
    r1.y = ((r1.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 101: add r3.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 102: mad r3.xyz, r1.yyyy, r3.xyzx, r10.xyzx
    r3.xyz = ((r1.yyyy)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 103: dp3 r1.y, r3.xyzx, r4.xyzx
    r1.y = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 104: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 105: mul r6.xyz, cb0[4].xyzx, cb0[7].wwww
    r6.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 106: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 107: mad r6.xyz, cb0[8].xxxx, r9.xyzx, -r5.xyzx
    r6.xyz = ((source[8].xxxx)*(r9.xyzx)+(-(r5.xyzx))).xyz;
    // 108: mad r5.xyz, r1.xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 109: mad r4.xyz, v7.xyzx, r1.wwww, r4.xyzx
    r4.xyz = ((v7.xyzx)*(r1.wwww)+(r4.xyzx)).xyz;
    // 110: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 111: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 112: div r4.xyz, r4.xyzx, r1.xxxx
    r4.xyz = ((r4.xyzx)/(r1.xxxx)).xyz;
    // 113: dp3 r1.x, r4.xyzx, r3.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 114: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 115: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 116: mul r1.x, r1.x, cb0[8].y
    r1.x = ((r1.xxxx)*(source[8].yyyy)).x;
    // 117: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 118: min r1.xz, r1.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r1.xz = (min(r1.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 119: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 120: mul r3.xyz, r5.xyzx, r1.xxxx
    r3.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 121: mad r1.xzw, r1.zzzz, r2.xxyz, r3.xxyz
    r1.xzw = ((r1.zzzz)*(r2.xxyz)+(r3.xxyz)).xzw;
    // 122: mul r2.xyz, r1.yyyy, cb2[3].xyzx
    r2.xyz = ((r1.yyyy)*(passValues[3].xyzx)).xyz;
    // 123: mad r1.xyz, r1.xzwx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xzwx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 124: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 125: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 126: mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // 127: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 128: ret
    return output;
}

// source.character.static-map-native-1412.v1 / source program 661789fe77b44340b861ac73e78db253
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1412(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=float4(input.lightColor,1.f);
    source[15].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[15].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[15].yyyy)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: mul r0.xyzw, v8.yyyy, cb0[11].xyzw
    r0.xyzw = ((v8.yyyy)*(source[11].xyzw)).xyzw;
    // 4: mad r0.xyzw, cb0[10].xyzw, v8.xxxx, r0.xyzw
    r0.xyzw = ((source[10].xyzw)*(v8.xxxx)+(r0.xyzw)).xyzw;
    // 5: mad r0.xyzw, cb0[12].xyzw, v8.zzzz, r0.xyzw
    r0.xyzw = ((source[12].xyzw)*(v8.zzzz)+(r0.xyzw)).xyzw;
    // 6: mad r0.xyzw, cb0[13].xyzw, v8.wwww, r0.xyzw
    r0.xyzw = ((source[13].xyzw)*(v8.wwww)+(r0.xyzw)).xyzw;
    // 7: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 8: sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t3.xyzw, s4
    r1.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 9: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 10: mov r2.yz, cb0[14].wwzw
    r2.yz = (source[14].wwzw).yz;
    // 11: add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // 12: sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t3.yxzw, s4
    r1.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 13: sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t3.yzxw, s4
    r1.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 14: add r2.xy, r0.xyxx, cb0[14].zwzz
    r2.xy = ((r0.xyxx)+(source[14].zwzz)).xy;
    // 15: sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t3.yzwx, s4
    r1.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 16: lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // 17: and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 18: mul r0.xy, r0.xyxx, cb0[14].xyxx
    r0.xy = ((r0.xyxx)*(source[14].xyxx)).xy;
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
    // 25: mul r0.xyz, r0.xxxx, cb0[15].xxxx
    r0.xyz = ((r0.xxxx)*(source[15].xxxx)).xyz;
    // 26: else
    } else {
    // 27: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 28: endif
    }
    // 29: add r0.w, -v2.w, cb0[8].z
    r0.w = ((-(v2.wwww))+(source[8].zzzz)).w;
    // 30: add r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)+(source[8].yyyy)).w;
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
    // 37: dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 38: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 39: mul r1.xyz, r1.xxxx, v1.xyzx
    r1.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // 40: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 41: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 42: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 43: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 44: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 45: mul r3.xyz, r3.xyzx, v1.wwww
    r3.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 46: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 47: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 48: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 49: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 50: mul r4.xyz, r2.wwww, v5.xyzx
    r4.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mul r6.xyz, cb0[2].xyzx, cb0[6].zzzz
    r6.xyz = ((source[2].xyzx)*(source[6].zzzz)).xyz;
    // 53: mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 54: mul r8.xyz, cb0[3].xyzx, cb0[6].wwww
    r8.xyz = ((source[3].xyzx)*(source[6].wwww)).xyz;
    // 55: mul r9.xy, v4.xyxx, cb0[6].yyyy
    r9.xy = ((v4.xyxx)*(source[6].yyyy)).xy;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 57: mul r10.xyz, r8.xyzx, r9.xyzx
    r10.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 58: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 59: mad r8.xyz, -r8.xyzx, r9.xyzx, r2.wwww
    r8.xyz = ((-(r8.xyzx))*(r9.xyzx)+(r2.wwww)).xyz;
    // 60: mad r8.xyz, cb0[7].yyyy, r8.xyzx, r10.xyzx
    r8.xyz = ((source[7].yyyy)*(r8.xyzx)+(r10.xyzx)).xyz;
    // 61: max r2.w, cb0[5].y, l(0.000000)
    r2.w = (max(source[5].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 62: min r2.w, r2.w, l(0.990000)
    r2.w = (min(r2.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 63: add r3.w, -r2.w, l(1.000000)
    r3.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r3.w
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r3.wwww)).w;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 66: mad r10.xy, r10.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((r10.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 67: dp2 r4.w, r10.xyxx, r10.xyxx
    r4.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 68: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: max r4.w, r4.w, l(0.000000)
    r4.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 70: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 71: add r11.z, r4.w, l(0.000010)
    r11.z = ((r4.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 72: mul r10.xy, r10.xyxx, cb0[5].xxxx
    r10.xy = ((r10.xyxx)*(source[5].xxxx)).xy;
    // 73: mul r11.xy, r10.xyxx, v2.wwww
    r11.xy = ((r10.xyxx)*(v2.wwww)).xy;
    // 74: dp3 r4.w, r11.xyzx, r11.xyzx
    r4.w = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 75: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 76: div r10.xyz, r11.xyzx, r4.wwww
    r10.xyz = ((r11.xyzx)/(r4.wwww)).xyz;
    // 77: dp3 r2.x, r2.xyzx, r10.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 78: dp3 r2.y, r3.xyzx, r10.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 79: dp3 r2.z, r1.xyzx, r10.xyzx
    r2.z = (dot((r1.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 80: max r1.xyz, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r1.xyz = (max(source[1].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 81: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 82: dp3 r1.x, r2.xyzx, r1.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 83: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 84: mad r1.x, r1.x, l(0.500000), cb0[6].x
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].xxxx)).x;
    // 85: mul r1.y, r10.z, r10.z
    r1.y = ((r10.zzzz)*(r10.zzzz)).y;
    // 86: mul_sat r1.y, r1.y, r5.w
    r1.y = (saturate((r1.yyyy)*(r5.wwww))).y;
    // 87: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 88: mul r1.z, r9.w, r9.w
    r1.z = ((r9.wwww)*(r9.wwww)).z;
    // 89: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 90: mul r1.z, r1.y, r2.w
    r1.z = ((r1.yyyy)*(r2.wwww)).z;
    // 91: mad r1.x, r1.x, r1.z, r1.x
    r1.x = ((r1.xxxx)*(r1.zzzz)+(r1.xxxx)).x;
    // 92: add r1.z, -r2.w, r1.x
    r1.z = ((-(r2.wwww))+(r1.xxxx)).z;
    // 93: mul r2.x, r1.z, r3.w
    r2.x = ((r1.zzzz)*(r3.wwww)).x;
    // 94: mad r1.x, -r3.w, r1.z, r1.x
    r1.x = ((-(r3.wwww))*(r1.zzzz)+(r1.xxxx)).x;
    // 95: mad_sat r1.x, r1.y, r1.x, r2.x
    r1.x = (saturate((r1.yyyy)*(r1.xxxx)+(r2.xxxx))).x;
    // 96: mad r2.xyz, -r5.xyzx, r6.xyzx, r8.xyzx
    r2.xyz = ((-(r5.xyzx))*(r6.xyzx)+(r8.xyzx)).xyz;
    // 97: mad r2.xyz, r1.xxxx, r2.xyzx, r7.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 98: mul r1.y, r1.x, l(0.650000)
    r1.y = ((r1.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 99: add r3.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 100: mad r3.xyz, r1.yyyy, r3.xyzx, r10.xyzx
    r3.xyz = ((r1.yyyy)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 101: dp3 r1.y, r3.xyzx, r4.xyzx
    r1.y = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 102: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 103: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 104: mul r6.xyz, cb0[4].xyzx, cb0[7].zzzz
    r6.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // 105: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 106: mad r6.xyz, cb0[7].wwww, r9.xyzx, -r5.xyzx
    r6.xyz = ((source[7].wwww)*(r9.xyzx)+(-(r5.xyzx))).xyz;
    // 107: mad r5.xyz, r1.xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 108: mad r4.xyz, v7.xyzx, r1.wwww, r4.xyzx
    r4.xyz = ((v7.xyzx)*(r1.wwww)+(r4.xyzx)).xyz;
    // 109: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 110: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 111: div r4.xyz, r4.xyzx, r1.xxxx
    r4.xyz = ((r4.xyzx)/(r1.xxxx)).xyz;
    // 112: dp3 r1.x, r4.xyzx, r3.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 113: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 114: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 115: mul r1.x, r1.x, cb0[8].x
    r1.x = ((r1.xxxx)*(source[8].xxxx)).x;
    // 116: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 117: min r1.xz, r1.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r1.xz = (min(r1.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 118: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 119: mul r3.xyz, r5.xyzx, r1.xxxx
    r3.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 120: mad r1.xzw, r1.zzzz, r2.xxyz, r3.xxyz
    r1.xzw = ((r1.zzzz)*(r2.xxyz)+(r3.xxyz)).xzw;
    // 121: mul r2.xyz, r1.yyyy, cb2[3].xyzx
    r2.xyz = ((r1.yyyy)*(passValues[3].xyzx)).xyz;
    // 122: mad r1.xyz, r1.xzwx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xzwx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 123: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 124: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 125: mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // 126: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 127: ret
    return output;
}

// source.character.static-map-native-1413.v1 / source program f40e57508fee82409fcc8d442b60e6a0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1413(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterLightConstants[1];
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[16].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[16].yyyy)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: mul r0.xyzw, v8.yyyy, cb0[12].xyzw
    r0.xyzw = ((v8.yyyy)*(source[12].xyzw)).xyzw;
    // 4: mad r0.xyzw, cb0[11].xyzw, v8.xxxx, r0.xyzw
    r0.xyzw = ((source[11].xyzw)*(v8.xxxx)+(r0.xyzw)).xyzw;
    // 5: mad r0.xyzw, cb0[13].xyzw, v8.zzzz, r0.xyzw
    r0.xyzw = ((source[13].xyzw)*(v8.zzzz)+(r0.xyzw)).xyzw;
    // 6: mad r0.xyzw, cb0[14].xyzw, v8.wwww, r0.xyzw
    r0.xyzw = ((source[14].xyzw)*(v8.wwww)+(r0.xyzw)).xyzw;
    // 7: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 8: sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t4.xyzw, s5
    r1.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 9: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 10: mov r2.yz, cb0[15].wwzw
    r2.yz = (source[15].wwzw).yz;
    // 11: add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // 12: sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t4.yxzw, s5
    r1.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 13: sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t4.yzxw, s5
    r1.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 14: add r2.xy, r0.xyxx, cb0[15].zwzz
    r2.xy = ((r0.xyxx)+(source[15].zwzz)).xy;
    // 15: sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t4.yzwx, s5
    r1.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 16: lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // 17: and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 18: mul r0.xy, r0.xyxx, cb0[15].xyxx
    r0.xy = ((r0.xyxx)*(source[15].xyxx)).xy;
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
    // 25: mul r0.xyz, r0.xxxx, cb0[16].xxxx
    r0.xyz = ((r0.xxxx)*(source[16].xxxx)).xyz;
    // 26: else
    } else {
    // 27: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 28: endif
    }
    // 29: add r0.w, -v2.w, cb0[9].w
    r0.w = ((-(v2.wwww))+(source[9].wwww)).w;
    // 30: add r0.w, r0.w, cb0[9].z
    r0.w = ((r0.wwww)+(source[9].zzzz)).w;
    // 31: add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 32: mul r0.w, r0.w, cb0[1].x
    r0.w = ((r0.wwww)*(source[1].xxxx)).w;
    // 33: lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // 34: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 35: discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // 36: endif
    }
    // 37: dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 38: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 39: mul r1.xyz, r1.xxxx, v1.xyzx
    r1.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // 40: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 41: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 42: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 43: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 44: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 45: mul r3.xyz, r3.xyzx, v1.wwww
    r3.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 46: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 47: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 48: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 49: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 50: mul r4.xyz, r2.wwww, v5.xyzx
    r4.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: add r6.xyz, -r5.xyzx, r2.wwww
    r6.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 54: mad r5.xyz, cb0[7].zzzz, r6.xyzx, r5.xyzx
    r5.xyz = ((source[7].zzzz)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 55: mul r6.xyz, cb0[2].xyzx, cb0[7].wwww
    r6.xyz = ((source[2].xyzx)*(source[7].wwww)).xyz;
    // 56: mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 57: mul r8.xyz, cb0[3].xyzx, cb0[8].xxxx
    r8.xyz = ((source[3].xyzx)*(source[8].xxxx)).xyz;
    // 58: mul r9.xy, v4.xyxx, cb0[7].xxxx
    r9.xy = ((v4.xyxx)*(source[7].xxxx)).xy;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t3.xyzw, s3, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 60: mul r10.xyz, r8.xyzx, r9.xyzx
    r10.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 61: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: mad r8.xyz, -r8.xyzx, r9.xyzx, r2.wwww
    r8.xyz = ((-(r8.xyzx))*(r9.xyzx)+(r2.wwww)).xyz;
    // 63: mad r8.xyz, cb0[8].zzzz, r8.xyzx, r10.xyzx
    r8.xyz = ((source[8].zzzz)*(r8.xyzx)+(r10.xyzx)).xyz;
    // 64: max r2.w, cb0[5].w, l(0.000000)
    r2.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 65: min r2.w, r2.w, l(0.990000)
    r2.w = (min(r2.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 66: add r3.w, -r2.w, l(1.000000)
    r3.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 67: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r3.w
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r3.wwww)).w;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 69: mad r10.xy, r10.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((r10.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 70: dp2 r4.w, r10.xyxx, r10.xyxx
    r4.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 71: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: max r4.w, r4.w, l(0.000000)
    r4.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 73: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 74: add r11.z, r4.w, l(0.000010)
    r11.z = ((r4.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 75: mul r10.zw, v4.xxxy, cb0[5].yyyy
    r10.zw = ((v4.xxxy)*(source[5].yyyy)).zw;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r10.zw, r10.zwzz, t1.zwxy, s1, l(0.000000)
    r10.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r10.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 77: mad r10.zw, r10.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r10.zw = ((r10.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 78: mul r10.zw, r10.zzzw, cb0[5].zzzz
    r10.zw = ((r10.zzzw)*(source[5].zzzz)).zw;
    // 79: mad r10.xy, cb0[5].xxxx, r10.xyxx, r10.zwzz
    r10.xy = ((source[5].xxxx)*(r10.xyxx)+(r10.zwzz)).xy;
    // 80: mul r11.xy, r10.xyxx, v2.wwww
    r11.xy = ((r10.xyxx)*(v2.wwww)).xy;
    // 81: dp3 r4.w, r11.xyzx, r11.xyzx
    r4.w = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 82: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 83: div r10.xyz, r11.xyzx, r4.wwww
    r10.xyz = ((r11.xyzx)/(r4.wwww)).xyz;
    // 84: dp3 r2.x, r2.xyzx, r10.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 85: dp3 r2.y, r3.xyzx, r10.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 86: dp3 r2.z, r1.xyzx, r10.xyzx
    r2.z = (dot((r1.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 87: mul r1.xy, cb0[0].xyxx, cb0[6].xxxx
    r1.xy = ((source[0].xyxx)*(source[6].xxxx)).xy;
    // 88: max r1.xy, -r1.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = (max(-(r1.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 89: min r1.xy, r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = (min(r1.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 90: mov r1.z, l(1.000000)
    r1.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 91: dp3 r1.x, r2.xyzx, r1.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 92: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 93: mad r1.x, r1.x, l(0.500000), cb0[6].w
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].wwww)).x;
    // 94: mul r1.y, r10.z, r10.z
    r1.y = ((r10.zzzz)*(r10.zzzz)).y;
    // 95: mul_sat r1.y, r1.y, r5.w
    r1.y = (saturate((r1.yyyy)*(r5.wwww))).y;
    // 96: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 97: mul r1.z, r9.w, r9.w
    r1.z = ((r9.wwww)*(r9.wwww)).z;
    // 98: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 99: mul r1.z, r1.y, r2.w
    r1.z = ((r1.yyyy)*(r2.wwww)).z;
    // 100: mad r1.x, r1.x, r1.z, r1.x
    r1.x = ((r1.xxxx)*(r1.zzzz)+(r1.xxxx)).x;
    // 101: add r1.z, -r2.w, r1.x
    r1.z = ((-(r2.wwww))+(r1.xxxx)).z;
    // 102: mul r2.x, r1.z, r3.w
    r2.x = ((r1.zzzz)*(r3.wwww)).x;
    // 103: mad r1.x, -r3.w, r1.z, r1.x
    r1.x = ((-(r3.wwww))*(r1.zzzz)+(r1.xxxx)).x;
    // 104: mad_sat r1.x, r1.y, r1.x, r2.x
    r1.x = (saturate((r1.yyyy)*(r1.xxxx)+(r2.xxxx))).x;
    // 105: mad r2.xyz, -r5.xyzx, r6.xyzx, r8.xyzx
    r2.xyz = ((-(r5.xyzx))*(r6.xyzx)+(r8.xyzx)).xyz;
    // 106: mad r2.xyz, r1.xxxx, r2.xyzx, r7.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 107: mul r1.y, r1.x, l(0.650000)
    r1.y = ((r1.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 108: add r3.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 109: mad r3.xyz, r1.yyyy, r3.xyzx, r10.xyzx
    r3.xyz = ((r1.yyyy)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 110: dp3 r1.y, r3.xyzx, r4.xyzx
    r1.y = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 111: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 112: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 113: mul r6.xyz, cb0[4].xyzx, cb0[8].wwww
    r6.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 114: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 115: mad r6.xyz, cb0[9].xxxx, r9.xyzx, -r5.xyzx
    r6.xyz = ((source[9].xxxx)*(r9.xyzx)+(-(r5.xyzx))).xyz;
    // 116: mad r5.xyz, r1.xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 117: mad r4.xyz, v7.xyzx, r1.wwww, r4.xyzx
    r4.xyz = ((v7.xyzx)*(r1.wwww)+(r4.xyzx)).xyz;
    // 118: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 119: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 120: div r4.xyz, r4.xyzx, r1.xxxx
    r4.xyz = ((r4.xyzx)/(r1.xxxx)).xyz;
    // 121: dp3 r1.x, r4.xyzx, r3.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 122: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 123: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 124: mul r1.x, r1.x, cb0[9].y
    r1.x = ((r1.xxxx)*(source[9].yyyy)).x;
    // 125: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 126: min r1.xz, r1.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r1.xz = (min(r1.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 127: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 128: mul r3.xyz, r5.xyzx, r1.xxxx
    r3.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 129: mad r1.xzw, r1.zzzz, r2.xxyz, r3.xxyz
    r1.xzw = ((r1.zzzz)*(r2.xxyz)+(r3.xxyz)).xzw;
    // 130: mul r2.xyz, r1.yyyy, cb2[3].xyzx
    r2.xyz = ((r1.yyyy)*(passValues[3].xyzx)).xyz;
    // 131: mad r1.xyz, r1.xzwx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xzwx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 132: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 133: mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 134: mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // 135: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 136: ret
    return output;
}

