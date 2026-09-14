
// source.character.classic-skin.v1 / source program 7d17844b3bb9a546828dcab0427fff0a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[16]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].y=(g_SourceCharacterTime.xxxx).x;
    source[22].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[22].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.wxyz, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 24: lt r1.z, |r1.y|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 25: log r1.y, |r1.y|
    r1.y = (log2(abs(r1.yyyy))).y;
    // 26: add r1.w, -cb0[18].y, cb0[18].x
    r1.w = ((-(source[18].yyyy))+(source[18].xxxx)).w;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 28: mad r1.w, r3.w, r1.w, cb0[18].y
    r1.w = ((r3.wwww)*(r1.wwww)+(source[18].yyyy)).w;
    // 29: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 30: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 31: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: movc r1.y, r1.z, l(0), r1.y
    r1.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 33: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 34: add r1.w, -r1.z, cb0[19].y
    r1.w = ((-(r1.zzzz))+(source[19].yyyy)).w;
    // 35: mad r1.z, r3.w, r1.w, r1.z
    r1.z = ((r3.wwww)*(r1.wwww)+(r1.zzzz)).z;
    // 36: mul r1.z, r1.z, cb0[19].z
    r1.z = ((r1.zzzz)*(source[19].zzzz)).z;
    // 37: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 38: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 39: add r0.x, |r0.x|, r1.x
    r0.x = ((abs(r0.xxxx))+(r1.xxxx)).x;
    // 40: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r1.xz, v4.xyxx, t0.xzyw, s0, l(0.000000)
    r1.xz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzyw).xz;
    // 42: mad r4.xyzw, r1.xzxz, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r4.xyzw = ((r1.xzxz)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 43: dp2 r1.x, r4.zwzz, r4.zwzz
    r1.x = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).x;
    // 44: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 46: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 47: add r5.z, r1.x, l(0.000010)
    r5.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: mul r5.xy, r4.xyxx, cb0[17].xxxx
    r5.xy = ((r4.xyxx)*(source[17].xxxx)).xy;
    // 49: mad r4.xy, cb0[17].wwww, r4.zwzz, -r5.xyxx
    r4.xy = ((source[17].wwww)*(r4.zwzz)+(-(r5.xyxx))).xy;
    // 50: mov r4.z, l(0)
    r4.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 51: mad r1.xzw, r3.wwww, r4.xxyz, r5.xxyz
    r1.xzw = ((r3.wwww)*(r4.xxyz)+(r5.xxyz)).xzw;
    // 52: add r4.xyz, -r1.xzwx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.xzwx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[19].xxxx, r4.xyzx, r1.xzwx
    r4.xyz = ((source[19].xxxx)*(r4.xyzx)+(r1.xzwx)).xyz;
    // 54: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 55: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 56: div r4.xyz, r4.xyzx, r2.wwww
    r4.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // 57: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 58: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 59: mul r5.xyz, r2.wwww, v1.xyzx
    r5.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 60: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 61: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 62: mul r6.xyz, r2.wwww, v0.xyzx
    r6.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 63: mul r7.xyz, r5.zxyz, r6.yzxy
    r7.xyz = ((r5.zxyz)*(r6.yzxy)).xyz;
    // 64: mad r7.xyz, r5.yzxy, r6.zxyz, -r7.xyzx
    r7.xyz = ((r5.yzxy)*(r6.zxyz)+(-(r7.xyzx))).xyz;
    // 65: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 66: dp3 r8.y, r7.xyzx, r4.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 67: dp3 r8.x, r6.xyzx, r4.xyzx
    r8.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 68: dp3 r8.z, r5.xyzx, r4.xyzx
    r8.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 69: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 70: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 71: mul r4.xyz, r2.wwww, v5.xyzx
    r4.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 72: mad r9.xyz, v5.xyzx, r2.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r2.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: dp3 r10.y, r7.xyzx, r4.xyzx
    r10.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 74: dp3 r10.x, r6.xyzx, r4.xyzx
    r10.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 75: dp3 r10.z, r5.xyzx, r4.xyzx
    r10.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 76: dp3 r2.w, r8.xyzx, r10.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 77: mul r8.xyz, r8.xyzx, r2.wwww
    r8.xyz = ((r8.xyzx)*(r2.wwww)).xyz;
    // 78: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 79: mov r8.w, -r8.x
    r8.w = (-(r8.xxxx)).w;
    // 80: dp2 r2.w, r8.ywyy, r8.ywyy
    r2.w = (dot((r8.ywyy).xy,(r8.ywyy).xy).xxxx).w;
    // 81: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 82: div r8.xy, r8.ywyy, r2.wwww
    r8.xy = ((r8.ywyy)/(r2.wwww)).xy;
    // 83: mad r2.w, -r8.z, l(0.250000), l(0.250000)
    r2.w = ((-(r8.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 84: add r4.w, r8.z, l(1.000000)
    r4.w = ((r8.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r4.w, r4.w, l(0.500000)
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 86: mad r8.xy, r2.wwww, r8.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r2.wwww)*(r8.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 87: sample_l_indexable(texture2d)(float,float,float,float) r8.xyz, r8.xyxx, t4.xyzw, s4, r0.x
    r8.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r8.xyxx).xy, (r0.xxxx).x)).xyzw).xyz;
    // 88: log r10.xyz, r8.xyzx
    r10.xyz = (log2(r8.xyzx)).xyz;
    // 89: rcp r0.x, cb0[19].w
    r0.x = (1.0/(source[19].wwww)).x;
    // 90: mul r11.xyz, r10.xyzx, r0.xxxx
    r11.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 91: mul r10.xyz, r10.xyzx, cb0[19].wwww
    r10.xyz = ((r10.xyzx)*(source[19].wwww)).xyz;
    // 92: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 93: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 94: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 95: mad r10.xyz, r10.xyzx, cb0[19].wwww, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[19].wwww)+(r11.xyzx)).xyz;
    // 96: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 97: mul r8.xyz, r8.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 98: add r0.x, cb0[19].w, l(1.000000)
    r0.x = ((source[19].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 99: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 100: dp3 r0.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r8.xyz, -cb0[8].xyzx, cb0[9].xyzx
    r8.xyz = ((-(source[8].xyzx))+(source[9].xyzx)).xyz;
    // 102: mad r8.xyz, r4.wwww, r8.xyzx, cb0[8].xyzx
    r8.xyz = ((r4.wwww)*(r8.xyzx)+(source[8].xyzx)).xyz;
    // 103: mul r8.xyz, r0.xxxx, r8.xyzx
    r8.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 104: mul r8.xyz, r8.xyzx, cb0[20].xxxx
    r8.xyz = ((r8.xyzx)*(source[20].xxxx)).xyz;
    // 105: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 106: add r10.xyz, -r2.xyzx, r0.xxxx
    r10.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 107: mad r2.yzw, cb0[18].zzzz, r10.xxyz, r2.xxyz
    r2.yzw = ((source[18].zzzz)*(r10.xxyz)+(r2.xxyz)).yzw;
    // 108: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 109: add r10.xyz, -r2.yzwy, r0.xxxx
    r10.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 110: mad r2.yzw, cb0[18].wwww, r10.xxyz, r2.yyzw
    r2.yzw = ((source[18].wwww)*(r10.xxyz)+(r2.yyzw)).yzw;
    // 111: dp3 r0.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 112: add r10.xyz, -r2.yzwy, r0.xxxx
    r10.xyz = ((-(r2.yzwy))+(r0.xxxx)).xyz;
    // 113: mul r10.xyz, r10.xyzx, cb0[20].yyyy
    r10.xyz = ((r10.xyzx)*(source[20].yyyy)).xyz;
    // 114: add r0.x, r3.y, r3.x
    r0.x = ((r3.yyyy)+(r3.xxxx)).x;
    // 115: add r0.x, r3.z, r0.x
    r0.x = ((r3.zzzz)+(r0.xxxx)).x;
    // 116: add_sat r0.x, r3.w, r0.x
    r0.x = (saturate((r3.wwww)+(r0.xxxx))).x;
    // 117: mad r2.yzw, r0.xxxx, r10.xxyz, r2.yyzw
    r2.yzw = ((r0.xxxx)*(r10.xxyz)+(r2.yyzw)).yzw;
    // 118: add r10.xyz, -r2.yzwy, r2.xxxx
    r10.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 119: mad r2.xyz, r3.wwww, r10.xyzx, r2.yzwy
    r2.xyz = ((r3.wwww)*(r10.xyzx)+(r2.yzwy)).xyz;
    // 120: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 121: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 122: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 123: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 124: dp3 r0.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 125: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 126: add r2.w, -cb0[21].z, cb0[21].y
    r2.w = ((-(source[21].zzzz))+(source[21].yyyy)).w;
    // 127: mad r2.w, r3.w, r2.w, cb0[21].z
    r2.w = ((r3.wwww)*(r2.wwww)+(source[21].zzzz)).w;
    // 128: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 129: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 130: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 131: mad r2.w, -r0.x, r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 133: div r2.w, cb0[21].w, r2.w
    r2.w = ((source[21].wwww)/(r2.wwww)).w;
    // 134: dp3 r3.x, r1.xzwx, r1.xzwx
    r3.x = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 135: sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // 136: div r1.xzw, r1.xxzw, r3.xxxx
    r1.xzw = ((r1.xxzw)/(r3.xxxx)).xzw;
    // 137: dp3 r3.x, r1.xzwx, r4.xyzx
    r3.x = (dot((r1.xzwx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 138: mul_sat r3.y, r3.x, cb0[20].z
    r3.y = (saturate((r3.xxxx)*(source[20].zzzz))).y;
    // 139: add r3.x, -|r3.x|, l(1.000000)
    r3.x = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 140: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 141: mul_sat r4.w, r4.z, cb0[20].z
    r4.w = (saturate((r4.zzzz)*(source[20].zzzz))).w;
    // 142: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: add_sat r4.w, r4.w, -cb0[20].w
    r4.w = (saturate((r4.wwww)+(-(source[20].wwww)))).w;
    // 144: log r5.w, r4.w
    r5.w = (log2(r4.wwww)).w;
    // 145: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 146: mul r5.w, r5.w, cb0[21].x
    r5.w = ((r5.wwww)*(source[21].xxxx)).w;
    // 147: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 148: mul r3.y, r3.y, r5.w
    r3.y = ((r3.yyyy)*(r5.wwww)).y;
    // 149: movc r3.y, r4.w, l(0), r3.y
    r3.y = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).y;
    // 150: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 151: mul r10.xyz, r8.xyzx, r2.wwww
    r10.xyz = ((r8.xyzx)*(r2.wwww)).xyz;
    // 152: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 154: mad r0.yzw, cb0[18].zzzz, r11.xxyz, r0.yyzw
    r0.yzw = ((source[18].zzzz)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 155: dp3 r2.w, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: add r11.xyz, -r0.yzwy, r2.wwww
    r11.xyz = ((-(r0.yzwy))+(r2.wwww)).xyz;
    // 157: mad r0.yzw, cb0[18].wwww, r11.xxyz, r0.yyzw
    r0.yzw = ((source[18].wwww)*(r11.xxyz)+(r0.yyzw)).yzw;
    // 158: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 159: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 160: mad r12.xyz, -cb0[4].wwww, cb0[4].xyzx, r2.wwww
    r12.xyz = ((-(source[4].wwww))*(source[4].xyzx)+(r2.wwww)).xyz;
    // 161: mad r11.xyz, cb0[18].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[18].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 162: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r12.xyz, -r11.xyzx, r2.wwww
    r12.xyz = ((-(r11.xyzx))+(r2.wwww)).xyz;
    // 164: mad r11.xyz, cb0[18].wwww, r12.xyzx, r11.xyzx
    r11.xyz = ((source[18].wwww)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 165: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 166: mad r13.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 168: mul r13.xyz, r11.xyzx, r12.xyzx
    r13.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 169: mad r11.xyz, -r11.xyzx, r12.xyzx, cb0[7].xyzx
    r11.xyz = ((-(r11.xyzx))*(r12.xyzx)+(source[7].xyzx)).xyz;
    // 170: mad r11.xyz, r3.wwww, r11.xyzx, r13.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)+(r13.xyzx)).xyz;
    // 171: mul r0.yzw, r0.yyzw, r11.xxyz
    r0.yzw = ((r0.yyzw)*(r11.xxyz)).yzw;
    // 172: mul r8.xyz, r8.xyzx, r0.yzwy
    r8.xyz = ((r8.xyzx)*(r0.yzwy)).xyz;
    // 173: mad r2.xyz, r2.xyzx, r10.xyzx, -r8.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r8.xyzx))).xyz;
    // 174: add r2.w, -r0.x, l(1.000000)
    r2.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: mul r2.w, r2.w, cb0[22].x
    r2.w = ((r2.wwww)*(source[22].xxxx)).w;
    // 176: mad r2.xyz, r2.wwww, r2.xyzx, r8.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 177: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 178: sqrt r3.w, r2.w
    r3.w = (sqrt(r2.wwww)).w;
    // 179: div r8.xyz, r9.xyzx, r3.wwww
    r8.xyz = ((r9.xyzx)/(r3.wwww)).xyz;
    // 180: dp3 r3.w, r8.xyzx, r4.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 181: add r4.x, -|r4.z|, l(1.000000)
    r4.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 182: mul r3.x, r3.x, r4.x
    r3.x = ((r3.xxxx)*(r4.xxxx)).x;
    // 183: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: mul r4.x, |r3.w|, |r3.w|
    r4.x = ((abs(r3.wwww))*(abs(r3.wwww))).x;
    // 185: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 186: mul r4.x, |r3.w|, r4.x
    r4.x = ((abs(r3.wwww))*(r4.xxxx)).x;
    // 187: lt r3.w, |r3.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 188: movc r3.w, r3.w, l(0), r4.x
    r3.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).w;
    // 189: add r4.x, r3.w, l(-0.027778)
    r4.x = ((r3.wwww)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).x;
    // 190: mad r3.w, r3.w, r4.x, l(0.027778)
    r3.w = ((r3.wwww)*(r4.xxxx)+(float4(0.027778,0.027778,0.027778,0.027778))).w;
    // 191: div_sat r2.w, r3.w, r2.w
    r2.w = (saturate((r3.wwww)/(r2.wwww))).w;
    // 192: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 194: mad r4.xyz, r1.yyyy, r2.xyzx, -r0.yzwy
    r4.xyz = ((r1.yyyy)*(r2.xyzx)+(-(r0.yzwy))).xyz;
    // 195: mad r0.xyz, r0.xxxx, r4.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r4.xyzx)+(r0.yzwy)).xyz;
    // 196: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 197: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 198: mad r0.xyz, cb0[18].zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((source[18].zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 199: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 200: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 201: mad r0.xyz, cb0[18].wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((source[18].wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 202: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 203: add r0.w, -cb0[3].w, l(1.000000)
    r0.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 204: mul r0.w, r0.w, cb0[22].y
    r0.w = ((r0.wwww)*(source[22].yyyy)).w;
    // 205: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 206: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 207: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 208: mul r1.y, cb0[3].z, l(1.500000)
    r1.y = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 209: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 210: mad r0.w, r0.w, l(0.500000), cb0[3].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 211: frc r1.y, v4.x
    r1.y = (frac(v4.xxxx)).y;
    // 212: mul r4.x, r1.y, l(0.125000)
    r4.x = ((r1.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 213: mul r8.y, cb0[3].y, cb0[13].y
    r8.y = ((source[3].yyyy)*(source[13].yyyy)).y;
    // 214: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 215: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 216: add r4.xy, r4.xyxx, r8.xyxx
    r4.xy = ((r4.xyxx)+(r8.xyxx)).xy;
    // 217: frc r1.y, cb0[3].x
    r1.y = (frac(source[3].xxxx)).y;
    // 218: add r2.w, -r1.y, cb0[3].x
    r2.w = ((-(r1.yyyy))+(source[3].xxxx)).w;
    // 219: mul r8.z, r2.w, l(0.125000)
    r8.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 220: add r4.xy, r4.xyxx, r8.zwzz
    r4.xy = ((r4.xyxx)+(r8.zwzz)).xy;
    // 221: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 222: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 223: mul r0.w, r1.y, r4.w
    r0.w = ((r1.yyyy)*(r4.wwww)).w;
    // 224: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 225: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 226: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 227: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 228: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 229: add r4.xy, -r4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r4.xy = ((-(r4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 230: add r4.xy, -r4.zwzz, r4.xyxx
    r4.xy = ((-(r4.zwzz))+(r4.xyxx)).xy;
    // 231: mad r4.xy, cb0[14].wwww, r4.xyxx, r4.zwzz
    r4.xy = ((source[14].wwww)*(r4.xyxx)+(r4.zwzz)).xy;
    // 232: mul r0.w, cb0[14].y, cb0[22].y
    r0.w = ((source[14].yyyy)*(source[22].yyyy)).w;
    // 233: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 234: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 235: mul r8.y, r0.w, l(0.020000)
    r8.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 236: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 238: mul r2.w, cb0[14].x, l(0.001000)
    r2.w = ((source[14].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 239: mov r8.x, l(0)
    r8.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 240: mad r4.xy, r2.wwww, r4.xyxx, r8.xyxx
    r4.xy = ((r2.wwww)*(r4.xyxx)+(r8.xyxx)).xy;
    // 241: dp2 r2.w, cb0[15].xyxx, r4.xyxx
    r2.w = (dot((source[15].xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 242: dp2 r4.y, cb0[16].xyxx, r4.xyxx
    r4.y = (dot((source[16].xyxx).xy,(r4.xyxx).xy).xxxx).y;
    // 243: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 244: mul r4.x, r2.w, l(0.125000)
    r4.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 245: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 246: mad r4.xyz, r4.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 247: mul r2.w, r4.w, l(0.900000)
    r2.w = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 248: mad r4.xyz, r2.wwww, r4.xyzx, r0.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 249: mul_sat r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = (saturate((r0.wwww)*(r4.xyzx))).xyz;
    // 250: mad r8.xyz, cb0[14].zzzz, r4.xyzx, -r0.xyzx
    r8.xyz = ((source[14].zzzz)*(r4.xyzx)+(-(r0.xyzx))).xyz;
    // 251: mul r4.xyz, r4.xyzx, cb0[14].zzzz
    r4.xyz = ((r4.xyzx)*(source[14].zzzz)).xyz;
    // 252: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 253: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 254: mad r0.xyz, r0.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 255: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 256: add r0.w, -r3.z, r3.y
    r0.w = ((-(r3.zzzz))+(r3.yyyy)).w;
    // 257: mad r4.xyz, r3.yyyy, cb0[11].xyzx, -cb0[11].xyzx
    r4.xyz = ((r3.yyyy)*(source[11].xyzx)+(-(source[11].xyzx))).xyz;
    // 258: mad r4.xyz, cb0[11].wwww, r4.xyzx, cb0[11].xyzx
    r4.xyz = ((source[11].wwww)*(r4.xyzx)+(source[11].xyzx)).xyz;
    // 259: mad r0.w, cb0[10].w, r0.w, r3.z
    r0.w = ((source[10].wwww)*(r0.wwww)+(r3.zzzz)).w;
    // 260: mad r3.yzw, r0.wwww, cb0[10].xxyz, r4.xxyz
    r3.yzw = ((r0.wwww)*(source[10].xxyz)+(r4.xxyz)).yzw;
    // 261: mul r4.xyz, r2.xyzx, r1.yyyy
    r4.xyz = ((r2.xyzx)*(r1.yyyy)).xyz;
    // 262: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: mad r2.xyz, -r1.yyyy, r2.xyzx, r0.wwww
    r2.xyz = ((-(r1.yyyy))*(r2.xyzx)+(r0.wwww)).xyz;
    // 264: mad r2.xyz, cb0[18].zzzz, r2.xyzx, r4.xyzx
    r2.xyz = ((source[18].zzzz)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 265: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 267: mad r2.xyz, cb0[18].wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((source[18].wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 268: mad r2.xyz, r2.xyzx, r12.xyzx, r3.yzwy
    r2.xyz = ((r2.xyzx)*(r12.xyzx)+(r3.yzwy)).xyz;
    // 269: log r0.w, |r3.x|
    r0.w = (log2(abs(r3.xxxx))).w;
    // 270: lt r1.y, |r3.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 271: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 272: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 273: mul r3.xyz, r0.wwww, cb0[12].xyzx
    r3.xyz = ((r0.wwww)*(source[12].xyzx)).xyz;
    // 274: movc r3.xyz, r1.yyyy, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 275: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 276: add r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)+(source[2].xyzx)).xyz;
    // 277: dp3 r0.w, r1.xzwx, r1.xzwx
    r0.w = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).w;
    // 278: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 279: mul r1.xyz, r0.wwww, r1.xzwx
    r1.xyz = ((r0.wwww)*(r1.xzwx)).xyz;
    // 280: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 281: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 282: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 283: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 284: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 285: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 286: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 287: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 288: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 289: mad r2.xyz, r3.xyzx, r0.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 290: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 291: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 292: mad o0.xyz, r0.xyzx, cb0[25].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)+(r2.xyzx)).xyz;
    // 293: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 294: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 295: dp3 r0.x, r6.xyzx, r1.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 296: dp3 r0.z, r5.xyzx, r1.xyzx
    r0.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 297: dp3 r0.y, r7.xyzx, r1.xyzx
    r0.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 298: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 299: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 300: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 301: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 302: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 303: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 304: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 305: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 306: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 307: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 308: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 309: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 310: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 311: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 312: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 313: ret
    return output;
}

// source.character.classic-variation.v1 / source program c4bf60162b08d14198d517cdf87d5684
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase2(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[17]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].w=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[18].xyzw
    r1.xyzw = ((r0.xyzw)*(source[18].xyzw)).xyzw;
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
    // 32: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 33: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 34: mul r0.w, r0.w, cb0[20].x
    r0.w = ((r0.wwww)*(source[20].xxxx)).w;
    // 35: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 36: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 38: sqrt r0.w, r0.z
    r0.w = (sqrt(r0.zzzz)).w;
    // 39: mul r0.w, r0.w, cb0[20].y
    r0.w = ((r0.wwww)*(source[20].yyyy)).w;
    // 40: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 41: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 42: add r0.x, r0.y, |r0.x|
    r0.x = ((r0.yyyy)+(abs(r0.xxxx))).x;
    // 43: round_ni r0.x, r0.x
    r0.x = (floor(r0.xxxx)).x;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r0.yw, v4.xyxx, t0.zxwy, s0, l(0.000000)
    r0.yw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 45: mad r0.yw, r0.yyyw, l(0.000000, 2.000000, 0.000000, 2.000000), l(0.000000, -1.000000, 0.000000, -1.000000)
    r0.yw = ((r0.yyyw)*(float4(0.000000,2.000000,0.000000,2.000000))+(float4(0.000000,-1.000000,0.000000,-1.000000))).yw;
    // 46: dp2 r1.w, r0.ywyy, r0.ywyy
    r1.w = (dot((r0.ywyy).xy,(r0.ywyy).xy).xxxx).w;
    // 47: mul r3.xy, r0.ywyy, cb0[19].xxxx
    r3.xy = ((r0.ywyy)*(source[19].xxxx)).xy;
    // 48: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 50: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 51: add r3.z, r0.y, l(0.000010)
    r3.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r4.xyz, cb0[19].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[19].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 54: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 55: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 56: div r4.xyz, r4.xyzx, r0.yyyy
    r4.xyz = ((r4.xyzx)/(r0.yyyy)).xyz;
    // 57: dp3 r0.y, v0.xyzx, v0.xyzx
    r0.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r5.xyz, r0.yyyy, v0.xyzx
    r5.xyz = ((r0.yyyy)*(v0.xyzx)).xyz;
    // 60: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 61: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r7.xyz, r0.yyyy, v1.xyzx
    r7.xyz = ((r0.yyyy)*(v1.xyzx)).xyz;
    // 64: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 65: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 66: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 67: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 68: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 69: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 70: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 71: mul r4.xyz, r0.yyyy, v5.xyzx
    r4.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 72: mad r9.xyz, v5.xyzx, r0.yyyy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r0.yyyy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 74: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 75: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 76: dp3 r0.y, r6.xyzx, r10.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 77: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 78: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 79: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 80: dp2 r0.y, r6.ywyy, r6.ywyy
    r0.y = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).y;
    // 81: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 82: div r0.yw, r6.yyyw, r0.yyyy
    r0.yw = ((r6.yyyw)/(r0.yyyy)).yw;
    // 83: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 84: add r2.w, r6.z, l(1.000000)
    r2.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 86: mad r0.yw, r1.wwww, r0.yyyw, l(0.000000, 0.500000, 0.000000, 0.500000)
    r0.yw = ((r1.wwww)*(r0.yyyw)+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 87: sample_l_indexable(texture2d)(float,float,float,float) r0.xyw, r0.ywyy, t5.xywz, s4, r0.x
    r0.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.ywyy).xy, (r0.xxxx).x)).xywz).xyw;
    // 88: log r6.xyz, r0.xywx
    r6.xyz = (log2(r0.xywx)).xyz;
    // 89: rcp r1.w, cb0[20].z
    r1.w = (1.0/(source[20].zzzz)).w;
    // 90: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 91: mul r6.xyz, r6.xyzx, cb0[20].zzzz
    r6.xyz = ((r6.xyzx)*(source[20].zzzz)).xyz;
    // 92: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 93: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 94: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 95: mad r6.xyz, r6.xyzx, cb0[20].zzzz, r10.xyzx
    r6.xyz = ((r6.xyzx)*(source[20].zzzz)+(r10.xyzx)).xyz;
    // 96: add r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)+(r6.xyxz)).xyw;
    // 97: mul r0.xyw, r0.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r0.xyw = ((r0.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 98: add r1.w, cb0[20].z, l(1.000000)
    r1.w = ((source[20].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.xyw, r0.xyxw, r1.wwww
    r0.xyw = ((r0.xyxw)*(r1.wwww)).xyw;
    // 100: dp3 r0.x, r0.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 101: add r6.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r6.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 102: mad r6.xyz, r2.wwww, r6.xyzx, cb0[9].xyzx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(source[9].xyzx)).xyz;
    // 103: mul r0.xyw, r0.xxxx, r6.xyxz
    r0.xyw = ((r0.xxxx)*(r6.xyxz)).xyw;
    // 104: mul r0.xyw, r0.xyxw, cb0[20].wwww
    r0.xyw = ((r0.xyxw)*(source[20].wwww)).xyw;
    // 105: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 106: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 107: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 108: dp3 r1.w, r3.xyzx, r4.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 109: mul_sat r2.w, r1.w, cb0[21].y
    r2.w = (saturate((r1.wwww)*(source[21].yyyy))).w;
    // 110: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul_sat r3.w, r4.z, cb0[21].y
    r3.w = (saturate((r4.zzzz)*(source[21].yyyy))).w;
    // 113: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: add_sat r3.w, r3.w, -cb0[21].z
    r3.w = (saturate((r3.wwww)+(-(source[21].zzzz)))).w;
    // 115: log r4.w, r3.w
    r4.w = (log2(r3.wwww)).w;
    // 116: lt r3.w, r3.w, l(0.000001)
    r3.w = (asfloat((uint4)((r3.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 117: mul r4.w, r4.w, cb0[21].w
    r4.w = ((r4.wwww)*(source[21].wwww)).w;
    // 118: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 119: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 120: movc r2.w, r3.w, l(0), r2.w
    r2.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 121: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 123: mad r2.xyz, cb0[19].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[19].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 124: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 126: mad r2.xyz, cb0[19].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 127: dp3 r3.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 128: add r6.xyz, -r2.xyzx, r3.wwww
    r6.xyz = ((-(r2.xyzx))+(r3.wwww)).xyz;
    // 129: mul r6.xyz, r6.xyzx, cb0[21].xxxx
    r6.xyz = ((r6.xyzx)*(source[21].xxxx)).xyz;
    // 130: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 131: add r3.w, r10.y, r10.x
    r3.w = ((r10.yyyy)+(r10.xxxx)).w;
    // 132: add r3.w, r10.z, r3.w
    r3.w = ((r10.zzzz)+(r3.wwww)).w;
    // 133: add_sat r3.w, r10.w, r3.w
    r3.w = (saturate((r10.wwww)+(r3.wwww))).w;
    // 134: mad r2.xyz, r3.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r3.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 135: max r6.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 136: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 137: mul r6.xyz, r6.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 138: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 139: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 140: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 141: mul r3.w, r3.w, cb0[22].x
    r3.w = ((r3.wwww)*(source[22].xxxx)).w;
    // 142: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 143: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mad r4.w, -r3.w, r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: max r4.w, r4.w, l(0.001000)
    r4.w = (max(r4.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 146: div r4.w, cb0[22].y, r4.w
    r4.w = ((source[22].yyyy)/(r4.wwww)).w;
    // 147: mul r4.w, r2.w, r4.w
    r4.w = ((r2.wwww)*(r4.wwww)).w;
    // 148: mul r6.xyz, r0.xywx, r4.wwww
    r6.xyz = ((r0.xywx)*(r4.wwww)).xyz;
    // 149: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 150: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 151: mad r1.xyz, cb0[19].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 152: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 153: add r11.xyz, -r1.xyzx, r4.wwww
    r11.xyz = ((-(r1.xyzx))+(r4.wwww)).xyz;
    // 154: mad r1.xyz, cb0[19].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 155: mul r11.xyz, cb0[4].xyzx, cb0[4].wwww
    r11.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 156: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 157: mad r11.xyz, r10.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r10.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 158: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 159: mad r10.xyw, r10.yyyy, r12.xyxz, r11.xyxz
    r10.xyw = ((r10.yyyy)*(r12.xyxz)+(r11.xyxz)).xyw;
    // 160: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 162: mad r10.xyw, cb0[19].yyyy, r11.xyxz, r10.xyxw
    r10.xyw = ((source[19].yyyy)*(r11.xyxz)+(r10.xyxw)).xyw;
    // 163: dp3 r4.w, r10.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 164: add r11.xyz, -r10.xywx, r4.wwww
    r11.xyz = ((-(r10.xywx))+(r4.wwww)).xyz;
    // 165: mad r10.xyw, cb0[19].zzzz, r11.xyxz, r10.xyxw
    r10.xyw = ((source[19].zzzz)*(r11.xyxz)+(r10.xyxw)).xyw;
    // 166: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 169: mul r10.xyw, r10.xyxw, r11.xyxz
    r10.xyw = ((r10.xyxw)*(r11.xyxz)).xyw;
    // 170: mul r1.xyz, r1.xyzx, r10.xywx
    r1.xyz = ((r1.xyzx)*(r10.xywx)).xyz;
    // 171: mul r0.xyw, r0.xyxw, r1.xyxz
    r0.xyw = ((r0.xyxw)*(r1.xyxz)).xyw;
    // 172: mad r2.xyz, r2.xyzx, r6.xyzx, -r0.xywx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(-(r0.xywx))).xyz;
    // 173: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: mul r4.w, r4.w, cb0[22].z
    r4.w = ((r4.wwww)*(source[22].zzzz)).w;
    // 175: mad r0.xyw, r4.wwww, r2.xyxz, r0.xyxw
    r0.xyw = ((r4.wwww)*(r2.xyxz)+(r0.xyxw)).xyw;
    // 176: dp3 r2.x, r9.xyzx, r9.xyzx
    r2.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 177: sqrt r2.y, r2.x
    r2.y = (sqrt(r2.xxxx)).y;
    // 178: div r6.xyz, r9.xyzx, r2.yyyy
    r6.xyz = ((r9.xyzx)/(r2.yyyy)).xyz;
    // 179: dp3 r2.y, r6.xyzx, r4.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 180: add r2.z, -|r4.z|, l(1.000000)
    r2.z = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 181: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 182: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 183: mul r2.z, |r2.y|, |r2.y|
    r2.z = ((abs(r2.yyyy))*(abs(r2.yyyy))).z;
    // 184: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 185: mul r2.z, r2.z, |r2.y|
    r2.z = ((r2.zzzz)*(abs(r2.yyyy))).z;
    // 186: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 187: movc r2.y, r2.y, l(0), r2.z
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).y;
    // 188: add r2.z, r2.y, l(-0.027778)
    r2.z = ((r2.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 189: mad r2.y, r2.y, r2.z, l(0.027778)
    r2.y = ((r2.yyyy)*(r2.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 190: div_sat r2.x, r2.y, r2.x
    r2.x = (saturate((r2.yyyy)/(r2.xxxx))).x;
    // 191: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 192: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 193: mad r2.xyz, r0.zzzz, r0.xywx, -r1.xyzx
    r2.xyz = ((r0.zzzz)*(r0.xywx)+(-(r1.xyzx))).xyz;
    // 194: mad r1.xyz, r3.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 195: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r1.xyz, cb0[19].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 198: dp3 r0.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 199: add r2.xyz, -r1.xyzx, r0.zzzz
    r2.xyz = ((-(r1.xyzx))+(r0.zzzz)).xyz;
    // 200: mad r1.xyz, cb0[19].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 201: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 202: add r0.z, -cb0[3].w, l(1.000000)
    r0.z = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 203: mul r0.z, r0.z, cb0[22].w
    r0.z = ((r0.zzzz)*(source[22].wwww)).z;
    // 204: mul r0.z, r0.z, l(6.283185)
    r0.z = ((r0.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 205: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 206: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 207: mul r2.x, cb0[3].z, l(1.500000)
    r2.x = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 208: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 209: mad r0.z, r0.z, l(0.500000), cb0[3].z
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).z;
    // 210: frc r2.x, v4.x
    r2.x = (frac(v4.xxxx)).x;
    // 211: mul r2.x, r2.x, l(0.125000)
    r2.x = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 212: mul r4.y, cb0[3].y, cb0[14].y
    r4.y = ((source[3].yyyy)*(source[14].yyyy)).y;
    // 213: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 214: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 215: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 216: frc r2.z, cb0[3].x
    r2.z = (frac(source[3].xxxx)).z;
    // 217: add r3.w, -r2.z, cb0[3].x
    r3.w = ((-(r2.zzzz))+(source[3].xxxx)).w;
    // 218: mul r4.z, r3.w, l(0.125000)
    r4.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 219: add r2.xy, r2.xyxx, r4.zwzz
    r2.xy = ((r2.xyxx)+(r4.zwzz)).xy;
    // 220: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.xyxx, t6.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 221: mul r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = ((r0.zzzz)*(r4.xyzx)).xyz;
    // 222: mul r0.z, r2.z, r4.w
    r0.z = ((r2.zzzz)*(r4.wwww)).z;
    // 223: add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 224: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 225: mad r1.xyz, r0.zzzz, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 226: add r4.xyzw, v7.yzxy, cb0[0].yzxy
    r4.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 227: add r4.xyzw, r4.xyzw, -cb0[1].yzxy
    r4.xyzw = ((r4.xyzw)+(-(source[1].yzxy))).xyzw;
    // 228: add r2.yz, -r4.xxyx, l(0.000000, 1.000000, 1.000000, 0.000000)
    r2.yz = ((-(r4.xxyx))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 229: add r2.yz, -r4.zzwz, r2.yyzy
    r2.yz = ((-(r4.zzwz))+(r2.yyzy)).yz;
    // 230: mad r2.yz, cb0[15].wwww, r2.yyzy, r4.zzwz
    r2.yz = ((source[15].wwww)*(r2.yyzy)+(r4.zzwz)).yz;
    // 231: mul r0.z, cb0[15].y, cb0[22].w
    r0.z = ((source[15].yyyy)*(source[22].wwww)).z;
    // 232: mul r0.z, r0.z, l(0.628319)
    r0.z = ((r0.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 233: sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // 234: mul r4.y, r0.z, l(0.020000)
    r4.y = ((r0.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 235: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 236: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 237: mul r3.w, cb0[15].x, l(0.001000)
    r3.w = ((source[15].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 238: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 239: mad r2.yz, r3.wwww, r2.yyzy, r4.xxyx
    r2.yz = ((r3.wwww)*(r2.yyzy)+(r4.xxyx)).yz;
    // 240: dp2 r3.w, cb0[16].xyxx, r2.yzyy
    r3.w = (dot((source[16].xyxx).xy,(r2.yzyy).xy).xxxx).w;
    // 241: dp2 r4.y, cb0[17].xyxx, r2.yzyy
    r4.y = (dot((source[17].xyxx).xy,(r2.yzyy).xy).xxxx).y;
    // 242: frc r2.y, r3.w
    r2.y = (frac(r3.wwww)).y;
    // 243: mul r4.x, r2.y, l(0.125000)
    r4.x = ((r2.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 244: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t6.xyzw, s5, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 245: mad r4.xyz, r4.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r4.xyz = ((r4.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 246: mul r2.y, r4.w, l(0.900000)
    r2.y = ((r4.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).y;
    // 247: mad r4.xyz, r2.yyyy, r4.xyzx, r1.xyzx
    r4.xyz = ((r2.yyyy)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 248: mul_sat r4.xyz, r0.zzzz, r4.xyzx
    r4.xyz = (saturate((r0.zzzz)*(r4.xyzx))).xyz;
    // 249: mad r6.xyz, cb0[15].zzzz, r4.xyzx, -r1.xyzx
    r6.xyz = ((source[15].zzzz)*(r4.xyzx)+(-(r1.xyzx))).xyz;
    // 250: mul r4.xyz, r4.xyzx, cb0[15].zzzz
    r4.xyz = ((r4.xyzx)*(source[15].zzzz)).xyz;
    // 251: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 252: mul r0.z, r0.z, l(3.000000)
    r0.z = ((r0.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 253: mad r1.xyz, r0.zzzz, r6.xyzx, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 254: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 255: add r0.z, r2.w, -r10.z
    r0.z = ((r2.wwww)+(-(r10.zzzz))).z;
    // 256: mad r2.yzw, r2.wwww, cb0[12].xxyz, -cb0[12].xxyz
    r2.yzw = ((r2.wwww)*(source[12].xxyz)+(-(source[12].xxyz))).yzw;
    // 257: mad r2.yzw, cb0[12].wwww, r2.yyzw, cb0[12].xxyz
    r2.yzw = ((source[12].wwww)*(r2.yyzw)+(source[12].xxyz)).yzw;
    // 258: mad r0.z, cb0[11].w, r0.z, r10.z
    r0.z = ((source[11].wwww)*(r0.zzzz)+(r10.zzzz)).z;
    // 259: mad r2.yzw, r0.zzzz, cb0[11].xxyz, r2.yyzw
    r2.yzw = ((r0.zzzz)*(source[11].xxyz)+(r2.yyzw)).yzw;
    // 260: mul r4.xyz, r0.xywx, r2.xxxx
    r4.xyz = ((r0.xywx)*(r2.xxxx)).xyz;
    // 261: dp3 r0.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 262: mad r0.xyz, -r2.xxxx, r0.xywx, r0.zzzz
    r0.xyz = ((-(r2.xxxx))*(r0.xywx)+(r0.zzzz)).xyz;
    // 263: mad r0.xyz, cb0[19].yyyy, r0.xyzx, r4.xyzx
    r0.xyz = ((source[19].yyyy)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 264: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 265: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 266: mad r0.xyz, cb0[19].zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 267: mad r0.xyz, r0.xyzx, r11.xyzx, r2.yzwy
    r0.xyz = ((r0.xyzx)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 268: log r0.w, |r1.w|
    r0.w = (log2(abs(r1.wwww))).w;
    // 269: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 270: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 271: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 272: mul r2.xyz, r0.wwww, cb0[13].xyzx
    r2.xyz = ((r0.wwww)*(source[13].xyzx)).xyz;
    // 273: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 274: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 275: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 276: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 277: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 278: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 279: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 280: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 281: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 282: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 283: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 284: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 285: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 286: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 287: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 288: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 289: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 290: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 291: mad o0.xyz, r1.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 292: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 293: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 294: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 295: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 296: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 297: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 298: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 299: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 300: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 301: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 302: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 303: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 304: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 305: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 306: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 307: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 308: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 309: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 310: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 311: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 312: ret
    return output;
}

// source.character.realpbr-avatar-v2.v1 / source program df8494f90d8a7640bcb7ede29704a7b4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase3(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[26] = g_SourceCharacterEnvironmentColor;
        source[27] = g_SourceCharacterEnvironmentRotation;
    }
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[21].x=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s5, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[16].xyzw
    r1.xyzw = ((r0.xyzw)*(source[16].xyzw)).xyzw;
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
    // 15: add r0.x, -cb0[8].w, l(1.000000)
    r0.x = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 16: mul r0.x, r0.x, cb0[21].x
    r0.x = ((r0.xxxx)*(source[21].xxxx)).x;
    // 17: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 18: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 19: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: mul r0.y, cb0[8].z, l(1.500000)
    r0.y = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 21: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 22: mad r0.x, r0.x, l(0.500000), cb0[8].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).x;
    // 23: frc r0.y, v4.x
    r0.y = (frac(v4.xxxx)).y;
    // 24: mul r2.x, r0.y, l(0.125000)
    r2.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 25: mul r3.y, cb0[8].y, cb0[15].y
    r3.y = ((source[8].yyyy)*(source[15].yyyy)).y;
    // 26: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 27: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 28: add r0.yz, r2.xxyx, r3.xxyx
    r0.yz = ((r2.xxyx)+(r3.xxyx)).yz;
    // 29: frc r0.w, cb0[8].x
    r0.w = (frac(source[8].xxxx)).w;
    // 30: add r1.w, -r0.w, cb0[8].x
    r1.w = ((-(r0.wwww))+(source[8].xxxx)).w;
    // 31: mul r3.z, r1.w, l(0.125000)
    r3.z = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 32: add r0.yz, r0.yyzy, r3.zzwz
    r0.yz = ((r0.yyzy)+(r3.zzwz)).yz;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.yzyy, t5.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 34: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 35: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 36: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 38: mad r2.xyz, cb0[19].yyyy, r2.xyzx, r1.xyzx
    r2.xyz = ((source[19].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 39: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 41: mad r2.xyz, cb0[19].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 42: mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
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
    // 51: mul r1.w, r6.y, cb0[18].y
    r1.w = ((r6.yyyy)*(source[18].yyyy)).w;
    // 52: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 53: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 55: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 56: mul r4.xyz, cb0[3].xyzx, cb0[3].wwww
    r4.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
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
    // 63: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 64: round_ni r5.yw, v7.xxxy
    r5.yw = (floor(v7.xxxy)).yw;
    // 65: dp2 r2.w, r5.ywyy, l(12.989800, 78.233002, 0.000000, 0.000000)
    r2.w = (dot((r5.ywyy).xy,(float4(12.989800,78.233002,0.000000,0.000000)).xy).xxxx).w;
    // 66: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 67: mul r2.w, r2.w, l(43758.546875)
    r2.w = ((r2.wwww)*(float4(43758.546875,43758.546875,43758.546875,43758.546875))).w;
    // 68: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 69: add r2.w, r2.w, l(-0.500000)
    r2.w = ((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 71: mad r2.w, r2.w, l(0.010000), r7.x
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))+(r7.xxxx)).w;
    // 72: lt r3.w, cb0[18].w, r2.w
    r3.w = (asfloat((uint4)((source[18].wwww)<(r2.wwww)) * 0xffffffffu)).w;
    // 73: lt r2.w, r2.w, cb0[18].z
    r2.w = (asfloat((uint4)((r2.wwww)<(source[18].zzzz)) * 0xffffffffu)).w;
    // 74: movc r2.w, r2.w, l(-1.000000), l(-0.000000)
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(-1.000000,-1.000000,-1.000000,-1.000000)) : (float4(-0.000000,-0.000000,-0.000000,-0.000000))).w;
    // 75: and r4.w, r3.w, l(0x3f800000)
    r4.w = (asfloat(asuint(r3.wwww) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).w;
    // 76: movc r3.w, r3.w, l(0), l(1.000000)
    r3.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: add r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)+(r3.wwww)).w;
    // 78: mad r3.xyz, r4.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r4.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 79: mul r4.xyz, cb0[5].xyzx, cb0[5].wwww
    r4.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 80: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 81: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 82: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 83: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 84: mad r4.xyz, r1.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 85: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 86: mad r3.xyz, r2.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 87: mul r4.xyz, cb0[6].xyzx, cb0[6].wwww
    r4.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 88: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 89: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 90: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 91: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 92: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 93: mad r4.xyz, r1.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 94: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 95: mad r3.xyz, r7.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 96: mul r4.xyz, cb0[7].xyzx, cb0[7].wwww
    r4.xyz = ((source[7].xyzx)*(source[7].wwww)).xyz;
    // 97: max r8.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 98: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 99: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 100: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 101: add r8.xyz, -r4.xyzx, r8.xyzx
    r8.xyz = ((-(r4.xyzx))+(r8.xyzx)).xyz;
    // 102: mad r4.xyz, r1.wwww, r8.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 103: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 104: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 105: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r4.xyz, -r3.xyzx, r3.wwww
    r4.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 107: mad r4.xyz, cb0[19].yyyy, r4.xyzx, r3.xyzx
    r4.xyz = ((source[19].yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 108: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 109: dp3 r3.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 110: add r3.xyz, -r4.xyzx, r3.xxxx
    r3.xyz = ((-(r4.xyzx))+(r3.xxxx)).xyz;
    // 111: mad r3.xyz, cb0[19].zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((source[19].zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 112: mad r4.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 113: mad r8.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 114: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 115: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 116: mul r8.xyz, r2.xyzx, r3.xyzx
    r8.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 117: dp3 r3.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 118: mad r2.xyz, -r3.xyzx, r2.xyzx, r3.wwww
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r3.wwww)).xyz;
    // 119: mad r2.xyz, cb0[19].yyyy, r2.xyzx, r8.xyzx
    r2.xyz = ((source[19].yyyy)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 120: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: add r3.xyz, -r2.xyzx, r3.xxxx
    r3.xyz = ((-(r2.xyzx))+(r3.xxxx)).xyz;
    // 122: mad r2.xyz, cb0[19].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 123: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 124: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 125: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 126: add r0.w, r0.y, r0.x
    r0.w = ((r0.yyyy)+(r0.xxxx)).w;
    // 127: add r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)+(r0.wwww)).w;
    // 128: mul r0.w, r0.w, l(0.333330)
    r0.w = ((r0.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 129: max r0.w, r0.w, cb0[21].z
    r0.w = (max(r0.wwww,source[21].zzzz)).w;
    // 130: min r0.w, r0.w, cb0[21].y
    r0.w = (min(r0.wwww,source[21].yyyy)).w;
    // 131: add r2.x, -r0.w, l(1.000000)
    r2.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 132: mad r0.w, r1.w, r2.x, r0.w
    r0.w = ((r1.wwww)*(r2.xxxx)+(r0.wwww)).w;
    // 133: mul_sat r3.w, r1.w, cb2[3].w
    r3.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 134: add r1.w, r0.w, l(-1.000000)
    r1.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 135: mad r1.w, cb0[22].x, r1.w, l(1.000000)
    r1.w = ((source[22].xxxx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mul r2.xyz, r0.xyzx, r1.wwww
    r2.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 137: mul r3.x, r6.x, cb0[20].z
    r3.x = ((r6.xxxx)*(source[20].zzzz)).x;
    // 138: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 139: movc r3.x, r5.x, l(0), r3.x
    r3.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 140: add_sat r3.x, r3.x, cb0[20].w
    r3.x = (saturate((r3.xxxx)+(source[20].wwww))).x;
    // 141: add r3.y, -r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 142: mul r4.xyz, r3.yyyy, cb0[14].xyzx
    r4.xyz = ((r3.yyyy)*(source[14].xyzx)).xyz;
    // 143: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 144: mad r0.xyz, r1.wwww, r0.xyzx, -r2.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 145: mad r0.xyz, r3.xxxx, r0.xyzx, r2.xyzx
    r0.xyz = ((r3.xxxx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 146: mul_sat r0.w, r0.w, r3.x
    r0.w = (saturate((r0.wwww)*(r3.xxxx))).w;
    // 147: add r2.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 148: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 149: mad_sat r4.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 150: dp3 r0.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 151: add r0.y, -cb0[23].y, cb0[23].x
    r0.y = ((-(source[23].yyyy))+(source[23].xxxx)).y;
    // 152: mad r0.y, r7.x, r0.y, cb0[23].y
    r0.y = ((r7.xxxx)*(r0.yyyy)+(source[23].yyyy)).y;
    // 153: add r0.z, -r0.y, cb0[23].w
    r0.z = ((-(r0.yyyy))+(source[23].wwww)).z;
    // 154: mad r0.y, r7.y, r0.z, r0.y
    r0.y = ((r7.yyyy)*(r0.zzzz)+(r0.yyyy)).y;
    // 155: add r0.z, -r0.y, cb0[24].y
    r0.z = ((-(r0.yyyy))+(source[24].yyyy)).z;
    // 156: mad r0.y, r7.z, r0.z, r0.y
    r0.y = ((r7.zzzz)*(r0.zzzz)+(r0.yyyy)).y;
    // 157: add r0.z, -r0.y, cb0[24].w
    r0.z = ((-(r0.yyyy))+(source[24].wwww)).z;
    // 158: mad r0.y, r2.w, r0.z, r0.y
    r0.y = ((r2.wwww)*(r0.zzzz)+(r0.yyyy)).y;
    // 159: mul r0.y, r6.z, r0.y
    r0.y = ((r6.zzzz)*(r0.yyyy)).y;
    // 160: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 161: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 162: movc r0.y, r5.z, l(0), r0.y
    r0.y = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 163: max r0.y, r0.y, cb0[0].x
    r0.y = (max(r0.yyyy,source[0].xxxx)).y;
    // 164: min r3.z, r0.y, l(1.000000)
    r3.z = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 165: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 166: mad r0.yz, r0.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r0.yz = ((r0.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 167: dp2 r1.w, r0.yzyy, r0.yzyy
    r1.w = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).w;
    // 168: mul r2.xy, r0.yzyy, cb0[18].xxxx
    r2.xy = ((r0.yzyy)*(source[18].xxxx)).xy;
    // 169: add r0.y, -r1.w, l(1.000000)
    r0.y = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 170: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 171: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 172: add r2.z, r0.y, l(0.000010)
    r2.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 173: dp3 r0.y, r2.xyzx, r2.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 174: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 175: div r2.xyz, r2.xyzx, r0.yyyy
    r2.xyz = ((r2.xyzx)/(r0.yyyy)).xyz;
    // 176: dp3 r0.y, r2.xyzx, r2.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 177: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 178: mul r5.xyz, r0.yyyy, r2.xyzx
    r5.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 179: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 180: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 181: mul r6.xyz, r0.yyyy, v5.xyzx
    r6.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 182: dp3 r0.y, r5.xyzx, r6.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 183: deriv_rtx_coarse r3.x, r0.y
    r3.x = (ddx_coarse(r0.yyyy)).x;
    // 184: deriv_rty_coarse r3.y, r0.y
    r3.y = (ddy_coarse(r0.yyyy)).y;
    // 185: dp2 r0.z, r3.xyxx, r3.xyxx
    r0.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 186: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 187: mad r0.z, r0.z, l(0.300000), r3.z
    r0.z = ((r0.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz)).z;
    // 188: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 189: min r3.y, r0.z, l(1.000000)
    r3.y = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 190: mul r0.z, r3.y, l(5.000000)
    r0.z = ((r3.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 191: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 192: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 193: mul r7.xyz, r1.wwww, v1.xyzx
    r7.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 194: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 195: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 196: mul r8.xyz, r1.wwww, v0.xyzx
    r8.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 197: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 198: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 199: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 200: mul r10.xyz, r0.yyyy, r5.xyzx
    r10.xyz = ((r0.yyyy)*(r5.xyzx)).xyz;
    // 201: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r6.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r6.xyzx))).xyz;
    // 202: dp3 r11.y, r9.xyzx, r10.xyzx
    r11.y = (dot((r9.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 203: dp3 r9.y, r9.xyzx, r5.xyzx
    r9.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 204: dp3 r11.x, r8.xyzx, r10.xyzx
    r11.x = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 205: dp3 r9.x, r8.xyzx, r5.xyzx
    r9.x = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 206: dp2 r8.z, r11.xyxx, cb0[27].xyxx
    r8.z = (dot((r11.xyxx).xy,(source[27].xyxx).xy).xxxx).z;
    // 207: mul r11.zw, cb0[27].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r11.zw = ((source[27].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 208: dp2 r8.x, r11.xyxx, r11.zwzz
    r8.x = (dot((r11.xyxx).xy,(r11.zwzz).xy).xxxx).x;
    // 209: dp2 r11.x, r9.xyxx, r11.zwzz
    r11.x = (dot((r9.xyxx).xy,(r11.zwzz).xy).xxxx).x;
    // 210: dp3 r8.y, r7.xyzx, r10.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 211: dp3 r11.y, r7.xyzx, r5.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 212: sample_l_indexable(texturecube)(float,float,float,float) r8.xyzw, r8.xyzx, t7.xyzw, s6, r0.z
    r8.xyzw = g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, r8.xyz, r0.z) : 0.f;
    // 213: mul r7.xyz, r8.xyzx, r8.wwww
    r7.xyz = ((r8.xyzx)*(r8.wwww)).xyz;
    // 214: mul r7.xyz, r7.xyzx, cb0[26].xyzx
    r7.xyz = ((r7.xyzx)*(source[26].xyzx)).xyz;
    // 215: mul r7.xyz, r7.xyzx, cb0[27].zzzz
    r7.xyz = ((r7.xyzx)*(source[27].zzzz)).xyz;
    // 216: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[26].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[26].wwww)).xyz;
    // 217: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 218: add r7.xyz, -r0.zzzz, r7.xyzx
    r7.xyz = ((-(r0.zzzz))+(r7.xyzx)).xyz;
    // 219: mad r7.xyz, r7.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r0.zzzz
    r7.xyz = ((r7.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r0.zzzz)).xyz;
    // 220: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 221: mad r1.w, r3.y, l(2.000000), l(2.000000)
    r1.w = ((r3.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 222: div r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)/(r1.wwww)).z;
    // 223: mad r0.z, r0.x, l(5.000000), r0.z
    r0.z = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.zzzz)).z;
    // 224: add_sat r0.z, r3.w, r0.z
    r0.z = (saturate((r3.wwww)+(r0.zzzz))).z;
    // 225: mad r2.w, r0.z, l(-2.000000), l(3.000000)
    r2.w = ((r0.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 226: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 227: mul r0.z, r0.z, r2.w
    r0.z = ((r0.zzzz)*(r2.wwww)).z;
    // 228: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 229: mul r0.z, r0.z, l(1.500000)
    r0.z = ((r0.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 230: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 231: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 232: mov_sat r4.w, cb0[22].y
    r4.w = (saturate(source[22].yyyy)).w;
    // 233: mad r8.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r8.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 234: mul r0.z, r4.w, l(0.080000)
    r0.z = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 235: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 236: mad r8.xyz, r3.wwww, r8.xyzx, r0.zzzz
    r8.xyz = ((r3.wwww)*(r8.xyzx)+(r0.zzzz)).xyz;
    // 237: mul_sat r0.z, r8.y, l(50.000000)
    r0.z = (saturate((r8.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 238: add r2.w, -r3.y, l(1.000000)
    r2.w = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: max r12.xyz, r8.xyzx, r2.wwww
    r12.xyz = (max(r8.xyzx,r2.wwww)).xyz;
    // 240: add r12.xyz, -r8.xyzx, r12.xyzx
    r12.xyz = ((-(r8.xyzx))+(r12.xyzx)).xyz;
    // 241: mul r12.xyz, r0.zzzz, r12.xyzx
    r12.xyz = ((r0.zzzz)*(r12.xyzx)).xyz;
    // 242: add r0.z, r0.y, l(1.000000)
    r0.z = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 243: mov_sat r0.y, r0.y
    r0.y = (saturate(r0.yyyy)).y;
    // 244: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 245: mul r0.y, r0.y, cb0[1].y
    r0.y = ((r0.yyyy)*(source[1].yyyy)).y;
    // 246: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 247: mad_sat r0.y, r0.y, cb0[1].w, cb0[1].z
    r0.y = (saturate((r0.yyyy)*(source[1].wwww)+(source[1].zzzz))).y;
    // 248: mul r0.y, r0.y, cb0[25].x
    r0.y = ((r0.yyyy)*(source[25].xxxx)).y;
    // 249: add r2.w, r10.z, l(1.000000)
    r2.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 250: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 251: add_sat r3.x, r0.z, -r2.w
    r3.x = (saturate((r0.zzzz)+(-(r2.wwww)))).x;
    // 252: sample_indexable(texture2d)(float,float,float,float) r13.xy, r3.xyxx, t6.xyzw, s7
    r13.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 253: mul r0.z, r3.y, r3.y
    r0.z = ((r3.yyyy)*(r3.yyyy)).z;
    // 254: add r2.w, r0.w, r3.x
    r2.w = ((r0.wwww)+(r3.xxxx)).w;
    // 255: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 256: mul r0.z, r0.z, r2.w
    r0.z = ((r0.zzzz)*(r2.wwww)).z;
    // 257: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 258: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 259: add_sat r0.z, r0.z, l(-1.000000)
    r0.z = (saturate((r0.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).z;
    // 260: mul r3.xyz, r8.xyzx, r13.yyyy
    r3.xyz = ((r8.xyzx)*(r13.yyyy)).xyz;
    // 261: mad r3.xyz, r12.xyzx, r13.xxxx, r3.xyzx
    r3.xyz = ((r12.xyzx)*(r13.xxxx)+(r3.xyzx)).xyz;
    // 262: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.y
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r13.yyyy)).w;
    // 263: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 264: mad r12.xyz, r8.xyzx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r8.xyzx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 265: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: mad r8.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r8.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 267: mul r13.xyz, r3.xyzx, r12.xyzx
    r13.xyz = ((r3.xyzx)*(r12.xyzx)).xyz;
    // 268: mad r3.xyz, -r3.xyzx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r3.xyzx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 269: mul r12.xyz, r7.xyzx, r13.xyzx
    r12.xyz = ((r7.xyzx)*(r13.xyzx)).xyz;
    // 270: mad r14.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r14.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 271: mad r15.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r15.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 272: mad r16.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r16.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 273: mad r15.xyz, r0.wwww, r15.xyzx, r16.xyzx
    r15.xyz = ((r0.wwww)*(r15.xyzx)+(r16.xyzx)).xyz;
    // 274: mad r14.xyz, r15.xyzx, r0.wwww, r14.xyzx
    r14.xyz = ((r15.xyzx)*(r0.wwww)+(r14.xyzx)).xyz;
    // 275: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 276: max r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = (max(r0.wwww,r14.xyzx)).xyz;
    // 277: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 278: dp2 r11.z, r9.xyxx, cb0[27].xyxx
    r11.z = (dot((r9.xyxx).xy,(source[27].xyxx).xy).xxxx).z;
    // 279: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 280: dp4 r15.x, cb0[28].xyzw, r11.xyzw
    r15.x = (dot((source[28].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 281: dp4 r15.y, cb0[29].xyzw, r11.xyzw
    r15.y = (dot((source[29].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 282: dp4 r15.z, cb0[30].xyzw, r11.xyzw
    r15.z = (dot((source[30].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 283: mul r16.xyzw, r11.yzzx, r11.xyzz
    r16.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 284: dp4 r17.x, cb0[31].xyzw, r16.xyzw
    r17.x = (dot((source[31].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 285: dp4 r17.y, cb0[32].xyzw, r16.xyzw
    r17.y = (dot((source[32].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 286: dp4 r17.z, cb0[33].xyzw, r16.xyzw
    r17.z = (dot((source[33].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 287: add r15.xyz, r15.xyzx, r17.xyzx
    r15.xyz = ((r15.xyzx)+(r17.xyzx)).xyz;
    // 288: mul r0.w, r11.y, r11.y
    r0.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 289: mov r9.z, r11.y
    r9.z = (r11.yyyy).z;
    // 290: mad r0.w, r11.x, r11.x, -r0.w
    r0.w = ((r11.xxxx)*(r11.xxxx)+(-(r0.wwww))).w;
    // 291: mad r11.xyz, cb0[34].xyzx, r0.wwww, r15.xyzx
    r11.xyz = ((source[34].xyzx)*(r0.wwww)+(r15.xyzx)).xyz;
    // 292: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 293: mul r11.xyz, r11.xyzx, cb0[26].xyzx
    r11.xyz = ((r11.xyzx)*(source[26].xyzx)).xyz;
    // 294: mul r11.xyz, r11.xyzx, cb0[27].zzzz
    r11.xyz = ((r11.xyzx)*(source[27].zzzz)).xyz;
    // 295: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[26].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[26].wwww)).xyz;
    // 296: dp3 r0.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 297: add r11.xyz, -r0.wwww, r11.xyzx
    r11.xyz = ((-(r0.wwww))+(r11.xyzx)).xyz;
    // 298: mad r11.xyz, r11.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r0.wwww
    r11.xyz = ((r11.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r0.wwww)).xyz;
    // 299: dp3 r0.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 300: div r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)/(r1.wwww)).w;
    // 301: mad r0.x, r0.x, l(5.000000), r0.w
    r0.x = ((r0.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.wwww)).x;
    // 302: add_sat r0.x, r3.w, r0.x
    r0.x = (saturate((r3.wwww)+(r0.xxxx))).x;
    // 303: mad r0.w, r0.x, l(-2.000000), l(3.000000)
    r0.w = ((r0.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 304: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 305: mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // 306: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 307: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 308: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 309: mul r11.xyz, r0.xxxx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 310: mul r15.xyz, r4.xyzx, r3.xyzx
    r15.xyz = ((r4.xyzx)*(r3.xyzx)).xyz;
    // 311: mul r3.xyz, r3.xyzx, r11.xyzx
    r3.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 312: add r0.x, -r3.w, l(1.000000)
    r0.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 313: mul r15.xyz, r0.xxxx, r15.xyzx
    r15.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 314: mul r11.xyz, r11.xyzx, r15.xyzx
    r11.xyz = ((r11.xyzx)*(r15.xyzx)).xyz;
    // 315: mul r11.xyz, r14.xyzx, r11.xyzx
    r11.xyz = ((r14.xyzx)*(r11.xyzx)).xyz;
    // 316: mad r0.w, r0.z, r8.x, r8.y
    r0.w = ((r0.zzzz)*(r8.xxxx)+(r8.yyyy)).w;
    // 317: mad r0.w, r0.w, r0.z, r8.z
    r0.w = ((r0.wwww)*(r0.zzzz)+(r8.zzzz)).w;
    // 318: mul r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)*(r0.wwww)).w;
    // 319: max r0.z, r0.w, r0.z
    r0.z = (max(r0.wwww,r0.zzzz)).z;
    // 320: mad r8.xyz, r12.xyzx, r0.zzzz, r11.xyzx
    r8.xyz = ((r12.xyzx)*(r0.zzzz)+(r11.xyzx)).xyz;
    // 321: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 322: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 323: mul r11.xyz, r0.wwww, v6.xyzx
    r11.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 324: dp3 r0.w, r11.xyzx, r5.xyzx
    r0.w = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 325: dp3 r1.w, -r11.xyzx, r5.xyzx
    r1.w = (dot((-(r11.xyzx)).xyz,(r5.xyzx).xyz).xxxx).w;
    // 326: dp3 r2.w, r11.xyzx, r10.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 327: mad r5.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 328: mad r5.zw, r1.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r5.zw = ((r1.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 329: mul r5.xyzw, r5.xyzw, r5.xyzw
    r5.xyzw = ((r5.xyzw)*(r5.xyzw)).xyzw;
    // 330: mad r10.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 331: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 332: mul r10.yzw, r10.yyyy, cb0[37].xxyz
    r10.yzw = ((r10.yyyy)*(source[37].xxyz)).yzw;
    // 333: mad r10.xyz, r10.xxxx, cb0[36].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[36].xyzx)+(r10.yzwy)).xyz;
    // 334: mul r10.xyz, r10.xyzx, cb0[38].wwww
    r10.xyz = ((r10.xyzx)*(source[38].wwww)).xyz;
    // 335: mul r10.xyz, r4.xyzx, r10.xyzx
    r10.xyz = ((r4.xyzx)*(r10.xyzx)).xyz;
    // 336: mul r10.xyz, r14.xyzx, r10.xyzx
    r10.xyz = ((r14.xyzx)*(r10.xyzx)).xyz;
    // 337: mul r10.xyz, r10.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 338: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 339: mad r3.xyz, -r3.xyzx, r3.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r3.wwww)+(r3.xyzx)).xyz;
    // 340: mad r3.xyz, r8.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r3.xyzx
    r3.xyz = ((r8.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r3.xyzx)).xyz;
    // 341: mul r8.xyz, r5.yyyy, cb0[37].xyzx
    r8.xyz = ((r5.yyyy)*(source[37].xyzx)).xyz;
    // 342: mad r8.xyz, cb0[36].xyzx, r5.xxxx, r8.xyzx
    r8.xyz = ((source[36].xyzx)*(r5.xxxx)+(r8.xyzx)).xyz;
    // 343: mul r8.xyz, r8.xyzx, cb0[38].wwww
    r8.xyz = ((r8.xyzx)*(source[38].wwww)).xyz;
    // 344: mul r8.xyz, r0.zzzz, r8.xyzx
    r8.xyz = ((r0.zzzz)*(r8.xyzx)).xyz;
    // 345: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 346: mul r7.xyz, r7.xyzx, r13.xyzx
    r7.xyz = ((r7.xyzx)*(r13.xyzx)).xyz;
    // 347: mad r3.xyz, r7.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r3.xyz = ((r7.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 348: mul r7.xyz, r7.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 349: dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 350: dp3 r0.z, r2.xyzx, r6.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 351: mul_sat r0.w, r0.z, cb0[19].w
    r0.w = (saturate((r0.zzzz)*(source[19].wwww))).w;
    // 352: add r0.z, -|r0.z|, l(1.000000)
    r0.z = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 353: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 354: mul_sat r1.w, r6.z, cb0[19].w
    r1.w = (saturate((r6.zzzz)*(source[19].wwww))).w;
    // 355: add r2.x, -|r6.z|, l(1.000000)
    r2.x = ((-(abs(r6.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 356: mul r0.z, r0.z, r2.x
    r0.z = ((r0.zzzz)*(r2.xxxx)).z;
    // 357: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 358: add_sat r1.w, r1.w, -cb0[20].x
    r1.w = (saturate((r1.wwww)+(-(source[20].xxxx)))).w;
    // 359: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 360: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 361: mul r2.x, r2.x, cb0[20].y
    r2.x = ((r2.xxxx)*(source[20].yyyy)).x;
    // 362: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 363: mul r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)*(r2.xxxx)).w;
    // 364: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 365: add r1.w, -r7.w, r0.w
    r1.w = ((-(r7.wwww))+(r0.wwww)).w;
    // 366: mad r2.xyz, r0.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r2.xyz = ((r0.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 367: mad r2.xyz, cb0[12].wwww, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((source[12].wwww)*(r2.xyzx)+(source[12].xyzx)).xyz;
    // 368: mad r0.w, cb0[11].w, r1.w, r7.w
    r0.w = ((source[11].wwww)*(r1.wwww)+(r7.wwww)).w;
    // 369: mad r2.xyz, r0.wwww, cb0[11].xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(source[11].xyzx)+(r2.xyzx)).xyz;
    // 370: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 371: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 372: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 373: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 374: mul r6.xyz, r0.wwww, cb0[13].xyzx
    r6.xyz = ((r0.wwww)*(source[13].xyzx)).xyz;
    // 375: movc r6.xyz, r0.zzzz, l(0,0,0,0), r6.xyzx
    r6.xyz = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xyzx)).xyz;
    // 376: add r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)+(r6.xyzx)).xyz;
    // 377: mad r1.xyz, cb0[19].xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((source[19].xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 378: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 379: mul r2.xyz, r5.wwww, cb0[37].xyzx
    r2.xyz = ((r5.wwww)*(source[37].xyzx)).xyz;
    // 380: mad r2.xyz, r5.zzzz, cb0[36].xyzx, r2.xyzx
    r2.xyz = ((r5.zzzz)*(source[36].xyzx)+(r2.xyzx)).xyz;
    // 381: mul r2.xyz, r2.xyzx, cb0[38].wwww
    r2.xyz = ((r2.xyzx)*(source[38].wwww)).xyz;
    // 382: mul_sat r5.xyz, cb0[17].xyzx, cb0[17].wwww
    r5.xyz = (saturate((source[17].xyzx)*(source[17].wwww))).xyz;
    // 383: mul r0.yzw, r0.yyyy, r5.xxyz
    r0.yzw = ((r0.yyyy)*(r5.xxyz)).yzw;
    // 384: mul r5.xyz, r5.xyzx, cb0[25].xxxx
    r5.xyz = ((r5.xyzx)*(source[25].xxxx)).xyz;
    // 385: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 386: mul r0.xyz, r0.xxxx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 387: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 388: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 389: mad r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r1.xyzx)).xyz;
    // 390: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 391: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 392: mad o0.xyz, r4.xyzx, cb0[38].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[38].xyzx)+(r0.xyzx)).xyz;
    // 393: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 394: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 395: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 396: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 397: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 398: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 399: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 400: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 401: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 402: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 403: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 404: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 405: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 406: ftou r0.x, cb0[35].z
    r0.x = (asfloat((uint4)(source[35].zzzz))).x;
    // 407: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 408: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 409: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 410: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 411: ret
    return output;
}

// source.character.classic-head.v1 / source program 8365eeb73a4be146bad5502a36665b4a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase4(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    r0.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterStampSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t7.xyzw, s7, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterStampSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterStampSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
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
    r1.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterStampSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
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
    r2.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterStampSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    r2.y = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterStampSampler, (r2.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
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
    r0.w = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterStampSampler, (r3.zyzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r2.w = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterStampSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
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

// source.character.eye.v1 / source program 89aca02185ddc144a9ac0ed414f3de10
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase5(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12].y=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0;
    // 1: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 2: mad r0.xyzw, v4.xywz, l(0.250000, 0.250000, 0.250000, 0.250000), l(0.375000, 0.375000, 0.375000, 0.375000)
    r0.xyzw = ((v4.xywz)*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.375000,0.375000,0.375000,0.375000))).xyzw;
    // 3: add r0.xyzw, r0.xyzw, -v4.xywz
    r0.xyzw = ((r0.xyzw)+(-(v4.xywz))).xyzw;
    // 4: mad r0.xyzw, cb0[9].zzzz, r0.xyzw, v4.xywz
    r0.xyzw = ((source[9].zzzz)*(r0.xyzw)+(v4.xywz)).xyzw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.zwzz, t1.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.wzww, t2.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.wzww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 7: add r1.xyz, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)+(-(r2.xyzx))).xyz;
    // 8: mul r0.z, r1.w, cb0[5].w
    r0.z = ((r1.wwww)*(source[5].wwww)).z;
    // 9: mad r1.xyz, r0.zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 10: add r2.xyz, -cb0[4].xyzx, cb0[5].xyzx
    r2.xyz = ((-(source[4].xyzx))+(source[5].xyzx)).xyz;
    // 11: mad r2.xyz, r0.zzzz, r2.xyzx, cb0[4].xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)+(source[4].xyzx)).xyz;
    // 12: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.xyxx, t3.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 15: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 17: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 18: mul r0.w, r2.w, cb0[7].w
    r0.w = ((r2.wwww)*(source[7].wwww)).w;
    // 19: mad r2.xyz, r0.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 20: mad r3.xyz, cb0[6].wwww, cb0[7].xyzx, -cb0[6].xyzx
    r3.xyz = ((source[6].wwww)*(source[7].xyzx)+(-(source[6].xyzx))).xyz;
    // 21: mad r3.xyz, r0.wwww, r3.xyzx, cb0[6].xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(source[6].xyzx)).xyz;
    // 22: mad r2.xyz, r3.xyzx, r2.xyzx, -r1.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 23: add r0.w, v5.x, l(0.500000)
    r0.w = ((v5.xxxx)+(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 24: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 25: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 26: mad r0.w, cb0[12].x, r0.w, l(1.000000)
    r0.w = ((source[12].xxxx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 28: add r0.w, -cb0[1].w, l(1.000000)
    r0.w = ((-(source[1].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: mul r0.w, r0.w, cb0[12].y
    r0.w = ((r0.wwww)*(source[12].yyyy)).w;
    // 30: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 31: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 32: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mul r1.w, cb0[1].z, l(1.500000)
    r1.w = ((source[1].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 34: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 35: mad r0.w, r0.w, l(0.500000), cb0[1].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[1].zzzz)).w;
    // 36: mov r2.y, cb0[1].y
    r2.y = (source[1].yyyy).y;
    // 37: mul r3.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r3.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 38: frc r1.w, r3.x
    r1.w = (frac(r3.xxxx)).w;
    // 39: mul r3.y, r1.w, l(0.125000)
    r3.y = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 40: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 41: mad r2.xy, r2.xyxx, cb0[8].xyxx, r3.yzyy
    r2.xy = ((r2.xyxx)*(source[8].xyxx)+(r3.yzyy)).xy;
    // 42: frc r1.w, cb0[1].x
    r1.w = (frac(source[1].xxxx)).w;
    // 43: add r3.x, -r1.w, cb0[1].x
    r3.x = ((-(r1.wwww))+(source[1].xxxx)).x;
    // 44: mul r2.z, r3.x, l(0.125000)
    r2.z = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 45: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t4.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 47: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 48: mul r0.w, r1.w, r2.w
    r0.w = ((r1.wwww)*(r2.wwww)).w;
    // 49: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 50: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 51: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 52: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 53: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 55: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 56: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 57: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 58: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 59: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 60: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 61: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 62: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 63: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 64: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 65: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 66: dp3 r0.w, r0.xyzx, r2.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 67: mul r2.zw, r0.wwww, r0.xxxy
    r2.zw = ((r0.wwww)*(r0.xxxy)).zw;
    // 68: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), -r2.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(-(r2.xxxy))).zw;
    // 69: mul r2.xy, r2.xyxx, l(1.500000, 1.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(1.500000,1.500000,0.000000,0.000000))).xy;
    // 70: add r2.zw, r2.zzzw, -v4.xxxy
    r2.zw = ((r2.zzzw)+(-(v4.xxxy))).zw;
    // 71: mad r2.zw, cb0[11].xxxx, r2.zzzw, v4.xxxy
    r2.zw = ((source[11].xxxx)*(r2.zzzw)+(v4.xxxy)).zw;
    // 72: mad r2.zw, r2.zzzw, cb0[2].xxxy, cb0[3].xxxy
    r2.zw = ((r2.zzzw)*(source[2].xxxy)+(source[3].xxxy)).zw;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.zwzz, t5.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 74: add r2.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r2.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 75: dp2 r0.w, r2.zwzz, r2.zwzz
    r0.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 76: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 77: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 78: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: max r0.w, r0.w, cb0[9].w
    r0.w = (max(r0.wwww,source[9].wwww)).w;
    // 80: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mad r2.xy, r0.wwww, -r2.xyxx, v4.xyxx
    r2.xy = ((r0.wwww)*(-(r2.xyxx))+(v4.xyxx)).xy;
    // 82: add r2.xy, r2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 83: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 84: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 85: add r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)+(r0.wwww)).w;
    // 86: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 87: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: log r1.w, r0.w
    r1.w = (log2(r0.wwww)).w;
    // 89: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 90: mul r1.w, r1.w, cb0[10].x
    r1.w = ((r1.wwww)*(source[10].xxxx)).w;
    // 91: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 92: mul r1.w, r1.w, cb0[10].y
    r1.w = ((r1.wwww)*(source[10].yyyy)).w;
    // 93: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 94: mad r2.xyz, cb0[11].wwww, r3.xyzx, r0.wwww
    r2.xyz = ((source[11].wwww)*(r3.xyzx)+(r0.wwww)).xyz;
    // 95: max r2.xyz, r2.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 96: min r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 97: add r0.w, cb0[1].y, cb0[1].x
    r0.w = ((source[1].yyyy)+(source[1].xxxx)).w;
    // 98: add r0.w, r0.w, cb0[1].z
    r0.w = ((r0.wwww)+(source[1].zzzz)).w;
    // 99: round_pi_sat r0.w, r0.w
    r0.w = (saturate(ceil(r0.wwww))).w;
    // 100: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 101: mul r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // 102: mad r2.xyz, r2.xyzx, l(0.700000, 0.700000, 0.700000, 0.000000), cb0[0].xyzx
    r2.xyz = ((r2.xyzx)*(float4(0.700000,0.700000,0.700000,0.000000))+(source[0].xyzx)).xyz;
    // 103: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 104: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 105: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 106: dp3 r0.w, r3.xyzx, r0.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 107: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 108: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 109: mul r3.yzw, r3.yyyy, cb0[14].xxyz
    r3.yzw = ((r3.yyyy)*(source[14].xxyz)).yzw;
    // 110: mad r3.xyz, r3.xxxx, cb0[13].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[13].xyzx)+(r3.yzwy)).xyz;
    // 111: mul r3.xyz, r3.xyzx, cb0[15].wwww
    r3.xyz = ((r3.xyzx)*(source[15].wwww)).xyz;
    // 112: mad r2.xyz, r3.xyzx, r1.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 113: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 114: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 115: mad o0.xyz, r1.xyzx, cb0[15].xyzx, r2.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[15].xyzx)+(r2.xyzx)).xyz;
    // 116: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 117: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 118: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 119: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 120: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 121: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 122: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 123: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 124: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 125: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 126: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 127: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 128: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 129: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 130: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 131: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 132: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 133: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 134: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 135: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 136: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 137: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 138: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 139: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 140: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 141: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 142: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 143: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 144: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 145: ret
    return output;
}

// source.character.eyelash.v1 / source program 9859dedeace20041ba271c94538f478a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase6(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0].xy=float2(1.0,1.0);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v7.z
    r0.x = ((r0.xxxx)*(v7.zzzz)).x;
    // 4: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 6: mul r0.yzw, r0.yyyy, cb0[6].xxyz
    r0.yzw = ((r0.yyyy)*(source[6].xxyz)).yzw;
    // 7: mad r0.xyz, r0.xxxx, cb0[5].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[5].xyzx)+(r0.yzwy)).xyz;
    // 8: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 9: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 11: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 12: mul r0.w, r2.w, cb0[3].x
    r0.w = ((r2.wwww)*(source[3].xxxx)).w;
    // 13: mul o0.w, r0.w, cb0[0].y
    output.targets[0].w = ((r0.wwww)*(source[0].yyyy)).w;
    // 14: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 15: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 16: mad r2.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 17: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 18: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 19: mad r0.xyz, r1.xyzx, cb0[7].xyzx, r2.xyzx
    r0.xyz = ((r1.xyzx)*(source[7].xyzx)+(r2.xyzx)).xyz;
    // 20: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 21: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 22: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 23: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 24: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 25: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 26: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 27: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 28: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 29: mad r0.x, r0.x, r1.y, -r0.y
    r0.x = ((r0.xxxx)*(r1.yyyy)+(-(r0.yyyy))).x;
    // 30: mul r0.x, r0.x, v1.w
    r0.x = ((r0.xxxx)*(v1.wwww)).x;
    // 31: movc r0.y, v9.x, l(1.000000), l(-1.000000)
    r0.y = ((asuint(v9.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 32: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 33: mul r2.x, r0.y, r1.z
    r2.x = ((r0.yyyy)*(r1.zzzz)).x;
    // 34: mul r2.yz, r0.yyyy, r0.xxzx
    r2.yz = ((r0.yyyy)*(r0.xxzx)).yz;
    // 35: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 36: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 37: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 38: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 39: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 40: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 41: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 42: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 43: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 44: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 45: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 46: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 47: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 48: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 49: ftou r0.x, cb0[4].z
    r0.x = (asfloat((uint4)(source[4].zzzz))).x;
    // 50: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 51: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 52: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 53: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 54: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 55: ret
    return output;
}

// source.character.hair.v1 / source program 44672672c5779b49bbaf371a34212e54
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase7(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].z=(g_SourceCharacterTime.xxxx).x;
    source[22].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[0].x=1.0; source[1].w=1.0; source[26].x=0.0;
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
    // 17: div r3.yzw, r3.xxyz, r0.wwww
    r3.yzw = ((r3.xxyz)/(r0.wwww)).yzw;
    // 18: dp3 r0.w, r3.yzwy, r3.yzwy
    r0.w = (dot((r3.yzwy).xyz,(r3.yzwy).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r4.xyz, r0.wwww, r3.yzwy
    r4.xyz = ((r0.wwww)*(r3.yzwy)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r0.w, r5.w, cb0[1].w
    r0.w = ((r5.wwww)*(source[1].wwww)).w;
    // 23: add r6.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r6.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 24: mul r6.xyz, r6.xyzx, cb0[16].xxxx
    r6.xyz = ((r6.xyzx)*(source[16].xxxx)).xyz;
    // 25: mad r1.w, cb0[16].y, l(-3.500000), l(5.000000)
    r1.w = ((source[16].yyyy)*(float4(-3.500000,-3.500000,-3.500000,-3.500000))+(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 26: mul r1.w, r1.w, cb0[17].x
    r1.w = ((r1.wwww)*(source[17].xxxx)).w;
    // 27: add r2.w, -v4.z, l(1.000000)
    r2.w = ((-(v4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: add r4.w, -r2.w, v4.z
    r4.w = ((-(r2.wwww))+(v4.zzzz)).w;
    // 29: mad r2.w, cb0[17].y, r4.w, r2.w
    r2.w = ((source[17].yyyy)*(r4.wwww)+(r2.wwww)).w;
    // 30: mul r4.w, r2.w, cb0[17].z
    r4.w = ((r2.wwww)*(source[17].zzzz)).w;
    // 31: mad r2.w, r4.w, l(0.750000), r2.w
    r2.w = ((r4.wwww)*(float4(0.750000,0.750000,0.750000,0.750000))+(r2.wwww)).w;
    // 32: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 33: mad_sat r2.w, cb0[18].x, r2.w, r2.w
    r2.w = (saturate((source[18].xxxx)*(r2.wwww)+(r2.wwww))).w;
    // 34: mad r3.x, r3.x, r5.x, l(0.200000)
    r3.x = ((r3.xxxx)*(r5.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 35: add r4.w, -r5.y, l(1.000000)
    r4.w = ((-(r5.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: add r4.w, -r3.x, r4.w
    r4.w = ((-(r3.xxxx))+(r4.wwww)).w;
    // 37: mad r6.w, cb0[18].z, r4.w, r3.x
    r6.w = ((source[18].zzzz)*(r4.wwww)+(r3.xxxx)).w;
    // 38: mul r7.x, cb0[17].w, l(0.700000)
    r7.x = ((source[17].wwww)*(float4(0.700000,0.700000,0.700000,0.700000))).x;
    // 39: add r6.w, -r2.w, r6.w
    r6.w = ((-(r2.wwww))+(r6.wwww)).w;
    // 40: mad r6.w, r7.x, r6.w, r2.w
    r6.w = ((r7.xxxx)*(r6.wwww)+(r2.wwww)).w;
    // 41: div r6.w, r6.w, cb0[18].y
    r6.w = ((r6.wwww)/(source[18].yyyy)).w;
    // 42: add r6.w, -r6.w, l(1.000000)
    r6.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul r6.w, r1.w, r6.w
    r6.w = ((r1.wwww)*(r6.wwww)).w;
    // 44: mul r6.w, r6.w, l(4.000000)
    r6.w = ((r6.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 45: add r7.y, v4.w, l(0.500000)
    r7.y = ((v4.wwww)+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 46: round_ni r7.y, r7.y
    r7.y = (floor(r7.yyyy)).y;
    // 47: mul_sat r6.w, r6.w, r7.y
    r6.w = (saturate((r6.wwww)*(r7.yyyy))).w;
    // 48: mad r3.x, cb0[19].x, r4.w, r3.x
    r3.x = ((source[19].xxxx)*(r4.wwww)+(r3.xxxx)).x;
    // 49: add r3.x, -r2.w, r3.x
    r3.x = ((-(r2.wwww))+(r3.xxxx)).x;
    // 50: mad r2.w, r7.x, r3.x, r2.w
    r2.w = ((r7.xxxx)*(r3.xxxx)+(r2.wwww)).w;
    // 51: div r2.w, r2.w, cb0[18].w
    r2.w = ((r2.wwww)/(source[18].wwww)).w;
    // 52: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 54: mul r1.w, r1.w, l(4.000000)
    r1.w = ((r1.wwww)*(float4(4.000000,4.000000,4.000000,4.000000))).w;
    // 55: add r2.w, -r7.y, l(1.000000)
    r2.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 57: add r1.w, r1.w, r6.w
    r1.w = ((r1.wwww)+(r6.wwww)).w;
    // 58: add r1.w, -r5.y, r1.w
    r1.w = ((-(r5.yyyy))+(r1.wwww)).w;
    // 59: mad r1.w, cb0[19].y, r1.w, r5.y
    r1.w = ((source[19].yyyy)*(r1.wwww)+(r5.yyyy)).w;
    // 60: mad r6.xyz, r1.wwww, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(source[3].xyzx)).xyz;
    // 61: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 63: mad r6.xyz, cb0[19].zzzz, r7.xyzx, r6.xyzx
    r6.xyz = ((source[19].zzzz)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 64: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 66: mad r6.xyz, cb0[19].wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((source[19].wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 67: mad r7.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mad r8.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 70: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 71: mul r8.xyz, r5.xxxx, r8.xyzx
    r8.xyz = ((r5.xxxx)*(r8.xyzx)).xyz;
    // 72: mul r1.w, cb0[11].z, l(1.500000)
    r1.w = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 73: add r2.w, -cb0[11].w, l(1.000000)
    r2.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 74: mul r2.w, r2.w, cb0[22].z
    r2.w = ((r2.wwww)*(source[22].zzzz)).w;
    // 75: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 76: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 77: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 78: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 79: mad r1.w, r1.w, l(0.500000), cb0[11].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 80: frc r2.w, cb0[11].x
    r2.w = (frac(source[11].xxxx)).w;
    // 81: add r3.x, -r2.w, cb0[11].x
    r3.x = ((-(r2.wwww))+(source[11].xxxx)).x;
    // 82: mul r9.z, r3.x, l(0.125000)
    r9.z = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 83: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 84: mul r9.y, cb0[11].y, cb0[12].y
    r9.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 85: mul r10.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r10.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 86: frc r3.x, r10.x
    r3.x = (frac(r10.xxxx)).x;
    // 87: mul r10.y, r3.x, l(0.125000)
    r10.y = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 88: add r5.xy, r9.xyxx, r10.yzyy
    r5.xy = ((r9.xyxx)+(r10.yzyy)).xy;
    // 89: add r5.xy, r5.xyxx, r9.zwzz
    r5.xy = ((r5.xyxx)+(r9.zwzz)).xy;
    // 90: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 91: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 92: mul r1.w, r2.w, r9.w
    r1.w = ((r2.wwww)*(r9.wwww)).w;
    // 93: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r8.xyzx))).xyz;
    // 94: mad r8.xyz, r1.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 95: mul r1.w, cb0[13].y, cb0[22].z
    r1.w = ((source[13].yyyy)*(source[22].zzzz)).w;
    // 96: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 97: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 98: mul r5.y, r1.w, l(0.020000)
    r5.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 99: add r9.xyzw, r1.yzxy, -cb0[1].yzxy
    r9.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 100: add r9.xy, -r9.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r9.xy = ((-(r9.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 101: add r9.xy, -r9.zwzz, r9.xyxx
    r9.xy = ((-(r9.zwzz))+(r9.xyxx)).xy;
    // 102: mad r9.xy, cb0[13].wwww, r9.xyxx, r9.zwzz
    r9.xy = ((source[13].wwww)*(r9.xyxx)+(r9.zwzz)).xy;
    // 103: mul r2.w, cb0[13].x, l(0.001000)
    r2.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 104: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 105: mad r5.xy, r2.wwww, r9.xyxx, r5.xyxx
    r5.xy = ((r2.wwww)*(r9.xyxx)+(r5.xyxx)).xy;
    // 106: dp2 r2.w, cb0[14].xyxx, r5.xyxx
    r2.w = (dot((source[14].xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 107: dp2 r5.y, cb0[15].xyxx, r5.xyxx
    r5.y = (dot((source[15].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 108: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 109: mul r5.x, r2.w, l(0.125000)
    r5.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 110: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 111: mul r2.w, r9.w, l(0.900000)
    r2.w = ((r9.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 112: mad r9.xyz, r9.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r8.xyzx
    r9.xyz = ((r9.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r8.xyzx))).xyz;
    // 113: mad r9.xyz, r2.wwww, r9.xyzx, r8.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 114: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 116: mul_sat r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = (saturate((r9.xyzx)*(r1.wwww))).xyz;
    // 117: mul r10.xyz, r9.xyzx, cb0[13].zzzz
    r10.xyz = ((r9.xyzx)*(source[13].zzzz)).xyz;
    // 118: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 120: mad r9.xyz, cb0[13].zzzz, r9.xyzx, -r8.xyzx
    r9.xyz = ((source[13].zzzz)*(r9.xyzx)+(-(r8.xyzx))).xyz;
    // 121: mad r8.xyz, r1.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r1.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 122: mad r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = ((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 123: mad r6.xyz, r6.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r6.xyz = ((r6.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 124: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 125: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 126: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 127: mul r5.xyz, r5.zzzz, r6.xyzx
    r5.xyz = ((r5.zzzz)*(r6.xyzx)).xyz;
    // 128: mov_sat r1.w, r2.z
    r1.w = (saturate(r2.zzzz)).w;
    // 129: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 130: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 131: mul r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)*(r1.wwww)).xyz;
    // 132: mul r5.xyz, r5.xyzx, cb0[20].xxxx
    r5.xyz = ((r5.xyzx)*(source[20].xxxx)).xyz;
    // 133: add r1.xyz, -r1.xyzx, cb0[0].yzwy
    r1.xyz = ((-(r1.xyzx))+(source[0].yzwy)).xyz;
    // 134: mul r6.xyz, r2.zzzz, r1.xyzx
    r6.xyz = ((r2.zzzz)*(r1.xyzx)).xyz;
    // 135: mad r1.xyz, r6.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r6.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 136: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 137: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 138: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 139: add r1.x, r1.x, cb0[7].z
    r1.x = ((r1.xxxx)+(source[7].zzzz)).x;
    // 140: dp3 r1.y, r0.xyzx, r3.yzwy
    r1.y = (dot((r0.xyzx).xyz,(r3.yzwy).xyz).xxxx).y;
    // 141: add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // 142: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 143: add r1.x, r1.x, l(-0.500000)
    r1.x = ((r1.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 144: add r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)+(r1.xxxx)).x;
    // 145: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 146: mad r1.y, cb0[20].w, l(4.500000), l(0.500000)
    r1.y = ((source[20].wwww)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 147: mul r1.y, r1.y, cb0[21].x
    r1.y = ((r1.yyyy)*(source[21].xxxx)).y;
    // 148: mul r1.y, r1.y, l(0.050000)
    r1.y = ((r1.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 149: lt r1.z, r1.x, l(0.000001)
    r1.z = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 150: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 151: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 152: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 153: mul r1.x, r1.x, cb0[21].y
    r1.x = ((r1.xxxx)*(source[21].yyyy)).x;
    // 154: movc r1.x, r1.z, l(0), r1.x
    r1.x = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 155: dp3 r1.y, r3.yzwy, r2.xyzx
    r1.y = (dot((r3.yzwy).xyz,(r2.xyzx).xyz).xxxx).y;
    // 156: add r1.z, -r1.y, l(1.000000)
    r1.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 157: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: add r2.x, -|r1.y|, l(1.000000)
    r2.x = ((-(abs(r1.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 159: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 160: mad r2.xyw, r1.wwww, cb0[9].xyxz, -cb0[9].xyxz
    r2.xyw = ((r1.wwww)*(source[9].xyxz)+(-(source[9].xyxz))).xyw;
    // 161: mad r2.xyw, cb0[9].wwww, r2.xyxw, cb0[9].xyxz
    r2.xyw = ((source[9].wwww)*(r2.xyxw)+(source[9].xyxz)).xyw;
    // 162: mad r2.xyw, r1.zzzz, cb0[8].xyxz, r2.xyxw
    r2.xyw = ((r1.zzzz)*(source[8].xyxz)+(r2.xyxw)).xyw;
    // 163: mul_sat r1.y, r1.y, cb0[21].z
    r1.y = (saturate((r1.yyyy)*(source[21].zzzz))).y;
    // 164: mul_sat r1.z, r2.z, cb0[21].z
    r1.z = (saturate((r2.zzzz)*(source[21].zzzz))).z;
    // 165: add r1.yz, -r1.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r1.yz = ((-(r1.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // 166: add_sat r1.z, r1.z, -cb0[21].w
    r1.z = (saturate((r1.zzzz)+(-(source[21].wwww)))).z;
    // 167: lt r1.w, r1.z, l(0.000001)
    r1.w = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 168: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 169: mul r1.z, r1.z, cb0[22].x
    r1.z = ((r1.zzzz)*(source[22].xxxx)).z;
    // 170: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 171: mul r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)*(r1.yyyy)).y;
    // 172: mul r1.y, r1.y, cb0[10].w
    r1.y = ((r1.yyyy)*(source[10].wwww)).y;
    // 173: mul r3.xyz, r1.yyyy, cb0[10].xyzx
    r3.xyz = ((r1.yyyy)*(source[10].xyzx)).xyz;
    // 174: movc r1.yzw, r1.wwww, l(0,0,0,0), r3.xxyz
    r1.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 175: add r1.yzw, r1.yyzw, r2.xxyw
    r1.yzw = ((r1.yyzw)+(r2.xxyw)).yzw;
    // 176: mad r1.xyz, r1.xxxx, r5.xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r5.xyzx)+(r1.yzwy)).xyz;
    // 177: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 178: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 179: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 180: mul r2.xyz, r1.wwww, v7.xyzx
    r2.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 181: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 182: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 183: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 184: mul r2.yzw, r2.yyyy, cb0[24].xxyz
    r2.yzw = ((r2.yyyy)*(source[24].xxyz)).yzw;
    // 185: mad r2.xyz, r2.xxxx, cb0[23].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[23].xyzx)+(r2.yzwy)).xyz;
    // 186: mul r2.xyz, r2.xyzx, cb0[25].wwww
    r2.xyz = ((r2.xyzx)*(source[25].wwww)).xyz;
    // 187: mul r3.xyz, r8.xyzx, r2.xyzx
    r3.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 188: mad r1.xyz, r2.xyzx, r8.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 189: mad r1.xyz, r8.xyzx, cb0[25].xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(source[25].xyzx)+(r1.xyzx)).xyz;
    // 190: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 191: mad r1.w, r1.w, l(-0.250000), l(0.400000)
    r1.w = ((r1.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 192: eq r2.x, cb0[26].x, l(0.000000)
    r2.x = (asfloat((uint4)((source[26].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 193: not r2.y, r2.x
    r2.y = (asfloat(~asuint(r2.xxxx))).y;
    // 194: lt r2.z, r0.w, r1.w
    r2.z = (asfloat((uint4)((r0.wwww)<(r1.wwww)) * 0xffffffffu)).z;
    // 195: and r2.y, r2.z, r2.y
    r2.y = (asfloat(asuint(r2.zzzz) & asuint(r2.yyyy))).y;
    // 196: discard_nz r2.y
    if ((asuint(r2.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 197: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 198: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 199: mul r2.yzw, r2.yyyy, v0.xxyz
    r2.yzw = ((r2.yyyy)*(v0.xxyz)).yzw;
    // 200: mul r5.xyz, r0.zxyz, r2.zwyz
    r5.xyz = ((r0.zxyz)*(r2.zwyz)).xyz;
    // 201: mad r5.xyz, r0.yzxy, r2.wyzw, -r5.xyzx
    r5.xyz = ((r0.yzxy)*(r2.wyzw)+(-(r5.xyzx))).xyz;
    // 202: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 203: movc r3.w, v9.x, l(1.000000), l(-1.000000)
    r3.w = ((asuint(v9.xxxx) != 0u) ? (float4(1.000000,1.000000,1.000000,1.000000)) : (float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 204: mul r3.w, r3.w, cb0[0].x
    r3.w = ((r3.wwww)*(source[0].xxxx)).w;
    // 205: mul r4.xyz, r3.wwww, r4.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 206: ge r1.w, r0.w, r1.w
    r1.w = (asfloat((uint4)((r0.wwww)>=(r1.wwww)) * 0xffffffffu)).w;
    // 207: mad r3.w, r5.w, cb0[1].w, l(-0.900000)
    r3.w = ((r5.wwww)*(source[1].wwww)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 208: mul_sat r3.w, r3.w, l(9.999998)
    r3.w = (saturate((r3.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 209: mad r4.w, r3.w, l(-2.000000), l(3.000000)
    r4.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 210: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 211: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 212: mul r3.w, r0.w, r3.w
    r3.w = ((r0.wwww)*(r3.wwww)).w;
    // 213: movc r1.w, r1.w, r3.w, r0.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (r3.wwww) : (r0.wwww)).w;
    // 214: movc o0.w, r2.x, r1.w, r0.w
    output.targets[0].w = ((asuint(r2.xxxx) != 0u) ? (r1.wwww) : (r0.wwww)).w;
    // 215: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 216: dp3 r1.x, r2.yzwy, r4.xyzx
    r1.x = (dot((r2.yzwy).xyz,(r4.xyzx).xyz).xxxx).x;
    // 217: dp3 r1.y, r5.xyzx, r4.xyzx
    r1.y = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 218: dp3 r1.z, r0.xyzx, r4.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 219: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 220: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 221: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 222: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 223: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 224: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 225: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 226: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 227: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 228: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 229: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 230: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 231: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 232: mov o3.xyz, r8.xyzx
    output.targets[3].xyz = (r8.xyzx).xyz;
    // 233: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 234: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 235: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 236: ret
    return output;
}

// source.character.realpbr-weapon.v1 / source program 8932162a08b66149b75f1c65adc3df7d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase8(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    if (g_SourceCharacterEnvironmentEnabled != 0u)
    {
        source[25] = g_SourceCharacterEnvironmentColor;
        source[26] = g_SourceCharacterEnvironmentRotation;
    }
    source[16]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19].z=(g_SourceCharacterTime.xxxx).x;
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
    // 74: mul_sat r8.w, r1.z, cb2[3].w
    r8.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 75: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 76: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 77: dp3 r1.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 78: add r4.xyz, -r3.xyzx, r1.zzzz
    r4.xyz = ((-(r3.xyzx))+(r1.zzzz)).xyz;
    // 79: mad r4.xyz, cb0[20].xxxx, r4.xyzx, r3.xyzx
    r4.xyz = ((source[20].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 80: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 81: dp3 r1.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 82: add r3.xyz, -r4.xyzx, r1.zzzz
    r3.xyz = ((-(r4.xyzx))+(r1.zzzz)).xyz;
    // 83: mad r3.xyz, cb0[20].yyyy, r3.xyzx, r4.xyzx
    r3.xyz = ((source[20].yyyy)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 84: mad r4.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mad r9.xyz, cb0[11].wwww, cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[11].wwww)*(source[11].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 86: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 87: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 88: mul r9.xyz, r2.xyzx, r3.xyzx
    r9.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 89: dp3 r1.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 90: mad r2.xyz, -r3.xyzx, r2.xyzx, r1.zzzz
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r1.zzzz)).xyz;
    // 91: mad r2.xyz, cb0[20].xxxx, r2.xyzx, r9.xyzx
    r2.xyz = ((source[20].xxxx)*(r2.xyzx)+(r9.xyzx)).xyz;
    // 92: dp3 r1.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 93: add r3.xyz, -r2.xyzx, r1.zzzz
    r3.xyz = ((-(r2.xyzx))+(r1.zzzz)).xyz;
    // 94: mad r2.xyz, cb0[20].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[20].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 95: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 96: mad r1.xyz, r1.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 97: mad r1.xyz, r0.xxxx, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 98: mul r0.x, r6.x, cb0[21].y
    r0.x = ((r6.xxxx)*(source[21].yyyy)).x;
    // 99: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 100: movc r0.x, r5.x, l(0), r0.x
    r0.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 101: add_sat r0.x, r0.x, cb0[21].z
    r0.x = (saturate((r0.xxxx)+(source[21].zzzz))).x;
    // 102: add r1.w, -r0.x, l(1.000000)
    r1.w = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: mul r2.xyz, r1.wwww, cb0[15].xyzx
    r2.xyz = ((r1.wwww)*(source[15].xyzx)).xyz;
    // 104: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 105: mad r1.xyz, -r2.xyzx, r1.xyzx, r1.xyzx
    r1.xyz = ((-(r2.xyzx))*(r1.xyzx)+(r1.xyzx)).xyz;
    // 106: mad r1.xyz, r0.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 107: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 108: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 109: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 110: mad r2.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r2.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 111: mad r3.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 112: mad r2.xyz, r0.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 113: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 114: mad r2.xyz, r2.xyzx, r0.xxxx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 115: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 116: max r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = (max(r0.xxxx,r2.xyzx)).xyz;
    // 117: mov_sat r1.w, cb0[22].y
    r1.w = (saturate(source[22].yyyy)).w;
    // 118: mad r3.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r3.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 119: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 120: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 121: mad r3.xyz, r8.wwww, r3.xyzx, r2.wwww
    r3.xyz = ((r8.wwww)*(r3.xyzx)+(r2.wwww)).xyz;
    // 122: add r1.w, -cb0[23].y, cb0[23].x
    r1.w = ((-(source[23].yyyy))+(source[23].xxxx)).w;
    // 123: mad r1.w, r7.x, r1.w, cb0[23].y
    r1.w = ((r7.xxxx)*(r1.wwww)+(source[23].yyyy)).w;
    // 124: add r2.w, -r1.w, cb0[23].w
    r2.w = ((-(r1.wwww))+(source[23].wwww)).w;
    // 125: mad r1.w, r7.y, r2.w, r1.w
    r1.w = ((r7.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 126: add r2.w, -r1.w, cb0[24].y
    r2.w = ((-(r1.wwww))+(source[24].yyyy)).w;
    // 127: mad r1.w, r7.z, r2.w, r1.w
    r1.w = ((r7.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 128: mul r1.w, r6.z, r1.w
    r1.w = ((r6.zzzz)*(r1.wwww)).w;
    // 129: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 130: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: movc r1.w, r5.z, l(0), r1.w
    r1.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 132: max r1.w, r1.w, cb0[1].x
    r1.w = (max(r1.wwww,source[1].xxxx)).w;
    // 133: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 135: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 136: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 137: mul r5.xy, r5.xyxx, cb0[18].xxxx
    r5.xy = ((r5.xyxx)*(source[18].xxxx)).xy;
    // 138: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 140: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 141: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 142: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 143: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 144: div r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 145: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 146: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 147: mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 148: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 149: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 150: mul r7.xyz, r1.wwww, v5.xyzx
    r7.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 151: dp3 r1.w, r6.xyzx, r7.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 152: deriv_rtx_coarse r8.x, r1.w
    r8.x = (ddx_coarse(r1.wwww)).x;
    // 153: deriv_rty_coarse r8.y, r1.w
    r8.y = (ddy_coarse(r1.wwww)).y;
    // 154: dp2 r2.w, r8.xyxx, r8.xyxx
    r2.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 155: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 156: mad r2.w, r2.w, l(0.300000), r8.z
    r2.w = ((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz)).w;
    // 157: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 158: min r8.y, r2.w, l(1.000000)
    r8.y = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 159: add r2.w, -r8.y, l(1.000000)
    r2.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: max r9.xyz, r3.xyzx, r2.wwww
    r9.xyz = (max(r3.xyzx,r2.wwww)).xyz;
    // 161: add r9.xyz, -r3.xyzx, r9.xyzx
    r9.xyz = ((-(r3.xyzx))+(r9.xyzx)).xyz;
    // 162: mul_sat r2.w, r3.y, l(50.000000)
    r2.w = (saturate((r3.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 163: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 164: mul r10.xyz, r1.wwww, r6.xyzx
    r10.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 165: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 166: add r2.w, r10.z, l(1.000000)
    r2.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: add r3.w, r1.w, l(1.000000)
    r3.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: mov_sat r1.w, r1.w
    r1.w = (saturate(r1.wwww)).w;
    // 170: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 171: mul r1.w, r1.w, cb0[2].y
    r1.w = ((r1.wwww)*(source[2].yyyy)).w;
    // 172: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 173: mad_sat r1.w, r1.w, cb0[2].w, cb0[2].z
    r1.w = (saturate((r1.wwww)*(source[2].wwww)+(source[2].zzzz))).w;
    // 174: mul r1.w, r1.w, cb0[24].z
    r1.w = ((r1.wwww)*(source[24].zzzz)).w;
    // 175: add_sat r8.x, -r2.w, r3.w
    r8.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 176: sample_indexable(texture2d)(float,float,float,float) r11.xy, r8.xyxx, t6.xyzw, s7
    r11.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 177: add r2.w, r0.x, r8.x
    r2.w = ((r0.xxxx)+(r8.xxxx)).w;
    // 178: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 179: mul r12.xyz, r3.xyzx, r11.yyyy
    r12.xyz = ((r3.xyzx)*(r11.yyyy)).xyz;
    // 180: mad r9.xyz, r9.xyzx, r11.xxxx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xxxx)+(r12.xyzx)).xyz;
    // 181: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r11.y
    r3.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r11.yyyy)).w;
    // 182: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 183: mad r11.xyz, r3.xyzx, r3.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((r3.xyzx)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: dp3 r3.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 185: mad r3.xyz, r3.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r3.xyz = ((r3.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 186: mad r12.xyz, -r9.xyzx, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r9.xyzx))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 188: mul r11.xyz, r1.xyzx, r12.xyzx
    r11.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 189: add r3.w, -r8.w, l(1.000000)
    r3.w = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: mul r11.xyz, r3.wwww, r11.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)).xyz;
    // 191: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 192: dp3 r5.w, v1.xyzx, v1.xyzx
    r5.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 193: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 194: mul r13.xyz, r5.wwww, v1.xyzx
    r13.xyz = ((r5.wwww)*(v1.xyzx)).xyz;
    // 195: dp3 r5.w, v0.xyzx, v0.xyzx
    r5.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 196: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 197: mul r14.xyz, r5.wwww, v0.xyzx
    r14.xyz = ((r5.wwww)*(v0.xyzx)).xyz;
    // 198: mul r15.xyz, r13.zxyz, r14.yzxy
    r15.xyz = ((r13.zxyz)*(r14.yzxy)).xyz;
    // 199: mad r15.xyz, r13.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r13.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 200: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 201: dp3 r16.y, r15.xyzx, r6.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 202: dp3 r15.y, r15.xyzx, r10.xyzx
    r15.y = (dot((r15.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 203: dp3 r16.x, r14.xyzx, r6.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 204: dp3 r15.x, r14.xyzx, r10.xyzx
    r15.x = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 205: dp2 r14.z, r16.xyxx, cb0[26].xyxx
    r14.z = (dot((r16.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 206: mul r8.xz, cb0[26].yyxy, l(1.000000, 0.000000, -1.000000, 0.000000)
    r8.xz = ((source[26].yyxy)*(float4(1.000000,0.000000,-1.000000,0.000000))).xz;
    // 207: dp2 r14.x, r16.xyxx, r8.xzxx
    r14.x = (dot((r16.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 208: dp2 r17.x, r15.xyxx, r8.xzxx
    r17.x = (dot((r15.xyxx).xy,(r8.xzxx).xy).xxxx).x;
    // 209: dp2 r17.z, r15.xyxx, cb0[26].xyxx
    r17.z = (dot((r15.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 210: dp3 r14.y, r13.xyzx, r6.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 211: dp3 r17.y, r13.xyzx, r10.xyzx
    r17.y = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 212: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 213: dp4 r13.x, cb0[27].xyzw, r14.xyzw
    r13.x = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 214: dp4 r13.y, cb0[28].xyzw, r14.xyzw
    r13.y = (dot((source[28].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 215: dp4 r13.z, cb0[29].xyzw, r14.xyzw
    r13.z = (dot((source[29].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 216: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 217: dp4 r18.x, cb0[30].xyzw, r15.xyzw
    r18.x = (dot((source[30].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 218: dp4 r18.y, cb0[31].xyzw, r15.xyzw
    r18.y = (dot((source[31].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 219: dp4 r18.z, cb0[32].xyzw, r15.xyzw
    r18.z = (dot((source[32].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 220: add r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)+(r18.xyzx)).xyz;
    // 221: mul r5.w, r14.y, r14.y
    r5.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 222: mov r16.z, r14.y
    r16.z = (r14.yyyy).z;
    // 223: mad r5.w, r14.x, r14.x, -r5.w
    r5.w = ((r14.xxxx)*(r14.xxxx)+(-(r5.wwww))).w;
    // 224: mad r13.xyz, cb0[33].xyzx, r5.wwww, r13.xyzx
    r13.xyz = ((source[33].xyzx)*(r5.wwww)+(r13.xyzx)).xyz;
    // 225: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 226: mul r13.xyz, r13.xyzx, cb0[25].xyzx
    r13.xyz = ((r13.xyzx)*(source[25].xyzx)).xyz;
    // 227: mul r13.xyz, r13.xyzx, cb0[26].zzzz
    r13.xyz = ((r13.xyzx)*(source[26].zzzz)).xyz;
    // 228: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[25].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[25].wwww)).xyz;
    // 229: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 230: add r13.xyz, -r5.wwww, r13.xyzx
    r13.xyz = ((-(r5.wwww))+(r13.xyzx)).xyz;
    // 231: mad r13.xyz, r13.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r5.wwww
    r13.xyz = ((r13.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r5.wwww)).xyz;
    // 232: dp3 r5.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 233: mad r6.w, r8.y, l(2.000000), l(2.000000)
    r6.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 234: div r5.w, r5.w, r6.w
    r5.w = ((r5.wwww)/(r6.wwww)).w;
    // 235: mad r5.w, r4.w, l(5.000000), r5.w
    r5.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r5.wwww)).w;
    // 236: add_sat r5.w, r8.w, r5.w
    r5.w = (saturate((r8.wwww)+(r5.wwww))).w;
    // 237: mad r7.w, r5.w, l(-2.000000), l(3.000000)
    r7.w = ((r5.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 238: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 239: mul r5.w, r5.w, r7.w
    r5.w = ((r5.wwww)*(r7.wwww)).w;
    // 240: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 241: mul r5.w, r5.w, l(1.500000)
    r5.w = ((r5.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 242: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 243: mul r13.xyz, r5.wwww, r13.xyzx
    r13.xyz = ((r5.wwww)*(r13.xyzx)).xyz;
    // 244: mul r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)*(r13.xyzx)).xyz;
    // 245: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 246: mul r11.xyz, r2.xyzx, r11.xyzx
    r11.xyz = ((r2.xyzx)*(r11.xyzx)).xyz;
    // 247: mul r5.w, r8.y, l(5.000000)
    r5.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 248: mul r7.w, r8.y, r8.y
    r7.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 249: mul r2.w, r2.w, r7.w
    r2.w = ((r2.wwww)*(r7.wwww)).w;
    // 250: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 251: add r2.w, r0.x, r2.w
    r2.w = ((r0.xxxx)+(r2.wwww)).w;
    // 252: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 253: add_sat r0.x, r2.w, l(-1.000000)
    r0.x = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 254: sample_l_indexable(texturecube)(float,float,float,float) r13.xyzw, r17.xyzx, t7.xyzw, s6, r5.w
    r13.xyzw = g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, r17.xyz, r5.w) : 0.f;
    // 255: mul r8.xyz, r13.xyzx, r13.wwww
    r8.xyz = ((r13.xyzx)*(r13.wwww)).xyz;
    // 256: mul r8.xyz, r8.xyzx, cb0[25].xyzx
    r8.xyz = ((r8.xyzx)*(source[25].xyzx)).xyz;
    // 257: mul r8.xyz, r8.xyzx, cb0[26].zzzz
    r8.xyz = ((r8.xyzx)*(source[26].zzzz)).xyz;
    // 258: mad r8.xyz, r8.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[25].wwww
    r8.xyz = ((r8.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[25].wwww)).xyz;
    // 259: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 260: add r8.xyz, -r2.wwww, r8.xyzx
    r8.xyz = ((-(r2.wwww))+(r8.xyzx)).xyz;
    // 261: mad r8.xyz, r8.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r2.wwww
    r8.xyz = ((r8.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r2.wwww)).xyz;
    // 262: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 263: div r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)/(r6.wwww)).w;
    // 264: mad r2.w, r4.w, l(5.000000), r2.w
    r2.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 265: add_sat r2.w, r8.w, r2.w
    r2.w = (saturate((r8.wwww)+(r2.wwww))).w;
    // 266: mad r4.w, r2.w, l(-2.000000), l(3.000000)
    r4.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 267: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 268: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 269: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 270: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 271: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 272: mul r8.xyz, r2.wwww, r8.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 273: mul r13.xyz, r8.xyzx, r9.xyzx
    r13.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 274: mad r2.w, r0.x, r3.x, r3.y
    r2.w = ((r0.xxxx)*(r3.xxxx)+(r3.yyyy)).w;
    // 275: mad r2.w, r2.w, r0.x, r3.z
    r2.w = ((r2.wwww)*(r0.xxxx)+(r3.zzzz)).w;
    // 276: mul r2.w, r0.x, r2.w
    r2.w = ((r0.xxxx)*(r2.wwww)).w;
    // 277: max r0.x, r0.x, r2.w
    r0.x = (max(r0.xxxx,r2.wwww)).x;
    // 278: mad r3.xyz, r13.xyzx, r0.xxxx, r11.xyzx
    r3.xyz = ((r13.xyzx)*(r0.xxxx)+(r11.xyzx)).xyz;
    // 279: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 280: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 281: mul r11.xyz, r2.wwww, v6.xyzx
    r11.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 282: dp3 r2.w, r11.xyzx, r6.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 283: dp3 r4.w, -r11.xyzx, r6.xyzx
    r4.w = (dot((-(r11.xyzx)).xyz,(r6.xyzx).xyz).xxxx).w;
    // 284: dp3 r5.w, r11.xyzx, r10.xyzx
    r5.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 285: mad r6.xy, r5.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r5.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 286: mad r6.zw, r4.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r4.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 287: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 288: mad r10.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 289: mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // 290: mul r10.yzw, r10.yyyy, cb0[36].xxyz
    r10.yzw = ((r10.yyyy)*(source[36].xxyz)).yzw;
    // 291: mad r10.xyz, r10.xxxx, cb0[35].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[35].xyzx)+(r10.yzwy)).xyz;
    // 292: mul r10.xyz, r10.xyzx, cb0[37].wwww
    r10.xyz = ((r10.xyzx)*(source[37].wwww)).xyz;
    // 293: mul r10.xyz, r1.xyzx, r10.xyzx
    r10.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 294: mul r2.xyz, r2.xyzx, r10.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)).xyz;
    // 295: mul r2.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 296: mul r2.xyz, r12.xyzx, r2.xyzx
    r2.xyz = ((r12.xyzx)*(r2.xyzx)).xyz;
    // 297: mad r2.xyz, -r2.xyzx, r8.wwww, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(r8.wwww)+(r2.xyzx)).xyz;
    // 298: mad r2.xyz, r3.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r2.xyzx)).xyz;
    // 299: mul r3.xyz, r6.yyyy, cb0[36].xyzx
    r3.xyz = ((r6.yyyy)*(source[36].xyzx)).xyz;
    // 300: mad r3.xyz, cb0[35].xyzx, r6.xxxx, r3.xyzx
    r3.xyz = ((source[35].xyzx)*(r6.xxxx)+(r3.xyzx)).xyz;
    // 301: mul r3.xyz, r3.xyzx, cb0[37].wwww
    r3.xyz = ((r3.xyzx)*(source[37].wwww)).xyz;
    // 302: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 303: mul r3.xyz, r8.xyzx, r3.xyzx
    r3.xyz = ((r8.xyzx)*(r3.xyzx)).xyz;
    // 304: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 305: mad r2.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r2.xyzx)).xyz;
    // 306: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 307: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 308: dp3 r0.x, r5.xyzx, r7.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 309: mul_sat r2.w, r0.x, cb0[20].z
    r2.w = (saturate((r0.xxxx)*(source[20].zzzz))).w;
    // 310: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 311: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 312: mul_sat r3.x, r7.z, cb0[20].z
    r3.x = (saturate((r7.zzzz)*(source[20].zzzz))).x;
    // 313: add r3.y, -|r7.z|, l(1.000000)
    r3.y = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 314: mul r0.x, r0.x, r3.y
    r0.x = ((r0.xxxx)*(r3.yyyy)).x;
    // 315: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 316: add_sat r3.x, r3.x, -cb0[20].w
    r3.x = (saturate((r3.xxxx)+(-(source[20].wwww)))).x;
    // 317: log r3.y, r3.x
    r3.y = (log2(r3.xxxx)).y;
    // 318: lt r3.x, r3.x, l(0.000001)
    r3.x = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 319: mul r3.y, r3.y, cb0[21].x
    r3.y = ((r3.yyyy)*(source[21].xxxx)).y;
    // 320: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 321: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 322: movc r2.w, r3.x, l(0), r2.w
    r2.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 323: mad r3.xyz, r2.wwww, cb0[13].xyzx, -cb0[13].xyzx
    r3.xyz = ((r2.wwww)*(source[13].xyzx)+(-(source[13].xyzx))).xyz;
    // 324: mul r2.w, r2.w, cb0[12].w
    r2.w = ((r2.wwww)*(source[12].wwww)).w;
    // 325: mad r3.xyz, cb0[13].wwww, r3.xyzx, cb0[13].xyzx
    r3.xyz = ((source[13].wwww)*(r3.xyzx)+(source[13].xyzx)).xyz;
    // 326: mad r3.xyz, r2.wwww, cb0[12].xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // 327: add r2.w, cb0[0].y, cb0[0].x
    r2.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 328: add r2.w, r2.w, cb0[0].z
    r2.w = ((r2.wwww)+(source[0].zzzz)).w;
    // 329: add r4.w, -r2.w, l(1000.000000)
    r4.w = ((-(r2.wwww))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).w;
    // 330: mad r2.w, cb0[19].w, r4.w, r2.w
    r2.w = ((source[19].wwww)*(r4.wwww)+(r2.wwww)).w;
    // 331: mul r2.w, r2.w, l(0.010000)
    r2.w = ((r2.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 332: mad r2.w, cb0[19].y, cb0[19].z, r2.w
    r2.w = ((source[19].yyyy)*(source[19].zzzz)+(r2.wwww)).w;
    // 333: mul r4.w, r2.w, l(3.524534)
    r4.w = ((r2.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 334: sincos null, r4.w, r4.w
    r4.w = (cos(r4.wwww)).w;
    // 335: add r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)+(r4.wwww)).w;
    // 336: mul r2.w, r2.w, l(1.328987)
    r2.w = ((r2.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 337: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 338: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 339: mad r2.w, r2.w, l(0.500000), cb0[19].x
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[19].xxxx)).w;
    // 340: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 341: mul r7.xyz, cb0[9].xyzx, cb0[18].wwww
    r7.xyz = ((source[9].xyzx)*(source[18].wwww)).xyz;
    // 342: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 343: mul r7.xyz, r2.wwww, r5.xyzx
    r7.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 344: dp3 r4.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 345: mad r5.xyz, -r2.wwww, r5.xyzx, r4.wwww
    r5.xyz = ((-(r2.wwww))*(r5.xyzx)+(r4.wwww)).xyz;
    // 346: mad r5.xyz, cb0[20].xxxx, r5.xyzx, r7.xyzx
    r5.xyz = ((source[20].xxxx)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 347: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 348: add r7.xyz, -r5.xyzx, r2.wwww
    r7.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 349: mad r5.xyz, cb0[20].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[20].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 350: mad r3.xyz, r5.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 351: log r2.w, |r0.x|
    r2.w = (log2(abs(r0.xxxx))).w;
    // 352: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 353: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 354: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 355: mul r4.xyz, r2.wwww, cb0[14].xyzx
    r4.xyz = ((r2.wwww)*(source[14].xyzx)).xyz;
    // 356: movc r4.xyz, r0.xxxx, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 357: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 358: mad r0.xyz, cb0[18].zzzz, r0.yzwy, r3.xyzx
    r0.xyz = ((source[18].zzzz)*(r0.yzwy)+(r3.xyzx)).xyz;
    // 359: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 360: mul r3.xyz, r6.wwww, cb0[36].xyzx
    r3.xyz = ((r6.wwww)*(source[36].xyzx)).xyz;
    // 361: mad r3.xyz, r6.zzzz, cb0[35].xyzx, r3.xyzx
    r3.xyz = ((r6.zzzz)*(source[35].xyzx)+(r3.xyzx)).xyz;
    // 362: mul r3.xyz, r3.xyzx, cb0[37].wwww
    r3.xyz = ((r3.xyzx)*(source[37].wwww)).xyz;
    // 363: mul_sat r4.xyz, cb0[17].xyzx, cb0[17].wwww
    r4.xyz = (saturate((source[17].xyzx)*(source[17].wwww))).xyz;
    // 364: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 365: mul r4.xyz, r4.xyzx, cb0[24].zzzz
    r4.xyz = ((r4.xyzx)*(source[24].zzzz)).xyz;
    // 366: dp3_sat o5.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 367: mul r4.xyz, r3.wwww, r5.xyzx
    r4.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 368: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 369: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 370: mad r0.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 371: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 372: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 373: mad o0.xyz, r1.xyzx, cb0[37].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[37].xyzx)+(r0.xyzx)).xyz;
    // 374: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 375: dp3 r0.x, r16.xyzx, r16.xyzx
    r0.x = (dot((r16.xyzx).xyz,(r16.xyzx).xyz).xxxx).x;
    // 376: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 377: mul r0.xyz, r0.xxxx, r16.xyzx
    r0.xyz = ((r0.xxxx)*(r16.xyzx)).xyz;
    // 378: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 379: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 380: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 381: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 382: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 383: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 384: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 385: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 386: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 387: ftou r0.x, cb0[34].z
    r0.x = (asfloat((uint4)(source[34].zzzz))).x;
    // 388: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 389: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 390: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 391: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 392: ret
    return output;
}

// source.character.realpbr-weapon-variation.v1 / source program 814e0fda0a51d94ab7b0e75d2e7ca814
