SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight25(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].y=(g_SourceCharacterTime.xxxx).x;
    source[20].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[23]=float4(input.lightColor,1.0);
    source[24].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[24].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[24].xxxx)) * 0xffffffffu)).w;
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
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 72: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 73: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 74: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 75: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 76: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 77: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 78: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 79: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 80: mul r1.x, r1.x, cb0[17].y
    r1.x = ((r1.xxxx)*(source[17].yyyy)).x;
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
    // 86: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 87: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 88: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 89: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 90: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 91: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 92: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 93: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 94: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 95: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t5.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 97: rcp r0.x, cb0[17].z
    r0.x = (1.0/(source[17].zzzz)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r9.xyz, r4.xyzx, cb0[17].zzzz
    r9.xyz = ((r4.xyzx)*(source[17].zzzz)).xyz;
    // 100: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r9.xyzx, cb0[17].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[17].zzzz)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[17].z, l(1.000000)
    r0.x = ((source[17].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 109: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 110: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 111: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 112: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 113: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 114: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 116: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 117: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 118: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 119: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 120: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 121: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 122: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 123: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 125: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 127: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 128: mul r11.xyz, r9.xyzx, cb0[21].yyyy
    r11.xyz = ((r9.xyzx)*(source[21].yyyy)).xyz;
    // 129: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 130: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 131: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 132: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 133: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 136: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 137: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 138: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 140: mul_sat r6.xy, r6.xzxx, cb0[18].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[18].yyyy))).xy;
    // 141: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 142: add_sat r6.y, r6.y, -cb0[18].z
    r6.y = (saturate((r6.yyyy)+(-(source[18].zzzz)))).y;
    // 143: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 144: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 145: mul r6.y, r6.y, cb0[18].w
    r6.y = ((r6.yyyy)*(source[18].wwww)).y;
    // 146: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 147: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 148: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 149: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 151: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 152: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 153: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 154: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 155: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 156: mul r0.z, r0.z, cb0[21].w
    r0.z = ((r0.zzzz)*(source[21].wwww)).z;
    // 157: mad r6.y, cb0[21].z, r6.y, -r4.z
    r6.y = ((source[21].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 158: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 159: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 160: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 161: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 162: mad r6.yzw, -cb0[21].yyyy, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[21].yyyy))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 163: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 164: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 165: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 166: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 167: mad r7.xyz, r9.xxxx, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.xxxx)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 168: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 169: mad r7.xyz, r9.yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 170: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 171: mad r7.xyz, r9.zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 172: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 173: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 174: mad r7.xyz, cb0[16].yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((source[16].yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 175: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 176: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 177: mad r7.xyz, cb0[16].zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((source[16].zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 178: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 181: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 182: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 184: mad r8.xyz, cb0[16].yyyy, r12.xyzx, r8.xyzx
    r8.xyz = ((source[16].yyyy)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 185: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 186: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 187: mad r8.xyz, cb0[16].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[16].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 188: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 189: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 190: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 191: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 192: add r1.yzw, -cb0[9].xxyz, cb0[10].xxyz
    r1.yzw = ((-(source[9].xxyz))+(source[10].xxyz)).yzw;
    // 193: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[9].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[9].xyzx)).xyz;
    // 194: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 195: mul r1.xyz, r1.xyzx, cb0[17].wwww
    r1.xyz = ((r1.xyzx)*(source[17].wwww)).xyz;
    // 196: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 197: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 198: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 199: mad r2.xyz, cb0[16].yyyy, r14.xyzx, r2.xyzx
    r2.xyz = ((source[16].yyyy)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 200: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 201: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 202: mad r2.xyz, cb0[16].zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((source[16].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 203: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 204: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 205: mul r14.xyz, r14.xyzx, cb0[18].xxxx
    r14.xyz = ((r14.xyzx)*(source[18].xxxx)).xyz;
    // 206: add r0.z, r9.y, r9.x
    r0.z = ((r9.yyyy)+(r9.xxxx)).z;
    // 207: add r0.z, r9.z, r0.z
    r0.z = ((r9.zzzz)+(r0.zzzz)).z;
    // 208: add_sat r0.z, r9.w, r0.z
    r0.z = (saturate((r9.wwww)+(r0.zzzz))).z;
    // 209: mad r2.xyz, r0.zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 210: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 211: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 212: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 213: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 214: dp3 r0.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 215: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 216: mul r0.z, r0.z, cb0[19].x
    r0.z = ((r0.zzzz)*(source[19].xxxx)).z;
    // 217: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 218: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 219: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 220: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 221: div r1.w, cb0[19].y, r1.w
    r1.w = ((source[19].yyyy)/(r1.wwww)).w;
    // 222: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 223: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 224: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 225: mul r1.w, r1.w, cb0[19].z
    r1.w = ((r1.wwww)*(source[19].zzzz)).w;
    // 226: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 227: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 228: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 229: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 230: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 231: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 232: mad r1.xyz, cb0[16].yyyy, r9.xyzx, r1.xyzx
    r1.xyz = ((source[16].yyyy)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 233: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 234: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 235: mad r1.xyz, cb0[16].zzzz, r9.xyzx, r1.xyzx
    r1.xyz = ((source[16].zzzz)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 236: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 237: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 238: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 239: mul r4.z, r4.z, cb0[20].y
    r4.z = ((r4.zzzz)*(source[20].yyyy)).z;
    // 240: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 241: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 242: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 243: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 244: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 245: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 246: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 247: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 248: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 249: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 250: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 251: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 252: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 253: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 254: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 255: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 256: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 257: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 258: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 259: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 260: mul r1.w, cb0[12].y, cb0[20].y
    r1.w = ((source[12].yyyy)*(source[20].yyyy)).w;
    // 261: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 262: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 263: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 264: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 265: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 266: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 267: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 268: mul r3.z, cb0[12].x, l(0.001000)
    r3.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 269: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 270: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 271: dp2 r3.z, cb0[13].xyxx, r3.xyxx
    r3.z = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 272: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 273: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 274: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 275: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 276: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 277: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 278: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 279: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 280: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 281: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 282: mul r9.xyz, r3.xyzx, cb0[12].zzzz
    r9.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 283: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 284: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 285: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 286: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 287: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 288: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 289: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 290: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 291: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 292: mul r0.y, r2.w, cb0[22].x
    r0.y = ((r2.wwww)*(source[22].xxxx)).y;
    // 293: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 294: add r0.w, -cb0[22].y, l(2.000000)
    r0.w = ((-(source[22].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 295: mad r0.w, r4.x, r0.w, cb0[22].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[22].yyyy)).w;
    // 296: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 297: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 298: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 299: mul r0.xyz, r0.xyzx, cb0[22].zzzz
    r0.xyz = ((r0.xyzx)*(source[22].zzzz)).xyz;
    // 300: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 301: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 302: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 303: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 304: mul r0.xyz, r0.xyzx, cb0[22].wwww
    r0.xyz = ((r0.xyzx)*(source[22].wwww)).xyz;
    // 305: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 306: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 307: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 308: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
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

// source.character.monster-6ff78ae19259.v1 / source program acc28065f1b00341a5874feb06c9082f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight26(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].y=(g_SourceCharacterTime.xxxx).x;
    source[16].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[19]=float4(input.lightColor,1.0);
    source[20].x=1.0;
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

// source.character.monster-4184f1980abb.v1 / source program 58f86aa5c4c55a479d0c6894d2eceb52
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight27(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].z=(g_SourceCharacterTime.xxxx).x;
    source[21].x=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[24]=float4(input.lightColor,1.0);
    source[25].x=1.0;
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
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 72: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 73: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 74: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 75: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 76: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 77: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 78: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 79: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 80: mul r1.x, r1.x, cb0[17].y
    r1.x = ((r1.xxxx)*(source[17].yyyy)).x;
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
    // 86: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 87: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 88: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 89: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 90: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 91: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 92: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 93: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 94: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 95: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t5.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 97: rcp r0.x, cb0[17].z
    r0.x = (1.0/(source[17].zzzz)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r9.xyz, r4.xyzx, cb0[17].zzzz
    r9.xyz = ((r4.xyzx)*(source[17].zzzz)).xyz;
    // 100: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r9.xyzx, cb0[17].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[17].zzzz)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[17].z, l(1.000000)
    r0.x = ((source[17].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 109: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 110: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 111: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 112: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 113: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 114: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 116: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 117: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 118: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 119: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 120: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 121: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 122: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 123: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 125: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 127: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 128: mul r11.xyz, r9.xyzx, cb0[21].zzzz
    r11.xyz = ((r9.xyzx)*(source[21].zzzz)).xyz;
    // 129: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 130: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 131: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 132: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 133: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 136: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 137: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 138: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 140: mul_sat r6.xy, r6.xzxx, cb0[18].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[18].yyyy))).xy;
    // 141: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 142: add_sat r6.y, r6.y, -cb0[18].z
    r6.y = (saturate((r6.yyyy)+(-(source[18].zzzz)))).y;
    // 143: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 144: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 145: mul r6.y, r6.y, cb0[18].w
    r6.y = ((r6.yyyy)*(source[18].wwww)).y;
    // 146: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 147: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 148: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 149: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 151: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 152: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 153: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 154: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 155: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 156: mul r0.z, r0.z, cb0[22].x
    r0.z = ((r0.zzzz)*(source[22].xxxx)).z;
    // 157: mad r6.y, cb0[21].w, r6.y, -r4.z
    r6.y = ((source[21].wwww)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 158: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 159: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 160: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 161: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 162: mad r6.yzw, -cb0[21].zzzz, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[21].zzzz))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 163: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 164: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 165: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 166: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 167: mad r7.xyz, r9.xxxx, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.xxxx)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 168: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 169: mad r7.xyz, r9.yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 170: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 171: mad r7.xyz, r9.zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 172: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 173: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 174: mad r7.xyz, cb0[16].yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((source[16].yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 175: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 176: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 177: mad r7.xyz, cb0[16].zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((source[16].zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 178: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 181: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 182: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 184: mad r8.xyz, cb0[16].yyyy, r12.xyzx, r8.xyzx
    r8.xyz = ((source[16].yyyy)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 185: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 186: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 187: mad r8.xyz, cb0[16].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[16].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 188: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 189: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 190: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 191: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 192: add r1.yzw, -cb0[9].xxyz, cb0[10].xxyz
    r1.yzw = ((-(source[9].xxyz))+(source[10].xxyz)).yzw;
    // 193: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[9].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[9].xyzx)).xyz;
    // 194: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 195: mul r1.xyz, r1.xyzx, cb0[17].wwww
    r1.xyz = ((r1.xyzx)*(source[17].wwww)).xyz;
    // 196: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 197: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 198: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 199: mad r2.xyz, cb0[16].yyyy, r14.xyzx, r2.xyzx
    r2.xyz = ((source[16].yyyy)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 200: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 201: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 202: mad r2.xyz, cb0[16].zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((source[16].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 203: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 204: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 205: mul r14.xyz, r14.xyzx, cb0[18].xxxx
    r14.xyz = ((r14.xyzx)*(source[18].xxxx)).xyz;
    // 206: add r0.z, r9.y, r9.x
    r0.z = ((r9.yyyy)+(r9.xxxx)).z;
    // 207: add r0.z, r9.z, r0.z
    r0.z = ((r9.zzzz)+(r0.zzzz)).z;
    // 208: add_sat r0.z, r9.w, r0.z
    r0.z = (saturate((r9.wwww)+(r0.zzzz))).z;
    // 209: mad r2.xyz, r0.zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 210: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 211: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 212: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 213: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 214: dp3 r0.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 215: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 216: mul r0.z, r0.z, cb0[19].x
    r0.z = ((r0.zzzz)*(source[19].xxxx)).z;
    // 217: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 218: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 219: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 220: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 221: div r1.w, cb0[19].y, r1.w
    r1.w = ((source[19].yyyy)/(r1.wwww)).w;
    // 222: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 223: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 224: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 225: mul r1.w, r1.w, cb0[19].z
    r1.w = ((r1.wwww)*(source[19].zzzz)).w;
    // 226: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 227: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 228: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 229: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 230: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 231: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 232: mad r1.xyz, cb0[16].yyyy, r9.xyzx, r1.xyzx
    r1.xyz = ((source[16].yyyy)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 233: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 234: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 235: mad r1.xyz, cb0[16].zzzz, r9.xyzx, r1.xyzx
    r1.xyz = ((source[16].zzzz)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 236: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 237: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 238: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 239: mul r4.z, r4.z, cb0[20].z
    r4.z = ((r4.zzzz)*(source[20].zzzz)).z;
    // 240: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 241: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 242: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 243: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 244: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 245: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 246: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 247: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 248: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 249: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 250: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 251: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 252: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 253: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 254: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 255: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 256: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 257: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 258: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 259: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 260: mul r1.w, cb0[12].y, cb0[20].z
    r1.w = ((source[12].yyyy)*(source[20].zzzz)).w;
    // 261: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 262: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 263: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 264: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 265: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 266: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 267: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 268: mul r3.z, cb0[12].x, l(0.001000)
    r3.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 269: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 270: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 271: dp2 r3.z, cb0[13].xyxx, r3.xyxx
    r3.z = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 272: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 273: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 274: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 275: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 276: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 277: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 278: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 279: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 280: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 281: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 282: mul r9.xyz, r3.xyzx, cb0[12].zzzz
    r9.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 283: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 284: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 285: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 286: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 287: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 288: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 289: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 290: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 291: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 292: mul r0.y, r2.w, cb0[22].y
    r0.y = ((r2.wwww)*(source[22].yyyy)).y;
    // 293: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 294: add r0.w, -cb0[22].z, l(2.000000)
    r0.w = ((-(source[22].zzzz))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 295: mad r0.w, r4.x, r0.w, cb0[22].z
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[22].zzzz)).w;
    // 296: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 297: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 298: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 299: mul r0.xyz, r0.xyzx, cb0[22].wwww
    r0.xyz = ((r0.xyzx)*(source[22].wwww)).xyz;
    // 300: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 301: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 302: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 303: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 304: mul r0.xyz, r0.xyzx, cb0[23].xxxx
    r0.xyz = ((r0.xyzx)*(source[23].xxxx)).xyz;
    // 305: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 306: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 307: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 308: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 309: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 310: mul o0.xyz, r0.xyzx, cb0[24].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[24].xyzx)).xyz;
    // 311: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 312: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 313: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 314: ret
    return output;
}

// source.character.monster-ce6425dcbdeb.v1 / source program 4e0184af3910df4aa76329673c2ede10
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight28(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[17].z=(g_SourceCharacterTime.xxxx).x;
    source[20]=float4(input.lightColor,1.0);
    source[21].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).w;
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
    // 31: mul r9.xyzw, r8.xyzw, cb0[12].xyzw
    r9.xyzw = ((r8.xyzw)*(source[12].xyzw)).xyzw;
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
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 45: mul r10.xy, r9.xyxx, cb0[13].xxxx
    r10.xy = ((r9.xyxx)*(source[13].xxxx)).xy;
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
    // 52: mad r9.xyz, cb0[13].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[13].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 72: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 73: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 74: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 75: mul r0.z, r0.z, cb0[14].x
    r0.z = ((r0.zzzz)*(source[14].xxxx)).z;
    // 76: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 77: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 78: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 79: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 80: mul r1.x, r1.x, cb0[14].y
    r1.x = ((r1.xxxx)*(source[14].yyyy)).x;
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
    // 86: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 87: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 88: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 89: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 90: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 91: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 92: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 93: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 94: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 95: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s4, r1.x
    r1.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 97: rcp r0.x, cb0[14].z
    r0.x = (1.0/(source[14].zzzz)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r9.xyz, r4.xyzx, cb0[14].zzzz
    r9.xyz = ((r4.xyzx)*(source[14].zzzz)).xyz;
    // 100: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r9.xyzx, cb0[14].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[14].zzzz)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[14].z, l(1.000000)
    r0.x = ((source[14].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 109: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 110: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 111: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 112: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 113: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 114: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 116: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 117: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 118: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 119: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 120: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 121: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 122: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 123: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 125: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 127: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 128: mul r11.xyz, r9.xyzx, cb0[18].yyyy
    r11.xyz = ((r9.xyzx)*(source[18].yyyy)).xyz;
    // 129: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 130: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 131: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 132: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 133: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 136: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 137: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 138: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 140: mul_sat r6.xy, r6.xzxx, cb0[15].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[15].yyyy))).xy;
    // 141: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 142: add_sat r6.y, r6.y, -cb0[15].z
    r6.y = (saturate((r6.yyyy)+(-(source[15].zzzz)))).y;
    // 143: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 144: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 145: mul r6.y, r6.y, cb0[15].w
    r6.y = ((r6.yyyy)*(source[15].wwww)).y;
    // 146: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 147: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 148: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 149: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 151: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 152: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 153: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 154: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 155: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 156: mul r0.z, r0.z, cb0[18].w
    r0.z = ((r0.zzzz)*(source[18].wwww)).z;
    // 157: mad r6.y, cb0[18].z, r6.y, -r4.z
    r6.y = ((source[18].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 158: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 159: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 160: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 161: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 162: mad r6.yzw, -cb0[18].yyyy, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[18].yyyy))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 163: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 164: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 165: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 166: mad r9.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r9.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 167: mad r7.xyz, cb0[13].yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((source[13].yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 168: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 169: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 170: mad r7.xyz, cb0[13].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[13].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 171: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 173: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 174: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 175: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 176: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 177: mad r8.xyz, cb0[13].yyyy, r11.xyzx, r8.xyzx
    r8.xyz = ((source[13].yyyy)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 178: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 180: mad r8.xyz, cb0[13].zzzz, r11.xyzx, r8.xyzx
    r8.xyz = ((source[13].zzzz)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 181: mul r11.xyz, r7.xyzx, r8.xyzx
    r11.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 182: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 184: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 185: add r1.yzw, -cb0[6].xxyz, cb0[7].xxyz
    r1.yzw = ((-(source[6].xxyz))+(source[7].xxyz)).yzw;
    // 186: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[6].xyzx)).xyz;
    // 187: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 188: mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // 189: mul r12.xyz, r1.xyzx, r11.xyzx
    r12.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 190: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 191: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 192: mad r2.xyz, cb0[13].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[13].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 193: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 194: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 195: mad r2.xyz, cb0[13].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[13].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 196: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 197: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 198: mul r13.xyz, r13.xyzx, cb0[15].xxxx
    r13.xyz = ((r13.xyzx)*(source[15].xxxx)).xyz;
    // 199: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r14.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 200: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 201: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 202: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 203: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 204: max r13.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r13.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 205: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 206: mul r13.xyz, r13.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 207: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 208: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 209: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 210: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 211: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 212: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 213: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 214: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 215: div r1.w, cb0[16].y, r1.w
    r1.w = ((source[16].yyyy)/(r1.wwww)).w;
    // 216: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 217: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 218: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 219: mul r1.w, r1.w, cb0[16].z
    r1.w = ((r1.wwww)*(source[16].zzzz)).w;
    // 220: mad r1.xyz, r2.xyzx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 221: mad r1.xyz, r1.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 222: mad r1.xyz, r4.xxxx, r1.xyzx, -r11.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r11.xyzx))).xyz;
    // 223: mad r1.xyz, r0.zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 224: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 225: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 226: mad r1.xyz, cb0[13].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[13].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 227: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 228: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 229: mad r1.xyz, cb0[13].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[13].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 230: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 231: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 232: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 233: mul r4.z, r4.z, cb0[17].z
    r4.z = ((r4.zzzz)*(source[17].zzzz)).z;
    // 234: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 235: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 236: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 237: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 238: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 239: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 240: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 241: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 242: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 243: mul r9.y, cb0[2].y, cb0[8].y
    r9.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 244: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 245: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 246: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 247: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 248: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 249: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 250: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 251: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 252: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 253: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 254: mul r1.w, cb0[9].y, cb0[17].z
    r1.w = ((source[9].yyyy)*(source[17].zzzz)).w;
    // 255: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 256: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 257: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 258: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 259: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 260: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 261: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 262: mul r3.z, cb0[9].x, l(0.001000)
    r3.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 263: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 264: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 265: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 266: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 267: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 268: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 269: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 270: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 271: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 272: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 273: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 274: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 275: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 276: mul r9.xyz, r3.xyzx, cb0[9].zzzz
    r9.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 277: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 278: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 279: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 280: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 281: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 282: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 283: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 284: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 285: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 286: mul r0.y, r2.w, cb0[19].x
    r0.y = ((r2.wwww)*(source[19].xxxx)).y;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 288: add r0.w, -cb0[19].y, l(2.000000)
    r0.w = ((-(source[19].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 289: mad r0.w, r4.x, r0.w, cb0[19].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[19].yyyy)).w;
    // 290: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 291: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 292: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 293: mul r0.xyz, r0.xyzx, cb0[19].zzzz
    r0.xyz = ((r0.xyzx)*(source[19].zzzz)).xyz;
    // 294: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 295: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 296: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 297: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 298: mul r0.xyz, r0.xyzx, cb0[19].wwww
    r0.xyz = ((r0.xyzx)*(source[19].wwww)).xyz;
    // 299: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 300: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 301: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 302: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 303: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 304: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 305: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 306: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 307: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 308: ret
    return output;
}

// source.character.monster-63e86c4fac91.v1 / source program 235cc83193fbb44a9a64f1a8751731a9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight29(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].y=(g_SourceCharacterTime.xxxx).x;
    source[16].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[20]=float4(input.lightColor,1.0);
    source[21].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).w;
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
    // 31: mad r8.xy, r8.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r8.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: mul r9.xy, r8.xyxx, cb0[12].xxxx
    r9.xy = ((r8.xyxx)*(source[12].xxxx)).xy;
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
    // 39: mad r8.xyz, cb0[12].wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((source[12].wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
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
    // 59: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 61: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 62: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 63: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 64: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 66: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 67: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 68: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 69: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 70: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 71: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 72: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 73: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 74: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 75: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 76: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 77: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 78: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 79: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 80: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 81: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 82: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t3.xywz, s4, r1.x
    r1.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 84: rcp r0.x, cb0[13].z
    r0.x = (1.0/(source[13].zzzz)).x;
    // 85: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 86: mul r8.xyz, r4.xyzx, cb0[13].zzzz
    r8.xyz = ((r4.xyzx)*(source[13].zzzz)).xyz;
    // 87: exp r8.xyz, r8.xyzx
    r8.xyz = (exp2(r8.xyzx)).xyz;
    // 88: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 89: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 90: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 91: mad r4.xyz, r8.xyzx, cb0[13].zzzz, r4.xyzx
    r4.xyz = ((r8.xyzx)*(source[13].zzzz)+(r4.xyzx)).xyz;
    // 92: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 93: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 94: add r0.x, cb0[13].z, l(1.000000)
    r0.x = ((source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 95: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 96: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 97: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 98: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 99: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 100: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 101: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 102: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 103: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 104: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 105: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 106: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 107: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 108: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 109: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 110: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 111: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 112: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 114: mul r8.xyz, r1.xywx, r4.xxxx
    r8.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 115: mul r10.xyz, r8.xyzx, cb0[17].zzzz
    r10.xyz = ((r8.xyzx)*(source[17].zzzz)).xyz;
    // 116: dp3 r4.z, r9.xyzx, r9.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 117: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 118: div r9.xyz, r9.xyzx, r4.zzzz
    r9.xyz = ((r9.xyzx)/(r4.zzzz)).xyz;
    // 119: dp3 r4.z, r9.xyzx, r7.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 120: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 121: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 123: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 124: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 125: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 127: mul_sat r6.xy, r6.xzxx, cb0[14].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[14].yyyy))).xy;
    // 128: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 129: add_sat r6.y, r6.y, -cb0[14].z
    r6.y = (saturate((r6.yyyy)+(-(source[14].zzzz)))).y;
    // 130: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 131: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 132: mul r6.y, r6.y, cb0[14].w
    r6.y = ((r6.yyyy)*(source[14].wwww)).y;
    // 133: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 134: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 135: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 136: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 137: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 138: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 139: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 140: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 141: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 142: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 143: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 144: mad r6.y, cb0[17].w, r6.y, -r4.z
    r6.y = ((source[17].wwww)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 145: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 146: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 147: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 148: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 149: mad r6.yzw, -cb0[17].zzzz, r8.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[17].zzzz))*(r8.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 150: mad r6.yzw, r5.xxyz, r6.yyzw, r10.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r10.xxyz)).yzw;
    // 151: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 152: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 153: mad r8.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r8.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 154: mad r7.xyz, cb0[12].yyyy, r8.xyzx, r7.xyzx
    r7.xyz = ((source[12].yyyy)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 155: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 156: add r8.xyz, -r7.xyzx, r0.zzzz
    r8.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 157: mad r7.xyz, cb0[12].zzzz, r8.xyzx, r7.xyzx
    r7.xyz = ((source[12].zzzz)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 158: mad r8.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 160: mul r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)).xyz;
    // 161: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 162: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 163: dp3 r0.z, r10.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r10.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 164: add r11.xyz, -r10.yzwy, r0.zzzz
    r11.xyz = ((-(r10.yzwy))+(r0.zzzz)).xyz;
    // 165: mad r10.yzw, cb0[12].yyyy, r11.xxyz, r10.yyzw
    r10.yzw = ((source[12].yyyy)*(r11.xxyz)+(r10.yyzw)).yzw;
    // 166: dp3 r0.z, r10.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r10.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 167: add r11.xyz, -r10.yzwy, r0.zzzz
    r11.xyz = ((-(r10.yzwy))+(r0.zzzz)).xyz;
    // 168: mad r10.yzw, cb0[12].zzzz, r11.xxyz, r10.yyzw
    r10.yzw = ((source[12].zzzz)*(r11.xxyz)+(r10.yyzw)).yzw;
    // 169: mul r11.xyz, r7.xyzx, r10.yzwy
    r11.xyz = ((r7.xyzx)*(r10.yzwy)).xyz;
    // 170: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 171: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 172: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 173: add r1.yzw, -cb0[6].xxyz, cb0[7].xxyz
    r1.yzw = ((-(source[6].xxyz))+(source[7].xxyz)).yzw;
    // 174: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[6].xyzx)).xyz;
    // 175: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 176: mul r1.xyz, r1.xyzx, cb0[13].wwww
    r1.xyz = ((r1.xyzx)*(source[13].wwww)).xyz;
    // 177: mul r12.xyz, r1.xyzx, r11.xyzx
    r12.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 178: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 180: mad r2.xyz, cb0[12].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 181: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 182: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 183: mad r2.xyz, cb0[12].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 184: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 185: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 186: mul r13.xyz, r13.xyzx, cb0[14].xxxx
    r13.xyz = ((r13.xyzx)*(source[14].xxxx)).xyz;
    // 187: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r14.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 188: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 189: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 190: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 191: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 192: max r13.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r13.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 193: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 194: mul r13.xyz, r13.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 195: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 196: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 197: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 198: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 199: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 200: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 201: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 202: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 203: div r1.w, cb0[15].y, r1.w
    r1.w = ((source[15].yyyy)/(r1.wwww)).w;
    // 204: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 205: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 206: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 207: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 208: mad r1.xyz, r2.xyzx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 209: mad r1.xyz, r1.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 210: mad r1.xyz, r4.xxxx, r1.xyzx, -r11.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r11.xyzx))).xyz;
    // 211: mad r1.xyz, r0.zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 212: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 213: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 214: mad r1.xyz, cb0[12].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 215: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 216: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 217: mad r1.xyz, cb0[12].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 218: mul r1.xyz, r8.xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(r1.xyzx)).xyz;
    // 219: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 220: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 221: mul r4.z, r4.z, cb0[16].y
    r4.z = ((r4.zzzz)*(source[16].yyyy)).z;
    // 222: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 223: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 224: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 225: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 226: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 227: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 228: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 229: mul r8.z, r6.x, l(0.125000)
    r8.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 230: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 231: mul r8.y, cb0[2].y, cb0[8].y
    r8.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 232: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 233: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 234: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 235: add r8.xy, r8.xyxx, r11.xyxx
    r8.xy = ((r8.xyxx)+(r11.xyxx)).xy;
    // 236: add r8.xy, r8.xyxx, r8.zwzz
    r8.xy = ((r8.xyxx)+(r8.zwzz)).xy;
    // 237: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r8.xyxx, t5.xyzw, s6, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 238: mul r8.xyz, r1.wwww, r8.xyzx
    r8.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 239: mul r1.w, r4.z, r8.w
    r1.w = ((r4.zzzz)*(r8.wwww)).w;
    // 240: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 241: mad r1.xyz, r1.wwww, r8.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 242: mul r1.w, cb0[9].y, cb0[16].y
    r1.w = ((source[9].yyyy)*(source[16].yyyy)).w;
    // 243: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 244: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 245: mul r8.y, r1.w, l(0.020000)
    r8.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 246: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 247: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 248: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 249: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 250: mul r3.z, cb0[9].x, l(0.001000)
    r3.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 251: mov r8.x, l(0)
    r8.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 252: mad r3.xy, r3.zzzz, r3.xyxx, r8.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r8.xyxx)).xy;
    // 253: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 254: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 255: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 256: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 257: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 258: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 259: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 260: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 261: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 262: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 263: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 264: mul r8.xyz, r3.xyzx, cb0[9].zzzz
    r8.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 265: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 267: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 268: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 269: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 270: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 271: mad r5.xyz, r7.xyzx, r10.yzwy, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r10.yzwy)+(-(r2.xyzx))).xyz;
    // 272: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 273: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 274: mul r0.y, r2.w, cb0[18].y
    r0.y = ((r2.wwww)*(source[18].yyyy)).y;
    // 275: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 276: add r0.w, -cb0[18].z, l(2.000000)
    r0.w = ((-(source[18].zzzz))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 277: mad r0.w, r4.x, r0.w, cb0[18].z
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[18].zzzz)).w;
    // 278: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 279: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 280: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 281: mul r0.xyz, r0.xyzx, cb0[18].wwww
    r0.xyz = ((r0.xyzx)*(source[18].wwww)).xyz;
    // 282: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 283: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 284: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 285: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 286: mul r0.xyz, r0.xyzx, cb0[19].xxxx
    r0.xyz = ((r0.xyzx)*(source[19].xxxx)).xyz;
    // 287: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 288: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 289: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 290: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 291: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 292: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 293: mov_sat r10.x, r10.x
    r10.x = (saturate(r10.xxxx)).x;
    // 294: mul_sat r0.x, r10.x, cb0[17].x
    r0.x = (saturate((r10.xxxx)*(source[17].xxxx))).x;
    // 295: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 296: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 297: ret
    return output;
}

// source.character.monster-3b27d7fb1b53.v1 / source program 539b8b83456127488672d7f0e2c65bb9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight30(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].y=(g_SourceCharacterTime.xxxx).x;
    source[16].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[19]=float4(input.lightColor,1.0);
    source[20].x=1.0;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r8.xy, r8.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r8.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: mul r9.xy, r8.xyxx, cb0[12].xxxx
    r9.xy = ((r8.xyxx)*(source[12].xxxx)).xy;
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
    // 39: mad r8.xyz, cb0[12].wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((source[12].wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
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
    // 59: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 61: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 62: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 63: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 64: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 66: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 67: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 68: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 69: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 70: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 71: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 72: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 73: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 74: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 75: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 76: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 77: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 78: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 79: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 80: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 81: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 82: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t3.xywz, s4, r1.x
    r1.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 84: rcp r0.x, cb0[13].z
    r0.x = (1.0/(source[13].zzzz)).x;
    // 85: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 86: mul r8.xyz, r4.xyzx, cb0[13].zzzz
    r8.xyz = ((r4.xyzx)*(source[13].zzzz)).xyz;
    // 87: exp r8.xyz, r8.xyzx
    r8.xyz = (exp2(r8.xyzx)).xyz;
    // 88: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 89: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 90: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 91: mad r4.xyz, r8.xyzx, cb0[13].zzzz, r4.xyzx
    r4.xyz = ((r8.xyzx)*(source[13].zzzz)+(r4.xyzx)).xyz;
    // 92: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 93: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 94: add r0.x, cb0[13].z, l(1.000000)
    r0.x = ((source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 95: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 96: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 97: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 98: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 99: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 100: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 101: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 102: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 103: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 104: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 105: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 106: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 107: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 108: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 109: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 110: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 111: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 112: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 114: mul r8.xyz, r1.xywx, r4.xxxx
    r8.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 115: mul r10.xyz, r8.xyzx, cb0[17].yyyy
    r10.xyz = ((r8.xyzx)*(source[17].yyyy)).xyz;
    // 116: dp3 r4.z, r9.xyzx, r9.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 117: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 118: div r9.xyz, r9.xyzx, r4.zzzz
    r9.xyz = ((r9.xyzx)/(r4.zzzz)).xyz;
    // 119: dp3 r4.z, r9.xyzx, r7.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 120: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 121: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 123: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 124: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 125: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 127: mul_sat r6.xy, r6.xzxx, cb0[14].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[14].yyyy))).xy;
    // 128: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 129: add_sat r6.y, r6.y, -cb0[14].z
    r6.y = (saturate((r6.yyyy)+(-(source[14].zzzz)))).y;
    // 130: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 131: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 132: mul r6.y, r6.y, cb0[14].w
    r6.y = ((r6.yyyy)*(source[14].wwww)).y;
    // 133: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 134: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 135: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 136: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 137: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 138: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 139: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 140: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 141: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 142: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 143: mul r0.z, r0.z, cb0[17].w
    r0.z = ((r0.zzzz)*(source[17].wwww)).z;
    // 144: mad r6.y, cb0[17].z, r6.y, -r4.z
    r6.y = ((source[17].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 145: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 146: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 147: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 148: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 149: mad r6.yzw, -cb0[17].yyyy, r8.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[17].yyyy))*(r8.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 150: mad r6.yzw, r5.xxyz, r6.yyzw, r10.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r10.xxyz)).yzw;
    // 151: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 152: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 153: mad r8.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r8.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 154: mad r7.xyz, cb0[12].yyyy, r8.xyzx, r7.xyzx
    r7.xyz = ((source[12].yyyy)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 155: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 156: add r8.xyz, -r7.xyzx, r0.zzzz
    r8.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 157: mad r7.xyz, cb0[12].zzzz, r8.xyzx, r7.xyzx
    r7.xyz = ((source[12].zzzz)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 158: mad r8.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 160: mul r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)).xyz;
    // 161: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 162: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 163: dp3 r0.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 164: add r11.xyz, -r10.xyzx, r0.zzzz
    r11.xyz = ((-(r10.xyzx))+(r0.zzzz)).xyz;
    // 165: mad r10.xyz, cb0[12].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[12].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 166: dp3 r0.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 167: add r11.xyz, -r10.xyzx, r0.zzzz
    r11.xyz = ((-(r10.xyzx))+(r0.zzzz)).xyz;
    // 168: mad r10.xyz, cb0[12].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[12].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 169: mul r11.xyz, r7.xyzx, r10.xyzx
    r11.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 170: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 171: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 172: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 173: add r1.yzw, -cb0[6].xxyz, cb0[7].xxyz
    r1.yzw = ((-(source[6].xxyz))+(source[7].xxyz)).yzw;
    // 174: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[6].xyzx)).xyz;
    // 175: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 176: mul r1.xyz, r1.xyzx, cb0[13].wwww
    r1.xyz = ((r1.xyzx)*(source[13].wwww)).xyz;
    // 177: mul r12.xyz, r1.xyzx, r11.xyzx
    r12.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 178: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 180: mad r2.xyz, cb0[12].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 181: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 182: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 183: mad r2.xyz, cb0[12].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 184: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 185: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 186: mul r13.xyz, r13.xyzx, cb0[14].xxxx
    r13.xyz = ((r13.xyzx)*(source[14].xxxx)).xyz;
    // 187: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r14.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 188: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 189: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 190: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 191: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 192: max r13.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r13.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 193: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 194: mul r13.xyz, r13.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 195: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 196: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 197: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 198: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 199: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 200: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 201: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 202: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 203: div r1.w, cb0[15].y, r1.w
    r1.w = ((source[15].yyyy)/(r1.wwww)).w;
    // 204: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 205: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 206: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 207: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 208: mad r1.xyz, r2.xyzx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 209: mad r1.xyz, r1.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 210: mad r1.xyz, r4.xxxx, r1.xyzx, -r11.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r11.xyzx))).xyz;
    // 211: mad r1.xyz, r0.zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 212: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 213: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 214: mad r1.xyz, cb0[12].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 215: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 216: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 217: mad r1.xyz, cb0[12].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 218: mul r1.xyz, r8.xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(r1.xyzx)).xyz;
    // 219: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 220: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 221: mul r4.z, r4.z, cb0[16].y
    r4.z = ((r4.zzzz)*(source[16].yyyy)).z;
    // 222: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 223: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 224: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 225: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 226: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 227: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 228: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 229: mul r8.z, r6.x, l(0.125000)
    r8.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 230: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 231: mul r8.y, cb0[2].y, cb0[8].y
    r8.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 232: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 233: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 234: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 235: add r8.xy, r8.xyxx, r11.xyxx
    r8.xy = ((r8.xyxx)+(r11.xyxx)).xy;
    // 236: add r8.xy, r8.xyxx, r8.zwzz
    r8.xy = ((r8.xyxx)+(r8.zwzz)).xy;
    // 237: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r8.xyxx, t5.xyzw, s6, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 238: mul r8.xyz, r1.wwww, r8.xyzx
    r8.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 239: mul r1.w, r4.z, r8.w
    r1.w = ((r4.zzzz)*(r8.wwww)).w;
    // 240: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 241: mad r1.xyz, r1.wwww, r8.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 242: mul r1.w, cb0[9].y, cb0[16].y
    r1.w = ((source[9].yyyy)*(source[16].yyyy)).w;
    // 243: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 244: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 245: mul r8.y, r1.w, l(0.020000)
    r8.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 246: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 247: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 248: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 249: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 250: mul r3.z, cb0[9].x, l(0.001000)
    r3.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 251: mov r8.x, l(0)
    r8.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 252: mad r3.xy, r3.zzzz, r3.xyxx, r8.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r8.xyxx)).xy;
    // 253: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 254: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 255: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 256: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 257: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 258: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 259: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 260: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 261: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 262: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 263: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 264: mul r8.xyz, r3.xyzx, cb0[9].zzzz
    r8.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 265: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 267: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 268: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 269: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 270: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 271: mad r5.xyz, r7.xyzx, r10.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r10.xyzx)+(-(r2.xyzx))).xyz;
    // 272: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 273: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 274: mul r0.y, r2.w, cb0[18].x
    r0.y = ((r2.wwww)*(source[18].xxxx)).y;
    // 275: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 276: add r0.w, -cb0[18].y, l(2.000000)
    r0.w = ((-(source[18].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 277: mad r0.w, r4.x, r0.w, cb0[18].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[18].yyyy)).w;
    // 278: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 279: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 280: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 281: mul r0.xyz, r0.xyzx, cb0[18].zzzz
    r0.xyz = ((r0.xyzx)*(source[18].zzzz)).xyz;
    // 282: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 283: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 284: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 285: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 286: mul r0.xyz, r0.xyzx, cb0[18].wwww
    r0.xyz = ((r0.xyzx)*(source[18].wwww)).xyz;
    // 287: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 288: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 289: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 290: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 291: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 292: mul o0.xyz, r0.xyzx, cb0[19].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[19].xyzx)).xyz;
    // 293: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 294: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 295: ret
    return output;
}

// source.character.monster-d1e4d915a43a.v1 / source program ec8029d75942a04abbd928cd56dcb0f0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight31(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].w=(g_SourceCharacterTime.xxxx).x;
    source[20].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[20].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[24]=float4(input.lightColor,1.0);
    source[25].x=1.0;
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
    // 42: mul r10.xy, r9.xyxx, cb0[14].xxxx
    r10.xy = ((r9.xyxx)*(source[14].xxxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mad r9.xy, cb0[14].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[14].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 45: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 46: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 47: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r12.xyz, cb0[16].xxxx, r10.xyzx, r9.xyzx
    r12.xyz = ((source[16].xxxx)*(r10.xyzx)+(r9.xyzx)).xyz;
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
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 68: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 69: add r1.x, -cb0[15].y, cb0[15].x
    r1.x = ((-(source[15].yyyy))+(source[15].xxxx)).x;
    // 70: mad r1.x, r11.w, r1.x, cb0[15].y
    r1.x = ((r11.wwww)*(r1.xxxx)+(source[15].yyyy)).x;
    // 71: lt r1.y, |r0.z|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 72: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 73: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 74: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 75: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 76: movc r0.z, r1.y, l(0), r0.z
    r0.z = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 77: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 78: add r1.y, -r1.x, cb0[16].y
    r1.y = ((-(r1.xxxx))+(source[16].yyyy)).y;
    // 79: mad r1.x, r11.w, r1.y, r1.x
    r1.x = ((r11.wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 80: mul r1.x, r1.x, cb0[16].z
    r1.x = ((r1.xxxx)*(source[16].zzzz)).x;
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
    // 86: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 87: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 88: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 89: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 90: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 91: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 92: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 93: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 94: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 95: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 97: rcp r0.x, cb0[16].w
    r0.x = (1.0/(source[16].wwww)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r12.xyz, r4.xyzx, cb0[16].wwww
    r12.xyz = ((r4.xyzx)*(source[16].wwww)).xyz;
    // 100: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r12.xyzx, cb0[16].wwww, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[16].wwww)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[16].w, l(1.000000)
    r0.x = ((source[16].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 109: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 110: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 111: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 112: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 113: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 114: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 116: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 117: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 118: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 119: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 120: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 121: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 122: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 123: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 125: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 127: mul r12.xyz, r1.xywx, r4.xxxx
    r12.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 128: mul r13.xyz, r12.xyzx, cb0[20].wwww
    r13.xyz = ((r12.xyzx)*(source[20].wwww)).xyz;
    // 129: mul r4.z, r11.w, l(0.500000)
    r4.z = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 130: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 132: mul r4.w, r0.z, r4.w
    r4.w = ((r0.zzzz)*(r4.wwww)).w;
    // 133: mul r4.z, r4.w, r4.z
    r4.z = ((r4.wwww)*(r4.zzzz)).z;
    // 134: mad r9.xyz, r4.zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((r4.zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 135: dp3 r4.z, r9.xyzx, r9.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 136: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 137: div r9.xyz, r9.xyzx, r4.zzzz
    r9.xyz = ((r9.xyzx)/(r4.zzzz)).xyz;
    // 138: dp3 r4.z, r9.xyzx, r7.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 139: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 140: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 142: dp3 r6.w, cb0[13].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[13].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 143: add r7.xyz, r6.wwww, -cb0[13].xyzx
    r7.xyz = ((r6.wwww)+(-(source[13].xyzx))).xyz;
    // 144: mad r7.xyz, r5.wwww, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[13].xyzx)).xyz;
    // 145: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 146: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 147: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 148: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 149: mad r7.xyz, r7.xyzx, r4.zzzz, r5.wwww
    r7.xyz = ((r7.xyzx)*(r4.zzzz)+(r5.wwww)).xyz;
    // 150: add_sat r4.z, r11.w, cb0[21].x
    r4.z = (saturate((r11.wwww)+(source[21].xxxx))).z;
    // 151: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 153: mul_sat r6.xy, r6.xzxx, cb0[17].zzzz
    r6.xy = (saturate((r6.xzxx)*(source[17].zzzz))).xy;
    // 154: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 155: add_sat r6.y, r6.y, -cb0[17].w
    r6.y = (saturate((r6.yyyy)+(-(source[17].wwww)))).y;
    // 156: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 157: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 158: mul r6.y, r6.y, cb0[18].x
    r6.y = ((r6.yyyy)*(source[18].xxxx)).y;
    // 159: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 160: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 161: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 162: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 163: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 164: mad r7.w, r2.w, l(2.000000), -r4.x
    r7.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).w;
    // 165: mad r6.yzw, r6.yyzw, r7.wwww, r4.xxxx
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r4.xxxx)).yzw;
    // 166: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 167: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 168: mul r7.w, r0.z, r0.z
    r7.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 169: mul r8.x, r7.w, cb0[21].y
    r8.x = ((r7.wwww)*(source[21].yyyy)).x;
    // 170: mad r0.z, -r7.w, cb0[21].y, r0.z
    r0.z = ((-(r7.wwww))*(source[21].yyyy)+(r0.zzzz)).z;
    // 171: mad r0.z, r11.w, r0.z, r8.x
    r0.z = ((r11.wwww)*(r0.zzzz)+(r8.xxxx)).z;
    // 172: mad r6.yzw, r4.zzzz, r6.yyzw, -r7.xxyz
    r6.yzw = ((r4.zzzz)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 173: mad r6.yzw, r0.zzzz, r6.yyzw, r7.xxyz
    r6.yzw = ((r0.zzzz)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 174: sqrt r0.z, r5.w
    r0.z = (sqrt(r5.wwww)).z;
    // 175: mul r5.xyz, r5.xyzx, r0.zzzz
    r5.xyz = ((r5.xyzx)*(r0.zzzz)).xyz;
    // 176: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 177: mad r6.yzw, -cb0[20].wwww, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[20].wwww))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 178: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 179: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 180: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 181: mad r10.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r10.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 182: mad r7.xyz, cb0[15].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[15].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 183: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 184: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 185: mad r7.xyz, cb0[15].wwww, r10.xyzx, r7.xyzx
    r7.xyz = ((source[15].wwww)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 186: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 189: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 190: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[6].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[6].xyzx)).xyz;
    // 191: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 192: dp3 r0.z, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r12.xyz, -r8.yzwy, r0.zzzz
    r12.xyz = ((-(r8.yzwy))+(r0.zzzz)).xyz;
    // 194: mad r8.xyz, cb0[15].zzzz, r12.xyzx, r8.yzwy
    r8.xyz = ((source[15].zzzz)*(r12.xyzx)+(r8.yzwy)).xyz;
    // 195: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r8.xyz, cb0[15].wwww, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].wwww)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 198: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 199: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 200: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 201: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 202: add r1.yzw, -cb0[7].xxyz, cb0[8].xxyz
    r1.yzw = ((-(source[7].xxyz))+(source[8].xxyz)).yzw;
    // 203: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[7].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[7].xyzx)).xyz;
    // 204: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 205: mul r1.xyz, r1.xyzx, cb0[17].xxxx
    r1.xyz = ((r1.xyzx)*(source[17].xxxx)).xyz;
    // 206: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 207: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 208: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 209: mad r14.xyz, cb0[15].zzzz, r14.xyzx, r2.xyzx
    r14.xyz = ((source[15].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 210: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 211: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 212: mad r14.xyz, cb0[15].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[15].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 213: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 214: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 215: mul r15.xyz, r15.xyzx, cb0[17].yyyy
    r15.xyz = ((r15.xyzx)*(source[17].yyyy)).xyz;
    // 216: add r0.z, r11.y, r11.x
    r0.z = ((r11.yyyy)+(r11.xxxx)).z;
    // 217: add r0.z, r11.z, r0.z
    r0.z = ((r11.zzzz)+(r0.zzzz)).z;
    // 218: add_sat r0.z, r11.w, r0.z
    r0.z = (saturate((r11.wwww)+(r0.zzzz))).z;
    // 219: mad r11.xyz, r0.zzzz, r15.xyzx, r14.xyzx
    r11.xyz = ((r0.zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 220: add r2.xyz, r2.xxxx, -r11.xyzx
    r2.xyz = ((r2.xxxx)+(-(r11.xyzx))).xyz;
    // 221: mad r2.xyz, r11.wwww, r2.xyzx, r11.xyzx
    r2.xyz = ((r11.wwww)*(r2.xyzx)+(r11.xyzx)).xyz;
    // 222: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 223: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 224: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 225: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 226: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 227: add r1.w, -cb0[18].z, cb0[18].y
    r1.w = ((-(source[18].zzzz))+(source[18].yyyy)).w;
    // 228: mad r1.w, r11.w, r1.w, cb0[18].z
    r1.w = ((r11.wwww)*(r1.wwww)+(source[18].zzzz)).w;
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
    // 235: div r1.w, cb0[18].w, r1.w
    r1.w = ((source[18].wwww)/(r1.wwww)).w;
    // 236: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 237: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 238: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: mul r1.w, r1.w, cb0[19].x
    r1.w = ((r1.wwww)*(source[19].xxxx)).w;
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
    // 245: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 246: mad r1.xyz, cb0[15].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[15].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 247: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 248: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 249: mad r1.xyz, cb0[15].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[15].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 250: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 251: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 252: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 253: mul r4.z, r4.z, cb0[19].w
    r4.z = ((r4.zzzz)*(source[19].wwww)).z;
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
    // 261: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 262: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 263: mul r10.y, cb0[2].y, cb0[9].y
    r10.y = ((source[2].yyyy)*(source[9].yyyy)).y;
    // 264: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 265: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 266: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 267: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 268: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 269: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t5.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 270: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 271: mul r1.w, r4.z, r10.w
    r1.w = ((r4.zzzz)*(r10.wwww)).w;
    // 272: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 273: mad r1.xyz, r1.wwww, r10.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 274: mul r1.w, cb0[10].y, cb0[19].w
    r1.w = ((source[10].yyyy)*(source[19].wwww)).w;
    // 275: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 276: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 277: mul r10.y, r1.w, l(0.020000)
    r10.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 278: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 279: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 280: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 281: mad r3.xy, cb0[10].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[10].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 282: mul r3.z, cb0[10].x, l(0.001000)
    r3.z = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 283: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 284: mad r3.xy, r3.zzzz, r3.xyxx, r10.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r10.xyxx)).xy;
    // 285: dp2 r3.z, cb0[11].xyxx, r3.xyxx
    r3.z = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 286: dp2 r3.y, cb0[12].xyxx, r3.xyxx
    r3.y = (dot((source[12].xyxx).xy,(r3.xyxx).xy).xxxx).y;
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
    // 296: mul r10.xyz, r3.xyzx, cb0[10].zzzz
    r10.xyz = ((r3.xyzx)*(source[10].zzzz)).xyz;
    // 297: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 298: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 299: mad r3.xyz, cb0[10].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[10].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 300: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 301: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 302: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 303: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 304: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 305: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 306: mul r0.z, r2.w, cb0[21].z
    r0.z = ((r2.wwww)*(source[21].zzzz)).z;
    // 307: mul r0.w, r11.w, r0.z
    r0.w = ((r11.wwww)*(r0.zzzz)).w;
    // 308: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 309: min r0.w, r0.w, cb0[21].z
    r0.w = (min(r0.wwww,source[21].zzzz)).w;
    // 310: add r1.w, -cb0[22].y, cb0[22].x
    r1.w = ((-(source[22].yyyy))+(source[22].xxxx)).w;
    // 311: mad r1.w, cb0[21].w, r1.w, cb0[22].y
    r1.w = ((source[21].wwww)*(r1.wwww)+(source[22].yyyy)).w;
    // 312: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 313: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 314: mad r1.w, r11.w, r1.w, l(1.000000)
    r1.w = ((r11.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    // 321: add r0.y, cb0[22].w, -cb0[23].x
    r0.y = ((source[22].wwww)+(-(source[23].xxxx))).y;
    // 322: mad r0.y, cb0[22].z, r0.y, cb0[23].x
    r0.y = ((source[22].zzzz)*(r0.yyyy)+(source[23].xxxx)).y;
    // 323: mul r0.y, r0.y, r11.w
    r0.y = ((r0.yyyy)*(r11.wwww)).y;
    // 324: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t6.xwyz, s7, l(0.000000)
    r0.xzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // 325: mad r0.xyz, r0.yyyy, r5.xyzx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r5.xyzx)+(r0.xzwx)).xyz;
    // 326: add r0.w, -cb0[23].y, l(2.000000)
    r0.w = ((-(source[23].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 327: mad r0.w, r4.x, r0.w, cb0[23].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[23].yyyy)).w;
    // 328: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 329: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 330: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 331: mul r0.xyz, r0.xyzx, cb0[23].zzzz
    r0.xyz = ((r0.xyzx)*(source[23].zzzz)).xyz;
    // 332: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 333: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 334: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 335: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 336: mul r0.xyz, r0.xyzx, cb0[23].wwww
    r0.xyz = ((r0.xyzx)*(source[23].wwww)).xyz;
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
    // 342: mul o0.xyz, r0.xyzx, cb0[24].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[24].xyzx)).xyz;
    // 343: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 344: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 345: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 346: ret
    return output;
}

// source.character.monster-411db5c836e7.v1 / source program 6b368ae945f9814ba34651c9880020b0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight32(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].y=(g_SourceCharacterTime.xxxx).x;
    source[16].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[20]=float4(input.lightColor,1.0);
    source[21].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r9.xy, v4.xyxx, cb0[17].xxxx
    r9.xy = ((v4.xyxx)*(source[17].xxxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r9.xyxx, t2.yzwx, s7, l(0.000000)
    r1.w = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 33: add r1.w, r1.w, -cb0[17].y
    r1.w = ((r1.wwww)+(-(source[17].yyyy))).w;
    // 34: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 35: mul_sat r1.w, r1.w, r8.w
    r1.w = (saturate((r1.wwww)*(r8.wwww))).w;
    // 36: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 37: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 38: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 40: mad r9.xy, r9.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((r9.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: mul r10.xy, r9.xyxx, cb0[12].xxxx
    r10.xy = ((r9.xyxx)*(source[12].xxxx)).xy;
    // 42: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 43: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 46: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 47: add r9.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r9.xyz, cb0[12].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[12].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 49: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 52: dp3 r11.x, r1.xyzx, r9.xyzx
    r11.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 53: dp3 r11.y, r2.xyzx, r9.xyzx
    r11.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 54: dp3 r11.z, r0.xyzx, r9.xyzx
    r11.z = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 55: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 56: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 57: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: dp3 r0.x, r11.xyzx, r1.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 59: mul r0.xyz, r11.xyzx, r0.xxxx
    r0.xyz = ((r11.xyzx)*(r0.xxxx)).xyz;
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
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 68: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 69: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 70: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 71: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 72: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 73: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 74: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 75: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 76: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 77: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 78: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 79: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 80: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 81: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 82: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 83: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 84: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 85: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 86: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 87: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 88: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 89: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 90: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 91: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 92: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s4, r1.x
    r1.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 93: rcp r0.x, cb0[13].z
    r0.x = (1.0/(source[13].zzzz)).x;
    // 94: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 95: mul r9.xyz, r4.xyzx, cb0[13].zzzz
    r9.xyz = ((r4.xyzx)*(source[13].zzzz)).xyz;
    // 96: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 97: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 98: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 99: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 100: mad r4.xyz, r9.xyzx, cb0[13].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[13].zzzz)+(r4.xyzx)).xyz;
    // 101: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 102: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 103: add r0.x, cb0[13].z, l(1.000000)
    r0.x = ((source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 104: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 105: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 106: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 107: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 108: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 109: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 110: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 112: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 113: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 114: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 115: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 116: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 117: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 118: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 119: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 120: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 121: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 123: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 124: mul r11.xyz, r9.xyzx, cb0[17].wwww
    r11.xyz = ((r9.xyzx)*(source[17].wwww)).xyz;
    // 125: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 126: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 127: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 128: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 129: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 130: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 132: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 133: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 134: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 136: mul_sat r6.xy, r6.xzxx, cb0[14].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[14].yyyy))).xy;
    // 137: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 138: add_sat r6.y, r6.y, -cb0[14].z
    r6.y = (saturate((r6.yyyy)+(-(source[14].zzzz)))).y;
    // 139: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 140: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 141: mul r6.y, r6.y, cb0[14].w
    r6.y = ((r6.yyyy)*(source[14].wwww)).y;
    // 142: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 143: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 144: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 145: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 146: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 147: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 148: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 149: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 151: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 152: mul r0.z, r0.z, cb0[18].y
    r0.z = ((r0.zzzz)*(source[18].yyyy)).z;
    // 153: mad r6.y, cb0[18].x, r6.y, -r4.z
    r6.y = ((source[18].xxxx)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 154: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 155: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 156: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 157: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 158: mad r6.yzw, -cb0[17].wwww, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[17].wwww))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 159: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 160: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 161: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 162: mad r9.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r9.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 163: mad r7.xyz, cb0[12].yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 164: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 165: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 166: mad r7.xyz, cb0[12].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 167: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 170: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 171: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 172: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 173: mad r8.xyz, cb0[12].yyyy, r11.xyzx, r8.xyzx
    r8.xyz = ((source[12].yyyy)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 174: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 175: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 176: mad r8.xyz, cb0[12].zzzz, r11.xyzx, r8.xyzx
    r8.xyz = ((source[12].zzzz)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 177: mul r11.xyz, r7.xyzx, r8.xyzx
    r11.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 178: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 180: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 181: add r1.yzw, -cb0[6].xxyz, cb0[7].xxyz
    r1.yzw = ((-(source[6].xxyz))+(source[7].xxyz)).yzw;
    // 182: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[6].xyzx)).xyz;
    // 183: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 184: mul r1.xyz, r1.xyzx, cb0[13].wwww
    r1.xyz = ((r1.xyzx)*(source[13].wwww)).xyz;
    // 185: mul r12.xyz, r1.xyzx, r11.xyzx
    r12.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 186: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 187: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 188: mad r2.xyz, cb0[12].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 189: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 190: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 191: mad r2.xyz, cb0[12].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 192: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 194: mul r13.xyz, r13.xyzx, cb0[14].xxxx
    r13.xyz = ((r13.xyzx)*(source[14].xxxx)).xyz;
    // 195: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r14.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 196: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 197: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 198: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 199: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 200: max r13.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r13.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 201: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 202: mul r13.xyz, r13.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 203: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 204: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 205: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 206: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 207: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 208: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 209: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 210: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 211: div r1.w, cb0[15].y, r1.w
    r1.w = ((source[15].yyyy)/(r1.wwww)).w;
    // 212: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 213: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 214: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 215: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 216: mad r1.xyz, r2.xyzx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 217: mad r1.xyz, r1.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 218: mad r1.xyz, r4.xxxx, r1.xyzx, -r11.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r11.xyzx))).xyz;
    // 219: mad r1.xyz, r0.zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 220: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 221: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 222: mad r1.xyz, cb0[12].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 223: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 224: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 225: mad r1.xyz, cb0[12].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 226: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 227: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 228: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 229: mul r4.z, r4.z, cb0[16].y
    r4.z = ((r4.zzzz)*(source[16].yyyy)).z;
    // 230: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 231: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 232: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 233: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 234: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 235: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 236: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 237: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 238: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 239: mul r9.y, cb0[2].y, cb0[8].y
    r9.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 240: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 241: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 242: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 243: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 244: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 245: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 246: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 247: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 248: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 249: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 250: mul r1.w, cb0[9].y, cb0[16].y
    r1.w = ((source[9].yyyy)*(source[16].yyyy)).w;
    // 251: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 252: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 253: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 254: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 255: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 256: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 257: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 258: mul r3.z, cb0[9].x, l(0.001000)
    r3.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 259: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 260: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 261: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 262: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 263: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 264: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 265: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 266: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 267: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 268: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 269: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 270: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 271: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 272: mul r9.xyz, r3.xyzx, cb0[9].zzzz
    r9.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 273: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 274: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 275: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 276: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 277: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 278: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 279: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 280: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 281: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 282: mul r0.y, r2.w, cb0[18].z
    r0.y = ((r2.wwww)*(source[18].zzzz)).y;
    // 283: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 284: add r0.w, -cb0[18].w, l(2.000000)
    r0.w = ((-(source[18].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 285: mad r0.w, r4.x, r0.w, cb0[18].w
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[18].wwww)).w;
    // 286: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 287: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 288: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 289: mul r0.xyz, r0.xyzx, cb0[19].xxxx
    r0.xyz = ((r0.xyzx)*(source[19].xxxx)).xyz;
    // 290: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 291: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 292: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 293: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 294: mul r0.xyz, r0.xyzx, cb0[19].yyyy
    r0.xyz = ((r0.xyzx)*(source[19].yyyy)).xyz;
    // 295: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 296: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 297: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 298: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 299: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 300: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 301: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 302: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 303: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 304: ret
    return output;
}



// source.character.monster-862fa1000fe2.v1 / source program a038f08b5d590b44b40a34955f2d7a99
