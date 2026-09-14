SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight17(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].w=(g_SourceCharacterTime.xxxx).x;
    source[22]=float4(input.lightColor,1.0);
    source[23].x=1.0;
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
    // 129: mul r12.xyz, r11.xyzx, cb0[20].yyyy
    r12.xyz = ((r11.xyzx)*(source[20].yyyy)).xyz;
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
    // 157: mul r1.x, r1.x, cb0[20].w
    r1.x = ((r1.xxxx)*(source[20].wwww)).x;
    // 158: mad r6.y, cb0[20].z, r6.y, -r4.w
    r6.y = ((source[20].zzzz)*(r6.yyyy)+(-(r4.wwww))).y;
    // 159: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 160: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 161: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 162: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 163: mad r7.xyz, -cb0[20].yyyy, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[20].yyyy))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
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
    // 239: mul r2.w, r2.w, cb0[19].w
    r2.w = ((r2.wwww)*(source[19].wwww)).w;
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
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 256: mul r6.xyz, r1.zzzz, r9.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xyzx)).xyz;
    // 257: mul r1.z, r2.w, r9.w
    r1.z = ((r2.wwww)*(r9.wwww)).z;
    // 258: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 259: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 260: mul r1.z, cb0[12].y, cb0[19].w
    r1.z = ((source[12].yyyy)*(source[19].wwww)).z;
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
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 292: mul r4.y, r0.w, cb0[21].x
    r4.y = ((r0.wwww)*(source[21].xxxx)).y;
    // 293: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 294: add r0.w, -cb0[21].y, l(2.000000)
    r0.w = ((-(source[21].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 295: mad r0.w, r1.y, r0.w, cb0[21].y
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[21].yyyy)).w;
    // 296: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 297: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 298: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 299: mul r1.xyz, r1.xyzx, cb0[21].zzzz
    r1.xyz = ((r1.xyzx)*(source[21].zzzz)).xyz;
    // 300: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 301: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 302: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 303: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 304: mul r1.xyz, r1.xyzx, cb0[21].wwww
    r1.xyz = ((r1.xyzx)*(source[21].wwww)).xyz;
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
    // 310: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 311: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 312: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 313: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 314: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight18(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 49: add r4.w, -r2.w, v4.z
    r4.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 50: mad r2.w, cb0[14].y, r4.w, r2.w
    r2.w = ((source[14].yyyy)*(r4.wwww)+(r2.wwww)).w;
    // 51: mul r4.w, r2.w, cb0[14].z
    r4.w = ((r2.wwww)*(source[14].zzzz)).w;
    // 52: mad r2.w, r4.w, l(0.750000), r2.w
    r2.w = ((r4.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 53: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 54: mad_sat r2.w, cb0[15].x, r2.w, r2.w
    r2.w = (saturate((source[15].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t1.xyzw, s0, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 56: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 57: dp2 r4.w, r7.xyxx, r7.xyxx
    r4.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 58: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: max r4.w, r4.w, l(0.000000)
    r4.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 60: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 61: add r7.z, r4.w, l(0.000010)
    r7.z = ((r4.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 62: mad r4.w, r7.x, r6.x, l(0.200000)
    r4.w = ((r7.xxxx)*(r6.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 63: add r5.w, -r6.y, l(1.000000)
    r5.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: add r5.w, -r4.w, r5.w
    r5.w = ((-(r4.wwww))+(r5.wwww)).w;
    // 65: mad r7.w, cb0[15].z, r5.w, r4.w
    r7.w = ((source[15].zzzz)*(r5.wwww)+(r4.wwww)).w;
    // 66: mul r8.x, cb0[14].w, l(0.700000)
    r8.x = ((source[14].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 67: add r7.w, -r2.w, r7.w
    r7.w = ((-(r2.wwww))+(r7.wwww)).w;
    // 68: mad r7.w, r8.x, r7.w, r2.w
    r7.w = ((r8.xxxx)*(r7.wwww)+(r2.wwww)).w;
    // 69: div r7.w, r7.w, cb0[15].y
    r7.w = ((r7.wwww)/(source[15].yyyy)).w;
    // 70: add r7.w, -r7.w, l(1.000000)
    r7.w = ((-(r7.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r7.w, r1.w, r7.w
    r7.w = ((r1.wwww)*(r7.wwww)).w;
    // 72: mul r7.w, r7.w, l(4.000000)
    r7.w = ((r7.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 73: add r8.y, v4.w, l(0.500000)
    r8.y = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 74: round_ni r8.y, r8.y
    r8.y = (floor(r8.yyyy)).y;
    // 75: mul_sat r7.w, r7.w, r8.y
    r7.w = (saturate((r7.wwww)*(r8.yyyy))).w;
    // 76: mad r4.w, cb0[16].x, r5.w, r4.w
    r4.w = ((source[16].xxxx)*(r5.wwww)+(r4.wwww)).w;
    // 77: add r4.w, -r2.w, r4.w
    r4.w = ((-(r2.wwww))+(r4.wwww)).w;
    // 78: mad r2.w, r8.x, r4.w, r2.w
    r2.w = ((r8.xxxx)*(r4.wwww)+(r2.wwww)).w;
    // 79: div r2.w, r2.w, cb0[15].w
    r2.w = ((r2.wwww)/(source[15].wwww)).w;
    // 80: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 82: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 83: add r2.w, -r8.y, l(1.000000)
    r2.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 85: add r1.w, r1.w, r7.w
    r1.w = ((r1.wwww)+(r7.wwww)).w;
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
    // 118: add r4.w, -cb0[8].w, l(1.000000)
    r4.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: mul r4.w, r4.w, cb0[18].z
    r4.w = ((r4.wwww)*(source[18].zzzz)).w;
    // 120: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 121: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 122: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 124: mad r2.w, r2.w, l(0.500000), cb0[8].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 125: frc r4.w, cb0[8].x
    r4.w = (frac(source[8].xxxx)).w;
    // 126: add r5.w, -r4.w, cb0[8].x
    r5.w = ((-(r4.wwww))+(source[8].xxxx)).w;
    // 127: mul r11.z, r5.w, l(0.125000)
    r11.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 128: mov r11.xw, l(0,0,0,0)
    r11.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 129: mul r11.y, cb0[8].y, cb0[9].y
    r11.y = ((source[8].yyyy)*(source[9].yyyy)).y;
    // 130: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 131: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 132: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 133: add r6.xy, r6.xyxx, r11.xyxx
    r6.xy = ((r6.xyxx)+(r11.xyxx)).xy;
    // 134: add r6.xy, r6.xyxx, r11.zwzz
    r6.xy = ((r6.xyxx)+(r11.zwzz)).xy;
    // 135: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 136: mul r11.xyz, r2.wwww, r11.xyzx
    r11.xyz = ((r2.wwww)*(r11.xyzx)).xyz;
    // 137: mul r2.w, r4.w, r11.w
    r2.w = ((r4.wwww)*(r11.wwww)).w;
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
    // 148: mul r4.w, cb0[10].x, l(0.001000)
    r4.w = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 149: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 150: mad r6.xy, r4.wwww, r11.xyxx, r6.xyxx
    r6.xy = ((r4.wwww)*(r11.xyxx)+(r6.xyxx)).xy;
    // 151: dp2 r4.w, cb0[11].xyxx, r6.xyxx
    r4.w = (dot((source[11].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 152: dp2 r6.y, cb0[12].xyxx, r6.xyxx
    r6.y = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 153: frc r4.w, r4.w
    r4.w = (frac(r4.wwww)).w;
    // 154: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 155: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 156: mul r4.w, r11.w, l(0.900000)
    r4.w = ((r11.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 157: mad r11.xyz, r11.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r10.xyzx
    r11.xyz = ((r11.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r10.xyzx))).xyz;
    // 158: mad r11.xyz, r4.wwww, r11.xyzx, r10.xyzx
    r11.xyz = ((r4.wwww)*(r11.xyzx)+(r10.xyzx)).xyz;
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
    // 168: max r4.w, r2.w, l(0.000000)
    r4.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 169: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: mul r11.xyz, r2.xyzx, r5.wwww
    r11.xyz = ((r2.xyzx)*(r5.wwww)).xyz;
    // 171: mad r12.xyz, -r5.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r5.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mad r11.yzw, cb0[19].wwww, r12.xxyz, r11.xxyz
    r11.yzw = ((source[19].wwww)*(r12.xxyz)+(r11.xxyz)).yzw;
    // 173: dp3 r5.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 174: add r12.xyz, r5.wwww, -cb0[2].xyzx
    r12.xyz = ((r5.wwww)+(-(source[2].xyzx))).xyz;
    // 175: mad r12.xyz, cb0[16].zzzz, r12.xyzx, cb0[2].xyzx
    r12.xyz = ((source[16].zzzz)*(r12.xyzx)+(source[2].xyzx)).xyz;
    // 176: dp3 r5.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 177: add r13.xyz, -r12.xyzx, r5.wwww
    r13.xyz = ((-(r12.xyzx))+(r5.wwww)).xyz;
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
    // 207: dp3 r5.w, r0.xyzx, r7.xyzx
    r5.w = (dot((r0.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 208: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 209: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 210: mul r3.w, r3.z, r1.z
    r3.w = ((r3.zzzz)*(r1.zzzz)).w;
    // 211: mad r1.xyz, r3.zzwz, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r3.zzwz)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
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
    // 217: add r0.x, r0.x, r5.w
    r0.x = ((r0.xxxx)+(r5.wwww)).x;
    // 218: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 219: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 220: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 221: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 222: mad r0.y, cb0[17].w, l(4.500000), l(0.500000)
    r0.y = ((source[17].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 223: mul r0.y, r0.y, cb0[20].x
    r0.y = ((r0.yyyy)*(source[20].xxxx)).y;
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
    // 236: mul r0.x, r0.x, cb0[20].y
    r0.x = ((r0.xxxx)*(source[20].yyyy)).x;
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
    // 243: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 244: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 245: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 246: mul o0.w, r6.w, cb0[1].w
    output.targets[0].w = ((r6.wwww)*(source[1].wwww)).w;
    // 247: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 248: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight19(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[18].z=(g_SourceCharacterTime.xxxx).x;
    source[18].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[21]=float4(input.lightColor,1.0);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add_sat r0.w, r0.w, -cb0[19].w
    r0.w = (saturate((r0.wwww)+(-(source[19].wwww)))).w;
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
    // 9: add r1.y, -v4.z, l(1.000000)
    r1.y = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 10: add r1.z, -r1.y, v4.z
    r1.z = ((-(r1.yyyy))+(v4.zzzz)).z;
    // 11: mad r1.y, cb0[14].y, r1.z, r1.y
    r1.y = ((source[14].yyyy)*(r1.zzzz)+(r1.yyyy)).y;
    // 12: mul r1.z, r1.y, cb0[14].z
    r1.z = ((r1.yyyy)*(source[14].zzzz)).z;
    // 13: mad r1.y, r1.z, l(0.750000), r1.y
    r1.y = ((r1.zzzz)*(float4(0.750000,0.750000,0.750000,0.750000))+(r1.yyyy)).y;
    // 14: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 15: mad_sat r1.y, cb0[15].x, r1.y, r1.y
    r1.y = (saturate((source[15].xxxx)*(r1.yyyy)+(r1.yyyy))).y;
    // 16: add r1.xz, -r0.wwyw, l(1.000000, 0.000000, 1.000000, 0.000000)
    r1.xz = ((-(r0.wwyw))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 18: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 19: mad r1.w, r2.x, r0.x, l(0.200000)
    r1.w = ((r2.xxxx)*(r0.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 20: add r1.z, -r1.w, r1.z
    r1.z = ((-(r1.wwww))+(r1.zzzz)).z;
    // 21: mad r2.w, cb0[16].x, r1.z, r1.w
    r2.w = ((source[16].xxxx)*(r1.zzzz)+(r1.wwww)).w;
    // 22: mad r1.z, cb0[15].z, r1.z, r1.w
    r1.z = ((source[15].zzzz)*(r1.zzzz)+(r1.wwww)).z;
    // 23: add r1.z, -r1.y, r1.z
    r1.z = ((-(r1.yyyy))+(r1.zzzz)).z;
    // 24: add r1.w, -r1.y, r2.w
    r1.w = ((-(r1.yyyy))+(r2.wwww)).w;
    // 25: mul r2.w, cb0[14].w, l(0.700000)
    r2.w = ((source[14].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).w;
    // 26: mad r1.w, r2.w, r1.w, r1.y
    r1.w = ((r2.wwww)*(r1.wwww)+(r1.yyyy)).w;
    // 27: mad r1.y, r2.w, r1.z, r1.y
    r1.y = ((r2.wwww)*(r1.zzzz)+(r1.yyyy)).y;
    // 28: div r1.yz, r1.yywy, cb0[15].yywy
    r1.yz = ((r1.yywy)/(source[15].yywy)).yz;
    // 29: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 30: mad r1.w, cb0[13].y, l(-3.500000), l(5.000000)
    r1.w = ((source[13].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 31: mul r1.w, r1.w, cb0[14].x
    r1.w = ((r1.wwww)*(source[14].xxxx)).w;
    // 32: mul r1.yz, r1.yyzy, r1.wwww
    r1.yz = ((r1.yyzy)*(r1.wwww)).yz;
    // 33: mul r1.y, r1.y, l(4.000000)
    r1.y = ((r1.yyyy)*(float4(4.000000,4.000000,4.000000,4.000000))).y;
    // 34: mul_sat r0.w, r0.w, r1.y
    r0.w = (saturate((r0.wwww)*(r1.yyyy))).w;
    // 35: mul r1.y, r1.z, l(4.000000)
    r1.y = ((r1.zzzz)*(float4(4.000000,4.000000,4.000000,4.000000))).y;
    // 36: mul_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)*(r1.yyyy))).x;
    // 37: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 38: add r0.w, -r0.y, r0.w
    r0.w = ((-(r0.yyyy))+(r0.wwww)).w;
    // 39: mad r0.y, cb0[16].y, r0.w, r0.y
    r0.y = ((source[16].yyyy)*(r0.wwww)+(r0.yyyy)).y;
    // 40: add r1.xyz, -cb0[2].xyzx, cb0[3].xyzx
    r1.xyz = ((-(source[2].xyzx))+(source[3].xyzx)).xyz;
    // 41: mul r1.xyz, r1.xyzx, cb0[13].xxxx
    r1.xyz = ((r1.xyzx)*(source[13].xxxx)).xyz;
    // 42: mad r1.xyz, r0.yyyy, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(source[2].xyzx)).xyz;
    // 43: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 44: add r3.xyz, -r1.xyzx, r0.yyyy
    r3.xyz = ((-(r1.xyzx))+(r0.yyyy)).xyz;
    // 45: mad r1.xyz, cb0[16].zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((source[16].zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 46: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 47: add r3.xyz, -r1.xyzx, r0.yyyy
    r3.xyz = ((-(r1.xyzx))+(r0.yyyy)).xyz;
    // 48: mad r1.xyz, cb0[16].wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((source[16].wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 49: mad r3.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 50: mad r4.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 51: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 52: mul r4.xyz, r1.xyzx, r3.xyzx
    r4.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 53: mad r1.xyz, r1.xyzx, r3.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 54: mul r3.xyz, r0.xxxx, r4.xyzx
    r3.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 55: mad r0.xyw, r0.xxxx, r4.xyxz, l(0.010000, 0.010000, 0.000000, 0.010000)
    r0.xyw = ((r0.xxxx)*(r4.xyxz)+(float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 56: add r1.w, -cb0[7].w, l(1.000000)
    r1.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 58: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 59: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 60: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: mul r2.w, cb0[7].z, l(1.500000)
    r2.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 62: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 63: mad r1.w, r1.w, l(0.500000), cb0[7].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 64: mul r4.y, cb0[7].y, cb0[8].y
    r4.y = ((source[7].yyyy)*(source[8].yyyy)).y;
    // 65: mul r5.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r5.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 66: frc r2.w, r5.x
    r2.w = (frac(r5.xxxx)).w;
    // 67: mul r5.y, r2.w, l(0.125000)
    r5.y = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 68: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 69: add r4.xy, r4.xyxx, r5.yzyy
    r4.xy = ((r4.xyxx)+(r5.yzyy)).xy;
    // 70: frc r2.w, cb0[7].x
    r2.w = (frac(source[7].xxxx)).w;
    // 71: add r3.w, -r2.w, cb0[7].x
    r3.w = ((-(r2.wwww))+(source[7].xxxx)).w;
    // 72: mul r4.z, r3.w, l(0.125000)
    r4.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 73: add r4.xy, r4.xyxx, r4.zwzz
    r4.xy = ((r4.xyxx)+(r4.zwzz)).xy;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 75: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 76: mul r1.w, r2.w, r4.w
    r1.w = ((r2.wwww)*(r4.wwww)).w;
    // 77: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 78: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 79: add r4.xyz, v8.xyzx, cb0[0].xyzx
    r4.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 80: add r5.xyzw, r4.yzxy, -cb0[1].yzxy
    r5.xyzw = ((r4.yzxy)+(-(source[1].yzxy))).xyzw;
    // 81: add r4.xyz, -r4.xyzx, cb0[0].xyzx
    r4.xyz = ((-(r4.xyzx))+(source[0].xyzx)).xyz;
    // 82: add r5.xy, -r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = ((-(r5.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 83: add r5.xy, -r5.zwzz, r5.xyxx
    r5.xy = ((-(r5.zwzz))+(r5.xyxx)).xy;
    // 84: mad r5.xy, cb0[9].wwww, r5.xyxx, r5.zwzz
    r5.xy = ((source[9].wwww)*(r5.xyxx)+(r5.zwzz)).xy;
    // 85: mul r1.w, cb0[9].y, cb0[18].z
    r1.w = ((source[9].yyyy)*(source[18].zzzz)).w;
    // 86: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 87: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 88: mul r6.y, r1.w, l(0.020000)
    r6.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 89: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 91: mul r2.w, cb0[9].x, l(0.001000)
    r2.w = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 92: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 93: mad r5.xy, r2.wwww, r5.xyxx, r6.xyxx
    r5.xy = ((r2.wwww)*(r5.xyxx)+(r6.xyxx)).xy;
    // 94: dp2 r2.w, cb0[10].xyxx, r5.xyxx
    r2.w = (dot((source[10].xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 95: dp2 r5.y, cb0[11].xyxx, r5.xyxx
    r5.y = (dot((source[11].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 96: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 97: mul r5.x, r2.w, l(0.125000)
    r5.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 99: mad r5.xyz, r5.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r3.xyzx
    r5.xyz = ((r5.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r3.xyzx))).xyz;
    // 100: mul r2.w, r5.w, l(0.900000)
    r2.w = ((r5.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 101: mad r5.xyz, r2.wwww, r5.xyzx, r3.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 102: mul_sat r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = (saturate((r1.wwww)*(r5.xyzx))).xyz;
    // 103: mad r6.xyz, cb0[9].zzzz, r5.xyzx, -r3.xyzx
    r6.xyz = ((source[9].zzzz)*(r5.xyzx)+(-(r3.xyzx))).xyz;
    // 104: mul r5.xyz, r5.xyzx, cb0[9].zzzz
    r5.xyz = ((r5.xyzx)*(source[9].zzzz)).xyz;
    // 105: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 107: mad r3.xyz, r1.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 108: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 109: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 111: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 112: add r2.z, r1.w, l(0.000010)
    r2.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 113: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 114: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 115: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 116: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 117: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 118: mul r5.xyz, r1.wwww, v5.xyzx
    r5.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 119: dp3 r2.w, r2.xyzx, r5.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 120: add r3.w, r2.w, l(1.000000)
    r3.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 122: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 123: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 124: dp3 r4.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r6.xyz, r4.wwww, -cb0[2].xyzx
    r6.xyz = ((r4.wwww)+(-(source[2].xyzx))).xyz;
    // 126: mad r6.xyz, cb0[16].zzzz, r6.xyzx, cb0[2].xyzx
    r6.xyz = ((source[16].zzzz)*(r6.xyzx)+(source[2].xyzx)).xyz;
    // 127: dp3 r4.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 128: add r7.xyz, -r6.xyzx, r4.wwww
    r7.xyz = ((-(r6.xyzx))+(r4.wwww)).xyz;
    // 129: mad r6.xyz, cb0[16].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[16].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 130: mul r7.xyz, r6.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r7.xyz = ((r6.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 131: mul r7.xyz, r3.wwww, r7.xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)).xyz;
    // 132: min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: mul r8.xyz, r2.wwww, cb2[3].xyzx
    r8.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // 134: add r2.w, -r3.w, l(1.000000)
    r2.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mad r2.w, cb0[20].y, r2.w, r3.w
    r2.w = ((source[20].yyyy)*(r2.wwww)+(r3.wwww)).w;
    // 136: max r3.w, r3.w, l(0.500000)
    r3.w = (max(r3.wwww,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 137: mad r9.xyz, -r6.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r6.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 138: mad r7.xyz, r2.wwww, r9.xyzx, r7.xyzx
    r7.xyz = ((r2.wwww)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 139: add r9.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 140: mov_sat r2.w, r5.z
    r2.w = (saturate(r5.zzzz)).w;
    // 141: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 142: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 143: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mad r6.xyz, r2.wwww, r9.xyzx, r6.xyzx
    r6.xyz = ((r2.wwww)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 145: mad r6.xyz, r7.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r6.xyzx
    r6.xyz = ((r7.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r6.xyzx)).xyz;
    // 146: mul_sat r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = (saturate((r3.xyzx)*(r6.xyzx))).xyz;
    // 147: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 148: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 149: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 150: dp3 r2.w, r6.xyzx, r2.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 151: dp3 r4.w, r6.xyzx, r5.xyzx
    r4.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 152: dp3 r5.x, v7.xyzx, v7.xyzx
    r5.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 153: rsq r5.x, r5.x
    r5.x = (rsqrt(r5.xxxx)).x;
    // 154: mul r5.xyz, r5.xxxx, v7.xyzx
    r5.xyz = ((r5.xxxx)*(v7.xyzx)).xyz;
    // 155: mul r6.xyz, r4.xyzx, r5.zzzz
    r6.xyz = ((r4.xyzx)*(r5.zzzz)).xyz;
    // 156: mad r4.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r4.xyzx)).xyz;
    // 157: dp3 r4.x, r4.xyzx, r4.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 158: sqrt r4.x, r4.x
    r4.x = (sqrt(r4.xxxx)).x;
    // 159: div r4.x, r4.z, r4.x
    r4.x = ((r4.zzzz)/(r4.xxxx)).x;
    // 160: add r4.x, r4.x, cb0[6].z
    r4.x = ((r4.xxxx)+(source[6].zzzz)).x;
    // 161: add r4.x, -r4.x, r4.w
    r4.x = ((-(r4.xxxx))+(r4.wwww)).x;
    // 162: add r2.w, r2.w, r4.x
    r2.w = ((r2.wwww)+(r4.xxxx)).w;
    // 163: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 164: add r2.w, r2.w, l(-0.500000)
    r2.w = ((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 165: add r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)+(r2.wwww)).w;
    // 166: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: log r4.x, r2.w
    r4.x = (log2(r2.wwww)).x;
    // 168: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 169: mad r4.y, cb0[17].w, l(4.500000), l(0.500000)
    r4.y = ((source[17].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 170: mul r4.y, r4.y, cb0[20].z
    r4.y = ((r4.yyyy)*(source[20].zzzz)).y;
    // 171: mul r4.y, r4.y, l(0.050000)
    r4.y = ((r4.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 172: mul r4.x, r4.x, r4.y
    r4.x = ((r4.xxxx)*(r4.yyyy)).x;
    // 173: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 174: mul r3.w, r3.w, r4.x
    r3.w = ((r3.wwww)*(r4.xxxx)).w;
    // 175: movc r2.w, r2.w, l(0), r3.w
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 176: mad r4.xyz, v5.xyzx, r1.wwww, r5.xyzx
    r4.xyz = ((v5.xyzx)*(r1.wwww)+(r5.xyzx)).xyz;
    // 177: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 178: mad r1.w, r1.w, r1.w, l(0.100000)
    r1.w = ((r1.wwww)*(r1.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 179: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: div r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)/(r1.wwww)).w;
    // 181: mul r1.w, r1.w, cb0[20].w
    r1.w = ((r1.wwww)*(source[20].wwww)).w;
    // 182: dp3 r2.w, r1.xyzx, r1.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 183: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 184: div r1.xyz, r1.xyzx, r2.wwww
    r1.xyz = ((r1.xyzx)/(r2.wwww)).xyz;
    // 185: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 186: mov_sat r0.z, r5.z
    r0.z = (saturate(r5.zzzz)).z;
    // 187: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 188: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 189: mul r1.xyz, r1.xyzx, r0.zzzz
    r1.xyz = ((r1.xyzx)*(r0.zzzz)).xyz;
    // 190: mul r1.xyz, r1.xyzx, cb0[17].xxxx
    r1.xyz = ((r1.xyzx)*(source[17].xxxx)).xyz;
    // 191: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 192: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 193: min r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 194: mad r1.xyz, r3.xyzx, r7.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 195: dp3 r0.z, r0.xywx, r0.xywx
    r0.z = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).z;
    // 196: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 197: div r0.xyz, r0.xywx, r0.zzzz
    r0.xyz = ((r0.xywx)/(r0.zzzz)).xyz;
    // 198: mul r0.xyz, r0.xyzx, cb0[12].xyzx
    r0.xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 199: dp3 r0.w, r2.xyzx, r5.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 200: add r1.w, -|r5.z|, l(1.000000)
    r1.w = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 201: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 202: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 203: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 204: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 205: mul r0.w, r0.w, cb0[20].x
    r0.w = ((r0.wwww)*(source[20].xxxx)).w;
    // 206: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 207: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 208: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 209: mad r0.xyz, r0.xyzx, cb2[3].wwww, r8.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r8.xyzx)).xyz;
    // 210: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 211: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 212: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 213: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight20(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 83: add r3.w, -cb0[8].w, l(1.000000)
    r3.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul r3.w, r3.w, cb0[15].y
    r3.w = ((r3.wwww)*(source[15].yyyy)).w;
    // 85: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 86: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 87: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 89: mad r2.w, r2.w, l(0.500000), cb0[8].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 90: frc r3.w, cb0[8].x
    r3.w = (frac(source[8].xxxx)).w;
    // 91: add r4.w, -r3.w, cb0[8].x
    r4.w = ((-(r3.wwww))+(source[8].xxxx)).w;
    // 92: mul r11.z, r4.w, l(0.125000)
    r11.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 93: mov r11.xw, l(0,0,0,0)
    r11.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 94: mul r11.y, cb0[8].y, cb0[9].y
    r11.y = ((source[8].yyyy)*(source[9].yyyy)).y;
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
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: mul r11.xyz, r2.wwww, r11.xyzx
    r11.xyz = ((r2.wwww)*(r11.xyzx)).xyz;
    // 102: mul r2.w, r3.w, r11.w
    r2.w = ((r3.wwww)*(r11.wwww)).w;
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
    // 113: mul r3.w, cb0[10].x, l(0.001000)
    r3.w = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 114: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 115: mad r6.xy, r3.wwww, r11.xyxx, r6.xyxx
    r6.xy = ((r3.wwww)*(r11.xyxx)+(r6.xyxx)).xy;
    // 116: dp2 r3.w, cb0[11].xyxx, r6.xyxx
    r3.w = (dot((source[11].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 117: dp2 r6.y, cb0[12].xyxx, r6.xyxx
    r6.y = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 118: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 119: mul r6.x, r3.w, l(0.125000)
    r6.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 120: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 133: max r3.w, r2.w, l(0.000000)
    r3.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r4.w, r3.w, l(1.000000)
    r4.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r11.xyz, r2.xyzx, r4.wwww
    r11.xyz = ((r2.xyzx)*(r4.wwww)).xyz;
    // 136: mad r12.xyz, -r4.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r4.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 137: mad r11.yzw, cb0[16].zzzz, r12.xxyz, r11.xxyz
    r11.yzw = ((source[16].zzzz)*(r12.xxyz)+(r11.xxyz)).yzw;
    // 138: dp3 r4.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 139: add r12.xyz, r4.wwww, -cb0[2].xyzx
    r12.xyz = ((r4.wwww)+(-(source[2].xyzx))).xyz;
    // 140: mad r12.xyz, cb0[13].yyyy, r12.xyzx, cb0[2].xyzx
    r12.xyz = ((source[13].yyyy)*(r12.xyzx)+(source[2].xyzx)).xyz;
    // 141: dp3 r4.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r13.xyz, -r12.xyzx, r4.wwww
    r13.xyz = ((-(r12.xyzx))+(r4.wwww)).xyz;
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
    // 208: mul r1.xyz, r3.wwww, cb2[3].xyzx
    r1.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
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


SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight21(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].z=(g_SourceCharacterTime.xxxx).x;
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
    // 15: ne r0.w, l(0x00000000,0x00000000,0x00000000,0x00000000), cb0[20].x
    r0.w = (asfloat((uint4)((float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))!=(source[20].xxxx)) * 0xffffffffu)).w;
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
    // 22: mov r5.xyz, l(0x3f800000,0x3f800000,0x3f800000,0x00000000)
    r5.xyz = (float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u))).xyz;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s2, l(0x00000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).wxyz).xyzw;
    // 31: mov_sat r8.x, r8.x
    r8.x = (saturate(r8.xxxx)).x;
    // 32: add r1.w, r8.x, l(0xbeaaa64c)
    r1.w = ((r8.xxxx)+(float4(asfloat(0xbeaaa64cu),asfloat(0xbeaaa64cu),asfloat(0xbeaaa64cu),asfloat(0xbeaaa64cu)))).w;
    // 33: lt r1.w, r1.w, l(0x00000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))) * 0xffffffffu)).w;
    // 34: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0x00000000)
    r9.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xy;
    // 36: mad r9.xy, r9.xyxx, l(0x40000000,0x40000000,0x00000000,0x00000000), l(0xbf800000,0xbf800000,0x00000000,0x00000000)
    r9.xy = ((r9.xyxx)*(float4(asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x00000000u),asfloat(0x00000000u)))+(float4(asfloat(0xbf800000u),asfloat(0xbf800000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 37: mul r10.xy, r9.xyxx, cb0[12].xxxx
    r10.xy = ((r9.xyxx)*(source[12].xxxx)).xy;
    // 38: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 39: add r1.w, -r1.w, l(0x3f800000)
    r1.w = ((-(r1.wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 40: max r1.w, r1.w, l(0x00000000)
    r1.w = (max(r1.wwww,float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).w;
    // 41: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 42: add r10.z, r1.w, l(0x3727c5ac)
    r10.z = ((r1.wwww)+(float4(asfloat(0x3727c5acu),asfloat(0x3727c5acu),asfloat(0x3727c5acu),asfloat(0x3727c5acu)))).z;
    // 43: add r9.xyz, -r10.xyzx, l(0x00000000,0x00000000,0x3f800000,0x00000000)
    r9.xyz = ((-(r10.xyzx))+(float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x3f800000u),asfloat(0x00000000u)))).xyz;
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
    // 56: mad r1.xyz, r0.xyzx, l(0x40000000,0x40000000,0x40000000,0x00000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x00000000u)))+(-(r1.xyzx))).xyz;
    // 57: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 58: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 59: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 60: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 61: mad r0.z, -r1.z, l(0x3e800000), l(0x3e800000)
    r0.z = ((-(r1.zzzz))*(float4(asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u)))+(float4(asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u),asfloat(0x3e800000u)))).z;
    // 62: mad r0.xy, r0.zzzz, r0.xyxx, l(0x3f000000,0x3f000000,0x00000000,0x00000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0x00000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyzw;
    // 64: add r0.z, -r2.w, l(0x3f800000)
    r0.z = ((-(r2.wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 65: lt r1.x, |r0.z|, l(0x358637bd)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).x;
    // 66: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 67: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 68: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 69: min r0.z, r0.z, l(0x3f800000)
    r0.z = (min(r0.zzzz,float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 70: movc r0.z, r1.x, l(0x00000000), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r0.zzzz)).z;
    // 71: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 72: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 73: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 74: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 75: mul r1.yw, r1.yyyw, l(0x00000000,0x442f0000,0x00000000,0x442f0000)
    r1.yw = ((r1.yyyw)*(float4(asfloat(0x00000000u),asfloat(0x442f0000u),asfloat(0x00000000u),asfloat(0x442f0000u)))).yw;
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
    // 98: mul r1.xyw, r1.xyxw, l(0x3eaaaa9f,0x3eaaaa9f,0x00000000,0x3eaaaa9f)
    r1.xyw = ((r1.xyxw)*(float4(asfloat(0x3eaaaa9fu),asfloat(0x3eaaaa9fu),asfloat(0x00000000u),asfloat(0x3eaaaa9fu)))).xyw;
    // 99: add r0.x, cb0[13].z, l(0x3f800000)
    r0.x = ((source[13].zzzz)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
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
    // 106: add r4.x, -r4.x, l(0x3f800000)
    r4.x = ((-(r4.xxxx))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 107: lt r4.y, |r4.x|, l(0x358637bd)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).y;
    // 108: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 109: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 110: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 111: movc r4.x, r4.y, l(0x00000000), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r4.xxxx)).x;
    // 112: add r4.y, r4.x, l(0xbce38eb0)
    r4.y = ((r4.xxxx)+(float4(asfloat(0xbce38eb0u),asfloat(0xbce38eb0u),asfloat(0xbce38eb0u),asfloat(0xbce38eb0u)))).y;
    // 113: mad r4.x, r4.x, r4.y, l(0x3ce38eb0)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(asfloat(0x3ce38eb0u),asfloat(0x3ce38eb0u),asfloat(0x3ce38eb0u),asfloat(0x3ce38eb0u)))).x;
    // 114: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 115: max r2.w, r2.w, l(0x00000000)
    r2.w = (max(r2.wwww,float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).w;
    // 116: min r4.xy, r2.wwww, l(0x3f800000,0x40400000,0x00000000,0x00000000)
    r4.xy = (min(r2.wwww,float4(asfloat(0x3f800000u),asfloat(0x40400000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 117: add r2.w, -r4.x, l(0x3f800000)
    r2.w = ((-(r4.xxxx))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
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
    // 125: max r4.w, r4.z, l(0x00000000)
    r4.w = (max(r4.zzzz,float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u)))).w;
    // 126: min r5.w, r4.w, l(0x3f800000)
    r5.w = (min(r4.wwww,float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 127: add r4.z, r4.z, l(0x3f800000)
    r4.z = ((r4.zzzz)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 128: mad r4.z, r4.z, l(0x3f000000), -r5.w
    r4.z = ((r4.zzzz)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))+(-(r5.wwww))).z;
    // 129: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 130: mad r2.w, -r0.z, r2.w, l(0x3f800000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 131: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 132: mul_sat r6.xy, r6.xzxx, cb0[14].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[14].yyyy))).xy;
    // 133: add r6.xy, -r6.xyxx, l(0x3f800000,0x3f800000,0x00000000,0x00000000)
    r6.xy = ((-(r6.xyxx))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 134: add_sat r6.y, r6.y, -cb0[14].z
    r6.y = (saturate((r6.yyyy)+(-(source[14].zzzz)))).y;
    // 135: lt r6.z, r6.y, l(0x358637bd)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu)))) * 0xffffffffu)).z;
    // 136: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 137: mul r6.y, r6.y, cb0[14].w
    r6.y = ((r6.yyyy)*(source[14].wwww)).y;
    // 138: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 139: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 140: movc r6.x, r6.z, l(0x00000000), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))) : (r6.xxxx)).x;
    // 141: add r6.y, -r6.x, l(0x3f800000)
    r6.y = ((-(r6.xxxx))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).y;
    // 142: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 143: mad r6.z, r2.w, l(0x40000000), -r4.x
    r6.z = ((r2.wwww)*(float4(asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u)))+(-(r4.xxxx))).z;
    // 144: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 145: add r6.y, r6.y, l(0x3f800000)
    r6.y = ((r6.yyyy)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).y;
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
    // 154: mad r6.yzw, -cb0[17].yyyy, r9.xxyz, l(0x00000000,0x3f800000,0x3f800000,0x3f800000)
    r6.yzw = ((-(source[17].yyyy))*(r9.xxyz)+(float4(asfloat(0x00000000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).yzw;
    // 155: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 156: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 157: dp3 r0.z, r7.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).z;
    // 158: mad r9.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r9.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 159: mad r7.xyz, cb0[12].yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 160: dp3 r0.z, r7.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).z;
    // 161: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 162: mad r7.xyz, cb0[12].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 163: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, l(0x3f800000,0x3f800000,0x3f800000,0x00000000)
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u)))).xyz;
    // 164: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(0x3f800000,0x3f800000,0x3f800000,0x00000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u)))).xyz;
    // 165: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 166: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 167: dp3 r0.z, r8.yzwy, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.z = (dot((r8.yzwy).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).z;
    // 168: add r11.xyz, -r8.yzwy, r0.zzzz
    r11.xyz = ((-(r8.yzwy))+(r0.zzzz)).xyz;
    // 169: mad r8.xyz, cb0[12].yyyy, r11.xyzx, r8.yzwy
    r8.xyz = ((source[12].yyyy)*(r11.xyzx)+(r8.yzwy)).xyz;
    // 170: dp3 r0.z, r8.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).z;
    // 171: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 172: mad r8.xyz, cb0[12].zzzz, r11.xyzx, r8.xyzx
    r8.xyz = ((source[12].zzzz)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 173: mul r11.xyz, r7.xyzx, r8.xyzx
    r11.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 174: dp3 r0.z, r1.xywx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.z = (dot((r1.xywx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).z;
    // 175: add r1.x, r1.z, l(0x3f800000)
    r1.x = ((r1.zzzz)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).x;
    // 176: mul r1.x, r1.x, l(0x3f000000)
    r1.x = ((r1.xxxx)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).x;
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
    // 182: dp3 r0.z, r2.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).z;
    // 183: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 184: mad r2.xyz, cb0[12].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 185: dp3 r0.z, r2.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).z;
    // 186: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 187: mad r2.xyz, cb0[12].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 188: dp3 r0.z, r2.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).z;
    // 189: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 190: mul r13.xyz, r13.xyzx, cb0[14].xxxx
    r13.xyz = ((r13.xyzx)*(source[14].xxxx)).xyz;
    // 191: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t4.xyzw, s5, l(0x00000000)
    r14.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyzw;
    // 192: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 193: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 194: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 195: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 196: max r13.xyz, |r2.xyzx|, l(0x358637bd,0x358637bd,0x358637bd,0x00000000)
    r13.xyz = (max(abs(r2.xyzx),float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x00000000u)))).xyz;
    // 197: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 198: mul r13.xyz, r13.xyzx, l(0x3ee8ba1f,0x3ee8ba1f,0x3ee8ba1f,0x00000000)
    r13.xyz = ((r13.xyzx)*(float4(asfloat(0x3ee8ba1fu),asfloat(0x3ee8ba1fu),asfloat(0x3ee8ba1fu),asfloat(0x00000000u)))).xyz;
    // 199: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 200: dp3 r0.z, r13.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).z;
    // 201: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 202: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 203: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 204: min r0.z, r0.z, l(0x3f800000)
    r0.z = (min(r0.zzzz,float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 205: mad r1.w, -r0.z, r0.z, l(0x3f800000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 206: max r1.w, r1.w, l(0x3a83126f)
    r1.w = (max(r1.wwww,float4(asfloat(0x3a83126fu),asfloat(0x3a83126fu),asfloat(0x3a83126fu),asfloat(0x3a83126fu)))).w;
    // 207: div r1.w, cb0[15].y, r1.w
    r1.w = ((source[15].yyyy)/(r1.wwww)).w;
    // 208: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 209: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 210: add r1.w, -r0.z, l(0x3f800000)
    r1.w = ((-(r0.zzzz))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
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
    // 216: dp3 r1.w, r1.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 217: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 218: mad r1.xyz, cb0[12].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 219: dp3 r1.w, r1.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 220: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 221: mad r1.xyz, cb0[12].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 222: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 223: mul r1.w, cb0[2].z, l(0x3fc00000)
    r1.w = ((source[2].zzzz)*(float4(asfloat(0x3fc00000u),asfloat(0x3fc00000u),asfloat(0x3fc00000u),asfloat(0x3fc00000u)))).w;
    // 224: add r4.z, -cb0[2].w, l(0x3f800000)
    r4.z = ((-(source[2].wwww))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 225: mul r4.z, r4.z, cb0[16].z
    r4.z = ((r4.zzzz)*(source[16].zzzz)).z;
    // 226: mul r4.z, r4.z, l(0x40c90fdb)
    r4.z = ((r4.zzzz)*(float4(asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu),asfloat(0x40c90fdbu)))).z;
    // 227: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 228: add r4.z, r4.z, l(0x3f800000)
    r4.z = ((r4.zzzz)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).z;
    // 229: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 230: mad r1.w, r1.w, l(0x3f000000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))+(source[2].zzzz)).w;
    // 231: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 232: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 233: mul r9.z, r6.x, l(0x3e000000)
    r9.z = ((r6.xxxx)*(float4(asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u)))).z;
    // 234: mov r9.xw, l(0x00000000,0x00000000,0x00000000,0x00000000)
    r9.xw = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).xw;
    // 235: mul r9.y, cb0[2].y, cb0[8].y
    r9.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 236: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 237: mul r11.x, r6.x, l(0x3e000000)
    r11.x = ((r6.xxxx)*(float4(asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u)))).x;
    // 238: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 239: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 240: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 241: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t5.xyzw, s6, l(0x00000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyzw;
    // 242: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 243: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 244: mad r9.xyz, r9.xyzx, l(0x40000000,0x40000000,0x40000000,0x00000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x00000000u)))+(-(r1.xyzx))).xyz;
    // 245: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 246: mul r1.w, cb0[9].y, cb0[16].z
    r1.w = ((source[9].yyyy)*(source[16].zzzz)).w;
    // 247: mul r1.w, r1.w, l(0x3f20d97c)
    r1.w = ((r1.wwww)*(float4(asfloat(0x3f20d97cu),asfloat(0x3f20d97cu),asfloat(0x3f20d97cu),asfloat(0x3f20d97cu)))).w;
    // 248: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 249: mul r9.y, r1.w, l(0x3ca3d70b)
    r9.y = ((r1.wwww)*(float4(asfloat(0x3ca3d70bu),asfloat(0x3ca3d70bu),asfloat(0x3ca3d70bu),asfloat(0x3ca3d70bu)))).y;
    // 250: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 251: add r3.xy, -r3.xyxx, l(0x3f800000,0x3f800000,0x00000000,0x00000000)
    r3.xy = ((-(r3.xyxx))+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u),asfloat(0x00000000u)))).xy;
    // 252: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 253: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 254: mul r3.z, cb0[9].x, l(0x3a83126f)
    r3.z = ((source[9].xxxx)*(float4(asfloat(0x3a83126fu),asfloat(0x3a83126fu),asfloat(0x3a83126fu),asfloat(0x3a83126fu)))).z;
    // 255: mov r9.x, l(0x00000000)
    r9.x = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x;
    // 256: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 257: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 258: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 259: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 260: mul r3.x, r3.z, l(0x3e000000)
    r3.x = ((r3.zzzz)*(float4(asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u),asfloat(0x3e000000u)))).x;
    // 261: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0x00000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyzw;
    // 262: mul r3.w, r3.w, l(0x3f666666)
    r3.w = ((r3.wwww)*(float4(asfloat(0x3f666666u),asfloat(0x3f666666u),asfloat(0x3f666666u),asfloat(0x3f666666u)))).w;
    // 263: mad r3.xyz, r3.xyzx, l(0x40600000,0x40600000,0x40600000,0x00000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(asfloat(0x40600000u),asfloat(0x40600000u),asfloat(0x40600000u),asfloat(0x00000000u)))+(-(r1.xyzx))).xyz;
    // 264: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 265: add r1.w, r1.w, l(0x3f800000)
    r1.w = ((r1.wwww)+(float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u)))).w;
    // 266: mul r1.w, r1.w, l(0x3f000000)
    r1.w = ((r1.wwww)*(float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u)))).w;
    // 267: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 268: mul r9.xyz, r3.xyzx, cb0[9].zzzz
    r9.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 269: dp3 r1.w, r9.xyzx, l(0x3e99999a,0x3f170a3d,0x3de147ae,0x00000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(asfloat(0x3e99999au),asfloat(0x3f170a3du),asfloat(0x3de147aeu),asfloat(0x00000000u))).xyz).xxxx).w;
    // 270: mul r1.w, r1.w, l(0x40400000)
    r1.w = ((r1.wwww)*(float4(asfloat(0x40400000u),asfloat(0x40400000u),asfloat(0x40400000u),asfloat(0x40400000u)))).w;
    // 271: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 272: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 273: max r3.xyz, r5.xyzx, l(0x3f000000,0x3f000000,0x3f000000,0x00000000)
    r3.xyz = (max(r5.xyzx,float4(asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x3f000000u),asfloat(0x00000000u)))).xyz;
    // 274: min r3.xyz, r3.xyzx, l(0x3f800000,0x3f800000,0x3f800000,0x00000000)
    r3.xyz = (min(r3.xyzx,float4(asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x3f800000u),asfloat(0x00000000u)))).xyz;
    // 275: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 276: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 277: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 278: mul r0.y, r2.w, cb0[18].x
    r0.y = ((r2.wwww)*(source[18].xxxx)).y;
    // 279: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s7, l(0x00000000)
    r0.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).x)).xyzw).xyz;
    // 280: add r0.w, -cb0[18].y, l(0x40000000)
    r0.w = ((-(source[18].yyyy))+(float4(asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u),asfloat(0x40000000u)))).w;
    // 281: mad r0.w, r4.x, r0.w, cb0[18].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[18].yyyy)).w;
    // 282: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 283: max r0.xyz, |r0.xyzx|, l(0x358637bd,0x358637bd,0x358637bd,0x00000000)
    r0.xyz = (max(abs(r0.xyzx),float4(asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x358637bdu),asfloat(0x00000000u)))).xyz;
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
    // 289: min r0.xyz, r0.xyzx, l(0x40400000,0x40400000,0x40400000,0x00000000)
    r0.xyz = (min(r0.xyzx,float4(asfloat(0x40400000u),asfloat(0x40400000u),asfloat(0x40400000u),asfloat(0x00000000u)))).xyz;
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
    // 297: mov o0.w, l(0x00000000)
    output.targets[0].w = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).w;
    // 298: mov o1.xyzw, l(0x00000000,0x00000000,0x00000000,0x00000000)
    output.targets[1].xyzw = (float4(asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u),asfloat(0x00000000u))).xyzw;
    // 299: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 300: ret
    return output;
}


// source.character.monster-7493dcdfd412.v1 / source program 4e80581e68732f4fa62a8a5141625a83
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight22(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[6]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[8]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[9]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[12].w=(g_SourceCharacterTime.xxxx).x;
    source[13].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[13].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[16]=float4(input.lightColor,1.0);
    source[17].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: add r0.xyzw, v6.yzxy, cb0[0].yzxy
    r0.xyzw = ((v6.yzxy)+(source[0].yzxy)).xyzw;
    // 2: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[17].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[17].xxxx)) * 0xffffffffu)).x;
    // 3: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 4: mul r1.xyz, v6.yyyy, cb1[1].xywx
    r1.xyz = ((v6.yyyy)*(projection[1].xywx)).xyz;
    // 5: mad r1.xyz, cb1[0].xywx, v6.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v6.xxxx)+(r1.xyzx)).xyz;
    // 6: mad r1.xyz, cb1[2].xywx, v6.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v6.zzzz)+(r1.xyzx)).xyz;
    // 7: mad r1.xyz, cb1[3].xywx, v6.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v6.wwww)+(r1.xyzx)).xyz;
    // 8: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
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
    // 15: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 16: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 17: mul r2.xyz, r1.wwww, v5.xyzx
    r2.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 18: dp3 r1.w, v3.xyzx, v3.xyzx
    r1.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r3.xyz, r1.wwww, v3.xyzx
    r3.xyz = ((r1.wwww)*(v3.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v2.xyxx, t1.wxyz, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 22: mov_sat r4.x, r4.x
    r4.x = (saturate(r4.xxxx)).x;
    // 23: add r2.w, r4.x, l(-0.333300)
    r2.w = ((r4.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 24: lt r2.w, r2.w, l(0.000000)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) { output.discarded = true; return output; }
    // 26: mul r5.xyz, cb0[4].xyzx, cb0[13].wwww
    r5.xyz = ((source[4].xyzx)*(source[13].wwww)).xyz;
    // 27: mul r6.xyz, r4.yzwy, r5.xyzx
    r6.xyz = ((r4.yzwy)*(r5.xyzx)).xyz;
    // 28: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: mad r4.xyz, -r4.yzwy, r5.xyzx, r2.wwww
    r4.xyz = ((-(r4.yzwy))*(r5.xyzx)+(r2.wwww)).xyz;
    // 30: mad r5.xyz, cb0[11].xxxx, r4.xyzx, r6.xyzx
    r5.xyz = ((source[11].xxxx)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 31: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r7.xyz, -r5.xyzx, r2.wwww
    r7.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 33: mad r5.xyz, cb0[11].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[11].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 34: mad r7.xyz, cb0[2].wwww, cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[2].wwww)*(source[2].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 35: mad r8.xyz, cb0[3].wwww, cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[3].wwww)*(source[3].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 36: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 37: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 38: mul r2.w, cb0[5].z, l(1.500000)
    r2.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 39: add r3.w, -cb0[5].w, l(1.000000)
    r3.w = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: mul r3.w, r3.w, cb0[12].w
    r3.w = ((r3.wwww)*(source[12].wwww)).w;
    // 41: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 42: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 43: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 45: mad r2.w, r2.w, l(0.500000), cb0[5].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 46: frc r3.w, cb0[5].x
    r3.w = (frac(source[5].xxxx)).w;
    // 47: add r4.w, -r3.w, cb0[5].x
    r4.w = ((-(r3.wwww))+(source[5].xxxx)).w;
    // 48: mul r7.z, r4.w, l(0.125000)
    r7.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 49: mov r7.xw, l(0,0,0,0)
    r7.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 50: mul r7.y, cb0[5].y, cb0[6].y
    r7.y = ((source[5].yyyy)*(source[6].yyyy)).y;
    // 51: frc r4.w, v2.x
    r4.w = (frac(v2.xxxx)).w;
    // 52: mul r8.x, r4.w, l(0.125000)
    r8.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 53: mov r8.y, v2.y
    r8.y = (v2.yyyy).y;
    // 54: add r7.xy, r7.xyxx, r8.xyxx
    r7.xy = ((r7.xyxx)+(r8.xyxx)).xy;
    // 55: add r7.xy, r7.xyxx, r7.zwzz
    r7.xy = ((r7.xyxx)+(r7.zwzz)).xy;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 57: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 58: mul r2.w, r3.w, r7.w
    r2.w = ((r3.wwww)*(r7.wwww)).w;
    // 59: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xyzx))).xyz;
    // 60: mad r5.xyz, r2.wwww, r7.xyzx, r5.xyzx
    r5.xyz = ((r2.wwww)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 61: mul r2.w, cb0[7].y, cb0[12].w
    r2.w = ((source[7].yyyy)*(source[12].wwww)).w;
    // 62: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 63: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 64: mul r7.y, r2.w, l(0.020000)
    r7.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 65: add r0.xyzw, r0.xyzw, -cb0[1].yzxy
    r0.xyzw = ((r0.xyzw)+(-(source[1].yzxy))).xyzw;
    // 66: add r0.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 67: add r0.xy, -r0.zwzz, r0.xyxx
    r0.xy = ((-(r0.zwzz))+(r0.xyxx)).xy;
    // 68: mad r0.xy, cb0[7].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[7].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 69: mul r0.z, cb0[7].x, l(0.001000)
    r0.z = ((source[7].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 70: mov r7.x, l(0)
    r7.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 71: mad r0.xy, r0.zzzz, r0.xyxx, r7.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(r7.xyxx)).xy;
    // 72: dp2 r0.z, cb0[8].xyxx, r0.xyxx
    r0.z = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 73: dp2 r0.y, cb0[9].xyxx, r0.xyxx
    r0.y = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 74: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 75: mul r0.x, r0.z, l(0.125000)
    r0.x = ((r0.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 77: mul r0.w, r0.w, l(0.900000)
    r0.w = ((r0.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 78: mad r0.xyz, r0.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r5.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r5.xyzx))).xyz;
    // 79: mad r0.xyz, r0.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 80: add r0.w, r2.w, l(1.000000)
    r0.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 82: mul_sat r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (saturate((r0.xyzx)*(r0.wwww))).xyz;
    // 83: mul r7.xyz, r0.xyzx, cb0[7].zzzz
    r7.xyz = ((r0.xyzx)*(source[7].zzzz)).xyz;
    // 84: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 85: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 86: mad r0.xyz, cb0[7].zzzz, r0.xyzx, -r5.xyzx
    r0.xyz = ((source[7].zzzz)*(r0.xyzx)+(-(r5.xyzx))).xyz;
    // 87: mad r0.xyz, r0.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 88: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 89: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 90: dp2 r0.w, r5.xyxx, r5.xyxx
    r0.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 91: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 93: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 94: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 95: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 96: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 97: div r7.xyz, r5.xyzx, r0.wwww
    r7.xyz = ((r5.xyzx)/(r0.wwww)).xyz;
    // 98: dp3_sat r0.w, r7.xyzx, r3.xyzx
    r0.w = (saturate(dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx)).w;
    // 99: mul r8.xyz, r1.xyzx, r0.wwww
    r8.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 100: mad r4.xyz, cb0[14].yyyy, r4.xyzx, r6.xyzx
    r4.xyz = ((source[14].yyyy)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 101: mul r4.xyz, r4.xyzx, cb0[14].zzzz
    r4.xyz = ((r4.xyzx)*(source[14].zzzz)).xyz;
    // 102: mad r6.xyz, v3.xyzx, r1.wwww, r2.xyzx
    r6.xyz = ((v3.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 103: dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 104: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 105: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 106: dp3 r1.w, r6.xyzx, r7.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 107: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 108: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 109: mul r1.w, r1.w, cb0[14].w
    r1.w = ((r1.wwww)*(source[14].wwww)).w;
    // 110: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 111: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 113: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 114: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 116: mul r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 117: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 118: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 119: min r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 120: mad r0.xyz, r0.xyzx, r8.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 121: mul r1.xyz, cb0[10].xyzx, cb0[15].xxxx
    r1.xyz = ((source[10].xyzx)*(source[15].xxxx)).xyz;
    // 122: add r0.w, -|r2.z|, l(1.000000)
    r0.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: dp3 r1.w, r7.xyzx, r2.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 124: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 126: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 127: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 128: mul r0.w, r0.w, cb0[15].y
    r0.w = ((r0.wwww)*(source[15].yyyy)).w;
    // 129: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 130: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 131: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 132: mov_sat r0.w, -r3.z
    r0.w = (saturate(-(r3.zzzz))).w;
    // 133: mad r2.xyz, r0.wwww, r1.xyzx, -r1.xyzx
    r2.xyz = ((r0.wwww)*(r1.xyzx)+(-(r1.xyzx))).xyz;
    // 134: mad r1.xyz, cb0[10].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[10].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 135: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 136: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 137: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 138: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 139: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 140: mul o0.xyz, r0.xyzx, cb0[16].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[16].xyzx)).xyz;
    // 141: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 142: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 143: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 144: ret
    return output;
}

// source.character.monster-d9d6c02905c3.v1 / source program 70c5c49c6e9e984d883a961f8f774c8d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight23(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 292: mul r0.y, r2.w, cb0[22].x
    r0.y = ((r2.wwww)*(source[22].xxxx)).y;
    // 293: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
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

// source.character.monster-d621a47e69ad.v1 / source program 70f1c24cc664dd42b7939ba6476f35d8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight24(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].y=(g_SourceCharacterTime.xxxx).x;
    source[19].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[22]=float4(input.lightColor,1.0);
    source[23].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[23].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[23].xxxx)) * 0xffffffffu)).w;
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
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 67: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 68: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 69: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 71: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 72: mul r1.x, r1.x, cb0[16].y
    r1.x = ((r1.xxxx)*(source[16].yyyy)).x;
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
    // 88: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 89: rcp r0.x, cb0[16].z
    r0.x = (1.0/(source[16].zzzz)).x;
    // 90: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 91: mul r9.xyz, r4.xyzx, cb0[16].zzzz
    r9.xyz = ((r4.xyzx)*(source[16].zzzz)).xyz;
    // 92: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 93: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 94: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 95: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 96: mad r4.xyz, r9.xyzx, cb0[16].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[16].zzzz)+(r4.xyzx)).xyz;
    // 97: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 98: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 99: add r0.x, cb0[16].z, l(1.000000)
    r0.x = ((source[16].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
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
    // 120: mul r11.xyz, r9.xyzx, cb0[20].yyyy
    r11.xyz = ((r9.xyzx)*(source[20].yyyy)).xyz;
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
    // 132: mul_sat r6.xy, r6.xzxx, cb0[17].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[17].yyyy))).xy;
    // 133: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 134: add_sat r6.y, r6.y, -cb0[17].z
    r6.y = (saturate((r6.yyyy)+(-(source[17].zzzz)))).y;
    // 135: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 136: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 137: mul r6.y, r6.y, cb0[17].w
    r6.y = ((r6.yyyy)*(source[17].wwww)).y;
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
    // 148: mul r0.z, r0.z, cb0[20].w
    r0.z = ((r0.zzzz)*(source[20].wwww)).z;
    // 149: mad r6.y, cb0[20].z, r6.y, -r4.z
    r6.y = ((source[20].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 150: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 151: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 152: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 153: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 154: mad r6.yzw, -cb0[20].yyyy, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[20].yyyy))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 155: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 156: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 157: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 158: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 159: mad r7.xyz, r9.xxxx, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.xxxx)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 160: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 161: mad r7.xyz, r9.yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 162: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 163: mad r7.xyz, r9.zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 164: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 165: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 166: mad r7.xyz, cb0[15].yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 167: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 168: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 169: mad r7.xyz, cb0[15].zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 170: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 171: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 173: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 174: dp3 r0.z, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 175: add r12.xyz, -r8.yzwy, r0.zzzz
    r12.xyz = ((-(r8.yzwy))+(r0.zzzz)).xyz;
    // 176: mad r8.xyz, cb0[15].yyyy, r12.xyzx, r8.yzwy
    r8.xyz = ((source[15].yyyy)*(r12.xyzx)+(r8.yzwy)).xyz;
    // 177: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 178: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 179: mad r8.xyz, cb0[15].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 180: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 181: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 182: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 183: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 184: add r1.yzw, -cb0[9].xxyz, cb0[10].xxyz
    r1.yzw = ((-(source[9].xxyz))+(source[10].xxyz)).yzw;
    // 185: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[9].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[9].xyzx)).xyz;
    // 186: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 187: mul r1.xyz, r1.xyzx, cb0[16].wwww
    r1.xyz = ((r1.xyzx)*(source[16].wwww)).xyz;
    // 188: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 189: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 190: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 191: mad r2.xyz, cb0[15].yyyy, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].yyyy)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 192: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 194: mad r2.xyz, cb0[15].zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 195: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 197: mul r14.xyz, r14.xyzx, cb0[17].xxxx
    r14.xyz = ((r14.xyzx)*(source[17].xxxx)).xyz;
    // 198: add r0.z, r9.y, r9.x
    r0.z = ((r9.yyyy)+(r9.xxxx)).z;
    // 199: add r0.z, r9.z, r0.z
    r0.z = ((r9.zzzz)+(r0.zzzz)).z;
    // 200: add_sat r0.z, r9.w, r0.z
    r0.z = (saturate((r9.wwww)+(r0.zzzz))).z;
    // 201: mad r2.xyz, r0.zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 202: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 203: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 204: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 205: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 206: dp3 r0.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 207: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 208: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 209: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 210: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 211: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 212: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 213: div r1.w, cb0[18].y, r1.w
    r1.w = ((source[18].yyyy)/(r1.wwww)).w;
    // 214: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 215: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 216: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 217: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 218: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 219: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 220: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 221: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 222: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 223: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 224: mad r1.xyz, cb0[15].yyyy, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].yyyy)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 225: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 226: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 227: mad r1.xyz, cb0[15].zzzz, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].zzzz)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 228: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 229: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 230: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 231: mul r4.z, r4.z, cb0[19].y
    r4.z = ((r4.zzzz)*(source[19].yyyy)).z;
    // 232: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 233: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 234: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 235: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 236: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 237: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 238: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 239: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 240: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 241: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 242: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 243: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 244: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 245: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 246: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 247: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 248: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 249: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 250: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 251: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 252: mul r1.w, cb0[12].y, cb0[19].y
    r1.w = ((source[12].yyyy)*(source[19].yyyy)).w;
    // 253: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 254: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 255: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 256: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 257: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 258: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 259: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 260: mul r3.z, cb0[12].x, l(0.001000)
    r3.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 261: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 262: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 263: dp2 r3.z, cb0[13].xyxx, r3.xyxx
    r3.z = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 264: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 265: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 266: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 267: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 268: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 269: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 270: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 271: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 272: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 273: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 274: mul r9.xyz, r3.xyzx, cb0[12].zzzz
    r9.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 275: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 276: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 277: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 278: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 279: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 280: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 281: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 282: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 283: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 284: mul r0.y, r2.w, cb0[21].x
    r0.y = ((r2.wwww)*(source[21].xxxx)).y;
    // 285: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 286: add r0.w, -cb0[21].y, l(2.000000)
    r0.w = ((-(source[21].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 287: mad r0.w, r4.x, r0.w, cb0[21].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[21].yyyy)).w;
    // 288: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 289: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 290: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 291: mul r0.xyz, r0.xyzx, cb0[21].zzzz
    r0.xyz = ((r0.xyzx)*(source[21].zzzz)).xyz;
    // 292: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 293: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 294: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 295: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 296: mul r0.xyz, r0.xyzx, cb0[21].wwww
    r0.xyz = ((r0.xyzx)*(source[21].wwww)).xyz;
    // 297: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 298: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 299: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 300: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 301: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 302: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 303: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 304: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 305: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 306: ret
    return output;
}

// source.character.monster-be5bc0ded311.v1 / source program 9aee034d8132b443b793d2f5a72f49b4
