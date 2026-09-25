SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight600(SOURCE_CHARACTER_NATIVE_INPUT input)
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

// source.character.selection-native-601.v1 / source program eed43b786b215f408ce12d58a3fe6e55
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight601(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[4]=float4(input.lightColor,1.0);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 4: movc r0.w, v7.x, l(1.000000), l(-1.000000)
    r0.w = ((asuint(v7.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 5: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 6: mul r1.xy, r0.zzzz, r0.wwww
    r1.xy = ((r0.zzzz)*(r0.wwww)).xy;
    // 7: mul r1.z, r0.w, r1.y
    r1.z = ((r0.wwww)*(r1.yyyy)).z;
    // 8: mad r0.xyz, r1.xyzx, l(0.000000, 0.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r1.xyzx)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 9: dp3 r1.x, v3.xyzx, v3.xyzx
    r1.x = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).x;
    // 10: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 11: mul r1.xyz, r1.xxxx, v3.xyzx
    r1.xyz = ((r1.xxxx)*(v3.xyzx)).xyz;
    // 12: dp3_sat r0.x, r0.xyzx, r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 13: mul_sat r0.y, r0.w, r1.z
    r0.y = (saturate((r0.wwww)*(r1.zzzz))).y;
    // 14: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 15: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.z, r0.z, l(15.000000)
    r0.z = ((r0.zzzz)*(float4(15.000000,15.000000,15.000000,15.000000))).z;
    // 17: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 18: mul r1.xyz, r0.zzzz, cb2[4].xyzx
    r1.xyz = ((r0.zzzz)*(passValues[4].xyzx)).xyz;
    // 19: movc r0.xzw, r0.xxxx, l(0,0,0,0), r1.xxyz
    r0.xzw = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxyz)).xzw;
    // 20: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 23: mul r1.w, r2.w, cb0[3].x
    r1.w = ((r2.wwww)*(source[3].xxxx)).w;
    // 24: mul o0.w, r1.w, cb0[0].y
    output.targets[0].w = ((r1.wwww)*(source[0].yyyy)).w;
    // 25: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 26: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 27: lt r1.w, r0.y, l(0.000001)
    r1.w = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 28: movc r0.y, r1.w, l(0), r0.y
    r0.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 29: mad r0.xyz, r1.xyzx, r0.yyyy, r0.xzwx
    r0.xyz = ((r1.xyzx)*(r0.yyyy)+(r0.xzwx)).xyz;
    // 30: mul o0.xyz, r0.xyzx, cb0[4].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[4].xyzx)).xyz;
    // 31: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 32: ret
    return output;
}

// source.character.selection-native-700.v1 / source program 5262375c2e4ee34d8b51f43c41173a05
