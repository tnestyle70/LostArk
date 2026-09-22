SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight160(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].z=(g_SourceCharacterTime.xxxx).x;
    source[19].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[20].x=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[20].y=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[20].z=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[22]=float4(input.lightColor,1.0);
    source[28].x=1.0;
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
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[28].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[28].yyyy)) * 0xffffffffu)).w;
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
    // 11: mul r3.xyzw, r2.yyyy, cb0[24].xyzw
    r3.xyzw = ((r2.yyyy)*(source[24].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[23].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[23].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[25].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[25].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[26].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[26].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s4
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[27].wwzw
    r4.yz = (source[27].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s4
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s4
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[27].zwzz
    r4.xy = ((r2.xyxx)+(source[27].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s4
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[27].xyxx
    r2.xy = ((r2.xyxx)*(source[27].xyxx)).xy;
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
    // 33: mul r2.xyz, r0.wwww, cb0[28].xxxx
    r2.xyz = ((r0.wwww)*(source[28].xxxx)).xyz;
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
    // 44: mul r5.xyz, r5.xyzx, cb0[14].xxxx
    r5.xyz = ((r5.xyzx)*(source[14].xxxx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r1.w, cb0[14].y, l(-3.500000), l(5.000000)
    r1.w = ((source[14].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 47: mul r1.w, r1.w, cb0[15].x
    r1.w = ((r1.wwww)*(source[15].xxxx)).w;
    // 48: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: add r3.w, -r2.w, v4.z
    r3.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 50: mad r2.w, cb0[15].y, r3.w, r2.w
    r2.w = ((source[15].yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 51: mul r3.w, r2.w, cb0[15].z
    r3.w = ((r2.wwww)*(source[15].zzzz)).w;
    // 52: mad r2.w, r3.w, l(0.750000), r2.w
    r2.w = ((r3.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 53: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 54: mad_sat r2.w, cb0[16].x, r2.w, r2.w
    r2.w = (saturate((source[16].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t1.xyzw, s0, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 56: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 58: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 61: add r7.z, r3.w, l(0.000010)
    r7.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mad r3.w, r7.x, r6.x, l(0.200000)
    r3.w = ((r7.xxxx)*(r6.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 63: add r4.w, -r6.y, l(1.000000)
    r4.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: add r4.w, -r3.w, r4.w
    r4.w = ((-(r3.wwww))+(r4.wwww)).w;
    // 65: mad r5.w, cb0[16].z, r4.w, r3.w
    r5.w = ((source[16].zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 66: mul r7.w, cb0[15].w, l(0.700000)
    r7.w = ((source[15].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 67: add r5.w, -r2.w, r5.w
    r5.w = ((-(r2.wwww))+(r5.wwww)).w;
    // 68: mad r5.w, r7.w, r5.w, r2.w
    r5.w = ((r7.wwww)*(r5.wwww)+(r2.wwww)).w;
    // 69: div r5.w, r5.w, cb0[16].y
    r5.w = ((r5.wwww)/(source[16].yyyy)).w;
    // 70: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r5.w, r1.w, r5.w
    r5.w = ((r1.wwww)*(r5.wwww)).w;
    // 72: mul r5.w, r5.w, l(4.000000)
    r5.w = ((r5.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 73: add r8.x, v4.w, l(0.500000)
    r8.x = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 74: round_ni r8.x, r8.x
    r8.x = (floor(r8.xxxx)).x;
    // 75: mul_sat r5.w, r5.w, r8.x
    r5.w = (saturate((r5.wwww)*(r8.xxxx))).w;
    // 76: mad r3.w, cb0[17].x, r4.w, r3.w
    r3.w = ((source[17].xxxx)*(r4.wwww)+(r3.wwww)).w;
    // 77: add r3.w, -r2.w, r3.w
    r3.w = ((-(r2.wwww))+(r3.wwww)).w;
    // 78: mad r2.w, r7.w, r3.w, r2.w
    r2.w = ((r7.wwww)*(r3.wwww)+(r2.wwww)).w;
    // 79: div r2.w, r2.w, cb0[16].w
    r2.w = ((r2.wwww)/(source[16].wwww)).w;
    // 80: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 82: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 83: add r2.w, -r8.x, l(1.000000)
    r2.w = ((-(r8.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 85: add r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)+(r5.wwww)).w;
    // 86: add r1.w, -r6.y, r1.w
    r1.w = ((-(r6.yyyy))+(r1.wwww)).w;
    // 87: mad r1.w, cb0[17].y, r1.w, r6.y
    r1.w = ((source[17].yyyy)*(r1.wwww)+(r6.yyyy)).w;
    // 88: mad r5.xyz, r1.wwww, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 89: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: add r8.xyz, -r5.xyzx, r1.wwww
    r8.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 91: mad r5.xyz, cb0[17].zzzz, r8.xyzx, r5.xyzx
    r5.xyz = ((source[17].zzzz)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 92: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r8.xyz, -r5.xyzx, r1.wwww
    r8.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 94: mad r5.xyz, cb0[17].wwww, r8.xyzx, r5.xyzx
    r5.xyz = ((source[17].wwww)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 95: mad r8.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 96: mad r9.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 97: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 98: mul r9.xyz, r5.xyzx, r8.xyzx
    r9.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 99: mul r10.xyz, r6.xxxx, r9.xyzx
    r10.xyz = ((r6.xxxx)*(r9.xyzx)).xyz;
    // 100: mad r9.xyz, r6.xxxx, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = ((r6.xxxx)*(r9.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 101: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 102: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 103: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 104: mul r9.xyz, r9.xyzx, cb0[8].xyzx
    r9.xyz = ((r9.xyzx)*(source[8].xyzx)).xyz;
    // 105: add r1.w, -|r3.z|, l(1.000000)
    r1.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: dp3 r2.w, r7.xyzx, r7.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 107: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 108: div r7.xyz, r7.xyzx, r2.wwww
    r7.xyz = ((r7.xyzx)/(r2.wwww)).xyz;
    // 109: dp3 r2.w, r7.xyzx, r3.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 110: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 112: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 113: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 114: mul r1.w, r1.w, cb0[19].y
    r1.w = ((r1.wwww)*(source[19].yyyy)).w;
    // 115: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 116: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 117: mul r2.w, cb0[9].z, l(1.500000)
    r2.w = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 118: add r3.w, -cb0[9].w, l(1.000000)
    r3.w = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: mul r3.w, r3.w, cb0[19].z
    r3.w = ((r3.wwww)*(source[19].zzzz)).w;
    // 120: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 121: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 122: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 124: mad r2.w, r2.w, l(0.500000), cb0[9].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).w;
    // 125: frc r3.w, cb0[9].x
    r3.w = (frac(source[9].xxxx)).w;
    // 126: add r4.w, -r3.w, cb0[9].x
    r4.w = ((-(r3.wwww))+(source[9].xxxx)).w;
    // 127: mul r11.z, r4.w, l(0.125000)
    r11.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 128: mov r11.xw, l(0,0,0,0)
    r11.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 129: mul r11.y, cb0[9].y, cb0[10].y
    r11.y = ((source[9].yyyy)*(source[10].yyyy)).y;
    // 130: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 131: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 132: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 133: add r6.xy, r6.xyxx, r11.xyxx
    r6.xy = ((r6.xyxx)+(r11.xyxx)).xy;
    // 134: add r6.xy, r6.xyxx, r11.zwzz
    r6.xy = ((r6.xyxx)+(r11.zwzz)).xy;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 136: mul r11.xyz, r2.wwww, r11.xyzx
    r11.xyz = ((r2.wwww)*(r11.xyzx)).xyz;
    // 137: mul r2.w, r3.w, r11.w
    r2.w = ((r3.wwww)*(r11.wwww)).w;
    // 138: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 139: mad r10.xyz, r2.wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((r2.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 140: mul r2.w, cb0[11].y, cb0[19].z
    r2.w = ((source[11].yyyy)*(source[19].zzzz)).w;
    // 141: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 142: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 143: mul r6.y, r2.w, l(0.020000)
    r6.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 144: add r11.xyzw, r1.yzxy, -cb0[1].yzxy
    r11.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 145: add r11.xy, -r11.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r11.xy = ((-(r11.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 146: add r11.xy, -r11.zwzz, r11.xyxx
    r11.xy = ((-(r11.zwzz))+(r11.xyxx)).xy;
    // 147: mad r11.xy, cb0[11].wwww, r11.xyxx, r11.zwzz
    r11.xy = ((source[11].wwww)*(r11.xyxx)+(r11.zwzz)).xy;
    // 148: mul r3.w, cb0[11].x, l(0.001000)
    r3.w = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 149: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 150: mad r6.xy, r3.wwww, r11.xyxx, r6.xyxx
    r6.xy = ((r3.wwww)*(r11.xyxx)+(r6.xyxx)).xy;
    // 151: dp2 r3.w, cb0[12].xyxx, r6.xyxx
    r3.w = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 152: dp2 r6.y, cb0[13].xyxx, r6.xyxx
    r6.y = (dot((source[13].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 153: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 154: mul r6.x, r3.w, l(0.125000)
    r6.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 155: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 156: mul r3.w, r11.w, l(0.900000)
    r3.w = ((r11.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 157: mad r11.xyz, r11.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r10.xyzx
    r11.xyz = ((r11.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r10.xyzx))).xyz;
    // 158: mad r11.xyz, r3.wwww, r11.xyzx, r10.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 159: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 161: mul_sat r11.xyz, r11.xyzx, r2.wwww
    r11.xyz = (saturate((r11.xyzx)*(r2.wwww))).xyz;
    // 162: mul r12.xyz, r11.xyzx, cb0[11].zzzz
    r12.xyz = ((r11.xyzx)*(source[11].zzzz)).xyz;
    // 163: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 164: mul r2.w, r2.w, l(3.000000)
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 165: mad r11.xyz, cb0[11].zzzz, r11.xyzx, -r10.xyzx
    r11.xyz = ((source[11].zzzz)*(r11.xyzx)+(-(r10.xyzx))).xyz;
    // 166: mad r10.xyz, r2.wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((r2.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 167: dp3 r2.w, r7.xyzx, r4.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 168: max r3.w, r2.w, l(0.000000)
    r3.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 169: min r4.w, r3.w, l(1.000000)
    r4.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: mul r11.xyz, r2.xyzx, r4.wwww
    r11.xyz = ((r2.xyzx)*(r4.wwww)).xyz;
    // 171: mad r12.xyz, -r4.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r4.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mad r11.yzw, cb0[20].wwww, r12.xxyz, r11.xxyz
    r11.yzw = ((source[20].wwww)*(r12.xxyz)+(r11.xxyz)).yzw;
    // 173: dp3 r4.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 174: add r12.xyz, r4.wwww, -cb0[2].xyzx
    r12.xyz = ((r4.wwww)+(-(source[2].xyzx))).xyz;
    // 175: mad r12.xyz, cb0[17].zzzz, r12.xyzx, cb0[2].xyzx
    r12.xyz = ((source[17].zzzz)*(r12.xyzx)+(source[2].xyzx)).xyz;
    // 176: dp3 r4.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 177: add r13.xyz, -r12.xyzx, r4.wwww
    r13.xyz = ((-(r12.xyzx))+(r4.wwww)).xyz;
    // 178: mad r12.xyz, cb0[17].wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((source[17].wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 179: mul r13.xyz, r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r13.xyz = ((r12.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 180: mad r14.xyz, -r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 182: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 183: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 184: mul r13.xyz, r13.xyzx, r2.wwww
    r13.xyz = ((r13.xyzx)*(r2.wwww)).xyz;
    // 185: mad r11.yzw, r11.yyzw, r14.xxyz, r13.xxyz
    r11.yzw = ((r11.yyzw)*(r14.xxyz)+(r13.xxyz)).yzw;
    // 186: mov_sat r2.w, r4.z
    r2.w = (saturate(r4.zzzz)).w;
    // 187: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 188: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 189: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: add r13.xyz, -r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r12.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 191: mad r12.xyz, r2.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r2.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 192: mad r12.xyz, r11.yzwy, l(0.500000, 0.500000, 0.500000, 0.000000), r12.xyzx
    r12.xyz = ((r11.yzwy)*(float4(0.500000,0.500000,0.500000,0.000000))+(r12.xyzx)).xyz;
    // 193: mul_sat r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = (saturate((r10.xyzx)*(r12.xyzx))).xyz;
    // 194: max r2.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 195: mad r5.xyz, r5.xyzx, r8.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r5.xyz = ((r5.xyzx)*(r8.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 196: dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 197: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 198: div r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)/(r2.wwww)).xyz;
    // 199: mul r5.xyz, r5.xyzx, r6.zzzz
    r5.xyz = ((r5.xyzx)*(r6.zzzz)).xyz;
    // 200: mov_sat r2.w, r3.z
    r2.w = (saturate(r3.zzzz)).w;
    // 201: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 202: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 203: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 204: mul r5.xyz, r5.xyzx, cb0[18].xxxx
    r5.xyz = ((r5.xyzx)*(source[18].xxxx)).xyz;
    // 205: max r2.w, r11.x, l(0.500000)
    r2.w = (max(r11.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 206: min r2.xyzw, r2.xyzw, l(1.000000, 1.000000, 1.000000, 1.000000)
    r2.xyzw = (min(r2.xyzw,float4(1.000000,1.000000,1.000000,1.000000))).xyzw;
    // 207: dp3 r4.w, r0.xyzx, r7.xyzx
    r4.w = (dot((r0.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 208: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 209: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 210: mul r4.xyz, r3.zzzz, r1.xyzx
    r4.xyz = ((r3.zzzz)*(r1.xyzx)).xyz;
    // 211: mad r1.xyz, r4.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 212: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 213: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 214: div r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)/(r0.yyyy)).y;
    // 215: add r0.y, r0.y, cb0[6].z
    r0.y = ((r0.yyyy)+(source[6].zzzz)).y;
    // 216: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 217: add r0.x, r0.x, r4.w
    r0.x = ((r0.xxxx)+(r4.wwww)).x;
    // 218: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 219: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 220: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 221: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 222: mad r0.y, cb0[18].w, l(4.500000), l(0.500000)
    r0.y = ((source[18].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 223: mul r0.y, r0.y, cb0[21].x
    r0.y = ((r0.yyyy)*(source[21].xxxx)).y;
    // 224: mul r0.y, r0.y, l(0.050000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 225: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 226: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 227: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 228: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 229: mul r0.x, r2.w, r0.x
    r0.x = ((r2.wwww)*(r0.xxxx)).x;
    // 230: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 231: mad r0.yzw, v5.xxyz, r0.wwww, r3.xxyz
    r0.yzw = ((v5.xxyz)*(r0.wwww)+(r3.xxyz)).yzw;
    // 232: dp3 r0.y, r0.yzwy, r0.yzwy
    r0.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 233: mad r0.y, r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 234: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 235: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 236: mul r0.x, r0.x, cb0[21].y
    r0.x = ((r0.xxxx)*(source[21].yyyy)).x;
    // 237: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 238: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 239: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 240: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 241: mad r0.xyz, r10.xyzx, r11.yzwy, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r11.yzwy)+(r0.xyzx)).xyz;
    // 242: mad r0.xyz, r1.wwww, r9.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r9.xyzx)+(r0.xyzx)).xyz;
    // 243: mul r1.xyz, r3.wwww, cb2[3].xyzx
    r1.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // 244: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 245: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 246: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t4.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 247: dp3 r0.x, r0.xyzx, cb0[7].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[7].xyzx).xyz).xxxx).x;
    // 248: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[7].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[7].xyzx).xyz).xxxx)).y;
    // 249: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 250: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 251: mul r0.x, r0.x, r6.w
    r0.x = ((r0.xxxx)*(r6.wwww)).x;
    // 252: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 253: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 254: ret
    return output;
}

// source.character.equipment-native-161.v1 / source program b50a7a6517571a448c81c96ff5d72f07
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight161(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].z=(g_SourceCharacterTime.xxxx).x;
    source[19].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[22]=float4(input.lightColor,1.0);
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: dp3 r0.x, r0.xyzx, cb0[12].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[12].xyzx).xyz).xxxx).x;
    // 3: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[12].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[12].xyzx).xyz).xxxx)).y;
    // 4: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 5: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 7: add_sat r0.y, r1.w, -cb0[20].w
    r0.y = (saturate((r1.wwww)+(-(source[20].wwww)))).y;
    // 8: mad r0.x, r0.y, r0.x, l(-0.001000)
    r0.x = ((r0.yyyy)*(r0.xxxx)+(float4(-0.001000,-0.001000,-0.001000,-0.001000))).x;
    // 9: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 11: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 12: add r0.x, v4.w, l(0.500000)
    r0.x = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 13: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 14: add r0.z, -v4.z, l(1.000000)
    r0.z = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 15: add r0.w, -r0.z, v4.z
    r0.w = ((-(r0.zzzz))+(v4.zzzz)).w;
    // 16: mad r0.z, cb0[15].y, r0.w, r0.z
    r0.z = ((source[15].yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 17: mul r0.w, r0.z, cb0[15].z
    r0.w = ((r0.zzzz)*(source[15].zzzz)).w;
    // 18: mad r0.z, r0.w, l(0.750000), r0.z
    r0.z = ((r0.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.zzzz)).z;
    // 19: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 20: mad_sat r0.z, cb0[16].x, r0.z, r0.z
    r0.z = (saturate((source[16].xxxx)*(r0.zzzz)+(r0.zzzz))).z;
    // 21: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 23: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mad r1.w, r2.x, r1.x, l(0.200000)
    r1.w = ((r2.xxxx)*(r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 25: add r0.w, r0.w, -r1.w
    r0.w = ((r0.wwww)+(-(r1.wwww))).w;
    // 26: mad r2.w, cb0[17].x, r0.w, r1.w
    r2.w = ((source[17].xxxx)*(r0.wwww)+(r1.wwww)).w;
    // 27: mad r0.w, cb0[16].z, r0.w, r1.w
    r0.w = ((source[16].zzzz)*(r0.wwww)+(r1.wwww)).w;
    // 28: add r0.w, -r0.z, r0.w
    r0.w = ((-(r0.zzzz))+(r0.wwww)).w;
    // 29: add r1.w, -r0.z, r2.w
    r1.w = ((-(r0.zzzz))+(r2.wwww)).w;
    // 30: mul r2.w, cb0[15].w, l(0.700000)
    r2.w = ((source[15].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 31: mad r1.w, r2.w, r1.w, r0.z
    r1.w = ((r2.wwww)*(r1.wwww)+(r0.zzzz)).w;
    // 32: mad r0.z, r2.w, r0.w, r0.z
    r0.z = ((r2.wwww)*(r0.wwww)+(r0.zzzz)).z;
    // 33: div r0.z, r0.z, cb0[16].y
    r0.z = ((r0.zzzz)/(source[16].yyyy)).z;
    // 34: div r0.w, r1.w, cb0[16].w
    r0.w = ((r1.wwww)/(source[16].wwww)).w;
    // 35: add r0.yzw, -r0.xxzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r0.yzw = ((-(r0.xxzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 36: mad r1.w, cb0[14].y, l(-3.500000), l(5.000000)
    r1.w = ((source[14].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 37: mul r1.w, r1.w, cb0[15].x
    r1.w = ((r1.wwww)*(source[15].xxxx)).w;
    // 38: mul r0.zw, r0.zzzw, r1.wwww
    r0.zw = ((r0.zzzw)*(r1.wwww)).zw;
    // 39: mul r0.z, r0.z, l(4.000000)
    r0.z = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).z;
    // 40: mul_sat r0.x, r0.x, r0.z
    r0.x = (saturate((r0.xxxx)*(r0.zzzz))).x;
    // 41: mul r0.z, r0.w, l(4.000000)
    r0.z = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).z;
    // 42: mul_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)*(r0.zzzz))).y;
    // 43: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 44: add r0.x, -r1.y, r0.x
    r0.x = ((-(r1.yyyy))+(r0.xxxx)).x;
    // 45: mad r0.x, cb0[17].y, r0.x, r1.y
    r0.x = ((source[17].yyyy)*(r0.xxxx)+(r1.yyyy)).x;
    // 46: add r0.yzw, -cb0[2].xxyz, cb0[3].xxyz
    r0.yzw = ((-(source[2].xxyz))+(source[3].xxyz)).yzw;
    // 47: mul r0.yzw, r0.yyzw, cb0[14].xxxx
    r0.yzw = ((r0.yyzw)*(source[14].xxxx)).yzw;
    // 48: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[2].xyzx)).xyz;
    // 49: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 50: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 51: mad r0.xyz, cb0[17].zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((source[17].zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 52: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 54: mad r0.xyz, cb0[17].wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((source[17].wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 55: mad r3.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 56: mad r4.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 57: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 58: mul r4.xyz, r0.xyzx, r3.xyzx
    r4.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 59: mad r0.xyz, r0.xyzx, r3.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 60: mul r3.xyz, r1.xxxx, r4.xyzx
    r3.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // 61: mad r1.xyw, r1.xxxx, r4.xyxz, l(0.010000, 0.010000, 0.000000, 0.010000)
    r1.xyw = ((r1.xxxx)*(r4.xyxz)+(float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 62: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 63: mul r0.w, r0.w, cb0[19].z
    r0.w = ((r0.wwww)*(source[19].zzzz)).w;
    // 64: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 65: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 66: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 67: mul r2.w, cb0[7].z, l(1.500000)
    r2.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 68: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 69: mad r0.w, r0.w, l(0.500000), cb0[7].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 70: mul r4.y, cb0[7].y, cb0[8].y
    r4.y = ((source[7].yyyy)*(source[8].yyyy)).y;
    // 71: mul r5.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r5.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 72: frc r2.w, r5.x
    r2.w = (frac(r5.xxxx)).w;
    // 73: mul r5.y, r2.w, l(0.125000)
    r5.y = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 74: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 75: add r4.xy, r4.xyxx, r5.yzyy
    r4.xy = ((r4.xyxx)+(r5.yzyy)).xy;
    // 76: frc r2.w, cb0[7].x
    r2.w = (frac(source[7].xxxx)).w;
    // 77: add r3.w, -r2.w, cb0[7].x
    r3.w = ((-(r2.wwww))+(source[7].xxxx)).w;
    // 78: mul r4.z, r3.w, l(0.125000)
    r4.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 79: add r4.xy, r4.xyxx, r4.zwzz
    r4.xy = ((r4.xyxx)+(r4.zwzz)).xy;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t3.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 81: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 82: mul r0.w, r2.w, r4.w
    r0.w = ((r2.wwww)*(r4.wwww)).w;
    // 83: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 84: mad r3.xyz, r0.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 85: add r4.xyz, v8.xyzx, cb0[0].xyzx
    r4.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 86: add r5.xyzw, r4.yzxy, -cb0[1].yzxy
    r5.xyzw = ((r4.yzxy)+(-(source[1].yzxy))).xyzw;
    // 87: add r4.xyz, -r4.xyzx, cb0[0].xyzx
    r4.xyz = ((-(r4.xyzx))+(source[0].xyzx)).xyz;
    // 88: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 89: add r5.xy, -r5.zwzz, r5.xyxx
    r5.xy = ((-(r5.zwzz))+(r5.xyxx)).xy;
    // 90: mad r5.xy, cb0[9].wwww, r5.xyxx, r5.zwzz
    r5.xy = ((source[9].wwww)*(r5.xyxx)+(r5.zwzz)).xy;
    // 91: mul r0.w, cb0[9].y, cb0[19].z
    r0.w = ((source[9].yyyy)*(source[19].zzzz)).w;
    // 92: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 93: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 94: mul r6.y, r0.w, l(0.020000)
    r6.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 95: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 96: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 97: mul r2.w, cb0[9].x, l(0.001000)
    r2.w = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 98: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 99: mad r5.xy, r2.wwww, r5.xyxx, r6.xyxx
    r5.xy = ((r2.wwww)*(r5.xyxx)+(r6.xyxx)).xy;
    // 100: dp2 r2.w, cb0[10].xyxx, r5.xyxx
    r2.w = (dot((source[10].xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 101: dp2 r5.y, cb0[11].xyxx, r5.xyxx
    r5.y = (dot((source[11].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 102: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 103: mul r5.x, r2.w, l(0.125000)
    r5.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 104: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 105: mad r5.xyz, r5.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r3.xyzx
    r5.xyz = ((r5.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r3.xyzx))).xyz;
    // 106: mul r2.w, r5.w, l(0.900000)
    r2.w = ((r5.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 107: mad r5.xyz, r2.wwww, r5.xyzx, r3.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 108: mul_sat r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = (saturate((r0.wwww)*(r5.xyzx))).xyz;
    // 109: mad r6.xyz, cb0[9].zzzz, r5.xyzx, -r3.xyzx
    r6.xyz = ((source[9].zzzz)*(r5.xyzx)+(-(r3.xyzx))).xyz;
    // 110: mul r5.xyz, r5.xyzx, cb0[9].zzzz
    r5.xyz = ((r5.xyzx)*(source[9].zzzz)).xyz;
    // 111: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 112: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 113: mad r3.xyz, r0.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 114: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 115: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 117: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 118: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 119: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 120: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 121: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 122: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 123: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 124: mul r5.xyz, r0.wwww, v5.xyzx
    r5.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 125: dp3 r2.w, r2.xyzx, r5.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 126: add r3.w, r2.w, l(1.000000)
    r3.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 128: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 129: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 130: dp3 r4.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 131: add r6.xyz, r4.wwww, -cb0[2].xyzx
    r6.xyz = ((r4.wwww)+(-(source[2].xyzx))).xyz;
    // 132: mad r6.xyz, cb0[17].zzzz, r6.xyzx, cb0[2].xyzx
    r6.xyz = ((source[17].zzzz)*(r6.xyzx)+(source[2].xyzx)).xyz;
    // 133: dp3 r4.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 134: add r7.xyz, -r6.xyzx, r4.wwww
    r7.xyz = ((-(r6.xyzx))+(r4.wwww)).xyz;
    // 135: mad r6.xyz, cb0[17].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[17].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 136: mul r7.xyz, r6.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r7.xyz = ((r6.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 137: mul r7.xyz, r3.wwww, r7.xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)).xyz;
    // 138: min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r8.xyz, r2.wwww, cb2[3].xyzx
    r8.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // 140: add r2.w, -r3.w, l(1.000000)
    r2.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: mad r2.w, cb0[21].y, r2.w, r3.w
    r2.w = ((source[21].yyyy)*(r2.wwww)+(r3.wwww)).w;
    // 142: max r3.w, r3.w, l(0.500000)
    r3.w = (max(r3.wwww,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 143: mad r9.xyz, -r6.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r6.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 144: mad r7.xyz, r2.wwww, r9.xyzx, r7.xyzx
    r7.xyz = ((r2.wwww)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 145: add r9.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 146: mov_sat r2.w, r5.z
    r2.w = (saturate(r5.zzzz)).w;
    // 147: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 148: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 149: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: mad r6.xyz, r2.wwww, r9.xyzx, r6.xyzx
    r6.xyz = ((r2.wwww)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 151: mad r6.xyz, r7.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r6.xyzx
    r6.xyz = ((r7.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r6.xyzx)).xyz;
    // 152: mul_sat r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = (saturate((r3.xyzx)*(r6.xyzx))).xyz;
    // 153: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 154: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 155: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 156: dp3 r2.w, r6.xyzx, r2.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 157: dp3 r4.w, r6.xyzx, r5.xyzx
    r4.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 158: dp3 r5.x, v7.xyzx, v7.xyzx
    r5.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 159: rsq r5.x, r5.x
    r5.x = (rsqrt(r5.xxxx)).x;
    // 160: mul r5.xyz, r5.xxxx, v7.xyzx
    r5.xyz = ((r5.xxxx)*(v7.xyzx)).xyz;
    // 161: mul r6.xyz, r4.xyzx, r5.zzzz
    r6.xyz = ((r4.xyzx)*(r5.zzzz)).xyz;
    // 162: mad r4.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r4.xyzx)).xyz;
    // 163: dp3 r4.x, r4.xyzx, r4.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 164: sqrt r4.x, r4.x
    r4.x = (sqrt(r4.xxxx)).x;
    // 165: div r4.x, r4.z, r4.x
    r4.x = ((r4.zzzz)/(r4.xxxx)).x;
    // 166: add r4.x, r4.x, cb0[6].z
    r4.x = ((r4.xxxx)+(source[6].zzzz)).x;
    // 167: add r4.x, -r4.x, r4.w
    r4.x = ((-(r4.xxxx))+(r4.wwww)).x;
    // 168: add r2.w, r2.w, r4.x
    r2.w = ((r2.wwww)+(r4.xxxx)).w;
    // 169: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 170: add r2.w, r2.w, l(-0.500000)
    r2.w = ((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 171: add r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)+(r2.wwww)).w;
    // 172: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: log r4.x, r2.w
    r4.x = (log2(r2.wwww)).x;
    // 174: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 175: mad r4.y, cb0[18].w, l(4.500000), l(0.500000)
    r4.y = ((source[18].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 176: mul r4.y, r4.y, cb0[21].z
    r4.y = ((r4.yyyy)*(source[21].zzzz)).y;
    // 177: mul r4.y, r4.y, l(0.050000)
    r4.y = ((r4.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 178: mul r4.x, r4.x, r4.y
    r4.x = ((r4.xxxx)*(r4.yyyy)).x;
    // 179: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 180: mul r3.w, r3.w, r4.x
    r3.w = ((r3.wwww)*(r4.xxxx)).w;
    // 181: movc r2.w, r2.w, l(0), r3.w
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 182: mad r4.xyz, v5.xyzx, r0.wwww, r5.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r5.xyzx)).xyz;
    // 183: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 184: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 185: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: div r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)/(r0.wwww)).w;
    // 187: mul r0.w, r0.w, cb0[21].w
    r0.w = ((r0.wwww)*(source[21].wwww)).w;
    // 188: dp3 r2.w, r0.xyzx, r0.xyzx
    r2.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 189: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 190: div r0.xyz, r0.xyzx, r2.wwww
    r0.xyz = ((r0.xyzx)/(r2.wwww)).xyz;
    // 191: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 192: mov_sat r1.z, r5.z
    r1.z = (saturate(r5.zzzz)).z;
    // 193: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 194: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 195: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 196: mul r0.xyz, r0.xyzx, cb0[18].xxxx
    r0.xyz = ((r0.xyzx)*(source[18].xxxx)).xyz;
    // 197: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 198: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 199: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 200: mad r0.xyz, r3.xyzx, r7.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r7.xyzx)+(r0.xyzx)).xyz;
    // 201: dp3 r0.w, r1.xywx, r1.xywx
    r0.w = (dot((r1.xywx).xyz,(r1.xywx).xyz).xxxx).w;
    // 202: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 203: div r1.xyz, r1.xywx, r0.wwww
    r1.xyz = ((r1.xywx)/(r0.wwww)).xyz;
    // 204: mul r1.xyz, r1.xyzx, cb0[13].xyzx
    r1.xyz = ((r1.xyzx)*(source[13].xyzx)).xyz;
    // 205: dp3 r0.w, r2.xyzx, r5.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 206: add r1.w, -|r5.z|, l(1.000000)
    r1.w = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 207: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 208: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 209: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 210: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 211: mul r0.w, r0.w, cb0[21].x
    r0.w = ((r0.wwww)*(source[21].xxxx)).w;
    // 212: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 213: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 214: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 215: mad r0.xyz, r0.xyzx, cb2[3].wwww, r8.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r8.xyzx)).xyz;
    // 216: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 217: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 218: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 219: ret
    return output;
}

// source.character.equipment-native-162.v1 / source program 6cf25ec40f73404185757ea06ccb3996
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight162(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].w=(g_SourceCharacterTime.xxxx).x;
    source[28]=float4(input.lightColor,1.0);
    source[29].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[29].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[29].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.wxyz, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 36: mad r9.xyzw, r9.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r9.xyzw = ((r9.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 37: dp2 r1.w, r9.zwzz, r9.zwzz
    r1.w = (dot((r9.zwzz).xy,(r9.zwzz).xy).xxxx).w;
    // 38: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 41: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 42: mul r10.xy, r9.xyxx, cb0[17].xxxx
    r10.xy = ((r9.xyxx)*(source[17].xxxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mad r9.xy, cb0[17].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[17].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 45: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 46: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 47: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r12.xyz, cb0[19].wwww, r10.xyzx, r9.xyzx
    r12.xyz = ((source[19].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 49: dp3 r1.w, r12.xyzx, r12.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: div r12.xyz, r12.xyzx, r1.wwww
    r12.xyz = ((r12.xyzx)/(r1.wwww)).xyz;
    // 52: dp3 r13.x, r1.xyzx, r12.xyzx
    r13.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 53: dp3 r13.y, r2.xyzx, r12.xyzx
    r13.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 54: dp3 r13.z, r0.xyzx, r12.xyzx
    r13.z = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).z;
    // 55: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 56: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 57: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: dp3 r0.x, r13.xyzx, r1.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 59: mul r0.xyz, r13.xyzx, r0.xxxx
    r0.xyz = ((r13.xyzx)*(r0.xxxx)).xyz;
    // 60: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 61: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 62: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 65: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 66: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: add r0.z, -cb0[20].y, cb0[20].x
    r0.z = ((-(source[20].yyyy))+(source[20].xxxx)).z;
    // 68: mad r0.z, r11.x, r0.z, cb0[20].y
    r0.z = ((r11.xxxx)*(r0.zzzz)+(source[20].yyyy)).z;
    // 69: add r1.x, -r0.z, cb0[20].z
    r1.x = ((-(r0.zzzz))+(source[20].zzzz)).x;
    // 70: mad r0.z, r11.y, r1.x, r0.z
    r0.z = ((r11.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 71: add r1.x, -r0.z, cb0[20].w
    r1.x = ((-(r0.zzzz))+(source[20].wwww)).x;
    // 72: mad r0.z, r11.z, r1.x, r0.z
    r0.z = ((r11.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 74: add r1.x, -r2.w, l(1.000000)
    r1.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 75: add r1.y, -cb0[18].y, cb0[18].x
    r1.y = ((-(source[18].yyyy))+(source[18].xxxx)).y;
    // 76: mad r1.y, r11.x, r1.y, cb0[18].y
    r1.y = ((r11.xxxx)*(r1.yyyy)+(source[18].yyyy)).y;
    // 77: add r1.w, -r1.y, cb0[18].z
    r1.w = ((-(r1.yyyy))+(source[18].zzzz)).w;
    // 78: mad r1.y, r11.y, r1.w, r1.y
    r1.y = ((r11.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 79: add r1.w, -r1.y, cb0[18].w
    r1.w = ((-(r1.yyyy))+(source[18].wwww)).w;
    // 80: mad r1.y, r11.z, r1.w, r1.y
    r1.y = ((r11.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 81: add r1.w, -r1.y, cb0[19].x
    r1.w = ((-(r1.yyyy))+(source[19].xxxx)).w;
    // 82: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 83: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 84: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 85: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 86: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 87: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 88: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 89: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 90: add r1.w, -r1.y, cb0[21].x
    r1.w = ((-(r1.yyyy))+(source[21].xxxx)).w;
    // 91: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 92: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 93: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 94: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 95: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 96: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 97: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 98: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 99: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 100: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 101: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 102: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 103: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 104: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 105: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 106: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 107: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 108: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 109: rcp r1.y, cb0[21].y
    r1.y = (1.0/(source[21].yyyy)).y;
    // 110: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 111: mul r12.xyz, r4.xyzx, cb0[21].yyyy
    r12.xyz = ((r4.xyzx)*(source[21].yyyy)).xyz;
    // 112: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 113: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 114: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 115: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 116: mad r4.xyz, r12.xyzx, cb0[21].yyyy, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[21].yyyy)+(r4.xyzx)).xyz;
    // 117: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 119: add r1.y, cb0[21].y, l(1.000000)
    r1.y = ((source[21].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 120: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 121: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 122: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 123: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 124: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 125: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 126: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 127: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 128: mul r2.w, |r1.y|, |r1.y|
    r2.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 129: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 130: mul r1.y, |r1.y|, r2.w
    r1.y = ((abs(r1.yyyy))*(r2.wwww)).y;
    // 131: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 132: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 133: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 134: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 135: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 136: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 137: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 139: mul r12.xyz, r0.xyzx, r1.yyyy
    r12.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 140: mul r13.xyz, r12.xyzx, cb0[24].yyyy
    r13.xyz = ((r12.xyzx)*(source[24].yyyy)).xyz;
    // 141: mul r2.w, r11.w, l(0.500000)
    r2.w = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 142: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 144: mul r4.w, r1.x, r4.w
    r4.w = ((r1.xxxx)*(r4.wwww)).w;
    // 145: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 146: mad r9.xyz, r2.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 147: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 148: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 149: div r9.xyz, r9.xyzx, r2.wwww
    r9.xyz = ((r9.xyzx)/(r2.wwww)).xyz;
    // 150: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 151: max r4.w, r2.w, l(0.000000)
    r4.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 152: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: dp3 r6.w, cb0[16].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[16].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r7.xyz, r6.wwww, -cb0[16].xyzx
    r7.xyz = ((r6.wwww)+(-(source[16].xyzx))).xyz;
    // 156: mad r7.xyz, r5.wwww, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[16].xyzx)).xyz;
    // 157: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 158: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mul r7.xyz, r1.xxxx, r7.xyzx
    r7.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // 160: mad r2.w, r2.w, l(0.500000), -r5.w
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).w;
    // 161: mad r7.xyz, r7.xyzx, r2.wwww, r5.wwww
    r7.xyz = ((r7.xyzx)*(r2.wwww)+(r5.wwww)).xyz;
    // 162: add_sat r2.w, r11.w, cb0[24].z
    r2.w = (saturate((r11.wwww)+(source[24].zzzz))).w;
    // 163: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 165: mul_sat r6.xy, r6.xzxx, cb0[22].xxxx
    r6.xy = (saturate((r6.xzxx)*(source[22].xxxx))).xy;
    // 166: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 167: add_sat r6.y, r6.y, -cb0[22].y
    r6.y = (saturate((r6.yyyy)+(-(source[22].yyyy)))).y;
    // 168: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 169: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 170: mul r6.y, r6.y, cb0[22].z
    r6.y = ((r6.yyyy)*(source[22].zzzz)).y;
    // 171: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 172: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 173: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 174: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 175: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 176: mad r7.w, r0.w, l(2.000000), -r1.y
    r7.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).w;
    // 177: mad r6.yzw, r6.yyzw, r7.wwww, r1.yyyy
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r1.yyyy)).yzw;
    // 178: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 179: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 180: mul r7.w, r1.x, r1.x
    r7.w = ((r1.xxxx)*(r1.xxxx)).w;
    // 181: mul r8.x, r7.w, cb0[24].w
    r8.x = ((r7.wwww)*(source[24].wwww)).x;
    // 182: mad r1.x, -r7.w, cb0[24].w, r1.x
    r1.x = ((-(r7.wwww))*(source[24].wwww)+(r1.xxxx)).x;
    // 183: mad r1.x, r11.w, r1.x, r8.x
    r1.x = ((r11.wwww)*(r1.xxxx)+(r8.xxxx)).x;
    // 184: mad r6.yzw, r2.wwww, r6.yyzw, -r7.xxyz
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 185: mad r6.yzw, r1.xxxx, r6.yyzw, r7.xxyz
    r6.yzw = ((r1.xxxx)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 186: sqrt r1.x, r5.w
    r1.x = (sqrt(r5.wwww)).x;
    // 187: mul r5.xyz, r5.xyzx, r1.xxxx
    r5.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 188: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 189: mad r6.yzw, -cb0[24].yyyy, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[24].yyyy))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 190: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 191: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 192: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 193: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 194: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 195: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 196: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 197: mad r7.xyz, r11.zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 198: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 199: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 200: mad r7.xyz, cb0[19].yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((source[19].yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 201: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 202: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 203: mad r7.xyz, cb0[19].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[19].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 204: mad r10.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 205: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 206: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 207: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 208: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[9].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[9].xyzx)).xyz;
    // 209: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 210: dp3 r1.x, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 211: add r12.xyz, -r8.yzwy, r1.xxxx
    r12.xyz = ((-(r8.yzwy))+(r1.xxxx)).xyz;
    // 212: mad r8.xyz, cb0[19].yyyy, r12.xyzx, r8.yzwy
    r8.xyz = ((source[19].yyyy)*(r12.xyzx)+(r8.yzwy)).xyz;
    // 213: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 214: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 215: mad r8.xyz, cb0[19].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[19].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 216: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 217: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 218: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 219: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 220: add r13.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r13.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 221: mad r13.xyz, r0.yyyy, r13.xyzx, cb0[10].xyzx
    r13.xyz = ((r0.yyyy)*(r13.xyzx)+(source[10].xyzx)).xyz;
    // 222: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 223: mul r0.xyz, r0.xyzx, cb0[21].zzzz
    r0.xyz = ((r0.xyzx)*(source[21].zzzz)).xyz;
    // 224: mul r13.xyz, r0.xyzx, r12.xyzx
    r13.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 225: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: add r14.xyz, -r2.xyzx, r1.xxxx
    r14.xyz = ((-(r2.xyzx))+(r1.xxxx)).xyz;
    // 227: mad r2.yzw, cb0[19].yyyy, r14.xxyz, r2.xxyz
    r2.yzw = ((source[19].yyyy)*(r14.xxyz)+(r2.xxyz)).yzw;
    // 228: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 229: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 230: mad r2.yzw, cb0[19].zzzz, r14.xxyz, r2.yyzw
    r2.yzw = ((source[19].zzzz)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 231: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 232: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 233: mul r14.xyz, r14.xyzx, cb0[21].wwww
    r14.xyz = ((r14.xyzx)*(source[21].wwww)).xyz;
    // 234: add r1.x, r11.y, r11.x
    r1.x = ((r11.yyyy)+(r11.xxxx)).x;
    // 235: add r1.x, r11.z, r1.x
    r1.x = ((r11.zzzz)+(r1.xxxx)).x;
    // 236: add_sat r1.x, r11.w, r1.x
    r1.x = (saturate((r11.wwww)+(r1.xxxx))).x;
    // 237: mad r2.yzw, r1.xxxx, r14.xxyz, r2.yyzw
    r2.yzw = ((r1.xxxx)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 238: add r11.xyz, -r2.yzwy, r2.xxxx
    r11.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 239: mad r2.xyz, r11.wwww, r11.xyzx, r2.yzwy
    r2.xyz = ((r11.wwww)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 240: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 241: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 242: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 243: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 244: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 245: add r1.z, cb0[22].w, -cb0[23].x
    r1.z = ((source[22].wwww)+(-(source[23].xxxx))).z;
    // 246: mad r1.z, r11.w, r1.z, cb0[23].x
    r1.z = ((r11.wwww)*(r1.zzzz)+(source[23].xxxx)).z;
    // 247: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 248: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 249: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 250: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 251: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 252: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 253: div r1.z, cb0[23].y, r1.z
    r1.z = ((source[23].yyyy)/(r1.zzzz)).z;
    // 254: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 255: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 256: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 257: mul r1.z, r1.z, cb0[23].z
    r1.z = ((r1.zzzz)*(source[23].zzzz)).z;
    // 258: mad r0.xyz, r2.xyzx, r0.xyzx, -r13.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 259: mad r0.xyz, r1.zzzz, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 260: mad r0.xyz, r1.yyyy, r0.xyzx, -r12.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r12.xyzx))).xyz;
    // 261: mad r0.xyz, r1.xxxx, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 262: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 263: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 264: mad r0.xyz, cb0[19].yyyy, r11.xyzx, r0.xyzx
    r0.xyz = ((source[19].yyyy)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 265: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 266: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 267: mad r0.xyz, cb0[19].zzzz, r11.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 268: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 269: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 270: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 271: mul r2.w, r2.w, cb0[23].w
    r2.w = ((r2.wwww)*(source[23].wwww)).w;
    // 272: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 273: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 274: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 275: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 276: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 277: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 278: add r6.x, -r2.w, cb0[2].x
    r6.x = ((-(r2.wwww))+(source[2].xxxx)).x;
    // 279: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 280: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 281: mul r10.y, cb0[2].y, cb0[12].y
    r10.y = ((source[2].yyyy)*(source[12].yyyy)).y;
    // 282: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 283: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 284: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 285: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 286: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t5.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 288: mul r10.xyz, r1.zzzz, r10.xyzx
    r10.xyz = ((r1.zzzz)*(r10.xyzx)).xyz;
    // 289: mul r1.z, r2.w, r10.w
    r1.z = ((r2.wwww)*(r10.wwww)).z;
    // 290: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 291: mad r0.xyz, r1.zzzz, r10.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 292: mul r1.z, cb0[13].y, cb0[23].w
    r1.z = ((source[13].yyyy)*(source[23].wwww)).z;
    // 293: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 294: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 295: mul r10.y, r1.z, l(0.020000)
    r10.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 296: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 297: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 298: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 299: mad r3.xy, cb0[13].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[13].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 300: mul r2.w, cb0[13].x, l(0.001000)
    r2.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 301: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 302: mad r3.xy, r2.wwww, r3.xyxx, r10.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r10.xyxx)).xy;
    // 303: dp2 r2.w, cb0[14].xyxx, r3.xyxx
    r2.w = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 304: dp2 r3.y, cb0[15].xyxx, r3.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 305: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 306: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 307: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 308: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 309: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 310: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 311: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 312: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 313: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 314: mul r10.xyz, r3.xyzx, cb0[13].zzzz
    r10.xyz = ((r3.xyzx)*(source[13].zzzz)).xyz;
    // 315: dp3 r1.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 316: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 317: mad r3.xyz, cb0[13].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[13].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 318: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 319: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 320: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 321: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 322: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 323: dp3 r4.x, r4.xyzx, r9.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 324: mul r4.z, r0.w, cb0[25].x
    r4.z = ((r0.wwww)*(source[25].xxxx)).z;
    // 325: mul r0.w, r11.w, r4.z
    r0.w = ((r11.wwww)*(r4.zzzz)).w;
    // 326: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 327: min r0.w, r0.w, cb0[25].x
    r0.w = (min(r0.wwww,source[25].xxxx)).w;
    // 328: add r1.x, -cb0[25].w, cb0[25].z
    r1.x = ((-(source[25].wwww))+(source[25].zzzz)).x;
    // 329: mad r1.x, cb0[25].y, r1.x, cb0[25].w
    r1.x = ((source[25].yyyy)*(r1.xxxx)+(source[25].wwww)).x;
    // 330: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 331: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 332: mad r1.x, r11.w, r1.x, l(1.000000)
    r1.x = ((r11.wwww)*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 333: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 334: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 335: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 336: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 337: movc r4.y, r1.z, l(0), r0.w
    r4.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 338: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 339: add r0.w, -cb0[26].z, cb0[26].y
    r0.w = ((-(source[26].zzzz))+(source[26].yyyy)).w;
    // 340: mad r0.w, cb0[26].x, r0.w, cb0[26].z
    r0.w = ((source[26].xxxx)*(r0.wwww)+(source[26].zzzz)).w;
    // 341: mul r0.w, r0.w, r11.w
    r0.w = ((r0.wwww)*(r11.wwww)).w;
    // 342: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xzxx, t6.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 343: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 344: add r0.w, -cb0[26].w, l(2.000000)
    r0.w = ((-(source[26].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 345: mad r0.w, r1.y, r0.w, cb0[26].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[26].wwww)).w;
    // 346: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 347: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 348: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 349: mul r1.xyz, r1.xyzx, cb0[27].xxxx
    r1.xyz = ((r1.xyzx)*(source[27].xxxx)).xyz;
    // 350: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 351: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 352: mul r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)*(r5.wwww)).xyz;
    // 353: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 354: mul r1.xyz, r1.xyzx, cb0[27].yyyy
    r1.xyz = ((r1.xyzx)*(source[27].yyyy)).xyz;
    // 355: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 356: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 357: mad r0.xyz, r6.yzwy, r0.xyzx, r1.xyzx
    r0.xyz = ((r6.yzwy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 358: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 359: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 360: mul o0.xyz, r0.xyzx, cb0[28].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[28].xyzx)).xyz;
    // 361: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 362: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 363: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 364: ret
    return output;
}

// source.character.equipment-native-163.v1 / source program da95a0e74c58b742a16a915dad74e224
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight163(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].x=(g_SourceCharacterTime.xxxx).x;
    source[23].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[23].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[24].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[24].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[28]=float4(input.lightColor,1.0);
    source[29].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[29].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[29].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s7, l(0.000000)
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
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 44: mad r9.xyzw, r9.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r9.xyzw = ((r9.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 45: dp2 r1.w, r9.zwzz, r9.zwzz
    r1.w = (dot((r9.zwzz).xy,(r9.zwzz).xy).xxxx).w;
    // 46: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 48: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 49: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 50: mul r10.xy, r9.xyxx, cb0[17].xxxx
    r10.xy = ((r9.xyxx)*(source[17].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mad r9.xy, cb0[17].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[17].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 53: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 54: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 55: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r12.xyz, cb0[19].xxxx, r10.xyzx, r9.xyzx
    r12.xyz = ((source[19].xxxx)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 57: dp3 r1.w, r12.xyzx, r12.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 58: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 59: div r12.xyz, r12.xyzx, r1.wwww
    r12.xyz = ((r12.xyzx)/(r1.wwww)).xyz;
    // 60: dp3 r13.x, r1.xyzx, r12.xyzx
    r13.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 61: dp3 r13.y, r2.xyzx, r12.xyzx
    r13.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 62: dp3 r13.z, r0.xyzx, r12.xyzx
    r13.z = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).z;
    // 63: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 64: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 65: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 66: dp3 r0.x, r13.xyzx, r1.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 67: mul r0.xyz, r13.xyzx, r0.xxxx
    r0.xyz = ((r13.xyzx)*(r0.xxxx)).xyz;
    // 68: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 69: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 70: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 71: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 72: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 73: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 74: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 76: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 77: add r1.x, -cb0[18].y, cb0[18].x
    r1.x = ((-(source[18].yyyy))+(source[18].xxxx)).x;
    // 78: mad r1.x, r11.w, r1.x, cb0[18].y
    r1.x = ((r11.wwww)*(r1.xxxx)+(source[18].yyyy)).x;
    // 79: lt r1.y, |r0.z|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 81: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 82: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 83: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 84: movc r0.z, r1.y, l(0), r0.z
    r0.z = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 85: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 86: add r1.y, -r1.x, cb0[19].y
    r1.y = ((-(r1.xxxx))+(source[19].yyyy)).y;
    // 87: mad r1.x, r11.w, r1.y, r1.x
    r1.x = ((r11.wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 88: mul r1.x, r1.x, cb0[19].z
    r1.x = ((r1.xxxx)*(source[19].zzzz)).x;
    // 89: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 90: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 91: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 92: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 93: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 94: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 95: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 96: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 97: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 98: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 99: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 100: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 101: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 102: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 103: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 104: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t5.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 105: rcp r0.x, cb0[19].w
    r0.x = (1.0/(source[19].wwww)).x;
    // 106: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 107: mul r12.xyz, r4.xyzx, cb0[19].wwww
    r12.xyz = ((r4.xyzx)*(source[19].wwww)).xyz;
    // 108: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 109: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 110: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 111: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 112: mad r4.xyz, r12.xyzx, cb0[19].wwww, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[19].wwww)+(r4.xyzx)).xyz;
    // 113: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 114: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 115: add r0.x, cb0[19].w, l(1.000000)
    r0.x = ((source[19].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 116: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 117: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 118: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 119: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 120: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 121: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 122: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 123: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 124: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 125: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 126: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 127: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 128: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 129: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 130: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 131: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 132: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 133: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 135: mul r12.xyz, r1.xywx, r4.xxxx
    r12.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 136: mul r13.xyz, r12.xyzx, cb0[24].wwww
    r13.xyz = ((r12.xyzx)*(source[24].wwww)).xyz;
    // 137: mul r4.z, r11.w, l(0.500000)
    r4.z = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 138: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 140: mul r4.w, r0.z, r4.w
    r4.w = ((r0.zzzz)*(r4.wwww)).w;
    // 141: mul r4.z, r4.w, r4.z
    r4.z = ((r4.wwww)*(r4.zzzz)).z;
    // 142: mad r9.xyz, r4.zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((r4.zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 143: dp3 r4.z, r9.xyzx, r9.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 144: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 145: div r9.xyz, r9.xyzx, r4.zzzz
    r9.xyz = ((r9.xyzx)/(r4.zzzz)).xyz;
    // 146: dp3 r4.z, r9.xyzx, r7.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 147: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 148: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 150: dp3 r6.w, cb0[16].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[16].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 151: add r7.xyz, r6.wwww, -cb0[16].xyzx
    r7.xyz = ((r6.wwww)+(-(source[16].xyzx))).xyz;
    // 152: mad r7.xyz, r5.wwww, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[16].xyzx)).xyz;
    // 153: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 154: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 155: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 156: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 157: mad r7.xyz, r7.xyzx, r4.zzzz, r5.wwww
    r7.xyz = ((r7.xyzx)*(r4.zzzz)+(r5.wwww)).xyz;
    // 158: add_sat r4.z, r11.w, cb0[25].x
    r4.z = (saturate((r11.wwww)+(source[25].xxxx))).z;
    // 159: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 161: mul_sat r6.xy, r6.xzxx, cb0[20].zzzz
    r6.xy = (saturate((r6.xzxx)*(source[20].zzzz))).xy;
    // 162: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 163: add_sat r6.y, r6.y, -cb0[20].w
    r6.y = (saturate((r6.yyyy)+(-(source[20].wwww)))).y;
    // 164: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 165: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 166: mul r6.y, r6.y, cb0[21].x
    r6.y = ((r6.yyyy)*(source[21].xxxx)).y;
    // 167: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 168: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 169: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 170: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 171: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 172: mad r7.w, r2.w, l(2.000000), -r4.x
    r7.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).w;
    // 173: mad r6.yzw, r6.yyzw, r7.wwww, r4.xxxx
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r4.xxxx)).yzw;
    // 174: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 175: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 176: mul r7.w, r0.z, r0.z
    r7.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 177: mul r8.w, r7.w, cb0[25].y
    r8.w = ((r7.wwww)*(source[25].yyyy)).w;
    // 178: mad r0.z, -r7.w, cb0[25].y, r0.z
    r0.z = ((-(r7.wwww))*(source[25].yyyy)+(r0.zzzz)).z;
    // 179: mad r0.z, r11.w, r0.z, r8.w
    r0.z = ((r11.wwww)*(r0.zzzz)+(r8.wwww)).z;
    // 180: mad r6.yzw, r4.zzzz, r6.yyzw, -r7.xxyz
    r6.yzw = ((r4.zzzz)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 181: mad r6.yzw, r0.zzzz, r6.yyzw, r7.xxyz
    r6.yzw = ((r0.zzzz)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 182: sqrt r0.z, r5.w
    r0.z = (sqrt(r5.wwww)).z;
    // 183: mul r5.xyz, r5.xyzx, r0.zzzz
    r5.xyz = ((r5.xyzx)*(r0.zzzz)).xyz;
    // 184: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 185: mad r6.yzw, -cb0[24].wwww, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[24].wwww))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 186: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 187: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 188: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 189: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 190: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 191: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 192: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 194: mad r7.xyz, cb0[18].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[18].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 195: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r7.xyz, cb0[18].wwww, r10.xyzx, r7.xyzx
    r7.xyz = ((source[18].wwww)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 198: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 199: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 200: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 201: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 202: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[8].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[8].xyzx)).xyz;
    // 203: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 204: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 205: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 206: mad r8.xyz, cb0[18].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[18].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 207: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 208: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 209: mad r8.xyz, cb0[18].wwww, r12.xyzx, r8.xyzx
    r8.xyz = ((source[18].wwww)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 210: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 211: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 212: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 213: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 214: add r1.yzw, -cb0[9].xxyz, cb0[10].xxyz
    r1.yzw = ((-(source[9].xxyz))+(source[10].xxyz)).yzw;
    // 215: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[9].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[9].xyzx)).xyz;
    // 216: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 217: mul r1.xyz, r1.xyzx, cb0[20].xxxx
    r1.xyz = ((r1.xyzx)*(source[20].xxxx)).xyz;
    // 218: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 219: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 220: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 221: mad r14.xyz, cb0[18].zzzz, r14.xyzx, r2.xyzx
    r14.xyz = ((source[18].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 222: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 223: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 224: mad r14.xyz, cb0[18].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[18].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 225: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 226: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 227: mul r15.xyz, r15.xyzx, cb0[20].yyyy
    r15.xyz = ((r15.xyzx)*(source[20].yyyy)).xyz;
    // 228: add r0.z, r11.y, r11.x
    r0.z = ((r11.yyyy)+(r11.xxxx)).z;
    // 229: add r0.z, r11.z, r0.z
    r0.z = ((r11.zzzz)+(r0.zzzz)).z;
    // 230: add_sat r0.z, r11.w, r0.z
    r0.z = (saturate((r11.wwww)+(r0.zzzz))).z;
    // 231: mad r11.xyz, r0.zzzz, r15.xyzx, r14.xyzx
    r11.xyz = ((r0.zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 232: add r2.xyz, r2.xxxx, -r11.xyzx
    r2.xyz = ((r2.xxxx)+(-(r11.xyzx))).xyz;
    // 233: mad r2.xyz, r11.wwww, r2.xyzx, r11.xyzx
    r2.xyz = ((r11.wwww)*(r2.xyzx)+(r11.xyzx)).xyz;
    // 234: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 235: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 236: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 237: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 238: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 239: add r1.w, -cb0[21].z, cb0[21].y
    r1.w = ((-(source[21].zzzz))+(source[21].yyyy)).w;
    // 240: mad r1.w, r11.w, r1.w, cb0[21].z
    r1.w = ((r11.wwww)*(r1.wwww)+(source[21].zzzz)).w;
    // 241: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 242: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 243: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 244: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 245: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 246: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 247: div r1.w, cb0[21].w, r1.w
    r1.w = ((source[21].wwww)/(r1.wwww)).w;
    // 248: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 249: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 250: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 251: mul r1.w, r1.w, cb0[22].x
    r1.w = ((r1.wwww)*(source[22].xxxx)).w;
    // 252: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 253: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 254: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 255: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 256: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 257: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 258: mad r1.xyz, cb0[18].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[18].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 259: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 260: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 261: mad r1.xyz, cb0[18].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[18].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 262: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 263: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 264: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 265: mul r4.z, r4.z, cb0[23].x
    r4.z = ((r4.zzzz)*(source[23].xxxx)).z;
    // 266: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 267: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 268: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 269: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 270: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 271: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 272: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 273: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 274: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 275: mul r10.y, cb0[2].y, cb0[11].y
    r10.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 276: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 277: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 278: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 279: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 280: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 281: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t6.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 282: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 283: mul r1.w, r4.z, r10.w
    r1.w = ((r4.zzzz)*(r10.wwww)).w;
    // 284: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 285: mad r1.xyz, r1.wwww, r10.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 286: mul r1.w, cb0[12].y, cb0[23].x
    r1.w = ((source[12].yyyy)*(source[23].xxxx)).w;
    // 287: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 288: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 289: mul r10.y, r1.w, l(0.020000)
    r10.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 290: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 291: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 292: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 293: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 294: mul r3.z, cb0[12].x, l(0.001000)
    r3.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 295: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 296: mad r3.xy, r3.zzzz, r3.xyxx, r10.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r10.xyxx)).xy;
    // 297: dp2 r3.z, cb0[13].xyxx, r3.xyxx
    r3.z = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 298: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 299: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 300: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 301: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 302: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 303: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 304: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 305: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 306: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 307: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 308: mul r10.xyz, r3.xyzx, cb0[12].zzzz
    r10.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 309: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 310: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 311: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 312: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 313: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 314: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 315: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 316: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 317: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 318: mul r0.z, r2.w, cb0[25].z
    r0.z = ((r2.wwww)*(source[25].zzzz)).z;
    // 319: mul r0.w, r11.w, r0.z
    r0.w = ((r11.wwww)*(r0.zzzz)).w;
    // 320: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 321: min r0.w, r0.w, cb0[25].z
    r0.w = (min(r0.wwww,source[25].zzzz)).w;
    // 322: add r1.w, -cb0[26].y, cb0[26].x
    r1.w = ((-(source[26].yyyy))+(source[26].xxxx)).w;
    // 323: mad r1.w, cb0[25].w, r1.w, cb0[26].y
    r1.w = ((source[25].wwww)*(r1.wwww)+(source[26].yyyy)).w;
    // 324: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 325: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 326: mad r1.w, r11.w, r1.w, l(1.000000)
    r1.w = ((r11.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 327: lt r2.w, |r0.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 328: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 329: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 330: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 331: movc r0.y, r2.w, l(0), r0.w
    r0.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 332: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 333: add r0.y, cb0[26].w, -cb0[27].x
    r0.y = ((source[26].wwww)+(-(source[27].xxxx))).y;
    // 334: mad r0.y, cb0[26].z, r0.y, cb0[27].x
    r0.y = ((source[26].zzzz)*(r0.yyyy)+(source[27].xxxx)).y;
    // 335: mul r0.y, r0.y, r11.w
    r0.y = ((r0.yyyy)*(r11.wwww)).y;
    // 336: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t7.xwyz, s8, l(0.000000)
    r0.xzw = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // 337: mad r0.xyz, r0.yyyy, r5.xyzx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r5.xyzx)+(r0.xzwx)).xyz;
    // 338: add r0.w, -cb0[27].y, l(2.000000)
    r0.w = ((-(source[27].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 339: mad r0.w, r4.x, r0.w, cb0[27].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[27].yyyy)).w;
    // 340: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 341: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 342: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 343: mul r0.xyz, r0.xyzx, cb0[27].zzzz
    r0.xyz = ((r0.xyzx)*(source[27].zzzz)).xyz;
    // 344: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 345: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 346: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 347: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 348: mul r0.xyz, r0.xyzx, cb0[27].wwww
    r0.xyz = ((r0.xyzx)*(source[27].wwww)).xyz;
    // 349: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 350: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 351: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 352: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 353: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 354: mul o0.xyz, r0.xyzx, cb0[28].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[28].xyzx)).xyz;
    // 355: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 356: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 357: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 358: ret
    return output;
}

// source.character.equipment-native-164.v1 / source program c5df3fee8e45da48bd2c274cccd1ca3a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight164(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].y=(g_SourceCharacterTime.xxxx).x;
    source[22].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[22].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[23].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[23].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[27]=float4(input.lightColor,1.0);
    source[28].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[28].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[28].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s7, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 44: mad r9.xyzw, r9.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r9.xyzw = ((r9.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 45: dp2 r1.w, r9.zwzz, r9.zwzz
    r1.w = (dot((r9.zwzz).xy,(r9.zwzz).xy).xxxx).w;
    // 46: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 48: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 49: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 50: mul r10.xy, r9.xyxx, cb0[17].xxxx
    r10.xy = ((r9.xyxx)*(source[17].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mad r9.xy, cb0[17].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[17].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 53: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 54: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 55: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r12.xyz, cb0[19].xxxx, r10.xyzx, r9.xyzx
    r12.xyz = ((source[19].xxxx)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 57: dp3 r1.w, r12.xyzx, r12.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 58: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 59: div r12.xyz, r12.xyzx, r1.wwww
    r12.xyz = ((r12.xyzx)/(r1.wwww)).xyz;
    // 60: dp3 r13.x, r1.xyzx, r12.xyzx
    r13.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 61: dp3 r13.y, r2.xyzx, r12.xyzx
    r13.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 62: dp3 r13.z, r0.xyzx, r12.xyzx
    r13.z = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).z;
    // 63: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 64: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 65: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 66: dp3 r0.x, r13.xyzx, r1.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 67: mul r0.xyz, r13.xyzx, r0.xxxx
    r0.xyz = ((r13.xyzx)*(r0.xxxx)).xyz;
    // 68: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 69: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 70: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 71: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 72: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 73: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 74: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 76: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 77: add r1.x, -cb0[18].y, cb0[18].x
    r1.x = ((-(source[18].yyyy))+(source[18].xxxx)).x;
    // 78: mad r1.x, r11.w, r1.x, cb0[18].y
    r1.x = ((r11.wwww)*(r1.xxxx)+(source[18].yyyy)).x;
    // 79: lt r1.y, |r0.z|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 81: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 82: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 83: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 84: movc r0.z, r1.y, l(0), r0.z
    r0.z = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 85: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 86: add r1.y, -r1.x, cb0[19].y
    r1.y = ((-(r1.xxxx))+(source[19].yyyy)).y;
    // 87: mad r1.x, r11.w, r1.y, r1.x
    r1.x = ((r11.wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 88: mul r1.x, r1.x, cb0[19].z
    r1.x = ((r1.xxxx)*(source[19].zzzz)).x;
    // 89: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 90: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 91: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 92: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 93: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 94: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 95: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 96: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 97: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 98: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 99: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 100: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 101: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 102: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 103: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 104: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t5.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 105: rcp r0.x, cb0[19].w
    r0.x = (1.0/(source[19].wwww)).x;
    // 106: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 107: mul r12.xyz, r4.xyzx, cb0[19].wwww
    r12.xyz = ((r4.xyzx)*(source[19].wwww)).xyz;
    // 108: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 109: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 110: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 111: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 112: mad r4.xyz, r12.xyzx, cb0[19].wwww, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[19].wwww)+(r4.xyzx)).xyz;
    // 113: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 114: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 115: add r0.x, cb0[19].w, l(1.000000)
    r0.x = ((source[19].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 116: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 117: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 118: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 119: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 120: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 121: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 122: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 123: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 124: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 125: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 126: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 127: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 128: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 129: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 130: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 131: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 132: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 133: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 135: mul r12.xyz, r1.xywx, r4.xxxx
    r12.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 136: mul r13.xyz, r12.xyzx, cb0[23].wwww
    r13.xyz = ((r12.xyzx)*(source[23].wwww)).xyz;
    // 137: mul r4.z, r11.w, l(0.500000)
    r4.z = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 138: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 140: mul r4.w, r0.z, r4.w
    r4.w = ((r0.zzzz)*(r4.wwww)).w;
    // 141: mul r4.z, r4.w, r4.z
    r4.z = ((r4.wwww)*(r4.zzzz)).z;
    // 142: mad r9.xyz, r4.zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((r4.zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 143: dp3 r4.z, r9.xyzx, r9.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 144: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 145: div r9.xyz, r9.xyzx, r4.zzzz
    r9.xyz = ((r9.xyzx)/(r4.zzzz)).xyz;
    // 146: dp3 r4.z, r9.xyzx, r7.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 147: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 148: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 150: dp3 r6.w, cb0[16].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[16].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 151: add r7.xyz, r6.wwww, -cb0[16].xyzx
    r7.xyz = ((r6.wwww)+(-(source[16].xyzx))).xyz;
    // 152: mad r7.xyz, r5.wwww, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[16].xyzx)).xyz;
    // 153: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 154: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 155: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 156: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 157: mad r7.xyz, r7.xyzx, r4.zzzz, r5.wwww
    r7.xyz = ((r7.xyzx)*(r4.zzzz)+(r5.wwww)).xyz;
    // 158: add_sat r4.z, r11.w, cb0[24].x
    r4.z = (saturate((r11.wwww)+(source[24].xxxx))).z;
    // 159: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 161: mul_sat r6.xy, r6.xzxx, cb0[20].zzzz
    r6.xy = (saturate((r6.xzxx)*(source[20].zzzz))).xy;
    // 162: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 163: add_sat r6.y, r6.y, -cb0[20].w
    r6.y = (saturate((r6.yyyy)+(-(source[20].wwww)))).y;
    // 164: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 165: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 166: mul r6.y, r6.y, cb0[21].x
    r6.y = ((r6.yyyy)*(source[21].xxxx)).y;
    // 167: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 168: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 169: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 170: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 171: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 172: mad r7.w, r2.w, l(2.000000), -r4.x
    r7.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).w;
    // 173: mad r6.yzw, r6.yyzw, r7.wwww, r4.xxxx
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r4.xxxx)).yzw;
    // 174: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 175: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 176: mul r7.w, r0.z, r0.z
    r7.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 177: mul r8.w, r7.w, cb0[24].y
    r8.w = ((r7.wwww)*(source[24].yyyy)).w;
    // 178: mad r0.z, -r7.w, cb0[24].y, r0.z
    r0.z = ((-(r7.wwww))*(source[24].yyyy)+(r0.zzzz)).z;
    // 179: mad r0.z, r11.w, r0.z, r8.w
    r0.z = ((r11.wwww)*(r0.zzzz)+(r8.wwww)).z;
    // 180: mad r6.yzw, r4.zzzz, r6.yyzw, -r7.xxyz
    r6.yzw = ((r4.zzzz)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 181: mad r6.yzw, r0.zzzz, r6.yyzw, r7.xxyz
    r6.yzw = ((r0.zzzz)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 182: sqrt r0.z, r5.w
    r0.z = (sqrt(r5.wwww)).z;
    // 183: mul r5.xyz, r5.xyzx, r0.zzzz
    r5.xyz = ((r5.xyzx)*(r0.zzzz)).xyz;
    // 184: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 185: mad r6.yzw, -cb0[23].wwww, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[23].wwww))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 186: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 187: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 188: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 189: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 190: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 191: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 192: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 194: mad r7.xyz, cb0[18].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[18].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 195: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r7.xyz, cb0[18].wwww, r10.xyzx, r7.xyzx
    r7.xyz = ((source[18].wwww)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 198: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 199: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 200: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 201: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 202: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[8].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[8].xyzx)).xyz;
    // 203: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 204: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 205: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 206: mad r8.xyz, cb0[18].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[18].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 207: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 208: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 209: mad r8.xyz, cb0[18].wwww, r12.xyzx, r8.xyzx
    r8.xyz = ((source[18].wwww)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 210: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 211: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 212: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 213: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 214: add r1.yzw, -cb0[9].xxyz, cb0[10].xxyz
    r1.yzw = ((-(source[9].xxyz))+(source[10].xxyz)).yzw;
    // 215: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[9].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[9].xyzx)).xyz;
    // 216: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 217: mul r1.xyz, r1.xyzx, cb0[20].xxxx
    r1.xyz = ((r1.xyzx)*(source[20].xxxx)).xyz;
    // 218: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 219: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 220: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 221: mad r14.xyz, cb0[18].zzzz, r14.xyzx, r2.xyzx
    r14.xyz = ((source[18].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 222: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 223: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 224: mad r14.xyz, cb0[18].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[18].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 225: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 226: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 227: mul r15.xyz, r15.xyzx, cb0[20].yyyy
    r15.xyz = ((r15.xyzx)*(source[20].yyyy)).xyz;
    // 228: add r0.z, r11.y, r11.x
    r0.z = ((r11.yyyy)+(r11.xxxx)).z;
    // 229: add r0.z, r11.z, r0.z
    r0.z = ((r11.zzzz)+(r0.zzzz)).z;
    // 230: add_sat r0.z, r11.w, r0.z
    r0.z = (saturate((r11.wwww)+(r0.zzzz))).z;
    // 231: mad r11.xyz, r0.zzzz, r15.xyzx, r14.xyzx
    r11.xyz = ((r0.zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 232: add r2.xyz, r2.xxxx, -r11.xyzx
    r2.xyz = ((r2.xxxx)+(-(r11.xyzx))).xyz;
    // 233: mad r2.xyz, r11.wwww, r2.xyzx, r11.xyzx
    r2.xyz = ((r11.wwww)*(r2.xyzx)+(r11.xyzx)).xyz;
    // 234: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 235: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 236: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 237: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 238: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 239: add r1.w, -cb0[21].z, cb0[21].y
    r1.w = ((-(source[21].zzzz))+(source[21].yyyy)).w;
    // 240: mad r1.w, r11.w, r1.w, cb0[21].z
    r1.w = ((r11.wwww)*(r1.wwww)+(source[21].zzzz)).w;
    // 241: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 242: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 243: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 244: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 245: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 246: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 247: div r1.w, cb0[21].w, r1.w
    r1.w = ((source[21].wwww)/(r1.wwww)).w;
    // 248: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 249: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 250: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 251: mul r1.w, r1.w, cb0[22].x
    r1.w = ((r1.wwww)*(source[22].xxxx)).w;
    // 252: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 253: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 254: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 255: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 256: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 257: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 258: mad r1.xyz, cb0[18].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[18].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 259: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 260: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 261: mad r1.xyz, cb0[18].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[18].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 262: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 263: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 264: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 265: mul r4.z, r4.z, cb0[22].y
    r4.z = ((r4.zzzz)*(source[22].yyyy)).z;
    // 266: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 267: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 268: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 269: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 270: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 271: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 272: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 273: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 274: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 275: mul r10.y, cb0[2].y, cb0[11].y
    r10.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 276: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 277: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 278: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 279: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 280: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 281: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t6.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 282: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 283: mul r1.w, r4.z, r10.w
    r1.w = ((r4.zzzz)*(r10.wwww)).w;
    // 284: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 285: mad r1.xyz, r1.wwww, r10.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 286: mul r1.w, cb0[12].y, cb0[22].y
    r1.w = ((source[12].yyyy)*(source[22].yyyy)).w;
    // 287: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 288: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 289: mul r10.y, r1.w, l(0.020000)
    r10.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 290: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 291: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 292: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 293: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 294: mul r3.z, cb0[12].x, l(0.001000)
    r3.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 295: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 296: mad r3.xy, r3.zzzz, r3.xyxx, r10.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r10.xyxx)).xy;
    // 297: dp2 r3.z, cb0[13].xyxx, r3.xyxx
    r3.z = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 298: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 299: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 300: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 301: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 302: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 303: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 304: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 305: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 306: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 307: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 308: mul r10.xyz, r3.xyzx, cb0[12].zzzz
    r10.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 309: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 310: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 311: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 312: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 313: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 314: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 315: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 316: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 317: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 318: mul r0.z, r2.w, cb0[24].z
    r0.z = ((r2.wwww)*(source[24].zzzz)).z;
    // 319: mul r0.w, r11.w, r0.z
    r0.w = ((r11.wwww)*(r0.zzzz)).w;
    // 320: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 321: min r0.w, r0.w, cb0[24].z
    r0.w = (min(r0.wwww,source[24].zzzz)).w;
    // 322: add r1.w, -cb0[25].y, cb0[25].x
    r1.w = ((-(source[25].yyyy))+(source[25].xxxx)).w;
    // 323: mad r1.w, cb0[24].w, r1.w, cb0[25].y
    r1.w = ((source[24].wwww)*(r1.wwww)+(source[25].yyyy)).w;
    // 324: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 325: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 326: mad r1.w, r11.w, r1.w, l(1.000000)
    r1.w = ((r11.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 327: lt r2.w, |r0.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 328: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 329: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 330: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 331: movc r0.y, r2.w, l(0), r0.w
    r0.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 332: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 333: add r0.y, cb0[25].w, -cb0[26].x
    r0.y = ((source[25].wwww)+(-(source[26].xxxx))).y;
    // 334: mad r0.y, cb0[25].z, r0.y, cb0[26].x
    r0.y = ((source[25].zzzz)*(r0.yyyy)+(source[26].xxxx)).y;
    // 335: mul r0.y, r0.y, r11.w
    r0.y = ((r0.yyyy)*(r11.wwww)).y;
    // 336: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t7.xwyz, s8, l(0.000000)
    r0.xzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // 337: mad r0.xyz, r0.yyyy, r5.xyzx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r5.xyzx)+(r0.xzwx)).xyz;
    // 338: add r0.w, -cb0[26].y, l(2.000000)
    r0.w = ((-(source[26].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 339: mad r0.w, r4.x, r0.w, cb0[26].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[26].yyyy)).w;
    // 340: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 341: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 342: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 343: mul r0.xyz, r0.xyzx, cb0[26].zzzz
    r0.xyz = ((r0.xyzx)*(source[26].zzzz)).xyz;
    // 344: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 345: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 346: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 347: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 348: mul r0.xyz, r0.xyzx, cb0[26].wwww
    r0.xyz = ((r0.xyzx)*(source[26].wwww)).xyz;
    // 349: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 350: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 351: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 352: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 353: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 354: mul o0.xyz, r0.xyzx, cb0[27].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[27].xyzx)).xyz;
    // 355: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 356: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 357: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 358: ret
    return output;
}

// source.character.equipment-native-165.v1 / source program cafe08c34ef6d141a65e7050c567476b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight165(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].y=(g_SourceCharacterTime.xxxx).x;
    source[21].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[21].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[22].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[22].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[26]=float4(input.lightColor,1.0);
    source[27].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[27].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[27].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r8.xyzw, r8.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r8.xyzw = ((r8.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 32: dp2 r1.w, r8.zwzz, r8.zwzz
    r1.w = (dot((r8.zwzz).xy,(r8.zwzz).xy).xxxx).w;
    // 33: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 35: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 36: add r9.z, r1.w, l(0.000010)
    r9.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 37: mul r9.xy, r8.xyxx, cb0[16].xxxx
    r9.xy = ((r8.xyxx)*(source[16].xxxx)).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mad r8.xy, cb0[16].wwww, r8.zwzz, -r9.xyxx
    r8.xy = ((source[16].wwww)*(r8.zwzz)+(-(r9.xyxx))).xy;
    // 40: mov r8.z, l(0)
    r8.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 41: mad r8.xyz, r10.wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((r10.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 42: add r9.xyz, -r8.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r8.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 43: mad r11.xyz, cb0[18].xxxx, r9.xyzx, r8.xyzx
    r11.xyz = ((source[18].xxxx)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 44: dp3 r1.w, r11.xyzx, r11.xyzx
    r1.w = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 45: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 46: div r11.xyz, r11.xyzx, r1.wwww
    r11.xyz = ((r11.xyzx)/(r1.wwww)).xyz;
    // 47: dp3 r12.x, r1.xyzx, r11.xyzx
    r12.x = (dot((r1.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 48: dp3 r12.y, r2.xyzx, r11.xyzx
    r12.y = (dot((r2.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 49: dp3 r12.z, r0.xyzx, r11.xyzx
    r12.z = (dot((r0.xyzx).xyz,(r11.xyzx).xyz).xxxx).z;
    // 50: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 51: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 52: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 53: dp3 r0.x, r12.xyzx, r1.xyzx
    r0.x = (dot((r12.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 54: mul r0.xyz, r12.xyzx, r0.xxxx
    r0.xyz = ((r12.xyzx)*(r0.xxxx)).xyz;
    // 55: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 56: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 57: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 58: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 59: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 60: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 61: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 63: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 64: add r1.x, -cb0[17].y, cb0[17].x
    r1.x = ((-(source[17].yyyy))+(source[17].xxxx)).x;
    // 65: mad r1.x, r10.w, r1.x, cb0[17].y
    r1.x = ((r10.wwww)*(r1.xxxx)+(source[17].yyyy)).x;
    // 66: lt r1.y, |r0.z|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 67: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 68: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 69: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 70: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 71: movc r0.z, r1.y, l(0), r0.z
    r0.z = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 72: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 73: add r1.y, -r1.x, cb0[18].y
    r1.y = ((-(r1.xxxx))+(source[18].yyyy)).y;
    // 74: mad r1.x, r10.w, r1.y, r1.x
    r1.x = ((r10.wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 75: mul r1.x, r1.x, cb0[18].z
    r1.x = ((r1.xxxx)*(source[18].zzzz)).x;
    // 76: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 77: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 78: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 79: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 80: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 81: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 82: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 83: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 84: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 85: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 86: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 87: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 88: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 89: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 90: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 91: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 92: rcp r0.x, cb0[18].w
    r0.x = (1.0/(source[18].wwww)).x;
    // 93: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 94: mul r11.xyz, r4.xyzx, cb0[18].wwww
    r11.xyz = ((r4.xyzx)*(source[18].wwww)).xyz;
    // 95: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 96: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 97: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 98: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 99: mad r4.xyz, r11.xyzx, cb0[18].wwww, r4.xyzx
    r4.xyz = ((r11.xyzx)*(source[18].wwww)+(r4.xyzx)).xyz;
    // 100: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 101: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 102: add r0.x, cb0[18].w, l(1.000000)
    r0.x = ((source[18].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 104: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 105: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 106: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 107: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 108: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 109: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 110: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 111: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 112: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 113: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 114: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 115: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 116: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 117: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 118: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 119: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 120: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 122: mul r11.xyz, r1.xywx, r4.xxxx
    r11.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 123: mul r12.xyz, r11.xyzx, cb0[22].wwww
    r12.xyz = ((r11.xyzx)*(source[22].wwww)).xyz;
    // 124: mul r4.z, r10.w, l(0.500000)
    r4.z = ((r10.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 125: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 127: mul r4.w, r0.z, r4.w
    r4.w = ((r0.zzzz)*(r4.wwww)).w;
    // 128: mul r4.z, r4.w, r4.z
    r4.z = ((r4.wwww)*(r4.zzzz)).z;
    // 129: mad r8.xyz, r4.zzzz, r9.xyzx, r8.xyzx
    r8.xyz = ((r4.zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 130: dp3 r4.z, r8.xyzx, r8.xyzx
    r4.z = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 131: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 132: div r8.xyz, r8.xyzx, r4.zzzz
    r8.xyz = ((r8.xyzx)/(r4.zzzz)).xyz;
    // 133: dp3 r4.z, r8.xyzx, r7.xyzx
    r4.z = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 134: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 135: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 137: dp3 r6.w, cb0[15].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[15].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 138: add r7.xyz, r6.wwww, -cb0[15].xyzx
    r7.xyz = ((r6.wwww)+(-(source[15].xyzx))).xyz;
    // 139: mad r7.xyz, r5.wwww, r7.xyzx, cb0[15].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[15].xyzx)).xyz;
    // 140: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 141: mad r7.xyz, r10.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r10.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 142: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 143: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 144: mad r7.xyz, r7.xyzx, r4.zzzz, r5.wwww
    r7.xyz = ((r7.xyzx)*(r4.zzzz)+(r5.wwww)).xyz;
    // 145: add_sat r4.z, r10.w, cb0[23].x
    r4.z = (saturate((r10.wwww)+(source[23].xxxx))).z;
    // 146: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: dp3 r6.x, r8.xyzx, r6.xyzx
    r6.x = (dot((r8.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 148: mul_sat r6.xy, r6.xzxx, cb0[19].zzzz
    r6.xy = (saturate((r6.xzxx)*(source[19].zzzz))).xy;
    // 149: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 150: add_sat r6.y, r6.y, -cb0[19].w
    r6.y = (saturate((r6.yyyy)+(-(source[19].wwww)))).y;
    // 151: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 152: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 153: mul r6.y, r6.y, cb0[20].x
    r6.y = ((r6.yyyy)*(source[20].xxxx)).y;
    // 154: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 155: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 156: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 157: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 158: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 159: mad r7.w, r2.w, l(2.000000), -r4.x
    r7.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).w;
    // 160: mad r6.yzw, r6.yyzw, r7.wwww, r4.xxxx
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r4.xxxx)).yzw;
    // 161: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 162: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 163: mul r7.w, r0.z, r0.z
    r7.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 164: mul r8.w, r7.w, cb0[23].y
    r8.w = ((r7.wwww)*(source[23].yyyy)).w;
    // 165: mad r0.z, -r7.w, cb0[23].y, r0.z
    r0.z = ((-(r7.wwww))*(source[23].yyyy)+(r0.zzzz)).z;
    // 166: mad r0.z, r10.w, r0.z, r8.w
    r0.z = ((r10.wwww)*(r0.zzzz)+(r8.wwww)).z;
    // 167: mad r6.yzw, r4.zzzz, r6.yyzw, -r7.xxyz
    r6.yzw = ((r4.zzzz)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 168: mad r6.yzw, r0.zzzz, r6.yyzw, r7.xxyz
    r6.yzw = ((r0.zzzz)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 169: sqrt r0.z, r5.w
    r0.z = (sqrt(r5.wwww)).z;
    // 170: mul r5.xyz, r5.xyzx, r0.zzzz
    r5.xyz = ((r5.xyzx)*(r0.zzzz)).xyz;
    // 171: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 172: mad r6.yzw, -cb0[22].wwww, r11.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[22].wwww))*(r11.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 173: mad r6.yzw, r5.xxyz, r6.yyzw, r12.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r12.xxyz)).yzw;
    // 174: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 175: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 176: mad r7.xyz, r10.xxxx, r9.xyzx, r7.xyzx
    r7.xyz = ((r10.xxxx)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 177: mad r9.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r9.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 178: mad r7.xyz, r10.yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((r10.yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 179: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 180: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 181: mad r7.xyz, cb0[17].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[17].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 182: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 184: mad r7.xyz, cb0[17].wwww, r9.xyzx, r7.xyzx
    r7.xyz = ((source[17].wwww)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 185: mad r9.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 186: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 188: mul r11.xyz, r7.xyzx, r9.xyzx
    r11.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 189: mad r7.xyz, -r7.xyzx, r9.xyzx, cb0[8].xyzx
    r7.xyz = ((-(r7.xyzx))*(r9.xyzx)+(source[8].xyzx)).xyz;
    // 190: mad r7.xyz, r10.wwww, r7.xyzx, r11.xyzx
    r7.xyz = ((r10.wwww)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 191: sample_b_indexable(texture2d)(float,float,float,float) r11.xyz, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r11.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 192: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r12.xyz, -r11.xyzx, r0.zzzz
    r12.xyz = ((-(r11.xyzx))+(r0.zzzz)).xyz;
    // 194: mad r11.xyz, cb0[17].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[17].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 195: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r12.xyz, -r11.xyzx, r0.zzzz
    r12.xyz = ((-(r11.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r11.xyz, cb0[17].wwww, r12.xyzx, r11.xyzx
    r11.xyz = ((source[17].wwww)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 198: mul r12.xyz, r7.xyzx, r11.xyzx
    r12.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 199: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 200: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 201: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 202: add r1.yzw, -cb0[9].xxyz, cb0[10].xxyz
    r1.yzw = ((-(source[9].xxyz))+(source[10].xxyz)).yzw;
    // 203: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[9].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[9].xyzx)).xyz;
    // 204: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 205: mul r1.xyz, r1.xyzx, cb0[19].xxxx
    r1.xyz = ((r1.xyzx)*(source[19].xxxx)).xyz;
    // 206: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 207: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 208: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 209: mad r14.xyz, cb0[17].zzzz, r14.xyzx, r2.xyzx
    r14.xyz = ((source[17].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 210: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 211: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 212: mad r14.xyz, cb0[17].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[17].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 213: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 214: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 215: mul r15.xyz, r15.xyzx, cb0[19].yyyy
    r15.xyz = ((r15.xyzx)*(source[19].yyyy)).xyz;
    // 216: add r0.z, r10.y, r10.x
    r0.z = ((r10.yyyy)+(r10.xxxx)).z;
    // 217: add r0.z, r10.z, r0.z
    r0.z = ((r10.zzzz)+(r0.zzzz)).z;
    // 218: add_sat r0.z, r10.w, r0.z
    r0.z = (saturate((r10.wwww)+(r0.zzzz))).z;
    // 219: mad r10.xyz, r0.zzzz, r15.xyzx, r14.xyzx
    r10.xyz = ((r0.zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 220: add r2.xyz, r2.xxxx, -r10.xyzx
    r2.xyz = ((r2.xxxx)+(-(r10.xyzx))).xyz;
    // 221: mad r2.xyz, r10.wwww, r2.xyzx, r10.xyzx
    r2.xyz = ((r10.wwww)*(r2.xyzx)+(r10.xyzx)).xyz;
    // 222: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 223: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 224: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 225: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 226: dp3 r0.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 227: add r1.w, -cb0[20].z, cb0[20].y
    r1.w = ((-(source[20].zzzz))+(source[20].yyyy)).w;
    // 228: mad r1.w, r10.w, r1.w, cb0[20].z
    r1.w = ((r10.wwww)*(r1.wwww)+(source[20].zzzz)).w;
    // 229: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 230: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 231: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 232: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 233: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 234: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 235: div r1.w, cb0[20].w, r1.w
    r1.w = ((source[20].wwww)/(r1.wwww)).w;
    // 236: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 237: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 238: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: mul r1.w, r1.w, cb0[21].x
    r1.w = ((r1.wwww)*(source[21].xxxx)).w;
    // 240: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 241: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 242: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 243: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 244: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 245: add r10.xyz, -r1.xyzx, r1.wwww
    r10.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 246: mad r1.xyz, cb0[17].zzzz, r10.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 247: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 248: add r10.xyz, -r1.xyzx, r1.wwww
    r10.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 249: mad r1.xyz, cb0[17].wwww, r10.xyzx, r1.xyzx
    r1.xyz = ((source[17].wwww)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 250: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 251: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 252: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 253: mul r4.z, r4.z, cb0[21].y
    r4.z = ((r4.zzzz)*(source[21].yyyy)).z;
    // 254: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 255: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 256: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 257: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 258: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 259: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 260: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 261: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 262: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 263: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 264: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 265: mul r10.x, r6.x, l(0.125000)
    r10.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 266: mov r10.y, v4.y
    r10.y = (v4.yyyy).y;
    // 267: add r9.xy, r9.xyxx, r10.xyxx
    r9.xy = ((r9.xyxx)+(r10.xyxx)).xy;
    // 268: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 269: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 270: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 271: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 272: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 273: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 274: mul r1.w, cb0[12].y, cb0[21].y
    r1.w = ((source[12].yyyy)*(source[21].yyyy)).w;
    // 275: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 276: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 277: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 278: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 279: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 280: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 281: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 282: mul r3.z, cb0[12].x, l(0.001000)
    r3.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 283: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 284: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 285: dp2 r3.z, cb0[13].xyxx, r3.xyxx
    r3.z = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 286: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 287: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 288: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 289: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 290: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 291: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 292: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 293: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 294: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 295: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 296: mul r9.xyz, r3.xyzx, cb0[12].zzzz
    r9.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 297: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 298: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 299: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 300: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 301: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 302: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 303: mad r5.xyz, r7.xyzx, r11.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r11.xyzx)+(-(r2.xyzx))).xyz;
    // 304: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 305: dp3 r0.x, r0.xywx, r8.xyzx
    r0.x = (dot((r0.xywx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 306: mul r0.z, r2.w, cb0[23].z
    r0.z = ((r2.wwww)*(source[23].zzzz)).z;
    // 307: mul r0.w, r10.w, r0.z
    r0.w = ((r10.wwww)*(r0.zzzz)).w;
    // 308: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 309: min r0.w, r0.w, cb0[23].z
    r0.w = (min(r0.wwww,source[23].zzzz)).w;
    // 310: add r1.w, -cb0[24].y, cb0[24].x
    r1.w = ((-(source[24].yyyy))+(source[24].xxxx)).w;
    // 311: mad r1.w, cb0[23].w, r1.w, cb0[24].y
    r1.w = ((source[23].wwww)*(r1.wwww)+(source[24].yyyy)).w;
    // 312: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 313: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 314: mad r1.w, r10.w, r1.w, l(1.000000)
    r1.w = ((r10.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 315: lt r2.w, |r0.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 316: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 317: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 318: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 319: movc r0.y, r2.w, l(0), r0.w
    r0.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 320: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 321: add r0.y, cb0[24].w, -cb0[25].x
    r0.y = ((source[24].wwww)+(-(source[25].xxxx))).y;
    // 322: mad r0.y, cb0[24].z, r0.y, cb0[25].x
    r0.y = ((source[24].zzzz)*(r0.yyyy)+(source[25].xxxx)).y;
    // 323: mul r0.y, r0.y, r10.w
    r0.y = ((r0.yyyy)*(r10.wwww)).y;
    // 324: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t6.xwyz, s7, l(0.000000)
    r0.xzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // 325: mad r0.xyz, r0.yyyy, r5.xyzx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r5.xyzx)+(r0.xzwx)).xyz;
    // 326: add r0.w, -cb0[25].y, l(2.000000)
    r0.w = ((-(source[25].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 327: mad r0.w, r4.x, r0.w, cb0[25].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[25].yyyy)).w;
    // 328: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 329: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 330: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 331: mul r0.xyz, r0.xyzx, cb0[25].zzzz
    r0.xyz = ((r0.xyzx)*(source[25].zzzz)).xyz;
    // 332: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 333: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 334: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 335: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 336: mul r0.xyz, r0.xyzx, cb0[25].wwww
    r0.xyz = ((r0.xyzx)*(source[25].wwww)).xyz;
    // 337: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 338: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 339: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 340: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 341: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 342: mul o0.xyz, r0.xyzx, cb0[26].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[26].xyzx)).xyz;
    // 343: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 344: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 345: ret
    return output;
}

// source.character.equipment-native-166.v1 / source program 2482d72e2a8d1a44880af1e2187322d1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight166(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].w=(g_SourceCharacterTime.xxxx).x;
    source[23]=float4(input.lightColor,1.0);
    source[29].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[29].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[29].yyyy)) * 0xffffffffu)).w;
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
    // 11: mul r3.xyzw, r2.yyyy, cb0[25].xyzw
    r3.xyzw = ((r2.yyyy)*(source[25].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[24].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[24].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[26].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[26].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[27].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[27].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s4
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[28].wwzw
    r4.yz = (source[28].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s4
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s4
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[28].zwzz
    r4.xy = ((r2.xyxx)+(source[28].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s4
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[28].xyxx
    r2.xy = ((r2.xyxx)*(source[28].xyxx)).xy;
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
    // 33: mul r2.xyz, r0.wwww, cb0[29].xxxx
    r2.xyz = ((r0.wwww)*(source[29].xxxx)).xyz;
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
    // 44: mul r5.xyz, r5.xyzx, cb0[15].xxxx
    r5.xyz = ((r5.xyzx)*(source[15].xxxx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r1.w, cb0[15].y, l(-3.500000), l(5.000000)
    r1.w = ((source[15].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 47: mul r1.w, r1.w, cb0[16].x
    r1.w = ((r1.wwww)*(source[16].xxxx)).w;
    // 48: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: add r4.w, -r2.w, v4.z
    r4.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 50: mad r2.w, cb0[16].y, r4.w, r2.w
    r2.w = ((source[16].yyyy)*(r4.wwww)+(r2.wwww)).w;
    // 51: mul r4.w, r2.w, cb0[16].z
    r4.w = ((r2.wwww)*(source[16].zzzz)).w;
    // 52: mad r2.w, r4.w, l(0.750000), r2.w
    r2.w = ((r4.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 53: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 54: mad_sat r2.w, cb0[17].x, r2.w, r2.w
    r2.w = (saturate((source[17].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t1.xywz, s0, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 56: mad r8.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r4.w, r8.xyxx, r8.xyxx
    r4.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 58: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r4.w, r4.w, l(0.000000)
    r4.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 61: add r8.z, r4.w, l(0.000010)
    r8.z = ((r4.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mad r4.w, r8.x, r6.x, l(0.200000)
    r4.w = ((r8.xxxx)*(r6.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 63: add r5.w, -r6.y, l(1.000000)
    r5.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: add r5.w, -r4.w, r5.w
    r5.w = ((-(r4.wwww))+(r5.wwww)).w;
    // 65: mad r7.x, cb0[17].z, r5.w, r4.w
    r7.x = ((source[17].zzzz)*(r5.wwww)+(r4.wwww)).x;
    // 66: mul r7.y, cb0[16].w, l(0.700000)
    r7.y = ((source[16].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).y;
    // 67: add r7.x, -r2.w, r7.x
    r7.x = ((-(r2.wwww))+(r7.xxxx)).x;
    // 68: mad r7.x, r7.y, r7.x, r2.w
    r7.x = ((r7.yyyy)*(r7.xxxx)+(r2.wwww)).x;
    // 69: div r7.x, r7.x, cb0[17].y
    r7.x = ((r7.xxxx)/(source[17].yyyy)).x;
    // 70: add r7.x, -r7.x, l(1.000000)
    r7.x = ((-(r7.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 71: mul r7.x, r1.w, r7.x
    r7.x = ((r1.wwww)*(r7.xxxx)).x;
    // 72: mul r7.x, r7.x, l(4.000000)
    r7.x = ((r7.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 73: add r7.w, v4.w, l(0.500000)
    r7.w = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 74: round_ni r7.w, r7.w
    r7.w = (floor(r7.wwww)).w;
    // 75: mul_sat r7.x, r7.w, r7.x
    r7.x = (saturate((r7.wwww)*(r7.xxxx))).x;
    // 76: mad r4.w, cb0[18].x, r5.w, r4.w
    r4.w = ((source[18].xxxx)*(r5.wwww)+(r4.wwww)).w;
    // 77: add r4.w, -r2.w, r4.w
    r4.w = ((-(r2.wwww))+(r4.wwww)).w;
    // 78: mad r2.w, r7.y, r4.w, r2.w
    r2.w = ((r7.yyyy)*(r4.wwww)+(r2.wwww)).w;
    // 79: div r2.w, r2.w, cb0[17].w
    r2.w = ((r2.wwww)/(source[17].wwww)).w;
    // 80: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 82: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 83: add r2.w, -r7.w, l(1.000000)
    r2.w = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 85: add r1.w, r1.w, r7.x
    r1.w = ((r1.wwww)+(r7.xxxx)).w;
    // 86: add r1.w, -r6.y, r1.w
    r1.w = ((-(r6.yyyy))+(r1.wwww)).w;
    // 87: mad r1.w, cb0[18].y, r1.w, r6.y
    r1.w = ((source[18].yyyy)*(r1.wwww)+(r6.yyyy)).w;
    // 88: mad r5.xyz, r1.wwww, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 89: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: add r7.xyw, -r5.xyxz, r1.wwww
    r7.xyw = ((-(r5.xyxz))+(r1.wwww)).xyw;
    // 91: mad r5.xyz, cb0[18].zzzz, r7.xywx, r5.xyzx
    r5.xyz = ((source[18].zzzz)*(r7.xywx)+(r5.xyzx)).xyz;
    // 92: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r7.xyw, -r5.xyxz, r1.wwww
    r7.xyw = ((-(r5.xyxz))+(r1.wwww)).xyw;
    // 94: mad r5.xyz, cb0[18].wwww, r7.xywx, r5.xyzx
    r5.xyz = ((source[18].wwww)*(r7.xywx)+(r5.xyzx)).xyz;
    // 95: mad r7.xyw, cb0[4].wwww, cb0[4].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[4].wwww)*(source[4].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 96: mad r9.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 97: mul r7.xyw, r7.xyxw, r9.xyxz
    r7.xyw = ((r7.xyxw)*(r9.xyxz)).xyw;
    // 98: mul r9.xyz, r5.xyzx, r7.xywx
    r9.xyz = ((r5.xyzx)*(r7.xywx)).xyz;
    // 99: mul r10.xyz, r6.xxxx, r9.xyzx
    r10.xyz = ((r6.xxxx)*(r9.xyzx)).xyz;
    // 100: lt r1.w, |r7.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r7.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 101: mul r2.w, |r7.z|, |r7.z|
    r2.w = ((abs(r7.zzzz))*(abs(r7.zzzz))).w;
    // 102: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 103: mad r11.xyz, cb0[6].xyzx, r6.xyzx, -r6.xyzx
    r11.xyz = ((source[6].xyzx)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 104: mad r11.xyz, r1.wwww, r11.xyzx, r6.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)+(r6.xyzx)).xyz;
    // 105: mad r9.xyz, -r6.xxxx, r9.xyzx, r11.xyzx
    r9.xyz = ((-(r6.xxxx))*(r9.xyzx)+(r11.xyzx)).xyz;
    // 106: mad r9.xyz, r1.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 107: add r10.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = ((r9.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 108: dp3 r2.w, r10.xyzx, r10.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 109: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 110: div r10.xyz, r10.xyzx, r2.wwww
    r10.xyz = ((r10.xyzx)/(r2.wwww)).xyz;
    // 111: mul r10.xyz, r10.xyzx, cb0[9].xyzx
    r10.xyz = ((r10.xyzx)*(source[9].xyzx)).xyz;
    // 112: add r2.w, -|r3.z|, l(1.000000)
    r2.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: dp3 r4.w, r8.xyzx, r8.xyzx
    r4.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 114: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 115: div r8.xyz, r8.xyzx, r4.wwww
    r8.xyz = ((r8.xyzx)/(r4.wwww)).xyz;
    // 116: dp3 r4.w, r8.xyzx, r3.xyzx
    r4.w = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 117: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 119: lt r4.w, |r2.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 120: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 121: mul r2.w, r2.w, cb0[21].z
    r2.w = ((r2.wwww)*(source[21].zzzz)).w;
    // 122: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 123: movc r2.w, r4.w, l(0), r2.w
    r2.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 124: mul r4.w, cb0[10].z, l(1.500000)
    r4.w = ((source[10].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 125: add r5.w, -cb0[10].w, l(1.000000)
    r5.w = ((-(source[10].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r5.w, r5.w, cb0[21].w
    r5.w = ((r5.wwww)*(source[21].wwww)).w;
    // 127: mul r5.w, r5.w, l(6.283185)
    r5.w = ((r5.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 128: sincos r5.w, null, r5.w
    r5.w = (sin(r5.wwww)).w;
    // 129: add r5.w, r5.w, l(1.000000)
    r5.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 131: mad r4.w, r4.w, l(0.500000), cb0[10].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].zzzz)).w;
    // 132: frc r5.w, cb0[10].x
    r5.w = (frac(source[10].xxxx)).w;
    // 133: add r6.x, -r5.w, cb0[10].x
    r6.x = ((-(r5.wwww))+(source[10].xxxx)).x;
    // 134: mul r12.z, r6.x, l(0.125000)
    r12.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 135: mov r12.xw, l(0,0,0,0)
    r12.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 136: mul r12.y, cb0[10].y, cb0[11].y
    r12.y = ((source[10].yyyy)*(source[11].yyyy)).y;
    // 137: mul r13.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r13.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 138: frc r6.x, r13.x
    r6.x = (frac(r13.xxxx)).x;
    // 139: mul r13.y, r6.x, l(0.125000)
    r13.y = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 140: add r6.xy, r12.xyxx, r13.yzyy
    r6.xy = ((r12.xyxx)+(r13.yzyy)).xy;
    // 141: add r6.xy, r6.xyxx, r12.zwzz
    r6.xy = ((r6.xyxx)+(r12.zwzz)).xy;
    // 142: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 143: mul r12.xyz, r4.wwww, r12.xyzx
    r12.xyz = ((r4.wwww)*(r12.xyzx)).xyz;
    // 144: mul r4.w, r5.w, r12.w
    r4.w = ((r5.wwww)*(r12.wwww)).w;
    // 145: mad r12.xyz, r12.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r12.xyz = ((r12.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 146: mad r9.xyz, r4.wwww, r12.xyzx, r9.xyzx
    r9.xyz = ((r4.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 147: mul r4.w, cb0[12].y, cb0[21].w
    r4.w = ((source[12].yyyy)*(source[21].wwww)).w;
    // 148: mul r4.w, r4.w, l(0.628319)
    r4.w = ((r4.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 149: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 150: mul r6.y, r4.w, l(0.020000)
    r6.y = ((r4.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 151: add r12.xyzw, r1.yzxy, -cb0[1].yzxy
    r12.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 152: add r12.xy, -r12.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r12.xy = ((-(r12.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 153: add r12.xy, -r12.zwzz, r12.xyxx
    r12.xy = ((-(r12.zwzz))+(r12.xyxx)).xy;
    // 154: mad r12.xy, cb0[12].wwww, r12.xyxx, r12.zwzz
    r12.xy = ((source[12].wwww)*(r12.xyxx)+(r12.zwzz)).xy;
    // 155: mul r5.w, cb0[12].x, l(0.001000)
    r5.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 156: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 157: mad r6.xy, r5.wwww, r12.xyxx, r6.xyxx
    r6.xy = ((r5.wwww)*(r12.xyxx)+(r6.xyxx)).xy;
    // 158: dp2 r5.w, cb0[13].xyxx, r6.xyxx
    r5.w = (dot((source[13].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 159: dp2 r6.y, cb0[14].xyxx, r6.xyxx
    r6.y = (dot((source[14].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 160: frc r5.w, r5.w
    r5.w = (frac(r5.wwww)).w;
    // 161: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 162: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 163: mul r5.w, r12.w, l(0.900000)
    r5.w = ((r12.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 164: mad r12.xyz, r12.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r9.xyzx
    r12.xyz = ((r12.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r9.xyzx))).xyz;
    // 165: mad r12.xyz, r5.wwww, r12.xyzx, r9.xyzx
    r12.xyz = ((r5.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 166: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 168: mul_sat r12.xyz, r12.xyzx, r4.wwww
    r12.xyz = (saturate((r12.xyzx)*(r4.wwww))).xyz;
    // 169: mul r13.xyz, r12.xyzx, cb0[12].zzzz
    r13.xyz = ((r12.xyzx)*(source[12].zzzz)).xyz;
    // 170: dp3 r4.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 171: mul r4.w, r4.w, l(3.000000)
    r4.w = ((r4.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 172: mad r12.xyz, cb0[12].zzzz, r12.xyzx, -r9.xyzx
    r12.xyz = ((source[12].zzzz)*(r12.xyzx)+(-(r9.xyzx))).xyz;
    // 173: mad r9.xyz, r4.wwww, r12.xyzx, r9.xyzx
    r9.xyz = ((r4.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 174: dp3 r4.w, r8.xyzx, r4.xyzx
    r4.w = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 175: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 176: min r6.x, r5.w, l(1.000000)
    r6.x = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 177: mul r12.xyz, r2.xyzx, r6.xxxx
    r12.xyz = ((r2.xyzx)*(r6.xxxx)).xyz;
    // 178: mad r13.xyz, -r6.xxxx, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r6.xxxx))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mad r13.xyz, cb0[22].xxxx, r13.xyzx, r12.xyzx
    r13.xyz = ((source[22].xxxx)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 180: dp3 r6.x, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.x = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 181: add r14.xyz, r6.xxxx, -cb0[2].xyzx
    r14.xyz = ((r6.xxxx)+(-(source[2].xyzx))).xyz;
    // 182: mad r14.xyz, cb0[18].zzzz, r14.xyzx, cb0[2].xyzx
    r14.xyz = ((source[18].zzzz)*(r14.xyzx)+(source[2].xyzx)).xyz;
    // 183: dp3 r6.x, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.x = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 184: add r15.xyz, -r14.xyzx, r6.xxxx
    r15.xyz = ((-(r14.xyzx))+(r6.xxxx)).xyz;
    // 185: mad r14.xyz, cb0[18].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[18].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 186: mul r15.xyz, r14.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r15.xyz = ((r14.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 187: mad r16.xyz, -r14.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 190: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 191: mul r15.xyz, r15.xyzx, r4.wwww
    r15.xyz = ((r15.xyzx)*(r4.wwww)).xyz;
    // 192: mad r13.xyz, r13.xyzx, r16.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r16.xyzx)+(r15.xyzx)).xyz;
    // 193: mov_sat r4.w, r4.z
    r4.w = (saturate(r4.zzzz)).w;
    // 194: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 195: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 196: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 197: add r15.xyz, -r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mad r14.xyz, r4.wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((r4.wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 199: mad r14.xyz, r13.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r14.xyzx
    r14.xyz = ((r13.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r14.xyzx)).xyz;
    // 200: mul_sat r9.xyz, r9.xyzx, r14.xyzx
    r9.xyz = (saturate((r9.xyzx)*(r14.xyzx))).xyz;
    // 201: mul r9.xyz, r13.xyzx, r9.xyzx
    r9.xyz = ((r13.xyzx)*(r9.xyzx)).xyz;
    // 202: mad r12.yzw, r12.xxyz, r11.xxyz, -r9.xxyz
    r12.yzw = ((r12.xxyz)*(r11.xxyz)+(-(r9.xxyz))).yzw;
    // 203: mad r9.xyz, r1.wwww, r12.yzwy, r9.xyzx
    r9.xyz = ((r1.wwww)*(r12.yzwy)+(r9.xyzx)).xyz;
    // 204: max r2.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 205: min r2.xyz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 206: mad r5.xyz, r5.xyzx, r7.xywx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r5.xyz = ((r5.xyzx)*(r7.xywx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 207: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 208: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 209: div r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)/(r4.wwww)).xyz;
    // 210: mul r5.xyz, r5.xyzx, r6.zzzz
    r5.xyz = ((r5.xyzx)*(r6.zzzz)).xyz;
    // 211: mov_sat r4.w, r3.z
    r4.w = (saturate(r3.zzzz)).w;
    // 212: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 213: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 214: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 215: mul r5.xyz, r5.xyzx, cb0[19].xxxx
    r5.xyz = ((r5.xyzx)*(source[19].xxxx)).xyz;
    // 216: mad r6.xyz, cb0[19].yyyy, r11.xyzx, -r5.xyzx
    r6.xyz = ((source[19].yyyy)*(r11.xyzx)+(-(r5.xyzx))).xyz;
    // 217: mad r5.xyz, r1.wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 218: max r4.w, r12.x, l(0.500000)
    r4.w = (max(r12.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 219: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 220: dp3 r6.x, r0.xyzx, r8.xyzx
    r6.x = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 221: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 222: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 223: mul r3.w, r3.z, r1.z
    r3.w = ((r3.zzzz)*(r1.zzzz)).w;
    // 224: mad r1.xyz, r3.zzwz, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r3.zzwz)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 225: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 226: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 227: div r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)/(r0.yyyy)).y;
    // 228: add r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)+(source[7].zzzz)).y;
    // 229: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 230: add r0.x, r0.x, r6.x
    r0.x = ((r0.xxxx)+(r6.xxxx)).x;
    // 231: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 232: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 233: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 234: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 235: add r0.y, -cb0[20].x, l(0.200000)
    r0.y = ((-(source[20].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 236: mad r0.y, r1.w, r0.y, cb0[20].x
    r0.y = ((r1.wwww)*(r0.yyyy)+(source[20].xxxx)).y;
    // 237: mad r0.y, r0.y, l(4.500000), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 238: mul r0.y, r0.y, cb0[22].y
    r0.y = ((r0.yyyy)*(source[22].yyyy)).y;
    // 239: mul r0.y, r0.y, l(0.050000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 240: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 241: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 242: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 243: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 244: mul r0.x, r4.w, r0.x
    r0.x = ((r4.wwww)*(r0.xxxx)).x;
    // 245: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 246: mad r0.yzw, v5.xxyz, r0.wwww, r3.xxyz
    r0.yzw = ((v5.xxyz)*(r0.wwww)+(r3.xxyz)).yzw;
    // 247: dp3 r0.y, r0.yzwy, r0.yzwy
    r0.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 248: mad r0.y, r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 249: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 250: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 251: mul r0.x, r0.x, cb0[22].z
    r0.x = ((r0.xxxx)*(source[22].zzzz)).x;
    // 252: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 253: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 254: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 255: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 256: add r0.xyz, r0.xyzx, r9.xyzx
    r0.xyz = ((r0.xyzx)+(r9.xyzx)).xyz;
    // 257: mad r0.xyz, r2.wwww, r10.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 258: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 259: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 260: mul o0.xyz, r0.xyzx, cb0[23].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)).xyz;
    // 261: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t4.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 262: dp3 r0.x, r0.xyzx, cb0[8].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[8].xyzx).xyz).xxxx).x;
    // 263: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[8].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[8].xyzx).xyz).xxxx)).y;
    // 264: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 265: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 266: mul r0.x, r0.x, r6.w
    r0.x = ((r0.xxxx)*(r6.wwww)).x;
    // 267: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 268: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 269: ret
    return output;
}

// source.character.equipment-native-167.v1 / source program bbdf754c82c11b408ace8b1cf461f5b2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight167(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].w=(g_SourceCharacterTime.xxxx).x;
    source[23]=float4(input.lightColor,1.0);
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: dp3 r0.x, r0.xyzx, cb0[13].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[13].xyzx).xyz).xxxx).x;
    // 3: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[13].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[13].xyzx).xyz).xxxx)).y;
    // 4: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 5: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 7: add_sat r0.y, r1.w, -cb0[21].x
    r0.y = (saturate((r1.wwww)+(-(source[21].xxxx)))).y;
    // 8: mad r0.x, r0.y, r0.x, l(-0.001000)
    r0.x = ((r0.yyyy)*(r0.xxxx)+(float4(-0.001000,-0.001000,-0.001000,-0.001000))).x;
    // 9: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 10: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 11: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 12: add r0.x, v4.w, l(0.500000)
    r0.x = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 13: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 14: add r0.y, -r0.x, l(1.000000)
    r0.y = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mad r0.z, cb0[15].y, l(-3.500000), l(5.000000)
    r0.z = ((source[15].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 16: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 17: add r0.w, -v4.z, l(1.000000)
    r0.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: add r1.w, -r0.w, v4.z
    r1.w = ((-(r0.wwww))+(v4.zzzz)).w;
    // 19: mad r0.w, cb0[16].y, r1.w, r0.w
    r0.w = ((source[16].yyyy)*(r1.wwww)+(r0.wwww)).w;
    // 20: mul r1.w, r0.w, cb0[16].z
    r1.w = ((r0.wwww)*(source[16].zzzz)).w;
    // 21: mad r0.w, r1.w, l(0.750000), r0.w
    r0.w = ((r1.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.wwww)).w;
    // 22: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 23: mad_sat r0.w, cb0[17].x, r0.w, r0.w
    r0.w = (saturate((source[17].xxxx)*(r0.wwww)+(r0.wwww))).w;
    // 24: mul r1.w, cb0[16].w, l(0.700000)
    r1.w = ((source[16].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 25: add r2.x, -r1.y, l(1.000000)
    r2.x = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r2.yzw, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yzw;
    // 27: mad r3.xy, r2.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r2.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 28: mad r2.y, r3.x, r1.x, l(0.200000)
    r2.y = ((r3.xxxx)*(r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 29: add r2.x, -r2.y, r2.x
    r2.x = ((-(r2.yyyy))+(r2.xxxx)).x;
    // 30: mad r2.z, cb0[18].x, r2.x, r2.y
    r2.z = ((source[18].xxxx)*(r2.xxxx)+(r2.yyyy)).z;
    // 31: mad r2.x, cb0[17].z, r2.x, r2.y
    r2.x = ((source[17].zzzz)*(r2.xxxx)+(r2.yyyy)).x;
    // 32: add r2.x, -r0.w, r2.x
    r2.x = ((-(r0.wwww))+(r2.xxxx)).x;
    // 33: mad r2.x, r1.w, r2.x, r0.w
    r2.x = ((r1.wwww)*(r2.xxxx)+(r0.wwww)).x;
    // 34: div r2.x, r2.x, cb0[17].y
    r2.x = ((r2.xxxx)/(source[17].yyyy)).x;
    // 35: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mul r2.x, r0.z, r2.x
    r2.x = ((r0.zzzz)*(r2.xxxx)).x;
    // 37: mul r2.x, r2.x, l(4.000000)
    r2.x = ((r2.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 38: mul_sat r0.x, r0.x, r2.x
    r0.x = (saturate((r0.xxxx)*(r2.xxxx))).x;
    // 39: add r2.x, -r0.w, r2.z
    r2.x = ((-(r0.wwww))+(r2.zzzz)).x;
    // 40: mad r0.w, r1.w, r2.x, r0.w
    r0.w = ((r1.wwww)*(r2.xxxx)+(r0.wwww)).w;
    // 41: div r0.w, r0.w, cb0[17].w
    r0.w = ((r0.wwww)/(source[17].wwww)).w;
    // 42: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 44: mul r0.z, r0.z, l(4.000000)
    r0.z = ((r0.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).z;
    // 45: mul_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)*(r0.zzzz))).y;
    // 46: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 47: add r0.x, -r1.y, r0.x
    r0.x = ((-(r1.yyyy))+(r0.xxxx)).x;
    // 48: mad r0.x, cb0[18].y, r0.x, r1.y
    r0.x = ((source[18].yyyy)*(r0.xxxx)+(r1.yyyy)).x;
    // 49: add r0.yzw, -cb0[2].xxyz, cb0[3].xxyz
    r0.yzw = ((-(source[2].xxyz))+(source[3].xxyz)).yzw;
    // 50: mul r0.yzw, r0.yyzw, cb0[15].xxxx
    r0.yzw = ((r0.yyzw)*(source[15].xxxx)).yzw;
    // 51: mad r0.xyz, r0.xxxx, r0.yzwy, cb0[2].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[2].xyzx)).xyz;
    // 52: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 54: mad r0.xyz, cb0[18].zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((source[18].zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 55: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 56: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 57: mad r0.xyz, cb0[18].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[18].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 58: mad r2.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 59: mad r4.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 60: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 61: mul r4.xyz, r0.xyzx, r2.xyzx
    r4.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 62: mad r0.xyz, r0.xyzx, r2.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 63: mad r2.xyz, cb0[7].xyzx, r1.xyzx, -r1.xyzx
    r2.xyz = ((source[7].xyzx)*(r1.xyzx)+(-(r1.xyzx))).xyz;
    // 64: mad r2.xyz, r2.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 65: mad r5.xyz, -r1.xxxx, r4.xyzx, r2.xyzx
    r5.xyz = ((-(r1.xxxx))*(r4.xyzx)+(r2.xyzx)).xyz;
    // 66: mul r4.xyz, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // 67: mad r4.xyz, r2.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 68: add r0.w, -cb0[8].w, l(1.000000)
    r0.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: mul r0.w, r0.w, cb0[20].w
    r0.w = ((r0.wwww)*(source[20].wwww)).w;
    // 70: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 71: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 72: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: mul r1.w, cb0[8].z, l(1.500000)
    r1.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 74: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 75: mad r0.w, r0.w, l(0.500000), cb0[8].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 76: mul r5.y, cb0[8].y, cb0[9].y
    r5.y = ((source[8].yyyy)*(source[9].yyyy)).y;
    // 77: mul r6.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r6.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 78: frc r1.w, r6.x
    r1.w = (frac(r6.xxxx)).w;
    // 79: mul r6.y, r1.w, l(0.125000)
    r6.y = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 80: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 81: add r5.xy, r5.xyxx, r6.yzyy
    r5.xy = ((r5.xyxx)+(r6.yzyy)).xy;
    // 82: frc r1.w, cb0[8].x
    r1.w = (frac(source[8].xxxx)).w;
    // 83: add r3.w, -r1.w, cb0[8].x
    r3.w = ((-(r1.wwww))+(source[8].xxxx)).w;
    // 84: mul r5.z, r3.w, l(0.125000)
    r5.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 85: add r5.xy, r5.xyxx, r5.zwzz
    r5.xy = ((r5.xyxx)+(r5.zwzz)).xy;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 87: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 88: mul r0.w, r1.w, r5.w
    r0.w = ((r1.wwww)*(r5.wwww)).w;
    // 89: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 90: mad r5.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 91: add r4.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 92: add r6.xyz, v8.xyzx, cb0[0].xyzx
    r6.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 93: add r7.xyzw, r6.yzxy, -cb0[1].yzxy
    r7.xyzw = ((r6.yzxy)+(-(source[1].yzxy))).xyzw;
    // 94: add r6.xyz, -r6.xyzx, cb0[0].xyzx
    r6.xyz = ((-(r6.xyzx))+(source[0].xyzx)).xyz;
    // 95: add r7.xy, -r7.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r7.xy = ((-(r7.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 96: add r7.xy, -r7.zwzz, r7.xyxx
    r7.xy = ((-(r7.zwzz))+(r7.xyxx)).xy;
    // 97: mad r7.xy, cb0[10].wwww, r7.xyxx, r7.zwzz
    r7.xy = ((source[10].wwww)*(r7.xyxx)+(r7.zwzz)).xy;
    // 98: mul r0.w, cb0[10].y, cb0[20].w
    r0.w = ((source[10].yyyy)*(source[20].wwww)).w;
    // 99: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 100: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 101: mul r8.y, r0.w, l(0.020000)
    r8.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 102: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 104: mul r1.w, cb0[10].x, l(0.001000)
    r1.w = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 105: mov r8.x, l(0)
    r8.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 106: mad r7.xy, r1.wwww, r7.xyxx, r8.xyxx
    r7.xy = ((r1.wwww)*(r7.xyxx)+(r8.xyxx)).xy;
    // 107: dp2 r1.w, cb0[11].xyxx, r7.xyxx
    r1.w = (dot((source[11].xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 108: dp2 r7.y, cb0[12].xyxx, r7.xyxx
    r7.y = (dot((source[12].xyxx).xy,(r7.xyxx).xy).xxxx).y;
    // 109: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 110: mul r7.x, r1.w, l(0.125000)
    r7.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 111: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t3.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 112: mad r7.xyz, r7.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r5.xyzx
    r7.xyz = ((r7.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r5.xyzx))).xyz;
    // 113: mul r1.w, r7.w, l(0.900000)
    r1.w = ((r7.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 114: mad r7.xyz, r1.wwww, r7.xyzx, r5.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 115: mul_sat r7.xyz, r0.wwww, r7.xyzx
    r7.xyz = (saturate((r0.wwww)*(r7.xyzx))).xyz;
    // 116: mad r8.xyz, cb0[10].zzzz, r7.xyzx, -r5.xyzx
    r8.xyz = ((source[10].zzzz)*(r7.xyzx)+(-(r5.xyzx))).xyz;
    // 117: mul r7.xyz, r7.xyzx, cb0[10].zzzz
    r7.xyz = ((r7.xyzx)*(source[10].zzzz)).xyz;
    // 118: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 120: mad r5.xyz, r0.wwww, r8.xyzx, r5.xyzx
    r5.xyz = ((r0.wwww)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 121: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 122: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 125: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 126: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 127: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 128: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 129: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 130: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 131: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 132: dp3 r1.w, r3.xyzx, r7.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 133: add r3.w, r1.w, l(1.000000)
    r3.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 135: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 136: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 137: dp3 r4.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 138: add r8.xyz, r4.wwww, -cb0[2].xyzx
    r8.xyz = ((r4.wwww)+(-(source[2].xyzx))).xyz;
    // 139: mad r8.xyz, cb0[18].zzzz, r8.xyzx, cb0[2].xyzx
    r8.xyz = ((source[18].zzzz)*(r8.xyzx)+(source[2].xyzx)).xyz;
    // 140: dp3 r4.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 141: add r9.xyz, -r8.xyzx, r4.wwww
    r9.xyz = ((-(r8.xyzx))+(r4.wwww)).xyz;
    // 142: mad r8.xyz, cb0[18].wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((source[18].wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 143: mul r9.xyz, r8.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r9.xyz = ((r8.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 144: mul r9.xyz, r3.wwww, r9.xyzx
    r9.xyz = ((r3.wwww)*(r9.xyzx)).xyz;
    // 145: mad r10.xyz, -r8.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r8.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 146: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul r11.xyz, r1.wwww, cb2[3].xyzx
    r11.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 148: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: mad r1.w, cb0[21].z, r1.w, r3.w
    r1.w = ((source[21].zzzz)*(r1.wwww)+(r3.wwww)).w;
    // 150: mad r9.xyz, r1.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r1.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 151: add r10.xyz, -r8.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r8.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 152: mov_sat r1.w, r7.z
    r1.w = (saturate(r7.zzzz)).w;
    // 153: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 154: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 155: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: mad r8.xyz, r1.wwww, r10.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r10.xyzx)+(r8.xyzx)).xyz;
    // 157: mad r8.xyz, r9.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r8.xyzx
    r8.xyz = ((r9.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r8.xyzx)).xyz;
    // 158: mul_sat r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = (saturate((r5.xyzx)*(r8.xyzx))).xyz;
    // 159: mul r5.xyz, r9.xyzx, r5.xyzx
    r5.xyz = ((r9.xyzx)*(r5.xyzx)).xyz;
    // 160: mad r2.xyz, r3.wwww, r2.xyzx, -r5.xyzx
    r2.xyz = ((r3.wwww)*(r2.xyzx)+(-(r5.xyzx))).xyz;
    // 161: mad r2.xyz, r2.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 162: max r1.w, r3.w, l(0.500000)
    r1.w = (max(r3.wwww,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 163: add r3.w, -cb0[20].x, l(0.200000)
    r3.w = ((-(source[20].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 164: mad r3.w, r2.w, r3.w, cb0[20].x
    r3.w = ((r2.wwww)*(r3.wwww)+(source[20].xxxx)).w;
    // 165: mad r3.w, r3.w, l(4.500000), l(0.500000)
    r3.w = ((r3.wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 166: mul r3.w, r3.w, cb0[21].w
    r3.w = ((r3.wwww)*(source[21].wwww)).w;
    // 167: mul r3.w, r3.w, l(0.050000)
    r3.w = ((r3.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 168: dp3 r4.w, v1.xyzx, v1.xyzx
    r4.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 169: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 170: mul r5.xyz, r4.wwww, v1.xyzx
    r5.xyz = ((r4.wwww)*(v1.xyzx)).xyz;
    // 171: dp3 r4.w, r5.xyzx, r3.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 172: dp3 r5.x, r5.xyzx, r7.xyzx
    r5.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 173: dp3 r5.y, v7.xyzx, v7.xyzx
    r5.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 174: rsq r5.y, r5.y
    r5.y = (rsqrt(r5.yyyy)).y;
    // 175: mul r5.yzw, r5.yyyy, v7.xxyz
    r5.yzw = ((r5.yyyy)*(v7.xxyz)).yzw;
    // 176: mul r7.xyz, r5.wwww, r6.xyzx
    r7.xyz = ((r5.wwww)*(r6.xyzx)).xyz;
    // 177: mad r6.xyz, r7.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r6.xyzx
    r6.xyz = ((r7.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r6.xyzx)).xyz;
    // 178: dp3 r6.x, r6.xyzx, r6.xyzx
    r6.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 179: sqrt r6.x, r6.x
    r6.x = (sqrt(r6.xxxx)).x;
    // 180: div r6.x, r6.z, r6.x
    r6.x = ((r6.zzzz)/(r6.xxxx)).x;
    // 181: add r6.x, r6.x, cb0[6].z
    r6.x = ((r6.xxxx)+(source[6].zzzz)).x;
    // 182: add r5.x, r5.x, -r6.x
    r5.x = ((r5.xxxx)+(-(r6.xxxx))).x;
    // 183: add r4.w, r4.w, r5.x
    r4.w = ((r4.wwww)+(r5.xxxx)).w;
    // 184: frc r4.w, r4.w
    r4.w = (frac(r4.wwww)).w;
    // 185: add r4.w, r4.w, l(-0.500000)
    r4.w = ((r4.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 186: add r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)+(r4.wwww)).w;
    // 187: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 188: log r5.x, r4.w
    r5.x = (log2(r4.wwww)).x;
    // 189: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 190: mul r3.w, r3.w, r5.x
    r3.w = ((r3.wwww)*(r5.xxxx)).w;
    // 191: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 192: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 193: movc r1.w, r4.w, l(0), r1.w
    r1.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 194: mad r6.xyz, v5.xyzx, r0.wwww, r5.yzwy
    r6.xyz = ((v5.xyzx)*(r0.wwww)+(r5.yzwy)).xyz;
    // 195: dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 196: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 197: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 198: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 199: mul r0.w, r0.w, cb0[22].x
    r0.w = ((r0.wwww)*(source[22].xxxx)).w;
    // 200: dp3 r1.w, r0.xyzx, r0.xyzx
    r1.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 201: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 202: div r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)/(r1.wwww)).xyz;
    // 203: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 204: mov_sat r1.w, r5.w
    r1.w = (saturate(r5.wwww)).w;
    // 205: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 206: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 207: mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 208: mul r0.xyz, r0.xyzx, cb0[19].xxxx
    r0.xyz = ((r0.xyzx)*(source[19].xxxx)).xyz;
    // 209: mad r1.xyz, cb0[19].yyyy, r1.xyzx, -r0.xyzx
    r1.xyz = ((source[19].yyyy)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 210: mad r0.xyz, r2.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 211: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 212: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 213: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 214: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 215: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 216: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 217: div r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 218: mul r1.xyz, r1.xyzx, cb0[14].xyzx
    r1.xyz = ((r1.xyzx)*(source[14].xyzx)).xyz;
    // 219: dp3 r0.w, r3.xyzx, r5.yzwy
    r0.w = (dot((r3.xyzx).xyz,(r5.yzwy).xyz).xxxx).w;
    // 220: add r1.w, -|r5.w|, l(1.000000)
    r1.w = ((-(abs(r5.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 221: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 222: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 223: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 224: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 225: mul r0.w, r0.w, cb0[21].y
    r0.w = ((r0.wwww)*(source[21].yyyy)).w;
    // 226: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 227: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 228: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 229: mad r0.xyz, r0.xyzx, cb2[3].wwww, r11.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r11.xyzx)).xyz;
    // 230: mul o0.xyz, r0.xyzx, cb0[23].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)).xyz;
    // 231: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 232: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 233: ret
    return output;
}

// source.character.equipment-native-168.v1 / source program e36b84e020d38e408af80c720418b668
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight168(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].w=(g_SourceCharacterTime.xxxx).x;
    source[23]=float4(input.lightColor,1.0);
    source[29].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[29].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[29].yyyy)) * 0xffffffffu)).w;
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
    // 11: mul r3.xyzw, r2.yyyy, cb0[25].xyzw
    r3.xyzw = ((r2.yyyy)*(source[25].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[24].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[24].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[26].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[26].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[27].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[27].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s4
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[28].wwzw
    r4.yz = (source[28].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s4
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s4
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[28].zwzz
    r4.xy = ((r2.xyxx)+(source[28].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s4
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[28].xyxx
    r2.xy = ((r2.xyxx)*(source[28].xyxx)).xy;
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
    // 33: mul r2.xyz, r0.wwww, cb0[29].xxxx
    r2.xyz = ((r0.wwww)*(source[29].xxxx)).xyz;
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
    // 44: mul r5.xyz, r5.xyzx, cb0[15].xxxx
    r5.xyz = ((r5.xyzx)*(source[15].xxxx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r1.w, cb0[15].y, l(-3.500000), l(5.000000)
    r1.w = ((source[15].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 47: mul r1.w, r1.w, cb0[16].x
    r1.w = ((r1.wwww)*(source[16].xxxx)).w;
    // 48: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: add r4.w, -r2.w, v4.z
    r4.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 50: mad r2.w, cb0[16].y, r4.w, r2.w
    r2.w = ((source[16].yyyy)*(r4.wwww)+(r2.wwww)).w;
    // 51: mul r4.w, r2.w, cb0[16].z
    r4.w = ((r2.wwww)*(source[16].zzzz)).w;
    // 52: mad r2.w, r4.w, l(0.750000), r2.w
    r2.w = ((r4.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 53: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 54: mad_sat r2.w, cb0[17].x, r2.w, r2.w
    r2.w = (saturate((source[17].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t1.xywz, s0, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 56: mad r8.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r4.w, r8.xyxx, r8.xyxx
    r4.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 58: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r4.w, r4.w, l(0.000000)
    r4.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 61: add r8.z, r4.w, l(0.000010)
    r8.z = ((r4.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mad r4.w, r8.x, r6.x, l(0.200000)
    r4.w = ((r8.xxxx)*(r6.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 63: add r5.w, -r6.y, l(1.000000)
    r5.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: add r5.w, -r4.w, r5.w
    r5.w = ((-(r4.wwww))+(r5.wwww)).w;
    // 65: mad r7.x, cb0[17].z, r5.w, r4.w
    r7.x = ((source[17].zzzz)*(r5.wwww)+(r4.wwww)).x;
    // 66: mul r7.y, cb0[16].w, l(0.700000)
    r7.y = ((source[16].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).y;
    // 67: add r7.x, -r2.w, r7.x
    r7.x = ((-(r2.wwww))+(r7.xxxx)).x;
    // 68: mad r7.x, r7.y, r7.x, r2.w
    r7.x = ((r7.yyyy)*(r7.xxxx)+(r2.wwww)).x;
    // 69: div r7.x, r7.x, cb0[17].y
    r7.x = ((r7.xxxx)/(source[17].yyyy)).x;
    // 70: add r7.x, -r7.x, l(1.000000)
    r7.x = ((-(r7.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 71: mul r7.x, r1.w, r7.x
    r7.x = ((r1.wwww)*(r7.xxxx)).x;
    // 72: mul r7.x, r7.x, l(4.000000)
    r7.x = ((r7.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 73: add r7.w, v4.w, l(0.500000)
    r7.w = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 74: round_ni r7.w, r7.w
    r7.w = (floor(r7.wwww)).w;
    // 75: mul_sat r7.x, r7.w, r7.x
    r7.x = (saturate((r7.wwww)*(r7.xxxx))).x;
    // 76: mad r4.w, cb0[18].x, r5.w, r4.w
    r4.w = ((source[18].xxxx)*(r5.wwww)+(r4.wwww)).w;
    // 77: add r4.w, -r2.w, r4.w
    r4.w = ((-(r2.wwww))+(r4.wwww)).w;
    // 78: mad r2.w, r7.y, r4.w, r2.w
    r2.w = ((r7.yyyy)*(r4.wwww)+(r2.wwww)).w;
    // 79: div r2.w, r2.w, cb0[17].w
    r2.w = ((r2.wwww)/(source[17].wwww)).w;
    // 80: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 82: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 83: add r2.w, -r7.w, l(1.000000)
    r2.w = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 85: add r1.w, r1.w, r7.x
    r1.w = ((r1.wwww)+(r7.xxxx)).w;
    // 86: add r1.w, -r6.y, r1.w
    r1.w = ((-(r6.yyyy))+(r1.wwww)).w;
    // 87: mad r1.w, cb0[18].y, r1.w, r6.y
    r1.w = ((source[18].yyyy)*(r1.wwww)+(r6.yyyy)).w;
    // 88: mad r5.xyz, r1.wwww, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 89: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: add r7.xyw, -r5.xyxz, r1.wwww
    r7.xyw = ((-(r5.xyxz))+(r1.wwww)).xyw;
    // 91: mad r5.xyz, cb0[18].zzzz, r7.xywx, r5.xyzx
    r5.xyz = ((source[18].zzzz)*(r7.xywx)+(r5.xyzx)).xyz;
    // 92: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r7.xyw, -r5.xyxz, r1.wwww
    r7.xyw = ((-(r5.xyxz))+(r1.wwww)).xyw;
    // 94: mad r5.xyz, cb0[18].wwww, r7.xywx, r5.xyzx
    r5.xyz = ((source[18].wwww)*(r7.xywx)+(r5.xyzx)).xyz;
    // 95: mad r7.xyw, cb0[4].wwww, cb0[4].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[4].wwww)*(source[4].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 96: mad r9.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 97: mul r7.xyw, r7.xyxw, r9.xyxz
    r7.xyw = ((r7.xyxw)*(r9.xyxz)).xyw;
    // 98: mul r9.xyz, r5.xyzx, r7.xywx
    r9.xyz = ((r5.xyzx)*(r7.xywx)).xyz;
    // 99: mul r10.xyz, r6.xxxx, r9.xyzx
    r10.xyz = ((r6.xxxx)*(r9.xyzx)).xyz;
    // 100: lt r1.w, |r7.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r7.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 101: mul r2.w, |r7.z|, |r7.z|
    r2.w = ((abs(r7.zzzz))*(abs(r7.zzzz))).w;
    // 102: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 103: mad r11.xyz, cb0[6].xyzx, r6.xyzx, -r6.xyzx
    r11.xyz = ((source[6].xyzx)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 104: mad r11.xyz, r1.wwww, r11.xyzx, r6.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)+(r6.xyzx)).xyz;
    // 105: mad r9.xyz, -r6.xxxx, r9.xyzx, r11.xyzx
    r9.xyz = ((-(r6.xxxx))*(r9.xyzx)+(r11.xyzx)).xyz;
    // 106: mad r9.xyz, r1.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 107: add r10.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = ((r9.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 108: dp3 r2.w, r10.xyzx, r10.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 109: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 110: div r10.xyz, r10.xyzx, r2.wwww
    r10.xyz = ((r10.xyzx)/(r2.wwww)).xyz;
    // 111: mul r10.xyz, r10.xyzx, cb0[9].xyzx
    r10.xyz = ((r10.xyzx)*(source[9].xyzx)).xyz;
    // 112: add r2.w, -|r3.z|, l(1.000000)
    r2.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: dp3 r4.w, r8.xyzx, r8.xyzx
    r4.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 114: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 115: div r8.xyz, r8.xyzx, r4.wwww
    r8.xyz = ((r8.xyzx)/(r4.wwww)).xyz;
    // 116: dp3 r4.w, r8.xyzx, r3.xyzx
    r4.w = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 117: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 119: lt r4.w, |r2.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 120: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 121: mul r2.w, r2.w, cb0[21].z
    r2.w = ((r2.wwww)*(source[21].zzzz)).w;
    // 122: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 123: movc r2.w, r4.w, l(0), r2.w
    r2.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 124: mul r4.w, cb0[10].z, l(1.500000)
    r4.w = ((source[10].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 125: add r5.w, -cb0[10].w, l(1.000000)
    r5.w = ((-(source[10].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r5.w, r5.w, cb0[21].w
    r5.w = ((r5.wwww)*(source[21].wwww)).w;
    // 127: mul r5.w, r5.w, l(6.283185)
    r5.w = ((r5.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 128: sincos r5.w, null, r5.w
    r5.w = (sin(r5.wwww)).w;
    // 129: add r5.w, r5.w, l(1.000000)
    r5.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 131: mad r4.w, r4.w, l(0.500000), cb0[10].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].zzzz)).w;
    // 132: frc r5.w, cb0[10].x
    r5.w = (frac(source[10].xxxx)).w;
    // 133: add r6.x, -r5.w, cb0[10].x
    r6.x = ((-(r5.wwww))+(source[10].xxxx)).x;
    // 134: mul r12.z, r6.x, l(0.125000)
    r12.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 135: mov r12.xw, l(0,0,0,0)
    r12.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 136: mul r12.y, cb0[10].y, cb0[11].y
    r12.y = ((source[10].yyyy)*(source[11].yyyy)).y;
    // 137: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 138: mul r6.x, r6.x, l(0.125000)
    r6.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 139: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 140: add r6.xy, r6.xyxx, r12.xyxx
    r6.xy = ((r6.xyxx)+(r12.xyxx)).xy;
    // 141: add r6.xy, r6.xyxx, r12.zwzz
    r6.xy = ((r6.xyxx)+(r12.zwzz)).xy;
    // 142: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 143: mul r12.xyz, r4.wwww, r12.xyzx
    r12.xyz = ((r4.wwww)*(r12.xyzx)).xyz;
    // 144: mul r4.w, r5.w, r12.w
    r4.w = ((r5.wwww)*(r12.wwww)).w;
    // 145: mad r12.xyz, r12.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r12.xyz = ((r12.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 146: mad r9.xyz, r4.wwww, r12.xyzx, r9.xyzx
    r9.xyz = ((r4.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 147: mul r4.w, cb0[12].y, cb0[21].w
    r4.w = ((source[12].yyyy)*(source[21].wwww)).w;
    // 148: mul r4.w, r4.w, l(0.628319)
    r4.w = ((r4.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 149: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 150: mul r6.y, r4.w, l(0.020000)
    r6.y = ((r4.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 151: add r12.xyzw, r1.yzxy, -cb0[1].yzxy
    r12.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 152: add r12.xy, -r12.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r12.xy = ((-(r12.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 153: add r12.xy, -r12.zwzz, r12.xyxx
    r12.xy = ((-(r12.zwzz))+(r12.xyxx)).xy;
    // 154: mad r12.xy, cb0[12].wwww, r12.xyxx, r12.zwzz
    r12.xy = ((source[12].wwww)*(r12.xyxx)+(r12.zwzz)).xy;
    // 155: mul r5.w, cb0[12].x, l(0.001000)
    r5.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 156: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 157: mad r6.xy, r5.wwww, r12.xyxx, r6.xyxx
    r6.xy = ((r5.wwww)*(r12.xyxx)+(r6.xyxx)).xy;
    // 158: dp2 r5.w, cb0[13].xyxx, r6.xyxx
    r5.w = (dot((source[13].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 159: dp2 r6.y, cb0[14].xyxx, r6.xyxx
    r6.y = (dot((source[14].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 160: frc r5.w, r5.w
    r5.w = (frac(r5.wwww)).w;
    // 161: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 162: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 163: mul r5.w, r12.w, l(0.900000)
    r5.w = ((r12.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 164: mad r12.xyz, r12.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r9.xyzx
    r12.xyz = ((r12.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r9.xyzx))).xyz;
    // 165: mad r12.xyz, r5.wwww, r12.xyzx, r9.xyzx
    r12.xyz = ((r5.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 166: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 168: mul_sat r12.xyz, r12.xyzx, r4.wwww
    r12.xyz = (saturate((r12.xyzx)*(r4.wwww))).xyz;
    // 169: mul r13.xyz, r12.xyzx, cb0[12].zzzz
    r13.xyz = ((r12.xyzx)*(source[12].zzzz)).xyz;
    // 170: dp3 r4.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 171: mul r4.w, r4.w, l(3.000000)
    r4.w = ((r4.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 172: mad r12.xyz, cb0[12].zzzz, r12.xyzx, -r9.xyzx
    r12.xyz = ((source[12].zzzz)*(r12.xyzx)+(-(r9.xyzx))).xyz;
    // 173: mad r9.xyz, r4.wwww, r12.xyzx, r9.xyzx
    r9.xyz = ((r4.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 174: dp3 r4.w, r8.xyzx, r4.xyzx
    r4.w = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 175: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 176: min r6.x, r5.w, l(1.000000)
    r6.x = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 177: mul r12.xyz, r2.xyzx, r6.xxxx
    r12.xyz = ((r2.xyzx)*(r6.xxxx)).xyz;
    // 178: mad r13.xyz, -r6.xxxx, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r6.xxxx))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mad r13.xyz, cb0[22].xxxx, r13.xyzx, r12.xyzx
    r13.xyz = ((source[22].xxxx)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 180: dp3 r6.x, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.x = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 181: add r14.xyz, r6.xxxx, -cb0[2].xyzx
    r14.xyz = ((r6.xxxx)+(-(source[2].xyzx))).xyz;
    // 182: mad r14.xyz, cb0[18].zzzz, r14.xyzx, cb0[2].xyzx
    r14.xyz = ((source[18].zzzz)*(r14.xyzx)+(source[2].xyzx)).xyz;
    // 183: dp3 r6.x, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.x = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 184: add r15.xyz, -r14.xyzx, r6.xxxx
    r15.xyz = ((-(r14.xyzx))+(r6.xxxx)).xyz;
    // 185: mad r14.xyz, cb0[18].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[18].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 186: mul r15.xyz, r14.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r15.xyz = ((r14.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 187: mad r16.xyz, -r14.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 190: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 191: mul r15.xyz, r15.xyzx, r4.wwww
    r15.xyz = ((r15.xyzx)*(r4.wwww)).xyz;
    // 192: mad r13.xyz, r13.xyzx, r16.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r16.xyzx)+(r15.xyzx)).xyz;
    // 193: mov_sat r4.w, r4.z
    r4.w = (saturate(r4.zzzz)).w;
    // 194: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 195: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 196: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 197: add r15.xyz, -r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mad r14.xyz, r4.wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((r4.wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 199: mad r14.xyz, r13.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r14.xyzx
    r14.xyz = ((r13.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r14.xyzx)).xyz;
    // 200: mul_sat r9.xyz, r9.xyzx, r14.xyzx
    r9.xyz = (saturate((r9.xyzx)*(r14.xyzx))).xyz;
    // 201: mul r9.xyz, r13.xyzx, r9.xyzx
    r9.xyz = ((r13.xyzx)*(r9.xyzx)).xyz;
    // 202: mad r12.yzw, r12.xxyz, r11.xxyz, -r9.xxyz
    r12.yzw = ((r12.xxyz)*(r11.xxyz)+(-(r9.xxyz))).yzw;
    // 203: mad r9.xyz, r1.wwww, r12.yzwy, r9.xyzx
    r9.xyz = ((r1.wwww)*(r12.yzwy)+(r9.xyzx)).xyz;
    // 204: max r2.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 205: min r2.xyz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 206: mad r5.xyz, r5.xyzx, r7.xywx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r5.xyz = ((r5.xyzx)*(r7.xywx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 207: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 208: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 209: div r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)/(r4.wwww)).xyz;
    // 210: mul r5.xyz, r5.xyzx, r6.zzzz
    r5.xyz = ((r5.xyzx)*(r6.zzzz)).xyz;
    // 211: mov_sat r4.w, r3.z
    r4.w = (saturate(r3.zzzz)).w;
    // 212: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 213: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 214: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 215: mul r5.xyz, r5.xyzx, cb0[19].xxxx
    r5.xyz = ((r5.xyzx)*(source[19].xxxx)).xyz;
    // 216: mad r6.xyz, cb0[19].yyyy, r11.xyzx, -r5.xyzx
    r6.xyz = ((source[19].yyyy)*(r11.xyzx)+(-(r5.xyzx))).xyz;
    // 217: mad r5.xyz, r1.wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 218: max r4.w, r12.x, l(0.500000)
    r4.w = (max(r12.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 219: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 220: dp3 r6.x, r0.xyzx, r8.xyzx
    r6.x = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 221: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 222: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 223: mul r3.w, r3.z, r1.z
    r3.w = ((r3.zzzz)*(r1.zzzz)).w;
    // 224: mad r1.xyz, r3.zzwz, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r3.zzwz)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 225: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 226: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 227: div r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)/(r0.yyyy)).y;
    // 228: add r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)+(source[7].zzzz)).y;
    // 229: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 230: add r0.x, r0.x, r6.x
    r0.x = ((r0.xxxx)+(r6.xxxx)).x;
    // 231: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 232: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 233: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 234: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 235: add r0.y, -cb0[20].x, l(0.200000)
    r0.y = ((-(source[20].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 236: mad r0.y, r1.w, r0.y, cb0[20].x
    r0.y = ((r1.wwww)*(r0.yyyy)+(source[20].xxxx)).y;
    // 237: mad r0.y, r0.y, l(4.500000), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 238: mul r0.y, r0.y, cb0[22].y
    r0.y = ((r0.yyyy)*(source[22].yyyy)).y;
    // 239: mul r0.y, r0.y, l(0.050000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 240: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 241: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 242: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 243: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 244: mul r0.x, r4.w, r0.x
    r0.x = ((r4.wwww)*(r0.xxxx)).x;
    // 245: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 246: mad r0.yzw, v5.xxyz, r0.wwww, r3.xxyz
    r0.yzw = ((v5.xxyz)*(r0.wwww)+(r3.xxyz)).yzw;
    // 247: dp3 r0.y, r0.yzwy, r0.yzwy
    r0.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 248: mad r0.y, r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 249: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 250: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 251: mul r0.x, r0.x, cb0[22].z
    r0.x = ((r0.xxxx)*(source[22].zzzz)).x;
    // 252: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 253: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 254: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 255: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 256: add r0.xyz, r0.xyzx, r9.xyzx
    r0.xyz = ((r0.xyzx)+(r9.xyzx)).xyz;
    // 257: mad r0.xyz, r2.wwww, r10.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 258: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 259: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 260: mul o0.xyz, r0.xyzx, cb0[23].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)).xyz;
    // 261: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t4.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 262: dp3 r0.x, r0.xyzx, cb0[8].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[8].xyzx).xyz).xxxx).x;
    // 263: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[8].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[8].xyzx).xyz).xxxx)).y;
    // 264: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 265: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 266: mul r0.x, r0.x, r6.w
    r0.x = ((r0.xxxx)*(r6.wwww)).x;
    // 267: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 268: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 269: ret
    return output;
}

// source.character.equipment-native-169.v1 / source program f50a76ea86607f4f8d19c4980059272a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight169(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].w=(g_SourceCharacterTime.xxxx).x;
    source[22]=float4(input.lightColor,1.0);
    source[28].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[28].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[28].yyyy)) * 0xffffffffu)).w;
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
    // 11: mul r3.xyzw, r2.yyyy, cb0[24].xyzw
    r3.xyzw = ((r2.yyyy)*(source[24].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[23].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[23].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[25].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[25].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[26].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[26].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s3
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[27].wwzw
    r4.yz = (source[27].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s3
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s3
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[27].zwzz
    r4.xy = ((r2.xyxx)+(source[27].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s3
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[27].xyxx
    r2.xy = ((r2.xyxx)*(source[27].xyxx)).xy;
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
    // 33: mul r2.xyz, r0.wwww, cb0[28].xxxx
    r2.xyz = ((r0.wwww)*(source[28].xxxx)).xyz;
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
    // 44: mul r5.xyz, r5.xyzx, cb0[14].xxxx
    r5.xyz = ((r5.xyzx)*(source[14].xxxx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r1.w, cb0[14].y, l(-3.500000), l(5.000000)
    r1.w = ((source[14].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 47: mul r1.w, r1.w, cb0[15].x
    r1.w = ((r1.wwww)*(source[15].xxxx)).w;
    // 48: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: add r3.w, -r2.w, v4.z
    r3.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 50: mad r2.w, cb0[15].y, r3.w, r2.w
    r2.w = ((source[15].yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 51: mul r3.w, r2.w, cb0[15].z
    r3.w = ((r2.wwww)*(source[15].zzzz)).w;
    // 52: mad r2.w, r3.w, l(0.750000), r2.w
    r2.w = ((r3.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 53: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 54: mad_sat r2.w, cb0[16].x, r2.w, r2.w
    r2.w = (saturate((source[16].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t1.xywz, s0, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 56: mad r8.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r3.w, r8.xyxx, r8.xyxx
    r3.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 58: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 61: add r8.z, r3.w, l(0.000010)
    r8.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mad r3.w, r8.x, r6.x, l(0.200000)
    r3.w = ((r8.xxxx)*(r6.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 63: add r4.w, -r6.y, l(1.000000)
    r4.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: add r4.w, -r3.w, r4.w
    r4.w = ((-(r3.wwww))+(r4.wwww)).w;
    // 65: mad r5.w, cb0[16].z, r4.w, r3.w
    r5.w = ((source[16].zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 66: mul r7.x, cb0[15].w, l(0.700000)
    r7.x = ((source[15].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 67: add r5.w, -r2.w, r5.w
    r5.w = ((-(r2.wwww))+(r5.wwww)).w;
    // 68: mad r5.w, r7.x, r5.w, r2.w
    r5.w = ((r7.xxxx)*(r5.wwww)+(r2.wwww)).w;
    // 69: div r5.w, r5.w, cb0[16].y
    r5.w = ((r5.wwww)/(source[16].yyyy)).w;
    // 70: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r5.w, r1.w, r5.w
    r5.w = ((r1.wwww)*(r5.wwww)).w;
    // 72: mul r5.w, r5.w, l(4.000000)
    r5.w = ((r5.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 73: add r7.y, v4.w, l(0.500000)
    r7.y = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 74: round_ni r7.y, r7.y
    r7.y = (floor(r7.yyyy)).y;
    // 75: mul_sat r5.w, r5.w, r7.y
    r5.w = (saturate((r5.wwww)*(r7.yyyy))).w;
    // 76: mad r3.w, cb0[17].x, r4.w, r3.w
    r3.w = ((source[17].xxxx)*(r4.wwww)+(r3.wwww)).w;
    // 77: add r3.w, -r2.w, r3.w
    r3.w = ((-(r2.wwww))+(r3.wwww)).w;
    // 78: mad r2.w, r7.x, r3.w, r2.w
    r2.w = ((r7.xxxx)*(r3.wwww)+(r2.wwww)).w;
    // 79: div r2.w, r2.w, cb0[16].w
    r2.w = ((r2.wwww)/(source[16].wwww)).w;
    // 80: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 82: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 83: add r2.w, -r7.y, l(1.000000)
    r2.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 85: add r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)+(r5.wwww)).w;
    // 86: add r1.w, -r6.y, r1.w
    r1.w = ((-(r6.yyyy))+(r1.wwww)).w;
    // 87: mad r1.w, cb0[17].y, r1.w, r6.y
    r1.w = ((source[17].yyyy)*(r1.wwww)+(r6.yyyy)).w;
    // 88: mad r5.xyz, r1.wwww, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 89: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: add r7.xyw, -r5.xyxz, r1.wwww
    r7.xyw = ((-(r5.xyxz))+(r1.wwww)).xyw;
    // 91: mad r5.xyz, cb0[17].zzzz, r7.xywx, r5.xyzx
    r5.xyz = ((source[17].zzzz)*(r7.xywx)+(r5.xyzx)).xyz;
    // 92: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r7.xyw, -r5.xyxz, r1.wwww
    r7.xyw = ((-(r5.xyxz))+(r1.wwww)).xyw;
    // 94: mad r5.xyz, cb0[17].wwww, r7.xywx, r5.xyzx
    r5.xyz = ((source[17].wwww)*(r7.xywx)+(r5.xyzx)).xyz;
    // 95: mad r7.xyw, cb0[4].wwww, cb0[4].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[4].wwww)*(source[4].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 96: mad r9.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 97: mul r7.xyw, r7.xyxw, r9.xyxz
    r7.xyw = ((r7.xyxw)*(r9.xyxz)).xyw;
    // 98: mul r9.xyz, r5.xyzx, r7.xywx
    r9.xyz = ((r5.xyzx)*(r7.xywx)).xyz;
    // 99: mul r10.xyz, r6.xxxx, r9.xyzx
    r10.xyz = ((r6.xxxx)*(r9.xyzx)).xyz;
    // 100: lt r1.w, |r7.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r7.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 101: mul r2.w, |r7.z|, |r7.z|
    r2.w = ((abs(r7.zzzz))*(abs(r7.zzzz))).w;
    // 102: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 103: mad r11.xyz, cb0[6].xyzx, r6.xyzx, -r6.xyzx
    r11.xyz = ((source[6].xyzx)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 104: mad r11.xyz, r1.wwww, r11.xyzx, r6.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)+(r6.xyzx)).xyz;
    // 105: mad r9.xyz, -r6.xxxx, r9.xyzx, r11.xyzx
    r9.xyz = ((-(r6.xxxx))*(r9.xyzx)+(r11.xyzx)).xyz;
    // 106: mad r9.xyz, r1.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 107: add r10.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = ((r9.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 108: dp3 r2.w, r10.xyzx, r10.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 109: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 110: div r10.xyz, r10.xyzx, r2.wwww
    r10.xyz = ((r10.xyzx)/(r2.wwww)).xyz;
    // 111: mul r10.xyz, r10.xyzx, cb0[8].xyzx
    r10.xyz = ((r10.xyzx)*(source[8].xyzx)).xyz;
    // 112: add r2.w, -|r3.z|, l(1.000000)
    r2.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: dp3 r3.w, r8.xyzx, r8.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 114: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 115: div r8.xyz, r8.xyzx, r3.wwww
    r8.xyz = ((r8.xyzx)/(r3.wwww)).xyz;
    // 116: dp3 r3.w, r8.xyzx, r3.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 117: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 119: lt r3.w, |r2.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 120: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 121: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 122: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 123: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 124: mul r3.w, cb0[9].z, l(1.500000)
    r3.w = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 125: add r4.w, -cb0[9].w, l(1.000000)
    r4.w = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r4.w, r4.w, cb0[20].w
    r4.w = ((r4.wwww)*(source[20].wwww)).w;
    // 127: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 128: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 129: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 131: mad r3.w, r3.w, l(0.500000), cb0[9].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).w;
    // 132: frc r4.w, cb0[9].x
    r4.w = (frac(source[9].xxxx)).w;
    // 133: add r5.w, -r4.w, cb0[9].x
    r5.w = ((-(r4.wwww))+(source[9].xxxx)).w;
    // 134: mul r12.z, r5.w, l(0.125000)
    r12.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 135: mov r12.xw, l(0,0,0,0)
    r12.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 136: mul r12.y, cb0[9].y, cb0[10].y
    r12.y = ((source[9].yyyy)*(source[10].yyyy)).y;
    // 137: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 138: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 139: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 140: add r6.xy, r6.xyxx, r12.xyxx
    r6.xy = ((r6.xyxx)+(r12.xyxx)).xy;
    // 141: add r6.xy, r6.xyxx, r12.zwzz
    r6.xy = ((r6.xyxx)+(r12.zwzz)).xy;
    // 142: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 143: mul r12.xyz, r3.wwww, r12.xyzx
    r12.xyz = ((r3.wwww)*(r12.xyzx)).xyz;
    // 144: mul r3.w, r4.w, r12.w
    r3.w = ((r4.wwww)*(r12.wwww)).w;
    // 145: mad r12.xyz, r12.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r12.xyz = ((r12.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 146: mad r9.xyz, r3.wwww, r12.xyzx, r9.xyzx
    r9.xyz = ((r3.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 147: mul r3.w, cb0[11].y, cb0[20].w
    r3.w = ((source[11].yyyy)*(source[20].wwww)).w;
    // 148: mul r3.w, r3.w, l(0.628319)
    r3.w = ((r3.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 149: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 150: mul r6.y, r3.w, l(0.020000)
    r6.y = ((r3.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 151: add r12.xyzw, r1.yzxy, -cb0[1].yzxy
    r12.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 152: add r12.xy, -r12.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r12.xy = ((-(r12.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 153: add r12.xy, -r12.zwzz, r12.xyxx
    r12.xy = ((-(r12.zwzz))+(r12.xyxx)).xy;
    // 154: mad r12.xy, cb0[11].wwww, r12.xyxx, r12.zwzz
    r12.xy = ((source[11].wwww)*(r12.xyxx)+(r12.zwzz)).xy;
    // 155: mul r4.w, cb0[11].x, l(0.001000)
    r4.w = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 156: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 157: mad r6.xy, r4.wwww, r12.xyxx, r6.xyxx
    r6.xy = ((r4.wwww)*(r12.xyxx)+(r6.xyxx)).xy;
    // 158: dp2 r4.w, cb0[12].xyxx, r6.xyxx
    r4.w = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 159: dp2 r6.y, cb0[13].xyxx, r6.xyxx
    r6.y = (dot((source[13].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 160: frc r4.w, r4.w
    r4.w = (frac(r4.wwww)).w;
    // 161: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 162: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 163: mul r4.w, r12.w, l(0.900000)
    r4.w = ((r12.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 164: mad r12.xyz, r12.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r9.xyzx
    r12.xyz = ((r12.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r9.xyzx))).xyz;
    // 165: mad r12.xyz, r4.wwww, r12.xyzx, r9.xyzx
    r12.xyz = ((r4.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 166: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 168: mul_sat r12.xyz, r12.xyzx, r3.wwww
    r12.xyz = (saturate((r12.xyzx)*(r3.wwww))).xyz;
    // 169: mul r13.xyz, r12.xyzx, cb0[11].zzzz
    r13.xyz = ((r12.xyzx)*(source[11].zzzz)).xyz;
    // 170: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 171: mul r3.w, r3.w, l(3.000000)
    r3.w = ((r3.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 172: mad r12.xyz, cb0[11].zzzz, r12.xyzx, -r9.xyzx
    r12.xyz = ((source[11].zzzz)*(r12.xyzx)+(-(r9.xyzx))).xyz;
    // 173: mad r9.xyz, r3.wwww, r12.xyzx, r9.xyzx
    r9.xyz = ((r3.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 174: dp3 r3.w, r8.xyzx, r4.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 175: max r4.w, r3.w, l(0.000000)
    r4.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 176: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r12.xyz, r2.xyzx, r5.wwww
    r12.xyz = ((r2.xyzx)*(r5.wwww)).xyz;
    // 178: mad r13.xyz, -r5.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r5.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mad r13.xyz, cb0[21].xxxx, r13.xyzx, r12.xyzx
    r13.xyz = ((source[21].xxxx)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 180: dp3 r5.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 181: add r14.xyz, r5.wwww, -cb0[2].xyzx
    r14.xyz = ((r5.wwww)+(-(source[2].xyzx))).xyz;
    // 182: mad r14.xyz, cb0[17].zzzz, r14.xyzx, cb0[2].xyzx
    r14.xyz = ((source[17].zzzz)*(r14.xyzx)+(source[2].xyzx)).xyz;
    // 183: dp3 r5.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 184: add r15.xyz, -r14.xyzx, r5.wwww
    r15.xyz = ((-(r14.xyzx))+(r5.wwww)).xyz;
    // 185: mad r14.xyz, cb0[17].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[17].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 186: mul r15.xyz, r14.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r15.xyz = ((r14.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 187: mad r16.xyz, -r14.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 190: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 191: mul r15.xyz, r15.xyzx, r3.wwww
    r15.xyz = ((r15.xyzx)*(r3.wwww)).xyz;
    // 192: mad r13.xyz, r13.xyzx, r16.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r16.xyzx)+(r15.xyzx)).xyz;
    // 193: mov_sat r3.w, r4.z
    r3.w = (saturate(r4.zzzz)).w;
    // 194: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 195: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 196: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 197: add r15.xyz, -r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mad r14.xyz, r3.wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((r3.wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 199: mad r14.xyz, r13.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r14.xyzx
    r14.xyz = ((r13.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r14.xyzx)).xyz;
    // 200: mul_sat r9.xyz, r9.xyzx, r14.xyzx
    r9.xyz = (saturate((r9.xyzx)*(r14.xyzx))).xyz;
    // 201: mul r9.xyz, r13.xyzx, r9.xyzx
    r9.xyz = ((r13.xyzx)*(r9.xyzx)).xyz;
    // 202: mad r12.yzw, r12.xxyz, r11.xxyz, -r9.xxyz
    r12.yzw = ((r12.xxyz)*(r11.xxyz)+(-(r9.xxyz))).yzw;
    // 203: mad r9.xyz, r1.wwww, r12.yzwy, r9.xyzx
    r9.xyz = ((r1.wwww)*(r12.yzwy)+(r9.xyzx)).xyz;
    // 204: max r2.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 205: min r2.xyz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 206: mad r5.xyz, r5.xyzx, r7.xywx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r5.xyz = ((r5.xyzx)*(r7.xywx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 207: dp3 r3.w, r5.xyzx, r5.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 208: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 209: div r5.xyz, r5.xyzx, r3.wwww
    r5.xyz = ((r5.xyzx)/(r3.wwww)).xyz;
    // 210: mul r5.xyz, r5.xyzx, r6.zzzz
    r5.xyz = ((r5.xyzx)*(r6.zzzz)).xyz;
    // 211: mov_sat r3.w, r3.z
    r3.w = (saturate(r3.zzzz)).w;
    // 212: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 213: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 214: mul r5.xyz, r5.xyzx, r3.wwww
    r5.xyz = ((r5.xyzx)*(r3.wwww)).xyz;
    // 215: mul r5.xyz, r5.xyzx, cb0[18].xxxx
    r5.xyz = ((r5.xyzx)*(source[18].xxxx)).xyz;
    // 216: mad r6.xyz, cb0[18].yyyy, r11.xyzx, -r5.xyzx
    r6.xyz = ((source[18].yyyy)*(r11.xyzx)+(-(r5.xyzx))).xyz;
    // 217: mad r5.xyz, r1.wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 218: max r3.w, r12.x, l(0.500000)
    r3.w = (max(r12.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 219: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 220: dp3 r5.w, r0.xyzx, r8.xyzx
    r5.w = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 221: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 222: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 223: mul r4.xyz, r3.zzzz, r1.xyzx
    r4.xyz = ((r3.zzzz)*(r1.xyzx)).xyz;
    // 224: mad r1.xyz, r4.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 225: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 226: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 227: div r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)/(r0.yyyy)).y;
    // 228: add r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)+(source[7].zzzz)).y;
    // 229: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 230: add r0.x, r0.x, r5.w
    r0.x = ((r0.xxxx)+(r5.wwww)).x;
    // 231: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 232: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 233: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 234: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 235: add r0.y, -cb0[19].x, l(0.200000)
    r0.y = ((-(source[19].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 236: mad r0.y, r1.w, r0.y, cb0[19].x
    r0.y = ((r1.wwww)*(r0.yyyy)+(source[19].xxxx)).y;
    // 237: mad r0.y, r0.y, l(4.500000), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 238: mul r0.y, r0.y, cb0[21].y
    r0.y = ((r0.yyyy)*(source[21].yyyy)).y;
    // 239: mul r0.y, r0.y, l(0.050000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 240: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 241: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 242: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 243: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 244: mul r0.x, r3.w, r0.x
    r0.x = ((r3.wwww)*(r0.xxxx)).x;
    // 245: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 246: mad r0.yzw, v5.xxyz, r0.wwww, r3.xxyz
    r0.yzw = ((v5.xxyz)*(r0.wwww)+(r3.xxyz)).yzw;
    // 247: dp3 r0.y, r0.yzwy, r0.yzwy
    r0.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 248: mad r0.y, r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 249: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 250: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 251: mul r0.x, r0.x, cb0[21].z
    r0.x = ((r0.xxxx)*(source[21].zzzz)).x;
    // 252: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 253: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 254: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 255: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 256: add r0.xyz, r0.xyzx, r9.xyzx
    r0.xyz = ((r0.xyzx)+(r9.xyzx)).xyz;
    // 257: mad r0.xyz, r2.wwww, r10.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 258: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 259: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 260: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 261: mul o0.w, r6.w, cb0[1].w
    output.targets[0].w = ((r6.wwww)*(source[1].wwww)).w;
    // 262: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 263: ret
    return output;
}

// source.character.equipment-native-170.v1 / source program 1f48cb9cdd6c794bb8d02df8fe2a929f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight170(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[18].z=(g_SourceCharacterTime.xxxx).x;
    source[18].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[19].x=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[19].y=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[19].z=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[21]=float4(input.lightColor,1.0);
    source[27].x=1.0;
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
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[27].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[27].yyyy)) * 0xffffffffu)).w;
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
    // 11: mul r3.xyzw, r2.yyyy, cb0[23].xyzw
    r3.xyzw = ((r2.yyyy)*(source[23].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[22].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[22].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[24].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[24].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[25].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[25].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s3
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[26].wwzw
    r4.yz = (source[26].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s3
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s3
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[26].zwzz
    r4.xy = ((r2.xyxx)+(source[26].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s3
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[26].xyxx
    r2.xy = ((r2.xyxx)*(source[26].xyxx)).xy;
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
    // 33: mul r2.xyz, r0.wwww, cb0[27].xxxx
    r2.xyz = ((r0.wwww)*(source[27].xxxx)).xyz;
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
    // 46: mad r1.w, cb0[13].y, l(-3.500000), l(5.000000)
    r1.w = ((source[13].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 47: mul r1.w, r1.w, cb0[14].x
    r1.w = ((r1.wwww)*(source[14].xxxx)).w;
    // 48: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: add r3.w, -r2.w, v4.z
    r3.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 50: mad r2.w, cb0[14].y, r3.w, r2.w
    r2.w = ((source[14].yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 51: mul r3.w, r2.w, cb0[14].z
    r3.w = ((r2.wwww)*(source[14].zzzz)).w;
    // 52: mad r2.w, r3.w, l(0.750000), r2.w
    r2.w = ((r3.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 53: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 54: mad_sat r2.w, cb0[15].x, r2.w, r2.w
    r2.w = (saturate((source[15].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t1.xyzw, s0, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 56: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 58: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 61: add r7.z, r3.w, l(0.000010)
    r7.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mad r3.w, r7.x, r6.x, l(0.200000)
    r3.w = ((r7.xxxx)*(r6.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 63: add r4.w, -r6.y, l(1.000000)
    r4.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: add r4.w, -r3.w, r4.w
    r4.w = ((-(r3.wwww))+(r4.wwww)).w;
    // 65: mad r5.w, cb0[15].z, r4.w, r3.w
    r5.w = ((source[15].zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 66: mul r7.w, cb0[14].w, l(0.700000)
    r7.w = ((source[14].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 67: add r5.w, -r2.w, r5.w
    r5.w = ((-(r2.wwww))+(r5.wwww)).w;
    // 68: mad r5.w, r7.w, r5.w, r2.w
    r5.w = ((r7.wwww)*(r5.wwww)+(r2.wwww)).w;
    // 69: div r5.w, r5.w, cb0[15].y
    r5.w = ((r5.wwww)/(source[15].yyyy)).w;
    // 70: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r5.w, r1.w, r5.w
    r5.w = ((r1.wwww)*(r5.wwww)).w;
    // 72: mul r5.w, r5.w, l(4.000000)
    r5.w = ((r5.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 73: add r8.x, v4.w, l(0.500000)
    r8.x = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 74: round_ni r8.x, r8.x
    r8.x = (floor(r8.xxxx)).x;
    // 75: mul_sat r5.w, r5.w, r8.x
    r5.w = (saturate((r5.wwww)*(r8.xxxx))).w;
    // 76: mad r3.w, cb0[16].x, r4.w, r3.w
    r3.w = ((source[16].xxxx)*(r4.wwww)+(r3.wwww)).w;
    // 77: add r3.w, -r2.w, r3.w
    r3.w = ((-(r2.wwww))+(r3.wwww)).w;
    // 78: mad r2.w, r7.w, r3.w, r2.w
    r2.w = ((r7.wwww)*(r3.wwww)+(r2.wwww)).w;
    // 79: div r2.w, r2.w, cb0[15].w
    r2.w = ((r2.wwww)/(source[15].wwww)).w;
    // 80: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 82: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 83: add r2.w, -r8.x, l(1.000000)
    r2.w = ((-(r8.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 85: add r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)+(r5.wwww)).w;
    // 86: add r1.w, -r6.y, r1.w
    r1.w = ((-(r6.yyyy))+(r1.wwww)).w;
    // 87: mad r1.w, cb0[16].y, r1.w, r6.y
    r1.w = ((source[16].yyyy)*(r1.wwww)+(r6.yyyy)).w;
    // 88: mad r5.xyz, r1.wwww, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 89: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: add r8.xyz, -r5.xyzx, r1.wwww
    r8.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 91: mad r5.xyz, cb0[16].zzzz, r8.xyzx, r5.xyzx
    r5.xyz = ((source[16].zzzz)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 92: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r8.xyz, -r5.xyzx, r1.wwww
    r8.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 94: mad r5.xyz, cb0[16].wwww, r8.xyzx, r5.xyzx
    r5.xyz = ((source[16].wwww)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 95: mad r8.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 96: mad r9.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 97: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 98: mul r9.xyz, r5.xyzx, r8.xyzx
    r9.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 99: mul r10.xyz, r6.xxxx, r9.xyzx
    r10.xyz = ((r6.xxxx)*(r9.xyzx)).xyz;
    // 100: mad r9.xyz, r6.xxxx, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = ((r6.xxxx)*(r9.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 101: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 102: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 103: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 104: mul r9.xyz, r9.xyzx, cb0[7].xyzx
    r9.xyz = ((r9.xyzx)*(source[7].xyzx)).xyz;
    // 105: add r1.w, -|r3.z|, l(1.000000)
    r1.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: dp3 r2.w, r7.xyzx, r7.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 107: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 108: div r7.xyz, r7.xyzx, r2.wwww
    r7.xyz = ((r7.xyzx)/(r2.wwww)).xyz;
    // 109: dp3 r2.w, r7.xyzx, r3.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 110: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 112: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 113: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 114: mul r1.w, r1.w, cb0[18].y
    r1.w = ((r1.wwww)*(source[18].yyyy)).w;
    // 115: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 116: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 117: mul r2.w, cb0[8].z, l(1.500000)
    r2.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 118: add r3.w, -cb0[8].w, l(1.000000)
    r3.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: mul r3.w, r3.w, cb0[18].z
    r3.w = ((r3.wwww)*(source[18].zzzz)).w;
    // 120: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 121: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 122: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 124: mad r2.w, r2.w, l(0.500000), cb0[8].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 125: frc r3.w, cb0[8].x
    r3.w = (frac(source[8].xxxx)).w;
    // 126: add r4.w, -r3.w, cb0[8].x
    r4.w = ((-(r3.wwww))+(source[8].xxxx)).w;
    // 127: mul r11.z, r4.w, l(0.125000)
    r11.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 128: mov r11.xw, l(0,0,0,0)
    r11.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 129: mul r11.y, cb0[8].y, cb0[9].y
    r11.y = ((source[8].yyyy)*(source[9].yyyy)).y;
    // 130: mul r12.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r12.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 131: frc r4.w, r12.x
    r4.w = (frac(r12.xxxx)).w;
    // 132: mul r12.y, r4.w, l(0.125000)
    r12.y = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 133: add r6.xy, r11.xyxx, r12.yzyy
    r6.xy = ((r11.xyxx)+(r12.yzyy)).xy;
    // 134: add r6.xy, r6.xyxx, r11.zwzz
    r6.xy = ((r6.xyxx)+(r11.zwzz)).xy;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 136: mul r11.xyz, r2.wwww, r11.xyzx
    r11.xyz = ((r2.wwww)*(r11.xyzx)).xyz;
    // 137: mul r2.w, r3.w, r11.w
    r2.w = ((r3.wwww)*(r11.wwww)).w;
    // 138: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 139: mad r10.xyz, r2.wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((r2.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 140: mul r2.w, cb0[10].y, cb0[18].z
    r2.w = ((source[10].yyyy)*(source[18].zzzz)).w;
    // 141: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 142: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 143: mul r6.y, r2.w, l(0.020000)
    r6.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 144: add r11.xyzw, r1.yzxy, -cb0[1].yzxy
    r11.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 145: add r11.xy, -r11.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r11.xy = ((-(r11.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 146: add r11.xy, -r11.zwzz, r11.xyxx
    r11.xy = ((-(r11.zwzz))+(r11.xyxx)).xy;
    // 147: mad r11.xy, cb0[10].wwww, r11.xyxx, r11.zwzz
    r11.xy = ((source[10].wwww)*(r11.xyxx)+(r11.zwzz)).xy;
    // 148: mul r3.w, cb0[10].x, l(0.001000)
    r3.w = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 149: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 150: mad r6.xy, r3.wwww, r11.xyxx, r6.xyxx
    r6.xy = ((r3.wwww)*(r11.xyxx)+(r6.xyxx)).xy;
    // 151: dp2 r3.w, cb0[11].xyxx, r6.xyxx
    r3.w = (dot((source[11].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 152: dp2 r6.y, cb0[12].xyxx, r6.xyxx
    r6.y = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 153: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 154: mul r6.x, r3.w, l(0.125000)
    r6.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 155: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 156: mul r3.w, r11.w, l(0.900000)
    r3.w = ((r11.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 157: mad r11.xyz, r11.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r10.xyzx
    r11.xyz = ((r11.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r10.xyzx))).xyz;
    // 158: mad r11.xyz, r3.wwww, r11.xyzx, r10.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 159: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 161: mul_sat r11.xyz, r11.xyzx, r2.wwww
    r11.xyz = (saturate((r11.xyzx)*(r2.wwww))).xyz;
    // 162: mul r12.xyz, r11.xyzx, cb0[10].zzzz
    r12.xyz = ((r11.xyzx)*(source[10].zzzz)).xyz;
    // 163: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 164: mul r2.w, r2.w, l(3.000000)
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 165: mad r11.xyz, cb0[10].zzzz, r11.xyzx, -r10.xyzx
    r11.xyz = ((source[10].zzzz)*(r11.xyzx)+(-(r10.xyzx))).xyz;
    // 166: mad r10.xyz, r2.wwww, r11.xyzx, r10.xyzx
    r10.xyz = ((r2.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 167: dp3 r2.w, r7.xyzx, r4.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 168: max r3.w, r2.w, l(0.000000)
    r3.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 169: min r4.w, r3.w, l(1.000000)
    r4.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: mul r11.xyz, r2.xyzx, r4.wwww
    r11.xyz = ((r2.xyzx)*(r4.wwww)).xyz;
    // 171: mad r12.xyz, -r4.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r4.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mad r11.yzw, cb0[19].wwww, r12.xxyz, r11.xxyz
    r11.yzw = ((source[19].wwww)*(r12.xxyz)+(r11.xxyz)).yzw;
    // 173: dp3 r4.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 174: add r12.xyz, r4.wwww, -cb0[2].xyzx
    r12.xyz = ((r4.wwww)+(-(source[2].xyzx))).xyz;
    // 175: mad r12.xyz, cb0[16].zzzz, r12.xyzx, cb0[2].xyzx
    r12.xyz = ((source[16].zzzz)*(r12.xyzx)+(source[2].xyzx)).xyz;
    // 176: dp3 r4.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 177: add r13.xyz, -r12.xyzx, r4.wwww
    r13.xyz = ((-(r12.xyzx))+(r4.wwww)).xyz;
    // 178: mad r12.xyz, cb0[16].wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((source[16].wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 179: mul r13.xyz, r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r13.xyz = ((r12.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 180: mad r14.xyz, -r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 182: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 183: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 184: mul r13.xyz, r13.xyzx, r2.wwww
    r13.xyz = ((r13.xyzx)*(r2.wwww)).xyz;
    // 185: mad r11.yzw, r11.yyzw, r14.xxyz, r13.xxyz
    r11.yzw = ((r11.yyzw)*(r14.xxyz)+(r13.xxyz)).yzw;
    // 186: mov_sat r2.w, r4.z
    r2.w = (saturate(r4.zzzz)).w;
    // 187: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 188: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 189: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: add r13.xyz, -r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r12.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 191: mad r12.xyz, r2.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r2.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 192: mad r12.xyz, r11.yzwy, l(0.500000, 0.500000, 0.500000, 0.000000), r12.xyzx
    r12.xyz = ((r11.yzwy)*(float4(0.500000,0.500000,0.500000,0.000000))+(r12.xyzx)).xyz;
    // 193: mul_sat r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = (saturate((r10.xyzx)*(r12.xyzx))).xyz;
    // 194: max r2.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 195: mad r5.xyz, r5.xyzx, r8.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r5.xyz = ((r5.xyzx)*(r8.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 196: dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 197: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 198: div r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)/(r2.wwww)).xyz;
    // 199: mul r5.xyz, r5.xyzx, r6.zzzz
    r5.xyz = ((r5.xyzx)*(r6.zzzz)).xyz;
    // 200: mov_sat r2.w, r3.z
    r2.w = (saturate(r3.zzzz)).w;
    // 201: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 202: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 203: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 204: mul r5.xyz, r5.xyzx, cb0[17].xxxx
    r5.xyz = ((r5.xyzx)*(source[17].xxxx)).xyz;
    // 205: max r2.w, r11.x, l(0.500000)
    r2.w = (max(r11.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 206: min r2.xyzw, r2.xyzw, l(1.000000, 1.000000, 1.000000, 1.000000)
    r2.xyzw = (min(r2.xyzw,float4(1.000000,1.000000,1.000000,1.000000))).xyzw;
    // 207: dp3 r4.w, r0.xyzx, r7.xyzx
    r4.w = (dot((r0.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 208: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 209: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 210: mul r4.xyz, r3.zzzz, r1.xyzx
    r4.xyz = ((r3.zzzz)*(r1.xyzx)).xyz;
    // 211: mad r1.xyz, r4.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 212: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 213: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 214: div r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)/(r0.yyyy)).y;
    // 215: add r0.y, r0.y, cb0[6].z
    r0.y = ((r0.yyyy)+(source[6].zzzz)).y;
    // 216: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 217: add r0.x, r0.x, r4.w
    r0.x = ((r0.xxxx)+(r4.wwww)).x;
    // 218: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 219: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 220: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 221: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 222: mad r0.y, -cb0[17].z, cb0[17].w, l(1.000000)
    r0.y = ((-(source[17].zzzz))*(source[17].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 223: mad r0.y, r0.y, l(4.500000), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 224: mul r0.y, r0.y, cb0[20].x
    r0.y = ((r0.yyyy)*(source[20].xxxx)).y;
    // 225: mul r0.y, r0.y, l(0.050000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 226: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 227: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 228: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 229: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 230: mul r0.x, r2.w, r0.x
    r0.x = ((r2.wwww)*(r0.xxxx)).x;
    // 231: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 232: mad r0.yzw, v5.xxyz, r0.wwww, r3.xxyz
    r0.yzw = ((v5.xxyz)*(r0.wwww)+(r3.xxyz)).yzw;
    // 233: dp3 r0.y, r0.yzwy, r0.yzwy
    r0.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 234: mad r0.y, r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 235: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 236: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 237: mul r0.x, r0.x, cb0[20].y
    r0.x = ((r0.xxxx)*(source[20].yyyy)).x;
    // 238: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 239: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 240: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 241: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 242: mad r0.xyz, r10.xyzx, r11.yzwy, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r11.yzwy)+(r0.xyzx)).xyz;
    // 243: mad r0.xyz, r1.wwww, r9.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r9.xyzx)+(r0.xyzx)).xyz;
    // 244: mul r0.xyz, r0.xyzx, cb0[20].zzzz
    r0.xyz = ((r0.xyzx)*(source[20].zzzz)).xyz;
    // 245: mul r1.xyz, r3.wwww, cb2[3].xyzx
    r1.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // 246: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 247: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 248: mul o0.w, r6.w, cb0[1].w
    output.targets[0].w = ((r6.wwww)*(source[1].wwww)).w;
    // 249: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 250: ret
    return output;
}

// source.character.equipment-native-171.v1 / source program 82d67a2701a2ff48bb06f7761f2caee9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight171(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].y=(g_SourceCharacterTime.xxxx).x;
    source[16].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[16].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[17].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[17].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[19]=float4(input.lightColor,1.0);
    source[25].x=1.0;
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
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[25].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[25].yyyy)) * 0xffffffffu)).w;
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
    // 11: mul r3.xyzw, r2.yyyy, cb0[21].xyzw
    r3.xyzw = ((r2.yyyy)*(source[21].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[20].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[20].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[22].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[22].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[23].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[23].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s4
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[24].wwzw
    r4.yz = (source[24].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s4
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s4
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[24].zwzz
    r4.xy = ((r2.xyxx)+(source[24].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s4
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[24].xyxx
    r2.xy = ((r2.xyxx)*(source[24].xyxx)).xy;
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
    // 33: mul r2.xyz, r0.wwww, cb0[25].xxxx
    r2.xyz = ((r0.wwww)*(source[25].xxxx)).xyz;
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
    // 44: mul r5.xyz, r5.xyzx, cb0[14].xxxx
    r5.xyz = ((r5.xyzx)*(source[14].xxxx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r5.xyz, r6.yyyy, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r6.yyyy)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 47: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 48: add r7.xyz, -r5.xyzx, r1.wwww
    r7.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 49: mad r5.xyz, cb0[14].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[14].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 50: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: add r7.xyz, -r5.xyzx, r1.wwww
    r7.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 52: mad r5.xyz, cb0[14].zzzz, r7.xyzx, r5.xyzx
    r5.xyz = ((source[14].zzzz)*(r7.xyzx)+(r5.xyzx)).xyz;
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
    // 62: mul r8.xyz, r8.xyzx, cb0[8].xyzx
    r8.xyz = ((r8.xyzx)*(source[8].xyzx)).xyz;
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
    // 79: mul r1.w, r1.w, cb0[16].x
    r1.w = ((r1.wwww)*(source[16].xxxx)).w;
    // 80: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 81: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 82: mul r2.w, cb0[9].z, l(1.500000)
    r2.w = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 83: add r3.w, -cb0[9].w, l(1.000000)
    r3.w = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul r3.w, r3.w, cb0[16].y
    r3.w = ((r3.wwww)*(source[16].yyyy)).w;
    // 85: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 86: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 87: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 89: mad r2.w, r2.w, l(0.500000), cb0[9].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).w;
    // 90: frc r3.w, cb0[9].x
    r3.w = (frac(source[9].xxxx)).w;
    // 91: add r4.w, -r3.w, cb0[9].x
    r4.w = ((-(r3.wwww))+(source[9].xxxx)).w;
    // 92: mul r11.z, r4.w, l(0.125000)
    r11.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 93: mov r11.xw, l(0,0,0,0)
    r11.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 94: mul r11.y, cb0[9].y, cb0[10].y
    r11.y = ((source[9].yyyy)*(source[10].yyyy)).y;
    // 95: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 96: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 97: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 98: add r6.xy, r6.xyxx, r11.xyxx
    r6.xy = ((r6.xyxx)+(r11.xyxx)).xy;
    // 99: add r6.xy, r6.xyxx, r11.zwzz
    r6.xy = ((r6.xyxx)+(r11.zwzz)).xy;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: mul r11.xyz, r2.wwww, r11.xyzx
    r11.xyz = ((r2.wwww)*(r11.xyzx)).xyz;
    // 102: mul r2.w, r3.w, r11.w
    r2.w = ((r3.wwww)*(r11.wwww)).w;
    // 103: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 104: mad r9.xyz, r2.wwww, r11.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 105: mul r2.w, cb0[11].y, cb0[16].y
    r2.w = ((source[11].yyyy)*(source[16].yyyy)).w;
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
    // 112: mad r11.xy, cb0[11].wwww, r11.xyxx, r11.zwzz
    r11.xy = ((source[11].wwww)*(r11.xyxx)+(r11.zwzz)).xy;
    // 113: mul r3.w, cb0[11].x, l(0.001000)
    r3.w = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 114: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 115: mad r6.xy, r3.wwww, r11.xyxx, r6.xyxx
    r6.xy = ((r3.wwww)*(r11.xyxx)+(r6.xyxx)).xy;
    // 116: dp2 r3.w, cb0[12].xyxx, r6.xyxx
    r3.w = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 117: dp2 r6.y, cb0[13].xyxx, r6.xyxx
    r6.y = (dot((source[13].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 118: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 119: mul r6.x, r3.w, l(0.125000)
    r6.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 120: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 121: mul r3.w, r11.w, l(0.900000)
    r3.w = ((r11.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 122: mad r11.xyz, r11.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r9.xyzx))).xyz;
    // 123: mad r11.xyz, r3.wwww, r11.xyzx, r9.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 124: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 126: mul_sat r11.xyz, r11.xyzx, r2.wwww
    r11.xyz = (saturate((r11.xyzx)*(r2.wwww))).xyz;
    // 127: mul r12.xyz, r11.xyzx, cb0[11].zzzz
    r12.xyz = ((r11.xyzx)*(source[11].zzzz)).xyz;
    // 128: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: mul r2.w, r2.w, l(3.000000)
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 130: mad r11.xyz, cb0[11].zzzz, r11.xyzx, -r9.xyzx
    r11.xyz = ((source[11].zzzz)*(r11.xyzx)+(-(r9.xyzx))).xyz;
    // 131: mad r9.xyz, r2.wwww, r11.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 132: dp3 r2.w, r10.xyzx, r4.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 133: max r3.w, r2.w, l(0.000000)
    r3.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r4.w, r3.w, l(1.000000)
    r4.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r11.xyz, r2.xyzx, r4.wwww
    r11.xyz = ((r2.xyzx)*(r4.wwww)).xyz;
    // 136: mad r12.xyz, -r4.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r4.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 137: mad r11.yzw, cb0[17].zzzz, r12.xxyz, r11.xxyz
    r11.yzw = ((source[17].zzzz)*(r12.xxyz)+(r11.xxyz)).yzw;
    // 138: dp3 r4.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 139: add r12.xyz, r4.wwww, -cb0[2].xyzx
    r12.xyz = ((r4.wwww)+(-(source[2].xyzx))).xyz;
    // 140: mad r12.xyz, cb0[14].yyyy, r12.xyzx, cb0[2].xyzx
    r12.xyz = ((source[14].yyyy)*(r12.xyzx)+(source[2].xyzx)).xyz;
    // 141: dp3 r4.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r13.xyz, -r12.xyzx, r4.wwww
    r13.xyz = ((-(r12.xyzx))+(r4.wwww)).xyz;
    // 143: mad r12.xyz, cb0[14].zzzz, r13.xyzx, r12.xyzx
    r12.xyz = ((source[14].zzzz)*(r13.xyzx)+(r12.xyzx)).xyz;
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
    // 169: mul r5.xyz, r5.xyzx, cb0[14].wwww
    r5.xyz = ((r5.xyzx)*(source[14].wwww)).xyz;
    // 170: max r2.w, r11.x, l(0.500000)
    r2.w = (max(r11.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 171: min r2.xyzw, r2.xyzw, l(1.000000, 1.000000, 1.000000, 1.000000)
    r2.xyzw = (min(r2.xyzw,float4(1.000000,1.000000,1.000000,1.000000))).xyzw;
    // 172: dp3 r4.w, r0.xyzx, r10.xyzx
    r4.w = (dot((r0.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 173: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 174: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 175: mul r4.xyz, r3.zzzz, r1.xyzx
    r4.xyz = ((r3.zzzz)*(r1.xyzx)).xyz;
    // 176: mad r1.xyz, r4.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
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
    // 182: add r0.x, r0.x, r4.w
    r0.x = ((r0.xxxx)+(r4.wwww)).x;
    // 183: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 184: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 185: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 186: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 187: mad r0.y, cb0[15].z, l(4.500000), l(0.500000)
    r0.y = ((source[15].zzzz)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 188: mul r0.y, r0.y, cb0[17].w
    r0.y = ((r0.yyyy)*(source[17].wwww)).y;
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
    // 201: mul r0.x, r0.x, cb0[18].x
    r0.x = ((r0.xxxx)*(source[18].xxxx)).x;
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
    // 208: mul r1.xyz, r3.wwww, cb2[3].xyzx
    r1.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // 209: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 210: mul o0.xyz, r0.xyzx, cb0[19].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[19].xyzx)).xyz;
    // 211: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t4.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 212: dp3 r0.x, r0.xyzx, cb0[7].xyzx
    r0.x = (dot((r0.xyzx).xyz,(source[7].xyzx).xyz).xxxx).x;
    // 213: dp3_sat r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), cb0[7].xyzx
    r0.y = (saturate(dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(source[7].xyzx).xyz).xxxx)).y;
    // 214: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 215: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 216: mul r0.x, r0.x, r6.w
    r0.x = ((r0.xxxx)*(r6.wwww)).x;
    // 217: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 218: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 219: ret
    return output;
}

// source.character.equipment-native-172.v1 / source program 82cd2855cd63de46ab29a2ec0c8c93c5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight172(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].w=(g_SourceCharacterTime.xxxx).x;
    source[22]=float4(input.lightColor,1.0);
    source[28].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[28].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[28].yyyy)) * 0xffffffffu)).w;
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
    // 11: mul r3.xyzw, r2.yyyy, cb0[24].xyzw
    r3.xyzw = ((r2.yyyy)*(source[24].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[23].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[23].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[25].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[25].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[26].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[26].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s3
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[27].wwzw
    r4.yz = (source[27].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s3
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s3
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[27].zwzz
    r4.xy = ((r2.xyxx)+(source[27].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s3
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[27].xyxx
    r2.xy = ((r2.xyxx)*(source[27].xyxx)).xy;
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
    // 33: mul r2.xyz, r0.wwww, cb0[28].xxxx
    r2.xyz = ((r0.wwww)*(source[28].xxxx)).xyz;
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
    // 44: mul r5.xyz, r5.xyzx, cb0[14].xxxx
    r5.xyz = ((r5.xyzx)*(source[14].xxxx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r1.w, cb0[14].y, l(-3.500000), l(5.000000)
    r1.w = ((source[14].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 47: mul r1.w, r1.w, cb0[15].x
    r1.w = ((r1.wwww)*(source[15].xxxx)).w;
    // 48: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: add r3.w, -r2.w, v4.z
    r3.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 50: mad r2.w, cb0[15].y, r3.w, r2.w
    r2.w = ((source[15].yyyy)*(r3.wwww)+(r2.wwww)).w;
    // 51: mul r3.w, r2.w, cb0[15].z
    r3.w = ((r2.wwww)*(source[15].zzzz)).w;
    // 52: mad r2.w, r3.w, l(0.750000), r2.w
    r2.w = ((r3.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 53: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 54: mad_sat r2.w, cb0[16].x, r2.w, r2.w
    r2.w = (saturate((source[16].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t1.xywz, s0, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 56: mad r8.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r3.w, r8.xyxx, r8.xyxx
    r3.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 58: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 61: add r8.z, r3.w, l(0.000010)
    r8.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mad r3.w, r8.x, r6.x, l(0.200000)
    r3.w = ((r8.xxxx)*(r6.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 63: add r4.w, -r6.y, l(1.000000)
    r4.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: add r4.w, -r3.w, r4.w
    r4.w = ((-(r3.wwww))+(r4.wwww)).w;
    // 65: mad r5.w, cb0[16].z, r4.w, r3.w
    r5.w = ((source[16].zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 66: mul r7.x, cb0[15].w, l(0.700000)
    r7.x = ((source[15].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 67: add r5.w, -r2.w, r5.w
    r5.w = ((-(r2.wwww))+(r5.wwww)).w;
    // 68: mad r5.w, r7.x, r5.w, r2.w
    r5.w = ((r7.xxxx)*(r5.wwww)+(r2.wwww)).w;
    // 69: div r5.w, r5.w, cb0[16].y
    r5.w = ((r5.wwww)/(source[16].yyyy)).w;
    // 70: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r5.w, r1.w, r5.w
    r5.w = ((r1.wwww)*(r5.wwww)).w;
    // 72: mul r5.w, r5.w, l(4.000000)
    r5.w = ((r5.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 73: add r7.y, v4.w, l(0.500000)
    r7.y = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 74: round_ni r7.y, r7.y
    r7.y = (floor(r7.yyyy)).y;
    // 75: mul_sat r5.w, r5.w, r7.y
    r5.w = (saturate((r5.wwww)*(r7.yyyy))).w;
    // 76: mad r3.w, cb0[17].x, r4.w, r3.w
    r3.w = ((source[17].xxxx)*(r4.wwww)+(r3.wwww)).w;
    // 77: add r3.w, -r2.w, r3.w
    r3.w = ((-(r2.wwww))+(r3.wwww)).w;
    // 78: mad r2.w, r7.x, r3.w, r2.w
    r2.w = ((r7.xxxx)*(r3.wwww)+(r2.wwww)).w;
    // 79: div r2.w, r2.w, cb0[16].w
    r2.w = ((r2.wwww)/(source[16].wwww)).w;
    // 80: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 82: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 83: add r2.w, -r7.y, l(1.000000)
    r2.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 85: add r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)+(r5.wwww)).w;
    // 86: add r1.w, -r6.y, r1.w
    r1.w = ((-(r6.yyyy))+(r1.wwww)).w;
    // 87: mad r1.w, cb0[17].y, r1.w, r6.y
    r1.w = ((source[17].yyyy)*(r1.wwww)+(r6.yyyy)).w;
    // 88: mad r5.xyz, r1.wwww, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 89: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: add r7.xyw, -r5.xyxz, r1.wwww
    r7.xyw = ((-(r5.xyxz))+(r1.wwww)).xyw;
    // 91: mad r5.xyz, cb0[17].zzzz, r7.xywx, r5.xyzx
    r5.xyz = ((source[17].zzzz)*(r7.xywx)+(r5.xyzx)).xyz;
    // 92: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r7.xyw, -r5.xyxz, r1.wwww
    r7.xyw = ((-(r5.xyxz))+(r1.wwww)).xyw;
    // 94: mad r5.xyz, cb0[17].wwww, r7.xywx, r5.xyzx
    r5.xyz = ((source[17].wwww)*(r7.xywx)+(r5.xyzx)).xyz;
    // 95: mad r7.xyw, cb0[4].wwww, cb0[4].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[4].wwww)*(source[4].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 96: mad r9.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 97: mul r7.xyw, r7.xyxw, r9.xyxz
    r7.xyw = ((r7.xyxw)*(r9.xyxz)).xyw;
    // 98: mul r9.xyz, r5.xyzx, r7.xywx
    r9.xyz = ((r5.xyzx)*(r7.xywx)).xyz;
    // 99: mul r10.xyz, r6.xxxx, r9.xyzx
    r10.xyz = ((r6.xxxx)*(r9.xyzx)).xyz;
    // 100: lt r1.w, |r7.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r7.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 101: mul r2.w, |r7.z|, |r7.z|
    r2.w = ((abs(r7.zzzz))*(abs(r7.zzzz))).w;
    // 102: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 103: mad r11.xyz, cb0[6].xyzx, r6.xyzx, -r6.xyzx
    r11.xyz = ((source[6].xyzx)*(r6.xyzx)+(-(r6.xyzx))).xyz;
    // 104: mad r11.xyz, r1.wwww, r11.xyzx, r6.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)+(r6.xyzx)).xyz;
    // 105: mad r9.xyz, -r6.xxxx, r9.xyzx, r11.xyzx
    r9.xyz = ((-(r6.xxxx))*(r9.xyzx)+(r11.xyzx)).xyz;
    // 106: mad r9.xyz, r1.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 107: add r10.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = ((r9.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 108: dp3 r2.w, r10.xyzx, r10.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 109: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 110: div r10.xyz, r10.xyzx, r2.wwww
    r10.xyz = ((r10.xyzx)/(r2.wwww)).xyz;
    // 111: mul r10.xyz, r10.xyzx, cb0[8].xyzx
    r10.xyz = ((r10.xyzx)*(source[8].xyzx)).xyz;
    // 112: add r2.w, -|r3.z|, l(1.000000)
    r2.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: dp3 r3.w, r8.xyzx, r8.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 114: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 115: div r8.xyz, r8.xyzx, r3.wwww
    r8.xyz = ((r8.xyzx)/(r3.wwww)).xyz;
    // 116: dp3 r3.w, r8.xyzx, r3.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 117: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 119: lt r3.w, |r2.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 120: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 121: mul r2.w, r2.w, cb0[20].z
    r2.w = ((r2.wwww)*(source[20].zzzz)).w;
    // 122: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 123: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 124: mul r3.w, cb0[9].z, l(1.500000)
    r3.w = ((source[9].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 125: add r4.w, -cb0[9].w, l(1.000000)
    r4.w = ((-(source[9].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r4.w, r4.w, cb0[20].w
    r4.w = ((r4.wwww)*(source[20].wwww)).w;
    // 127: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 128: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 129: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 131: mad r3.w, r3.w, l(0.500000), cb0[9].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).w;
    // 132: frc r4.w, cb0[9].x
    r4.w = (frac(source[9].xxxx)).w;
    // 133: add r5.w, -r4.w, cb0[9].x
    r5.w = ((-(r4.wwww))+(source[9].xxxx)).w;
    // 134: mul r12.z, r5.w, l(0.125000)
    r12.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 135: mov r12.xw, l(0,0,0,0)
    r12.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 136: mul r12.y, cb0[9].y, cb0[10].y
    r12.y = ((source[9].yyyy)*(source[10].yyyy)).y;
    // 137: mul r13.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r13.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 138: frc r5.w, r13.x
    r5.w = (frac(r13.xxxx)).w;
    // 139: mul r13.y, r5.w, l(0.125000)
    r13.y = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 140: add r6.xy, r12.xyxx, r13.yzyy
    r6.xy = ((r12.xyxx)+(r13.yzyy)).xy;
    // 141: add r6.xy, r6.xyxx, r12.zwzz
    r6.xy = ((r6.xyxx)+(r12.zwzz)).xy;
    // 142: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 143: mul r12.xyz, r3.wwww, r12.xyzx
    r12.xyz = ((r3.wwww)*(r12.xyzx)).xyz;
    // 144: mul r3.w, r4.w, r12.w
    r3.w = ((r4.wwww)*(r12.wwww)).w;
    // 145: mad r12.xyz, r12.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r12.xyz = ((r12.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 146: mad r9.xyz, r3.wwww, r12.xyzx, r9.xyzx
    r9.xyz = ((r3.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 147: mul r3.w, cb0[11].y, cb0[20].w
    r3.w = ((source[11].yyyy)*(source[20].wwww)).w;
    // 148: mul r3.w, r3.w, l(0.628319)
    r3.w = ((r3.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 149: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 150: mul r6.y, r3.w, l(0.020000)
    r6.y = ((r3.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 151: add r12.xyzw, r1.yzxy, -cb0[1].yzxy
    r12.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 152: add r12.xy, -r12.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r12.xy = ((-(r12.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 153: add r12.xy, -r12.zwzz, r12.xyxx
    r12.xy = ((-(r12.zwzz))+(r12.xyxx)).xy;
    // 154: mad r12.xy, cb0[11].wwww, r12.xyxx, r12.zwzz
    r12.xy = ((source[11].wwww)*(r12.xyxx)+(r12.zwzz)).xy;
    // 155: mul r4.w, cb0[11].x, l(0.001000)
    r4.w = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 156: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 157: mad r6.xy, r4.wwww, r12.xyxx, r6.xyxx
    r6.xy = ((r4.wwww)*(r12.xyxx)+(r6.xyxx)).xy;
    // 158: dp2 r4.w, cb0[12].xyxx, r6.xyxx
    r4.w = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 159: dp2 r6.y, cb0[13].xyxx, r6.xyxx
    r6.y = (dot((source[13].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 160: frc r4.w, r4.w
    r4.w = (frac(r4.wwww)).w;
    // 161: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 162: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 163: mul r4.w, r12.w, l(0.900000)
    r4.w = ((r12.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 164: mad r12.xyz, r12.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r9.xyzx
    r12.xyz = ((r12.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r9.xyzx))).xyz;
    // 165: mad r12.xyz, r4.wwww, r12.xyzx, r9.xyzx
    r12.xyz = ((r4.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 166: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 168: mul_sat r12.xyz, r12.xyzx, r3.wwww
    r12.xyz = (saturate((r12.xyzx)*(r3.wwww))).xyz;
    // 169: mul r13.xyz, r12.xyzx, cb0[11].zzzz
    r13.xyz = ((r12.xyzx)*(source[11].zzzz)).xyz;
    // 170: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 171: mul r3.w, r3.w, l(3.000000)
    r3.w = ((r3.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 172: mad r12.xyz, cb0[11].zzzz, r12.xyzx, -r9.xyzx
    r12.xyz = ((source[11].zzzz)*(r12.xyzx)+(-(r9.xyzx))).xyz;
    // 173: mad r9.xyz, r3.wwww, r12.xyzx, r9.xyzx
    r9.xyz = ((r3.wwww)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 174: dp3 r3.w, r8.xyzx, r4.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 175: max r4.w, r3.w, l(0.000000)
    r4.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 176: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r12.xyz, r2.xyzx, r5.wwww
    r12.xyz = ((r2.xyzx)*(r5.wwww)).xyz;
    // 178: mad r13.xyz, -r5.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r5.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mad r13.xyz, cb0[21].xxxx, r13.xyzx, r12.xyzx
    r13.xyz = ((source[21].xxxx)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 180: dp3 r5.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 181: add r14.xyz, r5.wwww, -cb0[2].xyzx
    r14.xyz = ((r5.wwww)+(-(source[2].xyzx))).xyz;
    // 182: mad r14.xyz, cb0[17].zzzz, r14.xyzx, cb0[2].xyzx
    r14.xyz = ((source[17].zzzz)*(r14.xyzx)+(source[2].xyzx)).xyz;
    // 183: dp3 r5.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 184: add r15.xyz, -r14.xyzx, r5.wwww
    r15.xyz = ((-(r14.xyzx))+(r5.wwww)).xyz;
    // 185: mad r14.xyz, cb0[17].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[17].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 186: mul r15.xyz, r14.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r15.xyz = ((r14.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 187: mad r16.xyz, -r14.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 190: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 191: mul r15.xyz, r15.xyzx, r3.wwww
    r15.xyz = ((r15.xyzx)*(r3.wwww)).xyz;
    // 192: mad r13.xyz, r13.xyzx, r16.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)*(r16.xyzx)+(r15.xyzx)).xyz;
    // 193: mov_sat r3.w, r4.z
    r3.w = (saturate(r4.zzzz)).w;
    // 194: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 195: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 196: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 197: add r15.xyz, -r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mad r14.xyz, r3.wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((r3.wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 199: mad r14.xyz, r13.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r14.xyzx
    r14.xyz = ((r13.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r14.xyzx)).xyz;
    // 200: mul_sat r9.xyz, r9.xyzx, r14.xyzx
    r9.xyz = (saturate((r9.xyzx)*(r14.xyzx))).xyz;
    // 201: mul r9.xyz, r13.xyzx, r9.xyzx
    r9.xyz = ((r13.xyzx)*(r9.xyzx)).xyz;
    // 202: mad r12.yzw, r12.xxyz, r11.xxyz, -r9.xxyz
    r12.yzw = ((r12.xxyz)*(r11.xxyz)+(-(r9.xxyz))).yzw;
    // 203: mad r9.xyz, r1.wwww, r12.yzwy, r9.xyzx
    r9.xyz = ((r1.wwww)*(r12.yzwy)+(r9.xyzx)).xyz;
    // 204: max r2.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 205: min r2.xyz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 206: mad r5.xyz, r5.xyzx, r7.xywx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r5.xyz = ((r5.xyzx)*(r7.xywx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 207: dp3 r3.w, r5.xyzx, r5.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 208: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 209: div r5.xyz, r5.xyzx, r3.wwww
    r5.xyz = ((r5.xyzx)/(r3.wwww)).xyz;
    // 210: mul r5.xyz, r5.xyzx, r6.zzzz
    r5.xyz = ((r5.xyzx)*(r6.zzzz)).xyz;
    // 211: mov_sat r3.w, r3.z
    r3.w = (saturate(r3.zzzz)).w;
    // 212: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 213: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 214: mul r5.xyz, r5.xyzx, r3.wwww
    r5.xyz = ((r5.xyzx)*(r3.wwww)).xyz;
    // 215: mul r5.xyz, r5.xyzx, cb0[18].xxxx
    r5.xyz = ((r5.xyzx)*(source[18].xxxx)).xyz;
    // 216: mad r6.xyz, cb0[18].yyyy, r11.xyzx, -r5.xyzx
    r6.xyz = ((source[18].yyyy)*(r11.xyzx)+(-(r5.xyzx))).xyz;
    // 217: mad r5.xyz, r1.wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 218: max r3.w, r12.x, l(0.500000)
    r3.w = (max(r12.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 219: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 220: dp3 r5.w, r0.xyzx, r8.xyzx
    r5.w = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 221: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 222: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 223: mul r4.xyz, r3.zzzz, r1.xyzx
    r4.xyz = ((r3.zzzz)*(r1.xyzx)).xyz;
    // 224: mad r1.xyz, r4.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 225: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 226: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 227: div r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)/(r0.yyyy)).y;
    // 228: add r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)+(source[7].zzzz)).y;
    // 229: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 230: add r0.x, r0.x, r5.w
    r0.x = ((r0.xxxx)+(r5.wwww)).x;
    // 231: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 232: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 233: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 234: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 235: add r0.y, -cb0[19].x, l(0.200000)
    r0.y = ((-(source[19].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 236: mad r0.y, r1.w, r0.y, cb0[19].x
    r0.y = ((r1.wwww)*(r0.yyyy)+(source[19].xxxx)).y;
    // 237: mad r0.y, r0.y, l(4.500000), l(0.500000)
    r0.y = ((r0.yyyy)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 238: mul r0.y, r0.y, cb0[21].y
    r0.y = ((r0.yyyy)*(source[21].yyyy)).y;
    // 239: mul r0.y, r0.y, l(0.050000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 240: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 241: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 242: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 243: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 244: mul r0.x, r3.w, r0.x
    r0.x = ((r3.wwww)*(r0.xxxx)).x;
    // 245: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 246: mad r0.yzw, v5.xxyz, r0.wwww, r3.xxyz
    r0.yzw = ((v5.xxyz)*(r0.wwww)+(r3.xxyz)).yzw;
    // 247: dp3 r0.y, r0.yzwy, r0.yzwy
    r0.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 248: mad r0.y, r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 249: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 250: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 251: mul r0.x, r0.x, cb0[21].z
    r0.x = ((r0.xxxx)*(source[21].zzzz)).x;
    // 252: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 253: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 254: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 255: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 256: add r0.xyz, r0.xyzx, r9.xyzx
    r0.xyz = ((r0.xyzx)+(r9.xyzx)).xyz;
    // 257: mad r0.xyz, r2.wwww, r10.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 258: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 259: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 260: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 261: mul o0.w, r6.w, cb0[1].w
    output.targets[0].w = ((r6.wwww)*(source[1].wwww)).w;
    // 262: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 263: ret
    return output;
}

// source.character.equipment-native-173.v1 / source program 0bd42d28d69ef24bba1d6deec5d42f7e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight173(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].x=(g_SourceCharacterTime.xxxx).x;
    source[22].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[22].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[23].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[23].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[26]=float4(input.lightColor,1.0);
    source[27].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[27].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[27].xxxx)) * 0xffffffffu)).w;
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
    // 76: add r1.x, -r0.z, cb0[17].w
    r1.x = ((-(r0.zzzz))+(source[17].wwww)).x;
    // 77: mad r0.z, r2.z, r1.x, r0.z
    r0.z = ((r2.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 79: add r1.x, -r9.w, l(1.000000)
    r1.x = ((-(r9.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 80: add r1.y, -cb0[18].y, cb0[18].x
    r1.y = ((-(source[18].yyyy))+(source[18].xxxx)).y;
    // 81: mad r1.y, r2.x, r1.y, cb0[18].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[18].yyyy)).y;
    // 82: add r1.w, -r1.y, cb0[18].z
    r1.w = ((-(r1.yyyy))+(source[18].zzzz)).w;
    // 83: mad r1.y, r2.y, r1.w, r1.y
    r1.y = ((r2.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 84: add r1.w, -r1.y, cb0[18].w
    r1.w = ((-(r1.yyyy))+(source[18].wwww)).w;
    // 85: mad r1.y, r2.z, r1.w, r1.y
    r1.y = ((r2.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 86: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 87: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 88: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 89: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 90: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 91: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 92: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 93: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 94: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 95: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 96: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 97: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 98: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 99: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 100: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 101: max r1.y, r1.y, r4.x
    r1.y = (max(r1.yyyy,r4.xxxx)).y;
    // 102: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 103: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 104: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 105: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 106: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 107: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 108: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 109: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 110: rcp r1.y, cb0[19].x
    r1.y = (1.0/(source[19].xxxx)).y;
    // 111: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 112: mul r11.xyz, r4.xyzx, cb0[19].xxxx
    r11.xyz = ((r4.xyzx)*(source[19].xxxx)).xyz;
    // 113: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 114: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 115: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 116: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 117: mad r4.xyz, r11.xyzx, cb0[19].xxxx, r4.xyzx
    r4.xyz = ((r11.xyzx)*(source[19].xxxx)+(r4.xyzx)).xyz;
    // 118: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 119: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 120: add r1.y, cb0[19].x, l(1.000000)
    r1.y = ((source[19].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 121: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 122: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 123: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 124: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 125: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 126: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 127: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 128: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 129: mul r4.w, |r1.y|, |r1.y|
    r4.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 130: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 131: mul r1.y, |r1.y|, r4.w
    r1.y = ((abs(r1.yyyy))*(r4.wwww)).y;
    // 132: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 133: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 134: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 135: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 136: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 137: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 138: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 140: mul r11.xyz, r0.xyzx, r1.yyyy
    r11.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 141: mul r12.xyz, r11.xyzx, cb0[23].wwww
    r12.xyz = ((r11.xyzx)*(source[23].wwww)).xyz;
    // 142: dp3 r4.w, r10.xyzx, r10.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 143: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 144: div r10.xyz, r10.xyzx, r4.wwww
    r10.xyz = ((r10.xyzx)/(r4.wwww)).xyz;
    // 145: dp3 r4.w, r10.xyzx, r7.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 146: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 147: min r6.w, r5.w, l(1.000000)
    r6.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: mad r4.w, r4.w, l(0.500000), -r6.w
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r6.wwww))).w;
    // 150: mad r4.w, r1.x, r4.w, r6.w
    r4.w = ((r1.xxxx)*(r4.wwww)+(r6.wwww)).w;
    // 151: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 153: mul_sat r6.xy, r6.xzxx, cb0[19].wwww
    r6.xy = (saturate((r6.xzxx)*(source[19].wwww))).xy;
    // 154: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 155: add_sat r6.y, r6.y, -cb0[20].x
    r6.y = (saturate((r6.yyyy)+(-(source[20].xxxx)))).y;
    // 156: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 157: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 158: mul r6.y, r6.y, cb0[20].y
    r6.y = ((r6.yyyy)*(source[20].yyyy)).y;
    // 159: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 160: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 161: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 162: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 163: mul r6.y, r4.w, r6.y
    r6.y = ((r4.wwww)*(r6.yyyy)).y;
    // 164: mad r6.z, r0.w, l(2.000000), -r1.y
    r6.z = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).z;
    // 165: mad r6.y, r6.y, r6.z, r1.y
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r1.yyyy)).y;
    // 166: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 167: mul_sat r6.y, r4.w, r6.y
    r6.y = (saturate((r4.wwww)*(r6.yyyy))).y;
    // 168: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 169: mul r1.x, r1.x, cb0[24].y
    r1.x = ((r1.xxxx)*(source[24].yyyy)).x;
    // 170: mad r6.y, cb0[24].x, r6.y, -r4.w
    r6.y = ((source[24].xxxx)*(r6.yyyy)+(-(r4.wwww))).y;
    // 171: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 172: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 173: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 174: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 175: mad r7.xyz, -cb0[23].wwww, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[23].wwww))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 176: mad r7.xyz, r5.xyzx, r7.xyzx, r12.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 177: mul r11.xyz, cb0[3].xyzx, cb0[3].wwww
    r11.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 178: mad r12.xyz, cb0[4].wwww, cb0[4].xyzx, -r11.xyzx
    r12.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r11.xyzx))).xyz;
    // 179: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 180: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 181: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 182: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 183: mad r11.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 184: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 185: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 186: mad r11.xyz, cb0[16].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[16].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 187: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 188: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 189: mad r11.xyz, cb0[16].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[16].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 190: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 191: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 193: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 194: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 195: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 196: mad r8.xyz, cb0[16].yyyy, r13.xyzx, r8.xyzx
    r8.xyz = ((source[16].yyyy)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 197: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 198: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 199: mad r8.xyz, cb0[16].zzzz, r13.xyzx, r8.xyzx
    r8.xyz = ((source[16].zzzz)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 200: mul r13.xyz, r8.xyzx, r11.xyzx
    r13.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 201: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 202: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 203: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 204: add r14.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r14.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 205: mad r14.xyz, r0.yyyy, r14.xyzx, cb0[9].xyzx
    r14.xyz = ((r0.yyyy)*(r14.xyzx)+(source[9].xyzx)).xyz;
    // 206: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 207: mul r0.xyz, r0.xyzx, cb0[19].yyyy
    r0.xyz = ((r0.xyzx)*(source[19].yyyy)).xyz;
    // 208: mul r14.xyz, r0.xyzx, r13.xyzx
    r14.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 209: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 210: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 211: mad r9.xyz, cb0[16].yyyy, r15.xyzx, r9.xyzx
    r9.xyz = ((source[16].yyyy)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 212: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 213: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 214: mad r9.xyz, cb0[16].zzzz, r15.xyzx, r9.xyzx
    r9.xyz = ((source[16].zzzz)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 215: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 216: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 217: mul r15.xyz, r15.xyzx, cb0[19].zzzz
    r15.xyz = ((r15.xyzx)*(source[19].zzzz)).xyz;
    // 218: add r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 219: add r1.x, r2.z, r1.x
    r1.x = ((r2.zzzz)+(r1.xxxx)).x;
    // 220: add_sat r1.x, r2.w, r1.x
    r1.x = (saturate((r2.wwww)+(r1.xxxx))).x;
    // 221: mad r2.xyz, r1.xxxx, r15.xyzx, r9.xyzx
    r2.xyz = ((r1.xxxx)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 222: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 223: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 224: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 225: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 226: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 227: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 228: mul r1.x, r1.x, cb0[20].z
    r1.x = ((r1.xxxx)*(source[20].zzzz)).x;
    // 229: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 230: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 231: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 232: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 233: div r1.z, cb0[20].w, r1.z
    r1.z = ((source[20].wwww)/(r1.zzzz)).z;
    // 234: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 235: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 236: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 237: mul r1.z, r1.z, cb0[21].x
    r1.z = ((r1.zzzz)*(source[21].xxxx)).z;
    // 238: mad r0.xyz, r2.xyzx, r0.xyzx, -r14.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r14.xyzx))).xyz;
    // 239: mad r0.xyz, r1.zzzz, r0.xyzx, r14.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r14.xyzx)).xyz;
    // 240: mad r0.xyz, r1.yyyy, r0.xyzx, -r13.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 241: mad r0.xyz, r1.xxxx, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 242: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 243: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 244: mad r0.xyz, cb0[16].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[16].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 245: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 246: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 247: mad r0.xyz, cb0[16].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[16].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 248: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 249: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 250: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 251: mul r2.w, r2.w, cb0[22].x
    r2.w = ((r2.wwww)*(source[22].xxxx)).w;
    // 252: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 253: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 254: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 255: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 256: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 257: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 258: add r4.w, -r2.w, cb0[2].x
    r4.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 259: mul r9.z, r4.w, l(0.125000)
    r9.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 260: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 261: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 262: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 263: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 264: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 265: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 266: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 267: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 268: mul r6.xyz, r1.zzzz, r9.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xyzx)).xyz;
    // 269: mul r1.z, r2.w, r9.w
    r1.z = ((r2.wwww)*(r9.wwww)).z;
    // 270: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 271: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 272: mul r1.z, cb0[12].y, cb0[22].x
    r1.z = ((source[12].yyyy)*(source[22].xxxx)).z;
    // 273: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 274: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 275: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 276: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 277: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 278: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 279: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 280: mul r2.w, cb0[12].x, l(0.001000)
    r2.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 281: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 282: mad r3.xy, r2.wwww, r3.xyxx, r6.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r6.xyxx)).xy;
    // 283: dp2 r2.w, cb0[13].xyxx, r3.xyxx
    r2.w = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 284: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 285: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 286: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 288: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 289: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 290: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 291: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 292: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 293: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 294: mul r6.xyz, r3.xyzx, cb0[12].zzzz
    r6.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 295: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 296: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 297: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 298: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 299: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 300: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 301: mad r5.xyz, r11.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r11.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 302: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 303: dp3 r4.x, r4.xyzx, r10.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 304: mul r4.y, r0.w, cb0[24].z
    r4.y = ((r0.wwww)*(source[24].zzzz)).y;
    // 305: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t7.xyzw, s8, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 306: add r0.w, -cb0[24].w, l(2.000000)
    r0.w = ((-(source[24].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 307: mad r0.w, r1.y, r0.w, cb0[24].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[24].wwww)).w;
    // 308: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 309: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 310: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 311: mul r1.xyz, r1.xyzx, cb0[25].xxxx
    r1.xyz = ((r1.xyzx)*(source[25].xxxx)).xyz;
    // 312: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 313: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 314: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 315: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 316: mul r1.xyz, r1.xyzx, cb0[25].yyyy
    r1.xyz = ((r1.xyzx)*(source[25].yyyy)).xyz;
    // 317: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 318: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 319: mad r0.xyz, r7.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 320: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 321: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 322: mul o0.xyz, r0.xyzx, cb0[26].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[26].xyzx)).xyz;
    // 323: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 324: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 325: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 326: ret
    return output;
}

// source.character.equipment-native-174.v1 / source program 57aebc2252228640b69f9f0ef560874a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight174(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[18].y=(g_SourceCharacterTime.xxxx).x;
    source[18].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[18].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[19].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[19].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[22]=float4(input.lightColor,1.0);
    source[23].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].w=1.f;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[23].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[23].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r8.xy, r8.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r8.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: mul r9.xy, r8.xyxx, cb0[13].xxxx
    r9.xy = ((r8.xyxx)*(source[13].xxxx)).xy;
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
    // 38: add r8.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 39: mad r8.xyz, cb0[13].wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((source[13].wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 40: dp3 r1.w, r8.xyzx, r8.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 41: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 42: div r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)/(r1.wwww)).xyz;
    // 43: dp3 r10.x, r1.xyzx, r8.xyzx
    r10.x = (dot((r1.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 44: dp3 r10.y, r2.xyzx, r8.xyzx
    r10.y = (dot((r2.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 45: dp3 r10.z, r0.xyzx, r8.xyzx
    r10.z = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 46: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 47: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 48: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 49: dp3 r0.x, r10.xyzx, r1.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 50: mul r0.xyz, r10.xyzx, r0.xxxx
    r0.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 51: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 52: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 53: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 54: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 55: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 56: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 57: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 59: add r0.z, -cb0[14].y, cb0[14].x
    r0.z = ((-(source[14].yyyy))+(source[14].xxxx)).z;
    // 60: mad r0.z, r2.x, r0.z, cb0[14].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[14].yyyy)).z;
    // 61: add r1.x, -r0.z, cb0[14].z
    r1.x = ((-(r0.zzzz))+(source[14].zzzz)).x;
    // 62: mad r0.z, r2.y, r1.x, r0.z
    r0.z = ((r2.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 63: add r1.x, -r0.z, cb0[14].w
    r1.x = ((-(r0.zzzz))+(source[14].wwww)).x;
    // 64: mad r0.z, r2.z, r1.x, r0.z
    r0.z = ((r2.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 66: add r1.x, -r8.w, l(1.000000)
    r1.x = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 67: add r1.y, -cb0[15].y, cb0[15].x
    r1.y = ((-(source[15].yyyy))+(source[15].xxxx)).y;
    // 68: mad r1.y, r2.x, r1.y, cb0[15].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[15].yyyy)).y;
    // 69: add r1.w, -r1.y, cb0[15].z
    r1.w = ((-(r1.yyyy))+(source[15].zzzz)).w;
    // 70: mad r1.y, r2.y, r1.w, r1.y
    r1.y = ((r2.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 71: add r1.w, -r1.y, cb0[15].w
    r1.w = ((-(r1.yyyy))+(source[15].wwww)).w;
    // 72: mad r1.y, r2.z, r1.w, r1.y
    r1.y = ((r2.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 73: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 74: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 75: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 76: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 77: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 78: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 79: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 80: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 81: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 82: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 83: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 84: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 85: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 86: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 87: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 88: max r1.y, r1.y, r4.x
    r1.y = (max(r1.yyyy,r4.xxxx)).y;
    // 89: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 90: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 91: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 92: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 93: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 94: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 95: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 97: rcp r1.y, cb0[16].x
    r1.y = (1.0/(source[16].xxxx)).y;
    // 98: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 99: mul r10.xyz, r4.xyzx, cb0[16].xxxx
    r10.xyz = ((r4.xyzx)*(source[16].xxxx)).xyz;
    // 100: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 101: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r10.xyzx, cb0[16].xxxx, r4.xyzx
    r4.xyz = ((r10.xyzx)*(source[16].xxxx)+(r4.xyzx)).xyz;
    // 105: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 106: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 107: add r1.y, cb0[16].x, l(1.000000)
    r1.y = ((source[16].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 108: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 109: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 110: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 111: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 112: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 113: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 114: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 115: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 116: mul r4.w, |r1.y|, |r1.y|
    r4.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 117: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 118: mul r1.y, |r1.y|, r4.w
    r1.y = ((abs(r1.yyyy))*(r4.wwww)).y;
    // 119: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 120: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 121: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 122: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 123: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 125: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 127: mul r10.xyz, r0.xyzx, r1.yyyy
    r10.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 128: mul r11.xyz, r10.xyzx, cb0[20].xxxx
    r11.xyz = ((r10.xyzx)*(source[20].xxxx)).xyz;
    // 129: dp3 r4.w, r9.xyzx, r9.xyzx
    r4.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 130: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 131: div r9.xyz, r9.xyzx, r4.wwww
    r9.xyz = ((r9.xyzx)/(r4.wwww)).xyz;
    // 132: dp3 r4.w, r9.xyzx, r7.xyzx
    r4.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 133: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r6.w, r5.w, l(1.000000)
    r6.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mad r4.w, r4.w, l(0.500000), -r6.w
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r6.wwww))).w;
    // 137: mad r4.w, r1.x, r4.w, r6.w
    r4.w = ((r1.xxxx)*(r4.wwww)+(r6.wwww)).w;
    // 138: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 140: mul_sat r6.xy, r6.xzxx, cb0[16].wwww
    r6.xy = (saturate((r6.xzxx)*(source[16].wwww))).xy;
    // 141: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 142: add_sat r6.y, r6.y, -cb0[17].x
    r6.y = (saturate((r6.yyyy)+(-(source[17].xxxx)))).y;
    // 143: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 144: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 145: mul r6.y, r6.y, cb0[17].y
    r6.y = ((r6.yyyy)*(source[17].yyyy)).y;
    // 146: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 147: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 148: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 149: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul r6.y, r4.w, r6.y
    r6.y = ((r4.wwww)*(r6.yyyy)).y;
    // 151: mad r6.z, r0.w, l(2.000000), -r1.y
    r6.z = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).z;
    // 152: mad r6.y, r6.y, r6.z, r1.y
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r1.yyyy)).y;
    // 153: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 154: mul_sat r6.y, r4.w, r6.y
    r6.y = (saturate((r4.wwww)*(r6.yyyy))).y;
    // 155: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 156: mul r1.x, r1.x, cb0[20].z
    r1.x = ((r1.xxxx)*(source[20].zzzz)).x;
    // 157: mad r6.y, cb0[20].y, r6.y, -r4.w
    r6.y = ((source[20].yyyy)*(r6.yyyy)+(-(r4.wwww))).y;
    // 158: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 159: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 160: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 161: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 162: mad r7.xyz, -cb0[20].xxxx, r10.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[20].xxxx))*(r10.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 163: mad r7.xyz, r5.xyzx, r7.xyzx, r11.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 164: mul r10.xyz, cb0[3].xyzx, cb0[3].wwww
    r10.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 165: dp3 r1.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 166: mad r11.xyz, -cb0[3].wwww, cb0[3].xyzx, r1.xxxx
    r11.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r1.xxxx)).xyz;
    // 167: mad r10.xyz, cb0[13].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[13].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 168: dp3 r1.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 169: add r11.xyz, -r10.xyzx, r1.xxxx
    r11.xyz = ((-(r10.xyzx))+(r1.xxxx)).xyz;
    // 170: mad r10.xyz, cb0[13].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[13].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 171: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 173: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 174: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 175: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 176: dp3 r1.x, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 177: add r13.xyz, -r12.xyzx, r1.xxxx
    r13.xyz = ((-(r12.xyzx))+(r1.xxxx)).xyz;
    // 178: mad r12.xyz, cb0[13].yyyy, r13.xyzx, r12.xyzx
    r12.xyz = ((source[13].yyyy)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 179: dp3 r1.x, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 180: add r13.xyz, -r12.xyzx, r1.xxxx
    r13.xyz = ((-(r12.xyzx))+(r1.xxxx)).xyz;
    // 181: mad r12.xyz, cb0[13].zzzz, r13.xyzx, r12.xyzx
    r12.xyz = ((source[13].zzzz)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 182: mul r13.xyz, r10.xyzx, r12.xyzx
    r13.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 183: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 184: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 185: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 186: add r14.xyz, -cb0[6].xyzx, cb0[7].xyzx
    r14.xyz = ((-(source[6].xyzx))+(source[7].xyzx)).xyz;
    // 187: mad r14.xyz, r0.yyyy, r14.xyzx, cb0[6].xyzx
    r14.xyz = ((r0.yyyy)*(r14.xyzx)+(source[6].xyzx)).xyz;
    // 188: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 189: mul r0.xyz, r0.xyzx, cb0[16].yyyy
    r0.xyz = ((r0.xyzx)*(source[16].yyyy)).xyz;
    // 190: mul r14.xyz, r0.xyzx, r13.xyzx
    r14.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 191: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 192: add r15.xyz, -r8.xyzx, r1.xxxx
    r15.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 193: mad r8.xyz, cb0[13].yyyy, r15.xyzx, r8.xyzx
    r8.xyz = ((source[13].yyyy)*(r15.xyzx)+(r8.xyzx)).xyz;
    // 194: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 195: add r15.xyz, -r8.xyzx, r1.xxxx
    r15.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 196: mad r8.xyz, cb0[13].zzzz, r15.xyzx, r8.xyzx
    r8.xyz = ((source[13].zzzz)*(r15.xyzx)+(r8.xyzx)).xyz;
    // 197: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 198: add r15.xyz, -r8.xyzx, r1.xxxx
    r15.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 199: mul r15.xyz, r15.xyzx, cb0[16].zzzz
    r15.xyz = ((r15.xyzx)*(source[16].zzzz)).xyz;
    // 200: add r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 201: add r1.x, r2.z, r1.x
    r1.x = ((r2.zzzz)+(r1.xxxx)).x;
    // 202: add_sat r1.x, r2.w, r1.x
    r1.x = (saturate((r2.wwww)+(r1.xxxx))).x;
    // 203: mad r2.xyz, r1.xxxx, r15.xyzx, r8.xyzx
    r2.xyz = ((r1.xxxx)*(r15.xyzx)+(r8.xyzx)).xyz;
    // 204: max r8.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r8.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 205: log r8.xyz, r8.xyzx
    r8.xyz = (log2(r8.xyzx)).xyz;
    // 206: mul r8.xyz, r8.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 207: exp r8.xyz, r8.xyzx
    r8.xyz = (exp2(r8.xyzx)).xyz;
    // 208: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 209: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 210: mul r1.x, r1.x, cb0[17].z
    r1.x = ((r1.xxxx)*(source[17].zzzz)).x;
    // 211: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 212: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 213: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 214: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 215: div r1.z, cb0[17].w, r1.z
    r1.z = ((source[17].wwww)/(r1.zzzz)).z;
    // 216: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 217: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 218: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 219: mul r1.z, r1.z, cb0[18].x
    r1.z = ((r1.zzzz)*(source[18].xxxx)).z;
    // 220: mad r0.xyz, r2.xyzx, r0.xyzx, -r14.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r14.xyzx))).xyz;
    // 221: mad r0.xyz, r1.zzzz, r0.xyzx, r14.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r14.xyzx)).xyz;
    // 222: mad r0.xyz, r1.yyyy, r0.xyzx, -r13.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 223: mad r0.xyz, r1.xxxx, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 224: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 225: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 226: mad r0.xyz, cb0[13].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[13].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 227: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 228: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 229: mad r0.xyz, cb0[13].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[13].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 230: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 231: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 232: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 233: mul r2.w, r2.w, cb0[18].y
    r2.w = ((r2.wwww)*(source[18].yyyy)).w;
    // 234: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 235: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 236: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 238: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 239: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 240: add r4.w, -r2.w, cb0[2].x
    r4.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 241: mul r8.z, r4.w, l(0.125000)
    r8.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 242: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 243: mul r8.y, cb0[2].y, cb0[8].y
    r8.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 244: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 245: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 246: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 247: add r6.xy, r6.xyxx, r8.xyxx
    r6.xy = ((r6.xyxx)+(r8.xyxx)).xy;
    // 248: add r6.xy, r6.xyxx, r8.zwzz
    r6.xy = ((r6.xyxx)+(r8.zwzz)).xy;
    // 249: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r6.xyxx, t5.xyzw, s6, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 250: mul r6.xyz, r1.zzzz, r8.xyzx
    r6.xyz = ((r1.zzzz)*(r8.xyzx)).xyz;
    // 251: mul r1.z, r2.w, r8.w
    r1.z = ((r2.wwww)*(r8.wwww)).z;
    // 252: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 253: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 254: mul r1.z, cb0[9].y, cb0[18].y
    r1.z = ((source[9].yyyy)*(source[18].yyyy)).z;
    // 255: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 256: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 257: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 258: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 259: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 260: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 261: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 262: mul r2.w, cb0[9].x, l(0.001000)
    r2.w = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 263: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 264: mad r3.xy, r2.wwww, r3.xyxx, r6.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r6.xyxx)).xy;
    // 265: dp2 r2.w, cb0[10].xyxx, r3.xyxx
    r2.w = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 266: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 267: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 268: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 269: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 270: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 271: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 272: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 273: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 274: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 275: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 276: mul r6.xyz, r3.xyzx, cb0[9].zzzz
    r6.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 277: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 278: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 279: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 280: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 281: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 282: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 283: mad r5.xyz, r10.xyzx, r12.xyzx, -r2.xyzx
    r5.xyz = ((r10.xyzx)*(r12.xyzx)+(-(r2.xyzx))).xyz;
    // 284: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 285: dp3 r4.x, r4.xyzx, r9.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 286: mul r4.y, r0.w, cb0[20].w
    r4.y = ((r0.wwww)*(source[20].wwww)).y;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s8, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 288: add r0.w, -cb0[21].x, l(2.000000)
    r0.w = ((-(source[21].xxxx))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 289: mad r0.w, r1.y, r0.w, cb0[21].x
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[21].xxxx)).w;
    // 290: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 291: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 292: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 293: mul r1.xyz, r1.xyzx, cb0[21].yyyy
    r1.xyz = ((r1.xyzx)*(source[21].yyyy)).xyz;
    // 294: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 295: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 296: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 297: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 298: mul r1.xyz, r1.xyzx, cb0[21].zzzz
    r1.xyz = ((r1.xyzx)*(source[21].zzzz)).xyz;
    // 299: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 300: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 301: mad r0.xyz, r7.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 302: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 303: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 304: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 305: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t7.xyzw, s7, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 306: mul r1.xyzw, r0.xyzw, cb0[12].xyzw
    r1.xyzw = ((r0.xyzw)*(source[12].xyzw)).xyzw;
    // 307: add r1.xy, r1.ywyy, r1.xzxx
    r1.xy = ((r1.ywyy)+(r1.xzxx)).xy;
    // 308: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 309: add r0.xy, r0.ywyy, r0.xzxx
    r0.xy = ((r0.ywyy)+(r0.xzxx)).xy;
    // 310: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 311: add r0.y, r1.x, l(-1.000000)
    r0.y = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 312: mad_sat r0.x, r0.x, r0.y, l(1.000000)
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 313: mul_sat r0.x, r12.w, r0.x
    r0.x = (saturate((r12.wwww)*(r0.xxxx))).x;
    // 314: mul_sat r0.x, r0.x, cb0[19].z
    r0.x = (saturate((r0.xxxx)*(source[19].zzzz))).x;
    // 315: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 316: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 317: ret
    return output;
}

// source.character.equipment-native-175.v1 / source program 299481140a68ab41a81ec66eb735e8a5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight175(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].z=(g_SourceCharacterTime.xxxx).x;
    source[29]=float4(input.lightColor,1.0);
    source[30].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[30].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[30].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.wxyz, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 36: mad r9.xyzw, r9.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r9.xyzw = ((r9.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 37: dp2 r1.w, r9.zwzz, r9.zwzz
    r1.w = (dot((r9.zwzz).xy,(r9.zwzz).xy).xxxx).w;
    // 38: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 41: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 42: mul r10.xy, r9.xyxx, cb0[17].xxxx
    r10.xy = ((r9.xyxx)*(source[17].xxxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mad r9.xy, cb0[17].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[17].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 45: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 46: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 47: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r12.xyz, cb0[19].wwww, r10.xyzx, r9.xyzx
    r12.xyz = ((source[19].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 49: dp3 r1.w, r12.xyzx, r12.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: div r12.xyz, r12.xyzx, r1.wwww
    r12.xyz = ((r12.xyzx)/(r1.wwww)).xyz;
    // 52: dp3 r13.x, r1.xyzx, r12.xyzx
    r13.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 53: dp3 r13.y, r2.xyzx, r12.xyzx
    r13.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 54: dp3 r13.z, r0.xyzx, r12.xyzx
    r13.z = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).z;
    // 55: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 56: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 57: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: dp3 r0.x, r13.xyzx, r1.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 59: mul r0.xyz, r13.xyzx, r0.xxxx
    r0.xyz = ((r13.xyzx)*(r0.xxxx)).xyz;
    // 60: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 61: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 62: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 65: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 66: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: add r0.z, -cb0[20].y, cb0[20].x
    r0.z = ((-(source[20].yyyy))+(source[20].xxxx)).z;
    // 68: mad r0.z, r11.x, r0.z, cb0[20].y
    r0.z = ((r11.xxxx)*(r0.zzzz)+(source[20].yyyy)).z;
    // 69: add r1.x, -r0.z, cb0[20].z
    r1.x = ((-(r0.zzzz))+(source[20].zzzz)).x;
    // 70: mad r0.z, r11.y, r1.x, r0.z
    r0.z = ((r11.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 71: add r1.x, -r0.z, cb0[20].w
    r1.x = ((-(r0.zzzz))+(source[20].wwww)).x;
    // 72: mad r0.z, r11.z, r1.x, r0.z
    r0.z = ((r11.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 74: add r1.x, -r2.w, l(1.000000)
    r1.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 75: add r1.y, -cb0[18].y, cb0[18].x
    r1.y = ((-(source[18].yyyy))+(source[18].xxxx)).y;
    // 76: mad r1.y, r11.x, r1.y, cb0[18].y
    r1.y = ((r11.xxxx)*(r1.yyyy)+(source[18].yyyy)).y;
    // 77: add r1.w, -r1.y, cb0[18].z
    r1.w = ((-(r1.yyyy))+(source[18].zzzz)).w;
    // 78: mad r1.y, r11.y, r1.w, r1.y
    r1.y = ((r11.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 79: add r1.w, -r1.y, cb0[18].w
    r1.w = ((-(r1.yyyy))+(source[18].wwww)).w;
    // 80: mad r1.y, r11.z, r1.w, r1.y
    r1.y = ((r11.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 81: add r1.w, -r1.y, cb0[19].x
    r1.w = ((-(r1.yyyy))+(source[19].xxxx)).w;
    // 82: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 83: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 84: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 85: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 86: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 87: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 88: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 89: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 90: add r1.w, -r1.y, cb0[21].x
    r1.w = ((-(r1.yyyy))+(source[21].xxxx)).w;
    // 91: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 92: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 93: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 94: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 95: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 96: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 97: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 98: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 99: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 100: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 101: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 102: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 103: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 104: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 105: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 106: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 107: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 108: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 109: rcp r1.y, cb0[21].y
    r1.y = (1.0/(source[21].yyyy)).y;
    // 110: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 111: mul r12.xyz, r4.xyzx, cb0[21].yyyy
    r12.xyz = ((r4.xyzx)*(source[21].yyyy)).xyz;
    // 112: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 113: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 114: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 115: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 116: mad r4.xyz, r12.xyzx, cb0[21].yyyy, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[21].yyyy)+(r4.xyzx)).xyz;
    // 117: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 119: add r1.y, cb0[21].y, l(1.000000)
    r1.y = ((source[21].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 120: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 121: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 122: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 123: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 124: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 125: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 126: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 127: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 128: mul r2.w, |r1.y|, |r1.y|
    r2.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 129: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 130: mul r1.y, |r1.y|, r2.w
    r1.y = ((abs(r1.yyyy))*(r2.wwww)).y;
    // 131: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 132: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 133: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 134: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 135: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 136: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 137: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 139: mul r12.xyz, r0.xyzx, r1.yyyy
    r12.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 140: mul r13.xyz, r12.xyzx, cb0[25].yyyy
    r13.xyz = ((r12.xyzx)*(source[25].yyyy)).xyz;
    // 141: mul r2.w, r11.w, l(0.500000)
    r2.w = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 142: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 144: mul r4.w, r1.x, r4.w
    r4.w = ((r1.xxxx)*(r4.wwww)).w;
    // 145: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 146: mad r9.xyz, r2.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 147: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 148: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 149: div r9.xyz, r9.xyzx, r2.wwww
    r9.xyz = ((r9.xyzx)/(r2.wwww)).xyz;
    // 150: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 151: max r4.w, r2.w, l(0.000000)
    r4.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 152: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: dp3 r6.w, cb0[16].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[16].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r7.xyz, r6.wwww, -cb0[16].xyzx
    r7.xyz = ((r6.wwww)+(-(source[16].xyzx))).xyz;
    // 156: mad r7.xyz, r5.wwww, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[16].xyzx)).xyz;
    // 157: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 158: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mul r7.xyz, r1.xxxx, r7.xyzx
    r7.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // 160: mad r2.w, r2.w, l(0.500000), -r5.w
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).w;
    // 161: mad r7.xyz, r7.xyzx, r2.wwww, r5.wwww
    r7.xyz = ((r7.xyzx)*(r2.wwww)+(r5.wwww)).xyz;
    // 162: add_sat r2.w, r11.w, cb0[25].z
    r2.w = (saturate((r11.wwww)+(source[25].zzzz))).w;
    // 163: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 165: mul_sat r6.xy, r6.xzxx, cb0[22].xxxx
    r6.xy = (saturate((r6.xzxx)*(source[22].xxxx))).xy;
    // 166: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 167: add_sat r6.y, r6.y, -cb0[22].y
    r6.y = (saturate((r6.yyyy)+(-(source[22].yyyy)))).y;
    // 168: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 169: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 170: mul r6.y, r6.y, cb0[22].z
    r6.y = ((r6.yyyy)*(source[22].zzzz)).y;
    // 171: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 172: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 173: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 174: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 175: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 176: mad r7.w, r0.w, l(2.000000), -r1.y
    r7.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).w;
    // 177: mad r6.yzw, r6.yyzw, r7.wwww, r1.yyyy
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r1.yyyy)).yzw;
    // 178: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 179: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 180: mul r7.w, r1.x, r1.x
    r7.w = ((r1.xxxx)*(r1.xxxx)).w;
    // 181: mul r8.x, r7.w, cb0[25].w
    r8.x = ((r7.wwww)*(source[25].wwww)).x;
    // 182: mad r1.x, -r7.w, cb0[25].w, r1.x
    r1.x = ((-(r7.wwww))*(source[25].wwww)+(r1.xxxx)).x;
    // 183: mad r1.x, r11.w, r1.x, r8.x
    r1.x = ((r11.wwww)*(r1.xxxx)+(r8.xxxx)).x;
    // 184: mad r6.yzw, r2.wwww, r6.yyzw, -r7.xxyz
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 185: mad r6.yzw, r1.xxxx, r6.yyzw, r7.xxyz
    r6.yzw = ((r1.xxxx)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 186: sqrt r1.x, r5.w
    r1.x = (sqrt(r5.wwww)).x;
    // 187: mul r5.xyz, r5.xyzx, r1.xxxx
    r5.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 188: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 189: mad r6.yzw, -cb0[25].yyyy, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[25].yyyy))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 190: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 191: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 192: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 193: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 194: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 195: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 196: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 197: mad r7.xyz, r11.zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 198: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 199: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 200: mad r7.xyz, cb0[19].yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((source[19].yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 201: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 202: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 203: mad r7.xyz, cb0[19].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[19].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 204: mad r10.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 205: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 206: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 207: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 208: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[9].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[9].xyzx)).xyz;
    // 209: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 210: dp3 r1.x, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 211: add r12.xyz, -r8.yzwy, r1.xxxx
    r12.xyz = ((-(r8.yzwy))+(r1.xxxx)).xyz;
    // 212: mad r8.xyz, cb0[19].yyyy, r12.xyzx, r8.yzwy
    r8.xyz = ((source[19].yyyy)*(r12.xyzx)+(r8.yzwy)).xyz;
    // 213: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 214: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 215: mad r8.xyz, cb0[19].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[19].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 216: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 217: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 218: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 219: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 220: add r13.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r13.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 221: mad r13.xyz, r0.yyyy, r13.xyzx, cb0[10].xyzx
    r13.xyz = ((r0.yyyy)*(r13.xyzx)+(source[10].xyzx)).xyz;
    // 222: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 223: mul r0.xyz, r0.xyzx, cb0[21].zzzz
    r0.xyz = ((r0.xyzx)*(source[21].zzzz)).xyz;
    // 224: mul r13.xyz, r0.xyzx, r12.xyzx
    r13.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 225: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: add r14.xyz, -r2.xyzx, r1.xxxx
    r14.xyz = ((-(r2.xyzx))+(r1.xxxx)).xyz;
    // 227: mad r2.yzw, cb0[19].yyyy, r14.xxyz, r2.xxyz
    r2.yzw = ((source[19].yyyy)*(r14.xxyz)+(r2.xxyz)).yzw;
    // 228: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 229: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 230: mad r2.yzw, cb0[19].zzzz, r14.xxyz, r2.yyzw
    r2.yzw = ((source[19].zzzz)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 231: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 232: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 233: mul r14.xyz, r14.xyzx, cb0[21].wwww
    r14.xyz = ((r14.xyzx)*(source[21].wwww)).xyz;
    // 234: add r1.x, r11.y, r11.x
    r1.x = ((r11.yyyy)+(r11.xxxx)).x;
    // 235: add r1.x, r11.z, r1.x
    r1.x = ((r11.zzzz)+(r1.xxxx)).x;
    // 236: add_sat r1.x, r11.w, r1.x
    r1.x = (saturate((r11.wwww)+(r1.xxxx))).x;
    // 237: mad r2.yzw, r1.xxxx, r14.xxyz, r2.yyzw
    r2.yzw = ((r1.xxxx)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 238: add r11.xyz, -r2.yzwy, r2.xxxx
    r11.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 239: mad r2.xyz, r11.wwww, r11.xyzx, r2.yzwy
    r2.xyz = ((r11.wwww)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 240: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 241: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 242: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 243: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 244: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 245: add r1.z, cb0[22].w, -cb0[23].x
    r1.z = ((source[22].wwww)+(-(source[23].xxxx))).z;
    // 246: mad r1.z, r11.w, r1.z, cb0[23].x
    r1.z = ((r11.wwww)*(r1.zzzz)+(source[23].xxxx)).z;
    // 247: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 248: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 249: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 250: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 251: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 252: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 253: div r1.z, cb0[23].y, r1.z
    r1.z = ((source[23].yyyy)/(r1.zzzz)).z;
    // 254: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 255: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 256: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 257: mul r1.z, r1.z, cb0[23].z
    r1.z = ((r1.zzzz)*(source[23].zzzz)).z;
    // 258: mad r0.xyz, r2.xyzx, r0.xyzx, -r13.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 259: mad r0.xyz, r1.zzzz, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 260: mad r0.xyz, r1.yyyy, r0.xyzx, -r12.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r12.xyzx))).xyz;
    // 261: mad r0.xyz, r1.xxxx, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 262: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 263: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 264: mad r0.xyz, cb0[19].yyyy, r11.xyzx, r0.xyzx
    r0.xyz = ((source[19].yyyy)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 265: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 266: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 267: mad r0.xyz, cb0[19].zzzz, r11.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 268: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 269: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 270: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 271: mul r2.w, r2.w, cb0[24].z
    r2.w = ((r2.wwww)*(source[24].zzzz)).w;
    // 272: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 273: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 274: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 275: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 276: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 277: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 278: add r6.x, -r2.w, cb0[2].x
    r6.x = ((-(r2.wwww))+(source[2].xxxx)).x;
    // 279: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 280: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 281: mul r10.y, cb0[2].y, cb0[12].y
    r10.y = ((source[2].yyyy)*(source[12].yyyy)).y;
    // 282: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 283: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 284: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 285: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 286: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t5.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 288: mul r10.xyz, r1.zzzz, r10.xyzx
    r10.xyz = ((r1.zzzz)*(r10.xyzx)).xyz;
    // 289: mul r1.z, r2.w, r10.w
    r1.z = ((r2.wwww)*(r10.wwww)).z;
    // 290: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 291: mad r0.xyz, r1.zzzz, r10.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 292: mul r1.z, cb0[13].y, cb0[24].z
    r1.z = ((source[13].yyyy)*(source[24].zzzz)).z;
    // 293: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 294: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 295: mul r10.y, r1.z, l(0.020000)
    r10.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 296: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 297: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 298: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 299: mad r3.xy, cb0[13].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[13].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 300: mul r2.w, cb0[13].x, l(0.001000)
    r2.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 301: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 302: mad r3.xy, r2.wwww, r3.xyxx, r10.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r10.xyxx)).xy;
    // 303: dp2 r2.w, cb0[14].xyxx, r3.xyxx
    r2.w = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 304: dp2 r3.y, cb0[15].xyxx, r3.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 305: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 306: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 307: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 308: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 309: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 310: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 311: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 312: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 313: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 314: mul r10.xyz, r3.xyzx, cb0[13].zzzz
    r10.xyz = ((r3.xyzx)*(source[13].zzzz)).xyz;
    // 315: dp3 r1.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 316: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 317: mad r3.xyz, cb0[13].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[13].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 318: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 319: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 320: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 321: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 322: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 323: dp3 r4.x, r4.xyzx, r9.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 324: mul r4.z, r0.w, cb0[26].x
    r4.z = ((r0.wwww)*(source[26].xxxx)).z;
    // 325: mul r0.w, r11.w, r4.z
    r0.w = ((r11.wwww)*(r4.zzzz)).w;
    // 326: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 327: min r0.w, r0.w, cb0[26].x
    r0.w = (min(r0.wwww,source[26].xxxx)).w;
    // 328: add r1.x, -cb0[26].w, cb0[26].z
    r1.x = ((-(source[26].wwww))+(source[26].zzzz)).x;
    // 329: mad r1.x, cb0[26].y, r1.x, cb0[26].w
    r1.x = ((source[26].yyyy)*(r1.xxxx)+(source[26].wwww)).x;
    // 330: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 331: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 332: mad r1.x, r11.w, r1.x, l(1.000000)
    r1.x = ((r11.wwww)*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 333: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 334: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 335: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 336: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 337: movc r4.y, r1.z, l(0), r0.w
    r4.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 338: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 339: add r0.w, -cb0[27].z, cb0[27].y
    r0.w = ((-(source[27].zzzz))+(source[27].yyyy)).w;
    // 340: mad r0.w, cb0[27].x, r0.w, cb0[27].z
    r0.w = ((source[27].xxxx)*(r0.wwww)+(source[27].zzzz)).w;
    // 341: mul r0.w, r0.w, r11.w
    r0.w = ((r0.wwww)*(r11.wwww)).w;
    // 342: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xzxx, t6.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 343: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 344: add r0.w, -cb0[27].w, l(2.000000)
    r0.w = ((-(source[27].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 345: mad r0.w, r1.y, r0.w, cb0[27].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[27].wwww)).w;
    // 346: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 347: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 348: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 349: mul r1.xyz, r1.xyzx, cb0[28].xxxx
    r1.xyz = ((r1.xyzx)*(source[28].xxxx)).xyz;
    // 350: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 351: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 352: mul r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)*(r5.wwww)).xyz;
    // 353: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 354: mul r1.xyz, r1.xyzx, cb0[28].yyyy
    r1.xyz = ((r1.xyzx)*(source[28].yyyy)).xyz;
    // 355: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 356: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 357: mad r0.xyz, r6.yzwy, r0.xyzx, r1.xyzx
    r0.xyz = ((r6.yzwy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 358: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 359: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 360: mul o0.xyz, r0.xyzx, cb0[29].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[29].xyzx)).xyz;
    // 361: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 362: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 363: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 364: ret
    return output;
}

// source.character.equipment-native-176.v1 / source program 77af4946ec583d4aa29c0068e443e48c
