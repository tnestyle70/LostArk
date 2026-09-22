SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase208(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.x, r0.x, v6.z
    r0.x = ((r0.xxxx)*(v6.zzzz)).x;
    // 4: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 5: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 6: mul r0.yzw, r0.yyyy, cb0[4].xxyz
    r0.yzw = ((r0.yyyy)*(source[4].xxyz)).yzw;
    // 7: mad r0.xyz, r0.xxxx, cb0[3].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[3].xyzx)+(r0.yzwy)).xyz;
    // 8: mul r0.xyz, r0.xyzx, cb0[5].wwww
    r0.xyz = ((r0.xyzx)*(source[5].wwww)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 10: mul r2.xyz, r1.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(source[1].xyzx)).xyz;
    // 11: mad r1.xyz, -r1.xyzx, cb0[1].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[1].xyzx)+(r1.xyzx)).xyz;
    // 12: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 13: mul r1.xyz, r1.xyzx, cb0[2].xxxx
    r1.xyz = ((r1.xyzx)*(source[2].xxxx)).xyz;
    // 14: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 15: mad r2.xyz, r0.xyzx, r1.xyzx, cb0[0].xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(source[0].xyzx)).xyz;
    // 16: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 17: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 18: mad o0.xyz, r1.xyzx, cb0[5].xyzx, r2.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[5].xyzx)+(r2.xyzx)).xyz;
    // 19: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 20: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 21: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 22: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 23: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 24: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 27: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 28: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 29: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 30: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 31: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 32: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 33: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 34: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 35: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 36: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 37: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 38: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 39: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 40: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 41: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 42: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 43: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 44: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 45: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 46: ret
    return output;
}

// source.character.static-map-real-pbr-masked.v1 / source program 628539a6a0c8ae4189734205f8186c74
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase210(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[21].z=(g_SourceCharacterTime.xxxx).x;
    source[22].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[37]=1.f; source[38]=1.f;
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[24]=g_SourceCharacterEnvironmentColor;source[25]=g_SourceCharacterEnvironmentRotation;}
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
    // 7: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 8: mul r1.x, r0.x, l(0.125000)
    r1.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 9: mul r2.y, cb0[4].y, cb0[14].y
    r2.y = ((source[4].yyyy)*(source[14].yyyy)).y;
    // 10: mov r1.y, v4.y
    r1.y = (v4.yyyy).y;
    // 11: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 12: add r1.xy, r1.xyxx, r2.xyxx
    r1.xy = ((r1.xyxx)+(r2.xyxx)).xy;
    // 13: frc r0.x, cb0[4].x
    r0.x = (frac(source[4].xxxx)).x;
    // 14: add r1.z, -r0.x, cb0[4].x
    r1.z = ((-(r0.xxxx))+(source[4].xxxx)).z;
    // 15: mul r2.z, r1.z, l(0.125000)
    r2.z = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 16: add r1.xy, r1.xyxx, r2.zwzz
    r1.xy = ((r1.xyxx)+(r2.zwzz)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 18: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 19: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: add r2.x, -cb0[4].w, l(1.000000)
    r2.x = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 21: mul r2.x, r2.x, cb0[21].z
    r2.x = ((r2.xxxx)*(source[21].zzzz)).x;
    // 22: mul r2.x, r2.x, l(6.283185)
    r2.x = ((r2.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 23: sincos r2.x, null, r2.x
    r2.x = (sin(r2.xxxx)).x;
    // 24: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: mul r2.y, cb0[4].z, l(1.500000)
    r2.y = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 26: mul r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)*(r2.xxxx)).x;
    // 27: mad r2.x, r2.x, l(0.500000), cb0[4].z
    r2.x = ((r2.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).x;
    // 28: mul r1.xyz, r1.xyzx, r2.xxxx
    r1.xyz = ((r1.xyzx)*(r2.xxxx)).xyz;
    // 29: dp3 r2.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 30: add r2.xyz, -r0.yzwy, r2.xxxx
    r2.xyz = ((-(r0.yzwy))+(r2.xxxx)).xyz;
    // 31: mad r2.xyz, cb0[16].wwww, r2.xyzx, r0.yzwy
    r2.xyz = ((source[16].wwww)*(r2.xyzx)+(r0.yzwy)).xyz;
    // 32: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 33: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 34: mad r2.xyz, cb0[17].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[17].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 35: mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 36: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 37: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 38: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 39: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 40: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 42: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 43: mul r2.w, r6.y, cb0[16].y
    r2.w = ((r6.yyyy)*(source[16].yyyy)).w;
    // 44: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 45: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: lt r5.xzw, |r5.xxzy|, l(0.000001, 0.000000, 0.000001, 0.000001)
    r5.xzw = (asfloat((uint4)((abs(r5.xxzy))<(float4(0.000001,0.000000,0.000001,0.000001))) * 0xffffffffu)).xzw;
    // 47: movc r2.w, r5.z, l(0), r2.w
    r2.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 48: mad r3.xyz, r2.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 49: mul_sat r4.w, r2.w, cb2[3].w
    r4.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 50: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: add r7.xyz, -r3.xyzx, r2.wwww
    r7.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 52: mad r7.xyz, cb0[16].wwww, r7.xyzx, r3.xyzx
    r7.xyz = ((source[16].wwww)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 53: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 54: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 55: add r3.xyz, -r7.xyzx, r2.wwww
    r3.xyz = ((-(r7.xyzx))+(r2.wwww)).xyz;
    // 56: mad r3.xyz, cb0[17].xxxx, r3.xyzx, r7.xyzx
    r3.xyz = ((source[17].xxxx)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 57: mad r7.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 58: mad r8.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 59: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 60: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 61: mul r8.xyz, r2.xyzx, r3.xyzx
    r8.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 62: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 63: mad r9.xyz, -r3.xyzx, r2.xyzx, r2.wwww
    r9.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 64: mad r2.xyz, r3.xyzx, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r2.xyz = ((r3.xyzx)*(r2.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 65: mad r3.xyz, cb0[16].wwww, r9.xyzx, r8.xyzx
    r3.xyz = ((source[16].wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 66: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 67: add r8.xyz, -r3.xyzx, r2.wwww
    r8.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 68: mad r3.xyz, cb0[17].xxxx, r8.xyzx, r3.xyzx
    r3.xyz = ((source[17].xxxx)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 69: mul r3.xyz, r7.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r3.xyzx)).xyz;
    // 70: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 71: mad r1.xyz, r1.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 72: mul r1.w, r6.x, cb0[22].y
    r1.w = ((r6.xxxx)*(source[22].yyyy)).w;
    // 73: mul r2.w, r6.z, cb0[23].z
    r2.w = ((r6.zzzz)*(source[23].zzzz)).w;
    // 74: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 75: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: movc r2.w, r5.w, l(0), r2.w
    r2.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 77: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 78: min r4.z, r2.w, l(1.000000)
    r4.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 79: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 80: movc r1.w, r5.x, l(0), r1.w
    r1.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 81: add_sat r1.w, r1.w, cb0[22].z
    r1.w = (saturate((r1.wwww)+(source[22].zzzz))).w;
    // 82: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: mul r3.xyz, r2.wwww, cb0[13].xyzx
    r3.xyz = ((r2.wwww)*(source[13].xyzx)).xyz;
    // 84: mul r5.xzw, r1.xxyz, r3.xxyz
    r5.xzw = ((r1.xxyz)*(r3.xxyz)).xzw;
    // 85: mad r1.xyz, -r3.xyzx, r1.xyzx, r1.xyzx
    r1.xyz = ((-(r3.xyzx))*(r1.xyzx)+(r1.xyzx)).xyz;
    // 86: mad r1.xyz, r1.wwww, r1.xyzx, r5.xzwx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r5.xzwx)).xyz;
    // 87: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 88: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 89: mad_sat r3.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 90: mad r1.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r1.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 91: mad r5.xzw, r3.xxyz, l(-4.795100, 0.000000, -4.795100, -4.795100), l(0.641700, 0.000000, 0.641700, 0.641700)
    r5.xzw = ((r3.xxyz)*(float4(-4.795100,0.000000,-4.795100,-4.795100))+(float4(0.641700,0.000000,0.641700,0.641700))).xzw;
    // 92: mad r1.xyz, r1.wwww, r1.xyzx, r5.xzwx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r5.xzwx)).xyz;
    // 93: mad r5.xzw, r3.xxyz, l(2.755200, 0.000000, 2.755200, 2.755200), l(0.690300, 0.000000, 0.690300, 0.690300)
    r5.xzw = ((r3.xxyz)*(float4(2.755200,0.000000,2.755200,2.755200))+(float4(0.690300,0.000000,0.690300,0.690300))).xzw;
    // 94: mad r1.xyz, r1.xyzx, r1.wwww, r5.xzwx
    r1.xyz = ((r1.xyzx)*(r1.wwww)+(r5.xzwx)).xyz;
    // 95: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 96: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 97: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: dp3 r4.x, v1.xyzx, v1.xyzx
    r4.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 99: rsq r4.x, r4.x
    r4.x = (rsqrt(r4.xxxx)).x;
    // 100: mul r5.xzw, r4.xxxx, v1.xxyz
    r5.xzw = ((r4.xxxx)*(v1.xxyz)).xzw;
    // 101: dp3 r4.x, v0.xyzx, v0.xyzx
    r4.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 102: rsq r4.x, r4.x
    r4.x = (rsqrt(r4.xxxx)).x;
    // 103: mul r6.xyz, r4.xxxx, v0.xyzx
    r6.xyz = ((r4.xxxx)*(v0.xyzx)).xyz;
    // 104: mul r8.xyz, r5.wxzw, r6.yzxy
    r8.xyz = ((r5.wxzw)*(r6.yzxy)).xyz;
    // 105: mad r8.xyz, r5.zwxz, r6.zxyz, -r8.xyzx
    r8.xyz = ((r5.zwxz)*(r6.zxyz)+(-(r8.xyzx))).xyz;
    // 106: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 107: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 108: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 109: dp2 r6.w, r4.xyxx, r4.xyxx
    r6.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 110: mul r9.xy, r4.xyxx, cb0[16].xxxx
    r9.xy = ((r4.xyxx)*(source[16].xxxx)).xy;
    // 111: add r4.x, -r6.w, l(1.000000)
    r4.x = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: max r4.x, r4.x, l(0.000000)
    r4.x = (max(r4.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 113: sqrt r4.x, r4.x
    r4.x = (sqrt(r4.xxxx)).x;
    // 114: add r9.z, r4.x, l(0.000010)
    r9.z = ((r4.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 115: dp3 r4.x, r9.xyzx, r9.xyzx
    r4.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 116: sqrt r4.x, r4.x
    r4.x = (sqrt(r4.xxxx)).x;
    // 117: div r9.xyz, r9.xyzx, r4.xxxx
    r9.xyz = ((r9.xyzx)/(r4.xxxx)).xyz;
    // 118: dp3 r4.x, r9.xyzx, r9.xyzx
    r4.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 119: rsq r4.x, r4.x
    r4.x = (rsqrt(r4.xxxx)).x;
    // 120: mul r10.xyz, r4.xxxx, r9.xyzx
    r10.xyz = ((r4.xxxx)*(r9.xyzx)).xyz;
    // 121: dp3 r11.y, r8.xyzx, r10.xyzx
    r11.y = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 122: dp3 r11.x, r6.xyzx, r10.xyzx
    r11.x = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 123: dp2 r12.z, r11.xyxx, cb0[25].xyxx
    r12.z = (dot((r11.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 124: mul r4.xy, cb0[25].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((source[25].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 125: dp2 r12.x, r11.xyxx, r4.xyxx
    r12.x = (dot((r11.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 126: dp3 r12.y, r5.xzwx, r10.xyzx
    r12.y = (dot((r5.xzwx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 127: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 128: dp4 r13.x, cb0[26].xyzw, r12.xyzw
    r13.x = (dot((source[26].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 129: dp4 r13.y, cb0[27].xyzw, r12.xyzw
    r13.y = (dot((source[27].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 130: dp4 r13.z, cb0[28].xyzw, r12.xyzw
    r13.z = (dot((source[28].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 131: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 132: dp4 r15.x, cb0[29].xyzw, r14.xyzw
    r15.x = (dot((source[29].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 133: dp4 r15.y, cb0[30].xyzw, r14.xyzw
    r15.y = (dot((source[30].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 134: dp4 r15.z, cb0[31].xyzw, r14.xyzw
    r15.z = (dot((source[31].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 135: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 136: mul r6.w, r12.y, r12.y
    r6.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 137: mov r11.z, r12.y
    r11.z = (r12.yyyy).z;
    // 138: mad r6.w, r12.x, r12.x, -r6.w
    r6.w = ((r12.xxxx)*(r12.xxxx)+(-(r6.wwww))).w;
    // 139: mad r12.xyz, cb0[32].xyzx, r6.wwww, r13.xyzx
    r12.xyz = ((source[32].xyzx)*(r6.wwww)+(r13.xyzx)).xyz;
    // 140: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 141: mul r12.xyz, r12.xyzx, cb0[24].xyzx
    r12.xyz = ((r12.xyzx)*(source[24].xyzx)).xyz;
    // 142: mul r12.xyz, r12.xyzx, cb0[25].zzzz
    r12.xyz = ((r12.xyzx)*(source[25].zzzz)).xyz;
    // 143: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[24].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[24].wwww)).xyz;
    // 144: dp3 r6.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 145: add r12.xyz, -r6.wwww, r12.xyzx
    r12.xyz = ((-(r6.wwww))+(r12.xyzx)).xyz;
    // 146: mad r12.xyz, r12.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r6.wwww
    r12.xyz = ((r12.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r6.wwww)).xyz;
    // 147: dp3 r6.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 148: dp3 r7.w, v5.xyzx, v5.xyzx
    r7.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 149: rsq r7.w, r7.w
    r7.w = (rsqrt(r7.wwww)).w;
    // 150: mul r13.xyz, r7.wwww, v5.xyzx
    r13.xyz = ((r7.wwww)*(v5.xyzx)).xyz;
    // 151: dp3 r7.w, r10.xyzx, r13.xyzx
    r7.w = (dot((r10.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 152: deriv_rtx_coarse r14.x, r7.w
    r14.x = (ddx_coarse(r7.wwww)).x;
    // 153: deriv_rty_coarse r14.y, r7.w
    r14.y = (ddy_coarse(r7.wwww)).y;
    // 154: dp2 r8.w, r14.xyxx, r14.xyxx
    r8.w = (dot((r14.xyxx).xy,(r14.xyxx).xy).xxxx).w;
    // 155: sqrt r8.w, r8.w
    r8.w = (sqrt(r8.wwww)).w;
    // 156: mad r8.w, r8.w, l(0.300000), r4.z
    r8.w = ((r8.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz)).w;
    // 157: min r14.y, r8.w, l(1.000000)
    r14.y = (min(r8.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 158: mad r8.w, r14.y, l(2.000000), l(2.000000)
    r8.w = ((r14.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 159: div r6.w, r6.w, r8.w
    r6.w = ((r6.wwww)/(r8.wwww)).w;
    // 160: mad r6.w, r2.w, l(5.000000), r6.w
    r6.w = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r6.wwww)).w;
    // 161: add_sat r6.w, r4.w, r6.w
    r6.w = (saturate((r4.wwww)+(r6.wwww))).w;
    // 162: mad r9.w, r6.w, l(-2.000000), l(3.000000)
    r9.w = ((r6.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 163: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 164: mul r6.w, r6.w, r9.w
    r6.w = ((r6.wwww)*(r9.wwww)).w;
    // 165: log r6.w, r6.w
    r6.w = (log2(r6.wwww)).w;
    // 166: mul r6.w, r6.w, l(1.500000)
    r6.w = ((r6.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 167: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 168: mul r12.xyz, r6.wwww, r12.xyzx
    r12.xyz = ((r6.wwww)*(r12.xyzx)).xyz;
    // 169: mov_sat r3.w, cb0[23].y
    r3.w = (saturate(source[23].yyyy)).w;
    // 170: mad r15.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r15.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 171: mul r6.w, r3.w, l(0.080000)
    r6.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 172: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 173: mad r15.xyz, r4.wwww, r15.xyzx, r6.wwww
    r15.xyz = ((r4.wwww)*(r15.xyzx)+(r6.wwww)).xyz;
    // 174: mul_sat r3.w, r15.y, l(50.000000)
    r3.w = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 175: add r6.w, -r14.y, l(1.000000)
    r6.w = ((-(r14.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: max r16.xyz, r15.xyzx, r6.wwww
    r16.xyz = (max(r15.xyzx,r6.wwww)).xyz;
    // 177: add r16.xyz, -r15.xyzx, r16.xyzx
    r16.xyz = ((-(r15.xyzx))+(r16.xyzx)).xyz;
    // 178: mul r16.xyz, r3.wwww, r16.xyzx
    r16.xyz = ((r3.wwww)*(r16.xyzx)).xyz;
    // 179: add r3.w, r7.w, l(1.000000)
    r3.w = ((r7.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: mul r17.xyz, r7.wwww, r10.xyzx
    r17.xyz = ((r7.wwww)*(r10.xyzx)).xyz;
    // 181: mov_sat r7.w, r7.w
    r7.w = (saturate(r7.wwww)).w;
    // 182: log r6.w, r7.w
    r6.w = (log2(r7.wwww)).w;
    // 183: mul r6.w, r6.w, cb0[1].y
    r6.w = ((r6.wwww)*(source[1].yyyy)).w;
    // 184: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 185: mad_sat r6.w, r6.w, cb0[1].w, cb0[1].z
    r6.w = (saturate((r6.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 186: mul r6.w, r6.w, cb0[23].w
    r6.w = ((r6.wwww)*(source[23].wwww)).w;
    // 187: mad r17.xyz, r17.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r13.xyzx
    r17.xyz = ((r17.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r13.xyzx))).xyz;
    // 188: add r7.w, r17.z, l(1.000000)
    r7.w = ((r17.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: min r7.w, r7.w, l(1.000000)
    r7.w = (min(r7.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: add_sat r14.x, r3.w, -r7.w
    r14.x = (saturate((r3.wwww)+(-(r7.wwww)))).x;
    // 191: sample_indexable(texture2d)(float,float,float,float) r14.zw, r14.xyxx, t5.zwxy, s7
    r14.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 192: add r3.w, r1.w, r14.x
    r3.w = ((r1.wwww)+(r14.xxxx)).w;
    // 193: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 194: mul r18.xyz, r14.wwww, r15.xyzx
    r18.xyz = ((r14.wwww)*(r15.xyzx)).xyz;
    // 195: mad r16.xyz, r16.xyzx, r14.zzzz, r18.xyzx
    r16.xyz = ((r16.xyzx)*(r14.zzzz)+(r18.xyzx)).xyz;
    // 196: div r7.w, l(1.000000, 1.000000, 1.000000, 1.000000), r14.w
    r7.w = r14.w != 0.f ? 1.f / r14.w : 0.f; // Unrecovered engine BRDF contributes zero, never NaN.
    // 197: add r7.w, r7.w, l(-1.000000)
    r7.w = ((r7.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 198: mad r14.xzw, r15.xxyz, r7.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r14.xzw = ((r15.xxyz)*(r7.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 199: dp3 r7.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r7.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 200: mad r15.xyz, r7.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r7.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 201: mad r18.xyz, -r16.xyzx, r14.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r16.xyzx))*(r14.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 202: mul r14.xzw, r14.xxzw, r16.xxyz
    r14.xzw = ((r14.xxzw)*(r16.xxyz)).xzw;
    // 203: mul r16.xyz, r3.xyzx, r18.xyzx
    r16.xyz = ((r3.xyzx)*(r18.xyzx)).xyz;
    // 204: mul r18.xyz, r12.xyzx, r18.xyzx
    r18.xyz = ((r12.xyzx)*(r18.xyzx)).xyz;
    // 205: add r7.w, -r4.w, l(1.000000)
    r7.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 206: mul r16.xyz, r7.wwww, r16.xyzx
    r16.xyz = ((r7.wwww)*(r16.xyzx)).xyz;
    // 207: mul r12.xyz, r12.xyzx, r16.xyzx
    r12.xyz = ((r12.xyzx)*(r16.xyzx)).xyz;
    // 208: mul r12.xyz, r1.xyzx, r12.xyzx
    r12.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 209: dp3 r8.y, r8.xyzx, r17.xyzx
    r8.y = (dot((r8.xyzx).xyz,(r17.xyzx).xyz).xxxx).y;
    // 210: dp3 r8.x, r6.xyzx, r17.xyzx
    r8.x = (dot((r6.xyzx).xyz,(r17.xyzx).xyz).xxxx).x;
    // 211: dp2 r6.x, r8.xyxx, r4.xyxx
    r6.x = (dot((r8.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 212: dp2 r6.z, r8.xyxx, cb0[25].xyxx
    r6.z = (dot((r8.xyxx).xy,(source[25].xyxx).xy).xxxx).z;
    // 213: mul r4.x, r14.y, l(5.000000)
    r4.x = ((r14.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 214: mul r4.y, r14.y, r14.y
    r4.y = ((r14.yyyy)*(r14.yyyy)).y;
    // 215: mul r3.w, r3.w, r4.y
    r3.w = ((r3.wwww)*(r4.yyyy)).w;
    // 216: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 217: add r3.w, r1.w, r3.w
    r3.w = ((r1.wwww)+(r3.wwww)).w;
    // 218: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 219: add_sat r1.w, r3.w, l(-1.000000)
    r1.w = (saturate((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 220: dp3 r6.y, r5.xzwx, r17.xyzx
    r6.y = (dot((r5.xzwx).xyz,(r17.xyzx).xyz).xxxx).y;
    // 221: sample_l_indexable(texturecube)(float,float,float,float) r16.xyzw, r6.xyzx, t6.xyzw, s6, r4.x
    r16.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r4.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 222: mul r5.xzw, r16.xxyz, r16.wwww
    r5.xzw = ((r16.xxyz)*(r16.wwww)).xzw;
    // 223: mul r5.xzw, r5.xxzw, cb0[24].xxyz
    r5.xzw = ((r5.xxzw)*(source[24].xxyz)).xzw;
    // 224: mul r5.xzw, r5.xxzw, cb0[25].zzzz
    r5.xzw = ((r5.xxzw)*(source[25].zzzz)).xzw;
    // 225: mad r5.xzw, r5.xxzw, l(6.000000, 0.000000, 6.000000, 6.000000), cb0[24].wwww
    r5.xzw = ((r5.xxzw)*(float4(6.000000,0.000000,6.000000,6.000000))+(source[24].wwww)).xzw;
    // 226: dp3 r3.w, r5.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 227: add r5.xzw, -r3.wwww, r5.xxzw
    r5.xzw = ((-(r3.wwww))+(r5.xxzw)).xzw;
    // 228: mad r5.xzw, r5.xxzw, l(0.800000, 0.000000, 0.800000, 0.800000), r3.wwww
    r5.xzw = ((r5.xxzw)*(float4(0.800000,0.000000,0.800000,0.800000))+(r3.wwww)).xzw;
    // 229: dp3 r3.w, r5.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 230: div r3.w, r3.w, r8.w
    r3.w = ((r3.wwww)/(r8.wwww)).w;
    // 231: mad r2.w, r2.w, l(5.000000), r3.w
    r2.w = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 232: add_sat r2.w, r4.w, r2.w
    r2.w = (saturate((r4.wwww)+(r2.wwww))).w;
    // 233: mad r3.w, r2.w, l(-2.000000), l(3.000000)
    r3.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 234: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 235: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 236: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 237: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 238: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 239: mul r5.xzw, r2.wwww, r5.xxzw
    r5.xzw = ((r2.wwww)*(r5.xxzw)).xzw;
    // 240: mul r6.xyz, r5.xzwx, r14.xzwx
    r6.xyz = ((r5.xzwx)*(r14.xzwx)).xyz;
    // 241: mad r2.w, r1.w, r15.x, r15.y
    r2.w = ((r1.wwww)*(r15.xxxx)+(r15.yyyy)).w;
    // 242: mad r2.w, r2.w, r1.w, r15.z
    r2.w = ((r2.wwww)*(r1.wwww)+(r15.zzzz)).w;
    // 243: mul r2.w, r1.w, r2.w
    r2.w = ((r1.wwww)*(r2.wwww)).w;
    // 244: max r1.w, r1.w, r2.w
    r1.w = (max(r1.wwww,r2.wwww)).w;
    // 245: mad r6.xyz, r6.xyzx, r1.wwww, r12.xyzx
    r6.xyz = ((r6.xyzx)*(r1.wwww)+(r12.xyzx)).xyz;
    // 246: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 247: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 248: mul r8.xyz, r2.wwww, v6.xyzx
    r8.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 249: dp3 r2.w, r8.xyzx, r10.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 250: dp3 r3.w, -r8.xyzx, r10.xyzx
    r3.w = (dot((-(r8.xyzx)).xyz,(r10.xyzx).xyz).xxxx).w;
    // 251: mad r4.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 252: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 253: mad r8.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 254: mul r8.xy, r8.xyxx, r8.xyxx
    r8.xy = ((r8.xyxx)*(r8.xyxx)).xy;
    // 255: mul r8.yzw, r8.yyyy, cb0[35].xxyz
    r8.yzw = ((r8.yyyy)*(source[35].xxyz)).yzw;
    // 256: mad r8.xyz, r8.xxxx, cb0[34].xyzx, r8.yzwy
    r8.xyz = ((r8.xxxx)*(source[34].xyzx)+(r8.yzwy)).xyz;
    // 257: mul r8.xyz, r8.xyzx, cb0[36].wwww
    r8.xyz = ((r8.xyzx)*(source[36].wwww)).xyz;
    // 258: mul r12.xyz, r3.xyzx, r8.xyzx
    r12.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 259: dp2_sat r15.x, r10.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r15.x = (saturate(dot((r10.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 260: dp3_sat r15.y, r10.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r15.y = (saturate(dot((r10.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 261: dp3_sat r15.z, r10.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r15.z = (saturate(dot((r10.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 262: mul r10.xyz, r15.xyzx, r15.xyzx
    r10.xyz = ((r15.xyzx)*(r15.xyzx)).xyz;
    // 263: sample_indexable(texture2d)(float,float,float,float) r15.xyz, v3.zwzz, t8.xyzw, s5
    r15.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 264: mul r15.xyz, r15.xyzx, cb0[38].xyzx
    r15.xyz = ((r15.xyzx)*(source[38].xyzx)).xyz;
    // 265: dp3 r2.w, r15.xyzx, r10.xyzx
    r2.w = (dot((r15.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 266: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t7.xyzw, s5
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 267: mul r10.xyz, r10.xyzx, cb0[37].xyzx
    r10.xyz = ((r10.xyzx)*(source[37].xyzx)).xyz;
    // 268: mul r16.xyz, r2.wwww, r10.xyzx
    r16.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 269: mad r12.xyz, r3.xyzx, r16.xyzx, r12.xyzx
    r12.xyz = ((r3.xyzx)*(r16.xyzx)+(r12.xyzx)).xyz;
    // 270: mul r1.xyz, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 271: mul r1.xyz, r1.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 272: mul r1.xyz, r18.xyzx, r1.xyzx
    r1.xyz = ((r18.xyzx)*(r1.xyzx)).xyz;
    // 273: mad r1.xyz, -r1.xyzx, r4.wwww, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(r4.wwww)+(r1.xyzx)).xyz;
    // 274: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 275: mad r1.xyz, r6.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r1.xyzx
    r1.xyz = ((r6.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r1.xyzx)).xyz;
    // 276: dp2_sat r6.x, r17.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r17.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 277: dp3_sat r6.y, r17.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r17.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 278: dp3_sat r6.z, r17.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r17.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 279: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 280: dp3 r3.w, r15.xyzx, r6.xyzx
    r3.w = (dot((r15.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 281: add r2.w, r2.w, -r3.w
    r2.w = ((r2.wwww)+(-(r3.wwww))).w;
    // 282: mad r2.w, r4.z, r2.w, r3.w
    r2.w = ((r4.zzzz)*(r2.wwww)+(r3.wwww)).w;
    // 283: mad r6.xyz, r10.xyzx, r2.wwww, r8.xyzx
    r6.xyz = ((r10.xyzx)*(r2.wwww)+(r8.xyzx)).xyz;
    // 284: mul r8.xyz, r2.wwww, r10.xyzx
    r8.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 285: mul r12.xyz, r1.wwww, r6.xyzx
    r12.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 286: add r6.xyz, r6.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r6.xyz = ((r6.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 287: div r6.xyz, r8.xyzx, r6.xyzx
    r6.xyz = ((r8.xyzx)/(r6.xyzx)).xyz;
    // 288: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 289: mul r5.xzw, r5.xxzw, r12.xxyz
    r5.xzw = ((r5.xxzw)*(r12.xxyz)).xzw;
    // 290: mul r5.xzw, r5.xxzw, r14.xxzw
    r5.xzw = ((r5.xxzw)*(r14.xxzw)).xzw;
    // 291: mad r1.xyz, r5.xzwx, l(0.600000, 0.600000, 0.600000, 0.000000), r1.xyzx
    r1.xyz = ((r5.xzwx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r1.xyzx)).xyz;
    // 292: mul r5.xzw, r5.xxzw, l(0.600000, 0.000000, 0.600000, 0.600000)
    r5.xzw = ((r5.xxzw)*(float4(0.600000,0.000000,0.600000,0.600000))).xzw;
    // 293: dp3 o4.x, r5.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 294: dp3 r2.w, r5.yyyy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.yyyy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 295: add r2.w, -r5.y, r2.w
    r2.w = ((-(r5.yyyy))+(r2.wwww)).w;
    // 296: mad r2.w, cb0[16].w, r2.w, r5.y
    r2.w = ((source[16].wwww)*(r2.wwww)+(r5.yyyy)).w;
    // 297: dp3 r3.w, r2.wwww, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.wwww).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 298: add r3.w, -r2.w, r3.w
    r3.w = ((-(r2.wwww))+(r3.wwww)).w;
    // 299: mad r2.w, cb0[17].x, r3.w, r2.w
    r2.w = ((source[17].xxxx)*(r3.wwww)+(r2.wwww)).w;
    // 300: dp3 r3.w, r2.wwww, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r2.wwww).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 301: add r3.w, -r2.w, r3.w
    r3.w = ((-(r2.wwww))+(r3.wwww)).w;
    // 302: mul r3.w, r3.w, cb0[17].y
    r3.w = ((r3.wwww)*(source[17].yyyy)).w;
    // 303: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 304: add r4.z, r5.y, r5.x
    r4.z = ((r5.yyyy)+(r5.xxxx)).z;
    // 305: add r4.z, r5.z, r4.z
    r4.z = ((r5.zzzz)+(r4.zzzz)).z;
    // 306: add_sat r4.z, r5.w, r4.z
    r4.z = (saturate((r5.wwww)+(r4.zzzz))).z;
    // 307: mad r2.w, r4.z, r3.w, r2.w
    r2.w = ((r4.zzzz)*(r3.wwww)+(r2.wwww)).w;
    // 308: dp3 r3.w, r9.xyzx, r13.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 309: mul_sat r4.z, r3.w, cb0[17].z
    r4.z = (saturate((r3.wwww)*(source[17].zzzz))).z;
    // 310: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 311: mul_sat r4.w, r13.z, cb0[17].z
    r4.w = (saturate((r13.zzzz)*(source[17].zzzz))).w;
    // 312: add r5.x, -|r13.z|, l(1.000000)
    r5.x = ((-(abs(r13.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 313: mul r3.w, r3.w, r5.x
    r3.w = ((r3.wwww)*(r5.xxxx)).w;
    // 314: add r4.zw, -r4.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r4.zw = ((-(r4.zzzw))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 315: add_sat r4.w, r4.w, -cb0[17].w
    r4.w = (saturate((r4.wwww)+(-(source[17].wwww)))).w;
    // 316: log r5.x, r4.w
    r5.x = (log2(r4.wwww)).x;
    // 317: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 318: mul r5.x, r5.x, cb0[18].x
    r5.x = ((r5.xxxx)*(source[18].xxxx)).x;
    // 319: exp r5.x, r5.x
    r5.x = (exp2(r5.xxxx)).x;
    // 320: mul r4.z, r4.z, r5.x
    r4.z = ((r4.zzzz)*(r5.xxxx)).z;
    // 321: movc r4.z, r4.w, l(0), r4.z
    r4.z = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.zzzz)).z;
    // 322: mul r4.w, r4.z, cb0[18].y
    r4.w = ((r4.zzzz)*(source[18].yyyy)).w;
    // 323: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 324: max r2.w, |r2.w|, l(0.000001)
    r2.w = (max(abs(r2.wwww),float4(0.000001,0.000001,0.000001,0.000001))).w;
    // 325: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 326: mul r2.w, r2.w, cb0[18].w
    r2.w = ((r2.wwww)*(source[18].wwww)).w;
    // 327: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 328: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 329: mov_sat r4.w, cb0[19].x
    r4.w = (saturate(source[19].xxxx)).w;
    // 330: log r5.x, r4.w
    r5.x = (log2(r4.wwww)).x;
    // 331: lt r4.w, r4.w, l(0.000001)
    r4.w = (asfloat((uint4)((r4.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 332: mul r5.x, r5.x, cb0[19].y
    r5.x = ((r5.xxxx)*(source[19].yyyy)).x;
    // 333: exp r5.x, r5.x
    r5.x = (exp2(r5.xxxx)).x;
    // 334: min r5.x, r5.x, l(1.000000)
    r5.x = (min(r5.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 335: mul r5.x, -r2.w, r5.x
    r5.x = ((-(r2.wwww))*(r5.xxxx)).x;
    // 336: movc r4.w, r4.w, l(0), r5.x
    r4.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).w;
    // 337: add r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)+(r4.wwww)).w;
    // 338: mul r5.xyz, cb0[5].xyzx, cb0[5].wwww
    r5.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 339: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 340: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 341: dp2 r2.w, cb0[6].xyxx, r6.xyxx
    r2.w = (dot((source[6].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 342: add r2.w, r2.w, cb0[20].w
    r2.w = ((r2.wwww)+(source[20].wwww)).w;
    // 343: add_sat r2.w, r2.w, l(-0.500000)
    r2.w = (saturate((r2.wwww)+(float4(-0.500000,-0.500000,-0.500000,-0.500000)))).w;
    // 344: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 345: mul r6.xyz, r0.xxxx, r5.xyzx
    r6.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 346: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 347: mad r5.xyz, -r0.xxxx, r5.xyzx, r2.wwww
    r5.xyz = ((-(r0.xxxx))*(r5.xyzx)+(r2.wwww)).xyz;
    // 348: mad r5.xyz, cb0[16].wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((source[16].wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 349: dp3 r0.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 350: add r6.xyz, -r5.xyzx, r0.xxxx
    r6.xyz = ((-(r5.xyzx))+(r0.xxxx)).xyz;
    // 351: mad r5.xyz, cb0[17].xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((source[17].xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 352: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 353: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 354: div r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 355: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 356: add r6.xyz, -r2.xyzx, r0.xxxx
    r6.xyz = ((-(r2.xyzx))+(r0.xxxx)).xyz;
    // 357: add r2.xyz, r2.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)+(-(r6.xyzx))).xyz;
    // 358: mul r6.xyz, cb0[11].xyzx, cb0[21].yyyy
    r6.xyz = ((source[11].xyzx)*(source[21].yyyy)).xyz;
    // 359: mul r6.xyz, r6.xyzx, cb0[22].xxxx
    r6.xyz = ((r6.xyzx)*(source[22].xxxx)).xyz;
    // 360: mul r6.xyz, r4.zzzz, r6.xyzx
    r6.xyz = ((r4.zzzz)*(r6.xyzx)).xyz;
    // 361: mad r8.xyz, r4.zzzz, cb0[10].xyzx, -cb0[10].xyzx
    r8.xyz = ((r4.zzzz)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 362: add r0.x, r4.z, l(-1.000000)
    r0.x = ((r4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 363: mad r0.x, cb0[9].w, r0.x, l(1.000000)
    r0.x = ((source[9].wwww)*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 364: mad r8.xyz, cb0[10].wwww, r8.xyzx, cb0[10].xyzx
    r8.xyz = ((source[10].wwww)*(r8.xyzx)+(source[10].xyzx)).xyz;
    // 365: mad r2.xyz, r2.xyzx, r6.xyzx, r8.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 366: mad r2.xyz, r0.xxxx, cb0[9].xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(source[9].xyzx)+(r2.xyzx)).xyz;
    // 367: mad r2.xyz, r5.xyzx, r7.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)*(r7.xyzx)+(r2.xyzx)).xyz;
    // 368: log r0.x, |r3.w|
    r0.x = (log2(abs(r3.wwww))).x;
    // 369: lt r2.w, |r3.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 370: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 371: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 372: mul r5.xyz, r0.xxxx, cb0[12].xyzx
    r5.xyz = ((r0.xxxx)*(source[12].xyzx)).xyz;
    // 373: movc r5.xyz, r2.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 374: add r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)+(r5.xyzx)).xyz;
    // 375: mad r0.xyz, cb0[16].zzzz, r0.yzwy, r2.xyzx
    r0.xyz = ((source[16].zzzz)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 376: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 377: mul_sat r2.xyz, cb0[15].xyzx, cb0[15].wwww
    r2.xyz = (saturate((source[15].xyzx)*(source[15].wwww))).xyz;
    // 378: mul r5.xyz, r2.xyzx, r6.wwww
    r5.xyz = ((r2.xyzx)*(r6.wwww)).xyz;
    // 379: mul r2.xyz, r2.xyzx, cb0[23].wwww
    r2.xyz = ((r2.xyzx)*(source[23].wwww)).xyz;
    // 380: dp3_sat o5.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 381: mul r2.xyz, r7.wwww, r5.xyzx
    r2.xyz = ((r7.wwww)*(r5.xyzx)).xyz;
    // 382: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 383: dp3 r0.w, r15.xyzx, r0.wwww
    r0.w = (dot((r15.xyzx).xyz,(r0.wwww).xyz).xxxx).w;
    // 384: mul r5.xyz, r0.wwww, r10.xyzx
    r5.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 385: mul r4.yzw, r4.yyyy, cb0[35].xxyz
    r4.yzw = ((r4.yyyy)*(source[35].xxyz)).yzw;
    // 386: mad r4.xyz, r4.xxxx, cb0[34].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[34].xyzx)+(r4.yzwy)).xyz;
    // 387: mul r4.xyz, r4.xyzx, cb0[36].wwww
    r4.xyz = ((r4.xyzx)*(source[36].wwww)).xyz;
    // 388: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 389: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 390: mad r2.xyz, r3.xyzx, r5.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 391: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 392: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 393: mad o0.xyz, r3.xyzx, cb0[36].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[36].xyzx)+(r0.xyzx)).xyz;
    // 394: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 395: dp3 r0.x, r11.xyzx, r11.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 396: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 397: mul r0.xyz, r0.xxxx, r11.xyzx
    r0.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 398: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 399: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 400: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 401: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 402: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 403: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 404: movc r0.xy, r0.wwww, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 405: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 406: mul o4.z, r1.w, r1.x
    output.targets[4].z = ((r1.wwww)*(r1.xxxx)).z;
    // 407: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 408: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 409: ftou r0.x, cb0[33].z
    r0.x = (asfloat((uint4)(source[33].zzzz))).x;
    // 410: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 411: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 412: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 413: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 414: ret
    return output;
}


// source.character.selection-native-211.v1 / source program d436372ab9f93e4dbc85ad001cda5635
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase211(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17].z=(g_SourceCharacterTime.xxxx).x;
    // Original engine primitive opacity/environment rows.
    if (g_SourceCharacterEnvironmentEnabled != 0u) { source[20]=g_SourceCharacterEnvironmentColor; source[21]=g_SourceCharacterEnvironmentRotation; }
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
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
    // 7: dp3 r0.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 8: add r1.xyz, -r0.yzwy, r0.xxxx
    r1.xyz = ((-(r0.yzwy))+(r0.xxxx)).xyz;
    // 9: mad r1.xyz, cb0[15].wwww, r1.xyzx, r0.yzwy
    r1.xyz = ((source[15].wwww)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 10: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 11: add r2.xyz, -r1.xyzx, r0.xxxx
    r2.xyz = ((-(r1.xyzx))+(r0.xxxx)).xyz;
    // 12: mad r1.xyz, cb0[16].xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((source[16].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 13: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 14: max r3.xyz, r2.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r3.xyz = (max(r2.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 15: max r2.xyz, r2.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 16: min r2.xyz, r2.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 17: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 18: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 20: log r5.xyz, |r4.xzyx|
    r5.xyz = (log2(abs(r4.xzyx))).xyz;
    // 21: lt r4.xyz, |r4.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (asfloat((uint4)((abs(r4.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 22: mul r0.x, r5.y, cb0[15].y
    r0.x = ((r5.yyyy)*(source[15].yyyy)).x;
    // 23: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 24: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 25: movc r0.x, r4.y, l(0), r0.x
    r0.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 26: mad r2.xyz, r0.xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 27: mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 28: max r6.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 29: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 30: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 31: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 32: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 33: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 34: add r2.xyz, r2.xyzx, -r3.xyzx
    r2.xyz = ((r2.xyzx)+(-(r3.xyzx))).xyz;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r4.yw, v4.xyxx, t3.zxwy, s3, l(0.000000)
    r4.yw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxwy).yw;
    // 36: mad r2.xyz, r4.yyyy, r2.xyzx, r3.xyzx
    r2.xyz = ((r4.yyyy)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 37: mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 38: max r6.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r6.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 39: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 40: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 41: min r6.xyz, r6.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 42: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 43: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 44: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 45: mad r2.xyz, r4.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r4.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 46: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 47: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 48: mad r3.xyz, cb0[15].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[15].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 49: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 50: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: add r2.xyz, -r3.xyzx, r1.wwww
    r2.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 52: mad r2.xyz, cb0[16].xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((source[16].xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 53: mad r3.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mad r6.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 55: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 56: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 57: mul r6.xyz, r1.xyzx, r2.xyzx
    r6.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 58: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 59: mad r1.xyz, -r2.xyzx, r1.xyzx, r1.wwww
    r1.xyz = ((-(r2.xyzx))*(r1.xyzx)+(r1.wwww)).xyz;
    // 60: mad r1.xyz, cb0[15].wwww, r1.xyzx, r6.xyzx
    r1.xyz = ((source[15].wwww)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 61: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 63: mad r1.xyz, cb0[16].xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((source[16].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 64: mul r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.xyzx)).xyz;
    // 65: add r1.w, -cb0[6].w, l(1.000000)
    r1.w = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: mul r1.w, r1.w, cb0[17].z
    r1.w = ((r1.wwww)*(source[17].zzzz)).w;
    // 67: mul r1.w, r1.w, l(6.283185)
    r1.w = ((r1.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 68: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 69: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 70: mul r2.x, cb0[6].z, l(1.500000)
    r2.x = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 71: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 72: mad r1.w, r1.w, l(0.500000), cb0[6].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 73: frc r2.x, v4.x
    r2.x = (frac(v4.xxxx)).x;
    // 74: mul r2.x, r2.x, l(0.125000)
    r2.x = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 75: mul r3.y, cb0[6].y, cb0[13].y
    r3.y = ((source[6].yyyy)*(source[13].yyyy)).y;
    // 76: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 77: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 78: add r2.xy, r2.xyxx, r3.xyxx
    r2.xy = ((r2.xyxx)+(r3.xyxx)).xy;
    // 79: frc r2.z, cb0[6].x
    r2.z = (frac(source[6].xxxx)).z;
    // 80: add r2.w, -r2.z, cb0[6].x
    r2.w = ((-(r2.zzzz))+(source[6].xxxx)).w;
    // 81: mul r3.z, r2.w, l(0.125000)
    r3.z = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 82: add r2.xy, r2.xyxx, r3.zwzz
    r2.xy = ((r2.xyxx)+(r3.zwzz)).xy;
    // 83: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 84: mul r2.xyw, r1.wwww, r3.xyxz
    r2.xyw = ((r1.wwww)*(r3.xyxz)).xyw;
    // 85: mul r1.w, r2.z, r3.w
    r1.w = ((r2.zzzz)*(r3.wwww)).w;
    // 86: mad r2.xyz, r2.xywx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xywx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 87: mad r1.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 88: add r1.w, r1.y, r1.x
    r1.w = ((r1.yyyy)+(r1.xxxx)).w;
    // 89: add r1.w, r1.z, r1.w
    r1.w = ((r1.zzzz)+(r1.wwww)).w;
    // 90: mul r1.w, r1.w, l(0.333330)
    r1.w = ((r1.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 91: max r1.w, r1.w, cb0[18].x
    r1.w = (max(r1.wwww,source[18].xxxx)).w;
    // 92: min r1.w, r1.w, cb0[17].w
    r1.w = (min(r1.wwww,source[17].wwww)).w;
    // 93: add r2.x, -r1.w, l(1.000000)
    r2.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 94: mad r1.w, r0.x, r2.x, r1.w
    r1.w = ((r0.xxxx)*(r2.xxxx)+(r1.wwww)).w;
    // 95: mul_sat r2.w, r0.x, cb2[3].w
    r2.w = (saturate((r0.xxxx)*(passValues[3].wwww))).w;
    // 96: add r0.x, r1.w, l(-1.000000)
    r0.x = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 97: mad r0.x, cb0[18].z, r0.x, l(1.000000)
    r0.x = ((source[18].zzzz)*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 98: mul r3.xyz, r1.xyzx, r0.xxxx
    r3.xyz = ((r1.xyzx)*(r0.xxxx)).xyz;
    // 99: mul r2.x, r5.x, cb0[17].x
    r2.x = ((r5.xxxx)*(source[17].xxxx)).x;
    // 100: mul r2.y, r5.z, cb0[19].x
    r2.y = ((r5.zzzz)*(source[19].xxxx)).y;
    // 101: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 102: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 103: movc r2.y, r4.z, l(0), r2.y
    r2.y = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).y;
    // 104: max r2.y, r2.y, cb0[0].x
    r2.y = (max(r2.yyyy,source[0].xxxx)).y;
    // 105: min r2.z, r2.y, l(1.000000)
    r2.z = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 106: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 107: movc r2.x, r4.x, l(0), r2.x
    r2.x = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 108: add_sat r2.x, r2.x, cb0[17].y
    r2.x = (saturate((r2.xxxx)+(source[17].yyyy))).x;
    // 109: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 110: mul r4.xyz, r2.yyyy, cb0[12].xyzx
    r4.xyz = ((r2.yyyy)*(source[12].xyzx)).xyz;
    // 111: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 112: mad r1.xyz, r0.xxxx, r1.xyzx, -r3.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 113: mad r1.xyz, r2.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 114: mul_sat r0.x, r1.w, r2.x
    r0.x = (saturate((r1.wwww)*(r2.xxxx))).x;
    // 115: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 116: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 117: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 118: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 119: mad r4.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 120: mad r5.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 121: mad r4.xyz, r0.xxxx, r4.xyzx, r5.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 122: mad r3.xyz, r4.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 123: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 124: max r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = (max(r0.xxxx,r3.xyzx)).xyz;
    // 125: mov_sat r1.w, cb0[18].w
    r1.w = (saturate(source[18].wwww)).w;
    // 126: mad r4.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r4.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 127: mul r2.x, r1.w, l(0.080000)
    r2.x = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 128: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 129: mad r4.xyz, r2.wwww, r4.xyzx, r2.xxxx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xxxx)).xyz;
    // 130: mul_sat r1.w, r4.y, l(50.000000)
    r1.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 131: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 132: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 133: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 134: mul r5.xy, r2.xyxx, cb0[15].xxxx
    r5.xy = ((r2.xyxx)*(source[15].xxxx)).xy;
    // 135: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 136: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 137: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 138: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 139: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 140: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 141: div r5.xyz, r5.xyzx, r2.xxxx
    r5.xyz = ((r5.xyzx)/(r2.xxxx)).xyz;
    // 142: dp3 r2.x, r5.xyzx, r5.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 143: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 144: mul r6.xyz, r2.xxxx, r5.xyzx
    r6.xyz = ((r2.xxxx)*(r5.xyzx)).xyz;
    // 145: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 146: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 147: mul r7.xyz, r2.xxxx, v5.xyzx
    r7.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 148: dp3 r2.x, r6.xyzx, r7.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 149: deriv_rtx_coarse r8.x, r2.x
    r8.x = (ddx_coarse(r2.xxxx)).x;
    // 150: deriv_rty_coarse r8.y, r2.x
    r8.y = (ddy_coarse(r2.xxxx)).y;
    // 151: dp2 r2.y, r8.xyxx, r8.xyxx
    r2.y = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).y;
    // 152: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 153: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 154: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 155: min r8.y, r2.y, l(1.000000)
    r8.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 156: add r2.y, -r8.y, l(1.000000)
    r2.y = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 157: max r9.xyz, r4.xyzx, r2.yyyy
    r9.xyz = (max(r4.xyzx,r2.yyyy)).xyz;
    // 158: add r9.xyz, -r4.xyzx, r9.xyzx
    r9.xyz = ((-(r4.xyzx))+(r9.xyzx)).xyz;
    // 159: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 160: mul r10.xyz, r2.xxxx, r6.xyzx
    r10.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 161: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 162: add r1.w, r10.z, l(1.000000)
    r1.w = ((r10.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 166: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 167: mul r2.x, r2.x, cb0[1].y
    r2.x = ((r2.xxxx)*(source[1].yyyy)).x;
    // 168: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 169: mad_sat r2.x, r2.x, cb0[1].w, cb0[1].z
    r2.x = (saturate((r2.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 170: mul r2.x, r2.x, cb0[19].y
    r2.x = ((r2.xxxx)*(source[19].yyyy)).x;
    // 171: add_sat r8.x, -r1.w, r2.y
    r8.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 172: sample_indexable(texture2d)(float,float,float,float) r2.yz, r8.xyxx, t5.zxyw, s6
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 173: add r1.w, r0.x, r8.x
    r1.w = ((r0.xxxx)+(r8.xxxx)).w;
    // 174: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 175: mul r8.xzw, r2.zzzz, r4.xxyz
    r8.xzw = ((r2.zzzz)*(r4.xxyz)).xzw;
    // 176: mad r8.xzw, r9.xxyz, r2.yyyy, r8.xxzw
    r8.xzw = ((r9.xxyz)*(r2.yyyy)+(r8.xxzw)).xzw;
    // 177: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = r2.z != 0.f ? 1.f / r2.z : 0.f;
    // 178: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 179: mad r9.xyz, r4.xyzx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((r4.xyzx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: dp3 r2.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 181: mad r4.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 182: mad r11.xyz, -r8.xzwx, r9.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((-(r8.xzwx))*(r9.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 183: mul r8.xzw, r8.xxzw, r9.xxyz
    r8.xzw = ((r8.xxzw)*(r9.xxyz)).xzw;
    // 184: mul r9.xyz, r1.xyzx, r11.xyzx
    r9.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 185: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 186: mul r9.xyz, r2.yyyy, r9.xyzx
    r9.xyz = ((r2.yyyy)*(r9.xyzx)).xyz;
    // 187: dp3 r2.z, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 188: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 189: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 190: mul r12.xyz, r3.wwww, v1.xyzx
    r12.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 191: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 192: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 193: mul r13.xyz, r3.wwww, v0.xyzx
    r13.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 194: mul r14.xyz, r12.zxyz, r13.yzxy
    r14.xyz = ((r12.zxyz)*(r13.yzxy)).xyz;
    // 195: mad r14.xyz, r12.yzxy, r13.zxyz, -r14.xyzx
    r14.xyz = ((r12.yzxy)*(r13.zxyz)+(-(r14.xyzx))).xyz;
    // 196: mul r14.xyz, r14.xyzx, v1.wwww
    r14.xyz = ((r14.xyzx)*(v1.wwww)).xyz;
    // 197: dp3 r15.y, r14.xyzx, r6.xyzx
    r15.y = (dot((r14.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 198: dp3 r14.y, r14.xyzx, r10.xyzx
    r14.y = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 199: dp3 r15.x, r13.xyzx, r6.xyzx
    r15.x = (dot((r13.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 200: dp3 r14.x, r13.xyzx, r10.xyzx
    r14.x = (dot((r13.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 201: dp2 r13.z, r15.xyxx, cb0[21].xyxx
    r13.z = (dot((r15.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 202: mul r14.zw, cb0[21].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r14.zw = ((source[21].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 203: dp2 r13.x, r15.xyxx, r14.zwzz
    r13.x = (dot((r15.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 204: dp2 r16.x, r14.xyxx, r14.zwzz
    r16.x = (dot((r14.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 205: dp2 r16.z, r14.xyxx, cb0[21].xyxx
    r16.z = (dot((r14.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 206: dp3 r13.y, r12.xyzx, r6.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 207: dp3 r16.y, r12.xyzx, r10.xyzx
    r16.y = (dot((r12.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 208: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 209: dp4 r12.x, cb0[22].xyzw, r13.xyzw
    r12.x = (dot((source[22].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 210: dp4 r12.y, cb0[23].xyzw, r13.xyzw
    r12.y = (dot((source[23].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 211: dp4 r12.z, cb0[24].xyzw, r13.xyzw
    r12.z = (dot((source[24].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 212: mul r14.xyzw, r13.yzzx, r13.xyzz
    r14.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 213: dp4 r17.x, cb0[25].xyzw, r14.xyzw
    r17.x = (dot((source[25].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 214: dp4 r17.y, cb0[26].xyzw, r14.xyzw
    r17.y = (dot((source[26].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 215: dp4 r17.z, cb0[27].xyzw, r14.xyzw
    r17.z = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 216: add r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)+(r17.xyzx)).xyz;
    // 217: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 218: mov r15.z, r13.y
    r15.z = (r13.yyyy).z;
    // 219: mad r3.w, r13.x, r13.x, -r3.w
    r3.w = ((r13.xxxx)*(r13.xxxx)+(-(r3.wwww))).w;
    // 220: mad r12.xyz, cb0[28].xyzx, r3.wwww, r12.xyzx
    r12.xyz = ((source[28].xyzx)*(r3.wwww)+(r12.xyzx)).xyz;
    // 221: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 222: mul r12.xyz, r12.xyzx, cb0[20].xyzx
    r12.xyz = ((r12.xyzx)*(source[20].xyzx)).xyz;
    // 223: mul r12.xyz, r12.xyzx, cb0[21].zzzz
    r12.xyz = ((r12.xyzx)*(source[21].zzzz)).xyz;
    // 224: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[20].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[20].wwww)).xyz;
    // 225: dp3 r3.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 226: add r12.xyz, -r3.wwww, r12.xyzx
    r12.xyz = ((-(r3.wwww))+(r12.xyzx)).xyz;
    // 227: mad r12.xyz, r12.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r12.xyz = ((r12.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 228: dp3 r3.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: mad r4.w, r8.y, l(2.000000), l(2.000000)
    r4.w = ((r8.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 230: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 231: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 232: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 233: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 234: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 235: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 236: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 237: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 238: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 239: mul r12.xyz, r3.wwww, r12.xyzx
    r12.xyz = ((r3.wwww)*(r12.xyzx)).xyz;
    // 240: mul r9.xyz, r9.xyzx, r12.xyzx
    r9.xyz = ((r9.xyzx)*(r12.xyzx)).xyz;
    // 241: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 242: mul r9.xyz, r3.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 243: mul r3.w, r8.y, l(5.000000)
    r3.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 244: mul r5.w, r8.y, r8.y
    r5.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 245: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 246: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 247: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 248: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 249: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 250: sample_l_indexable(texturecube)(float,float,float,float) r12.xyzw, r16.xyzx, t6.xyzw, s5, r3.w
    r12.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r16.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 251: mul r12.xyz, r12.xyzx, r12.wwww
    r12.xyz = ((r12.xyzx)*(r12.wwww)).xyz;
    // 252: mul r12.xyz, r12.xyzx, cb0[20].xyzx
    r12.xyz = ((r12.xyzx)*(source[20].xyzx)).xyz;
    // 253: mul r12.xyz, r12.xyzx, cb0[21].zzzz
    r12.xyz = ((r12.xyzx)*(source[21].zzzz)).xyz;
    // 254: mad r12.xyz, r12.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[20].wwww
    r12.xyz = ((r12.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[20].wwww)).xyz;
    // 255: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 256: add r12.xyz, -r1.wwww, r12.xyzx
    r12.xyz = ((-(r1.wwww))+(r12.xyzx)).xyz;
    // 257: mad r12.xyz, r12.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r12.xyz = ((r12.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 258: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 259: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 260: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 261: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 262: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 263: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 264: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 265: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 266: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 267: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 268: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 269: mul r13.xyz, r8.xzwx, r12.xyzx
    r13.xyz = ((r8.xzwx)*(r12.xyzx)).xyz;
    // 270: mad r1.w, r0.x, r4.x, r4.y
    r1.w = ((r0.xxxx)*(r4.xxxx)+(r4.yyyy)).w;
    // 271: mad r1.w, r1.w, r0.x, r4.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r4.zzzz)).w;
    // 272: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 273: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 274: mad r4.xyz, r13.xyzx, r0.xxxx, r9.xyzx
    r4.xyz = ((r13.xyzx)*(r0.xxxx)+(r9.xyzx)).xyz;
    // 275: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 276: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 277: mul r9.xyz, r1.wwww, v6.xyzx
    r9.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 278: dp3 r1.w, r9.xyzx, r6.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 279: dp3 r2.z, -r9.xyzx, r6.xyzx
    r2.z = (dot((-(r9.xyzx)).xyz,(r6.xyzx).xyz).xxxx).z;
    // 280: dp3 r3.w, r9.xyzx, r10.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 281: mad r6.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 282: mad r6.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r6.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 283: mul r6.xyzw, r6.xyzw, r6.xyzw
    r6.xyzw = ((r6.xyzw)*(r6.xyzw)).xyzw;
    // 284: mad r9.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 285: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 286: mul r9.yzw, r9.yyyy, cb0[31].xxyz
    r9.yzw = ((r9.yyyy)*(source[31].xxyz)).yzw;
    // 287: mad r9.xyz, r9.xxxx, cb0[30].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[30].xyzx)+(r9.yzwy)).xyz;
    // 288: mul r9.xyz, r9.xyzx, cb0[32].wwww
    r9.xyz = ((r9.xyzx)*(source[32].wwww)).xyz;
    // 289: mul r9.xyz, r1.xyzx, r9.xyzx
    r9.xyz = ((r1.xyzx)*(r9.xyzx)).xyz;
    // 290: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 291: mul r3.xyz, r3.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 292: mul r3.xyz, r11.xyzx, r3.xyzx
    r3.xyz = ((r11.xyzx)*(r3.xyzx)).xyz;
    // 293: mad r3.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 294: mad r3.xyz, r4.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r3.xyzx)).xyz;
    // 295: mul r4.xyz, r6.yyyy, cb0[31].xyzx
    r4.xyz = ((r6.yyyy)*(source[31].xyzx)).xyz;
    // 296: mad r4.xyz, cb0[30].xyzx, r6.xxxx, r4.xyzx
    r4.xyz = ((source[30].xyzx)*(r6.xxxx)+(r4.xyzx)).xyz;
    // 297: mul r4.xyz, r4.xyzx, cb0[32].wwww
    r4.xyz = ((r4.xyzx)*(source[32].wwww)).xyz;
    // 298: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 299: mul r4.xyz, r12.xyzx, r4.xyzx
    r4.xyz = ((r12.xyzx)*(r4.xyzx)).xyz;
    // 300: mul r4.xyz, r4.xyzx, r8.xzwx
    r4.xyz = ((r4.xyzx)*(r8.xzwx)).xyz;
    // 301: mad r3.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r3.xyzx)).xyz;
    // 302: mul r4.xyz, r4.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 303: dp3 o4.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 304: dp3 r0.x, r5.xyzx, r7.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 305: mul_sat r1.w, r0.x, cb0[16].y
    r1.w = (saturate((r0.xxxx)*(source[16].yyyy))).w;
    // 306: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 307: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 308: mul_sat r2.z, r7.z, cb0[16].y
    r2.z = (saturate((r7.zzzz)*(source[16].yyyy))).z;
    // 309: add r2.w, -|r7.z|, l(1.000000)
    r2.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 310: mul r0.x, r0.x, r2.w
    r0.x = ((r0.xxxx)*(r2.wwww)).x;
    // 311: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 312: add_sat r2.z, r2.z, -cb0[16].z
    r2.z = (saturate((r2.zzzz)+(-(source[16].zzzz)))).z;
    // 313: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 314: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 315: mul r2.w, r2.w, cb0[16].w
    r2.w = ((r2.wwww)*(source[16].wwww)).w;
    // 316: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 317: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 318: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 319: mad r4.xyz, r1.wwww, cb0[10].xyzx, -cb0[10].xyzx
    r4.xyz = ((r1.wwww)*(source[10].xyzx)+(-(source[10].xyzx))).xyz;
    // 320: mul r1.w, r1.w, cb0[9].w
    r1.w = ((r1.wwww)*(source[9].wwww)).w;
    // 321: mad r4.xyz, cb0[10].wwww, r4.xyzx, cb0[10].xyzx
    r4.xyz = ((source[10].wwww)*(r4.xyzx)+(source[10].xyzx)).xyz;
    // 322: mad r4.xyz, r1.wwww, cb0[9].xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(source[9].xyzx)+(r4.xyzx)).xyz;
    // 323: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 324: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 325: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 326: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 327: mul r5.xyz, r1.wwww, cb0[11].xyzx
    r5.xyz = ((r1.wwww)*(source[11].xyzx)).xyz;
    // 328: movc r5.xyz, r0.xxxx, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 329: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 330: mad r0.xyz, cb0[15].zzzz, r0.yzwy, r4.xyzx
    r0.xyz = ((source[15].zzzz)*(r0.yzwy)+(r4.xyzx)).xyz;
    // 331: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 332: mul r4.xyz, r6.wwww, cb0[31].xyzx
    r4.xyz = ((r6.wwww)*(source[31].xyzx)).xyz;
    // 333: mad r4.xyz, r6.zzzz, cb0[30].xyzx, r4.xyzx
    r4.xyz = ((r6.zzzz)*(source[30].xyzx)+(r4.xyzx)).xyz;
    // 334: mul r4.xyz, r4.xyzx, cb0[32].wwww
    r4.xyz = ((r4.xyzx)*(source[32].wwww)).xyz;
    // 335: mul_sat r5.xyz, cb0[14].xyzx, cb0[14].wwww
    r5.xyz = (saturate((source[14].xyzx)*(source[14].wwww))).xyz;
    // 336: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 337: mul r5.xyz, r5.xyzx, cb0[19].yyyy
    r5.xyz = ((r5.xyzx)*(source[19].yyyy)).xyz;
    // 338: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 339: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 340: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 341: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 342: mad r0.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 343: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 344: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 345: mad o0.xyz, r1.xyzx, cb0[32].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[32].xyzx)+(r0.xyzx)).xyz;
    // 346: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 347: dp3 r0.x, r15.xyzx, r15.xyzx
    r0.x = (dot((r15.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 348: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 349: mul r0.xyz, r0.xxxx, r15.xyzx
    r0.xyz = ((r0.xxxx)*(r15.xyzx)).xyz;
    // 350: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 351: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 352: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 353: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 354: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 355: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 356: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 357: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 358: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 359: ftou r0.x, cb0[29].z
    r0.x = (asfloat((uint4)(source[29].zzzz))).x;
    // 360: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 361: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 362: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 363: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 364: ret
    return output;
}

// source.character.selection-native-212.v1 / source program 9859dedeace20041ba271c94538f478a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase212(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    // Original engine primitive opacity/environment rows.
    source[0].xy = 1.f;
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

// source.character.selection-native-213.v1 / source program 44672672c5779b49bbaf371a34212e54
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase213(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].z=(g_SourceCharacterTime.xxxx).x;
    source[22].w=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    // Original engine primitive opacity/environment rows.
    source[0].x = 1.f;
    source[1].w = 1.f;
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


// source.character.selection-native-235.v1 / source program 350d50f47aaa5943b8544d1085614d22
