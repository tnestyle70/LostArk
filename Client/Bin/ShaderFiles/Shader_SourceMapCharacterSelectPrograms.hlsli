// source.map.emissive-reflection.v1 / source program ab7d8da0dfb75842a3686f894374a7fe
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapForward209(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[8]=0.f; source[9]=0.f; source[10]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 3: mul r1.xyz, cb0[3].xyzx, cb0[6].xxxx
    r1.xyz = ((source[3].xyzx)*(source[6].xxxx)).xyz;
    // 4: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 5: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 8: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 10: mul r1.yzw, r1.yyyy, cb0[9].xxyz
    r1.yzw = ((r1.yyyy)*(source[9].xxyz)).yzw;
    // 11: mad r1.xyz, r1.xxxx, cb0[8].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[8].xyzx)+(r1.yzwy)).xyz;
    // 12: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 13: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 16: mad r2.xyz, r2.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r2.xyzx
    r2.xyz = ((r2.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 17: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 18: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 19: mul r3.xyz, r0.wwww, v1.xyzx
    r3.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 20: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r4.xyz, r0.wwww, v0.xyzx
    r4.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 23: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 24: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 25: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 26: dp3 r5.y, r5.xyzx, r2.xyzx
    r5.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 27: dp3 r5.x, r4.xyzx, r2.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 28: mov r3.x, r4.z
    r3.x = (r4.zzzz).x;
    // 29: mov r3.y, r5.z
    r3.y = (r5.zzzz).y;
    // 30: mul r2.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r2.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 31: mad r2.xy, cb0[6].yyyy, r5.xyxx, r2.xyxx
    r2.xy = ((source[6].yyyy)*(r5.xyxx)+(r2.xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 33: mul r2.xyz, r2.xyzx, cb0[4].xyzx
    r2.xyz = ((r2.xyzx)*(source[4].xyzx)).xyz;
    // 34: mad r2.xyz, cb0[6].zzzz, r2.xyzx, r2.xyzx
    r2.xyz = ((source[6].zzzz)*(r2.xyzx)+(r2.xyzx)).xyz;
    // 35: add r2.xyz, r2.xyzx, -cb0[6].zzzz
    r2.xyz = ((r2.xyzx)+(-(source[6].zzzz))).xyz;
    // 36: mov_sat r4.xyz, -r2.xyzx
    r4.xyz = (saturate(-(r2.xyzx))).xyz;
    // 37: mov_sat r2.xyz, r2.xyzx
    r2.xyz = (saturate(r2.xyzx)).xyz;
    // 38: mad r4.xyz, -cb0[6].wwww, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[6].wwww))*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 39: mul r5.xyz, cb0[5].xyzx, cb0[7].xxxx
    r5.xyz = ((source[5].xyzx)*(source[7].xxxx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t0.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 42: mul_sat r0.w, r6.w, cb0[7].y
    r0.w = (saturate((r6.wwww)*(source[7].yyyy))).w;
    // 43: mul o0.w, r0.w, cb0[0].w
    output.targets[0].w = ((r0.wwww)*(source[0].wwww)).w;
    // 44: mad r2.xyz, cb0[6].wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((source[6].wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 45: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 46: max r2.xyz, r2.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 47: min r2.xyz, r2.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 48: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 49: mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 50: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 51: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 52: mad r0.xyz, r2.xyzx, cb0[10].xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(source[10].xyzx)+(r0.xyzx)).xyz;
    // 53: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 54: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 55: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 56: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 57: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 58: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 59: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 60: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 61: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 62: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 63: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 64: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 65: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 66: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 67: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 68: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 69: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 70: ret
    return output;
}

// source.map.emissive-reflection.v1 / source program e9ecb7147d1fc049a867d159d41fa341
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapForward209Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[8]=0.f;source[9]=0.f;source[10]=0.f;
    source[11]=g_LightmapAverageScale;source[12]=g_LightmapDirectionalScale;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 3: mul r1.xyz, cb0[3].xyzx, cb0[6].xxxx
    r1.xyz = ((source[3].xyzx)*(source[6].xxxx)).xyz;
    // 4: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 5: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 8: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 9: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 10: mul r1.yzw, r1.yyyy, cb0[9].xxyz
    r1.yzw = ((r1.yyyy)*(source[9].xxyz)).yzw;
    // 11: mad r1.xyz, r1.xxxx, cb0[8].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[8].xyzx)+(r1.yzwy)).xyz;
    // 12: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 13: mul r2.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r2.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 14: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 15: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 16: mul r3.xyz, r0.wwww, v1.xyzx
    r3.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 17: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 18: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 19: mul r4.xyz, r0.wwww, v0.xyzx
    r4.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 20: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 21: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 22: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 23: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 24: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 25: mul r6.xyz, r0.wwww, v6.xyzx
    r6.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 26: mad r6.xyz, r6.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r6.xyzx
    r6.xyz = ((r6.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r6.xyzx))).xyz;
    // 27: dp3 r5.y, r5.xyzx, r6.xyzx
    r5.y = (dot((r5.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 28: mov r3.y, r5.z
    r3.y = (r5.zzzz).y;
    // 29: dp3 r5.x, r4.xyzx, r6.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 30: mov r3.x, r4.z
    r3.x = (r4.zzzz).x;
    // 31: mad r2.xy, cb0[6].yyyy, r5.xyxx, r2.xyxx
    r2.xy = ((source[6].yyyy)*(r5.xyxx)+(r2.xyxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 33: mul r2.xyz, r2.xyzx, cb0[4].xyzx
    r2.xyz = ((r2.xyzx)*(source[4].xyzx)).xyz;
    // 34: mad r2.xyz, cb0[6].zzzz, r2.xyzx, r2.xyzx
    r2.xyz = ((source[6].zzzz)*(r2.xyzx)+(r2.xyzx)).xyz;
    // 35: add r2.xyz, r2.xyzx, -cb0[6].zzzz
    r2.xyz = ((r2.xyzx)+(-(source[6].zzzz))).xyz;
    // 36: mov_sat r4.xyz, r2.xyzx
    r4.xyz = (saturate(r2.xyzx)).xyz;
    // 37: mov_sat r2.xyz, -r2.xyzx
    r2.xyz = (saturate(-(r2.xyzx))).xyz;
    // 38: mad r2.xyz, -cb0[6].wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[6].wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 39: mul r5.xyz, cb0[5].xyzx, cb0[7].xxxx
    r5.xyz = ((source[5].xyzx)*(source[7].xxxx)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t0.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 42: mul_sat r0.w, r7.w, cb0[7].y
    r0.w = (saturate((r7.wwww)*(source[7].yyyy))).w;
    // 43: mul o0.w, r0.w, cb0[0].w
    output.targets[0].w = ((r0.wwww)*(source[0].wwww)).w;
    // 44: mad r4.xyz, cb0[6].wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((source[6].wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 45: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 46: max r2.xyz, r2.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 47: min r2.xyz, r2.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 48: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 49: mul r4.xyz, r1.xyzx, r2.xyzx
    r4.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 50: sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t4.xyzw, s3
    r5.xyz = ((g_BakedDirectionalTexture.Sample(SourceCharacterLookupSampler, (v3.zwzz).xy)).xyzw).xyz;
    // 51: mul r5.xyz, r5.xyzx, cb0[12].xyzx
    r5.xyz = ((r5.xyzx)*(source[12].xyzx)).xyz;
    // 52: dp3 r0.w, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // 53: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t3.xyzw, s3
    r7.xyz = ((g_BakedAverageTexture.Sample(SourceCharacterLookupSampler, (v3.zwzz).xy)).xyzw).xyz;
    // 54: mul r7.xyz, r7.xyzx, cb0[11].xyzx
    r7.xyz = ((r7.xyzx)*(source[11].xyzx)).xyz;
    // 55: mul r8.xyz, r0.wwww, r7.xyzx
    r8.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 56: mad r1.xyz, r7.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r7.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 57: mul r7.xyz, r7.xyzx, cb2[4].xyzx
    r7.xyz = ((r7.xyzx)*(passValues[4].xyzx)).xyz;
    // 58: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 59: div r1.xyz, r8.xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)/(r1.xyzx)).xyz;
    // 60: mad r4.xyz, r2.xyzx, r8.xyzx, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 61: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: dp2_sat r1.x, r6.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r1.x = (saturate(dot((r6.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 63: dp3_sat r1.y, r6.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r1.y = (saturate(dot((r6.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 64: dp3_sat r1.z, r6.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r1.z = (saturate(dot((r6.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 65: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 66: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 67: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 68: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 69: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 70: mad r1.yzw, r7.xxyz, r1.xxxx, r4.xxyz
    r1.yzw = ((r7.xxyz)*(r1.xxxx)+(r4.xxyz)).yzw;
    // 71: mul r4.xyz, r1.xxxx, r7.xyzx
    r4.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // 72: dp3 o4.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 73: add r0.xyz, r0.xyzx, r1.yzwy
    r0.xyz = ((r0.xyzx)+(r1.yzwy)).xyz;
    // 74: mad r0.xyz, r2.xyzx, cb0[10].xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(source[10].xyzx)+(r0.xyzx)).xyz;
    // 75: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 76: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 77: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 78: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 79: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 80: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 81: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 82: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 83: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 84: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 85: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 86: movc r0.xy, r1.xxxx, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 87: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 88: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 89: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 90: mul o4.z, r0.w, r1.y
    output.targets[4].z = ((r0.wwww)*(r1.yyyy)).z;
    // 91: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 92: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 93: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 94: ret
    return output;
}
