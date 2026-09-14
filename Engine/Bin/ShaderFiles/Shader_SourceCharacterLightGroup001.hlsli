
// source.character.classic-skin.v1 / source program c9cc424f33404648b77b8413ec31eb75
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].y=(g_SourceCharacterTime.xxxx).x;
    source[19].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[19].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
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
    // 253: mul r4.z, r4.z, cb0[19].y
    r4.z = ((r4.zzzz)*(source[19].yyyy)).z;
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
    // 274: mul r1.w, cb0[10].y, cb0[19].y
    r1.w = ((source[10].yyyy)*(source[19].yyyy)).w;
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

// source.character.classic-variation.v1 / source program 020eea4728f1c6419a53edc47d79a498
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight2(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[18].w=(g_SourceCharacterTime.xxxx).x;
    source[21]=float4(input.lightColor,1.0);
    source[22].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[22].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[22].xxxx)) * 0xffffffffu)).w;
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
    // 31: mul r9.xyzw, r8.xyzw, cb0[14].xyzw
    r9.xyzw = ((r8.xyzw)*(source[14].xyzw)).xyzw;
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
    // 45: mul r10.xy, r9.xyxx, cb0[15].xxxx
    r10.xy = ((r9.xyxx)*(source[15].xxxx)).xy;
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
    // 52: mad r9.xyz, cb0[15].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[15].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 75: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 76: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 77: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 78: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 79: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 80: mul r1.x, r1.x, cb0[16].y
    r1.x = ((r1.xxxx)*(source[16].yyyy)).x;
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
    // 97: rcp r0.x, cb0[16].z
    r0.x = (1.0/(source[16].zzzz)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r9.xyz, r4.xyzx, cb0[16].zzzz
    r9.xyz = ((r4.xyzx)*(source[16].zzzz)).xyz;
    // 100: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r9.xyzx, cb0[16].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[16].zzzz)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[16].z, l(1.000000)
    r0.x = ((source[16].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
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
    // 128: mul r11.xyz, r9.xyzx, cb0[19].yyyy
    r11.xyz = ((r9.xyzx)*(source[19].yyyy)).xyz;
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
    // 140: mul_sat r6.xy, r6.xzxx, cb0[17].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[17].yyyy))).xy;
    // 141: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 142: add_sat r6.y, r6.y, -cb0[17].z
    r6.y = (saturate((r6.yyyy)+(-(source[17].zzzz)))).y;
    // 143: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 144: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 145: mul r6.y, r6.y, cb0[17].w
    r6.y = ((r6.yyyy)*(source[17].wwww)).y;
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
    // 156: mul r0.z, r0.z, cb0[19].w
    r0.z = ((r0.zzzz)*(source[19].wwww)).z;
    // 157: mad r6.y, cb0[19].z, r6.y, -r4.z
    r6.y = ((source[19].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 158: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 159: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 160: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 161: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 162: mad r6.yzw, -cb0[19].yyyy, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[19].yyyy))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
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
    // 170: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 171: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 172: mad r7.xyz, cb0[15].yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 173: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 174: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 175: mad r7.xyz, cb0[15].zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 176: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 179: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 180: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 181: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 182: mad r8.xyz, cb0[15].yyyy, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].yyyy)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 183: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 184: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 185: mad r8.xyz, cb0[15].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 186: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 187: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 188: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 189: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 190: add r1.yzw, -cb0[8].xxyz, cb0[9].xxyz
    r1.yzw = ((-(source[8].xxyz))+(source[9].xxyz)).yzw;
    // 191: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[8].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[8].xyzx)).xyz;
    // 192: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 193: mul r1.xyz, r1.xyzx, cb0[16].wwww
    r1.xyz = ((r1.xyzx)*(source[16].wwww)).xyz;
    // 194: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 195: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r2.xyz, cb0[15].yyyy, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].yyyy)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 198: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 199: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 200: mad r2.xyz, cb0[15].zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 201: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 202: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 203: mul r14.xyz, r14.xyzx, cb0[17].xxxx
    r14.xyz = ((r14.xyzx)*(source[17].xxxx)).xyz;
    // 204: add r0.z, r9.y, r9.x
    r0.z = ((r9.yyyy)+(r9.xxxx)).z;
    // 205: add r0.z, r9.z, r0.z
    r0.z = ((r9.zzzz)+(r0.zzzz)).z;
    // 206: add_sat r0.z, r9.w, r0.z
    r0.z = (saturate((r9.wwww)+(r0.zzzz))).z;
    // 207: mad r2.xyz, r0.zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 208: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 209: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 210: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 211: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 212: dp3 r0.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 213: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 214: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 215: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 216: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 217: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 219: div r1.w, cb0[18].y, r1.w
    r1.w = ((source[18].yyyy)/(r1.wwww)).w;
    // 220: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 221: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 222: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 223: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 224: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 225: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 226: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 227: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 228: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 230: mad r1.xyz, cb0[15].yyyy, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].yyyy)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 231: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 232: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 233: mad r1.xyz, cb0[15].zzzz, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].zzzz)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 234: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 235: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 236: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 237: mul r4.z, r4.z, cb0[18].w
    r4.z = ((r4.zzzz)*(source[18].wwww)).z;
    // 238: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 239: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 240: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 241: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 242: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 243: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 244: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 245: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 246: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 247: mul r9.y, cb0[2].y, cb0[10].y
    r9.y = ((source[2].yyyy)*(source[10].yyyy)).y;
    // 248: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 249: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 250: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 251: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 252: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 253: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 254: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 255: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 256: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 257: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 258: mul r1.w, cb0[11].y, cb0[18].w
    r1.w = ((source[11].yyyy)*(source[18].wwww)).w;
    // 259: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 260: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 261: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 262: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 263: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 264: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 265: mad r3.xy, cb0[11].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[11].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 266: mul r3.z, cb0[11].x, l(0.001000)
    r3.z = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 267: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 268: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 269: dp2 r3.z, cb0[12].xyxx, r3.xyxx
    r3.z = (dot((source[12].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 270: dp2 r3.y, cb0[13].xyxx, r3.xyxx
    r3.y = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 271: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 272: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 273: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 274: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 275: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 276: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 277: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 278: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 279: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 280: mul r9.xyz, r3.xyzx, cb0[11].zzzz
    r9.xyz = ((r3.xyzx)*(source[11].zzzz)).xyz;
    // 281: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 282: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 283: mad r3.xyz, cb0[11].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 284: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 285: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 286: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 287: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 288: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 289: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 290: mul r0.y, r2.w, cb0[20].x
    r0.y = ((r2.wwww)*(source[20].xxxx)).y;
    // 291: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 292: add r0.w, -cb0[20].y, l(2.000000)
    r0.w = ((-(source[20].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 293: mad r0.w, r4.x, r0.w, cb0[20].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[20].yyyy)).w;
    // 294: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 295: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 296: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 297: mul r0.xyz, r0.xyzx, cb0[20].zzzz
    r0.xyz = ((r0.xyzx)*(source[20].zzzz)).xyz;
    // 298: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 299: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 300: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 301: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 302: mul r0.xyz, r0.xyzx, cb0[20].wwww
    r0.xyz = ((r0.xyzx)*(source[20].wwww)).xyz;
    // 303: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 304: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 305: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 306: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 307: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 308: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 309: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 310: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 311: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 312: ret
    return output;
}

// source.character.realpbr-avatar-v2.v1 / source program 6118ed17f8161044b11fc3f86ca1c833
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight3(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[18].x=(g_SourceCharacterTime.xxxx).x;
    source[23]=float4(input.lightColor,1.0);
    source[24].x=1.0;
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
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 47: mul r1.w, r6.x, cb0[17].z
    r1.w = ((r6.xxxx)*(source[17].zzzz)).w;
    // 48: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 49: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 50: add_sat r1.w, r1.w, cb0[17].w
    r1.w = (saturate((r1.wwww)+(source[17].wwww))).w;
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
    // 115: mad r6.xyw, cb0[16].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 116: dp3 r4.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 117: add r7.xyw, -r6.xyxw, r4.wwww
    r7.xyw = ((-(r6.xyxw))+(r4.wwww)).xyw;
    // 118: mad r6.xyw, cb0[16].zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
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
    // 125: mad r3.xyz, cb0[16].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 126: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 127: add r10.xyz, -r3.xyzx, r4.wwww
    r10.xyz = ((-(r3.xyzx))+(r4.wwww)).xyz;
    // 128: mad r3.xyz, cb0[16].zzzz, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].zzzz)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 129: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 130: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 131: mad r3.xyz, -r6.xywx, r3.xyzx, r4.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r4.wwww)).xyz;
    // 132: mad r3.xyz, cb0[16].yyyy, r3.xyzx, r10.xyzx
    r3.xyz = ((source[16].yyyy)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 133: dp3 r4.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 134: add r6.xyw, -r3.xyxz, r4.wwww
    r6.xyw = ((-(r3.xyxz))+(r4.wwww)).xyw;
    // 135: mad r3.xyz, cb0[16].zzzz, r6.xywx, r3.xyzx
    r3.xyz = ((source[16].zzzz)*(r6.xywx)+(r3.xyzx)).xyz;
    // 136: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 137: mul r4.w, cb0[8].z, l(1.500000)
    r4.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 138: add r5.w, -cb0[8].w, l(1.000000)
    r5.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r5.w, r5.w, cb0[18].x
    r5.w = ((r5.wwww)*(source[18].xxxx)).w;
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
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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

// source.character.classic-head.v1 / source program aa285f67042a5f42b4eef02bdfe0d632
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight4(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    r9.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterStampSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    r9.xz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterStampSampler, (r9.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
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
    r12.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterStampSampler, (r11.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
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
    r11.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterStampSampler, (r11.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
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
    r1.z = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterStampSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).z;
    // 213: add r10.z, -r10.x, l(1.990000)
    r10.z = ((-(r10.xxxx))+(float4(1.990000,1.990000,1.990000,1.990000))).z;
    // 214: sample_b_indexable(texture2d)(float,float,float,float) r8.w, r10.zyzz, t6.xyzw, s7, l(0.000000)
    r8.w = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterStampSampler, (r10.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
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
    r10.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterStampSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    r7.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterStampSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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

// source.character.eye.v1 / source program 2c76bd802b618647bd90fff42bb54069
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight5(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[7]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11].y=(g_SourceCharacterTime.xxxx).x;
    source[12]=float4(input.lightColor,1.0);
    source[13].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].xxxx)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 4: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 5: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s0
    r0.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 6: mul r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)*(r0.xyzx)).xyz;
    // 7: else
    } else {
    // 8: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 9: endif
    }
    // 10: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 13: dp3 r0.w, v4.xyzx, v4.xyzx
    r0.w = (dot((v4.xyzx).xyz,(v4.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r2.xyz, r0.wwww, v4.xyzx
    r2.xyz = ((r0.wwww)*(v4.xyzx)).xyz;
    // 16: mad r3.xyzw, v2.xywz, l(0.250000, 0.250000, 0.250000, 0.250000), l(0.375000, 0.375000, 0.375000, 0.375000)
    r3.xyzw = ((v2.xywz)*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.375000,0.375000,0.375000,0.375000))).xyzw;
    // 17: add r3.xyzw, r3.xyzw, -v2.xywz
    r3.xyzw = ((r3.xyzw)+(-(v2.xywz))).xyzw;
    // 18: mad r3.xyzw, cb0[8].zzzz, r3.xyzw, v2.xywz
    r3.xyzw = ((source[8].zzzz)*(r3.xyzw)+(v2.xywz)).xyzw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r3.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 20: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 22: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 25: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 27: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 28: div r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 29: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 30: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 31: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 32: dp3 r0.w, r5.xyzx, r1.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 33: mul r1.zw, r0.wwww, r5.xxxy
    r1.zw = ((r0.wwww)*(r5.xxxy)).zw;
    // 34: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), -r1.xxxy
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(-(r1.xxxy))).zw;
    // 35: max r0.xyz, r0.xyzx, cb0[11].wwww
    r0.xyz = (max(r0.xyzx,source[11].wwww)).xyz;
    // 36: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r3.zwzz, t1.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: mul r0.w, r5.w, cb0[4].w
    r0.w = ((r5.wwww)*(source[4].wwww)).w;
    // 39: add r6.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r6.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 40: mad r6.xyz, r0.wwww, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)+(source[3].xyzx)).xyz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v2.wzww, t2.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v2.wzww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 42: add r5.xyz, r5.xyzx, -r7.xyzx
    r5.xyz = ((r5.xyzx)+(-(r7.xyzx))).xyz;
    // 43: mad r5.xyz, r0.wwww, r5.xyzx, r7.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 44: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t3.xyzw, s5, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r0.w, r3.w, cb0[6].w
    r0.w = ((r3.wwww)*(source[6].wwww)).w;
    // 47: mad r6.xyz, cb0[5].wwww, cb0[6].xyzx, -cb0[5].xyzx
    r6.xyz = ((source[5].wwww)*(source[6].xyzx)+(-(source[5].xyzx))).xyz;
    // 48: mad r6.xyz, r0.wwww, r6.xyzx, cb0[5].xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)+(source[5].xyzx)).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v2.xyxx, t2.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 50: add r3.xyz, r3.xyzx, -r7.xyzx
    r3.xyz = ((r3.xyzx)+(-(r7.xyzx))).xyz;
    // 51: mad r3.xyz, r0.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 52: add r0.w, v3.x, l(0.500000)
    r0.w = ((v3.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 53: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 54: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 55: mad r0.w, cb0[11].x, r0.w, l(1.000000)
    r0.w = ((source[11].xxxx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: mad r3.xyz, r6.xyzx, r3.xyzx, -r5.xyzx
    r3.xyz = ((r6.xyzx)*(r3.xyzx)+(-(r5.xyzx))).xyz;
    // 57: mad r3.xyz, r0.wwww, r3.xyzx, r5.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 58: mul r0.w, cb0[0].z, l(1.500000)
    r0.w = ((source[0].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 59: add r2.w, -cb0[0].w, l(1.000000)
    r2.w = ((-(source[0].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: mul r2.w, r2.w, cb0[11].y
    r2.w = ((r2.wwww)*(source[11].yyyy)).w;
    // 61: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 62: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 63: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 65: mad r0.w, r0.w, l(0.500000), cb0[0].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[0].zzzz)).w;
    // 66: frc r2.w, cb0[0].x
    r2.w = (frac(source[0].xxxx)).w;
    // 67: add r3.w, -r2.w, cb0[0].x
    r3.w = ((-(r2.wwww))+(source[0].xxxx)).w;
    // 68: mul r5.z, r3.w, l(0.125000)
    r5.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 69: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 70: mov r5.y, cb0[0].y
    r5.y = (source[0].yyyy).y;
    // 71: mul r6.xz, v2.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r6.xz = ((v2.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 72: frc r3.w, r6.x
    r3.w = (frac(r6.xxxx)).w;
    // 73: mul r6.y, r3.w, l(0.125000)
    r6.y = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 74: mad r5.xy, r5.xyxx, cb0[7].xyxx, r6.yzyy
    r5.xy = ((r5.xyxx)*(source[7].xyxx)+(r6.yzyy)).xy;
    // 75: add r5.xy, r5.xyxx, r5.zwzz
    r5.xy = ((r5.xyxx)+(r5.zwzz)).xy;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t4.xyzw, s6, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 77: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 78: mul r0.w, r2.w, r5.w
    r0.w = ((r2.wwww)*(r5.wwww)).w;
    // 79: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 80: mad r3.xyz, r0.wwww, r5.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 81: dp3 r0.w, r4.xyzx, r2.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 82: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 83: min r2.x, r0.w, l(1.000000)
    r2.x = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 84: max r2.x, r2.x, cb0[11].w
    r2.x = (max(r2.xxxx,source[11].wwww)).x;
    // 85: add r2.y, cb0[0].y, cb0[0].x
    r2.y = ((source[0].yyyy)+(source[0].xxxx)).y;
    // 86: add r2.y, r2.y, cb0[0].z
    r2.y = ((r2.yyyy)+(source[0].zzzz)).y;
    // 87: round_pi_sat r2.y, r2.y
    r2.y = (saturate(ceil(r2.yyyy))).y;
    // 88: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 89: add r2.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r2.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 90: dp2 r2.z, r2.zwzz, r2.zwzz
    r2.z = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).z;
    // 91: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 92: add r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)+(r2.zzzz)).z;
    // 93: min r2.xz, r2.xxzx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r2.xz = (min(r2.xxzx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 94: max r2.z, r2.z, cb0[8].w
    r2.z = (max(r2.zzzz,source[8].wwww)).z;
    // 95: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 96: mul r1.xy, r1.xyxx, l(1.500000, 1.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(1.500000,1.500000,0.000000,0.000000))).xy;
    // 97: mad r1.xy, r2.zzzz, -r1.xyxx, v2.xyxx
    r1.xy = ((r2.zzzz)*(-(r1.xyxx))+(v2.xyxx)).xy;
    // 98: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 99: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 100: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 101: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 102: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 104: lt r1.y, r1.x, l(0.000001)
    r1.y = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 105: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 106: mul r1.x, r1.x, cb0[9].x
    r1.x = ((r1.xxxx)*(source[9].xxxx)).x;
    // 107: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 108: mul r1.x, r1.x, cb0[9].y
    r1.x = ((r1.xxxx)*(source[9].yyyy)).x;
    // 109: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 110: add r1.yz, r1.zzwz, -v2.xxyx
    r1.yz = ((r1.zzwz)+(-(v2.xxyx))).yz;
    // 111: mad r1.yz, cb0[10].xxxx, r1.yyzy, v2.xxyx
    r1.yz = ((source[10].xxxx)*(r1.yyzy)+(v2.xxyx)).yz;
    // 112: mad r1.yz, r1.yyzy, cb0[1].xxyx, cb0[2].xxyx
    r1.yz = ((r1.yyzy)*(source[1].xxyx)+(source[2].xxyx)).yz;
    // 113: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t5.wxyz, s2, l(0.000000)
    r1.yzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 114: mad r1.xyz, cb0[10].wwww, r1.yzwy, r1.xxxx
    r1.xyz = ((source[10].wwww)*(r1.yzwy)+(r1.xxxx)).xyz;
    // 115: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 116: min r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 117: mul r1.xyz, r1.xyzx, r2.yyyy
    r1.xyz = ((r1.xyzx)*(r2.yyyy)).xyz;
    // 118: mad r1.xyz, r2.xxxx, r3.xyzx, r1.xyzx
    r1.xyz = ((r2.xxxx)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 119: mul r2.xyz, r0.wwww, cb2[3].xyzx
    r2.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 120: mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 121: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 122: mul o0.xyz, r0.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 123: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 124: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 125: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 126: ret
    return output;
}

// source.character.eyelash.v1 / source program eed43b786b215f408ce12d58a3fe6e55
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight6(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[4]=float4(input.lightColor,1.0);
    source[0].xy=float2(1.0,1.0);
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

// source.character.hair.v1 / source program de8663cde8aa96469b31217fd40cff9a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight7(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 243: mul r1.xyz, r3.wwww, cb2[3].xyzx
    r1.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
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

// source.character.realpbr-weapon.v1 / source program 7c61b712a5030e45877ebd73573e1088
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight8(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14].z=(g_SourceCharacterTime.xxxx).x;
    source[20]=float4(input.lightColor,1.0);
    source[21].x=1.0;
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
    // 45: mul r2.w, r6.y, cb0[13].y
    r2.w = ((r6.yyyy)*(source[13].yyyy)).w;
    // 46: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 47: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 49: mul r6.xyw, cb0[3].xyxz, cb0[3].wwww
    r6.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // 50: max r7.xyw, r6.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r6.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 51: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 52: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 53: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 54: add r6.xyw, -r7.xyxw, r6.xyxw
    r6.xyw = ((-(r7.xyxw))+(r6.xyxw)).xyw;
    // 55: mad r6.xyw, r2.wwww, r6.xyxw, r7.xyxw
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r7.xyxw)).xyw;
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
    // 131: mul r6.xyw, r3.xyxz, r8.xyxz
    r6.xyw = ((r3.xyxz)*(r8.xyxz)).xyw;
    // 132: mad r3.xyz, -r8.xyzx, r3.xyzx, r3.xyzx
    r3.xyz = ((-(r8.xyzx))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 133: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 134: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 135: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 136: mov_sat r1.w, cb0[17].y
    r1.w = (saturate(source[17].yyyy)).w;
    // 137: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 138: add r3.w, -cb0[18].y, cb0[18].x
    r3.w = ((-(source[18].yyyy))+(source[18].xxxx)).w;
    // 139: mad r3.w, r9.x, r3.w, cb0[18].y
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[18].yyyy)).w;
    // 140: add r4.w, -r3.w, cb0[18].w
    r4.w = ((-(r3.wwww))+(source[18].wwww)).w;
    // 141: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 142: add r4.w, -r3.w, cb0[19].y
    r4.w = ((-(r3.wwww))+(source[19].yyyy)).w;
    // 143: mad r3.w, r9.z, r4.w, r3.w
    r3.w = ((r9.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 144: mul r3.w, r6.z, r3.w
    r3.w = ((r6.zzzz)*(r3.wwww)).w;
    // 145: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 146: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 148: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 149: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 151: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 152: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 153: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 154: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 155: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 156: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 157: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 159: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 160: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 163: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 164: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 166: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 167: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 168: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 169: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 170: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 171: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 172: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 173: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 174: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 176: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 177: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 178: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 179: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 180: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 181: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 182: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 183: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 184: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 185: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 186: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 187: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 188: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 189: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 190: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 192: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 193: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 194: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 195: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 196: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 197: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 198: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 199: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 200: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 201: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 202: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 203: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 204: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 205: mul_sat r6.xyz, cb0[12].xyzx, cb0[12].wwww
    r6.xyz = (saturate((source[12].xyzx)*(source[12].wwww))).xyz;
    // 206: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 207: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 208: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 209: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 210: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 211: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 212: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 213: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 214: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 215: mul r0.x, r0.x, cb0[19].z
    r0.x = ((r0.xxxx)*(source[19].zzzz)).x;
    // 216: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 217: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 218: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 219: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 220: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 221: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 222: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 223: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 224: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 225: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 226: ret
    return output;
}

// source.character.realpbr-weapon-variation.v1 / source program 5ed916e69e1c4940b3b55037c283930f
