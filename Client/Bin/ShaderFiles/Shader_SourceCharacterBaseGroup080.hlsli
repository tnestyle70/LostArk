SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase80(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll] for(uint target=0u;target<6u;++target) output.targets[target]=0.f;
    output.discarded=false;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[18].x=(g_SourceCharacterTime.xxxx).x;
    source[18].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[18].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[19].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[19].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[19].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[20].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[20].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[20].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
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
    // 7: mul r1.xyz, cb0[10].xyzx, cb0[20].xxxx
    r1.xyz = ((source[10].xyzx)*(source[20].xxxx)).xyz;
    // 8: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 9: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 10: mad r0.xyz, -r0.yzwy, r1.xyzx, r0.xxxx
    r0.xyz = ((-(r0.yzwy))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 11: mad r0.xyz, cb0[16].yyyy, r0.xyzx, r2.xyzx
    r0.xyz = ((source[16].yyyy)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 12: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 13: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 14: mad r0.xyz, cb0[16].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[16].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 15: mad r1.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 16: mad r2.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 17: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 18: add r0.w, -cb0[11].w, l(1.000000)
    r0.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 19: mul r0.w, r0.w, cb0[18].x
    r0.w = ((r0.wwww)*(source[18].xxxx)).w;
    // 20: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 21: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 22: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: mul r1.w, cb0[11].z, l(1.500000)
    r1.w = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 24: mul r0.xyzw, r0.xyzw, r1.xyzw
    r0.xyzw = ((r0.xyzw)*(r1.xyzw)).xyzw;
    // 25: mad r0.w, r0.w, l(0.500000), cb0[11].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 26: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 27: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 28: mul r3.y, cb0[11].y, cb0[12].y
    r3.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 29: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 30: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 31: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 32: frc r1.w, cb0[11].x
    r1.w = (frac(source[11].xxxx)).w;
    // 33: add r2.z, -r1.w, cb0[11].x
    r2.z = ((-(r1.wwww))+(source[11].xxxx)).z;
    // 34: mul r3.z, r2.z, l(0.125000)
    r3.z = ((r2.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 35: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 38: mul r0.w, r1.w, r2.w
    r0.w = ((r1.wwww)*(r2.wwww)).w;
    // 39: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 40: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 41: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 42: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 43: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 44: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 45: mad r2.xy, cb0[13].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[13].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 46: mul r0.w, cb0[13].y, cb0[18].x
    r0.w = ((source[13].yyyy)*(source[18].xxxx)).w;
    // 47: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 48: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 49: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 50: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 52: mul r1.w, cb0[13].x, l(0.001000)
    r1.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 53: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 54: mad r2.xy, r1.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 55: dp2 r1.w, cb0[14].xyxx, r2.xyxx
    r1.w = (dot((source[14].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 56: dp2 r2.y, cb0[15].xyxx, r2.xyxx
    r2.y = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 57: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 58: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 60: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 61: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 62: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 63: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 64: mad r3.xyz, cb0[13].zzzz, r2.xyzx, -r0.xyzx
    r3.xyz = ((source[13].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 65: mul r2.xyz, r2.xyzx, cb0[13].zzzz
    r2.xyz = ((r2.xyzx)*(source[13].zzzz)).xyz;
    // 66: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 67: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 68: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 69: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 71: mul r3.xyz, cb0[4].xyzx, cb0[16].xxxx
    r3.xyz = ((source[4].xyzx)*(source[16].xxxx)).xyz;
    // 72: mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 73: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: mad r2.xyz, -r2.xyzx, r3.xyzx, r0.wwww
    r2.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r0.wwww)).xyz;
    // 75: mad r2.xyz, cb0[16].yyyy, r2.xyzx, r4.xyzx
    r2.xyz = ((source[16].yyyy)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[16].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r3.xyz, cb0[9].xyzx, cb0[17].wwww
    r3.xyz = ((source[9].xyzx)*(source[17].wwww)).xyz;
    // 80: mul r3.xyz, r3.xyzx, cb0[19].zzzz
    r3.xyz = ((r3.xyzx)*(source[19].zzzz)).xyz;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 82: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 83: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 84: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 86: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 87: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 88: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 89: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 90: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 91: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 92: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 93: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 94: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 95: mul r6.xyz, r0.wwww, v5.xyzx
    r6.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 96: dp3 r0.w, r4.xyzx, r6.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 97: add r1.w, -|r6.z|, l(1.000000)
    r1.w = ((-(abs(r6.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 100: mul r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 101: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 102: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 103: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 104: mul r3.xyz, r3.xyzx, cb0[19].wwww
    r3.xyz = ((r3.xyzx)*(source[19].wwww)).xyz;
    // 105: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 106: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 107: mad_sat r1.w, r0.w, cb0[16].w, -cb0[17].x
    r1.w = (saturate((r0.wwww)*(source[16].wwww)+(-(source[17].xxxx)))).w;
    // 108: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 109: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 110: mul r2.w, r2.w, cb0[17].y
    r2.w = ((r2.wwww)*(source[17].yyyy)).w;
    // 111: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 112: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 113: mad r4.xyz, r1.wwww, cb0[8].xyzx, -cb0[8].xyzx
    r4.xyz = ((r1.wwww)*(source[8].xyzx)+(-(source[8].xyzx))).xyz;
    // 114: mad r6.xyz, r1.wwww, cb0[7].xyzx, -cb0[7].xyzx
    r6.xyz = ((r1.wwww)*(source[7].xyzx)+(-(source[7].xyzx))).xyz;
    // 115: mad r6.xyz, cb0[7].wwww, r6.xyzx, cb0[7].xyzx
    r6.xyz = ((source[7].wwww)*(r6.xyzx)+(source[7].xyzx)).xyz;
    // 116: mad r4.xyz, cb0[8].wwww, r4.xyzx, cb0[8].xyzx
    r4.xyz = ((source[8].wwww)*(r4.xyzx)+(source[8].xyzx)).xyz;
    // 117: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 118: mad r3.xyz, cb0[17].zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((source[17].zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 119: add r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)+(r6.xyzx)).xyz;
    // 120: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 121: mad r1.xyz, r2.xyzx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 122: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 123: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 124: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 125: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 126: mul r2.xyz, r1.wwww, cb0[3].xyzx
    r2.xyz = ((r1.wwww)*(source[3].xyzx)).xyz;
    // 127: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 128: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 129: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 130: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 131: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 132: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 133: dp3 r0.w, r2.xyzx, r5.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 134: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 135: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 136: mul r2.yzw, r2.yyyy, cb0[22].xxyz
    r2.yzw = ((r2.yyyy)*(source[22].xxyz)).yzw;
    // 137: mad r2.xyz, r2.xxxx, cb0[21].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[21].xyzx)+(r2.yzwy)).xyz;
    // 138: mul r2.xyz, r2.xyzx, cb0[23].wwww
    r2.xyz = ((r2.xyzx)*(source[23].wwww)).xyz;
    // 139: mad r1.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 140: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 141: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 142: mad o0.xyz, r0.xyzx, cb0[23].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)+(r1.xyzx)).xyz;
    // 143: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 144: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 145: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 146: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 147: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 148: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 149: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 150: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 151: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 152: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 153: dp3 r0.z, r0.xyzx, r5.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 154: dp3 r0.x, r1.xyzx, r5.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 155: mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 156: dp3 r0.y, r1.xyzx, r5.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 157: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 158: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 159: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 160: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 161: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 162: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 163: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 164: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 165: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 166: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 167: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 168: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 169: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 170: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 171: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 172: ret
    return output;
}

// source.character.monster-3c300c108ac5.v1 / source program e8c1b29a3770b241ad49af2eee5a988b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase81(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll] for(uint target=0u;target<6u;++target) output.targets[target]=0.f;
    output.discarded=false;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[18].x=(g_SourceCharacterTime.xxxx).x;
    source[18].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[18].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[19].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[19].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[19].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: add r0.x, -cb0[11].w, l(1.000000)
    r0.x = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: mul r0.x, r0.x, cb0[18].x
    r0.x = ((r0.xxxx)*(source[18].xxxx)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 4: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 5: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: mul r0.y, cb0[11].z, l(1.500000)
    r0.y = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 7: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 8: mad r0.x, r0.x, l(0.500000), cb0[11].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).x;
    // 9: frc r0.y, v4.x
    r0.y = (frac(v4.xxxx)).y;
    // 10: mul r1.x, r0.y, l(0.125000)
    r1.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 11: mul r2.y, cb0[11].y, cb0[12].y
    r2.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 12: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 13: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 14: add r0.yz, r1.xxyx, r2.xxyx
    r0.yz = ((r1.xxyx)+(r2.xxyx)).yz;
    // 15: frc r0.w, cb0[11].x
    r0.w = (frac(source[11].xxxx)).w;
    // 16: add r1.x, -r0.w, cb0[11].x
    r1.x = ((-(r0.wwww))+(source[11].xxxx)).x;
    // 17: mul r2.z, r1.x, l(0.125000)
    r2.z = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 18: add r0.yz, r0.yyzy, r2.zzwz
    r0.yz = ((r0.yyzy)+(r2.zzwz)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t2.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 20: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 21: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 23: mul r2.xyz, cb0[10].xyzx, cb0[19].wwww
    r2.xyz = ((source[10].xyzx)*(source[19].wwww)).xyz;
    // 24: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 25: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: mad r1.xyz, -r1.xyzx, r2.xyzx, r1.wwww
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r1.wwww)).xyz;
    // 27: mad r1.xyz, cb0[16].yyyy, r1.xyzx, r3.xyzx
    r1.xyz = ((source[16].yyyy)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 28: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 30: mad r1.xyz, cb0[16].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[16].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 31: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 32: mad r3.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 33: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 34: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 35: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 36: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 37: add r1.xyzw, v7.yzxy, cb0[0].yzxy
    r1.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 38: add r1.xyzw, r1.xyzw, -cb0[1].yzxy
    r1.xyzw = ((r1.xyzw)+(-(source[1].yzxy))).xyzw;
    // 39: add r1.xy, -r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r1.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 40: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 41: mad r1.xy, cb0[13].wwww, r1.xyxx, r1.zwzz
    r1.xy = ((source[13].wwww)*(r1.xyxx)+(r1.zwzz)).xy;
    // 42: mul r0.w, cb0[13].y, cb0[18].x
    r0.w = ((source[13].yyyy)*(source[18].xxxx)).w;
    // 43: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 44: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 45: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 46: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 48: mul r1.z, cb0[13].x, l(0.001000)
    r1.z = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 49: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 50: mad r1.xy, r1.zzzz, r1.xyxx, r3.xyxx
    r1.xy = ((r1.zzzz)*(r1.xyxx)+(r3.xyxx)).xy;
    // 51: dp2 r1.z, cb0[14].xyxx, r1.xyxx
    r1.z = (dot((source[14].xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 52: dp2 r1.y, cb0[15].xyxx, r1.xyxx
    r1.y = (dot((source[15].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 53: frc r1.z, r1.z
    r1.z = (frac(r1.zzzz)).z;
    // 54: mul r1.x, r1.z, l(0.125000)
    r1.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 56: mad r1.xyz, r1.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 57: mul r1.w, r1.w, l(0.900000)
    r1.w = ((r1.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 58: mad r1.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 59: mul_sat r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (saturate((r0.wwww)*(r1.xyzx))).xyz;
    // 60: mad r3.xyz, cb0[13].zzzz, r1.xyzx, -r0.xyzx
    r3.xyz = ((source[13].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 61: mul r1.xyz, r1.xyzx, cb0[13].zzzz
    r1.xyz = ((r1.xyzx)*(source[13].zzzz)).xyz;
    // 62: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 63: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 64: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 65: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 67: mul r3.xyz, cb0[4].xyzx, cb0[16].xxxx
    r3.xyz = ((source[4].xyzx)*(source[16].xxxx)).xyz;
    // 68: mul r4.xyz, r1.xyzx, r3.xyzx
    r4.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 69: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: mad r1.xyz, -r1.xyzx, r3.xyzx, r0.wwww
    r1.xyz = ((-(r1.xyzx))*(r3.xyzx)+(r0.wwww)).xyz;
    // 71: mad r1.xyz, cb0[16].yyyy, r1.xyzx, r4.xyzx
    r1.xyz = ((source[16].yyyy)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 72: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r3.xyz, -r1.xyzx, r0.wwww
    r3.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 74: mad r1.xyz, cb0[16].zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((source[16].zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 75: mul r3.xyz, cb0[9].xyzx, cb0[17].wwww
    r3.xyz = ((source[9].xyzx)*(source[17].wwww)).xyz;
    // 76: mul r3.xyz, r3.xyzx, cb0[19].zzzz
    r3.xyz = ((r3.xyzx)*(source[19].zzzz)).xyz;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 78: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 79: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 80: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 82: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 83: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 84: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 85: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 86: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 87: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 88: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 89: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 90: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 91: mul r6.xyz, r0.wwww, v5.xyzx
    r6.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 92: dp3 r0.w, r4.xyzx, r6.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 93: add r1.w, -|r6.z|, l(1.000000)
    r1.w = ((-(abs(r6.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 96: mul r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 97: mad_sat r1.w, r0.w, cb0[16].w, -cb0[17].x
    r1.w = (saturate((r0.wwww)*(source[16].wwww)+(-(source[17].xxxx)))).w;
    // 98: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 99: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 100: mul r2.w, r2.w, cb0[17].y
    r2.w = ((r2.wwww)*(source[17].yyyy)).w;
    // 101: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 102: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 103: mad r4.xyz, r1.wwww, cb0[8].xyzx, -cb0[8].xyzx
    r4.xyz = ((r1.wwww)*(source[8].xyzx)+(-(source[8].xyzx))).xyz;
    // 104: mad r6.xyz, r1.wwww, cb0[7].xyzx, -cb0[7].xyzx
    r6.xyz = ((r1.wwww)*(source[7].xyzx)+(-(source[7].xyzx))).xyz;
    // 105: mad r6.xyz, cb0[7].wwww, r6.xyzx, cb0[7].xyzx
    r6.xyz = ((source[7].wwww)*(r6.xyzx)+(source[7].xyzx)).xyz;
    // 106: mad r4.xyz, cb0[8].wwww, r4.xyzx, cb0[8].xyzx
    r4.xyz = ((source[8].wwww)*(r4.xyzx)+(source[8].xyzx)).xyz;
    // 107: mad r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), -r4.xyzx
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(-(r4.xyzx))).xyz;
    // 108: mad r3.xyz, cb0[17].zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((source[17].zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 109: add r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)+(r6.xyzx)).xyz;
    // 110: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 111: mad r1.xyz, r1.xyzx, r2.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 112: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 113: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 114: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 115: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 116: mul r2.xyz, cb0[3].xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = ((source[3].xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 117: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 118: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 119: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 120: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 121: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 122: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 123: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 124: dp3 r0.w, r2.xyzx, r5.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 125: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 126: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 127: mul r2.yzw, r2.yyyy, cb0[21].xxyz
    r2.yzw = ((r2.yyyy)*(source[21].xxyz)).yzw;
    // 128: mad r2.xyz, r2.xxxx, cb0[20].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[20].xyzx)+(r2.yzwy)).xyz;
    // 129: mul r2.xyz, r2.xyzx, cb0[22].wwww
    r2.xyz = ((r2.xyzx)*(source[22].wwww)).xyz;
    // 130: mad r1.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 131: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 132: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 133: mad o0.xyz, r0.xyzx, cb0[22].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)+(r1.xyzx)).xyz;
    // 134: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 135: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 136: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 137: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 138: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 139: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 140: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 141: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 142: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 143: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 144: dp3 r0.z, r0.xyzx, r5.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 145: dp3 r0.x, r1.xyzx, r5.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 146: mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 147: dp3 r0.y, r1.xyzx, r5.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 148: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 149: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 150: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 151: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 152: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 153: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 154: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 155: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 156: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 157: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 158: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 159: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 160: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 161: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 162: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 163: ret
    return output;
}

// source.character.monster-7373ec8df226.v1 / source program de8d94433e56da409e5a815c735ab968
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase82(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll] for(uint target=0u;target<6u;++target) output.targets[target]=0.f;
    output.discarded=false;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[8].w=(g_SourceCharacterTime.xxxx).x;
    source[9].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[9].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[10].xxxx
    r0.xy = ((v4.xyxx)*(source[10].xxxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[10].y
    r0.x = ((r0.xxxx)+(-(source[10].yyyy))).x;
    // 4: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 6: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 7: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 8: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 9: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 10: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 12: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 13: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 14: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 15: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 19: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r2.xyz, r0.wwww, r0.xyzx
    r2.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 22: div r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)/(r1.wwww)).xyz;
    // 23: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 24: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 25: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 26: dp3 r0.x, r0.xyzx, r3.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 27: add r0.y, -|r3.z|, l(1.000000)
    r0.y = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 29: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 30: mad_sat r0.y, r0.x, cb0[7].z, -cb0[7].w
    r0.y = (saturate((r0.xxxx)*(source[7].zzzz)+(-(source[7].wwww)))).y;
    // 31: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 32: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 33: mul r0.z, r0.z, cb0[8].x
    r0.z = ((r0.zzzz)*(source[8].xxxx)).z;
    // 34: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 35: mul r3.xyz, r0.zzzz, cb0[4].xyzx
    r3.xyz = ((r0.zzzz)*(source[4].xyzx)).xyz;
    // 36: movc r0.yzw, r0.yyyy, l(0,0,0,0), r3.xxyz
    r0.yzw = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxyz)).yzw;
    // 37: add r0.yzw, r0.yyzw, -cb0[4].xxyz
    r0.yzw = ((r0.yyzw)+(-(source[4].xxyz))).yzw;
    // 38: mad r0.yzw, cb0[4].wwww, r0.yyzw, cb0[4].xxyz
    r0.yzw = ((source[4].wwww)*(r0.yyzw)+(source[4].xxyz)).yzw;
    // 39: mul r3.xyz, cb0[5].xyzx, cb0[8].zzzz
    r3.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 40: mul r3.xyz, r3.xyzx, cb0[9].yyyy
    r3.xyz = ((r3.xyzx)*(source[9].yyyy)).xyz;
    // 41: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 42: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 43: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 44: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 45: mul r3.xyz, r3.xyzx, cb0[9].zzzz
    r3.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 46: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 47: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 48: add r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)+(r3.xxyz)).yzw;
    // 49: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 50: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 51: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 52: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 53: mul r3.xyz, r1.wwww, cb0[1].xyzx
    r3.xyz = ((r1.wwww)*(source[1].xyzx)).xyz;
    // 54: movc r3.xyz, r0.xxxx, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 55: mad r0.xyz, r0.yzwy, l(0.250000, 0.250000, 0.250000, 0.000000), r3.xyzx
    r0.xyz = ((r0.yzwy)*(float4(0.250000,0.250000,0.250000,0.000000))+(r3.xyzx)).xyz;
    // 56: add r0.xyz, r0.xyzx, cb0[0].xyzx
    r0.xyz = ((r0.xyzx)+(source[0].xyzx)).xyz;
    // 57: mul r3.xyz, cb0[6].xyzx, cb0[9].wwww
    r3.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 58: mul r4.xyz, r1.xyzx, r3.xyzx
    r4.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 59: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 60: mad r1.xyz, -r1.xyzx, r3.xyzx, r0.wwww
    r1.xyz = ((-(r1.xyzx))*(r3.xyzx)+(r0.wwww)).xyz;
    // 61: mad r1.xyz, cb0[7].xxxx, r1.xyzx, r4.xyzx
    r1.xyz = ((source[7].xxxx)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 62: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 63: add r3.xyz, -r1.xyzx, r0.wwww
    r3.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 64: mad r1.xyz, cb0[7].yyyy, r3.xyzx, r1.xyzx
    r1.xyz = ((source[7].yyyy)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 65: mad r3.xyz, cb0[2].wwww, cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[2].wwww)*(source[2].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 66: mad r4.xyz, cb0[3].wwww, cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[3].wwww)*(source[3].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 67: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 68: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 69: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 70: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 71: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 72: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 73: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 74: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 75: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 76: mul r3.yzw, r3.yyyy, cb0[12].xxyz
    r3.yzw = ((r3.yyyy)*(source[12].xxyz)).yzw;
    // 77: mad r3.xyz, r3.xxxx, cb0[11].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[11].xyzx)+(r3.yzwy)).xyz;
    // 78: mul r3.xyz, r3.xyzx, cb0[13].wwww
    r3.xyz = ((r3.xyzx)*(source[13].wwww)).xyz;
    // 79: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 80: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 81: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 82: mad o0.xyz, r1.xyzx, cb0[13].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[13].xyzx)+(r0.xyzx)).xyz;
    // 83: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 84: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 85: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 86: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 87: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 88: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 91: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 92: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 93: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 94: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 95: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 96: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 97: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 98: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 99: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 100: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 101: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 102: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 103: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 104: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 105: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 106: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 107: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 108: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 109: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 110: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 111: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 112: ret
    return output;
}

// source.character.monster-a2e0ec089348.v1 / source program dbaf02cc9bf6ac499eebfb2b73f16544
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase83(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll] for(uint target=0u;target<6u;++target) output.targets[target]=0.f;
    output.discarded=false;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[17].w=(g_SourceCharacterTime.xxxx).x;
    source[18].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[18].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 4: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 5: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 6: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 7: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 9: sqrt r1.x, r0.w
    r1.x = (sqrt(r0.wwww)).x;
    // 10: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 11: mul r1.yzw, r0.wwww, r0.xxyz
    r1.yzw = ((r0.wwww)*(r0.xxyz)).yzw;
    // 12: div r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)/(r1.xxxx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r2.xyz, r0.wwww, v5.xyzx
    r2.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: dp3 r0.x, r0.xyzx, r2.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 17: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 18: add r0.y, -|r2.z|, l(1.000000)
    r0.y = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 19: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 20: mad_sat r0.y, r0.x, cb0[16].z, -cb0[16].w
    r0.y = (saturate((r0.xxxx)*(source[16].zzzz)+(-(source[16].wwww)))).y;
    // 21: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 22: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 23: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 24: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 25: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 26: mad r3.xyz, r0.yyyy, cb0[7].xyzx, -cb0[7].xyzx
    r3.xyz = ((r0.yyyy)*(source[7].xyzx)+(-(source[7].xyzx))).xyz;
    // 27: mad r0.yzw, r0.yyyy, cb0[6].xxyz, -cb0[6].xxyz
    r0.yzw = ((r0.yyyy)*(source[6].xxyz)+(-(source[6].xxyz))).yzw;
    // 28: mad r0.yzw, cb0[6].wwww, r0.yyzw, cb0[6].xxyz
    r0.yzw = ((source[6].wwww)*(r0.yyzw)+(source[6].xxyz)).yzw;
    // 29: mad r3.xyz, cb0[7].wwww, r3.xyzx, cb0[7].xyzx
    r3.xyz = ((source[7].wwww)*(r3.xyzx)+(source[7].xyzx)).xyz;
    // 30: mul r4.xyz, cb0[8].xyzx, cb0[17].zzzz
    r4.xyz = ((source[8].xyzx)*(source[17].zzzz)).xyz;
    // 31: mul r4.xyz, r4.xyzx, cb0[18].yyyy
    r4.xyz = ((r4.xyzx)*(source[18].yyyy)).xyz;
    // 32: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 33: mad r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), -r3.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(-(r3.xyzx))).xyz;
    // 34: mad r3.xyz, cb0[17].yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((source[17].yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 35: add r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)+(r3.xxyz)).yzw;
    // 36: log r1.x, |r0.x|
    r1.x = (log2(abs(r0.xxxx))).x;
    // 37: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 38: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 39: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 40: mul r3.xyz, cb0[3].xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = ((source[3].xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 41: mul r3.xyz, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 42: movc r3.xyz, r0.xxxx, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 43: mad r0.xyz, r0.yzwy, l(0.250000, 0.250000, 0.250000, 0.000000), r3.xyzx
    r0.xyz = ((r0.yzwy)*(float4(0.250000,0.250000,0.250000,0.000000))+(r3.xyzx)).xyz;
    // 44: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 45: dp3 r0.w, r1.yzwy, r2.xyzx
    r0.w = (dot((r1.yzwy).xyz,(r2.xyzx).xyz).xxxx).w;
    // 46: mul r2.zw, r0.wwww, r1.yyyz
    r2.zw = ((r0.wwww)*(r1.yyyz)).zw;
    // 47: mad r2.xy, r2.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), -r2.xyxx
    r2.xy = ((r2.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r2.xyxx))).xy;
    // 48: add r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 49: mul r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 52: mul r4.xyz, cb0[9].xyzx, cb0[18].zzzz
    r4.xyz = ((source[9].xyzx)*(source[18].zzzz)).xyz;
    // 53: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 54: mad r2.xyz, r2.xyzx, cb0[10].xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)*(source[10].xyzx)+(-(r3.xyzx))).xyz;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r0.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 56: add r0.w, r0.w, l(-0.650000)
    r0.w = ((r0.wwww)+(float4(-0.650000,-0.650000,-0.650000,-0.650000))).w;
    // 57: mul_sat r0.w, r0.w, l(2.857142)
    r0.w = (saturate((r0.wwww)*(float4(2.857142,2.857142,2.857142,2.857142)))).w;
    // 58: mul r0.w, r0.w, cb0[18].w
    r0.w = ((r0.wwww)*(source[18].wwww)).w;
    // 59: mad r2.xyz, r0.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 60: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 62: mad r2.xyz, cb0[16].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 63: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 64: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 65: mad r2.xyz, cb0[16].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 66: mad r3.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 67: mad r4.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 69: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 70: add r0.w, -cb0[11].w, l(1.000000)
    r0.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul r0.w, r0.w, cb0[17].w
    r0.w = ((r0.wwww)*(source[17].wwww)).w;
    // 72: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 73: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 74: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: mul r1.x, cb0[11].z, l(1.500000)
    r1.x = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 76: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 77: mad r0.w, r0.w, l(0.500000), cb0[11].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 78: frc r1.x, v4.x
    r1.x = (frac(v4.xxxx)).x;
    // 79: mul r3.x, r1.x, l(0.125000)
    r3.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 80: mul r4.y, cb0[11].y, cb0[12].y
    r4.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 81: mov r3.y, v4.y
    r3.y = (v4.yyyy).y;
    // 82: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 83: add r3.xy, r3.xyxx, r4.xyxx
    r3.xy = ((r3.xyxx)+(r4.xyxx)).xy;
    // 84: frc r1.x, cb0[11].x
    r1.x = (frac(source[11].xxxx)).x;
    // 85: add r2.w, -r1.x, cb0[11].x
    r2.w = ((-(r1.xxxx))+(source[11].xxxx)).w;
    // 86: mul r4.z, r2.w, l(0.125000)
    r4.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 87: add r3.xy, r3.xyxx, r4.zwzz
    r3.xy = ((r3.xyxx)+(r4.zwzz)).xy;
    // 88: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceMapMonsterStateSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 89: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 90: mul r0.w, r1.x, r3.w
    r0.w = ((r1.xxxx)*(r3.wwww)).w;
    // 91: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 92: mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 93: add r3.xyzw, v7.yzxy, cb0[0].yzxy
    r3.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 94: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 95: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 96: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 97: mad r3.xy, cb0[13].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[13].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 98: mul r0.w, cb0[13].y, cb0[17].w
    r0.w = ((source[13].yyyy)*(source[17].wwww)).w;
    // 99: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 100: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 101: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 102: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 104: mul r1.x, cb0[13].x, l(0.001000)
    r1.x = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 105: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 106: mad r3.xy, r1.xxxx, r3.xyxx, r4.xyxx
    r3.xy = ((r1.xxxx)*(r3.xyxx)+(r4.xyxx)).xy;
    // 107: dp2 r1.x, cb0[14].xyxx, r3.xyxx
    r1.x = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 108: dp2 r3.y, cb0[15].xyxx, r3.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 109: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 110: mul r3.x, r1.x, l(0.125000)
    r3.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 111: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceMapMonsterStateSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 112: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r2.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r2.xyzx))).xyz;
    // 113: mul r1.x, r3.w, l(0.900000)
    r1.x = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 114: mad r3.xyz, r1.xxxx, r3.xyzx, r2.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 115: mul_sat r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = (saturate((r0.wwww)*(r3.xyzx))).xyz;
    // 116: mad r4.xyz, cb0[13].zzzz, r3.xyzx, -r2.xyzx
    r4.xyz = ((source[13].zzzz)*(r3.xyzx)+(-(r2.xyzx))).xyz;
    // 117: mul r3.xyz, r3.xyzx, cb0[13].zzzz
    r3.xyz = ((r3.xyzx)*(source[13].zzzz)).xyz;
    // 118: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 120: mad r2.xyz, r0.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 121: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 122: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 123: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 124: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 125: dp3 r0.w, r3.xyzx, r1.yzwy
    r0.w = (dot((r3.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 126: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 127: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 128: mul r3.yzw, r3.yyyy, cb0[20].xxyz
    r3.yzw = ((r3.yyyy)*(source[20].xxyz)).yzw;
    // 129: mad r3.xyz, r3.xxxx, cb0[19].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[19].xyzx)+(r3.yzwy)).xyz;
    // 130: mul r3.xyz, r3.xyzx, cb0[21].wwww
    r3.xyz = ((r3.xyzx)*(source[21].wwww)).xyz;
    // 131: mad r0.xyz, r3.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 132: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 133: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 134: mad o0.xyz, r2.xyzx, cb0[21].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[21].xyzx)+(r0.xyzx)).xyz;
    // 135: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 136: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 137: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 138: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 139: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 140: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 141: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 142: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 143: mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // 144: mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 145: dp3 r0.z, r0.xyzx, r1.yzwy
    r0.z = (dot((r0.xyzx).xyz,(r1.yzwy).xyz).xxxx).z;
    // 146: dp3 r0.x, r2.xyzx, r1.yzwy
    r0.x = (dot((r2.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // 147: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 148: dp3 r0.y, r2.xyzx, r1.yzwy
    r0.y = (dot((r2.xyzx).xyz,(r1.yzwy).xyz).xxxx).y;
    // 149: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 150: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 151: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 152: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 153: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 154: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 155: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 156: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 157: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 158: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 159: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 160: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 161: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 162: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 163: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 164: ret
    return output;
}



// source.character.monster-862fa1000fe2.v1 / source program 7aa9cbdcb2d02c45a2433d199e53bb6e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked80(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.0;
    [unroll] for (uint packed=0u;packed<32u;++packed) source[packed]=g_SourceCharacterBaseConstants[packed+32u]; source[63]=g_SourceCharacterBaseConstants[63];
    source[25]=1.0; source[26]=1.0;
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[18].x=(g_SourceCharacterTime.xxxx).x;
    source[18].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[18].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[19].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[19].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[19].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[20].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[20].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[20].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[21].x=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(0,0,0,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
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
    // 7: mul r1.xyz, cb0[10].xyzx, cb0[20].xxxx
    r1.xyz = ((source[10].xyzx)*(source[20].xxxx)).xyz;
    // 8: mul r2.xyz, r0.yzwy, r1.xyzx
    r2.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 9: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 10: mad r0.xyz, -r0.yzwy, r1.xyzx, r0.xxxx
    r0.xyz = ((-(r0.yzwy))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 11: mad r0.xyz, cb0[16].yyyy, r0.xyzx, r2.xyzx
    r0.xyz = ((source[16].yyyy)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 12: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 13: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 14: mad r0.xyz, cb0[16].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[16].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 15: mad r1.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 16: mad r2.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 17: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 18: add r0.w, -cb0[11].w, l(1.000000)
    r0.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 19: mul r0.w, r0.w, cb0[18].x
    r0.w = ((r0.wwww)*(source[18].xxxx)).w;
    // 20: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 21: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 22: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: mul r1.w, cb0[11].z, l(1.500000)
    r1.w = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 24: mul r0.xyzw, r0.xyzw, r1.xyzw
    r0.xyzw = ((r0.xyzw)*(r1.xyzw)).xyzw;
    // 25: mad r0.w, r0.w, l(0.500000), cb0[11].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 26: frc r1.w, v4.x
    r1.w = (frac(v4.xxxx)).w;
    // 27: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 28: mul r3.y, cb0[11].y, cb0[12].y
    r3.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 29: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 30: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 31: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 32: frc r1.w, cb0[11].x
    r1.w = (frac(source[11].xxxx)).w;
    // 33: add r2.z, -r1.w, cb0[11].x
    r2.z = ((-(r1.wwww))+(source[11].xxxx)).z;
    // 34: mul r3.z, r2.z, l(0.125000)
    r3.z = ((r2.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 35: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 38: mul r0.w, r1.w, r2.w
    r0.w = ((r1.wwww)*(r2.wwww)).w;
    // 39: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 40: mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 41: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 42: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 43: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 44: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 45: mad r2.xy, cb0[13].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[13].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 46: mul r0.w, cb0[13].y, cb0[18].x
    r0.w = ((source[13].yyyy)*(source[18].xxxx)).w;
    // 47: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 48: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 49: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 50: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 52: mul r1.w, cb0[13].x, l(0.001000)
    r1.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 53: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 54: mad r2.xy, r1.wwww, r2.xyxx, r3.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r3.xyxx)).xy;
    // 55: dp2 r1.w, cb0[14].xyxx, r2.xyxx
    r1.w = (dot((source[14].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 56: dp2 r2.y, cb0[15].xyxx, r2.xyxx
    r2.y = (dot((source[15].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 57: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 58: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 60: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 61: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 62: mad r2.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 63: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 64: mad r3.xyz, cb0[13].zzzz, r2.xyzx, -r0.xyzx
    r3.xyz = ((source[13].zzzz)*(r2.xyzx)+(-(r0.xyzx))).xyz;
    // 65: mul r2.xyz, r2.xyzx, cb0[13].zzzz
    r2.xyz = ((r2.xyzx)*(source[13].zzzz)).xyz;
    // 66: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 67: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 68: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 69: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 71: mul r3.xyz, cb0[4].xyzx, cb0[16].xxxx
    r3.xyz = ((source[4].xyzx)*(source[16].xxxx)).xyz;
    // 72: mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 73: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: mad r2.xyz, -r2.xyzx, r3.xyzx, r0.wwww
    r2.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r0.wwww)).xyz;
    // 75: mad r2.xyz, cb0[16].yyyy, r2.xyzx, r4.xyzx
    r2.xyz = ((source[16].yyyy)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 76: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 77: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 78: mad r2.xyz, cb0[16].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[16].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r3.xyz, cb0[9].xyzx, cb0[17].wwww
    r3.xyz = ((source[9].xyzx)*(source[17].wwww)).xyz;
    // 80: mul r3.xyz, r3.xyzx, cb0[19].zzzz
    r3.xyz = ((r3.xyzx)*(source[19].zzzz)).xyz;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 82: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 83: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 84: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 85: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 86: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 87: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 88: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 89: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 90: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 91: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 92: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 93: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 94: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 95: mul r6.xyz, r0.wwww, v5.xyzx
    r6.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 96: dp3 r0.w, r4.xyzx, r6.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 97: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: add r1.w, -|r6.z|, l(1.000000)
    r1.w = ((-(abs(r6.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 100: mul r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 101: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 102: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 103: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 104: mul r3.xyz, r3.xyzx, cb0[19].wwww
    r3.xyz = ((r3.xyzx)*(source[19].wwww)).xyz;
    // 105: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 106: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 107: mad_sat r1.w, r0.w, cb0[16].w, -cb0[17].x
    r1.w = (saturate((r0.wwww)*(source[16].wwww)+(-(source[17].xxxx)))).w;
    // 108: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 109: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 110: mul r2.w, r2.w, cb0[17].y
    r2.w = ((r2.wwww)*(source[17].yyyy)).w;
    // 111: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 112: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 113: mad r4.xyz, r1.wwww, cb0[8].xyzx, -cb0[8].xyzx
    r4.xyz = ((r1.wwww)*(source[8].xyzx)+(-(source[8].xyzx))).xyz;
    // 114: mad r7.xyz, r1.wwww, cb0[7].xyzx, -cb0[7].xyzx
    r7.xyz = ((r1.wwww)*(source[7].xyzx)+(-(source[7].xyzx))).xyz;
    // 115: mad r7.xyz, cb0[7].wwww, r7.xyzx, cb0[7].xyzx
    r7.xyz = ((source[7].wwww)*(r7.xyzx)+(source[7].xyzx)).xyz;
    // 116: mad r4.xyz, cb0[8].wwww, r4.xyzx, cb0[8].xyzx
    r4.xyz = ((source[8].wwww)*(r4.xyzx)+(source[8].xyzx)).xyz;
    // 117: add r3.xyz, r3.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 118: mad r3.xyz, cb0[17].zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((source[17].zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 119: add r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)+(r7.xyzx)).xyz;
    // 120: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 121: mad r1.xyz, r2.xyzx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 122: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 123: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 124: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 125: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 126: mul r2.xyz, r1.wwww, cb0[3].xyzx
    r2.xyz = ((r1.wwww)*(source[3].xyzx)).xyz;
    // 127: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 128: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 129: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 130: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 131: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 132: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 133: dp3 r0.w, r2.xyzx, r5.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 134: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 135: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 136: mul r2.yzw, r2.yyyy, cb0[23].xxyz
    r2.yzw = ((r2.yyyy)*(source[23].xxyz)).yzw;
    // 137: mad r2.xyz, r2.xxxx, cb0[22].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[22].xyzx)+(r2.yzwy)).xyz;
    // 138: mul r2.xyz, r2.xyzx, cb0[24].wwww
    r2.xyz = ((r2.xyzx)*(source[24].wwww)).xyz;
    // 139: mul r3.xyz, r0.xyzx, r2.xyzx
    r3.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 140: dp2_sat r4.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 141: dp3_sat r4.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 142: dp3_sat r4.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 143: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 144: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t6.xyzw, s5
    r7.xyz = ((float4(input.bakedCoefficients,1.0)).xyzw).xyz;
    // 145: mul r7.xyz, r7.xyzx, cb0[26].xyzx
    r7.xyz = ((r7.xyzx)*(source[26].xyzx)).xyz;
    // 146: dp3 r0.w, r7.xyzx, r4.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 147: sample_indexable(texture2d)(float,float,float,float) r4.xyz, v3.zwzz, t5.xyzw, s5
    r4.xyz = ((float4(input.bakedAverage,1.0)).xyzw).xyz;
    // 148: mul r4.xyz, r4.xyzx, cb0[25].xyzx
    r4.xyz = ((r4.xyzx)*(source[25].xyzx)).xyz;
    // 149: mul r8.xyz, r0.wwww, r4.xyzx
    r8.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 150: mad r2.xyz, r4.xyzx, r0.wwww, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 151: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 152: div r2.xyz, r8.xyzx, r2.xyzx
    r2.xyz = ((r8.xyzx)/(r2.xyzx)).xyz;
    // 153: mad r3.xyz, r0.xyzx, r8.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 154: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: dp3 r1.w, r5.xyzx, r6.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 156: mul r2.xyz, r1.wwww, r5.xyzx
    r2.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 157: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r6.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r6.xyzx))).xyz;
    // 158: dp2_sat r6.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 159: dp3_sat r6.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 160: dp3_sat r6.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 161: log r2.xyz, r6.xyzx
    r2.xyz = (log2(r6.xyzx)).xyz;
    // 162: add r1.w, cb0[21].z, l(-1.000000)
    r1.w = ((source[21].zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 163: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 164: mad r1.w, r6.w, r1.w, l(2.000000)
    r1.w = ((r6.wwww)*(r1.wwww)+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 165: mul r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)*(r1.wwww)).xyz;
    // 166: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 167: dp3 r1.w, r7.xyzx, r2.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 168: dp3 r2.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 169: add r2.xyz, -r6.xyzx, r2.xxxx
    r2.xyz = ((-(r6.xyzx))+(r2.xxxx)).xyz;
    // 170: mad r2.xyz, cb0[16].yyyy, r2.xyzx, r6.xyzx
    r2.xyz = ((source[16].yyyy)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 171: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: add r6.xyz, -r2.xyzx, r2.wwww
    r6.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 173: mad r2.xyz, cb0[16].zzzz, r6.xyzx, r2.xyzx
    r2.xyz = ((source[16].zzzz)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 174: mul r2.xyz, r2.xyzx, cb0[21].yyyy
    r2.xyz = ((r2.xyzx)*(source[21].yyyy)).xyz;
    // 175: mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 176: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 177: mad r3.xyz, r2.xyzx, r1.wwww, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 178: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 179: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 180: add r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)+(r3.xyzx)).xyz;
    // 181: mad o0.xyz, r0.xyzx, cb0[24].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[24].xyzx)+(r1.xyzx)).xyz;
    // 182: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 183: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 184: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 185: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 186: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 187: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 188: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 189: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 190: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 191: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 192: dp3 r0.z, r0.xyzx, r5.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 193: dp3 r0.x, r1.xyzx, r5.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 194: mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 195: dp3 r0.y, r1.xyzx, r5.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 196: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 197: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 198: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 199: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 200: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 201: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 202: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 203: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 204: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 205: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 206: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 207: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 208: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 209: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 210: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 211: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 212: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 213: ret
    return output;
}

// source.character.monster-3c300c108ac5.v1 / source program 5efca7973955cd44b6c4240a26112891
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked81(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.0;
    [unroll] for (uint packed=0u;packed<32u;++packed) source[packed]=g_SourceCharacterBaseConstants[packed+32u]; source[63]=g_SourceCharacterBaseConstants[63];
    source[24]=1.0; source[25]=1.0;
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[18].x=(g_SourceCharacterTime.xxxx).x;
    source[18].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[18].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[19].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[19].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[19].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(0,0,0,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: add r0.x, -cb0[11].w, l(1.000000)
    r0.x = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: mul r0.x, r0.x, cb0[18].x
    r0.x = ((r0.xxxx)*(source[18].xxxx)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 4: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 5: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 6: mul r0.y, cb0[11].z, l(1.500000)
    r0.y = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 7: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 8: mad r0.x, r0.x, l(0.500000), cb0[11].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).x;
    // 9: frc r0.y, v4.x
    r0.y = (frac(v4.xxxx)).y;
    // 10: mul r1.x, r0.y, l(0.125000)
    r1.x = ((r0.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 11: mul r2.y, cb0[11].y, cb0[12].y
    r2.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 12: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 13: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 14: add r0.yz, r1.xxyx, r2.xxyx
    r0.yz = ((r1.xxyx)+(r2.xxyx)).yz;
    // 15: frc r0.w, cb0[11].x
    r0.w = (frac(source[11].xxxx)).w;
    // 16: add r1.x, -r0.w, cb0[11].x
    r1.x = ((-(r0.wwww))+(source[11].xxxx)).x;
    // 17: mul r2.z, r1.x, l(0.125000)
    r2.z = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 18: add r0.yz, r0.yyzy, r2.zzwz
    r0.yz = ((r0.yyzy)+(r2.zzwz)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.yzyy, t2.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 20: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 21: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 23: mul r2.xyz, cb0[10].xyzx, cb0[19].wwww
    r2.xyz = ((source[10].xyzx)*(source[19].wwww)).xyz;
    // 24: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 25: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: mad r1.xyz, -r1.xyzx, r2.xyzx, r1.wwww
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r1.wwww)).xyz;
    // 27: mad r1.xyz, cb0[16].yyyy, r1.xyzx, r3.xyzx
    r1.xyz = ((source[16].yyyy)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 28: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 29: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 30: mad r1.xyz, cb0[16].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[16].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 31: mad r2.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 32: mad r3.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 33: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 34: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 35: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 36: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 37: add r1.xyzw, v7.yzxy, cb0[0].yzxy
    r1.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 38: add r1.xyzw, r1.xyzw, -cb0[1].yzxy
    r1.xyzw = ((r1.xyzw)+(-(source[1].yzxy))).xyzw;
    // 39: add r1.xy, -r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = ((-(r1.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 40: add r1.xy, -r1.zwzz, r1.xyxx
    r1.xy = ((-(r1.zwzz))+(r1.xyxx)).xy;
    // 41: mad r1.xy, cb0[13].wwww, r1.xyxx, r1.zwzz
    r1.xy = ((source[13].wwww)*(r1.xyxx)+(r1.zwzz)).xy;
    // 42: mul r0.w, cb0[13].y, cb0[18].x
    r0.w = ((source[13].yyyy)*(source[18].xxxx)).w;
    // 43: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 44: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 45: mul r3.y, r0.w, l(0.020000)
    r3.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 46: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 48: mul r1.z, cb0[13].x, l(0.001000)
    r1.z = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 49: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 50: mad r1.xy, r1.zzzz, r1.xyxx, r3.xyxx
    r1.xy = ((r1.zzzz)*(r1.xyxx)+(r3.xyxx)).xy;
    // 51: dp2 r1.z, cb0[14].xyxx, r1.xyxx
    r1.z = (dot((source[14].xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 52: dp2 r1.y, cb0[15].xyxx, r1.xyxx
    r1.y = (dot((source[15].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 53: frc r1.z, r1.z
    r1.z = (frac(r1.zzzz)).z;
    // 54: mul r1.x, r1.z, l(0.125000)
    r1.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceMapMonsterStateSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 56: mad r1.xyz, r1.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 57: mul r1.w, r1.w, l(0.900000)
    r1.w = ((r1.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 58: mad r1.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 59: mul_sat r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (saturate((r0.wwww)*(r1.xyzx))).xyz;
    // 60: mad r3.xyz, cb0[13].zzzz, r1.xyzx, -r0.xyzx
    r3.xyz = ((source[13].zzzz)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 61: mul r1.xyz, r1.xyzx, cb0[13].zzzz
    r1.xyz = ((r1.xyzx)*(source[13].zzzz)).xyz;
    // 62: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 63: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 64: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 65: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t4.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 67: mul r3.xyz, cb0[4].xyzx, cb0[16].xxxx
    r3.xyz = ((source[4].xyzx)*(source[16].xxxx)).xyz;
    // 68: mul r4.xyz, r1.xyzx, r3.xyzx
    r4.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 69: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: mad r1.xyz, -r1.xyzx, r3.xyzx, r0.wwww
    r1.xyz = ((-(r1.xyzx))*(r3.xyzx)+(r0.wwww)).xyz;
    // 71: mad r1.xyz, cb0[16].yyyy, r1.xyzx, r4.xyzx
    r1.xyz = ((source[16].yyyy)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 72: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r3.xyz, -r1.xyzx, r0.wwww
    r3.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 74: mad r1.xyz, cb0[16].zzzz, r3.xyzx, r1.xyzx
    r1.xyz = ((source[16].zzzz)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 76: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 77: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 78: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 80: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 81: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 82: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 83: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 84: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 85: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 86: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 87: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 88: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 89: mul r5.xyz, r0.wwww, v5.xyzx
    r5.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 90: dp3 r0.w, r3.xyzx, r5.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 91: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: add r1.w, -|r5.z|, l(1.000000)
    r1.w = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 94: mad_sat r1.w, r0.w, cb0[16].w, -cb0[17].x
    r1.w = (saturate((r0.wwww)*(source[16].wwww)+(-(source[17].xxxx)))).w;
    // 95: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 96: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 97: mul r2.w, r2.w, cb0[17].y
    r2.w = ((r2.wwww)*(source[17].yyyy)).w;
    // 98: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 99: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 100: mad r3.xyz, r1.wwww, cb0[8].xyzx, -cb0[8].xyzx
    r3.xyz = ((r1.wwww)*(source[8].xyzx)+(-(source[8].xyzx))).xyz;
    // 101: mad r6.xyz, r1.wwww, cb0[7].xyzx, -cb0[7].xyzx
    r6.xyz = ((r1.wwww)*(source[7].xyzx)+(-(source[7].xyzx))).xyz;
    // 102: mad r6.xyz, cb0[7].wwww, r6.xyzx, cb0[7].xyzx
    r6.xyz = ((source[7].wwww)*(r6.xyzx)+(source[7].xyzx)).xyz;
    // 103: mad r3.xyz, cb0[8].wwww, r3.xyzx, cb0[8].xyzx
    r3.xyz = ((source[8].wwww)*(r3.xyzx)+(source[8].xyzx)).xyz;
    // 104: mul r7.xyz, cb0[9].xyzx, cb0[17].wwww
    r7.xyz = ((source[9].xyzx)*(source[17].wwww)).xyz;
    // 105: mul r7.xyz, r7.xyzx, cb0[19].zzzz
    r7.xyz = ((r7.xyzx)*(source[19].zzzz)).xyz;
    // 106: mul r7.xyz, r0.wwww, r7.xyzx
    r7.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 107: mad r7.xyz, r7.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), -r3.xyzx
    r7.xyz = ((r7.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(-(r3.xyzx))).xyz;
    // 108: mad r3.xyz, cb0[17].zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((source[17].zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 109: add r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)+(r6.xyzx)).xyz;
    // 110: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 111: mad r1.xyz, r1.xyzx, r2.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 112: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 113: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 114: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 115: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 116: mul r2.xyz, cb0[3].xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = ((source[3].xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 117: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 118: movc r2.xyz, r0.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 119: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 120: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 121: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 122: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 123: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 124: dp3 r0.w, r2.xyzx, r4.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 125: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 126: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 127: mul r2.yzw, r2.yyyy, cb0[22].xxyz
    r2.yzw = ((r2.yyyy)*(source[22].xxyz)).yzw;
    // 128: mad r2.xyz, r2.xxxx, cb0[21].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[21].xyzx)+(r2.yzwy)).xyz;
    // 129: mul r2.xyz, r2.xyzx, cb0[23].wwww
    r2.xyz = ((r2.xyzx)*(source[23].wwww)).xyz;
    // 130: mul r3.xyz, r0.xyzx, r2.xyzx
    r3.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 131: dp2_sat r6.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 132: dp3_sat r6.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 133: dp3_sat r6.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 134: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 135: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t6.xyzw, s5
    r7.xyz = ((float4(input.bakedCoefficients,1.0)).xyzw).xyz;
    // 136: mul r7.xyz, r7.xyzx, cb0[25].xyzx
    r7.xyz = ((r7.xyzx)*(source[25].xyzx)).xyz;
    // 137: dp3 r0.w, r7.xyzx, r6.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 138: sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t5.xyzw, s5
    r6.xyz = ((float4(input.bakedAverage,1.0)).xyzw).xyz;
    // 139: mul r6.xyz, r6.xyzx, cb0[24].xyzx
    r6.xyz = ((r6.xyzx)*(source[24].xyzx)).xyz;
    // 140: mul r8.xyz, r0.wwww, r6.xyzx
    r8.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 141: mad r2.xyz, r6.xyzx, r0.wwww, r2.xyzx
    r2.xyz = ((r6.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 142: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 143: div r2.xyz, r8.xyzx, r2.xyzx
    r2.xyz = ((r8.xyzx)/(r2.xyzx)).xyz;
    // 144: mad r3.xyz, r0.xyzx, r8.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 145: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 146: dp3 r1.w, r4.xyzx, r5.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 147: mul r2.xyz, r1.wwww, r4.xyzx
    r2.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 148: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xyzx))).xyz;
    // 149: dp2_sat r5.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 150: dp3_sat r5.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 151: dp3_sat r5.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 152: log r2.xyz, r5.xyzx
    r2.xyz = (log2(r5.xyzx)).xyz;
    // 153: add r1.w, cb0[20].y, l(-1.000000)
    r1.w = ((source[20].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 154: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 155: mad r1.w, r5.w, r1.w, l(2.000000)
    r1.w = ((r5.wwww)*(r1.wwww)+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 156: mul r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)*(r1.wwww)).xyz;
    // 157: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 158: dp3 r1.w, r7.xyzx, r2.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 159: dp3 r2.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 160: add r2.xyz, -r5.xyzx, r2.xxxx
    r2.xyz = ((-(r5.xyzx))+(r2.xxxx)).xyz;
    // 161: mad r2.xyz, cb0[16].yyyy, r2.xyzx, r5.xyzx
    r2.xyz = ((source[16].yyyy)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 162: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r5.xyz, -r2.xyzx, r2.wwww
    r5.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 164: mad r2.xyz, cb0[16].zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((source[16].zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 165: mul r2.xyz, r2.xyzx, cb0[20].xxxx
    r2.xyz = ((r2.xyzx)*(source[20].xxxx)).xyz;
    // 166: mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 167: mul r2.xyz, r6.xyzx, r2.xyzx
    r2.xyz = ((r6.xyzx)*(r2.xyzx)).xyz;
    // 168: mad r3.xyz, r2.xyzx, r1.wwww, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 169: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 170: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 171: add r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)+(r3.xyzx)).xyz;
    // 172: mad o0.xyz, r0.xyzx, cb0[23].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)+(r1.xyzx)).xyz;
    // 173: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 174: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 175: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 176: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 177: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 178: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 179: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 180: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 181: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 182: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 183: dp3 r0.z, r0.xyzx, r4.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 184: dp3 r0.x, r1.xyzx, r4.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 185: mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 186: dp3 r0.y, r1.xyzx, r4.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 187: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 188: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 189: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 190: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 191: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 192: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 193: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 194: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 195: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 196: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 197: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 198: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 199: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 200: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 201: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 202: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 203: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 204: ret
    return output;
}

// source.character.monster-7373ec8df226.v1 / source program 59c140c20873ce429661f64b9a7a53a3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked82(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.0;
    [unroll] for (uint packed=0u;packed<32u;++packed) source[packed]=g_SourceCharacterBaseConstants[packed+32u]; source[63]=g_SourceCharacterBaseConstants[63];
    source[15]=1.0; source[16]=1.0;
    source[8].w=(g_SourceCharacterTime.xxxx).x;
    source[9].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[9].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(0,0,0,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[11].xxxx
    r0.xy = ((v4.xyxx)*(source[11].xxxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[11].y
    r0.x = ((r0.xxxx)+(-(source[11].yyyy))).x;
    // 4: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 6: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 7: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 8: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 9: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 10: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 12: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 13: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 14: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 15: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 19: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r2.xyz, r0.wwww, r0.xyzx
    r2.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 22: div r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)/(r1.wwww)).xyz;
    // 23: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 24: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 25: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 26: dp3 r0.x, r0.xyzx, r3.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 27: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 28: add r0.y, -|r3.z|, l(1.000000)
    r0.y = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 29: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 30: mad_sat r0.y, r0.x, cb0[7].z, -cb0[7].w
    r0.y = (saturate((r0.xxxx)*(source[7].zzzz)+(-(source[7].wwww)))).y;
    // 31: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 32: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 33: mul r0.z, r0.z, cb0[8].x
    r0.z = ((r0.zzzz)*(source[8].xxxx)).z;
    // 34: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 35: mul r4.xyz, r0.zzzz, cb0[4].xyzx
    r4.xyz = ((r0.zzzz)*(source[4].xyzx)).xyz;
    // 36: movc r0.yzw, r0.yyyy, l(0,0,0,0), r4.xxyz
    r0.yzw = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxyz)).yzw;
    // 37: add r0.yzw, r0.yyzw, -cb0[4].xxyz
    r0.yzw = ((r0.yyzw)+(-(source[4].xxyz))).yzw;
    // 38: mad r0.yzw, cb0[4].wwww, r0.yyzw, cb0[4].xxyz
    r0.yzw = ((source[4].wwww)*(r0.yyzw)+(source[4].xxyz)).yzw;
    // 39: mul r4.xyz, cb0[5].xyzx, cb0[8].zzzz
    r4.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 40: mul r4.xyz, r4.xyzx, cb0[9].yyyy
    r4.xyz = ((r4.xyzx)*(source[9].yyyy)).xyz;
    // 41: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 42: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 43: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 44: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 45: mul r4.xyz, r4.xyzx, cb0[9].zzzz
    r4.xyz = ((r4.xyzx)*(source[9].zzzz)).xyz;
    // 46: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 47: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 48: add r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)+(r4.xxyz)).yzw;
    // 49: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 50: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 51: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 52: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 53: mul r4.xyz, r1.wwww, cb0[1].xyzx
    r4.xyz = ((r1.wwww)*(source[1].xyzx)).xyz;
    // 54: movc r4.xyz, r0.xxxx, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 55: mad r0.xyz, r0.yzwy, l(0.250000, 0.250000, 0.250000, 0.000000), r4.xyzx
    r0.xyz = ((r0.yzwy)*(float4(0.250000,0.250000,0.250000,0.000000))+(r4.xyzx)).xyz;
    // 56: add r0.xyz, r0.xyzx, cb0[0].xyzx
    r0.xyz = ((r0.xyzx)+(source[0].xyzx)).xyz;
    // 57: dp3 r0.w, r2.xyzx, r3.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 58: mul r4.xyz, r0.wwww, r2.xyzx
    r4.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 59: mad r3.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 60: dp2_sat r4.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 61: dp3_sat r4.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 62: dp3_sat r4.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 63: log r3.xyz, r4.xyzx
    r3.xyz = (log2(r4.xyzx)).xyz;
    // 64: add r0.w, cb0[10].w, l(-1.000000)
    r0.w = ((source[10].wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 65: add r0.w, r0.w, l(2.000000)
    r0.w = ((r0.wwww)+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 66: mul r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 67: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 68: sample_indexable(texture2d)(float,float,float,float) r4.xyz, v3.zwzz, t4.xyzw, s3
    r4.xyz = ((float4(input.bakedCoefficients,1.0)).xyzw).xyz;
    // 69: mul r4.xyz, r4.xyzx, cb0[16].xyzx
    r4.xyz = ((r4.xyzx)*(source[16].xyzx)).xyz;
    // 70: dp3 r0.w, r4.xyzx, r3.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 71: dp2_sat r3.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 72: dp3_sat r3.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 73: dp3_sat r3.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 74: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 75: dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 76: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t3.xyzw, s3
    r3.xyz = ((float4(input.bakedAverage,1.0)).xyzw).xyz;
    // 77: mul r3.xyz, r3.xyzx, cb0[15].xyzx
    r3.xyz = ((r3.xyzx)*(source[15].xyzx)).xyz;
    // 78: mul r4.xyz, r1.wwww, r3.xyzx
    r4.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 79: mul r5.xyz, cb0[6].xyzx, cb0[9].wwww
    r5.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 80: mul r6.xyz, r1.xyzx, r5.xyzx
    r6.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 81: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 82: mad r1.xyz, -r1.xyzx, r5.xyzx, r2.wwww
    r1.xyz = ((-(r1.xyzx))*(r5.xyzx)+(r2.wwww)).xyz;
    // 83: mad r5.xyz, cb0[7].xxxx, r1.xyzx, r6.xyzx
    r5.xyz = ((source[7].xxxx)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 84: mad r1.xyz, cb0[10].yyyy, r1.xyzx, r6.xyzx
    r1.xyz = ((source[10].yyyy)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 85: mul r1.xyz, r1.xyzx, cb0[10].zzzz
    r1.xyz = ((r1.xyzx)*(source[10].zzzz)).xyz;
    // 86: mad r1.xyz, r1.xyzx, cb2[4].wwww, cb2[4].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 87: mul r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.xyzx)).xyz;
    // 88: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: add r6.xyz, -r5.xyzx, r2.wwww
    r6.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 90: mad r5.xyz, cb0[7].yyyy, r6.xyzx, r5.xyzx
    r5.xyz = ((source[7].yyyy)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 91: mad r6.xyz, cb0[2].wwww, cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[2].wwww)*(source[2].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mad r7.xyz, cb0[3].wwww, cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[3].wwww)*(source[3].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 93: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 94: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 95: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 96: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 97: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 98: mul r6.xyz, r2.wwww, v6.xyzx
    r6.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 99: dp3 r2.w, r6.xyzx, r2.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 100: mad r6.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 101: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 102: mul r6.yzw, r6.yyyy, cb0[13].xxyz
    r6.yzw = ((r6.yyyy)*(source[13].xxyz)).yzw;
    // 103: mad r6.xyz, r6.xxxx, cb0[12].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[12].xyzx)+(r6.yzwy)).xyz;
    // 104: mul r6.xyz, r6.xyzx, cb0[14].wwww
    r6.xyz = ((r6.xyzx)*(source[14].wwww)).xyz;
    // 105: mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 106: mad r3.xyz, r3.xyzx, r1.wwww, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r1.wwww)+(r6.xyzx)).xyz;
    // 107: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 108: div r3.xyz, r4.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)/(r3.xyzx)).xyz;
    // 109: mad r4.xyz, r5.xyzx, r4.xyzx, r7.xyzx
    r4.xyz = ((r5.xyzx)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 110: mad r4.xyz, r1.xyzx, r0.wwww, r4.xyzx
    r4.xyz = ((r1.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // 111: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 112: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 113: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: mul o4.z, r0.w, r4.x
    output.targets[4].z = ((r0.wwww)*(r4.xxxx)).z;
    // 115: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 116: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 117: mad o0.xyz, r5.xyzx, cb0[14].xyzx, r0.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(source[14].xyzx)+(r0.xyzx)).xyz;
    // 118: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 119: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 120: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 121: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 122: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 123: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 124: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 125: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 126: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 127: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 128: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 129: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 130: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 131: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 132: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 133: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 134: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 135: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 136: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 137: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 138: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 139: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 140: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 141: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 142: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 143: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 144: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 145: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 146: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 147: ret
    return output;
}

// source.character.monster-a2e0ec089348.v1 / source program dd70f8da53528746ab79c9638b0691b7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked83(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.0;
    [unroll] for (uint packed=0u;packed<32u;++packed) source[packed]=g_SourceCharacterBaseConstants[packed+32u]; source[63]=g_SourceCharacterBaseConstants[63];
    source[23]=1.0; source[24]=1.0;
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[17].w=(g_SourceCharacterTime.xxxx).x;
    source[18].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[18].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(0,0,0,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 4: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 5: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 6: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 7: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 9: sqrt r1.x, r0.w
    r1.x = (sqrt(r0.wwww)).x;
    // 10: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 11: mul r1.yzw, r0.wwww, r0.xxyz
    r1.yzw = ((r0.wwww)*(r0.xxyz)).yzw;
    // 12: div r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)/(r1.xxxx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r2.xyz, r0.wwww, v5.xyzx
    r2.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: dp3 r0.x, r0.xyzx, r2.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 17: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 18: add r0.y, -|r2.z|, l(1.000000)
    r0.y = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 19: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 20: mad_sat r0.y, r0.x, cb0[16].z, -cb0[16].w
    r0.y = (saturate((r0.xxxx)*(source[16].zzzz)+(-(source[16].wwww)))).y;
    // 21: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 22: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 23: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 24: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 25: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 26: mad r3.xyz, r0.yyyy, cb0[7].xyzx, -cb0[7].xyzx
    r3.xyz = ((r0.yyyy)*(source[7].xyzx)+(-(source[7].xyzx))).xyz;
    // 27: mad r0.yzw, r0.yyyy, cb0[6].xxyz, -cb0[6].xxyz
    r0.yzw = ((r0.yyyy)*(source[6].xxyz)+(-(source[6].xxyz))).yzw;
    // 28: mad r0.yzw, cb0[6].wwww, r0.yyzw, cb0[6].xxyz
    r0.yzw = ((source[6].wwww)*(r0.yyzw)+(source[6].xxyz)).yzw;
    // 29: mad r3.xyz, cb0[7].wwww, r3.xyzx, cb0[7].xyzx
    r3.xyz = ((source[7].wwww)*(r3.xyzx)+(source[7].xyzx)).xyz;
    // 30: mul r4.xyz, cb0[8].xyzx, cb0[17].zzzz
    r4.xyz = ((source[8].xyzx)*(source[17].zzzz)).xyz;
    // 31: mul r4.xyz, r4.xyzx, cb0[18].yyyy
    r4.xyz = ((r4.xyzx)*(source[18].yyyy)).xyz;
    // 32: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 33: mad r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), -r3.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(-(r3.xyzx))).xyz;
    // 34: mad r3.xyz, cb0[17].yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((source[17].yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 35: add r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)+(r3.xxyz)).yzw;
    // 36: log r1.x, |r0.x|
    r1.x = (log2(abs(r0.xxxx))).x;
    // 37: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 38: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 39: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 40: mul r3.xyz, cb0[3].xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = ((source[3].xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 41: mul r3.xyz, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 42: movc r3.xyz, r0.xxxx, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 43: mad r0.xyz, r0.yzwy, l(0.250000, 0.250000, 0.250000, 0.000000), r3.xyzx
    r0.xyz = ((r0.yzwy)*(float4(0.250000,0.250000,0.250000,0.000000))+(r3.xyzx)).xyz;
    // 44: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 45: add r3.xyzw, v7.yzxy, cb0[0].yzxy
    r3.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 46: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 47: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 48: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 49: mad r3.xy, cb0[13].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[13].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 50: mul r0.w, cb0[13].y, cb0[17].w
    r0.w = ((source[13].yyyy)*(source[17].wwww)).w;
    // 51: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 52: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 53: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 54: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 56: mul r1.x, cb0[13].x, l(0.001000)
    r1.x = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 57: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 58: mad r3.xy, r1.xxxx, r3.xyxx, r4.xyxx
    r3.xy = ((r1.xxxx)*(r3.xyxx)+(r4.xyxx)).xy;
    // 59: dp2 r1.x, cb0[14].xyxx, r3.xyxx
    r1.x = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 60: dp2 r3.y, cb0[15].xyxx, r3.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 61: frc r1.x, r1.x
    r1.x = (frac(r1.xxxx)).x;
    // 62: mul r3.x, r1.x, l(0.125000)
    r3.x = ((r1.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceMapMonsterStateSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: mul r1.x, r3.w, l(0.900000)
    r1.x = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 65: frc r2.w, v4.x
    r2.w = (frac(v4.xxxx)).w;
    // 66: mul r4.x, r2.w, l(0.125000)
    r4.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 67: mul r5.y, cb0[11].y, cb0[12].y
    r5.y = ((source[11].yyyy)*(source[12].yyyy)).y;
    // 68: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 69: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 70: add r4.xy, r4.xyxx, r5.xyxx
    r4.xy = ((r4.xyxx)+(r5.xyxx)).xy;
    // 71: frc r2.w, cb0[11].x
    r2.w = (frac(source[11].xxxx)).w;
    // 72: add r3.w, -r2.w, cb0[11].x
    r3.w = ((-(r2.wwww))+(source[11].xxxx)).w;
    // 73: mul r5.z, r3.w, l(0.125000)
    r5.z = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 74: add r4.xy, r4.xyxx, r5.zwzz
    r4.xy = ((r4.xyxx)+(r5.zwzz)).xy;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t4.xyzw, s4, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceMapMonsterStateSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 76: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 77: add r3.w, -cb0[11].w, l(1.000000)
    r3.w = ((-(source[11].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 78: mul r3.w, r3.w, cb0[17].w
    r3.w = ((r3.wwww)*(source[17].wwww)).w;
    // 79: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 80: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 81: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 82: mul r4.w, cb0[11].z, l(1.500000)
    r4.w = ((source[11].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 83: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 84: mad r3.w, r3.w, l(0.500000), cb0[11].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].zzzz)).w;
    // 85: mul r4.xyz, r4.xyzx, r3.wwww
    r4.xyz = ((r4.xyzx)*(r3.wwww)).xyz;
    // 86: mad r5.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 87: mad r6.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 88: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 89: dp3 r3.w, r1.yzwy, r2.xyzx
    r3.w = (dot((r1.yzwy).xyz,(r2.xyzx).xyz).xxxx).w;
    // 90: mul r6.xyz, r1.yzwy, r3.wwww
    r6.xyz = ((r1.yzwy)*(r3.wwww)).xyz;
    // 91: mad r2.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r2.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 92: add r6.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 93: mul r6.xy, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 94: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 95: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 96: mul r8.xyz, cb0[9].xyzx, cb0[18].zzzz
    r8.xyz = ((source[9].xyzx)*(source[18].zzzz)).xyz;
    // 97: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 98: mad r6.xyz, r6.xyzx, cb0[10].xyzx, -r7.xyzx
    r6.xyz = ((r6.xyzx)*(source[10].xyzx)+(-(r7.xyzx))).xyz;
    // 99: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 100: add r3.w, r8.w, l(-0.650000)
    r3.w = ((r8.wwww)+(float4(-0.650000,-0.650000,-0.650000,-0.650000))).w;
    // 101: mul_sat r3.w, r3.w, l(2.857142)
    r3.w = (saturate((r3.wwww)*(float4(2.857142,2.857142,2.857142,2.857142)))).w;
    // 102: mul r3.w, r3.w, cb0[18].w
    r3.w = ((r3.wwww)*(source[18].wwww)).w;
    // 103: mad r6.xyz, r3.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 104: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r7.xyz, -r6.xyzx, r3.wwww
    r7.xyz = ((-(r6.xyzx))+(r3.wwww)).xyz;
    // 106: mad r6.xyz, cb0[16].xxxx, r7.xyzx, r6.xyzx
    r6.xyz = ((source[16].xxxx)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 107: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 108: add r7.xyz, -r6.xyzx, r3.wwww
    r7.xyz = ((-(r6.xyzx))+(r3.wwww)).xyz;
    // 109: mad r6.xyz, cb0[16].yyyy, r7.xyzx, r6.xyzx
    r6.xyz = ((source[16].yyyy)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 110: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 111: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xyzx))).xyz;
    // 112: mad r4.xyz, r2.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 113: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r4.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r4.xyzx))).xyz;
    // 114: mad r3.xyz, r1.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 115: mul_sat r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = (saturate((r0.wwww)*(r3.xyzx))).xyz;
    // 116: mad r5.xyz, cb0[13].zzzz, r3.xyzx, -r4.xyzx
    r5.xyz = ((source[13].zzzz)*(r3.xyzx)+(-(r4.xyzx))).xyz;
    // 117: mul r3.xyz, r3.xyzx, cb0[13].zzzz
    r3.xyz = ((r3.xyzx)*(source[13].zzzz)).xyz;
    // 118: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 120: mad r3.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r3.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 121: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 122: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 123: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 124: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 125: dp3 r0.w, r4.xyzx, r1.yzwy
    r0.w = (dot((r4.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 126: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 127: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 128: mul r4.yzw, r4.yyyy, cb0[21].xxyz
    r4.yzw = ((r4.yyyy)*(source[21].xxyz)).yzw;
    // 129: mad r4.xyz, r4.xxxx, cb0[20].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[20].xyzx)+(r4.yzwy)).xyz;
    // 130: mul r4.xyz, r4.xyzx, cb0[22].wwww
    r4.xyz = ((r4.xyzx)*(source[22].wwww)).xyz;
    // 131: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 132: dp2_sat r6.x, r1.zwzz, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r1.zwzz).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 133: dp3_sat r6.y, r1.yzwy, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r1.yzwy).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 134: dp3_sat r6.z, r1.yzwy, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r1.yzwy).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 135: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 136: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t6.xyzw, s5
    r7.xyz = ((float4(input.bakedCoefficients,1.0)).xyzw).xyz;
    // 137: mul r7.xyz, r7.xyzx, cb0[24].xyzx
    r7.xyz = ((r7.xyzx)*(source[24].xyzx)).xyz;
    // 138: dp3 r0.w, r7.xyzx, r6.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 139: sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t5.xyzw, s5
    r6.xyz = ((float4(input.bakedAverage,1.0)).xyzw).xyz;
    // 140: mul r6.xyz, r6.xyzx, cb0[23].xyzx
    r6.xyz = ((r6.xyzx)*(source[23].xyzx)).xyz;
    // 141: mul r9.xyz, r0.wwww, r6.xyzx
    r9.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 142: mad r4.xyz, r6.xyzx, r0.wwww, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // 143: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 144: div r4.xyz, r9.xyzx, r4.xyzx
    r4.xyz = ((r9.xyzx)/(r4.xyzx)).xyz;
    // 145: mad r5.xyz, r3.xyzx, r9.xyzx, r5.xyzx
    r5.xyz = ((r3.xyzx)*(r9.xyzx)+(r5.xyzx)).xyz;
    // 146: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 147: dp2_sat r4.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 148: dp3_sat r4.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 149: dp3_sat r4.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 150: log r2.xyz, r4.xyzx
    r2.xyz = (log2(r4.xyzx)).xyz;
    // 151: add r1.x, cb0[19].y, l(-1.000000)
    r1.x = ((source[19].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 152: mad r1.x, r8.w, r1.x, l(2.000000)
    r1.x = ((r8.wwww)*(r1.xxxx)+(float4(2.000000,2.000000,2.000000,2.000000))).x;
    // 153: mul r2.xyz, r2.xyzx, r1.xxxx
    r2.xyz = ((r2.xyzx)*(r1.xxxx)).xyz;
    // 154: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 155: dp3 r1.x, r7.xyzx, r2.xyzx
    r1.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 156: dp3 r2.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 157: add r2.xyz, -r8.xyzx, r2.xxxx
    r2.xyz = ((-(r8.xyzx))+(r2.xxxx)).xyz;
    // 158: mad r2.xyz, cb0[16].xxxx, r2.xyzx, r8.xyzx
    r2.xyz = ((source[16].xxxx)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 159: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 160: add r4.xyz, -r2.xyzx, r2.wwww
    r4.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 161: mad r2.xyz, cb0[16].yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((source[16].yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 162: mul r2.xyz, r2.xyzx, cb0[19].xxxx
    r2.xyz = ((r2.xyzx)*(source[19].xxxx)).xyz;
    // 163: mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 164: mul r2.xyz, r6.xyzx, r2.xyzx
    r2.xyz = ((r6.xyzx)*(r2.xyzx)).xyz;
    // 165: mad r4.xyz, r2.xyzx, r1.xxxx, r5.xyzx
    r4.xyz = ((r2.xyzx)*(r1.xxxx)+(r5.xyzx)).xyz;
    // 166: mul r2.xyz, r1.xxxx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // 167: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 168: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 169: mad o0.xyz, r3.xyzx, cb0[22].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[22].xyzx)+(r0.xyzx)).xyz;
    // 170: mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // 171: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 172: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 173: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 174: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 175: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 176: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 177: mul r2.xyz, r1.xxxx, v0.xyzx
    r2.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 178: mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // 179: mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 180: dp3 r0.z, r0.xyzx, r1.yzwy
    r0.z = (dot((r0.xyzx).xyz,(r1.yzwy).xyz).xxxx).z;
    // 181: dp3 r0.x, r2.xyzx, r1.yzwy
    r0.x = (dot((r2.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // 182: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 183: dp3 r0.y, r2.xyzx, r1.yzwy
    r0.y = (dot((r2.xyzx).xyz,(r1.yzwy).xyz).xxxx).y;
    // 184: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 185: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 186: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 187: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 188: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 189: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 190: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 191: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 192: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 193: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 194: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 195: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 196: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 197: mul o4.z, r0.w, r4.x
    output.targets[4].z = ((r0.wwww)*(r4.xxxx)).z;
    // 198: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 199: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 200: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 201: ret
    return output;
}



// source.character.monster-8d18db0756e4.v1 / source program dc0bd97d65a4484796b92f7deeb55fc5
