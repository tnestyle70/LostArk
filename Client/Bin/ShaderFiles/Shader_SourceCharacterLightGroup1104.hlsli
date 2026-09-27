#ifndef SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
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
#else // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
    case 1104u: return SourceCharacterLight1104(input);
    case 1105u: return SourceCharacterLight1105(input);
    case 1106u: return SourceCharacterLight1106(input);
    case 1107u: return SourceCharacterLight1107(input);
    case 1108u: return SourceCharacterLight1108(input);
    case 1109u: return SourceCharacterLight1109(input);
    case 1110u: return SourceCharacterLight1110(input);
    case 1111u: return SourceCharacterLight1111(input);
    case 1112u: return SourceCharacterLight1112(input);
    case 1113u: return SourceCharacterLight1113(input);
    case 1114u: return SourceCharacterLight1114(input);
    case 1115u: return SourceCharacterLight1115(input);
    case 1116u: return SourceCharacterLight1116(input);
    case 1117u: return SourceCharacterLight1117(input);
    case 1118u: return SourceCharacterLight1118(input);
    case 1119u: return SourceCharacterLight1119(input);
#endif // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
