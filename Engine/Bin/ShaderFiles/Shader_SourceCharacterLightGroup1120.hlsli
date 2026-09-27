#ifndef SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
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
#else // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
    case 1120u: return SourceCharacterLight1120(input);
    case 1121u: return SourceCharacterLight1121(input);
    case 1122u: return SourceCharacterLight1122(input);
    case 1123u: return SourceCharacterLight1123(input);
    case 1124u: return SourceCharacterLight1124(input);
    case 1125u: return SourceCharacterLight1125(input);
    case 1126u: return SourceCharacterLight1126(input);
    case 1127u: return SourceCharacterLight1127(input);
    case 1128u: return SourceCharacterLight1128(input);
    case 1129u: return SourceCharacterLight1129(input);
    case 1130u: return SourceCharacterLight1130(input);
    case 1131u: return SourceCharacterLight1131(input);
    case 1132u: return SourceCharacterLight1132(input);
    case 1133u: return SourceCharacterLight1133(input);
    case 1134u: return SourceCharacterLight1134(input);
    case 1135u: return SourceCharacterLight1135(input);
#endif // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
