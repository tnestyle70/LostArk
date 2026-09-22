SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase192(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].y=(g_SourceCharacterTime.xxxx).x;
    source[19].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[19].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].x=1.f;
    source[1].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: add r1.xyz, v8.xyzx, cb0[0].yzwy
    r1.xyz = ((v8.xyzx)+(source[0].yzwy)).xyz;
    // 5: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 9: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 18: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r0.w, r5.w, cb0[1].w
    r0.w = ((r5.wwww)*(source[1].wwww)).w;
    // 23: add r6.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r6.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 24: mul r6.xyz, r6.xyzx, cb0[16].xxxx
    r6.xyz = ((r6.xyzx)*(source[16].xxxx)).xyz;
    // 25: mad r6.xyz, r5.yyyy, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r5.yyyy)*(r6.xyzx)+(source[3].xyzx)).xyz;
    // 26: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 27: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 28: mad r6.xyz, cb0[16].yyyy, r7.xyzx, r6.xyzx
    r6.xyz = ((source[16].yyyy)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 29: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 30: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 31: mad r6.xyz, cb0[16].zzzz, r7.xyzx, r6.xyzx
    r6.xyz = ((source[16].zzzz)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 32: mad r7.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 33: mad r8.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 34: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 35: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 36: mul r8.xyz, r5.xxxx, r8.xyzx
    r8.xyz = ((r5.xxxx)*(r8.xyzx)).xyz;
    // 37: mul r1.w, cb0[11].z, l(1.500000)
    r1.w = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 38: add r2.w, -cb0[11].w, l(1.000000)
    r2.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r2.w, r2.w, cb0[19].y
    r2.w = ((r2.wwww)*(source[19].yyyy)).w;
    // 40: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 41: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 42: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 44: mad r1.w, r1.w, l(0.500000), cb0[11].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 45: frc r2.w, cb0[11].x
    r2.w = (frac(source[11].xxxx)).w;
    // 46: add r3.w, -r2.w, cb0[11].x
    r3.w = ((-(r2.wwww))+(source[11].xxxx)).w;
    // 47: mul r9.z, r3.w, l(0.125000)
    r9.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 48: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 49: mul r9.y, cb0[11].y, cb0[12].y
    r9.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 50: mul r10.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r10.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 51: frc r3.w, r10.x
    r3.w = (frac(r10.xxxx)).w;
    // 52: mul r10.y, r3.w, l(0.125000)
    r10.y = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 53: add r5.xy, r9.xyxx, r10.yzyy
    r5.xy = ((r9.xyxx)+(r10.yzyy)).xy;
    // 54: add r5.xy, r5.xyxx, r9.zwzz
    r5.xy = ((r5.xyxx)+(r9.zwzz)).xy;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 56: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 57: mul r1.w, r2.w, r9.w
    r1.w = ((r2.wwww)*(r9.wwww)).w;
    // 58: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r8.xyzx))).xyz;
    // 59: mad r8.xyz, r1.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 60: mul r1.w, cb0[13].y, cb0[19].y
    r1.w = ((source[13].yyyy)*(source[19].yyyy)).w;
    // 61: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 62: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 63: mul r5.y, r1.w, l(0.020000)
    r5.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 64: add r9.xyzw, r1.yzxy, -cb0[1].yzxy
    r9.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 65: add r9.xy, -r9.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r9.xy = ((-(r9.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 66: add r9.xy, -r9.zwzz, r9.xyxx
    r9.xy = ((-(r9.zwzz))+(r9.xyxx)).xy;
    // 67: mad r9.xy, cb0[13].wwww, r9.xyxx, r9.zwzz
    r9.xy = ((source[13].wwww)*(r9.xyxx)+(r9.zwzz)).xy;
    // 68: mul r2.w, cb0[13].x, l(0.001000)
    r2.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 69: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 70: mad r5.xy, r2.wwww, r9.xyxx, r5.xyxx
    r5.xy = ((r2.wwww)*(r9.xyxx)+(r5.xyxx)).xy;
    // 71: dp2 r2.w, cb0[14].xyxx, r5.xyxx
    r2.w = (dot((source[14].xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 72: dp2 r5.y, cb0[15].xyxx, r5.xyxx
    r5.y = (dot((source[15].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 73: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 74: mul r5.x, r2.w, l(0.125000)
    r5.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 76: mul r2.w, r9.w, l(0.900000)
    r2.w = ((r9.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 77: mad r9.xyz, r9.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r8.xyzx))).xyz;
    // 78: mad r9.xyz, r2.wwww, r9.xyzx, r8.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 79: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 81: mul_sat r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = (saturate((r9.xyzx)*(r1.wwww))).xyz;
    // 82: mul r10.xyz, r9.xyzx, cb0[13].zzzz
    r10.xyz = ((r9.xyzx)*(source[13].zzzz)).xyz;
    // 83: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 84: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 85: mad r9.xyz, cb0[13].zzzz, r9.xyzx, -r8.xyzx
    r9.xyz = ((source[13].zzzz)*(r9.xyzx)+(-(r8.xyzx))).xyz;
    // 86: mad r8.xyz, r1.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 87: mad r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = ((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 88: mad r6.xyz, r6.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r6.xyz = ((r6.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 89: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 90: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 91: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 92: mul r5.xyz, r5.zzzz, r6.xyzx
    r5.xyz = ((r5.zzzz)*(r6.xyzx)).xyz;
    // 93: mov_sat r1.w, r2.z
    r1.w = (saturate(r2.zzzz)).w;
    // 94: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 95: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 96: mul r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)*(r1.wwww)).xyz;
    // 97: mul r5.xyz, r5.xyzx, cb0[16].wwww
    r5.xyz = ((r5.xyzx)*(source[16].wwww)).xyz;
    // 98: add r1.xyz, -r1.xyzx, cb0[0].yzwy
    r1.xyz = ((-(r1.xyzx))+(source[0].yzwy)).xyz;
    // 99: mul r6.xyz, r2.zzzz, r1.xyzx
    r6.xyz = ((r2.zzzz)*(r1.xyzx)).xyz;
    // 100: mad r1.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 101: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 102: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 103: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 104: add r1.x, r1.x, cb0[7].z
    r1.x = ((r1.xxxx)+(source[7].zzzz)).x;
    // 105: dp3 r1.y, r0.xyzx, r3.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 106: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 107: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 108: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 109: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 110: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: mad r1.y, cb0[17].z, l(4.500000), l(0.500000)
    r1.y = ((source[17].zzzz)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 112: mul r1.y, r1.y, cb0[17].w
    r1.y = ((r1.yyyy)*(source[17].wwww)).y;
    // 113: mul r1.y, r1.y, l(0.050000)
    r1.y = ((r1.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 114: lt r1.z, r1.x, l(0.000001)
    r1.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 115: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 116: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 117: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 118: mul r1.x, r1.x, cb0[18].x
    r1.x = ((r1.xxxx)*(source[18].xxxx)).x;
    // 119: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 120: dp3 r1.y, r3.xyzx, r2.xyzx
    r1.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 121: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 122: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: add r2.x, -|r1.y|, l(1.000000)
    r2.x = ((-(abs(r1.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 124: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 125: mad r2.xyw, r1.wwww, cb0[9].xyxz, -cb0[9].xyxz
    r2.xyw = ((r1.wwww)*(source[9].xyxz)+(-(source[9].xyxz))).xyw;
    // 126: mad r2.xyw, cb0[9].wwww, r2.xyxw, cb0[9].xyxz
    r2.xyw = ((source[9].wwww)*(r2.xyxw)+(source[9].xyxz)).xyw;
    // 127: mad r2.xyw, r1.zzzz, cb0[8].xyxz, r2.xyxw
    r2.xyw = ((r1.zzzz)*(source[8].xyxz)+(r2.xyxw)).xyw;
    // 128: mul_sat r1.y, r1.y, cb0[18].y
    r1.y = (saturate((r1.yyyy)*(source[18].yyyy))).y;
    // 129: mul_sat r1.z, r2.z, cb0[18].y
    r1.z = (saturate((r2.zzzz)*(source[18].yyyy))).z;
    // 130: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 131: add_sat r1.z, r1.z, -cb0[18].z
    r1.z = (saturate((r1.zzzz)+(-(source[18].zzzz)))).z;
    // 132: lt r1.w, r1.z, l(0.000001)
    r1.w = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 133: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 134: mul r1.z, r1.z, cb0[18].w
    r1.z = ((r1.zzzz)*(source[18].wwww)).z;
    // 135: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 136: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 137: mul r1.y, r1.y, cb0[10].w
    r1.y = ((r1.yyyy)*(source[10].wwww)).y;
    // 138: mul r3.xyz, r1.yyyy, cb0[10].xyzx
    r3.xyz = ((r1.yyyy)*(source[10].xyzx)).xyz;
    // 139: movc r1.yzw, r1.wwww, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 140: add r1.yzw, r1.yyzw, r2.xxyw
    r1.yzw = ((r1.yyzw)+(r2.xxyw)).yzw;
    // 141: mad r1.xyz, r1.xxxx, r5.xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r5.xyzx)+(r1.yzwy)).xyz;
    // 142: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 143: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 144: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 145: mul r2.xyz, r1.wwww, v7.xyzx
    r2.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 146: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 147: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 148: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 149: mul r2.yzw, r2.yyyy, cb0[21].xxyz
    r2.yzw = ((r2.yyyy)*(source[21].xxyz)).yzw;
    // 150: mad r2.xyz, r2.xxxx, cb0[20].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[20].xyzx)+(r2.yzwy)).xyz;
    // 151: mul r2.xyz, r2.xyzx, cb0[22].wwww
    r2.xyz = ((r2.xyzx)*(source[22].wwww)).xyz;
    // 152: mul r3.xyz, r8.xyzx, r2.xyzx
    r3.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 153: mad r1.xyz, r2.xyzx, r8.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 154: mad r1.xyz, r8.xyzx, cb0[22].xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(source[22].xyzx)+(r1.xyzx)).xyz;
    // 155: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: mad r1.w, r1.w, l(-0.250000), l(0.400000)
    r1.w = ((r1.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 157: eq r2.x, cb0[23].x, l(0.000000)
    r2.x = (asfloat((uint4)((source[23].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 158: not r2.y, r2.x
    r2.y = (asfloat(~asuint(r2.xxxx))).y;
    // 159: lt r2.z, r0.w, r1.w
    r2.z = (asfloat((uint4)((r0.wwww)<(r1.wwww)) * 0xffffffffu)).z;
    // 160: and r2.y, r2.z, r2.y
    r2.y = (asfloat(asuint(r2.zzzz) & asuint(r2.yyyy))).y;
    // 161: discard_nz r2.y
    if ((asuint(r2.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 162: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 163: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 164: mul r2.yzw, r2.yyyy, v0.xxyz
    r2.yzw = ((r2.yyyy)*(v0.xxyz)).yzw;
    // 165: mul r5.xyz, r0.zxyz, r2.zwyz
    r5.xyz = ((r0.zxyz)*(r2.zwyz)).xyz;
    // 166: mad r5.xyz, r0.yzxy, r2.wyzw, -r5.xyzx
    r5.xyz = ((r0.yzxy)*(r2.wyzw)+(-(r5.xyzx))).xyz;
    // 167: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 168: movc r3.w, v9.x, l(1.000000), l(-1.000000)
    r3.w = ((asuint(v9.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 169: mul r3.w, r3.w, cb0[0].x
    r3.w = ((r3.wwww)*(source[0].xxxx)).w;
    // 170: mul r4.xyz, r3.wwww, r4.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 171: ge r1.w, r0.w, r1.w
    r1.w = (asfloat((uint4)((r0.wwww)>=(r1.wwww)) * 0xffffffffu)).w;
    // 172: mad r3.w, r5.w, cb0[1].w, l(-0.900000)
    r3.w = ((r5.wwww)*(source[1].wwww)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 173: mul_sat r3.w, r3.w, l(9.999998)
    r3.w = (saturate((r3.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 174: mad r4.w, r3.w, l(-2.000000), l(3.000000)
    r4.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 175: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 176: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 177: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 178: movc r1.w, r1.w, r3.w, r0.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (r3.wwww) : (r0.wwww)).w;
    // 179: movc o0.w, r2.x, r1.w, r0.w
    output.targets[0].w = ((asuint(r2.xxxx) != 0u) ? (r1.wwww) : (r0.wwww)).w;
    // 180: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 181: dp3 r1.x, r2.yzwy, r4.xyzx
    r1.x = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).x;
    // 182: dp3 r1.y, r5.xyzx, r4.xyzx
    r1.y = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 183: dp3 r1.z, r0.xyzx, r4.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 184: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 185: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 186: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 187: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 188: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 189: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 190: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 191: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 192: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 193: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 194: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 195: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 196: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 197: mov o3.xyz, r8.xyzx
    output.targets[3].xyz = (r8.xyzx).xyz;
    // 198: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 199: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 200: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 201: ret
    return output;
}

// source.character.equipment-native-193.v1 / source program 43ce15c43ef48349ae3ac5f8fca062eb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase193(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[20]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[27].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s7, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[21].xyzw
    r1.xyzw = ((r0.xyzw)*(source[21].xyzw)).xyzw;
    // 3: add r0.xy, r0.ywyy, r0.xzxx
    r0.xy = ((r0.ywyy)+(r0.xzxx)).xy;
    // 4: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 5: add r0.yz, r1.yywy, r1.xxzx
    r0.yz = ((r1.yywy)+(r1.xxzx)).yz;
    // 6: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 7: add r0.y, r0.y, l(-1.000000)
    r0.y = ((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 8: mad_sat r0.x, r0.x, r0.y, l(1.000000)
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 10: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 11: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 12: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 13: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 14: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 15: mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 16: mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // 17: mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // 18: mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // 19: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 20: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 21: mul r0.xy, r0.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 22: deriv_rtx_coarse r0.zw, r0.xxxy
    r0.zw = (ddx_coarse(r0.xxxy)).zw;
    // 23: deriv_rty_coarse r0.xy, r0.xyxx
    r0.xy = (ddy_coarse(r0.xyxx)).xy;
    // 24: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 25: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 26: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 27: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 28: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 29: rcp r0.y, |r0.x|
    r0.y = (1.0/(abs(r0.xxxx))).y;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 32: lt r0.w, |r0.z|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 33: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 34: add r1.w, cb0[23].w, -cb0[24].x
    r1.w = ((source[23].wwww)+(-(source[24].xxxx))).w;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mad r1.w, r3.x, r1.w, cb0[24].x
    r1.w = ((r3.xxxx)*(r1.wwww)+(source[24].xxxx)).w;
    // 37: add r2.w, -r1.w, cb0[24].y
    r2.w = ((-(r1.wwww))+(source[24].yyyy)).w;
    // 38: mad r1.w, r3.y, r2.w, r1.w
    r1.w = ((r3.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 39: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 40: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 41: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 43: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 44: add r1.w, -cb0[23].y, cb0[23].x
    r1.w = ((-(source[23].yyyy))+(source[23].xxxx)).w;
    // 45: mad r1.w, r3.x, r1.w, cb0[23].y
    r1.w = ((r3.xxxx)*(r1.wwww)+(source[23].yyyy)).w;
    // 46: add r2.w, -r1.w, cb0[23].z
    r2.w = ((-(r1.wwww))+(source[23].zzzz)).w;
    // 47: mad r1.w, r3.y, r2.w, r1.w
    r1.w = ((r3.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 48: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 49: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 50: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 51: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 52: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 54: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 55: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 56: mul r4.xy, r0.ywyy, cb0[22].xxxx
    r4.xy = ((r0.ywyy)*(source[22].xxxx)).xy;
    // 57: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 58: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 59: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 60: add r4.z, r0.y, l(0.000010)
    r4.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 61: add r5.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 62: mad r5.xyz, cb0[22].wwww, r5.xyzx, r4.xyzx
    r5.xyz = ((source[22].wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 63: dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 64: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 65: div r5.xyz, r5.xyzx, r0.yyyy
    r5.xyz = ((r5.xyzx)/(r0.yyyy)).xyz;
    // 66: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 67: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 68: mul r6.xyz, r0.yyyy, v0.xyzx
    r6.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 69: dp3 r7.x, r6.xyzx, r5.xyzx
    r7.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 70: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 71: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 72: mul r8.xyz, r0.yyyy, v1.xyzx
    r8.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 73: mul r9.xyz, r6.yzxy, r8.zxyz
    r9.xyz = ((r6.yzxy)*(r8.zxyz)).xyz;
    // 74: mad r9.xyz, r8.yzxy, r6.zxyz, -r9.xyzx
    r9.xyz = ((r8.yzxy)*(r6.zxyz)+(-(r9.xyzx))).xyz;
    // 75: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 76: dp3 r7.y, r9.xyzx, r5.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 77: dp3 r7.z, r8.xyzx, r5.xyzx
    r7.z = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 78: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 79: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 80: mul r5.xyz, r0.yyyy, v5.xyzx
    r5.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 81: mad r10.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 82: dp3 r11.y, r9.xyzx, r5.xyzx
    r11.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 83: dp3 r11.x, r6.xyzx, r5.xyzx
    r11.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 84: dp3 r11.z, r8.xyzx, r5.xyzx
    r11.z = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 85: dp3 r0.y, r7.xyzx, r11.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 86: mul r7.xyz, r7.xyzx, r0.yyyy
    r7.xyz = ((r7.xyzx)*(r0.yyyy)).xyz;
    // 87: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 88: mov r7.w, -r7.x
    r7.w = (-(r7.xxxx)).w;
    // 89: dp2 r0.y, r7.ywyy, r7.ywyy
    r0.y = (dot((r7.ywyy).xy,(r7.ywyy).xy).xxxx).y;
    // 90: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 91: div r0.yw, r7.yyyw, r0.yyyy
    r0.yw = ((r7.yyyw)/(r0.yyyy)).yw;
    // 92: mad r1.w, -r7.z, l(0.250000), l(0.250000)
    r1.w = ((-(r7.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 93: add r2.w, r7.z, l(1.000000)
    r2.w = ((r7.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 95: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 97: log r7.xyz, r0.xywx
    r7.xyz = (log2(r0.xywx)).xyz;
    // 98: rcp r1.w, cb0[24].z
    r1.w = (1.0/(source[24].zzzz)).w;
    // 99: mul r11.xyz, r7.xyzx, r1.wwww
    r11.xyz = ((r7.xyzx)*(r1.wwww)).xyz;
    // 100: mul r7.xyz, r7.xyzx, cb0[24].zzzz
    r7.xyz = ((r7.xyzx)*(source[24].zzzz)).xyz;
    // 101: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 102: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 103: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 104: mad r7.xyz, r7.xyzx, cb0[24].zzzz, r11.xyzx
    r7.xyz = ((r7.xyzx)*(source[24].zzzz)+(r11.xyzx)).xyz;
    // 105: add r0.xyw, r0.xyxw, r7.xyxz
    r0.xyw = ((r0.xyxw)+(r7.xyxz)).xyw;
    // 106: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r1.w, cb0[24].z, l(1.000000)
    r1.w = ((source[24].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 109: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 110: add r7.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r7.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 111: mad r7.xyz, r2.wwww, r7.xyzx, cb0[11].xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)+(source[11].xyzx)).xyz;
    // 112: mul r0.xyw, r0.xxxx, r7.xyxz
    r0.xyw = ((r0.xxxx)*(r7.xyxz)).xyw;
    // 113: mul r0.xyw, r0.xyxw, cb0[24].wwww
    r0.xyw = ((r0.xyxw)*(source[24].wwww)).xyw;
    // 114: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 115: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 116: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 117: dp3 r1.w, r4.xyzx, r5.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 118: mul_sat r2.w, r1.w, cb0[25].y
    r2.w = (saturate((r1.wwww)*(source[25].yyyy))).w;
    // 119: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: mul_sat r4.w, r5.z, cb0[25].y
    r4.w = (saturate((r5.zzzz)*(source[25].yyyy))).w;
    // 122: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: add_sat r4.w, r4.w, -cb0[25].z
    r4.w = (saturate((r4.wwww)+(-(source[25].zzzz)))).w;
    // 124: log r5.w, r4.w
    r5.w = (log2(r4.wwww)).w;
    // 125: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 126: mul r5.w, r5.w, cb0[25].w
    r5.w = ((r5.wwww)*(source[25].wwww)).w;
    // 127: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 128: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 129: movc r2.w, r4.w, l(0), r2.w
    r2.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 130: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 131: add r7.xyz, -r2.xyzx, r4.wwww
    r7.xyz = ((-(r2.xyzx))+(r4.wwww)).xyz;
    // 132: mad r2.xyz, cb0[22].yyyy, r7.xyzx, r2.xyzx
    r2.xyz = ((source[22].yyyy)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 133: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 134: add r7.xyz, -r2.xyzx, r4.wwww
    r7.xyz = ((-(r2.xyzx))+(r4.wwww)).xyz;
    // 135: mad r2.xyz, cb0[22].zzzz, r7.xyzx, r2.xyzx
    r2.xyz = ((source[22].zzzz)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 136: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: add r7.xyz, -r2.xyzx, r4.wwww
    r7.xyz = ((-(r2.xyzx))+(r4.wwww)).xyz;
    // 138: mul r7.xyz, r7.xyzx, cb0[25].xxxx
    r7.xyz = ((r7.xyzx)*(source[25].xxxx)).xyz;
    // 139: add r4.w, r3.y, r3.x
    r4.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 140: add r4.w, r3.z, r4.w
    r4.w = ((r3.zzzz)+(r4.wwww)).w;
    // 141: add_sat r3.w, r3.w, r4.w
    r3.w = (saturate((r3.wwww)+(r4.wwww))).w;
    // 142: mad r2.xyz, r3.wwww, r7.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 143: max r7.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 144: log r7.xyz, r7.xyzx
    r7.xyz = (log2(r7.xyzx)).xyz;
    // 145: mul r7.xyz, r7.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 146: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 147: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 148: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 149: mul r3.w, r3.w, cb0[26].x
    r3.w = ((r3.wwww)*(source[26].xxxx)).w;
    // 150: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 151: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 154: div r4.w, cb0[26].y, r4.w
    r4.w = ((source[26].yyyy)/(r4.wwww)).w;
    // 155: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 156: mul r7.xyz, r0.xywx, r4.wwww
    r7.xyz = ((r0.xywx)*(r4.wwww)).xyz;
    // 157: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 158: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 159: mad r1.xyz, cb0[22].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 160: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 162: mad r1.xyz, cb0[22].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 163: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 164: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 165: mad r11.xyz, r3.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 166: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 167: mad r11.xyz, r3.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 168: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, -r11.xyzx
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r11.xyzx))).xyz;
    // 169: mad r11.xyz, r3.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 170: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 171: add r12.xyz, -r11.xyzx, r3.xxxx
    r12.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 172: mad r11.xyz, cb0[22].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[22].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 173: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 174: add r12.xyz, -r11.xyzx, r3.xxxx
    r12.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 175: mad r11.xyz, cb0[22].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[22].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 176: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: mad r13.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 179: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 180: mul r1.xyz, r1.xyzx, r11.xyzx
    r1.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 181: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 182: mad r2.xyz, r2.xyzx, r7.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)+(-(r0.xywx))).xyz;
    // 183: add r3.x, -r3.w, l(1.000000)
    r3.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 184: mul r3.x, r3.x, cb0[26].z
    r3.x = ((r3.xxxx)*(source[26].zzzz)).x;
    // 185: mad r0.xyw, r3.xxxx, r2.xyxz, r0.xyxw
    r0.xyw = ((r3.xxxx)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 186: dp3 r2.x, r10.xyzx, r10.xyzx
    r2.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 187: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 188: div r7.xyz, r10.xyzx, r2.yyyy
    r7.xyz = ((r10.xyzx)/(r2.yyyy)).xyz;
    // 189: dp3 r2.y, r7.xyzx, r5.xyzx
    r2.y = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 190: add r2.z, -|r5.z|, l(1.000000)
    r2.z = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 191: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 192: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 193: mul r2.z, |r2.y|, |r2.y|
    r2.z = ((abs(r2.yyyy))*(abs(r2.yyyy))).z;
    // 194: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 195: mul r2.z, r2.z, |r2.y|
    r2.z = ((r2.zzzz)*(abs(r2.yyyy))).z;
    // 196: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 197: movc r2.y, r2.y, l(0), r2.z
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).y;
    // 198: add r2.z, r2.y, l(-0.027778)
    r2.z = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 199: mad r2.y, r2.y, r2.z, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 200: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 201: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 202: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 203: mad r2.xyz, r0.zzzz, r0.xywx, -r1.xyzx
    r2.xyz = ((r0.zzzz)*(r0.xywx)+(-(r1.xyzx))).xyz;
    // 204: mad r1.xyz, r3.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 205: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 206: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 207: mad r1.xyz, cb0[22].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[22].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 208: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 209: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 210: mad r1.xyz, cb0[22].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[22].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 211: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 212: add r0.z, -cb0[4].w, l(1.000000)
    r0.z = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 213: mul r0.z, r0.z, cb0[27].z
    r0.z = ((r0.zzzz)*(source[27].zzzz)).z;
    // 214: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 215: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 216: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 217: mul r2.x, cb0[4].z, l(1.500000)
    r2.x = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 218: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 219: mad r0.z, r0.z, l(0.500000), cb0[4].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).z;
    // 220: frc r2.x, v4.x
    r2.x = (frac(v4.xxxx)).x;
    // 221: mul r2.x, r2.x, l(0.125000)
    r2.x = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 222: mul r5.y, cb0[4].y, cb0[17].y
    r5.y = ((source[4].yyyy)*(source[17].yyyy)).y;
    // 223: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 224: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 225: add r2.xy, r2.xyxx, r5.xyxx
    r2.xy = ((r2.xyxx)+(r5.xyxx)).xy;
    // 226: frc r2.z, cb0[4].x
    r2.z = (frac(source[4].xxxx)).z;
    // 227: add r3.x, -r2.z, cb0[4].x
    r3.x = ((-(r2.zzzz))+(source[4].xxxx)).x;
    // 228: mul r5.z, r3.x, l(0.125000)
    r5.z = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 229: add r2.xy, r2.xyxx, r5.zwzz
    r2.xy = ((r2.xyxx)+(r5.zwzz)).xy;
    // 230: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 231: mul r3.xyw, r0.zzzz, r5.xyxz
    r3.xyw = ((r0.zzzz)*(r5.xyxz)).xyw;
    // 232: mul r0.z, r2.z, r5.w
    r0.z = ((r2.zzzz)*(r5.wwww)).z;
    // 233: add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 234: mad r3.xyw, r3.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r1.xyxz
    r3.xyw = ((r3.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r1.xyxz))).xyw;
    // 235: mad r1.xyz, r0.zzzz, r3.xywx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r3.xywx)+(r1.xyzx)).xyz;
    // 236: add r5.xyzw, v7.yzxy, cb0[0].yzxy
    r5.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 237: add r5.xyzw, r5.xyzw, -cb0[1].yzxy
    r5.xyzw = ((r5.xyzw)+(-(source[1].yzxy))).xyzw;
    // 238: add r2.yz, -r5.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r2.yz = ((-(r5.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 239: add r2.yz, -r5.zzwz, r2.yyzy
    r2.yz = ((-(r5.zzwz))+(r2.yyzy)).yz;
    // 240: mad r2.yz, cb0[18].wwww, r2.yyzy, r5.zzwz
    r2.yz = ((source[18].wwww)*(r2.yyzy)+(r5.zzwz)).yz;
    // 241: mul r0.z, cb0[18].y, cb0[27].z
    r0.z = ((source[18].yyyy)*(source[27].zzzz)).z;
    // 242: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 243: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 244: mul r3.y, r0.z, l(0.020000)
    r3.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 245: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 246: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 247: mul r3.w, cb0[18].x, l(0.001000)
    r3.w = ((source[18].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 248: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 249: mad r2.yz, r3.wwww, r2.yyzy, r3.xxyx
    r2.yz = ((r3.wwww)*(r2.yyzy)+(r3.xxyx)).yz;
    // 250: dp2 r3.x, cb0[19].xyxx, r2.yzyy
    r3.x = (dot((source[19].xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 251: dp2 r5.y, cb0[20].xyxx, r2.yzyy
    r5.y = (dot((source[20].xyxx).xy,(r2.yzyy).xy).xxxx).y;
    // 252: frc r2.y, r3.x
    r2.y = (frac(r3.xxxx)).y;
    // 253: mul r5.x, r2.y, l(0.125000)
    r5.x = ((r2.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 254: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t6.xyzw, s6, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 255: mad r3.xyw, r5.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r1.xyxz
    r3.xyw = ((r5.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r1.xyxz))).xyw;
    // 256: mul r2.y, r5.w, l(0.900000)
    r2.y = ((r5.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 257: mad r3.xyw, r2.yyyy, r3.xyxw, r1.xyxz
    r3.xyw = ((r2.yyyy)*(r3.xyxw)+(r1.xyxz)).xyw;
    // 258: mul_sat r3.xyw, r0.zzzz, r3.xyxw
    r3.xyw = (saturate((r0.zzzz)*(r3.xyxw))).xyw;
    // 259: mad r5.xyz, cb0[18].zzzz, r3.xywx, -r1.xyzx
    r5.xyz = ((source[18].zzzz)*(r3.xywx)+(-(r1.xyzx))).xyz;
    // 260: mul r3.xyw, r3.xyxw, cb0[18].zzzz
    r3.xyw = ((r3.xyxw)*(source[18].zzzz)).xyw;
    // 261: dp3 r0.z, r3.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 262: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 263: mad r1.xyz, r0.zzzz, r5.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 264: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 265: add r0.z, -r3.z, r2.w
    r0.z = ((-(r3.zzzz))+(r2.wwww)).z;
    // 266: mad r2.yzw, r2.wwww, cb0[15].xxyz, -cb0[15].xxyz
    r2.yzw = ((r2.wwww)*(source[15].xxyz)+(-(source[15].xxyz))).yzw;
    // 267: mad r2.yzw, cb0[15].wwww, r2.yyzw, cb0[15].xxyz
    r2.yzw = ((source[15].wwww)*(r2.yyzw)+(source[15].xxyz)).yzw;
    // 268: mad r0.z, cb0[14].w, r0.z, r3.z
    r0.z = ((source[14].wwww)*(r0.zzzz)+(r3.zzzz)).z;
    // 269: mad r2.yzw, r0.zzzz, cb0[14].xxyz, r2.yyzw
    r2.yzw = ((r0.zzzz)*(source[14].xxyz)+(r2.yyzw)).yzw;
    // 270: add r0.z, cb0[2].y, cb0[2].x
    r0.z = ((source[2].yyyy)+(source[2].xxxx)).z;
    // 271: add r0.z, r0.z, cb0[2].z
    r0.z = ((r0.zzzz)+(source[2].zzzz)).z;
    // 272: add r3.x, -r0.z, l(1000.000000)
    r3.x = ((-(r0.zzzz))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 273: mad r0.z, cb0[27].w, r3.x, r0.z
    r0.z = ((source[27].wwww)*(r3.xxxx)+(r0.zzzz)).z;
    // 274: mul r0.z, r0.z, l(0.010000)
    r0.z = ((r0.zzzz)*(float4(0.010000,0.010000,0.010000,0.010000))).z;
    // 275: mad r0.z, cb0[27].y, cb0[27].z, r0.z
    r0.z = ((source[27].yyyy)*(source[27].zzzz)+(r0.zzzz)).z;
    // 276: mul r3.x, r0.z, l(3.524534)
    r3.x = ((r0.zzzz)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 277: sincos null, r3.x, r3.x
    r3.x = (cos(r3.xxxx)).x;
    // 278: add r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)+(r3.xxxx)).z;
    // 279: mul r0.z, r0.z, l(1.328987)
    r0.z = ((r0.zzzz)*(float4(1.328987,1.328987,1.328987,1.328987))).z;
    // 280: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 281: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 282: mad r0.z, r0.z, l(0.500000), cb0[27].x
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[27].xxxx)).z;
    // 283: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t7.xyzw, s5, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 284: mul r5.xyz, cb0[13].xyzx, cb0[26].wwww
    r5.xyz = ((source[13].xyzx)*(source[26].wwww)).xyz;
    // 285: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 286: mul r3.xyz, r0.zzzz, r3.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)).xyz;
    // 287: mad r0.xyz, r2.xxxx, r0.xywx, r3.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xywx)+(r3.xyzx)).xyz;
    // 288: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 289: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 290: mad r0.xyz, cb0[22].yyyy, r3.xyzx, r0.xyzx
    r0.xyz = ((source[22].yyyy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 291: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 292: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 293: mad r0.xyz, cb0[22].zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((source[22].zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 294: mad r0.xyz, r0.xyzx, r12.xyzx, r2.yzwy
    r0.xyz = ((r0.xyzx)*(r12.xyzx)+(r2.yzwy)).xyz;
    // 295: log r0.w, |r1.w|
    r0.w = (log2(abs(r1.wwww))).w;
    // 296: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 297: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 298: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 299: mul r2.xyz, r0.wwww, cb0[16].xyzx
    r2.xyz = ((r0.wwww)*(source[16].xyzx)).xyz;
    // 300: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 301: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 302: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 303: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 304: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 305: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 306: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 307: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 308: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 309: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 310: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 311: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 312: mul r3.yzw, r3.yyyy, cb0[29].xxyz
    r3.yzw = ((r3.yyyy)*(source[29].xxyz)).yzw;
    // 313: mad r3.xyz, r3.xxxx, cb0[28].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[28].xyzx)+(r3.yzwy)).xyz;
    // 314: mul r3.xyz, r3.xyzx, cb0[30].wwww
    r3.xyz = ((r3.xyzx)*(source[30].wwww)).xyz;
    // 315: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 316: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 317: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 318: mad o0.xyz, r1.xyzx, cb0[30].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[30].xyzx)+(r0.xyzx)).xyz;
    // 319: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 320: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 321: dp3 r0.x, r6.xyzx, r2.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 322: dp3 r0.z, r8.xyzx, r2.xyzx
    r0.z = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 323: dp3 r0.y, r9.xyzx, r2.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 324: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 325: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 326: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 327: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 328: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 329: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 330: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 331: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 332: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 333: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 334: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 335: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 336: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 337: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 338: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 339: ret
    return output;
}

// source.character.equipment-native-194.v1 / source program cb13a7480b15fa44a6e320c466dbc87d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase194(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[20]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[26].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: lt r1.z, |r1.y|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 25: log r1.y, |r1.y|
    r1.y = (log2(abs(r1.yyyy))).y;
    // 26: add r1.w, cb0[22].w, -cb0[23].x
    r1.w = ((source[22].wwww)+(-(source[23].xxxx))).w;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 28: mad r1.w, r3.x, r1.w, cb0[23].x
    r1.w = ((r3.xxxx)*(r1.wwww)+(source[23].xxxx)).w;
    // 29: add r2.w, -r1.w, cb0[23].y
    r2.w = ((-(r1.wwww))+(source[23].yyyy)).w;
    // 30: mad r1.w, r3.y, r2.w, r1.w
    r1.w = ((r3.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 31: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 32: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 33: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 35: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 36: add r1.w, -cb0[22].y, cb0[22].x
    r1.w = ((-(source[22].yyyy))+(source[22].xxxx)).w;
    // 37: mad r1.w, r3.x, r1.w, cb0[22].y
    r1.w = ((r3.xxxx)*(r1.wwww)+(source[22].yyyy)).w;
    // 38: add r2.w, -r1.w, cb0[22].z
    r2.w = ((-(r1.wwww))+(source[22].zzzz)).w;
    // 39: mad r1.w, r3.y, r2.w, r1.w
    r1.w = ((r3.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 40: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 41: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 42: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 43: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 44: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 46: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 47: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 48: mul r4.xy, r1.xzxx, cb0[21].xxxx
    r4.xy = ((r1.xzxx)*(source[21].xxxx)).xy;
    // 49: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 51: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 52: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 53: add r1.xzw, -r4.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r4.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 54: mad r1.xzw, cb0[21].wwww, r1.xxzw, r4.xxyz
    r1.xzw = ((source[21].wwww)*(r1.xxzw)+(r4.xxyz)).xzw;
    // 55: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 56: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 57: div r1.xzw, r1.xxzw, r2.wwww
    r1.xzw = ((r1.xxzw)/(r2.wwww)).xzw;
    // 58: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 59: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 60: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 61: dp3 r6.x, r5.xyzx, r1.xzwx
    r6.x = (dot((r5.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 62: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 63: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 64: mul r7.xyz, r2.wwww, v1.xyzx
    r7.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 65: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 66: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 67: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 68: dp3 r6.y, r8.xyzx, r1.xzwx
    r6.y = (dot((r8.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 69: dp3 r6.z, r7.xyzx, r1.xzwx
    r6.z = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 70: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 71: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 72: mul r9.xyz, r1.xxxx, v5.xyzx
    r9.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 73: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 74: dp3 r10.y, r8.xyzx, r9.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 75: dp3 r10.x, r5.xyzx, r9.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 76: dp3 r10.z, r7.xyzx, r9.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 77: dp3 r2.w, r6.xyzx, r10.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 78: mul r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 79: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 80: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 81: dp2 r2.w, r6.ywyy, r6.ywyy
    r2.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 82: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 83: div r6.xy, r6.ywyy, r2.wwww
    r6.xy = ((r6.ywyy)/(r2.wwww)).xy;
    // 84: mad r2.w, -r6.z, l(0.250000), l(0.250000)
    r2.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 85: add r4.w, r6.z, l(1.000000)
    r4.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 86: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 87: mad r6.xy, r2.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 88: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s4, r0.x
    r6.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 89: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 90: rcp r0.x, cb0[23].z
    r0.x = (1.0/(source[23].zzzz)).x;
    // 91: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 92: mul r10.xyz, r10.xyzx, cb0[23].zzzz
    r10.xyz = ((r10.xyzx)*(source[23].zzzz)).xyz;
    // 93: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 94: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 95: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 96: mad r10.xyz, r10.xyzx, cb0[23].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[23].zzzz)+(r11.xyzx)).xyz;
    // 97: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 98: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 99: add r0.x, cb0[23].z, l(1.000000)
    r0.x = ((source[23].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 100: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 101: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 102: add r6.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r6.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 103: mad r6.xyz, r4.wwww, r6.xyzx, cb0[11].xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)+(source[11].xyzx)).xyz;
    // 104: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 105: mul r6.xyz, r6.xyzx, cb0[23].wwww
    r6.xyz = ((r6.xyzx)*(source[23].wwww)).xyz;
    // 106: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 107: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 108: div r4.xyz, r4.xyzx, r0.xxxx
    r4.xyz = ((r4.xyzx)/(r0.xxxx)).xyz;
    // 109: dp3 r0.x, r4.xyzx, r9.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 110: mul_sat r2.w, r0.x, cb0[24].y
    r2.w = (saturate((r0.xxxx)*(source[24].yyyy))).w;
    // 111: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mul_sat r4.w, r9.z, cb0[24].y
    r4.w = (saturate((r9.zzzz)*(source[24].yyyy))).w;
    // 114: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: add_sat r4.w, r4.w, -cb0[24].z
    r4.w = (saturate((r4.wwww)+(-(source[24].zzzz)))).w;
    // 116: log r5.w, r4.w
    r5.w = (log2(r4.wwww)).w;
    // 117: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 118: mul r5.w, r5.w, cb0[24].w
    r5.w = ((r5.wwww)*(source[24].wwww)).w;
    // 119: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 120: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 121: movc r2.w, r4.w, l(0), r2.w
    r2.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 122: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 123: add r10.xyz, -r2.xyzx, r4.wwww
    r10.xyz = ((-(r2.xyzx))+(r4.wwww)).xyz;
    // 124: mad r2.xyz, cb0[21].yyyy, r10.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 125: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r10.xyz, -r2.xyzx, r4.wwww
    r10.xyz = ((-(r2.xyzx))+(r4.wwww)).xyz;
    // 127: mad r2.xyz, cb0[21].zzzz, r10.xyzx, r2.xyzx
    r2.xyz = ((source[21].zzzz)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 128: dp3 r4.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: add r10.xyz, -r2.xyzx, r4.wwww
    r10.xyz = ((-(r2.xyzx))+(r4.wwww)).xyz;
    // 130: mul r10.xyz, r10.xyzx, cb0[24].xxxx
    r10.xyz = ((r10.xyzx)*(source[24].xxxx)).xyz;
    // 131: add r4.w, r3.y, r3.x
    r4.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 132: add r4.w, r3.z, r4.w
    r4.w = ((r3.zzzz)+(r4.wwww)).w;
    // 133: add_sat r3.w, r3.w, r4.w
    r3.w = (saturate((r3.wwww)+(r4.wwww))).w;
    // 134: mad r2.xyz, r3.wwww, r10.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 135: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 136: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 137: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 138: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 139: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 141: mul r3.w, r3.w, cb0[25].x
    r3.w = ((r3.wwww)*(source[25].xxxx)).w;
    // 142: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 143: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 146: div r4.w, cb0[25].y, r4.w
    r4.w = ((source[25].yyyy)/(r4.wwww)).w;
    // 147: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 148: mul r10.xyz, r6.xyzx, r4.wwww
    r10.xyz = ((r6.xyzx)*(r4.wwww)).xyz;
    // 149: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 150: add r11.xyz, -r0.yzwy, r4.wwww
    r11.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 151: mad r0.yzw, cb0[21].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[21].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 152: dp3 r4.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r0.yzwy, r4.wwww
    r11.xyz = ((-(r0.yzwy))+(r4.wwww)).xyz;
    // 154: mad r0.yzw, cb0[21].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[21].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 155: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 156: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 157: mad r11.xyz, r3.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 158: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 159: mad r11.xyz, r3.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 160: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, -r11.xyzx
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r11.xyzx))).xyz;
    // 161: mad r11.xyz, r3.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r3.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 162: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 163: add r12.xyz, -r11.xyzx, r3.xxxx
    r12.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 164: mad r11.xyz, cb0[21].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[21].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 165: dp3 r3.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 166: add r12.xyz, -r11.xyzx, r3.xxxx
    r12.xyz = ((-(r11.xyzx))+(r3.xxxx)).xyz;
    // 167: mad r11.xyz, cb0[21].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[21].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 168: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mad r13.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 171: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 172: mul r0.yzw, r0.yyzw, r11.xxyz
    r0.yzw = ((r0.yyzw)*(r11.xxyz)).yzw;
    // 173: mul r6.xyz, r6.xyzx, r0.yzwy
    r6.xyz = ((r6.xyzx)*(r0.yzwy)).xyz;
    // 174: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 175: add r3.x, -r3.w, l(1.000000)
    r3.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 176: mul r3.x, r3.x, cb0[25].z
    r3.x = ((r3.xxxx)*(source[25].zzzz)).x;
    // 177: mad r2.xyz, r3.xxxx, r2.xyzx, r6.xyzx
    r2.xyz = ((r3.xxxx)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 178: dp3 r3.x, r1.xzwx, r1.xzwx
    r3.x = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 179: sqrt r3.y, r3.x
    r3.y = (sqrt(r3.xxxx)).y;
    // 180: div r1.xzw, r1.xxzw, r3.yyyy
    r1.xzw = ((r1.xxzw)/(r3.yyyy)).xzw;
    // 181: dp3 r1.x, r1.xzwx, r9.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 182: add r1.z, -|r9.z|, l(1.000000)
    r1.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 183: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 184: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 185: mul r1.z, |r1.x|, |r1.x|
    r1.z = ((abs(r1.xxxx))*(abs(r1.xxxx))).z;
    // 186: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 187: mul r1.z, r1.z, |r1.x|
    r1.z = ((r1.zzzz)*(abs(r1.xxxx))).z;
    // 188: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 189: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 190: add r1.z, r1.x, l(-0.027778)
    r1.z = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 191: mad r1.x, r1.x, r1.z, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 192: div_sat r1.x, r1.x, r3.x
    r1.x = (saturate((r1.xxxx)/(r3.xxxx))).x;
    // 193: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 194: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 195: mad r1.xyz, r1.xxxx, r2.xyzx, -r0.yzwy
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r0.yzwy))).xyz;
    // 196: mad r0.yzw, r3.wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((r3.wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 197: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 198: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 199: mad r0.yzw, cb0[21].yyyy, r1.xxyz, r0.yyzw
    r0.yzw = ((source[21].yyyy)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 200: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 201: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 202: mad r0.yzw, cb0[21].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[21].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 203: mul r0.yzw, r12.xxyz, r0.yyzw
    r0.yzw = ((r12.xxyz)*(r0.yyzw)).yzw;
    // 204: add r1.x, -cb0[4].w, l(1.000000)
    r1.x = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r1.x, r1.x, cb0[26].z
    r1.x = ((r1.xxxx)*(source[26].zzzz)).x;
    // 206: mul r1.x, r1.x, l(6.283185)
    r1.x = ((r1.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 207: sincos r1.x, null, r1.x
    r1.x = (sin(r1.xxxx)).x;
    // 208: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 209: mul r1.y, cb0[4].z, l(1.500000)
    r1.y = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 210: mul r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)*(r1.xxxx)).x;
    // 211: mad r1.x, r1.x, l(0.500000), cb0[4].z
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).x;
    // 212: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 213: mul r3.x, r1.y, l(0.125000)
    r3.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 214: mul r6.y, cb0[4].y, cb0[17].y
    r6.y = ((source[4].yyyy)*(source[17].yyyy)).y;
    // 215: mov r3.y, v4.y
    r3.y = (v4.yyyy).y;
    // 216: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 217: add r1.yz, r3.xxyx, r6.xxyx
    r1.yz = ((r3.xxyx)+(r6.xxyx)).yz;
    // 218: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 219: add r3.x, -r1.w, cb0[4].x
    r3.x = ((-(r1.wwww))+(source[4].xxxx)).x;
    // 220: mul r6.z, r3.x, l(0.125000)
    r6.z = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 221: add r1.yz, r1.yyzy, r6.zzwz
    r1.yz = ((r1.yyzy)+(r6.zzwz)).yz;
    // 222: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r1.yzyy, t5.xyzw, s6, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 223: mul r1.xyz, r1.xxxx, r6.xyzx
    r1.xyz = ((r1.xxxx)*(r6.xyzx)).xyz;
    // 224: mul r3.x, r1.w, r6.w
    r3.x = ((r1.wwww)*(r6.wwww)).x;
    // 225: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 226: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.yzwy
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.yzwy))).xyz;
    // 227: mad r0.yzw, r3.xxxx, r1.xxyz, r0.yyzw
    r0.yzw = ((r3.xxxx)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 228: add r6.xyzw, v7.yzxy, cb0[0].yzxy
    r6.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 229: add r6.xyzw, r6.xyzw, -cb0[1].yzxy
    r6.xyzw = ((r6.xyzw)+(-(source[1].yzxy))).xyzw;
    // 230: add r1.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 231: add r1.xy, -r6.zwzz, r1.xyxx
    r1.xy = ((-(r6.zwzz))+(r1.xyxx)).xy;
    // 232: mad r1.xy, cb0[18].wwww, r1.xyxx, r6.zwzz
    r1.xy = ((source[18].wwww)*(r1.xyxx)+(r6.zwzz)).xy;
    // 233: mul r1.z, cb0[18].y, cb0[26].z
    r1.z = ((source[18].yyyy)*(source[26].zzzz)).z;
    // 234: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 235: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 236: mul r3.y, r1.z, l(0.020000)
    r3.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 237: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 238: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 239: mul r3.w, cb0[18].x, l(0.001000)
    r3.w = ((source[18].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 240: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 241: mad r1.xy, r3.wwww, r1.xyxx, r3.xyxx
    r1.xy = ((r3.wwww)*(r1.xyxx)+(r3.xyxx)).xy;
    // 242: dp2 r3.x, cb0[19].xyxx, r1.xyxx
    r3.x = (dot((source[19].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 243: dp2 r1.y, cb0[20].xyxx, r1.xyxx
    r1.y = (dot((source[20].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 244: frc r3.x, r3.x
    r3.x = (frac(r3.xxxx)).x;
    // 245: mul r1.x, r3.x, l(0.125000)
    r1.x = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 246: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r1.xyxx, t5.xyzw, s6, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 247: mad r3.xyw, r6.xyxz, l(3.500000, 3.500000, 0.000000, 3.500000), -r0.yzyw
    r3.xyw = ((r6.xyxz)*(float4(3.500000,3.500000,0.000000,3.500000))+(-(r0.yzyw))).xyw;
    // 248: mul r1.x, r6.w, l(0.900000)
    r1.x = ((r6.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 249: mad r3.xyw, r1.xxxx, r3.xyxw, r0.yzyw
    r3.xyw = ((r1.xxxx)*(r3.xyxw)+(r0.yzyw)).xyw;
    // 250: mul_sat r1.xyz, r1.zzzz, r3.xywx
    r1.xyz = (saturate((r1.zzzz)*(r3.xywx))).xyz;
    // 251: mad r3.xyw, cb0[18].zzzz, r1.xyxz, -r0.yzyw
    r3.xyw = ((source[18].zzzz)*(r1.xyxz)+(-(r0.yzyw))).xyw;
    // 252: mul r1.xyz, r1.xyzx, cb0[18].zzzz
    r1.xyz = ((r1.xyzx)*(source[18].zzzz)).xyz;
    // 253: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 254: mul r1.x, r1.x, l(3.000000)
    r1.x = ((r1.xxxx)*(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 255: mad r0.yzw, r1.xxxx, r3.xxyw, r0.yyzw
    r0.yzw = ((r1.xxxx)*(r3.xxyw)+(r0.yyzw)).yzw;
    // 256: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 257: add r1.x, -r3.z, r2.w
    r1.x = ((-(r3.zzzz))+(r2.wwww)).x;
    // 258: mad r3.xyw, r2.wwww, cb0[15].xyxz, -cb0[15].xyxz
    r3.xyw = ((r2.wwww)*(source[15].xyxz)+(-(source[15].xyxz))).xyw;
    // 259: mad r3.xyw, cb0[15].wwww, r3.xyxw, cb0[15].xyxz
    r3.xyw = ((source[15].wwww)*(r3.xyxw)+(source[15].xyxz)).xyw;
    // 260: mad r1.x, cb0[14].w, r1.x, r3.z
    r1.x = ((source[14].wwww)*(r1.xxxx)+(r3.zzzz)).x;
    // 261: mad r1.xyz, r1.xxxx, cb0[14].xyzx, r3.xywx
    r1.xyz = ((r1.xxxx)*(source[14].xyzx)+(r3.xywx)).xyz;
    // 262: add r2.w, cb0[2].y, cb0[2].x
    r2.w = ((source[2].yyyy)+(source[2].xxxx)).w;
    // 263: add r2.w, r2.w, cb0[2].z
    r2.w = ((r2.wwww)+(source[2].zzzz)).w;
    // 264: add r3.x, -r2.w, l(1000.000000)
    r3.x = ((-(r2.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).x;
    // 265: mad r2.w, cb0[26].w, r3.x, r2.w
    r2.w = ((source[26].wwww)*(r3.xxxx)+(r2.wwww)).w;
    // 266: mul r2.w, r2.w, l(0.010000)
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 267: mad r2.w, cb0[26].y, cb0[26].z, r2.w
    r2.w = ((source[26].yyyy)*(source[26].zzzz)+(r2.wwww)).w;
    // 268: mul r3.x, r2.w, l(3.524534)
    r3.x = ((r2.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).x;
    // 269: sincos null, r3.x, r3.x
    r3.x = (cos(r3.xxxx)).x;
    // 270: add r2.w, r2.w, r3.x
    r2.w = ((r2.wwww)+(r3.xxxx)).w;
    // 271: mul r2.w, r2.w, l(1.328987)
    r2.w = ((r2.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 272: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 273: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 274: mad r2.w, r2.w, l(0.500000), cb0[26].x
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[26].xxxx)).w;
    // 275: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t6.xyzw, s5, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 276: mul r6.xyz, cb0[13].xyzx, cb0[25].wwww
    r6.xyz = ((source[13].xyzx)*(source[25].wwww)).xyz;
    // 277: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 278: mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 279: mad r2.xyz, r1.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 280: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 281: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 282: mad r2.xyz, cb0[21].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 283: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 284: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 285: mad r2.xyz, cb0[21].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[21].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 286: mad r1.xyz, r2.xyzx, r12.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r12.xyzx)+(r1.xyzx)).xyz;
    // 287: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 288: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 289: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 290: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 291: mul r2.xyz, r1.wwww, cb0[16].xyzx
    r2.xyz = ((r1.wwww)*(source[16].xyzx)).xyz;
    // 292: movc r2.xyz, r0.xxxx, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 293: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 294: add r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(source[3].xyzx)).xyz;
    // 295: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 296: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 297: mul r2.xyz, r0.xxxx, r4.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 298: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 299: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 300: mul r3.xyz, r0.xxxx, v6.xyzx
    r3.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 301: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 302: mad r3.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 303: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 304: mul r3.yzw, r3.yyyy, cb0[28].xxyz
    r3.yzw = ((r3.yyyy)*(source[28].xxyz)).yzw;
    // 305: mad r3.xyz, r3.xxxx, cb0[27].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[27].xyzx)+(r3.yzwy)).xyz;
    // 306: mul r3.xyz, r3.xyzx, cb0[29].wwww
    r3.xyz = ((r3.xyzx)*(source[29].wwww)).xyz;
    // 307: mad r1.xyz, r3.xyzx, r0.yzwy, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 308: mul r3.xyz, r0.yzwy, r3.xyzx
    r3.xyz = ((r0.yzwy)*(r3.xyzx)).xyz;
    // 309: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 310: mad o0.xyz, r0.yzwy, cb0[29].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[29].xyzx)+(r1.xyzx)).xyz;
    // 311: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 312: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 313: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 314: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 315: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 316: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 317: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 318: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 319: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 320: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 321: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 322: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 323: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 324: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 325: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 326: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 327: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 328: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 329: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 330: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 331: ret
    return output;
}

// source.character.equipment-native-195.v1 / source program daeaa10d1c8bd94db36feae13f988984
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase195(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].y=1.f;
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[23]=g_SourceCharacterEnvironmentColor; source[24]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0;
    // 1: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 2: mul r0.x, r0.x, l(0.125000)
    r0.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 3: mul r1.y, cb0[7].y, cb0[15].y
    r1.y = ((source[7].yyyy)*(source[15].yyyy)).y;
    // 4: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 5: mov r1.xw, l(0,0,0,0)
    r1.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 6: add r0.xy, r0.xyxx, r1.xyxx
    r0.xy = ((r0.xyxx)+(r1.xyxx)).xy;
    // 7: frc r0.z, cb0[7].x
    r0.z = (frac(source[7].xxxx)).z;
    // 8: add r0.w, -r0.z, cb0[7].x
    r0.w = ((-(r0.zzzz))+(source[7].xxxx)).w;
    // 9: mul r1.z, r0.w, l(0.125000)
    r1.z = ((r0.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 10: add r0.xy, r0.xyxx, r1.zwzz
    r0.xy = ((r0.xyxx)+(r1.zwzz)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t4.xyzw, s5, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 12: mul r0.x, r0.z, r1.w
    r0.x = ((r0.zzzz)*(r1.wwww)).x;
    // 13: add r0.y, -cb0[7].w, l(1.000000)
    r0.y = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 14: mul r0.y, r0.y, cb0[17].z
    r0.y = ((r0.yyyy)*(source[17].zzzz)).y;
    // 15: mul r0.y, r0.y, l(6.283185)
    r0.y = ((r0.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
    // 16: sincos r0.y, null, r0.y
    r0.y = (sin(r0.yyyy)).y;
    // 17: add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 18: mul r0.z, cb0[7].z, l(1.500000)
    r0.z = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 19: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 20: mad r0.y, r0.y, l(0.500000), cb0[7].z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).y;
    // 21: mul r0.yzw, r1.xxyz, r0.yyyy
    r0.yzw = ((r1.xxyz)*(r0.yyyy)).yzw;
    // 22: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 23: max r2.xyz, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r2.xyz = (max(r1.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 24: max r1.xyz, r1.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 25: min r1.xyz, r1.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 26: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 27: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 29: log r4.xyz, |r3.xzyx|
    r4.xyz = (log2(abs(r3.xzyx))).xyz;
    // 30: lt r3.xyz, |r3.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (asfloat((uint4)((abs(r3.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 31: mul r1.w, r4.y, cb0[16].y
    r1.w = ((r4.yyyy)*(source[16].yyyy)).w;
    // 32: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 33: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: movc r1.w, r3.y, l(0), r1.w
    r1.w = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 35: mad r1.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 36: mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 37: max r5.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r5.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 38: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 39: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 40: min r5.xyz, r5.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 41: add r5.xyz, -r2.xyzx, r5.xyzx
    r5.xyz = ((-(r2.xyzx))+(r5.xyzx)).xyz;
    // 42: mad r2.xyz, r1.wwww, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 43: add r1.xyz, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)+(-(r2.xyzx))).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: mad r1.xyz, r5.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r5.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 46: mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 47: max r6.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 48: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 49: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 50: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 51: add r6.xyz, -r2.xyzx, r6.xyzx
    r6.xyz = ((-(r2.xyzx))+(r6.xyzx)).xyz;
    // 52: mad r2.xyz, r1.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 53: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 54: mad r1.xyz, r5.yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((r5.yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 55: mul r2.xyz, cb0[6].xyzx, cb0[6].wwww
    r2.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 56: max r6.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 57: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 58: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 59: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 60: add r6.xyz, -r2.xyzx, r6.xyzx
    r6.xyz = ((-(r2.xyzx))+(r6.xyzx)).xyz;
    // 61: mad r2.xyz, r1.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 62: mul_sat r6.w, r1.w, cb2[3].w
    r6.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 63: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 64: mad r1.xyz, r5.zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((r5.zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 65: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 66: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 67: mad r2.xyz, cb0[17].wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((source[17].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 68: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 69: add r7.xyz, -r2.xyzx, r1.wwww
    r7.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 70: mad r2.xyz, cb0[18].xxxx, r7.xyzx, r2.xyzx
    r2.xyz = ((source[18].xxxx)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 71: mad r7.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 72: mad r8.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 73: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 74: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s1, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 76: dp3 r1.w, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r9.xyz, -r8.yzwy, r1.wwww
    r9.xyz = ((-(r8.yzwy))+(r1.wwww)).xyz;
    // 78: mad r9.xyz, cb0[17].wwww, r9.xyzx, r8.yzwy
    r9.xyz = ((source[17].wwww)*(r9.xyzx)+(r8.yzwy)).xyz;
    // 79: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 80: add r10.xyz, -r9.xyzx, r1.wwww
    r10.xyz = ((-(r9.xyzx))+(r1.wwww)).xyz;
    // 81: mad r9.xyz, cb0[18].xxxx, r10.xyzx, r9.xyzx
    r9.xyz = ((source[18].xxxx)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 82: mul r10.xyz, r2.xyzx, r9.xyzx
    r10.xyz = ((r2.xyzx)*(r9.xyzx)).xyz;
    // 83: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 84: mad r2.xyz, -r2.xyzx, r9.xyzx, r1.wwww
    r2.xyz = ((-(r2.xyzx))*(r9.xyzx)+(r1.wwww)).xyz;
    // 85: mad r2.xyz, cb0[17].wwww, r2.xyzx, r10.xyzx
    r2.xyz = ((source[17].wwww)*(r2.xyzx)+(r10.xyzx)).xyz;
    // 86: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 87: add r9.xyz, -r2.xyzx, r1.wwww
    r9.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 88: mad r2.xyz, cb0[18].xxxx, r9.xyzx, r2.xyzx
    r2.xyz = ((source[18].xxxx)*(r9.xyzx)+(r2.xyzx)).xyz;
    // 89: mul r2.xyz, r7.xyzx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 90: mad r0.yzw, r0.yyzw, l(0.000000, 2.000000, 2.000000, 2.000000), -r2.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r2.xxyz))).yzw;
    // 91: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 92: mul r0.w, r4.x, cb0[19].x
    r0.w = ((r4.xxxx)*(source[19].xxxx)).w;
    // 93: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 94: movc r0.w, r3.x, l(0), r0.w
    r0.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 95: add_sat r0.w, r0.w, cb0[19].y
    r0.w = (saturate((r0.wwww)+(source[19].yyyy))).w;
    // 96: add r1.w, r0.w, l(-1.000000)
    r1.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 97: mad r1.w, cb0[19].w, r1.w, l(1.000000)
    r1.w = ((source[19].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul r2.xyz, r0.xyzx, r1.wwww
    r2.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 99: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: mul r3.xyw, r2.wwww, cb0[14].xyxz
    r3.xyw = ((r2.wwww)*(source[14].xyxz)).xyw;
    // 101: mul r2.xyz, r2.xyzx, r3.xywx
    r2.xyz = ((r2.xyzx)*(r3.xywx)).xyz;
    // 102: mad r0.xyz, r1.wwww, r0.xyzx, -r2.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 103: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 104: add r2.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 105: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 106: mad_sat r2.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 107: mad r0.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r0.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 108: mad r3.xyw, r2.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r3.xyw = ((r2.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 109: mad r0.xyz, r0.wwww, r0.xyzx, r3.xywx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r3.xywx)).xyz;
    // 110: mad r3.xyw, r2.xyxz, l(2.755200, 2.755200, 0.000000, 2.755200), l(0.690300, 0.690300, 0.000000, 0.690300)
    r3.xyw = ((r2.xyxz)*(float4(2.755200,2.755200,0.000000,2.755200))+(float4(0.690300,0.690300,0.000000,0.690300))).xyw;
    // 111: mad r0.xyz, r0.xyzx, r0.wwww, r3.xywx
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r3.xywx)).xyz;
    // 112: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 113: max r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (max(r0.xyzx,r0.wwww)).xyz;
    // 114: mov_sat r2.w, cb0[20].x
    r2.w = (saturate(source[20].xxxx)).w;
    // 115: mad r3.xyw, -r2.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r2.xyxz
    r3.xyw = ((-(r2.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r2.xyxz)).xyw;
    // 116: mul r1.w, r2.w, l(0.080000)
    r1.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 117: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 118: mad r3.xyw, r6.wwww, r3.xyxw, r1.wwww
    r3.xyw = ((r6.wwww)*(r3.xyxw)+(r1.wwww)).xyw;
    // 119: add r1.w, -cb0[21].y, cb0[21].x
    r1.w = ((-(source[21].yyyy))+(source[21].xxxx)).w;
    // 120: mad r1.w, r5.x, r1.w, cb0[21].y
    r1.w = ((r5.xxxx)*(r1.wwww)+(source[21].yyyy)).w;
    // 121: add r2.w, -r1.w, cb0[21].w
    r2.w = ((-(r1.wwww))+(source[21].wwww)).w;
    // 122: mad r1.w, r5.y, r2.w, r1.w
    r1.w = ((r5.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 123: add r2.w, -r1.w, cb0[22].y
    r2.w = ((-(r1.wwww))+(source[22].yyyy)).w;
    // 124: mad r1.w, r5.z, r2.w, r1.w
    r1.w = ((r5.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 125: mul r1.w, r4.z, r1.w
    r1.w = ((r4.zzzz)*(r1.wwww)).w;
    // 126: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 127: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: movc r1.w, r3.z, l(0), r1.w
    r1.w = ((asuint(r3.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 129: max r1.w, r1.w, cb0[1].x
    r1.w = (max(r1.wwww,source[1].xxxx)).w;
    // 130: min r6.z, r1.w, l(1.000000)
    r6.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 131: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 132: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 133: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 134: mul r4.xy, r4.xyxx, cb0[16].xxxx
    r4.xy = ((r4.xyxx)*(source[16].xxxx)).xy;
    // 135: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 137: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 138: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 139: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 140: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 141: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 142: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 143: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 144: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 145: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 146: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 147: mul r9.xyz, r1.wwww, v6.xyzx
    r9.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 148: dp3 r1.w, r5.xyzx, r9.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 149: deriv_rtx_coarse r6.x, r1.w
    r6.x = (ddx_coarse(r1.wwww)).x;
    // 150: deriv_rty_coarse r6.y, r1.w
    r6.y = (ddy_coarse(r1.wwww)).y;
    // 151: dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 152: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 153: mad r2.w, r2.w, l(0.300000), r6.z
    r2.w = ((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz)).w;
    // 154: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 155: min r6.y, r2.w, l(1.000000)
    r6.y = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 156: add r2.w, -r6.y, l(1.000000)
    r2.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: max r10.xyz, r3.xywx, r2.wwww
    r10.xyz = (max(r3.xywx,r2.wwww)).xyz;
    // 158: add r10.xyz, -r3.xywx, r10.xyzx
    r10.xyz = ((-(r3.xywx))+(r10.xyzx)).xyz;
    // 159: mul_sat r2.w, r3.y, l(50.000000)
    r2.w = (saturate((r3.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 160: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 161: mul r11.xyz, r1.wwww, r5.xyzx
    r11.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 162: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 164: add r2.w, r11.z, l(1.000000)
    r2.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: add_sat r6.x, r1.w, -r2.w
    r6.x = (saturate((r1.wwww)+(-(r2.wwww)))).x;
    // 167: sample_indexable(texture2d)(float,float,float,float) r12.xy, r6.xyxx, t6.xyzw, s7
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 168: add r1.w, r0.w, r6.x
    r1.w = ((r0.wwww)+(r6.xxxx)).w;
    // 169: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 170: mul r13.xyz, r3.xywx, r12.yyyy
    r13.xyz = ((r3.xywx)*(r12.yyyy)).xyz;
    // 171: mad r10.xyz, r10.xyzx, r12.xxxx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xxxx)+(r13.xyzx)).xyz;
    // 172: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r2.w = r12.y != 0.f ? 1.f / r12.y : 0.f;
    // 173: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 174: mad r12.xyz, r3.xywx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r3.xywx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 175: dp3 r2.w, r3.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 176: mad r3.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r3.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 177: mad r13.xyz, -r10.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r10.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 179: mul r12.xyz, r2.xyzx, r13.xyzx
    r12.xyz = ((r2.xyzx)*(r13.xyzx)).xyz;
    // 180: add r2.w, -r6.w, l(1.000000)
    r2.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 181: mul r12.xyz, r2.wwww, r12.xyzx
    r12.xyz = ((r2.wwww)*(r12.xyzx)).xyz;
    // 182: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 183: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 184: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 185: mul r14.xyz, r3.wwww, v1.xyzx
    r14.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 186: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 187: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 188: mul r15.xyz, r3.wwww, v0.xyzx
    r15.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 189: mul r16.xyz, r14.zxyz, r15.yzxy
    r16.xyz = ((r14.zxyz)*(r15.yzxy)).xyz;
    // 190: mad r16.xyz, r14.yzxy, r15.zxyz, -r16.xyzx
    r16.xyz = ((r14.yzxy)*(r15.zxyz)+(-(r16.xyzx))).xyz;
    // 191: mul r16.xyz, r16.xyzx, v1.wwww
    r16.xyz = ((r16.xyzx)*(v1.wwww)).xyz;
    // 192: dp3 r17.y, r16.xyzx, r5.xyzx
    r17.y = (dot((r16.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 193: dp3 r16.y, r16.xyzx, r11.xyzx
    r16.y = (dot((r16.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 194: dp3 r17.x, r15.xyzx, r5.xyzx
    r17.x = (dot((r15.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 195: dp3 r16.x, r15.xyzx, r11.xyzx
    r16.x = (dot((r15.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 196: dp2 r15.z, r17.xyxx, cb0[24].xyxx
    r15.z = (dot((r17.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 197: mul r6.xz, cb0[24].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r6.xz = ((source[24].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 198: dp2 r15.x, r17.xyxx, r6.xzxx
    r15.x = (dot((r17.xyxx).xy,(r6.xzxx).xy).xxxx).x;
    // 199: dp2 r18.x, r16.xyxx, r6.xzxx
    r18.x = (dot((r16.xyxx).xy,(r6.xzxx).xy).xxxx).x;
    // 200: dp2 r18.z, r16.xyxx, cb0[24].xyxx
    r18.z = (dot((r16.xyxx).xy,(source[24].xyxx).xy).xxxx).z;
    // 201: dp3 r15.y, r14.xyzx, r5.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 202: dp3 r18.y, r14.xyzx, r11.xyzx
    r18.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 203: mov r15.w, l(1.000000)
    r15.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 204: dp4 r14.x, cb0[25].xyzw, r15.xyzw
    r14.x = (dot((source[25].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 205: dp4 r14.y, cb0[26].xyzw, r15.xyzw
    r14.y = (dot((source[26].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 206: dp4 r14.z, cb0[27].xyzw, r15.xyzw
    r14.z = (dot((source[27].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 207: mul r16.xyzw, r15.yzzx, r15.xyzz
    r16.xyzw = ((r15.yzzx)*(r15.xyzz)).xyzw;
    // 208: dp4 r19.x, cb0[28].xyzw, r16.xyzw
    r19.x = (dot((source[28].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 209: dp4 r19.y, cb0[29].xyzw, r16.xyzw
    r19.y = (dot((source[29].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 210: dp4 r19.z, cb0[30].xyzw, r16.xyzw
    r19.z = (dot((source[30].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 211: add r14.xyz, r14.xyzx, r19.xyzx
    r14.xyz = ((r14.xyzx)+(r19.xyzx)).xyz;
    // 212: mul r3.w, r15.y, r15.y
    r3.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 213: mov r17.z, r15.y
    r17.z = (r15.yyyy).z;
    // 214: mad r3.w, r15.x, r15.x, -r3.w
    r3.w = ((r15.xxxx)*(r15.xxxx)+(-(r3.wwww))).w;
    // 215: mad r14.xyz, cb0[31].xyzx, r3.wwww, r14.xyzx
    r14.xyz = ((source[31].xyzx)*(r3.wwww)+(r14.xyzx)).xyz;
    // 216: max r14.xyz, r14.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r14.xyz = (max(r14.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 217: mul r14.xyz, r14.xyzx, cb0[23].xyzx
    r14.xyz = ((r14.xyzx)*(source[23].xyzx)).xyz;
    // 218: mul r14.xyz, r14.xyzx, cb0[24].zzzz
    r14.xyz = ((r14.xyzx)*(source[24].zzzz)).xyz;
    // 219: mad r14.xyz, r14.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[23].wwww
    r14.xyz = ((r14.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[23].wwww)).xyz;
    // 220: dp3 r3.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 221: add r14.xyz, -r3.wwww, r14.xyzx
    r14.xyz = ((-(r3.wwww))+(r14.xyzx)).xyz;
    // 222: mad r14.xyz, r14.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r14.xyz = ((r14.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 223: dp3 r3.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 224: mad r4.w, r6.y, l(2.000000), l(2.000000)
    r4.w = ((r6.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 225: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 226: mad r3.w, r2.w, l(5.000000), r3.w
    r3.w = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 227: add_sat r3.w, r6.w, r3.w
    r3.w = (saturate((r6.wwww)+(r3.wwww))).w;
    // 228: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 229: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 230: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 231: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 232: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 233: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 234: mul r14.xyz, r3.wwww, r14.xyzx
    r14.xyz = ((r3.wwww)*(r14.xyzx)).xyz;
    // 235: mul r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)*(r14.xyzx)).xyz;
    // 236: mul r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)*(r14.xyzx)).xyz;
    // 237: mul r12.xyz, r0.xyzx, r12.xyzx
    r12.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 238: mul r3.w, r6.y, l(5.000000)
    r3.w = ((r6.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 239: mul r5.w, r6.y, r6.y
    r5.w = ((r6.yyyy)*(r6.yyyy)).w;
    // 240: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 241: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 242: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 243: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 244: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 245: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r18.xyzx, t7.xyzw, s6, r3.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r18.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 246: mul r6.xyz, r14.xyzx, r14.wwww
    r6.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 247: mul r6.xyz, r6.xyzx, cb0[23].xyzx
    r6.xyz = ((r6.xyzx)*(source[23].xyzx)).xyz;
    // 248: mul r6.xyz, r6.xyzx, cb0[24].zzzz
    r6.xyz = ((r6.xyzx)*(source[24].zzzz)).xyz;
    // 249: mad r6.xyz, r6.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[23].wwww
    r6.xyz = ((r6.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[23].wwww)).xyz;
    // 250: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 251: add r6.xyz, -r1.wwww, r6.xyzx
    r6.xyz = ((-(r1.wwww))+(r6.xyzx)).xyz;
    // 252: mad r6.xyz, r6.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r6.xyz = ((r6.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 253: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 254: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 255: mad r1.w, r2.w, l(5.000000), r1.w
    r1.w = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 256: add_sat r1.w, r6.w, r1.w
    r1.w = (saturate((r6.wwww)+(r1.wwww))).w;
    // 257: mad r2.w, r1.w, l(-2.000000), l(3.000000)
    r2.w = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 258: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 259: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 260: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 261: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 262: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 263: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 264: mul r14.xyz, r6.xyzx, r10.xyzx
    r14.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 265: mad r1.w, r0.w, r3.x, r3.y
    r1.w = ((r0.wwww)*(r3.xxxx)+(r3.yyyy)).w;
    // 266: mad r1.w, r1.w, r0.w, r3.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r3.zzzz)).w;
    // 267: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 268: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 269: mad r3.xyz, r14.xyzx, r0.wwww, r12.xyzx
    r3.xyz = ((r14.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 270: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 271: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 272: mul r12.xyz, r1.wwww, v7.xyzx
    r12.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 273: dp3 r1.w, r12.xyzx, r5.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 274: dp3 r2.w, r12.xyzx, r11.xyzx
    r2.w = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 275: mad r5.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 276: mad r5.zw, r1.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r5.zw = ((r1.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 277: mul r5.xyzw, r5.xyzw, r5.xyzw
    r5.xyzw = ((r5.xyzw)*(r5.xyzw)).xyzw;
    // 278: mul r11.xyz, r5.wwww, cb0[34].xyzx
    r11.xyz = ((r5.wwww)*(source[34].xyzx)).xyz;
    // 279: mad r11.xyz, r5.zzzz, cb0[33].xyzx, r11.xyzx
    r11.xyz = ((r5.zzzz)*(source[33].xyzx)+(r11.xyzx)).xyz;
    // 280: mul r11.xyz, r11.xyzx, cb0[35].wwww
    r11.xyz = ((r11.xyzx)*(source[35].wwww)).xyz;
    // 281: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 282: mul r0.xyz, r0.xyzx, r11.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 283: mul r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 284: mul r0.xyz, r13.xyzx, r0.xyzx
    r0.xyz = ((r13.xyzx)*(r0.xyzx)).xyz;
    // 285: mad r0.xyz, -r0.xyzx, r6.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r6.wwww)+(r0.xyzx)).xyz;
    // 286: mad r0.xyz, r3.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r0.xyzx)).xyz;
    // 287: mul r3.xyz, r5.yyyy, cb0[34].xyzx
    r3.xyz = ((r5.yyyy)*(source[34].xyzx)).xyz;
    // 288: mad r3.xyz, cb0[33].xyzx, r5.xxxx, r3.xyzx
    r3.xyz = ((source[33].xyzx)*(r5.xxxx)+(r3.xyzx)).xyz;
    // 289: mul r3.xyz, r3.xyzx, cb0[35].wwww
    r3.xyz = ((r3.xyzx)*(source[35].wwww)).xyz;
    // 290: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 291: mul r3.xyz, r6.xyzx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 292: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 293: mad r0.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 294: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 295: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 296: dp3 r0.w, r4.xyzx, r9.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 297: mul_sat r1.w, r0.w, cb0[18].y
    r1.w = (saturate((r0.wwww)*(source[18].yyyy))).w;
    // 298: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 299: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 300: mul_sat r2.w, r9.z, cb0[18].y
    r2.w = (saturate((r9.zzzz)*(source[18].yyyy))).w;
    // 301: add r3.x, -|r9.z|, l(1.000000)
    r3.x = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 302: mul r0.w, r0.w, r3.x
    r0.w = ((r0.wwww)*(r3.xxxx)).w;
    // 303: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 304: add_sat r2.w, r2.w, -cb0[18].z
    r2.w = (saturate((r2.wwww)+(-(source[18].zzzz)))).w;
    // 305: log r3.x, r2.w
    r3.x = (log2(r2.wwww)).x;
    // 306: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 307: mul r3.x, r3.x, cb0[18].w
    r3.x = ((r3.xxxx)*(source[18].wwww)).x;
    // 308: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 309: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 310: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 311: mad r3.xyz, r1.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r3.xyz = ((r1.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 312: mul r1.w, r1.w, cb0[11].w
    r1.w = ((r1.wwww)*(source[11].wwww)).w;
    // 313: mad r3.xyz, cb0[12].wwww, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((source[12].wwww)*(r3.xyzx)+(source[12].xyzx)).xyz;
    // 314: mad r3.xyz, r1.wwww, cb0[11].xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(source[11].xyzx)+(r3.xyzx)).xyz;
    // 315: add r1.w, cb0[0].y, cb0[0].x
    r1.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 316: add r1.w, r1.w, cb0[0].z
    r1.w = ((r1.wwww)+(source[0].zzzz)).w;
    // 317: mul r1.w, r1.w, l(0.010000)
    r1.w = ((r1.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 318: mad r1.w, cb0[17].y, cb0[17].z, r1.w
    r1.w = ((source[17].yyyy)*(source[17].zzzz)+(r1.wwww)).w;
    // 319: mul r2.w, r1.w, l(3.524534)
    r2.w = ((r1.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 320: sincos null, r2.w, r2.w
    r2.w = (cos(r2.wwww)).w;
    // 321: add r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)+(r2.wwww)).w;
    // 322: mul r1.w, r1.w, l(1.328987)
    r1.w = ((r1.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 323: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 324: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 325: mad r1.w, r1.w, l(0.500000), cb0[17].x
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[17].xxxx)).w;
    // 326: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 327: mul r5.xyz, cb0[8].xyzx, cb0[16].wwww
    r5.xyz = ((source[8].xyzx)*(source[16].wwww)).xyz;
    // 328: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 329: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 330: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 331: mad r4.xyz, -r1.wwww, r4.xyzx, r2.wwww
    r4.xyz = ((-(r1.wwww))*(r4.xyzx)+(r2.wwww)).xyz;
    // 332: mad r4.xyz, cb0[17].wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((source[17].wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 333: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 334: add r5.xyz, -r4.xyzx, r1.wwww
    r5.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 335: mad r4.xyz, cb0[18].xxxx, r5.xyzx, r4.xyzx
    r4.xyz = ((source[18].xxxx)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 336: mad r3.xyz, r4.xyzx, r7.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 337: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 338: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 339: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 340: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 341: mul r4.xyz, r1.wwww, cb0[13].xyzx
    r4.xyz = ((r1.wwww)*(source[13].xyzx)).xyz;
    // 342: movc r4.xyz, r0.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 343: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 344: mul r1.xyz, r1.xyzx, r8.yzwy
    r1.xyz = ((r1.xyzx)*(r8.yzwy)).xyz;
    // 345: mov_sat r8.x, r8.x
    r8.x = (saturate(r8.xxxx)).x;
    // 346: mul_sat r0.w, r8.x, cb0[20].y
    r0.w = (saturate((r8.xxxx)*(source[20].yyyy))).w;
    // 347: mul o0.w, r0.w, cb0[1].y
    output.targets[0].w = ((r0.wwww)*(source[1].yyyy)).w;
    // 348: mad r1.xyz, cb0[16].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[16].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 349: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 350: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 351: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 352: mad r0.xyz, r2.xyzx, cb0[35].xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(source[35].xyzx)+(r1.xyzx)).xyz;
    // 353: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 354: dp3 r0.x, r17.xyzx, r17.xyzx
    r0.x = (dot((r17.xyzx).xyz,(r17.xyzx).xyz).xxxx).x;
    // 355: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 356: mul r0.xyz, r0.xxxx, r17.xyzx
    r0.xyz = ((r0.xxxx)*(r17.xyzx)).xyz;
    // 357: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 358: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 359: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 360: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 361: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 362: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 363: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 364: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 365: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 366: ftou r0.x, cb0[32].z
    r0.x = (asfloat((uint4)(source[32].zzzz))).x;
    // 367: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 368: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 369: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 370: mov o5.xz, l(0,0,0.450000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),0.450000,asfloat(0u))).xz;
    // 371: ret
    return output;
}

// source.character.equipment-native-196.v1 / source program 4734cecc349738418a40c8db414d236e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase196(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[18]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].y=(g_SourceCharacterTime.xxxx).x;
    source[24].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[24].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
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
    // 22: add r1.y, -cb0[21].y, cb0[21].x
    r1.y = ((-(source[21].yyyy))+(source[21].xxxx)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 24: mad r1.y, r2.x, r1.y, cb0[21].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[21].yyyy)).y;
    // 25: add r1.z, -r1.y, cb0[21].z
    r1.z = ((-(r1.yyyy))+(source[21].zzzz)).z;
    // 26: mad r1.y, r2.y, r1.z, r1.y
    r1.y = ((r2.yyyy)*(r1.zzzz)+(r1.yyyy)).y;
    // 27: add r1.z, -r1.y, cb0[21].w
    r1.z = ((-(r1.yyyy))+(source[21].wwww)).z;
    // 28: mad r1.y, r2.z, r1.z, r1.y
    r1.y = ((r2.zzzz)*(r1.zzzz)+(r1.yyyy)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 30: add r1.z, -r3.w, l(1.000000)
    r1.z = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 32: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 33: mul r1.y, r1.w, r1.y
    r1.y = ((r1.wwww)*(r1.yyyy)).y;
    // 34: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 35: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 37: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 38: add r1.w, -cb0[20].y, cb0[20].x
    r1.w = ((-(source[20].yyyy))+(source[20].xxxx)).w;
    // 39: mad r1.w, r2.x, r1.w, cb0[20].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[20].yyyy)).w;
    // 40: add r3.w, -r1.w, cb0[20].z
    r3.w = ((-(r1.wwww))+(source[20].zzzz)).w;
    // 41: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 42: add r3.w, -r1.w, cb0[20].w
    r3.w = ((-(r1.wwww))+(source[20].wwww)).w;
    // 43: mad r1.w, r2.z, r3.w, r1.w
    r1.w = ((r2.zzzz)*(r3.wwww)+(r1.wwww)).w;
    // 44: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 45: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 46: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 47: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 48: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 50: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 51: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 52: mul r4.xy, r1.xzxx, cb0[19].xxxx
    r4.xy = ((r1.xzxx)*(source[19].xxxx)).xy;
    // 53: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 55: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 56: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 57: add r1.xzw, -r4.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r4.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 58: mad r1.xzw, cb0[19].wwww, r1.xxzw, r4.xxyz
    r1.xzw = ((source[19].wwww)*(r1.xxzw)+(r4.xxyz)).xzw;
    // 59: dp3 r3.w, r1.xzwx, r1.xzwx
    r3.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 60: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 61: div r1.xzw, r1.xxzw, r3.wwww
    r1.xzw = ((r1.xxzw)/(r3.wwww)).xzw;
    // 62: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 63: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 64: mul r5.xyz, r3.wwww, v0.xyzx
    r5.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 65: dp3 r6.x, r5.xyzx, r1.xzwx
    r6.x = (dot((r5.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 66: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 67: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 68: mul r7.xyz, r3.wwww, v1.xyzx
    r7.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 69: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 70: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 71: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 72: dp3 r6.y, r8.xyzx, r1.xzwx
    r6.y = (dot((r8.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 73: dp3 r6.z, r7.xyzx, r1.xzwx
    r6.z = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 74: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 75: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 76: mul r9.xyz, r1.xxxx, v5.xyzx
    r9.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 77: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 78: dp3 r10.y, r8.xyzx, r9.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 79: dp3 r10.x, r5.xyzx, r9.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 80: dp3 r10.z, r7.xyzx, r9.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 81: dp3 r3.w, r6.xyzx, r10.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 82: mul r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)*(r3.wwww)).xyz;
    // 83: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 84: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 85: dp2 r3.w, r6.ywyy, r6.ywyy
    r3.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 86: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 87: div r6.xy, r6.ywyy, r3.wwww
    r6.xy = ((r6.ywyy)/(r3.wwww)).xy;
    // 88: mad r3.w, -r6.z, l(0.250000), l(0.250000)
    r3.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 89: add r4.w, r6.z, l(1.000000)
    r4.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 91: mad r6.xy, r3.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r3.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 92: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s4, r0.x
    r6.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 93: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 94: rcp r0.x, cb0[22].x
    r0.x = (1.0/(source[22].xxxx)).x;
    // 95: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 96: mul r10.xyz, r10.xyzx, cb0[22].xxxx
    r10.xyz = ((r10.xyzx)*(source[22].xxxx)).xyz;
    // 97: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 98: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 99: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 100: mad r10.xyz, r10.xyzx, cb0[22].xxxx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[22].xxxx)+(r11.xyzx)).xyz;
    // 101: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 102: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 103: add r0.x, cb0[22].x, l(1.000000)
    r0.x = ((source[22].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 104: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 105: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 106: add r6.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r6.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 107: mad r6.xyz, r4.wwww, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 108: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 109: mul r6.xyz, r6.xyzx, cb0[22].yyyy
    r6.xyz = ((r6.xyzx)*(source[22].yyyy)).xyz;
    // 110: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 111: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 112: mad r3.xyz, cb0[19].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[19].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 113: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 114: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 115: mad r3.xyz, cb0[19].zzzz, r10.xyzx, r3.xyzx
    r3.xyz = ((source[19].zzzz)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 116: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 117: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 118: mul r10.xyz, r10.xyzx, cb0[22].zzzz
    r10.xyz = ((r10.xyzx)*(source[22].zzzz)).xyz;
    // 119: add r0.x, r2.y, r2.x
    r0.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 120: add r0.x, r2.z, r0.x
    r0.x = ((r2.zzzz)+(r0.xxxx)).x;
    // 121: add_sat r0.x, r2.w, r0.x
    r0.x = (saturate((r2.wwww)+(r0.xxxx))).x;
    // 122: mad r3.xyz, r0.xxxx, r10.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 123: max r10.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 124: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 125: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 126: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 127: dp3 r0.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 128: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 129: mul r0.x, r0.x, cb0[23].z
    r0.x = ((r0.xxxx)*(source[23].zzzz)).x;
    // 130: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 131: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 132: mad r2.w, -r0.x, r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 134: div r2.w, cb0[23].w, r2.w
    r2.w = ((source[23].wwww)/(r2.wwww)).w;
    // 135: dp3 r3.w, r4.xyzx, r4.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 136: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 137: div r4.xyz, r4.xyzx, r3.wwww
    r4.xyz = ((r4.xyzx)/(r3.wwww)).xyz;
    // 138: dp3 r3.w, r4.xyzx, r9.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 139: mul_sat r4.w, r3.w, cb0[22].w
    r4.w = (saturate((r3.wwww)*(source[22].wwww))).w;
    // 140: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: mul_sat r5.w, r9.z, cb0[22].w
    r5.w = (saturate((r9.zzzz)*(source[22].wwww))).w;
    // 143: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: add_sat r5.w, r5.w, -cb0[23].x
    r5.w = (saturate((r5.wwww)+(-(source[23].xxxx)))).w;
    // 145: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 146: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 147: mul r6.w, r6.w, cb0[23].y
    r6.w = ((r6.wwww)*(source[23].yyyy)).w;
    // 148: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 149: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 150: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 151: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 152: mul r10.xyz, r6.xyzx, r2.wwww
    r10.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 153: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 154: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 155: mad r0.yzw, cb0[19].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 156: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 157: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 158: mad r0.yzw, cb0[19].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[19].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 159: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 160: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 161: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 162: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 163: mad r2.xyw, r2.yyyy, r12.xyxz, r11.xyxz
    r2.xyw = ((r2.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 164: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, -r2.xywx
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r2.xywx))).xyz;
    // 165: mad r2.xyz, r2.zzzz, r11.xyzx, r2.xywx
    r2.xyz = ((r2.zzzz)*(r11.xyzx)+(r2.xywx)).xyz;
    // 166: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 167: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 168: mad r2.xyz, cb0[19].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[19].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 169: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 170: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 171: mad r2.xyz, cb0[19].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 172: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 173: mad r12.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 174: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 175: mul r2.xyz, r2.xyzx, r11.xyzx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 176: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 177: mul r2.xyz, r6.xyzx, r0.yzwy
    r2.xyz = ((r6.xyzx)*(r0.yzwy)).xyz;
    // 178: mad r3.xyz, r3.xyzx, r10.xyzx, -r2.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)+(-(r2.xyzx))).xyz;
    // 179: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: mul r2.w, r2.w, cb0[24].x
    r2.w = ((r2.wwww)*(source[24].xxxx)).w;
    // 181: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 182: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 183: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 184: div r1.xzw, r1.xxzw, r3.xxxx
    r1.xzw = ((r1.xxzw)/(r3.xxxx)).xzw;
    // 185: dp3 r1.x, r1.xzwx, r9.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 186: add r1.z, -|r9.z|, l(1.000000)
    r1.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 187: mul r1.z, r3.w, r1.z
    r1.z = ((r3.wwww)*(r1.zzzz)).z;
    // 188: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 189: mul r1.w, |r1.x|, |r1.x|
    r1.w = ((abs(r1.xxxx))*(abs(r1.xxxx))).w;
    // 190: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 191: mul r1.w, r1.w, |r1.x|
    r1.w = ((r1.wwww)*(abs(r1.xxxx))).w;
    // 192: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 193: movc r1.x, r1.x, l(0), r1.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).x;
    // 194: add r1.w, r1.x, l(-0.027778)
    r1.w = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 195: mad r1.x, r1.x, r1.w, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 196: div_sat r1.x, r1.x, r2.w
    r1.x = (saturate((r1.xxxx)/(r2.wwww))).x;
    // 197: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 198: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 199: mad r1.xyw, r1.xxxx, r2.xyxz, -r0.yzyw
    r1.xyw = ((r1.xxxx)*(r2.xyxz)+(-(r0.yzyw))).xyw;
    // 200: mad r0.xyz, r0.xxxx, r1.xywx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xywx)+(r0.yzwy)).xyz;
    // 201: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 202: add r1.xyw, -r0.xyxz, r0.wwww
    r1.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 203: mad r0.xyz, cb0[19].yyyy, r1.xywx, r0.xyzx
    r0.xyz = ((source[19].yyyy)*(r1.xywx)+(r0.xyzx)).xyz;
    // 204: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 205: add r1.xyw, -r0.xyxz, r0.wwww
    r1.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 206: mad r0.xyz, cb0[19].zzzz, r1.xywx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r1.xywx)+(r0.xyzx)).xyz;
    // 207: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 208: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 209: mul r0.w, r0.w, cb0[24].y
    r0.w = ((r0.wwww)*(source[24].yyyy)).w;
    // 210: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 211: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 212: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 213: mul r1.x, cb0[3].z, l(1.500000)
    r1.x = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 214: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 215: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 216: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 217: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 218: mul r3.y, cb0[3].y, cb0[15].y
    r3.y = ((source[3].yyyy)*(source[15].yyyy)).y;
    // 219: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 220: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 221: add r1.xy, r1.xyxx, r3.xyxx
    r1.xy = ((r1.xyxx)+(r3.xyxx)).xy;
    // 222: frc r1.w, cb0[3].x
    r1.w = (frac(source[3].xxxx)).w;
    // 223: add r2.w, -r1.w, cb0[3].x
    r2.w = ((-(r1.wwww))+(source[3].xxxx)).w;
    // 224: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 225: add r1.xy, r1.xyxx, r3.zwzz
    r1.xy = ((r1.xyxx)+(r3.zwzz)).xy;
    // 226: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 227: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 228: mul r0.w, r1.w, r3.w
    r0.w = ((r1.wwww)*(r3.wwww)).w;
    // 229: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 230: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 231: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 232: add r3.xyzw, v7.yzxy, cb0[0].yzxy
    r3.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 233: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 234: add r1.yw, -r3.xxxy, l(0.000000, 1.000000, 0.000000, 1.000000)
    r1.yw = ((-(r3.xxxy))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 235: add r1.yw, -r3.zzzw, r1.yyyw
    r1.yw = ((-(r3.zzzw))+(r1.yyyw)).yw;
    // 236: mad r1.yw, cb0[16].wwww, r1.yyyw, r3.zzzw
    r1.yw = ((source[16].wwww)*(r1.yyyw)+(r3.zzzw)).yw;
    // 237: mul r0.w, cb0[16].y, cb0[24].y
    r0.w = ((source[16].yyyy)*(source[24].yyyy)).w;
    // 238: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 239: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 240: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 241: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 242: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 243: mul r2.w, cb0[16].x, l(0.001000)
    r2.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 244: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 245: mad r1.yw, r2.wwww, r1.yyyw, r3.xxxy
    r1.yw = ((r2.wwww)*(r1.yyyw)+(r3.xxxy)).yw;
    // 246: dp2 r2.w, cb0[17].xyxx, r1.ywyy
    r2.w = (dot((source[17].xyxx).xy,(r1.ywyy).xy).xxxx).w;
    // 247: dp2 r3.y, cb0[18].xyxx, r1.ywyy
    r3.y = (dot((source[18].xyxx).xy,(r1.ywyy).xy).xxxx).y;
    // 248: frc r1.y, r2.w
    r1.y = (frac(r2.wwww)).y;
    // 249: mul r3.x, r1.y, l(0.125000)
    r3.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 250: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 251: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 252: mul r1.y, r3.w, l(0.900000)
    r1.y = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 253: mad r3.xyz, r1.yyyy, r3.xyzx, r0.xyzx
    r3.xyz = ((r1.yyyy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 254: mul_sat r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = (saturate((r0.wwww)*(r3.xyzx))).xyz;
    // 255: mad r6.xyz, cb0[16].zzzz, r3.xyzx, -r0.xyzx
    r6.xyz = ((source[16].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 256: mul r3.xyz, r3.xyzx, cb0[16].zzzz
    r3.xyz = ((r3.xyzx)*(source[16].zzzz)).xyz;
    // 257: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 258: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 259: mad r0.xyz, r0.wwww, r6.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 260: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 261: mul r3.xyz, r2.xyzx, r1.xxxx
    r3.xyz = ((r2.xyzx)*(r1.xxxx)).xyz;
    // 262: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: mad r1.xyw, -r1.xxxx, r2.xyxz, r0.wwww
    r1.xyw = ((-(r1.xxxx))*(r2.xyxz)+(r0.wwww)).xyw;
    // 264: mad r1.xyw, cb0[19].yyyy, r1.xyxw, r3.xyxz
    r1.xyw = ((source[19].yyyy)*(r1.xyxw)+(r3.xyxz)).xyw;
    // 265: dp3 r0.w, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r2.xyz, -r1.xywx, r0.wwww
    r2.xyz = ((-(r1.xywx))+(r0.wwww)).xyz;
    // 267: mad r1.xyw, cb0[19].zzzz, r2.xyxz, r1.xyxw
    r1.xyw = ((source[19].zzzz)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 268: mad r2.xyz, r4.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r2.xyz = ((r4.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 269: mul r0.w, r4.w, cb0[12].w
    r0.w = ((r4.wwww)*(source[12].wwww)).w;
    // 270: mad r2.xyz, cb0[13].wwww, r2.xyzx, cb0[13].xyzx
    r2.xyz = ((source[13].wwww)*(r2.xyzx)+(source[13].xyzx)).xyz;
    // 271: mad r2.xyz, r0.wwww, cb0[12].xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(source[12].xyzx)+(r2.xyzx)).xyz;
    // 272: mad r1.xyw, r1.xyxw, r11.xyxz, r2.xyxz
    r1.xyw = ((r1.xyxw)*(r11.xyxz)+(r2.xyxz)).xyw;
    // 273: log r0.w, |r1.z|
    r0.w = (log2(abs(r1.zzzz))).w;
    // 274: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 275: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 276: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 277: mul r2.xyz, r0.wwww, cb0[14].xyzx
    r2.xyz = ((r0.wwww)*(source[14].xyzx)).xyz;
    // 278: movc r2.xyz, r1.zzzz, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 279: add r1.xyz, r1.xywx, r2.xyzx
    r1.xyz = ((r1.xywx)+(r2.xyzx)).xyz;
    // 280: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 281: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 282: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 283: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 284: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 285: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 286: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 287: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 288: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 289: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 290: mul r3.yzw, r3.yyyy, cb0[26].xxyz
    r3.yzw = ((r3.yyyy)*(source[26].xxyz)).yzw;
    // 291: mad r3.xyz, r3.xxxx, cb0[25].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[25].xyzx)+(r3.yzwy)).xyz;
    // 292: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 293: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 294: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 295: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 296: mad o0.xyz, r0.xyzx, cb0[27].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[27].xyzx)+(r1.xyzx)).xyz;
    // 297: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 298: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 299: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 300: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 301: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 302: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 303: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 304: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 305: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 306: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 307: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 308: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 309: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 310: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 311: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 312: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 313: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 314: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 315: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 316: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 317: ret
    return output;
}

// source.character.equipment-native-197.v1 / source program 6eddc4f7a5b5774aa036dd6672343bb1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase197(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[20]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[27].x=(g_SourceCharacterTime.xxxx).x;
    source[27].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[27].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
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
    // 22: add r1.y, -cb0[23].y, cb0[23].x
    r1.y = ((-(source[23].yyyy))+(source[23].xxxx)).y;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 24: mad r1.y, r2.x, r1.y, cb0[23].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[23].yyyy)).y;
    // 25: add r1.z, -r1.y, cb0[23].z
    r1.z = ((-(r1.yyyy))+(source[23].zzzz)).z;
    // 26: mad r1.y, r2.y, r1.z, r1.y
    r1.y = ((r2.yyyy)*(r1.zzzz)+(r1.yyyy)).y;
    // 27: add r1.z, -r1.y, cb0[23].w
    r1.z = ((-(r1.yyyy))+(source[23].wwww)).z;
    // 28: mad r1.y, r2.z, r1.z, r1.y
    r1.y = ((r2.zzzz)*(r1.zzzz)+(r1.yyyy)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 30: add r1.z, -r3.w, l(1.000000)
    r1.z = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 32: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 33: mul r1.y, r1.w, r1.y
    r1.y = ((r1.wwww)*(r1.yyyy)).y;
    // 34: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 35: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 37: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 38: add r1.w, -cb0[22].y, cb0[22].x
    r1.w = ((-(source[22].yyyy))+(source[22].xxxx)).w;
    // 39: mad r1.w, r2.x, r1.w, cb0[22].y
    r1.w = ((r2.xxxx)*(r1.wwww)+(source[22].yyyy)).w;
    // 40: add r3.w, -r1.w, cb0[22].z
    r3.w = ((-(r1.wwww))+(source[22].zzzz)).w;
    // 41: mad r1.w, r2.y, r3.w, r1.w
    r1.w = ((r2.yyyy)*(r3.wwww)+(r1.wwww)).w;
    // 42: add r3.w, -r1.w, cb0[22].w
    r3.w = ((-(r1.wwww))+(source[22].wwww)).w;
    // 43: mad r1.w, r2.z, r3.w, r1.w
    r1.w = ((r2.zzzz)*(r3.wwww)+(r1.wwww)).w;
    // 44: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 45: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 46: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 47: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 48: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 50: mad r1.xz, r1.xxzx, l(2.000000, 0.000000, 2.000000, 0.000000), l(-1.000000, 0.000000, -1.000000, 0.000000)
    r1.xz = ((r1.xxzx)*(float4(2.000000,0.000000,2.000000,0.000000))+(float4(-1.000000,0.000000,-1.000000,0.000000))).xz;
    // 51: dp2 r1.w, r1.xzxx, r1.xzxx
    r1.w = (dot((r1.xzxx).xy,(r1.xzxx).xy).xxxx).w;
    // 52: mul r4.xy, r1.xzxx, cb0[21].xxxx
    r4.xy = ((r1.xzxx)*(source[21].xxxx)).xy;
    // 53: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 55: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 56: add r4.z, r1.x, l(0.000010)
    r4.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 57: add r1.xzw, -r4.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r4.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 58: mad r1.xzw, cb0[21].wwww, r1.xxzw, r4.xxyz
    r1.xzw = ((source[21].wwww)*(r1.xxzw)+(r4.xxyz)).xzw;
    // 59: dp3 r3.w, r1.xzwx, r1.xzwx
    r3.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 60: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 61: div r1.xzw, r1.xxzw, r3.wwww
    r1.xzw = ((r1.xxzw)/(r3.wwww)).xzw;
    // 62: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 63: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 64: mul r5.xyz, r3.wwww, v0.xyzx
    r5.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 65: dp3 r6.x, r5.xyzx, r1.xzwx
    r6.x = (dot((r5.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 66: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 67: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 68: mul r7.xyz, r3.wwww, v1.xyzx
    r7.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 69: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 70: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 71: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 72: dp3 r6.y, r8.xyzx, r1.xzwx
    r6.y = (dot((r8.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // 73: dp3 r6.z, r7.xyzx, r1.xzwx
    r6.z = (dot((r7.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // 74: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 75: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 76: mul r9.xyz, r1.xxxx, v5.xyzx
    r9.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 77: mad r1.xzw, v5.xxyz, r1.xxxx, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((v5.xxyz)*(r1.xxxx)+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 78: dp3 r10.y, r8.xyzx, r9.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 79: dp3 r10.x, r5.xyzx, r9.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 80: dp3 r10.z, r7.xyzx, r9.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 81: dp3 r3.w, r6.xyzx, r10.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 82: mul r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)*(r3.wwww)).xyz;
    // 83: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 84: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 85: dp2 r3.w, r6.ywyy, r6.ywyy
    r3.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 86: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 87: div r6.xy, r6.ywyy, r3.wwww
    r6.xy = ((r6.ywyy)/(r3.wwww)).xy;
    // 88: mad r3.w, -r6.z, l(0.250000), l(0.250000)
    r3.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 89: add r4.w, r6.z, l(1.000000)
    r4.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 91: mad r6.xy, r3.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r3.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 92: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s4, r0.x
    r6.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 93: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 94: rcp r0.x, cb0[24].x
    r0.x = (1.0/(source[24].xxxx)).x;
    // 95: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 96: mul r10.xyz, r10.xyzx, cb0[24].xxxx
    r10.xyz = ((r10.xyzx)*(source[24].xxxx)).xyz;
    // 97: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 98: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 99: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 100: mad r10.xyz, r10.xyzx, cb0[24].xxxx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[24].xxxx)+(r11.xyzx)).xyz;
    // 101: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 102: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 103: add r0.x, cb0[24].x, l(1.000000)
    r0.x = ((source[24].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 104: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 105: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 106: add r6.xyz, -cb0[11].xyzx, cb0[12].xyzx
    r6.xyz = ((-(source[11].xyzx))+(source[12].xyzx)).xyz;
    // 107: mad r6.xyz, r4.wwww, r6.xyzx, cb0[11].xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)+(source[11].xyzx)).xyz;
    // 108: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 109: mul r6.xyz, r6.xyzx, cb0[24].yyyy
    r6.xyz = ((r6.xyzx)*(source[24].yyyy)).xyz;
    // 110: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 111: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 112: mad r3.xyz, cb0[21].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[21].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 113: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 114: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 115: mad r3.xyz, cb0[21].zzzz, r10.xyzx, r3.xyzx
    r3.xyz = ((source[21].zzzz)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 116: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 117: add r10.xyz, -r3.xyzx, r0.xxxx
    r10.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 118: mul r10.xyz, r10.xyzx, cb0[24].zzzz
    r10.xyz = ((r10.xyzx)*(source[24].zzzz)).xyz;
    // 119: add r0.x, r2.y, r2.x
    r0.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 120: add r0.x, r2.z, r0.x
    r0.x = ((r2.zzzz)+(r0.xxxx)).x;
    // 121: add_sat r0.x, r2.w, r0.x
    r0.x = (saturate((r2.wwww)+(r0.xxxx))).x;
    // 122: mad r3.xyz, r0.xxxx, r10.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 123: max r10.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 124: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 125: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 126: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 127: dp3 r0.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 128: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 129: mul r0.x, r0.x, cb0[25].z
    r0.x = ((r0.xxxx)*(source[25].zzzz)).x;
    // 130: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 131: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 132: mad r2.w, -r0.x, r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 134: div r2.w, cb0[25].w, r2.w
    r2.w = ((source[25].wwww)/(r2.wwww)).w;
    // 135: dp3 r3.w, r4.xyzx, r4.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 136: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 137: div r4.xyz, r4.xyzx, r3.wwww
    r4.xyz = ((r4.xyzx)/(r3.wwww)).xyz;
    // 138: dp3 r3.w, r4.xyzx, r9.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 139: mul_sat r4.w, r3.w, cb0[24].w
    r4.w = (saturate((r3.wwww)*(source[24].wwww))).w;
    // 140: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: mul_sat r5.w, r9.z, cb0[24].w
    r5.w = (saturate((r9.zzzz)*(source[24].wwww))).w;
    // 143: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: add_sat r5.w, r5.w, -cb0[25].x
    r5.w = (saturate((r5.wwww)+(-(source[25].xxxx)))).w;
    // 145: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 146: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 147: mul r6.w, r6.w, cb0[25].y
    r6.w = ((r6.wwww)*(source[25].yyyy)).w;
    // 148: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 149: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 150: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 151: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 152: mul r10.xyz, r6.xyzx, r2.wwww
    r10.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 153: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 154: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 155: mad r0.yzw, cb0[21].yyyy, r11.xxyz, r0.yyzw
    r0.yzw = ((source[21].yyyy)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 156: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 157: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 158: mad r0.yzw, cb0[21].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[21].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 159: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 160: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 161: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 162: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, -r11.xyzx
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r11.xyzx))).xyz;
    // 163: mad r2.xyw, r2.yyyy, r12.xyxz, r11.xyxz
    r2.xyw = ((r2.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 164: mad r11.xyz, cb0[8].wwww, cb0[8].xyzx, -r2.xywx
    r11.xyz = ((source[8].wwww)*(source[8].xyzx)+(-(r2.xywx))).xyz;
    // 165: mad r2.xyz, r2.zzzz, r11.xyzx, r2.xywx
    r2.xyz = ((r2.zzzz)*(r11.xyzx)+(r2.xywx)).xyz;
    // 166: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 167: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 168: mad r2.xyz, cb0[21].yyyy, r11.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 169: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 170: add r11.xyz, -r2.xyzx, r2.wwww
    r11.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 171: mad r2.xyz, cb0[21].zzzz, r11.xyzx, r2.xyzx
    r2.xyz = ((source[21].zzzz)*(r11.xyzx)+(r2.xyzx)).xyz;
    // 172: mad r11.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 173: mad r12.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 174: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 175: mul r2.xyz, r2.xyzx, r11.xyzx
    r2.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 176: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 177: mul r2.xyz, r6.xyzx, r0.yzwy
    r2.xyz = ((r6.xyzx)*(r0.yzwy)).xyz;
    // 178: mad r3.xyz, r3.xyzx, r10.xyzx, -r2.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)+(-(r2.xyzx))).xyz;
    // 179: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: mul r2.w, r2.w, cb0[26].x
    r2.w = ((r2.wwww)*(source[26].xxxx)).w;
    // 181: mad r2.xyz, r2.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 182: dp3 r2.w, r1.xzwx, r1.xzwx
    r2.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 183: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 184: div r1.xzw, r1.xxzw, r3.xxxx
    r1.xzw = ((r1.xxzw)/(r3.xxxx)).xzw;
    // 185: dp3 r1.x, r1.xzwx, r9.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 186: add r1.z, -|r9.z|, l(1.000000)
    r1.z = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 187: mul r1.z, r3.w, r1.z
    r1.z = ((r3.wwww)*(r1.zzzz)).z;
    // 188: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 189: mul r1.w, |r1.x|, |r1.x|
    r1.w = ((abs(r1.xxxx))*(abs(r1.xxxx))).w;
    // 190: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 191: mul r1.w, r1.w, |r1.x|
    r1.w = ((r1.wwww)*(abs(r1.xxxx))).w;
    // 192: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 193: movc r1.x, r1.x, l(0), r1.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).x;
    // 194: add r1.w, r1.x, l(-0.027778)
    r1.w = ((r1.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 195: mad r1.x, r1.x, r1.w, l(0.027778)
    r1.x = ((r1.xxxx)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 196: div_sat r1.x, r1.x, r2.w
    r1.x = (saturate((r1.xxxx)/(r2.wwww))).x;
    // 197: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 198: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 199: mad r1.xyw, r1.xxxx, r2.xyxz, -r0.yzyw
    r1.xyw = ((r1.xxxx)*(r2.xyxz)+(-(r0.yzyw))).xyw;
    // 200: mad r0.xyz, r0.xxxx, r1.xywx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xywx)+(r0.yzwy)).xyz;
    // 201: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 202: add r1.xyw, -r0.xyxz, r0.wwww
    r1.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 203: mad r0.xyz, cb0[21].yyyy, r1.xywx, r0.xyzx
    r0.xyz = ((source[21].yyyy)*(r1.xywx)+(r0.xyzx)).xyz;
    // 204: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 205: add r1.xyw, -r0.xyxz, r0.wwww
    r1.xyw = ((-(r0.xyxz))+(r0.wwww)).xyw;
    // 206: mad r0.xyz, cb0[21].zzzz, r1.xywx, r0.xyzx
    r0.xyz = ((source[21].zzzz)*(r1.xywx)+(r0.xyzx)).xyz;
    // 207: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 208: add r0.w, -cb0[4].w, l(1.000000)
    r0.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 209: mul r0.w, r0.w, cb0[27].x
    r0.w = ((r0.wwww)*(source[27].xxxx)).w;
    // 210: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 211: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 212: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 213: mul r1.x, cb0[4].z, l(1.500000)
    r1.x = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 214: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 215: mad r0.w, r0.w, l(0.500000), cb0[4].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 216: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 217: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 218: mul r3.y, cb0[4].y, cb0[17].y
    r3.y = ((source[4].yyyy)*(source[17].yyyy)).y;
    // 219: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 220: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 221: add r1.xy, r1.xyxx, r3.xyxx
    r1.xy = ((r1.xyxx)+(r3.xyxx)).xy;
    // 222: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 223: add r2.w, -r1.w, cb0[4].x
    r2.w = ((-(r1.wwww))+(source[4].xxxx)).w;
    // 224: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 225: add r1.xy, r1.xyxx, r3.zwzz
    r1.xy = ((r1.xyxx)+(r3.zwzz)).xy;
    // 226: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 227: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 228: mul r0.w, r1.w, r3.w
    r0.w = ((r1.wwww)*(r3.wwww)).w;
    // 229: add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 230: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 231: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 232: add r3.xyzw, v7.yzxy, cb0[0].yzxy
    r3.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 233: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 234: add r1.yw, -r3.xxxy, l(0.000000, 1.000000, 0.000000, 1.000000)
    r1.yw = ((-(r3.xxxy))+(float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 235: add r1.yw, -r3.zzzw, r1.yyyw
    r1.yw = ((-(r3.zzzw))+(r1.yyyw)).yw;
    // 236: mad r1.yw, cb0[18].wwww, r1.yyyw, r3.zzzw
    r1.yw = ((source[18].wwww)*(r1.yyyw)+(r3.zzzw)).yw;
    // 237: mul r0.w, cb0[18].y, cb0[27].x
    r0.w = ((source[18].yyyy)*(source[27].xxxx)).w;
    // 238: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 239: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 240: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 241: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 242: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 243: mul r2.w, cb0[18].x, l(0.001000)
    r2.w = ((source[18].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 244: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 245: mad r1.yw, r2.wwww, r1.yyyw, r3.xxxy
    r1.yw = ((r2.wwww)*(r1.yyyw)+(r3.xxxy)).yw;
    // 246: dp2 r2.w, cb0[19].xyxx, r1.ywyy
    r2.w = (dot((source[19].xyxx).xy,(r1.ywyy).xy).xxxx).w;
    // 247: dp2 r3.y, cb0[20].xyxx, r1.ywyy
    r3.y = (dot((source[20].xyxx).xy,(r1.ywyy).xy).xxxx).y;
    // 248: frc r1.y, r2.w
    r1.y = (frac(r2.wwww)).y;
    // 249: mul r3.x, r1.y, l(0.125000)
    r3.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 250: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 251: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 252: mul r1.y, r3.w, l(0.900000)
    r1.y = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 253: mad r3.xyz, r1.yyyy, r3.xyzx, r0.xyzx
    r3.xyz = ((r1.yyyy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 254: mul_sat r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = (saturate((r0.wwww)*(r3.xyzx))).xyz;
    // 255: mad r6.xyz, cb0[18].zzzz, r3.xyzx, -r0.xyzx
    r6.xyz = ((source[18].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 256: mul r3.xyz, r3.xyzx, cb0[18].zzzz
    r3.xyz = ((r3.xyzx)*(source[18].zzzz)).xyz;
    // 257: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 258: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 259: mad r0.xyz, r0.wwww, r6.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 260: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 261: mad r3.xyz, r4.wwww, cb0[15].xyzx, -cb0[15].xyzx
    r3.xyz = ((r4.wwww)*(source[15].xyzx)+(-(source[15].xyzx))).xyz;
    // 262: mul r0.w, r4.w, cb0[14].w
    r0.w = ((r4.wwww)*(source[14].wwww)).w;
    // 263: mad r3.xyz, cb0[15].wwww, r3.xyzx, cb0[15].xyzx
    r3.xyz = ((source[15].wwww)*(r3.xyzx)+(source[15].xyzx)).xyz;
    // 264: mad r3.xyz, r0.wwww, cb0[14].xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(source[14].xyzx)+(r3.xyzx)).xyz;
    // 265: add r0.w, cb0[2].y, cb0[2].x
    r0.w = ((source[2].yyyy)+(source[2].xxxx)).w;
    // 266: add r0.w, r0.w, cb0[2].z
    r0.w = ((r0.wwww)+(source[2].zzzz)).w;
    // 267: add r1.y, -r0.w, l(1000.000000)
    r1.y = ((-(r0.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).y;
    // 268: mad r0.w, cb0[27].y, r1.y, r0.w
    r0.w = ((source[27].yyyy)*(r1.yyyy)+(r0.wwww)).w;
    // 269: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 270: mad r0.w, cb0[26].w, cb0[27].x, r0.w
    r0.w = ((source[26].wwww)*(source[27].xxxx)+(r0.wwww)).w;
    // 271: mul r1.y, r0.w, l(3.524534)
    r1.y = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).y;
    // 272: sincos null, r1.y, r1.y
    r1.y = (cos(r1.yyyy)).y;
    // 273: add r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)+(r1.yyyy)).w;
    // 274: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 275: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 276: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 277: mad r0.w, r0.w, l(0.500000), cb0[26].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[26].zzzz)).w;
    // 278: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t6.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 279: mul r9.xyz, cb0[13].xyzx, cb0[26].yyyy
    r9.xyz = ((source[13].xyzx)*(source[26].yyyy)).xyz;
    // 280: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 281: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 282: mad r1.xyw, r1.xxxx, r2.xyxz, r6.xyxz
    r1.xyw = ((r1.xxxx)*(r2.xyxz)+(r6.xyxz)).xyw;
    // 283: dp3 r0.w, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 284: add r2.xyz, -r1.xywx, r0.wwww
    r2.xyz = ((-(r1.xywx))+(r0.wwww)).xyz;
    // 285: mad r1.xyw, cb0[21].yyyy, r2.xyxz, r1.xyxw
    r1.xyw = ((source[21].yyyy)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 286: dp3 r0.w, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 287: add r2.xyz, -r1.xywx, r0.wwww
    r2.xyz = ((-(r1.xywx))+(r0.wwww)).xyz;
    // 288: mad r1.xyw, cb0[21].zzzz, r2.xyxz, r1.xyxw
    r1.xyw = ((source[21].zzzz)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 289: mad r1.xyw, r1.xyxw, r11.xyxz, r3.xyxz
    r1.xyw = ((r1.xyxw)*(r11.xyxz)+(r3.xyxz)).xyw;
    // 290: log r0.w, |r1.z|
    r0.w = (log2(abs(r1.zzzz))).w;
    // 291: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 292: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 293: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 294: mul r2.xyz, r0.wwww, cb0[16].xyzx
    r2.xyz = ((r0.wwww)*(source[16].xyzx)).xyz;
    // 295: movc r2.xyz, r1.zzzz, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 296: add r1.xyz, r1.xywx, r2.xyzx
    r1.xyz = ((r1.xywx)+(r2.xyzx)).xyz;
    // 297: add r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(source[3].xyzx)).xyz;
    // 298: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 299: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 300: mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 301: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 302: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 303: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 304: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 305: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 306: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 307: mul r3.yzw, r3.yyyy, cb0[29].xxyz
    r3.yzw = ((r3.yyyy)*(source[29].xxyz)).yzw;
    // 308: mad r3.xyz, r3.xxxx, cb0[28].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[28].xyzx)+(r3.yzwy)).xyz;
    // 309: mul r3.xyz, r3.xyzx, cb0[30].wwww
    r3.xyz = ((r3.xyzx)*(source[30].wwww)).xyz;
    // 310: mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 311: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 312: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 313: mad o0.xyz, r0.xyzx, cb0[30].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[30].xyzx)+(r1.xyzx)).xyz;
    // 314: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 315: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 316: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 317: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 318: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 319: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 320: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 321: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 322: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 323: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 324: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 325: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 326: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 327: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 328: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 329: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 330: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 331: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 332: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 333: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 334: ret
    return output;
}

// source.character.equipment-native-198.v1 / source program 1dd6ab93ae2aed4b9cc82de6d5999e12
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase198(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[27]=g_SourceCharacterEnvironmentColor; source[28]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[17].xyzw
    r1.xyzw = ((r0.xyzw)*(source[17].xyzw)).xyzw;
    // 3: add r0.xy, r0.ywyy, r0.xzxx
    r0.xy = ((r0.ywyy)+(r0.xzxx)).xy;
    // 4: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 5: add r0.yz, r1.yywy, r1.xxzx
    r0.yz = ((r1.yywy)+(r1.xxzx)).yz;
    // 6: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 7: add r0.y, r0.y, l(-1.000000)
    r0.y = ((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 8: mad_sat r0.x, r0.x, r0.y, l(1.000000)
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 10: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 11: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 12: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 13: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 14: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 15: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 16: mul r0.x, r0.x, l(0.125000)
    r0.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 17: mul r2.y, cb0[8].y, cb0[16].y
    r2.y = ((source[8].yyyy)*(source[16].yyyy)).y;
    // 18: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 19: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 20: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 21: frc r0.z, cb0[8].x
    r0.z = (frac(source[8].xxxx)).z;
    // 22: add r0.w, -r0.z, cb0[8].x
    r0.w = ((-(r0.zzzz))+(source[8].xxxx)).w;
    // 23: mul r2.z, r0.w, l(0.125000)
    r2.z = ((r0.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 24: add r0.xy, r0.xyxx, r2.zwzz
    r0.xy = ((r0.xyxx)+(r2.zwzz)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: mul r0.x, r0.z, r2.w
    r0.x = ((r0.zzzz)*(r2.wwww)).x;
    // 27: add r0.y, -cb0[8].w, l(1.000000)
    r0.y = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: mul r0.y, r0.y, cb0[20].z
    r0.y = ((r0.yyyy)*(source[20].zzzz)).y;
    // 29: mul r0.y, r0.y, l(6.283185)
    r0.y = ((r0.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
    // 30: sincos r0.y, null, r0.y
    r0.y = (sin(r0.yyyy)).y;
    // 31: add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: mul r0.z, cb0[8].z, l(1.500000)
    r0.z = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 33: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 34: mad r0.y, r0.y, l(0.500000), cb0[8].z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).y;
    // 35: mul r0.yzw, r2.xxyz, r0.yyyy
    r0.yzw = ((r2.xxyz)*(r0.yyyy)).yzw;
    // 36: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 38: mad r2.xyz, cb0[21].xxxx, r2.xyzx, r1.xyzx
    r2.xyz = ((source[21].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 39: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 41: mad r2.xyz, cb0[21].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 42: mul r3.xyz, cb0[6].xyzx, cb0[6].wwww
    r3.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 43: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 44: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 45: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 46: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 47: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 49: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 50: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 51: mul r1.w, r6.y, cb0[19].y
    r1.w = ((r6.yyyy)*(source[19].yyyy)).w;
    // 52: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 53: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 55: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 56: mul r4.xyz, cb0[4].xyzx, cb0[4].wwww
    r4.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 57: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 58: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 59: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 60: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 61: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 62: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 63: mul r7.xyz, cb0[5].xyzx, cb0[5].wwww
    r7.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 64: max r8.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 65: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 66: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 67: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 68: add r8.xyz, -r7.xyzx, r8.xyzx
    r8.xyz = ((-(r7.xyzx))+(r8.xyzx)).xyz;
    // 69: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 70: add r8.xyz, -r4.xyzx, r7.xyzx
    r8.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 72: mad r4.xyz, r9.xxxx, r8.xyzx, r4.xyzx
    r4.xyz = ((r9.xxxx)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 73: add r8.xyz, r3.xyzx, -r4.xyzx
    r8.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 74: mad r3.xyz, -r9.xxxx, r7.xyzx, r3.xyzx
    r3.xyz = ((-(r9.xxxx))*(r7.xyzx)+(r3.xyzx)).xyz;
    // 75: mul r7.xyz, r7.xyzx, r9.xxxx
    r7.xyz = ((r7.xyzx)*(r9.xxxx)).xyz;
    // 76: mad r3.xyz, r9.yyyy, r3.xyzx, r7.xyzx
    r3.xyz = ((r9.yyyy)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 77: mad r4.xyz, r9.yyyy, r8.xyzx, r4.xyzx
    r4.xyz = ((r9.yyyy)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 78: mul r7.xyz, cb0[7].xyzx, cb0[7].wwww
    r7.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 79: max r8.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 80: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 81: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 82: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 83: add r8.xyz, -r7.xyzx, r8.xyzx
    r8.xyz = ((-(r7.xyzx))+(r8.xyzx)).xyz;
    // 84: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 85: add r8.xyz, -r4.xyzx, r7.xyzx
    r8.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 86: mad r4.xyz, r9.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r9.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 87: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 88: mad r3.xyz, r9.zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((r9.zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 89: add r3.xyz, r3.xyzx, -cb0[9].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[9].xyzx))).xyz;
    // 90: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 91: add r7.xyz, -r4.xyzx, r2.wwww
    r7.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 92: mad r7.xyz, cb0[21].xxxx, r7.xyzx, r4.xyzx
    r7.xyz = ((source[21].xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 93: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 94: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: add r4.xyz, -r7.xyzx, r2.wwww
    r4.xyz = ((-(r7.xyzx))+(r2.wwww)).xyz;
    // 96: mad r4.xyz, cb0[21].yyyy, r4.xyzx, r7.xyzx
    r4.xyz = ((source[21].yyyy)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 97: mad r7.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 98: mad r8.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 99: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 100: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 101: mul r8.xyz, r2.xyzx, r4.xyzx
    r8.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 102: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: mad r2.xyz, -r4.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r4.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 104: mad r2.xyz, cb0[21].xxxx, r2.xyzx, r8.xyzx
    r2.xyz = ((source[21].xxxx)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 105: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r4.xyz, -r2.xyzx, r2.wwww
    r4.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 107: mad r2.xyz, cb0[21].yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((source[21].yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 108: mul r2.xyz, r7.xyzx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 109: mad r0.yzw, r0.yyzw, l(0.000000, 2.000000, 2.000000, 2.000000), -r2.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r2.xxyz))).yzw;
    // 110: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 111: add r0.w, r0.y, r0.x
    r0.w = ((r0.yyyy)+(r0.xxxx)).w;
    // 112: add r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)+(r0.wwww)).w;
    // 113: mul r0.w, r0.w, l(0.333330)
    r0.w = ((r0.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 114: max r0.w, r0.w, cb0[23].x
    r0.w = (max(r0.wwww,source[23].xxxx)).w;
    // 115: min r0.w, r0.w, cb0[22].w
    r0.w = (min(r0.wwww,source[22].wwww)).w;
    // 116: add r2.x, -r0.w, l(1.000000)
    r2.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 117: mad r0.w, r1.w, r2.x, r0.w
    r0.w = ((r1.wwww)*(r2.xxxx)+(r0.wwww)).w;
    // 118: mul_sat r2.w, r1.w, cb2[3].w
    r2.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 119: add r1.w, r0.w, l(-1.000000)
    r1.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 120: mad r1.w, cb0[23].z, r1.w, l(1.000000)
    r1.w = ((source[23].zzzz)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: mul r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 122: mul r2.x, r6.x, cb0[22].y
    r2.x = ((r6.xxxx)*(source[22].yyyy)).x;
    // 123: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 124: movc r2.x, r5.x, l(0), r2.x
    r2.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 125: add_sat r2.x, r2.x, cb0[22].z
    r2.x = (saturate((r2.xxxx)+(source[22].zzzz))).x;
    // 126: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 127: mul r5.xyw, r2.yyyy, cb0[15].xyxz
    r5.xyw = ((r2.yyyy)*(source[15].xyxz)).xyw;
    // 128: mul r4.xyz, r4.xyzx, r5.xywx
    r4.xyz = ((r4.xyzx)*(r5.xywx)).xyz;
    // 129: mad r0.xyz, r1.wwww, r0.xyzx, -r4.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 130: mad r0.xyz, r2.xxxx, r0.xyzx, r4.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 131: mul_sat r0.w, r0.w, r2.x
    r0.w = (saturate((r0.wwww)*(r2.xxxx))).w;
    // 132: add r4.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 133: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 134: mad_sat r4.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 135: mad r0.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r0.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 136: mad r5.xyw, r4.xyxz, l(2.040400, 2.040400, 0.000000, 2.040400), l(-0.332400, -0.332400, 0.000000, -0.332400)
    r5.xyw = ((r4.xyxz)*(float4(2.040400,2.040400,0.000000,2.040400))+(float4(-0.332400,-0.332400,0.000000,-0.332400))).xyw;
    // 137: mad r6.xyw, r4.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r6.xyw = ((r4.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 138: mad r5.xyw, r0.wwww, r5.xyxw, r6.xyxw
    r5.xyw = ((r0.wwww)*(r5.xyxw)+(r6.xyxw)).xyw;
    // 139: mad r0.xyz, r5.xywx, r0.wwww, r0.xyzx
    r0.xyz = ((r5.xywx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 140: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 141: max r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (max(r0.xyzx,r0.wwww)).xyz;
    // 142: mov_sat r4.w, cb0[23].w
    r4.w = (saturate(source[23].wwww)).w;
    // 143: mad r5.xyw, -r4.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r4.xyxz
    r5.xyw = ((-(r4.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r4.xyxz)).xyw;
    // 144: mul r1.w, r4.w, l(0.080000)
    r1.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 145: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 146: mad r5.xyw, r2.wwww, r5.xyxw, r1.wwww
    r5.xyw = ((r2.wwww)*(r5.xyxw)+(r1.wwww)).xyw;
    // 147: mul_sat r1.w, r5.y, l(50.000000)
    r1.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 148: add r2.x, -cb0[24].w, cb0[24].z
    r2.x = ((-(source[24].wwww))+(source[24].zzzz)).x;
    // 149: mad r2.x, r9.x, r2.x, cb0[24].w
    r2.x = ((r9.xxxx)*(r2.xxxx)+(source[24].wwww)).x;
    // 150: add r2.y, -r2.x, cb0[25].y
    r2.y = ((-(r2.xxxx))+(source[25].yyyy)).y;
    // 151: mad r2.x, r9.y, r2.y, r2.x
    r2.x = ((r9.yyyy)*(r2.yyyy)+(r2.xxxx)).x;
    // 152: add r2.y, -r2.x, cb0[25].w
    r2.y = ((-(r2.xxxx))+(source[25].wwww)).y;
    // 153: mad r2.x, r9.z, r2.y, r2.x
    r2.x = ((r9.zzzz)*(r2.yyyy)+(r2.xxxx)).x;
    // 154: mul r2.x, r6.z, r2.x
    r2.x = ((r6.zzzz)*(r2.xxxx)).x;
    // 155: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 156: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 157: movc r2.x, r5.z, l(0), r2.x
    r2.x = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 158: max r2.x, r2.x, cb0[1].x
    r2.x = (max(r2.xxxx,source[1].xxxx)).x;
    // 159: min r2.z, r2.x, l(1.000000)
    r2.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 160: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 161: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 162: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 163: mul r6.xy, r2.xyxx, cb0[19].xxxx
    r6.xy = ((r2.xyxx)*(source[19].xxxx)).xy;
    // 164: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 165: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 166: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 167: add r6.z, r2.x, l(0.000010)
    r6.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 168: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 169: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 170: div r6.xyz, r6.xyzx, r2.xxxx
    r6.xyz = ((r6.xyzx)/(r2.xxxx)).xyz;
    // 171: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 172: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 173: mul r8.xyz, r2.xxxx, r6.xyzx
    r8.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 174: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 175: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 176: mul r10.xyz, r2.xxxx, v5.xyzx
    r10.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 177: dp3 r2.x, r8.xyzx, r10.xyzx
    r2.x = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 178: deriv_rtx_coarse r11.x, r2.x
    r11.x = (ddx_coarse(r2.xxxx)).x;
    // 179: deriv_rty_coarse r11.y, r2.x
    r11.y = (ddy_coarse(r2.xxxx)).y;
    // 180: dp2 r2.y, r11.xyxx, r11.xyxx
    r2.y = (dot((r11.xyxx).xy,(r11.xyxx).xy).xxxx).y;
    // 181: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 182: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 183: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 184: min r11.y, r2.y, l(1.000000)
    r11.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 185: add r2.y, -r11.y, l(1.000000)
    r2.y = ((-(r11.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 186: max r12.xyz, r5.xywx, r2.yyyy
    r12.xyz = (max(r5.xywx,r2.yyyy)).xyz;
    // 187: add r12.xyz, -r5.xywx, r12.xyzx
    r12.xyz = ((-(r5.xywx))+(r12.xyzx)).xyz;
    // 188: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 189: mul r13.xyz, r2.xxxx, r8.xyzx
    r13.xyz = ((r2.xxxx)*(r8.xyzx)).xyz;
    // 190: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 191: add r1.w, r13.z, l(1.000000)
    r1.w = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 192: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 194: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 195: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 196: mul r2.x, r2.x, cb0[2].y
    r2.x = ((r2.xxxx)*(source[2].yyyy)).x;
    // 197: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 198: mad_sat r2.x, r2.x, cb0[2].w, cb0[2].z
    r2.x = (saturate((r2.xxxx)*(source[2].wwww)+(source[2].zzzz))).x;
    // 199: mul r2.x, r2.x, cb0[26].x
    r2.x = ((r2.xxxx)*(source[26].xxxx)).x;
    // 200: add_sat r11.x, -r1.w, r2.y
    r11.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 201: sample_indexable(texture2d)(float,float,float,float) r2.yz, r11.xyxx, t7.zxyw, s8
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 202: add r1.w, r0.w, r11.x
    r1.w = ((r0.wwww)+(r11.xxxx)).w;
    // 203: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 204: mul r11.xzw, r2.zzzz, r5.xxyw
    r11.xzw = ((r2.zzzz)*(r5.xxyw)).xzw;
    // 205: mad r11.xzw, r12.xxyz, r2.yyyy, r11.xxzw
    r11.xzw = ((r12.xxyz)*(r2.yyyy)+(r11.xxzw)).xzw;
    // 206: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = r2.z != 0.f ? 1.f / r2.z : 0.f;
    // 207: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 208: mad r12.xyz, r5.xywx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xywx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 209: dp3 r2.y, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 210: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 211: mad r14.xyz, -r11.xzwx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r11.xzwx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 212: mul r11.xzw, r11.xxzw, r12.xxyz
    r11.xzw = ((r11.xxzw)*(r12.xxyz)).xzw;
    // 213: mul r12.xyz, r4.xyzx, r14.xyzx
    r12.xyz = ((r4.xyzx)*(r14.xyzx)).xyz;
    // 214: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 215: mul r12.xyz, r2.yyyy, r12.xyzx
    r12.xyz = ((r2.yyyy)*(r12.xyzx)).xyz;
    // 216: dp3 r2.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 217: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 218: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 219: mul r15.xyz, r3.wwww, v1.xyzx
    r15.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 220: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 221: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 222: mul r16.xyz, r3.wwww, v0.xyzx
    r16.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 223: mul r17.xyz, r15.zxyz, r16.yzxy
    r17.xyz = ((r15.zxyz)*(r16.yzxy)).xyz;
    // 224: mad r17.xyz, r15.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r15.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 225: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 226: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 227: dp3 r17.y, r17.xyzx, r13.xyzx
    r17.y = (dot((r17.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 228: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 229: dp3 r17.x, r16.xyzx, r13.xyzx
    r17.x = (dot((r16.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 230: dp2 r16.z, r18.xyxx, cb0[28].xyxx
    r16.z = (dot((r18.xyxx).xy,(source[28].xyxx).xy).xxxx).z;
    // 231: mul r17.zw, cb0[28].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r17.zw = ((source[28].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 232: dp2 r16.x, r18.xyxx, r17.zwzz
    r16.x = (dot((r18.xyxx).xy,(r17.zwzz).xy).xxxx).x;
    // 233: dp2 r19.x, r17.xyxx, r17.zwzz
    r19.x = (dot((r17.xyxx).xy,(r17.zwzz).xy).xxxx).x;
    // 234: dp2 r19.z, r17.xyxx, cb0[28].xyxx
    r19.z = (dot((r17.xyxx).xy,(source[28].xyxx).xy).xxxx).z;
    // 235: dp3 r16.y, r15.xyzx, r8.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 236: dp3 r19.y, r15.xyzx, r13.xyzx
    r19.y = (dot((r15.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 237: mov r16.w, l(1.000000)
    r16.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 238: dp4 r15.x, cb0[29].xyzw, r16.xyzw
    r15.x = (dot((source[29].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 239: dp4 r15.y, cb0[30].xyzw, r16.xyzw
    r15.y = (dot((source[30].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 240: dp4 r15.z, cb0[31].xyzw, r16.xyzw
    r15.z = (dot((source[31].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 241: mul r17.xyzw, r16.yzzx, r16.xyzz
    r17.xyzw = ((r16.yzzx)*(r16.xyzz)).xyzw;
    // 242: dp4 r20.x, cb0[32].xyzw, r17.xyzw
    r20.x = (dot((source[32].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).x;
    // 243: dp4 r20.y, cb0[33].xyzw, r17.xyzw
    r20.y = (dot((source[33].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).y;
    // 244: dp4 r20.z, cb0[34].xyzw, r17.xyzw
    r20.z = (dot((source[34].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).z;
    // 245: add r15.xyz, r15.xyzx, r20.xyzx
    r15.xyz = ((r15.xyzx)+(r20.xyzx)).xyz;
    // 246: mul r3.w, r16.y, r16.y
    r3.w = ((r16.yyyy)*(r16.yyyy)).w;
    // 247: mov r18.z, r16.y
    r18.z = (r16.yyyy).z;
    // 248: mad r3.w, r16.x, r16.x, -r3.w
    r3.w = ((r16.xxxx)*(r16.xxxx)+(-(r3.wwww))).w;
    // 249: mad r15.xyz, cb0[35].xyzx, r3.wwww, r15.xyzx
    r15.xyz = ((source[35].xyzx)*(r3.wwww)+(r15.xyzx)).xyz;
    // 250: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 251: mul r15.xyz, r15.xyzx, cb0[27].xyzx
    r15.xyz = ((r15.xyzx)*(source[27].xyzx)).xyz;
    // 252: mul r15.xyz, r15.xyzx, cb0[28].zzzz
    r15.xyz = ((r15.xyzx)*(source[28].zzzz)).xyz;
    // 253: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[27].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[27].wwww)).xyz;
    // 254: dp3 r3.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 255: add r15.xyz, -r3.wwww, r15.xyzx
    r15.xyz = ((-(r3.wwww))+(r15.xyzx)).xyz;
    // 256: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 257: dp3 r3.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 258: mad r4.w, r11.y, l(2.000000), l(2.000000)
    r4.w = ((r11.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 259: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 260: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 261: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 262: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 263: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 264: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 265: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 266: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 267: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 268: mul r15.xyz, r3.wwww, r15.xyzx
    r15.xyz = ((r3.wwww)*(r15.xyzx)).xyz;
    // 269: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 270: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 271: mul r12.xyz, r0.xyzx, r12.xyzx
    r12.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 272: mul r3.w, r11.y, l(5.000000)
    r3.w = ((r11.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 273: mul r5.w, r11.y, r11.y
    r5.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 274: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 275: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 276: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 277: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 278: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 279: sample_l_indexable(texturecube)(float,float,float,float) r15.xyzw, r19.xyzx, t8.xyzw, s7, r3.w
    r15.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r19.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 280: mul r15.xyz, r15.xyzx, r15.wwww
    r15.xyz = ((r15.xyzx)*(r15.wwww)).xyz;
    // 281: mul r15.xyz, r15.xyzx, cb0[27].xyzx
    r15.xyz = ((r15.xyzx)*(source[27].xyzx)).xyz;
    // 282: mul r15.xyz, r15.xyzx, cb0[28].zzzz
    r15.xyz = ((r15.xyzx)*(source[28].zzzz)).xyz;
    // 283: mad r15.xyz, r15.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[27].wwww
    r15.xyz = ((r15.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[27].wwww)).xyz;
    // 284: dp3 r1.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 285: add r15.xyz, -r1.wwww, r15.xyzx
    r15.xyz = ((-(r1.wwww))+(r15.xyzx)).xyz;
    // 286: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 287: dp3 r1.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 288: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 289: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 290: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 291: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 292: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 293: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 294: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 295: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 296: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 297: mul r15.xyz, r1.wwww, r15.xyzx
    r15.xyz = ((r1.wwww)*(r15.xyzx)).xyz;
    // 298: mul r16.xyz, r11.xzwx, r15.xyzx
    r16.xyz = ((r11.xzwx)*(r15.xyzx)).xyz;
    // 299: mad r1.w, r0.w, r5.x, r5.y
    r1.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 300: mad r1.w, r1.w, r0.w, r5.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 301: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 302: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 303: mad r5.xyz, r16.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r16.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 304: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 305: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 306: mul r12.xyz, r1.wwww, v6.xyzx
    r12.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 307: dp3 r1.w, r12.xyzx, r8.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 308: dp3 r2.z, -r12.xyzx, r8.xyzx
    r2.z = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).z;
    // 309: dp3 r3.w, r12.xyzx, r13.xyzx
    r3.w = (dot((r12.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 310: mad r8.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 311: mad r8.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 312: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 313: mad r12.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 314: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 315: mul r12.yzw, r12.yyyy, cb0[38].xxyz
    r12.yzw = ((r12.yyyy)*(source[38].xxyz)).yzw;
    // 316: mad r12.xyz, r12.xxxx, cb0[37].xyzx, r12.yzwy
    r12.xyz = ((r12.xxxx)*(source[37].xyzx)+(r12.yzwy)).xyz;
    // 317: mul r12.xyz, r12.xyzx, cb0[39].wwww
    r12.xyz = ((r12.xyzx)*(source[39].wwww)).xyz;
    // 318: mul r12.xyz, r4.xyzx, r12.xyzx
    r12.xyz = ((r4.xyzx)*(r12.xyzx)).xyz;
    // 319: mul r0.xyz, r0.xyzx, r12.xyzx
    r0.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 320: mul r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 321: mul r0.xyz, r14.xyzx, r0.xyzx
    r0.xyz = ((r14.xyzx)*(r0.xyzx)).xyz;
    // 322: mad r0.xyz, -r0.xyzx, r2.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r2.wwww)+(r0.xyzx)).xyz;
    // 323: mad r0.xyz, r5.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r0.xyzx
    r0.xyz = ((r5.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r0.xyzx)).xyz;
    // 324: mul r5.xyz, r8.yyyy, cb0[38].xyzx
    r5.xyz = ((r8.yyyy)*(source[38].xyzx)).xyz;
    // 325: mad r5.xyz, cb0[37].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[37].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 326: mul r5.xyz, r5.xyzx, cb0[39].wwww
    r5.xyz = ((r5.xyzx)*(source[39].wwww)).xyz;
    // 327: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 328: mul r5.xyz, r15.xyzx, r5.xyzx
    r5.xyz = ((r15.xyzx)*(r5.xyzx)).xyz;
    // 329: mul r5.xyz, r5.xyzx, r11.xzwx
    r5.xyz = ((r5.xyzx)*(r11.xzwx)).xyz;
    // 330: mad r0.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 331: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 332: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 333: add r0.w, r9.y, r9.x
    r0.w = ((r9.yyyy)+(r9.xxxx)).w;
    // 334: add_sat r0.w, r9.z, r0.w
    r0.w = (saturate((r9.zzzz)+(r0.wwww))).w;
    // 335: mad r3.xyz, r0.wwww, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(source[9].xyzx)).xyz;
    // 336: mul r3.xyz, r3.xyzx, cb0[19].wwww
    r3.xyz = ((r3.xyzx)*(source[19].wwww)).xyz;
    // 337: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 338: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 339: add r0.w, cb0[0].y, cb0[0].x
    r0.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 340: add r0.w, r0.w, cb0[0].z
    r0.w = ((r0.wwww)+(source[0].zzzz)).w;
    // 341: add r1.w, -r0.w, l(1000.000000)
    r1.w = ((-(r0.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 342: mad r0.w, cb0[20].w, r1.w, r0.w
    r0.w = ((source[20].wwww)*(r1.wwww)+(r0.wwww)).w;
    // 343: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 344: mad r0.w, cb0[20].y, cb0[20].z, r0.w
    r0.w = ((source[20].yyyy)*(source[20].zzzz)+(r0.wwww)).w;
    // 345: mul r1.w, r0.w, l(3.524534)
    r1.w = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 346: sincos null, r1.w, r1.w
    r1.w = (cos(r1.wwww)).w;
    // 347: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 348: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 349: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 350: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 351: mad r0.w, r0.w, l(0.500000), cb0[20].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[20].xxxx)).w;
    // 352: mul r5.xyz, r3.xyzx, r0.wwww
    r5.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 353: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 354: mad r3.xyz, -r0.wwww, r3.xyzx, r1.wwww
    r3.xyz = ((-(r0.wwww))*(r3.xyzx)+(r1.wwww)).xyz;
    // 355: mad r3.xyz, cb0[21].xxxx, r3.xyzx, r5.xyzx
    r3.xyz = ((source[21].xxxx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 356: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 357: add r5.xyz, -r3.xyzx, r0.wwww
    r5.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 358: mad r3.xyz, cb0[21].yyyy, r5.xyzx, r3.xyzx
    r3.xyz = ((source[21].yyyy)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 359: dp3 r0.w, r6.xyzx, r10.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 360: mul_sat r1.w, r0.w, cb0[21].z
    r1.w = (saturate((r0.wwww)*(source[21].zzzz))).w;
    // 361: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 362: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 363: mul_sat r2.z, r10.z, cb0[21].z
    r2.z = (saturate((r10.zzzz)*(source[21].zzzz))).z;
    // 364: add r2.w, -|r10.z|, l(1.000000)
    r2.w = ((-(abs(r10.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 365: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 366: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 367: add_sat r2.z, r2.z, -cb0[21].w
    r2.z = (saturate((r2.zzzz)+(-(source[21].wwww)))).z;
    // 368: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 369: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 370: mul r2.w, r2.w, cb0[22].x
    r2.w = ((r2.wwww)*(source[22].xxxx)).w;
    // 371: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 372: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 373: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 374: mad r5.xyz, r1.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r5.xyz = ((r1.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 375: mul r1.w, r1.w, cb0[12].w
    r1.w = ((r1.wwww)*(source[12].wwww)).w;
    // 376: mad r5.xyz, cb0[13].wwww, r5.xyzx, cb0[13].xyzx
    r5.xyz = ((source[13].wwww)*(r5.xyzx)+(source[13].xyzx)).xyz;
    // 377: mad r5.xyz, r1.wwww, cb0[12].xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(source[12].xyzx)+(r5.xyzx)).xyz;
    // 378: mad r3.xyz, r3.xyzx, r7.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 379: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 380: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 381: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 382: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 383: mul r5.xyz, r1.wwww, cb0[14].xyzx
    r5.xyz = ((r1.wwww)*(source[14].xyzx)).xyz;
    // 384: movc r5.xyz, r0.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 385: add r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)+(r5.xyzx)).xyz;
    // 386: mad r1.xyz, cb0[19].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[19].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 387: add r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(source[3].xyzx)).xyz;
    // 388: mul r3.xyz, r8.wwww, cb0[38].xyzx
    r3.xyz = ((r8.wwww)*(source[38].xyzx)).xyz;
    // 389: mad r3.xyz, r8.zzzz, cb0[37].xyzx, r3.xyzx
    r3.xyz = ((r8.zzzz)*(source[37].xyzx)+(r3.xyzx)).xyz;
    // 390: mul r3.xyz, r3.xyzx, cb0[39].wwww
    r3.xyz = ((r3.xyzx)*(source[39].wwww)).xyz;
    // 391: mul_sat r5.xyz, cb0[18].xyzx, cb0[18].wwww
    r5.xyz = (saturate((source[18].xyzx)*(source[18].wwww))).xyz;
    // 392: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 393: mul r5.xyz, r5.xyzx, cb0[26].xxxx
    r5.xyz = ((r5.xyzx)*(source[26].xxxx)).xyz;
    // 394: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 395: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 396: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 397: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 398: mad r1.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r1.xyzx)).xyz;
    // 399: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 400: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 401: mad o0.xyz, r4.xyzx, cb0[39].xyzx, r1.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[39].xyzx)+(r1.xyzx)).xyz;
    // 402: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 403: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 404: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 405: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 406: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 407: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 408: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 409: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 410: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 411: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 412: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 413: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 414: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 415: ftou r0.x, cb0[36].z
    r0.x = (asfloat((uint4)(source[36].zzzz))).x;
    // 416: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 417: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 418: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 419: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 420: ret
    return output;
}

// source.character.equipment-native-199.v1 / source program 65b6f2f753175c4eac1abd09163a3486
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase199(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19].z=(g_SourceCharacterTime.xxxx).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[26]=g_SourceCharacterEnvironmentColor; source[27]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0;
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
    // 7: add r0.x, -cb0[8].w, l(1.000000)
    r0.x = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
    // 9: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 10: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 11: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: mul r1.x, cb0[8].z, l(1.500000)
    r1.x = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 13: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 14: mad r0.x, r0.x, l(0.500000), cb0[8].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).x;
    // 15: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 16: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 17: mul r2.y, cb0[8].y, cb0[16].y
    r2.y = ((source[8].yyyy)*(source[16].yyyy)).y;
    // 18: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 19: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 20: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 21: frc r1.z, cb0[8].x
    r1.z = (frac(source[8].xxxx)).z;
    // 22: add r1.w, -r1.z, cb0[8].x
    r1.w = ((-(r1.zzzz))+(source[8].xxxx)).w;
    // 23: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 24: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t4.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: mul r1.xyw, r0.xxxx, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r2.xyxz)).xyw;
    // 27: mul r0.x, r1.z, r2.w
    r0.x = ((r1.zzzz)*(r2.wwww)).x;
    // 28: dp3 r1.z, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 29: add r2.xyz, -r0.yzwy, r1.zzzz
    r2.xyz = ((-(r0.yzwy))+(r1.zzzz)).xyz;
    // 30: mad r2.xyz, cb0[20].xxxx, r2.xyzx, r0.yzwy
    r2.xyz = ((source[20].xxxx)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 31: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 32: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 33: mad r2.xyz, cb0[20].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 34: mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 35: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 36: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 37: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 38: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 39: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 42: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 43: mul r1.z, r6.y, cb0[18].y
    r1.z = ((r6.yyyy)*(source[18].yyyy)).z;
    // 44: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 45: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: movc r1.z, r5.y, l(0), r1.z
    r1.z = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 47: mad r3.xyz, r1.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 48: mul r4.xyz, cb0[4].xyzx, cb0[4].wwww
    r4.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 49: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 50: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 51: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 52: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 53: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 54: mad r4.xyz, r1.zzzz, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 55: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 57: mad r3.xyz, r7.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r7.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 58: mul r4.xyz, cb0[6].xyzx, cb0[6].wwww
    r4.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 59: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 60: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 61: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 62: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 63: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 64: mad r4.xyz, r1.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 65: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 66: mad r3.xyz, r7.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 67: mul r4.xyz, cb0[7].xyzx, cb0[7].wwww
    r4.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 68: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 69: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 70: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 71: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 72: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 73: mad r4.xyz, r1.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 74: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 75: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 76: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 78: mad r4.xyz, cb0[20].xxxx, r4.xyzx, r3.xyzx
    r4.xyz = ((source[20].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 79: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 80: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: add r3.xyz, -r4.xyzx, r2.wwww
    r3.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 82: mad r3.xyz, cb0[20].yyyy, r3.xyzx, r4.xyzx
    r3.xyz = ((source[20].yyyy)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 83: mad r4.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: mad r8.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 86: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 87: mul r8.xyz, r2.xyzx, r3.xyzx
    r8.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 88: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: mad r2.xyz, -r3.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 90: mad r2.xyz, cb0[20].xxxx, r2.xyzx, r8.xyzx
    r2.xyz = ((source[20].xxxx)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 91: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 93: mad r2.xyz, cb0[20].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 94: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 95: mad r1.xyw, r1.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r2.xyxz
    r1.xyw = ((r1.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r2.xyxz))).xyw;
    // 96: mad r1.xyw, r0.xxxx, r1.xyxw, r2.xyxz
    r1.xyw = ((r0.xxxx)*(r1.xyxw)+(r2.xyxz)).xyw;
    // 97: add r0.x, r1.y, r1.x
    r0.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 98: add r0.x, r1.w, r0.x
    r0.x = ((r1.wwww)+(r0.xxxx)).x;
    // 99: mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // 100: max r0.x, r0.x, cb0[22].x
    r0.x = (max(r0.xxxx,source[22].xxxx)).x;
    // 101: min r0.x, r0.x, cb0[21].w
    r0.x = (min(r0.xxxx,source[21].wwww)).x;
    // 102: add r2.x, -r0.x, l(1.000000)
    r2.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: mad r0.x, r1.z, r2.x, r0.x
    r0.x = ((r1.zzzz)*(r2.xxxx)+(r0.xxxx)).x;
    // 104: mul_sat r2.w, r1.z, cb2[3].w
    r2.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 105: add r1.z, r0.x, l(-1.000000)
    r1.z = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 106: mad r1.z, cb0[22].z, r1.z, l(1.000000)
    r1.z = ((source[22].zzzz)*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 107: mul r3.xyz, r1.xywx, r1.zzzz
    r3.xyz = ((r1.xywx)*(r1.zzzz)).xyz;
    // 108: mul r2.x, r6.x, cb0[21].y
    r2.x = ((r6.xxxx)*(source[21].yyyy)).x;
    // 109: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 110: movc r2.x, r5.x, l(0), r2.x
    r2.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 111: add_sat r2.x, r2.x, cb0[21].z
    r2.x = (saturate((r2.xxxx)+(source[21].zzzz))).x;
    // 112: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 113: mul r5.xyw, r2.yyyy, cb0[15].xyxz
    r5.xyw = ((r2.yyyy)*(source[15].xyxz)).xyw;
    // 114: mul r3.xyz, r3.xyzx, r5.xywx
    r3.xyz = ((r3.xyzx)*(r5.xywx)).xyz;
    // 115: mad r1.xyz, r1.zzzz, r1.xywx, -r3.xyzx
    r1.xyz = ((r1.zzzz)*(r1.xywx)+(-(r3.xyzx))).xyz;
    // 116: mad r1.xyz, r2.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 117: mul_sat r0.x, r0.x, r2.x
    r0.x = (saturate((r0.xxxx)*(r2.xxxx))).x;
    // 118: add r3.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 119: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 120: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 121: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 122: mad r5.xyw, r1.xyxz, l(2.040400, 2.040400, 0.000000, 2.040400), l(-0.332400, -0.332400, 0.000000, -0.332400)
    r5.xyw = ((r1.xyxz)*(float4(2.040400,2.040400,0.000000,2.040400))+(float4(-0.332400,-0.332400,0.000000,-0.332400))).xyw;
    // 123: mad r6.xyw, r1.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r6.xyw = ((r1.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 124: mad r5.xyw, r0.xxxx, r5.xyxw, r6.xyxw
    r5.xyw = ((r0.xxxx)*(r5.xyxw)+(r6.xyxw)).xyw;
    // 125: mad r3.xyz, r5.xywx, r0.xxxx, r3.xyzx
    r3.xyz = ((r5.xywx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 126: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 127: max r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = (max(r0.xxxx,r3.xyzx)).xyz;
    // 128: mov_sat r1.w, cb0[22].w
    r1.w = (saturate(source[22].wwww)).w;
    // 129: mad r5.xyw, -r1.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r1.xyxz
    r5.xyw = ((-(r1.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r1.xyxz)).xyw;
    // 130: mul r2.x, r1.w, l(0.080000)
    r2.x = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 131: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 132: mad r5.xyw, r2.wwww, r5.xyxw, r2.xxxx
    r5.xyw = ((r2.wwww)*(r5.xyxw)+(r2.xxxx)).xyw;
    // 133: mul_sat r1.w, r5.y, l(50.000000)
    r1.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 134: add r2.x, -cb0[23].w, cb0[23].z
    r2.x = ((-(source[23].wwww))+(source[23].zzzz)).x;
    // 135: mad r2.x, r7.x, r2.x, cb0[23].w
    r2.x = ((r7.xxxx)*(r2.xxxx)+(source[23].wwww)).x;
    // 136: add r2.y, -r2.x, cb0[24].y
    r2.y = ((-(r2.xxxx))+(source[24].yyyy)).y;
    // 137: mad r2.x, r7.y, r2.y, r2.x
    r2.x = ((r7.yyyy)*(r2.yyyy)+(r2.xxxx)).x;
    // 138: add r2.y, -r2.x, cb0[24].w
    r2.y = ((-(r2.xxxx))+(source[24].wwww)).y;
    // 139: mad r2.x, r7.z, r2.y, r2.x
    r2.x = ((r7.zzzz)*(r2.yyyy)+(r2.xxxx)).x;
    // 140: mul r2.x, r6.z, r2.x
    r2.x = ((r6.zzzz)*(r2.xxxx)).x;
    // 141: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 142: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 143: movc r2.x, r5.z, l(0), r2.x
    r2.x = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 144: max r2.x, r2.x, cb0[1].x
    r2.x = (max(r2.xxxx,source[1].xxxx)).x;
    // 145: min r2.z, r2.x, l(1.000000)
    r2.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 147: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 148: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 149: mul r6.xy, r2.xyxx, cb0[18].xxxx
    r6.xy = ((r2.xyxx)*(source[18].xxxx)).xy;
    // 150: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 151: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 152: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 153: add r6.z, r2.x, l(0.000010)
    r6.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 154: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 155: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 156: div r6.xyz, r6.xyzx, r2.xxxx
    r6.xyz = ((r6.xyzx)/(r2.xxxx)).xyz;
    // 157: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 158: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 159: mul r7.xyz, r2.xxxx, r6.xyzx
    r7.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 160: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 161: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 162: mul r8.xyz, r2.xxxx, v5.xyzx
    r8.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 163: dp3 r2.x, r7.xyzx, r8.xyzx
    r2.x = (dot((r7.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 164: deriv_rtx_coarse r9.x, r2.x
    r9.x = (ddx_coarse(r2.xxxx)).x;
    // 165: deriv_rty_coarse r9.y, r2.x
    r9.y = (ddy_coarse(r2.xxxx)).y;
    // 166: dp2 r2.y, r9.xyxx, r9.xyxx
    r2.y = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).y;
    // 167: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 168: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 169: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 170: min r9.y, r2.y, l(1.000000)
    r9.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 171: add r2.y, -r9.y, l(1.000000)
    r2.y = ((-(r9.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 172: max r10.xyz, r5.xywx, r2.yyyy
    r10.xyz = (max(r5.xywx,r2.yyyy)).xyz;
    // 173: add r10.xyz, -r5.xywx, r10.xyzx
    r10.xyz = ((-(r5.xywx))+(r10.xyzx)).xyz;
    // 174: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 175: mul r11.xyz, r2.xxxx, r7.xyzx
    r11.xyz = ((r2.xxxx)*(r7.xyzx)).xyz;
    // 176: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r8.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r8.xyzx))).xyz;
    // 177: add r1.w, r11.z, l(1.000000)
    r1.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 180: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 181: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 182: mul r2.x, r2.x, cb0[2].y
    r2.x = ((r2.xxxx)*(source[2].yyyy)).x;
    // 183: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 184: mad_sat r2.x, r2.x, cb0[2].w, cb0[2].z
    r2.x = (saturate((r2.xxxx)*(source[2].wwww)+(source[2].zzzz))).x;
    // 185: mul r2.x, r2.x, cb0[25].x
    r2.x = ((r2.xxxx)*(source[25].xxxx)).x;
    // 186: add_sat r9.x, -r1.w, r2.y
    r9.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 187: sample_indexable(texture2d)(float,float,float,float) r2.yz, r9.xyxx, t6.zxyw, s7
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 188: add r1.w, r0.x, r9.x
    r1.w = ((r0.xxxx)+(r9.xxxx)).w;
    // 189: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 190: mul r9.xzw, r2.zzzz, r5.xxyw
    r9.xzw = ((r2.zzzz)*(r5.xxyw)).xzw;
    // 191: mad r9.xzw, r10.xxyz, r2.yyyy, r9.xxzw
    r9.xzw = ((r10.xxyz)*(r2.yyyy)+(r9.xxzw)).xzw;
    // 192: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = r2.z != 0.f ? 1.f / r2.z : 0.f;
    // 193: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 194: mad r10.xyz, r5.xywx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((r5.xywx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 195: dp3 r2.y, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 196: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 197: mad r12.xyz, -r9.xzwx, r10.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r9.xzwx))*(r10.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mul r9.xzw, r9.xxzw, r10.xxyz
    r9.xzw = ((r9.xxzw)*(r10.xxyz)).xzw;
    // 199: mul r10.xyz, r1.xyzx, r12.xyzx
    r10.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 200: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 201: mul r10.xyz, r2.yyyy, r10.xyzx
    r10.xyz = ((r2.yyyy)*(r10.xyzx)).xyz;
    // 202: dp3 r2.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 203: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 204: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 205: mul r13.xyz, r3.wwww, v1.xyzx
    r13.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 206: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 207: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 208: mul r14.xyz, r3.wwww, v0.xyzx
    r14.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 209: mul r15.xyz, r13.zxyz, r14.yzxy
    r15.xyz = ((r13.zxyz)*(r14.yzxy)).xyz;
    // 210: mad r15.xyz, r13.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r13.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 211: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 212: dp3 r16.y, r15.xyzx, r7.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 213: dp3 r15.y, r15.xyzx, r11.xyzx
    r15.y = (dot((r15.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 214: dp3 r16.x, r14.xyzx, r7.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 215: dp3 r15.x, r14.xyzx, r11.xyzx
    r15.x = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 216: dp2 r14.z, r16.xyxx, cb0[27].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[27].xyxx).xy).xxxx).z;
    // 217: mul r15.zw, cb0[27].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r15.zw = ((source[27].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 218: dp2 r14.x, r16.xyxx, r15.zwzz
    r14.x = (dot((r16.xyxx).xy,(r15.zwzz).xy).xxxx).x;
    // 219: dp2 r17.x, r15.xyxx, r15.zwzz
    r17.x = (dot((r15.xyxx).xy,(r15.zwzz).xy).xxxx).x;
    // 220: dp2 r17.z, r15.xyxx, cb0[27].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[27].xyxx).xy).xxxx).z;
    // 221: dp3 r14.y, r13.xyzx, r7.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 222: dp3 r17.y, r13.xyzx, r11.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 223: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 224: dp4 r13.x, cb0[28].xyzw, r14.xyzw
    r13.x = (dot((source[28].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 225: dp4 r13.y, cb0[29].xyzw, r14.xyzw
    r13.y = (dot((source[29].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 226: dp4 r13.z, cb0[30].xyzw, r14.xyzw
    r13.z = (dot((source[30].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 227: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 228: dp4 r18.x, cb0[31].xyzw, r15.xyzw
    r18.x = (dot((source[31].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 229: dp4 r18.y, cb0[32].xyzw, r15.xyzw
    r18.y = (dot((source[32].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 230: dp4 r18.z, cb0[33].xyzw, r15.xyzw
    r18.z = (dot((source[33].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 231: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 232: mul r3.w, r14.y, r14.y
    r3.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 233: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 234: mad r3.w, r14.x, r14.x, -r3.w
    r3.w = ((r14.xxxx)*(r14.xxxx)+(-(r3.wwww))).w;
    // 235: mad r13.xyz, cb0[34].xyzx, r3.wwww, r13.xyzx
    r13.xyz = ((source[34].xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 236: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 237: mul r13.xyz, r13.xyzx, cb0[26].xyzx
    r13.xyz = ((r13.xyzx)*(source[26].xyzx)).xyz;
    // 238: mul r13.xyz, r13.xyzx, cb0[27].zzzz
    r13.xyz = ((r13.xyzx)*(source[27].zzzz)).xyz;
    // 239: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[26].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[26].wwww)).xyz;
    // 240: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 241: add r13.xyz, -r3.wwww, r13.xyzx
    r13.xyz = ((-(r3.wwww))+(r13.xyzx)).xyz;
    // 242: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 243: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 244: mad r4.w, r9.y, l(2.000000), l(2.000000)
    r4.w = ((r9.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 245: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 246: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 247: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 248: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 249: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 250: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 251: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 252: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 253: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 254: mul r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = ((r3.wwww)*(r13.xyzx)).xyz;
    // 255: mul r10.xyz, r10.xyzx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r13.xyzx)).xyz;
    // 256: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 257: mul r10.xyz, r3.xyzx, r10.xyzx
    r10.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 258: mul r3.w, r9.y, l(5.000000)
    r3.w = ((r9.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 259: mul r5.w, r9.y, r9.y
    r5.w = ((r9.yyyy)*(r9.yyyy)).w;
    // 260: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 261: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 262: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 263: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 264: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 265: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r17.xyzx, t7.xyzw, s6, r3.w
    r13.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r17.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 266: mul r13.xyz, r13.xyzx, r13.wwww
    r13.xyz = ((r13.xyzx)*(r13.wwww)).xyz;
    // 267: mul r13.xyz, r13.xyzx, cb0[26].xyzx
    r13.xyz = ((r13.xyzx)*(source[26].xyzx)).xyz;
    // 268: mul r13.xyz, r13.xyzx, cb0[27].zzzz
    r13.xyz = ((r13.xyzx)*(source[27].zzzz)).xyz;
    // 269: mad r13.xyz, r13.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[26].wwww
    r13.xyz = ((r13.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[26].wwww)).xyz;
    // 270: dp3 r1.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 271: add r13.xyz, -r1.wwww, r13.xyzx
    r13.xyz = ((-(r1.wwww))+(r13.xyzx)).xyz;
    // 272: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 273: dp3 r1.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 274: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 275: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 276: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 277: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 278: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 279: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 280: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 281: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 282: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 283: mul r13.xyz, r1.wwww, r13.xyzx
    r13.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 284: mul r14.xyz, r9.xzwx, r13.xyzx
    r14.xyz = ((r9.xzwx)*(r13.xyzx)).xyz;
    // 285: mad r1.w, r0.x, r5.x, r5.y
    r1.w = ((r0.xxxx)*(r5.xxxx)+(r5.yyyy)).w;
    // 286: mad r1.w, r1.w, r0.x, r5.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r5.zzzz)).w;
    // 287: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 288: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 289: mad r5.xyz, r14.xyzx, r0.xxxx, r10.xyzx
    r5.xyz = ((r14.xyzx)*(r0.xxxx)+(r10.xyzx)).xyz;
    // 290: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 291: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 292: mul r10.xyz, r1.wwww, v6.xyzx
    r10.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 293: dp3 r1.w, r10.xyzx, r7.xyzx
    r1.w = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 294: dp3 r2.z, -r10.xyzx, r7.xyzx
    r2.z = (dot((-(r10.xyzx)).xyz,(r7.xyzx).xyz).xxxx).z;
    // 295: dp3 r3.w, r10.xyzx, r11.xyzx
    r3.w = (dot((r10.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 296: mad r7.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 297: mad r7.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r7.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 298: mul r7.xyzw, r7.xyzw, r7.xyzw
    r7.xyzw = ((r7.xyzw)*(r7.xyzw)).xyzw;
    // 299: mad r10.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 300: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 301: mul r10.yzw, r10.yyyy, cb0[37].xxyz
    r10.yzw = ((r10.yyyy)*(source[37].xxyz)).yzw;
    // 302: mad r10.xyz, r10.xxxx, cb0[36].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[36].xyzx)+(r10.yzwy)).xyz;
    // 303: mul r10.xyz, r10.xyzx, cb0[38].wwww
    r10.xyz = ((r10.xyzx)*(source[38].wwww)).xyz;
    // 304: mul r10.xyz, r1.xyzx, r10.xyzx
    r10.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 305: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 306: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 307: mul r3.xyz, r12.xyzx, r3.xyzx
    r3.xyz = ((r12.xyzx)*(r3.xyzx)).xyz;
    // 308: mad r3.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 309: mad r3.xyz, r5.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r3.xyzx)).xyz;
    // 310: mul r5.xyz, r7.yyyy, cb0[37].xyzx
    r5.xyz = ((r7.yyyy)*(source[37].xyzx)).xyz;
    // 311: mad r5.xyz, cb0[36].xyzx, r7.xxxx, r5.xyzx
    r5.xyz = ((source[36].xyzx)*(r7.xxxx)+(r5.xyzx)).xyz;
    // 312: mul r5.xyz, r5.xyzx, cb0[38].wwww
    r5.xyz = ((r5.xyzx)*(source[38].wwww)).xyz;
    // 313: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 314: mul r5.xyz, r13.xyzx, r5.xyzx
    r5.xyz = ((r13.xyzx)*(r5.xyzx)).xyz;
    // 315: mul r5.xyz, r5.xyzx, r9.xzwx
    r5.xyz = ((r5.xyzx)*(r9.xzwx)).xyz;
    // 316: mad r3.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 317: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 318: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 319: dp3 r0.x, r6.xyzx, r8.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 320: mul_sat r1.w, r0.x, cb0[20].z
    r1.w = (saturate((r0.xxxx)*(source[20].zzzz))).w;
    // 321: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 322: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 323: mul_sat r2.z, r8.z, cb0[20].z
    r2.z = (saturate((r8.zzzz)*(source[20].zzzz))).z;
    // 324: add r2.w, -|r8.z|, l(1.000000)
    r2.w = ((-(abs(r8.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 325: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 326: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 327: add_sat r2.z, r2.z, -cb0[20].w
    r2.z = (saturate((r2.zzzz)+(-(source[20].wwww)))).z;
    // 328: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 329: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 330: mul r2.w, r2.w, cb0[21].x
    r2.w = ((r2.wwww)*(source[21].xxxx)).w;
    // 331: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 332: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 333: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 334: mad r5.xyz, r1.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r5.xyz = ((r1.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 335: mul r1.w, r1.w, cb0[12].w
    r1.w = ((r1.wwww)*(source[12].wwww)).w;
    // 336: mad r5.xyz, cb0[13].wwww, r5.xyzx, cb0[13].xyzx
    r5.xyz = ((source[13].wwww)*(r5.xyzx)+(source[13].xyzx)).xyz;
    // 337: mad r5.xyz, r1.wwww, cb0[12].xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(source[12].xyzx)+(r5.xyzx)).xyz;
    // 338: add r1.w, cb0[0].y, cb0[0].x
    r1.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 339: add r1.w, r1.w, cb0[0].z
    r1.w = ((r1.wwww)+(source[0].zzzz)).w;
    // 340: add r2.z, -r1.w, l(1000.000000)
    r2.z = ((-(r1.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).z;
    // 341: mad r1.w, cb0[19].w, r2.z, r1.w
    r1.w = ((source[19].wwww)*(r2.zzzz)+(r1.wwww)).w;
    // 342: mul r1.w, r1.w, l(0.010000)
    r1.w = ((r1.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 343: mad r1.w, cb0[19].y, cb0[19].z, r1.w
    r1.w = ((source[19].yyyy)*(source[19].zzzz)+(r1.wwww)).w;
    // 344: mul r2.z, r1.w, l(3.524534)
    r2.z = ((r1.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).z;
    // 345: sincos null, r2.z, r2.z
    r2.z = (cos(r2.zzzz)).z;
    // 346: add r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)+(r2.zzzz)).w;
    // 347: mul r1.w, r1.w, l(1.328987)
    r1.w = ((r1.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 348: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 349: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 350: mad r1.w, r1.w, l(0.500000), cb0[19].x
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[19].xxxx)).w;
    // 351: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 352: mul r8.xyz, cb0[9].xyzx, cb0[18].wwww
    r8.xyz = ((source[9].xyzx)*(source[18].wwww)).xyz;
    // 353: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 354: mul r8.xyz, r1.wwww, r6.xyzx
    r8.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 355: dp3 r2.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 356: mad r6.xyz, -r1.wwww, r6.xyzx, r2.zzzz
    r6.xyz = ((-(r1.wwww))*(r6.xyzx)+(r2.zzzz)).xyz;
    // 357: mad r6.xyz, cb0[20].xxxx, r6.xyzx, r8.xyzx
    r6.xyz = ((source[20].xxxx)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 358: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 359: add r8.xyz, -r6.xyzx, r1.wwww
    r8.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 360: mad r6.xyz, cb0[20].yyyy, r8.xyzx, r6.xyzx
    r6.xyz = ((source[20].yyyy)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 361: mad r4.xyz, r6.xyzx, r4.xyzx, r5.xyzx
    r4.xyz = ((r6.xyzx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 362: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 363: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 364: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 365: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 366: mul r5.xyz, r1.wwww, cb0[14].xyzx
    r5.xyz = ((r1.wwww)*(source[14].xyzx)).xyz;
    // 367: movc r5.xyz, r0.xxxx, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 368: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 369: mad r0.xyz, cb0[18].zzzz, r0.yzwy, r4.xyzx
    r0.xyz = ((source[18].zzzz)*(r0.yzwy)+(r4.xyzx)).xyz;
    // 370: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 371: mul r4.xyz, r7.wwww, cb0[37].xyzx
    r4.xyz = ((r7.wwww)*(source[37].xyzx)).xyz;
    // 372: mad r4.xyz, r7.zzzz, cb0[36].xyzx, r4.xyzx
    r4.xyz = ((r7.zzzz)*(source[36].xyzx)+(r4.xyzx)).xyz;
    // 373: mul r4.xyz, r4.xyzx, cb0[38].wwww
    r4.xyz = ((r4.xyzx)*(source[38].wwww)).xyz;
    // 374: mul_sat r5.xyz, cb0[17].xyzx, cb0[17].wwww
    r5.xyz = (saturate((source[17].xyzx)*(source[17].wwww))).xyz;
    // 375: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 376: mul r5.xyz, r5.xyzx, cb0[25].xxxx
    r5.xyz = ((r5.xyzx)*(source[25].xxxx)).xyz;
    // 377: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 378: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 379: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 380: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 381: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 382: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 383: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 384: mad o0.xyz, r1.xyzx, cb0[38].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[38].xyzx)+(r0.xyzx)).xyz;
    // 385: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 386: dp3 r0.x, r16.xyzx, r16.xyzx
    r0.x = (dot((r16.xyzx).xyz,(r16.xyzx).xyz).xxxx).x;
    // 387: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 388: mul r0.xyz, r0.xxxx, r16.xyzx
    r0.xyz = ((r0.xxxx)*(r16.xyzx)).xyz;
    // 389: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 390: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 391: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 392: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 393: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 394: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 395: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 396: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 397: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 398: ftou r0.x, cb0[35].z
    r0.x = (asfloat((uint4)(source[35].zzzz))).x;
    // 399: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 400: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 401: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 402: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 403: ret
    return output;
}

// source.character.equipment-native-200.v1 / source program 8365eeb73a4be146bad5502a36665b4a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase200(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[23]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[25]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[26]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[38].x=(g_SourceCharacterTime.xxxx).x;
    source[38].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[38].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[38].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: mad r0.xy, -cb0[14].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[14].zwzz
    r0.xy = ((-(source[14].xyxx))*(float4(0.500000,0.500000,0.000000,0.000000))+(source[14].zwzz)).xy;
    // 2: div r0.xy, r0.xyxx, cb0[14].xyxx
    r0.xy = ((r0.xyxx)/(source[14].xyxx)).xy;
    // 3: mad r0.zw, cb0[31].wwww, cb0[15].xxxy, r0.xxxy
    r0.zw = ((source[31].wwww)*(source[15].xxxy)+(r0.xxxy)).zw;
    // 4: add r0.xy, r0.xyxx, cb0[16].xyxx
    r0.xy = ((r0.xyxx)+(source[16].xyxx)).xy;
    // 5: div r1.xy, l(1024.000000, 1024.000000, 0.000000, 0.000000), cb0[14].xyxx
    r1.xy = ((float4(1024.000000,1024.000000,0.000000,0.000000))/(source[14].xyxx)).xy;
    // 6: mad r0.zw, v4.xxxy, r1.xxxy, -r0.zzzw
    r0.zw = ((v4.xxxy)*(r1.xxxy)+(-(r0.zzzw))).zw;
    // 7: mad r0.xy, v4.xyxx, r1.xyxx, -r0.xyxx
    r0.xy = ((v4.xyxx)*(r1.xyxx)+(-(r0.xyxx))).xy;
    // 8: add r0.xyzw, r0.xyzw, l(-0.500000, -0.500000, -0.500000, -0.500000)
    r0.xyzw = ((r0.xyzw)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).xyzw;
    // 9: mul r1.x, cb0[31].w, cb0[32].x
    r1.x = ((source[31].wwww)*(source[32].xxxx)).x;
    // 10: mul r1.x, r1.x, l(6.283185)
    r1.x = ((r1.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 11: sincos r1.x, r2.x, r1.x
    r1.x = (sin(r1.xxxx)).x; r2.x = (cos(r1.xxxx)).x;
    // 12: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 13: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 14: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 15: dp2 r1.y, r0.wzww, r3.yzyy
    r1.y = (dot((r0.wzww).xy,(r3.yzyy).xy).xxxx).y;
    // 16: dp2 r1.x, r0.wzww, r3.xyxx
    r1.x = (dot((r0.wzww).xy,(r3.xyxx).xy).xxxx).x;
    // 17: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 18: mul r1.x, cb0[31].w, cb0[33].x
    r1.x = ((source[31].wwww)*(source[33].xxxx)).x;
    // 19: mul r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)*(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 20: mad r1.y, -cb0[31].w, cb0[33].x, l(1.000000)
    r1.y = ((-(source[31].wwww))*(source[33].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 21: mad r0.zw, r1.yyyy, r0.zzzw, -r1.xxxx
    r0.zw = ((r1.yyyy)*(r0.zzzw)+(-(r1.xxxx))).zw;
    // 22: mul r1.z, cb0[33].z, l(6.283185)
    r1.z = ((source[33].zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 23: sincos r2.x, r3.x, r1.z
    r2.x = (sin(r1.zzzz)).x; r3.x = (cos(r1.zzzz)).x;
    // 24: mov r4.x, -r2.x
    r4.x = (-(r2.xxxx)).x;
    // 25: mov r4.y, r3.x
    r4.y = (r3.xxxx).y;
    // 26: mov r4.z, r2.x
    r4.z = (r2.xxxx).z;
    // 27: dp2 r2.y, r0.yxyy, r4.yzyy
    r2.y = (dot((r0.yxyy).xy,(r4.yzyy).xy).xxxx).y;
    // 28: dp2 r2.x, r0.yxyy, r4.xyxx
    r2.x = (dot((r0.yxyy).xy,(r4.xyxx).xy).xxxx).x;
    // 29: add r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 30: add r2.z, -r2.x, l(1.000000)
    r2.z = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: mad r0.xy, r1.yyyy, r2.zyzz, -r1.xxxx
    r0.xy = ((r1.yyyy)*(r2.zyzz)+(-(r1.xxxx))).xy;
    // 32: add r1.xy, -r0.xyxx, r0.zwzz
    r1.xy = ((-(r0.xyxx))+(r0.zwzz)).xy;
    // 33: mad r1.xy, cb0[33].wwww, r1.xyxx, r0.xyxx
    r1.xy = ((source[33].wwww)*(r1.xyxx)+(r0.xyxx)).xy;
    // 34: add r0.xy, -r0.zwzz, r0.xyxx
    r0.xy = ((-(r0.zwzz))+(r0.xyxx)).xy;
    // 35: mul r0.xy, r0.xyxx, cb0[33].wwww
    r0.xy = ((r0.xyxx)*(source[33].wwww)).xy;
    // 36: mad r0.xy, cb0[31].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[31].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t7.xyzw, s7, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t7.xyzw, s7, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: add r2.x, r0.w, r1.w
    r2.x = ((r0.wwww)+(r1.wwww)).x;
    // 40: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 41: mad r1.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 42: mad r2.yzw, r2.xxxx, cb0[17].xxyz, -r1.xxyz
    r2.yzw = ((r2.xxxx)*(source[17].xxyz)+(-(r1.xxyz))).yzw;
    // 43: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 44: add r1.w, -r0.w, r2.x
    r1.w = ((-(r0.wwww))+(r2.xxxx)).w;
    // 45: mad r1.w, cb0[34].y, r1.w, r0.w
    r1.w = ((source[34].yyyy)*(r1.wwww)+(r0.wwww)).w;
    // 46: mad r3.xyz, r0.wwww, cb0[17].xyzx, -r0.xyzx
    r3.xyz = ((r0.wwww)*(source[17].xyzx)+(-(r0.xyzx))).xyz;
    // 47: mad r0.xyz, cb0[34].xxxx, r3.xyzx, r0.xyzx
    r0.xyz = ((source[34].xxxx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 48: mad r1.xyz, cb0[34].xxxx, r2.yzwy, r1.xyzx
    r1.xyz = ((source[34].xxxx)*(r2.yzwy)+(r1.xyzx)).xyz;
    // 49: add r1.xyz, -r0.xyzx, r1.xyzx
    r1.xyz = ((-(r0.xyzx))+(r1.xyzx)).xyz;
    // 50: mad r0.xyz, cb0[34].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[34].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 51: mul r0.w, cb0[28].w, cb0[30].x
    r0.w = ((source[28].wwww)*(source[30].xxxx)).w;
    // 52: mul r0.w, r0.w, l(-0.500000)
    r0.w = ((r0.wwww)*(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 53: mul r2.w, cb0[28].w, cb0[29].z
    r2.w = ((source[28].wwww)*(source[29].zzzz)).w;
    // 54: mov r3.zw, l(0,0,0.004000,-0.004000)
    r3.zw = (float4(asfloat(0u),asfloat(0u),0.004000,-0.004000)).zw;
    // 55: mad r1.xy, -cb0[11].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[11].zwzz
    r1.xy = ((-(source[11].xyxx))*(float4(0.500000,0.500000,0.000000,0.000000))+(source[11].zwzz)).xy;
    // 56: div r3.xy, r1.xyxx, cb0[11].xyxx
    r3.xy = ((r1.xyxx)/(source[11].xyxx)).xy;
    // 57: mad r2.x, -cb0[28].w, cb0[29].x, r3.x
    r2.x = ((-(source[28].wwww))*(source[29].xxxx)+(r3.xxxx)).x;
    // 58: add r1.xy, r2.xwxx, r3.wyww
    r1.xy = ((r2.xwxx)+(r3.wyww)).xy;
    // 59: div r2.xz, l(1024.000000, 0.000000, 1024.000000, 0.000000), cb0[11].xxyx
    r2.xz = ((float4(1024.000000,0.000000,1024.000000,0.000000))/(source[11].xxyx)).xz;
    // 60: mad r1.xy, v4.xyxx, r2.xzxx, -r1.xyxx
    r1.xy = ((v4.xyxx)*(r2.xzxx)+(-(r1.xyxx))).xy;
    // 61: mad r1.z, -cb0[28].w, cb0[30].x, l(1.000000)
    r1.z = ((-(source[28].wwww))*(source[30].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 62: mad r4.yz, r1.zzzz, r1.xxyx, -r0.wwww
    r4.yz = ((r1.zzzz)*(r1.xxyx)+(-(r0.wwww))).yz;
    // 63: add r1.x, r1.z, -r4.y
    r1.x = ((r1.zzzz)+(-(r4.yyyy))).x;
    // 64: add r4.x, r1.x, l(1.000000)
    r4.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xzxx, t6.xyzw, s6, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 66: mad r2.y, cb0[28].w, cb0[29].x, r3.x
    r2.y = ((source[28].wwww)*(source[29].xxxx)+(r3.xxxx)).y;
    // 67: add r1.xy, r2.ywyy, r3.zyzz
    r1.xy = ((r2.ywyy)+(r3.zyzz)).xy;
    // 68: mad r3.xy, v4.xyxx, r2.xzxx, -r3.xyxx
    r3.xy = ((v4.xyxx)*(r2.xzxx)+(-(r3.xyxx))).xy;
    // 69: mad r1.xy, v4.xyxx, r2.xzxx, -r1.xyxx
    r1.xy = ((v4.xyxx)*(r2.xzxx)+(-(r1.xyxx))).xy;
    // 70: mad r1.xy, r1.zzzz, r1.xyxx, -r0.wwww
    r1.xy = ((r1.zzzz)*(r1.xyxx)+(-(r0.wwww))).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t6.xyzw, s6, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 72: add r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)+(r1.xyzx)).xyz;
    // 73: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 74: mad r2.xy, -cb0[8].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[8].zwzz
    r2.xy = ((-(source[8].xyxx))*(float4(0.500000,0.500000,0.000000,0.000000))+(source[8].zwzz)).xy;
    // 75: div r2.xy, r2.xyxx, cb0[8].xyxx
    r2.xy = ((r2.xyxx)/(source[8].xyxx)).xy;
    // 76: div r2.zw, l(0.000000, 0.000000, 1024.000000, 1024.000000), cb0[8].xxxy
    r2.zw = ((float4(0.000000,0.000000,1024.000000,1024.000000))/(source[8].xxxy)).zw;
    // 77: mad r2.xy, v4.xyxx, r2.zwzz, -r2.xyxx
    r2.xy = ((v4.xyxx)*(r2.zwzz)+(-(r2.xyxx))).xy;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 79: mul r0.w, r2.x, cb0[7].w
    r0.w = ((r2.xxxx)*(source[7].wwww)).w;
    // 80: mul r2.x, r2.y, cb0[9].w
    r2.x = ((r2.yyyy)*(source[9].wwww)).x;
    // 81: mad r2.yz, -cb0[6].xxyx, l(0.000000, 0.500000, 0.500000, 0.000000), cb0[6].zzwz
    r2.yz = ((-(source[6].xxyx))*(float4(0.000000,0.500000,0.500000,0.000000))+(source[6].zzwz)).yz;
    // 82: div r2.yz, r2.yyzy, cb0[6].xxyx
    r2.yz = ((r2.yyzy)/(source[6].xxyx)).yz;
    // 83: div r4.xy, l(1024.000000, 1024.000000, 0.000000, 0.000000), cb0[6].xyxx
    r4.xy = ((float4(1024.000000,1024.000000,0.000000,0.000000))/(source[6].xyxx)).xy;
    // 84: mad r2.yz, v4.xxyx, r4.xxyx, -r2.yyzy
    r2.yz = ((v4.xxyx)*(r4.xxyx)+(-(r2.yyzy))).yz;
    // 85: sample_b_indexable(texture2d)(float,float,float,float) r2.y, r2.yzyy, t4.yxzw, s4, l(0.000000)
    r2.y = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 86: mul r2.y, r2.y, cb0[5].w
    r2.y = ((r2.yyyy)*(source[5].wwww)).y;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 88: mul r5.xyz, r4.xyzx, cb0[4].xyzx
    r5.xyz = ((r4.xyzx)*(source[4].xyzx)).xyz;
    // 89: mad r6.xyz, -r4.xyzx, cb0[4].xyzx, cb0[5].xyzx
    r6.xyz = ((-(r4.xyzx))*(source[4].xyzx)+(source[5].xyzx)).xyz;
    // 90: mad r2.yzw, r2.yyyy, r6.xxyz, r5.xxyz
    r2.yzw = ((r2.yyyy)*(r6.xxyz)+(r5.xxyz)).yzw;
    // 91: add r5.xyz, -r2.yzwy, cb0[7].xyzx
    r5.xyz = ((-(r2.yzwy))+(source[7].xyzx)).xyz;
    // 92: mad r2.yzw, r0.wwww, r5.xxyz, r2.yyzw
    r2.yzw = ((r0.wwww)*(r5.xxyz)+(r2.yyzw)).yzw;
    // 93: add r5.xyz, -r2.yzwy, cb0[9].xyzx
    r5.xyz = ((-(r2.yzwy))+(source[9].xyzx)).xyz;
    // 94: mad r2.xyz, r2.xxxx, r5.xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(r5.xyzx)+(r2.yzwy)).xyz;
    // 95: add r5.xyz, -r2.xyzx, cb0[10].xyzx
    r5.xyz = ((-(r2.xyzx))+(source[10].xyzx)).xyz;
    // 96: mad r1.xyz, r1.xyzx, r5.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 97: add r2.xyz, -r1.xyzx, cb0[12].xyzx
    r2.xyz = ((-(r1.xyzx))+(source[12].xyzx)).xyz;
    // 98: add r3.z, -r3.x, l(1.990000)
    r3.z = ((-(r3.xxxx))+(float4(1.990000,1.990000,1.990000,1.990000))).z;
    // 99: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.zyzz, t6.xyzw, s6, l(0.000000)
    r0.w = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r2.w = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 101: add r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)+(r2.wwww)).w;
    // 102: mul r0.w, r0.w, cb0[12].w
    r0.w = ((r0.wwww)*(source[12].wwww)).w;
    // 103: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 104: add r2.xyz, -r1.xyzx, cb0[13].xyzx
    r2.xyz = ((-(r1.xyzx))+(source[13].xyzx)).xyz;
    // 105: mul r0.w, r4.w, cb0[13].w
    r0.w = ((r4.wwww)*(source[13].wwww)).w;
    // 106: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 107: add r2.xyz, -r1.xyzx, r4.xyzx
    r2.xyz = ((-(r1.xyzx))+(r4.xyzx)).xyz;
    // 108: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t2.xywz, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 109: mad r1.xyz, r3.yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 110: add r0.xyz, r0.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)+(-(r1.xyzx))).xyz;
    // 111: mad r0.w, r1.w, cb0[17].w, -r1.w
    r0.w = ((r1.wwww)*(source[17].wwww)+(-(r1.wwww))).w;
    // 112: mad r0.w, cb0[34].x, r0.w, r1.w
    r0.w = ((source[34].xxxx)*(r0.wwww)+(r1.wwww)).w;
    // 113: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 114: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t8.xyzw, s8, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 115: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 116: mul r1.xyz, v7.yyyy, cb1[1].xywx
    r1.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 117: mad r1.xyz, cb1[0].xywx, v7.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v7.xxxx)+(r1.xyzx)).xyz;
    // 118: mad r1.xyz, cb1[2].xywx, v7.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v7.zzzz)+(r1.xyzx)).xyz;
    // 119: mad r1.xyz, cb1[3].xywx, v7.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v7.wwww)+(r1.xyzx)).xyz;
    // 120: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 121: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 122: mul r1.xy, r1.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 123: deriv_rtx_coarse r1.zw, r1.xxxy
    r1.zw = (ddx_coarse(r1.xxxy)).zw;
    // 124: deriv_rty_coarse r1.xy, r1.xyxx
    r1.xy = (ddy_coarse(r1.xyxx)).xy;
    // 125: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 126: dp2 r1.x, r1.zwzz, r1.zwzz
    r1.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // 127: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 128: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 129: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 130: rcp r1.x, |r0.w|
    r1.x = (1.0/(abs(r0.wwww))).x;
    // 131: mul r1.x, r1.x, cb0[35].y
    r1.x = ((r1.xxxx)*(source[35].yyyy)).x;
    // 132: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 133: add r0.w, |r0.w|, r1.x
    r0.w = ((abs(r0.wwww))+(r1.xxxx)).w;
    // 134: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 136: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 137: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 138: mul r2.xy, r1.xyxx, cb0[27].zzzz
    r2.xy = ((r1.xyxx)*(source[27].zzzz)).xy;
    // 139: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 140: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 141: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 142: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 143: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 144: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 145: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 146: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 148: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 149: add r1.z, r1.w, l(0.000010)
    r1.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 150: mad r1.xyz, cb0[27].wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((source[27].wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 151: add r2.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 152: mad r2.xyz, cb0[34].zzzz, r2.xyzx, r1.xyzx
    r2.xyz = ((source[34].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 153: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 154: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 155: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 156: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 157: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 158: mul r4.xyz, r1.wwww, v0.xyzx
    r4.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 159: dp3 r5.x, r4.xyzx, r2.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 160: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 161: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 162: mul r6.xyz, r1.wwww, v1.xyzx
    r6.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 163: dp3 r5.z, r6.xyzx, r2.xyzx
    r5.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 164: mul r7.xyz, r4.yzxy, r6.zxyz
    r7.xyz = ((r4.yzxy)*(r6.zxyz)).xyz;
    // 165: mad r7.xyz, r6.yzxy, r4.zxyz, -r7.xyzx
    r7.xyz = ((r6.yzxy)*(r4.zxyz)+(-(r7.xyzx))).xyz;
    // 166: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 167: dp3 r5.y, r7.xyzx, r2.xyzx
    r5.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 168: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 169: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 170: mul r2.xyz, r1.wwww, v5.xyzx
    r2.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 171: mad r8.xyz, v5.xyzx, r1.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((v5.xyzx)*(r1.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 172: dp3 r9.y, r7.xyzx, r2.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 173: dp3 r9.x, r4.xyzx, r2.xyzx
    r9.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 174: dp3 r9.z, r6.xyzx, r2.xyzx
    r9.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 175: dp3 r1.w, r5.xyzx, r9.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 176: mul r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)*(r1.wwww)).xyz;
    // 177: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 178: mov r5.w, -r5.x
    r5.w = (-(r5.xxxx)).w;
    // 179: dp2 r1.w, r5.ywyy, r5.ywyy
    r1.w = (dot((r5.ywyy).xy,(r5.ywyy).xy).xxxx).w;
    // 180: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 181: div r3.yw, r5.yyyw, r1.wwww
    r3.yw = ((r5.yyyw)/(r1.wwww)).yw;
    // 182: mad r1.w, -r5.z, l(0.250000), l(0.250000)
    r1.w = ((-(r5.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 183: mad r3.yw, r1.wwww, r3.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r3.yw = ((r1.wwww)*(r3.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 184: sample_l_indexable(texture2d)(float,float,float,float) r5.xyz, r3.ywyy, t9.xyzw, s9, r0.w
    r5.xyz = ((g_SourceCharacterTexture9.SampleLevel(SourceCharacterLookupSampler, (r3.ywyy).xy, (r0.wwww).x)).xyzw).xyz;
    // 185: log r9.xyz, r5.xyzx
    r9.xyz = (log2(r5.xyzx)).xyz;
    // 186: rcp r0.w, cb0[35].z
    r0.w = (1.0/(source[35].zzzz)).w;
    // 187: mul r10.xyz, r9.xyzx, r0.wwww
    r10.xyz = ((r9.xyzx)*(r0.wwww)).xyz;
    // 188: mul r9.xyz, r9.xyzx, cb0[35].zzzz
    r9.xyz = ((r9.xyzx)*(source[35].zzzz)).xyz;
    // 189: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 190: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 191: mul r10.xyz, r0.wwww, r10.xyzx
    r10.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 192: mad r9.xyz, r9.xyzx, cb0[35].zzzz, r10.xyzx
    r9.xyz = ((r9.xyzx)*(source[35].zzzz)+(r10.xyzx)).xyz;
    // 193: add r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)+(r9.xyzx)).xyz;
    // 194: mul r5.xyz, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 195: add r0.w, cb0[35].z, l(1.000000)
    r0.w = ((source[35].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 196: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 197: mul r5.xyz, r5.xyzx, cb0[35].wwww
    r5.xyz = ((r5.xyzx)*(source[35].wwww)).xyz;
    // 198: mul r9.xyz, r0.xyzx, r5.xyzx
    r9.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 199: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 200: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 201: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 202: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 203: mul_sat r1.w, r0.w, cb0[36].x
    r1.w = (saturate((r0.wwww)*(source[36].xxxx))).w;
    // 204: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 205: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 206: mul_sat r2.w, r2.z, cb0[36].x
    r2.w = (saturate((r2.zzzz)*(source[36].xxxx))).w;
    // 207: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 208: add_sat r2.w, r2.w, -cb0[36].y
    r2.w = (saturate((r2.wwww)+(-(source[36].yyyy)))).w;
    // 209: log r3.y, r2.w
    r3.y = (log2(r2.wwww)).y;
    // 210: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 211: mul r3.y, r3.y, cb0[36].z
    r3.y = ((r3.yyyy)*(source[36].zzzz)).y;
    // 212: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 213: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 214: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 215: max r2.w, |r3.x|, l(0.000001)
    r2.w = (max(abs(r3.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 216: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 217: mul r2.w, r2.w, l(0.454545)
    r2.w = ((r2.wwww)*(float4(0.454545,0.454545,0.454545,0.454545))).w;
    // 218: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 219: dp3 r2.w, r2.wwww, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.wwww).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 220: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 221: mul r2.w, r2.w, cb0[36].w
    r2.w = ((r2.wwww)*(source[36].wwww)).w;
    // 222: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 223: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 224: mad r3.y, -r2.w, r2.w, l(1.000000)
    r3.y = ((-(r2.wwww))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 225: max r3.y, r3.y, l(0.001000)
    r3.y = (max(r3.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 226: div r3.y, cb0[37].x, r3.y
    r3.y = ((source[37].xxxx)/(r3.yyyy)).y;
    // 227: mul r3.y, r1.w, r3.y
    r3.y = ((r1.wwww)*(r3.yyyy)).y;
    // 228: mul r5.xyz, r5.xyzx, r3.yyyy
    r5.xyz = ((r5.xyzx)*(r3.yyyy)).xyz;
    // 229: mad r3.xyw, r3.xxxx, r5.xyxz, -r9.xyxz
    r3.xyw = ((r3.xxxx)*(r5.xyxz)+(-(r9.xyxz))).xyw;
    // 230: add r3.z, -r3.z, l(1.000000)
    r3.z = ((-(r3.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 231: add r4.w, -r2.w, l(1.000000)
    r4.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 232: mul r4.w, r4.w, cb0[37].y
    r4.w = ((r4.wwww)*(source[37].yyyy)).w;
    // 233: mad r3.xyw, r4.wwww, r3.xyxw, r9.xyxz
    r3.xyw = ((r4.wwww)*(r3.xyxw)+(r9.xyxz)).xyw;
    // 234: dp3 r4.w, r8.xyzx, r8.xyzx
    r4.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 235: sqrt r5.x, r4.w
    r5.x = (sqrt(r4.wwww)).x;
    // 236: div r5.xyz, r8.xyzx, r5.xxxx
    r5.xyz = ((r8.xyzx)/(r5.xxxx)).xyz;
    // 237: dp3 r2.x, r5.xyzx, r2.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 238: add r2.y, -|r2.z|, l(1.000000)
    r2.y = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 239: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 240: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 241: mul r2.y, |r2.x|, |r2.x|
    r2.y = ((abs(r2.xxxx))*(abs(r2.xxxx))).y;
    // 242: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 243: mul r2.y, r2.y, |r2.x|
    r2.y = ((r2.yyyy)*(abs(r2.xxxx))).y;
    // 244: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 245: movc r2.x, r2.x, l(0), r2.y
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).x;
    // 246: add r2.y, r2.x, l(-0.027778)
    r2.y = ((r2.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 247: mad r2.x, r2.x, r2.y, l(0.027778)
    r2.x = ((r2.xxxx)*(r2.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 248: div_sat r2.x, r2.x, r4.w
    r2.x = (saturate((r2.xxxx)/(r4.wwww))).x;
    // 249: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 250: log r2.y, |r3.z|
    r2.y = (log2(abs(r3.zzzz))).y;
    // 251: lt r2.z, |r3.z|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(r3.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 252: mul r2.y, r2.y, cb0[28].x
    r2.y = ((r2.yyyy)*(source[28].xxxx)).y;
    // 253: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 254: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 255: mul r2.x, r2.x, r2.y
    r2.x = ((r2.xxxx)*(r2.yyyy)).x;
    // 256: movc r2.x, r2.z, l(0), r2.x
    r2.x = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 257: mad r2.xyz, r2.xxxx, r3.xywx, -r0.xyzx
    r2.xyz = ((r2.xxxx)*(r3.xywx)+(-(r0.xyzx))).xyz;
    // 258: mad r0.xyz, r2.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 259: dp3 r2.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 260: add r2.xyz, -r0.xyzx, r2.xxxx
    r2.xyz = ((-(r0.xyzx))+(r2.xxxx)).xyz;
    // 261: mad r0.xyz, cb0[37].zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((source[37].zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 262: dp3 r2.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 263: add r2.xyz, -r0.xyzx, r2.xxxx
    r2.xyz = ((-(r0.xyzx))+(r2.xxxx)).xyz;
    // 264: mad r0.xyz, cb0[37].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[37].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 265: mad r2.xyz, cb0[18].wwww, cb0[18].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[18].wwww)*(source[18].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 266: mad r5.xyz, cb0[19].wwww, cb0[19].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[19].wwww)*(source[19].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 267: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 268: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 269: add r2.w, -cb0[3].w, l(1.000000)
    r2.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 270: mul r2.w, r2.w, cb0[38].x
    r2.w = ((r2.wwww)*(source[38].xxxx)).w;
    // 271: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 272: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 273: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 274: mul r3.z, cb0[3].z, l(1.500000)
    r3.z = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 275: mul r2.w, r2.w, r3.z
    r2.w = ((r2.wwww)*(r3.zzzz)).w;
    // 276: mad r2.w, r2.w, l(0.500000), cb0[3].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 277: frc r3.z, v4.x
    r3.z = (frac(v4.xxxx)).z;
    // 278: mul r5.x, r3.z, l(0.125000)
    r5.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 279: mul r8.y, cb0[3].y, cb0[23].y
    r8.y = ((source[3].yyyy)*(source[23].yyyy)).y;
    // 280: mov r5.y, v4.y
    r5.y = (v4.yyyy).y;
    // 281: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 282: add r5.xy, r5.xyxx, r8.xyxx
    r5.xy = ((r5.xyxx)+(r8.xyxx)).xy;
    // 283: frc r3.z, cb0[3].x
    r3.z = (frac(source[3].xxxx)).z;
    // 284: add r4.w, -r3.z, cb0[3].x
    r4.w = ((-(r3.zzzz))+(source[3].xxxx)).w;
    // 285: mul r8.z, r4.w, l(0.125000)
    r8.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 286: add r5.xy, r5.xyxx, r8.zwzz
    r5.xy = ((r5.xyxx)+(r8.zwzz)).xy;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t10.xyzw, s10, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture10.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 288: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 289: mul r2.w, r3.z, r5.w
    r2.w = ((r3.zzzz)*(r5.wwww)).w;
    // 290: add r3.z, -r3.z, l(1.000000)
    r3.z = ((-(r3.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 291: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 292: mad r0.xyz, r2.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 293: add r5.xyzw, v7.yzxy, cb0[0].yzxy
    r5.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 294: add r5.xyzw, r5.xyzw, -cb0[1].yzxy
    r5.xyzw = ((r5.xyzw)+(-(source[1].yzxy))).xyzw;
    // 295: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 296: add r5.xy, -r5.zwzz, r5.xyxx
    r5.xy = ((-(r5.zwzz))+(r5.xyxx)).xy;
    // 297: mad r5.xy, cb0[24].wwww, r5.xyxx, r5.zwzz
    r5.xy = ((source[24].wwww)*(r5.xyxx)+(r5.zwzz)).xy;
    // 298: mul r2.w, cb0[24].y, cb0[38].x
    r2.w = ((source[24].yyyy)*(source[38].xxxx)).w;
    // 299: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 300: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 301: mul r8.y, r2.w, l(0.020000)
    r8.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 302: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 303: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 304: mul r4.w, cb0[24].x, l(0.001000)
    r4.w = ((source[24].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 305: mov r8.x, l(0)
    r8.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 306: mad r5.xy, r4.wwww, r5.xyxx, r8.xyxx
    r5.xy = ((r4.wwww)*(r5.xyxx)+(r8.xyxx)).xy;
    // 307: dp2 r4.w, cb0[25].xyxx, r5.xyxx
    r4.w = (dot((source[25].xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 308: dp2 r5.y, cb0[26].xyxx, r5.xyxx
    r5.y = (dot((source[26].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 309: frc r4.w, r4.w
    r4.w = (frac(r4.wwww)).w;
    // 310: mul r5.x, r4.w, l(0.125000)
    r5.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 311: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t10.xyzw, s10, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture10.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 312: mad r5.xyz, r5.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r5.xyz = ((r5.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 313: mul r4.w, r5.w, l(0.900000)
    r4.w = ((r5.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 314: mad r5.xyz, r4.wwww, r5.xyzx, r0.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 315: mul_sat r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = (saturate((r2.wwww)*(r5.xyzx))).xyz;
    // 316: mad r8.xyz, cb0[24].zzzz, r5.xyzx, -r0.xyzx
    r8.xyz = ((source[24].zzzz)*(r5.xyzx)+(-(r0.xyzx))).xyz;
    // 317: mul r5.xyz, r5.xyzx, cb0[24].zzzz
    r5.xyz = ((r5.xyzx)*(source[24].zzzz)).xyz;
    // 318: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 319: mul r2.w, r2.w, l(3.000000)
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 320: mad r0.xyz, r2.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 321: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 322: mul r5.xyz, r3.xywx, r3.zzzz
    r5.xyz = ((r3.xywx)*(r3.zzzz)).xyz;
    // 323: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 324: mad r3.xyz, -r3.zzzz, r3.xywx, r2.wwww
    r3.xyz = ((-(r3.zzzz))*(r3.xywx)+(r2.wwww)).xyz;
    // 325: mad r3.xyz, cb0[37].zzzz, r3.xyzx, r5.xyzx
    r3.xyz = ((source[37].zzzz)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 326: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 327: add r5.xyz, -r3.xyzx, r2.wwww
    r5.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 328: mad r3.xyz, cb0[37].wwww, r5.xyzx, r3.xyzx
    r3.xyz = ((source[37].wwww)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 329: mad r5.xyz, r1.wwww, cb0[21].xyzx, -cb0[21].xyzx
    r5.xyz = ((r1.wwww)*(source[21].xyzx)+(-(source[21].xyzx))).xyz;
    // 330: mul r1.w, r1.w, cb0[20].w
    r1.w = ((r1.wwww)*(source[20].wwww)).w;
    // 331: mad r5.xyz, cb0[21].wwww, r5.xyzx, cb0[21].xyzx
    r5.xyz = ((source[21].wwww)*(r5.xyzx)+(source[21].xyzx)).xyz;
    // 332: mad r5.xyz, r1.wwww, cb0[20].xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(source[20].xyzx)+(r5.xyzx)).xyz;
    // 333: mad r2.xyz, r3.xyzx, r2.xyzx, r5.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 334: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 335: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 336: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 337: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 338: mul r3.xyz, r1.wwww, cb0[22].xyzx
    r3.xyz = ((r1.wwww)*(source[22].xyzx)).xyz;
    // 339: movc r3.xyz, r0.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 340: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 341: add r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)+(source[2].xyzx)).xyz;
    // 342: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 343: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 344: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 345: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 346: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 347: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 348: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 349: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 350: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 351: mul r3.yzw, r3.yyyy, cb0[40].xxyz
    r3.yzw = ((r3.yyyy)*(source[40].xxyz)).yzw;
    // 352: mad r3.xyz, r3.xxxx, cb0[39].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[39].xyzx)+(r3.yzwy)).xyz;
    // 353: mul r3.xyz, r3.xyzx, cb0[41].wwww
    r3.xyz = ((r3.xyzx)*(source[41].wwww)).xyz;
    // 354: mad r2.xyz, r3.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 355: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 356: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 357: mad o0.xyz, r0.xyzx, cb0[41].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[41].xyzx)+(r2.xyzx)).xyz;
    // 358: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 359: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 360: dp3 r0.x, r4.xyzx, r1.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 361: dp3 r0.z, r6.xyzx, r1.xyzx
    r0.z = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 362: dp3 r0.y, r7.xyzx, r1.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 363: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 364: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 365: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 366: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 367: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 368: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 369: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 370: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 371: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 372: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 373: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 374: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 375: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 376: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 377: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 378: ret
    return output;
}

// source.character.static-map-color-mask.v1 / source program 52d02377bfee2249bb1a7fb96f803686
