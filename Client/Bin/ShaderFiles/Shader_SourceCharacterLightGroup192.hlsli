SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight192(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[15].y=(g_SourceCharacterTime.xxxx).x;
    source[15].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[15].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[16].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[16].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[18]=float4(input.lightColor,1.0);
    source[24].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[24].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[24].yyyy)) * 0xffffffffu)).w;
    // 6: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 7: mul r2.xyzw, v8.yyyy, cb1[1].xyzw
    r2.xyzw = ((v8.yyyy)*(projection[1].xyzw)).xyzw;
    // 8: mad r2.xyzw, cb1[0].xyzw, v8.xxxx, r2.xyzw
    r2.xyzw = ((projection[0].xyzw)*(v8.xxxx)+(r2.xyzw)).xyzw;
    // 9: mad r2.xyzw, cb1[2].xyzw, v8.zzzz, r2.xyzw
    r2.xyzw = ((projection[2].xyzw)*(v8.zzzz)+(r2.xyzw)).xyzw;
    // 10: mad r2.xyzw, cb1[3].xyzw, v8.wwww, r2.xyzw
    r2.xyzw = ((projection[3].xyzw)*(v8.wwww)+(r2.xyzw)).xyzw;
    // 11: mul r3.xyzw, r2.yyyy, cb0[20].xyzw
    r3.xyzw = ((r2.yyyy)*(source[20].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[19].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[19].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[21].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[21].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[22].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[22].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s3
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[23].wwzw
    r4.yz = (source[23].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s3
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s3
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[23].zwzz
    r4.xy = ((r2.xyxx)+(source[23].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s3
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[23].xyxx
    r2.xy = ((r2.xyxx)*(source[23].xyxx)).xy;
    // 27: frc r2.xy, r2.xyxx
    r2.xy = (frac(r2.xyxx)).xy;
    // 28: movc r2.zw, r3.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r2.zw = ((asuint(r3.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 29: add r2.zw, r2.zzzw, r4.zzzw
    r2.zw = ((r2.zzzw)+(r4.zzzw)).zw;
    // 30: mad r2.xz, r2.xxxx, r2.zzwz, r4.xxyx
    r2.xz = ((r2.xxxx)*(r2.zzwz)+(r4.xxyx)).xz;
    // 31: add r0.w, -r2.x, r2.z
    r0.w = ((-(r2.xxxx))+(r2.zzzz)).w;
    // 32: mad r0.w, r2.y, r0.w, r2.x
    r0.w = ((r2.yyyy)*(r0.wwww)+(r2.xxxx)).w;
    // 33: mul r2.xyz, r0.wwww, cb0[24].xxxx
    r2.xyz = ((r0.wwww)*(source[24].xxxx)).xyz;
    // 34: else
    } else {
    // 35: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 36: endif
    }
    // 37: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 38: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 39: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 40: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 41: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 42: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 43: add r5.xyz, -cb0[2].xyzx, cb0[3].xyzx
    r5.xyz = ((-(source[2].xyzx))+(source[3].xyzx)).xyz;
    // 44: mul r5.xyz, r5.xyzx, cb0[13].xxxx
    r5.xyz = ((r5.xyzx)*(source[13].xxxx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r5.xyz, r6.yyyy, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r6.yyyy)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 47: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 48: add r7.xyz, -r5.xyzx, r1.wwww
    r7.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 49: mad r5.xyz, cb0[13].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[13].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 50: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: add r7.xyz, -r5.xyzx, r1.wwww
    r7.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 52: mad r5.xyz, cb0[13].zzzz, r7.xyzx, r5.xyzx
    r5.xyz = ((source[13].zzzz)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 53: mad r7.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mad r8.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 55: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 56: mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 57: mul r9.xyz, r6.xxxx, r8.xyzx
    r9.xyz = ((r6.xxxx)*(r8.xyzx)).xyz;
    // 58: mad r8.xyz, r6.xxxx, r8.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = ((r6.xxxx)*(r8.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 59: dp3 r1.w, r8.xyzx, r8.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 60: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 61: div r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)/(r1.wwww)).xyz;
    // 62: mul r8.xyz, r8.xyzx, cb0[7].xyzx
    r8.xyz = ((r8.xyzx)*(source[7].xyzx)).xyz;
    // 63: add r1.w, -|r3.z|, l(1.000000)
    r1.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t1.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 65: mad r10.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 66: dp2 r2.w, r10.xyxx, r10.xyxx
    r2.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 67: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 68: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 69: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 70: add r10.z, r2.w, l(0.000010)
    r10.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 71: dp3 r2.w, r10.xyzx, r10.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 72: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 73: div r10.xyz, r10.xyzx, r2.wwww
    r10.xyz = ((r10.xyzx)/(r2.wwww)).xyz;
    // 74: dp3 r2.w, r10.xyzx, r3.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 75: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 77: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 78: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 79: mul r1.w, r1.w, cb0[15].x
    r1.w = ((r1.wwww)*(source[15].xxxx)).w;
    // 80: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 81: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 82: mul r2.w, cb0[8].z, l(1.500000)
    r2.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 83: add r4.w, -cb0[8].w, l(1.000000)
    r4.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul r4.w, r4.w, cb0[15].y
    r4.w = ((r4.wwww)*(source[15].yyyy)).w;
    // 85: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 86: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 87: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 89: mad r2.w, r2.w, l(0.500000), cb0[8].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 90: frc r4.w, cb0[8].x
    r4.w = (frac(source[8].xxxx)).w;
    // 91: add r5.w, -r4.w, cb0[8].x
    r5.w = ((-(r4.wwww))+(source[8].xxxx)).w;
    // 92: mul r11.z, r5.w, l(0.125000)
    r11.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 93: mov r11.xw, l(0,0,0,0)
    r11.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 94: mul r11.y, cb0[8].y, cb0[9].y
    r11.y = ((source[8].yyyy)*(source[9].yyyy)).y;
    // 95: mul r12.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r12.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 96: frc r5.w, r12.x
    r5.w = (frac(r12.xxxx)).w;
    // 97: mul r12.y, r5.w, l(0.125000)
    r12.y = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 98: add r6.xy, r11.xyxx, r12.yzyy
    r6.xy = ((r11.xyxx)+(r12.yzyy)).xy;
    // 99: add r6.xy, r6.xyxx, r11.zwzz
    r6.xy = ((r6.xyxx)+(r11.zwzz)).xy;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: mul r11.xyz, r2.wwww, r11.xyzx
    r11.xyz = ((r2.wwww)*(r11.xyzx)).xyz;
    // 102: mul r2.w, r4.w, r11.w
    r2.w = ((r4.wwww)*(r11.wwww)).w;
    // 103: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 104: mad r9.xyz, r2.wwww, r11.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 105: mul r2.w, cb0[10].y, cb0[15].y
    r2.w = ((source[10].yyyy)*(source[15].yyyy)).w;
    // 106: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 107: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 108: mul r6.y, r2.w, l(0.020000)
    r6.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 109: add r11.xyzw, r1.yzxy, -cb0[1].yzxy
    r11.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 110: add r11.xy, -r11.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r11.xy = ((-(r11.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 111: add r11.xy, -r11.zwzz, r11.xyxx
    r11.xy = ((-(r11.zwzz))+(r11.xyxx)).xy;
    // 112: mad r11.xy, cb0[10].wwww, r11.xyxx, r11.zwzz
    r11.xy = ((source[10].wwww)*(r11.xyxx)+(r11.zwzz)).xy;
    // 113: mul r4.w, cb0[10].x, l(0.001000)
    r4.w = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 114: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 115: mad r6.xy, r4.wwww, r11.xyxx, r6.xyxx
    r6.xy = ((r4.wwww)*(r11.xyxx)+(r6.xyxx)).xy;
    // 116: dp2 r4.w, cb0[11].xyxx, r6.xyxx
    r4.w = (dot((source[11].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 117: dp2 r6.y, cb0[12].xyxx, r6.xyxx
    r6.y = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 118: frc r4.w, r4.w
    r4.w = (frac(r4.wwww)).w;
    // 119: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 120: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 121: mul r4.w, r11.w, l(0.900000)
    r4.w = ((r11.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 122: mad r11.xyz, r11.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r9.xyzx))).xyz;
    // 123: mad r11.xyz, r4.wwww, r11.xyzx, r9.xyzx
    r11.xyz = ((r4.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 124: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 126: mul_sat r11.xyz, r11.xyzx, r2.wwww
    r11.xyz = (saturate((r11.xyzx)*(r2.wwww))).xyz;
    // 127: mul r12.xyz, r11.xyzx, cb0[10].zzzz
    r12.xyz = ((r11.xyzx)*(source[10].zzzz)).xyz;
    // 128: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: mul r2.w, r2.w, l(3.000000)
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 130: mad r11.xyz, cb0[10].zzzz, r11.xyzx, -r9.xyzx
    r11.xyz = ((source[10].zzzz)*(r11.xyzx)+(-(r9.xyzx))).xyz;
    // 131: mad r9.xyz, r2.wwww, r11.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 132: dp3 r2.w, r10.xyzx, r4.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 133: max r4.w, r2.w, l(0.000000)
    r4.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r11.xyz, r2.xyzx, r5.wwww
    r11.xyz = ((r2.xyzx)*(r5.wwww)).xyz;
    // 136: mad r12.xyz, -r5.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r5.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 137: mad r11.yzw, cb0[16].zzzz, r12.xxyz, r11.xxyz
    r11.yzw = ((source[16].zzzz)*(r12.xxyz)+(r11.xxyz)).yzw;
    // 138: dp3 r5.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 139: add r12.xyz, r5.wwww, -cb0[2].xyzx
    r12.xyz = ((r5.wwww)+(-(source[2].xyzx))).xyz;
    // 140: mad r12.xyz, cb0[13].yyyy, r12.xyzx, cb0[2].xyzx
    r12.xyz = ((source[13].yyyy)*(r12.xyzx)+(source[2].xyzx)).xyz;
    // 141: dp3 r5.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r13.xyz, -r12.xyzx, r5.wwww
    r13.xyz = ((-(r12.xyzx))+(r5.wwww)).xyz;
    // 143: mad r12.xyz, cb0[13].zzzz, r13.xyzx, r12.xyzx
    r12.xyz = ((source[13].zzzz)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 144: mul r13.xyz, r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r13.xyz = ((r12.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 145: mad r14.xyz, -r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 146: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 148: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 149: mul r13.xyz, r13.xyzx, r2.wwww
    r13.xyz = ((r13.xyzx)*(r2.wwww)).xyz;
    // 150: mad r11.yzw, r11.yyzw, r14.xxyz, r13.xxyz
    r11.yzw = ((r11.yyzw)*(r14.xxyz)+(r13.xxyz)).yzw;
    // 151: mov_sat r2.w, r4.z
    r2.w = (saturate(r4.zzzz)).w;
    // 152: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 153: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 154: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: add r13.xyz, -r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r12.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 156: mad r12.xyz, r2.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r2.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 157: mad r12.xyz, r11.yzwy, l(0.500000, 0.500000, 0.500000, 0.000000), r12.xyzx
    r12.xyz = ((r11.yzwy)*(float4(0.500000,0.500000,0.500000,0.000000))+(r12.xyzx)).xyz;
    // 158: mul_sat r9.xyz, r9.xyzx, r12.xyzx
    r9.xyz = (saturate((r9.xyzx)*(r12.xyzx))).xyz;
    // 159: max r2.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 160: mad r5.xyz, r5.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r5.xyz = ((r5.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 161: dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 162: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 163: div r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)/(r2.wwww)).xyz;
    // 164: mul r5.xyz, r5.xyzx, r6.zzzz
    r5.xyz = ((r5.xyzx)*(r6.zzzz)).xyz;
    // 165: mov_sat r2.w, r3.z
    r2.w = (saturate(r3.zzzz)).w;
    // 166: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 167: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 168: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 169: mul r5.xyz, r5.xyzx, cb0[13].wwww
    r5.xyz = ((r5.xyzx)*(source[13].wwww)).xyz;
    // 170: max r2.w, r11.x, l(0.500000)
    r2.w = (max(r11.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 171: min r2.xyzw, r2.xyzw, l(1.000000, 1.000000, 1.000000, 1.000000)
    r2.xyzw = (min(r2.xyzw,float4(1.000000,1.000000,1.000000,1.000000))).xyzw;
    // 172: dp3 r5.w, r0.xyzx, r10.xyzx
    r5.w = (dot((r0.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 173: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 174: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 175: mul r3.w, r3.z, r1.z
    r3.w = ((r3.zzzz)*(r1.zzzz)).w;
    // 176: mad r1.xyz, r3.zzwz, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r3.zzwz)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 177: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 178: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 179: div r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)/(r0.yyyy)).y;
    // 180: add r0.y, r0.y, cb0[6].z
    r0.y = ((r0.yyyy)+(source[6].zzzz)).y;
    // 181: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 182: add r0.x, r0.x, r5.w
    r0.x = ((r0.xxxx)+(r5.wwww)).x;
    // 183: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 184: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 185: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 186: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 187: mad r0.y, cb0[14].z, l(4.500000), l(0.500000)
    r0.y = ((source[14].zzzz)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 188: mul r0.y, r0.y, cb0[16].w
    r0.y = ((r0.yyyy)*(source[16].wwww)).y;
    // 189: mul r0.y, r0.y, l(0.050000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 190: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 191: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 192: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 193: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 194: mul r0.x, r2.w, r0.x
    r0.x = ((r2.wwww)*(r0.xxxx)).x;
    // 195: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 196: mad r0.yzw, v5.xxyz, r0.wwww, r3.xxyz
    r0.yzw = ((v5.xxyz)*(r0.wwww)+(r3.xxyz)).yzw;
    // 197: dp3 r0.y, r0.yzwy, r0.yzwy
    r0.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 198: mad r0.y, r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 199: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 200: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 201: mul r0.x, r0.x, cb0[17].x
    r0.x = ((r0.xxxx)*(source[17].xxxx)).x;
    // 202: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 203: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 204: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 205: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 206: mad r0.xyz, r9.xyzx, r11.yzwy, r0.xyzx
    r0.xyz = ((r9.xyzx)*(r11.yzwy)+(r0.xyzx)).xyz;
    // 207: mad r0.xyz, r1.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 208: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 209: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 210: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 211: mul o0.w, r6.w, cb0[1].w
    output.targets[0].w = ((r6.wwww)*(source[1].wwww)).w;
    // 212: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 213: ret
    return output;
}

// source.character.equipment-native-193.v1 / source program 40b7e1a56e64204faaf8a7f75ede7b9f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight193(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].z=(g_SourceCharacterTime.xxxx).x;
    source[24]=float4(input.lightColor,1.0);
    source[25].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[25].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[25].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t8.xyzw, s0
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.xyzw, s7, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r9.xyzw, r8.xyzw, cb0[15].xyzw
    r9.xyzw = ((r8.xyzw)*(source[15].xyzw)).xyzw;
    // 32: add r9.xy, r9.ywyy, r9.xzxx
    r9.xy = ((r9.ywyy)+(r9.xzxx)).xy;
    // 33: add r1.w, r9.y, r9.x
    r1.w = ((r9.yyyy)+(r9.xxxx)).w;
    // 34: add r8.xy, r8.ywyy, r8.xzxx
    r8.xy = ((r8.ywyy)+(r8.xzxx)).xy;
    // 35: add r2.w, r8.y, r8.x
    r2.w = ((r8.yyyy)+(r8.xxxx)).w;
    // 36: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 37: mad_sat r1.w, r2.w, r1.w, l(1.000000)
    r1.w = (saturate((r2.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul_sat r1.w, r1.w, r8.w
    r1.w = (saturate((r1.wwww)*(r8.wwww))).w;
    // 40: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 41: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 42: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 44: mad r9.xy, r9.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((r9.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: mul r10.xy, r9.xyxx, cb0[16].xxxx
    r10.xy = ((r9.xyxx)*(source[16].xxxx)).xy;
    // 46: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 47: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 49: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 50: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 51: add r9.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 52: mad r9.xyz, cb0[16].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[16].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 53: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 54: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 55: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 56: dp3 r11.x, r1.xyzx, r9.xyzx
    r11.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 57: dp3 r11.y, r2.xyzx, r9.xyzx
    r11.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 58: dp3 r11.z, r0.xyzx, r9.xyzx
    r11.z = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 59: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 60: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 61: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 62: dp3 r0.x, r11.xyzx, r1.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 63: mul r0.xyz, r11.xyzx, r0.xxxx
    r0.xyz = ((r11.xyzx)*(r0.xxxx)).xyz;
    // 64: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 65: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 66: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 67: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 68: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 69: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 70: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 72: add r0.z, -cb0[17].y, cb0[17].x
    r0.z = ((-(source[17].yyyy))+(source[17].xxxx)).z;
    // 73: mad r0.z, r2.x, r0.z, cb0[17].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[17].yyyy)).z;
    // 74: add r1.x, -r0.z, cb0[17].z
    r1.x = ((-(r0.zzzz))+(source[17].zzzz)).x;
    // 75: mad r0.z, r2.y, r1.x, r0.z
    r0.z = ((r2.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 77: add r1.x, -r9.w, l(1.000000)
    r1.x = ((-(r9.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 78: add r1.y, cb0[17].w, -cb0[18].x
    r1.y = ((source[17].wwww)+(-(source[18].xxxx))).y;
    // 79: mad r1.y, r2.x, r1.y, cb0[18].x
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[18].xxxx)).y;
    // 80: add r1.w, -r1.y, cb0[18].y
    r1.w = ((-(r1.yyyy))+(source[18].yyyy)).w;
    // 81: mad r1.y, r2.y, r1.w, r1.y
    r1.y = ((r2.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 82: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 83: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 84: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 85: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 86: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 87: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 88: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 89: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 90: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 91: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 92: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 93: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 94: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 95: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 96: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 97: max r1.y, r1.y, r4.x
    r1.y = (max(r1.yyyy,r4.xxxx)).y;
    // 98: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 99: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 100: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 101: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 102: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 103: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 104: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 105: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 106: rcp r1.y, cb0[18].z
    r1.y = (1.0/(source[18].zzzz)).y;
    // 107: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 108: mul r11.xyz, r4.xyzx, cb0[18].zzzz
    r11.xyz = ((r4.xyzx)*(source[18].zzzz)).xyz;
    // 109: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 110: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 111: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 112: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 113: mad r4.xyz, r11.xyzx, cb0[18].zzzz, r4.xyzx
    r4.xyz = ((r11.xyzx)*(source[18].zzzz)+(r4.xyzx)).xyz;
    // 114: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 115: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 116: add r1.y, cb0[18].z, l(1.000000)
    r1.y = ((source[18].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 117: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 118: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 119: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 120: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 121: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 122: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 123: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 124: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 125: mul r4.w, |r1.y|, |r1.y|
    r4.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 126: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 127: mul r1.y, |r1.y|, r4.w
    r1.y = ((abs(r1.yyyy))*(r4.wwww)).y;
    // 128: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 129: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 130: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 131: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 132: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 133: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 134: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 136: mul r11.xyz, r0.xyzx, r1.yyyy
    r11.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 137: mul r12.xyz, r11.xyzx, cb0[22].yyyy
    r12.xyz = ((r11.xyzx)*(source[22].yyyy)).xyz;
    // 138: dp3 r4.w, r10.xyzx, r10.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 139: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 140: div r10.xyz, r10.xyzx, r4.wwww
    r10.xyz = ((r10.xyzx)/(r4.wwww)).xyz;
    // 141: dp3 r4.w, r10.xyzx, r7.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 142: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 143: min r6.w, r5.w, l(1.000000)
    r6.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mad r4.w, r4.w, l(0.500000), -r6.w
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r6.wwww))).w;
    // 146: mad r4.w, r1.x, r4.w, r6.w
    r4.w = ((r1.xxxx)*(r4.wwww)+(r6.wwww)).w;
    // 147: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 149: mul_sat r6.xy, r6.xzxx, cb0[19].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[19].yyyy))).xy;
    // 150: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 151: add_sat r6.y, r6.y, -cb0[19].z
    r6.y = (saturate((r6.yyyy)+(-(source[19].zzzz)))).y;
    // 152: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 153: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 154: mul r6.y, r6.y, cb0[19].w
    r6.y = ((r6.yyyy)*(source[19].wwww)).y;
    // 155: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 156: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 157: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 158: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 159: mul r6.y, r4.w, r6.y
    r6.y = ((r4.wwww)*(r6.yyyy)).y;
    // 160: mad r6.z, r0.w, l(2.000000), -r1.y
    r6.z = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).z;
    // 161: mad r6.y, r6.y, r6.z, r1.y
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r1.yyyy)).y;
    // 162: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 163: mul_sat r6.y, r4.w, r6.y
    r6.y = (saturate((r4.wwww)*(r6.yyyy))).y;
    // 164: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 165: mul r1.x, r1.x, cb0[22].w
    r1.x = ((r1.xxxx)*(source[22].wwww)).x;
    // 166: mad r6.y, cb0[22].z, r6.y, -r4.w
    r6.y = ((source[22].zzzz)*(r6.yyyy)+(-(r4.wwww))).y;
    // 167: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 168: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 169: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 170: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 171: mad r7.xyz, -cb0[22].yyyy, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[22].yyyy))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mad r7.xyz, r5.xyzx, r7.xyzx, r12.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 173: mul r11.xyz, cb0[3].xyzx, cb0[3].wwww
    r11.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 174: mad r12.xyz, cb0[4].wwww, cb0[4].xyzx, -r11.xyzx
    r12.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r11.xyzx))).xyz;
    // 175: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 176: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 177: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 178: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 179: mad r11.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 180: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 181: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 182: mad r11.xyz, cb0[16].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[16].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 183: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 184: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 185: mad r11.xyz, cb0[16].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[16].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 186: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 189: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 190: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 191: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 192: mad r8.xyz, cb0[16].yyyy, r13.xyzx, r8.xyzx
    r8.xyz = ((source[16].yyyy)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 193: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 194: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 195: mad r8.xyz, cb0[16].zzzz, r13.xyzx, r8.xyzx
    r8.xyz = ((source[16].zzzz)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 196: mul r13.xyz, r8.xyzx, r11.xyzx
    r13.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 197: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 198: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 199: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 200: add r14.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r14.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 201: mad r14.xyz, r0.yyyy, r14.xyzx, cb0[9].xyzx
    r14.xyz = ((r0.yyyy)*(r14.xyzx)+(source[9].xyzx)).xyz;
    // 202: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 203: mul r0.xyz, r0.xyzx, cb0[18].wwww
    r0.xyz = ((r0.xyzx)*(source[18].wwww)).xyz;
    // 204: mul r14.xyz, r0.xyzx, r13.xyzx
    r14.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 205: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 206: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 207: mad r9.xyz, cb0[16].yyyy, r15.xyzx, r9.xyzx
    r9.xyz = ((source[16].yyyy)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 208: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 209: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 210: mad r9.xyz, cb0[16].zzzz, r15.xyzx, r9.xyzx
    r9.xyz = ((source[16].zzzz)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 211: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 212: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 213: mul r15.xyz, r15.xyzx, cb0[19].xxxx
    r15.xyz = ((r15.xyzx)*(source[19].xxxx)).xyz;
    // 214: add r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 215: add r1.x, r2.z, r1.x
    r1.x = ((r2.zzzz)+(r1.xxxx)).x;
    // 216: add_sat r1.x, r2.w, r1.x
    r1.x = (saturate((r2.wwww)+(r1.xxxx))).x;
    // 217: mad r2.xyz, r1.xxxx, r15.xyzx, r9.xyzx
    r2.xyz = ((r1.xxxx)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 218: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 219: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 220: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 221: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 222: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 223: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 224: mul r1.x, r1.x, cb0[20].x
    r1.x = ((r1.xxxx)*(source[20].xxxx)).x;
    // 225: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 226: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 227: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 228: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 229: div r1.z, cb0[20].y, r1.z
    r1.z = ((source[20].yyyy)/(r1.zzzz)).z;
    // 230: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 231: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 232: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 233: mul r1.z, r1.z, cb0[20].z
    r1.z = ((r1.zzzz)*(source[20].zzzz)).z;
    // 234: mad r0.xyz, r2.xyzx, r0.xyzx, -r14.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r14.xyzx))).xyz;
    // 235: mad r0.xyz, r1.zzzz, r0.xyzx, r14.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r14.xyzx)).xyz;
    // 236: mad r0.xyz, r1.yyyy, r0.xyzx, -r13.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 237: mad r0.xyz, r1.xxxx, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 238: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 239: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 240: mad r0.xyz, cb0[16].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[16].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 241: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 242: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 243: mad r0.xyz, cb0[16].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[16].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 244: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 245: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 246: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 247: mul r2.w, r2.w, cb0[21].z
    r2.w = ((r2.wwww)*(source[21].zzzz)).w;
    // 248: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 249: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 250: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 251: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 252: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 253: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 254: add r4.w, -r2.w, cb0[2].x
    r4.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 255: mul r9.z, r4.w, l(0.125000)
    r9.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 256: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 257: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 258: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 259: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 260: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 261: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 262: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 263: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 264: mul r6.xyz, r1.zzzz, r9.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xyzx)).xyz;
    // 265: mul r1.z, r2.w, r9.w
    r1.z = ((r2.wwww)*(r9.wwww)).z;
    // 266: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 267: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 268: mul r1.z, cb0[12].y, cb0[21].z
    r1.z = ((source[12].yyyy)*(source[21].zzzz)).z;
    // 269: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 270: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 271: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 272: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 273: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 274: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 275: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 276: mul r2.w, cb0[12].x, l(0.001000)
    r2.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 277: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 278: mad r3.xy, r2.wwww, r3.xyxx, r6.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r6.xyxx)).xy;
    // 279: dp2 r2.w, cb0[13].xyxx, r3.xyxx
    r2.w = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 280: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 281: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 282: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 283: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 284: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 285: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 286: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 287: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 288: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 289: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 290: mul r6.xyz, r3.xyzx, cb0[12].zzzz
    r6.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 291: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 292: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 293: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 294: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 295: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 296: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 297: mad r5.xyz, r11.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r11.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 298: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 299: dp3 r4.x, r4.xyzx, r10.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 300: mul r4.y, r0.w, cb0[23].x
    r4.y = ((r0.wwww)*(source[23].xxxx)).y;
    // 301: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t7.xyzw, s8, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 302: add r0.w, -cb0[23].y, l(2.000000)
    r0.w = ((-(source[23].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 303: mad r0.w, r1.y, r0.w, cb0[23].y
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[23].yyyy)).w;
    // 304: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 305: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 306: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 307: mul r1.xyz, r1.xyzx, cb0[23].zzzz
    r1.xyz = ((r1.xyzx)*(source[23].zzzz)).xyz;
    // 308: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 309: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 310: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 311: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 312: mul r1.xyz, r1.xyzx, cb0[23].wwww
    r1.xyz = ((r1.xyzx)*(source[23].wwww)).xyz;
    // 313: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 314: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 315: mad r0.xyz, r7.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 316: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 317: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 318: mul o0.xyz, r0.xyzx, cb0[24].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[24].xyzx)).xyz;
    // 319: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 320: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 321: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 322: ret
    return output;
}

// source.character.equipment-native-194.v1 / source program 1901889a33a1cf4884bfda64ebdfcd5f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight194(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].z=(g_SourceCharacterTime.xxxx).x;
    source[23]=float4(input.lightColor,1.0);
    source[24].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[24].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[24].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 37: mul r10.xy, r9.xyxx, cb0[15].xxxx
    r10.xy = ((r9.xyxx)*(source[15].xxxx)).xy;
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
    // 44: mad r9.xyz, cb0[15].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[15].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: add r0.z, -cb0[16].y, cb0[16].x
    r0.z = ((-(source[16].yyyy))+(source[16].xxxx)).z;
    // 65: mad r0.z, r2.x, r0.z, cb0[16].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[16].yyyy)).z;
    // 66: add r1.x, -r0.z, cb0[16].z
    r1.x = ((-(r0.zzzz))+(source[16].zzzz)).x;
    // 67: mad r0.z, r2.y, r1.x, r0.z
    r0.z = ((r2.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 69: add r1.x, -r9.w, l(1.000000)
    r1.x = ((-(r9.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 70: add r1.y, cb0[16].w, -cb0[17].x
    r1.y = ((source[16].wwww)+(-(source[17].xxxx))).y;
    // 71: mad r1.y, r2.x, r1.y, cb0[17].x
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[17].xxxx)).y;
    // 72: add r1.w, -r1.y, cb0[17].y
    r1.w = ((-(r1.yyyy))+(source[17].yyyy)).w;
    // 73: mad r1.y, r2.y, r1.w, r1.y
    r1.y = ((r2.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 74: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 75: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 76: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 77: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 78: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 79: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 80: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 81: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 82: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 83: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 84: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 85: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 86: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 87: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 88: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 89: max r1.y, r1.y, r4.x
    r1.y = (max(r1.yyyy,r4.xxxx)).y;
    // 90: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 91: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 92: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 93: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 94: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 95: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 96: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 97: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 98: rcp r1.y, cb0[17].z
    r1.y = (1.0/(source[17].zzzz)).y;
    // 99: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 100: mul r11.xyz, r4.xyzx, cb0[17].zzzz
    r11.xyz = ((r4.xyzx)*(source[17].zzzz)).xyz;
    // 101: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 102: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 103: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 104: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 105: mad r4.xyz, r11.xyzx, cb0[17].zzzz, r4.xyzx
    r4.xyz = ((r11.xyzx)*(source[17].zzzz)+(r4.xyzx)).xyz;
    // 106: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 107: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 108: add r1.y, cb0[17].z, l(1.000000)
    r1.y = ((source[17].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 109: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 110: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 111: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 112: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 113: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 114: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 115: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 116: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 117: mul r4.w, |r1.y|, |r1.y|
    r4.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 118: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 119: mul r1.y, |r1.y|, r4.w
    r1.y = ((abs(r1.yyyy))*(r4.wwww)).y;
    // 120: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 121: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 122: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 123: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 124: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 125: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 126: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 128: mul r11.xyz, r0.xyzx, r1.yyyy
    r11.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 129: mul r12.xyz, r11.xyzx, cb0[21].yyyy
    r12.xyz = ((r11.xyzx)*(source[21].yyyy)).xyz;
    // 130: dp3 r4.w, r10.xyzx, r10.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 131: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 132: div r10.xyz, r10.xyzx, r4.wwww
    r10.xyz = ((r10.xyzx)/(r4.wwww)).xyz;
    // 133: dp3 r4.w, r10.xyzx, r7.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 134: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 135: min r6.w, r5.w, l(1.000000)
    r6.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: mad r4.w, r4.w, l(0.500000), -r6.w
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r6.wwww))).w;
    // 138: mad r4.w, r1.x, r4.w, r6.w
    r4.w = ((r1.xxxx)*(r4.wwww)+(r6.wwww)).w;
    // 139: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 141: mul_sat r6.xy, r6.xzxx, cb0[18].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[18].yyyy))).xy;
    // 142: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 143: add_sat r6.y, r6.y, -cb0[18].z
    r6.y = (saturate((r6.yyyy)+(-(source[18].zzzz)))).y;
    // 144: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 145: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 146: mul r6.y, r6.y, cb0[18].w
    r6.y = ((r6.yyyy)*(source[18].wwww)).y;
    // 147: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 148: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 149: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 150: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 151: mul r6.y, r4.w, r6.y
    r6.y = ((r4.wwww)*(r6.yyyy)).y;
    // 152: mad r6.z, r0.w, l(2.000000), -r1.y
    r6.z = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).z;
    // 153: mad r6.y, r6.y, r6.z, r1.y
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r1.yyyy)).y;
    // 154: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 155: mul_sat r6.y, r4.w, r6.y
    r6.y = (saturate((r4.wwww)*(r6.yyyy))).y;
    // 156: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 157: mul r1.x, r1.x, cb0[21].w
    r1.x = ((r1.xxxx)*(source[21].wwww)).x;
    // 158: mad r6.y, cb0[21].z, r6.y, -r4.w
    r6.y = ((source[21].zzzz)*(r6.yyyy)+(-(r4.wwww))).y;
    // 159: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 160: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 161: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 162: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 163: mad r7.xyz, -cb0[21].yyyy, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[21].yyyy))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 164: mad r7.xyz, r5.xyzx, r7.xyzx, r12.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 165: mul r11.xyz, cb0[3].xyzx, cb0[3].wwww
    r11.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 166: mad r12.xyz, cb0[4].wwww, cb0[4].xyzx, -r11.xyzx
    r12.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r11.xyzx))).xyz;
    // 167: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 168: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 169: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 170: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 171: mad r11.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 172: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 173: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 174: mad r11.xyz, cb0[15].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[15].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 175: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 176: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 177: mad r11.xyz, cb0[15].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[15].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 178: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 181: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 182: dp3 r1.x, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 183: add r13.xyz, -r8.yzwy, r1.xxxx
    r13.xyz = ((-(r8.yzwy))+(r1.xxxx)).xyz;
    // 184: mad r8.xyz, cb0[15].yyyy, r13.xyzx, r8.yzwy
    r8.xyz = ((source[15].yyyy)*(r13.xyzx)+(r8.yzwy)).xyz;
    // 185: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 186: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 187: mad r8.xyz, cb0[15].zzzz, r13.xyzx, r8.xyzx
    r8.xyz = ((source[15].zzzz)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 188: mul r13.xyz, r8.xyzx, r11.xyzx
    r13.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 189: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 190: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 191: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 192: add r14.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r14.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 193: mad r14.xyz, r0.yyyy, r14.xyzx, cb0[9].xyzx
    r14.xyz = ((r0.yyyy)*(r14.xyzx)+(source[9].xyzx)).xyz;
    // 194: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 195: mul r0.xyz, r0.xyzx, cb0[17].wwww
    r0.xyz = ((r0.xyzx)*(source[17].wwww)).xyz;
    // 196: mul r14.xyz, r0.xyzx, r13.xyzx
    r14.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 197: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 198: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 199: mad r9.xyz, cb0[15].yyyy, r15.xyzx, r9.xyzx
    r9.xyz = ((source[15].yyyy)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 200: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 201: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 202: mad r9.xyz, cb0[15].zzzz, r15.xyzx, r9.xyzx
    r9.xyz = ((source[15].zzzz)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 203: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 204: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 205: mul r15.xyz, r15.xyzx, cb0[18].xxxx
    r15.xyz = ((r15.xyzx)*(source[18].xxxx)).xyz;
    // 206: add r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 207: add r1.x, r2.z, r1.x
    r1.x = ((r2.zzzz)+(r1.xxxx)).x;
    // 208: add_sat r1.x, r2.w, r1.x
    r1.x = (saturate((r2.wwww)+(r1.xxxx))).x;
    // 209: mad r2.xyz, r1.xxxx, r15.xyzx, r9.xyzx
    r2.xyz = ((r1.xxxx)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 210: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 211: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 212: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 213: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 214: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 215: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 216: mul r1.x, r1.x, cb0[19].x
    r1.x = ((r1.xxxx)*(source[19].xxxx)).x;
    // 217: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 218: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 219: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 220: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 221: div r1.z, cb0[19].y, r1.z
    r1.z = ((source[19].yyyy)/(r1.zzzz)).z;
    // 222: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 223: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 224: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 225: mul r1.z, r1.z, cb0[19].z
    r1.z = ((r1.zzzz)*(source[19].zzzz)).z;
    // 226: mad r0.xyz, r2.xyzx, r0.xyzx, -r14.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r14.xyzx))).xyz;
    // 227: mad r0.xyz, r1.zzzz, r0.xyzx, r14.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r14.xyzx)).xyz;
    // 228: mad r0.xyz, r1.yyyy, r0.xyzx, -r13.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 229: mad r0.xyz, r1.xxxx, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 230: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 231: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 232: mad r0.xyz, cb0[15].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[15].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 233: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 234: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 235: mad r0.xyz, cb0[15].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[15].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 236: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 237: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 238: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 240: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 241: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 242: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 243: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 244: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 245: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 246: add r4.w, -r2.w, cb0[2].x
    r4.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 247: mul r9.z, r4.w, l(0.125000)
    r9.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 248: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 249: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 250: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 251: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 252: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 253: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 254: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 255: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 256: mul r6.xyz, r1.zzzz, r9.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xyzx)).xyz;
    // 257: mul r1.z, r2.w, r9.w
    r1.z = ((r2.wwww)*(r9.wwww)).z;
    // 258: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 259: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 260: mul r1.z, cb0[12].y, cb0[20].z
    r1.z = ((source[12].yyyy)*(source[20].zzzz)).z;
    // 261: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 262: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 263: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 264: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 265: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 266: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 267: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 268: mul r2.w, cb0[12].x, l(0.001000)
    r2.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 269: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 270: mad r3.xy, r2.wwww, r3.xyxx, r6.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r6.xyxx)).xy;
    // 271: dp2 r2.w, cb0[13].xyxx, r3.xyxx
    r2.w = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 272: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 273: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 274: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 275: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 276: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 277: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 278: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 279: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 280: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 281: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 282: mul r6.xyz, r3.xyzx, cb0[12].zzzz
    r6.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 283: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 284: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 285: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 286: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 287: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 288: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 289: mad r5.xyz, r11.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r11.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 290: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 291: dp3 r4.x, r4.xyzx, r10.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 292: mul r4.y, r0.w, cb0[22].x
    r4.y = ((r0.wwww)*(source[22].xxxx)).y;
    // 293: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 294: add r0.w, -cb0[22].y, l(2.000000)
    r0.w = ((-(source[22].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 295: mad r0.w, r1.y, r0.w, cb0[22].y
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[22].yyyy)).w;
    // 296: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 297: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 298: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 299: mul r1.xyz, r1.xyzx, cb0[22].zzzz
    r1.xyz = ((r1.xyzx)*(source[22].zzzz)).xyz;
    // 300: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 301: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 302: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 303: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 304: mul r1.xyz, r1.xyzx, cb0[22].wwww
    r1.xyz = ((r1.xyzx)*(source[22].wwww)).xyz;
    // 305: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 306: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 307: mad r0.xyz, r7.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 308: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 309: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 310: mul o0.xyz, r0.xyzx, cb0[23].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)).xyz;
    // 311: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 312: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 313: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 314: ret
    return output;
}

// source.character.equipment-native-195.v1 / source program 24cf0806ed9c54498ada190979c23a7c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight195(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12].z=(g_SourceCharacterTime.xxxx).x;
    source[18]=float4(input.lightColor,1.0);
    source[19].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].y=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
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
    // 9: mul r3.xy, r2.xyxx, cb0[11].xxxx
    r3.xy = ((r2.xyxx)*(source[11].xxxx)).xy;
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
    // 15: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 17: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 18: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 21: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[19].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[19].xxxx)) * 0xffffffffu)).w;
    // 22: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 23: div r3.xy, v8.xyxx, v8.wwww
    r3.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 24: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 25: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t5.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 26: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 27: else
    } else {
    // 28: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 29: endif
    }
    // 30: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: lt r6.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 33: log r5.xyz, |r5.xzyx|
    r5.xyz = (log2(abs(r5.xzyx))).xyz;
    // 34: mul r1.w, r5.x, cb0[14].x
    r1.w = ((r5.xxxx)*(source[14].xxxx)).w;
    // 35: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 36: movc r1.w, r6.x, l(0), r1.w
    r1.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 37: add_sat r1.w, r1.w, cb0[14].y
    r1.w = (saturate((r1.wwww)+(source[14].yyyy))).w;
    // 38: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r7.xyz, r2.wwww, cb0[9].xyzx
    r7.xyz = ((r2.wwww)*(source[9].xyzx)).xyz;
    // 40: add r2.w, r1.w, l(-1.000000)
    r2.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 41: mad r2.w, cb0[14].w, r2.w, l(1.000000)
    r2.w = ((source[14].wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: mul r8.xyz, cb0[2].xyzx, cb0[2].wwww
    r8.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 43: max r9.xyz, r8.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r8.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 44: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 45: max r8.xyz, r8.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r8.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 46: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 47: mul r3.w, r5.y, cb0[11].y
    r3.w = ((r5.yyyy)*(source[11].yyyy)).w;
    // 48: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 49: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: movc r3.w, r6.y, l(0), r3.w
    r3.w = ((asuint(r6.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 51: add r5.xyw, -r9.xyxz, r8.xyxz
    r5.xyw = ((-(r9.xyxz))+(r8.xyxz)).xyw;
    // 52: mad r5.xyw, r3.wwww, r5.xyxw, r9.xyxz
    r5.xyw = ((r3.wwww)*(r5.xyxw)+(r9.xyxz)).xyw;
    // 53: mul r6.xyw, cb0[3].xyxz, cb0[3].wwww
    r6.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // 54: max r8.xyz, r6.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r8.xyz = (max(r6.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 55: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 56: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 57: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 58: add r6.xyw, -r8.xyxz, r6.xyxw
    r6.xyw = ((-(r8.xyxz))+(r6.xyxw)).xyw;
    // 59: mad r6.xyw, r3.wwww, r6.xyxw, r8.xyxz
    r6.xyw = ((r3.wwww)*(r6.xyxw)+(r8.xyxz)).xyw;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, v4.xyxx, t2.xyzw, s4, l(0.000000)
    r8.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 61: add r6.xyw, -r5.xyxw, r6.xyxw
    r6.xyw = ((-(r5.xyxw))+(r6.xyxw)).xyw;
    // 62: mad r5.xyw, r8.xxxx, r6.xyxw, r5.xyxw
    r5.xyw = ((r8.xxxx)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 63: mul r6.xyw, cb0[4].xyxz, cb0[4].wwww
    r6.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 64: max r9.xyz, r6.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r6.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 65: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 66: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 67: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 68: add r6.xyw, -r9.xyxz, r6.xyxw
    r6.xyw = ((-(r9.xyxz))+(r6.xyxw)).xyw;
    // 69: mad r6.xyw, r3.wwww, r6.xyxw, r9.xyxz
    r6.xyw = ((r3.wwww)*(r6.xyxw)+(r9.xyxz)).xyw;
    // 70: add r6.xyw, -r5.xyxw, r6.xyxw
    r6.xyw = ((-(r5.xyxw))+(r6.xyxw)).xyw;
    // 71: mad r5.xyw, r8.yyyy, r6.xyxw, r5.xyxw
    r5.xyw = ((r8.yyyy)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 72: mul r6.xyw, cb0[5].xyxz, cb0[5].wwww
    r6.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 73: max r9.xyz, r6.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r6.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 74: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 75: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 76: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 77: add r6.xyw, -r9.xyxz, r6.xyxw
    r6.xyw = ((-(r9.xyxz))+(r6.xyxw)).xyw;
    // 78: mad r6.xyw, r3.wwww, r6.xyxw, r9.xyxz
    r6.xyw = ((r3.wwww)*(r6.xyxw)+(r9.xyxz)).xyw;
    // 79: add r6.xyw, -r5.xyxw, r6.xyxw
    r6.xyw = ((-(r5.xyxw))+(r6.xyxw)).xyw;
    // 80: mad r5.xyw, r8.zzzz, r6.xyxw, r5.xyxw
    r5.xyw = ((r8.zzzz)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 81: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 82: add r6.xyw, -r5.xyxw, r4.wwww
    r6.xyw = ((-(r5.xyxw))+(r4.wwww)).xyw;
    // 83: mad r5.xyw, cb0[12].wwww, r6.xyxw, r5.xyxw
    r5.xyw = ((source[12].wwww)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 84: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 85: add r6.xyw, -r5.xyxw, r4.wwww
    r6.xyw = ((-(r5.xyxw))+(r4.wwww)).xyw;
    // 86: mad r5.xyw, cb0[13].xxxx, r6.xyxw, r5.xyxw
    r5.xyw = ((source[13].xxxx)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 87: mad r6.xyw, cb0[7].wwww, cb0[7].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r6.xyw = ((source[7].wwww)*(source[7].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 88: mad r9.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 89: mul r6.xyw, r6.xyxw, r9.xyxz
    r6.xyw = ((r6.xyxw)*(r9.xyxz)).xyw;
    // 90: mul r5.xyw, r5.xyxw, r6.xyxw
    r5.xyw = ((r5.xyxw)*(r6.xyxw)).xyw;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.wxyz, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 92: dp3 r4.w, r9.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r10.xyz, -r9.yzwy, r4.wwww
    r10.xyz = ((-(r9.yzwy))+(r4.wwww)).xyz;
    // 94: mad r9.yzw, cb0[12].wwww, r10.xxyz, r9.yyzw
    r9.yzw = ((source[12].wwww)*(r10.xxyz)+(r9.yyzw)).yzw;
    // 95: dp3 r4.w, r9.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r10.xyz, -r9.yzwy, r4.wwww
    r10.xyz = ((-(r9.yzwy))+(r4.wwww)).xyz;
    // 97: mad r9.yzw, cb0[13].xxxx, r10.xxyz, r9.yyzw
    r9.yzw = ((source[13].xxxx)*(r10.xxyz)+(r9.yyzw)).yzw;
    // 98: mul r10.xyz, r5.xywx, r9.yzwy
    r10.xyz = ((r5.xywx)*(r9.yzwy)).xyz;
    // 99: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: mad r5.xyw, -r5.xyxw, r9.yzyw, r4.wwww
    r5.xyw = ((-(r5.xyxw))*(r9.yzyw)+(r4.wwww)).xyw;
    // 101: mad r5.xyw, cb0[12].wwww, r5.xyxw, r10.xyxz
    r5.xyw = ((source[12].wwww)*(r5.xyxw)+(r10.xyxz)).xyw;
    // 102: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r9.yzw, -r5.xxyw, r4.wwww
    r9.yzw = ((-(r5.xxyw))+(r4.wwww)).yzw;
    // 104: mad r5.xyw, cb0[13].xxxx, r9.yzyw, r5.xyxw
    r5.xyw = ((source[13].xxxx)*(r9.yzyw)+(r5.xyxw)).xyw;
    // 105: mul r5.xyw, r6.xyxw, r5.xyxw
    r5.xyw = ((r6.xyxw)*(r5.xyxw)).xyw;
    // 106: mul r4.w, cb0[6].z, l(1.500000)
    r4.w = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 107: add r6.x, -cb0[6].w, l(1.000000)
    r6.x = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r6.x, r6.x, cb0[12].z
    r6.x = ((r6.xxxx)*(source[12].zzzz)).x;
    // 109: mul r6.x, r6.x, l(6.283185)
    r6.x = ((r6.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 110: sincos r6.x, null, r6.x
    r6.x = (sin(r6.xxxx)).x;
    // 111: add r6.x, r6.x, l(1.000000)
    r6.x = ((r6.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: mul r4.w, r4.w, r6.x
    r4.w = ((r4.wwww)*(r6.xxxx)).w;
    // 113: mad r4.w, r4.w, l(0.500000), cb0[6].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 114: frc r6.x, cb0[6].x
    r6.x = (frac(source[6].xxxx)).x;
    // 115: add r6.y, -r6.x, cb0[6].x
    r6.y = ((-(r6.xxxx))+(source[6].xxxx)).y;
    // 116: mul r10.z, r6.y, l(0.125000)
    r10.z = ((r6.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 117: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 118: mul r10.y, cb0[6].y, cb0[10].y
    r10.y = ((source[6].yyyy)*(source[10].yyyy)).y;
    // 119: frc r6.y, v4.x
    r6.y = (frac(v4.xxxx)).y;
    // 120: mul r11.x, r6.y, l(0.125000)
    r11.x = ((r6.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 121: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 122: add r6.yw, r10.xxxy, r11.xxxy
    r6.yw = ((r10.xxxy)+(r11.xxxy)).yw;
    // 123: add r6.yw, r6.yyyw, r10.zzzw
    r6.yw = ((r6.yyyw)+(r10.zzzw)).yw;
    // 124: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.ywyy, t4.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 125: mul r9.yzw, r4.wwww, r10.xxyz
    r9.yzw = ((r4.wwww)*(r10.xxyz)).yzw;
    // 126: mul r4.w, r6.x, r10.w
    r4.w = ((r6.xxxx)*(r10.wwww)).w;
    // 127: mad r6.xyw, r9.yzyw, l(2.000000, 2.000000, 0.000000, 2.000000), -r5.xyxw
    r6.xyw = ((r9.yzyw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r5.xyxw))).xyw;
    // 128: mad r5.xyw, r4.wwww, r6.xyxw, r5.xyxw
    r5.xyw = ((r4.wwww)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 129: mul r6.xyw, r2.wwww, r5.xyxw
    r6.xyw = ((r2.wwww)*(r5.xyxw)).xyw;
    // 130: mul r6.xyw, r6.xyxw, r7.xyxz
    r6.xyw = ((r6.xyxw)*(r7.xyxz)).xyw;
    // 131: mad r5.xyw, r2.wwww, r5.xyxw, -r6.xyxw
    r5.xyw = ((r2.wwww)*(r5.xyxw)+(-(r6.xyxw))).xyw;
    // 132: mad r5.xyw, r1.wwww, r5.xyxw, r6.xyxw
    r5.xyw = ((r1.wwww)*(r5.xyxw)+(r6.xyxw)).xyw;
    // 133: mul r4.xyz, r4.xyzx, r5.xywx
    r4.xyz = ((r4.xyzx)*(r5.xywx)).xyz;
    // 134: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 135: mov_sat r1.w, cb0[15].x
    r1.w = (saturate(source[15].xxxx)).w;
    // 136: mul_sat r2.w, r3.w, cb2[3].w
    r2.w = (saturate((r3.wwww)*(passValues[3].wwww))).w;
    // 137: add r3.w, -cb0[16].y, cb0[16].x
    r3.w = ((-(source[16].yyyy))+(source[16].xxxx)).w;
    // 138: mad r3.w, r8.x, r3.w, cb0[16].y
    r3.w = ((r8.xxxx)*(r3.wwww)+(source[16].yyyy)).w;
    // 139: add r4.w, -r3.w, cb0[16].w
    r4.w = ((-(r3.wwww))+(source[16].wwww)).w;
    // 140: mad r3.w, r8.y, r4.w, r3.w
    r3.w = ((r8.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 141: add r4.w, -r3.w, cb0[17].y
    r4.w = ((-(r3.wwww))+(source[17].yyyy)).w;
    // 142: mad r3.w, r8.z, r4.w, r3.w
    r3.w = ((r8.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 143: mul r3.w, r5.z, r3.w
    r3.w = ((r5.zzzz)*(r3.wwww)).w;
    // 144: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 145: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: movc r3.w, r6.z, l(0), r3.w
    r3.w = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 147: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 148: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 150: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 151: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 152: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 153: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 154: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 155: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 156: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: dp3_sat r1.x, r2.xyzx, r1.xyzx
    r1.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 158: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 159: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 160: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 161: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 162: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 163: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 164: mad r0.yzw, -r4.xxyz, r2.wwww, r4.xxyz
    r0.yzw = ((-(r4.xxyz))*(r2.wwww)+(r4.xxyz)).yzw;
    // 165: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 166: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 167: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 168: mad r2.x, r4.w, r1.z, -r4.w
    r2.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 169: mad r2.x, r2.x, r4.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 170: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 171: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 172: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 173: mad r2.x, -r3.w, r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 174: mad r2.y, r5.w, r2.x, r1.y
    r2.y = ((r5.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 175: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 176: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 177: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 178: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 179: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 180: mul r1.z, r1.w, l(0.080000)
    r1.z = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 181: mad r2.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r2.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 182: mad r2.xyz, r2.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 183: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 184: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 185: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 186: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 187: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 188: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 189: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: max r4.xyz, r2.xyzx, r1.wwww
    r4.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 191: add r4.xyz, -r2.xyzx, r4.xyzx
    r4.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 192: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 193: mad r2.xyz, r1.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 194: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 195: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 196: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 197: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 198: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 199: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 200: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 201: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 202: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 203: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 204: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 205: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 206: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 207: mov_sat r9.x, r9.x
    r9.x = (saturate(r9.xxxx)).x;
    // 208: mul_sat r0.x, r9.x, cb0[15].y
    r0.x = (saturate((r9.xxxx)*(source[15].yyyy))).x;
    // 209: mul o0.w, r0.x, cb0[0].y
    output.targets[0].w = ((r0.xxxx)*(source[0].yyyy)).w;
    // 210: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 211: ret
    return output;
}

// source.character.equipment-native-196.v1 / source program 52f5610043d75e4e8f588d3202f908f2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight196(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].y=(g_SourceCharacterTime.xxxx).x;
    source[20].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[20].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[21].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[21].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[24]=float4(input.lightColor,1.0);
    source[25].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[25].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[25].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 37: mul r10.xy, r9.xyxx, cb0[15].xxxx
    r10.xy = ((r9.xyxx)*(source[15].xxxx)).xy;
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
    // 44: mad r9.xyz, cb0[15].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[15].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: add r0.z, -cb0[16].y, cb0[16].x
    r0.z = ((-(source[16].yyyy))+(source[16].xxxx)).z;
    // 65: mad r0.z, r2.x, r0.z, cb0[16].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[16].yyyy)).z;
    // 66: add r1.x, -r0.z, cb0[16].z
    r1.x = ((-(r0.zzzz))+(source[16].zzzz)).x;
    // 67: mad r0.z, r2.y, r1.x, r0.z
    r0.z = ((r2.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 68: add r1.x, -r0.z, cb0[16].w
    r1.x = ((-(r0.zzzz))+(source[16].wwww)).x;
    // 69: mad r0.z, r2.z, r1.x, r0.z
    r0.z = ((r2.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 71: add r1.x, -r9.w, l(1.000000)
    r1.x = ((-(r9.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 72: add r1.y, -cb0[17].y, cb0[17].x
    r1.y = ((-(source[17].yyyy))+(source[17].xxxx)).y;
    // 73: mad r1.y, r2.x, r1.y, cb0[17].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[17].yyyy)).y;
    // 74: add r1.w, -r1.y, cb0[17].z
    r1.w = ((-(r1.yyyy))+(source[17].zzzz)).w;
    // 75: mad r1.y, r2.y, r1.w, r1.y
    r1.y = ((r2.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 76: add r1.w, -r1.y, cb0[17].w
    r1.w = ((-(r1.yyyy))+(source[17].wwww)).w;
    // 77: mad r1.y, r2.z, r1.w, r1.y
    r1.y = ((r2.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 78: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 79: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 80: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 81: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 82: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 84: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 85: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 86: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 87: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 88: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 89: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 90: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 91: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 92: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 93: max r1.y, r1.y, r4.x
    r1.y = (max(r1.yyyy,r4.xxxx)).y;
    // 94: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 95: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 96: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 97: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 98: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 99: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 100: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 101: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 102: rcp r1.y, cb0[18].x
    r1.y = (1.0/(source[18].xxxx)).y;
    // 103: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 104: mul r11.xyz, r4.xyzx, cb0[18].xxxx
    r11.xyz = ((r4.xyzx)*(source[18].xxxx)).xyz;
    // 105: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 106: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 107: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 108: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 109: mad r4.xyz, r11.xyzx, cb0[18].xxxx, r4.xyzx
    r4.xyz = ((r11.xyzx)*(source[18].xxxx)+(r4.xyzx)).xyz;
    // 110: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 111: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 112: add r1.y, cb0[18].x, l(1.000000)
    r1.y = ((source[18].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 113: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 114: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 115: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 116: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 117: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 118: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 119: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 120: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 121: mul r4.w, |r1.y|, |r1.y|
    r4.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 122: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 123: mul r1.y, |r1.y|, r4.w
    r1.y = ((abs(r1.yyyy))*(r4.wwww)).y;
    // 124: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 125: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 126: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 127: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 128: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 129: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 130: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 132: mul r11.xyz, r0.xyzx, r1.yyyy
    r11.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 133: mul r12.xyz, r11.xyzx, cb0[21].wwww
    r12.xyz = ((r11.xyzx)*(source[21].wwww)).xyz;
    // 134: dp3 r4.w, r10.xyzx, r10.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 135: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 136: div r10.xyz, r10.xyzx, r4.wwww
    r10.xyz = ((r10.xyzx)/(r4.wwww)).xyz;
    // 137: dp3 r4.w, r10.xyzx, r7.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 138: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 139: min r6.w, r5.w, l(1.000000)
    r6.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: mad r4.w, r4.w, l(0.500000), -r6.w
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r6.wwww))).w;
    // 142: mad r4.w, r1.x, r4.w, r6.w
    r4.w = ((r1.xxxx)*(r4.wwww)+(r6.wwww)).w;
    // 143: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 145: mul_sat r6.xy, r6.xzxx, cb0[18].wwww
    r6.xy = (saturate((r6.xzxx)*(source[18].wwww))).xy;
    // 146: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 147: add_sat r6.y, r6.y, -cb0[19].x
    r6.y = (saturate((r6.yyyy)+(-(source[19].xxxx)))).y;
    // 148: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 149: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 150: mul r6.y, r6.y, cb0[19].y
    r6.y = ((r6.yyyy)*(source[19].yyyy)).y;
    // 151: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 152: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 153: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 154: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 155: mul r6.y, r4.w, r6.y
    r6.y = ((r4.wwww)*(r6.yyyy)).y;
    // 156: mad r6.z, r0.w, l(2.000000), -r1.y
    r6.z = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).z;
    // 157: mad r6.y, r6.y, r6.z, r1.y
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r1.yyyy)).y;
    // 158: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 159: mul_sat r6.y, r4.w, r6.y
    r6.y = (saturate((r4.wwww)*(r6.yyyy))).y;
    // 160: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 161: mul r1.x, r1.x, cb0[22].y
    r1.x = ((r1.xxxx)*(source[22].yyyy)).x;
    // 162: mad r6.y, cb0[22].x, r6.y, -r4.w
    r6.y = ((source[22].xxxx)*(r6.yyyy)+(-(r4.wwww))).y;
    // 163: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 164: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 165: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 166: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 167: mad r7.xyz, -cb0[21].wwww, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[21].wwww))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mad r7.xyz, r5.xyzx, r7.xyzx, r12.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 169: mul r11.xyz, cb0[3].xyzx, cb0[3].wwww
    r11.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 170: mad r12.xyz, cb0[4].wwww, cb0[4].xyzx, -r11.xyzx
    r12.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r11.xyzx))).xyz;
    // 171: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 172: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 173: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 174: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 175: mad r11.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 176: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 177: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 178: mad r11.xyz, cb0[15].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[15].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 179: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 180: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 181: mad r11.xyz, cb0[15].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[15].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 182: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 183: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 185: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 186: dp3 r1.x, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 187: add r13.xyz, -r8.yzwy, r1.xxxx
    r13.xyz = ((-(r8.yzwy))+(r1.xxxx)).xyz;
    // 188: mad r8.xyz, cb0[15].yyyy, r13.xyzx, r8.yzwy
    r8.xyz = ((source[15].yyyy)*(r13.xyzx)+(r8.yzwy)).xyz;
    // 189: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 190: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 191: mad r8.xyz, cb0[15].zzzz, r13.xyzx, r8.xyzx
    r8.xyz = ((source[15].zzzz)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 192: mul r13.xyz, r8.xyzx, r11.xyzx
    r13.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 193: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 194: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 195: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 196: add r14.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r14.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 197: mad r14.xyz, r0.yyyy, r14.xyzx, cb0[9].xyzx
    r14.xyz = ((r0.yyyy)*(r14.xyzx)+(source[9].xyzx)).xyz;
    // 198: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 199: mul r0.xyz, r0.xyzx, cb0[18].yyyy
    r0.xyz = ((r0.xyzx)*(source[18].yyyy)).xyz;
    // 200: mul r14.xyz, r0.xyzx, r13.xyzx
    r14.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 201: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 202: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 203: mad r9.xyz, cb0[15].yyyy, r15.xyzx, r9.xyzx
    r9.xyz = ((source[15].yyyy)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 204: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 205: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 206: mad r9.xyz, cb0[15].zzzz, r15.xyzx, r9.xyzx
    r9.xyz = ((source[15].zzzz)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 207: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 208: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 209: mul r15.xyz, r15.xyzx, cb0[18].zzzz
    r15.xyz = ((r15.xyzx)*(source[18].zzzz)).xyz;
    // 210: add r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 211: add r1.x, r2.z, r1.x
    r1.x = ((r2.zzzz)+(r1.xxxx)).x;
    // 212: add_sat r1.x, r2.w, r1.x
    r1.x = (saturate((r2.wwww)+(r1.xxxx))).x;
    // 213: mad r2.xyz, r1.xxxx, r15.xyzx, r9.xyzx
    r2.xyz = ((r1.xxxx)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 214: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 215: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 216: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 217: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 218: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 219: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 220: mul r1.x, r1.x, cb0[19].z
    r1.x = ((r1.xxxx)*(source[19].zzzz)).x;
    // 221: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 222: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 223: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 224: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 225: div r1.z, cb0[19].w, r1.z
    r1.z = ((source[19].wwww)/(r1.zzzz)).z;
    // 226: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 227: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 228: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 229: mul r1.z, r1.z, cb0[20].x
    r1.z = ((r1.zzzz)*(source[20].xxxx)).z;
    // 230: mad r0.xyz, r2.xyzx, r0.xyzx, -r14.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r14.xyzx))).xyz;
    // 231: mad r0.xyz, r1.zzzz, r0.xyzx, r14.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r14.xyzx)).xyz;
    // 232: mad r0.xyz, r1.yyyy, r0.xyzx, -r13.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 233: mad r0.xyz, r1.xxxx, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 234: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 235: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 236: mad r0.xyz, cb0[15].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[15].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 237: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 238: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 239: mad r0.xyz, cb0[15].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[15].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 240: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 241: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 242: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 243: mul r2.w, r2.w, cb0[20].y
    r2.w = ((r2.wwww)*(source[20].yyyy)).w;
    // 244: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 245: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 246: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 247: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 248: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 249: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 250: add r4.w, -r2.w, cb0[2].x
    r4.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 251: mul r9.z, r4.w, l(0.125000)
    r9.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 252: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 253: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 254: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 255: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 256: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 257: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 258: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 259: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 260: mul r6.xyz, r1.zzzz, r9.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xyzx)).xyz;
    // 261: mul r1.z, r2.w, r9.w
    r1.z = ((r2.wwww)*(r9.wwww)).z;
    // 262: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 263: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 264: mul r1.z, cb0[12].y, cb0[20].y
    r1.z = ((source[12].yyyy)*(source[20].yyyy)).z;
    // 265: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 266: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 267: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 268: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 269: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 270: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 271: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 272: mul r2.w, cb0[12].x, l(0.001000)
    r2.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 273: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 274: mad r3.xy, r2.wwww, r3.xyxx, r6.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r6.xyxx)).xy;
    // 275: dp2 r2.w, cb0[13].xyxx, r3.xyxx
    r2.w = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 276: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 277: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 278: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 279: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 280: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 281: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 282: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 283: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 284: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 285: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 286: mul r6.xyz, r3.xyzx, cb0[12].zzzz
    r6.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 287: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 288: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 289: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 290: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 291: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 292: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 293: mad r5.xyz, r11.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r11.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 294: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 295: dp3 r4.x, r4.xyzx, r10.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 296: mul r4.y, r0.w, cb0[22].z
    r4.y = ((r0.wwww)*(source[22].zzzz)).y;
    // 297: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 298: add r0.w, -cb0[22].w, l(2.000000)
    r0.w = ((-(source[22].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 299: mad r0.w, r1.y, r0.w, cb0[22].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[22].wwww)).w;
    // 300: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 301: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 302: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 303: mul r1.xyz, r1.xyzx, cb0[23].xxxx
    r1.xyz = ((r1.xyzx)*(source[23].xxxx)).xyz;
    // 304: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 305: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 306: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 307: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 308: mul r1.xyz, r1.xyzx, cb0[23].yyyy
    r1.xyz = ((r1.xyzx)*(source[23].yyyy)).xyz;
    // 309: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 310: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 311: mad r0.xyz, r7.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 312: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 313: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 314: mul o0.xyz, r0.xyzx, cb0[24].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[24].xyzx)).xyz;
    // 315: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 316: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 317: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 318: ret
    return output;
}

// source.character.equipment-native-197.v1 / source program 6138ff4e6ffadc42ba06defb1b378718
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight197(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].x=(g_SourceCharacterTime.xxxx).x;
    source[21].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[21].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[22].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[22].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[25]=float4(input.lightColor,1.0);
    source[26].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[26].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[26].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 37: mul r10.xy, r9.xyxx, cb0[15].xxxx
    r10.xy = ((r9.xyxx)*(source[15].xxxx)).xy;
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
    // 44: mad r9.xyz, cb0[15].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[15].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: add r0.z, -cb0[16].y, cb0[16].x
    r0.z = ((-(source[16].yyyy))+(source[16].xxxx)).z;
    // 65: mad r0.z, r2.x, r0.z, cb0[16].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[16].yyyy)).z;
    // 66: add r1.x, -r0.z, cb0[16].z
    r1.x = ((-(r0.zzzz))+(source[16].zzzz)).x;
    // 67: mad r0.z, r2.y, r1.x, r0.z
    r0.z = ((r2.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 68: add r1.x, -r0.z, cb0[16].w
    r1.x = ((-(r0.zzzz))+(source[16].wwww)).x;
    // 69: mad r0.z, r2.z, r1.x, r0.z
    r0.z = ((r2.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 71: add r1.x, -r9.w, l(1.000000)
    r1.x = ((-(r9.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 72: add r1.y, -cb0[17].y, cb0[17].x
    r1.y = ((-(source[17].yyyy))+(source[17].xxxx)).y;
    // 73: mad r1.y, r2.x, r1.y, cb0[17].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[17].yyyy)).y;
    // 74: add r1.w, -r1.y, cb0[17].z
    r1.w = ((-(r1.yyyy))+(source[17].zzzz)).w;
    // 75: mad r1.y, r2.y, r1.w, r1.y
    r1.y = ((r2.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 76: add r1.w, -r1.y, cb0[17].w
    r1.w = ((-(r1.yyyy))+(source[17].wwww)).w;
    // 77: mad r1.y, r2.z, r1.w, r1.y
    r1.y = ((r2.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 78: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 79: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 80: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 81: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 82: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 84: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 85: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 86: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 87: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 88: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 89: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 90: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 91: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 92: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 93: max r1.y, r1.y, r4.x
    r1.y = (max(r1.yyyy,r4.xxxx)).y;
    // 94: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 95: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 96: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 97: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 98: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 99: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 100: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 101: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 102: rcp r1.y, cb0[18].x
    r1.y = (1.0/(source[18].xxxx)).y;
    // 103: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 104: mul r11.xyz, r4.xyzx, cb0[18].xxxx
    r11.xyz = ((r4.xyzx)*(source[18].xxxx)).xyz;
    // 105: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 106: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 107: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 108: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 109: mad r4.xyz, r11.xyzx, cb0[18].xxxx, r4.xyzx
    r4.xyz = ((r11.xyzx)*(source[18].xxxx)+(r4.xyzx)).xyz;
    // 110: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 111: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 112: add r1.y, cb0[18].x, l(1.000000)
    r1.y = ((source[18].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 113: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 114: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 115: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 116: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 117: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 118: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 119: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 120: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 121: mul r4.w, |r1.y|, |r1.y|
    r4.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 122: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 123: mul r1.y, |r1.y|, r4.w
    r1.y = ((abs(r1.yyyy))*(r4.wwww)).y;
    // 124: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 125: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 126: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 127: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 128: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 129: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 130: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 132: mul r11.xyz, r0.xyzx, r1.yyyy
    r11.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 133: mul r12.xyz, r11.xyzx, cb0[22].wwww
    r12.xyz = ((r11.xyzx)*(source[22].wwww)).xyz;
    // 134: dp3 r4.w, r10.xyzx, r10.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 135: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 136: div r10.xyz, r10.xyzx, r4.wwww
    r10.xyz = ((r10.xyzx)/(r4.wwww)).xyz;
    // 137: dp3 r4.w, r10.xyzx, r7.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 138: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 139: min r6.w, r5.w, l(1.000000)
    r6.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: mad r4.w, r4.w, l(0.500000), -r6.w
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r6.wwww))).w;
    // 142: mad r4.w, r1.x, r4.w, r6.w
    r4.w = ((r1.xxxx)*(r4.wwww)+(r6.wwww)).w;
    // 143: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 145: mul_sat r6.xy, r6.xzxx, cb0[18].wwww
    r6.xy = (saturate((r6.xzxx)*(source[18].wwww))).xy;
    // 146: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 147: add_sat r6.y, r6.y, -cb0[19].x
    r6.y = (saturate((r6.yyyy)+(-(source[19].xxxx)))).y;
    // 148: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 149: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 150: mul r6.y, r6.y, cb0[19].y
    r6.y = ((r6.yyyy)*(source[19].yyyy)).y;
    // 151: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 152: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 153: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 154: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 155: mul r6.y, r4.w, r6.y
    r6.y = ((r4.wwww)*(r6.yyyy)).y;
    // 156: mad r6.z, r0.w, l(2.000000), -r1.y
    r6.z = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).z;
    // 157: mad r6.y, r6.y, r6.z, r1.y
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r1.yyyy)).y;
    // 158: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 159: mul_sat r6.y, r4.w, r6.y
    r6.y = (saturate((r4.wwww)*(r6.yyyy))).y;
    // 160: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 161: mul r1.x, r1.x, cb0[23].y
    r1.x = ((r1.xxxx)*(source[23].yyyy)).x;
    // 162: mad r6.y, cb0[23].x, r6.y, -r4.w
    r6.y = ((source[23].xxxx)*(r6.yyyy)+(-(r4.wwww))).y;
    // 163: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 164: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 165: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 166: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 167: mad r7.xyz, -cb0[22].wwww, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[22].wwww))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mad r7.xyz, r5.xyzx, r7.xyzx, r12.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 169: mul r11.xyz, cb0[3].xyzx, cb0[3].wwww
    r11.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 170: mad r12.xyz, cb0[4].wwww, cb0[4].xyzx, -r11.xyzx
    r12.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r11.xyzx))).xyz;
    // 171: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 172: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 173: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 174: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 175: mad r11.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 176: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 177: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 178: mad r11.xyz, cb0[15].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[15].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 179: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 180: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 181: mad r11.xyz, cb0[15].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[15].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 182: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 183: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 185: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 186: dp3 r1.x, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 187: add r13.xyz, -r8.yzwy, r1.xxxx
    r13.xyz = ((-(r8.yzwy))+(r1.xxxx)).xyz;
    // 188: mad r8.xyz, cb0[15].yyyy, r13.xyzx, r8.yzwy
    r8.xyz = ((source[15].yyyy)*(r13.xyzx)+(r8.yzwy)).xyz;
    // 189: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 190: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 191: mad r8.xyz, cb0[15].zzzz, r13.xyzx, r8.xyzx
    r8.xyz = ((source[15].zzzz)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 192: mul r13.xyz, r8.xyzx, r11.xyzx
    r13.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 193: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 194: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 195: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 196: add r14.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r14.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 197: mad r14.xyz, r0.yyyy, r14.xyzx, cb0[9].xyzx
    r14.xyz = ((r0.yyyy)*(r14.xyzx)+(source[9].xyzx)).xyz;
    // 198: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 199: mul r0.xyz, r0.xyzx, cb0[18].yyyy
    r0.xyz = ((r0.xyzx)*(source[18].yyyy)).xyz;
    // 200: mul r14.xyz, r0.xyzx, r13.xyzx
    r14.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 201: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 202: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 203: mad r9.xyz, cb0[15].yyyy, r15.xyzx, r9.xyzx
    r9.xyz = ((source[15].yyyy)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 204: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 205: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 206: mad r9.xyz, cb0[15].zzzz, r15.xyzx, r9.xyzx
    r9.xyz = ((source[15].zzzz)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 207: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 208: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 209: mul r15.xyz, r15.xyzx, cb0[18].zzzz
    r15.xyz = ((r15.xyzx)*(source[18].zzzz)).xyz;
    // 210: add r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 211: add r1.x, r2.z, r1.x
    r1.x = ((r2.zzzz)+(r1.xxxx)).x;
    // 212: add_sat r1.x, r2.w, r1.x
    r1.x = (saturate((r2.wwww)+(r1.xxxx))).x;
    // 213: mad r2.xyz, r1.xxxx, r15.xyzx, r9.xyzx
    r2.xyz = ((r1.xxxx)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 214: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 215: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 216: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 217: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 218: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 219: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 220: mul r1.x, r1.x, cb0[19].z
    r1.x = ((r1.xxxx)*(source[19].zzzz)).x;
    // 221: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 222: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 223: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 224: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 225: div r1.z, cb0[19].w, r1.z
    r1.z = ((source[19].wwww)/(r1.zzzz)).z;
    // 226: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 227: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 228: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 229: mul r1.z, r1.z, cb0[20].x
    r1.z = ((r1.zzzz)*(source[20].xxxx)).z;
    // 230: mad r0.xyz, r2.xyzx, r0.xyzx, -r14.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r14.xyzx))).xyz;
    // 231: mad r0.xyz, r1.zzzz, r0.xyzx, r14.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r14.xyzx)).xyz;
    // 232: mad r0.xyz, r1.yyyy, r0.xyzx, -r13.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 233: mad r0.xyz, r1.xxxx, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 234: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 235: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 236: mad r0.xyz, cb0[15].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[15].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 237: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 238: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 239: mad r0.xyz, cb0[15].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[15].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 240: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 241: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 242: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 243: mul r2.w, r2.w, cb0[21].x
    r2.w = ((r2.wwww)*(source[21].xxxx)).w;
    // 244: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 245: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 246: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 247: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 248: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 249: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 250: add r4.w, -r2.w, cb0[2].x
    r4.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 251: mul r9.z, r4.w, l(0.125000)
    r9.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 252: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 253: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 254: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 255: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 256: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 257: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 258: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 259: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 260: mul r6.xyz, r1.zzzz, r9.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xyzx)).xyz;
    // 261: mul r1.z, r2.w, r9.w
    r1.z = ((r2.wwww)*(r9.wwww)).z;
    // 262: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 263: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 264: mul r1.z, cb0[12].y, cb0[21].x
    r1.z = ((source[12].yyyy)*(source[21].xxxx)).z;
    // 265: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 266: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 267: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 268: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 269: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 270: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 271: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 272: mul r2.w, cb0[12].x, l(0.001000)
    r2.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 273: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 274: mad r3.xy, r2.wwww, r3.xyxx, r6.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r6.xyxx)).xy;
    // 275: dp2 r2.w, cb0[13].xyxx, r3.xyxx
    r2.w = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 276: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 277: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 278: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 279: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 280: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 281: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 282: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 283: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 284: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 285: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 286: mul r6.xyz, r3.xyzx, cb0[12].zzzz
    r6.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 287: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 288: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 289: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 290: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 291: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 292: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 293: mad r5.xyz, r11.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r11.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 294: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 295: dp3 r4.x, r4.xyzx, r10.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 296: mul r4.y, r0.w, cb0[23].z
    r4.y = ((r0.wwww)*(source[23].zzzz)).y;
    // 297: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 298: add r0.w, -cb0[23].w, l(2.000000)
    r0.w = ((-(source[23].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 299: mad r0.w, r1.y, r0.w, cb0[23].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[23].wwww)).w;
    // 300: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 301: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 302: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 303: mul r1.xyz, r1.xyzx, cb0[24].xxxx
    r1.xyz = ((r1.xyzx)*(source[24].xxxx)).xyz;
    // 304: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 305: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 306: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 307: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 308: mul r1.xyz, r1.xyzx, cb0[24].yyyy
    r1.xyz = ((r1.xyzx)*(source[24].yyyy)).xyz;
    // 309: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 310: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 311: mad r0.xyz, r7.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 312: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 313: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 314: mul o0.xyz, r0.xyzx, cb0[25].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)).xyz;
    // 315: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 316: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 317: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 318: ret
    return output;
}

// source.character.equipment-native-198.v1 / source program 4e96a2f17ef26a418c96e9a47c312773
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight198(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].z=(g_SourceCharacterTime.xxxx).x;
    source[22]=float4(input.lightColor,1.0);
    source[23].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
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
    // 9: mul r3.xy, r2.xyxx, cb0[14].xxxx
    r3.xy = ((r2.xyxx)*(source[14].xxxx)).xy;
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
    // 15: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 17: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 18: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r4.xyzw, r3.xyzw, cb0[12].xyzw
    r4.xyzw = ((r3.xyzw)*(source[12].xyzw)).xyzw;
    // 23: add r4.xy, r4.ywyy, r4.xzxx
    r4.xy = ((r4.ywyy)+(r4.xzxx)).xy;
    // 24: add r1.w, r4.y, r4.x
    r1.w = ((r4.yyyy)+(r4.xxxx)).w;
    // 25: add r3.xy, r3.ywyy, r3.xzxx
    r3.xy = ((r3.ywyy)+(r3.xzxx)).xy;
    // 26: add r2.w, r3.y, r3.x
    r2.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 27: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 28: mad_sat r1.w, r2.w, r1.w, l(1.000000)
    r1.w = (saturate((r2.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 30: mul_sat r1.w, r1.w, r3.w
    r1.w = (saturate((r1.wwww)*(r3.wwww))).w;
    // 31: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 32: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 33: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 34: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[23].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[23].xxxx)) * 0xffffffffu)).w;
    // 35: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 36: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 37: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 39: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 40: else
    } else {
    // 41: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 42: endif
    }
    // 43: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 46: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 47: mul r1.w, r6.x, cb0[17].y
    r1.w = ((r6.xxxx)*(source[17].yyyy)).w;
    // 48: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 49: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 50: add_sat r1.w, r1.w, cb0[17].z
    r1.w = (saturate((r1.wwww)+(source[17].zzzz))).w;
    // 51: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mul r8.xyz, r2.wwww, cb0[10].xyzx
    r8.xyz = ((r2.wwww)*(source[10].xyzx)).xyz;
    // 53: mul r9.xyz, cb0[3].xyzx, cb0[3].wwww
    r9.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 54: max r10.xyz, r9.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r9.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 55: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 56: max r9.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 57: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 58: mul r2.w, r6.y, cb0[14].y
    r2.w = ((r6.yyyy)*(source[14].yyyy)).w;
    // 59: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 60: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 62: add r6.xyw, -r10.xyxz, r9.xyxz
    r6.xyw = ((-(r10.xyxz))+(r9.xyxz)).xyw;
    // 63: mad r6.xyw, r2.wwww, r6.xyxw, r10.xyxz
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r10.xyxz)).xyw;
    // 64: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 65: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 66: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 67: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 68: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 69: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 70: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 72: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 73: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 74: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 75: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 76: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 77: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 78: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 79: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 80: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 81: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 82: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 83: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 84: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 85: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 86: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 87: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 88: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 89: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 90: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 91: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 92: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 94: mad r6.xyw, cb0[16].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 95: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 97: mad r6.xyw, cb0[16].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 98: mad r7.xyw, cb0[8].wwww, cb0[8].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[8].wwww)*(source[8].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 99: mad r10.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 100: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 101: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 102: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 104: mad r3.xyz, cb0[16].xxxx, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].xxxx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 105: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 107: mad r3.xyz, cb0[16].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 108: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 109: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 111: mad r3.xyz, cb0[16].xxxx, r3.xyzx, r10.xyzx
    r3.xyz = ((source[16].xxxx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 112: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 114: mad r3.xyz, cb0[16].yyyy, r6.xywx, r3.xyzx
    r3.xyz = ((source[16].yyyy)*(r6.xywx)+(r3.xyzx)).xyz;
    // 115: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 116: mul r3.w, cb0[7].z, l(1.500000)
    r3.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 117: add r4.w, -cb0[7].w, l(1.000000)
    r4.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r4.w, r4.w, cb0[15].z
    r4.w = ((r4.wwww)*(source[15].zzzz)).w;
    // 119: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 120: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 121: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 123: mad r3.w, r3.w, l(0.500000), cb0[7].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 124: frc r4.w, cb0[7].x
    r4.w = (frac(source[7].xxxx)).w;
    // 125: add r5.w, -r4.w, cb0[7].x
    r5.w = ((-(r4.wwww))+(source[7].xxxx)).w;
    // 126: mul r10.z, r5.w, l(0.125000)
    r10.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 127: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 128: mul r10.y, cb0[7].y, cb0[11].y
    r10.y = ((source[7].yyyy)*(source[11].yyyy)).y;
    // 129: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 130: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 131: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 132: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 133: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t5.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 135: mul r6.xyw, r3.wwww, r10.xyxz
    r6.xyw = ((r3.wwww)*(r10.xyxz)).xyw;
    // 136: mul r3.w, r4.w, r10.w
    r3.w = ((r4.wwww)*(r10.wwww)).w;
    // 137: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 138: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 139: add r3.w, r3.y, r3.x
    r3.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 140: add r3.w, r3.z, r3.w
    r3.w = ((r3.zzzz)+(r3.wwww)).w;
    // 141: mul r3.w, r3.w, l(0.333330)
    r3.w = ((r3.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 142: max r3.w, r3.w, cb0[18].x
    r3.w = (max(r3.wwww,source[18].xxxx)).w;
    // 143: min r3.w, r3.w, cb0[17].w
    r3.w = (min(r3.wwww,source[17].wwww)).w;
    // 144: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mad r3.w, r2.w, r4.w, r3.w
    r3.w = ((r2.wwww)*(r4.wwww)+(r3.wwww)).w;
    // 146: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 147: mad r3.w, cb0[18].z, r3.w, l(1.000000)
    r3.w = ((source[18].zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: mul r6.xyw, r3.xyxz, r3.wwww
    r6.xyw = ((r3.xyxz)*(r3.wwww)).xyw;
    // 149: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 150: mad r3.xyz, r3.wwww, r3.xyzx, -r6.xywx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(-(r6.xywx))).xyz;
    // 151: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 152: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 153: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 154: mov_sat r1.w, cb0[18].w
    r1.w = (saturate(source[18].wwww)).w;
    // 155: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 156: add r3.w, -cb0[19].w, cb0[19].z
    r3.w = ((-(source[19].wwww))+(source[19].zzzz)).w;
    // 157: mad r3.w, r9.x, r3.w, cb0[19].w
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[19].wwww)).w;
    // 158: add r4.w, -r3.w, cb0[20].y
    r4.w = ((-(r3.wwww))+(source[20].yyyy)).w;
    // 159: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 160: add r4.w, -r3.w, cb0[20].w
    r4.w = ((-(r3.wwww))+(source[20].wwww)).w;
    // 161: mad r3.w, r9.z, r4.w, r3.w
    r3.w = ((r9.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 162: mul r3.w, r6.z, r3.w
    r3.w = ((r6.zzzz)*(r3.wwww)).w;
    // 163: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 164: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 166: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 167: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 169: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 170: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 171: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 172: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 173: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 174: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 175: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 177: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 178: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 181: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 182: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 183: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 184: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 185: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 186: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 187: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 188: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 190: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 191: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 192: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 193: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 194: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 195: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 196: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 197: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 198: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 199: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 200: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 201: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 202: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 203: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 204: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 205: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 206: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 207: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 208: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 209: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 210: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 211: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 212: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 213: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 214: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 215: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 216: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 217: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 218: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 219: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 220: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 221: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 222: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 223: mul_sat r6.xyz, cb0[13].xyzx, cb0[13].wwww
    r6.xyz = (saturate((source[13].xyzx)*(source[13].wwww))).xyz;
    // 224: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 225: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 226: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 227: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 228: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 229: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 230: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 231: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 232: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 233: mul r0.x, r0.x, cb0[21].x
    r0.x = ((r0.xxxx)*(source[21].xxxx)).x;
    // 234: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 235: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 236: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 238: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 239: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 240: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 241: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 242: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 243: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 244: ret
    return output;
}

// source.character.equipment-native-199.v1 / source program 5bc35f777d8e3b47bb06a8781051ad17
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight199(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14].z=(g_SourceCharacterTime.xxxx).x;
    source[21]=float4(input.lightColor,1.0);
    source[22].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
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
    // 9: mul r3.xy, r2.xyxx, cb0[13].xxxx
    r3.xy = ((r2.xyxx)*(source[13].xxxx)).xy;
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
    // 15: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 17: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 18: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 22: mov_sat r3.x, r3.x
    r3.x = (saturate(r3.xxxx)).x;
    // 23: add r1.w, r3.x, l(-0.333300)
    r1.w = ((r3.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 24: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 26: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[22].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[22].xxxx)) * 0xffffffffu)).w;
    // 27: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 28: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 29: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 30: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t5.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 31: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 32: else
    } else {
    // 33: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 34: endif
    }
    // 35: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 37: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 38: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 39: mul r1.w, r6.x, cb0[16].y
    r1.w = ((r6.xxxx)*(source[16].yyyy)).w;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 42: add_sat r1.w, r1.w, cb0[16].z
    r1.w = (saturate((r1.wwww)+(source[16].zzzz))).w;
    // 43: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul r8.xyz, r2.wwww, cb0[10].xyzx
    r8.xyz = ((r2.wwww)*(source[10].xyzx)).xyz;
    // 45: mul r9.xyz, cb0[3].xyzx, cb0[3].wwww
    r9.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 46: max r10.xyz, r9.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r9.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 47: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 48: max r9.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 49: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 50: mul r2.w, r6.y, cb0[13].y
    r2.w = ((r6.yyyy)*(source[13].yyyy)).w;
    // 51: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 52: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 54: add r6.xyw, -r10.xyxz, r9.xyxz
    r6.xyw = ((-(r10.xyxz))+(r9.xyxz)).xyw;
    // 55: mad r6.xyw, r2.wwww, r6.xyxw, r10.xyxz
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r10.xyxz)).xyw;
    // 56: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 57: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 58: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 59: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 60: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 61: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 62: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 64: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 65: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 66: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 67: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 68: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 69: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 70: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 71: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 72: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 73: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 74: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 75: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 76: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 77: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 78: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 79: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 80: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 81: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 82: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 83: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 84: dp3 r3.x, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 85: add r7.xyw, -r6.xyxw, r3.xxxx
    r7.xyw = ((-(r6.xyxw))+(r3.xxxx)).xyw;
    // 86: mad r6.xyw, cb0[15].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 87: dp3 r3.x, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 88: add r7.xyw, -r6.xyxw, r3.xxxx
    r7.xyw = ((-(r6.xyxw))+(r3.xxxx)).xyw;
    // 89: mad r6.xyw, cb0[15].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 90: mad r7.xyw, cb0[8].wwww, cb0[8].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[8].wwww)*(source[8].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 91: mad r10.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 93: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 94: dp3 r3.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 95: add r10.xyz, -r3.yzwy, r3.xxxx
    r10.xyz = ((-(r3.yzwy))+(r3.xxxx)).xyz;
    // 96: mad r3.xyz, cb0[15].xxxx, r10.xyzx, r3.yzwy
    r3.xyz = ((source[15].xxxx)*(r10.xyzx)+(r3.yzwy)).xyz;
    // 97: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 99: mad r3.xyz, cb0[15].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[15].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 100: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 101: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 103: mad r3.xyz, cb0[15].xxxx, r3.xyzx, r10.xyzx
    r3.xyz = ((source[15].xxxx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 104: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 106: mad r3.xyz, cb0[15].yyyy, r6.xywx, r3.xyzx
    r3.xyz = ((source[15].yyyy)*(r6.xywx)+(r3.xyzx)).xyz;
    // 107: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 108: mul r3.w, cb0[7].z, l(1.500000)
    r3.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 109: add r4.w, -cb0[7].w, l(1.000000)
    r4.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: mul r4.w, r4.w, cb0[14].z
    r4.w = ((r4.wwww)*(source[14].zzzz)).w;
    // 111: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 112: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 113: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 115: mad r3.w, r3.w, l(0.500000), cb0[7].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 116: frc r4.w, cb0[7].x
    r4.w = (frac(source[7].xxxx)).w;
    // 117: add r5.w, -r4.w, cb0[7].x
    r5.w = ((-(r4.wwww))+(source[7].xxxx)).w;
    // 118: mul r10.z, r5.w, l(0.125000)
    r10.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 119: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 120: mul r10.y, cb0[7].y, cb0[11].y
    r10.y = ((source[7].yyyy)*(source[11].yyyy)).y;
    // 121: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 122: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 123: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 124: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 125: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 126: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t4.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 127: mul r6.xyw, r3.wwww, r10.xyxz
    r6.xyw = ((r3.wwww)*(r10.xyxz)).xyw;
    // 128: mul r3.w, r4.w, r10.w
    r3.w = ((r4.wwww)*(r10.wwww)).w;
    // 129: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 130: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 131: add r3.w, r3.y, r3.x
    r3.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 132: add r3.w, r3.z, r3.w
    r3.w = ((r3.zzzz)+(r3.wwww)).w;
    // 133: mul r3.w, r3.w, l(0.333330)
    r3.w = ((r3.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 134: max r3.w, r3.w, cb0[17].x
    r3.w = (max(r3.wwww,source[17].xxxx)).w;
    // 135: min r3.w, r3.w, cb0[16].w
    r3.w = (min(r3.wwww,source[16].wwww)).w;
    // 136: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: mad r3.w, r2.w, r4.w, r3.w
    r3.w = ((r2.wwww)*(r4.wwww)+(r3.wwww)).w;
    // 138: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 139: mad r3.w, cb0[17].z, r3.w, l(1.000000)
    r3.w = ((source[17].zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: mul r6.xyw, r3.xyxz, r3.wwww
    r6.xyw = ((r3.xyxz)*(r3.wwww)).xyw;
    // 141: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 142: mad r3.xyz, r3.wwww, r3.xyzx, -r6.xywx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(-(r6.xywx))).xyz;
    // 143: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 144: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 145: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 146: mov_sat r1.w, cb0[17].w
    r1.w = (saturate(source[17].wwww)).w;
    // 147: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 148: add r3.w, -cb0[18].w, cb0[18].z
    r3.w = ((-(source[18].wwww))+(source[18].zzzz)).w;
    // 149: mad r3.w, r9.x, r3.w, cb0[18].w
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[18].wwww)).w;
    // 150: add r4.w, -r3.w, cb0[19].y
    r4.w = ((-(r3.wwww))+(source[19].yyyy)).w;
    // 151: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 152: add r4.w, -r3.w, cb0[19].w
    r4.w = ((-(r3.wwww))+(source[19].wwww)).w;
    // 153: mad r3.w, r9.z, r4.w, r3.w
    r3.w = ((r9.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 154: mul r3.w, r6.z, r3.w
    r3.w = ((r6.zzzz)*(r3.wwww)).w;
    // 155: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 156: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 158: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 159: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 161: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 162: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 163: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 164: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 165: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 166: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 167: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 169: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 170: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 171: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 173: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 174: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 176: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 177: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 178: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 179: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 180: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 181: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 182: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 183: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 184: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 185: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 186: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 187: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 188: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 189: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 190: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 191: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 192: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 193: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 194: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 195: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 196: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 197: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 198: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 199: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 200: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 201: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 202: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 203: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 204: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 205: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 206: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 207: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 208: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 209: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 210: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 211: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 212: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 213: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 214: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 215: mul_sat r6.xyz, cb0[12].xyzx, cb0[12].wwww
    r6.xyz = (saturate((source[12].xyzx)*(source[12].wwww))).xyz;
    // 216: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 217: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 218: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 219: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 220: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 221: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 222: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 223: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 224: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 225: mul r0.x, r0.x, cb0[20].x
    r0.x = ((r0.xxxx)*(source[20].xxxx)).x;
    // 226: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 227: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 228: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 229: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 230: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 231: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 232: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 233: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 234: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 235: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 236: ret
    return output;
}

// source.character.equipment-native-200.v1 / source program aa285f67042a5f42b4eef02bdfe0d632
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight200(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[19]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[21]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[22]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[35].x=(g_SourceCharacterTime.xxxx).x;
    source[35].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[35].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[35].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[36].x=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[40]=float4(input.lightColor,1.0);
    source[41].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[41].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[41].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t12.xyzw, s0
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r8.xy, r8.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r8.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: mul r9.xy, r8.xyxx, cb0[24].zzzz
    r9.xy = ((r8.xyxx)*(source[24].zzzz)).xy;
    // 33: dp2 r1.w, r8.xyxx, r8.xyxx
    r1.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 34: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 37: add r9.z, r1.w, l(0.000010)
    r9.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 39: mad r8.xy, r8.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r8.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: dp2 r1.w, r8.xyxx, r8.xyxx
    r1.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 41: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 43: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 44: add r8.z, r1.w, l(0.000010)
    r8.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 45: mad r8.xyz, cb0[24].wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((source[24].wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 46: add r9.xyz, -r8.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r8.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 47: mad r10.xyz, cb0[31].zzzz, r9.xyzx, r8.xyzx
    r10.xyz = ((source[31].zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 48: dp3 r1.w, r10.xyzx, r10.xyzx
    r1.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 49: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 50: div r10.xyz, r10.xyzx, r1.wwww
    r10.xyz = ((r10.xyzx)/(r1.wwww)).xyz;
    // 51: dp3 r11.x, r1.xyzx, r10.xyzx
    r11.x = (dot((r1.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 52: dp3 r11.y, r2.xyzx, r10.xyzx
    r11.y = (dot((r2.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 53: dp3 r11.z, r0.xyzx, r10.xyzx
    r11.z = (dot((r0.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 54: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 55: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 56: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 57: dp3 r0.x, r11.xyzx, r1.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 58: mul r0.xyz, r11.xyzx, r0.xxxx
    r0.xyz = ((r11.xyzx)*(r0.xxxx)).xyz;
    // 59: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 60: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 61: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 62: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 63: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 64: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 65: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 66: div r1.xy, r4.xyxx, r4.zzzz
    r1.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 67: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 68: mul r1.xy, r1.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 69: deriv_rtx_coarse r1.zw, r1.xxxy
    r1.zw = (ddx_coarse(r1.xxxy)).zw;
    // 70: deriv_rty_coarse r1.xy, r1.xyxx
    r1.xy = (ddy_coarse(r1.xyxx)).xy;
    // 71: dp2 r0.z, r1.zwzz, r1.zwzz
    r0.z = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).z;
    // 72: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 73: max r0.z, r0.z, r1.x
    r0.z = (max(r0.zzzz,r1.xxxx)).z;
    // 74: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 75: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 76: rcp r1.x, |r0.z|
    r1.x = (1.0/(abs(r0.zzzz))).x;
    // 77: mul r1.x, r1.x, cb0[32].y
    r1.x = ((r1.xxxx)*(source[32].yyyy)).x;
    // 78: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 79: add r0.z, |r0.z|, r1.x
    r0.z = ((abs(r0.zzzz))+(r1.xxxx)).z;
    // 80: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 81: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t9.xyzw, s10, r0.z
    r0.xyz = ((g_SourceCharacterTexture9.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 82: rcp r1.x, cb0[32].z
    r1.x = (1.0/(source[32].zzzz)).x;
    // 83: log r1.yzw, r0.xxyz
    r1.yzw = (log2(r0.xxyz)).yzw;
    // 84: mul r2.xyz, r1.yzwy, cb0[32].zzzz
    r2.xyz = ((r1.yzwy)*(source[32].zzzz)).xyz;
    // 85: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 86: mul r1.yzw, r1.yyzw, r1.xxxx
    r1.yzw = ((r1.yyzw)*(r1.xxxx)).yzw;
    // 87: exp r1.yzw, r1.yyzw
    r1.yzw = (exp2(r1.yyzw)).yzw;
    // 88: mul r1.xyz, r1.xxxx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 89: mad r1.xyz, r2.xyzx, cb0[32].zzzz, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[32].zzzz)+(r1.xyzx)).xyz;
    // 90: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 91: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 92: add r1.x, cb0[32].z, l(1.000000)
    r1.x = ((source[32].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 93: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 94: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t2.xywz, s3, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 95: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 96: lt r1.w, |r1.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 97: log r1.z, |r1.z|
    r1.z = (log2(abs(r1.zzzz))).z;
    // 98: mul r1.z, r1.z, cb0[25].x
    r1.z = ((r1.zzzz)*(source[25].xxxx)).z;
    // 99: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 100: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 101: movc r1.z, r1.w, l(0), r1.z
    r1.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 102: mad r2.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r2.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 103: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 104: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 105: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 106: dp3 r1.w, r2.xyzx, r6.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 107: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 109: mul r4.x, |r1.w|, |r1.w|
    r4.x = ((abs(r1.wwww))*(abs(r1.wwww))).x;
    // 110: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 111: mul r1.w, |r1.w|, r4.x
    r1.w = ((abs(r1.wwww))*(r4.xxxx)).w;
    // 112: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 113: add r2.w, r1.w, l(-0.027778)
    r2.w = ((r1.wwww)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 114: mad r1.w, r1.w, r2.w, l(0.027778)
    r1.w = ((r1.wwww)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).w;
    // 115: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 116: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 117: min r4.xy, r0.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r0.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 118: add r0.w, -r4.x, l(1.000000)
    r0.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: mul r1.w, r0.w, r1.z
    r1.w = ((r0.wwww)*(r1.zzzz)).w;
    // 120: mul r4.xzw, r0.xxyz, r1.wwww
    r4.xzw = ((r0.xxyz)*(r1.wwww)).xzw;
    // 121: mul r10.xyz, r4.xzwx, cb0[36].zzzz
    r10.xyz = ((r4.xzwx)*(source[36].zzzz)).xyz;
    // 122: add r2.w, -|r7.z|, l(1.000000)
    r2.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 124: mul r2.w, r1.z, r2.w
    r2.w = ((r1.zzzz)*(r2.wwww)).w;
    // 125: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 126: mad r8.xyz, r2.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r2.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 127: dp3 r2.w, r8.xyzx, r8.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 128: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 129: div r8.xyz, r8.xyzx, r2.wwww
    r8.xyz = ((r8.xyzx)/(r2.wwww)).xyz;
    // 130: dp3 r2.w, r8.xyzx, r7.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 131: max r5.w, r2.w, l(0.000000)
    r5.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 132: min r6.w, r5.w, l(1.000000)
    r6.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: dp3 r7.x, cb0[23].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r7.x = (dot((source[23].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 135: add r7.xyz, r7.xxxx, -cb0[23].xyzx
    r7.xyz = ((r7.xxxx)+(-(source[23].xyzx))).xyz;
    // 136: mad r7.xyz, r6.wwww, r7.xyzx, cb0[23].xyzx
    r7.xyz = ((r6.wwww)*(r7.xyzx)+(source[23].xyzx)).xyz;
    // 137: mul r7.xyz, r1.zzzz, r7.xyzx
    r7.xyz = ((r1.zzzz)*(r7.xyzx)).xyz;
    // 138: mad r2.w, r2.w, l(0.500000), -r6.w
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r6.wwww))).w;
    // 139: mad r7.xyz, r7.xyzx, r2.wwww, r6.wwww
    r7.xyz = ((r7.xyzx)*(r2.wwww)+(r6.wwww)).xyz;
    // 140: mad r0.w, -r1.z, r0.w, l(1.000000)
    r0.w = ((-(r1.zzzz))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: dp3 r2.w, r8.xyzx, r6.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 142: mul_sat r2.w, r2.w, cb0[33].x
    r2.w = (saturate((r2.wwww)*(source[33].xxxx))).w;
    // 143: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mul_sat r6.x, r6.z, cb0[33].x
    r6.x = (saturate((r6.zzzz)*(source[33].xxxx))).x;
    // 145: add r6.x, -r6.x, l(1.000000)
    r6.x = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 146: add_sat r6.x, r6.x, -cb0[33].y
    r6.x = (saturate((r6.xxxx)+(-(source[33].yyyy)))).x;
    // 147: lt r6.y, r6.x, l(0.000001)
    r6.y = (asfloat((uint4)((r6.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 148: log r6.x, r6.x
    r6.x = (log2(r6.xxxx)).x;
    // 149: mul r6.x, r6.x, cb0[33].z
    r6.x = ((r6.xxxx)*(source[33].zzzz)).x;
    // 150: exp r6.x, r6.x
    r6.x = (exp2(r6.xxxx)).x;
    // 151: mul r2.w, r2.w, r6.x
    r2.w = ((r2.wwww)*(r6.xxxx)).w;
    // 152: movc r2.w, r6.y, l(0), r2.w
    r2.w = ((asuint(r6.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 153: add r6.x, -r2.w, l(1.000000)
    r6.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 154: mul r6.xyz, r7.xyzx, r6.xxxx
    r6.xyz = ((r7.xyzx)*(r6.xxxx)).xyz;
    // 155: mad r7.w, r0.w, l(2.000000), -r1.w
    r7.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.wwww))).w;
    // 156: mad r6.xyz, r6.xyzx, r7.wwww, r1.wwww
    r6.xyz = ((r6.xyzx)*(r7.wwww)+(r1.wwww)).xyz;
    // 157: add r6.xyz, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 158: mul_sat r6.xyz, r7.xyzx, r6.xyzx
    r6.xyz = (saturate((r7.xyzx)*(r6.xyzx))).xyz;
    // 159: add r6.xyz, -r7.xyzx, r6.xyzx
    r6.xyz = ((-(r7.xyzx))+(r6.xyzx)).xyz;
    // 160: mad r6.xyz, r1.zzzz, r6.xyzx, r7.xyzx
    r6.xyz = ((r1.zzzz)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 161: sqrt r1.z, r6.w
    r1.z = (sqrt(r6.wwww)).z;
    // 162: mul r5.xyz, r5.xyzx, r1.zzzz
    r5.xyz = ((r5.xyzx)*(r1.zzzz)).xyz;
    // 163: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 164: mad r4.xzw, -cb0[36].zzzz, r4.xxzw, l(1.000000, 0.000000, 1.000000, 1.000000)
    r4.xzw = ((-(source[36].zzzz))*(r4.xxzw)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 165: mad r4.xzw, r5.xxyz, r4.xxzw, r10.xxyz
    r4.xzw = ((r5.xxyz)*(r4.xxzw)+(r10.xxyz)).xzw;
    // 166: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 167: mul r6.xyz, r7.xyzx, cb0[3].xyzx
    r6.xyz = ((r7.xyzx)*(source[3].xyzx)).xyz;
    // 168: div r9.xy, l(1024.000000, 1024.000000, 0.000000, 0.000000), cb0[5].xyxx
    r9.xy = ((float4(1024.000000,1024.000000,0.000000,0.000000))/(source[5].xyxx)).xy;
    // 169: mad r9.zw, -cb0[5].xxxy, l(0.000000, 0.000000, 0.500000, 0.500000), cb0[5].zzzw
    r9.zw = ((-(source[5].xxxy))*(float4(0.000000,0.000000,0.500000,0.500000))+(source[5].zzzw)).zw;
    // 170: div r9.zw, r9.zzzw, cb0[5].xxxy
    r9.zw = ((r9.zzzw)/(source[5].xxxy)).zw;
    // 171: mad r9.xy, v4.xyxx, r9.xyxx, -r9.zwzz
    r9.xy = ((v4.xyxx)*(r9.xyxx)+(-(r9.zwzz))).xy;
    // 172: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, r9.xyxx, t4.xyzw, s5, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 173: mul r1.z, r9.x, cb0[4].w
    r1.z = ((r9.xxxx)*(source[4].wwww)).z;
    // 174: mad r9.xzw, -r7.xxyz, cb0[3].xxyz, cb0[4].xxyz
    r9.xzw = ((-(r7.xxyz))*(source[3].xxyz)+(source[4].xxyz)).xzw;
    // 175: mad r6.xyz, r1.zzzz, r9.xzwx, r6.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xzwx)+(r6.xyzx)).xyz;
    // 176: div r9.xz, l(1024.000000, 0.000000, 1024.000000, 0.000000), cb0[7].xxyx
    r9.xz = ((float4(1024.000000,0.000000,1024.000000,0.000000))/(source[7].xxyx)).xz;
    // 177: mad r10.xy, -cb0[7].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[7].zwzz
    r10.xy = ((-(source[7].xyxx))*(float4(0.500000,0.500000,0.000000,0.000000))+(source[7].zwzz)).xy;
    // 178: div r10.xy, r10.xyxx, cb0[7].xyxx
    r10.xy = ((r10.xyxx)/(source[7].xyxx)).xy;
    // 179: mad r9.xz, v4.xxyx, r9.xxzx, -r10.xxyx
    r9.xz = ((v4.xxyx)*(r9.xxzx)+(-(r10.xxyx))).xz;
    // 180: sample_b_indexable(texture2d)(float,float,float,float) r9.xz, r9.xzxx, t5.xzyw, s6, l(0.000000)
    r9.xz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 181: mul r1.z, r9.x, cb0[6].w
    r1.z = ((r9.xxxx)*(source[6].wwww)).z;
    // 182: add r10.xyz, -r6.xyzx, cb0[6].xyzx
    r10.xyz = ((-(r6.xyzx))+(source[6].xyzx)).xyz;
    // 183: mad r6.xyz, r1.zzzz, r10.xyzx, r6.xyzx
    r6.xyz = ((r1.zzzz)*(r10.xyzx)+(r6.xyzx)).xyz;
    // 184: mul r1.z, r9.z, cb0[8].w
    r1.z = ((r9.zzzz)*(source[8].wwww)).z;
    // 185: add r9.xzw, -r6.xxyz, cb0[8].xxyz
    r9.xzw = ((-(r6.xxyz))+(source[8].xxyz)).xzw;
    // 186: mad r6.xyz, r1.zzzz, r9.xzwx, r6.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xzwx)+(r6.xyzx)).xyz;
    // 187: div r9.xz, l(1024.000000, 0.000000, 1024.000000, 0.000000), cb0[10].xxyx
    r9.xz = ((float4(1024.000000,0.000000,1024.000000,0.000000))/(source[10].xxyx)).xz;
    // 188: mad r10.xy, -cb0[10].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[10].zwzz
    r10.xy = ((-(source[10].xyxx))*(float4(0.500000,0.500000,0.000000,0.000000))+(source[10].zwzz)).xy;
    // 189: div r10.xy, r10.xyxx, cb0[10].xyxx
    r10.xy = ((r10.xyxx)/(source[10].xyxx)).xy;
    // 190: mul r11.w, cb0[25].w, cb0[26].z
    r11.w = ((source[25].wwww)*(source[26].zzzz)).w;
    // 191: mad r11.y, cb0[25].w, cb0[26].x, r10.x
    r11.y = ((source[25].wwww)*(source[26].xxxx)+(r10.xxxx)).y;
    // 192: mov r10.zw, l(0,0,0.004000,-0.004000)
    r10.zw = (float4(asfloat(0u),asfloat(0u),0.004000,-0.004000)).zw;
    // 193: add r11.yz, r10.zzyz, r11.yywy
    r11.yz = ((r10.zzyz)+(r11.yywy)).yz;
    // 194: mad r11.yz, v4.xxyx, r9.xxzx, -r11.yyzy
    r11.yz = ((v4.xxyx)*(r9.xxzx)+(-(r11.yyzy))).yz;
    // 195: mul r1.z, cb0[25].w, cb0[27].x
    r1.z = ((source[25].wwww)*(source[27].xxxx)).z;
    // 196: mad r8.w, -cb0[25].w, cb0[27].x, l(1.000000)
    r8.w = ((-(source[25].wwww))*(source[27].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 197: mul r1.z, r1.z, l(-0.500000)
    r1.z = ((r1.zzzz)*(float4(-0.500000,-0.500000,-0.500000,-0.500000))).z;
    // 198: mad r11.yz, r8.wwww, r11.yyzy, -r1.zzzz
    r11.yz = ((r8.wwww)*(r11.yyzy)+(-(r1.zzzz))).yz;
    // 199: sample_b_indexable(texture2d)(float,float,float,float) r12.xyz, r11.yzyy, t6.xyzw, s7, l(0.000000)
    r12.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r11.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 200: mad r11.x, -cb0[25].w, cb0[26].x, r10.x
    r11.x = ((-(source[25].wwww))*(source[26].xxxx)+(r10.xxxx)).x;
    // 201: add r10.zw, r10.wwwy, r11.xxxw
    r10.zw = ((r10.wwwy)+(r11.xxxw)).zw;
    // 202: mad r10.zw, v4.xxxy, r9.xxxz, -r10.zzzw
    r10.zw = ((v4.xxxy)*(r9.xxxz)+(-(r10.zzzw))).zw;
    // 203: mad r11.yz, r8.wwww, r10.zzwz, -r1.zzzz
    r11.yz = ((r8.wwww)*(r10.zzwz)+(-(r1.zzzz))).yz;
    // 204: add r1.z, r8.w, -r11.y
    r1.z = ((r8.wwww)+(-(r11.yyyy))).z;
    // 205: add r11.x, r1.z, l(1.000000)
    r11.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 206: sample_b_indexable(texture2d)(float,float,float,float) r11.xyz, r11.xzxx, t6.xyzw, s7, l(0.000000)
    r11.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r11.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 207: add r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)+(r12.xyzx)).xyz;
    // 208: mul r11.xyz, r11.xyzx, cb0[9].wwww
    r11.xyz = ((r11.xyzx)*(source[9].wwww)).xyz;
    // 209: add r12.xyz, -r6.xyzx, cb0[9].xyzx
    r12.xyz = ((-(r6.xyzx))+(source[9].xyzx)).xyz;
    // 210: mad r6.xyz, r11.xyzx, r12.xyzx, r6.xyzx
    r6.xyz = ((r11.xyzx)*(r12.xyzx)+(r6.xyzx)).xyz;
    // 211: mad r10.xy, v4.xyxx, r9.xzxx, -r10.xyxx
    r10.xy = ((v4.xyxx)*(r9.xzxx)+(-(r10.xyxx))).xy;
    // 212: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r10.xyxx, t6.xywz, s7, l(0.000000)
    r1.z = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).z;
    // 213: add r10.z, -r10.x, l(1.990000)
    r10.z = ((-(r10.xxxx))+(float4(1.990000,1.990000,1.990000,1.990000))).z;
    // 214: sample_b_indexable(texture2d)(float,float,float,float) r8.w, r10.zyzz, t6.xyzw, s7, l(0.000000)
    r8.w = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r10.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 215: add r1.z, r1.z, r8.w
    r1.z = ((r1.zzzz)+(r8.wwww)).z;
    // 216: mul r1.z, r1.z, cb0[11].w
    r1.z = ((r1.zzzz)*(source[11].wwww)).z;
    // 217: add r9.xzw, -r6.xxyz, cb0[11].xxyz
    r9.xzw = ((-(r6.xxyz))+(source[11].xxyz)).xzw;
    // 218: mad r6.xyz, r1.zzzz, r9.xzwx, r6.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xzwx)+(r6.xyzx)).xyz;
    // 219: mul r1.z, r7.w, cb0[12].w
    r1.z = ((r7.wwww)*(source[12].wwww)).z;
    // 220: add r9.xzw, -r6.xxyz, cb0[12].xxyz
    r9.xzw = ((-(r6.xxyz))+(source[12].xxyz)).xzw;
    // 221: mad r6.xyz, r1.zzzz, r9.xzwx, r6.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xzwx)+(r6.xyzx)).xyz;
    // 222: add r7.xyz, -r6.xyzx, r7.xyzx
    r7.xyz = ((-(r6.xyzx))+(r7.xyzx)).xyz;
    // 223: mad r6.xyz, r1.yyyy, r7.xyzx, r6.xyzx
    r6.xyz = ((r1.yyyy)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 224: div r1.yz, l(0.000000, 1024.000000, 1024.000000, 0.000000), cb0[13].xxyx
    r1.yz = ((float4(0.000000,1024.000000,1024.000000,0.000000))/(source[13].xxyx)).yz;
    // 225: mad r7.xy, -cb0[13].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), cb0[13].zwzz
    r7.xy = ((-(source[13].xyxx))*(float4(0.500000,0.500000,0.000000,0.000000))+(source[13].zwzz)).xy;
    // 226: div r7.xy, r7.xyxx, cb0[13].xyxx
    r7.xy = ((r7.xyxx)/(source[13].xyxx)).xy;
    // 227: mad r7.zw, cb0[28].wwww, cb0[14].xxxy, r7.xxxy
    r7.zw = ((source[28].wwww)*(source[14].xxxy)+(r7.xxxy)).zw;
    // 228: mad r7.zw, v4.xxxy, r1.yyyz, -r7.zzzw
    r7.zw = ((v4.xxxy)*(r1.yyyz)+(-(r7.zzzw))).zw;
    // 229: add r7.zw, r7.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r7.zw = ((r7.zzzw)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 230: mul r8.w, cb0[28].w, cb0[29].x
    r8.w = ((source[28].wwww)*(source[29].xxxx)).w;
    // 231: mul r8.w, r8.w, l(6.283185)
    r8.w = ((r8.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 232: sincos r9.x, r10.x, r8.w
    r9.x = (sin(r8.wwww)).x; r10.x = (cos(r8.wwww)).x;
    // 233: mov r11.x, -r9.x
    r11.x = (-(r9.xxxx)).x;
    // 234: mov r11.y, r10.x
    r11.y = (r10.xxxx).y;
    // 235: dp2 r10.x, r7.wzww, r11.xyxx
    r10.x = (dot((r7.wzww).xy,(r11.xyxx).xy).xxxx).x;
    // 236: mov r11.z, r9.x
    r11.z = (r9.xxxx).z;
    // 237: dp2 r10.y, r7.wzww, r11.yzyy
    r10.y = (dot((r7.wzww).xy,(r11.yzyy).xy).xxxx).y;
    // 238: add r7.zw, r10.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r7.zw = ((r10.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 239: mul r8.w, cb0[28].w, cb0[30].x
    r8.w = ((source[28].wwww)*(source[30].xxxx)).w;
    // 240: mad r9.x, -cb0[28].w, cb0[30].x, l(1.000000)
    r9.x = ((-(source[28].wwww))*(source[30].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 241: mul r8.w, r8.w, l(-0.500000)
    r8.w = ((r8.wwww)*(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 242: mad r7.zw, r9.xxxx, r7.zzzw, -r8.wwww
    r7.zw = ((r9.xxxx)*(r7.zzzw)+(-(r8.wwww))).zw;
    // 243: add r7.xy, r7.xyxx, cb0[15].xyxx
    r7.xy = ((r7.xyxx)+(source[15].xyxx)).xy;
    // 244: mad r1.yz, v4.xxyx, r1.yyzy, -r7.xxyx
    r1.yz = ((v4.xxyx)*(r1.yyzy)+(-(r7.xxyx))).yz;
    // 245: add r1.yz, r1.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((r1.yyzy)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 246: mul r7.x, cb0[30].z, l(6.283185)
    r7.x = ((source[30].zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 247: sincos r7.x, r10.x, r7.x
    r7.x = (sin(r7.xxxx)).x; r10.x = (cos(r7.xxxx)).x;
    // 248: mov r11.x, -r7.x
    r11.x = (-(r7.xxxx)).x;
    // 249: mov r11.y, r10.x
    r11.y = (r10.xxxx).y;
    // 250: dp2 r10.x, r1.zyzz, r11.xyxx
    r10.x = (dot((r1.zyzz).xy,(r11.xyxx).xy).xxxx).x;
    // 251: mov r11.z, r7.x
    r11.z = (r7.xxxx).z;
    // 252: dp2 r10.y, r1.zyzz, r11.yzyy
    r10.y = (dot((r1.zyzz).xy,(r11.yzyy).xy).xxxx).y;
    // 253: add r10.xy, r10.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r10.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 254: add r10.z, -r10.x, l(1.000000)
    r10.z = ((-(r10.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 255: mad r1.yz, r9.xxxx, r10.zzyz, -r8.wwww
    r1.yz = ((r9.xxxx)*(r10.zzyz)+(-(r8.wwww))).yz;
    // 256: add r7.xy, -r7.zwzz, r1.yzyy
    r7.xy = ((-(r7.zwzz))+(r1.yzyy)).xy;
    // 257: mul r7.xy, r7.xyxx, cb0[30].wwww
    r7.xy = ((r7.xyxx)*(source[30].wwww)).xy;
    // 258: mad r7.xy, cb0[28].wwww, r7.xyxx, r7.zwzz
    r7.xy = ((source[28].wwww)*(r7.xyxx)+(r7.zwzz)).xy;
    // 259: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r7.xyxx, t7.xyzw, s8, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 260: mul r9.xzw, r10.xxyz, r10.wwww
    r9.xzw = ((r10.xxyz)*(r10.wwww)).xzw;
    // 261: mad r10.xyz, r10.wwww, cb0[16].xyzx, -r9.xzwx
    r10.xyz = ((r10.wwww)*(source[16].xyzx)+(-(r9.xzwx))).xyz;
    // 262: mad r10.xyz, cb0[31].xxxx, r10.xyzx, r9.xzwx
    r10.xyz = ((source[31].xxxx)*(r10.xyzx)+(r9.xzwx)).xyz;
    // 263: add r7.xy, -r1.yzyy, r7.zwzz
    r7.xy = ((-(r1.yzyy))+(r7.zwzz)).xy;
    // 264: mad r1.yz, cb0[30].wwww, r7.xxyx, r1.yyzy
    r1.yz = ((source[30].wwww)*(r7.xxyx)+(r1.yyzy)).yz;
    // 265: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r1.yzyy, t7.xyzw, s8, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 266: mad r7.xyz, r7.wwww, r7.xyzx, r9.xzwx
    r7.xyz = ((r7.wwww)*(r7.xyzx)+(r9.xzwx)).xyz;
    // 267: add r1.y, r7.w, r10.w
    r1.y = ((r7.wwww)+(r10.wwww)).y;
    // 268: mad r9.xzw, r1.yyyy, cb0[16].xxyz, -r7.xxyz
    r9.xzw = ((r1.yyyy)*(source[16].xxyz)+(-(r7.xxyz))).xzw;
    // 269: mad r7.xyz, cb0[31].xxxx, r9.xzwx, r7.xyzx
    r7.xyz = ((source[31].xxxx)*(r9.xzwx)+(r7.xyzx)).xyz;
    // 270: add r7.xyz, -r10.xyzx, r7.xyzx
    r7.xyz = ((-(r10.xyzx))+(r7.xyzx)).xyz;
    // 271: mad r7.xyz, cb0[31].yyyy, r7.xyzx, r10.xyzx
    r7.xyz = ((source[31].yyyy)*(r7.xyzx)+(r10.xyzx)).xyz;
    // 272: mov_sat r1.y, r1.y
    r1.y = (saturate(r1.yyyy)).y;
    // 273: add r1.y, -r10.w, r1.y
    r1.y = ((-(r10.wwww))+(r1.yyyy)).y;
    // 274: mad r1.y, cb0[31].y, r1.y, r10.w
    r1.y = ((source[31].yyyy)*(r1.yyyy)+(r10.wwww)).y;
    // 275: mad r1.z, r1.y, cb0[16].w, -r1.y
    r1.z = ((r1.yyyy)*(source[16].wwww)+(-(r1.yyyy))).z;
    // 276: mad r1.y, cb0[31].x, r1.z, r1.y
    r1.y = ((source[31].xxxx)*(r1.zzzz)+(r1.yyyy)).y;
    // 277: add r7.xyz, -r6.xyzx, r7.xyzx
    r7.xyz = ((-(r6.xyzx))+(r7.xyzx)).xyz;
    // 278: mad r6.xyz, r1.yyyy, r7.xyzx, r6.xyzx
    r6.xyz = ((r1.yyyy)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 279: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t8.xyzw, s9, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 280: mul r9.xzw, r6.xxyz, r7.xxyz
    r9.xzw = ((r6.xxyz)*(r7.xxyz)).xzw;
    // 281: mul r0.xyz, r0.xyzx, cb0[32].wwww
    r0.xyz = ((r0.xyzx)*(source[32].wwww)).xyz;
    // 282: mul r10.xyz, r0.xyzx, r9.xzwx
    r10.xyz = ((r0.xyzx)*(r9.xzwx)).xyz;
    // 283: max r1.y, |r1.x|, l(0.000001)
    r1.y = (max(abs(r1.xxxx),float4(0.000001,0.000001,0.000001,0.000001))).y;
    // 284: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 285: mul r1.y, r1.y, l(0.454545)
    r1.y = ((r1.yyyy)*(float4(0.454545,0.454545,0.454545,0.454545))).y;
    // 286: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 287: dp3 r1.y, r1.yyyy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r1.yyyy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 288: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 289: mul r1.y, r1.y, cb0[33].w
    r1.y = ((r1.yyyy)*(source[33].wwww)).y;
    // 290: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 291: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 292: mad r1.z, -r1.y, r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 293: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 294: div r1.z, cb0[34].x, r1.z
    r1.z = ((source[34].xxxx)/(r1.zzzz)).z;
    // 295: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 296: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 297: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 298: mul r1.z, r1.z, cb0[34].y
    r1.z = ((r1.zzzz)*(source[34].yyyy)).z;
    // 299: mad r0.xyz, r1.xxxx, r0.xyzx, -r10.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(-(r10.xyzx))).xyz;
    // 300: mad r0.xyz, r1.zzzz, r0.xyzx, r10.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r10.xyzx)).xyz;
    // 301: mad r0.xyz, r1.wwww, r0.xyzx, -r9.xzwx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(-(r9.xzwx))).xyz;
    // 302: mad r0.xyz, r1.yyyy, r0.xyzx, r9.xzwx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(r9.xzwx)).xyz;
    // 303: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 304: add r9.xzw, -r0.xxyz, r1.zzzz
    r9.xzw = ((-(r0.xxyz))+(r1.zzzz)).xzw;
    // 305: mad r0.xyz, cb0[34].zzzz, r9.xzwx, r0.xyzx
    r0.xyz = ((source[34].zzzz)*(r9.xzwx)+(r0.xyzx)).xyz;
    // 306: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 307: add r9.xzw, -r0.xxyz, r1.zzzz
    r9.xzw = ((-(r0.xxyz))+(r1.zzzz)).xzw;
    // 308: mad r0.xyz, cb0[34].wwww, r9.xzwx, r0.xyzx
    r0.xyz = ((source[34].wwww)*(r9.xzwx)+(r0.xyzx)).xyz;
    // 309: mad r9.xzw, cb0[17].wwww, cb0[17].xxyz, l(1.000000, 0.000000, 1.000000, 1.000000)
    r9.xzw = ((source[17].wwww)*(source[17].xxyz)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 310: mad r10.xyz, cb0[18].wwww, cb0[18].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[18].wwww)*(source[18].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 311: mul r9.xzw, r9.xxzw, r10.xxyz
    r9.xzw = ((r9.xxzw)*(r10.xxyz)).xzw;
    // 312: mul r0.xyz, r0.xyzx, r9.xzwx
    r0.xyz = ((r0.xyzx)*(r9.xzwx)).xyz;
    // 313: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 314: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 315: mul r2.w, r2.w, cb0[35].x
    r2.w = ((r2.wwww)*(source[35].xxxx)).w;
    // 316: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 317: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 318: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 319: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 320: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 321: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 322: add r7.w, -r2.w, cb0[2].x
    r7.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 323: mul r10.z, r7.w, l(0.125000)
    r10.z = ((r7.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 324: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 325: mul r10.y, cb0[2].y, cb0[19].y
    r10.y = ((source[2].yyyy)*(source[19].yyyy)).y;
    // 326: frc r7.w, v4.x
    r7.w = (frac(v4.xxxx)).w;
    // 327: mul r11.x, r7.w, l(0.125000)
    r11.x = ((r7.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 328: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 329: add r9.xz, r10.xxyx, r11.xxyx
    r9.xz = ((r10.xxyx)+(r11.xxyx)).xz;
    // 330: add r9.xz, r9.xxzx, r10.zzwz
    r9.xz = ((r9.xxzx)+(r10.zzwz)).xz;
    // 331: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r9.xzxx, t10.xyzw, s11, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture10.SampleBias(SourceCharacterSampler, (r9.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 332: mul r9.xzw, r1.zzzz, r10.xxyz
    r9.xzw = ((r1.zzzz)*(r10.xxyz)).xzw;
    // 333: mul r1.z, r2.w, r10.w
    r1.z = ((r2.wwww)*(r10.wwww)).z;
    // 334: mad r9.xzw, r9.xxzw, l(2.000000, 0.000000, 2.000000, 2.000000), -r0.xxyz
    r9.xzw = ((r9.xxzw)*(float4(2.000000,0.000000,2.000000,2.000000))+(-(r0.xxyz))).xzw;
    // 335: mad r0.xyz, r1.zzzz, r9.xzwx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r9.xzwx)+(r0.xyzx)).xyz;
    // 336: mul r1.z, cb0[20].y, cb0[35].x
    r1.z = ((source[20].yyyy)*(source[35].xxxx)).z;
    // 337: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 338: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 339: mul r10.y, r1.z, l(0.020000)
    r10.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 340: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 341: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 342: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 343: mad r3.xy, cb0[20].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[20].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 344: mul r2.w, cb0[20].x, l(0.001000)
    r2.w = ((source[20].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 345: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 346: mad r3.xy, r2.wwww, r3.xyxx, r10.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r10.xyxx)).xy;
    // 347: dp2 r2.w, cb0[21].xyxx, r3.xyxx
    r2.w = (dot((source[21].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 348: dp2 r3.y, cb0[22].xyxx, r3.xyxx
    r3.y = (dot((source[22].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 349: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 350: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 351: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t10.xyzw, s11, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture10.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 352: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 353: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 354: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 355: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 356: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 357: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 358: mul r9.xzw, r3.xxyz, cb0[20].zzzz
    r9.xzw = ((r3.xxyz)*(source[20].zzzz)).xzw;
    // 359: dp3 r1.z, r9.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r9.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 360: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 361: mad r3.xyz, cb0[20].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[20].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 362: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 363: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 364: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 365: mad r5.xyz, r6.xyzx, r7.xyzx, -r1.xxxx
    r5.xyz = ((r6.xyzx)*(r7.xyzx)+(-(r1.xxxx))).xyz;
    // 366: mad r1.xyz, r1.yyyy, r5.xyzx, r1.xxxx
    r1.xyz = ((r1.yyyy)*(r5.xyzx)+(r1.xxxx)).xyz;
    // 367: dp3 r2.x, r2.xyzx, r8.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 368: mul r2.z, r0.w, cb0[36].w
    r2.z = ((r0.wwww)*(source[36].wwww)).z;
    // 369: max r0.w, r2.z, l(0.000000)
    r0.w = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 370: min r0.w, r0.w, cb0[36].w
    r0.w = (min(r0.wwww,source[36].wwww)).w;
    // 371: add r2.w, -cb0[37].z, cb0[37].y
    r2.w = ((-(source[37].zzzz))+(source[37].yyyy)).w;
    // 372: mad r2.w, cb0[37].x, r2.w, cb0[37].z
    r2.w = ((source[37].xxxx)*(r2.wwww)+(source[37].zzzz)).w;
    // 373: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 374: lt r3.w, |r0.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 375: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 376: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 377: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 378: movc r2.y, r3.w, l(0), r0.w
    r2.y = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 379: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r2.xyxx, t11.xyzw, s12, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture11.SampleBias(SourceCharacterLookupSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 380: add r0.w, cb0[37].w, -cb0[38].x
    r0.w = ((source[37].wwww)+(-(source[38].xxxx))).w;
    // 381: mad r0.w, r9.y, r0.w, cb0[38].x
    r0.w = ((r9.yyyy)*(r0.wwww)+(source[38].xxxx)).w;
    // 382: add r2.y, -cb0[38].z, cb0[38].y
    r2.y = ((-(source[38].zzzz))+(source[38].yyyy)).y;
    // 383: mad r0.w, r0.w, r2.y, cb0[38].z
    r0.w = ((r0.wwww)*(r2.yyyy)+(source[38].zzzz)).w;
    // 384: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xzxx, t11.xyzw, s12, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture11.SampleBias(SourceCharacterLookupSampler, (r2.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 385: mad r2.xyz, r0.wwww, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 386: add r0.w, -cb0[38].w, l(2.000000)
    r0.w = ((-(source[38].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 387: mad r0.w, r1.w, r0.w, cb0[38].w
    r0.w = ((r1.wwww)*(r0.wwww)+(source[38].wwww)).w;
    // 388: mul r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // 389: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 390: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 391: mul r2.xyz, r2.xyzx, cb0[39].xxxx
    r2.xyz = ((r2.xyzx)*(source[39].xxxx)).xyz;
    // 392: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 393: mul r2.xyz, r2.xyzx, r4.yyyy
    r2.xyz = ((r2.xyzx)*(r4.yyyy)).xyz;
    // 394: mul r2.xyz, r2.xyzx, r6.wwww
    r2.xyz = ((r2.xyzx)*(r6.wwww)).xyz;
    // 395: min r2.xyz, r2.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 396: mul r2.xyz, r2.xyzx, cb0[39].yyyy
    r2.xyz = ((r2.xyzx)*(source[39].yyyy)).xyz;
    // 397: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 398: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 399: mad r0.xyz, r4.xzwx, r0.xyzx, r1.xyzx
    r0.xyz = ((r4.xzwx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 400: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 401: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 402: mul o0.xyz, r0.xyzx, cb0[40].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[40].xyzx)).xyz;
    // 403: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 404: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 405: ret
    return output;
}

// source.character.static-map-color-mask.v1 / source program cd993bbcb1d322469e313d7f20ff8fe4
