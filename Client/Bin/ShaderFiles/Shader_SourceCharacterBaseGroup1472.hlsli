SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1500(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f;
    // 1: mul r0.xy, v4.xyxx, l(0.750000, 3.000000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)*(float4(0.750000,3.000000,0.000000,0.000000))).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 4: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 5: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 6: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 7: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 8: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 9: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v4.xyxx, t0.yzxw, s0, l(0.000000)
    r0.z = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).z;
    // 11: mul r0.y, r0.z, |r0.y|
    r0.y = ((r0.zzzz)*(abs(r0.yyyy))).y;
    // 12: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 13: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 14: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 15: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 16: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 17: mad r1.xyz, -cb0[2].wwww, cb0[2].xyzx, r1.xxxx
    r1.xyz = ((-(source[2].wwww))*(source[2].xyzx)+(r1.xxxx)).xyz;
    // 18: mad r0.yzw, r1.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r0.yyzw
    r0.yzw = ((r1.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r0.yyzw)).yzw;
    // 19: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 20: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 21: mul o0.xyz, r0.xxxx, r0.yzwy
    output.targets[0].xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 22: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 23: ret
    return output;
}

// source.character.static-map-native-1501.v1 / source program f0d24fec647e5c4195be3c40a23b612c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1501(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f;
    // 1: mul r0.xy, v4.xyxx, l(0.750000, 3.000000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)*(float4(0.750000,3.000000,0.000000,0.000000))).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: max r0.x, |r0.x|, l(0.000001)
    r0.x = (max(abs(r0.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).x;
    // 4: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 5: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 6: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 7: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 8: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 9: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.z, v4.xyxx, t0.yzxw, s0, l(0.000000)
    r0.z = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).z;
    // 11: mul r0.y, r0.z, |r0.y|
    r0.y = ((r0.zzzz)*(abs(r0.yyyy))).y;
    // 12: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 13: mul_sat r0.x, r0.x, l(5.000000)
    r0.x = (saturate((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 14: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 15: mul r0.yzw, cb0[2].xxyz, cb0[2].wwww
    r0.yzw = ((source[2].xxyz)*(source[2].wwww)).yzw;
    // 16: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 17: mad r1.xyz, -cb0[2].wwww, cb0[2].xyzx, r1.xxxx
    r1.xyz = ((-(source[2].wwww))*(source[2].xyzx)+(r1.xxxx)).xyz;
    // 18: mad r0.yzw, r1.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r0.yyzw
    r0.yzw = ((r1.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r0.yyzw)).yzw;
    // 19: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 20: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 21: mul o0.xyz, r0.xxxx, r0.yzwy
    output.targets[0].xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 22: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 23: ret
    return output;
}

// source.character.static-map-native-1502.v1 / source program abd407a677b45447981a5b9fc023037c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1502(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[6].w=(g_SourceCharacterTime.xxxx).x;
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[12]=g_SourceCharacterBaseConstants[11];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s0, l(0.000000)
    r0.x = ((float4(0.0,0.0,0.0,0.0)).xyzw).x;
    // 4: min r0.x, r0.x, l(0.999000)
    r0.x = (min(r0.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // 5: mad r0.y, r0.x, cb2[1].z, -cb2[1].w
    r0.y = ((r0.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // 6: mad r0.x, r0.x, cb2[1].x, cb2[1].y
    r0.x = ((r0.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).x;
    // 7: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 8: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 9: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 10: add r0.y, -cb0[11].z, l(1.000000)
    r0.y = ((-(source[11].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.yzw, r0.yyyy, v6.xxyz
    r0.yzw = ((r0.yyyy)*(v6.xxyz)).yzw;
    // 16: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 17: mul r1.x, r1.x, cb0[11].w
    r1.x = ((r1.xxxx)*(source[11].wwww)).x;
    // 18: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 19: mul_sat r1.x, r1.x, cb0[12].x
    r1.x = (saturate((r1.xxxx)*(source[12].xxxx))).x;
    // 20: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 21: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 22: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 23: mad r0.w, cb0[6].w, cb0[9].y, cb0[9].z
    r0.w = ((source[6].wwww)*(source[9].yyyy)+(source[9].zzzz)).w;
    // 24: sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
    // 25: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 26: add r1.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 27: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 28: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 29: dp2 r2.y, r3.zyzz, r1.yzyy
    r2.y = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).y;
    // 30: dp2 r2.x, r3.yxyy, r1.yzyy
    r2.x = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).x;
    // 31: mad r1.xy, r2.xyxx, cb0[4].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(source[4].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t2.yzwx, s3, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 33: mul r1.xy, v4.xyxx, cb0[8].yzyy
    r1.xy = ((v4.xyxx)*(source[8].yzyy)).xy;
    // 34: mul r1.z, cb0[6].z, cb0[6].w
    r1.z = ((source[6].zzzz)*(source[6].wwww)).z;
    // 35: mad r1.xy, r1.zzzz, cb0[8].xwxx, r1.xyxx
    r1.xy = ((r1.zzzz)*(source[8].xwxx)+(r1.xyxx)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 37: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 38: mul r1.xy, v4.xyxx, cb0[7].yzyy
    r1.xy = ((v4.xyxx)*(source[7].yzyy)).xy;
    // 39: mad r1.xy, r1.zzzz, cb0[7].xwxx, r1.xyxx
    r1.xy = ((r1.zzzz)*(source[7].xwxx)+(r1.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 41: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 42: mul_sat r1.x, r0.w, cb0[11].x
    r1.x = (saturate((r0.wwww)*(source[11].xxxx))).x;
    // 43: mad r1.yz, cb0[3].wwww, cb0[3].xxyx, r0.wwww
    r1.yz = ((source[3].wwww)*(source[3].xxyx)+(r0.wwww)).yz;
    // 44: mul r1.yz, r1.yyzy, cb0[10].yyyy
    r1.yz = ((r1.yyzy)*(source[10].yyyy)).yz;
    // 45: mad r0.yz, r0.yyzy, cb0[2].xxyx, r1.yyzy
    r0.yz = ((r0.yyzy)*(source[2].xxyx)+(r1.yyzy)).yz;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s4, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 47: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 48: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 49: mul r1.y, r1.y, cb0[11].y
    r1.y = ((r1.yyyy)*(source[11].yyyy)).y;
    // 50: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 51: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 52: mul_sat r0.x, r0.x, v2.w
    r0.x = (saturate((r0.xxxx)*(v2.wwww))).x;
    // 53: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 54: movc r0.x, r1.x, l(0), r0.x
    r0.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 55: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 56: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 57: mad r0.yzw, cb0[10].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[10].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 58: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 59: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 60: mul r0.yzw, r0.yyzw, cb0[10].wwww
    r0.yzw = ((r0.yyzw)*(source[10].wwww)).yzw;
    // 61: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 62: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 63: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 64: mad r0.yzw, r0.yyzw, r2.xxyz, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)+(r1.xxyz)).yzw;
    // 65: mad r0.yzw, r0.yyzw, v2.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v2.xxyz)+(source[1].xxyz)).yzw;
    // 66: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 67: mul o0.xyz, r0.xxxx, r0.yzwy
    output.targets[0].xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 68: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 69: ret
    return output;
}

// source.character.static-map-native-1503.v1 / source program a2c20a1e6f809443970604f6fa558462
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1503(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: mul r0.xyz, cb0[3].xyzx, cb0[3].wwww
    r0.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 2: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 3: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 4: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 5: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 6: mul r3.xyzw, v4.xyxy, l(44.000000, 15.000000, 2.000000, 3.000000)
    r3.xyzw = ((v4.xyxy)*(float4(44.000000,15.000000,2.000000,3.000000))).xyzw;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.zwzz, t3.yzwx, s6, l(0.000000)
    r0.w = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 9: max r0.w, |r0.w|, l(0.000001)
    r0.w = (max(abs(r0.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 10: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 11: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 12: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 13: mad r3.xy, r3.xyxx, l(0.005000, 0.005000, 0.000000, 0.000000), v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.005000,0.005000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r1.w = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 15: mad r1.w, r1.w, l(0.010000), l(-0.025000)
    r1.w = ((r1.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).w;
    // 16: mad r3.xy, r1.wwww, r2.xyxx, r3.xyxx
    r3.xy = ((r1.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mad r4.xyzw, r3.wwww, l(0.100000, 0.100000, 0.050000, 0.050000), l(-0.025000, -0.025000, -0.025000, -0.025000)
    r4.xyzw = ((r3.wwww)*(float4(0.100000,0.100000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).xyzw;
    // 19: mad r4.xyzw, r4.xyzw, r2.xyxy, v4.xyxy
    r4.xyzw = ((r4.xyzw)*(r2.xyxy)+(v4.xyxy)).xyzw;
    // 20: mul r4.xyzw, r4.xyzw, l(8.500000, 7.000000, 6.000000, 5.000000)
    r4.xyzw = ((r4.xyzw)*(float4(8.500000,7.000000,6.000000,5.000000))).xyzw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.zwzz, t6.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t5.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 23: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 24: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 25: add r6.xyz, -r5.xyzx, r1.wwww
    r6.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 26: mad r5.xyz, r6.xyzx, l(0.700000, 0.700000, 0.700000, 0.000000), r5.xyzx
    r5.xyz = ((r6.xyzx)*(float4(0.700000,0.700000,0.700000,0.000000))+(r5.xyzx)).xyz;
    // 27: max r5.xyz, |r5.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (max(abs(r5.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 28: mul r6.xyz, r5.xyzx, r5.xyzx
    r6.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 29: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 30: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 31: mul r1.xyz, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 32: mul r5.xyz, r4.xyzx, r4.xyzx
    r5.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 33: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 34: mad r0.xyz, r4.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 35: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 36: mad r2.xy, r3.xyxx, l(0.200000, 0.200000, 0.000000, 0.000000), -r2.xyxx
    r2.xy = ((r3.xyxx)*(float4(0.200000,0.200000,0.000000,0.000000))+(-(r2.xyxx))).xy;
    // 37: add r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)+(r2.xyxx)).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r2.xyw, r2.xyxx, t4.xywz, s2, l(0.000000)
    r2.xyw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyw;
    // 39: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: mul r2.xyw, r2.xyxw, r1.wwww
    r2.xyw = ((r2.xyxw)*(r1.wwww)).xyw;
    // 41: max r2.xyw, |r2.xyxw|, l(0.000001, 0.000001, 0.000000, 0.000001)
    r2.xyw = (max(abs(r2.xyxw),float4(0.000001,0.000001,0.000000,0.000001))).xyw;
    // 42: mul r4.xyz, r2.xywx, r2.xywx
    r4.xyz = ((r2.xywx)*(r2.xywx)).xyz;
    // 43: mul r2.xyw, r2.xyxw, r4.xyxz
    r2.xyw = ((r2.xyxw)*(r4.xyxz)).xyw;
    // 44: mad r0.xyz, r2.xywx, r1.xyzx, r0.xyzx
    r0.xyz = ((r2.xywx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 45: dp3 r1.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 46: add r1.xyz, -r3.xyzx, r1.xxxx
    r1.xyz = ((-(r3.xyzx))+(r1.xxxx)).xyz;
    // 47: mad r1.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r3.xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r3.xyzx)).xyz;
    // 48: mul r2.xyw, cb0[5].xyxz, cb0[5].wwww
    r2.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 49: mul r1.xyz, r1.xyzx, r2.xywx
    r1.xyz = ((r1.xyzx)*(r2.xywx)).xyz;
    // 50: mad r0.xyz, r3.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 51: mul r1.xyz, cb0[6].xyzx, cb0[6].wwww
    r1.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 52: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 53: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 54: mul r0.x, |r3.w|, |r3.w|
    r0.x = ((abs(r3.wwww))*(abs(r3.wwww))).x;
    // 55: mul r0.x, r0.x, |r3.w|
    r0.x = ((r0.xxxx)*(abs(r3.wwww))).x;
    // 56: lt r0.y, |r3.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 57: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 58: add r0.x, |r2.z|, r0.x
    r0.x = ((abs(r2.zzzz))+(r0.xxxx)).x;
    // 59: min r0.y, r0.x, l(1.000000)
    r0.y = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 60: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 61: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 62: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 63: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 64: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 65: mul r0.x, r0.x, r3.w
    r0.x = ((r0.xxxx)*(r3.wwww)).x;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t2.yxzw, s5, l(0.000000)
    r0.y = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 67: mul_sat r0.y, r0.y, l(3.000000)
    r0.y = (saturate((r0.yyyy)*(float4(3.000000,3.000000,3.000000,3.000000)))).y;
    // 68: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 69: mul r0.x, r0.w, r0.x
    r0.x = ((r0.wwww)*(r0.xxxx)).x;
    // 70: mul_sat r0.x, r0.x, l(9.000000)
    r0.x = (saturate((r0.xxxx)*(float4(9.000000,9.000000,9.000000,9.000000)))).x;
    // 71: mul_sat r0.x, r0.x, cb0[7].x
    r0.x = (saturate((r0.xxxx)*(source[7].xxxx))).x;
    // 72: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 73: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 74: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 75: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 76: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 77: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 78: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 79: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 80: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 81: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 82: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 83: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 84: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 85: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 86: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 87: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 88: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 89: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 90: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 91: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 92: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 93: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 94: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 95: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 96: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 97: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 98: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 99: ret
    return output;
}

// source.character.static-map-native-1504.v1 / source program b29b1d29a2878e47ace0bf77236d5e8f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1504(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[3];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: mul r0.xy, r0.xyxx, cb0[2].xxxx
    r0.xy = ((r0.xyxx)*(source[2].xxxx)).xy;
    // 5: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 6: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 9: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 11: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 12: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 13: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 16: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 19: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 20: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 22: mul r1.yzw, r1.yyyy, cb0[4].xxyz
    r1.yzw = ((r1.yyyy)*(source[4].xxyz)).yzw;
    // 23: mad r1.xyz, r1.xxxx, cb0[3].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[3].xyzx)+(r1.yzwy)).xyz;
    // 24: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 26: mul r3.xyz, cb0[1].xyzx, cb0[2].yyyy
    r3.xyz = ((source[1].xyzx)*(source[2].yyyy)).xyz;
    // 27: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 28: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 29: mad r3.xyz, r1.xyzx, r2.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)+(source[0].xyzx)).xyz;
    // 30: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 31: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 32: mad o0.xyz, r2.xyzx, cb0[5].xyzx, r3.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[5].xyzx)+(r3.xyzx)).xyz;
    // 33: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 34: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 35: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 36: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 37: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 38: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 39: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 40: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 41: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 42: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 43: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 44: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 45: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 46: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 47: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 48: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 49: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 50: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 51: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 52: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 53: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 54: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 55: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 56: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 57: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 59: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 60: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 61: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 62: ret
    return output;
}

// source.character.static-map-native-1505.v1 / source program d6937fdf673c9745b36c6e88ee4e9880
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1505(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.x, -v4.y, cb0[5].x
    r0.x = ((-(v4.yyyy))+(source[5].xxxx)).x;
    // 2: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 3: mul r0.x, r0.x, cb0[5].y
    r0.x = ((r0.xxxx)*(source[5].yyyy)).x;
    // 4: mul r0.yzw, cb0[3].xxyz, cb0[3].wwww
    r0.yzw = ((source[3].xxyz)*(source[3].wwww)).yzw;
    // 5: mul r1.xyz, r0.yzwy, r0.xxxx
    r1.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 7: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 8: mul o0.w, r2.w, cb0[0].x
    output.targets[0].w = ((r2.wwww)*(source[0].xxxx)).w;
    // 9: mad r0.xyz, r0.xxxx, r0.yzwy, -r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(-(r2.xyzx))).xyz;
    // 10: mad r0.xyz, r1.xyzx, r0.xyzx, r2.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 11: mul r1.xyz, r0.xyzx, cb0[5].zzzz
    r1.xyz = ((r0.xyzx)*(source[5].zzzz)).xyz;
    // 12: mad r0.xyz, -cb0[5].zzzz, r0.xyzx, cb0[4].xyzx
    r0.xyz = ((-(source[5].zzzz))*(r0.xyzx)+(source[4].xyzx)).xyz;
    // 13: mad r0.xyz, cb0[4].wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 15: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 16: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 17: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 18: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 19: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 22: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 23: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 24: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 25: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 26: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 27: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 28: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 29: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 30: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 31: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 32: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 33: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 34: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 35: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 36: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 37: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 38: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 39: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 40: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 41: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 42: ret
    return output;
}

// source.character.static-map-native-1506.v1 / source program 8b228c7b319b544781cca408746673a2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1506(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=SourceCharacterAppend((sign(((g_SourceCharacterBaseConstants[11]*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((g_SourceCharacterBaseConstants[11]*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),(sign(((g_SourceCharacterBaseConstants[11]*g_SourceCharacterTime.xxxx)*float4(0.300000012,0,0,0)))*frac(abs(((g_SourceCharacterBaseConstants[11]*g_SourceCharacterTime.xxxx)*float4(0.300000012,0,0,0))))),1u);
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[8].z=(g_SourceCharacterTime.xxxx).x;
    source[9]=g_SourceCharacterBaseConstants[8];
    source[9].x=((sign(((g_SourceCharacterBaseConstants[11]*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((g_SourceCharacterBaseConstants[11]*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))))).x;
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: mul r0.zw, v4.xxxy, cb0[8].xxxy
    r0.zw = ((v4.xxxy)*(source[8].xxxy)).zw;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s0, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 5: mad r1.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 8: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 9: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 10: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.z, r1.xyzx, r1.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 12: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 13: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 14: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 15: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 16: mul r2.xyz, r0.zzzz, v6.xyzx
    r2.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 17: dp3 r0.z, r1.xyzx, r2.xyzx
    r0.z = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 18: mul r3.xyz, r0.zzzz, r1.xyzx
    r3.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 19: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 20: dp3 r0.z, r2.xyzx, r3.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 21: add r2.xy, r3.xyxx, cb0[3].xyxx
    r2.xy = ((r3.xyxx)+(source[3].xyxx)).xy;
    // 22: div r2.xy, r2.xyxx, cb0[9].yyyy
    r2.xy = ((r2.xyxx)/(source[9].yyyy)).xy;
    // 23: mad r2.xy, r2.xyxx, cb0[9].zwzz, cb0[10].xyxx
    r2.xy = ((r2.xyxx)*(source[9].zwzz)+(source[10].xyxx)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r2.xyw, r2.xyxx, t1.xywz, s1, l(0.000000)
    r2.xyw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyw;
    // 25: max r0.w, r2.z, l(0.000000)
    r0.w = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 26: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: add r1.w, -|r0.z|, l(1.000000)
    r1.w = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: mad r0.xy, r1.wwww, cb0[5].xxxx, r0.xyxx
    r0.xy = ((r1.wwww)*(source[5].xxxx)+(r0.xyxx)).xy;
    // 29: add r1.w, cb0[5].y, cb0[5].y
    r1.w = ((source[5].yyyy)+(source[5].yyyy)).w;
    // 30: div r0.xy, r0.xyxx, r1.wwww
    r0.xy = ((r0.xyxx)/(r1.wwww)).xy;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 33: add r4.xyz, -r3.xyzx, r0.xxxx
    r4.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 34: mad r3.xyz, r4.xyzx, l(0.880000, 0.880000, 0.880000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.880000,0.880000,0.880000,0.000000))+(r3.xyzx)).xyz;
    // 35: max r4.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 36: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 37: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 38: mad r3.xyz, -r4.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000), r3.xyzx
    r3.xyz = ((-(r4.xyzx))*(float4(4.000000,4.000000,4.000000,0.000000))+(r3.xyzx)).xyz;
    // 39: mul r4.xyz, r4.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000))).xyz;
    // 40: mad_sat r0.xyz, |r0.zzzz|, r3.xyzx, r4.xyzx
    r0.xyz = (saturate((abs(r0.zzzz))*(r3.xyzx)+(r4.xyzx))).xyz;
    // 41: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 42: mul r1.w, |r0.w|, |r0.w|
    r1.w = ((abs(r0.wwww))*(abs(r0.wwww))).w;
    // 43: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 44: mul r1.w, |r0.w|, r1.w
    r1.w = ((abs(r0.wwww))*(r1.wwww)).w;
    // 45: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 46: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 47: mul r3.xyz, r0.wwww, cb0[7].xyzx
    r3.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 48: mul r0.w, r0.w, l(5.000000)
    r0.w = ((r0.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 49: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mad r0.xyz, r0.xyzx, cb0[6].xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(source[6].xyzx)+(r3.xyzx)).xyz;
    // 51: dp3 r1.w, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 52: add r3.xyz, -r2.xywx, r1.wwww
    r3.xyz = ((-(r2.xywx))+(r1.wwww)).xyz;
    // 53: mad r2.xyz, cb0[10].zzzz, r3.xyzx, r2.xywx
    r2.xyz = ((source[10].zzzz)*(r3.xyzx)+(r2.xywx)).xyz;
    // 54: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 55: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 56: mul r2.xyz, r2.xyzx, cb0[10].wwww
    r2.xyz = ((r2.xyzx)*(source[10].wwww)).xyz;
    // 57: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 58: mad r0.xyz, r2.xyzx, cb0[4].xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(source[4].xyzx)+(r0.xyzx)).xyz;
    // 59: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 60: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 61: log r0.x, r0.w
    r0.x = (log2(r0.wwww)).x;
    // 62: lt r0.y, r0.w, l(0.000001)
    r0.y = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 63: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 64: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 65: mov_sat r0.z, cb0[1].w
    r0.z = (saturate(source[1].wwww)).z;
    // 66: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 67: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 68: movc o0.w, r0.y, l(0), r0.x
    output.targets[0].w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 69: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 70: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 71: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 72: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 73: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 74: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 75: mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // 76: mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 77: dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 78: dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 79: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 80: dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 81: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 82: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 83: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 84: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 85: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 86: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 87: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 88: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 89: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 90: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 91: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 92: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 93: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 94: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 95: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 96: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 97: ret
    return output;
}

// source.character.static-map-native-1507.v1 / source program 0ce8e5f54aa7754e98b17ea081d356ec
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1507(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=SourceCharacterAppend((g_SourceCharacterBaseConstants[27]*g_SourceCharacterTime.xxxx),(g_SourceCharacterBaseConstants[28]*g_SourceCharacterTime.xxxx),1u);
    source[3]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0.00999999978,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.00999999978,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(0.0299999993,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.0299999993,0,0,0))))),1u);
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=SourceCharacterAppend((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(1,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(1,0,0,0))))),(sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(-0.230000004,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(-0.230000004,0,0,0))))),1u);
    source[8]=SourceCharacterAppend((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(-0.5,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(-0.5,0,0,0))))),(sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(0.370000005,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(0.370000005,0,0,0))))),1u);
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(0.125,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.125,0,0,0))))),1u);
    source[11]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(0.174999997,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.174999997,0,0,0))))),1u);
    source[12]=g_SourceCharacterBaseConstants[11];
    source[13]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(-0.5,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(-0.5,0,0,0))))),1u);
    source[14]=g_SourceCharacterBaseConstants[14];
    source[15]=g_SourceCharacterBaseConstants[20];
    source[15].x=((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(0.370000005,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(0.370000005,0,0,0)))))).x;
    source[15].y=((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(-0.5,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[29])*float4(-0.5,0,0,0)))))).x;
    source[16]=g_SourceCharacterBaseConstants[21];
    source[17]=g_SourceCharacterBaseConstants[22];
    source[17].x=((g_SourceCharacterTime.xxxx*float4(0,0,0,0))).x;
    source[17].y=((g_SourceCharacterTime.xxxx*float4(0.125,0,0,0))).x;
    source[18]=g_SourceCharacterBaseConstants[23];
    source[19]=g_SourceCharacterBaseConstants[24];
    source[19].z=((g_SourceCharacterBaseConstants[30]*g_SourceCharacterTime.xxxx)).x;
    source[19].w=((sign((g_SourceCharacterTime.xxxx*float4(0.125,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.125,0,0,0)))))).x;
    source[20]=g_SourceCharacterBaseConstants[25];
    source[20].x=((sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))))).x;
    source[20].y=((g_SourceCharacterTime.xxxx*float4(0.174999997,0,0,0))).x;
    source[20].z=((sign((g_SourceCharacterTime.xxxx*float4(0.174999997,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.174999997,0,0,0)))))).x;
    source[21]=g_SourceCharacterBaseConstants[26];
    source[21].y=((g_SourceCharacterTime.xxxx*float4(-0.5,0,0,0))).x;
    source[21].z=((sign((g_SourceCharacterTime.xxxx*float4(-0.5,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(-0.5,0,0,0)))))).x;
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, v4.yxyy, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.yxyy)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 22: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 24: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 25: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 26: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 27: mul r0.y, r0.y, cb0[17].z
    r0.y = ((r0.yyyy)*(source[17].zzzz)).y;
    // 28: add r0.z, -r0.x, l(0.500000)
    r0.z = ((-(r0.xxxx))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: dp2 r0.z, r0.zzzz, cb0[17].wwww
    r0.z = (dot((r0.zzzz).xy,(source[17].wwww).xy).xxxx).z;
    // 30: mad r0.y, r0.y, l(0.318310), r0.z
    r0.y = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(r0.zzzz)).y;
    // 31: add r0.y, r0.y, cb0[18].x
    r0.y = ((r0.yyyy)+(source[18].xxxx)).y;
    // 32: mul r1.x, r0.y, cb0[18].w
    r1.x = ((r0.yyyy)*(source[18].wwww)).x;
    // 33: add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // 34: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 35: mul r0.y, r0.y, cb0[18].y
    r0.y = ((r0.yyyy)*(source[18].yyyy)).y;
    // 36: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 37: lt r0.z, r0.x, l(0.000000)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 38: mad r0.x, -r0.x, l(2.000000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 40: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 41: add r0.z, cb0[18].z, l(1.000000)
    r0.z = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: mad r0.y, r0.z, l(-0.400000), r0.y
    r0.y = ((r0.zzzz)*(float4(-0.400000,-0.400000,-0.400000,-0.400000))+(r0.yyyy)).y;
    // 43: mad r1.y, r0.y, cb0[19].x, cb0[19].z
    r1.y = ((r0.yyyy)*(source[19].xxxx)+(source[19].zzzz)).y;
    // 44: add r0.yz, r1.xxyx, cb0[10].xxyx
    r0.yz = ((r1.xxyx)+(source[10].xxyx)).yz;
    // 45: add r1.xy, r1.xyxx, cb0[11].xyxx
    r1.xy = ((r1.xyxx)+(source[11].xyxx)).xy;
    // 46: sample_l_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s5, l(-1.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterSampler, (r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x)).xyzw).xyz;
    // 47: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t4.wxyz, s4, l(-1.000000)
    r0.yzw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterSampler, (r0.yzyy).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x)).wxyz).yzw;
    // 48: mul r2.xyz, r1.xyzx, r0.yzwy
    r2.xyz = ((r1.xyzx)*(r0.yzwy)).xyz;
    // 49: add r0.yzw, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)+(r0.yyzw)).yzw;
    // 50: max r1.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 51: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 52: mul r1.xyz, r1.xyzx, cb0[20].wwww
    r1.xyz = ((r1.xyzx)*(source[20].wwww)).xyz;
    // 53: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 54: add r2.xy, v4.xyxx, -cb0[4].xyxx
    r2.xy = ((v4.xyxx)+(-(source[4].xyxx))).xy;
    // 55: max r1.w, |r2.y|, |r2.x|
    r1.w = (max(abs(r2.yyyy),abs(r2.xxxx))).w;
    // 56: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 57: min r2.z, |r2.y|, |r2.x|
    r2.z = (min(abs(r2.yyyy),abs(r2.xxxx))).z;
    // 58: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 59: mul r2.z, r1.w, r1.w
    r2.z = ((r1.wwww)*(r1.wwww)).z;
    // 60: mad r2.w, r2.z, l(0.020835), l(-0.085133)
    r2.w = ((r2.zzzz)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).w;
    // 61: mad r2.w, r2.z, r2.w, l(0.180141)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(0.180141,0.180141,0.180141,0.180141))).w;
    // 62: mad r2.w, r2.z, r2.w, l(-0.330299)
    r2.w = ((r2.zzzz)*(r2.wwww)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).w;
    // 63: mad r2.z, r2.z, r2.w, l(0.999866)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.999866,0.999866,0.999866,0.999866))).z;
    // 64: mul r2.w, r1.w, r2.z
    r2.w = ((r1.wwww)*(r2.zzzz)).w;
    // 65: mad r2.w, r2.w, l(-2.000000), l(1.570796)
    r2.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).w;
    // 66: lt r3.x, |r2.y|, |r2.x|
    r3.x = (asfloat((uint4)((abs(r2.yyyy))<(abs(r2.xxxx))) * 0xffffffffu)).x;
    // 67: and r2.w, r2.w, r3.x
    r2.w = (asfloat(asuint(r2.wwww) & asuint(r3.xxxx))).w;
    // 68: mad r1.w, r1.w, r2.z, r2.w
    r1.w = ((r1.wwww)*(r2.zzzz)+(r2.wwww)).w;
    // 69: lt r2.z, r2.y, -r2.y
    r2.z = (asfloat((uint4)((r2.yyyy)<(-(r2.yyyy))) * 0xffffffffu)).z;
    // 70: and r2.z, r2.z, l(0xc0490fdb)
    r2.z = (asfloat(asuint(r2.zzzz) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).z;
    // 71: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 72: min r2.z, r2.y, r2.x
    r2.z = (min(r2.yyyy,r2.xxxx)).z;
    // 73: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 74: ge r2.x, r2.x, -r2.x
    r2.x = (asfloat((uint4)((r2.xxxx)>=(-(r2.xxxx))) * 0xffffffffu)).x;
    // 75: lt r2.y, r2.z, -r2.z
    r2.y = (asfloat((uint4)((r2.zzzz)<(-(r2.zzzz))) * 0xffffffffu)).y;
    // 76: and r2.x, r2.x, r2.y
    r2.x = (asfloat(asuint(r2.xxxx) & asuint(r2.yyyy))).x;
    // 77: movc r1.w, r2.x, -r1.w, r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (-(r1.wwww)) : (r1.wwww)).w;
    // 78: mad r2.x, r1.w, l(0.159155), l(0.500000)
    r2.x = ((r1.wwww)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 79: mad r3.xy, v4.xyxx, cb0[6].xyxx, cb0[8].xyxx
    r3.xy = ((v4.xyxx)*(source[6].xyxx)+(source[8].xyxx)).xy;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r1.w = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 81: mad r1.w, r1.w, l(2.000000), l(-1.000000)
    r1.w = ((r1.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 82: mul r1.w, r1.w, l(0.100000)
    r1.w = ((r1.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 83: mad r3.xy, v4.xyxx, cb0[6].xyxx, cb0[7].xyxx
    r3.xy = ((v4.xyxx)*(source[6].xyxx)+(source[7].xyxx)).xy;
    // 84: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t0.yzwx, s1, l(0.000000)
    r2.w = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 85: mad r2.w, r2.w, l(2.000000), l(-1.000000)
    r2.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 86: mad r1.w, r2.w, l(0.100000), r1.w
    r1.w = ((r2.wwww)*(float4(0.100000,0.100000,0.100000,0.100000))+(r1.wwww)).w;
    // 87: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 88: add r3.xy, v4.xyxx, -cb0[5].xyxx
    r3.xy = ((v4.xyxx)+(-(source[5].xyxx))).xy;
    // 89: dp2 r2.w, r3.xyxx, r3.xyxx
    r2.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 90: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 91: mad r3.x, r2.w, l(3.000000), l(-0.500000)
    r3.x = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 92: mad r2.w, -r2.w, l(2.000000), l(1.000000)
    r2.w = ((-(r2.wwww))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: max r3.x, r3.x, l(-0.100000)
    r3.x = (max(r3.xxxx,float4(-0.100000,-0.100000,-0.100000,-0.100000))).x;
    // 94: min r2.y, r3.x, l(10.000000)
    r2.y = (min(r3.xxxx,float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 95: mad r3.xy, r2.wwww, r1.wwww, r2.xyxx
    r3.xy = ((r2.wwww)*(r1.wwww)+(r2.xyxx)).xy;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r2.yz, r3.xyxx, t1.yzxw, s2, l(-1.000000)
    r2.yz = ((g_SourceCharacterTexture1.SampleLevel(SourceCharacterSampler, (r3.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x)).yzxw).yz;
    // 97: mul r3.xyz, r0.yzwy, r2.zzzz
    r3.xyz = ((r0.yzwy)*(r2.zzzz)).xyz;
    // 98: mul r3.xyz, r3.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 99: mad r1.xyz, cb0[21].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[21].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 100: mad r2.xw, r2.xxxz, l(7.000000, 0.000000, 0.000000, 1.000000), cb0[13].xxxy
    r2.xw = ((r2.xxxz)*(float4(7.000000,0.000000,0.000000,1.000000))+(source[13].xxxy)).xw;
    // 101: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xwxx, t6.xyzw, s6, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 102: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 103: mul_sat r3.xyz, r3.xyzx, l(4.000000, 4.000000, 4.000000, 0.000000)
    r3.xyz = (saturate((r3.xyzx)*(float4(4.000000,4.000000,4.000000,0.000000)))).xyz;
    // 104: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 105: mad r1.xyz, r1.xyzx, cb0[12].xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // 106: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 107: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 108: mul r2.xw, r0.wwww, v6.xxxy
    r2.xw = ((r0.wwww)*(v6.xxxy)).xw;
    // 109: mad r0.yz, r0.yyzy, l(0.000000, 0.200000, 0.200000, 0.000000), r2.xxwx
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.200000,0.200000,0.000000))+(r2.xxwx)).yz;
    // 110: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t7.wxyz, s7, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 111: add r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)+(r1.xxyz)).yzw;
    // 112: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 113: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 114: mad r0.yzw, cb0[21].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[21].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 115: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 116: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 117: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s0, l(0.000000)
    r1.x = ((float4(0.0,0.0,0.0,0.0)).xyzw).x;
    // 118: min r1.x, r1.x, l(0.999000)
    r1.x = (min(r1.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // 119: mad r1.y, r1.x, cb2[1].z, -cb2[1].w
    r1.y = ((r1.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // 120: mad r1.x, r1.x, cb2[1].x, cb2[1].y
    r1.x = ((r1.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).x;
    // 121: div r1.y, l(1.000000, 1.000000, 1.000000, 1.000000), r1.y
    r1.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.yyyy)).y;
    // 122: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 123: add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // 124: mul_sat r1.xy, r1.xxxx, l(0.034483, 0.066667, 0.000000, 0.000000)
    r1.xy = (saturate((r1.xxxx)*(float4(0.034483,0.066667,0.000000,0.000000)))).xy;
    // 125: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 126: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 127: lt r1.y, r1.y, l(0.000001)
    r1.y = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 128: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 129: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 130: mul r3.xyz, r1.zzzz, l(10.000000, 10.000000, 20.000000, 0.000000)
    r3.xyz = ((r1.zzzz)*(float4(10.000000,10.000000,20.000000,0.000000))).xyz;
    // 131: movc r1.yzw, r1.yyyy, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 132: mad r0.yzw, r2.yyyy, r0.yyzw, r1.yyzw
    r0.yzw = ((r2.yyyy)*(r0.yyzw)+(r1.yyzw)).yzw;
    // 133: mul r0.yzw, r0.yyzw, v2.xxyz
    r0.yzw = ((r0.yyzw)*(v2.xxyz)).yzw;
    // 134: add r1.y, -r2.y, l(1.000000)
    r1.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 135: add_sat r1.z, r1.y, r2.z
    r1.z = (saturate((r1.yyyy)+(r2.zzzz))).z;
    // 136: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 137: mul_sat r1.x, r1.x, v2.w
    r1.x = (saturate((r1.xxxx)*(v2.wwww))).x;
    // 138: mul o0.w, r1.x, cb0[0].x
    output.targets[0].w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 139: log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // 140: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 141: mul r1.x, r1.x, cb0[14].x
    r1.x = ((r1.xxxx)*(source[14].xxxx)).x;
    // 142: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 143: mul r1.x, r1.x, cb0[14].z
    r1.x = ((r1.xxxx)*(source[14].zzzz)).x;
    // 144: add r1.zw, -v4.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((-(v4.xxxy))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 145: mul r1.xz, r1.zzwz, r1.xxxx
    r1.xz = ((r1.zzwz)*(r1.xxxx)).xz;
    // 146: movc r1.xz, r0.xxxx, l(0,0,0,0), r1.xxzx
    r1.xz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxzx)).xz;
    // 147: add r1.xz, r1.xxzx, v4.xxyx
    r1.xz = ((r1.xxzx)+(v4.xxyx)).xz;
    // 148: mad r1.xz, cb0[14].wwww, r1.xxzx, cb0[2].xxyx
    r1.xz = ((source[14].wwww)*(r1.xxzx)+(source[2].xxyx)).xz;
    // 149: add r1.xz, r1.xxzx, cb0[3].xxyx
    r1.xz = ((r1.xxzx)+(source[3].xxyx)).xz;
    // 150: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s1, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 151: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 152: mul r2.yz, r1.yyyy, r2.yyzy
    r2.yz = ((r1.yyyy)*(r2.yyzy)).yz;
    // 153: mad r1.xz, cb0[15].wwww, r2.yyzy, r1.xxzx
    r1.xz = ((source[15].wwww)*(r2.yyzy)+(r1.xxzx)).xz;
    // 154: mad r0.x, cb0[16].x, l(0.500000), l(-0.250000)
    r0.x = ((source[16].xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(-0.250000,-0.250000,-0.250000,-0.250000))).x;
    // 155: mad r1.xz, r0.xxxx, r2.xxwx, r1.xxzx
    r1.xz = ((r0.xxxx)*(r2.xxwx)+(r1.xxzx)).xz;
    // 156: sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r1.xzxx, t3.xwyz, s3, l(0.000000)
    r1.xzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // 157: max r1.xzw, |r1.xxzw|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r1.xzw = (max(abs(r1.xxzw),float4(0.000001,0.000000,0.000001,0.000001))).xzw;
    // 158: log r1.xzw, r1.xxzw
    r1.xzw = (log2(r1.xxzw)).xzw;
    // 159: mul r1.xzw, r1.xxzw, cb0[16].yyyy
    r1.xzw = ((r1.xxzw)*(source[16].yyyy)).xzw;
    // 160: exp r1.xzw, r1.xxzw
    r1.xzw = (exp2(r1.xxzw)).xzw;
    // 161: mul r2.xyz, r1.xzwx, cb0[16].zzzz
    r2.xyz = ((r1.xzwx)*(source[16].zzzz)).xyz;
    // 162: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 163: mad r1.xzw, -cb0[16].zzzz, r1.xxzw, r0.xxxx
    r1.xzw = ((-(source[16].zzzz))*(r1.xxzw)+(r0.xxxx)).xzw;
    // 164: mad r1.xzw, cb0[16].wwww, r1.xxzw, r2.xxyz
    r1.xzw = ((source[16].wwww)*(r1.xxzw)+(r2.xxyz)).xzw;
    // 165: mul r1.xzw, r1.xxzw, cb0[9].xxyz
    r1.xzw = ((r1.xxzw)*(source[9].xxyz)).xzw;
    // 166: mad r0.xyz, r1.yyyy, r1.xzwx, r0.yzwy
    r0.xyz = ((r1.yyyy)*(r1.xzwx)+(r0.yzwy)).xyz;
    // 167: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 168: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 169: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 170: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 171: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 172: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 173: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 174: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 175: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 176: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 177: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 178: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 179: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 180: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 181: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 182: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 183: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 184: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 185: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 186: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 187: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 188: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 189: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 190: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 191: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 192: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 193: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 194: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 195: ret
    return output;
}

// source.character.static-map-native-1508.v1 / source program 19b23b9abdcb55418e084692eb0efb82
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1508(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=SourceCharacterAppend((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[15])*float4(1,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[15])*float4(1,0,0,0))))),(sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[15])*float4(0,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[15])*float4(0,0,0,0))))),1u);
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[7];
    source[8]=g_SourceCharacterBaseConstants[9];
    source[8].x=(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[15])*float4(0,0,0,0))).x;
    source[8].z=((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[15])*float4(0,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[15])*float4(0,0,0,0)))))).x;
    source[8].w=((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[15])*float4(1,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[15])*float4(1,0,0,0)))))).x;
    source[9]=g_SourceCharacterBaseConstants[10];
    source[10]=g_SourceCharacterBaseConstants[12];
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t4.xyzw, s2, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 4: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 5: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 6: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 7: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: dp3 r0.x, r0.xyzx, r0.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 9: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 10: div r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)/(r0.xxxx)).x;
    // 11: max r0.x, r0.x, l(0.200000)
    r0.x = (max(r0.xxxx,float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 12: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v4.xyxx, t5.wxyz, s3, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 14: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 15: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 16: mad r0.yzw, cb0[9].xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((source[9].xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 17: mul r1.xyz, cb0[5].xyzx, cb0[5].wwww
    r1.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 18: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 19: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 20: mad r1.xy, cb0[8].yyyy, v4.xyxx, cb0[3].xyxx
    r1.xy = ((source[8].yyyy)*(v4.xyxx)+(source[3].xyxx)).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 22: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 23: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 24: add r2.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 25: mad r2.zw, r2.xxxy, cb0[2].xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r2.xxxy)*(source[2].xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 26: dp2 r0.w, cb0[6].xyxx, r2.xyxx
    r0.w = (dot((source[6].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 27: add r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r2.zwzz, t2.xzwy, s0, l(0.000000)
    r1.w = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).w;
    // 29: mov_sat r2.xy, v2.xyxx
    r2.xy = (saturate(v2.xyxx)).xy;
    // 30: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 31: add r1.w, r1.w, -r2.y
    r1.w = ((r1.wwww)+(-(r2.yyyy))).w;
    // 32: mul_sat r1.w, r1.w, cb0[7].z
    r1.w = (saturate((r1.wwww)*(source[7].zzzz))).w;
    // 33: log r2.y, r1.w
    r2.y = (log2(r1.wwww)).y;
    // 34: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: mul r2.y, r2.y, cb0[7].w
    r2.y = ((r2.yyyy)*(source[7].wwww)).y;
    // 36: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 37: movc r1.w, r1.w, l(0), r2.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).w;
    // 38: mad r0.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 39: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 41: mul r0.xy, v4.xyxx, cb0[9].zzzz
    r0.xy = ((v4.xyxx)*(source[9].zzzz)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s5, l(0.000000)
    r0.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 43: mul r0.x, r0.x, cb0[9].w
    r0.x = ((r0.xxxx)*(source[9].wwww)).x;
    // 44: max r0.x, r0.x, l(0.010000)
    r0.x = (max(r0.xxxx,float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 45: min r0.x, r0.x, l(0.950000)
    r0.x = (min(r0.xxxx,float4(0.950000,0.950000,0.950000,0.950000))).x;
    // 46: add r0.y, -r0.x, r0.w
    r0.y = ((-(r0.xxxx))+(r0.wwww)).y;
    // 47: mad r0.x, cb0[10].z, r0.y, r0.x
    r0.x = ((source[10].zzzz)*(r0.yyyy)+(r0.xxxx)).x;
    // 48: add r0.x, -r2.x, r0.x
    r0.x = ((-(r2.xxxx))+(r0.xxxx)).x;
    // 49: mul_sat r0.x, r0.x, cb0[10].w
    r0.x = (saturate((r0.xxxx)*(source[10].wwww))).x;
    // 50: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 51: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 52: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 53: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 54: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t0.yxzw, s4, l(0.000000)
    r0.y = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 56: mul_sat r0.y, r0.y, cb0[9].y
    r0.y = (saturate((r0.yyyy)*(source[9].yyyy))).y;
    // 57: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 58: mad_sat r0.x, r0.x, v2.w, cb0[12].x
    r0.x = (saturate((r0.xxxx)*(v2.wwww)+(source[12].xxxx))).x;
    // 59: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 60: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 61: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 62: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 63: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 66: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 67: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 68: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 69: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 70: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 71: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 72: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 73: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 74: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 75: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 76: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 77: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 78: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 79: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 80: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 81: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 82: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 83: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 84: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 85: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 86: ret
    return output;
}

// source.character.static-map-native-1509.v1 / source program 325809c48976ff43960c811a1a466739
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1509(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7]=g_SourceCharacterBaseConstants[7];
    source[8]=(g_SourceCharacterTime.xxxx*SourceCharacterAppend(g_SourceCharacterBaseConstants[16],float4(0,0,0,0),1u));
    source[9]=g_SourceCharacterBaseConstants[10];
    source[10]=g_SourceCharacterBaseConstants[11];
    source[11]=g_SourceCharacterBaseConstants[12];
    source[12]=g_SourceCharacterBaseConstants[13];
    source[13]=g_SourceCharacterBaseConstants[14];
    source[13].z=(g_SourceCharacterTime.xxxx).x;
    source[14]=g_SourceCharacterBaseConstants[15];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: add r0.xy, -v4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v4.xyxx
    r0.xy = ((r0.xyxx)*(v4.xyxx)).xy;
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
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).x;
    // 12: add r0.x, r0.x, -cb0[10].w
    r0.x = ((r0.xxxx)+(-(source[10].wwww))).x;
    // 13: mul r0.x, r0.x, cb0[11].x
    r0.x = ((r0.xxxx)*(source[11].xxxx)).x;
    // 14: mad r0.x, r0.x, l(0.050000), l(-0.025000)
    r0.x = ((r0.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).x;
    // 15: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 16: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 17: mul r0.yz, r0.yyyy, v6.xxyx
    r0.yz = ((r0.yyyy)*(v6.xxyx)).yz;
    // 18: mul r1.xy, r0.yzyy, r0.xxxx
    r1.xy = ((r0.yzyy)*(r0.xxxx)).xy;
    // 19: mad r1.zw, v4.xxxy, cb0[3].xxxy, r1.xxxy
    r1.zw = ((v4.xxxy)*(source[3].xxxy)+(r1.xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t3.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 21: mad r2.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 23: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 25: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 26: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 28: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 29: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 30: mad r1.zw, cb0[11].zzzz, v4.xxxy, r1.xxxy
    r1.zw = ((source[11].zzzz)*(v4.xxxy)+(r1.xxxy)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.zwzz, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mad r1.zw, r2.xxxy, cb0[4].xxxy, r3.xxxy
    r1.zw = ((r2.xxxy)*(source[4].xxxy)+(r3.xxxy)).zw;
    // 33: mul r0.w, r2.z, cb0[4].z
    r0.w = ((r2.zzzz)*(source[4].zzzz)).w;
    // 34: max r0.w, r0.w, l(0.150000)
    r0.w = (max(r0.wwww,float4(0.150000,0.150000,0.150000,0.150000))).w;
    // 35: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: mul r1.zw, r1.zzzw, cb0[11].wwww
    r1.zw = ((r1.zzzw)*(source[11].wwww)).zw;
    // 37: mad r1.zw, -r0.yyyz, cb0[2].xxxy, r1.zzzw
    r1.zw = ((-(r0.yyyz))*(source[2].xxxy)+(r1.zzzw)).zw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.zwzz, t4.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 39: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 40: add r4.xyz, -r2.xyzx, r1.zzzz
    r4.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 41: mad r2.xyz, cb0[12].xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((source[12].xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 42: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 43: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 44: mul r2.xyz, r2.xyzx, cb0[12].yyyy
    r2.xyz = ((r2.xyzx)*(source[12].yyyy)).xyz;
    // 45: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 46: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 47: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 48: mul r2.xyz, r2.xyzx, r1.zzzz
    r2.xyz = ((r2.xyzx)*(r1.zzzz)).xyz;
    // 49: dp3 r1.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 50: add r4.xyz, -r3.xyzx, r1.zzzz
    r4.xyz = ((-(r3.xyzx))+(r1.zzzz)).xyz;
    // 51: mad r4.xyz, cb0[12].zzzz, r4.xyzx, r3.xyzx
    r4.xyz = ((source[12].zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 52: mul r5.xyz, cb0[6].xyzx, cb0[6].wwww
    r5.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 53: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 54: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 55: add r1.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r1.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 56: mad r1.xy, r1.zwzz, cb0[7].xyxx, r1.xyxx
    r1.xy = ((r1.zwzz)*(source[7].xyxx)+(r1.xyxx)).xy;
    // 57: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t5.xzwy, s4, l(0.000000)
    r0.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).w;
    // 59: add r1.x, -r0.w, l(1.000000)
    r1.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 60: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 61: mul r1.xyz, r4.xyzx, r1.xxxx
    r1.xyz = ((r4.xyzx)*(r1.xxxx)).xyz;
    // 62: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 63: mad r1.xyz, r2.xyzx, r4.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 64: mul r1.xyz, r1.xyzx, v2.xyzx
    r1.xyz = ((r1.xyzx)*(v2.xyzx)).xyz;
    // 65: mad r2.xy, r3.xyxx, l(0.200000, 0.200000, 0.000000, 0.000000), v4.xyxx
    r2.xy = ((r3.xyxx)*(float4(0.200000,0.200000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 66: mul r1.w, r3.w, cb0[14].x
    r1.w = ((r3.wwww)*(source[14].xxxx)).w;
    // 67: add r2.xy, r2.xyxx, cb0[8].xyxx
    r2.xy = ((r2.xyxx)+(source[8].xyxx)).xy;
    // 68: mad r0.xy, r0.xxxx, r0.yzyy, r2.xyxx
    r0.xy = ((r0.xxxx)*(r0.yzyy)+(r2.xyxx)).xy;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 70: add r0.y, r0.x, r0.w
    r0.y = ((r0.xxxx)+(r0.wwww)).y;
    // 71: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 72: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 73: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 74: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 75: mul r0.x, r0.x, cb0[13].w
    r0.x = ((r0.xxxx)*(source[13].wwww)).x;
    // 76: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 77: mul r2.xyz, cb0[9].xyzx, cb0[9].wwww
    r2.xyz = ((source[9].xyzx)*(source[9].wwww)).xyz;
    // 78: mul r0.xzw, r0.xxxx, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)).xzw;
    // 79: mul r0.xzw, r0.xxzw, v2.xxyz
    r0.xzw = ((r0.xxzw)*(v2.xxyz)).xzw;
    // 80: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 81: mad r0.xyz, r1.xyzx, l(0.150000, 0.150000, 0.150000, 0.000000), r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.150000,0.150000,0.150000,0.000000))+(r0.xyzx)).xyz;
    // 82: mul r1.xyz, r1.xyzx, cb2[3].wwww
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)).xyz;
    // 83: mad r1.xyz, r1.xyzx, l(0.850000, 0.850000, 0.850000, 0.000000), cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.850000,0.850000,0.850000,0.000000))+(passValues[3].xyzx)).xyz;
    // 84: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 85: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 86: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 87: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 88: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 89: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 90: mul r2.yzw, r2.yyyy, cb0[16].xxyz
    r2.yzw = ((r2.yyyy)*(source[16].xxyz)).yzw;
    // 91: mad r2.xyz, r2.xxxx, cb0[15].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[15].xyzx)+(r2.yzwy)).xyz;
    // 92: mul r2.xyz, r2.xyzx, cb0[17].wwww
    r2.xyz = ((r2.xyzx)*(source[17].wwww)).xyz;
    // 93: mad r0.xyz, r2.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 94: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 95: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 96: mad r0.xyz, r1.xyzx, cb0[17].xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(source[17].xyzx)+(r0.xyzx)).xyz;
    // 97: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 98: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 99: log r0.x, |r1.w|
    r0.x = (log2(abs(r1.wwww))).x;
    // 100: lt r0.y, |r1.w|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 101: mul r0.x, r0.x, cb0[14].y
    r0.x = ((r0.xxxx)*(source[14].yyyy)).x;
    // 102: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 103: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 104: mul r0.yz, v4.xxyx, cb0[14].zzzz
    r0.yz = ((v4.xxyx)*(source[14].zzzz)).yz;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s5, l(0.000000)
    r0.y = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 106: mov_sat r0.z, v2.w
    r0.z = (saturate(v2.wwww)).z;
    // 107: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 108: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 109: mad r0.x, -r0.y, l(10.000000), r0.x
    r0.x = ((-(r0.yyyy))*(float4(10.000000,10.000000,10.000000,10.000000))+(r0.xxxx)).x;
    // 110: mul_sat r0.x, r0.x, v2.w
    r0.x = (saturate((r0.xxxx)*(v2.wwww))).x;
    // 111: mul r0.x, r0.x, cb0[0].z
    r0.x = ((r0.xxxx)*(source[0].zzzz)).x;
    // 112: mul r0.y, cb0[0].y, l(5.000000)
    r0.y = ((source[0].yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 113: div_sat r0.y, r0.y, cb0[0].y
    r0.y = (saturate((r0.yyyy)/(source[0].yyyy))).y;
    // 114: mul o0.w, r0.y, r0.x
    output.targets[0].w = ((r0.yyyy)*(r0.xxxx)).w;
    // 115: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 116: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 117: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 118: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 119: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 120: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 121: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 122: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 123: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 124: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 125: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 126: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 127: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 128: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 129: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 130: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 131: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 132: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 133: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 134: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 135: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 136: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 137: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 138: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 139: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 140: ret
    return output;
}

// source.character.static-map-native-1510.v1 / source program 74817e52053bd347b3fe1b50d072ab84
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1510(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
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
    // 7: mul r0.w, r0.y, r1.x
    r0.w = ((r0.yyyy)*(r1.xxxx)).w;
    // 8: mad r0.w, r0.x, r1.y, -r0.w
    r0.w = ((r0.xxxx)*(r1.yyyy)+(-(r0.wwww))).w;
    // 9: mul r0.y, r0.w, v1.w
    r0.y = ((r0.wwww)*(v1.wwww)).y;
    // 10: add r2.xyzw, v4.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r2.xyzw = ((v4.xyxy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 11: mul r1.xy, r2.xyxx, r2.xyxx
    r1.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 12: add r0.w, r1.y, r1.x
    r0.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 15: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 16: mul r0.w, r0.w, cb0[2].x
    r0.w = ((r0.wwww)*(source[2].xxxx)).w;
    // 17: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 18: mul_sat r0.w, r0.w, cb0[2].y
    r0.w = (saturate((r0.wwww)*(source[2].yyyy))).w;
    // 19: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 20: mul r1.x, r0.w, v2.w
    r1.x = ((r0.wwww)*(v2.wwww)).x;
    // 21: mul o0.w, r1.x, cb0[0].x
    output.targets[0].w = ((r1.xxxx)*(source[0].xxxx)).w;
    // 22: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 23: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 24: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r1.xyxx, t0.xyzw, s0
    r3.xyz = ((float4(0.0,0.0,0.0,0.0)).xyzw).xyz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xyz = ((float4(0.0,0.0,0.0,0.0)).xyzw).xyz;
    // 26: dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 27: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 28: mul r2.xy, r1.wwww, r2.zwzz
    r2.xy = ((r1.wwww)*(r2.zwzz)).xy;
    // 29: mad r2.zw, -r2.zzzw, r1.wwww, r2.zzzw
    r2.zw = ((-(r2.zzzw))*(r1.wwww)+(r2.zzzw)).zw;
    // 30: mad r2.xy, r2.zwzz, l(0.900000, 0.900000, 0.000000, 0.000000), r2.xyxx
    r2.xy = ((r2.zwzz)*(float4(0.900000,0.900000,0.000000,0.000000))+(r2.xyxx)).xy;
    // 31: mad r2.xy, r2.xyxx, l(-0.020000, 0.020000, 0.000000, 0.000000), l(-1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(-0.020000,0.020000,0.000000,0.000000))+(float4(-1.000000,1.000000,0.000000,0.000000))).xy;
    // 32: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 33: mov r5.xyz, r4.xyzx
    r5.xyz = (r4.xyzx).xyz;
    // 34: mov r2.zw, r1.xxxy
    r2.zw = (r1.xxxy).zw;
    // 35: mov r1.w, l(0)
    r1.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 36: loop
    [unroll] for(uint movieBlurSample=0u;movieBlurSample<3u;++movieBlurSample) {
    // 37: ge r3.w, r1.w, l(3.000000)
    r3.w = (asfloat((uint4)((r1.wwww)>=(float4(3.000000,3.000000,3.000000,3.000000))) * 0xffffffffu)).w;
    // 38: breakc_nz r3.w
    if ((asuint(r3.wwww)).x != 0u) break;
    // 39: add r2.zw, r2.xxxy, r2.zzzw
    r2.zw = ((r2.xxxy)+(r2.zzzw)).zw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.zwzz, t0.xyzw, s0, l(0.000000)
    r6.xyz = ((float4(0.0,0.0,0.0,0.0)).xyzw).xyz;
    // 41: add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // 42: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: endloop
    }
    // 44: mad r1.xyw, r5.xyxz, l(0.250000, 0.250000, 0.000000, 0.250000), -r3.xyxz
    r1.xyw = ((r5.xyxz)*(float4(0.250000,0.250000,0.000000,0.250000))+(-(r3.xyxz))).xyw;
    // 45: mul r1.xyw, r0.wwww, r1.xyxw
    r1.xyw = ((r0.wwww)*(r1.xyxw)).xyw;
    // 46: mad r1.xyw, cb0[2].zzzz, r1.xyxw, r3.xyxz
    r1.xyw = ((source[2].zzzz)*(r1.xyxw)+(r3.xyxz)).xyw;
    // 47: mad r1.xyw, v2.xyxz, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((v2.xyxz)*(r1.xyxw)+(source[1].xyxz)).xyw;
    // 48: mad o0.xyz, r1.xywx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xywx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 49: mov r0.x, r1.z
    r0.x = (r1.zzzz).x;
    // 50: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 51: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 52: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 53: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 54: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 55: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 56: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 57: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 58: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 59: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 60: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 61: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 62: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 63: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 64: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 65: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 66: ret
    return output;
}

// source.character.static-map-native-1511.v1 / source program a872becaf961ef41a03041f6da197ef2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1511(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.xy, v4.xyxx, l(0.800000, 1.000000, 0.000000, 0.000000), -cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(float4(0.800000,1.000000,0.000000,0.000000))+(-(source[2].xyxx))).xy;
    // 2: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 3: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 4: mad r0.x, -r0.x, cb0[4].y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[4].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 5: mul_sat r0.x, r0.x, cb0[5].y
    r0.x = (saturate((r0.xxxx)*(source[5].yyyy))).x;
    // 6: mad r0.x, -r0.x, r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 10: mul r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = ((r0.yyzw)*(r1.xxxx)).yzw;
    // 11: mul r0.yzw, r0.yyzw, cb0[3].xxxx
    r0.yzw = ((r0.yyzw)*(source[3].xxxx)).yzw;
    // 12: div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 13: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 14: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t0.xyzw, s0
    r1.xyz = ((float4(0.0,0.0,0.0,0.0)).xyzw).xyz;
    // 15: mad_sat r0.xyz, r0.xxxx, r0.yzwy, r1.xyzx
    r0.xyz = (saturate((r0.xxxx)*(r0.yzwy)+(r1.xyzx))).xyz;
    // 16: add r0.xyz, -r1.xyzx, r0.xyzx
    r0.xyz = ((-(r1.xyzx))+(r0.xyzx)).xyz;
    // 17: mad r0.xyz, cb0[5].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[5].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 18: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 19: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 20: mov o0.w, cb0[0].x
    output.targets[0].w = (source[0].xxxx).w;
    // 21: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 22: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 23: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 24: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 27: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 28: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 29: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 30: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 31: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 32: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 33: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 34: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 35: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 36: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 37: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 38: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 39: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 40: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 41: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 42: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 43: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 44: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 45: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 46: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 47: ret
    return output;
}

// source.character.static-map-native-1512.v1 / source program bf478233c1a2cf41af50f2bd10ca7eab
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1512(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v5.z
    r0.x = ((r0.xxxx)*(v5.zzzz)).x;
    // 4: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 6: mul r0.yzw, r0.yyyy, cb0[4].xxyz
    r0.yzw = ((r0.yyyy)*(source[4].xxyz)).yzw;
    // 7: mad r0.xyz, r0.xxxx, cb0[3].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[3].xyzx)+(r0.yzwy)).xyz;
    // 8: mul r0.xyz, r0.xyzx, cb0[5].wwww
    r0.xyz = ((r0.xyzx)*(source[5].wwww)).xyz;
    // 9: add r1.xyz, cb0[0].xyzx, cb0[1].xyzx
    r1.xyz = ((source[0].xyzx)+(source[1].xyzx)).xyz;
    // 10: add r2.xyz, -cb0[0].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[0].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 11: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 12: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 13: mad r1.xyz, r0.xyzx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 14: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 15: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 16: mad o0.xyz, r2.xyzx, cb0[5].xyzx, r1.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[5].xyzx)+(r1.xyzx)).xyz;
    // 17: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 18: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 19: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 20: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 21: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 22: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 25: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 26: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 27: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 28: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 29: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 30: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 31: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 32: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 33: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 34: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 35: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 36: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 37: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 38: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 39: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 40: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 41: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 42: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 43: ftou r0.x, cb0[2].z
    r0.x = (asfloat((uint4)(source[2].zzzz))).x;
    // 44: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 45: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 46: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 47: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 48: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 49: ret
    return output;
}

// source.character.static-map-native-1513.v1 / source program 3b3abe5b3d623749aeec90310df73939
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1513(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 2: mul r0.x, r0.x, cb0[3].x
    r0.x = ((r0.xxxx)*(source[3].xxxx)).x;
    // 3: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 4: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 5: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 6: mul r0.x, r0.x, v7.z
    r0.x = ((r0.xxxx)*(v7.zzzz)).x;
    // 7: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 8: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 9: mul r0.yzw, r0.yyyy, cb0[6].xxyz
    r0.yzw = ((r0.yyyy)*(source[6].xxyz)).yzw;
    // 10: mad r0.xyz, r0.xxxx, cb0[5].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[5].xyzx)+(r0.yzwy)).xyz;
    // 11: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 12: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 13: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 14: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 15: mad r2.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 16: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 17: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 18: mad r0.xyz, r1.xyzx, cb0[7].xyzx, r2.xyzx
    r0.xyz = ((r1.xyzx)*(source[7].xyzx)+(r2.xyzx)).xyz;
    // 19: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 20: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 21: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 22: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 23: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 24: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 27: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 28: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 29: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 30: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 31: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 32: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 33: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 34: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 35: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 36: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 37: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 38: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 39: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 40: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 41: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 42: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 43: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 44: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 45: ftou r0.x, cb0[4].z
    r0.x = (asfloat((uint4)(source[4].zzzz))).x;
    // 46: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 47: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 48: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 49: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 50: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 51: ret
    return output;
}

// source.character.static-map-native-1514.v1 / source program c9505bc5c6ebbf47bb68fef6e425d295
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1514(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: mul r0.xy, r0.xyxx, cb0[3].xxxx
    r0.xy = ((r0.xyxx)*(source[3].xxxx)).xy;
    // 5: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 6: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 9: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 11: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 12: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 13: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 15: mul_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)*(r1.wwww))).w;
    // 16: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: mul r2.xy, v4.xyxx, cb0[3].zzzz
    r2.xy = ((v4.xyxx)*(source[3].zzzz)).xy;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 19: mul r1.w, r2.w, r2.w
    r1.w = ((r2.wwww)*(r2.wwww)).w;
    // 20: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 21: max r1.w, cb0[3].y, l(0.000000)
    r1.w = (max(source[3].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 22: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 23: mul r2.w, r0.w, r1.w
    r2.w = ((r0.wwww)*(r1.wwww)).w;
    // 24: add r3.x, -v2.x, l(1.000000)
    r3.x = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: mad r2.w, r3.x, r2.w, r3.x
    r2.w = ((r3.xxxx)*(r2.wwww)+(r3.xxxx)).w;
    // 26: add r3.x, -r1.w, r2.w
    r3.x = ((-(r1.wwww))+(r2.wwww)).x;
    // 27: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 29: mad r2.w, -r1.w, r3.x, r2.w
    r2.w = ((-(r1.wwww))*(r3.xxxx)+(r2.wwww)).w;
    // 30: mul r1.w, r3.x, r1.w
    r1.w = ((r3.xxxx)*(r1.wwww)).w;
    // 31: mad_sat r0.w, r0.w, r2.w, r1.w
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r1.wwww))).w;
    // 32: mul r3.xyz, cb0[2].xyzx, cb0[4].xxxx
    r3.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // 33: mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 34: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: mad r2.xyz, -r3.xyzx, r2.xyzx, r1.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r1.wwww)).xyz;
    // 36: mad r2.xyz, cb0[4].zzzz, r2.xyzx, r4.xyzx
    r2.xyz = ((source[4].zzzz)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 37: mul r3.xyz, cb0[1].xyzx, cb0[3].wwww
    r3.xyz = ((source[1].xyzx)*(source[3].wwww)).xyz;
    // 38: mad r2.xyz, -r1.xyzx, r3.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))*(r3.xyzx)+(r2.xyzx)).xyz;
    // 39: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 40: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 41: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 42: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 43: add r2.xyz, -r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r0.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 44: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 45: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 46: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 47: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 48: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 49: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 50: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 51: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 52: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 53: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 54: mul r2.yzw, r2.yyyy, cb0[6].xxyz
    r2.yzw = ((r2.yyyy)*(source[6].xxyz)).yzw;
    // 55: mad r2.xyz, r2.xxxx, cb0[5].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[5].xyzx)+(r2.yzwy)).xyz;
    // 56: mul r2.xyz, r2.xyzx, cb0[7].wwww
    r2.xyz = ((r2.xyzx)*(source[7].wwww)).xyz;
    // 57: mad r3.xyz, r2.xyzx, r1.xyzx, cb0[0].xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)+(source[0].xyzx)).xyz;
    // 58: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 59: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 60: mad o0.xyz, r1.xyzx, cb0[7].xyzx, r3.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[7].xyzx)+(r3.xyzx)).xyz;
    // 61: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 62: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 63: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 66: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 67: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 68: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 69: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 70: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 71: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 72: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 73: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 74: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 75: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 76: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 77: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 78: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 79: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 80: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 81: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 82: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 83: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 84: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 85: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 86: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 87: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 88: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 89: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 90: ret
    return output;
}

// source.character.static-map-native-1515.v1 / source program 7a989652aa31e248a615e7af65d1fb6f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1515(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 2: mul r0.x, r0.x, cb0[2].y
    r0.x = ((r0.xxxx)*(source[2].yyyy)).x;
    // 3: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 4: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 5: mul r0.y, r0.y, cb0[2].z
    r0.y = ((r0.yyyy)*(source[2].zzzz)).y;
    // 6: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 7: mul r0.y, r0.y, cb0[2].w
    r0.y = ((r0.yyyy)*(source[2].wwww)).y;
    // 8: ge r0.y, l(1.000000), r0.y
    r0.y = (asfloat((uint4)((float4(1.000000,1.000000,1.000000,1.000000))>=(r0.yyyy)) * 0xffffffffu)).y;
    // 9: or r0.x, r0.y, r0.x
    r0.x = (asfloat(asuint(r0.yyyy) | asuint(r0.xxxx))).x;
    // 10: and r0.x, r0.x, l(0x3f800000)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).x;
    // 11: add r0.y, cb0[1].x, l(-1.000000)
    r0.y = ((source[1].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 12: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 13: mad r0.yz, cb0[1].xxxx, v4.xxyx, -r0.yyyy
    r0.yz = ((source[1].xxxx)*(v4.xxyx)+(-(r0.yyyy))).yz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 15: mul r2.xyzw, r1.wxyz, cb0[1].wyyy
    r2.xyzw = ((r1.wxyz)*(source[1].wyyy)).xyzw;
    // 16: log r0.y, |r2.x|
    r0.y = (log2(abs(r2.xxxx))).y;
    // 17: mul r0.y, r0.y, cb0[2].x
    r0.y = ((r0.yyyy)*(source[2].xxxx)).y;
    // 18: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 19: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 20: mul_sat r0.x, r0.x, v2.w
    r0.x = (saturate((r0.xxxx)*(v2.wwww))).x;
    // 21: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 22: lt r0.xy, r0.xyxx, l(0.000000, 0.000001, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((r0.xyxx)<(float4(0.000000,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 23: lt r0.z, |r2.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 24: or r0.y, r0.z, r0.y
    r0.y = (asfloat(asuint(r0.zzzz) | asuint(r0.yyyy))).y;
    // 25: or r0.x, r0.x, r0.y
    r0.x = (asfloat(asuint(r0.xxxx) | asuint(r0.yyyy))).x;
    // 26: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 27: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 28: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 29: mad r0.xyz, -r1.xyzx, cb0[1].yyyy, r0.xxxx
    r0.xyz = ((-(r1.xyzx))*(source[1].yyyy)+(r0.xxxx)).xyz;
    // 30: mad r0.xyz, cb0[1].zzzz, r0.xyzx, r2.yzwy
    r0.xyz = ((source[1].zzzz)*(r0.xyzx)+(r2.yzwy)).xyz;
    // 31: mad o0.xyz, r0.xyzx, v2.xyzx, cb0[0].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v2.xyzx)+(source[0].xyzx)).xyz;
    // 32: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 33: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 34: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 35: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 36: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 37: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 38: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 39: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 40: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 41: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 42: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 43: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 44: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 45: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 46: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 47: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 48: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 49: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 50: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 51: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 52: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 53: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 54: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 55: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 56: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 57: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 58: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 59: ret
    return output;
}

// source.character.static-map-native-1516.v1 / source program 29f037b116fdaa43b77ca3573d14d7fa
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1516(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f;
    // 1: add r0.x, cb0[2].x, l(-1.000000)
    r0.x = ((source[2].xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mad r0.xy, cb0[2].xxxx, v4.xyxx, -r0.xxxx
    r0.xy = ((source[2].xxxx)*(v4.xyxx)+(-(r0.xxxx))).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 5: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 6: mul r1.x, r1.x, cb0[2].w
    r1.x = ((r1.xxxx)*(source[2].wwww)).x;
    // 7: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 8: mul r1.x, r1.x, v2.w
    r1.x = ((r1.xxxx)*(v2.wwww)).x;
    // 9: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 10: movc r0.w, r0.w, l(0), |r1.x|
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).w;
    // 11: min r1.x, r0.w, l(1.000000)
    r1.x = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 13: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 14: movc o0.w, r0.w, l(0), r1.x
    output.targets[0].w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 15: mul r1.xyz, r0.xyzx, cb0[2].yyyy
    r1.xyz = ((r0.xyzx)*(source[2].yyyy)).xyz;
    // 16: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: mad r0.xyz, -cb0[2].yyyy, r0.xyzx, r0.wwww
    r0.xyz = ((-(source[2].yyyy))*(r0.xyzx)+(r0.wwww)).xyz;
    // 18: mad r0.xyz, cb0[2].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[2].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 19: mad r0.xyz, r0.xyzx, v2.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v2.xyzx)+(source[1].xyzx)).xyz;
    // 20: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 21: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 22: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 23: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 24: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 27: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 28: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 29: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 30: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 31: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 32: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 33: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 34: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 35: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 36: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 37: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 38: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 39: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 40: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 41: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 42: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 43: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 44: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 45: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 46: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 47: ret
    return output;
}

// source.character.static-map-native-1517.v1 / source program e6dc2a914f9a804eba9adbe50b4f6305
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1517(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=SourceCharacterAppend((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(1,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(1,0,0,0))))),(sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(0,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(0,0,0,0))))),1u);
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[7].w=(g_SourceCharacterTime.xxxx).x;
    source[8]=g_SourceCharacterBaseConstants[7];
    source[8].x=((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])).x;
    source[8].y=(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(1,0,0,0))).x;
    source[8].z=(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(0,0,0,0))).x;
    source[9]=g_SourceCharacterBaseConstants[8];
    source[9].x=((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(0,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(0,0,0,0)))))).x;
    source[9].y=((sign(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(1,0,0,0)))*frac(abs(((g_SourceCharacterTime.xxxx*g_SourceCharacterBaseConstants[10])*float4(1,0,0,0)))))).x;
    source[10]=g_SourceCharacterBaseConstants[9];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: add r0.xy, -v4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v4.xyxx
    r0.xy = ((r0.xyxx)*(v4.xyxx)).xy;
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
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).x;
    // 12: add r0.x, r0.x, -cb0[6].x
    r0.x = ((r0.xxxx)+(-(source[6].xxxx))).x;
    // 13: mul r0.x, r0.x, cb0[6].y
    r0.x = ((r0.xxxx)*(source[6].yyyy)).x;
    // 14: mad r0.x, r0.x, l(0.050000), l(-0.025000)
    r0.x = ((r0.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).x;
    // 15: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 16: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 17: mul r0.yz, r0.yyyy, v6.xxyx
    r0.yz = ((r0.yyyy)*(v6.xxyx)).yz;
    // 18: mad r0.xy, r0.xxxx, r0.yzyy, v4.xyxx
    r0.xy = ((r0.xxxx)*(r0.yzyy)+(v4.xyxx)).xy;
    // 19: add r0.zw, r0.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r0.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 20: mad r0.zw, r0.zzzw, cb0[2].xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzw)*(source[2].xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s1, l(0.000000)
    r0.z = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).z;
    // 22: mov_sat r0.w, v2.y
    r0.w = (saturate(v2.yyyy)).w;
    // 23: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 25: mul_sat r0.z, r0.z, cb0[7].x
    r0.z = (saturate((r0.zzzz)*(source[7].xxxx))).z;
    // 26: log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // 27: lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 28: mul r0.w, r0.w, cb0[7].y
    r0.w = ((r0.wwww)*(source[7].yyyy)).w;
    // 29: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 30: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 31: mad r1.xy, cb0[8].wwww, r0.xyxx, cb0[3].xyxx
    r1.xy = ((source[8].wwww)*(r0.xyxx)+(source[3].xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 33: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 34: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 35: mad r1.xyz, r0.zzzz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 36: dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 37: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 38: mul r0.z, r0.z, v7.z
    r0.z = ((r0.zzzz)*(v7.zzzz)).z;
    // 39: mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 40: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 41: mul r2.xyz, r0.wwww, cb0[12].xyzx
    r2.xyz = ((r0.wwww)*(source[12].xyzx)).xyz;
    // 42: mad r2.xyz, r0.zzzz, cb0[11].xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(source[11].xyzx)+(r2.xyzx)).xyz;
    // 43: mul r2.xyz, r2.xyzx, cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(source[13].wwww)).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t2.zwxy, s4, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 47: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 48: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 50: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 51: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: dp3 r0.x, r0.xyzx, r0.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 53: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 54: div r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)/(r0.xxxx)).x;
    // 55: max r0.x, r0.x, l(0.200000)
    r0.x = (max(r0.xxxx,float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 56: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 58: add r0.yzw, -r3.xxyz, r0.yyyy
    r0.yzw = ((-(r3.xxyz))+(r0.yyyy)).yzw;
    // 59: mad r0.yzw, cb0[10].wwww, r0.yyzw, r3.xxyz
    r0.yzw = ((source[10].wwww)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 60: mul_sat r1.w, r3.w, cb0[9].z
    r1.w = (saturate((r3.wwww)*(source[9].zzzz))).w;
    // 61: mad_sat r1.w, r1.w, v2.w, cb0[10].z
    r1.w = (saturate((r1.wwww)*(v2.wwww)+(source[10].zzzz))).w;
    // 62: mul r1.w, r1.w, cb0[0].z
    r1.w = ((r1.wwww)*(source[0].zzzz)).w;
    // 63: mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 64: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 65: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 66: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 67: mad r1.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 68: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 69: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 70: mad r1.xyz, r0.xyzx, cb0[13].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[13].xyzx)+(r1.xyzx)).xyz;
    // 71: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 72: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 73: mul r0.x, cb0[0].y, l(5.000000)
    r0.x = ((source[0].yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 74: div_sat r0.x, r0.x, cb0[0].y
    r0.x = (saturate((r0.xxxx)/(source[0].yyyy))).x;
    // 75: mul o0.w, r0.x, r1.w
    output.targets[0].w = ((r0.xxxx)*(r1.wwww)).w;
    // 76: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 77: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 78: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 79: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 80: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 81: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 82: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 83: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 84: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 85: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 86: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 87: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 88: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 89: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 90: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 91: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 92: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 93: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 94: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 95: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 96: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 97: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 98: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 99: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 100: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 101: ret
    return output;
}

// source.character.static-map-native-1518.v1 / source program e5ff54c5c354204e951b91b524341cb3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1518(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[10]=g_SourceCharacterBaseConstants[8];
    source[11]=g_SourceCharacterBaseConstants[9];
    source[12]=g_SourceCharacterBaseConstants[10];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=g_SourceCharacterBaseConstants[12];
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[17]=g_SourceCharacterBaseConstants[15];
    source[18]=g_SourceCharacterBaseConstants[16];
    source[19]=g_SourceCharacterBaseConstants[17];
    source[20]=g_SourceCharacterBaseConstants[18];
    source[21]=g_SourceCharacterBaseConstants[19];
    source[21].y=(g_SourceCharacterTime.xxxx).x;
    source[21].w=((g_SourceCharacterBaseConstants[24]*g_SourceCharacterTime.xxxx)).x;
    source[22]=g_SourceCharacterBaseConstants[20];
    source[22].x=(((g_SourceCharacterBaseConstants[24]*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[22].y=(sin(((g_SourceCharacterBaseConstants[24]*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[22].z=((float4(1.5,0,0,0)+sin(((g_SourceCharacterBaseConstants[24]*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[22].w=(((float4(1.5,0,0,0)+sin(((g_SourceCharacterBaseConstants[24]*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 7: mul r1.xyz, v7.yyyy, cb1[1].xywx
    r1.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 8: mad r1.xyz, cb1[0].xywx, v7.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v7.xxxx)+(r1.xyzx)).xyz;
    // 9: mad r1.xyz, cb1[2].xywx, v7.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v7.zzzz)+(r1.xyzx)).xyz;
    // 10: mad r1.xyz, cb1[3].xywx, v7.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v7.wwww)+(r1.xyzx)).xyz;
    // 11: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 12: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 13: mul r1.xy, r1.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 14: deriv_rtx_coarse r1.zw, r1.xxxy
    r1.zw = (ddx_coarse(r1.xxxy)).zw;
    // 15: deriv_rty_coarse r1.xy, r1.xyxx
    r1.xy = (ddy_coarse(r1.xyxx)).xy;
    // 16: dp2 r0.x, r1.xyxx, r1.xyxx
    r0.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 17: dp2 r1.x, r1.zwzz, r1.zwzz
    r1.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 18: max r0.x, r0.x, r1.x
    r0.x = (max(r0.xxxx,r1.xxxx)).x;
    // 19: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 20: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 21: rcp r1.x, |r0.x|
    r1.x = (1.0/(abs(r0.xxxx))).x;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: log r1.z, |r1.y|
    r1.z = (log2(abs(r1.yyyy))).z;
    // 25: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 26: mul r1.z, r1.z, cb0[18].x
    r1.z = ((r1.zzzz)*(source[18].xxxx)).z;
    // 27: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 28: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 29: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 30: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 31: mul r1.z, r1.z, cb0[18].y
    r1.z = ((r1.zzzz)*(source[18].yyyy)).z;
    // 32: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 33: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 34: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 35: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 37: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 38: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 39: mul r3.xy, r1.xzxx, cb0[17].xxxx
    r3.xy = ((r1.xzxx)*(source[17].xxxx)).xy;
    // 40: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 41: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 42: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 43: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: add r1.xzw, -r3.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r3.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 45: mad r1.xzw, cb0[17].wwww, r1.xxzw, r3.xxyz
    r1.xzw = ((source[17].wwww)*(r1.xxzw)+(r3.xxyz)).xzw;
    // 46: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 47: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 48: div r1.xzw, r1.xxzw, r2.wwww
    r1.xzw = ((r1.xxzw)/(r2.wwww)).xzw;
    // 49: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 50: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 51: mul r4.xyz, r2.wwww, v0.xyzx
    r4.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 52: dp3 r5.x, r4.xyzx, r1.xzwx
    r5.x = (dot((r4.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 53: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 54: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 55: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 56: mul r7.xyz, r4.yzxy, r6.zxyz
    r7.xyz = ((r4.yzxy)*(r6.zxyz)).xyz;
    // 57: mad r7.xyz, r6.yzxy, r4.zxyz, -r7.xyzx
    r7.xyz = ((r6.yzxy)*(r4.zxyz)+(-(r7.xyzx))).xyz;
    // 58: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 59: dp3 r5.y, r7.xyzx, r1.xzwx
    r5.y = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 60: dp3 r5.z, r6.xyzx, r1.xzwx
    r5.z = (dot((r6.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 61: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 62: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 63: mul r8.xyz, r1.xxxx, v5.xyzx
    r8.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 64: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 65: dp3 r9.y, r7.xyzx, r8.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 66: dp3 r9.x, r4.xyzx, r8.xyzx
    r9.x = (dot((r4.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 67: dp3 r9.z, r6.xyzx, r8.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 68: dp3 r2.w, r5.xyzx, r9.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 69: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 70: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 71: mov r5.w, -r5.x
    r5.w = (-(r5.xxxx)).w;
    // 72: dp2 r2.w, r5.ywyy, r5.ywyy
    r2.w = (dot((r5.ywyy).xy,(r5.ywyy).xy).xxxx).w;
    // 73: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 74: div r5.xy, r5.ywyy, r2.wwww
    r5.xy = ((r5.ywyy)/(r2.wwww)).xy;
    // 75: mad r2.w, -r5.z, l(0.250000), l(0.250000)
    r2.w = ((-(r5.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 76: add r3.w, r5.z, l(1.000000)
    r3.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 78: mad r5.xy, r2.wwww, r5.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(r5.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 79: sample_l_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s3, r0.x
    r5.xyz = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r5.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 80: log r9.xyz, r5.xyzx
    r9.xyz = (log2(r5.xyzx)).xyz;
    // 81: rcp r0.x, cb0[18].z
    r0.x = (1.0/(source[18].zzzz)).x;
    // 82: mul r10.xyz, r9.xyzx, r0.xxxx
    r10.xyz = ((r9.xyzx)*(r0.xxxx)).xyz;
    // 83: mul r9.xyz, r9.xyzx, cb0[18].zzzz
    r9.xyz = ((r9.xyzx)*(source[18].zzzz)).xyz;
    // 84: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 85: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 86: mul r10.xyz, r0.xxxx, r10.xyzx
    r10.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 87: mad r9.xyz, r9.xyzx, cb0[18].zzzz, r10.xyzx
    r9.xyz = ((r9.xyzx)*(source[18].zzzz)+(r10.xyzx)).xyz;
    // 88: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 89: mul r5.xyz, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 90: add r0.x, cb0[18].z, l(1.000000)
    r0.x = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 91: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 92: dp3 r0.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 93: add r5.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r5.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 94: mad r5.xyz, r3.wwww, r5.xyzx, cb0[7].xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(source[7].xyzx)).xyz;
    // 95: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 96: mul r5.xyz, r5.xyzx, cb0[18].wwww
    r5.xyz = ((r5.xyzx)*(source[18].wwww)).xyz;
    // 97: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 98: add r9.xyz, -r2.xyzx, r0.xxxx
    r9.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 99: mad r2.xyz, cb0[17].yyyy, r9.xyzx, r2.xyzx
    r2.xyz = ((source[17].yyyy)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 100: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r9.xyz, -r2.xyzx, r0.xxxx
    r9.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 102: mad r2.xyz, cb0[17].zzzz, r9.xyzx, r2.xyzx
    r2.xyz = ((source[17].zzzz)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 103: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 104: add r9.xyz, -r2.xyzx, r0.xxxx
    r9.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 105: mul r9.xyz, r9.xyzx, cb0[19].xxxx
    r9.xyz = ((r9.xyzx)*(source[19].xxxx)).xyz;
    // 106: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 107: add r0.x, r10.y, r10.x
    r0.x = ((r10.yyyy)+(r10.xxxx)).x;
    // 108: add r0.x, r10.z, r0.x
    r0.x = ((r10.zzzz)+(r0.xxxx)).x;
    // 109: add_sat r0.x, r10.w, r0.x
    r0.x = (saturate((r10.wwww)+(r0.xxxx))).x;
    // 110: mad r2.xyz, r0.xxxx, r9.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 111: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 112: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 113: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 114: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 115: dp3 r0.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 116: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 117: mul r0.x, r0.x, cb0[20].x
    r0.x = ((r0.xxxx)*(source[20].xxxx)).x;
    // 118: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 119: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 120: mad r2.w, -r0.x, r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 122: div r2.w, cb0[20].y, r2.w
    r2.w = ((source[20].yyyy)/(r2.wwww)).w;
    // 123: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 124: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 125: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 126: dp3 r3.w, r3.xyzx, r8.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 127: mul_sat r4.w, r3.w, cb0[19].y
    r4.w = (saturate((r3.wwww)*(source[19].yyyy))).w;
    // 128: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul_sat r5.w, r8.z, cb0[19].y
    r5.w = (saturate((r8.zzzz)*(source[19].yyyy))).w;
    // 131: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: add_sat r5.w, r5.w, -cb0[19].z
    r5.w = (saturate((r5.wwww)+(-(source[19].zzzz)))).w;
    // 133: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 134: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 135: mul r6.w, r6.w, cb0[19].w
    r6.w = ((r6.wwww)*(source[19].wwww)).w;
    // 136: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 137: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 138: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 139: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 140: mul r9.xyz, r5.xyzx, r2.wwww
    r9.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 141: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r10.xyz, -r0.yzwy, r2.wwww
    r10.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 143: mad r0.yzw, cb0[17].yyyy, r10.xxyz, r0.yyzw
    r0.yzw = ((source[17].yyyy)*(r10.xxyz)+(r0.yyzw)).yzw;
    // 144: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 145: add r10.xyz, -r0.yzwy, r2.wwww
    r10.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 146: mad r0.yzw, cb0[17].zzzz, r10.xxyz, r0.yyzw
    r0.yzw = ((source[17].zzzz)*(r10.xxyz)+(r0.yyzw)).yzw;
    // 147: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 148: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 149: mad r11.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r11.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 150: mad r10.xyz, cb0[17].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 151: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 152: add r11.xyz, -r10.xyzx, r2.wwww
    r11.xyz = ((-(r10.xyzx))+(r2.wwww)).xyz;
    // 153: mad r10.xyz, cb0[17].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[17].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 154: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 155: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 156: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 157: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 158: mul r12.xyz, r0.yzwy, r10.xyzx
    r12.xyz = ((r0.yzwy)*(r10.xyzx)).xyz;
    // 159: mad r0.yzw, r10.xxyz, r0.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r0.yzw = ((r10.xxyz)*(r0.yyzw)+(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 160: mul r5.xyz, r5.xyzx, r12.xyzx
    r5.xyz = ((r5.xyzx)*(r12.xyzx)).xyz;
    // 161: mad r2.xyz, r2.xyzx, r9.xyzx, -r5.xyzx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)+(-(r5.xyzx))).xyz;
    // 162: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 164: mad r2.xyz, r2.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 165: frc r2.w, cb0[3].x
    r2.w = (frac(source[3].xxxx)).w;
    // 166: add r5.x, -r2.w, l(1.000000)
    r5.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 167: mul r5.yzw, r2.xxyz, r5.xxxx
    r5.yzw = ((r2.xxyz)*(r5.xxxx)).yzw;
    // 168: dp3 r6.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 169: mad r9.xyz, -r5.xxxx, r2.xyzx, r6.wwww
    r9.xyz = ((-(r5.xxxx))*(r2.xyzx)+(r6.wwww)).xyz;
    // 170: mad r5.xyz, cb0[17].yyyy, r9.xyzx, r5.yzwy
    r5.xyz = ((source[17].yyyy)*(r9.xyzx)+(r5.yzwy)).xyz;
    // 171: dp3 r5.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: add r9.xyz, -r5.xyzx, r5.wwww
    r9.xyz = ((-(r5.xyzx))+(r5.wwww)).xyz;
    // 173: mad r5.xyz, cb0[17].zzzz, r9.xyzx, r5.xyzx
    r5.xyz = ((source[17].zzzz)*(r9.xyzx)+(r5.xyzx)).xyz;
    // 174: dp3 r5.w, r0.yzwy, r0.yzwy
    r5.w = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).w;
    // 175: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 176: div r0.yzw, r0.yyzw, r5.wwww
    r0.yzw = ((r0.yyzw)/(r5.wwww)).yzw;
    // 177: dp3 r5.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: add r9.xyz, -r0.yzwy, r5.wwww
    r9.xyz = ((-(r0.yzwy))+(r5.wwww)).xyz;
    // 179: add r0.yzw, r0.yyzw, -r9.xxyz
    r0.yzw = ((r0.yyzw)+(-(r9.xxyz))).yzw;
    // 180: mul r9.xyz, cb0[11].xyzx, cb0[21].xxxx
    r9.xyz = ((source[11].xyzx)*(source[21].xxxx)).xyz;
    // 181: mul r9.xyz, r9.xyzx, cb0[22].wwww
    r9.xyz = ((r9.xyzx)*(source[22].wwww)).xyz;
    // 182: mul r9.xyz, r4.wwww, r9.xyzx
    r9.xyz = ((r4.wwww)*(r9.xyzx)).xyz;
    // 183: mad r10.xyz, r4.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r10.xyz = ((r4.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 184: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 185: mad r4.w, cb0[9].w, r4.w, l(1.000000)
    r4.w = ((source[9].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: mad r10.xyz, cb0[10].wwww, r10.xyzx, cb0[10].xyzx
    r10.xyz = ((source[10].wwww)*(r10.xyzx)+(source[10].xyzx)).xyz;
    // 187: mad r0.yzw, r0.yyzw, r9.xxyz, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r9.xxyz)+(r10.xxyz)).yzw;
    // 188: mad r0.yzw, r4.wwww, cb0[9].xxyz, r0.yyzw
    r0.yzw = ((r4.wwww)*(source[9].xxyz)+(r0.yyzw)).yzw;
    // 189: mad r0.yzw, r5.xxyz, r11.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 190: add r4.w, -|r8.z|, l(1.000000)
    r4.w = ((-(abs(r8.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 192: log r4.w, |r3.w|
    r4.w = (log2(abs(r3.wwww))).w;
    // 193: lt r3.w, |r3.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 194: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 195: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 196: mul r5.xyz, r4.wwww, cb0[12].xyzx
    r5.xyz = ((r4.wwww)*(source[12].xyzx)).xyz;
    // 197: movc r5.xyz, r3.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 198: add r0.yzw, r0.yyzw, r5.xxyz
    r0.yzw = ((r0.yyzw)+(r5.xxyz)).yzw;
    // 199: add r0.yzw, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)+(source[2].xxyz)).yzw;
    // 200: dp3 r3.w, r1.xzwx, r1.xzwx
    r3.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 201: sqrt r4.w, r3.w
    r4.w = (sqrt(r3.wwww)).w;
    // 202: div r1.xzw, r1.xxzw, r4.wwww
    r1.xzw = ((r1.xxzw)/(r4.wwww)).xzw;
    // 203: dp3 r1.x, r1.xzwx, r8.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 204: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 206: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 207: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 208: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 209: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 210: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 211: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 212: div_sat r1.x, r1.x, r3.w
    r1.x = (saturate((r1.xxxx)/(r3.wwww))).x;
    // 213: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 214: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 215: mad r1.xyz, r1.xxxx, r2.xyzx, -r12.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r12.xyzx))).xyz;
    // 216: mad r1.xyz, r0.xxxx, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 217: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 218: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 219: mad r1.xyz, cb0[17].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 220: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 221: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 222: mad r1.xyz, cb0[17].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 223: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 224: add r0.x, -cb0[3].w, l(1.000000)
    r0.x = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 225: mul r0.x, r0.x, cb0[21].y
    r0.x = ((r0.xxxx)*(source[21].yyyy)).x;
    // 226: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 227: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 228: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 229: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 230: mul r0.x, r0.x, r1.w
    r0.x = ((r0.xxxx)*(r1.wwww)).x;
    // 231: mad r0.x, r0.x, l(0.500000), cb0[3].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).x;
    // 232: add r1.w, -r2.w, cb0[3].x
    r1.w = ((-(r2.wwww))+(source[3].xxxx)).w;
    // 233: mul r5.z, r1.w, l(0.125000)
    r5.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 234: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 235: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 236: mul r5.y, cb0[3].y, cb0[13].y
    r5.y = ((source[3].yyyy)*(source[13].yyyy)).y;
    // 237: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 238: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 239: add r2.xy, r2.xyxx, r5.xyxx
    r2.xy = ((r2.xyxx)+(r5.xyxx)).xy;
    // 240: add r2.xy, r2.xyxx, r5.zwzz
    r2.xy = ((r2.xyxx)+(r5.zwzz)).xy;
    // 241: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 242: mul r2.xyz, r0.xxxx, r5.xyzx
    r2.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 243: mul r0.x, r2.w, r5.w
    r0.x = ((r2.wwww)*(r5.wwww)).x;
    // 244: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 245: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 246: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 247: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 248: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 249: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 250: mad r2.xy, cb0[14].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[14].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 251: mul r0.x, cb0[14].y, cb0[21].y
    r0.x = ((source[14].yyyy)*(source[21].yyyy)).x;
    // 252: mul r0.x, r0.x, l(0.628319)
    r0.x = ((r0.xxxx)*(float4(0.628319,0.628319,0.628319,0.628319))).x;
    // 253: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 254: mul r5.y, r0.x, l(0.020000)
    r5.y = ((r0.xxxx)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 255: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 256: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 257: mul r1.w, cb0[14].x, l(0.001000)
    r1.w = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 258: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 259: mad r2.xy, r1.wwww, r2.xyxx, r5.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r5.xyxx)).xy;
    // 260: dp2 r1.w, cb0[15].xyxx, r2.xyxx
    r1.w = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 261: dp2 r2.y, cb0[16].xyxx, r2.xyxx
    r2.y = (dot((source[16].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 262: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 263: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 264: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 265: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 266: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 267: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 268: mul_sat r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = (saturate((r0.xxxx)*(r2.xyzx))).xyz;
    // 269: mad r5.xyz, cb0[14].zzzz, r2.xyzx, -r1.xyzx
    r5.xyz = ((source[14].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 270: mul r2.xyz, r2.xyzx, cb0[14].zzzz
    r2.xyz = ((r2.xyzx)*(source[14].zzzz)).xyz;
    // 271: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 272: mul r0.x, r0.x, l(3.000000)
    r0.x = ((r0.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 273: mad r1.xyz, r0.xxxx, r5.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 274: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 275: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 276: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 277: mul r2.xyz, r0.xxxx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 278: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 279: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 280: mul r3.xyz, r0.xxxx, v6.xyzx
    r3.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 281: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 282: mad r3.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 283: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 284: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 285: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 286: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 287: mad r0.xyz, r3.xyzx, r1.xyzx, r0.yzwy
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 288: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 289: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 290: mad o0.xyz, r1.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 291: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 292: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 293: dp3 r0.x, r4.xyzx, r2.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 294: dp3 r0.z, r6.xyzx, r2.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 295: dp3 r0.y, r7.xyzx, r2.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 296: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 297: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 298: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 299: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 300: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 301: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 302: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 303: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 304: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 305: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 306: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 307: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 308: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 309: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 310: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 311: ret
    return output;
}

// source.character.static-map-native-1519.v1 / source program e2f757283f26754693b258b92f7e1912
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1519(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xy, r0.xxxx, v6.xyxx
    r0.xy = ((r0.xxxx)*(v6.xyxx)).xy;
    // 4: mad r0.zw, v4.xxxy, l(0.000000, 0.000000, 1.500000, 2.000000), l(0.000000, 0.000000, 0.200000, 0.200000)
    r0.zw = ((v4.xxxy)*(float4(0.000000,0.000000,1.500000,2.000000))+(float4(0.000000,0.000000,0.200000,0.200000))).zw;
    // 5: mad r0.xy, r0.xyxx, l(-1.025000, -1.025000, 0.000000, 0.000000), r0.zwzz
    r0.xy = ((r0.xyxx)*(float4(-1.025000,-1.025000,0.000000,0.000000))+(r0.zwzz)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 7: mul r0.x, r0.x, l(0.800000)
    r0.x = ((r0.xxxx)*(float4(0.800000,0.800000,0.800000,0.800000))).x;
    // 8: mul r0.yz, v4.xxyx, l(0.000000, 3.000000, 3.000000, 0.000000)
    r0.yz = ((v4.xxyx)*(float4(0.000000,3.000000,3.000000,0.000000))).yz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t1.wxyz, s1, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 10: add r0.yzw, r0.yyzw, l(0.000000, -0.800000, -0.800000, -0.800000)
    r0.yzw = ((r0.yyzw)+(float4(0.000000,-0.800000,-0.800000,-0.800000))).yzw;
    // 11: mad r0.yzw, r0.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000), l(0.000000, 0.800000, 0.800000, 0.800000)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))+(float4(0.000000,0.800000,0.800000,0.800000))).yzw;
    // 12: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 13: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 14: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 15: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 16: add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 17: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 18: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 19: mad r0.x, -r0.x, cb0[3].z, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[3].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: mul_sat r0.x, r0.x, cb0[4].z
    r0.x = (saturate((r0.xxxx)*(source[4].zzzz))).x;
    // 21: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 22: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 23: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 24: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 25: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 26: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 27: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 28: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 29: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 30: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 31: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 33: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 34: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 35: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 36: dp3 r2.y, r2.xyzx, r3.xyzx
    r2.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 37: dp3 r2.x, r1.xyzx, r3.xyzx
    r2.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 38: dp3 r2.z, r0.xyzx, r3.xyzx
    r2.z = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 39: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 40: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 41: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 42: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 43: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 44: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 45: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 46: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 47: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 48: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 49: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 50: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 51: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 52: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 53: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 54: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 55: ret
    return output;
}

// source.character.static-map-native-1520.v1 / source program 6e7b9a08f01bef4ca32ebf014794bb5f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1520(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[6].w=(g_SourceCharacterTime.xxxx).x;
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[12]=g_SourceCharacterBaseConstants[11];
    source[13]=g_SourceCharacterBaseConstants[12];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mad r0.x, cb0[6].w, cb0[11].y, cb0[11].z
    r0.x = ((source[6].wwww)*(source[11].yyyy)+(source[11].zzzz)).x;
    // 2: sincos r0.x, r1.x, r0.x
    r0.x = (sin(r0.xxxx)).x; r1.x = (cos(r0.xxxx)).x;
    // 3: mov r2.x, -r0.x
    r2.x = (-(r0.xxxx)).x;
    // 4: mul r0.yz, v4.xxyx, cb0[8].xxyx
    r0.yz = ((v4.xxyx)*(source[8].xxyx)).yz;
    // 5: mul r0.w, cb0[6].z, cb0[6].w
    r0.w = ((source[6].zzzz)*(source[6].wwww)).w;
    // 6: mad r3.x, r0.w, cb0[7].w, r0.y
    r3.x = ((r0.wwww)*(source[7].wwww)+(r0.yyyy)).x;
    // 7: mad r3.y, r0.w, cb0[8].z, r0.z
    r3.y = ((r0.wwww)*(source[8].zzzz)+(r0.zzzz)).y;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r3.xyxx, t0.zxyw, s0, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 9: mad r0.yz, cb0[9].xxxx, r0.yyzy, v4.xxyx
    r0.yz = ((source[9].xxxx)*(r0.yyzy)+(v4.xxyx)).yz;
    // 10: add r1.yz, r0.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r0.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 11: mov r2.y, r1.x
    r2.y = (r1.xxxx).y;
    // 12: mov r2.z, r0.x
    r2.z = (r0.xxxx).z;
    // 13: dp2 r3.y, r2.zyzz, r1.yzyy
    r3.y = (dot((r2.zyzz).xy,(r1.yzyy).xy).xxxx).y;
    // 14: dp2 r3.x, r2.yxyy, r1.yzyy
    r3.x = (dot((r2.yxyy).xy,(r1.yzyy).xy).xxxx).x;
    // 15: mad r1.xy, r3.xyxx, cb0[4].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r3.xyxx)*(source[4].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 17: mul r1.xy, r0.yzyy, cb0[10].yzyy
    r1.xy = ((r0.yzyy)*(source[10].yzyy)).xy;
    // 18: mul r0.yz, r0.yyzy, cb0[7].yyzy
    r0.yz = ((r0.yyzy)*(source[7].yyzy)).yz;
    // 19: mad r1.xy, r0.wwww, cb0[10].xwxx, r1.xyxx
    r1.xy = ((r0.wwww)*(source[10].xwxx)+(r1.xyxx)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 21: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 22: mad r1.x, r0.w, cb0[7].x, r0.y
    r1.x = ((r0.wwww)*(source[7].xxxx)+(r0.yyyy)).x;
    // 23: mad r1.y, r0.w, cb0[9].y, r0.z
    r1.y = ((r0.wwww)*(source[9].yyyy)+(r0.zzzz)).y;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t3.wxyz, s1, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 25: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 26: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 27: mad r0.yzw, cb0[9].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[9].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 28: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 29: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 30: mul r0.yzw, r0.yyzw, cb0[9].wwww
    r0.yzw = ((r0.yyzw)*(source[9].wwww)).yzw;
    // 31: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 32: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 33: mad r2.xy, r0.yzyy, r1.xyxx, r0.xxxx
    r2.xy = ((r0.yzyy)*(r1.xyxx)+(r0.xxxx)).xy;
    // 34: mul_sat r0.x, r0.x, cb0[13].x
    r0.x = (saturate((r0.xxxx)*(source[13].xxxx))).x;
    // 35: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 36: mul r1.xy, r2.xyxx, cb0[12].yyyy
    r1.xy = ((r2.xyxx)*(source[12].yyyy)).xy;
    // 37: dp3 r1.z, v6.xyzx, v6.xyzx
    r1.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 38: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 39: mul r1.zw, r1.zzzz, v6.xxxy
    r1.zw = ((r1.zzzz)*(v6.xxxy)).zw;
    // 40: mad r1.xy, r1.zwzz, cb0[2].xyxx, r1.xyxx
    r1.xy = ((r1.zwzz)*(source[2].xyxx)+(r1.xyxx)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 42: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 43: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 44: mad r1.xyz, cb0[12].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 45: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 46: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 47: mul r1.xyz, r1.xyzx, cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 48: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 49: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 50: mad r0.yzw, r1.xxyz, r2.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(r2.xxyz)+(r0.yyzw)).yzw;
    // 51: mad r0.yzw, r0.yyzw, v2.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v2.xxyz)+(source[1].xxyz)).yzw;
    // 52: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 53: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 54: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 55: mul r0.y, r0.y, cb0[13].y
    r0.y = ((r0.yyyy)*(source[13].yyyy)).y;
    // 56: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 57: mul_sat r0.y, r0.y, v2.w
    r0.y = (saturate((r0.yyyy)*(v2.wwww))).y;
    // 58: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 59: movc o0.w, r0.x, l(0), r0.y
    output.targets[0].w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 60: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 61: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 62: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 63: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 66: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 67: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 68: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 69: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 70: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 71: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 72: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 73: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 74: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 75: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 76: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 77: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 78: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 79: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 80: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 81: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 82: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 83: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 84: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 85: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 86: ret
    return output;
}

// source.character.static-map-native-1521.v1 / source program 8b8f3485e4160246ab11905ba25f2a6e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1521(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[5]=g_SourceCharacterBaseConstants[6];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 7: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 8: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 10: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: mul r2.xy, r2.xyxx, cb0[4].xxxx
    r2.xy = ((r2.xyxx)*(source[4].xxxx)).xy;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 23: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 24: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 26: mul r3.xyz, cb0[3].xyzx, cb0[5].yyyy
    r3.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).zw;
    // 28: mad r4.xyz, r1.zzzz, r3.xyzx, -r1.yyyy
    r4.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 29: mul r5.xyz, r3.xyzx, r1.zzzz
    r5.xyz = ((r3.xyzx)*(r1.zzzz)).xyz;
    // 30: mad r3.xyz, r1.zzzz, r3.xyzx, -r1.xxxx
    r3.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 31: mad r3.xyz, r5.xyzx, r3.xyzx, r1.xxxx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r1.xxxx)).xyz;
    // 32: mad r1.xyz, r5.xyzx, r4.xyzx, r1.yyyy
    r1.xyz = ((r5.xyzx)*(r4.xyzx)+(r1.yyyy)).xyz;
    // 33: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 34: mad r1.xyz, r3.xyzx, cb0[6].xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(source[6].xyzx)+(r1.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[8].wwww
    r1.xyz = ((r1.xyzx)*(source[8].wwww)).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 37: mul r4.xyz, cb0[1].xyzx, cb0[4].yyyy
    r4.xyz = ((source[1].xyzx)*(source[4].yyyy)).xyz;
    // 38: mad r3.xyz, r3.xyzx, r4.xyzx, cb0[0].xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)+(source[0].xyzx)).xyz;
    // 39: mad r4.xyz, cb0[4].zzzz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[4].zzzz)*(source[2].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 40: mad r4.xyz, r1.wwww, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 41: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 42: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 43: mad r3.xyz, r1.xyzx, r0.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 44: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 45: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 46: mad o0.xyz, r0.xyzx, cb0[8].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // 47: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 48: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 49: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 50: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 51: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 52: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 53: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 54: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 55: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 56: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 57: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 58: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 59: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 60: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 61: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 62: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 63: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 64: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 65: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 66: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 67: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 68: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 69: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 70: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 71: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 72: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 73: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 74: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 75: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 76: ret
    return output;
}

// source.character.static-map-native-1522.v1 / source program 2a6f65c31375d747ad24127d09857677
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1522(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 3: mul r1.xyz, cb0[3].xyzx, cb0[5].yyyy
    r1.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // 4: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 8: mul r1.xy, r1.xyxx, cb0[5].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // 9: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 10: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 13: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 17: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 18: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 19: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 20: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 23: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 24: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 26: mul r2.yzw, r2.yyyy, cb0[8].xxyz
    r2.yzw = ((r2.yyyy)*(source[8].xxyz)).yzw;
    // 27: mad r2.xyz, r2.xxxx, cb0[7].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[7].xyzx)+(r2.yzwy)).xyz;
    // 28: mul r2.xyz, r2.xyzx, cb0[9].wwww
    r2.xyz = ((r2.xyzx)*(source[9].wwww)).xyz;
    // 29: mul r3.xyz, cb0[4].xyzx, cb0[5].zzzz
    r3.xyz = ((source[4].xyzx)*(source[5].zzzz)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 32: mul_sat r0.w, r4.w, cb0[6].y
    r0.w = (saturate((r4.wwww)*(source[6].yyyy))).w;
    // 33: mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // 34: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 35: mad r0.xyz, r2.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 36: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 37: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 38: mad r0.xyz, r3.xyzx, cb0[9].xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(source[9].xyzx)+(r0.xyzx)).xyz;
    // 39: mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 41: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 42: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 43: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 44: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 45: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 46: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 47: mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // 48: mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 49: dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 50: dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 51: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 52: dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 53: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 54: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 55: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 56: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 57: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 58: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 59: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 60: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 61: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 62: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 63: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 64: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 65: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 66: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 67: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 68: ret
    return output;
}

// source.character.static-map-native-1523.v1 / source program 3789a80bb056dd4b83e1ad5837bb2707
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1523(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(0.200000003,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.200000003,0,0,0))))),1u);
    source[7]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0.200000003,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.200000003,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0))))),1u);
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=SourceCharacterAppend((sign((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))),(sign((g_SourceCharacterTime.xxxx*float4(0,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0,0,0,0))))),1u);
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[12]=g_SourceCharacterBaseConstants[11];
    source[13]=g_SourceCharacterBaseConstants[12];
    source[13].w=(g_SourceCharacterTime.xxxx).x;
    source[14]=g_SourceCharacterBaseConstants[14];
    source[15]=g_SourceCharacterBaseConstants[15];
    source[15].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[15].w=((sign((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))*frac(abs((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))))).x;
    source[16]=g_SourceCharacterBaseConstants[16];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.xy, -v4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(v4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 2: mul r0.xy, r0.xyxx, v4.xyxx
    r0.xy = ((r0.xyxx)*(v4.xyxx)).xy;
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
    // 11: add r0.xy, v8.xyxx, cb0[0].xyxx
    r0.xy = ((v8.xyxx)+(source[0].xyxx)).xy;
    // 12: mul r0.zw, r0.xxxy, cb0[10].zzzz
    r0.zw = ((r0.xxxy)*(source[10].zzzz)).zw;
    // 13: mad r0.xy, cb0[10].zzzz, r0.xyxx, cb0[6].xyxx
    r0.xy = ((source[10].zzzz)*(r0.xyxx)+(source[6].xyxx)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t0.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 15: mad r0.xy, r0.zwzz, l(1.300000, 1.300000, 0.000000, 0.000000), l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(1.300000,1.300000,0.000000,0.000000))+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 16: dp2 r2.x, l(0.540302, -0.841471, 0.000000, 0.000000), r0.xyxx
    r2.x = (dot((float4(0.540302,-0.841471,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).x;
    // 17: dp2 r2.y, l(0.841471, 0.540302, 0.000000, 0.000000), r0.xyxx
    r2.y = (dot((float4(0.841471,0.540302,0.000000,0.000000)).xy,(r0.xyxx).xy).xxxx).y;
    // 18: add r0.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 19: mul r2.xy, r0.xyxx, cb0[10].wwww
    r2.xy = ((r0.xyxx)*(source[10].wwww)).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t3.xyzw, s0, l(0.000000)
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
    // 27: mul r3.xy, r0.zwzz, cb0[10].wwww
    r3.xy = ((r0.zwzz)*(source[10].wwww)).xy;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t3.xyzw, s0, l(0.000000)
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
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t4.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: add r1.w, r4.y, r4.x
    r1.w = ((r4.yyyy)+(r4.xxxx)).w;
    // 39: add r1.w, r4.z, r1.w
    r1.w = ((r4.zzzz)+(r1.wwww)).w;
    // 40: mul r1.w, r1.w, l(0.333330)
    r1.w = ((r1.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 41: mad r2.xyz, r1.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 42: mad r2.w, -r2.z, cb0[3].z, l(1.000000)
    r2.w = ((-(r2.zzzz))*(source[3].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)*(source[3].xyzx)).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.xyxx, t0.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.zwzz, t0.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 46: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 47: mad r3.xyz, r3.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r4.xyzx
    r3.xyz = ((r3.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r4.xyzx)).xyz;
    // 48: add r3.x, r3.y, r3.x
    r3.x = ((r3.yyyy)+(r3.xxxx)).x;
    // 49: add r3.x, r3.z, r3.x
    r3.x = ((r3.zzzz)+(r3.xxxx)).x;
    // 50: mad r3.x, r3.x, l(0.333330), l(-0.500000)
    r3.x = ((r3.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 51: mad r3.xy, cb0[12].wwww, r3.xxxx, v4.xyxx
    r3.xy = ((source[12].wwww)*(r3.xxxx)+(v4.xyxx)).xy;
    // 52: add r3.xy, r3.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 53: dp2 r3.x, r3.xyxx, r3.xyxx
    r3.x = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 54: sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // 55: mad_sat r3.x, -r3.x, cb0[13].z, l(1.000000)
    r3.x = (saturate((-(r3.xxxx))*(source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 56: add r3.yz, r0.xxyx, cb0[7].xxyx
    r3.yz = ((r0.xxyx)+(source[7].xxyx)).yz;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r3.yzw, r3.yzyy, t0.wxyz, s4, l(0.000000)
    r3.yzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 58: add r3.yzw, -r1.xxyz, r3.yyzw
    r3.yzw = ((-(r1.xxyz))+(r3.yyzw)).yzw;
    // 59: mad r1.xyz, r3.yzwy, l(0.500000, 0.500000, 0.500000, 0.000000), r1.xyzx
    r1.xyz = ((r3.yzwy)*(float4(0.500000,0.500000,0.500000,0.000000))+(r1.xyzx)).xyz;
    // 60: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 61: add r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)+(r1.xxxx)).x;
    // 62: mad r1.yz, r1.xxxx, l(0.000000, -0.016666, 0.005000, 0.000000), r3.xxxx
    r1.yz = ((r1.xxxx)*(float4(0.000000,-0.016666,0.005000,0.000000))+(r3.xxxx)).yz;
    // 63: mad r3.x, -v2.x, l(0.985000), l(1.000000)
    r3.x = ((-(v2.xxxx))*(float4(0.985000,0.985000,0.985000,0.985000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 64: add r1.yz, r1.zzyz, -r3.xxxx
    r1.yz = ((r1.zzyz)+(-(r3.xxxx))).yz;
    // 65: add r3.x, -|r1.z|, l(1.000000)
    r3.x = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: mul r2.w, r2.w, r3.x
    r2.w = ((r2.wwww)*(r3.xxxx)).w;
    // 67: mul r3.y, |r2.w|, |r2.w|
    r3.y = ((abs(r2.wwww))*(abs(r2.wwww))).y;
    // 68: mul r3.y, r3.y, r3.y
    r3.y = ((r3.yyyy)*(r3.yyyy)).y;
    // 69: mul r3.y, |r2.w|, r3.y
    r3.y = ((abs(r2.wwww))*(r3.yyyy)).y;
    // 70: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 71: movc r2.w, r2.w, l(0), r3.y
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).w;
    // 72: mul r3.yz, r0.xxyx, cb0[14].zzzz
    r3.yz = ((r0.xxyx)*(source[14].zzzz)).yz;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r3.yzw, r3.yzyy, t7.wxyz, s5, l(0.000000)
    r3.yzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 74: mul r4.xy, r0.zwzz, cb0[14].zzzz
    r4.xy = ((r0.zwzz)*(source[14].zzzz)).xy;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t7.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 76: add r3.yzw, r3.yyzw, -r4.xxyz
    r3.yzw = ((r3.yyzw)+(-(r4.xxyz))).yzw;
    // 77: mad r3.yzw, r1.wwww, r3.yyzw, r4.xxyz
    r3.yzw = ((r1.wwww)*(r3.yyzw)+(r4.xxyz)).yzw;
    // 78: add r3.y, r3.z, r3.y
    r3.y = ((r3.zzzz)+(r3.yyyy)).y;
    // 79: add r3.y, r3.w, r3.y
    r3.y = ((r3.wwww)+(r3.yyyy)).y;
    // 80: mul r3.y, r3.y, l(0.333330)
    r3.y = ((r3.yyyy)*(float4(0.333330,0.333330,0.333330,0.333330))).y;
    // 81: log r3.z, |r3.y|
    r3.z = (log2(abs(r3.yyyy))).z;
    // 82: lt r3.y, |r3.y|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 83: mul r3.z, r3.z, cb0[14].w
    r3.z = ((r3.zzzz)*(source[14].wwww)).z;
    // 84: exp r3.z, r3.z
    r3.z = (exp2(r3.zzzz)).z;
    // 85: movc r3.y, r3.y, l(0), r3.z
    r3.y = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.zzzz)).y;
    // 86: mad_sat r3.y, -r1.x, l(0.666660), r3.y
    r3.y = (saturate((-(r1.xxxx))*(float4(0.666660,0.666660,0.666660,0.666660))+(r3.yyyy))).y;
    // 87: mul r1.x, r1.x, l(0.333330)
    r1.x = ((r1.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 88: mad r1.x, -r1.x, r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 89: mad r1.x, r1.x, l(0.010000), l(0.990000)
    r1.x = ((r1.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(float4(0.990000,0.990000,0.990000,0.990000))).x;
    // 90: mad r1.x, -r1.x, r1.x, r3.x
    r1.x = ((-(r1.xxxx))*(r1.xxxx)+(r3.xxxx)).x;
    // 91: mul_sat r1.x, r1.x, l(100.000000)
    r1.x = (saturate((r1.xxxx)*(float4(100.000000,100.000000,100.000000,100.000000)))).x;
    // 92: add r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)+(r3.yyyy)).w;
    // 93: mad r3.yz, cb0[15].zzzz, r0.zzwz, cb0[9].xxyx
    r3.yz = ((source[15].zzzz)*(r0.zzwz)+(source[9].xxyx)).yz;
    // 94: sample_b_indexable(texture2d)(float,float,float,float) r3.yzw, r3.yzyy, t0.wxyz, s4, l(0.000000)
    r3.yzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 95: mad r4.xy, cb0[15].zzzz, r0.xyxx, cb0[9].xyxx
    r4.xy = ((source[15].zzzz)*(r0.xyxx)+(source[9].xyxx)).xy;
    // 96: mul r0.xy, r0.xyxx, cb0[12].xxxx
    r0.xy = ((r0.xyxx)*(source[12].xxxx)).xy;
    // 97: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t6.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t0.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 99: add r3.yzw, r3.yyzw, -r4.xxyz
    r3.yzw = ((r3.yyzw)+(-(r4.xxyz))).yzw;
    // 100: mad r3.yzw, r3.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000), r4.xxyz
    r3.yzw = ((r3.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))+(r4.xxyz)).yzw;
    // 101: add r0.x, r3.z, r3.y
    r0.x = ((r3.zzzz)+(r3.yyyy)).x;
    // 102: add r0.x, r3.w, r0.x
    r0.x = ((r3.wwww)+(r0.xxxx)).x;
    // 103: mul_sat r0.x, r0.x, l(0.333330)
    r0.x = (saturate((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330)))).x;
    // 104: mov_sat r0.y, v2.w
    r0.y = (saturate(v2.wwww)).y;
    // 105: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 106: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 107: mul_sat r0.x, r0.x, l(200.000000)
    r0.x = (saturate((r0.xxxx)*(float4(200.000000,200.000000,200.000000,200.000000)))).x;
    // 108: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 109: dp2 r0.y, r0.yyyy, r0.xxxx
    r0.y = (dot((r0.yyyy).xy,(r0.xxxx).xy).xxxx).y;
    // 110: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 111: add r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)+(r1.xxxx)).y;
    // 112: mul r3.yzw, cb0[8].xxyz, cb0[8].wwww
    r3.yzw = ((source[8].xxyz)*(source[8].wwww)).yzw;
    // 113: mul r3.yzw, r3.yyzw, v2.yyyy
    r3.yzw = ((r3.yyzw)*(v2.yyyy)).yzw;
    // 114: mul r4.xyz, r0.yyyy, r3.yzwy
    r4.xyz = ((r0.yyyy)*(r3.yzwy)).xyz;
    // 115: mul r6.xyz, r3.yzwy, cb0[15].xxxx
    r6.xyz = ((r3.yzwy)*(source[15].xxxx)).xyz;
    // 116: mul r3.yzw, r3.yyzw, l(0.000000, 0.010000, 0.010000, 0.010000)
    r3.yzw = ((r3.yyzw)*(float4(0.000000,0.010000,0.010000,0.010000))).yzw;
    // 117: mad r4.xyz, r2.wwww, r6.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 118: mul_sat r0.y, r1.z, l(500.000000)
    r0.y = (saturate((r1.zzzz)*(float4(500.000000,500.000000,500.000000,500.000000)))).y;
    // 119: mov_sat r1.y, r1.y
    r1.y = (saturate(r1.yyyy)).y;
    // 120: add r1.x, -r0.y, l(1.000000)
    r1.x = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 121: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r0.zwzz, t1.xyzw, s6, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 122: add r1.z, r6.y, r6.x
    r1.z = ((r6.yyyy)+(r6.xxxx)).z;
    // 123: add r1.z, r6.z, r1.z
    r1.z = ((r6.zzzz)+(r1.zzzz)).z;
    // 124: mad r2.w, -r1.z, l(0.333330), l(1.000000)
    r2.w = ((-(r1.zzzz))*(float4(0.333330,0.333330,0.333330,0.333330))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: mul r1.z, r1.z, l(0.333330)
    r1.z = ((r1.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))).z;
    // 126: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 127: mul_sat r1.y, r1.y, l(100.000000)
    r1.y = (saturate((r1.yyyy)*(float4(100.000000,100.000000,100.000000,100.000000)))).y;
    // 128: mad r0.y, r1.y, r1.y, r0.y
    r0.y = ((r1.yyyy)*(r1.yyyy)+(r0.yyyy)).y;
    // 129: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 130: mul r1.y, r2.w, r2.w
    r1.y = ((r2.wwww)*(r2.wwww)).y;
    // 131: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 132: mad r1.xyz, r1.xxxx, r3.yzwy, r4.xyzx
    r1.xyz = ((r1.xxxx)*(r3.yzwy)+(r4.xyzx)).xyz;
    // 133: mul r1.xyz, r1.xyzx, v2.yyyy
    r1.xyz = ((r1.xyzx)*(v2.yyyy)).xyz;
    // 134: mul r3.yz, r0.zzwz, cb0[12].xxxx
    r3.yz = ((r0.zzwz)*(source[12].xxxx)).yz;
    // 135: mul r0.zw, r0.zzzw, cb0[16].yyyy
    r0.zw = ((r0.zzzw)*(source[16].yyyy)).zw;
    // 136: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.zwzz, t2.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 137: sample_b_indexable(texture2d)(float,float,float,float) r3.yzw, r3.yzyy, t6.wxyz, s3, l(0.000000)
    r3.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 138: add r5.xyz, -r3.yzwy, r5.xyzx
    r5.xyz = ((-(r3.yzwy))+(r5.xyzx)).xyz;
    // 139: mad r3.yzw, r1.wwww, r5.xxyz, r3.yyzw
    r3.yzw = ((r1.wwww)*(r5.xxyz)+(r3.yyzw)).yzw;
    // 140: dp3 r0.z, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 141: add r5.xyz, -r3.yzwy, r0.zzzz
    r5.xyz = ((-(r3.yzwy))+(r0.zzzz)).xyz;
    // 142: mad r3.yzw, cb0[12].yyyy, r5.xxyz, r3.yyzw
    r3.yzw = ((source[12].yyyy)*(r5.xxyz)+(r3.yyzw)).yzw;
    // 143: mul r5.xyz, cb0[5].xyzx, cb0[5].wwww
    r5.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 144: mul r3.yzw, r3.yyzw, r5.xxyz
    r3.yzw = ((r3.yyzw)*(r5.xxyz)).yzw;
    // 145: mul r5.xyz, r2.zzzz, r3.yzwy
    r5.xyz = ((r2.zzzz)*(r3.yzwy)).xyz;
    // 146: add r0.z, r5.y, r5.x
    r0.z = ((r5.yyyy)+(r5.xxxx)).z;
    // 147: mad r0.z, r2.z, r3.w, r0.z
    r0.z = ((r2.zzzz)*(r3.wwww)+(r0.zzzz)).z;
    // 148: mul r2.xy, r2.xyxx, cb0[11].yyyy
    r2.xy = ((r2.xyxx)*(source[11].yyyy)).xy;
    // 149: mad r0.z, r0.z, l(0.333330), r3.x
    r0.z = ((r0.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))+(r3.xxxx)).z;
    // 150: mul r0.z, r3.x, r0.z
    r0.z = ((r3.xxxx)*(r0.zzzz)).z;
    // 151: mul_sat r0.z, r0.z, cb0[14].x
    r0.z = (saturate((r0.zzzz)*(source[14].xxxx))).z;
    // 152: lt r0.w, r0.z, l(0.000001)
    r0.w = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 153: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 154: mul r0.z, r0.z, cb0[14].y
    r0.z = ((r0.zzzz)*(source[14].yyyy)).z;
    // 155: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 156: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 157: mad r3.xyz, cb0[12].zzzz, r5.xyzx, -r5.xyzx
    r3.xyz = ((source[12].zzzz)*(r5.xyzx)+(-(r5.xyzx))).xyz;
    // 158: mad r3.xyz, r0.zzzz, r3.xyzx, r5.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 159: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 160: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 161: mul r0.zw, r0.zzzz, v6.xxxy
    r0.zw = ((r0.zzzz)*(v6.xxxy)).zw;
    // 162: mad r0.zw, cb0[10].yyyy, -r0.zzzw, r2.xxxy
    r0.zw = ((source[10].yyyy)*(-(r0.zzzw))+(r2.xxxy)).zw;
    // 163: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t5.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 164: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 165: add r5.xyz, -r2.xyzx, r0.zzzz
    r5.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 166: mad r2.xyz, cb0[11].zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((source[11].zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 167: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 168: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 169: mul r2.xyz, r2.xyzx, cb0[11].wwww
    r2.xyz = ((r2.xyzx)*(source[11].wwww)).xyz;
    // 170: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 171: mul r5.xyz, cb0[4].xyzx, cb0[4].wwww
    r5.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 172: mad r2.xyz, r2.xyzx, r5.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 173: mad r1.xyz, r2.xyzx, l(0.150000, 0.150000, 0.150000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.150000,0.150000,0.150000,0.000000))+(r1.xyzx)).xyz;
    // 174: mul r2.xyz, r2.xyzx, cb2[3].wwww
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)).xyz;
    // 175: mad r2.xyz, r2.xyzx, l(0.850000, 0.850000, 0.850000, 0.000000), cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(float4(0.850000,0.850000,0.850000,0.000000))+(passValues[3].xyzx)).xyz;
    // 176: add r1.xyz, r1.xyzx, cb0[16].xxxx
    r1.xyz = ((r1.xyzx)+(source[16].xxxx)).xyz;
    // 177: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 178: dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 179: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 180: mul r0.z, r0.z, v7.z
    r0.z = ((r0.zzzz)*(v7.zzzz)).z;
    // 181: mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 182: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 183: mul r3.xyz, r0.wwww, cb0[18].xyzx
    r3.xyz = ((r0.wwww)*(source[18].xyzx)).xyz;
    // 184: mad r3.xyz, r0.zzzz, cb0[17].xyzx, r3.xyzx
    r3.xyz = ((r0.zzzz)*(source[17].xyzx)+(r3.xyzx)).xyz;
    // 185: mul r3.xyz, r3.xyzx, cb0[19].wwww
    r3.xyz = ((r3.xyzx)*(source[19].wwww)).xyz;
    // 186: mad r1.xyz, r3.xyzx, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 187: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 188: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 189: mad r1.xyz, r2.xyzx, cb0[19].xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[19].xyzx)+(r1.xyzx)).xyz;
    // 190: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 191: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 192: add r0.z, r4.y, r4.x
    r0.z = ((r4.yyyy)+(r4.xxxx)).z;
    // 193: add r0.z, r4.z, r0.z
    r0.z = ((r4.zzzz)+(r0.zzzz)).z;
    // 194: mul r0.z, r0.z, cb0[16].z
    r0.z = ((r0.zzzz)*(source[16].zzzz)).z;
    // 195: mul r0.z, r0.z, l(0.333330)
    r0.z = ((r0.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))).z;
    // 196: mad_sat r0.x, r0.y, r0.x, -r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(-(r0.zzzz)))).x;
    // 197: mul r0.x, r0.x, cb0[16].w
    r0.x = ((r0.xxxx)*(source[16].wwww)).x;
    // 198: mul_sat r0.x, r0.x, v2.z
    r0.x = (saturate((r0.xxxx)*(v2.zzzz))).x;
    // 199: mul r0.x, r0.x, cb0[1].z
    r0.x = ((r0.xxxx)*(source[1].zzzz)).x;
    // 200: mul r0.y, cb0[1].y, l(5.000000)
    r0.y = ((source[1].yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 201: div_sat r0.y, r0.y, cb0[1].y
    r0.y = (saturate((r0.yyyy)/(source[1].yyyy))).y;
    // 202: mul o0.w, r0.y, r0.x
    output.targets[0].w = ((r0.yyyy)*(r0.xxxx)).w;
    // 203: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 204: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 205: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 206: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 207: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 208: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 209: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 210: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 211: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 212: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 213: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 214: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 215: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 216: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 217: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 218: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 219: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 220: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 221: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 222: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 223: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 224: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 225: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 226: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 227: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 228: ret
    return output;
}

// source.character.static-map-native-1524.v1 / source program f19b3d3238562c47b3da05758da321c5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1524(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[7].y=(g_SourceCharacterTime.xxxx).x;
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[10]=g_SourceCharacterBaseConstants[8];
    source[11]=g_SourceCharacterBaseConstants[9];
    source[12]=g_SourceCharacterBaseConstants[10];
    source[13]=g_SourceCharacterBaseConstants[11];
    source[14]=g_SourceCharacterBaseConstants[12];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v6.z
    r0.x = ((r0.xxxx)*(v6.zzzz)).x;
    // 4: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 5: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 6: mul r0.y, r0.y, cb0[14].z
    r0.y = ((r0.yyyy)*(source[14].zzzz)).y;
    // 7: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 8: mul_sat r0.y, r0.y, cb0[14].w
    r0.y = (saturate((r0.yyyy)*(source[14].wwww))).y;
    // 9: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 10: add r0.y, -cb0[3].x, l(1.000000)
    r0.y = ((-(source[3].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: mul r0.zw, v4.xxxy, cb0[8].zzzw
    r0.zw = ((v4.xxxy)*(source[8].zzzw)).zw;
    // 12: mul r1.x, cb0[7].x, cb0[7].y
    r1.x = ((source[7].xxxx)*(source[7].yyyy)).x;
    // 13: mad r2.x, r1.x, cb0[8].y, r0.z
    r2.x = ((r1.xxxx)*(source[8].yyyy)+(r0.zzzz)).x;
    // 14: mad r2.y, r1.x, cb0[9].x, r0.w
    r2.y = ((r1.xxxx)*(source[9].xxxx)+(r0.wwww)).y;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 16: mad r0.zw, cb0[9].zzzz, r0.zzzw, v4.xxxy
    r0.zw = ((source[9].zzzz)*(r0.zzzw)+(v4.xxxy)).zw;
    // 17: mul r1.yz, r0.zzwz, cb0[12].xxyx
    r1.yz = ((r0.zzwz)*(source[12].xxyx)).yz;
    // 18: mad r2.x, r1.x, cb0[11].w, r1.y
    r2.x = ((r1.xxxx)*(source[11].wwww)+(r1.yyyy)).x;
    // 19: mad r2.y, r1.x, cb0[12].z, r1.z
    r2.y = ((r1.xxxx)*(source[12].zzzz)+(r1.zzzz)).y;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s3, l(0.000000)
    r1.y = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 21: mad r1.z, cb0[7].y, cb0[13].x, cb0[13].y
    r1.z = ((source[7].yyyy)*(source[13].xxxx)+(source[13].yyyy)).z;
    // 22: sincos r2.x, r3.x, r1.z
    r2.x = (sin(r1.zzzz)).x; r3.x = (cos(r1.zzzz)).x;
    // 23: mov r4.x, -r2.x
    r4.x = (-(r2.xxxx)).x;
    // 24: add r1.zw, r0.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r1.zw = ((r0.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 25: mov r4.y, r3.x
    r4.y = (r3.xxxx).y;
    // 26: mov r4.z, r2.x
    r4.z = (r2.xxxx).z;
    // 27: dp2 r2.x, r4.zyzz, r1.zwzz
    r2.x = (dot((r4.zyzz).xy,(r1.zwzz).xy).xxxx).x;
    // 28: dp2 r2.y, r4.yxyy, r1.zwzz
    r2.y = (dot((r4.yxyy).xy,(r1.zwzz).xy).xxxx).y;
    // 29: mul r3.x, r2.y, cb0[6].x
    r3.x = ((r2.yyyy)*(source[6].xxxx)).x;
    // 30: add r2.y, cb0[3].y, l(-1.000000)
    r2.y = ((source[3].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 31: mad r3.z, r2.x, cb0[6].y, r2.y
    r3.z = ((r2.xxxx)*(source[6].yyyy)+(r2.yyyy)).z;
    // 32: add r2.xz, r3.xxzx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r2.xz = ((r3.xxzx)+(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.x, r2.xzxx, t2.xyzw, s4, l(0.000000)
    r2.x = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 34: mad r0.y, r1.y, r2.x, -r0.y
    r0.y = ((r1.yyyy)*(r2.xxxx)+(-(r0.yyyy))).y;
    // 35: mul_sat r0.y, r0.y, cb0[14].x
    r0.y = (saturate((r0.yyyy)*(source[14].xxxx))).y;
    // 36: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 37: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 38: mul r1.y, r1.y, cb0[14].y
    r1.y = ((r1.yyyy)*(source[14].yyyy)).y;
    // 39: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 40: mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // 41: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 42: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 43: movc o0.w, r0.y, l(0), r0.x
    output.targets[0].w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 44: mul r0.x, r0.z, cb0[7].w
    r0.x = ((r0.zzzz)*(source[7].wwww)).x;
    // 45: mad r0.x, r1.x, cb0[7].z, r0.x
    r0.x = ((r1.xxxx)*(source[7].zzzz)+(r0.xxxx)).x;
    // 46: mul r0.z, r1.x, cb0[9].w
    r0.z = ((r1.xxxx)*(source[9].wwww)).z;
    // 47: mad r0.y, cb0[8].x, r0.w, r0.z
    r0.y = ((source[8].xxxx)*(r0.wwww)+(r0.zzzz)).y;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 49: mad r0.w, cb0[7].y, cb0[10].y, cb0[10].z
    r0.w = ((source[7].yyyy)*(source[10].yyyy)+(source[10].zzzz)).w;
    // 50: sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
    // 51: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 52: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 53: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 54: dp2 r0.w, r3.yxyy, r1.zwzz
    r0.w = (dot((r3.yxyy).xy,(r1.zwzz).xy).xxxx).w;
    // 55: dp2 r1.x, r3.zyzz, r1.zwzz
    r1.x = (dot((r3.zyzz).xy,(r1.zwzz).xy).xxxx).x;
    // 56: mad r1.z, r1.x, cb0[4].y, r2.y
    r1.z = ((r1.xxxx)*(source[4].yyyy)+(r2.yyyy)).z;
    // 57: mul r1.x, r0.w, cb0[4].x
    r1.x = ((r0.wwww)*(source[4].xxxx)).x;
    // 58: add r1.xy, r1.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 60: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 61: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: mad r0.xyz, -r0.xyzx, r1.xyzx, r0.wwww
    r0.xyz = ((-(r0.xyzx))*(r1.xyzx)+(r0.wwww)).xyz;
    // 63: mad r0.xyz, cb0[11].yyyy, r0.xyzx, r2.xyzx
    r0.xyz = ((source[11].yyyy)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 64: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 65: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 66: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 67: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 68: mul r1.xyz, cb0[5].xyzx, cb0[5].wwww
    r1.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 69: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 70: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 71: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 72: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 73: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 74: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 75: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 76: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 77: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 78: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 79: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 80: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 81: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 82: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 83: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 84: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 85: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 86: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 87: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 88: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 89: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 90: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 91: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 92: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 93: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 94: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 95: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 96: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 97: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 98: ret
    return output;
}

// source.character.static-map-native-1525.v1 / source program 55ffe2ed40808b43a4f447b6f1dff569
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1525(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v7.z
    r0.x = ((r0.xxxx)*(v7.zzzz)).x;
    // 4: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 6: mul r0.yzw, r0.yyyy, cb0[6].xxyz
    r0.yzw = ((r0.yyyy)*(source[6].xxyz)).yzw;
    // 7: mad r0.xyz, r0.xxxx, cb0[5].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[5].xyzx)+(r0.yzwy)).xyz;
    // 8: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 10: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 11: mul r0.w, r1.w, cb0[3].x
    r0.w = ((r1.wwww)*(source[3].xxxx)).w;
    // 12: mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // 13: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 14: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 15: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 16: mad r1.xyz, r0.xyzx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 17: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 18: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 19: mad r0.xyz, r2.xyzx, cb0[7].xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(source[7].xyzx)+(r1.xyzx)).xyz;
    // 20: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 21: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 22: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 23: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 24: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 25: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 26: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 27: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 28: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 29: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 30: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 31: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 32: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 33: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 34: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 35: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 36: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 37: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 38: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 39: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 40: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 41: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 42: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 43: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 44: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 45: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 46: ftou r0.x, cb0[4].z
    r0.x = (asfloat((uint4)(source[4].zzzz))).x;
    // 47: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 48: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 49: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 50: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 51: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 52: ret
    return output;
}

