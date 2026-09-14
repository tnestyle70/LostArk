SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight80(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll] for(uint target=0u;target<6u;++target) output.targets[target]=0.f;
    output.discarded=false;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[6]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[8]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[9]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[12].x=(g_SourceCharacterTime.xxxx).x;
    source[12].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[12].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[13].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[13].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[13].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[14].x=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
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
    // 10: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s0
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
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 22: mov_sat r4.x, r4.x
    r4.x = (saturate(r4.xxxx)).x;
    // 23: add r2.w, r4.x, l(-0.333300)
    r2.w = ((r4.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 24: lt r2.w, r2.w, l(0.000000)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) { output.discarded = true; return output; }
    // 26: mul r5.xyz, cb0[4].xyzx, cb0[13].xxxx
    r5.xyz = ((source[4].xyzx)*(source[13].xxxx)).xyz;
    // 27: mul r6.xyz, r4.yzwy, r5.xyzx
    r6.xyz = ((r4.yzwy)*(r5.xyzx)).xyz;
    // 28: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: mad r4.xyz, -r4.yzwy, r5.xyzx, r2.wwww
    r4.xyz = ((-(r4.yzwy))*(r5.xyzx)+(r2.wwww)).xyz;
    // 30: mad r4.xyz, cb0[11].yyyy, r4.xyzx, r6.xyzx
    r4.xyz = ((source[11].yyyy)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 31: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: add r5.xyz, -r4.xyzx, r2.wwww
    r5.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 33: mad r4.xyz, cb0[11].zzzz, r5.xyzx, r4.xyzx
    r4.xyz = ((source[11].zzzz)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 34: mad r5.xyz, cb0[2].wwww, cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[2].wwww)*(source[2].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 35: mad r6.xyz, cb0[3].wwww, cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[3].wwww)*(source[3].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 36: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 37: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 38: mul r2.w, cb0[5].z, l(1.500000)
    r2.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 39: add r3.w, -cb0[5].w, l(1.000000)
    r3.w = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: mul r3.w, r3.w, cb0[12].x
    r3.w = ((r3.wwww)*(source[12].xxxx)).w;
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
    // 48: mul r5.z, r4.w, l(0.125000)
    r5.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 49: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 50: mul r5.y, cb0[5].y, cb0[6].y
    r5.y = ((source[5].yyyy)*(source[6].yyyy)).y;
    // 51: frc r4.w, v2.x
    r4.w = (frac(v2.xxxx)).w;
    // 52: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 53: mov r6.y, v2.y
    r6.y = (v2.yyyy).y;
    // 54: add r5.xy, r5.xyxx, r6.xyxx
    r5.xy = ((r5.xyxx)+(r6.xyxx)).xy;
    // 55: add r5.xy, r5.xyxx, r5.zwzz
    r5.xy = ((r5.xyxx)+(r5.zwzz)).xy;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 57: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 58: mul r2.w, r3.w, r5.w
    r2.w = ((r3.wwww)*(r5.wwww)).w;
    // 59: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 60: mad r4.xyz, r2.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 61: mul r2.w, cb0[7].y, cb0[12].x
    r2.w = ((source[7].yyyy)*(source[12].xxxx)).w;
    // 62: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 63: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 64: mul r5.y, r2.w, l(0.020000)
    r5.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
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
    // 70: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 71: mad r0.xy, r0.zzzz, r0.xyxx, r5.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(r5.xyxx)).xy;
    // 72: dp2 r0.z, cb0[8].xyxx, r0.xyxx
    r0.z = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 73: dp2 r0.y, cb0[9].xyxx, r0.xyxx
    r0.y = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 74: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 75: mul r0.x, r0.z, l(0.125000)
    r0.x = ((r0.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 77: mul r0.w, r0.w, l(0.900000)
    r0.w = ((r0.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 78: mad r0.xyz, r0.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r4.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r4.xyzx))).xyz;
    // 79: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 80: add r0.w, r2.w, l(1.000000)
    r0.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 82: mul_sat r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (saturate((r0.xyzx)*(r0.wwww))).xyz;
    // 83: mul r5.xyz, r0.xyzx, cb0[7].zzzz
    r5.xyz = ((r0.xyzx)*(source[7].zzzz)).xyz;
    // 84: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 85: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 86: mad r0.xyz, cb0[7].zzzz, r0.xyzx, -r4.xyzx
    r0.xyz = ((source[7].zzzz)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 87: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 88: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 89: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 90: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 91: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 93: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 94: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 95: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 96: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 97: div r5.xyz, r4.xyzx, r0.wwww
    r5.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 98: dp3_sat r0.w, r5.xyzx, r3.xyzx
    r0.w = (saturate(dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx)).w;
    // 99: mul r6.xyz, r1.xyzx, r0.wwww
    r6.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v2.xyxx, t2.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: add r8.xyz, -r7.xyzx, r0.wwww
    r8.xyz = ((-(r7.xyzx))+(r0.wwww)).xyz;
    // 103: mad r7.xyz, cb0[11].yyyy, r8.xyzx, r7.xyzx
    r7.xyz = ((source[11].yyyy)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 104: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r8.xyz, -r7.xyzx, r0.wwww
    r8.xyz = ((-(r7.xyzx))+(r0.wwww)).xyz;
    // 106: mad r7.xyz, cb0[11].zzzz, r8.xyzx, r7.xyzx
    r7.xyz = ((source[11].zzzz)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 107: mul r7.xyz, r7.xyzx, cb0[14].yyyy
    r7.xyz = ((r7.xyzx)*(source[14].yyyy)).xyz;
    // 108: mad r8.xyz, v3.xyzx, r1.wwww, r2.xyzx
    r8.xyz = ((v3.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 109: dp3 r0.w, r8.xyzx, r8.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 110: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 111: div r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)/(r1.wwww)).xyz;
    // 112: dp3 r1.w, r8.xyzx, r5.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 113: add r2.w, cb0[14].z, l(-1.000000)
    r2.w = ((source[14].zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 114: mad r2.w, r7.w, r2.w, l(1.000000)
    r2.w = ((r7.wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: lt r3.w, |r1.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 116: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 117: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 118: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 119: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 121: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 122: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 124: mul r7.xyz, r7.xyzx, r0.wwww
    r7.xyz = ((r7.xyzx)*(r0.wwww)).xyz;
    // 125: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 126: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 127: min r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 128: mad r0.xyz, r0.xyzx, r6.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 129: mul r1.xyz, cb0[10].xyzx, cb0[14].wwww
    r1.xyz = ((source[10].xyzx)*(source[14].wwww)).xyz;
    // 130: add r0.w, -|r2.z|, l(1.000000)
    r0.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: dp3 r1.w, r5.xyzx, r2.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 132: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 134: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 135: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 136: mul r0.w, r0.w, cb0[15].x
    r0.w = ((r0.wwww)*(source[15].xxxx)).w;
    // 137: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 138: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 139: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 140: mov_sat r0.w, -r3.z
    r0.w = (saturate(-(r3.zzzz))).w;
    // 141: mad r2.xyz, r0.wwww, r1.xyzx, -r1.xyzx
    r2.xyz = ((r0.wwww)*(r1.xyzx)+(-(r1.xyzx))).xyz;
    // 142: mad r1.xyz, cb0[10].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[10].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 143: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 144: dp3 r0.w, r4.xyzx, r3.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 145: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 146: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 147: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 148: mul o0.xyz, r0.xyzx, cb0[16].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[16].xyzx)).xyz;
    // 149: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 150: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 151: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 152: ret
    return output;
}

// source.character.monster-3c300c108ac5.v1 / source program e1c1d7945acda74bbb89550099576de3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight81(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll] for(uint target=0u;target<6u;++target) output.targets[target]=0.f;
    output.discarded=false;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[6]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[8]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[9]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[12].x=(g_SourceCharacterTime.xxxx).x;
    source[12].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[12].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[13].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[13].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[13].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[15]=float4(input.lightColor,1.0);
    source[16].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: add r0.xyzw, v6.yzxy, cb0[0].yzxy
    r0.xyzw = ((v6.yzxy)+(source[0].yzxy)).xyzw;
    // 2: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[16].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[16].xxxx)) * 0xffffffffu)).x;
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
    // 10: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s0
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
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 22: mul r5.xyz, cb0[4].xyzx, cb0[13].wwww
    r5.xyz = ((source[4].xyzx)*(source[13].wwww)).xyz;
    // 23: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 24: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 25: mad r4.xyz, -r4.xyzx, r5.xyzx, r2.wwww
    r4.xyz = ((-(r4.xyzx))*(r5.xyzx)+(r2.wwww)).xyz;
    // 26: mad r4.xyz, cb0[11].yyyy, r4.xyzx, r6.xyzx
    r4.xyz = ((source[11].yyyy)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 27: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 28: add r5.xyz, -r4.xyzx, r2.wwww
    r5.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 29: mad r4.xyz, cb0[11].zzzz, r5.xyzx, r4.xyzx
    r4.xyz = ((source[11].zzzz)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 30: mad r5.xyz, cb0[2].wwww, cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[2].wwww)*(source[2].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 31: mad r6.xyz, cb0[3].wwww, cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[3].wwww)*(source[3].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 32: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 33: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 34: mul r2.w, cb0[5].z, l(1.500000)
    r2.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 35: add r3.w, -cb0[5].w, l(1.000000)
    r3.w = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: mul r3.w, r3.w, cb0[12].x
    r3.w = ((r3.wwww)*(source[12].xxxx)).w;
    // 37: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 38: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 39: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 41: mad r2.w, r2.w, l(0.500000), cb0[5].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 42: frc r3.w, cb0[5].x
    r3.w = (frac(source[5].xxxx)).w;
    // 43: add r4.w, -r3.w, cb0[5].x
    r4.w = ((-(r3.wwww))+(source[5].xxxx)).w;
    // 44: mul r5.z, r4.w, l(0.125000)
    r5.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 45: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 46: mul r5.y, cb0[5].y, cb0[6].y
    r5.y = ((source[5].yyyy)*(source[6].yyyy)).y;
    // 47: frc r4.w, v2.x
    r4.w = (frac(v2.xxxx)).w;
    // 48: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 49: mov r6.y, v2.y
    r6.y = (v2.yyyy).y;
    // 50: add r5.xy, r5.xyxx, r6.xyxx
    r5.xy = ((r5.xyxx)+(r6.xyxx)).xy;
    // 51: add r5.xy, r5.xyxx, r5.zwzz
    r5.xy = ((r5.xyxx)+(r5.zwzz)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 53: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 54: mul r2.w, r3.w, r5.w
    r2.w = ((r3.wwww)*(r5.wwww)).w;
    // 55: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 56: mad r4.xyz, r2.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 57: mul r2.w, cb0[7].y, cb0[12].x
    r2.w = ((source[7].yyyy)*(source[12].xxxx)).w;
    // 58: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 59: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 60: mul r5.y, r2.w, l(0.020000)
    r5.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 61: add r0.xyzw, r0.xyzw, -cb0[1].yzxy
    r0.xyzw = ((r0.xyzw)+(-(source[1].yzxy))).xyzw;
    // 62: add r0.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 63: add r0.xy, -r0.zwzz, r0.xyxx
    r0.xy = ((-(r0.zwzz))+(r0.xyxx)).xy;
    // 64: mad r0.xy, cb0[7].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[7].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 65: mul r0.z, cb0[7].x, l(0.001000)
    r0.z = ((source[7].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 66: mov r5.x, l(0)
    r5.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 67: mad r0.xy, r0.zzzz, r0.xyxx, r5.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(r5.xyxx)).xy;
    // 68: dp2 r0.z, cb0[8].xyxx, r0.xyxx
    r0.z = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 69: dp2 r0.y, cb0[9].xyxx, r0.xyxx
    r0.y = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 70: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 71: mul r0.x, r0.z, l(0.125000)
    r0.x = ((r0.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 73: mul r0.w, r0.w, l(0.900000)
    r0.w = ((r0.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 74: mad r0.xyz, r0.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r4.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r4.xyzx))).xyz;
    // 75: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 76: add r0.w, r2.w, l(1.000000)
    r0.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 78: mul_sat r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (saturate((r0.xyzx)*(r0.wwww))).xyz;
    // 79: mul r5.xyz, r0.xyzx, cb0[7].zzzz
    r5.xyz = ((r0.xyzx)*(source[7].zzzz)).xyz;
    // 80: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 82: mad r0.xyz, cb0[7].zzzz, r0.xyzx, -r4.xyzx
    r0.xyz = ((source[7].zzzz)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 83: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 84: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 85: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 86: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 87: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 89: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 90: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 91: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 92: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 93: div r5.xyz, r4.xyzx, r0.wwww
    r5.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 94: dp3_sat r0.w, r5.xyzx, r3.xyzx
    r0.w = (saturate(dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx)).w;
    // 95: mul r6.xyz, r1.xyzx, r0.wwww
    r6.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v2.xyxx, t1.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 97: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: add r8.xyz, -r7.xyzx, r0.wwww
    r8.xyz = ((-(r7.xyzx))+(r0.wwww)).xyz;
    // 99: mad r7.xyz, cb0[11].yyyy, r8.xyzx, r7.xyzx
    r7.xyz = ((source[11].yyyy)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 100: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: add r8.xyz, -r7.xyzx, r0.wwww
    r8.xyz = ((-(r7.xyzx))+(r0.wwww)).xyz;
    // 102: mad r7.xyz, cb0[11].zzzz, r8.xyzx, r7.xyzx
    r7.xyz = ((source[11].zzzz)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 103: mul r7.xyz, r7.xyzx, cb0[14].xxxx
    r7.xyz = ((r7.xyzx)*(source[14].xxxx)).xyz;
    // 104: mad r8.xyz, v3.xyzx, r1.wwww, r2.xyzx
    r8.xyz = ((v3.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 105: dp3 r0.w, r8.xyzx, r8.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 106: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 107: div r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)/(r1.wwww)).xyz;
    // 108: dp3 r1.w, r8.xyzx, r5.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 109: add r2.w, cb0[14].y, l(-1.000000)
    r2.w = ((source[14].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 110: mad r2.w, r7.w, r2.w, l(1.000000)
    r2.w = ((r7.wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: lt r3.w, |r1.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 112: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 113: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 114: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 115: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 117: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 118: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 120: mul r7.xyz, r7.xyzx, r0.wwww
    r7.xyz = ((r7.xyzx)*(r0.wwww)).xyz;
    // 121: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 122: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 123: min r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 124: mad r0.xyz, r0.xyzx, r6.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 125: mul r1.xyz, cb0[10].xyzx, cb0[14].zzzz
    r1.xyz = ((source[10].xyzx)*(source[14].zzzz)).xyz;
    // 126: add r0.w, -|r2.z|, l(1.000000)
    r0.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: dp3 r1.w, r5.xyzx, r2.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 128: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 130: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 131: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 132: mul r0.w, r0.w, cb0[14].w
    r0.w = ((r0.wwww)*(source[14].wwww)).w;
    // 133: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 134: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 135: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 136: mov_sat r0.w, -r3.z
    r0.w = (saturate(-(r3.zzzz))).w;
    // 137: mad r2.xyz, r0.wwww, r1.xyzx, -r1.xyzx
    r2.xyz = ((r0.wwww)*(r1.xyzx)+(-(r1.xyzx))).xyz;
    // 138: mad r1.xyz, cb0[10].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[10].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 139: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 140: dp3 r0.w, r4.xyzx, r3.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 141: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 142: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 143: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 144: mul o0.xyz, r0.xyzx, cb0[15].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[15].xyzx)).xyz;
    // 145: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 146: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 147: ret
    return output;
}

// source.character.monster-7373ec8df226.v1 / source program 3c8414cd88a2e94289471d94c5ab08de
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight82(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll] for(uint target=0u;target<6u;++target) output.targets[target]=0.f;
    output.discarded=false;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[4].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[4].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[7]=float4(input.lightColor,1.0);
    source[8].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[8].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[8].xxxx)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
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
    // 10: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: dp3 r1.x, v3.xyzx, v3.xyzx
    r1.x = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).x;
    // 13: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 14: mul r1.xyz, r1.xxxx, v3.xyzx
    r1.xyz = ((r1.xxxx)*(v3.xyzx)).xyz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 16: mul r3.xy, v2.xyxx, cb0[6].xxxx
    r3.xy = ((v2.xyxx)*(source[6].xxxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.xyxx, t2.yzwx, s3, l(0.000000)
    r1.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 18: add r1.w, r1.w, -cb0[6].y
    r1.w = ((r1.wwww)+(-(source[6].yyyy))).w;
    // 19: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 20: mul_sat r1.w, r1.w, r2.w
    r1.w = (saturate((r1.wwww)*(r2.wwww))).w;
    // 21: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 22: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 23: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 24: mul r3.xyz, cb0[2].xyzx, cb0[4].wwww
    r3.xyz = ((source[2].xyzx)*(source[4].wwww)).xyz;
    // 25: mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 26: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 27: mad r2.xyz, -r2.xyzx, r3.xyzx, r1.wwww
    r2.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 28: mad r3.xyz, cb0[3].xxxx, r2.xyzx, r4.xyzx
    r3.xyz = ((source[3].xxxx)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 29: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 30: add r5.xyz, -r3.xyzx, r1.wwww
    r5.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 31: mad r3.xyz, cb0[3].yyyy, r5.xyzx, r3.xyzx
    r3.xyz = ((source[3].yyyy)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 32: mad r5.xyz, cb0[0].wwww, cb0[0].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[0].wwww)*(source[0].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 33: mad r6.xyz, cb0[1].wwww, cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[1].wwww)*(source[1].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 34: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 35: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 37: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 38: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 39: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 42: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 43: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 44: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 45: div r6.xyz, r5.xyzx, r1.wwww
    r6.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 46: dp3_sat r1.w, r6.xyzx, r1.xyzx
    r1.w = (saturate(dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx)).w;
    // 47: mul r7.xyz, r0.xyzx, r1.wwww
    r7.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 48: mad r2.xyz, cb0[5].yyyy, r2.xyzx, r4.xyzx
    r2.xyz = ((source[5].yyyy)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 49: mul r2.xyz, r2.xyzx, cb0[5].zzzz
    r2.xyz = ((r2.xyzx)*(source[5].zzzz)).xyz;
    // 50: mad r4.xyz, v5.xyzx, r0.wwww, r1.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 51: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 52: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 53: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 54: dp3 r1.w, r4.xyzx, r6.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 55: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 56: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 57: mul r1.w, r1.w, cb0[5].w
    r1.w = ((r1.wwww)*(source[5].wwww)).w;
    // 58: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 59: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 61: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 62: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 63: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 64: mul r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // 65: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 66: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 67: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 68: mad r0.xyz, r3.xyzx, r7.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r7.xyzx)+(r0.xyzx)).xyz;
    // 69: dp3 r0.w, r5.xyzx, r1.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 70: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 71: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 72: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 73: mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // 74: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 75: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 76: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 77: ret
    return output;
}

// source.character.monster-a2e0ec089348.v1 / source program f25a8efce0d76248b9ccf22495e19da3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight83(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll] for(uint target=0u;target<6u;++target) output.targets[target]=0.f;
    output.discarded=false;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[7]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[9]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[10]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[13].w=(g_SourceCharacterTime.xxxx).x;
    source[14].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[14].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
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
    // 10: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s0
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
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 22: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 23: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 24: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 25: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 26: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 27: add r4.z, r2.w, l(0.000010)
    r4.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 28: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 29: rsq r3.w, r2.w
    r3.w = (rsqrt(r2.wwww)).w;
    // 30: mul r5.xyz, r3.wwww, r4.xyzx
    r5.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 31: dp3 r3.w, r5.xyzx, r2.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 32: mul r5.xy, r3.wwww, r5.xyxx
    r5.xy = ((r3.wwww)*(r5.xyxx)).xy;
    // 33: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r2.xyxx
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r2.xyxx))).xy;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 35: mul r7.xyz, cb0[4].xyzx, cb0[14].zzzz
    r7.xyz = ((source[4].xyzx)*(source[14].zzzz)).xyz;
    // 36: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 37: add r5.xy, r5.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 38: mul r5.xy, r5.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v2.xyxx, t1.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: add r3.w, r7.w, l(-0.650000)
    r3.w = ((r7.wwww)+(float4(-0.650000,-0.650000,-0.650000,-0.650000))).w;
    // 42: mul_sat r3.w, r3.w, l(2.857142)
    r3.w = (saturate((r3.wwww)*(float4(2.857142,2.857142,2.857142,2.857142)))).w;
    // 43: mul r3.w, r3.w, cb0[14].w
    r3.w = ((r3.wwww)*(source[14].wwww)).w;
    // 44: mad r5.xyz, r5.xyzx, cb0[5].xyzx, -r6.xyzx
    r5.xyz = ((r5.xyzx)*(source[5].xyzx)+(-(r6.xyzx))).xyz;
    // 45: mad r5.xyz, r3.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 46: dp3 r3.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 47: add r6.xyz, -r5.xyzx, r3.wwww
    r6.xyz = ((-(r5.xyzx))+(r3.wwww)).xyz;
    // 48: mad r5.xyz, cb0[12].xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((source[12].xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 49: dp3 r3.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 50: add r6.xyz, -r5.xyzx, r3.wwww
    r6.xyz = ((-(r5.xyzx))+(r3.wwww)).xyz;
    // 51: mad r5.xyz, cb0[12].yyyy, r6.xyzx, r5.xyzx
    r5.xyz = ((source[12].yyyy)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 52: mad r6.xyz, cb0[2].wwww, cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[2].wwww)*(source[2].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 53: mad r8.xyz, cb0[3].wwww, cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[3].wwww)*(source[3].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 55: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 56: mul r3.w, cb0[6].z, l(1.500000)
    r3.w = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 57: add r4.w, -cb0[6].w, l(1.000000)
    r4.w = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: mul r4.w, r4.w, cb0[13].w
    r4.w = ((r4.wwww)*(source[13].wwww)).w;
    // 59: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 60: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 61: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 62: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 63: mad r3.w, r3.w, l(0.500000), cb0[6].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 64: frc r4.w, cb0[6].x
    r4.w = (frac(source[6].xxxx)).w;
    // 65: add r5.w, -r4.w, cb0[6].x
    r5.w = ((-(r4.wwww))+(source[6].xxxx)).w;
    // 66: mul r6.z, r5.w, l(0.125000)
    r6.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 67: mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 68: mul r6.y, cb0[6].y, cb0[7].y
    r6.y = ((source[6].yyyy)*(source[7].yyyy)).y;
    // 69: frc r5.w, v2.x
    r5.w = (frac(v2.xxxx)).w;
    // 70: mul r8.x, r5.w, l(0.125000)
    r8.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 71: mov r8.y, v2.y
    r8.y = (v2.yyyy).y;
    // 72: add r6.xy, r6.xyxx, r8.xyxx
    r6.xy = ((r6.xyxx)+(r8.xyxx)).xy;
    // 73: add r6.xy, r6.xyxx, r6.zwzz
    r6.xy = ((r6.xyxx)+(r6.zwzz)).xy;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t4.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceMapMonsterStateSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 75: mul r6.xyz, r3.wwww, r6.xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)).xyz;
    // 76: mul r3.w, r4.w, r6.w
    r3.w = ((r4.wwww)*(r6.wwww)).w;
    // 77: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xyzx))).xyz;
    // 78: mad r5.xyz, r3.wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((r3.wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 79: mul r3.w, cb0[8].y, cb0[13].w
    r3.w = ((source[8].yyyy)*(source[13].wwww)).w;
    // 80: mul r3.w, r3.w, l(0.628319)
    r3.w = ((r3.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 81: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 82: mul r6.y, r3.w, l(0.020000)
    r6.y = ((r3.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 83: add r0.xyzw, r0.xyzw, -cb0[1].yzxy
    r0.xyzw = ((r0.xyzw)+(-(source[1].yzxy))).xyzw;
    // 84: add r0.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 85: add r0.xy, -r0.zwzz, r0.xyxx
    r0.xy = ((-(r0.zwzz))+(r0.xyxx)).xy;
    // 86: mad r0.xy, cb0[8].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[8].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 87: mul r0.z, cb0[8].x, l(0.001000)
    r0.z = ((source[8].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 88: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 89: mad r0.xy, r0.zzzz, r0.xyxx, r6.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(r6.xyxx)).xy;
    // 90: dp2 r0.z, cb0[9].xyxx, r0.xyxx
    r0.z = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 91: dp2 r0.y, cb0[10].xyxx, r0.xyxx
    r0.y = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 92: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 93: mul r0.x, r0.z, l(0.125000)
    r0.x = ((r0.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 94: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t4.xyzw, s5, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceMapMonsterStateSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 95: mul r0.w, r0.w, l(0.900000)
    r0.w = ((r0.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 96: mad r0.xyz, r0.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r5.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r5.xyzx))).xyz;
    // 97: mad r0.xyz, r0.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 98: add r0.w, r3.w, l(1.000000)
    r0.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 100: mul_sat r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (saturate((r0.xyzx)*(r0.wwww))).xyz;
    // 101: mul r6.xyz, r0.xyzx, cb0[8].zzzz
    r6.xyz = ((r0.xyzx)*(source[8].zzzz)).xyz;
    // 102: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 104: mad r0.xyz, cb0[8].zzzz, r0.xyzx, -r5.xyzx
    r0.xyz = ((source[8].zzzz)*(r0.xyzx)+(-(r5.xyzx))).xyz;
    // 105: mad r0.xyz, r0.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 106: sqrt r0.w, r2.w
    r0.w = (sqrt(r2.wwww)).w;
    // 107: div r5.xyz, r4.xyzx, r0.wwww
    r5.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 108: dp3_sat r0.w, r5.xyzx, r3.xyzx
    r0.w = (saturate(dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx)).w;
    // 109: mul r6.xyz, r1.xyzx, r0.wwww
    r6.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 110: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 111: add r8.xyz, -r7.xyzx, r0.wwww
    r8.xyz = ((-(r7.xyzx))+(r0.wwww)).xyz;
    // 112: mad r7.xyz, cb0[12].xxxx, r8.xyzx, r7.xyzx
    r7.xyz = ((source[12].xxxx)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 113: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: add r8.xyz, -r7.xyzx, r0.wwww
    r8.xyz = ((-(r7.xyzx))+(r0.wwww)).xyz;
    // 115: mad r7.xyz, cb0[12].yyyy, r8.xyzx, r7.xyzx
    r7.xyz = ((source[12].yyyy)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 116: mul r7.xyz, r7.xyzx, cb0[15].xxxx
    r7.xyz = ((r7.xyzx)*(source[15].xxxx)).xyz;
    // 117: mad r8.xyz, v3.xyzx, r1.wwww, r2.xyzx
    r8.xyz = ((v3.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 118: dp3 r0.w, r8.xyzx, r8.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 119: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 120: div r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)/(r1.wwww)).xyz;
    // 121: dp3 r1.w, r8.xyzx, r5.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 122: add r2.w, cb0[15].y, l(-1.000000)
    r2.w = ((source[15].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 123: mad r2.w, r7.w, r2.w, l(1.000000)
    r2.w = ((r7.wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: lt r3.w, |r1.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 125: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 126: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 127: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 128: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 130: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 131: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 133: mul r7.xyz, r7.xyzx, r0.wwww
    r7.xyz = ((r7.xyzx)*(r0.wwww)).xyz;
    // 134: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 135: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 136: min r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 137: mad r0.xyz, r0.xyzx, r6.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 138: mul r1.xyz, cb0[11].xyzx, cb0[15].zzzz
    r1.xyz = ((source[11].xyzx)*(source[15].zzzz)).xyz;
    // 139: add r0.w, -|r2.z|, l(1.000000)
    r0.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: dp3 r1.w, r5.xyzx, r2.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 141: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 143: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 144: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 145: mul r0.w, r0.w, cb0[15].w
    r0.w = ((r0.wwww)*(source[15].wwww)).w;
    // 146: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 147: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 148: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 149: mov_sat r0.w, -r3.z
    r0.w = (saturate(-(r3.zzzz))).w;
    // 150: mad r2.xyz, r0.wwww, r1.xyzx, -r1.xyzx
    r2.xyz = ((r0.wwww)*(r1.xyzx)+(-(r1.xyzx))).xyz;
    // 151: mad r1.xyz, cb0[11].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[11].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 152: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 153: dp3 r0.w, r4.xyzx, r3.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 154: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 155: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 156: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 157: mul o0.xyz, r0.xyzx, cb0[16].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[16].xyzx)).xyz;
    // 158: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 159: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 160: ret
    return output;
}



// source.character.monster-8d18db0756e4.v1 / source program 3d121fd2c1f7524da32c33b18452083e
