SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1500(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1501(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1502(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1503(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

// source.character.static-map-native-1504.v1 / source program b6316f7b8f73b54dab1e0596c80e3a10
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1504(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 6: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[4].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[4].xxxx)) * 0xffffffffu)).x;
    // 7: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 8: div r1.xy, v6.xyxx, v6.wwww
    r1.xy = ((v6.xyxx)/(v6.wwww)).xy;
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
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 16: mul r3.xyz, cb0[0].xyzx, cb0[2].yyyy
    r3.xyz = ((source[0].xyzx)*(source[2].yyyy)).xyz;
    // 17: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    // 26: mul r5.xy, r4.xyxx, v0.wwww
    r5.xy = ((r4.xyxx)*(v0.wwww)).xy;
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
    // 35: mad r0.xyz, v5.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v5.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
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

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1505(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1506(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1507(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1508(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

// source.character.static-map-native-1509.v1 / source program f0e5b8d4a6327f4d92657445020edd65
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1509(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[4];
    source[4]=g_SourceCharacterLightConstants[5];
    source[5]=g_SourceCharacterLightConstants[6];
    source[6]=g_SourceCharacterLightConstants[7];
    source[7]=g_SourceCharacterLightConstants[11];
    source[8]=g_SourceCharacterLightConstants[12];
    source[9]=g_SourceCharacterLightConstants[13];
    source[10]=g_SourceCharacterLightConstants[15];
    source[11]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xy, -v2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v2.xyxx
    r0.xy = ((r0.xyxx)*(v2.xyxx)).xy;
    // 3: lt r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 4: add r1.x, -cb0[0].x, l(0.001000)
    r1.x = ((-(source[0].xxxx))+(float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 5: lt r0.z, r1.x, l(0.000000)
    r0.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 6: add r1.x, cb0[0].y, l(0.001000)
    r1.x = ((source[0].yyyy)+(float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 7: lt r0.w, r1.x, l(0.000000)
    r0.w = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 8: or r0.xy, r0.zwzz, r0.xyxx
    r0.xy = (asfloat(asuint(r0.zwzz) | asuint(r0.xyxx))).xy;
    // 9: or r0.x, r0.y, r0.x
    r0.x = (asfloat(asuint(r0.yyyy) | asuint(r0.xxxx))).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t0.zxyw, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).x;
    // 12: add r0.x, r0.x, -cb0[7].w
    r0.x = ((r0.xxxx)+(-(source[7].wwww))).x;
    // 13: mul r0.x, r0.x, cb0[8].x
    r0.x = ((r0.xxxx)*(source[8].xxxx)).x;
    // 14: mad r0.x, r0.x, l(0.050000), l(-0.025000)
    r0.x = ((r0.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).x;
    // 15: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 16: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 17: mul r0.yz, r0.yyyy, v5.xxyx
    r0.yz = ((r0.yyyy)*(v5.xxyx)).yz;
    // 18: mul r0.xw, r0.yyyz, r0.xxxx
    r0.xw = ((r0.yyyz)*(r0.xxxx)).xw;
    // 19: mad r1.xy, v2.xyxx, cb0[2].xyxx, r0.xwxx
    r1.xy = ((v2.xyxx)*(source[2].xyxx)+(r0.xwxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 21: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 23: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 25: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 26: add r1.z, r1.w, l(0.000010)
    r1.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 28: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 29: div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // 30: mad r2.xy, cb0[8].zzzz, v2.xyxx, r0.xwxx
    r2.xy = ((source[8].zzzz)*(v2.xyxx)+(r0.xwxx)).xy;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mad r3.xy, r1.xyxx, cb0[3].xyxx, r2.xyxx
    r3.xy = ((r1.xyxx)*(source[3].xyxx)+(r2.xyxx)).xy;
    // 33: mul r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)*(source[3].xyzx)).xyz;
    // 34: mul r3.xy, r3.xyxx, cb0[8].wwww
    r3.xy = ((r3.xyxx)*(source[8].wwww)).xy;
    // 35: mad r0.yz, -r0.yyzy, cb0[1].xxyx, r3.xxyx
    r0.yz = ((-(r0.yyzy))*(source[1].xxyx)+(r3.xxyx)).yz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.yzyy, t3.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 37: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 38: add r4.xyz, -r3.xyzx, r0.yyyy
    r4.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 39: mad r3.xyz, cb0[9].xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((source[9].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 40: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 41: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 42: mul r3.xyz, r3.xyzx, cb0[9].yyyy
    r3.xyz = ((r3.xyzx)*(source[9].yyyy)).xyz;
    // 43: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 44: dp3 r0.y, v3.xyzx, v3.xyzx
    r0.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // 45: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 46: mul r4.xyz, r0.yyyy, v3.xyzx
    r4.xyz = ((r0.yyyy)*(v3.xyzx)).xyz;
    // 47: dp3 r0.y, r4.xyzx, r1.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 48: max r0.z, r4.z, l(0.000000)
    r0.z = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 49: mul r1.xyz, r0.zzzz, cb2[3].xyzx
    r1.xyz = ((r0.zzzz)*(passValues[3].xyzx)).xyz;
    // 50: max r0.y, r0.y, l(0.150000)
    r0.y = (max(r0.yyyy,float4(0.150000,0.150000,0.150000,0.150000))).y;
    // 51: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 52: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 53: mul r0.z, r0.z, r0.y
    r0.z = ((r0.zzzz)*(r0.yyyy)).z;
    // 54: mul r3.xyz, r3.xyzx, r0.zzzz
    r3.xyz = ((r3.xyzx)*(r0.zzzz)).xyz;
    // 55: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 56: add r4.xyz, -r2.xyzx, r0.zzzz
    r4.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 57: mad r2.xyz, cb0[9].zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((source[9].zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 58: mul r0.z, r2.w, cb0[10].x
    r0.z = ((r2.wwww)*(source[10].xxxx)).z;
    // 59: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 60: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 61: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 62: add r4.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r4.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 63: mad r0.xy, r4.xyxx, cb0[6].xyxx, r0.xwxx
    r0.xy = ((r4.xyxx)*(source[6].xyxx)+(r0.xwxx)).xy;
    // 64: add r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.yxzw, s4, l(0.000000)
    r0.x = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).x;
    // 66: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 67: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 68: mul r0.xyw, r2.xyxz, r0.xxxx
    r0.xyw = ((r2.xyxz)*(r0.xxxx)).xyw;
    // 69: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 70: mad r0.xyw, r3.xyxz, r2.xyxz, r0.xyxw
    r0.xyw = ((r3.xyxz)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 71: mul r0.xyw, r0.xyxw, v0.xyxz
    r0.xyw = ((r0.xyxw)*(v0.xyxz)).xyw;
    // 72: mul r0.xyw, r0.xyxw, cb2[3].wwww
    r0.xyw = ((r0.xyxw)*(passValues[3].wwww)).xyw;
    // 73: mad r0.xyw, r0.xyxw, l(0.850000, 0.850000, 0.000000, 0.850000), r1.xyxz
    r0.xyw = ((r0.xyxw)*(float4(0.850000,0.850000,0.000000,0.850000))+(r1.xyxz)).xyw;
    // 74: mul o0.xyz, r0.xywx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xywx)*(source[11].xyzx)).xyz;
    // 75: log r0.x, |r0.z|
    r0.x = (log2(abs(r0.zzzz))).x;
    // 76: lt r0.y, |r0.z|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 77: mul r0.x, r0.x, cb0[10].y
    r0.x = ((r0.xxxx)*(source[10].yyyy)).x;
    // 78: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 79: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 80: mul r0.yz, v2.xxyx, cb0[10].zzzz
    r0.yz = ((v2.xxyx)*(source[10].zzzz)).yz;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t5.yxzw, s5, l(0.000000)
    r0.y = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 82: mov_sat r0.z, v0.w
    r0.z = (saturate(v0.wwww)).z;
    // 83: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 84: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 85: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 86: mul_sat r0.x, r0.x, v0.w
    r0.x = (saturate((r0.xxxx)*(v0.wwww))).x;
    // 87: mul o0.w, r0.x, cb0[0].z
    output.targets[0].w = ((r0.xxxx)*(source[0].zzzz)).w;
    // 88: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 89: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1510(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1511(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

// source.character.static-map-native-1512.v1 / source program bc5908842868314885c34e7865e4ef41
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1512(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 1: dp3 r0.x, v4.xyzx, v4.xyzx
    r0.x = (dot((v4.xyzx).xyz,(v4.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v4.xyzx
    r0.xyz = ((r0.xxxx)*(v4.xyzx)).xyz;
    // 4: dp3 r0.w, v2.xyzx, v2.xyzx
    r0.w = (dot((v2.xyzx).xyz,(v2.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v2.xyzx
    r1.xyz = ((r0.wwww)*(v2.xyzx)).xyz;
    // 7: mad r0.xyz, r0.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r0.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 8: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[3].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[3].xxxx)) * 0xffffffffu)).w;
    // 9: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 10: div r2.xy, v5.xyxx, v5.wwww
    r2.xy = ((v5.xyxx)/(v5.wwww)).xy;
    // 11: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 12: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t0.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 13: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 14: else
    } else {
    // 15: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 16: endif
    }
    // 17: add r3.xyz, -cb0[0].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[0].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 18: mul r3.xyz, r3.xyzx, cb0[1].xyzx
    r3.xyz = ((r3.xyzx)*(source[1].xyzx)).xyz;
    // 19: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 20: mov_sat r0.w, r1.z
    r0.w = (saturate(r1.zzzz)).w;
    // 21: lt r1.w, r0.w, l(0.000001)
    r1.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 22: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 23: dp3_sat r0.x, r0.xyzx, r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 24: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 26: mul r0.x, r0.x, l(15.000000)
    r0.x = ((r0.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 27: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 28: mul r1.xyz, r0.xxxx, cb2[4].xyzx
    r1.xyz = ((r0.xxxx)*(passValues[4].xyzx)).xyz;
    // 29: movc r0.xyz, r0.yyyy, l(0,0,0,0), r1.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 30: mad r0.xyz, r3.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 31: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 32: mul o0.xyz, r0.xyzx, cb0[2].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[2].xyzx)).xyz;
    // 33: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 34: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 35: ret
    return output;
}

// source.character.static-map-native-1513.v1 / source program d3542163f308a34e94adac8baf7fd59d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1513(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 4: mad r0.xyz, r0.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r0.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 5: dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r1.xyz, r0.wwww, v3.zxyz
    r1.xyz = ((r0.wwww)*(v3.zxyz)).xyz;
    // 8: dp3_sat r0.x, r0.zxyz, r1.xyzx
    r0.x = (saturate(dot((r0.zxyz).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 9: mov_sat r1.x, r1.x
    r1.x = (saturate(r1.xxxx)).x;
    // 10: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 11: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: mul r0.y, r0.y, l(15.000000)
    r0.y = ((r0.yyyy)*(float4(15.000000,15.000000,15.000000,15.000000))).y;
    // 13: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 14: mul r0.yzw, r0.yyyy, cb2[4].xxyz
    r0.yzw = ((r0.yyyy)*(passValues[4].xxyz)).yzw;
    // 15: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 16: add r1.yzw, -cb0[1].xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r1.yzw = ((-(source[1].xxyz))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 17: mul r1.yzw, r1.yyzw, cb0[2].xxyz
    r1.yzw = ((r1.yyzw)*(source[2].xxyz)).yzw;
    // 18: mad r1.yzw, r1.yyzw, cb2[3].wwww, cb2[3].xxyz
    r1.yzw = ((r1.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 19: lt r0.w, r1.x, l(0.000001)
    r0.w = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 20: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 21: mad r0.xyz, r1.yzwy, r0.wwww, r0.xyzx
    r0.xyz = ((r1.yzwy)*(r0.wwww)+(r0.xyzx)).xyz;
    // 22: mul o0.xyz, r0.xyzx, cb0[4].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[4].xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 24: mul r0.x, r0.x, cb0[3].x
    r0.x = ((r0.xxxx)*(source[3].xxxx)).x;
    // 25: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 26: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 27: ret
    return output;
}

// source.character.static-map-native-1514.v1 / source program 840bb87cbac3b74698627130a47d2f37
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1514(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[1];
    source[1]=g_SourceCharacterLightConstants[2];
    source[2]=g_SourceCharacterLightConstants[3];
    source[3]=g_SourceCharacterLightConstants[4];
    source[4]=g_SourceCharacterLightConstants[5];
    source[5]=g_SourceCharacterLightConstants[6];
    source[6]=float4(input.lightColor,1.f);
    source[7].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
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
    // 6: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[7].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[7].xxxx)) * 0xffffffffu)).x;
    // 7: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 8: div r1.xy, v6.xyxx, v6.wwww
    r1.xy = ((v6.xyxx)/(v6.wwww)).xy;
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
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 16: mul r3.xyz, cb0[0].xyzx, cb0[3].wwww
    r3.xyz = ((source[0].xyzx)*(source[3].wwww)).xyz;
    // 17: mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 18: mul r5.xyz, cb0[1].xyzx, cb0[4].xxxx
    r5.xyz = ((source[1].xyzx)*(source[4].xxxx)).xyz;
    // 19: mul r6.xy, v2.xyxx, cb0[3].zzzz
    r6.xy = ((v2.xyxx)*(source[3].zzzz)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 21: mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 22: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 23: mad r5.xyz, -r5.xyzx, r6.xyzx, r1.wwww
    r5.xyz = ((-(r5.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 24: mad r5.xyz, cb0[4].zzzz, r5.xyzx, r7.xyzx
    r5.xyz = ((source[4].zzzz)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 25: max r1.w, cb0[3].y, l(0.000000)
    r1.w = (max(source[3].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 26: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 27: add r3.w, -r1.w, l(1.000000)
    r3.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r3.w
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r3.wwww)).w;
    // 29: add r4.w, -v0.x, l(1.000000)
    r4.w = ((-(v0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: dp2 r5.w, r7.xyxx, r7.xyxx
    r5.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 33: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: max r5.w, r5.w, l(0.000000)
    r5.w = (max(r5.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 35: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 36: add r8.z, r5.w, l(0.000010)
    r8.z = ((r5.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 37: mul r7.xy, r7.xyxx, cb0[3].xxxx
    r7.xy = ((r7.xyxx)*(source[3].xxxx)).xy;
    // 38: mul r8.xy, r7.xyxx, v0.wwww
    r8.xy = ((r7.xyxx)*(v0.wwww)).xy;
    // 39: dp3 r5.w, r8.xyzx, r8.xyzx
    r5.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 40: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 41: div r7.xyz, r8.xyzx, r5.wwww
    r7.xyz = ((r8.xyzx)/(r5.wwww)).xyz;
    // 42: mul r5.w, r7.z, r7.z
    r5.w = ((r7.zzzz)*(r7.zzzz)).w;
    // 43: mul_sat r2.w, r2.w, r5.w
    r2.w = (saturate((r2.wwww)*(r5.wwww))).w;
    // 44: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: mul r5.w, r6.w, r6.w
    r5.w = ((r6.wwww)*(r6.wwww)).w;
    // 46: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 47: mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // 48: mad r4.w, r4.w, r5.w, r4.w
    r4.w = ((r4.wwww)*(r5.wwww)+(r4.wwww)).w;
    // 49: add r1.w, -r1.w, r4.w
    r1.w = ((-(r1.wwww))+(r4.wwww)).w;
    // 50: mul r5.w, r1.w, r3.w
    r5.w = ((r1.wwww)*(r3.wwww)).w;
    // 51: mad r1.w, -r3.w, r1.w, r4.w
    r1.w = ((-(r3.wwww))*(r1.wwww)+(r4.wwww)).w;
    // 52: mad_sat r1.w, r2.w, r1.w, r5.w
    r1.w = (saturate((r2.wwww)*(r1.wwww)+(r5.wwww))).w;
    // 53: mad r3.xyz, -r2.xyzx, r3.xyzx, r5.xyzx
    r3.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r5.xyzx)).xyz;
    // 54: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 55: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 56: add r4.xyz, -r7.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r7.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 57: mad r4.xyz, r2.wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 58: dp3 r2.w, r4.xyzx, r0.yzwy
    r2.w = (dot((r4.xyzx).xyz,(r0.yzwy).xyz).xxxx).w;
    // 59: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: mul r4.xyz, cb0[2].xyzx, cb0[4].wwww
    r4.xyz = ((source[2].xyzx)*(source[4].wwww)).xyz;
    // 62: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 63: dp3 r4.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 64: add r4.xyz, -r6.xyzx, r4.xxxx
    r4.xyz = ((-(r6.xyzx))+(r4.xxxx)).xyz;
    // 65: mad r4.xyz, cb0[4].zzzz, r4.xyzx, r6.xyzx
    r4.xyz = ((source[4].zzzz)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 66: mad r4.xyz, cb0[5].xxxx, r4.xyzx, -r2.xyzx
    r4.xyz = ((source[5].xxxx)*(r4.xyzx)+(-(r2.xyzx))).xyz;
    // 67: mad r2.xyz, r1.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 68: mad r0.xyz, v5.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v5.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 69: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 70: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 71: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 72: dp3 r0.x, r0.xyzx, r7.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 73: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 74: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 75: mul r0.x, r0.x, cb0[5].y
    r0.x = ((r0.xxxx)*(source[5].yyyy)).x;
    // 76: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 77: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 78: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 79: mul r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 80: mad r0.xyz, r3.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 81: mul r2.xyz, r2.wwww, cb2[3].xyzx
    r2.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // 82: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 83: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 84: mul o0.xyz, r0.xyzx, cb0[6].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[6].xyzx)).xyz;
    // 85: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 86: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 87: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1515(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1516(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

// source.character.static-map-native-1517.v1 / source program d0ba76144cdce2429aa5d85af09b76d8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1517(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[4];
    source[2]=g_SourceCharacterLightConstants[5];
    source[3]=g_SourceCharacterLightConstants[8];
    source[3].x=((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterLightConstants[10])*float4(0,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterLightConstants[10])*float4(0,0,0,0)))))).x;
    source[3].y=((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterLightConstants[10])*float4(1,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterLightConstants[10])*float4(1,0,0,0)))))).x;
    source[4]=g_SourceCharacterLightConstants[9];
    source[5]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, -v2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v2.xyxx
    r0.xy = ((r0.xyxx)*(v2.xyxx)).xy;
    // 3: lt r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 4: add r1.x, -cb0[0].x, l(0.001000)
    r1.x = ((-(source[0].xxxx))+(float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 5: lt r0.z, r1.x, l(0.000000)
    r0.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 6: add r1.x, cb0[0].y, l(0.001000)
    r1.x = ((source[0].yyyy)+(float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 7: lt r0.w, r1.x, l(0.000000)
    r0.w = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 8: or r0.xy, r0.zwzz, r0.xyxx
    r0.xy = (asfloat(asuint(r0.zwzz) | asuint(r0.xyxx))).xy;
    // 9: or r0.x, r0.y, r0.x
    r0.x = (asfloat(asuint(r0.yyyy) | asuint(r0.xxxx))).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t0.zxyw, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).x;
    // 12: add r0.x, r0.x, -cb0[2].x
    r0.x = ((r0.xxxx)+(-(source[2].xxxx))).x;
    // 13: mul r0.x, r0.x, cb0[2].y
    r0.x = ((r0.xxxx)*(source[2].yyyy)).x;
    // 14: mad r0.x, r0.x, l(0.050000), l(-0.025000)
    r0.x = ((r0.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).x;
    // 15: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 16: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 17: mul r0.yz, r0.yyyy, v5.xxyx
    r0.yz = ((r0.yyyy)*(v5.xxyx)).yz;
    // 18: mad r0.xy, r0.xxxx, r0.yzyy, v2.xyxx
    r0.xy = ((r0.xxxx)*(r0.yzyy)+(v2.xyxx)).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t1.zwxy, s2, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 21: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 23: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 25: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 26: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 28: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 29: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 30: dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 31: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 32: mul r2.xyz, r0.wwww, v3.xyzx
    r2.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // 33: dp3 r0.x, r2.xyzx, r0.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 34: max r0.y, r2.z, l(0.000000)
    r0.y = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 35: mul r0.yzw, r0.yyyy, cb2[3].xxyz
    r0.yzw = ((r0.yyyy)*(passValues[3].xxyz)).yzw;
    // 36: max r0.x, r0.x, l(0.200000)
    r0.x = (max(r0.xxxx,float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 37: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 38: dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 39: add r2.xyz, -r1.xyzx, r2.xxxx
    r2.xyz = ((-(r1.xyzx))+(r2.xxxx)).xyz;
    // 40: mad r1.xyz, cb0[4].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[4].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 41: mul_sat r1.w, r1.w, cb0[3].z
    r1.w = (saturate((r1.wwww)*(source[3].zzzz))).w;
    // 42: mad_sat r1.w, r1.w, v0.w, cb0[4].z
    r1.w = (saturate((r1.wwww)*(v0.wwww)+(source[4].zzzz))).w;
    // 43: mul o0.w, r1.w, cb0[0].z
    output.targets[0].w = ((r1.wwww)*(source[0].zzzz)).w;
    // 44: mul r2.xyz, cb0[1].xyzx, cb0[1].wwww
    r2.xyz = ((source[1].xyzx)*(source[1].wwww)).xyz;
    // 45: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 46: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 47: mad r0.xyz, r1.xyzx, cb2[3].wwww, r0.yzwy
    r0.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r0.yzwy)).xyz;
    // 48: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 49: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 50: ret
    return output;
}

// source.character.static-map-native-1518.v1 / source program acc28065f1b00341a5874feb06c9082f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1518(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=g_SourceCharacterLightConstants[3];
    source[5]=g_SourceCharacterLightConstants[4];
    source[6]=g_SourceCharacterLightConstants[5];
    source[7]=g_SourceCharacterLightConstants[6];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[9]=g_SourceCharacterLightConstants[12];
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[12]=g_SourceCharacterLightConstants[15];
    source[13]=g_SourceCharacterLightConstants[16];
    source[14]=g_SourceCharacterLightConstants[17];
    source[15]=g_SourceCharacterLightConstants[18];
    source[16]=g_SourceCharacterLightConstants[19];
    source[16].y=(g_SourceCharacterTime.xxxx).x;
    source[16].w=((g_SourceCharacterLightConstants[24]*g_SourceCharacterTime.xxxx)).x;
    source[17]=g_SourceCharacterLightConstants[22];
    source[18]=g_SourceCharacterLightConstants[23];
    source[19]=float4(input.lightColor,1.f);
    source[20].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[20].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[20].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t7.xyzw, s0
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 31: mov_sat r8.x, r8.x
    r8.x = (saturate(r8.xxxx)).x;
    // 32: add r1.w, r8.x, l(-0.333300)
    r1.w = ((r8.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 33: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 34: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 36: mad r9.xy, r9.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((r9.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: mul r10.xy, r9.xyxx, cb0[12].xxxx
    r10.xy = ((r9.xyxx)*(source[12].xxxx)).xy;
    // 38: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 39: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 42: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 43: add r9.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 44: mad r9.xyz, cb0[12].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[12].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 45: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 46: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 47: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 48: dp3 r11.x, r1.xyzx, r9.xyzx
    r11.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 49: dp3 r11.y, r2.xyzx, r9.xyzx
    r11.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 50: dp3 r11.z, r0.xyzx, r9.xyzx
    r11.z = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 51: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 52: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 53: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 54: dp3 r0.x, r11.xyzx, r1.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 55: mul r0.xyz, r11.xyzx, r0.xxxx
    r0.xyz = ((r11.xyzx)*(r0.xxxx)).xyz;
    // 56: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 57: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 58: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 59: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 60: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 61: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 62: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 67: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 68: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 69: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 71: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 72: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 73: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 74: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 75: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 76: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 77: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 78: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 79: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 80: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 81: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 82: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 83: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 84: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 85: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 86: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 87: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 88: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t3.xywz, s4, r1.x
    r1.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 89: rcp r0.x, cb0[13].z
    r0.x = (1.0/(source[13].zzzz)).x;
    // 90: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 91: mul r9.xyz, r4.xyzx, cb0[13].zzzz
    r9.xyz = ((r4.xyzx)*(source[13].zzzz)).xyz;
    // 92: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 93: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 94: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 95: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 96: mad r4.xyz, r9.xyzx, cb0[13].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[13].zzzz)+(r4.xyzx)).xyz;
    // 97: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 98: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 99: add r0.x, cb0[13].z, l(1.000000)
    r0.x = ((source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 100: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 101: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 102: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 103: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 104: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 105: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 106: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 107: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 108: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 109: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 110: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 111: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 112: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 113: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 114: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 115: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 116: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 117: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 119: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 120: mul r11.xyz, r9.xyzx, cb0[17].yyyy
    r11.xyz = ((r9.xyzx)*(source[17].yyyy)).xyz;
    // 121: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 122: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 123: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 124: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 125: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 126: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 128: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 129: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 130: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 132: mul_sat r6.xy, r6.xzxx, cb0[14].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[14].yyyy))).xy;
    // 133: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 134: add_sat r6.y, r6.y, -cb0[14].z
    r6.y = (saturate((r6.yyyy)+(-(source[14].zzzz)))).y;
    // 135: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 136: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 137: mul r6.y, r6.y, cb0[14].w
    r6.y = ((r6.yyyy)*(source[14].wwww)).y;
    // 138: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 139: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 140: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 141: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 142: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 143: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 144: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 145: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 146: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 147: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 148: mul r0.z, r0.z, cb0[17].w
    r0.z = ((r0.zzzz)*(source[17].wwww)).z;
    // 149: mad r6.y, cb0[17].z, r6.y, -r4.z
    r6.y = ((source[17].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 150: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 151: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 152: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 153: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 154: mad r6.yzw, -cb0[17].yyyy, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[17].yyyy))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 155: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 156: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 157: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 158: mad r9.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r9.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 159: mad r7.xyz, cb0[12].yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 160: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 161: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 162: mad r7.xyz, cb0[12].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 163: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 164: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 166: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 167: dp3 r0.z, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 168: add r11.xyz, -r8.yzwy, r0.zzzz
    r11.xyz = ((-(r8.yzwy))+(r0.zzzz)).xyz;
    // 169: mad r8.xyz, cb0[12].yyyy, r11.xyzx, r8.yzwy
    r8.xyz = ((source[12].yyyy)*(r11.xyzx)+(r8.yzwy)).xyz;
    // 170: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 171: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 172: mad r8.xyz, cb0[12].zzzz, r11.xyzx, r8.xyzx
    r8.xyz = ((source[12].zzzz)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 173: mul r11.xyz, r7.xyzx, r8.xyzx
    r11.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 174: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 175: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 176: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 177: add r1.yzw, -cb0[6].xxyz, cb0[7].xxyz
    r1.yzw = ((-(source[6].xxyz))+(source[7].xxyz)).yzw;
    // 178: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[6].xyzx)).xyz;
    // 179: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 180: mul r1.xyz, r1.xyzx, cb0[13].wwww
    r1.xyz = ((r1.xyzx)*(source[13].wwww)).xyz;
    // 181: mul r12.xyz, r1.xyzx, r11.xyzx
    r12.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 182: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 184: mad r2.xyz, cb0[12].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 185: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 186: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 187: mad r2.xyz, cb0[12].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 188: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 189: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 190: mul r13.xyz, r13.xyzx, cb0[14].xxxx
    r13.xyz = ((r13.xyzx)*(source[14].xxxx)).xyz;
    // 191: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r14.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 192: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 193: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 194: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 195: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 196: max r13.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r13.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 197: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 198: mul r13.xyz, r13.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 199: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 200: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 201: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 202: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 203: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 204: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 205: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 206: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 207: div r1.w, cb0[15].y, r1.w
    r1.w = ((source[15].yyyy)/(r1.wwww)).w;
    // 208: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 209: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 210: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 211: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 212: mad r1.xyz, r2.xyzx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 213: mad r1.xyz, r1.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 214: mad r1.xyz, r4.xxxx, r1.xyzx, -r11.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r11.xyzx))).xyz;
    // 215: mad r1.xyz, r0.zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 216: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 217: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 218: mad r1.xyz, cb0[12].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 219: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 220: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 221: mad r1.xyz, cb0[12].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 222: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 223: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 224: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 225: mul r4.z, r4.z, cb0[16].y
    r4.z = ((r4.zzzz)*(source[16].yyyy)).z;
    // 226: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 227: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 228: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 229: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 230: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 231: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 232: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 233: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 234: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 235: mul r9.y, cb0[2].y, cb0[8].y
    r9.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 236: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 237: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 238: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 239: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 240: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 241: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 242: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 243: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 244: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 245: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 246: mul r1.w, cb0[9].y, cb0[16].y
    r1.w = ((source[9].yyyy)*(source[16].yyyy)).w;
    // 247: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 248: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 249: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 250: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 251: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 252: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 253: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 254: mul r3.z, cb0[9].x, l(0.001000)
    r3.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 255: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 256: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 257: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 258: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 259: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 260: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 261: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 262: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 263: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 264: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 265: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 266: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 267: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 268: mul r9.xyz, r3.xyzx, cb0[9].zzzz
    r9.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 269: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 270: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 271: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 272: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 273: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 274: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 275: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 276: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 277: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 278: mul r0.y, r2.w, cb0[18].x
    r0.y = ((r2.wwww)*(source[18].xxxx)).y;
    // 279: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 280: add r0.w, -cb0[18].y, l(2.000000)
    r0.w = ((-(source[18].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 281: mad r0.w, r4.x, r0.w, cb0[18].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[18].yyyy)).w;
    // 282: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 283: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 284: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 285: mul r0.xyz, r0.xyzx, cb0[18].zzzz
    r0.xyz = ((r0.xyzx)*(source[18].zzzz)).xyz;
    // 286: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 287: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 288: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 289: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 290: mul r0.xyz, r0.xyzx, cb0[18].wwww
    r0.xyz = ((r0.xyzx)*(source[18].wwww)).xyz;
    // 291: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 292: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 293: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 294: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 295: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 296: mul o0.xyz, r0.xyzx, cb0[19].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[19].xyzx)).xyz;
    // 297: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 298: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 299: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 300: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1519(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1520(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

// source.character.static-map-native-1521.v1 / source program 0312186516fbac49ae9d46cb5fde57a3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1521(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[2];
    source[1]=g_SourceCharacterLightConstants[3];
    source[2]=g_SourceCharacterLightConstants[4];
    source[3]=g_SourceCharacterLightConstants[5];
    source[4]=g_SourceCharacterLightConstants[6];
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
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 16: add r1.w, r2.w, l(-0.333300)
    r1.w = ((r2.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 17: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 18: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t3.zwxy, s3, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).xy;
    // 20: mad r4.xyz, cb0[3].zzzz, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[3].zzzz)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 21: mad r3.yzw, r3.yyyy, r4.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r3.yzw = ((r3.yyyy)*(r4.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 22: mul r2.xyz, r2.xyzx, r3.yzwy
    r2.xyz = ((r2.xyzx)*(r3.yzwy)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r3.yz, v4.xyxx, t0.zxyw, s1, l(0.000000)
    r3.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 24: mad r3.yz, r3.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r3.yz = ((r3.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 25: dp2 r1.w, r3.yzyy, r3.yzyy
    r1.w = (dot((r3.yzyy).xy,(r3.yzyy).xy).xxxx).w;
    // 26: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 28: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 29: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 30: mul r4.xy, r3.yzyy, cb0[3].xxxx
    r4.xy = ((r3.yzyy)*(source[3].xxxx)).xy;
    // 31: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 32: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 33: div r3.yzw, r4.xxyz, r1.wwww
    r3.yzw = ((r4.xxyz)/(r1.wwww)).yzw;
    // 34: dp3 r1.w, r3.yzwy, r1.xyzx
    r1.w = (dot((r3.yzwy).xyz,(r1.xyzx).xyz).xxxx).w;
    // 35: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mul r4.xyz, r0.xyzx, r2.wwww
    r4.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 38: mul r5.xyz, cb0[2].xyzx, cb0[4].yyyy
    r5.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // 39: mul r5.xyz, r3.xxxx, r5.xyzx
    r5.xyz = ((r3.xxxx)*(r5.xyzx)).xyz;
    // 40: mad r6.xyz, r5.xyzx, r0.xyzx, -r4.xyzx
    r6.xyz = ((r5.xyzx)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 41: mad r4.xyz, r5.xyzx, r6.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 43: mul r5.xyz, r5.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(source[1].xyzx)).xyz;
    // 44: mul r5.xyz, r5.xyzx, cb0[3].wwww
    r5.xyz = ((r5.xyzx)*(source[3].wwww)).xyz;
    // 45: mad r1.xyz, v7.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((v7.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 46: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 47: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 48: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 49: dp3 r0.w, r1.xyzx, r3.yzwy
    r0.w = (dot((r1.xyzx).xyz,(r3.yzwy).xyz).xxxx).w;
    // 50: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 51: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 52: mul r0.w, r0.w, cb0[4].x
    r0.w = ((r0.wwww)*(source[4].xxxx)).w;
    // 53: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 54: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 56: mul r1.xyz, r5.xyzx, r0.wwww
    r1.xyz = ((r5.xyzx)*(r0.wwww)).xyz;
    // 57: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 58: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 59: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 60: mad r0.xyz, r2.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 61: mul r1.xyz, r1.wwww, cb2[3].xyzx
    r1.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 62: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 63: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 64: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 65: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 66: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 67: ret
    return output;
}

// source.character.static-map-native-1522.v1 / source program cc85d1ed0bcdf74981d1829e6628dc12
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1522(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[3];
    source[2]=g_SourceCharacterLightConstants[4];
    source[3]=g_SourceCharacterLightConstants[5];
    source[4]=g_SourceCharacterLightConstants[6];
    source[5]=float4(input.lightColor,1.f);
    source[11].x=1.f;
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
    // 6: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].y
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].yyyy)) * 0xffffffffu)).x;
    // 7: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 8: mul r1.xyzw, v6.yyyy, cb0[7].xyzw
    r1.xyzw = ((v6.yyyy)*(source[7].xyzw)).xyzw;
    // 9: mad r1.xyzw, cb0[6].xyzw, v6.xxxx, r1.xyzw
    r1.xyzw = ((source[6].xyzw)*(v6.xxxx)+(r1.xyzw)).xyzw;
    // 10: mad r1.xyzw, cb0[8].xyzw, v6.zzzz, r1.xyzw
    r1.xyzw = ((source[8].xyzw)*(v6.zzzz)+(r1.xyzw)).xyzw;
    // 11: mad r1.xyzw, cb0[9].xyzw, v6.wwww, r1.xyzw
    r1.xyzw = ((source[9].xyzw)*(v6.wwww)+(r1.xyzw)).xyzw;
    // 12: div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // 13: sample_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t1.xyzw, s3
    r2.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 14: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 15: mov r3.yz, cb0[10].wwzw
    r3.yz = (source[10].wwzw).yz;
    // 16: add r3.xyzw, r1.xyxy, r3.xyzw
    r3.xyzw = ((r1.xyxy)+(r3.xyzw)).xyzw;
    // 17: sample_indexable(texture2d)(float,float,float,float) r2.y, r3.xyxx, t1.yxzw, s3
    r2.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 18: sample_indexable(texture2d)(float,float,float,float) r2.z, r3.zwzz, t1.yzxw, s3
    r2.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 19: add r3.xy, r1.xyxx, cb0[10].zwzz
    r3.xy = ((r1.xyxx)+(source[10].zwzz)).xy;
    // 20: sample_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t1.yzwx, s3
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
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t3.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 35: mul r3.xyz, cb0[1].xyzx, cb0[3].zzzz
    r3.xyz = ((source[1].xyzx)*(source[3].zzzz)).xyz;
    // 36: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 38: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 39: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 40: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 42: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 43: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: mul r3.xy, r3.xyxx, cb0[3].xxxx
    r3.xy = ((r3.xyxx)*(source[3].xxxx)).xy;
    // 45: mul r4.xy, r3.xyxx, v0.wwww
    r4.xy = ((r3.xyxx)*(v0.wwww)).xy;
    // 46: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 47: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 48: div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 49: dp3 r1.w, r3.xyzx, r0.yzwy
    r1.w = (dot((r3.xyzx).xyz,(r0.yzwy).xyz).xxxx).w;
    // 50: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 51: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 53: mul r5.xyz, cb0[2].xyzx, cb0[3].wwww
    r5.xyz = ((source[2].xyzx)*(source[3].wwww)).xyz;
    // 54: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 55: mad r0.xyz, v5.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v5.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 56: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 57: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 58: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 59: dp3 r0.x, r0.xyzx, r3.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 60: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 61: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 62: mul r0.x, r0.x, cb0[4].x
    r0.x = ((r0.xxxx)*(source[4].xxxx)).x;
    // 63: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 64: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 66: mul r0.xyz, r4.xyzx, r0.xxxx
    r0.xyz = ((r4.xyzx)*(r0.xxxx)).xyz;
    // 67: mad r0.xyz, r3.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r3.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 68: mul r2.xyz, r1.wwww, cb2[3].xyzx
    r2.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 69: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 70: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 71: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 72: mul_sat r0.x, r2.w, cb0[4].y
    r0.x = (saturate((r2.wwww)*(source[4].yyyy))).x;
    // 73: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 74: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 75: ret
    return output;
}

// source.character.static-map-native-1523.v1 / source program 9466bf5836781a42a7ad47f4a4b3f8e7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1523(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(0.200000003,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.200000003,0,0,0))))),1u);
    source[6]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0.200000003,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.200000003,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0))))),1u);
    source[7]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0))))),1u);
    source[8]=g_SourceCharacterLightConstants[9];
    source[9]=g_SourceCharacterLightConstants[10];
    source[10]=g_SourceCharacterLightConstants[11];
    source[11]=g_SourceCharacterLightConstants[12];
    source[11].w=(g_SourceCharacterTime.xxxx).x;
    source[12]=g_SourceCharacterLightConstants[14];
    source[13]=g_SourceCharacterLightConstants[15];
    source[13].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[13].w=((sign((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))))).x;
    source[14]=g_SourceCharacterLightConstants[16];
    source[15]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xy, -v2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v2.xyxx
    r0.xy = ((r0.xyxx)*(v2.xyxx)).xy;
    // 3: lt r0.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 4: add r1.x, -cb0[1].x, l(0.001000)
    r1.x = ((-(source[1].xxxx))+(float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 5: lt r0.z, r1.x, l(0.000000)
    r0.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 6: add r1.x, cb0[1].y, l(0.001000)
    r1.x = ((source[1].yyyy)+(float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 7: lt r0.w, r1.x, l(0.000000)
    r0.w = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 8: or r0.xy, r0.zwzz, r0.xyxx
    r0.xy = (asfloat(asuint(r0.zwzz) | asuint(r0.xyxx))).xy;
    // 9: or r0.x, r0.y, r0.x
    r0.x = (asfloat(asuint(r0.yyyy) | asuint(r0.xxxx))).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 11: add r0.xy, v6.xyxx, cb0[0].xyxx
    r0.xy = ((v6.xyxx)+(source[0].xyxx)).xy;
    // 12: mul r0.zw, r0.xxxy, cb0[8].zzzz
    r0.zw = ((r0.xxxy)*(source[8].zzzz)).zw;
    // 13: mad r0.xy, cb0[8].zzzz, r0.xyxx, cb0[5].xyxx
    r0.xy = ((source[8].zzzz)*(r0.xyxx)+(source[5].xyxx)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 15: mad r0.xy, r0.zwzz, l(1.300000, 1.300000, 0.000000, 0.000000), l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(1.300000,1.300000,0.000000,0.000000))+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: dp2 r2.x, l(0.540302, -0.841471, 0.000000, 0.000000), r0.xyxx
    r2.x = (dot((float4(0.540302,-0.841471,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).x;
    // 17: dp2 r2.y, l(0.841471, 0.540302, 0.000000, 0.000000), r0.xyxx
    r2.y = (dot((float4(0.841471,0.540302,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).y;
    // 18: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 19: mul r2.xy, r0.xyxx, cb0[8].wwww
    r2.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    // 26: add r2.z, r1.w, l(0.000010)
    r2.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: mul r3.xy, r0.zwzz, cb0[8].wwww
    r3.xy = ((r0.zwzz)*(source[8].wwww)).xy;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 29: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 30: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 31: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 34: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 35: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 36: mul r4.xy, r0.xyxx, l(0.570000, 0.570000, 0.000000, 0.000000)
    r4.xy = ((r0.xyxx)*(float4(0.570000,0.570000,0.000000,0.000000))).xy;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: add r1.w, r4.y, r4.x
    r1.w = ((r4.yyyy)+(r4.xxxx)).w;
    // 39: add r1.w, r4.z, r1.w
    r1.w = ((r4.zzzz)+(r1.wwww)).w;
    // 40: mul r1.w, r1.w, l(0.333330)
    r1.w = ((r1.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 41: mad r2.xyz, r1.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 42: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 43: dp3 r2.w, v3.xyzx, v3.xyzx
    r2.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 44: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 45: mul r3.xyz, r2.wwww, v3.xyzx
    r3.xyz = ((r2.wwww)*(v3.xyzx)).xyz;
    // 46: dp3 r2.z, r3.xyzx, r2.xyzx
    r2.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 47: mul r2.xy, r2.xyxx, cb0[9].yyyy
    r2.xy = ((r2.xyxx)*(source[9].yyyy)).xy;
    // 48: max r2.w, r3.z, l(0.000000)
    r2.w = (max(r3.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 49: mul r3.xyz, r2.wwww, cb2[3].xyzx
    r3.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // 50: mul r4.xy, r0.xyxx, cb0[10].xxxx
    r4.xy = ((r0.xyxx)*(source[10].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 52: mul r5.xy, r0.zwzz, cb0[10].xxxx
    r5.xy = ((r0.zwzz)*(source[10].xxxx)).xy;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 54: add r4.xyz, r4.xyzx, -r5.xyzx
    r4.xyz = ((r4.xyzx)+(-(r5.xyzx))).xyz;
    // 55: mad r4.xyz, r1.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 56: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: add r5.xyz, -r4.xyzx, r1.wwww
    r5.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 58: mad r4.xyz, cb0[10].yyyy, r5.xyzx, r4.xyzx
    r4.xyz = ((source[10].yyyy)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 59: mul r5.xyz, cb0[4].xyzx, cb0[4].wwww
    r5.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 60: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 61: mul r4.xyw, r2.zzzz, r4.xyxz
    r4.xyw = ((r2.zzzz)*(r4.xyxz)).xyw;
    // 62: add r1.w, r4.y, r4.x
    r1.w = ((r4.yyyy)+(r4.xxxx)).w;
    // 63: mad r1.w, r2.z, r4.z, r1.w
    r1.w = ((r2.zzzz)*(r4.zzzz)+(r1.wwww)).w;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r0.zwzz, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 66: add r5.xyz, r5.xyzx, -r6.xyzx
    r5.xyz = ((r5.xyzx)+(-(r6.xyzx))).xyz;
    // 67: mad r5.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r6.xyzx
    r5.xyz = ((r5.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r6.xyzx)).xyz;
    // 68: add r2.z, r5.y, r5.x
    r2.z = ((r5.yyyy)+(r5.xxxx)).z;
    // 69: add r2.z, r5.z, r2.z
    r2.z = ((r5.zzzz)+(r2.zzzz)).z;
    // 70: mad r2.z, r2.z, l(0.333330), l(-0.500000)
    r2.z = ((r2.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).z;
    // 71: mad r2.zw, cb0[10].wwww, r2.zzzz, v2.xxxy
    r2.zw = ((source[10].wwww)*(r2.zzzz)+(v2.xxxy)).zw;
    // 72: add r2.zw, r2.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r2.zw = ((r2.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 73: dp2 r2.z, r2.zwzz, r2.zwzz
    r2.z = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).z;
    // 74: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 75: mad_sat r2.z, -r2.z, cb0[11].z, l(1.000000)
    r2.z = (saturate((-(r2.zzzz))*(source[11].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000)))).z;
    // 76: add r5.xy, r0.xyxx, cb0[6].xyxx
    r5.xy = ((r0.xyxx)+(source[6].xyxx)).xy;
    // 77: mad r0.xy, cb0[13].zzzz, r0.xyxx, cb0[7].xyxx
    r0.xy = ((source[13].zzzz)*(r0.xyxx)+(source[7].xyxx)).xy;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 79: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 80: add r5.xyz, -r1.xyzx, r5.xyzx
    r5.xyz = ((-(r1.xyzx))+(r5.xyzx)).xyz;
    // 81: mad r1.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r1.xyzx
    r1.xyz = ((r5.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r1.xyzx)).xyz;
    // 82: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 83: add r0.x, r1.z, r0.x
    r0.x = ((r1.zzzz)+(r0.xxxx)).x;
    // 84: mad r0.xy, r0.xxxx, l(-0.016666, 0.005000, 0.000000, 0.000000), r2.zzzz
    r0.xy = ((r0.xxxx)*(float4(-0.016666,0.005000,0.000000,0.000000))+(r2.zzzz)).xy;
    // 85: mad r1.x, -v0.x, l(0.985000), l(1.000000)
    r1.x = ((-(v0.xxxx))*(float4(0.985000,0.985000,0.985000,0.985000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 86: add r0.xy, r0.yxyy, -r1.xxxx
    r0.xy = ((r0.yxyy)+(-(r1.xxxx))).xy;
    // 87: add r1.x, -|r0.y|, l(1.000000)
    r1.x = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 88: mad r1.y, r1.w, l(0.333330), r1.x
    r1.y = ((r1.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))+(r1.xxxx)).y;
    // 89: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 90: mul_sat r1.x, r1.x, cb0[12].x
    r1.x = (saturate((r1.xxxx)*(source[12].xxxx))).x;
    // 91: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 92: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 93: mul r1.y, r1.y, cb0[12].y
    r1.y = ((r1.yyyy)*(source[12].yyyy)).y;
    // 94: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 95: movc r1.x, r1.x, l(0), r1.y
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).x;
    // 96: mad r1.yzw, cb0[10].zzzz, r4.xxyw, -r4.xxyw
    r1.yzw = ((source[10].zzzz)*(r4.xxyw)+(-(r4.xxyw))).yzw;
    // 97: mad r1.xyz, r1.xxxx, r1.yzwy, r4.xywx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(r4.xywx)).xyz;
    // 98: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 99: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 100: mul r2.zw, r1.wwww, v5.xxxy
    r2.zw = ((r1.wwww)*(v5.xxxy)).zw;
    // 101: mad r2.xy, cb0[8].yyyy, -r2.zwzz, r2.xyxx
    r2.xy = ((source[8].yyyy)*(-(r2.zwzz))+(r2.xyxx)).xy;
    // 102: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 103: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 104: add r4.xyz, -r2.xyzx, r1.wwww
    r4.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 105: mad r2.xyz, cb0[9].zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((source[9].zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 106: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 107: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 108: mul r2.xyz, r2.xyzx, cb0[9].wwww
    r2.xyz = ((r2.xyzx)*(source[9].wwww)).xyz;
    // 109: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 110: mul r4.xyz, cb0[3].xyzx, cb0[3].wwww
    r4.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 111: mad r1.xyz, r2.xyzx, r4.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 112: mul r1.xyz, r1.xyzx, cb2[3].wwww
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)).xyz;
    // 113: mad r1.xyz, r1.xyzx, l(0.850000, 0.850000, 0.850000, 0.000000), r3.xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.850000,0.850000,0.850000,0.000000))+(r3.xyzx)).xyz;
    // 114: mul o0.xyz, r1.xyzx, cb0[15].xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[15].xyzx)).xyz;
    // 115: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 116: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t5.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 117: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 118: add r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)+(r1.xxxx)).x;
    // 119: mul r1.x, r1.x, l(0.333330)
    r1.x = ((r1.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 120: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 121: mul_sat r0.xy, r0.xyxx, l(100.000000, 500.000000, 0.000000, 0.000000)
    r0.xy = (saturate((r0.xyxx)*(float4(100.000000,500.000000,0.000000,0.000000)))).xy;
    // 122: mad r0.x, r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.xxxx)+(r0.yyyy)).x;
    // 123: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 124: mad r1.xy, cb0[13].zzzz, r0.zwzz, cb0[7].xyxx
    r1.xy = ((source[13].zzzz)*(r0.zwzz)+(source[7].xyxx)).xy;
    // 125: mul r0.yz, r0.zzwz, cb0[14].yyyy
    r0.yz = ((r0.zzwz)*(source[14].yyyy)).yz;
    // 126: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t6.wxyz, s6, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 127: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 128: add r1.xyz, -r6.xyzx, r1.xyzx
    r1.xyz = ((-(r6.xyzx))+(r1.xyzx)).xyz;
    // 129: mad r1.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r6.xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r6.xyzx)).xyz;
    // 130: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 131: add r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)+(r1.xxxx)).x;
    // 132: mul_sat r1.x, r1.x, l(0.333330)
    r1.x = (saturate((r1.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330)))).x;
    // 133: mov_sat r1.y, v0.w
    r1.y = (saturate(v0.wwww)).y;
    // 134: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 135: add r1.x, -r1.y, r1.x
    r1.x = ((-(r1.yyyy))+(r1.xxxx)).x;
    // 136: mul_sat r1.x, r1.x, l(200.000000)
    r1.x = (saturate((r1.xxxx)*(float4(200.000000,200.000000,200.000000,200.000000)))).x;
    // 137: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 138: add r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)+(r0.yyyy)).y;
    // 139: mul r0.y, r0.y, cb0[14].z
    r0.y = ((r0.yyyy)*(source[14].zzzz)).y;
    // 140: mul r0.y, r0.y, l(0.333330)
    r0.y = ((r0.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))).y;
    // 141: mad_sat r0.x, r0.x, r1.x, -r0.y
    r0.x = (saturate((r0.xxxx)*(r1.xxxx)+(-(r0.yyyy)))).x;
    // 142: mul r0.x, r0.x, cb0[14].w
    r0.x = ((r0.xxxx)*(source[14].wwww)).x;
    // 143: mul_sat r0.x, r0.x, v0.z
    r0.x = (saturate((r0.xxxx)*(v0.zzzz))).x;
    // 144: mul o0.w, r0.x, cb0[1].z
    output.targets[0].w = ((r0.xxxx)*(source[1].zzzz)).w;
    // 145: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 146: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1524(SOURCE_CHARACTER_NATIVE_INPUT input)
{ return (SOURCE_CHARACTER_NATIVE_OUTPUT)0; }

// source.character.static-map-native-1525.v1 / source program 8bf47364b5e99146ad98324bc3ff5c30
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1525(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 4: mad r0.xyz, r0.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r0.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 5: dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r1.xyz, r0.wwww, v3.zxyz
    r1.xyz = ((r0.wwww)*(v3.zxyz)).xyz;
    // 8: dp3_sat r0.x, r0.zxyz, r1.xyzx
    r0.x = (saturate(dot((r0.zxyz).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 9: mov_sat r1.x, r1.x
    r1.x = (saturate(r1.xxxx)).x;
    // 10: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 11: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: mul r0.y, r0.y, l(15.000000)
    r0.y = ((r0.yyyy)*(float4(15.000000,15.000000,15.000000,15.000000))).y;
    // 13: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 14: mul r0.yzw, r0.yyyy, cb2[4].xxyz
    r0.yzw = ((r0.yyyy)*(passValues[4].xxyz)).yzw;
    // 15: movc r0.xyz, r0.xxxx, l(0,0,0,0), r0.yzwy
    r0.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yzwy)).xyz;
    // 16: add r1.yzw, -cb0[1].xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r1.yzw = ((-(source[1].xxyz))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 17: mul r1.yzw, r1.yyzw, cb0[2].xxyz
    r1.yzw = ((r1.yyzw)*(source[2].xxyz)).yzw;
    // 18: mad r1.yzw, r1.yyzw, cb2[3].wwww, cb2[3].xxyz
    r1.yzw = ((r1.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 19: lt r0.w, r1.x, l(0.000001)
    r0.w = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 20: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 21: mad r0.xyz, r1.yzwy, r0.wwww, r0.xyzx
    r0.xyz = ((r1.yzwy)*(r0.wwww)+(r0.xyzx)).xyz;
    // 22: mul o0.xyz, r0.xyzx, cb0[4].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[4].xyzx)).xyz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v2.xyxx, t0.wxyz, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).x;
    // 24: mul r0.x, r0.x, cb0[3].x
    r0.x = ((r0.xxxx)*(source[3].xxxx)).x;
    // 25: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 26: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 27: ret
    return output;
}

