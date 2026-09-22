SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight176(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[25].z=(g_SourceCharacterTime.xxxx).x;
    source[30]=float4(input.lightColor,1.0);
    source[31].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[31].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[31].xxxx)) * 0xffffffffu)).w;
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
    // 31: mul r9.xyzw, r8.xyzw, cb0[16].xyzw
    r9.xyzw = ((r8.xyzw)*(source[16].xyzw)).xyzw;
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
    // 50: mul r10.xy, r9.xyxx, cb0[18].xxxx
    r10.xy = ((r9.xyxx)*(source[18].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mad r9.xy, cb0[18].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[18].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 53: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 54: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 55: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r12.xyz, cb0[20].wwww, r10.xyzx, r9.xyzx
    r12.xyz = ((source[20].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
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
    // 75: add r0.z, -cb0[21].y, cb0[21].x
    r0.z = ((-(source[21].yyyy))+(source[21].xxxx)).z;
    // 76: mad r0.z, r11.x, r0.z, cb0[21].y
    r0.z = ((r11.xxxx)*(r0.zzzz)+(source[21].yyyy)).z;
    // 77: add r1.x, -r0.z, cb0[21].z
    r1.x = ((-(r0.zzzz))+(source[21].zzzz)).x;
    // 78: mad r0.z, r11.y, r1.x, r0.z
    r0.z = ((r11.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 79: add r1.x, -r0.z, cb0[21].w
    r1.x = ((-(r0.zzzz))+(source[21].wwww)).x;
    // 80: mad r0.z, r11.z, r1.x, r0.z
    r0.z = ((r11.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 82: add r1.x, -r2.w, l(1.000000)
    r1.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: add r1.y, -cb0[19].y, cb0[19].x
    r1.y = ((-(source[19].yyyy))+(source[19].xxxx)).y;
    // 84: mad r1.y, r11.x, r1.y, cb0[19].y
    r1.y = ((r11.xxxx)*(r1.yyyy)+(source[19].yyyy)).y;
    // 85: add r1.w, -r1.y, cb0[19].z
    r1.w = ((-(r1.yyyy))+(source[19].zzzz)).w;
    // 86: mad r1.y, r11.y, r1.w, r1.y
    r1.y = ((r11.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 87: add r1.w, -r1.y, cb0[19].w
    r1.w = ((-(r1.yyyy))+(source[19].wwww)).w;
    // 88: mad r1.y, r11.z, r1.w, r1.y
    r1.y = ((r11.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 89: add r1.w, -r1.y, cb0[20].x
    r1.w = ((-(r1.yyyy))+(source[20].xxxx)).w;
    // 90: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 91: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 92: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 93: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 94: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 95: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 96: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 97: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 98: add r1.w, -r1.y, cb0[22].x
    r1.w = ((-(r1.yyyy))+(source[22].xxxx)).w;
    // 99: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 100: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 101: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 102: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 103: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 104: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 105: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 106: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 107: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 108: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 109: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 110: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 111: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 112: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 113: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 114: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 115: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 116: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 117: rcp r1.y, cb0[22].y
    r1.y = (1.0/(source[22].yyyy)).y;
    // 118: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 119: mul r12.xyz, r4.xyzx, cb0[22].yyyy
    r12.xyz = ((r4.xyzx)*(source[22].yyyy)).xyz;
    // 120: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 121: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 122: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 123: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 124: mad r4.xyz, r12.xyzx, cb0[22].yyyy, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[22].yyyy)+(r4.xyzx)).xyz;
    // 125: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 126: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 127: add r1.y, cb0[22].y, l(1.000000)
    r1.y = ((source[22].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 128: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 129: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 130: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 131: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 132: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 133: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 134: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 135: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 136: mul r2.w, |r1.y|, |r1.y|
    r2.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 137: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 138: mul r1.y, |r1.y|, r2.w
    r1.y = ((abs(r1.yyyy))*(r2.wwww)).y;
    // 139: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 140: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 141: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 142: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 143: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 144: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 145: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 147: mul r12.xyz, r0.xyzx, r1.yyyy
    r12.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 148: mul r13.xyz, r12.xyzx, cb0[26].yyyy
    r13.xyz = ((r12.xyzx)*(source[26].yyyy)).xyz;
    // 149: mul r2.w, r11.w, l(0.500000)
    r2.w = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 150: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 152: mul r4.w, r1.x, r4.w
    r4.w = ((r1.xxxx)*(r4.wwww)).w;
    // 153: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 154: mad r9.xyz, r2.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 155: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 156: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 157: div r9.xyz, r9.xyzx, r2.wwww
    r9.xyz = ((r9.xyzx)/(r2.wwww)).xyz;
    // 158: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 159: max r4.w, r2.w, l(0.000000)
    r4.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 160: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: dp3 r6.w, cb0[17].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[17].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r7.xyz, r6.wwww, -cb0[17].xyzx
    r7.xyz = ((r6.wwww)+(-(source[17].xyzx))).xyz;
    // 164: mad r7.xyz, r5.wwww, r7.xyzx, cb0[17].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[17].xyzx)).xyz;
    // 165: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 166: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mul r7.xyz, r1.xxxx, r7.xyzx
    r7.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // 168: mad r2.w, r2.w, l(0.500000), -r5.w
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).w;
    // 169: mad r7.xyz, r7.xyzx, r2.wwww, r5.wwww
    r7.xyz = ((r7.xyzx)*(r2.wwww)+(r5.wwww)).xyz;
    // 170: add_sat r2.w, r11.w, cb0[26].z
    r2.w = (saturate((r11.wwww)+(source[26].zzzz))).w;
    // 171: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 173: mul_sat r6.xy, r6.xzxx, cb0[23].xxxx
    r6.xy = (saturate((r6.xzxx)*(source[23].xxxx))).xy;
    // 174: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 175: add_sat r6.y, r6.y, -cb0[23].y
    r6.y = (saturate((r6.yyyy)+(-(source[23].yyyy)))).y;
    // 176: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 177: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 178: mul r6.y, r6.y, cb0[23].z
    r6.y = ((r6.yyyy)*(source[23].zzzz)).y;
    // 179: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 180: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 181: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 182: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 183: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 184: mad r7.w, r0.w, l(2.000000), -r1.y
    r7.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).w;
    // 185: mad r6.yzw, r6.yyzw, r7.wwww, r1.yyyy
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r1.yyyy)).yzw;
    // 186: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 187: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 188: mul r7.w, r1.x, r1.x
    r7.w = ((r1.xxxx)*(r1.xxxx)).w;
    // 189: mul r8.w, r7.w, cb0[26].w
    r8.w = ((r7.wwww)*(source[26].wwww)).w;
    // 190: mad r1.x, -r7.w, cb0[26].w, r1.x
    r1.x = ((-(r7.wwww))*(source[26].wwww)+(r1.xxxx)).x;
    // 191: mad r1.x, r11.w, r1.x, r8.w
    r1.x = ((r11.wwww)*(r1.xxxx)+(r8.wwww)).x;
    // 192: mad r6.yzw, r2.wwww, r6.yyzw, -r7.xxyz
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 193: mad r6.yzw, r1.xxxx, r6.yyzw, r7.xxyz
    r6.yzw = ((r1.xxxx)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 194: sqrt r1.x, r5.w
    r1.x = (sqrt(r5.wwww)).x;
    // 195: mul r5.xyz, r5.xyzx, r1.xxxx
    r5.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 196: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 197: mad r6.yzw, -cb0[26].yyyy, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[26].yyyy))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 198: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 199: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 200: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 201: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 202: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 203: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 204: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 205: mad r7.xyz, r11.zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 206: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 207: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 208: mad r7.xyz, cb0[20].yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((source[20].yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 209: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 210: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 211: mad r7.xyz, cb0[20].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[20].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 212: mad r10.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 213: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 214: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 215: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 216: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[9].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[9].xyzx)).xyz;
    // 217: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 218: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 219: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 220: mad r8.xyz, cb0[20].yyyy, r12.xyzx, r8.xyzx
    r8.xyz = ((source[20].yyyy)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 221: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 222: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 223: mad r8.xyz, cb0[20].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[20].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 224: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 225: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 227: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 228: add r13.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r13.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 229: mad r13.xyz, r0.yyyy, r13.xyzx, cb0[10].xyzx
    r13.xyz = ((r0.yyyy)*(r13.xyzx)+(source[10].xyzx)).xyz;
    // 230: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 231: mul r0.xyz, r0.xyzx, cb0[22].zzzz
    r0.xyz = ((r0.xyzx)*(source[22].zzzz)).xyz;
    // 232: mul r13.xyz, r0.xyzx, r12.xyzx
    r13.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 233: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 234: add r14.xyz, -r2.xyzx, r1.xxxx
    r14.xyz = ((-(r2.xyzx))+(r1.xxxx)).xyz;
    // 235: mad r2.yzw, cb0[20].yyyy, r14.xxyz, r2.xxyz
    r2.yzw = ((source[20].yyyy)*(r14.xxyz)+(r2.xxyz)).yzw;
    // 236: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 237: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 238: mad r2.yzw, cb0[20].zzzz, r14.xxyz, r2.yyzw
    r2.yzw = ((source[20].zzzz)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 239: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 240: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 241: mul r14.xyz, r14.xyzx, cb0[22].wwww
    r14.xyz = ((r14.xyzx)*(source[22].wwww)).xyz;
    // 242: add r1.x, r11.y, r11.x
    r1.x = ((r11.yyyy)+(r11.xxxx)).x;
    // 243: add r1.x, r11.z, r1.x
    r1.x = ((r11.zzzz)+(r1.xxxx)).x;
    // 244: add_sat r1.x, r11.w, r1.x
    r1.x = (saturate((r11.wwww)+(r1.xxxx))).x;
    // 245: mad r2.yzw, r1.xxxx, r14.xxyz, r2.yyzw
    r2.yzw = ((r1.xxxx)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 246: add r11.xyz, -r2.yzwy, r2.xxxx
    r11.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 247: mad r2.xyz, r11.wwww, r11.xyzx, r2.yzwy
    r2.xyz = ((r11.wwww)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 248: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 249: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 250: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 251: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 252: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 253: add r1.z, cb0[23].w, -cb0[24].x
    r1.z = ((source[23].wwww)+(-(source[24].xxxx))).z;
    // 254: mad r1.z, r11.w, r1.z, cb0[24].x
    r1.z = ((r11.wwww)*(r1.zzzz)+(source[24].xxxx)).z;
    // 255: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 256: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 257: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 258: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 259: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 260: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 261: div r1.z, cb0[24].y, r1.z
    r1.z = ((source[24].yyyy)/(r1.zzzz)).z;
    // 262: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 263: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 264: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 265: mul r1.z, r1.z, cb0[24].z
    r1.z = ((r1.zzzz)*(source[24].zzzz)).z;
    // 266: mad r0.xyz, r2.xyzx, r0.xyzx, -r13.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 267: mad r0.xyz, r1.zzzz, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 268: mad r0.xyz, r1.yyyy, r0.xyzx, -r12.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r12.xyzx))).xyz;
    // 269: mad r0.xyz, r1.xxxx, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 270: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 271: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 272: mad r0.xyz, cb0[20].yyyy, r11.xyzx, r0.xyzx
    r0.xyz = ((source[20].yyyy)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 273: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 274: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 275: mad r0.xyz, cb0[20].zzzz, r11.xyzx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 276: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 277: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 278: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 279: mul r2.w, r2.w, cb0[25].z
    r2.w = ((r2.wwww)*(source[25].zzzz)).w;
    // 280: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 281: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 282: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 283: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 284: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 285: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 286: add r6.x, -r2.w, cb0[2].x
    r6.x = ((-(r2.wwww))+(source[2].xxxx)).x;
    // 287: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 288: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 289: mul r10.y, cb0[2].y, cb0[12].y
    r10.y = ((source[2].yyyy)*(source[12].yyyy)).y;
    // 290: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 291: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 292: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 293: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 294: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 295: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t6.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 296: mul r10.xyz, r1.zzzz, r10.xyzx
    r10.xyz = ((r1.zzzz)*(r10.xyzx)).xyz;
    // 297: mul r1.z, r2.w, r10.w
    r1.z = ((r2.wwww)*(r10.wwww)).z;
    // 298: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 299: mad r0.xyz, r1.zzzz, r10.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 300: mul r1.z, cb0[13].y, cb0[25].z
    r1.z = ((source[13].yyyy)*(source[25].zzzz)).z;
    // 301: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 302: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 303: mul r10.y, r1.z, l(0.020000)
    r10.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 304: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 305: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 306: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 307: mad r3.xy, cb0[13].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[13].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 308: mul r2.w, cb0[13].x, l(0.001000)
    r2.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 309: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 310: mad r3.xy, r2.wwww, r3.xyxx, r10.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r10.xyxx)).xy;
    // 311: dp2 r2.w, cb0[14].xyxx, r3.xyxx
    r2.w = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 312: dp2 r3.y, cb0[15].xyxx, r3.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 313: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 314: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 315: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 316: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 317: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 318: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 319: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 320: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 321: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 322: mul r10.xyz, r3.xyzx, cb0[13].zzzz
    r10.xyz = ((r3.xyzx)*(source[13].zzzz)).xyz;
    // 323: dp3 r1.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 324: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 325: mad r3.xyz, cb0[13].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[13].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 326: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 327: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 328: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 329: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 330: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 331: dp3 r4.x, r4.xyzx, r9.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 332: mul r4.z, r0.w, cb0[27].x
    r4.z = ((r0.wwww)*(source[27].xxxx)).z;
    // 333: mul r0.w, r11.w, r4.z
    r0.w = ((r11.wwww)*(r4.zzzz)).w;
    // 334: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 335: min r0.w, r0.w, cb0[27].x
    r0.w = (min(r0.wwww,source[27].xxxx)).w;
    // 336: add r1.x, -cb0[27].w, cb0[27].z
    r1.x = ((-(source[27].wwww))+(source[27].zzzz)).x;
    // 337: mad r1.x, cb0[27].y, r1.x, cb0[27].w
    r1.x = ((source[27].yyyy)*(r1.xxxx)+(source[27].wwww)).x;
    // 338: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 339: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 340: mad r1.x, r11.w, r1.x, l(1.000000)
    r1.x = ((r11.wwww)*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 341: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 342: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 343: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 344: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 345: movc r4.y, r1.z, l(0), r0.w
    r4.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 346: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t7.xyzw, s8, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 347: add r0.w, -cb0[28].z, cb0[28].y
    r0.w = ((-(source[28].zzzz))+(source[28].yyyy)).w;
    // 348: mad r0.w, cb0[28].x, r0.w, cb0[28].z
    r0.w = ((source[28].xxxx)*(r0.wwww)+(source[28].zzzz)).w;
    // 349: mul r0.w, r0.w, r11.w
    r0.w = ((r0.wwww)*(r11.wwww)).w;
    // 350: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xzxx, t7.xyzw, s8, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 351: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 352: add r0.w, -cb0[28].w, l(2.000000)
    r0.w = ((-(source[28].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 353: mad r0.w, r1.y, r0.w, cb0[28].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[28].wwww)).w;
    // 354: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 355: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 356: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 357: mul r1.xyz, r1.xyzx, cb0[29].xxxx
    r1.xyz = ((r1.xyzx)*(source[29].xxxx)).xyz;
    // 358: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 359: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 360: mul r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)*(r5.wwww)).xyz;
    // 361: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 362: mul r1.xyz, r1.xyzx, cb0[29].yyyy
    r1.xyz = ((r1.xyzx)*(source[29].yyyy)).xyz;
    // 363: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 364: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 365: mad r0.xyz, r6.yzwy, r0.xyzx, r1.xyzx
    r0.xyz = ((r6.yzwy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 366: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 367: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 368: mul o0.xyz, r0.xyzx, cb0[30].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[30].xyzx)).xyz;
    // 369: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 370: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 371: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 372: ret
    return output;
}

// source.character.equipment-native-177.v1 / source program 271de881c844de4a8c1f9d3c566aa553
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight177(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[19]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[21]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[22]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[31].z=(g_SourceCharacterTime.xxxx).x;
    source[36]=float4(input.lightColor,1.0);
    source[37].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[37].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[37].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.wxyz, s5, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 42: mul r10.xy, r9.xyxx, cb0[24].xxxx
    r10.xy = ((r9.xyxx)*(source[24].xxxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mad r9.xy, cb0[24].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[24].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 45: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 46: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 47: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r12.xyz, cb0[26].wwww, r10.xyzx, r9.xyzx
    r12.xyz = ((source[26].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
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
    // 67: add r0.z, -cb0[27].y, cb0[27].x
    r0.z = ((-(source[27].yyyy))+(source[27].xxxx)).z;
    // 68: mad r0.z, r11.x, r0.z, cb0[27].y
    r0.z = ((r11.xxxx)*(r0.zzzz)+(source[27].yyyy)).z;
    // 69: add r1.x, -r0.z, cb0[27].z
    r1.x = ((-(r0.zzzz))+(source[27].zzzz)).x;
    // 70: mad r0.z, r11.y, r1.x, r0.z
    r0.z = ((r11.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 71: add r1.x, -r0.z, cb0[27].w
    r1.x = ((-(r0.zzzz))+(source[27].wwww)).x;
    // 72: mad r0.z, r11.z, r1.x, r0.z
    r0.z = ((r11.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 74: add r1.x, -r2.w, l(1.000000)
    r1.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 75: add r1.y, -cb0[25].y, cb0[25].x
    r1.y = ((-(source[25].yyyy))+(source[25].xxxx)).y;
    // 76: mad r1.y, r11.x, r1.y, cb0[25].y
    r1.y = ((r11.xxxx)*(r1.yyyy)+(source[25].yyyy)).y;
    // 77: add r1.w, -r1.y, cb0[25].z
    r1.w = ((-(r1.yyyy))+(source[25].zzzz)).w;
    // 78: mad r1.y, r11.y, r1.w, r1.y
    r1.y = ((r11.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 79: add r1.w, -r1.y, cb0[25].w
    r1.w = ((-(r1.yyyy))+(source[25].wwww)).w;
    // 80: mad r1.y, r11.z, r1.w, r1.y
    r1.y = ((r11.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 81: add r1.w, -r1.y, cb0[26].x
    r1.w = ((-(r1.yyyy))+(source[26].xxxx)).w;
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
    // 90: add r1.w, -r1.y, cb0[28].x
    r1.w = ((-(r1.yyyy))+(source[28].xxxx)).w;
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
    // 108: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s6, r0.z
    r0.xyz = ((g_SourceCharacterTexture5.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 109: rcp r1.y, cb0[28].y
    r1.y = (1.0/(source[28].yyyy)).y;
    // 110: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 111: mul r12.xyz, r4.xyzx, cb0[28].yyyy
    r12.xyz = ((r4.xyzx)*(source[28].yyyy)).xyz;
    // 112: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 113: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 114: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 115: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 116: mad r4.xyz, r12.xyzx, cb0[28].yyyy, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[28].yyyy)+(r4.xyzx)).xyz;
    // 117: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 119: add r1.y, cb0[28].y, l(1.000000)
    r1.y = ((source[28].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
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
    // 140: mul r13.xyz, r12.xyzx, cb0[32].yyyy
    r13.xyz = ((r12.xyzx)*(source[32].yyyy)).xyz;
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
    // 154: dp3 r6.w, cb0[23].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[23].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r7.xyz, r6.wwww, -cb0[23].xyzx
    r7.xyz = ((r6.wwww)+(-(source[23].xyzx))).xyz;
    // 156: mad r7.xyz, r5.wwww, r7.xyzx, cb0[23].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[23].xyzx)).xyz;
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
    // 162: add_sat r2.w, r11.w, cb0[32].z
    r2.w = (saturate((r11.wwww)+(source[32].zzzz))).w;
    // 163: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 165: mul_sat r6.xy, r6.xzxx, cb0[29].xxxx
    r6.xy = (saturate((r6.xzxx)*(source[29].xxxx))).xy;
    // 166: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 167: add_sat r6.y, r6.y, -cb0[29].y
    r6.y = (saturate((r6.yyyy)+(-(source[29].yyyy)))).y;
    // 168: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 169: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 170: mul r6.y, r6.y, cb0[29].z
    r6.y = ((r6.yyyy)*(source[29].zzzz)).y;
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
    // 181: mul r8.x, r7.w, cb0[32].w
    r8.x = ((r7.wwww)*(source[32].wwww)).x;
    // 182: mad r1.x, -r7.w, cb0[32].w, r1.x
    r1.x = ((-(r7.wwww))*(source[32].wwww)+(r1.xxxx)).x;
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
    // 189: mad r6.yzw, -cb0[32].yyyy, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[32].yyyy))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 190: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 191: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 192: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 193: add r12.xy, v4.zwzz, l(-0.050000, -0.050000, 0.000000, 0.000000)
    r12.xy = ((v4.zwzz)+(float4(-0.050000,-0.050000,0.000000,0.000000))).xy;
    // 194: mul_sat r12.xy, r12.xyxx, l(256.000000, 256.000000, 0.000000, 0.000000)
    r12.xy = (saturate((r12.xyxx)*(float4(256.000000,256.000000,0.000000,0.000000)))).xy;
    // 195: add r12.xy, -r12.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r12.xy = ((-(r12.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 196: mad r1.x, -r12.x, r12.y, l(1.000000)
    r1.x = ((-(r12.xxxx))*(r12.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 197: mul r12.xy, v4.wzww, l(4.000000, 4.000000, 0.000000, 0.000000)
    r12.xy = ((v4.wzww)*(float4(4.000000,4.000000,0.000000,0.000000))).xy;
    // 198: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r12.xyxx, t4.xyzw, s4, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r12.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 199: dp4 r2.w, r12.xyzw, cb0[7].xyzw
    r2.w = (dot((r12.xyzw).xyzw,(source[7].xyzw).xyzw).xxxx).w;
    // 200: mul r2.w, r1.x, r2.w
    r2.w = ((r1.xxxx)*(r2.wwww)).w;
    // 201: mul r2.w, r2.w, cb0[6].x
    r2.w = ((r2.wwww)*(source[6].xxxx)).w;
    // 202: mad r13.xyz, cb0[5].wwww, cb0[5].xyzx, -r10.xyzx
    r13.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r10.xyzx))).xyz;
    // 203: mad r10.xyz, r2.wwww, r13.xyzx, r10.xyzx
    r10.xyz = ((r2.wwww)*(r13.xyzx)+(r10.xyzx)).xyz;
    // 204: mad r10.xyz, -cb0[3].wwww, cb0[3].xyzx, r10.xyzx
    r10.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r10.xyzx)).xyz;
    // 205: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 206: mul r10.xyz, cb0[8].xyzx, cb0[8].wwww
    r10.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 207: dp4 r2.w, r12.xyzw, cb0[10].xyzw
    r2.w = (dot((r12.xyzw).xyzw,(source[10].xyzw).xyzw).xxxx).w;
    // 208: mul r2.w, r1.x, r2.w
    r2.w = ((r1.xxxx)*(r2.wwww)).w;
    // 209: mul r2.w, r2.w, cb0[6].y
    r2.w = ((r2.wwww)*(source[6].yyyy)).w;
    // 210: mad r13.xyz, cb0[9].wwww, cb0[9].xyzx, -r10.xyzx
    r13.xyz = ((source[9].wwww)*(source[9].xyzx)+(-(r10.xyzx))).xyz;
    // 211: mad r10.xyz, r2.wwww, r13.xyzx, r10.xyzx
    r10.xyz = ((r2.wwww)*(r13.xyzx)+(r10.xyzx)).xyz;
    // 212: add r10.xyz, -r7.xyzx, r10.xyzx
    r10.xyz = ((-(r7.xyzx))+(r10.xyzx)).xyz;
    // 213: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 214: mul r10.xyz, cb0[11].xyzx, cb0[11].wwww
    r10.xyz = ((source[11].xyzx)*(source[11].wwww)).xyz;
    // 215: dp4 r2.w, r12.xyzw, cb0[13].xyzw
    r2.w = (dot((r12.xyzw).xyzw,(source[13].xyzw).xyzw).xxxx).w;
    // 216: mul r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)*(r2.wwww)).x;
    // 217: mul r1.x, r1.x, cb0[6].z
    r1.x = ((r1.xxxx)*(source[6].zzzz)).x;
    // 218: mad r12.xyz, cb0[12].wwww, cb0[12].xyzx, -r10.xyzx
    r12.xyz = ((source[12].wwww)*(source[12].xyzx)+(-(r10.xyzx))).xyz;
    // 219: mad r10.xyz, r1.xxxx, r12.xyzx, r10.xyzx
    r10.xyz = ((r1.xxxx)*(r12.xyzx)+(r10.xyzx)).xyz;
    // 220: add r10.xyz, -r7.xyzx, r10.xyzx
    r10.xyz = ((-(r7.xyzx))+(r10.xyzx)).xyz;
    // 221: mad r7.xyz, r11.zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 222: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 223: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 224: mad r7.xyz, cb0[26].yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((source[26].yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 225: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 227: mad r7.xyz, cb0[26].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[26].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 228: mad r10.xyz, cb0[14].wwww, cb0[14].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[14].wwww)*(source[14].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 229: mad r12.xyz, cb0[15].wwww, cb0[15].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[15].wwww)*(source[15].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 230: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 231: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 232: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[16].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[16].xyzx)).xyz;
    // 233: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 234: dp3 r1.x, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 235: add r12.xyz, -r8.yzwy, r1.xxxx
    r12.xyz = ((-(r8.yzwy))+(r1.xxxx)).xyz;
    // 236: mad r8.xyz, cb0[26].yyyy, r12.xyzx, r8.yzwy
    r8.xyz = ((source[26].yyyy)*(r12.xyzx)+(r8.yzwy)).xyz;
    // 237: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 238: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 239: mad r8.xyz, cb0[26].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[26].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 240: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 241: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 242: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 243: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 244: add r13.xyz, -cb0[17].xyzx, cb0[18].xyzx
    r13.xyz = ((-(source[17].xyzx))+(source[18].xyzx)).xyz;
    // 245: mad r13.xyz, r0.yyyy, r13.xyzx, cb0[17].xyzx
    r13.xyz = ((r0.yyyy)*(r13.xyzx)+(source[17].xyzx)).xyz;
    // 246: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 247: mul r0.xyz, r0.xyzx, cb0[28].zzzz
    r0.xyz = ((r0.xyzx)*(source[28].zzzz)).xyz;
    // 248: mul r13.xyz, r0.xyzx, r12.xyzx
    r13.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 249: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 250: add r14.xyz, -r2.xyzx, r1.xxxx
    r14.xyz = ((-(r2.xyzx))+(r1.xxxx)).xyz;
    // 251: mad r2.yzw, cb0[26].yyyy, r14.xxyz, r2.xxyz
    r2.yzw = ((source[26].yyyy)*(r14.xxyz)+(r2.xxyz)).yzw;
    // 252: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 253: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 254: mad r2.yzw, cb0[26].zzzz, r14.xxyz, r2.yyzw
    r2.yzw = ((source[26].zzzz)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 255: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 256: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 257: mul r14.xyz, r14.xyzx, cb0[28].wwww
    r14.xyz = ((r14.xyzx)*(source[28].wwww)).xyz;
    // 258: add r1.x, r11.y, r11.x
    r1.x = ((r11.yyyy)+(r11.xxxx)).x;
    // 259: add r1.x, r11.z, r1.x
    r1.x = ((r11.zzzz)+(r1.xxxx)).x;
    // 260: add_sat r1.x, r11.w, r1.x
    r1.x = (saturate((r11.wwww)+(r1.xxxx))).x;
    // 261: mad r2.yzw, r1.xxxx, r14.xxyz, r2.yyzw
    r2.yzw = ((r1.xxxx)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 262: add r11.xyz, -r2.yzwy, r2.xxxx
    r11.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 263: mad r2.xyz, r11.wwww, r11.xyzx, r2.yzwy
    r2.xyz = ((r11.wwww)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 264: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 265: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 266: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 267: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 268: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 269: add r1.z, cb0[29].w, -cb0[30].x
    r1.z = ((source[29].wwww)+(-(source[30].xxxx))).z;
    // 270: mad r1.z, r11.w, r1.z, cb0[30].x
    r1.z = ((r11.wwww)*(r1.zzzz)+(source[30].xxxx)).z;
    // 271: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 272: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 273: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 274: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 275: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 276: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 277: div r1.z, cb0[30].y, r1.z
    r1.z = ((source[30].yyyy)/(r1.zzzz)).z;
    // 278: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 279: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 280: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 281: mul r1.z, r1.z, cb0[30].z
    r1.z = ((r1.zzzz)*(source[30].zzzz)).z;
    // 282: mad r0.xyz, r2.xyzx, r0.xyzx, -r13.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 283: mad r0.xyz, r1.zzzz, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 284: mad r0.xyz, r1.yyyy, r0.xyzx, -r12.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r12.xyzx))).xyz;
    // 285: mad r0.xyz, r1.xxxx, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 286: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 287: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 288: mad r0.xyz, cb0[26].yyyy, r11.xyzx, r0.xyzx
    r0.xyz = ((source[26].yyyy)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 289: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 290: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 291: mad r0.xyz, cb0[26].zzzz, r11.xyzx, r0.xyzx
    r0.xyz = ((source[26].zzzz)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 292: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 293: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 294: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 295: mul r2.w, r2.w, cb0[31].z
    r2.w = ((r2.wwww)*(source[31].zzzz)).w;
    // 296: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 297: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 298: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 299: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 300: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 301: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 302: add r6.x, -r2.w, cb0[2].x
    r6.x = ((-(r2.wwww))+(source[2].xxxx)).x;
    // 303: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 304: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 305: mul r10.y, cb0[2].y, cb0[19].y
    r10.y = ((source[2].yyyy)*(source[19].yyyy)).y;
    // 306: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 307: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 308: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 309: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 310: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 311: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t6.xyzw, s7, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 312: mul r10.xyz, r1.zzzz, r10.xyzx
    r10.xyz = ((r1.zzzz)*(r10.xyzx)).xyz;
    // 313: mul r1.z, r2.w, r10.w
    r1.z = ((r2.wwww)*(r10.wwww)).z;
    // 314: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 315: mad r0.xyz, r1.zzzz, r10.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 316: mul r1.z, cb0[20].y, cb0[31].z
    r1.z = ((source[20].yyyy)*(source[31].zzzz)).z;
    // 317: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 318: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 319: mul r10.y, r1.z, l(0.020000)
    r10.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 320: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 321: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 322: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 323: mad r3.xy, cb0[20].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[20].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 324: mul r2.w, cb0[20].x, l(0.001000)
    r2.w = ((source[20].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 325: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 326: mad r3.xy, r2.wwww, r3.xyxx, r10.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r10.xyxx)).xy;
    // 327: dp2 r2.w, cb0[21].xyxx, r3.xyxx
    r2.w = (dot((source[21].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 328: dp2 r3.y, cb0[22].xyxx, r3.xyxx
    r3.y = (dot((source[22].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 329: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 330: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 331: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s7, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 332: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 333: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 334: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 335: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 336: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 337: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 338: mul r10.xyz, r3.xyzx, cb0[20].zzzz
    r10.xyz = ((r3.xyzx)*(source[20].zzzz)).xyz;
    // 339: dp3 r1.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 340: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 341: mad r3.xyz, cb0[20].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[20].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 342: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 343: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 344: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 345: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 346: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 347: dp3 r4.x, r4.xyzx, r9.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 348: mul r4.z, r0.w, cb0[33].x
    r4.z = ((r0.wwww)*(source[33].xxxx)).z;
    // 349: mul r0.w, r11.w, r4.z
    r0.w = ((r11.wwww)*(r4.zzzz)).w;
    // 350: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 351: min r0.w, r0.w, cb0[33].x
    r0.w = (min(r0.wwww,source[33].xxxx)).w;
    // 352: add r1.x, -cb0[33].w, cb0[33].z
    r1.x = ((-(source[33].wwww))+(source[33].zzzz)).x;
    // 353: mad r1.x, cb0[33].y, r1.x, cb0[33].w
    r1.x = ((source[33].yyyy)*(r1.xxxx)+(source[33].wwww)).x;
    // 354: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 355: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 356: mad r1.x, r11.w, r1.x, l(1.000000)
    r1.x = ((r11.wwww)*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 357: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 358: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 359: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 360: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 361: movc r4.y, r1.z, l(0), r0.w
    r4.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 362: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t7.xyzw, s8, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 363: add r0.w, -cb0[34].z, cb0[34].y
    r0.w = ((-(source[34].zzzz))+(source[34].yyyy)).w;
    // 364: mad r0.w, cb0[34].x, r0.w, cb0[34].z
    r0.w = ((source[34].xxxx)*(r0.wwww)+(source[34].zzzz)).w;
    // 365: mul r0.w, r0.w, r11.w
    r0.w = ((r0.wwww)*(r11.wwww)).w;
    // 366: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xzxx, t7.xyzw, s8, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 367: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 368: add r0.w, -cb0[34].w, l(2.000000)
    r0.w = ((-(source[34].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 369: mad r0.w, r1.y, r0.w, cb0[34].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[34].wwww)).w;
    // 370: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 371: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 372: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 373: mul r1.xyz, r1.xyzx, cb0[35].xxxx
    r1.xyz = ((r1.xyzx)*(source[35].xxxx)).xyz;
    // 374: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 375: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 376: mul r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)*(r5.wwww)).xyz;
    // 377: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 378: mul r1.xyz, r1.xyzx, cb0[35].yyyy
    r1.xyz = ((r1.xyzx)*(source[35].yyyy)).xyz;
    // 379: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 380: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 381: mad r0.xyz, r6.yzwy, r0.xyzx, r1.xyzx
    r0.xyz = ((r6.yzwy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 382: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 383: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 384: mul o0.xyz, r0.xyzx, cb0[36].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[36].xyzx)).xyz;
    // 385: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 386: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 387: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 388: ret
    return output;
}

// source.character.equipment-native-178.v1 / source program 640593a9f1cbf2459a5e73069a3ecf0a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight178(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[19]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[21]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[22]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[31].w=(g_SourceCharacterTime.xxxx).x;
    source[36]=float4(input.lightColor,1.0);
    source[37].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[37].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[37].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t9.xyzw, s0
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s8, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r9.xyzw, r8.xyzw, cb0[23].xyzw
    r9.xyzw = ((r8.xyzw)*(source[23].xyzw)).xyzw;
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
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 50: mul r10.xy, r9.xyxx, cb0[25].xxxx
    r10.xy = ((r9.xyxx)*(source[25].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mad r9.xy, cb0[25].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[25].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 53: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 54: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 55: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r12.xyz, cb0[27].wwww, r10.xyzx, r9.xyzx
    r12.xyz = ((source[27].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
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
    // 75: add r0.z, -cb0[28].y, cb0[28].x
    r0.z = ((-(source[28].yyyy))+(source[28].xxxx)).z;
    // 76: mad r0.z, r11.x, r0.z, cb0[28].y
    r0.z = ((r11.xxxx)*(r0.zzzz)+(source[28].yyyy)).z;
    // 77: add r1.x, -r0.z, cb0[28].z
    r1.x = ((-(r0.zzzz))+(source[28].zzzz)).x;
    // 78: mad r0.z, r11.y, r1.x, r0.z
    r0.z = ((r11.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 79: add r1.x, -r0.z, cb0[28].w
    r1.x = ((-(r0.zzzz))+(source[28].wwww)).x;
    // 80: mad r0.z, r11.z, r1.x, r0.z
    r0.z = ((r11.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 82: add r1.x, -r2.w, l(1.000000)
    r1.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: add r1.y, -cb0[26].y, cb0[26].x
    r1.y = ((-(source[26].yyyy))+(source[26].xxxx)).y;
    // 84: mad r1.y, r11.x, r1.y, cb0[26].y
    r1.y = ((r11.xxxx)*(r1.yyyy)+(source[26].yyyy)).y;
    // 85: add r1.w, -r1.y, cb0[26].z
    r1.w = ((-(r1.yyyy))+(source[26].zzzz)).w;
    // 86: mad r1.y, r11.y, r1.w, r1.y
    r1.y = ((r11.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 87: add r1.w, -r1.y, cb0[26].w
    r1.w = ((-(r1.yyyy))+(source[26].wwww)).w;
    // 88: mad r1.y, r11.z, r1.w, r1.y
    r1.y = ((r11.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 89: add r1.w, -r1.y, cb0[27].x
    r1.w = ((-(r1.yyyy))+(source[27].xxxx)).w;
    // 90: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 91: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 92: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 93: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 94: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 95: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 96: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 97: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 98: add r1.w, -r1.y, cb0[29].x
    r1.w = ((-(r1.yyyy))+(source[29].xxxx)).w;
    // 99: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 100: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 101: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 102: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 103: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 104: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 105: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 106: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 107: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 108: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 109: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 110: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 111: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 112: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 113: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 114: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 115: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 116: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s6, r0.z
    r0.xyz = ((g_SourceCharacterTexture5.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 117: rcp r1.y, cb0[29].y
    r1.y = (1.0/(source[29].yyyy)).y;
    // 118: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 119: mul r12.xyz, r4.xyzx, cb0[29].yyyy
    r12.xyz = ((r4.xyzx)*(source[29].yyyy)).xyz;
    // 120: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 121: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 122: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 123: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 124: mad r4.xyz, r12.xyzx, cb0[29].yyyy, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[29].yyyy)+(r4.xyzx)).xyz;
    // 125: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 126: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 127: add r1.y, cb0[29].y, l(1.000000)
    r1.y = ((source[29].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 128: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 129: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 130: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 131: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 132: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 133: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 134: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 135: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 136: mul r2.w, |r1.y|, |r1.y|
    r2.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 137: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 138: mul r1.y, |r1.y|, r2.w
    r1.y = ((abs(r1.yyyy))*(r2.wwww)).y;
    // 139: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 140: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 141: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 142: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 143: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 144: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 145: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 147: mul r12.xyz, r0.xyzx, r1.yyyy
    r12.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 148: mul r13.xyz, r12.xyzx, cb0[32].yyyy
    r13.xyz = ((r12.xyzx)*(source[32].yyyy)).xyz;
    // 149: mul r2.w, r11.w, l(0.500000)
    r2.w = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 150: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 152: mul r4.w, r1.x, r4.w
    r4.w = ((r1.xxxx)*(r4.wwww)).w;
    // 153: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 154: mad r9.xyz, r2.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 155: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 156: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 157: div r9.xyz, r9.xyzx, r2.wwww
    r9.xyz = ((r9.xyzx)/(r2.wwww)).xyz;
    // 158: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 159: max r4.w, r2.w, l(0.000000)
    r4.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 160: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: dp3 r6.w, cb0[24].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[24].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r7.xyz, r6.wwww, -cb0[24].xyzx
    r7.xyz = ((r6.wwww)+(-(source[24].xyzx))).xyz;
    // 164: mad r7.xyz, r5.wwww, r7.xyzx, cb0[24].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[24].xyzx)).xyz;
    // 165: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 166: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mul r7.xyz, r1.xxxx, r7.xyzx
    r7.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // 168: mad r2.w, r2.w, l(0.500000), -r5.w
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).w;
    // 169: mad r7.xyz, r7.xyzx, r2.wwww, r5.wwww
    r7.xyz = ((r7.xyzx)*(r2.wwww)+(r5.wwww)).xyz;
    // 170: add_sat r2.w, r11.w, cb0[32].z
    r2.w = (saturate((r11.wwww)+(source[32].zzzz))).w;
    // 171: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 173: mul_sat r6.xy, r6.xzxx, cb0[30].xxxx
    r6.xy = (saturate((r6.xzxx)*(source[30].xxxx))).xy;
    // 174: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 175: add_sat r6.y, r6.y, -cb0[30].y
    r6.y = (saturate((r6.yyyy)+(-(source[30].yyyy)))).y;
    // 176: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 177: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 178: mul r6.y, r6.y, cb0[30].z
    r6.y = ((r6.yyyy)*(source[30].zzzz)).y;
    // 179: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 180: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 181: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 182: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 183: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 184: mad r7.w, r0.w, l(2.000000), -r1.y
    r7.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).w;
    // 185: mad r6.yzw, r6.yyzw, r7.wwww, r1.yyyy
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r1.yyyy)).yzw;
    // 186: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 187: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 188: mul r7.w, r1.x, r1.x
    r7.w = ((r1.xxxx)*(r1.xxxx)).w;
    // 189: mul r8.w, r7.w, cb0[32].w
    r8.w = ((r7.wwww)*(source[32].wwww)).w;
    // 190: mad r1.x, -r7.w, cb0[32].w, r1.x
    r1.x = ((-(r7.wwww))*(source[32].wwww)+(r1.xxxx)).x;
    // 191: mad r1.x, r11.w, r1.x, r8.w
    r1.x = ((r11.wwww)*(r1.xxxx)+(r8.wwww)).x;
    // 192: mad r6.yzw, r2.wwww, r6.yyzw, -r7.xxyz
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 193: mad r6.yzw, r1.xxxx, r6.yyzw, r7.xxyz
    r6.yzw = ((r1.xxxx)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 194: sqrt r1.x, r5.w
    r1.x = (sqrt(r5.wwww)).x;
    // 195: mul r5.xyz, r5.xyzx, r1.xxxx
    r5.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 196: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 197: mad r6.yzw, -cb0[32].yyyy, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[32].yyyy))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 198: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 199: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 200: mul r10.xyz, cb0[4].xyzx, cb0[4].wwww
    r10.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 201: add r12.xy, v4.zwzz, l(-0.050000, -0.050000, 0.000000, 0.000000)
    r12.xy = ((v4.zwzz)+(float4(-0.050000,-0.050000,0.000000,0.000000))).xy;
    // 202: mul_sat r12.xy, r12.xyxx, l(256.000000, 256.000000, 0.000000, 0.000000)
    r12.xy = (saturate((r12.xyxx)*(float4(256.000000,256.000000,0.000000,0.000000)))).xy;
    // 203: add r12.xy, -r12.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r12.xy = ((-(r12.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 204: mad r1.x, -r12.x, r12.y, l(1.000000)
    r1.x = ((-(r12.xxxx))*(r12.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r12.xy, v4.wzww, l(4.000000, 4.000000, 0.000000, 0.000000)
    r12.xy = ((v4.wzww)*(float4(4.000000,4.000000,0.000000,0.000000))).xy;
    // 206: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, r12.xyxx, t5.xyzw, s4, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r12.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 207: dp4 r2.w, r12.xyzw, cb0[7].xyzw
    r2.w = (dot((r12.xyzw).xyzw,(source[7].xyzw).xyzw).xxxx).w;
    // 208: mul r2.w, r1.x, r2.w
    r2.w = ((r1.xxxx)*(r2.wwww)).w;
    // 209: mul r2.w, r2.w, cb0[6].x
    r2.w = ((r2.wwww)*(source[6].xxxx)).w;
    // 210: mad r13.xyz, cb0[5].wwww, cb0[5].xyzx, -r10.xyzx
    r13.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r10.xyzx))).xyz;
    // 211: mad r10.xyz, r2.wwww, r13.xyzx, r10.xyzx
    r10.xyz = ((r2.wwww)*(r13.xyzx)+(r10.xyzx)).xyz;
    // 212: mad r10.xyz, -cb0[3].wwww, cb0[3].xyzx, r10.xyzx
    r10.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r10.xyzx)).xyz;
    // 213: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 214: mul r10.xyz, cb0[8].xyzx, cb0[8].wwww
    r10.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 215: dp4 r2.w, r12.xyzw, cb0[10].xyzw
    r2.w = (dot((r12.xyzw).xyzw,(source[10].xyzw).xyzw).xxxx).w;
    // 216: mul r2.w, r1.x, r2.w
    r2.w = ((r1.xxxx)*(r2.wwww)).w;
    // 217: mul r2.w, r2.w, cb0[6].y
    r2.w = ((r2.wwww)*(source[6].yyyy)).w;
    // 218: mad r13.xyz, cb0[9].wwww, cb0[9].xyzx, -r10.xyzx
    r13.xyz = ((source[9].wwww)*(source[9].xyzx)+(-(r10.xyzx))).xyz;
    // 219: mad r10.xyz, r2.wwww, r13.xyzx, r10.xyzx
    r10.xyz = ((r2.wwww)*(r13.xyzx)+(r10.xyzx)).xyz;
    // 220: add r10.xyz, -r7.xyzx, r10.xyzx
    r10.xyz = ((-(r7.xyzx))+(r10.xyzx)).xyz;
    // 221: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 222: mul r10.xyz, cb0[11].xyzx, cb0[11].wwww
    r10.xyz = ((source[11].xyzx)*(source[11].wwww)).xyz;
    // 223: dp4 r2.w, r12.xyzw, cb0[13].xyzw
    r2.w = (dot((r12.xyzw).xyzw,(source[13].xyzw).xyzw).xxxx).w;
    // 224: mul r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)*(r2.wwww)).x;
    // 225: mul r1.x, r1.x, cb0[6].z
    r1.x = ((r1.xxxx)*(source[6].zzzz)).x;
    // 226: mad r12.xyz, cb0[12].wwww, cb0[12].xyzx, -r10.xyzx
    r12.xyz = ((source[12].wwww)*(source[12].xyzx)+(-(r10.xyzx))).xyz;
    // 227: mad r10.xyz, r1.xxxx, r12.xyzx, r10.xyzx
    r10.xyz = ((r1.xxxx)*(r12.xyzx)+(r10.xyzx)).xyz;
    // 228: add r10.xyz, -r7.xyzx, r10.xyzx
    r10.xyz = ((-(r7.xyzx))+(r10.xyzx)).xyz;
    // 229: mad r7.xyz, r11.zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 230: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 231: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 232: mad r7.xyz, cb0[27].yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((source[27].yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 233: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 234: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 235: mad r7.xyz, cb0[27].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[27].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 236: mad r10.xyz, cb0[14].wwww, cb0[14].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[14].wwww)*(source[14].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 237: mad r12.xyz, cb0[15].wwww, cb0[15].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[15].wwww)*(source[15].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 238: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 239: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 240: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[16].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[16].xyzx)).xyz;
    // 241: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 242: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 243: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 244: mad r8.xyz, cb0[27].yyyy, r12.xyzx, r8.xyzx
    r8.xyz = ((source[27].yyyy)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 245: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 246: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 247: mad r8.xyz, cb0[27].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[27].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 248: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 249: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 250: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 251: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 252: add r13.xyz, -cb0[17].xyzx, cb0[18].xyzx
    r13.xyz = ((-(source[17].xyzx))+(source[18].xyzx)).xyz;
    // 253: mad r13.xyz, r0.yyyy, r13.xyzx, cb0[17].xyzx
    r13.xyz = ((r0.yyyy)*(r13.xyzx)+(source[17].xyzx)).xyz;
    // 254: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 255: mul r0.xyz, r0.xyzx, cb0[29].zzzz
    r0.xyz = ((r0.xyzx)*(source[29].zzzz)).xyz;
    // 256: mul r13.xyz, r0.xyzx, r12.xyzx
    r13.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 257: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 258: add r14.xyz, -r2.xyzx, r1.xxxx
    r14.xyz = ((-(r2.xyzx))+(r1.xxxx)).xyz;
    // 259: mad r2.yzw, cb0[27].yyyy, r14.xxyz, r2.xxyz
    r2.yzw = ((source[27].yyyy)*(r14.xxyz)+(r2.xxyz)).yzw;
    // 260: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 261: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 262: mad r2.yzw, cb0[27].zzzz, r14.xxyz, r2.yyzw
    r2.yzw = ((source[27].zzzz)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 263: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 264: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 265: mul r14.xyz, r14.xyzx, cb0[29].wwww
    r14.xyz = ((r14.xyzx)*(source[29].wwww)).xyz;
    // 266: add r1.x, r11.y, r11.x
    r1.x = ((r11.yyyy)+(r11.xxxx)).x;
    // 267: add r1.x, r11.z, r1.x
    r1.x = ((r11.zzzz)+(r1.xxxx)).x;
    // 268: add_sat r1.x, r11.w, r1.x
    r1.x = (saturate((r11.wwww)+(r1.xxxx))).x;
    // 269: mad r2.yzw, r1.xxxx, r14.xxyz, r2.yyzw
    r2.yzw = ((r1.xxxx)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 270: add r11.xyz, -r2.yzwy, r2.xxxx
    r11.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 271: mad r2.xyz, r11.wwww, r11.xyzx, r2.yzwy
    r2.xyz = ((r11.wwww)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 272: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 273: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 274: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 275: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 276: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 277: add r1.z, cb0[30].w, -cb0[31].x
    r1.z = ((source[30].wwww)+(-(source[31].xxxx))).z;
    // 278: mad r1.z, r11.w, r1.z, cb0[31].x
    r1.z = ((r11.wwww)*(r1.zzzz)+(source[31].xxxx)).z;
    // 279: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 280: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 281: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 282: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 283: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 284: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 285: div r1.z, cb0[31].y, r1.z
    r1.z = ((source[31].yyyy)/(r1.zzzz)).z;
    // 286: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 287: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 288: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 289: mul r1.z, r1.z, cb0[31].z
    r1.z = ((r1.zzzz)*(source[31].zzzz)).z;
    // 290: mad r0.xyz, r2.xyzx, r0.xyzx, -r13.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 291: mad r0.xyz, r1.zzzz, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 292: mad r0.xyz, r1.yyyy, r0.xyzx, -r12.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r12.xyzx))).xyz;
    // 293: mad r0.xyz, r1.xxxx, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 294: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 295: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 296: mad r0.xyz, cb0[27].yyyy, r11.xyzx, r0.xyzx
    r0.xyz = ((source[27].yyyy)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 297: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 298: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 299: mad r0.xyz, cb0[27].zzzz, r11.xyzx, r0.xyzx
    r0.xyz = ((source[27].zzzz)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 300: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 301: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 302: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 303: mul r2.w, r2.w, cb0[31].w
    r2.w = ((r2.wwww)*(source[31].wwww)).w;
    // 304: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 305: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 306: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 307: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 308: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 309: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 310: add r6.x, -r2.w, cb0[2].x
    r6.x = ((-(r2.wwww))+(source[2].xxxx)).x;
    // 311: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 312: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 313: mul r10.y, cb0[2].y, cb0[19].y
    r10.y = ((source[2].yyyy)*(source[19].yyyy)).y;
    // 314: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 315: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 316: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 317: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 318: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 319: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t7.xyzw, s7, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 320: mul r10.xyz, r1.zzzz, r10.xyzx
    r10.xyz = ((r1.zzzz)*(r10.xyzx)).xyz;
    // 321: mul r1.z, r2.w, r10.w
    r1.z = ((r2.wwww)*(r10.wwww)).z;
    // 322: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 323: mad r0.xyz, r1.zzzz, r10.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 324: mul r1.z, cb0[20].y, cb0[31].w
    r1.z = ((source[20].yyyy)*(source[31].wwww)).z;
    // 325: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 326: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 327: mul r10.y, r1.z, l(0.020000)
    r10.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 328: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 329: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 330: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 331: mad r3.xy, cb0[20].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[20].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 332: mul r2.w, cb0[20].x, l(0.001000)
    r2.w = ((source[20].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 333: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 334: mad r3.xy, r2.wwww, r3.xyxx, r10.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r10.xyxx)).xy;
    // 335: dp2 r2.w, cb0[21].xyxx, r3.xyxx
    r2.w = (dot((source[21].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 336: dp2 r3.y, cb0[22].xyxx, r3.xyxx
    r3.y = (dot((source[22].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 337: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 338: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 339: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t7.xyzw, s7, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 340: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 341: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 342: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 343: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 344: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 345: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 346: mul r10.xyz, r3.xyzx, cb0[20].zzzz
    r10.xyz = ((r3.xyzx)*(source[20].zzzz)).xyz;
    // 347: dp3 r1.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 348: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 349: mad r3.xyz, cb0[20].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[20].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 350: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 351: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 352: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 353: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 354: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 355: dp3 r4.x, r4.xyzx, r9.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 356: mul r4.z, r0.w, cb0[33].x
    r4.z = ((r0.wwww)*(source[33].xxxx)).z;
    // 357: mul r0.w, r11.w, r4.z
    r0.w = ((r11.wwww)*(r4.zzzz)).w;
    // 358: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 359: min r0.w, r0.w, cb0[33].x
    r0.w = (min(r0.wwww,source[33].xxxx)).w;
    // 360: add r1.x, -cb0[33].w, cb0[33].z
    r1.x = ((-(source[33].wwww))+(source[33].zzzz)).x;
    // 361: mad r1.x, cb0[33].y, r1.x, cb0[33].w
    r1.x = ((source[33].yyyy)*(r1.xxxx)+(source[33].wwww)).x;
    // 362: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 363: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 364: mad r1.x, r11.w, r1.x, l(1.000000)
    r1.x = ((r11.wwww)*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 365: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 366: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 367: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 368: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 369: movc r4.y, r1.z, l(0), r0.w
    r4.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 370: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t8.xyzw, s9, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 371: add r0.w, -cb0[34].z, cb0[34].y
    r0.w = ((-(source[34].zzzz))+(source[34].yyyy)).w;
    // 372: mad r0.w, cb0[34].x, r0.w, cb0[34].z
    r0.w = ((source[34].xxxx)*(r0.wwww)+(source[34].zzzz)).w;
    // 373: mul r0.w, r0.w, r11.w
    r0.w = ((r0.wwww)*(r11.wwww)).w;
    // 374: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xzxx, t8.xyzw, s9, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 375: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 376: add r0.w, -cb0[34].w, l(2.000000)
    r0.w = ((-(source[34].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 377: mad r0.w, r1.y, r0.w, cb0[34].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[34].wwww)).w;
    // 378: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 379: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 380: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 381: mul r1.xyz, r1.xyzx, cb0[35].xxxx
    r1.xyz = ((r1.xyzx)*(source[35].xxxx)).xyz;
    // 382: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 383: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 384: mul r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)*(r5.wwww)).xyz;
    // 385: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 386: mul r1.xyz, r1.xyzx, cb0[35].yyyy
    r1.xyz = ((r1.xyzx)*(source[35].yyyy)).xyz;
    // 387: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 388: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 389: mad r0.xyz, r6.yzwy, r0.xyzx, r1.xyzx
    r0.xyz = ((r6.yzwy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 390: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 391: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 392: mul o0.xyz, r0.xyzx, cb0[36].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[36].xyzx)).xyz;
    // 393: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 394: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 395: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 396: ret
    return output;
}

// source.character.equipment-native-179.v1 / source program 0a8b95aea3d31f49a0e87e1095d11725
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight179(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].w=(g_SourceCharacterTime.xxxx).x;
    source[22]=float4(input.lightColor,1.0);
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add_sat r0.w, r0.w, -cb0[20].x
    r0.w = (saturate((r0.wwww)+(-(source[20].xxxx)))).w;
    // 3: add r0.w, r0.w, l(-0.001000)
    r0.w = ((r0.wwww)+(float4(-0.001000,-0.001000,-0.001000,-0.001000))).w;
    // 4: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 5: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: add r0.w, v4.w, l(0.500000)
    r0.w = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 8: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 9: add r1.x, -r0.w, l(1.000000)
    r1.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 10: mad r1.y, cb0[14].y, l(-3.500000), l(5.000000)
    r1.y = ((source[14].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 11: mul r1.y, r1.y, cb0[15].x
    r1.y = ((r1.yyyy)*(source[15].xxxx)).y;
    // 12: add r1.z, -v4.z, l(1.000000)
    r1.z = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 13: add r1.w, -r1.z, v4.z
    r1.w = ((-(r1.zzzz))+(v4.zzzz)).w;
    // 14: mad r1.z, cb0[15].y, r1.w, r1.z
    r1.z = ((source[15].yyyy)*(r1.wwww)+(r1.zzzz)).z;
    // 15: mul r1.w, r1.z, cb0[15].z
    r1.w = ((r1.zzzz)*(source[15].zzzz)).w;
    // 16: mad r1.z, r1.w, l(0.750000), r1.z
    r1.z = ((r1.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r1.zzzz)).z;
    // 17: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 18: mad_sat r1.z, cb0[16].x, r1.z, r1.z
    r1.z = (saturate((source[16].xxxx)*(r1.zzzz)+(r1.zzzz))).z;
    // 19: mul r1.w, cb0[15].w, l(0.700000)
    r1.w = ((source[15].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 20: add r2.x, -r0.y, l(1.000000)
    r2.x = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r2.yzw, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yzw;
    // 22: mad r3.xy, r2.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r2.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 23: mad r2.y, r3.x, r0.x, l(0.200000)
    r2.y = ((r3.xxxx)*(r0.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 24: add r2.x, -r2.y, r2.x
    r2.x = ((-(r2.yyyy))+(r2.xxxx)).x;
    // 25: mad r2.z, cb0[17].x, r2.x, r2.y
    r2.z = ((source[17].xxxx)*(r2.xxxx)+(r2.yyyy)).z;
    // 26: mad r2.x, cb0[16].z, r2.x, r2.y
    r2.x = ((source[16].zzzz)*(r2.xxxx)+(r2.yyyy)).x;
    // 27: add r2.x, -r1.z, r2.x
    r2.x = ((-(r1.zzzz))+(r2.xxxx)).x;
    // 28: mad r2.x, r1.w, r2.x, r1.z
    r2.x = ((r1.wwww)*(r2.xxxx)+(r1.zzzz)).x;
    // 29: div r2.x, r2.x, cb0[16].y
    r2.x = ((r2.xxxx)/(source[16].yyyy)).x;
    // 30: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mul r2.x, r1.y, r2.x
    r2.x = ((r1.yyyy)*(r2.xxxx)).x;
    // 32: mul r2.x, r2.x, l(4.000000)
    r2.x = ((r2.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 33: mul_sat r0.w, r0.w, r2.x
    r0.w = (saturate((r0.wwww)*(r2.xxxx))).w;
    // 34: add r2.x, -r1.z, r2.z
    r2.x = ((-(r1.zzzz))+(r2.zzzz)).x;
    // 35: mad r1.z, r1.w, r2.x, r1.z
    r1.z = ((r1.wwww)*(r2.xxxx)+(r1.zzzz)).z;
    // 36: div r1.z, r1.z, cb0[16].w
    r1.z = ((r1.zzzz)/(source[16].wwww)).z;
    // 37: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 38: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 39: mul r1.y, r1.y, l(4.000000)
    r1.y = ((r1.yyyy)*(float4(4.000000,4.000000,4.000000,4.000000))).y;
    // 40: mul_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)*(r1.yyyy))).x;
    // 41: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 42: add r0.w, -r0.y, r0.w
    r0.w = ((-(r0.yyyy))+(r0.wwww)).w;
    // 43: mad r0.w, cb0[17].y, r0.w, r0.y
    r0.w = ((source[17].yyyy)*(r0.wwww)+(r0.yyyy)).w;
    // 44: add r1.xyz, -cb0[2].xyzx, cb0[3].xyzx
    r1.xyz = ((-(source[2].xyzx))+(source[3].xyzx)).xyz;
    // 45: mul r1.xyz, r1.xyzx, cb0[14].xxxx
    r1.xyz = ((r1.xyzx)*(source[14].xxxx)).xyz;
    // 46: mad r1.xyz, r0.wwww, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(source[2].xyzx)).xyz;
    // 47: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 48: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 49: mad r1.xyz, cb0[17].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 50: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 52: mad r1.xyz, cb0[17].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[17].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 53: mad r2.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mad r4.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 55: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 56: mul r4.xyz, r1.xyzx, r2.xyzx
    r4.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 57: mad r1.xyz, r1.xyzx, r2.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 58: mad r2.xyz, cb0[7].xyzx, r0.xyzx, -r0.xyzx
    r2.xyz = ((source[7].xyzx)*(r0.xyzx)+(-(r0.xyzx))).xyz;
    // 59: mad r2.xyz, r2.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 60: mad r5.xyz, -r0.xxxx, r4.xyzx, r2.xyzx
    r5.xyz = ((-(r0.xxxx))*(r4.xyzx)+(r2.xyzx)).xyz;
    // 61: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 62: mad r4.xyz, r2.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 63: add r0.w, -cb0[8].w, l(1.000000)
    r0.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: mul r0.w, r0.w, cb0[19].w
    r0.w = ((r0.wwww)*(source[19].wwww)).w;
    // 65: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 66: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 67: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 68: mul r1.w, cb0[8].z, l(1.500000)
    r1.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 69: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 70: mad r0.w, r0.w, l(0.500000), cb0[8].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 71: mul r5.y, cb0[8].y, cb0[9].y
    r5.y = ((source[8].yyyy)*(source[9].yyyy)).y;
    // 72: mul r6.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r6.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 73: frc r1.w, r6.x
    r1.w = (frac(r6.xxxx)).w;
    // 74: mul r6.y, r1.w, l(0.125000)
    r6.y = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 75: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 76: add r5.xy, r5.xyxx, r6.yzyy
    r5.xy = ((r5.xyxx)+(r6.yzyy)).xy;
    // 77: frc r1.w, cb0[8].x
    r1.w = (frac(source[8].xxxx)).w;
    // 78: add r3.w, -r1.w, cb0[8].x
    r3.w = ((-(r1.wwww))+(source[8].xxxx)).w;
    // 79: mul r5.z, r3.w, l(0.125000)
    r5.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 80: add r5.xy, r5.xyxx, r5.zwzz
    r5.xy = ((r5.xyxx)+(r5.zwzz)).xy;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 82: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 83: mul r0.w, r1.w, r5.w
    r0.w = ((r1.wwww)*(r5.wwww)).w;
    // 84: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 85: mad r5.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 86: add r4.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 87: add r6.xyz, v8.xyzx, cb0[0].xyzx
    r6.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 88: add r7.xyzw, r6.yzxy, -cb0[1].yzxy
    r7.xyzw = ((r6.yzxy)+(-(source[1].yzxy))).xyzw;
    // 89: add r6.xyz, -r6.xyzx, cb0[0].xyzx
    r6.xyz = ((-(r6.xyzx))+(source[0].xyzx)).xyz;
    // 90: add r7.xy, -r7.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r7.xy = ((-(r7.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 91: add r7.xy, -r7.zwzz, r7.xyxx
    r7.xy = ((-(r7.zwzz))+(r7.xyxx)).xy;
    // 92: mad r7.xy, cb0[10].wwww, r7.xyxx, r7.zwzz
    r7.xy = ((source[10].wwww)*(r7.xyxx)+(r7.zwzz)).xy;
    // 93: mul r0.w, cb0[10].y, cb0[19].w
    r0.w = ((source[10].yyyy)*(source[19].wwww)).w;
    // 94: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 95: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 96: mul r8.y, r0.w, l(0.020000)
    r8.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 97: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 99: mul r1.w, cb0[10].x, l(0.001000)
    r1.w = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 100: mov r8.x, l(0)
    r8.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 101: mad r7.xy, r1.wwww, r7.xyxx, r8.xyxx
    r7.xy = ((r1.wwww)*(r7.xyxx)+(r8.xyxx)).xy;
    // 102: dp2 r1.w, cb0[11].xyxx, r7.xyxx
    r1.w = (dot((source[11].xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 103: dp2 r7.y, cb0[12].xyxx, r7.xyxx
    r7.y = (dot((source[12].xyxx).xy,(r7.xyxx).xy).xxxx).y;
    // 104: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 105: mul r7.x, r1.w, l(0.125000)
    r7.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 106: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t2.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 107: mad r7.xyz, r7.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r5.xyzx
    r7.xyz = ((r7.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r5.xyzx))).xyz;
    // 108: mul r1.w, r7.w, l(0.900000)
    r1.w = ((r7.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 109: mad r7.xyz, r1.wwww, r7.xyzx, r5.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 110: mul_sat r7.xyz, r0.wwww, r7.xyzx
    r7.xyz = (saturate((r0.wwww)*(r7.xyzx))).xyz;
    // 111: mad r8.xyz, cb0[10].zzzz, r7.xyzx, -r5.xyzx
    r8.xyz = ((source[10].zzzz)*(r7.xyzx)+(-(r5.xyzx))).xyz;
    // 112: mul r7.xyz, r7.xyzx, cb0[10].zzzz
    r7.xyz = ((r7.xyzx)*(source[10].zzzz)).xyz;
    // 113: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 115: mad r5.xyz, r0.wwww, r8.xyzx, r5.xyzx
    r5.xyz = ((r0.wwww)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 116: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 117: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 119: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 120: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 121: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 122: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 123: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 124: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 125: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 126: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 127: dp3 r1.w, r3.xyzx, r7.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 128: add r3.w, r1.w, l(1.000000)
    r3.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 130: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 131: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 132: dp3 r4.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 133: add r8.xyz, r4.wwww, -cb0[2].xyzx
    r8.xyz = ((r4.wwww)+(-(source[2].xyzx))).xyz;
    // 134: mad r8.xyz, cb0[17].zzzz, r8.xyzx, cb0[2].xyzx
    r8.xyz = ((source[17].zzzz)*(r8.xyzx)+(source[2].xyzx)).xyz;
    // 135: dp3 r4.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 136: add r9.xyz, -r8.xyzx, r4.wwww
    r9.xyz = ((-(r8.xyzx))+(r4.wwww)).xyz;
    // 137: mad r8.xyz, cb0[17].wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((source[17].wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 138: mul r9.xyz, r8.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r9.xyz = ((r8.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 139: mul r9.xyz, r3.wwww, r9.xyzx
    r9.xyz = ((r3.wwww)*(r9.xyzx)).xyz;
    // 140: mad r10.xyz, -r8.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r8.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 141: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: mul r11.xyz, r1.wwww, cb2[3].xyzx
    r11.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 143: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mad r1.w, cb0[20].z, r1.w, r3.w
    r1.w = ((source[20].zzzz)*(r1.wwww)+(r3.wwww)).w;
    // 145: mad r9.xyz, r1.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r1.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 146: add r10.xyz, -r8.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r8.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 147: mov_sat r1.w, r7.z
    r1.w = (saturate(r7.zzzz)).w;
    // 148: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 149: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 150: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mad r8.xyz, r1.wwww, r10.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r10.xyzx)+(r8.xyzx)).xyz;
    // 152: mad r8.xyz, r9.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r8.xyzx
    r8.xyz = ((r9.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r8.xyzx)).xyz;
    // 153: mul_sat r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = (saturate((r5.xyzx)*(r8.xyzx))).xyz;
    // 154: mul r5.xyz, r9.xyzx, r5.xyzx
    r5.xyz = ((r9.xyzx)*(r5.xyzx)).xyz;
    // 155: mad r2.xyz, r3.wwww, r2.xyzx, -r5.xyzx
    r2.xyz = ((r3.wwww)*(r2.xyzx)+(-(r5.xyzx))).xyz;
    // 156: mad r2.xyz, r2.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 157: max r1.w, r3.w, l(0.500000)
    r1.w = (max(r3.wwww,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 158: add r3.w, -cb0[19].x, l(0.200000)
    r3.w = ((-(source[19].xxxx))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 159: mad r3.w, r2.w, r3.w, cb0[19].x
    r3.w = ((r2.wwww)*(r3.wwww)+(source[19].xxxx)).w;
    // 160: mad r3.w, r3.w, l(4.500000), l(0.500000)
    r3.w = ((r3.wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 161: mul r3.w, r3.w, cb0[20].w
    r3.w = ((r3.wwww)*(source[20].wwww)).w;
    // 162: mul r3.w, r3.w, l(0.050000)
    r3.w = ((r3.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 163: dp3 r4.w, v1.xyzx, v1.xyzx
    r4.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 164: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 165: mul r5.xyz, r4.wwww, v1.xyzx
    r5.xyz = ((r4.wwww)*(v1.xyzx)).xyz;
    // 166: dp3 r4.w, r5.xyzx, r3.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 167: dp3 r5.x, r5.xyzx, r7.xyzx
    r5.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 168: dp3 r5.y, v7.xyzx, v7.xyzx
    r5.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 169: rsq r5.y, r5.y
    r5.y = (rsqrt(r5.yyyy)).y;
    // 170: mul r5.yzw, r5.yyyy, v7.xxyz
    r5.yzw = ((r5.yyyy)*(v7.xxyz)).yzw;
    // 171: mul r7.xyz, r5.wwww, r6.xyzx
    r7.xyz = ((r5.wwww)*(r6.xyzx)).xyz;
    // 172: mad r6.xyz, r7.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r6.xyzx
    r6.xyz = ((r7.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r6.xyzx)).xyz;
    // 173: dp3 r6.x, r6.xyzx, r6.xyzx
    r6.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 174: sqrt r6.x, r6.x
    r6.x = (sqrt(r6.xxxx)).x;
    // 175: div r6.x, r6.z, r6.x
    r6.x = ((r6.zzzz)/(r6.xxxx)).x;
    // 176: add r6.x, r6.x, cb0[6].z
    r6.x = ((r6.xxxx)+(source[6].zzzz)).x;
    // 177: add r5.x, r5.x, -r6.x
    r5.x = ((r5.xxxx)+(-(r6.xxxx))).x;
    // 178: add r4.w, r4.w, r5.x
    r4.w = ((r4.wwww)+(r5.xxxx)).w;
    // 179: frc r4.w, r4.w
    r4.w = (frac(r4.wwww)).w;
    // 180: add r4.w, r4.w, l(-0.500000)
    r4.w = ((r4.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 181: add r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)+(r4.wwww)).w;
    // 182: add r4.w, -|r4.w|, l(1.000000)
    r4.w = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 183: log r5.x, r4.w
    r5.x = (log2(r4.wwww)).x;
    // 184: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 185: mul r3.w, r3.w, r5.x
    r3.w = ((r3.wwww)*(r5.xxxx)).w;
    // 186: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 187: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 188: movc r1.w, r4.w, l(0), r1.w
    r1.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 189: mad r6.xyz, v5.xyzx, r0.wwww, r5.yzwy
    r6.xyz = ((v5.xyzx)*(r0.wwww)+(r5.yzwy)).xyz;
    // 190: dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 191: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 192: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 194: mul r0.w, r0.w, cb0[21].x
    r0.w = ((r0.wwww)*(source[21].xxxx)).w;
    // 195: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 196: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 197: div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // 198: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 199: mov_sat r1.w, r5.w
    r1.w = (saturate(r5.wwww)).w;
    // 200: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 201: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 202: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 203: mul r1.xyz, r1.xyzx, cb0[18].xxxx
    r1.xyz = ((r1.xyzx)*(source[18].xxxx)).xyz;
    // 204: mad r0.xyz, cb0[18].yyyy, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[18].yyyy)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 205: mad r0.xyz, r2.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 206: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 207: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 208: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 209: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 210: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 211: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 212: div r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 213: mul r1.xyz, r1.xyzx, cb0[13].xyzx
    r1.xyz = ((r1.xyzx)*(source[13].xyzx)).xyz;
    // 214: dp3 r0.w, r3.xyzx, r5.yzwy
    r0.w = (dot((r3.xyzx).xyz,(r5.yzwy).xyz).xxxx).w;
    // 215: add r1.w, -|r5.w|, l(1.000000)
    r1.w = ((-(abs(r5.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 216: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 217: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 218: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 219: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 220: mul r0.w, r0.w, cb0[20].y
    r0.w = ((r0.wwww)*(source[20].yyyy)).w;
    // 221: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 222: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 223: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 224: mad r0.xyz, r0.xyzx, cb2[3].wwww, r11.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r11.xyzx)).xyz;
    // 225: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 226: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 227: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 228: ret
    return output;
}

// source.character.equipment-native-180.v1 / source program cf59938eec3e9641848ec208c041121b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight180(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17].x=(g_SourceCharacterTime.xxxx).x;
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
    // 26: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[23].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[23].xxxx)) * 0xffffffffu)).w;
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
    // 39: mul r1.w, r6.x, cb0[16].z
    r1.w = ((r6.xxxx)*(source[16].zzzz)).w;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 42: add_sat r1.w, r1.w, cb0[16].w
    r1.w = (saturate((r1.wwww)+(source[16].wwww))).w;
    // 43: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul r8.xyz, r2.wwww, cb0[11].xyzx
    r8.xyz = ((r2.wwww)*(source[11].xyzx)).xyz;
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
    // 50: mul r2.w, r6.y, cb0[14].y
    r2.w = ((r6.yyyy)*(source[14].yyyy)).w;
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
    // 64: round_ni r10.xy, v8.xyxx
    r10.xy = (floor(v8.xyxx)).xy;
    // 65: dp2 r3.x, r10.xyxx, l(12.989800, 78.233002, 0.000000, 0.000000)
    r3.x = (dot((r10.xyxx).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).x;
    // 66: sincos r3.x, null, r3.x
    r3.x = (sin(r3.xxxx)).x;
    // 67: mul r3.x, r3.x, l(43758.546875)
    r3.x = ((r3.xxxx)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).x;
    // 68: frc r3.x, r3.x
    r3.x = (frac(r3.xxxx)).x;
    // 69: add r3.x, r3.x, l(-0.500000)
    r3.x = ((r3.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 70: mad r3.x, r3.x, l(0.010000), r9.x
    r3.x = ((r3.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r9.xxxx)).x;
    // 71: lt r4.w, cb0[14].w, r3.x
    r4.w = (asfloat((uint4)((source[14].wwww)<(r3.xxxx)) * 0xffffffffu)).w;
    // 72: and r5.w, r4.w, l(0x3f800000)
    r5.w = (asfloat(asuint(r4.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 73: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 74: mad r6.xyw, r5.wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((r5.wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 75: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 76: max r7.xyw, r7.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r7.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 77: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 78: max r10.xyz, r7.xywx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 79: add r10.xyz, -r7.xywx, r10.xyzx
    r10.xyz = ((-(r7.xywx))+(r10.xyzx)).xyz;
    // 80: mad r7.xyw, r2.wwww, r10.xyxz, r7.xyxw
    r7.xyw = ((r2.wwww)*(r10.xyxz)+(r7.xyxw)).xyw;
    // 81: lt r3.x, r3.x, cb0[14].z
    r3.x = (asfloat((uint4)((r3.xxxx)<(source[14].zzzz)) * 0xffffffffu)).x;
    // 82: movc r4.w, r4.w, l(0), l(1.000000)
    r4.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: movc r3.x, r3.x, l(-1.000000), l(-0.000000)
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).x;
    // 84: add r3.x, r3.x, r4.w
    r3.x = ((r3.xxxx)+(r4.wwww)).x;
    // 85: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 86: mad r6.xyw, r3.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r3.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 87: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 88: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 89: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 90: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 91: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 92: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 93: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 94: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 95: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 96: mul r7.xyw, cb0[7].xyxz, cb0[7].wwww
    r7.xyw = ((source[7].xyxz)*(source[7].wwww)).xyw;
    // 97: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 98: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 99: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 100: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 101: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 102: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 103: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 104: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 105: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 107: mad r6.xyw, cb0[15].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 108: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 110: mad r6.xyw, cb0[15].zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 111: mad r7.xyw, cb0[9].wwww, cb0[9].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[9].wwww)*(source[9].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 112: mad r10.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 113: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 114: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 115: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 116: add r10.xyz, -r3.yzwy, r4.wwww
    r10.xyz = ((-(r3.yzwy))+(r4.wwww)).xyz;
    // 117: mad r3.yzw, cb0[15].yyyy, r10.xxyz, r3.yyzw
    r3.yzw = ((source[15].yyyy)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 118: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r10.xyz, -r3.yzwy, r4.wwww
    r10.xyz = ((-(r3.yzwy))+(r4.wwww)).xyz;
    // 120: mad r3.yzw, cb0[15].zzzz, r10.xxyz, r3.yyzw
    r3.yzw = ((source[15].zzzz)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 121: mul r10.xyz, r3.yzwy, r6.xywx
    r10.xyz = ((r3.yzwy)*(r6.xywx)).xyz;
    // 122: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 123: mad r3.yzw, -r6.xxyw, r3.yyzw, r4.wwww
    r3.yzw = ((-(r6.xxyw))*(r3.yyzw)+(r4.wwww)).yzw;
    // 124: mad r3.yzw, cb0[15].yyyy, r3.yyzw, r10.xxyz
    r3.yzw = ((source[15].yyyy)*(r3.yyzw)+(r10.xxyz)).yzw;
    // 125: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r6.xyw, -r3.yzyw, r4.wwww
    r6.xyw = ((-(r3.yzyw))+(r4.wwww)).xyw;
    // 127: mad r3.yzw, cb0[15].zzzz, r6.xxyw, r3.yyzw
    r3.yzw = ((source[15].zzzz)*(r6.xxyw)+(r3.yyzw)).yzw;
    // 128: mul r3.yzw, r7.xxyw, r3.yyzw
    r3.yzw = ((r7.xxyw)*(r3.yyzw)).yzw;
    // 129: mul r4.w, cb0[8].z, l(1.500000)
    r4.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 130: add r5.w, -cb0[8].w, l(1.000000)
    r5.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r5.w, r5.w, cb0[17].x
    r5.w = ((r5.wwww)*(source[17].xxxx)).w;
    // 132: mul r5.w, r5.w, l(6.283185)
    r5.w = ((r5.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 133: sincos r5.w, null, r5.w
    r5.w = (sin(r5.wwww)).w;
    // 134: add r5.w, r5.w, l(1.000000)
    r5.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 136: mad r4.w, r4.w, l(0.500000), cb0[8].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 137: frc r5.w, cb0[8].x
    r5.w = (frac(source[8].xxxx)).w;
    // 138: add r6.x, -r5.w, cb0[8].x
    r6.x = ((-(r5.wwww))+(source[8].xxxx)).x;
    // 139: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 140: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 141: mul r10.y, cb0[8].y, cb0[12].y
    r10.y = ((source[8].yyyy)*(source[12].yyyy)).y;
    // 142: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 143: mul r6.x, r6.x, l(0.125000)
    r6.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 144: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 145: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 146: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 147: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t4.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 148: mul r6.xyw, r4.wwww, r10.xyxz
    r6.xyw = ((r4.wwww)*(r10.xyxz)).xyw;
    // 149: mul r4.w, r5.w, r10.w
    r4.w = ((r5.wwww)*(r10.wwww)).w;
    // 150: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.yzyw
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.yzyw))).xyw;
    // 151: mad r3.yzw, r4.wwww, r6.xxyw, r3.yyzw
    r3.yzw = ((r4.wwww)*(r6.xxyw)+(r3.yyzw)).yzw;
    // 152: add r4.w, r3.z, r3.y
    r4.w = ((r3.zzzz)+(r3.yyyy)).w;
    // 153: add r4.w, r3.w, r4.w
    r4.w = ((r3.wwww)+(r4.wwww)).w;
    // 154: mul r4.w, r4.w, l(0.333330)
    r4.w = ((r4.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 155: max r4.w, r4.w, cb0[17].z
    r4.w = (max(r4.wwww,source[17].zzzz)).w;
    // 156: min r4.w, r4.w, cb0[17].y
    r4.w = (min(r4.wwww,source[17].yyyy)).w;
    // 157: add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: mad r4.w, r2.w, r5.w, r4.w
    r4.w = ((r2.wwww)*(r5.wwww)+(r4.wwww)).w;
    // 159: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 160: mad r4.w, cb0[18].x, r4.w, l(1.000000)
    r4.w = ((source[18].xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul r6.xyw, r3.yzyw, r4.wwww
    r6.xyw = ((r3.yzyw)*(r4.wwww)).xyw;
    // 162: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 163: mad r3.yzw, r4.wwww, r3.yyzw, -r6.xxyw
    r3.yzw = ((r4.wwww)*(r3.yyzw)+(-(r6.xxyw))).yzw;
    // 164: mad r3.yzw, r1.wwww, r3.yyzw, r6.xxyw
    r3.yzw = ((r1.wwww)*(r3.yyzw)+(r6.xxyw)).yzw;
    // 165: mul r3.yzw, r5.xxyz, r3.yyzw
    r3.yzw = ((r5.xxyz)*(r3.yyzw)).yzw;
    // 166: mad_sat r3.yzw, r3.yyzw, cb2[3].wwww, cb2[3].xxyz
    r3.yzw = (saturate((r3.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz))).yzw;
    // 167: mov_sat r1.w, cb0[18].y
    r1.w = (saturate(source[18].yyyy)).w;
    // 168: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 169: add r4.w, -cb0[19].y, cb0[19].x
    r4.w = ((-(source[19].yyyy))+(source[19].xxxx)).w;
    // 170: mad r4.w, r9.x, r4.w, cb0[19].y
    r4.w = ((r9.xxxx)*(r4.wwww)+(source[19].yyyy)).w;
    // 171: add r5.x, -r4.w, cb0[19].w
    r5.x = ((-(r4.wwww))+(source[19].wwww)).x;
    // 172: mad r4.w, r9.y, r5.x, r4.w
    r4.w = ((r9.yyyy)*(r5.xxxx)+(r4.wwww)).w;
    // 173: add r5.x, -r4.w, cb0[20].y
    r5.x = ((-(r4.wwww))+(source[20].yyyy)).x;
    // 174: mad r4.w, r9.z, r5.x, r4.w
    r4.w = ((r9.zzzz)*(r5.xxxx)+(r4.wwww)).w;
    // 175: add r5.x, -r4.w, cb0[20].w
    r5.x = ((-(r4.wwww))+(source[20].wwww)).x;
    // 176: mad r3.x, r3.x, r5.x, r4.w
    r3.x = ((r3.xxxx)*(r5.xxxx)+(r4.wwww)).x;
    // 177: mul r3.x, r6.z, r3.x
    r3.x = ((r6.zzzz)*(r3.xxxx)).x;
    // 178: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 179: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 180: movc r3.x, r7.z, l(0), r3.x
    r3.x = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 181: max r3.x, r3.x, cb0[0].x
    r3.x = (max(r3.xxxx,source[0].xxxx)).x;
    // 182: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 183: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 184: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 185: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 186: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 187: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 188: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 189: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 190: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 192: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 193: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 194: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 195: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 196: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 197: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 198: mad r5.xyz, -r3.yzwy, r2.wwww, r3.yzwy
    r5.xyz = ((-(r3.yzwy))*(r2.wwww)+(r3.yzwy)).xyz;
    // 199: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 200: mul r6.y, r3.x, r3.x
    r6.y = ((r3.xxxx)*(r3.xxxx)).y;
    // 201: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 202: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 203: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 204: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 205: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 206: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 207: mad r6.z, -r3.x, r3.x, l(1.000000)
    r6.z = ((-(r3.xxxx))*(r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 208: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 209: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 210: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 211: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 212: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 213: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 214: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 215: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.yyzw
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.yyzw)).yzw;
    // 216: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 217: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 219: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 220: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 221: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 222: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 223: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 224: max r7.xyz, r6.yzwy, r3.xxxx
    r7.xyz = (max(r6.yzwy,r3.xxxx)).xyz;
    // 225: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 226: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 227: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 228: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 230: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 231: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 232: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 233: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 234: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 235: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 236: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 237: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 238: mul_sat r6.xyz, cb0[13].xyzx, cb0[13].wwww
    r6.xyz = (saturate((source[13].xyzx)*(source[13].wwww))).xyz;
    // 239: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 240: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 241: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 242: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 243: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 244: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 245: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 246: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 247: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 248: mul r0.x, r0.x, cb0[21].x
    r0.x = ((r0.xxxx)*(source[21].xxxx)).x;
    // 249: mul r0.xyz, r3.yzwy, r0.xxxx
    r0.xyz = ((r3.yzwy)*(r0.xxxx)).xyz;
    // 250: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 251: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 252: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 253: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 254: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 255: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 256: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 257: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 258: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 259: ret
    return output;
}

// source.character.equipment-native-181.v1 / source program 248f04eb2458f346a5a36b0eb569b328
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight181(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16].x=(g_SourceCharacterTime.xxxx).x;
    source[23]=float4(input.lightColor,1.0);
    source[24].x=1.0;
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
    // 9: mul r3.xy, r2.xyxx, cb0[15].xxxx
    r3.xy = ((r2.xyxx)*(source[15].xxxx)).xy;
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
    // 22: mul r4.xyzw, r3.xyzw, cb0[13].xyzw
    r4.xyzw = ((r3.xyzw)*(source[13].xyzw)).xyzw;
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
    // 34: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[24].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[24].xxxx)) * 0xffffffffu)).w;
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
    // 47: mul r1.w, r6.x, cb0[17].w
    r1.w = ((r6.xxxx)*(source[17].wwww)).w;
    // 48: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 49: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 50: add_sat r1.w, r1.w, cb0[18].x
    r1.w = (saturate((r1.wwww)+(source[18].xxxx))).w;
    // 51: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mul r8.xyz, r2.wwww, cb0[11].xyzx
    r8.xyz = ((r2.wwww)*(source[11].xyzx)).xyz;
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
    // 58: mul r2.w, r6.y, cb0[15].y
    r2.w = ((r6.yyyy)*(source[15].yyyy)).w;
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
    // 72: round_ni r10.xy, v8.xyxx
    r10.xy = (floor(v8.xyxx)).xy;
    // 73: dp2 r3.w, r10.xyxx, l(12.989800, 78.233002, 0.000000, 0.000000)
    r3.w = (dot((r10.xyxx).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).w;
    // 74: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 75: mul r3.w, r3.w, l(43758.546875)
    r3.w = ((r3.wwww)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).w;
    // 76: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 77: add r3.w, r3.w, l(-0.500000)
    r3.w = ((r3.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 78: mad r3.w, r3.w, l(0.010000), r9.x
    r3.w = ((r3.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))+(r9.xxxx)).w;
    // 79: lt r4.w, cb0[15].w, r3.w
    r4.w = (asfloat((uint4)((source[15].wwww)<(r3.wwww)) * 0xffffffffu)).w;
    // 80: and r5.w, r4.w, l(0x3f800000)
    r5.w = (asfloat(asuint(r4.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 81: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 82: mad r6.xyw, r5.wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((r5.wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 83: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 84: max r7.xyw, r7.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r7.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 85: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 86: max r10.xyz, r7.xywx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 87: add r10.xyz, -r7.xywx, r10.xyzx
    r10.xyz = ((-(r7.xywx))+(r10.xyzx)).xyz;
    // 88: mad r7.xyw, r2.wwww, r10.xyxz, r7.xyxw
    r7.xyw = ((r2.wwww)*(r10.xyxz)+(r7.xyxw)).xyw;
    // 89: lt r3.w, r3.w, cb0[15].z
    r3.w = (asfloat((uint4)((r3.wwww)<(source[15].zzzz)) * 0xffffffffu)).w;
    // 90: movc r4.w, r4.w, l(0), l(1.000000)
    r4.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 91: movc r3.w, r3.w, l(-1.000000), l(-0.000000)
    r3.w = ((asuint(r3.wwww) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).w;
    // 92: add r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)+(r4.wwww)).w;
    // 93: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 94: mad r6.xyw, r3.wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((r3.wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 95: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 96: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 97: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 98: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 99: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 100: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 101: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 102: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 103: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 104: mul r7.xyw, cb0[7].xyxz, cb0[7].wwww
    r7.xyw = ((source[7].xyxz)*(source[7].wwww)).xyw;
    // 105: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 106: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 107: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 108: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 109: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 110: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 111: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 112: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 113: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 115: mad r6.xyw, cb0[16].zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 116: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 117: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 118: mad r6.xyw, cb0[16].wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 119: mad r7.xyw, cb0[9].wwww, cb0[9].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[9].wwww)*(source[9].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 120: mad r10.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 121: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 122: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 123: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 124: add r10.xyz, -r3.xyzx, r4.wwww
    r10.xyz = ((-(r3.xyzx))+(r4.wwww)).xyz;
    // 125: mad r3.xyz, cb0[16].zzzz, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].zzzz)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 126: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 127: add r10.xyz, -r3.xyzx, r4.wwww
    r10.xyz = ((-(r3.xyzx))+(r4.wwww)).xyz;
    // 128: mad r3.xyz, cb0[16].wwww, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].wwww)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 129: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 130: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 131: mad r3.xyz, -r6.xywx, r3.xyzx, r4.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r4.wwww)).xyz;
    // 132: mad r3.xyz, cb0[16].zzzz, r3.xyzx, r10.xyzx
    r3.xyz = ((source[16].zzzz)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 133: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 134: add r6.xyw, -r3.xyxz, r4.wwww
    r6.xyw = ((-(r3.xyxz))+(r4.wwww)).xyw;
    // 135: mad r3.xyz, cb0[16].wwww, r6.xywx, r3.xyzx
    r3.xyz = ((source[16].wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 136: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 137: mul r4.w, cb0[8].z, l(1.500000)
    r4.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 138: add r5.w, -cb0[8].w, l(1.000000)
    r5.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r5.w, r5.w, cb0[16].x
    r5.w = ((r5.wwww)*(source[16].xxxx)).w;
    // 140: mul r5.w, r5.w, l(6.283185)
    r5.w = ((r5.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 141: sincos r5.w, null, r5.w
    r5.w = (sin(r5.wwww)).w;
    // 142: add r5.w, r5.w, l(1.000000)
    r5.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 144: mad r4.w, r4.w, l(0.500000), cb0[8].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 145: frc r5.w, cb0[8].x
    r5.w = (frac(source[8].xxxx)).w;
    // 146: add r6.x, -r5.w, cb0[8].x
    r6.x = ((-(r5.wwww))+(source[8].xxxx)).x;
    // 147: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 148: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 149: mul r10.y, cb0[8].y, cb0[12].y
    r10.y = ((source[8].yyyy)*(source[12].yyyy)).y;
    // 150: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 151: mul r6.x, r6.x, l(0.125000)
    r6.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 152: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 153: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 154: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 155: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t5.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 156: mul r6.xyw, r4.wwww, r10.xyxz
    r6.xyw = ((r4.wwww)*(r10.xyxz)).xyw;
    // 157: mul r4.w, r5.w, r10.w
    r4.w = ((r5.wwww)*(r10.wwww)).w;
    // 158: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 159: mad r3.xyz, r4.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r4.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 160: add r4.w, r3.y, r3.x
    r4.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 161: add r4.w, r3.z, r4.w
    r4.w = ((r3.zzzz)+(r4.wwww)).w;
    // 162: mul r4.w, r4.w, l(0.333330)
    r4.w = ((r4.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 163: max r4.w, r4.w, cb0[18].z
    r4.w = (max(r4.wwww,source[18].zzzz)).w;
    // 164: min r4.w, r4.w, cb0[18].y
    r4.w = (min(r4.wwww,source[18].yyyy)).w;
    // 165: add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: mad r4.w, r2.w, r5.w, r4.w
    r4.w = ((r2.wwww)*(r5.wwww)+(r4.wwww)).w;
    // 167: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 168: mad r4.w, cb0[19].x, r4.w, l(1.000000)
    r4.w = ((source[19].xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: mul r6.xyw, r3.xyxz, r4.wwww
    r6.xyw = ((r3.xyxz)*(r4.wwww)).xyw;
    // 170: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 171: mad r3.xyz, r4.wwww, r3.xyzx, -r6.xywx
    r3.xyz = ((r4.wwww)*(r3.xyzx)+(-(r6.xywx))).xyz;
    // 172: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 173: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 174: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 175: mov_sat r1.w, cb0[19].y
    r1.w = (saturate(source[19].yyyy)).w;
    // 176: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 177: add r4.w, -cb0[20].y, cb0[20].x
    r4.w = ((-(source[20].yyyy))+(source[20].xxxx)).w;
    // 178: mad r4.w, r9.x, r4.w, cb0[20].y
    r4.w = ((r9.xxxx)*(r4.wwww)+(source[20].yyyy)).w;
    // 179: add r5.x, -r4.w, cb0[20].w
    r5.x = ((-(r4.wwww))+(source[20].wwww)).x;
    // 180: mad r4.w, r9.y, r5.x, r4.w
    r4.w = ((r9.yyyy)*(r5.xxxx)+(r4.wwww)).w;
    // 181: add r5.x, -r4.w, cb0[21].y
    r5.x = ((-(r4.wwww))+(source[21].yyyy)).x;
    // 182: mad r4.w, r9.z, r5.x, r4.w
    r4.w = ((r9.zzzz)*(r5.xxxx)+(r4.wwww)).w;
    // 183: add r5.x, -r4.w, cb0[21].w
    r5.x = ((-(r4.wwww))+(source[21].wwww)).x;
    // 184: mad r3.w, r3.w, r5.x, r4.w
    r3.w = ((r3.wwww)*(r5.xxxx)+(r4.wwww)).w;
    // 185: mul r3.w, r6.z, r3.w
    r3.w = ((r6.zzzz)*(r3.wwww)).w;
    // 186: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 187: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 188: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 189: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 190: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 192: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 193: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 194: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 195: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 196: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 197: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 198: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 199: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 200: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 201: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 202: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 203: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 204: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 205: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 206: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 207: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 208: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 209: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 210: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 211: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 212: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 213: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 214: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 215: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 216: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 217: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 218: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 219: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 220: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 221: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 222: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 223: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 224: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 225: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 226: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 227: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 228: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 229: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 230: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 231: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 232: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 233: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 234: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 235: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 236: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 237: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 238: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 239: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 240: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 241: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 242: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 243: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 244: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 245: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 246: mul_sat r6.xyz, cb0[14].xyzx, cb0[14].wwww
    r6.xyz = (saturate((source[14].xyzx)*(source[14].wwww))).xyz;
    // 247: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 248: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 249: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 250: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 251: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 252: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 253: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 254: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 255: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 256: mul r0.x, r0.x, cb0[22].x
    r0.x = ((r0.xxxx)*(source[22].xxxx)).x;
    // 257: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 258: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 259: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 260: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 261: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 262: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 263: mul o0.xyz, r0.xyzx, cb0[23].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)).xyz;
    // 264: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 265: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 266: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 267: ret
    return output;
}

// source.character.equipment-native-182.v1 / source program d33814d0e1a2034ba899d7e315b64f6e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight182(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].y=(g_SourceCharacterTime.xxxx).x;
    source[17]=float4(input.lightColor,1.0);
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[0].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
    // 1: mad r0.x, cb0[14].w, l(4.500000), l(0.500000)
    r0.x = ((source[14].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 2: mul r0.x, r0.x, cb0[15].w
    r0.x = ((r0.xxxx)*(source[15].wwww)).x;
    // 3: mul r0.x, r0.x, l(0.050000)
    r0.x = ((r0.xxxx)*(float4(0.050000,0.050000,0.050000,0.050000))).x;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 5: mad r1.xy, r0.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.y, r1.xyxx, r1.xyxx
    r0.y = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 7: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 8: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 9: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 10: add r1.z, r0.y, l(0.000010)
    r1.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 12: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 13: div r0.yzw, r1.xxyz, r0.yyyy
    r0.yzw = ((r1.xxyz)/(r0.yyyy)).yzw;
    // 14: dp3 r1.y, v1.xyzx, v1.xyzx
    r1.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 15: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 16: mul r1.yzw, r1.yyyy, v1.xxyz
    r1.yzw = ((r1.yyyy)*(v1.xxyz)).yzw;
    // 17: dp3 r2.x, r1.yzwy, r0.yzwy
    r2.x = (dot((r1.yzwy).xyz,(r0.yzwy).xyz).xxxx).x;
    // 18: dp3 r2.y, v5.xyzx, v5.xyzx
    r2.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 19: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 20: mul r3.xyz, r2.yyyy, v5.zxyz
    r3.xyz = ((r2.yyyy)*(v5.zxyz)).xyz;
    // 21: dp3 r1.y, r1.wyzw, r3.xyzx
    r1.y = (dot((r1.wyzw).xyz,(r3.xyzx).xyz).xxxx).y;
    // 22: add r4.xyz, v8.xyzx, cb0[0].xyzx
    r4.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 23: add r4.xyz, -r4.xyzx, cb0[0].xyzx
    r4.xyz = ((-(r4.xyzx))+(source[0].xyzx)).xyz;
    // 24: dp3 r1.z, v7.xyzx, v7.xyzx
    r1.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 25: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 26: mul r5.xyz, r1.zzzz, v7.xyzx
    r5.xyz = ((r1.zzzz)*(v7.xyzx)).xyz;
    // 27: mul r6.xyz, r4.xyzx, r5.zzzz
    r6.xyz = ((r4.xyzx)*(r5.zzzz)).xyz;
    // 28: mad r4.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r4.xyzx)).xyz;
    // 29: dp3 r1.z, r4.xyzx, r4.xyzx
    r1.z = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 30: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 31: div r1.z, r4.z, r1.z
    r1.z = ((r4.zzzz)/(r1.zzzz)).z;
    // 32: add r1.z, r1.z, cb0[5].z
    r1.z = ((r1.zzzz)+(source[5].zzzz)).z;
    // 33: add r1.y, -r1.z, r1.y
    r1.y = ((-(r1.zzzz))+(r1.yyyy)).y;
    // 34: add r1.y, r1.y, r2.x
    r1.y = ((r1.yyyy)+(r2.xxxx)).y;
    // 35: frc r1.y, r1.y
    r1.y = (frac(r1.yyyy)).y;
    // 36: add r1.y, r1.y, l(-0.500000)
    r1.y = ((r1.yyyy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 37: add r1.y, r1.y, r1.y
    r1.y = ((r1.yyyy)+(r1.yyyy)).y;
    // 38: add r1.y, -|r1.y|, l(1.000000)
    r1.y = ((-(abs(r1.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 39: log r1.z, r1.y
    r1.z = (log2(r1.yyyy)).z;
    // 40: lt r1.y, r1.y, l(0.000001)
    r1.y = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 41: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 42: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 43: dp3 r1.z, r0.wyzw, r3.xyzx
    r1.z = (dot((r0.wyzw).xyz,(r3.xyzx).xyz).xxxx).z;
    // 44: dp3 r0.y, r0.yzwy, r5.xyzx
    r0.y = (dot((r0.yzwy).xyz,(r5.xyzx).xyz).xxxx).y;
    // 45: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 46: mov_sat r3.x, r3.x
    r3.x = (saturate(r3.xxxx)).x;
    // 47: mul r0.z, r3.x, r3.x
    r0.z = ((r3.xxxx)*(r3.xxxx)).z;
    // 48: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 49: max r0.w, r1.z, l(0.000000)
    r0.w = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 50: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 51: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 52: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r2.xzw, r0.wwww, cb2[3].xxyz
    r2.xzw = ((r0.wwww)*(passValues[3].xxyz)).xzw;
    // 54: max r0.w, r1.w, l(0.500000)
    r0.w = (max(r1.wwww,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 55: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 56: movc r0.x, r1.y, l(0), r0.x
    r0.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 57: mad r3.xyz, v5.xyzx, r2.yyyy, r5.xyzx
    r3.xyz = ((v5.xyzx)*(r2.yyyy)+(r5.xyzx)).xyz;
    // 58: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 59: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 60: min r0.zw, r0.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = (min(r0.zzzw,float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 61: div r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)/(r0.wwww)).x;
    // 62: mul r0.x, r0.x, cb0[16].x
    r0.x = ((r0.xxxx)*(source[16].xxxx)).x;
    // 63: add r0.w, -v4.z, l(1.000000)
    r0.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: add r1.y, -r0.w, v4.z
    r1.y = ((-(r0.wwww))+(v4.zzzz)).y;
    // 65: mad r0.w, cb0[11].y, r1.y, r0.w
    r0.w = ((source[11].yyyy)*(r1.yyyy)+(r0.wwww)).w;
    // 66: mul r1.y, r0.w, cb0[11].z
    r1.y = ((r0.wwww)*(source[11].zzzz)).y;
    // 67: mad r0.w, r1.y, l(0.750000), r0.w
    r0.w = ((r1.yyyy)*(float4(0.750000,0.750000,0.750000,0.750000))+(r0.wwww)).w;
    // 68: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 69: mad_sat r0.w, cb0[12].x, r0.w, r0.w
    r0.w = (saturate((source[12].xxxx)*(r0.wwww)+(r0.wwww))).w;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 71: mad r1.x, r1.x, r3.x, l(0.200000)
    r1.x = ((r1.xxxx)*(r3.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 72: add r1.y, -r3.y, l(1.000000)
    r1.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 73: add r1.y, -r1.x, r1.y
    r1.y = ((-(r1.xxxx))+(r1.yyyy)).y;
    // 74: mad r2.y, cb0[13].x, r1.y, r1.x
    r2.y = ((source[13].xxxx)*(r1.yyyy)+(r1.xxxx)).y;
    // 75: mad r1.x, cb0[12].z, r1.y, r1.x
    r1.x = ((source[12].zzzz)*(r1.yyyy)+(r1.xxxx)).x;
    // 76: add r1.x, -r0.w, r1.x
    r1.x = ((-(r0.wwww))+(r1.xxxx)).x;
    // 77: add r1.y, -r0.w, r2.y
    r1.y = ((-(r0.wwww))+(r2.yyyy)).y;
    // 78: mul r2.y, cb0[11].w, l(0.700000)
    r2.y = ((source[11].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).y;
    // 79: mad r1.y, r2.y, r1.y, r0.w
    r1.y = ((r2.yyyy)*(r1.yyyy)+(r0.wwww)).y;
    // 80: mad r0.w, r2.y, r1.x, r0.w
    r0.w = ((r2.yyyy)*(r1.xxxx)+(r0.wwww)).w;
    // 81: div r0.w, r0.w, cb0[12].y
    r0.w = ((r0.wwww)/(source[12].yyyy)).w;
    // 82: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: div r1.x, r1.y, cb0[12].w
    r1.x = ((r1.yyyy)/(source[12].wwww)).x;
    // 84: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 85: mad r1.y, cb0[10].y, l(-3.500000), l(5.000000)
    r1.y = ((source[10].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 86: mul r1.y, r1.y, cb0[11].x
    r1.y = ((r1.yyyy)*(source[11].xxxx)).y;
    // 87: mul r1.xz, r1.xxzx, r1.yyzy
    r1.xz = ((r1.xxzx)*(r1.yyzy)).xz;
    // 88: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 89: mul r0.w, r0.w, l(4.000000)
    r0.w = ((r0.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 90: mul r1.x, r1.x, l(4.000000)
    r1.x = ((r1.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))).x;
    // 91: add r1.y, v4.w, l(0.500000)
    r1.y = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 92: round_ni r1.y, r1.y
    r1.y = (floor(r1.yyyy)).y;
    // 93: add r2.y, -r1.y, l(1.000000)
    r2.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 94: mul_sat r0.w, r0.w, r1.y
    r0.w = (saturate((r0.wwww)*(r1.yyyy))).w;
    // 95: mul_sat r1.x, r1.x, r2.y
    r1.x = (saturate((r1.xxxx)*(r2.yyyy))).x;
    // 96: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 97: add r0.w, -r3.y, r0.w
    r0.w = ((-(r3.yyyy))+(r0.wwww)).w;
    // 98: mad r0.w, cb0[13].y, r0.w, r3.y
    r0.w = ((source[13].yyyy)*(r0.wwww)+(r3.yyyy)).w;
    // 99: add r4.xyz, -cb0[1].xyzx, cb0[2].xyzx
    r4.xyz = ((-(source[1].xyzx))+(source[2].xyzx)).xyz;
    // 100: mul r4.xyz, r4.xyzx, cb0[10].xxxx
    r4.xyz = ((r4.xyzx)*(source[10].xxxx)).xyz;
    // 101: mad r4.xyz, r0.wwww, r4.xyzx, cb0[1].xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // 102: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r5.xyw, -r4.xyxz, r0.wwww
    r5.xyw = ((-(r4.xyxz))+(r0.wwww)).xyw;
    // 104: mad r4.xyz, cb0[13].zzzz, r5.xywx, r4.xyzx
    r4.xyz = ((source[13].zzzz)*(r5.xywx)+(r4.xyzx)).xyz;
    // 105: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r5.xyw, -r4.xyxz, r0.wwww
    r5.xyw = ((-(r4.xyxz))+(r0.wwww)).xyw;
    // 107: mad r4.xyz, cb0[13].wwww, r5.xywx, r4.xyzx
    r4.xyz = ((source[13].wwww)*(r5.xywx)+(r4.xyzx)).xyz;
    // 108: mad r5.xyw, cb0[3].wwww, cb0[3].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r5.xyw = ((source[3].wwww)*(source[3].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 109: mad r6.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 110: mul r5.xyw, r5.xyxw, r6.xyxz
    r5.xyw = ((r5.xyxw)*(r6.xyxz)).xyw;
    // 111: mad r6.xyz, r4.xyzx, r5.xywx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r6.xyz = ((r4.xyzx)*(r5.xywx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 112: mul r4.xyz, r4.xyzx, r5.xywx
    r4.xyz = ((r4.xyzx)*(r5.xywx)).xyz;
    // 113: dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 114: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 115: div r5.xyw, r6.xyxz, r0.wwww
    r5.xyw = ((r6.xyxz)/(r0.wwww)).xyw;
    // 116: mul r5.xyw, r3.zzzz, r5.xyxw
    r5.xyw = ((r3.zzzz)*(r5.xyxw)).xyw;
    // 117: mov_sat r0.w, r5.z
    r0.w = (saturate(r5.zzzz)).w;
    // 118: add r1.x, -|r5.z|, l(1.000000)
    r1.x = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 119: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 120: mul r0.y, r0.y, cb0[9].x
    r0.y = ((r0.yyyy)*(source[9].xxxx)).y;
    // 121: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 122: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 123: mul r5.xyz, r5.xywx, r0.wwww
    r5.xyz = ((r5.xywx)*(r0.wwww)).xyz;
    // 124: mul r5.xyz, r5.xyzx, cb0[14].xxxx
    r5.xyz = ((r5.xyzx)*(source[14].xxxx)).xyz;
    // 125: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 126: max r5.xyz, r5.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r5.xyz = (max(r5.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 127: min r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 128: add r0.x, -r1.w, l(1.000000)
    r0.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 129: mad r0.x, cb0[15].z, r0.x, r1.w
    r0.x = ((source[15].zzzz)*(r0.xxxx)+(r1.wwww)).x;
    // 130: dp3 r0.w, cb0[1].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((source[1].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 131: add r1.xyw, r0.wwww, -cb0[1].xyxz
    r1.xyw = ((r0.wwww)+(-(source[1].xyxz))).xyw;
    // 132: mad r1.xyw, cb0[13].zzzz, r1.xyxw, cb0[1].xyxz
    r1.xyw = ((source[13].zzzz)*(r1.xyxw)+(source[1].xyxz)).xyw;
    // 133: dp3 r0.w, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 134: add r6.xyz, -r1.xywx, r0.wwww
    r6.xyz = ((-(r1.xywx))+(r0.wwww)).xyz;
    // 135: mad r1.xyw, cb0[13].wwww, r6.xyxz, r1.xyxw
    r1.xyw = ((source[13].wwww)*(r6.xyxz)+(r1.xyxw)).xyw;
    // 136: mul r6.xyz, r1.xywx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r6.xyz = ((r1.xywx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 137: mul r6.xyz, r1.zzzz, r6.xyzx
    r6.xyz = ((r1.zzzz)*(r6.xyzx)).xyz;
    // 138: mad r7.xyz, -r1.xywx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xywx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 139: mad r6.xyz, r0.xxxx, r7.xyzx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 140: add r7.xyz, -r1.xywx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xywx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 141: mad r0.xzw, r0.zzzz, r7.xxyz, r1.xxyw
    r0.xzw = ((r0.zzzz)*(r7.xxyz)+(r1.xxyw)).xzw;
    // 142: mad r0.xzw, r6.xxyz, l(0.500000, 0.000000, 0.500000, 0.500000), r0.xxzw
    r0.xzw = ((r6.xxyz)*(float4(0.500000,0.000000,0.500000,0.500000))+(r0.xxzw)).xzw;
    // 143: mul r1.xyz, r3.xxxx, r4.xyzx
    r1.xyz = ((r3.xxxx)*(r4.xyzx)).xyz;
    // 144: mad r3.xyz, r3.xxxx, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r3.xyz = ((r3.xxxx)*(r4.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 145: mul o0.w, r3.w, cb0[0].w
    output.targets[0].w = ((r3.wwww)*(source[0].wwww)).w;
    // 146: add r1.w, -cb0[7].w, l(1.000000)
    r1.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul r1.w, r1.w, cb0[15].y
    r1.w = ((r1.wwww)*(source[15].yyyy)).w;
    // 148: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 149: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 150: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mul r2.y, cb0[7].z, l(1.500000)
    r2.y = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 152: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 153: mad r1.w, r1.w, l(0.500000), cb0[7].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 154: mul r4.y, cb0[7].y, cb0[8].y
    r4.y = ((source[7].yyyy)*(source[8].yyyy)).y;
    // 155: mul r7.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r7.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 156: frc r2.y, r7.x
    r2.y = (frac(r7.xxxx)).y;
    // 157: mul r7.y, r2.y, l(0.125000)
    r7.y = ((r2.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 158: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 159: add r4.xy, r4.xyxx, r7.yzyy
    r4.xy = ((r4.xyxx)+(r7.yzyy)).xy;
    // 160: frc r2.y, cb0[7].x
    r2.y = (frac(source[7].xxxx)).y;
    // 161: add r3.w, -r2.y, cb0[7].x
    r3.w = ((-(r2.yyyy))+(source[7].xxxx)).w;
    // 162: mul r4.z, r3.w, l(0.125000)
    r4.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 163: add r4.xy, r4.xyxx, r4.zwzz
    r4.xy = ((r4.xyxx)+(r4.zwzz)).xy;
    // 164: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 165: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 166: mul r1.w, r2.y, r4.w
    r1.w = ((r2.yyyy)*(r4.wwww)).w;
    // 167: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 168: mad r1.xyz, r1.wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 169: mul_sat r0.xzw, r0.xxzw, r1.xxyz
    r0.xzw = (saturate((r0.xxzw)*(r1.xxyz))).xzw;
    // 170: mad r0.xzw, r0.xxzw, r6.xxyz, r5.xxyz
    r0.xzw = ((r0.xxzw)*(r6.xxyz)+(r5.xxyz)).xzw;
    // 171: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 172: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 173: div r1.xyz, r3.xyzx, r1.xxxx
    r1.xyz = ((r3.xyzx)/(r1.xxxx)).xyz;
    // 174: mul r1.xyz, r1.xyzx, cb0[6].xyzx
    r1.xyz = ((r1.xyzx)*(source[6].xyzx)).xyz;
    // 175: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 176: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 177: mul r1.w, r1.w, cb0[9].y
    r1.w = ((r1.wwww)*(source[9].yyyy)).w;
    // 178: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 179: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 180: mul r1.xyz, r1.xyzx, r0.yyyy
    r1.xyz = ((r1.xyzx)*(r0.yyyy)).xyz;
    // 181: add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 182: mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // 183: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 184: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 185: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 186: mul r1.w, r1.w, cb0[9].w
    r1.w = ((r1.wwww)*(source[9].wwww)).w;
    // 187: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 188: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 189: mul_sat r1.xyz, r1.xyzx, r0.yyyy
    r1.xyz = (saturate((r1.xyzx)*(r0.yyyy))).xyz;
    // 190: add r0.xyz, r0.xzwx, r1.xyzx
    r0.xyz = ((r0.xzwx)+(r1.xyzx)).xyz;
    // 191: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xzwx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xzwx)).xyz;
    // 192: mul o0.xyz, r0.xyzx, cb0[17].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[17].xyzx)).xyz;
    // 193: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 194: ret
    return output;
}

// source.character.equipment-native-183.v1 / source program 864cd607617aac45b162728fbbd0155c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight183(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].z=(g_SourceCharacterTime.xxxx).x;
    source[20]=float4(input.lightColor,1.0);
    source[21].x=1.0;
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
    // 26: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).w;
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
    // 39: mul r1.w, r6.x, cb0[15].x
    r1.w = ((r6.xxxx)*(source[15].xxxx)).w;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 42: add_sat r1.w, r1.w, cb0[15].y
    r1.w = (saturate((r1.wwww)+(source[15].yyyy))).w;
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
    // 86: mad r6.xyw, cb0[13].wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((source[13].wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 87: dp3 r3.x, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 88: add r7.xyw, -r6.xyxw, r3.xxxx
    r7.xyw = ((-(r6.xyxw))+(r3.xxxx)).xyw;
    // 89: mad r6.xyw, cb0[14].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[14].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
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
    // 96: mad r3.xyz, cb0[13].wwww, r10.xyzx, r3.yzwy
    r3.xyz = ((source[13].wwww)*(r10.xyzx)+(r3.yzwy)).xyz;
    // 97: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 99: mad r3.xyz, cb0[14].xxxx, r10.xyzx, r3.xyzx
    r3.xyz = ((source[14].xxxx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 100: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 101: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 103: mad r3.xyz, cb0[13].wwww, r3.xyzx, r10.xyzx
    r3.xyz = ((source[13].wwww)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 104: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 106: mad r3.xyz, cb0[14].xxxx, r6.xywx, r3.xyzx
    r3.xyz = ((source[14].xxxx)*(r6.xywx)+(r3.xyzx)).xyz;
    // 107: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 108: mul r3.w, cb0[7].z, l(1.500000)
    r3.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 109: add r4.w, -cb0[7].w, l(1.000000)
    r4.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: mul r4.w, r4.w, cb0[15].z
    r4.w = ((r4.wwww)*(source[15].zzzz)).w;
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
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 134: max r3.w, r3.w, cb0[16].x
    r3.w = (max(r3.wwww,source[16].xxxx)).w;
    // 135: min r3.w, r3.w, cb0[15].w
    r3.w = (min(r3.wwww,source[15].wwww)).w;
    // 136: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: mad r3.w, r2.w, r4.w, r3.w
    r3.w = ((r2.wwww)*(r4.wwww)+(r3.wwww)).w;
    // 138: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 139: mad r3.w, cb0[16].z, r3.w, l(1.000000)
    r3.w = ((source[16].zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    // 146: mov_sat r1.w, cb0[16].w
    r1.w = (saturate(source[16].wwww)).w;
    // 147: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 148: add r3.w, -cb0[17].w, cb0[17].z
    r3.w = ((-(source[17].wwww))+(source[17].zzzz)).w;
    // 149: mad r3.w, r9.x, r3.w, cb0[17].w
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[17].wwww)).w;
    // 150: add r4.w, -r3.w, cb0[18].y
    r4.w = ((-(r3.wwww))+(source[18].yyyy)).w;
    // 151: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 152: add r4.w, -r3.w, cb0[18].w
    r4.w = ((-(r3.wwww))+(source[18].wwww)).w;
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
    // 225: mul r0.x, r0.x, cb0[19].x
    r0.x = ((r0.xxxx)*(source[19].xxxx)).x;
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
    // 232: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 233: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 234: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 235: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 236: ret
    return output;
}

// source.character.equipment-native-184.v1 / source program 7c39266371aef84893f4bbfe65253a67
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight184(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].x=(g_SourceCharacterTime.xxxx).x;
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
    // 26: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[23].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[23].xxxx)) * 0xffffffffu)).w;
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
    // 39: mul r1.w, r6.x, cb0[17].y
    r1.w = ((r6.xxxx)*(source[17].yyyy)).w;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 42: add_sat r1.w, r1.w, cb0[17].z
    r1.w = (saturate((r1.wwww)+(source[17].zzzz))).w;
    // 43: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul r8.xyz, r2.wwww, cb0[11].xyzx
    r8.xyz = ((r2.wwww)*(source[11].xyzx)).xyz;
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
    // 50: mul r2.w, r6.y, cb0[14].y
    r2.w = ((r6.yyyy)*(source[14].yyyy)).w;
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
    // 64: round_ni r10.xy, v8.xyxx
    r10.xy = (floor(v8.xyxx)).xy;
    // 65: dp2 r3.x, r10.xyxx, l(12.989800, 78.233002, 0.000000, 0.000000)
    r3.x = (dot((r10.xyxx).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).x;
    // 66: sincos r3.x, null, r3.x
    r3.x = (sin(r3.xxxx)).x;
    // 67: mul r3.x, r3.x, l(43758.546875)
    r3.x = ((r3.xxxx)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).x;
    // 68: frc r3.x, r3.x
    r3.x = (frac(r3.xxxx)).x;
    // 69: add r3.x, r3.x, l(-0.500000)
    r3.x = ((r3.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 70: mad r3.x, r3.x, l(0.010000), r9.x
    r3.x = ((r3.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r9.xxxx)).x;
    // 71: lt r4.w, cb0[14].w, r3.x
    r4.w = (asfloat((uint4)((source[14].wwww)<(r3.xxxx)) * 0xffffffffu)).w;
    // 72: and r5.w, r4.w, l(0x3f800000)
    r5.w = (asfloat(asuint(r4.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 73: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 74: mad r6.xyw, r5.wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((r5.wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 75: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 76: max r7.xyw, r7.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r7.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 77: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 78: max r10.xyz, r7.xywx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 79: add r10.xyz, -r7.xywx, r10.xyzx
    r10.xyz = ((-(r7.xywx))+(r10.xyzx)).xyz;
    // 80: mad r7.xyw, r2.wwww, r10.xyxz, r7.xyxw
    r7.xyw = ((r2.wwww)*(r10.xyxz)+(r7.xyxw)).xyw;
    // 81: lt r3.x, r3.x, cb0[14].z
    r3.x = (asfloat((uint4)((r3.xxxx)<(source[14].zzzz)) * 0xffffffffu)).x;
    // 82: movc r4.w, r4.w, l(0), l(1.000000)
    r4.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: movc r3.x, r3.x, l(-1.000000), l(-0.000000)
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).x;
    // 84: add r3.x, r3.x, r4.w
    r3.x = ((r3.xxxx)+(r4.wwww)).x;
    // 85: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 86: mad r6.xyw, r3.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r3.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 87: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 88: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 89: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 90: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 91: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 92: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 93: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 94: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 95: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 96: mul r7.xyw, cb0[7].xyxz, cb0[7].wwww
    r7.xyw = ((source[7].xyxz)*(source[7].wwww)).xyw;
    // 97: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 98: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 99: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 100: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 101: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 102: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 103: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 104: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 105: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 107: mad r6.xyw, cb0[16].wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 108: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 110: mad r6.xyw, cb0[17].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[17].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 111: mad r7.xyw, cb0[9].wwww, cb0[9].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[9].wwww)*(source[9].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 112: mad r10.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 113: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 114: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 115: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 116: add r10.xyz, -r3.yzwy, r4.wwww
    r10.xyz = ((-(r3.yzwy))+(r4.wwww)).xyz;
    // 117: mad r3.yzw, cb0[16].wwww, r10.xxyz, r3.yyzw
    r3.yzw = ((source[16].wwww)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 118: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r10.xyz, -r3.yzwy, r4.wwww
    r10.xyz = ((-(r3.yzwy))+(r4.wwww)).xyz;
    // 120: mad r3.yzw, cb0[17].xxxx, r10.xxyz, r3.yyzw
    r3.yzw = ((source[17].xxxx)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 121: mul r10.xyz, r3.yzwy, r6.xywx
    r10.xyz = ((r3.yzwy)*(r6.xywx)).xyz;
    // 122: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 123: mad r3.yzw, -r6.xxyw, r3.yyzw, r4.wwww
    r3.yzw = ((-(r6.xxyw))*(r3.yyzw)+(r4.wwww)).yzw;
    // 124: mad r3.yzw, cb0[16].wwww, r3.yyzw, r10.xxyz
    r3.yzw = ((source[16].wwww)*(r3.yyzw)+(r10.xxyz)).yzw;
    // 125: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r6.xyw, -r3.yzyw, r4.wwww
    r6.xyw = ((-(r3.yzyw))+(r4.wwww)).xyw;
    // 127: mad r3.yzw, cb0[17].xxxx, r6.xxyw, r3.yyzw
    r3.yzw = ((source[17].xxxx)*(r6.xxyw)+(r3.yyzw)).yzw;
    // 128: mul r3.yzw, r7.xxyw, r3.yyzw
    r3.yzw = ((r7.xxyw)*(r3.yyzw)).yzw;
    // 129: mul r4.w, cb0[8].z, l(1.500000)
    r4.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 130: add r5.w, -cb0[8].w, l(1.000000)
    r5.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r5.w, r5.w, cb0[15].x
    r5.w = ((r5.wwww)*(source[15].xxxx)).w;
    // 132: mul r5.w, r5.w, l(6.283185)
    r5.w = ((r5.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 133: sincos r5.w, null, r5.w
    r5.w = (sin(r5.wwww)).w;
    // 134: add r5.w, r5.w, l(1.000000)
    r5.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 136: mad r4.w, r4.w, l(0.500000), cb0[8].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 137: frc r5.w, cb0[8].x
    r5.w = (frac(source[8].xxxx)).w;
    // 138: add r6.x, -r5.w, cb0[8].x
    r6.x = ((-(r5.wwww))+(source[8].xxxx)).x;
    // 139: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 140: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 141: mul r10.y, cb0[8].y, cb0[12].y
    r10.y = ((source[8].yyyy)*(source[12].yyyy)).y;
    // 142: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 143: mul r6.x, r6.x, l(0.125000)
    r6.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 144: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 145: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 146: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 147: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t4.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 148: mul r6.xyw, r4.wwww, r10.xyxz
    r6.xyw = ((r4.wwww)*(r10.xyxz)).xyw;
    // 149: mul r4.w, r5.w, r10.w
    r4.w = ((r5.wwww)*(r10.wwww)).w;
    // 150: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.yzyw
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.yzyw))).xyw;
    // 151: mad r3.yzw, r4.wwww, r6.xxyw, r3.yyzw
    r3.yzw = ((r4.wwww)*(r6.xxyw)+(r3.yyzw)).yzw;
    // 152: add r4.w, r3.z, r3.y
    r4.w = ((r3.zzzz)+(r3.yyyy)).w;
    // 153: add r4.w, r3.w, r4.w
    r4.w = ((r3.wwww)+(r4.wwww)).w;
    // 154: mul r4.w, r4.w, l(0.333330)
    r4.w = ((r4.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 155: max r4.w, r4.w, cb0[18].x
    r4.w = (max(r4.wwww,source[18].xxxx)).w;
    // 156: min r4.w, r4.w, cb0[17].w
    r4.w = (min(r4.wwww,source[17].wwww)).w;
    // 157: add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: mad r4.w, r2.w, r5.w, r4.w
    r4.w = ((r2.wwww)*(r5.wwww)+(r4.wwww)).w;
    // 159: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 160: mad r4.w, cb0[18].z, r4.w, l(1.000000)
    r4.w = ((source[18].zzzz)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul r6.xyw, r3.yzyw, r4.wwww
    r6.xyw = ((r3.yzyw)*(r4.wwww)).xyw;
    // 162: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 163: mad r3.yzw, r4.wwww, r3.yyzw, -r6.xxyw
    r3.yzw = ((r4.wwww)*(r3.yyzw)+(-(r6.xxyw))).yzw;
    // 164: mad r3.yzw, r1.wwww, r3.yyzw, r6.xxyw
    r3.yzw = ((r1.wwww)*(r3.yyzw)+(r6.xxyw)).yzw;
    // 165: mul r3.yzw, r5.xxyz, r3.yyzw
    r3.yzw = ((r5.xxyz)*(r3.yyzw)).yzw;
    // 166: mad_sat r3.yzw, r3.yyzw, cb2[3].wwww, cb2[3].xxyz
    r3.yzw = (saturate((r3.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz))).yzw;
    // 167: mov_sat r1.w, cb0[18].w
    r1.w = (saturate(source[18].wwww)).w;
    // 168: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 169: add r4.w, -cb0[19].w, cb0[19].z
    r4.w = ((-(source[19].wwww))+(source[19].zzzz)).w;
    // 170: mad r4.w, r9.x, r4.w, cb0[19].w
    r4.w = ((r9.xxxx)*(r4.wwww)+(source[19].wwww)).w;
    // 171: add r5.x, -r4.w, cb0[20].y
    r5.x = ((-(r4.wwww))+(source[20].yyyy)).x;
    // 172: mad r4.w, r9.y, r5.x, r4.w
    r4.w = ((r9.yyyy)*(r5.xxxx)+(r4.wwww)).w;
    // 173: add r5.x, -r4.w, cb0[20].w
    r5.x = ((-(r4.wwww))+(source[20].wwww)).x;
    // 174: mad r4.w, r9.z, r5.x, r4.w
    r4.w = ((r9.zzzz)*(r5.xxxx)+(r4.wwww)).w;
    // 175: add r5.x, -r4.w, cb0[21].y
    r5.x = ((-(r4.wwww))+(source[21].yyyy)).x;
    // 176: mad r3.x, r3.x, r5.x, r4.w
    r3.x = ((r3.xxxx)*(r5.xxxx)+(r4.wwww)).x;
    // 177: mul r3.x, r6.z, r3.x
    r3.x = ((r6.zzzz)*(r3.xxxx)).x;
    // 178: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 179: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 180: movc r3.x, r7.z, l(0), r3.x
    r3.x = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 181: max r3.x, r3.x, cb0[0].x
    r3.x = (max(r3.xxxx,source[0].xxxx)).x;
    // 182: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 183: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 184: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 185: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 186: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 187: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 188: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 189: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 190: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 192: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 193: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 194: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 195: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 196: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 197: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 198: mad r5.xyz, -r3.yzwy, r2.wwww, r3.yzwy
    r5.xyz = ((-(r3.yzwy))*(r2.wwww)+(r3.yzwy)).xyz;
    // 199: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 200: mul r6.y, r3.x, r3.x
    r6.y = ((r3.xxxx)*(r3.xxxx)).y;
    // 201: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 202: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 203: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 204: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 205: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 206: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 207: mad r6.z, -r3.x, r3.x, l(1.000000)
    r6.z = ((-(r3.xxxx))*(r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 208: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 209: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 210: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 211: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 212: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 213: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 214: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 215: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.yyzw
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.yyzw)).yzw;
    // 216: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 217: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 219: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 220: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 221: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 222: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 223: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 224: max r7.xyz, r6.yzwy, r3.xxxx
    r7.xyz = (max(r6.yzwy,r3.xxxx)).xyz;
    // 225: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 226: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 227: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 228: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 230: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 231: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 232: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 233: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 234: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 235: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 236: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 237: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 238: mul_sat r6.xyz, cb0[13].xyzx, cb0[13].wwww
    r6.xyz = (saturate((source[13].xyzx)*(source[13].wwww))).xyz;
    // 239: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 240: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 241: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 242: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 243: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 244: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 245: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 246: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 247: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 248: mul r0.x, r0.x, cb0[21].z
    r0.x = ((r0.xxxx)*(source[21].zzzz)).x;
    // 249: mul r0.xyz, r3.yzwy, r0.xxxx
    r0.xyz = ((r3.yzwy)*(r0.xxxx)).xyz;
    // 250: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 251: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 252: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 253: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 254: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 255: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 256: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 257: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 258: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 259: ret
    return output;
}

// source.character.equipment-native-185.v1 / source program b722163f631cf54da16ee454f7d38252
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight185(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].z=(g_SourceCharacterTime.xxxx).x;
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
    // 39: mul r1.w, r6.x, cb0[16].z
    r1.w = ((r6.xxxx)*(source[16].zzzz)).w;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 42: add_sat r1.w, r1.w, cb0[16].w
    r1.w = (saturate((r1.wwww)+(source[16].wwww))).w;
    // 43: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul r8.xyz, r2.wwww, cb0[11].xyzx
    r8.xyz = ((r2.wwww)*(source[11].xyzx)).xyz;
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
    // 50: mul r2.w, r6.y, cb0[14].y
    r2.w = ((r6.yyyy)*(source[14].yyyy)).w;
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
    // 64: round_ni r10.xy, v8.xyxx
    r10.xy = (floor(v8.xyxx)).xy;
    // 65: dp2 r3.x, r10.xyxx, l(12.989800, 78.233002, 0.000000, 0.000000)
    r3.x = (dot((r10.xyxx).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).x;
    // 66: sincos r3.x, null, r3.x
    r3.x = (sin(r3.xxxx)).x;
    // 67: mul r3.x, r3.x, l(43758.546875)
    r3.x = ((r3.xxxx)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).x;
    // 68: frc r3.x, r3.x
    r3.x = (frac(r3.xxxx)).x;
    // 69: add r3.x, r3.x, l(-0.500000)
    r3.x = ((r3.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 70: mad r3.x, r3.x, l(0.010000), r9.x
    r3.x = ((r3.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r9.xxxx)).x;
    // 71: lt r4.w, cb0[14].w, r3.x
    r4.w = (asfloat((uint4)((source[14].wwww)<(r3.xxxx)) * 0xffffffffu)).w;
    // 72: and r5.w, r4.w, l(0x3f800000)
    r5.w = (asfloat(asuint(r4.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 73: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 74: mad r6.xyw, r5.wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((r5.wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 75: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 76: max r7.xyw, r7.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r7.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 77: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 78: max r10.xyz, r7.xywx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 79: add r10.xyz, -r7.xywx, r10.xyzx
    r10.xyz = ((-(r7.xywx))+(r10.xyzx)).xyz;
    // 80: mad r7.xyw, r2.wwww, r10.xyxz, r7.xyxw
    r7.xyw = ((r2.wwww)*(r10.xyxz)+(r7.xyxw)).xyw;
    // 81: lt r3.x, r3.x, cb0[14].z
    r3.x = (asfloat((uint4)((r3.xxxx)<(source[14].zzzz)) * 0xffffffffu)).x;
    // 82: movc r4.w, r4.w, l(0), l(1.000000)
    r4.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: movc r3.x, r3.x, l(-1.000000), l(-0.000000)
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).x;
    // 84: add r3.x, r3.x, r4.w
    r3.x = ((r3.xxxx)+(r4.wwww)).x;
    // 85: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 86: mad r6.xyw, r3.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r3.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 87: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 88: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 89: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 90: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 91: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 92: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 93: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 94: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 95: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 96: mul r7.xyw, cb0[7].xyxz, cb0[7].wwww
    r7.xyw = ((source[7].xyxz)*(source[7].wwww)).xyw;
    // 97: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 98: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 99: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 100: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 101: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 102: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 103: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 104: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 105: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 107: mad r6.xyw, cb0[16].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 108: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 110: mad r6.xyw, cb0[16].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 111: mad r7.xyw, cb0[9].wwww, cb0[9].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[9].wwww)*(source[9].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 112: mad r10.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 113: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 114: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 115: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 116: add r10.xyz, -r3.yzwy, r4.wwww
    r10.xyz = ((-(r3.yzwy))+(r4.wwww)).xyz;
    // 117: mad r3.yzw, cb0[16].xxxx, r10.xxyz, r3.yyzw
    r3.yzw = ((source[16].xxxx)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 118: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r10.xyz, -r3.yzwy, r4.wwww
    r10.xyz = ((-(r3.yzwy))+(r4.wwww)).xyz;
    // 120: mad r3.yzw, cb0[16].yyyy, r10.xxyz, r3.yyzw
    r3.yzw = ((source[16].yyyy)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 121: mul r10.xyz, r3.yzwy, r6.xywx
    r10.xyz = ((r3.yzwy)*(r6.xywx)).xyz;
    // 122: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 123: mad r3.yzw, -r6.xxyw, r3.yyzw, r4.wwww
    r3.yzw = ((-(r6.xxyw))*(r3.yyzw)+(r4.wwww)).yzw;
    // 124: mad r3.yzw, cb0[16].xxxx, r3.yyzw, r10.xxyz
    r3.yzw = ((source[16].xxxx)*(r3.yyzw)+(r10.xxyz)).yzw;
    // 125: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r6.xyw, -r3.yzyw, r4.wwww
    r6.xyw = ((-(r3.yzyw))+(r4.wwww)).xyw;
    // 127: mad r3.yzw, cb0[16].yyyy, r6.xxyw, r3.yyzw
    r3.yzw = ((source[16].yyyy)*(r6.xxyw)+(r3.yyzw)).yzw;
    // 128: mul r3.yzw, r7.xxyw, r3.yyzw
    r3.yzw = ((r7.xxyw)*(r3.yyzw)).yzw;
    // 129: mul r4.w, cb0[8].z, l(1.500000)
    r4.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 130: add r5.w, -cb0[8].w, l(1.000000)
    r5.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r5.w, r5.w, cb0[15].z
    r5.w = ((r5.wwww)*(source[15].zzzz)).w;
    // 132: mul r5.w, r5.w, l(6.283185)
    r5.w = ((r5.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 133: sincos r5.w, null, r5.w
    r5.w = (sin(r5.wwww)).w;
    // 134: add r5.w, r5.w, l(1.000000)
    r5.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 136: mad r4.w, r4.w, l(0.500000), cb0[8].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 137: frc r5.w, cb0[8].x
    r5.w = (frac(source[8].xxxx)).w;
    // 138: add r6.x, -r5.w, cb0[8].x
    r6.x = ((-(r5.wwww))+(source[8].xxxx)).x;
    // 139: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 140: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 141: mul r10.y, cb0[8].y, cb0[12].y
    r10.y = ((source[8].yyyy)*(source[12].yyyy)).y;
    // 142: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 143: mul r6.x, r6.x, l(0.125000)
    r6.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 144: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 145: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 146: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 147: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t4.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 148: mul r6.xyw, r4.wwww, r10.xyxz
    r6.xyw = ((r4.wwww)*(r10.xyxz)).xyw;
    // 149: mul r4.w, r5.w, r10.w
    r4.w = ((r5.wwww)*(r10.wwww)).w;
    // 150: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.yzyw
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.yzyw))).xyw;
    // 151: mad r3.yzw, r4.wwww, r6.xxyw, r3.yyzw
    r3.yzw = ((r4.wwww)*(r6.xxyw)+(r3.yyzw)).yzw;
    // 152: add r4.w, r3.z, r3.y
    r4.w = ((r3.zzzz)+(r3.yyyy)).w;
    // 153: add r4.w, r3.w, r4.w
    r4.w = ((r3.wwww)+(r4.wwww)).w;
    // 154: mul r4.w, r4.w, l(0.333330)
    r4.w = ((r4.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 155: max r4.w, r4.w, cb0[17].y
    r4.w = (max(r4.wwww,source[17].yyyy)).w;
    // 156: min r4.w, r4.w, cb0[17].x
    r4.w = (min(r4.wwww,source[17].xxxx)).w;
    // 157: add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: mad r4.w, r2.w, r5.w, r4.w
    r4.w = ((r2.wwww)*(r5.wwww)+(r4.wwww)).w;
    // 159: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 160: mad r4.w, cb0[17].w, r4.w, l(1.000000)
    r4.w = ((source[17].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul r6.xyw, r3.yzyw, r4.wwww
    r6.xyw = ((r3.yzyw)*(r4.wwww)).xyw;
    // 162: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 163: mad r3.yzw, r4.wwww, r3.yyzw, -r6.xxyw
    r3.yzw = ((r4.wwww)*(r3.yyzw)+(-(r6.xxyw))).yzw;
    // 164: mad r3.yzw, r1.wwww, r3.yyzw, r6.xxyw
    r3.yzw = ((r1.wwww)*(r3.yyzw)+(r6.xxyw)).yzw;
    // 165: mul r3.yzw, r5.xxyz, r3.yyzw
    r3.yzw = ((r5.xxyz)*(r3.yyzw)).yzw;
    // 166: mad_sat r3.yzw, r3.yyzw, cb2[3].wwww, cb2[3].xxyz
    r3.yzw = (saturate((r3.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz))).yzw;
    // 167: mov_sat r1.w, cb0[18].x
    r1.w = (saturate(source[18].xxxx)).w;
    // 168: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 169: add r4.w, cb0[18].w, -cb0[19].x
    r4.w = ((source[18].wwww)+(-(source[19].xxxx))).w;
    // 170: mad r4.w, r9.x, r4.w, cb0[19].x
    r4.w = ((r9.xxxx)*(r4.wwww)+(source[19].xxxx)).w;
    // 171: add r5.x, -r4.w, cb0[19].z
    r5.x = ((-(r4.wwww))+(source[19].zzzz)).x;
    // 172: mad r4.w, r9.y, r5.x, r4.w
    r4.w = ((r9.yyyy)*(r5.xxxx)+(r4.wwww)).w;
    // 173: add r5.x, -r4.w, cb0[20].x
    r5.x = ((-(r4.wwww))+(source[20].xxxx)).x;
    // 174: mad r4.w, r9.z, r5.x, r4.w
    r4.w = ((r9.zzzz)*(r5.xxxx)+(r4.wwww)).w;
    // 175: add r5.x, -r4.w, cb0[20].z
    r5.x = ((-(r4.wwww))+(source[20].zzzz)).x;
    // 176: mad r3.x, r3.x, r5.x, r4.w
    r3.x = ((r3.xxxx)*(r5.xxxx)+(r4.wwww)).x;
    // 177: mul r3.x, r6.z, r3.x
    r3.x = ((r6.zzzz)*(r3.xxxx)).x;
    // 178: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 179: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 180: movc r3.x, r7.z, l(0), r3.x
    r3.x = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 181: max r3.x, r3.x, cb0[0].x
    r3.x = (max(r3.xxxx,source[0].xxxx)).x;
    // 182: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 183: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 184: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 185: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 186: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 187: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 188: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 189: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 190: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 192: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 193: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 194: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 195: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 196: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 197: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 198: mad r5.xyz, -r3.yzwy, r2.wwww, r3.yzwy
    r5.xyz = ((-(r3.yzwy))*(r2.wwww)+(r3.yzwy)).xyz;
    // 199: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 200: mul r6.y, r3.x, r3.x
    r6.y = ((r3.xxxx)*(r3.xxxx)).y;
    // 201: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 202: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 203: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 204: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 205: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 206: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 207: mad r6.z, -r3.x, r3.x, l(1.000000)
    r6.z = ((-(r3.xxxx))*(r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 208: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 209: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 210: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 211: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 212: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 213: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 214: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 215: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.yyzw
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.yyzw)).yzw;
    // 216: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 217: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 219: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 220: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 221: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 222: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 223: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 224: max r7.xyz, r6.yzwy, r3.xxxx
    r7.xyz = (max(r6.yzwy,r3.xxxx)).xyz;
    // 225: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 226: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 227: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 228: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 230: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 231: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 232: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 233: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 234: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 235: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 236: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 237: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 238: mul_sat r6.xyz, cb0[13].xyzx, cb0[13].wwww
    r6.xyz = (saturate((source[13].xyzx)*(source[13].wwww))).xyz;
    // 239: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 240: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 241: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 242: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 243: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 244: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 245: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 246: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 247: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 248: mul r0.x, r0.x, cb0[20].w
    r0.x = ((r0.xxxx)*(source[20].wwww)).x;
    // 249: mul r0.xyz, r3.yzwy, r0.xxxx
    r0.xyz = ((r3.yzwy)*(r0.xxxx)).xyz;
    // 250: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 251: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 252: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 253: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 254: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 255: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 256: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 257: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 258: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 259: ret
    return output;
}

// source.character.equipment-native-186.v1 / source program 864cd607617aac45b162728fbbd0155c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight186(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].z=(g_SourceCharacterTime.xxxx).x;
    source[20]=float4(input.lightColor,1.0);
    source[21].x=1.0;
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
    // 26: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).w;
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
    // 39: mul r1.w, r6.x, cb0[15].x
    r1.w = ((r6.xxxx)*(source[15].xxxx)).w;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 42: add_sat r1.w, r1.w, cb0[15].y
    r1.w = (saturate((r1.wwww)+(source[15].yyyy))).w;
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
    // 86: mad r6.xyw, cb0[13].wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((source[13].wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 87: dp3 r3.x, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 88: add r7.xyw, -r6.xyxw, r3.xxxx
    r7.xyw = ((-(r6.xyxw))+(r3.xxxx)).xyw;
    // 89: mad r6.xyw, cb0[14].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[14].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
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
    // 96: mad r3.xyz, cb0[13].wwww, r10.xyzx, r3.yzwy
    r3.xyz = ((source[13].wwww)*(r10.xyzx)+(r3.yzwy)).xyz;
    // 97: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 99: mad r3.xyz, cb0[14].xxxx, r10.xyzx, r3.xyzx
    r3.xyz = ((source[14].xxxx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 100: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 101: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 103: mad r3.xyz, cb0[13].wwww, r3.xyzx, r10.xyzx
    r3.xyz = ((source[13].wwww)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 104: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 106: mad r3.xyz, cb0[14].xxxx, r6.xywx, r3.xyzx
    r3.xyz = ((source[14].xxxx)*(r6.xywx)+(r3.xyzx)).xyz;
    // 107: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 108: mul r3.w, cb0[7].z, l(1.500000)
    r3.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 109: add r4.w, -cb0[7].w, l(1.000000)
    r4.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: mul r4.w, r4.w, cb0[15].z
    r4.w = ((r4.wwww)*(source[15].zzzz)).w;
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
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 134: max r3.w, r3.w, cb0[16].x
    r3.w = (max(r3.wwww,source[16].xxxx)).w;
    // 135: min r3.w, r3.w, cb0[15].w
    r3.w = (min(r3.wwww,source[15].wwww)).w;
    // 136: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: mad r3.w, r2.w, r4.w, r3.w
    r3.w = ((r2.wwww)*(r4.wwww)+(r3.wwww)).w;
    // 138: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 139: mad r3.w, cb0[16].z, r3.w, l(1.000000)
    r3.w = ((source[16].zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    // 146: mov_sat r1.w, cb0[16].w
    r1.w = (saturate(source[16].wwww)).w;
    // 147: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 148: add r3.w, -cb0[17].w, cb0[17].z
    r3.w = ((-(source[17].wwww))+(source[17].zzzz)).w;
    // 149: mad r3.w, r9.x, r3.w, cb0[17].w
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[17].wwww)).w;
    // 150: add r4.w, -r3.w, cb0[18].y
    r4.w = ((-(r3.wwww))+(source[18].yyyy)).w;
    // 151: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 152: add r4.w, -r3.w, cb0[18].w
    r4.w = ((-(r3.wwww))+(source[18].wwww)).w;
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
    // 225: mul r0.x, r0.x, cb0[19].x
    r0.x = ((r0.xxxx)*(source[19].xxxx)).x;
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
    // 232: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 233: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 234: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 235: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 236: ret
    return output;
}

// source.character.equipment-native-187.v1 / source program 03fce48ed0714940b32e543823a071a0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight187(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[7]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[9]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[10]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[12].w=(g_SourceCharacterTime.xxxx).x;
    source[14]=float4(input.lightColor,1.0);
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[1].w=1.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0;
    // 1: mul r0.xyz, cb0[3].xyzx, cb0[3].wwww
    r0.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 2: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 3: mad r1.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.wwww
    r1.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.wwww)).xyz;
    // 4: mad r0.xyz, cb0[11].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[11].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 5: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 6: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 7: mad r0.xyz, cb0[11].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[11].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 8: mad r1.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 9: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 10: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 11: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 13: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 14: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 15: mad r2.xyz, cb0[11].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[11].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 16: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 18: mad r2.xyz, cb0[11].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[11].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 19: mul r3.xyz, r0.xyzx, r2.xyzx
    r3.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 20: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 21: mad r0.xyz, -r0.xyzx, r2.xyzx, r0.wwww
    r0.xyz = ((-(r0.xyzx))*(r2.xyzx)+(r0.wwww)).xyz;
    // 22: mad r0.xyz, cb0[11].zzzz, r0.xyzx, r3.xyzx
    r0.xyz = ((source[11].zzzz)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 23: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 24: add r2.xyz, -r0.xyzx, r0.wwww
    r2.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 25: mad r0.xyz, cb0[11].wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((source[11].wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 26: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 27: add r0.w, -cb0[6].w, l(1.000000)
    r0.w = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: mul r0.w, r0.w, cb0[12].w
    r0.w = ((r0.wwww)*(source[12].wwww)).w;
    // 29: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 30: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 31: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: mul r1.x, cb0[6].z, l(1.500000)
    r1.x = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 33: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 34: mad r0.w, r0.w, l(0.500000), cb0[6].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 35: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 36: mul r1.x, r1.x, l(0.125000)
    r1.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 37: mul r2.y, cb0[6].y, cb0[7].y
    r2.y = ((source[6].yyyy)*(source[7].yyyy)).y;
    // 38: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 39: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 40: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 41: frc r1.z, cb0[6].x
    r1.z = (frac(source[6].xxxx)).z;
    // 42: add r1.w, -r1.z, cb0[6].x
    r1.w = ((-(r1.zzzz))+(source[6].xxxx)).w;
    // 43: mul r2.z, r1.w, l(0.125000)
    r2.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 44: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r1.xyw, r0.wwww, r2.xyxz
    r1.xyw = ((r0.wwww)*(r2.xyxz)).xyw;
    // 47: mul r0.w, r1.z, r2.w
    r0.w = ((r1.zzzz)*(r2.wwww)).w;
    // 48: mad r1.xyz, r1.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 49: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 50: add r1.xyzw, v9.yzxy, cb0[0].yzxy
    r1.xyzw = ((v9.yzxy)+(source[0].yzxy)).xyzw;
    // 51: add r1.xyzw, r1.xyzw, -cb0[1].yzxy
    r1.xyzw = ((r1.xyzw)+(-(source[1].yzxy))).xyzw;
    // 52: add r1.xy, -r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r1.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 53: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 54: mad r1.xy, cb0[8].wwww, r1.xyxx, r1.zwzz
    r1.xy = ((source[8].wwww)*(r1.xyxx)+(r1.zwzz)).xy;
    // 55: mul r0.w, cb0[8].y, cb0[12].w
    r0.w = ((source[8].yyyy)*(source[12].wwww)).w;
    // 56: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 57: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 58: mul r2.y, r0.w, l(0.020000)
    r2.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 59: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 61: mul r1.z, cb0[8].x, l(0.001000)
    r1.z = ((source[8].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 62: mov r2.x, l(0)
    r2.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 63: mad r1.xy, r1.zzzz, r1.xyxx, r2.xyxx
    r1.xy = ((r1.zzzz)*(r1.xyxx)+(r2.xyxx)).xy;
    // 64: dp2 r1.z, cb0[9].xyxx, r1.xyxx
    r1.z = (dot((source[9].xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 65: dp2 r1.y, cb0[10].xyxx, r1.xyxx
    r1.y = (dot((source[10].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 66: frc r1.z, r1.z
    r1.z = (frac(r1.zzzz)).z;
    // 67: mul r1.x, r1.z, l(0.125000)
    r1.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 69: mad r1.xyz, r1.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 70: mul r1.w, r1.w, l(0.900000)
    r1.w = ((r1.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 71: mad r1.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 72: mul_sat r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (saturate((r0.wwww)*(r1.xyzx))).xyz;
    // 73: mad r2.xyz, cb0[8].zzzz, r1.xyzx, -r0.xyzx
    r2.xyz = ((source[8].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 74: mul r1.xyz, r1.xyzx, cb0[8].zzzz
    r1.xyz = ((r1.xyzx)*(source[8].zzzz)).xyz;
    // 75: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 76: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 77: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 78: add r1.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 80: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v5.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 82: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 83: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 84: mul r1.xy, r1.xyxx, cb0[11].xxxx
    r1.xy = ((r1.xyxx)*(source[11].xxxx)).xy;
    // 85: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 86: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 87: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 88: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 89: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 90: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 91: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 92: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 93: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 94: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 95: dp3 r0.w, v8.xyzx, v8.xyzx
    r0.w = (dot((v8.xyzx).xyz,(v8.xyzx).xyz).xxxx).w;
    // 96: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 97: mul r2.xyz, r0.wwww, v8.xyzx
    r2.xyz = ((r0.wwww)*(v8.xyzx)).xyz;
    // 98: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 99: mul r3.xyz, r0.wwww, r1.xyzx
    r3.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 100: mad r2.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 101: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 102: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 103: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 104: dp3_sat r0.w, r2.xyzx, r3.xyzx
    r0.w = (saturate(dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx)).w;
    // 105: dp3_sat r1.x, r1.xyzx, r3.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx)).x;
    // 106: log r1.y, r0.w
    r1.y = (log2(r0.wwww)).y;
    // 107: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 108: mul r1.y, r1.y, l(15.000000)
    r1.y = ((r1.yyyy)*(float4(15.000000,15.000000,15.000000,15.000000))).y;
    // 109: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 110: mul r1.yzw, r1.yyyy, cb2[4].xxyz
    r1.yzw = ((r1.yyyy)*(passValues[4].xxyz)).yzw;
    // 111: movc r1.yzw, r0.wwww, l(0,0,0,0), r1.yyzw
    r1.yzw = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyzw)).yzw;
    // 112: lt r0.w, r1.x, l(0.000001)
    r0.w = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 113: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 114: mad r0.xyz, r0.xyzx, r0.wwww, r1.yzwy
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r1.yzwy)).xyz;
    // 115: mul o0.xyz, r0.xyzx, cb0[14].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[14].xyzx)).xyz;
    // 116: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v5.xyxx, t3.wxyz, s3, l(0.000000)
    r0.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).x;
    // 117: mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // 118: mul_sat r0.x, r0.x, cb0[13].x
    r0.x = (saturate((r0.xxxx)*(source[13].xxxx))).x;
    // 119: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 120: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 121: ret
    return output;
}

// source.character.equipment-native-188.v1 / source program 7c39266371aef84893f4bbfe65253a67
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight188(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].x=(g_SourceCharacterTime.xxxx).x;
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
    // 26: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[23].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[23].xxxx)) * 0xffffffffu)).w;
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
    // 39: mul r1.w, r6.x, cb0[17].y
    r1.w = ((r6.xxxx)*(source[17].yyyy)).w;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 42: add_sat r1.w, r1.w, cb0[17].z
    r1.w = (saturate((r1.wwww)+(source[17].zzzz))).w;
    // 43: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul r8.xyz, r2.wwww, cb0[11].xyzx
    r8.xyz = ((r2.wwww)*(source[11].xyzx)).xyz;
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
    // 50: mul r2.w, r6.y, cb0[14].y
    r2.w = ((r6.yyyy)*(source[14].yyyy)).w;
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
    // 64: round_ni r10.xy, v8.xyxx
    r10.xy = (floor(v8.xyxx)).xy;
    // 65: dp2 r3.x, r10.xyxx, l(12.989800, 78.233002, 0.000000, 0.000000)
    r3.x = (dot((r10.xyxx).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).x;
    // 66: sincos r3.x, null, r3.x
    r3.x = (sin(r3.xxxx)).x;
    // 67: mul r3.x, r3.x, l(43758.546875)
    r3.x = ((r3.xxxx)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).x;
    // 68: frc r3.x, r3.x
    r3.x = (frac(r3.xxxx)).x;
    // 69: add r3.x, r3.x, l(-0.500000)
    r3.x = ((r3.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 70: mad r3.x, r3.x, l(0.010000), r9.x
    r3.x = ((r3.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))+(r9.xxxx)).x;
    // 71: lt r4.w, cb0[14].w, r3.x
    r4.w = (asfloat((uint4)((source[14].wwww)<(r3.xxxx)) * 0xffffffffu)).w;
    // 72: and r5.w, r4.w, l(0x3f800000)
    r5.w = (asfloat(asuint(r4.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 73: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 74: mad r6.xyw, r5.wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((r5.wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 75: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 76: max r7.xyw, r7.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r7.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 77: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 78: max r10.xyz, r7.xywx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 79: add r10.xyz, -r7.xywx, r10.xyzx
    r10.xyz = ((-(r7.xywx))+(r10.xyzx)).xyz;
    // 80: mad r7.xyw, r2.wwww, r10.xyxz, r7.xyxw
    r7.xyw = ((r2.wwww)*(r10.xyxz)+(r7.xyxw)).xyw;
    // 81: lt r3.x, r3.x, cb0[14].z
    r3.x = (asfloat((uint4)((r3.xxxx)<(source[14].zzzz)) * 0xffffffffu)).x;
    // 82: movc r4.w, r4.w, l(0), l(1.000000)
    r4.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: movc r3.x, r3.x, l(-1.000000), l(-0.000000)
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).x;
    // 84: add r3.x, r3.x, r4.w
    r3.x = ((r3.xxxx)+(r4.wwww)).x;
    // 85: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 86: mad r6.xyw, r3.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r3.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 87: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 88: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 89: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 90: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 91: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 92: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 93: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 94: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 95: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 96: mul r7.xyw, cb0[7].xyxz, cb0[7].wwww
    r7.xyw = ((source[7].xyxz)*(source[7].wwww)).xyw;
    // 97: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 98: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 99: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 100: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 101: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 102: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 103: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 104: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 105: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 107: mad r6.xyw, cb0[16].wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 108: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 110: mad r6.xyw, cb0[17].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[17].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 111: mad r7.xyw, cb0[9].wwww, cb0[9].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[9].wwww)*(source[9].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 112: mad r10.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 113: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 114: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 115: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 116: add r10.xyz, -r3.yzwy, r4.wwww
    r10.xyz = ((-(r3.yzwy))+(r4.wwww)).xyz;
    // 117: mad r3.yzw, cb0[16].wwww, r10.xxyz, r3.yyzw
    r3.yzw = ((source[16].wwww)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 118: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r10.xyz, -r3.yzwy, r4.wwww
    r10.xyz = ((-(r3.yzwy))+(r4.wwww)).xyz;
    // 120: mad r3.yzw, cb0[17].xxxx, r10.xxyz, r3.yyzw
    r3.yzw = ((source[17].xxxx)*(r10.xxyz)+(r3.yyzw)).yzw;
    // 121: mul r10.xyz, r3.yzwy, r6.xywx
    r10.xyz = ((r3.yzwy)*(r6.xywx)).xyz;
    // 122: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 123: mad r3.yzw, -r6.xxyw, r3.yyzw, r4.wwww
    r3.yzw = ((-(r6.xxyw))*(r3.yyzw)+(r4.wwww)).yzw;
    // 124: mad r3.yzw, cb0[16].wwww, r3.yyzw, r10.xxyz
    r3.yzw = ((source[16].wwww)*(r3.yyzw)+(r10.xxyz)).yzw;
    // 125: dp3 r4.w, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: add r6.xyw, -r3.yzyw, r4.wwww
    r6.xyw = ((-(r3.yzyw))+(r4.wwww)).xyw;
    // 127: mad r3.yzw, cb0[17].xxxx, r6.xxyw, r3.yyzw
    r3.yzw = ((source[17].xxxx)*(r6.xxyw)+(r3.yyzw)).yzw;
    // 128: mul r3.yzw, r7.xxyw, r3.yyzw
    r3.yzw = ((r7.xxyw)*(r3.yyzw)).yzw;
    // 129: mul r4.w, cb0[8].z, l(1.500000)
    r4.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 130: add r5.w, -cb0[8].w, l(1.000000)
    r5.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r5.w, r5.w, cb0[15].x
    r5.w = ((r5.wwww)*(source[15].xxxx)).w;
    // 132: mul r5.w, r5.w, l(6.283185)
    r5.w = ((r5.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 133: sincos r5.w, null, r5.w
    r5.w = (sin(r5.wwww)).w;
    // 134: add r5.w, r5.w, l(1.000000)
    r5.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 136: mad r4.w, r4.w, l(0.500000), cb0[8].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 137: frc r5.w, cb0[8].x
    r5.w = (frac(source[8].xxxx)).w;
    // 138: add r6.x, -r5.w, cb0[8].x
    r6.x = ((-(r5.wwww))+(source[8].xxxx)).x;
    // 139: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 140: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 141: mul r10.y, cb0[8].y, cb0[12].y
    r10.y = ((source[8].yyyy)*(source[12].yyyy)).y;
    // 142: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 143: mul r6.x, r6.x, l(0.125000)
    r6.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 144: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 145: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 146: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 147: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t4.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 148: mul r6.xyw, r4.wwww, r10.xyxz
    r6.xyw = ((r4.wwww)*(r10.xyxz)).xyw;
    // 149: mul r4.w, r5.w, r10.w
    r4.w = ((r5.wwww)*(r10.wwww)).w;
    // 150: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.yzyw
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.yzyw))).xyw;
    // 151: mad r3.yzw, r4.wwww, r6.xxyw, r3.yyzw
    r3.yzw = ((r4.wwww)*(r6.xxyw)+(r3.yyzw)).yzw;
    // 152: add r4.w, r3.z, r3.y
    r4.w = ((r3.zzzz)+(r3.yyyy)).w;
    // 153: add r4.w, r3.w, r4.w
    r4.w = ((r3.wwww)+(r4.wwww)).w;
    // 154: mul r4.w, r4.w, l(0.333330)
    r4.w = ((r4.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 155: max r4.w, r4.w, cb0[18].x
    r4.w = (max(r4.wwww,source[18].xxxx)).w;
    // 156: min r4.w, r4.w, cb0[17].w
    r4.w = (min(r4.wwww,source[17].wwww)).w;
    // 157: add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: mad r4.w, r2.w, r5.w, r4.w
    r4.w = ((r2.wwww)*(r5.wwww)+(r4.wwww)).w;
    // 159: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 160: mad r4.w, cb0[18].z, r4.w, l(1.000000)
    r4.w = ((source[18].zzzz)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul r6.xyw, r3.yzyw, r4.wwww
    r6.xyw = ((r3.yzyw)*(r4.wwww)).xyw;
    // 162: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 163: mad r3.yzw, r4.wwww, r3.yyzw, -r6.xxyw
    r3.yzw = ((r4.wwww)*(r3.yyzw)+(-(r6.xxyw))).yzw;
    // 164: mad r3.yzw, r1.wwww, r3.yyzw, r6.xxyw
    r3.yzw = ((r1.wwww)*(r3.yyzw)+(r6.xxyw)).yzw;
    // 165: mul r3.yzw, r5.xxyz, r3.yyzw
    r3.yzw = ((r5.xxyz)*(r3.yyzw)).yzw;
    // 166: mad_sat r3.yzw, r3.yyzw, cb2[3].wwww, cb2[3].xxyz
    r3.yzw = (saturate((r3.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz))).yzw;
    // 167: mov_sat r1.w, cb0[18].w
    r1.w = (saturate(source[18].wwww)).w;
    // 168: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 169: add r4.w, -cb0[19].w, cb0[19].z
    r4.w = ((-(source[19].wwww))+(source[19].zzzz)).w;
    // 170: mad r4.w, r9.x, r4.w, cb0[19].w
    r4.w = ((r9.xxxx)*(r4.wwww)+(source[19].wwww)).w;
    // 171: add r5.x, -r4.w, cb0[20].y
    r5.x = ((-(r4.wwww))+(source[20].yyyy)).x;
    // 172: mad r4.w, r9.y, r5.x, r4.w
    r4.w = ((r9.yyyy)*(r5.xxxx)+(r4.wwww)).w;
    // 173: add r5.x, -r4.w, cb0[20].w
    r5.x = ((-(r4.wwww))+(source[20].wwww)).x;
    // 174: mad r4.w, r9.z, r5.x, r4.w
    r4.w = ((r9.zzzz)*(r5.xxxx)+(r4.wwww)).w;
    // 175: add r5.x, -r4.w, cb0[21].y
    r5.x = ((-(r4.wwww))+(source[21].yyyy)).x;
    // 176: mad r3.x, r3.x, r5.x, r4.w
    r3.x = ((r3.xxxx)*(r5.xxxx)+(r4.wwww)).x;
    // 177: mul r3.x, r6.z, r3.x
    r3.x = ((r6.zzzz)*(r3.xxxx)).x;
    // 178: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 179: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 180: movc r3.x, r7.z, l(0), r3.x
    r3.x = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 181: max r3.x, r3.x, cb0[0].x
    r3.x = (max(r3.xxxx,source[0].xxxx)).x;
    // 182: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 183: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 184: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 185: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 186: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 187: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 188: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 189: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 190: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 192: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 193: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 194: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 195: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 196: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 197: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 198: mad r5.xyz, -r3.yzwy, r2.wwww, r3.yzwy
    r5.xyz = ((-(r3.yzwy))*(r2.wwww)+(r3.yzwy)).xyz;
    // 199: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 200: mul r6.y, r3.x, r3.x
    r6.y = ((r3.xxxx)*(r3.xxxx)).y;
    // 201: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 202: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 203: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 204: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 205: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 206: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 207: mad r6.z, -r3.x, r3.x, l(1.000000)
    r6.z = ((-(r3.xxxx))*(r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 208: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 209: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 210: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 211: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 212: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 213: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 214: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 215: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.yyzw
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.yyzw)).yzw;
    // 216: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 217: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 219: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 220: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 221: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 222: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 223: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 224: max r7.xyz, r6.yzwy, r3.xxxx
    r7.xyz = (max(r6.yzwy,r3.xxxx)).xyz;
    // 225: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 226: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 227: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 228: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 230: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 231: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 232: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 233: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 234: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 235: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 236: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 237: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 238: mul_sat r6.xyz, cb0[13].xyzx, cb0[13].wwww
    r6.xyz = (saturate((source[13].xyzx)*(source[13].wwww))).xyz;
    // 239: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 240: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 241: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 242: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 243: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 244: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 245: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 246: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 247: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 248: mul r0.x, r0.x, cb0[21].z
    r0.x = ((r0.xxxx)*(source[21].zzzz)).x;
    // 249: mul r0.xyz, r3.yzwy, r0.xxxx
    r0.xyz = ((r3.yzwy)*(r0.xxxx)).xyz;
    // 250: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 251: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 252: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 253: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 254: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 255: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 256: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 257: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 258: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 259: ret
    return output;
}

// source.character.equipment-native-189.v1 / source program 5bc35f777d8e3b47bb06a8781051ad17
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight189(SOURCE_CHARACTER_NATIVE_INPUT input)
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

// source.character.equipment-native-190.v1 / source program 287c88c41a48164ea08282383bea6f18
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight190(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17].x=(g_SourceCharacterTime.xxxx).x;
    source[21]=float4(input.lightColor,1.0);
    source[22].x=1.0;
    // Native engine primitive/environment inputs; material uniforms remain unchanged.
    source[2].x=1.f;
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
    // 21: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[22].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[22].xxxx)) * 0xffffffffu)).w;
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
    // 30: add r4.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: lt r6.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 33: log r5.xyz, |r5.xzyx|
    r5.xyz = (log2(abs(r5.xzyx))).xyz;
    // 34: mul r1.w, r5.x, cb0[16].x
    r1.w = ((r5.xxxx)*(source[16].xxxx)).w;
    // 35: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 36: movc r1.w, r6.x, l(0), r1.w
    r1.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 37: add_sat r1.w, r1.w, cb0[16].y
    r1.w = (saturate((r1.wwww)+(source[16].yyyy))).w;
    // 38: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r7.xyz, r2.wwww, cb0[11].xyzx
    r7.xyz = ((r2.wwww)*(source[11].xyzx)).xyz;
    // 40: add r2.w, r1.w, l(-1.000000)
    r2.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 41: mad r2.w, cb0[16].w, r2.w, l(1.000000)
    r2.w = ((source[16].wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: mul r8.xyz, cb0[4].xyzx, cb0[4].wwww
    r8.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 43: max r9.xyz, r8.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r8.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 44: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 45: max r8.xyz, r8.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r8.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 46: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 47: mul r3.w, r5.y, cb0[14].y
    r3.w = ((r5.yyyy)*(source[14].yyyy)).w;
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
    // 53: mul r6.xyw, cb0[5].xyxz, cb0[5].wwww
    r6.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
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
    // 63: mul r6.xyw, cb0[6].xyxz, cb0[6].wwww
    r6.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
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
    // 72: mul r6.xyw, cb0[7].xyxz, cb0[7].wwww
    r6.xyw = ((source[7].xyxz)*(source[7].wwww)).xyw;
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
    // 83: mad r5.xyw, cb0[14].wwww, r6.xyxw, r5.xyxw
    r5.xyw = ((source[14].wwww)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 84: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 85: add r6.xyw, -r5.xyxw, r4.wwww
    r6.xyw = ((-(r5.xyxw))+(r4.wwww)).xyw;
    // 86: mad r5.xyw, cb0[15].xxxx, r6.xyxw, r5.xyxw
    r5.xyw = ((source[15].xxxx)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 87: mad r6.xyw, cb0[9].wwww, cb0[9].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r6.xyw = ((source[9].wwww)*(source[9].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 88: mad r9.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
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
    // 94: mad r9.yzw, cb0[14].wwww, r10.xxyz, r9.yyzw
    r9.yzw = ((source[14].wwww)*(r10.xxyz)+(r9.yyzw)).yzw;
    // 95: dp3 r4.w, r9.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r10.xyz, -r9.yzwy, r4.wwww
    r10.xyz = ((-(r9.yzwy))+(r4.wwww)).xyz;
    // 97: mad r9.yzw, cb0[15].xxxx, r10.xxyz, r9.yyzw
    r9.yzw = ((source[15].xxxx)*(r10.xxyz)+(r9.yyzw)).yzw;
    // 98: mul r10.xyz, r5.xywx, r9.yzwy
    r10.xyz = ((r5.xywx)*(r9.yzwy)).xyz;
    // 99: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: mad r5.xyw, -r5.xyxw, r9.yzyw, r4.wwww
    r5.xyw = ((-(r5.xyxw))*(r9.yzyw)+(r4.wwww)).xyw;
    // 101: mad r5.xyw, cb0[14].wwww, r5.xyxw, r10.xyxz
    r5.xyw = ((source[14].wwww)*(r5.xyxw)+(r10.xyxz)).xyw;
    // 102: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r9.yzw, -r5.xxyw, r4.wwww
    r9.yzw = ((-(r5.xxyw))+(r4.wwww)).yzw;
    // 104: mad r5.xyw, cb0[15].xxxx, r9.yzyw, r5.xyxw
    r5.xyw = ((source[15].xxxx)*(r9.yzyw)+(r5.xyxw)).xyw;
    // 105: mul r5.xyw, r6.xyxw, r5.xyxw
    r5.xyw = ((r6.xyxw)*(r5.xyxw)).xyw;
    // 106: mul r4.w, cb0[8].z, l(1.500000)
    r4.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 107: add r6.x, -cb0[8].w, l(1.000000)
    r6.x = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r6.x, r6.x, cb0[17].x
    r6.x = ((r6.xxxx)*(source[17].xxxx)).x;
    // 109: mul r6.x, r6.x, l(6.283185)
    r6.x = ((r6.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 110: sincos r6.x, null, r6.x
    r6.x = (sin(r6.xxxx)).x;
    // 111: add r6.x, r6.x, l(1.000000)
    r6.x = ((r6.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: mul r4.w, r4.w, r6.x
    r4.w = ((r4.wwww)*(r6.xxxx)).w;
    // 113: mad r4.w, r4.w, l(0.500000), cb0[8].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 114: frc r6.x, cb0[8].x
    r6.x = (frac(source[8].xxxx)).x;
    // 115: add r6.y, -r6.x, cb0[8].x
    r6.y = ((-(r6.xxxx))+(source[8].xxxx)).y;
    // 116: mul r10.z, r6.y, l(0.125000)
    r10.z = ((r6.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 117: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 118: mul r10.y, cb0[8].y, cb0[12].y
    r10.y = ((source[8].yyyy)*(source[12].yyyy)).y;
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
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 135: mov_sat r1.w, cb0[17].y
    r1.w = (saturate(source[17].yyyy)).w;
    // 136: mul_sat r2.w, r3.w, cb2[3].w
    r2.w = (saturate((r3.wwww)*(passValues[3].wwww))).w;
    // 137: add r3.w, -cb0[18].z, cb0[18].y
    r3.w = ((-(source[18].zzzz))+(source[18].yyyy)).w;
    // 138: mad r3.w, r8.x, r3.w, cb0[18].z
    r3.w = ((r8.xxxx)*(r3.wwww)+(source[18].zzzz)).w;
    // 139: add r4.w, -r3.w, cb0[19].x
    r4.w = ((-(r3.wwww))+(source[19].xxxx)).w;
    // 140: mad r3.w, r8.y, r4.w, r3.w
    r3.w = ((r8.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 141: add r4.w, -r3.w, cb0[19].z
    r4.w = ((-(r3.wwww))+(source[19].zzzz)).w;
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
    // 157: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 158: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 159: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 162: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 163: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: mad r5.xyz, -r4.xyzx, r2.wwww, r4.xyzx
    r5.xyz = ((-(r4.xyzx))*(r2.wwww)+(r4.xyzx)).xyz;
    // 165: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 166: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 167: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 168: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 169: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 171: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 172: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 173: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 174: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 175: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 176: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 177: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 178: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 179: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 180: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 181: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r4.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r4.xxyz)).yzw;
    // 182: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 183: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 185: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 186: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 187: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 188: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 189: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 191: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 192: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 193: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 194: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 195: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 196: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 197: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 198: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 199: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 200: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 201: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 202: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 203: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 204: mul_sat r6.xyz, cb0[13].xyzx, cb0[13].wwww
    r6.xyz = (saturate((source[13].xyzx)*(source[13].wwww))).xyz;
    // 205: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 206: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 207: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 208: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 209: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 210: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 211: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 212: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 213: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 214: mul r0.x, r0.x, cb0[20].y
    r0.x = ((r0.xxxx)*(source[20].yyyy)).x;
    // 215: mul r0.xyz, r4.xyzx, r0.xxxx
    r0.xyz = ((r4.xyzx)*(r0.xxxx)).xyz;
    // 216: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 217: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 219: mad r0.xyz, r5.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 220: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 221: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 222: mov_sat r9.x, r9.x
    r9.x = (saturate(r9.xxxx)).x;
    // 223: mul_sat r0.x, r9.x, cb0[17].z
    r0.x = (saturate((r9.xxxx)*(source[17].zzzz))).x;
    // 224: mul o0.w, r0.x, cb0[2].x
    output.targets[0].w = ((r0.xxxx)*(source[2].xxxx)).w;
    // 225: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 226: ret
    return output;
}

// source.character.equipment-native-191.v1 / source program 64f18b7d009ef84f8736200c4a221951
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight191(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[18]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[21]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[27].y=(g_SourceCharacterTime.xxxx).x;
    source[27].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[27].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[28].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[28].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[31]=float4(input.lightColor,1.0);
    source[32].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[32].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[32].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s4, l(0.000000)
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
    // 36: mad r9.xy, r9.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((r9.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: mul r10.xy, r9.xyxx, cb0[22].xxxx
    r10.xy = ((r9.xyxx)*(source[22].xxxx)).xy;
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
    // 44: mad r9.xyz, cb0[22].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[22].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: add r0.z, -cb0[23].y, cb0[23].x
    r0.z = ((-(source[23].yyyy))+(source[23].xxxx)).z;
    // 65: mad r0.z, r2.x, r0.z, cb0[23].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[23].yyyy)).z;
    // 66: add r1.x, -r0.z, cb0[23].z
    r1.x = ((-(r0.zzzz))+(source[23].zzzz)).x;
    // 67: mad r0.z, r2.y, r1.x, r0.z
    r0.z = ((r2.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 68: add r1.x, -r0.z, cb0[23].w
    r1.x = ((-(r0.zzzz))+(source[23].wwww)).x;
    // 69: mad r0.z, r2.z, r1.x, r0.z
    r0.z = ((r2.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 71: add r1.x, -r9.w, l(1.000000)
    r1.x = ((-(r9.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 72: add r1.y, -cb0[24].y, cb0[24].x
    r1.y = ((-(source[24].yyyy))+(source[24].xxxx)).y;
    // 73: mad r1.y, r2.x, r1.y, cb0[24].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[24].yyyy)).y;
    // 74: add r1.w, -r1.y, cb0[24].z
    r1.w = ((-(r1.yyyy))+(source[24].zzzz)).w;
    // 75: mad r1.y, r2.y, r1.w, r1.y
    r1.y = ((r2.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 76: add r1.w, -r1.y, cb0[24].w
    r1.w = ((-(r1.yyyy))+(source[24].wwww)).w;
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
    // 101: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s6, r0.z
    r0.xyz = ((g_SourceCharacterTexture5.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 102: rcp r1.y, cb0[25].x
    r1.y = (1.0/(source[25].xxxx)).y;
    // 103: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 104: mul r11.xyz, r4.xyzx, cb0[25].xxxx
    r11.xyz = ((r4.xyzx)*(source[25].xxxx)).xyz;
    // 105: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 106: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 107: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 108: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 109: mad r4.xyz, r11.xyzx, cb0[25].xxxx, r4.xyzx
    r4.xyz = ((r11.xyzx)*(source[25].xxxx)+(r4.xyzx)).xyz;
    // 110: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 111: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 112: add r1.y, cb0[25].x, l(1.000000)
    r1.y = ((source[25].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
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
    // 133: mul r12.xyz, r11.xyzx, cb0[28].wwww
    r12.xyz = ((r11.xyzx)*(source[28].wwww)).xyz;
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
    // 145: mul_sat r6.xy, r6.xzxx, cb0[25].wwww
    r6.xy = (saturate((r6.xzxx)*(source[25].wwww))).xy;
    // 146: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 147: add_sat r6.y, r6.y, -cb0[26].x
    r6.y = (saturate((r6.yyyy)+(-(source[26].xxxx)))).y;
    // 148: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 149: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 150: mul r6.y, r6.y, cb0[26].y
    r6.y = ((r6.yyyy)*(source[26].yyyy)).y;
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
    // 161: mul r1.x, r1.x, cb0[29].y
    r1.x = ((r1.xxxx)*(source[29].yyyy)).x;
    // 162: mad r6.y, cb0[29].x, r6.y, -r4.w
    r6.y = ((source[29].xxxx)*(r6.yyyy)+(-(r4.wwww))).y;
    // 163: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 164: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 165: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 166: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 167: mad r7.xyz, -cb0[28].wwww, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[28].wwww))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mad r7.xyz, r5.xyzx, r7.xyzx, r12.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 169: mul r11.xyz, cb0[3].xyzx, cb0[3].wwww
    r11.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 170: mul r12.xyz, cb0[4].xyzx, cb0[4].wwww
    r12.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 171: add r6.yz, v4.zzwz, l(0.000000, -0.050000, -0.050000, 0.000000)
    r6.yz = ((v4.zzwz)+(float4(0.000000,-0.050000,-0.050000,0.000000))).yz;
    // 172: mul_sat r6.yz, r6.yyzy, l(0.000000, 256.000000, 256.000000, 0.000000)
    r6.yz = (saturate((r6.yyzy)*(float4(0.000000,256.000000,256.000000,0.000000)))).yz;
    // 173: add r6.yz, -r6.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r6.yz = ((-(r6.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 174: mad r1.x, -r6.y, r6.z, l(1.000000)
    r1.x = ((-(r6.yyyy))*(r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 175: mul r6.yz, v4.wwzw, l(0.000000, 4.000000, 4.000000, 0.000000)
    r6.yz = ((v4.wwzw)*(float4(0.000000,4.000000,4.000000,0.000000))).yz;
    // 176: sample_b_indexable(texture2d)(float,float,float,float) r13.xyzw, r6.yzyy, t2.xyzw, s2, l(0.000000)
    r13.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 177: dp4 r4.w, r13.xyzw, cb0[7].xyzw
    r4.w = (dot((r13.xyzw).xyzw,(source[7].xyzw).xyzw).xxxx).w;
    // 178: mul r4.w, r1.x, r4.w
    r4.w = ((r1.xxxx)*(r4.wwww)).w;
    // 179: mul r4.w, r4.w, cb0[6].x
    r4.w = ((r4.wwww)*(source[6].xxxx)).w;
    // 180: mad r14.xyz, cb0[5].wwww, cb0[5].xyzx, -r12.xyzx
    r14.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r12.xyzx))).xyz;
    // 181: mad r12.xyz, r4.wwww, r14.xyzx, r12.xyzx
    r12.xyz = ((r4.wwww)*(r14.xyzx)+(r12.xyzx)).xyz;
    // 182: mad r12.xyz, -cb0[3].wwww, cb0[3].xyzx, r12.xyzx
    r12.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r12.xyzx)).xyz;
    // 183: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 184: mul r12.xyz, cb0[8].xyzx, cb0[8].wwww
    r12.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // 185: dp4 r4.w, r13.xyzw, cb0[10].xyzw
    r4.w = (dot((r13.xyzw).xyzw,(source[10].xyzw).xyzw).xxxx).w;
    // 186: mul r4.w, r1.x, r4.w
    r4.w = ((r1.xxxx)*(r4.wwww)).w;
    // 187: mul r4.w, r4.w, cb0[6].y
    r4.w = ((r4.wwww)*(source[6].yyyy)).w;
    // 188: mad r14.xyz, cb0[9].wwww, cb0[9].xyzx, -r12.xyzx
    r14.xyz = ((source[9].wwww)*(source[9].xyzx)+(-(r12.xyzx))).xyz;
    // 189: mad r12.xyz, r4.wwww, r14.xyzx, r12.xyzx
    r12.xyz = ((r4.wwww)*(r14.xyzx)+(r12.xyzx)).xyz;
    // 190: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 191: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 192: mul r12.xyz, cb0[11].xyzx, cb0[11].wwww
    r12.xyz = ((source[11].xyzx)*(source[11].wwww)).xyz;
    // 193: dp4 r4.w, r13.xyzw, cb0[13].xyzw
    r4.w = (dot((r13.xyzw).xyzw,(source[13].xyzw).xyzw).xxxx).w;
    // 194: mul r1.x, r1.x, r4.w
    r1.x = ((r1.xxxx)*(r4.wwww)).x;
    // 195: mul r1.x, r1.x, cb0[6].z
    r1.x = ((r1.xxxx)*(source[6].zzzz)).x;
    // 196: mad r13.xyz, cb0[12].wwww, cb0[12].xyzx, -r12.xyzx
    r13.xyz = ((source[12].wwww)*(source[12].xyzx)+(-(r12.xyzx))).xyz;
    // 197: mad r12.xyz, r1.xxxx, r13.xyzx, r12.xyzx
    r12.xyz = ((r1.xxxx)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 198: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 199: mad r11.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 200: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 201: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 202: mad r11.xyz, cb0[22].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[22].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 203: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 204: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 205: mad r11.xyz, cb0[22].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[22].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 206: mad r12.xyz, cb0[14].wwww, cb0[14].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[14].wwww)*(source[14].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 207: mad r13.xyz, cb0[15].wwww, cb0[15].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[15].wwww)*(source[15].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 208: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 209: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 210: dp3 r1.x, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 211: add r13.xyz, -r8.yzwy, r1.xxxx
    r13.xyz = ((-(r8.yzwy))+(r1.xxxx)).xyz;
    // 212: mad r8.xyz, cb0[22].yyyy, r13.xyzx, r8.yzwy
    r8.xyz = ((source[22].yyyy)*(r13.xyzx)+(r8.yzwy)).xyz;
    // 213: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 214: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 215: mad r8.xyz, cb0[22].zzzz, r13.xyzx, r8.xyzx
    r8.xyz = ((source[22].zzzz)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 216: mul r13.xyz, r8.xyzx, r11.xyzx
    r13.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 217: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 218: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 219: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 220: add r14.xyz, -cb0[16].xyzx, cb0[17].xyzx
    r14.xyz = ((-(source[16].xyzx))+(source[17].xyzx)).xyz;
    // 221: mad r14.xyz, r0.yyyy, r14.xyzx, cb0[16].xyzx
    r14.xyz = ((r0.yyyy)*(r14.xyzx)+(source[16].xyzx)).xyz;
    // 222: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 223: mul r0.xyz, r0.xyzx, cb0[25].yyyy
    r0.xyz = ((r0.xyzx)*(source[25].yyyy)).xyz;
    // 224: mul r14.xyz, r0.xyzx, r13.xyzx
    r14.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 225: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 227: mad r9.xyz, cb0[22].yyyy, r15.xyzx, r9.xyzx
    r9.xyz = ((source[22].yyyy)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 228: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 229: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 230: mad r9.xyz, cb0[22].zzzz, r15.xyzx, r9.xyzx
    r9.xyz = ((source[22].zzzz)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 231: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 232: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 233: mul r15.xyz, r15.xyzx, cb0[25].zzzz
    r15.xyz = ((r15.xyzx)*(source[25].zzzz)).xyz;
    // 234: add r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 235: add r1.x, r2.z, r1.x
    r1.x = ((r2.zzzz)+(r1.xxxx)).x;
    // 236: add_sat r1.x, r2.w, r1.x
    r1.x = (saturate((r2.wwww)+(r1.xxxx))).x;
    // 237: mad r2.xyz, r1.xxxx, r15.xyzx, r9.xyzx
    r2.xyz = ((r1.xxxx)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 238: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 239: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 240: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 241: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 242: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 243: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 244: mul r1.x, r1.x, cb0[26].z
    r1.x = ((r1.xxxx)*(source[26].zzzz)).x;
    // 245: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 246: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 247: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 248: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 249: div r1.z, cb0[26].w, r1.z
    r1.z = ((source[26].wwww)/(r1.zzzz)).z;
    // 250: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 251: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 252: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 253: mul r1.z, r1.z, cb0[27].x
    r1.z = ((r1.zzzz)*(source[27].xxxx)).z;
    // 254: mad r0.xyz, r2.xyzx, r0.xyzx, -r14.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r14.xyzx))).xyz;
    // 255: mad r0.xyz, r1.zzzz, r0.xyzx, r14.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r14.xyzx)).xyz;
    // 256: mad r0.xyz, r1.yyyy, r0.xyzx, -r13.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 257: mad r0.xyz, r1.xxxx, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 258: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 259: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 260: mad r0.xyz, cb0[22].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[22].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 261: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 262: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 263: mad r0.xyz, cb0[22].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[22].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 264: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 265: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 266: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 267: mul r2.w, r2.w, cb0[27].y
    r2.w = ((r2.wwww)*(source[27].yyyy)).w;
    // 268: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 269: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 270: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 271: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 272: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 273: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 274: add r4.w, -r2.w, cb0[2].x
    r4.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 275: mul r9.z, r4.w, l(0.125000)
    r9.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 276: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 277: mul r9.y, cb0[2].y, cb0[18].y
    r9.y = ((source[2].yyyy)*(source[18].yyyy)).y;
    // 278: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 279: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 280: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 281: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 282: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 283: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t6.xyzw, s7, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 284: mul r6.xyz, r1.zzzz, r9.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xyzx)).xyz;
    // 285: mul r1.z, r2.w, r9.w
    r1.z = ((r2.wwww)*(r9.wwww)).z;
    // 286: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 287: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 288: mul r1.z, cb0[19].y, cb0[27].y
    r1.z = ((source[19].yyyy)*(source[27].yyyy)).z;
    // 289: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 290: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 291: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 292: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 293: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 294: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 295: mad r3.xy, cb0[19].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[19].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 296: mul r2.w, cb0[19].x, l(0.001000)
    r2.w = ((source[19].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 297: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 298: mad r3.xy, r2.wwww, r3.xyxx, r6.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r6.xyxx)).xy;
    // 299: dp2 r2.w, cb0[20].xyxx, r3.xyxx
    r2.w = (dot((source[20].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 300: dp2 r3.y, cb0[21].xyxx, r3.xyxx
    r3.y = (dot((source[21].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 301: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 302: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 303: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s7, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 304: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 305: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 306: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 307: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 308: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 309: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 310: mul r6.xyz, r3.xyzx, cb0[19].zzzz
    r6.xyz = ((r3.xyzx)*(source[19].zzzz)).xyz;
    // 311: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 312: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 313: mad r3.xyz, cb0[19].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[19].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 314: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 315: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 316: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 317: mad r5.xyz, r11.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r11.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 318: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 319: dp3 r4.x, r4.xyzx, r10.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 320: mul r4.y, r0.w, cb0[29].z
    r4.y = ((r0.wwww)*(source[29].zzzz)).y;
    // 321: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t7.xyzw, s8, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 322: add r0.w, -cb0[29].w, l(2.000000)
    r0.w = ((-(source[29].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 323: mad r0.w, r1.y, r0.w, cb0[29].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[29].wwww)).w;
    // 324: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 325: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 326: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 327: mul r1.xyz, r1.xyzx, cb0[30].xxxx
    r1.xyz = ((r1.xyzx)*(source[30].xxxx)).xyz;
    // 328: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 329: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 330: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 331: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 332: mul r1.xyz, r1.xyzx, cb0[30].yyyy
    r1.xyz = ((r1.xyzx)*(source[30].yyyy)).xyz;
    // 333: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 334: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 335: mad r0.xyz, r7.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 336: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 337: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 338: mul o0.xyz, r0.xyzx, cb0[31].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[31].xyzx)).xyz;
    // 339: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 340: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 341: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 342: ret
    return output;
}

// source.character.equipment-native-192.v1 / source program 275d8c40a741a9468b9d366278929ce2
