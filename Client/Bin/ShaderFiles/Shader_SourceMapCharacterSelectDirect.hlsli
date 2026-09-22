// source.map.emissive-reflection.v1 / source program 22d716f6e56f444caff55e3ee66c46ca
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect209(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[5]=float4(input.lightColor,1.0);
    source[6].x=1.0;
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[6]=0.f; source[11]=0.f;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.zxyz
    r0.xyz = ((r0.xxxx)*(v1.zxyz)).xyz;
    // 4: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 7: mul r2.xyz, r0.xyzx, r1.yzxy
    r2.xyz = ((r0.xyzx)*(r1.yzxy)).xyz;
    // 8: mad r0.xyz, r0.zxyz, r1.zxyz, -r2.xyzx
    r0.xyz = ((r0.zxyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r0.xyz, r0.xyzx, v1.wwww
    r0.xyz = ((r0.xyzx)*(v1.wwww)).xyz;
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r0.w, r0.w, v5.z
    r0.w = ((r0.wwww)*(v5.zzzz)).w;
    // 16: mad r2.xyz, r2.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r2.xyzx
    r2.xyz = ((r2.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 17: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].y
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].yyyy)) * 0xffffffffu)).w;
    // 18: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 19: mul r3.xyzw, v8.yyyy, cb0[7].xyzw
    r3.xyzw = ((v8.yyyy)*(source[7].xyzw)).xyzw;
    // 20: mad r3.xyzw, cb0[6].xyzw, v8.xxxx, r3.xyzw
    r3.xyzw = ((source[6].xyzw)*(v8.xxxx)+(r3.xyzw)).xyzw;
    // 21: mad r3.xyzw, cb0[8].xyzw, v8.zzzz, r3.xyzw
    r3.xyzw = ((source[8].xyzw)*(v8.zzzz)+(r3.xyzw)).xyzw;
    // 22: mad r3.xyzw, cb0[9].xyzw, v8.wwww, r3.xyzw
    r3.xyzw = ((source[9].xyzw)*(v8.wwww)+(r3.xyzw)).xyzw;
    // 23: div r3.xy, r3.xyxx, r3.wwww
    r3.xy = ((r3.xyxx)/(r3.wwww)).xy;
    // 24: sample_indexable(texture2d)(float,float,float,float) r4.x, r3.xyxx, t0.xyzw, s2
    r4.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 25: mov r5.xw, l(0,0,0,0)
    r5.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 26: mov r5.yz, cb0[10].wwzw
    r5.yz = (source[10].wwzw).yz;
    // 27: add r5.xyzw, r3.xyxy, r5.xyzw
    r5.xyzw = ((r3.xyxy)+(r5.xyzw)).xyzw;
    // 28: sample_indexable(texture2d)(float,float,float,float) r4.y, r5.xyxx, t0.yxzw, s2
    r4.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 29: sample_indexable(texture2d)(float,float,float,float) r4.z, r5.zwzz, t0.yzxw, s2
    r4.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 30: add r5.xy, r3.xyxx, cb0[10].zwzz
    r5.xy = ((r3.xyxx)+(source[10].zwzz)).xy;
    // 31: sample_indexable(texture2d)(float,float,float,float) r4.w, r5.xyxx, t0.yzwx, s2
    r4.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 32: lt r4.xyzw, r3.zzzz, r4.xyzw
    r4.xyzw = (asfloat((uint4)((r3.zzzz)<(r4.xyzw)) * 0xffffffffu)).xyzw;
    // 33: and r5.xyzw, r4.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r5.xyzw = (asfloat(asuint(r4.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 34: mul r3.xy, r3.xyxx, cb0[10].xyxx
    r3.xy = ((r3.xyxx)*(source[10].xyxx)).xy;
    // 35: frc r3.xy, r3.xyxx
    r3.xy = (frac(r3.xyxx)).xy;
    // 36: movc r3.zw, r4.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r3.zw = ((asuint(r4.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 37: add r3.zw, r3.zzzw, r5.zzzw
    r3.zw = ((r3.zzzw)+(r5.zzzw)).zw;
    // 38: mad r3.xz, r3.xxxx, r3.zzwz, r5.xxyx
    r3.xz = ((r3.xxxx)*(r3.zzwz)+(r5.xxyx)).xz;
    // 39: add r1.w, -r3.x, r3.z
    r1.w = ((-(r3.xxxx))+(r3.zzzz)).w;
    // 40: mad r1.w, r3.y, r1.w, r3.x
    r1.w = ((r3.yyyy)*(r1.wwww)+(r3.xxxx)).w;
    // 41: mul r3.xyz, r1.wwww, cb0[11].xxxx
    r3.xyz = ((r1.wwww)*(source[11].xxxx)).xyz;
    // 42: else
    } else {
    // 43: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 44: endif
    }
    // 45: dp3 r1.x, r1.xyzx, r2.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 46: dp3 r1.y, r0.xyzx, r2.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 47: mul r0.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 48: mad r0.xy, cb0[3].yyyy, r1.xyxx, r0.xyxx
    r0.xy = ((source[3].yyyy)*(r1.xyxx)+(r0.xyxx)).xy;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t1.xyzw, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 50: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 51: mad r0.xyz, cb0[3].zzzz, r0.xyzx, r0.xyzx
    r0.xyz = ((source[3].zzzz)*(r0.xyzx)+(r0.xyzx)).xyz;
    // 52: add r0.xyz, r0.xyzx, -cb0[3].zzzz
    r0.xyz = ((r0.xyzx)+(-(source[3].zzzz))).xyz;
    // 53: mov_sat r1.xyz, r0.xyzx
    r1.xyz = (saturate(r0.xyzx)).xyz;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 55: mul r4.xyz, cb0[2].xyzx, cb0[4].xxxx
    r4.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // 56: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 57: mad r1.xyz, cb0[3].wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((source[3].wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 58: mov_sat r0.xyz, -r0.xyzx
    r0.xyz = (saturate(-(r0.xyzx))).xyz;
    // 59: mad r0.xyz, -cb0[3].wwww, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(source[3].wwww))*(r0.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 60: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 61: max r0.xyzw, r0.xyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyzw = (max(r0.xyzw,float4(0.000000,0.000000,0.000000,0.000000))).xyzw;
    // 62: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 63: min r1.x, r0.w, l(1.000000)
    r1.x = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 64: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 65: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 66: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 67: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 68: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 69: mul_sat r0.x, r2.w, cb0[4].y
    r0.x = (saturate((r2.wwww)*(source[4].yyyy))).x;
    // 70: mul o0.w, r0.x, cb0[0].w
    output.targets[0].w = ((r0.xxxx)*(source[0].wwww)).w;
    // 71: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 72: ret
    return output;
}
