#ifndef SOURCE_CHARACTER_BASE_DISPATCH_CASES
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1104(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[12]=g_SourceCharacterBaseConstants[11];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    source[26]=1.f;
    source[27]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
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
    // 10: mul r3.xy, v4.xyxx, cb0[2].xyxx
    r3.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t0.xywz, s0, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 12: mad r3.zw, r4.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r3.zw = ((r4.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 13: mul r0.w, r4.z, cb0[10].y
    r0.w = ((r4.zzzz)*(source[10].yyyy)).w;
    // 14: dp2 r1.w, r3.zwzz, r3.zwzz
    r1.w = (dot((r3.zwzz).xy,(r3.zwzz).xy).xxxx).w;
    // 15: mul r3.zw, r3.zzzw, cb0[8].wwww
    r3.zw = ((r3.zzzw)*(source[8].wwww)).zw;
    // 16: mul r4.xy, r3.zwzz, v2.wwww
    r4.xy = ((r3.zwzz)*(v2.wwww)).xy;
    // 17: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 19: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 20: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 21: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 24: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 25: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 26: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 27: dp3 r6.y, r2.xyzx, r5.xyzx
    r6.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 28: dp3 r6.x, r1.xyzx, r5.xyzx
    r6.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 29: dp2 r7.z, r6.xyxx, cb0[14].xyxx
    r7.z = (dot((r6.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 30: mul r3.zw, cb0[14].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r3.zw = ((source[14].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 31: dp2 r7.x, r6.xyxx, r3.zwzz
    r7.x = (dot((r6.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 32: dp3 r7.y, r0.xyzx, r5.xyzx
    r7.y = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 33: mov r7.w, l(1.000000)
    r7.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 34: dp4 r8.x, cb0[15].xyzw, r7.xyzw
    r8.x = (dot((source[15].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).x;
    // 35: dp4 r8.y, cb0[16].xyzw, r7.xyzw
    r8.y = (dot((source[16].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).y;
    // 36: dp4 r8.z, cb0[17].xyzw, r7.xyzw
    r8.z = (dot((source[17].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).z;
    // 37: mul r9.xyzw, r7.yzzx, r7.xyzz
    r9.xyzw = ((r7.yzzx)*(r7.xyzz)).xyzw;
    // 38: dp4 r10.x, cb0[18].xyzw, r9.xyzw
    r10.x = (dot((source[18].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 39: dp4 r10.y, cb0[19].xyzw, r9.xyzw
    r10.y = (dot((source[19].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 40: dp4 r10.z, cb0[20].xyzw, r9.xyzw
    r10.z = (dot((source[20].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 41: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 42: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 43: mov r6.z, r7.y
    r6.z = (r7.yyyy).z;
    // 44: mad r1.w, r7.x, r7.x, -r1.w
    r1.w = ((r7.xxxx)*(r7.xxxx)+(-(r1.wwww))).w;
    // 45: mad r7.xyz, cb0[21].xyzx, r1.wwww, r8.xyzx
    r7.xyz = ((source[21].xyzx)*(r1.wwww)+(r8.xyzx)).xyz;
    // 46: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 47: mul r7.xyz, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((r7.xyzx)*(source[13].xyzx)).xyz;
    // 48: mad r7.xyz, r7.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r7.xyz = ((r7.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, r3.xyxx, t3.xyzw, s4, l(0.000000)
    r8.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, r3.xyxx, t1.xyzw, s2, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: mul r1.w, r8.z, cb0[11].x
    r1.w = ((r8.zzzz)*(source[11].xxxx)).w;
    // 52: mul r3.xy, r8.yxyy, cb0[12].xzxx
    r3.xy = ((r8.yxyy)*(source[12].xzxx)).xy;
    // 53: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 54: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: mul r2.w, r2.w, cb0[11].y
    r2.w = ((r2.wwww)*(source[11].yyyy)).w;
    // 56: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 57: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 58: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: mul_sat r8.w, r1.w, cb2[3].w
    r8.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 60: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r10.xyz, -r9.xyzx, r1.wwww
    r10.xyz = ((-(r9.xyzx))+(r1.wwww)).xyz;
    // 62: mad r9.xyz, cb0[9].zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((source[9].zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 63: mul r10.xyz, cb0[6].xyzx, cb0[9].wwww
    r10.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 64: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 65: mul r8.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r8.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 66: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 67: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 68: mul r10.xyz, r1.wwww, v5.xyzx
    r10.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 69: dp3 r1.w, r5.xyzx, r10.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 70: mul r11.xyz, r1.wwww, r5.xyzx
    r11.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 71: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 72: dp3 r2.y, r2.xyzx, r11.xyzx
    r2.y = (dot((r2.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 73: dp3 r2.x, r1.xyzx, r11.xyzx
    r2.x = (dot((r1.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 74: mad r1.xy, cb0[10].xxxx, r2.xyxx, r8.xyxx
    r1.xy = ((source[10].xxxx)*(r2.xyxx)+(r8.xyxx)).xy;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 76: mad r1.xyz, r1.xyzx, cb0[7].xyzx, -r9.xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)+(-(r9.xyzx))).xyz;
    // 77: mad r1.xyz, r0.wwww, r1.xyzx, r9.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r9.xyzx)).xyz;
    // 78: mul r9.xyz, r1.xyzx, cb0[10].zzzz
    r9.xyz = ((r1.xyzx)*(source[10].zzzz)).xyz;
    // 79: mad r1.xyz, cb0[10].wwww, r1.xyzx, -r9.xyzx
    r1.xyz = ((source[10].wwww)*(r1.xyzx)+(-(r9.xyzx))).xyz;
    // 80: mad r1.xyz, r2.wwww, r1.xyzx, r9.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r9.xyzx)).xyz;
    // 81: add r9.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 82: mul r1.xyz, r1.xyzx, r9.xyzx
    r1.xyz = ((r1.xyzx)*(r9.xyzx)).xyz;
    // 83: mad_sat r9.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r9.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 84: mov_sat r9.w, cb0[11].z
    r9.w = (saturate(source[11].zzzz)).w;
    // 85: mad r1.xyz, -r9.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r9.xyzx
    r1.xyz = ((-(r9.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r9.xyzx)).xyz;
    // 86: mul r0.w, r9.w, l(0.080000)
    r0.w = ((r9.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 87: mov o3.xyzw, r9.xyzw
    output.targets[3].xyzw = (r9.xyzw).xyzw;
    // 88: mad r1.xyz, r8.wwww, r1.xyzx, r0.wwww
    r1.xyz = ((r8.wwww)*(r1.xyzx)+(r0.wwww)).xyz;
    // 89: log r2.zw, |r3.xxxy|
    r2.zw = (log2(abs(r3.xxxy))).zw;
    // 90: lt r3.xy, |r3.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r3.xy = (asfloat((uint4)((abs(r3.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 91: mul r2.zw, r2.zzzw, cb0[12].yyyw
    r2.zw = ((r2.zzzw)*(source[12].yyyw)).zw;
    // 92: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 93: movc r0.w, r3.x, l(0), r2.z
    r0.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 94: min r2.z, r2.w, l(1.000000)
    r2.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 95: movc r2.z, r3.y, l(0), r2.z
    r2.z = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).z;
    // 96: max r0.w, r0.w, cb0[0].w
    r0.w = (max(r0.wwww,source[0].wwww)).w;
    // 97: min r8.z, r0.w, l(1.000000)
    r8.z = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 98: deriv_rtx_coarse r3.x, r1.w
    r3.x = (ddx_coarse(r1.wwww)).x;
    // 99: deriv_rty_coarse r3.y, r1.w
    r3.y = (ddy_coarse(r1.wwww)).y;
    // 100: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 101: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 102: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 103: mad_sat r3.y, r1.w, l(0.300000), r8.z
    r3.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 104: add r1.w, -r3.y, l(1.000000)
    r1.w = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: max r12.xyz, r1.xyzx, r1.wwww
    r12.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 106: add r12.xyz, -r1.xyzx, r12.xyzx
    r12.xyz = ((-(r1.xyzx))+(r12.xyzx)).xyz;
    // 107: mul_sat r1.w, r1.y, l(50.000000)
    r1.w = (saturate((r1.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 108: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 109: add r1.w, r11.z, l(1.000000)
    r1.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: add_sat r3.x, r0.w, -r1.w
    r3.x = (saturate((r0.wwww)+(-(r1.wwww)))).x;
    // 112: sample_indexable(texture2d)(float,float,float,float) r8.xy, r3.xyxx, t5.xyzw, s7
    r8.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 113: add r0.w, r2.z, r3.x
    r0.w = ((r2.zzzz)+(r3.xxxx)).w;
    // 114: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 115: mul r13.xyz, r1.xyzx, r8.yyyy
    r13.xyz = ((r1.xyzx)*(r8.yyyy)).xyz;
    // 116: mad r12.xyz, r12.xyzx, r8.xxxx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r8.xxxx)+(r13.xyzx)).xyz;
    // 117: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r8.y
    r1.w = r8.y != 0.f ? 1.f / r8.y : 0.f;
    // 118: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 119: mad r13.xyz, r1.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r1.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 120: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: mad r1.xyz, r1.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r1.xyz = ((r1.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 122: mad r14.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 123: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 124: mul r7.xyz, r7.xyzx, r14.xyzx
    r7.xyz = ((r7.xyzx)*(r14.xyzx)).xyz;
    // 125: dp2_sat r13.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r13.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 126: dp3_sat r13.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r13.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 127: dp3_sat r13.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r13.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 128: mul r13.xyz, r13.xyzx, r13.xyzx
    r13.xyz = ((r13.xyzx)*(r13.xyzx)).xyz;
    // 129: sample_indexable(texture2d)(float,float,float,float) r14.xyz, v3.zwzz, t8.xyzw, s5
    r14.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 130: mul r14.xyz, r14.xyzx, cb0[27].xyzx
    r14.xyz = ((r14.xyzx)*(source[27].xyzx)).xyz;
    // 131: dp3 r1.w, r14.xyzx, r13.xyzx
    r1.w = (dot((r14.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 132: sample_indexable(texture2d)(float,float,float,float) r13.xyz, v3.zwzz, t7.xyzw, s5
    r13.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 133: mul r13.xyz, r13.xyzx, cb0[26].xyzx
    r13.xyz = ((r13.xyzx)*(source[26].xyzx)).xyz;
    // 134: mul r15.xyz, r1.wwww, r13.xyzx
    r15.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 135: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 136: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 137: mul r16.xyz, r2.wwww, v6.xyzx
    r16.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 138: dp3 r2.w, r16.xyzx, r5.xyzx
    r2.w = (dot((r16.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 139: mad r5.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 140: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 141: mul r5.yzw, r5.yyyy, cb0[24].xxyz
    r5.yzw = ((r5.yyyy)*(source[24].xxyz)).yzw;
    // 142: mad r5.xyz, r5.xxxx, cb0[23].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[23].xyzx)+(r5.yzwy)).xyz;
    // 143: mul r5.xyz, r5.xyzx, cb0[25].wwww
    r5.xyz = ((r5.xyzx)*(source[25].wwww)).xyz;
    // 144: mul r16.xyz, r9.xyzx, r5.xyzx
    r16.xyz = ((r9.xyzx)*(r5.xyzx)).xyz;
    // 145: mad r15.xyz, r9.xyzx, r15.xyzx, r16.xyzx
    r15.xyz = ((r9.xyzx)*(r15.xyzx)+(r16.xyzx)).xyz;
    // 146: mad r16.xyz, r9.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r16.xyz = ((r9.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 147: mad r17.xyz, r9.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r17.xyz = ((r9.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 148: mad r16.xyz, r2.zzzz, r16.xyzx, r17.xyzx
    r16.xyz = ((r2.zzzz)*(r16.xyzx)+(r17.xyzx)).xyz;
    // 149: mad r17.xyz, r9.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r17.xyz = ((r9.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 150: mad r16.xyz, r16.xyzx, r2.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r2.zzzz)+(r17.xyzx)).xyz;
    // 151: mul r16.xyz, r2.zzzz, r16.xyzx
    r16.xyz = ((r2.zzzz)*(r16.xyzx)).xyz;
    // 152: max r16.xyz, r2.zzzz, r16.xyzx
    r16.xyz = (max(r2.zzzz,r16.xyzx)).xyz;
    // 153: mul r15.xyz, r15.xyzx, r16.xyzx
    r15.xyz = ((r15.xyzx)*(r16.xyzx)).xyz;
    // 154: mul r7.xyz, r7.xyzx, r15.xyzx
    r7.xyz = ((r7.xyzx)*(r15.xyzx)).xyz;
    // 155: mad r7.xyz, -r7.xyzx, r8.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r8.wwww)+(r7.xyzx)).xyz;
    // 156: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 157: mul r2.w, r3.y, l(5.000000)
    r2.w = ((r3.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 158: mul r3.x, r3.y, r3.y
    r3.x = ((r3.yyyy)*(r3.yyyy)).x;
    // 159: mul r0.w, r0.w, r3.x
    r0.w = ((r0.wwww)*(r3.xxxx)).w;
    // 160: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 161: add r0.w, r2.z, r0.w
    r0.w = ((r2.zzzz)+(r0.wwww)).w;
    // 162: mov o5.y, r2.z
    output.targets[5].y = (r2.zzzz).y;
    // 163: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 164: dp3 r0.y, r0.xyzx, r11.xyzx
    r0.y = (dot((r0.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 165: dp2 r0.x, r2.xyxx, r3.zwzz
    r0.x = (dot((r2.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 166: dp2 r0.z, r2.xyxx, cb0[14].xyxx
    r0.z = (dot((r2.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 167: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r0.xyzx, t6.xyzw, s6, r2.w
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r0.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 168: mul r0.xyz, r2.xyzx, r2.wwww
    r0.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 169: mul r0.xyz, r0.xyzx, cb0[13].xyzx
    r0.xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 170: mad r0.xyz, r0.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r0.xyz = ((r0.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 171: dp2_sat r2.x, r11.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r11.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 172: dp3_sat r2.y, r11.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r11.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 173: dp3_sat r2.z, r11.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r11.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 174: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 175: dp3 r2.x, r14.xyzx, r2.xyzx
    r2.x = (dot((r14.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 176: add r1.w, r1.w, -r2.x
    r1.w = ((r1.wwww)+(-(r2.xxxx))).w;
    // 177: mad r1.w, r8.z, r1.w, r2.x
    r1.w = ((r8.zzzz)*(r1.wwww)+(r2.xxxx)).w;
    // 178: mad r2.xyz, r13.xyzx, r1.wwww, r5.xyzx
    r2.xyz = ((r13.xyzx)*(r1.wwww)+(r5.xyzx)).xyz;
    // 179: mul r3.xyz, r1.wwww, r13.xyzx
    r3.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 180: mad r1.x, r0.w, r1.x, r1.y
    r1.x = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).x;
    // 181: mad r1.x, r1.x, r0.w, r1.z
    r1.x = ((r1.xxxx)*(r0.wwww)+(r1.zzzz)).x;
    // 182: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 183: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 184: mul r1.xyz, r0.wwww, r2.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 185: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 186: div r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)/(r2.xyzx)).xyz;
    // 187: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 188: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 189: mad r1.xyz, r0.xyzx, r12.xyzx, r7.xyzx
    r1.xyz = ((r0.xyzx)*(r12.xyzx)+(r7.xyzx)).xyz;
    // 190: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 191: dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 192: dp3 r0.x, r4.xyzx, r10.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 193: add r0.y, -|r10.z|, l(1.000000)
    r0.y = ((-(abs(r10.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 194: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 195: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 196: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 197: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 198: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 199: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 200: mul r2.xyz, r0.xxxx, cb0[3].xyzx
    r2.xyz = ((r0.xxxx)*(source[3].xyzx)).xyz;
    // 201: movc r0.xyz, r0.yyyy, l(0,0,0,0), r2.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 202: mul r2.xy, v4.xyxx, cb0[4].xyxx
    r2.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 203: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 204: mul r3.xyz, cb0[5].xyzx, cb0[9].xxxx
    r3.xyz = ((source[5].xyzx)*(source[9].xxxx)).xyz;
    // 205: mad r0.xyz, r2.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 206: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 207: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 208: mad o0.xyz, r9.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 209: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 210: dp3 r0.x, r6.xyzx, r6.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 211: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 212: mul r0.xyz, r0.xxxx, r6.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 213: ge r1.w, l(0.000000), r0.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 214: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 215: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 216: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 217: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 218: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 219: movc r0.xy, r1.wwww, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 220: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 221: mul o4.z, r0.w, r1.x
    output.targets[4].z = ((r0.wwww)*(r1.xxxx)).z;
    // 222: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 223: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 224: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
    // 225: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 226: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 227: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 228: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 229: ret
    return output;
}

// source.character.static-map-native-1104.v1 / source program 06d14cef92ea834191fd4afb458fcefb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1104(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1104(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[12]=g_SourceCharacterBaseConstants[11];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 3: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 4: mul r1.x, r1.z, cb0[10].y
    r1.x = ((r1.zzzz)*(source[10].yyyy)).x;
    // 5: dp2 r1.y, r0.zwzz, r0.zwzz
    r1.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 6: mul r0.zw, r0.zzzw, cb0[8].wwww
    r0.zw = ((r0.zzzw)*(source[8].wwww)).zw;
    // 7: mul r2.xy, r0.zwzz, v2.wwww
    r2.xy = ((r0.zwzz)*(v2.wwww)).xy;
    // 8: add r0.z, -r1.y, l(1.000000)
    r0.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 9: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 10: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 11: add r2.z, r0.z, l(0.000010)
    r2.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 12: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 13: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 14: div r1.yzw, r2.xxyz, r0.zzzz
    r1.yzw = ((r2.xxyz)/(r0.zzzz)).yzw;
    // 15: dp3 r0.z, r1.yzwy, r1.yzwy
    r0.z = (dot((r1.yzwy).xyz,(r1.yzwy).xyz).xxxx).z;
    // 16: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 17: mul r2.xyz, r0.zzzz, r1.yzwy
    r2.xyz = ((r0.zzzz)*(r1.yzwy)).xyz;
    // 18: dp3 r0.z, v1.xyzx, v1.xyzx
    r0.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 19: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 20: mul r3.xyz, r0.zzzz, v1.xyzx
    r3.xyz = ((r0.zzzz)*(v1.xyzx)).xyz;
    // 21: dp3 r0.z, v0.xyzx, v0.xyzx
    r0.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 22: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 23: mul r4.xyz, r0.zzzz, v0.xyzx
    r4.xyz = ((r0.zzzz)*(v0.xyzx)).xyz;
    // 24: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 25: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 26: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 27: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 28: dp3 r6.x, r4.xyzx, r2.xyzx
    r6.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 29: dp2 r7.z, r6.xyxx, cb0[14].xyxx
    r7.z = (dot((r6.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 30: mul r0.zw, cb0[14].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r0.zw = ((source[14].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 31: dp2 r7.x, r6.xyxx, r0.zwzz
    r7.x = (dot((r6.xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 32: dp3 r7.y, r3.xyzx, r2.xyzx
    r7.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 33: mov r7.w, l(1.000000)
    r7.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 34: dp4 r8.x, cb0[15].xyzw, r7.xyzw
    r8.x = (dot((source[15].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).x;
    // 35: dp4 r8.y, cb0[16].xyzw, r7.xyzw
    r8.y = (dot((source[16].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).y;
    // 36: dp4 r8.z, cb0[17].xyzw, r7.xyzw
    r8.z = (dot((source[17].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).z;
    // 37: mul r9.xyzw, r7.yzzx, r7.xyzz
    r9.xyzw = ((r7.yzzx)*(r7.xyzz)).xyzw;
    // 38: dp4 r10.x, cb0[18].xyzw, r9.xyzw
    r10.x = (dot((source[18].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 39: dp4 r10.y, cb0[19].xyzw, r9.xyzw
    r10.y = (dot((source[19].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 40: dp4 r10.z, cb0[20].xyzw, r9.xyzw
    r10.z = (dot((source[20].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 41: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 42: mul r2.w, r7.y, r7.y
    r2.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 43: mov r6.z, r7.y
    r6.z = (r7.yyyy).z;
    // 44: mad r2.w, r7.x, r7.x, -r2.w
    r2.w = ((r7.xxxx)*(r7.xxxx)+(-(r2.wwww))).w;
    // 45: mad r7.xyz, cb0[21].xyzx, r2.wwww, r8.xyzx
    r7.xyz = ((source[21].xyzx)*(r2.wwww)+(r8.xyzx)).xyz;
    // 46: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 47: mul r7.xyz, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((r7.xyzx)*(source[13].xyzx)).xyz;
    // 48: mad r7.xyz, r7.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r7.xyz = ((r7.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r8.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, r0.xyxx, t3.xyzw, s4, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: dp3 r0.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 52: add r10.xyz, -r8.xyzx, r0.xxxx
    r10.xyz = ((-(r8.xyzx))+(r0.xxxx)).xyz;
    // 53: mad r8.xyz, cb0[9].zzzz, r10.xyzx, r8.xyzx
    r8.xyz = ((source[9].zzzz)*(r10.xyzx)+(r8.xyzx)).xyz;
    // 54: mul r10.xyz, cb0[6].xyzx, cb0[9].wwww
    r10.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 55: mul r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)).xyz;
    // 56: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 57: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 58: mul r10.xyz, r0.xxxx, v5.xyzx
    r10.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 59: dp3 r0.x, r2.xyzx, r10.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 60: mul r11.xyz, r0.xxxx, r2.xyzx
    r11.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 61: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 62: dp3 r5.y, r5.xyzx, r11.xyzx
    r5.y = (dot((r5.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 63: dp3 r5.x, r4.xyzx, r11.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 64: mul r4.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r4.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 65: mad r4.xy, cb0[10].xxxx, r5.xyxx, r4.xyxx
    r4.xy = ((source[10].xxxx)*(r5.xyxx)+(r4.xyxx)).xy;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t2.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 67: mad r4.xyz, r4.xyzx, cb0[7].xyzx, -r8.xyzx
    r4.xyz = ((r4.xyzx)*(source[7].xyzx)+(-(r8.xyzx))).xyz;
    // 68: mad r4.xyz, r1.xxxx, r4.xyzx, r8.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 69: mul r8.xyz, r4.xyzx, cb0[10].zzzz
    r8.xyz = ((r4.xyzx)*(source[10].zzzz)).xyz;
    // 70: mad r4.xyz, cb0[10].wwww, r4.xyzx, -r8.xyzx
    r4.xyz = ((source[10].wwww)*(r4.xyzx)+(-(r8.xyzx))).xyz;
    // 71: mul r0.y, r9.z, cb0[11].x
    r0.y = ((r9.zzzz)*(source[11].xxxx)).y;
    // 72: mul r5.zw, r9.yyyx, cb0[12].xxxz
    r5.zw = ((r9.yyyx)*(source[12].xxxz)).zw;
    // 73: log r1.x, |r0.y|
    r1.x = (log2(abs(r0.yyyy))).x;
    // 74: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 75: mul r1.x, r1.x, cb0[11].y
    r1.x = ((r1.xxxx)*(source[11].yyyy)).x;
    // 76: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 77: movc r0.y, r0.y, l(0), r1.x
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).y;
    // 78: min r1.x, r0.y, l(1.000000)
    r1.x = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 79: mul_sat r9.w, r0.y, cb2[3].w
    r9.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 80: mad r4.xyz, r1.xxxx, r4.xyzx, r8.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 81: add r8.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 82: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 83: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 84: mov_sat r4.w, cb0[11].z
    r4.w = (saturate(source[11].zzzz)).w;
    // 85: mad r8.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r8.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 86: mul r0.y, r4.w, l(0.080000)
    r0.y = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).y;
    // 87: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 88: mad r8.xyz, r9.wwww, r8.xyzx, r0.yyyy
    r8.xyz = ((r9.wwww)*(r8.xyzx)+(r0.yyyy)).xyz;
    // 89: deriv_rtx_coarse r9.x, r0.x
    r9.x = (ddx_coarse(r0.xxxx)).x;
    // 90: deriv_rty_coarse r9.y, r0.x
    r9.y = (ddy_coarse(r0.xxxx)).y;
    // 91: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 92: dp2 r0.y, r9.xyxx, r9.xyxx
    r0.y = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).y;
    // 93: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 94: log r9.xy, |r5.zwzz|
    r9.xy = (log2(abs(r5.zwzz))).xy;
    // 95: lt r5.zw, |r5.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r5.zw = (asfloat((uint4)((abs(r5.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 96: mul r9.xy, r9.xyxx, cb0[12].ywyy
    r9.xy = ((r9.xyxx)*(source[12].ywyy)).xy;
    // 97: exp r9.xy, r9.xyxx
    r9.xy = (exp2(r9.xyxx)).xy;
    // 98: movc r1.x, r5.z, l(0), r9.x
    r1.x = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xxxx)).x;
    // 99: min r2.w, r9.y, l(1.000000)
    r2.w = (min(r9.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: movc r2.w, r5.w, l(0), r2.w
    r2.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 101: max r1.x, r1.x, cb0[0].w
    r1.x = (max(r1.xxxx,source[0].wwww)).x;
    // 102: min r9.z, r1.x, l(1.000000)
    r9.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 103: mad_sat r9.y, r0.y, l(0.300000), r9.z
    r9.y = (saturate((r0.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r9.zzzz))).y;
    // 104: mov o2.zw, r9.zzzw
    output.targets[2].zw = (r9.zzzw).zw;
    // 105: add r0.y, -r9.y, l(1.000000)
    r0.y = ((-(r9.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 106: max r12.xyz, r8.xyzx, r0.yyyy
    r12.xyz = (max(r8.xyzx,r0.yyyy)).xyz;
    // 107: add r12.xyz, -r8.xyzx, r12.xyzx
    r12.xyz = ((-(r8.xyzx))+(r12.xyzx)).xyz;
    // 108: mul_sat r0.y, r8.y, l(50.000000)
    r0.y = (saturate((r8.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).y;
    // 109: mul r12.xyz, r0.yyyy, r12.xyzx
    r12.xyz = ((r0.yyyy)*(r12.xyzx)).xyz;
    // 110: add r0.y, r11.z, l(1.000000)
    r0.y = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 111: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 112: add_sat r9.x, -r0.y, r0.x
    r9.x = (saturate((-(r0.yyyy))+(r0.xxxx))).x;
    // 113: sample_indexable(texture2d)(float,float,float,float) r0.xy, r9.xyxx, t5.xyzw, s6
    r0.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 114: add r1.x, r2.w, r9.x
    r1.x = ((r2.wwww)+(r9.xxxx)).x;
    // 115: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 116: mul r13.xyz, r0.yyyy, r8.xyzx
    r13.xyz = ((r0.yyyy)*(r8.xyzx)).xyz;
    // 117: mad r12.xyz, r12.xyzx, r0.xxxx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r0.xxxx)+(r13.xyzx)).xyz;
    // 118: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.x = r0.y != 0.f ? 1.f / r0.y : 0.f;
    // 119: add r0.x, r0.x, l(-1.000000)
    r0.x = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 120: mad r13.xyz, r8.xyzx, r0.xxxx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r8.xyzx)*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 121: dp3 r0.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 122: mad r8.xyz, r0.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r8.xyz = ((r0.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 123: mad r14.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 124: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 125: mul r7.xyz, r7.xyzx, r14.xyzx
    r7.xyz = ((r7.xyzx)*(r14.xyzx)).xyz;
    // 126: mad r13.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r13.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 127: mad r14.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r14.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 128: mad r13.xyz, r2.wwww, r13.xyzx, r14.xyzx
    r13.xyz = ((r2.wwww)*(r13.xyzx)+(r14.xyzx)).xyz;
    // 129: mad r14.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r14.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 130: mad r13.xyz, r13.xyzx, r2.wwww, r14.xyzx
    r13.xyz = ((r13.xyzx)*(r2.wwww)+(r14.xyzx)).xyz;
    // 131: mul r13.xyz, r2.wwww, r13.xyzx
    r13.xyz = ((r2.wwww)*(r13.xyzx)).xyz;
    // 132: max r13.xyz, r2.wwww, r13.xyzx
    r13.xyz = (max(r2.wwww,r13.xyzx)).xyz;
    // 133: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 134: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 135: mul r14.xyz, r0.xxxx, v6.xyzx
    r14.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 136: dp3 r0.x, r14.xyzx, r2.xyzx
    r0.x = (dot((r14.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 137: dp3 r0.y, r14.xyzx, r11.xyzx
    r0.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 138: dp3 r2.y, r3.xyzx, r11.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 139: mad r3.xy, r0.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 140: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 141: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 142: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 143: mul r11.xyz, r0.yyyy, cb0[24].xyzx
    r11.xyz = ((r0.yyyy)*(source[24].xyzx)).xyz;
    // 144: mad r11.xyz, r0.xxxx, cb0[23].xyzx, r11.xyzx
    r11.xyz = ((r0.xxxx)*(source[23].xyzx)+(r11.xyzx)).xyz;
    // 145: mul r11.xyz, r11.xyzx, cb0[25].wwww
    r11.xyz = ((r11.xyzx)*(source[25].wwww)).xyz;
    // 146: mul r11.xyz, r4.xyzx, r11.xyzx
    r11.xyz = ((r4.xyzx)*(r11.xyzx)).xyz;
    // 147: mul r11.xyz, r13.xyzx, r11.xyzx
    r11.xyz = ((r13.xyzx)*(r11.xyzx)).xyz;
    // 148: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 149: mad r7.xyz, -r7.xyzx, r9.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r9.wwww)+(r7.xyzx)).xyz;
    // 150: dp2 r2.x, r5.xyxx, r0.zwzz
    r2.x = (dot((r5.xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 151: dp2 r2.z, r5.xyxx, cb0[14].xyxx
    r2.z = (dot((r5.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 152: mul r0.x, r9.y, l(5.000000)
    r0.x = ((r9.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 153: mul r0.y, r9.y, r9.y
    r0.y = ((r9.yyyy)*(r9.yyyy)).y;
    // 154: mul r0.y, r1.x, r0.y
    r0.y = ((r1.xxxx)*(r0.yyyy)).y;
    // 155: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 156: add r0.y, r2.w, r0.y
    r0.y = ((r2.wwww)+(r0.yyyy)).y;
    // 157: mov o5.y, r2.w
    output.targets[5].y = (r2.wwww).y;
    // 158: add_sat r0.y, r0.y, l(-1.000000)
    r0.y = (saturate((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 159: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r2.xyzx, t6.xyzw, s5, r0.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r2.xyzx).xyz, (r0.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 160: mul r0.xzw, r2.xxyz, r2.wwww
    r0.xzw = ((r2.xxyz)*(r2.wwww)).xzw;
    // 161: mul r0.xzw, r0.xxzw, cb0[13].xxyz
    r0.xzw = ((r0.xxzw)*(source[13].xxyz)).xzw;
    // 162: mad r0.xzw, r0.xxzw, l(6.000000, 0.000000, 6.000000, 6.000000), cb0[13].wwww
    r0.xzw = ((r0.xxzw)*(float4(6.000000,0.000000,6.000000,6.000000))+(source[13].wwww)).xzw;
    // 163: mad r1.x, r0.y, r8.x, r8.y
    r1.x = ((r0.yyyy)*(r8.xxxx)+(r8.yyyy)).x;
    // 164: mad r1.x, r1.x, r0.y, r8.z
    r1.x = ((r1.xxxx)*(r0.yyyy)+(r8.zzzz)).x;
    // 165: mul r1.x, r0.y, r1.x
    r1.x = ((r0.yyyy)*(r1.xxxx)).x;
    // 166: max r0.y, r0.y, r1.x
    r0.y = (max(r0.yyyy,r1.xxxx)).y;
    // 167: mul r2.xyz, r3.yyyy, cb0[24].xyzx
    r2.xyz = ((r3.yyyy)*(source[24].xyzx)).xyz;
    // 168: mad r2.xyz, cb0[23].xyzx, r3.xxxx, r2.xyzx
    r2.xyz = ((source[23].xyzx)*(r3.xxxx)+(r2.xyzx)).xyz;
    // 169: mul r2.xyz, r2.xyzx, cb0[25].wwww
    r2.xyz = ((r2.xyzx)*(source[25].wwww)).xyz;
    // 170: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 171: mul r0.xyz, r0.xzwx, r2.xyzx
    r0.xyz = ((r0.xzwx)*(r2.xyzx)).xyz;
    // 172: mad r2.xyz, r0.xyzx, r12.xyzx, r7.xyzx
    r2.xyz = ((r0.xyzx)*(r12.xyzx)+(r7.xyzx)).xyz;
    // 173: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 174: dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 175: dp3 r0.x, r1.yzwy, r10.xyzx
    r0.x = (dot((r1.yzwy).xyz,(r10.xyzx).xyz).xxxx).x;
    // 176: add r0.y, -|r10.z|, l(1.000000)
    r0.y = ((-(abs(r10.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 177: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 178: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 179: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 180: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 181: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 182: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 183: mul r0.xzw, r0.xxxx, cb0[3].xxyz
    r0.xzw = ((r0.xxxx)*(source[3].xxyz)).xzw;
    // 184: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 185: mul r1.xy, v4.xyxx, cb0[4].xyxx
    r1.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 186: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 187: mul r3.xyz, cb0[5].xyzx, cb0[9].xxxx
    r3.xyz = ((source[5].xyzx)*(source[9].xxxx)).xyz;
    // 188: mad r0.xyz, r1.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 189: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 190: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 191: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 192: mad o0.xyz, r4.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 193: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 194: dp3 r0.x, r6.xyzx, r6.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 195: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 196: mul r0.xyz, r0.xxxx, r6.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 197: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 198: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 199: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 200: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 201: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 202: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 203: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 204: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 205: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 206: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
    // 207: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 208: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 209: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 210: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 211: ret
    return output;
}

// source.character.static-map-native-1105.v1 / source program cfa9e267960eca4ca7470ddace4daa21
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1105(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[10]=g_SourceCharacterEnvironmentColor;source[11]=g_SourceCharacterEnvironmentRotation;}
    source[23]=1.f;
    source[24]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
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
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 11: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: mul r0.w, r3.z, cb0[6].w
    r0.w = ((r3.zzzz)*(source[6].wwww)).w;
    // 13: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 14: mul r3.xy, r3.xyxx, cb0[6].xxxx
    r3.xy = ((r3.xyxx)*(source[6].xxxx)).xy;
    // 15: mul r3.xy, r3.xyxx, v2.wwww
    r3.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 16: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 18: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 19: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 20: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 23: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 24: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 25: mul r4.xyz, r1.wwww, r3.xyzx
    r4.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 26: dp3 r5.y, r2.xyzx, r4.xyzx
    r5.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 27: dp3 r5.x, r1.xyzx, r4.xyzx
    r5.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 28: dp2 r6.z, r5.xyxx, cb0[11].xyxx
    r6.z = (dot((r5.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 29: mul r7.xy, cb0[11].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[11].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 30: dp2 r6.x, r5.xyxx, r7.xyxx
    r6.x = (dot((r5.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 31: dp3 r6.y, r0.xyzx, r4.xyzx
    r6.y = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 32: mov r6.w, l(1.000000)
    r6.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 33: dp4 r8.x, cb0[12].xyzw, r6.xyzw
    r8.x = (dot((source[12].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).x;
    // 34: dp4 r8.y, cb0[13].xyzw, r6.xyzw
    r8.y = (dot((source[13].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).y;
    // 35: dp4 r8.z, cb0[14].xyzw, r6.xyzw
    r8.z = (dot((source[14].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).z;
    // 36: mul r9.xyzw, r6.yzzx, r6.xyzz
    r9.xyzw = ((r6.yzzx)*(r6.xyzz)).xyzw;
    // 37: dp4 r10.x, cb0[15].xyzw, r9.xyzw
    r10.x = (dot((source[15].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 38: dp4 r10.y, cb0[16].xyzw, r9.xyzw
    r10.y = (dot((source[16].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 39: dp4 r10.z, cb0[17].xyzw, r9.xyzw
    r10.z = (dot((source[17].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 40: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 41: mul r1.w, r6.y, r6.y
    r1.w = ((r6.yyyy)*(r6.yyyy)).w;
    // 42: mov r5.z, r6.y
    r5.z = (r6.yyyy).z;
    // 43: mad r1.w, r6.x, r6.x, -r1.w
    r1.w = ((r6.xxxx)*(r6.xxxx)+(-(r1.wwww))).w;
    // 44: mad r6.xyz, cb0[18].xyzx, r1.wwww, r8.xyzx
    r6.xyz = ((source[18].xyzx)*(r1.wwww)+(r8.xyzx)).xyz;
    // 45: max r6.xyz, r6.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r6.xyz = (max(r6.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 46: mul r6.xyz, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r6.xyzx)*(source[10].xyzx)).xyz;
    // 47: mad r6.xyz, r6.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[10].wwww
    r6.xyz = ((r6.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[10].wwww)).xyz;
    // 48: mul r8.xyz, cb0[5].xyzx, cb0[7].yyyy
    r8.xyz = ((source[5].xyzx)*(source[7].yyyy)).xyz;
    // 49: mul r9.xyz, cb0[4].xyzx, cb0[7].xxxx
    r9.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 52: mad r8.xyz, r8.xyzx, r10.xyzx, -r9.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)+(-(r9.xyzx))).xyz;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 54: mul r1.w, r10.z, cb0[7].z
    r1.w = ((r10.zzzz)*(source[7].zzzz)).w;
    // 55: mul r7.zw, r10.yyyx, cb0[9].xxxz
    r7.zw = ((r10.yyyx)*(source[9].xxxz)).zw;
    // 56: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 57: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 58: mul r2.w, r2.w, cb0[7].w
    r2.w = ((r2.wwww)*(source[7].wwww)).w;
    // 59: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 60: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 61: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 62: mul_sat r10.w, r1.w, cb2[3].w
    r10.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 63: mad r8.xyz, r2.wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 64: mul r9.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r9.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 65: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 66: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 67: mul r11.xyz, r1.wwww, v5.xyzx
    r11.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 68: dp3 r1.w, r4.xyzx, r11.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 69: mul r12.xyz, r1.wwww, r4.xyzx
    r12.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 70: mad r12.xyz, r12.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r12.xyz = ((r12.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 71: dp3 r2.y, r2.xyzx, r12.xyzx
    r2.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 72: dp3 r2.x, r1.xyzx, r12.xyzx
    r2.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 73: mad r1.xy, cb0[6].yyyy, r2.xyxx, r9.xyxx
    r1.xy = ((source[6].yyyy)*(r2.xyxx)+(r9.xyxx)).xy;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 75: mul r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)*(source[3].xyzx)).xyz;
    // 76: mad r1.xyz, cb0[6].zzzz, r1.xyzx, r1.xyzx
    r1.xyz = ((source[6].zzzz)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 77: add r1.xyz, r1.xyzx, -cb0[6].zzzz
    r1.xyz = ((r1.xyzx)+(-(source[6].zzzz))).xyz;
    // 78: mov_sat r9.xyz, r1.xyzx
    r9.xyz = (saturate(r1.xyzx)).xyz;
    // 79: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 80: mad r1.xyz, -r0.wwww, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.wwww))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 81: mad r8.xyz, r0.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r0.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 82: mul r1.xyz, r1.xyzx, r8.xyzx
    r1.xyz = ((r1.xyzx)*(r8.xyzx)).xyz;
    // 83: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 84: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 85: mul r8.xyz, r1.xyzx, cb0[8].xxxx
    r8.xyz = ((r1.xyzx)*(source[8].xxxx)).xyz;
    // 86: mad r1.xyz, cb0[8].yyyy, r1.xyzx, -r8.xyzx
    r1.xyz = ((source[8].yyyy)*(r1.xyzx)+(-(r8.xyzx))).xyz;
    // 87: mad r1.xyz, r2.wwww, r1.xyzx, r8.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r8.xyzx)).xyz;
    // 88: add r8.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 89: mul r1.xyz, r1.xyzx, r8.xyzx
    r1.xyz = ((r1.xyzx)*(r8.xyzx)).xyz;
    // 90: mad_sat r8.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 91: mov_sat r8.w, cb0[8].z
    r8.w = (saturate(source[8].zzzz)).w;
    // 92: mad r1.xyz, -r8.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r8.xyzx
    r1.xyz = ((-(r8.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r8.xyzx)).xyz;
    // 93: mul r0.w, r8.w, l(0.080000)
    r0.w = ((r8.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 94: mov o3.xyzw, r8.xyzw
    output.targets[3].xyzw = (r8.xyzw).xyzw;
    // 95: mad r1.xyz, r10.wwww, r1.xyzx, r0.wwww
    r1.xyz = ((r10.wwww)*(r1.xyzx)+(r0.wwww)).xyz;
    // 96: deriv_rtx_coarse r9.x, r1.w
    r9.x = (ddx_coarse(r1.wwww)).x;
    // 97: deriv_rty_coarse r9.y, r1.w
    r9.y = (ddy_coarse(r1.wwww)).y;
    // 98: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 100: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 101: log r2.zw, |r7.zzzw|
    r2.zw = (log2(abs(r7.zzzw))).zw;
    // 102: lt r7.zw, |r7.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r7.zw = (asfloat((uint4)((abs(r7.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 103: mul r2.zw, r2.zzzw, cb0[9].yyyw
    r2.zw = ((r2.zzzw)*(source[9].yyyw)).zw;
    // 104: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 105: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: movc r2.zw, r7.zzzw, l(0,0,0,0), r2.zzzw
    r2.zw = ((asuint(r7.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzw)).zw;
    // 107: max r2.z, r2.z, cb0[0].w
    r2.z = (max(r2.zzzz,source[0].wwww)).z;
    // 108: min r10.z, r2.z, l(1.000000)
    r10.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 109: mad_sat r9.y, r1.w, l(0.300000), r10.z
    r9.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r10.zzzz))).y;
    // 110: add r1.w, -r9.y, l(1.000000)
    r1.w = ((-(r9.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: max r13.xyz, r1.xyzx, r1.wwww
    r13.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 112: add r13.xyz, -r1.xyzx, r13.xyzx
    r13.xyz = ((-(r1.xyzx))+(r13.xyzx)).xyz;
    // 113: mul_sat r1.w, r1.y, l(50.000000)
    r1.w = (saturate((r1.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 114: mul r13.xyz, r1.wwww, r13.xyzx
    r13.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 115: add r1.w, r12.z, l(1.000000)
    r1.w = ((r12.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: add_sat r9.x, r0.w, -r1.w
    r9.x = (saturate((r0.wwww)+(-(r1.wwww)))).x;
    // 118: sample_indexable(texture2d)(float,float,float,float) r7.zw, r9.xyxx, t4.zwxy, s6
    r7.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 119: add r0.w, r2.w, r9.x
    r0.w = ((r2.wwww)+(r9.xxxx)).w;
    // 120: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 121: mul r9.xzw, r1.xxyz, r7.wwww
    r9.xzw = ((r1.xxyz)*(r7.wwww)).xzw;
    // 122: mad r9.xzw, r13.xxyz, r7.zzzz, r9.xxzw
    r9.xzw = ((r13.xxyz)*(r7.zzzz)+(r9.xxzw)).xzw;
    // 123: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r7.w
    r1.w = r7.w != 0.f ? 1.f / r7.w : 0.f;
    // 124: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 125: mad r13.xyz, r1.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r1.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 126: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 127: mad r1.xyz, r1.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r1.xyz = ((r1.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 128: mad r14.xyz, -r9.xzwx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r9.xzwx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 129: mul r9.xzw, r9.xxzw, r13.xxyz
    r9.xzw = ((r9.xxzw)*(r13.xxyz)).xzw;
    // 130: mul r6.xyz, r6.xyzx, r14.xyzx
    r6.xyz = ((r6.xyzx)*(r14.xyzx)).xyz;
    // 131: dp2_sat r13.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r13.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 132: dp3_sat r13.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r13.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 133: dp3_sat r13.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r13.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 134: mul r13.xyz, r13.xyzx, r13.xyzx
    r13.xyz = ((r13.xyzx)*(r13.xyzx)).xyz;
    // 135: sample_indexable(texture2d)(float,float,float,float) r14.xyz, v3.zwzz, t7.xyzw, s4
    r14.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 136: mul r14.xyz, r14.xyzx, cb0[24].xyzx
    r14.xyz = ((r14.xyzx)*(source[24].xyzx)).xyz;
    // 137: dp3 r1.w, r14.xyzx, r13.xyzx
    r1.w = (dot((r14.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 138: sample_indexable(texture2d)(float,float,float,float) r13.xyz, v3.zwzz, t6.xyzw, s4
    r13.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 139: mul r13.xyz, r13.xyzx, cb0[23].xyzx
    r13.xyz = ((r13.xyzx)*(source[23].xyzx)).xyz;
    // 140: mul r15.xyz, r1.wwww, r13.xyzx
    r15.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 141: dp3 r2.z, v6.xyzx, v6.xyzx
    r2.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 142: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 143: mul r16.xyz, r2.zzzz, v6.xyzx
    r16.xyz = ((r2.zzzz)*(v6.xyzx)).xyz;
    // 144: dp3 r2.z, r16.xyzx, r4.xyzx
    r2.z = (dot((r16.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 145: mad r4.xy, r2.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r2.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 146: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 147: mul r4.yzw, r4.yyyy, cb0[21].xxyz
    r4.yzw = ((r4.yyyy)*(source[21].xxyz)).yzw;
    // 148: mad r4.xyz, r4.xxxx, cb0[20].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[20].xyzx)+(r4.yzwy)).xyz;
    // 149: mul r4.xyz, r4.xyzx, cb0[22].wwww
    r4.xyz = ((r4.xyzx)*(source[22].wwww)).xyz;
    // 150: mul r16.xyz, r8.xyzx, r4.xyzx
    r16.xyz = ((r8.xyzx)*(r4.xyzx)).xyz;
    // 151: mad r15.xyz, r8.xyzx, r15.xyzx, r16.xyzx
    r15.xyz = ((r8.xyzx)*(r15.xyzx)+(r16.xyzx)).xyz;
    // 152: mad r16.xyz, r8.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r16.xyz = ((r8.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 153: mad r17.xyz, r8.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r17.xyz = ((r8.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 154: mad r18.xyz, r8.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r18.xyz = ((r8.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 155: mad r17.xyz, r2.wwww, r17.xyzx, r18.xyzx
    r17.xyz = ((r2.wwww)*(r17.xyzx)+(r18.xyzx)).xyz;
    // 156: mad r16.xyz, r17.xyzx, r2.wwww, r16.xyzx
    r16.xyz = ((r17.xyzx)*(r2.wwww)+(r16.xyzx)).xyz;
    // 157: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 158: max r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = (max(r2.wwww,r16.xyzx)).xyz;
    // 159: mul r15.xyz, r15.xyzx, r16.xyzx
    r15.xyz = ((r15.xyzx)*(r16.xyzx)).xyz;
    // 160: mul r6.xyz, r6.xyzx, r15.xyzx
    r6.xyz = ((r6.xyzx)*(r15.xyzx)).xyz;
    // 161: mad r6.xyz, -r6.xyzx, r10.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r10.wwww)+(r6.xyzx)).xyz;
    // 162: mov o2.zw, r10.zzzw
    output.targets[2].zw = (r10.zzzw).zw;
    // 163: mul r2.z, r9.y, l(5.000000)
    r2.z = ((r9.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 164: mul r3.w, r9.y, r9.y
    r3.w = ((r9.yyyy)*(r9.yyyy)).w;
    // 165: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 166: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 167: add r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)+(r0.wwww)).w;
    // 168: mov o5.y, r2.w
    output.targets[5].y = (r2.wwww).y;
    // 169: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 170: dp3 r0.y, r0.xyzx, r12.xyzx
    r0.y = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 171: dp2 r0.x, r2.xyxx, r7.xyxx
    r0.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 172: dp2 r0.z, r2.xyxx, cb0[11].xyxx
    r0.z = (dot((r2.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 173: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r0.xyzx, t5.xyzw, s5, r2.z
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r0.xyzx).xyz, (r2.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 174: mul r0.xyz, r2.xyzx, r2.wwww
    r0.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 175: mul r0.xyz, r0.xyzx, cb0[10].xyzx
    r0.xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 176: mad r0.xyz, r0.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[10].wwww
    r0.xyz = ((r0.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[10].wwww)).xyz;
    // 177: dp2_sat r2.x, r12.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r12.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 178: dp3_sat r2.y, r12.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r12.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 179: dp3_sat r2.z, r12.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r12.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 180: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 181: dp3 r2.x, r14.xyzx, r2.xyzx
    r2.x = (dot((r14.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 182: add r1.w, r1.w, -r2.x
    r1.w = ((r1.wwww)+(-(r2.xxxx))).w;
    // 183: mad r1.w, r10.z, r1.w, r2.x
    r1.w = ((r10.zzzz)*(r1.wwww)+(r2.xxxx)).w;
    // 184: mad r2.xyz, r13.xyzx, r1.wwww, r4.xyzx
    r2.xyz = ((r13.xyzx)*(r1.wwww)+(r4.xyzx)).xyz;
    // 185: mul r4.xyz, r1.wwww, r13.xyzx
    r4.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 186: mad r1.x, r0.w, r1.x, r1.y
    r1.x = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).x;
    // 187: mad r1.x, r1.x, r0.w, r1.z
    r1.x = ((r1.xxxx)*(r0.wwww)+(r1.zzzz)).x;
    // 188: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 189: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 190: mul r1.xyz, r0.wwww, r2.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 191: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 192: div r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)/(r2.xyzx)).xyz;
    // 193: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 194: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 195: mad r1.xyz, r0.xyzx, r9.xzwx, r6.xyzx
    r1.xyz = ((r0.xyzx)*(r9.xzwx)+(r6.xyzx)).xyz;
    // 196: mul r0.xyz, r9.xzwx, r0.xyzx
    r0.xyz = ((r9.xzwx)*(r0.xyzx)).xyz;
    // 197: dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 198: dp3 r0.x, r3.xyzx, r11.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 199: add r0.y, -|r11.z|, l(1.000000)
    r0.y = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 200: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 201: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 202: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 203: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 204: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 205: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 206: mul r2.xyz, r0.xxxx, cb0[2].xyzx
    r2.xyz = ((r0.xxxx)*(source[2].xyzx)).xyz;
    // 207: movc r0.xyz, r0.yyyy, l(0,0,0,0), r2.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 208: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 209: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 210: mad o0.xyz, r8.xyzx, cb0[22].xyzx, r0.xyzx
    output.targets[0].xyz = ((r8.xyzx)*(source[22].xyzx)+(r0.xyzx)).xyz;
    // 211: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 212: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 213: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 214: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 215: ge r1.w, l(0.000000), r0.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 216: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 217: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 218: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 219: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 220: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 221: movc r0.xy, r1.wwww, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 222: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 223: mul o4.z, r0.w, r1.x
    output.targets[4].z = ((r0.wwww)*(r1.xxxx)).z;
    // 224: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 225: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 226: ftou r0.x, cb0[19].z
    r0.x = (asfloat((uint4)(source[19].zzzz))).x;
    // 227: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 228: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 229: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 230: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 231: ret
    return output;
}

// source.character.static-map-native-1105.v1 / source program 5d1994509ea9f04f8dbc2104e9a92a50
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1105(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1105(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[10]=g_SourceCharacterEnvironmentColor;source[11]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 4: mul r0.xyz, r0.xyzx, cb0[6].xxwx
    r0.xyz = ((r0.xyzx)*(source[6].xxwx)).xyz;
    // 5: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 6: add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 9: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 11: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 12: div r0.xyw, r1.xyxz, r0.xxxx
    r0.xyw = ((r1.xyxz)/(r0.xxxx)).xyw;
    // 13: dp3 r1.x, r0.xywx, r0.xywx
    r1.x = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).x;
    // 14: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 15: mul r1.xyz, r0.xywx, r1.xxxx
    r1.xyz = ((r0.xywx)*(r1.xxxx)).xyz;
    // 16: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 17: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 18: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 19: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 20: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 21: mul r3.xyz, r1.wwww, v0.xyzx
    r3.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 22: mul r4.xyz, r2.zxyz, r3.yzxy
    r4.xyz = ((r2.zxyz)*(r3.yzxy)).xyz;
    // 23: mad r4.xyz, r2.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r2.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 24: mul r4.xyz, r4.xyzx, v1.wwww
    r4.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 25: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 26: dp3 r5.x, r3.xyzx, r1.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 27: dp2 r6.z, r5.xyxx, cb0[11].xyxx
    r6.z = (dot((r5.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 28: mul r7.xy, cb0[11].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[11].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 29: dp2 r6.x, r5.xyxx, r7.xyxx
    r6.x = (dot((r5.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 30: dp3 r6.y, r2.xyzx, r1.xyzx
    r6.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 31: mov r6.w, l(1.000000)
    r6.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 32: dp4 r8.x, cb0[12].xyzw, r6.xyzw
    r8.x = (dot((source[12].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).x;
    // 33: dp4 r8.y, cb0[13].xyzw, r6.xyzw
    r8.y = (dot((source[13].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).y;
    // 34: dp4 r8.z, cb0[14].xyzw, r6.xyzw
    r8.z = (dot((source[14].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).z;
    // 35: mul r9.xyzw, r6.yzzx, r6.xyzz
    r9.xyzw = ((r6.yzzx)*(r6.xyzz)).xyzw;
    // 36: dp4 r10.x, cb0[15].xyzw, r9.xyzw
    r10.x = (dot((source[15].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 37: dp4 r10.y, cb0[16].xyzw, r9.xyzw
    r10.y = (dot((source[16].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 38: dp4 r10.z, cb0[17].xyzw, r9.xyzw
    r10.z = (dot((source[17].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 39: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 40: mul r1.w, r6.y, r6.y
    r1.w = ((r6.yyyy)*(r6.yyyy)).w;
    // 41: mov r5.z, r6.y
    r5.z = (r6.yyyy).z;
    // 42: mad r1.w, r6.x, r6.x, -r1.w
    r1.w = ((r6.xxxx)*(r6.xxxx)+(-(r1.wwww))).w;
    // 43: mad r6.xyz, cb0[18].xyzx, r1.wwww, r8.xyzx
    r6.xyz = ((source[18].xyzx)*(r1.wwww)+(r8.xyzx)).xyz;
    // 44: max r6.xyz, r6.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r6.xyz = (max(r6.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 45: mul r6.xyz, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r6.xyzx)*(source[10].xyzx)).xyz;
    // 46: mad r6.xyz, r6.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[10].wwww
    r6.xyz = ((r6.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[10].wwww)).xyz;
    // 47: mul r8.xyz, cb0[5].xyzx, cb0[7].yyyy
    r8.xyz = ((source[5].xyzx)*(source[7].yyyy)).xyz;
    // 48: mul r9.xyz, cb0[4].xyzx, cb0[7].xxxx
    r9.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 50: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 51: mad r8.xyz, r8.xyzx, r10.xyzx, -r9.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)+(-(r9.xyzx))).xyz;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 53: mul r1.w, r10.z, cb0[7].z
    r1.w = ((r10.zzzz)*(source[7].zzzz)).w;
    // 54: mul r7.zw, r10.yyyx, cb0[9].xxxz
    r7.zw = ((r10.yyyx)*(source[9].xxxz)).zw;
    // 55: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 56: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 57: mul r2.w, r2.w, cb0[7].w
    r2.w = ((r2.wwww)*(source[7].wwww)).w;
    // 58: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 59: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 60: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: mul_sat r10.w, r1.w, cb2[3].w
    r10.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 62: mad r8.xyz, r2.wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 63: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 64: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 65: mul r9.xyz, r1.wwww, v5.xyzx
    r9.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 66: dp3 r1.w, r1.xyzx, r9.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 67: mul r11.xyz, r1.wwww, r1.xyzx
    r11.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 68: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 69: dp3 r4.y, r4.xyzx, r11.xyzx
    r4.y = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 70: dp3 r4.x, r3.xyzx, r11.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 71: mul r3.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r3.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 72: mad r3.xy, cb0[6].yyyy, r4.xyxx, r3.xyxx
    r3.xy = ((source[6].yyyy)*(r4.xyxx)+(r3.xyxx)).xy;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 74: mul r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)*(source[3].xyzx)).xyz;
    // 75: mad r3.xyz, cb0[6].zzzz, r3.xyzx, r3.xyzx
    r3.xyz = ((source[6].zzzz)*(r3.xyzx)+(r3.xyzx)).xyz;
    // 76: add r3.xyz, r3.xyzx, -cb0[6].zzzz
    r3.xyz = ((r3.xyzx)+(-(source[6].zzzz))).xyz;
    // 77: mov_sat r12.xyz, r3.xyzx
    r12.xyz = (saturate(r3.xyzx)).xyz;
    // 78: mov_sat r3.xyz, -r3.xyzx
    r3.xyz = (saturate(-(r3.xyzx))).xyz;
    // 79: mad r3.xyz, -r0.zzzz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r0.zzzz))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mad r8.xyz, r0.zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((r0.zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 81: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 82: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 83: min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 84: mul r8.xyz, r3.xyzx, cb0[8].xxxx
    r8.xyz = ((r3.xyzx)*(source[8].xxxx)).xyz;
    // 85: mad r3.xyz, cb0[8].yyyy, r3.xyzx, -r8.xyzx
    r3.xyz = ((source[8].yyyy)*(r3.xyzx)+(-(r8.xyzx))).xyz;
    // 86: mad r3.xyz, r2.wwww, r3.xyzx, r8.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r8.xyzx)).xyz;
    // 87: add r8.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 88: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 89: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 90: mov_sat r3.w, cb0[8].z
    r3.w = (saturate(source[8].zzzz)).w;
    // 91: mad r8.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r8.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 92: mul r0.z, r3.w, l(0.080000)
    r0.z = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 93: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 94: mad r8.xyz, r10.wwww, r8.xyzx, r0.zzzz
    r8.xyz = ((r10.wwww)*(r8.xyzx)+(r0.zzzz)).xyz;
    // 95: deriv_rtx_coarse r10.x, r1.w
    r10.x = (ddx_coarse(r1.wwww)).x;
    // 96: deriv_rty_coarse r10.y, r1.w
    r10.y = (ddy_coarse(r1.wwww)).y;
    // 97: add r0.z, r1.w, l(1.000000)
    r0.z = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 98: dp2 r1.w, r10.xyxx, r10.xyxx
    r1.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 99: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 100: log r4.zw, |r7.zzzw|
    r4.zw = (log2(abs(r7.zzzw))).zw;
    // 101: lt r7.zw, |r7.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r7.zw = (asfloat((uint4)((abs(r7.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 102: mul r4.zw, r4.zzzw, cb0[9].yyyw
    r4.zw = ((r4.zzzw)*(source[9].yyyw)).zw;
    // 103: exp r4.zw, r4.zzzw
    r4.zw = (exp2(r4.zzzw)).zw;
    // 104: movc r2.w, r7.z, l(0), r4.z
    r2.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.zzzz)).w;
    // 105: min r3.w, r4.w, l(1.000000)
    r3.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: movc r3.w, r7.w, l(0), r3.w
    r3.w = ((asuint(r7.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 107: max r2.w, r2.w, cb0[0].w
    r2.w = (max(r2.wwww,source[0].wwww)).w;
    // 108: min r10.z, r2.w, l(1.000000)
    r10.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 109: mad_sat r10.y, r1.w, l(0.300000), r10.z
    r10.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r10.zzzz))).y;
    // 110: mov o2.zw, r10.zzzw
    output.targets[2].zw = (r10.zzzw).zw;
    // 111: add r1.w, -r10.y, l(1.000000)
    r1.w = ((-(r10.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: max r12.xyz, r8.xyzx, r1.wwww
    r12.xyz = (max(r8.xyzx,r1.wwww)).xyz;
    // 113: add r12.xyz, -r8.xyzx, r12.xyzx
    r12.xyz = ((-(r8.xyzx))+(r12.xyzx)).xyz;
    // 114: mul_sat r1.w, r8.y, l(50.000000)
    r1.w = (saturate((r8.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 115: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 116: add r1.w, r11.z, l(1.000000)
    r1.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: add_sat r10.x, r0.z, -r1.w
    r10.x = (saturate((r0.zzzz)+(-(r1.wwww)))).x;
    // 119: sample_indexable(texture2d)(float,float,float,float) r4.zw, r10.xyxx, t4.zwxy, s5
    r4.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 120: add r0.z, r3.w, r10.x
    r0.z = ((r3.wwww)+(r10.xxxx)).z;
    // 121: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 122: mul r13.xyz, r4.wwww, r8.xyzx
    r13.xyz = ((r4.wwww)*(r8.xyzx)).xyz;
    // 123: mad r12.xyz, r12.xyzx, r4.zzzz, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r4.zzzz)+(r13.xyzx)).xyz;
    // 124: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r4.w
    r1.w = r4.w != 0.f ? 1.f / r4.w : 0.f;
    // 125: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 126: mad r13.xyz, r8.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r8.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 127: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 128: mad r8.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r8.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 129: mad r14.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 130: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 131: mul r6.xyz, r6.xyzx, r14.xyzx
    r6.xyz = ((r6.xyzx)*(r14.xyzx)).xyz;
    // 132: mad r13.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 133: mad r14.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r14.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 134: mad r15.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r15.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 135: mad r14.xyz, r3.wwww, r14.xyzx, r15.xyzx
    r14.xyz = ((r3.wwww)*(r14.xyzx)+(r15.xyzx)).xyz;
    // 136: mad r13.xyz, r14.xyzx, r3.wwww, r13.xyzx
    r13.xyz = ((r14.xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 137: mul r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = ((r3.wwww)*(r13.xyzx)).xyz;
    // 138: max r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = (max(r3.wwww,r13.xyzx)).xyz;
    // 139: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 140: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 141: mul r14.xyz, r1.wwww, v6.xyzx
    r14.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 142: dp3 r1.x, r14.xyzx, r1.xyzx
    r1.x = (dot((r14.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 143: dp3 r1.y, r14.xyzx, r11.xyzx
    r1.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 144: dp3 r2.y, r2.xyzx, r11.xyzx
    r2.y = (dot((r2.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 145: mad r1.yz, r1.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r1.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 146: mad r1.xw, r1.xxxx, l(0.500000, 0.000000, 0.000000, -0.500000), l(0.500000, 0.000000, 0.000000, 0.500000)
    r1.xw = ((r1.xxxx)*(float4(0.500000,0.000000,0.000000,-0.500000))+(float4(0.500000,0.000000,0.000000,0.500000))).xw;
    // 147: mul r1.xyzw, r1.xyzw, r1.xyzw
    r1.xyzw = ((r1.xyzw)*(r1.xyzw)).xyzw;
    // 148: mul r11.xyz, r1.wwww, cb0[21].xyzx
    r11.xyz = ((r1.wwww)*(source[21].xyzx)).xyz;
    // 149: mad r11.xyz, r1.xxxx, cb0[20].xyzx, r11.xyzx
    r11.xyz = ((r1.xxxx)*(source[20].xyzx)+(r11.xyzx)).xyz;
    // 150: mul r11.xyz, r11.xyzx, cb0[22].wwww
    r11.xyz = ((r11.xyzx)*(source[22].wwww)).xyz;
    // 151: mul r11.xyz, r3.xyzx, r11.xyzx
    r11.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 152: mul r11.xyz, r13.xyzx, r11.xyzx
    r11.xyz = ((r13.xyzx)*(r11.xyzx)).xyz;
    // 153: mul r6.xyz, r6.xyzx, r11.xyzx
    r6.xyz = ((r6.xyzx)*(r11.xyzx)).xyz;
    // 154: mad r6.xyz, -r6.xyzx, r10.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r10.wwww)+(r6.xyzx)).xyz;
    // 155: dp2 r2.x, r4.xyxx, r7.xyxx
    r2.x = (dot((r4.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 156: dp2 r2.z, r4.xyxx, cb0[11].xyxx
    r2.z = (dot((r4.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 157: mul r1.x, r10.y, l(5.000000)
    r1.x = ((r10.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 158: mul r1.w, r10.y, r10.y
    r1.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 159: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 160: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 161: add r0.z, r3.w, r0.z
    r0.z = ((r3.wwww)+(r0.zzzz)).z;
    // 162: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 163: add_sat r0.z, r0.z, l(-1.000000)
    r0.z = (saturate((r0.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).z;
    // 164: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r2.xyzx, t5.xyzw, s4, r1.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r2.xyzx).xyz, (r1.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 165: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 166: mul r2.xyz, r2.xyzx, cb0[10].xyzx
    r2.xyz = ((r2.xyzx)*(source[10].xyzx)).xyz;
    // 167: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[10].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[10].wwww)).xyz;
    // 168: mad r1.x, r0.z, r8.x, r8.y
    r1.x = ((r0.zzzz)*(r8.xxxx)+(r8.yyyy)).x;
    // 169: mad r1.x, r1.x, r0.z, r8.z
    r1.x = ((r1.xxxx)*(r0.zzzz)+(r8.zzzz)).x;
    // 170: mul r1.x, r0.z, r1.x
    r1.x = ((r0.zzzz)*(r1.xxxx)).x;
    // 171: max r0.z, r0.z, r1.x
    r0.z = (max(r0.zzzz,r1.xxxx)).z;
    // 172: mul r1.xzw, r1.zzzz, cb0[21].xxyz
    r1.xzw = ((r1.zzzz)*(source[21].xxyz)).xzw;
    // 173: mad r1.xyz, cb0[20].xyzx, r1.yyyy, r1.xzwx
    r1.xyz = ((source[20].xyzx)*(r1.yyyy)+(r1.xzwx)).xyz;
    // 174: mul r1.xyz, r1.xyzx, cb0[22].wwww
    r1.xyz = ((r1.xyzx)*(source[22].wwww)).xyz;
    // 175: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 176: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 177: mad r2.xyz, r1.xyzx, r12.xyzx, r6.xyzx
    r2.xyz = ((r1.xyzx)*(r12.xyzx)+(r6.xyzx)).xyz;
    // 178: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 179: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 180: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 181: add r0.y, -|r9.z|, l(1.000000)
    r0.y = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 182: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 183: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 184: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 185: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 186: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 187: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 188: mul r0.xzw, r0.xxxx, cb0[2].xxyz
    r0.xzw = ((r0.xxxx)*(source[2].xxyz)).xzw;
    // 189: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 190: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 191: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 192: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 193: mad o0.xyz, r3.xyzx, cb0[22].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[22].xyzx)+(r0.xyzx)).xyz;
    // 194: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 195: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 196: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 197: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 198: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 199: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 200: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 201: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 202: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 203: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 204: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 205: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 206: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 207: ftou r0.x, cb0[19].z
    r0.x = (asfloat((uint4)(source[19].zzzz))).x;
    // 208: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 209: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 210: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 211: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 212: ret
    return output;
}

// source.character.static-map-native-1106.v1 / source program 402f03043b7c9e489d284feca29b6e8f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1106(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    source[26]=1.f;
    source[27]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f, r19=0.f;
    // 1: mul r0.xyz, cb0[6].xyzx, cb0[9].xxxx
    r0.xyz = ((source[6].xyzx)*(source[9].xxxx)).xyz;
    // 2: mul r1.xyz, cb0[5].xyzx, cb0[8].wwww
    r1.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r3.xyz, cb0[8].zzzz, r3.xyzx, r2.xyzx
    r3.xyz = ((source[8].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 8: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 10: mul r0.w, r2.z, cb0[9].y
    r0.w = ((r2.zzzz)*(source[9].yyyy)).w;
    // 11: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 12: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 13: mul r1.w, r1.w, cb0[9].z
    r1.w = ((r1.wwww)*(source[9].zzzz)).w;
    // 14: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 15: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 16: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 18: mul r1.xyz, r0.xyzx, cb0[9].wwww
    r1.xyz = ((r0.xyzx)*(source[9].wwww)).xyz;
    // 19: mad r0.xyz, cb0[10].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[10].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 20: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 21: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 22: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 23: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 24: mad r1.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 25: mad r3.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 26: mul r1.w, r2.x, cb0[12].y
    r1.w = ((r2.xxxx)*(source[12].yyyy)).w;
    // 27: mul r2.x, r2.y, cb0[11].w
    r2.x = ((r2.yyyy)*(source[11].wwww)).x;
    // 28: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 29: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 30: mul r2.y, r2.y, cb0[12].z
    r2.y = ((r2.yyyy)*(source[12].zzzz)).y;
    // 31: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 32: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: movc r1.w, r1.w, l(0), r2.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).w;
    // 34: mad r1.xyz, r1.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 35: mad r3.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 36: mad r1.xyz, r1.xyzx, r1.wwww, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 37: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 38: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 39: dp3 r2.y, v7.xyzx, v7.xyzx
    r2.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 40: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 41: mul r3.xyz, r2.yyyy, v7.xyzx
    r3.xyz = ((r2.yyyy)*(v7.xyzx)).xyz;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 43: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 44: dp2 r3.w, r2.yzyy, r2.yzyy
    r3.w = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).w;
    // 45: mul r2.yz, r2.yyzy, cb0[8].xxxx
    r2.yz = ((r2.yyzy)*(source[8].xxxx)).yz;
    // 46: mul r4.xy, r2.yzyy, v2.wwww
    r4.xy = ((r2.yzyy)*(v2.wwww)).xy;
    // 47: add r2.y, -r3.w, l(1.000000)
    r2.y = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 48: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 49: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 50: add r4.z, r2.y, l(0.000010)
    r4.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 51: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 52: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 53: div r4.xyz, r4.xyzx, r2.yyyy
    r4.xyz = ((r4.xyzx)/(r2.yyyy)).xyz;
    // 54: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 55: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 56: mul r5.xyz, r2.yyyy, r4.xyzx
    r5.xyz = ((r2.yyyy)*(r4.xyzx)).xyz;
    // 57: dp3 r2.y, r3.xyzx, r5.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 58: dp3 r2.z, -r3.xyzx, r5.xyzx
    r2.z = (dot((-(r3.xyzx)).xyz,(r5.xyzx).xyz).xxxx).z;
    // 59: mad r3.xy, r2.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 60: mad r2.yz, r2.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r2.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 61: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 62: mul r6.xyz, r2.zzzz, cb0[24].xyzx
    r6.xyz = ((r2.zzzz)*(source[24].xyzx)).xyz;
    // 63: mad r6.xyz, r2.yyyy, cb0[23].xyzx, r6.xyzx
    r6.xyz = ((r2.yyyy)*(source[23].xyzx)+(r6.xyzx)).xyz;
    // 64: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 65: mul r7.xyz, r0.xyzx, r6.xyzx
    r7.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 66: dp2_sat r8.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 67: dp3_sat r8.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 68: dp3_sat r8.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 69: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 70: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t8.xyzw, s5
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 71: mul r9.xyz, r9.xyzx, cb0[27].xyzx
    r9.xyz = ((r9.xyzx)*(source[27].xyzx)).xyz;
    // 72: dp3 r2.y, r9.xyzx, r8.xyzx
    r2.y = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 73: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t7.xyzw, s5
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 74: mul r8.xyz, r8.xyzx, cb0[26].xyzx
    r8.xyz = ((r8.xyzx)*(source[26].xyzx)).xyz;
    // 75: mul r10.xyz, r2.yyyy, r8.xyzx
    r10.xyz = ((r2.yyyy)*(r8.xyzx)).xyz;
    // 76: mad r7.xyz, r0.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r0.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 77: mul r7.xyz, r1.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 78: dp3 r2.z, v1.xyzx, v1.xyzx
    r2.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 79: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 80: mul r10.xyz, r2.zzzz, v1.xyzx
    r10.xyz = ((r2.zzzz)*(v1.xyzx)).xyz;
    // 81: dp3 r2.z, v0.xyzx, v0.xyzx
    r2.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 82: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 83: mul r11.xyz, r2.zzzz, v0.xyzx
    r11.xyz = ((r2.zzzz)*(v0.xyzx)).xyz;
    // 84: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 85: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 86: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 87: dp3 r13.y, r12.xyzx, r5.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 88: dp3 r13.x, r11.xyzx, r5.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 89: dp2 r14.z, r13.xyxx, cb0[15].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 90: mul r3.zw, cb0[15].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r3.zw = ((source[15].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 91: dp2 r14.x, r13.xyxx, r3.zwzz
    r14.x = (dot((r13.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 92: dp3 r14.y, r10.xyzx, r5.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 93: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 94: dp4 r13.x, cb0[16].xyzw, r14.xyzw
    r13.x = (dot((source[16].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 95: dp4 r13.y, cb0[17].xyzw, r14.xyzw
    r13.y = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 96: dp4 r13.z, cb0[18].xyzw, r14.xyzw
    r13.z = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 97: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 98: mul r2.z, r14.y, r14.y
    r2.z = ((r14.yyyy)*(r14.yyyy)).z;
    // 99: mad r2.z, r14.x, r14.x, -r2.z
    r2.z = ((r14.xxxx)*(r14.xxxx)+(-(r2.zzzz))).z;
    // 100: dp4 r14.x, cb0[19].xyzw, r15.xyzw
    r14.x = (dot((source[19].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 101: dp4 r14.y, cb0[20].xyzw, r15.xyzw
    r14.y = (dot((source[20].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 102: dp4 r14.z, cb0[21].xyzw, r15.xyzw
    r14.z = (dot((source[21].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 103: add r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)+(r14.xyzx)).xyz;
    // 104: mad r13.xyz, cb0[22].xyzx, r2.zzzz, r13.xyzx
    r13.xyz = ((source[22].xyzx)*(r2.zzzz)+(r13.xyzx)).xyz;
    // 105: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 106: mul r13.xyz, r13.xyzx, cb0[14].xyzx
    r13.xyz = ((r13.xyzx)*(source[14].xyzx)).xyz;
    // 107: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 108: dp3 r2.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 109: log r4.w, |r2.x|
    r4.w = (log2(abs(r2.xxxx))).w;
    // 110: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 111: mul r4.w, r4.w, cb0[12].x
    r4.w = ((r4.wwww)*(source[12].xxxx)).w;
    // 112: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 113: movc r2.x, r2.x, l(0), r4.w
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).x;
    // 114: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 115: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 116: dp3 r4.w, v6.xyzx, v6.xyzx
    r4.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 117: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 118: mul r14.xyz, r4.wwww, v6.xyzx
    r14.xyz = ((r4.wwww)*(v6.xyzx)).xyz;
    // 119: dp3 r4.w, r5.xyzx, r14.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 120: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 121: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 122: deriv_rtx_coarse r15.x, r4.w
    r15.x = (ddx_coarse(r4.wwww)).x;
    // 123: deriv_rty_coarse r15.y, r4.w
    r15.y = (ddy_coarse(r4.wwww)).y;
    // 124: dp2 r5.w, r15.xyxx, r15.xyxx
    r5.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 125: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 126: mad_sat r15.y, r5.w, l(0.300000), r2.x
    r15.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.xxxx))).y;
    // 127: mad r5.w, r15.y, l(0.200000), l(0.200000)
    r5.w = ((r15.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 128: div r2.z, r2.z, r5.w
    r2.z = ((r2.zzzz)/(r5.wwww)).z;
    // 129: dp3 r6.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 130: mad r2.z, r6.w, l(5.000000), r2.z
    r2.z = ((r6.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.zzzz)).z;
    // 131: add r7.w, r2.w, cb0[11].y
    r7.w = ((r2.wwww)+(source[11].yyyy)).w;
    // 132: add r2.w, r2.w, cb0[10].w
    r2.w = ((r2.wwww)+(source[10].wwww)).w;
    // 133: sample_b_indexable(texture2d)(float,float,float,float) r8.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r8.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 134: mad r7.w, r8.w, -r7.w, r7.w
    r7.w = ((r8.wwww)*(-(r7.wwww))+(r7.wwww)).w;
    // 135: add_sat r0.w, r0.w, r7.w
    r0.w = (saturate((r0.wwww)+(r7.wwww))).w;
    // 136: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 137: add_sat r2.z, r0.w, r2.z
    r2.z = (saturate((r0.wwww)+(r2.zzzz))).z;
    // 138: mad r7.w, r2.z, l(-2.000000), l(3.000000)
    r7.w = ((r2.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 139: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 140: mul r2.z, r2.z, r7.w
    r2.z = ((r2.zzzz)*(r7.wwww)).z;
    // 141: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 142: mul r2.z, r2.z, l(1.500000)
    r2.z = ((r2.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 143: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 144: mul r13.xyz, r2.zzzz, r13.xyzx
    r13.xyz = ((r2.zzzz)*(r13.xyzx)).xyz;
    // 145: mul r7.xyz, r7.xyzx, r13.xyzx
    r7.xyz = ((r7.xyzx)*(r13.xyzx)).xyz;
    // 146: add r2.z, -r2.w, l(1.000000)
    r2.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 147: mad_sat r2.z, r8.w, r2.z, r2.w
    r2.z = (saturate((r8.wwww)*(r2.zzzz)+(r2.wwww))).z;
    // 148: mad r2.z, -r2.z, cb0[2].x, l(1.000000)
    r2.z = ((-(r2.zzzz))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 149: mul r2.w, r15.y, r15.y
    r2.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 150: mad r7.w, r2.w, l(0.350000), l(1.000000)
    r7.w = ((r2.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: div_sat r2.z, r2.z, r7.w
    r2.z = (saturate((r2.zzzz)/(r7.wwww))).z;
    // 152: add r7.w, r4.w, l(1.000000)
    r7.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: mov_sat r4.w, r4.w
    r4.w = (saturate(r4.wwww)).w;
    // 154: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 155: mul r4.w, r4.w, cb0[1].y
    r4.w = ((r4.wwww)*(source[1].yyyy)).w;
    // 156: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 157: mad_sat r4.w, r4.w, cb0[1].w, cb0[1].z
    r4.w = (saturate((r4.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 158: add r8.w, r5.z, l(1.000000)
    r8.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: min r8.w, r8.w, l(1.000000)
    r8.w = (min(r8.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: add_sat r15.x, r7.w, -r8.w
    r15.x = (saturate((r7.wwww)+(-(r8.wwww)))).x;
    // 161: add r15.zw, -r15.yyyx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r15.zw = ((-(r15.yyyx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 162: mov_sat r7.w, cb0[10].y
    r7.w = (saturate(source[10].yyyy)).w;
    // 163: mad r16.xyz, -r7.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r16.xyz = ((-(r7.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 164: mul r7.w, r7.w, l(0.080000)
    r7.w = ((r7.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 165: mad r16.xyz, r0.wwww, r16.xyzx, r7.wwww
    r16.xyz = ((r0.wwww)*(r16.xyzx)+(r7.wwww)).xyz;
    // 166: max r17.xyz, r15.zzzz, r16.xyzx
    r17.xyz = (max(r15.zzzz,r16.xyzx)).xyz;
    // 167: add r17.xyz, -r16.xyzx, r17.xyzx
    r17.xyz = ((-(r16.xyzx))+(r17.xyzx)).xyz;
    // 168: mul_sat r7.w, r16.y, l(50.000000)
    r7.w = (saturate((r16.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 169: mul r17.xyz, r7.wwww, r17.xyzx
    r17.xyz = ((r7.wwww)*(r17.xyzx)).xyz;
    // 170: sample_indexable(texture2d)(float,float,float,float) r18.xy, r15.xyxx, t5.xyzw, s7
    r18.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 171: add r8.w, r1.w, r15.x
    r8.w = ((r1.wwww)+(r15.xxxx)).w;
    // 172: log r8.w, r8.w
    r8.w = (log2(r8.wwww)).w;
    // 173: mul r2.w, r2.w, r8.w
    r2.w = ((r2.wwww)*(r8.wwww)).w;
    // 174: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 175: add r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)+(r2.wwww)).w;
    // 176: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 177: mul r2.w, r15.y, l(5.000000)
    r2.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 178: mul r15.xyz, r16.xyzx, r18.yyyy
    r15.xyz = ((r16.xyzx)*(r18.yyyy)).xyz;
    // 179: mad r15.xyz, r17.xyzx, r18.xxxx, r15.xyzx
    r15.xyz = ((r17.xyzx)*(r18.xxxx)+(r15.xyzx)).xyz;
    // 180: div r8.w, l(1.000000, 1.000000, 1.000000, 1.000000), r18.y
    r8.w = r18.y != 0.f ? 1.f / r18.y : 0.f;
    // 181: add r8.w, r8.w, l(-1.000000)
    r8.w = ((r8.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 182: mad r17.xyz, r16.xyzx, r8.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r16.xyzx)*(r8.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 183: mad r18.xyz, -r15.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r15.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: mul r15.xyz, r15.xyzx, r17.xyzx
    r15.xyz = ((r15.xyzx)*(r17.xyzx)).xyz;
    // 185: mul r8.w, r15.w, r15.w
    r8.w = ((r15.wwww)*(r15.wwww)).w;
    // 186: mul r8.w, r8.w, r8.w
    r8.w = ((r8.wwww)*(r8.wwww)).w;
    // 187: mul r9.w, r15.w, r8.w
    r9.w = ((r15.wwww)*(r8.wwww)).w;
    // 188: mad r8.w, -r8.w, r15.w, l(1.000000)
    r8.w = ((-(r8.wwww))*(r15.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r17.xyz, r16.xyzx, r8.wwww
    r17.xyz = ((r16.xyzx)*(r8.wwww)).xyz;
    // 190: dp3 r8.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 191: mad r16.xyz, r8.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r16.xyz = ((r8.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 192: mad r17.xyz, r7.wwww, r9.wwww, r17.xyzx
    r17.xyz = ((r7.wwww)*(r9.wwww)+(r17.xyzx)).xyz;
    // 193: add r17.xyz, -r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r17.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 194: mul r17.xyz, r17.xyzx, r17.xyzx
    r17.xyz = ((r17.xyzx)*(r17.xyzx)).xyz;
    // 195: mad r19.xyz, -r2.zzzz, r17.xyzx, r18.xyzx
    r19.xyz = ((-(r2.zzzz))*(r17.xyzx)+(r18.xyzx)).xyz;
    // 196: mul r18.xyz, r0.xyzx, r18.xyzx
    r18.xyz = ((r0.xyzx)*(r18.xyzx)).xyz;
    // 197: mul r17.xyz, r2.zzzz, r17.xyzx
    r17.xyz = ((r2.zzzz)*(r17.xyzx)).xyz;
    // 198: mul r17.xyz, r0.xyzx, r17.xyzx
    r17.xyz = ((r0.xyzx)*(r17.xyzx)).xyz;
    // 199: mad r17.xyz, -r17.xyzx, r0.wwww, r17.xyzx
    r17.xyz = ((-(r17.xyzx))*(r0.wwww)+(r17.xyzx)).xyz;
    // 200: mul r7.xyz, r7.xyzx, r19.xyzx
    r7.xyz = ((r7.xyzx)*(r19.xyzx)).xyz;
    // 201: mad r7.xyz, -r7.xyzx, r0.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r0.wwww)+(r7.xyzx)).xyz;
    // 202: dp2_sat r19.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r19.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 203: dp3_sat r19.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r19.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 204: dp3_sat r19.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r19.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 205: mul r19.xyz, r19.xyzx, r19.xyzx
    r19.xyz = ((r19.xyzx)*(r19.xyzx)).xyz;
    // 206: dp3 r7.w, r9.xyzx, r19.xyzx
    r7.w = (dot((r9.xyzx).xyz,(r19.xyzx).xyz).xxxx).w;
    // 207: add r2.y, r2.y, -r7.w
    r2.y = ((r2.yyyy)+(-(r7.wwww))).y;
    // 208: mad r2.x, r2.x, r2.y, r7.w
    r2.x = ((r2.xxxx)*(r2.yyyy)+(r7.wwww)).x;
    // 209: mad r6.xyz, r8.xyzx, r2.xxxx, r6.xyzx
    r6.xyz = ((r8.xyzx)*(r2.xxxx)+(r6.xyzx)).xyz;
    // 210: mad r2.x, r1.w, r16.x, r16.y
    r2.x = ((r1.wwww)*(r16.xxxx)+(r16.yyyy)).x;
    // 211: mad r2.x, r2.x, r1.w, r16.z
    r2.x = ((r2.xxxx)*(r1.wwww)+(r16.zzzz)).x;
    // 212: mul r2.x, r1.w, r2.x
    r2.x = ((r1.wwww)*(r2.xxxx)).x;
    // 213: max r1.w, r1.w, r2.x
    r1.w = (max(r1.wwww,r2.xxxx)).w;
    // 214: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 215: dp3 r2.x, r11.xyzx, r5.xyzx
    r2.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 216: dp3 r2.y, r12.xyzx, r5.xyzx
    r2.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 217: dp3 r5.y, r10.xyzx, r5.xyzx
    r5.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 218: dp2 r5.x, r2.xyxx, r3.zwzz
    r5.x = (dot((r2.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 219: dp2 r5.z, r2.xyxx, cb0[15].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 220: sample_l_indexable(texturecube)(float,float,float,float) r10.xyzw, r5.xyzx, t6.xyzw, s6, r2.w
    r10.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 221: mul r2.xyw, r10.xyxz, r10.wwww
    r2.xyw = ((r10.xyxz)*(r10.wwww)).xyw;
    // 222: mul r2.xyw, r2.xyxw, cb0[14].xyxz
    r2.xyw = ((r2.xyxw)*(source[14].xyxz)).xyw;
    // 223: mad r2.xyw, r2.xyxw, l(6.000000, 6.000000, 0.000000, 6.000000), cb0[14].wwww
    r2.xyw = ((r2.xyxw)*(float4(6.000000,6.000000,0.000000,6.000000))+(source[14].wwww)).xyw;
    // 224: dp3 r3.z, r2.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.z = (dot((r2.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 225: div r3.z, r3.z, r5.w
    r3.z = ((r3.zzzz)/(r5.wwww)).z;
    // 226: mad r3.z, r6.w, l(5.000000), r3.z
    r3.z = ((r6.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.zzzz)).z;
    // 227: add_sat r3.z, r0.w, r3.z
    r3.z = (saturate((r0.wwww)+(r3.zzzz))).z;
    // 228: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 229: mad r3.w, r3.z, l(-2.000000), l(3.000000)
    r3.w = ((r3.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 230: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 231: mul r3.xyz, r3.xyzx, r3.xywx
    r3.xyz = ((r3.xyzx)*(r3.xywx)).xyz;
    // 232: log r3.z, r3.z
    r3.z = (log2(r3.zzzz)).z;
    // 233: mul r3.z, r3.z, l(1.500000)
    r3.z = ((r3.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 234: exp r3.z, r3.z
    r3.z = (exp2(r3.zzzz)).z;
    // 235: mul r2.xyw, r2.xyxw, r3.zzzz
    r2.xyw = ((r2.xyxw)*(r3.zzzz)).xyw;
    // 236: mul r5.xyz, r6.xyzx, r2.xywx
    r5.xyz = ((r6.xyzx)*(r2.xywx)).xyz;
    // 237: mul r2.xyw, r2.xyxw, r15.xyxz
    r2.xyw = ((r2.xyxw)*(r15.xyxz)).xyw;
    // 238: mad r5.xyz, r5.xyzx, r15.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r15.xyzx)+(r7.xyzx)).xyz;
    // 239: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 240: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 241: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 242: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 243: add_sat r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (saturate((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 244: add_sat r6.xyz, r6.xyzx, cb0[13].xxxx
    r6.xyz = (saturate((r6.xyzx)+(source[13].xxxx))).xyz;
    // 245: mul r3.w, r6.x, cb0[13].y
    r3.w = ((r6.xxxx)*(source[13].yyyy)).w;
    // 246: mul_sat r6.xyz, r6.xyzx, cb0[7].xyzx
    r6.xyz = (saturate((r6.xyzx)*(source[7].xyzx))).xyz;
    // 247: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 248: mul r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)*(r3.wwww)).xyz;
    // 249: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 250: mul r7.xyz, r0.wwww, r18.xyzx
    r7.xyz = ((r0.wwww)*(r18.xyzx)).xyz;
    // 251: mul r7.xyz, r13.xyzx, r7.xyzx
    r7.xyz = ((r13.xyzx)*(r7.xyzx)).xyz;
    // 252: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 253: mad r1.xyz, r2.xywx, r1.wwww, r1.xyzx
    r1.xyz = ((r2.xywx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 254: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 255: mul r2.xyw, r3.xyxz, r6.xyxz
    r2.xyw = ((r3.xyxz)*(r6.xyxz)).xyw;
    // 256: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 257: dp3 r0.w, r9.xyzx, r0.wwww
    r0.w = (dot((r9.xyzx).xyz,(r0.wwww).xyz).xxxx).w;
    // 258: mul r3.xyz, r0.wwww, r8.xyzx
    r3.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 259: mul r2.xyw, r0.xyxz, r2.xyxw
    r2.xyw = ((r0.xyxz)*(r2.xyxw)).xyw;
    // 260: mad r0.xyz, r0.xyzx, r3.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(r2.xywx)).xyz;
    // 261: dp3 r0.w, r4.xyzx, r14.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 262: add r1.w, -|r14.z|, l(1.000000)
    r1.w = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 263: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 264: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 265: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 266: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 267: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 268: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 269: mul r2.xyw, r0.wwww, cb0[4].xyxz
    r2.xyw = ((r0.wwww)*(source[4].xyxz)).xyw;
    // 270: movc r2.xyw, r1.wwww, l(0,0,0,0), r2.xyxw
    r2.xyw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyxw)).xyw;
    // 271: add r2.xyw, r2.xyxw, cb0[3].xyxz
    r2.xyw = ((r2.xyxw)+(source[3].xyxz)).xyw;
    // 272: add r0.w, -r2.z, l(1.000000)
    r0.w = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 273: mad r2.xyw, r1.xyxz, r0.wwww, r2.xyxw
    r2.xyw = ((r1.xyxz)*(r0.wwww)+(r2.xyxw)).xyw;
    // 274: mul r1.xyz, r2.zzzz, r1.xyzx
    r1.xyz = ((r2.zzzz)*(r1.xyzx)).xyz;
    // 275: mul r3.xyz, r2.zzzz, r0.xyzx
    r3.xyz = ((r2.zzzz)*(r0.xyzx)).xyz;
    // 276: mad r0.xyz, r0.xyzx, r0.wwww, r2.xywx
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r2.xywx)).xyz;
    // 277: add r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)+(r5.xyzx)).xyz;
    // 278: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 279: mad r0.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r17.xyzx
    r0.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r17.xyzx)).xyz;
    // 280: mad r0.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 281: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 282: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 283: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 284: ret
    return output;
}

// source.character.static-map-native-1106.v1 / source program 4f46c93dd2890248a377bed6d1c95b82
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1106(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1106(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: mul r0.xyz, cb0[6].xyzx, cb0[9].xxxx
    r0.xyz = ((source[6].xyzx)*(source[9].xxxx)).xyz;
    // 2: mul r1.xyz, cb0[5].xyzx, cb0[8].wwww
    r1.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r3.xyz, cb0[8].zzzz, r3.xyzx, r2.xyzx
    r3.xyz = ((source[8].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 8: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 10: mul r0.w, r2.z, cb0[9].y
    r0.w = ((r2.zzzz)*(source[9].yyyy)).w;
    // 11: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 12: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 13: mul r1.w, r1.w, cb0[9].z
    r1.w = ((r1.wwww)*(source[9].zzzz)).w;
    // 14: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 15: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 16: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 18: mul r1.xyz, r0.xyzx, cb0[9].wwww
    r1.xyz = ((r0.xyzx)*(source[9].wwww)).xyz;
    // 19: mad r0.xyz, cb0[10].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[10].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 20: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 21: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 22: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 23: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 24: mad r1.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 25: mad r3.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 26: mul r1.w, r2.x, cb0[12].y
    r1.w = ((r2.xxxx)*(source[12].yyyy)).w;
    // 27: mul r2.x, r2.y, cb0[11].w
    r2.x = ((r2.yyyy)*(source[11].wwww)).x;
    // 28: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 29: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 30: mul r2.y, r2.y, cb0[12].z
    r2.y = ((r2.yyyy)*(source[12].zzzz)).y;
    // 31: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 32: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: movc r1.w, r1.w, l(0), r2.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).w;
    // 34: mad r1.xyz, r1.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 35: mad r3.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 36: mad r1.xyz, r1.xyzx, r1.wwww, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 37: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 38: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 40: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 41: dp2 r3.x, r2.yzyy, r2.yzyy
    r3.x = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).x;
    // 42: mul r2.yz, r2.yyzy, cb0[8].xxxx
    r2.yz = ((r2.yyzy)*(source[8].xxxx)).yz;
    // 43: mul r4.xy, r2.yzyy, v2.wwww
    r4.xy = ((r2.yzyy)*(v2.wwww)).xy;
    // 44: add r2.y, -r3.x, l(1.000000)
    r2.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 45: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 46: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 47: add r4.z, r2.y, l(0.000010)
    r4.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 49: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 50: div r3.xyz, r4.xyzx, r2.yyyy
    r3.xyz = ((r4.xyzx)/(r2.yyyy)).xyz;
    // 51: dp3 r2.y, r3.xyzx, r3.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 52: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 53: mul r4.xyz, r2.yyyy, r3.xyzx
    r4.xyz = ((r2.yyyy)*(r3.xyzx)).xyz;
    // 54: dp3 r2.y, v7.xyzx, v7.xyzx
    r2.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 55: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 56: mul r5.xyz, r2.yyyy, v7.xyzx
    r5.xyz = ((r2.yyyy)*(v7.xyzx)).xyz;
    // 57: dp3 r2.y, r5.xyzx, r4.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 58: mad r2.yz, r2.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r2.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 59: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 60: mul r6.xyz, r2.zzzz, cb0[24].xyzx
    r6.xyz = ((r2.zzzz)*(source[24].xyzx)).xyz;
    // 61: mad r6.xyz, r2.yyyy, cb0[23].xyzx, r6.xyzx
    r6.xyz = ((r2.yyyy)*(source[23].xyzx)+(r6.xyzx)).xyz;
    // 62: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 63: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 64: mul r6.xyz, r1.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 65: dp3 r2.y, v1.xyzx, v1.xyzx
    r2.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 66: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 67: mul r7.xyz, r2.yyyy, v1.xyzx
    r7.xyz = ((r2.yyyy)*(v1.xyzx)).xyz;
    // 68: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 69: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 70: mul r8.xyz, r2.yyyy, v0.xyzx
    r8.xyz = ((r2.yyyy)*(v0.xyzx)).xyz;
    // 71: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 72: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 73: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 74: dp3 r10.y, r9.xyzx, r4.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 75: dp3 r10.x, r8.xyzx, r4.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 76: dp2 r11.z, r10.xyxx, cb0[15].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 77: mul r2.yz, cb0[15].yyxy, l(0.000000, 1.000000, -1.000000, 0.000000)
    r2.yz = ((source[15].yyxy)*(float4(0.000000,1.000000,-1.000000,0.000000))).yz;
    // 78: dp2 r11.x, r10.xyxx, r2.yzyy
    r11.x = (dot((r10.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 79: dp3 r11.y, r7.xyzx, r4.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 80: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 81: dp4 r10.x, cb0[16].xyzw, r11.xyzw
    r10.x = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 82: dp4 r10.y, cb0[17].xyzw, r11.xyzw
    r10.y = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 83: dp4 r10.z, cb0[18].xyzw, r11.xyzw
    r10.z = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 84: mul r12.xyzw, r11.yzzx, r11.xyzz
    r12.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 85: mul r3.w, r11.y, r11.y
    r3.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 86: mad r3.w, r11.x, r11.x, -r3.w
    r3.w = ((r11.xxxx)*(r11.xxxx)+(-(r3.wwww))).w;
    // 87: dp4 r11.x, cb0[19].xyzw, r12.xyzw
    r11.x = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 88: dp4 r11.y, cb0[20].xyzw, r12.xyzw
    r11.y = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 89: dp4 r11.z, cb0[21].xyzw, r12.xyzw
    r11.z = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 90: add r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)+(r11.xyzx)).xyz;
    // 91: mad r10.xyz, cb0[22].xyzx, r3.wwww, r10.xyzx
    r10.xyz = ((source[22].xyzx)*(r3.wwww)+(r10.xyzx)).xyz;
    // 92: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 93: mul r10.xyz, r10.xyzx, cb0[14].xyzx
    r10.xyz = ((r10.xyzx)*(source[14].xyzx)).xyz;
    // 94: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 95: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: log r4.w, |r2.x|
    r4.w = (log2(abs(r2.xxxx))).w;
    // 97: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 98: mul r4.w, r4.w, cb0[12].x
    r4.w = ((r4.wwww)*(source[12].xxxx)).w;
    // 99: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 100: movc r2.x, r2.x, l(0), r4.w
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).x;
    // 101: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 102: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: dp3 r4.w, v6.xyzx, v6.xyzx
    r4.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 104: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 105: mul r11.xyz, r4.wwww, v6.xyzx
    r11.xyz = ((r4.wwww)*(v6.xyzx)).xyz;
    // 106: dp3 r4.w, r4.xyzx, r11.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 107: deriv_rtx_coarse r12.x, r4.w
    r12.x = (ddx_coarse(r4.wwww)).x;
    // 108: deriv_rty_coarse r12.y, r4.w
    r12.y = (ddy_coarse(r4.wwww)).y;
    // 109: dp2 r5.w, r12.xyxx, r12.xyxx
    r5.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 110: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 111: mad_sat r12.y, r5.w, l(0.300000), r2.x
    r12.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.xxxx))).y;
    // 112: mad r2.x, r12.y, l(0.200000), l(0.200000)
    r2.x = ((r12.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 113: div r3.w, r3.w, r2.x
    r3.w = ((r3.wwww)/(r2.xxxx)).w;
    // 114: dp3 r5.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 115: mad r3.w, r5.w, l(5.000000), r3.w
    r3.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 116: add r6.w, r2.w, cb0[11].y
    r6.w = ((r2.wwww)+(source[11].yyyy)).w;
    // 117: add r2.w, r2.w, cb0[10].w
    r2.w = ((r2.wwww)+(source[10].wwww)).w;
    // 118: sample_b_indexable(texture2d)(float,float,float,float) r7.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r7.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 119: mad r6.w, r7.w, -r6.w, r6.w
    r6.w = ((r7.wwww)*(-(r6.wwww))+(r6.wwww)).w;
    // 120: add_sat r0.w, r0.w, r6.w
    r0.w = (saturate((r0.wwww)+(r6.wwww))).w;
    // 121: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 122: add_sat r3.w, r0.w, r3.w
    r3.w = (saturate((r0.wwww)+(r3.wwww))).w;
    // 123: mad r6.w, r3.w, l(-2.000000), l(3.000000)
    r6.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 124: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 125: mul r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)*(r6.wwww)).w;
    // 126: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 127: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 128: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 129: mul r10.xyz, r3.wwww, r10.xyzx
    r10.xyz = ((r3.wwww)*(r10.xyzx)).xyz;
    // 130: mul r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 131: mul r13.xyz, r4.wwww, r4.xyzx
    r13.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 132: dp3 r3.w, -r5.xyzx, r4.xyzx
    r3.w = (dot((-(r5.xyzx)).xyz,(r4.xyzx).xyz).xxxx).w;
    // 133: mad r4.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 134: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 135: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 136: add r3.w, r13.z, l(1.000000)
    r3.w = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: add r4.z, r4.w, l(1.000000)
    r4.z = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 139: mov_sat r4.w, r4.w
    r4.w = (saturate(r4.wwww)).w;
    // 140: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 141: mul r4.w, r4.w, cb0[1].y
    r4.w = ((r4.wwww)*(source[1].yyyy)).w;
    // 142: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 143: mad_sat r4.w, r4.w, cb0[1].w, cb0[1].z
    r4.w = (saturate((r4.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 144: add_sat r12.x, -r3.w, r4.z
    r12.x = (saturate((-(r3.wwww))+(r4.zzzz))).x;
    // 145: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t5.zwxy, s6
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 146: add r14.xy, -r12.yxyy, l(1.000000, 1.000000, 0.000000, 0.000000)
    r14.xy = ((-(r12.yxyy))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 147: add r3.w, r1.w, r12.x
    r3.w = ((r1.wwww)+(r12.xxxx)).w;
    // 148: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 149: mov_sat r4.z, cb0[10].y
    r4.z = (saturate(source[10].yyyy)).z;
    // 150: mad r15.xyz, -r4.zzzz, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r15.xyz = ((-(r4.zzzz))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 151: mul r4.z, r4.z, l(0.080000)
    r4.z = ((r4.zzzz)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 152: mad r15.xyz, r0.wwww, r15.xyzx, r4.zzzz
    r15.xyz = ((r0.wwww)*(r15.xyzx)+(r4.zzzz)).xyz;
    // 153: max r14.xzw, r14.xxxx, r15.xxyz
    r14.xzw = (max(r14.xxxx,r15.xxyz)).xzw;
    // 154: add r14.xzw, -r15.xxyz, r14.xxzw
    r14.xzw = ((-(r15.xxyz))+(r14.xxzw)).xzw;
    // 155: mul_sat r4.z, r15.y, l(50.000000)
    r4.z = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 156: mul r14.xzw, r4.zzzz, r14.xxzw
    r14.xzw = ((r4.zzzz)*(r14.xxzw)).xzw;
    // 157: mul r16.xyz, r12.wwww, r15.xyzx
    r16.xyz = ((r12.wwww)*(r15.xyzx)).xyz;
    // 158: mad r14.xzw, r14.xxzw, r12.zzzz, r16.xxyz
    r14.xzw = ((r14.xxzw)*(r12.zzzz)+(r16.xxyz)).xzw;
    // 159: div r6.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r6.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 160: add r6.w, r6.w, l(-1.000000)
    r6.w = ((r6.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 161: mad r12.xzw, r15.xxyz, r6.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r15.xxyz)*(r6.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 162: mad r16.xyz, -r14.xzwx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xzwx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 163: mul r12.xzw, r12.xxzw, r14.xxzw
    r12.xzw = ((r12.xxzw)*(r14.xxzw)).xzw;
    // 164: mul r6.w, r14.y, r14.y
    r6.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 165: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 166: mul r8.w, r14.y, r6.w
    r8.w = ((r14.yyyy)*(r6.wwww)).w;
    // 167: mad r6.w, -r6.w, r14.y, l(1.000000)
    r6.w = ((-(r6.wwww))*(r14.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: mul r14.xyz, r15.xyzx, r6.wwww
    r14.xyz = ((r15.xyzx)*(r6.wwww)).xyz;
    // 169: dp3 r6.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 170: mad r15.xyz, r6.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r6.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 171: mad r14.xyz, r4.zzzz, r8.wwww, r14.xyzx
    r14.xyz = ((r4.zzzz)*(r8.wwww)+(r14.xyzx)).xyz;
    // 172: add r14.xyz, -r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r14.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 173: mul r14.xyz, r14.xyzx, r14.xyzx
    r14.xyz = ((r14.xyzx)*(r14.xyzx)).xyz;
    // 174: add r4.z, -r2.w, l(1.000000)
    r4.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mad_sat r2.w, r7.w, r4.z, r2.w
    r2.w = (saturate((r7.wwww)*(r4.zzzz)+(r2.wwww))).w;
    // 176: mad r2.w, -r2.w, cb0[2].x, l(1.000000)
    r2.w = ((-(r2.wwww))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r4.z, r12.y, r12.y
    r4.z = ((r12.yyyy)*(r12.yyyy)).z;
    // 178: mul r6.w, r12.y, l(5.000000)
    r6.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 179: mad r7.w, r4.z, l(0.350000), l(1.000000)
    r7.w = ((r4.zzzz)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: mul r3.w, r3.w, r4.z
    r3.w = ((r3.wwww)*(r4.zzzz)).w;
    // 181: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 182: add r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)+(r3.wwww)).w;
    // 183: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 184: div_sat r2.w, r2.w, r7.w
    r2.w = (saturate((r2.wwww)/(r7.wwww))).w;
    // 185: mad r17.xyz, -r2.wwww, r14.xyzx, r16.xyzx
    r17.xyz = ((-(r2.wwww))*(r14.xyzx)+(r16.xyzx)).xyz;
    // 186: mul r16.xyz, r0.xyzx, r16.xyzx
    r16.xyz = ((r0.xyzx)*(r16.xyzx)).xyz;
    // 187: mul r14.xyz, r14.xyzx, r2.wwww
    r14.xyz = ((r14.xyzx)*(r2.wwww)).xyz;
    // 188: mul r14.xyz, r0.xyzx, r14.xyzx
    r14.xyz = ((r0.xyzx)*(r14.xyzx)).xyz;
    // 189: mad r14.xyz, -r14.xyzx, r0.wwww, r14.xyzx
    r14.xyz = ((-(r14.xyzx))*(r0.wwww)+(r14.xyzx)).xyz;
    // 190: mul r6.xyz, r6.xyzx, r17.xyzx
    r6.xyz = ((r6.xyzx)*(r17.xyzx)).xyz;
    // 191: mad r6.xyz, -r6.xyzx, r0.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r0.wwww)+(r6.xyzx)).xyz;
    // 192: dp3 r8.x, r8.xyzx, r13.xyzx
    r8.x = (dot((r8.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 193: dp3 r8.y, r9.xyzx, r13.xyzx
    r8.y = (dot((r9.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 194: dp2 r9.x, r8.xyxx, r2.yzyy
    r9.x = (dot((r8.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 195: dp2 r9.z, r8.xyxx, cb0[15].xyxx
    r9.z = (dot((r8.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 196: dp3 r9.y, r7.xyzx, r13.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 197: dp3 r2.y, r5.xyzx, r13.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 198: mad r2.yz, r2.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r2.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 199: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 200: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r9.xyzx, t6.xyzw, s5, r6.w
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r6.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 201: mul r5.xyz, r7.xyzx, r7.wwww
    r5.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 202: mul r5.xyz, r5.xyzx, cb0[14].xyzx
    r5.xyz = ((r5.xyzx)*(source[14].xyzx)).xyz;
    // 203: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 204: dp3 r3.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 205: div r2.x, r3.w, r2.x
    r2.x = ((r3.wwww)/(r2.xxxx)).x;
    // 206: mad r2.x, r5.w, l(5.000000), r2.x
    r2.x = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.xxxx)).x;
    // 207: add_sat r2.x, r0.w, r2.x
    r2.x = (saturate((r0.wwww)+(r2.xxxx))).x;
    // 208: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 209: mad r3.w, r2.x, l(-2.000000), l(3.000000)
    r3.w = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 210: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 211: mul r2.x, r2.x, r3.w
    r2.x = ((r2.xxxx)*(r3.wwww)).x;
    // 212: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 213: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 214: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 215: mul r5.xyz, r2.xxxx, r5.xyzx
    r5.xyz = ((r2.xxxx)*(r5.xyzx)).xyz;
    // 216: mad r2.x, r1.w, r15.x, r15.y
    r2.x = ((r1.wwww)*(r15.xxxx)+(r15.yyyy)).x;
    // 217: mad r2.x, r2.x, r1.w, r15.z
    r2.x = ((r2.xxxx)*(r1.wwww)+(r15.zzzz)).x;
    // 218: mul r2.x, r1.w, r2.x
    r2.x = ((r1.wwww)*(r2.xxxx)).x;
    // 219: max r1.w, r1.w, r2.x
    r1.w = (max(r1.wwww,r2.xxxx)).w;
    // 220: mul r7.xyz, r2.zzzz, cb0[24].xyzx
    r7.xyz = ((r2.zzzz)*(source[24].xyzx)).xyz;
    // 221: mad r2.xyz, cb0[23].xyzx, r2.yyyy, r7.xyzx
    r2.xyz = ((source[23].xyzx)*(r2.yyyy)+(r7.xyzx)).xyz;
    // 222: mul r2.xyz, r2.xyzx, cb0[25].wwww
    r2.xyz = ((r2.xyzx)*(source[25].wwww)).xyz;
    // 223: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 224: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 225: mul r5.xyz, r5.xyzx, r12.xzwx
    r5.xyz = ((r5.xyzx)*(r12.xzwx)).xyz;
    // 226: mad r2.xyz, r2.xyzx, r12.xzwx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r12.xzwx)+(r6.xyzx)).xyz;
    // 227: dp3 r3.x, r3.xyzx, r11.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 228: add r3.y, -|r11.z|, l(1.000000)
    r3.y = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 229: add r3.x, -|r3.x|, l(1.000000)
    r3.x = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 230: mul r3.x, r3.x, r3.y
    r3.x = ((r3.xxxx)*(r3.yyyy)).x;
    // 231: lt r3.y, |r3.x|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 232: log r3.x, |r3.x|
    r3.x = (log2(abs(r3.xxxx))).x;
    // 233: mul r3.x, r3.x, l(1.500000)
    r3.x = ((r3.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 234: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 235: mul r3.xzw, r3.xxxx, cb0[4].xxyz
    r3.xzw = ((r3.xxxx)*(source[4].xxyz)).xzw;
    // 236: movc r3.xyz, r3.yyyy, l(0,0,0,0), r3.xzwx
    r3.xyz = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xzwx)).xyz;
    // 237: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 238: mul r6.xyz, r0.wwww, r16.xyzx
    r6.xyz = ((r0.wwww)*(r16.xyzx)).xyz;
    // 239: mul r6.xyz, r10.xyzx, r6.xyzx
    r6.xyz = ((r10.xyzx)*(r6.xyzx)).xyz;
    // 240: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 241: mad r1.xyz, r5.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r5.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 242: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 243: add r1.w, -r2.w, l(1.000000)
    r1.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 244: mad r3.xyz, r1.xyzx, r1.wwww, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 245: mul r1.xyz, r2.wwww, r1.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)).xyz;
    // 246: mad r1.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r14.xyzx)).xyz;
    // 247: mul r5.xyz, r4.yyyy, cb0[24].xyzx
    r5.xyz = ((r4.yyyy)*(source[24].xyzx)).xyz;
    // 248: mad r4.xyz, r4.xxxx, cb0[23].xyzx, r5.xyzx
    r4.xyz = ((r4.xxxx)*(source[23].xyzx)+(r5.xyzx)).xyz;
    // 249: mul r4.xyz, r4.xyzx, cb0[25].wwww
    r4.xyz = ((r4.xyzx)*(source[25].wwww)).xyz;
    // 250: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 251: add_sat r5.xyz, -r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (saturate((-(r5.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 252: add_sat r5.xyz, r5.xyzx, cb0[13].xxxx
    r5.xyz = (saturate((r5.xyzx)+(source[13].xxxx))).xyz;
    // 253: mul r3.w, r5.x, cb0[13].y
    r3.w = ((r5.xxxx)*(source[13].yyyy)).w;
    // 254: mul_sat r5.xyz, r5.xyzx, cb0[7].xyzx
    r5.xyz = (saturate((r5.xyzx)*(source[7].xyzx))).xyz;
    // 255: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 256: mul r5.xyz, r5.xyzx, r3.wwww
    r5.xyz = ((r5.xyzx)*(r3.wwww)).xyz;
    // 257: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 258: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 259: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 260: mad r3.xyz, r0.xyzx, r1.wwww, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 261: mul r0.xyz, r2.wwww, r0.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)).xyz;
    // 262: mad r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 263: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 264: add r0.xyz, r2.xyzx, r3.xyzx
    r0.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 265: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 266: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 267: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 268: ret
    return output;
}

// source.character.static-map-native-1107.v1 / source program fc8ba647df66714a8700b3b13eced5e4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1107(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[10]=g_SourceCharacterBaseConstants[8];
    source[11]=g_SourceCharacterBaseConstants[9];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    source[25]=1.f;
    source[26]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
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
    // 10: dp3 r0.w, -v8.xyzx, -v8.xyzx
    r0.w = (dot((-(v8.xyzx)).xyz,(-(v8.xyzx)).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, -v8.xyzx
    r3.xyz = ((r0.wwww)*(-(v8.xyzx))).xyz;
    // 13: mul r4.xyz, r2.xyzx, r3.yyyy
    r4.xyz = ((r2.xyzx)*(r3.yyyy)).xyz;
    // 14: mad r3.xyw, r3.xxxx, r1.xyxz, r4.xyxz
    r3.xyw = ((r3.xxxx)*(r1.xyxz)+(r4.xyxz)).xyw;
    // 15: mad r3.xyz, r3.zzzz, r0.xyzx, r3.xywx
    r3.xyz = ((r3.zzzz)*(r0.xyzx)+(r3.xywx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 19: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 22: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r4.xy, r4.xyxx, cb0[7].xxxx
    r4.xy = ((r4.xyxx)*(source[7].xxxx)).xy;
    // 24: mul r5.xy, r4.xyxx, v6.wwww
    r5.xy = ((r4.xyxx)*(v6.wwww)).xy;
    // 25: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 26: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 27: div r4.xyw, r5.xyxz, r0.wwww
    r4.xyw = ((r5.xyxz)/(r0.wwww)).xyw;
    // 28: dp3 r0.w, r4.xywx, r4.xywx
    r0.w = (dot((r4.xywx).xyz,(r4.xywx).xyz).xxxx).w;
    // 29: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 30: mul r5.xyz, r0.wwww, r4.xywx
    r5.xyz = ((r0.wwww)*(r4.xywx)).xyz;
    // 31: lt r0.w, r5.z, l(0.999850)
    r0.w = (asfloat((uint4)((r5.zzzz)<(float4(0.999850,0.999850,0.999850,0.999850))) * 0xffffffffu)).w;
    // 32: movc r5.xyz, r0.wwww, r5.xyzx, l(0,0,1.000000,0)
    r5.xyz = ((asuint(r0.wwww) != 0u) ? (r5.xyzx) : (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u)))).xyz;
    // 33: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 34: mul r6.xyz, r0.wwww, r5.xyzx
    r6.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 35: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul_sat r1.w, r7.w, cb0[9].w
    r1.w = (saturate((r7.wwww)*(source[9].wwww))).w;
    // 38: mul r2.w, r1.w, cb0[1].x
    r2.w = ((r1.wwww)*(source[1].xxxx)).w;
    // 39: add r8.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 40: dp3 r9.x, r1.xyzx, r6.xyzx
    r9.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 41: dp3 r9.y, r2.xyzx, r6.xyzx
    r9.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 42: mul r9.zw, cb0[0].xxxy, l(0.000000, 0.000000, 0.000300, 0.000300)
    r9.zw = ((source[0].xxxy)*(float4(0.000000,0.000000,0.000300,0.000300))).zw;
    // 43: mad r9.zw, cb0[7].yyyy, r9.xxxy, r9.zzzw
    r9.zw = ((source[7].yyyy)*(r9.xxxy)+(r9.zzzw)).zw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, r9.zwzz, t2.xyzw, s1, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r9.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: mul r10.xyz, r10.xyzx, cb0[4].xyzx
    r10.xyz = ((r10.xyzx)*(source[4].xyzx)).xyz;
    // 46: mad r10.xyz, cb0[7].zzzz, r10.xyzx, r10.xyzx
    r10.xyz = ((source[7].zzzz)*(r10.xyzx)+(r10.xyzx)).xyz;
    // 47: add r10.xyz, r10.xyzx, -cb0[7].zzzz
    r10.xyz = ((r10.xyzx)+(-(source[7].zzzz))).xyz;
    // 48: mov_sat r11.xyz, r10.xyzx
    r11.xyz = (saturate(r10.xyzx)).xyz;
    // 49: mul r3.w, r4.z, cb0[7].w
    r3.w = ((r4.zzzz)*(source[7].wwww)).w;
    // 50: mul r12.xyz, cb0[5].xyzx, cb0[8].xxxx
    r12.xyz = ((source[5].xyzx)*(source[8].xxxx)).xyz;
    // 51: mul r12.xyz, r7.xyzx, r12.xyzx
    r12.xyz = ((r7.xyzx)*(r12.xyzx)).xyz;
    // 52: mul r13.xyz, cb0[6].xyzx, cb0[8].yyyy
    r13.xyz = ((source[6].xyzx)*(source[8].yyyy)).xyz;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r14.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r14.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 54: mul r4.z, r14.z, cb0[8].z
    r4.z = ((r14.zzzz)*(source[8].zzzz)).z;
    // 55: lt r5.w, |r4.z|, l(0.000001)
    r5.w = (asfloat((uint4)((abs(r4.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 56: log r4.z, |r4.z|
    r4.z = (log2(abs(r4.zzzz))).z;
    // 57: mul r4.z, r4.z, cb0[8].w
    r4.z = ((r4.zzzz)*(source[8].wwww)).z;
    // 58: exp r4.z, r4.z
    r4.z = (exp2(r4.zzzz)).z;
    // 59: movc r4.z, r5.w, l(0), r4.z
    r4.z = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.zzzz)).z;
    // 60: min r5.w, r4.z, l(1.000000)
    r5.w = (min(r4.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: mad r7.xyz, r13.xyzx, r7.xyzx, -r12.xyzx
    r7.xyz = ((r13.xyzx)*(r7.xyzx)+(-(r12.xyzx))).xyz;
    // 62: mad r7.xyz, r5.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 63: mad r7.xyz, r3.wwww, r11.xyzx, r7.xyzx
    r7.xyz = ((r3.wwww)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 64: mov_sat r10.xyz, -r10.xyzx
    r10.xyz = (saturate(-(r10.xyzx))).xyz;
    // 65: mad r10.xyz, -r3.wwww, r10.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r3.wwww))*(r10.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 66: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 67: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 68: min r7.xyz, r7.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 69: mul r10.xyz, r7.xyzx, cb0[9].xxxx
    r10.xyz = ((r7.xyzx)*(source[9].xxxx)).xyz;
    // 70: mad r7.xyz, cb0[9].yyyy, r7.xyzx, -r10.xyzx
    r7.xyz = ((source[9].yyyy)*(r7.xyzx)+(-(r10.xyzx))).xyz;
    // 71: mad r7.xyz, r5.wwww, r7.xyzx, r10.xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(r10.xyzx)).xyz;
    // 72: mul r7.xyz, r8.xyzx, r7.xyzx
    r7.xyz = ((r8.xyzx)*(r7.xyzx)).xyz;
    // 73: mad_sat r7.xyz, r7.xyzx, cb2[3].wwww, cb2[3].xyzx
    r7.xyz = (saturate((r7.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 74: dp3 r3.x, r4.xywx, r3.xyzx
    r3.x = (dot((r4.xywx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 75: add r3.xw, -|r3.xxxz|, l(1.000000, 0.000000, 0.000000, 1.000000)
    r3.xw = ((-(abs(r3.xxxz)))+(float4(1.000000,0.000000,0.000000,1.000000))).xw;
    // 76: mul r3.x, r3.x, r3.w
    r3.x = ((r3.xxxx)*(r3.wwww)).x;
    // 77: lt r3.y, |r3.x|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 78: log r3.x, |r3.x|
    r3.x = (log2(abs(r3.xxxx))).x;
    // 79: mul r3.x, r3.x, l(1.500000)
    r3.x = ((r3.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 80: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 81: mul r3.xzw, r3.xxxx, cb0[3].xxyz
    r3.xzw = ((r3.xxxx)*(source[3].xxyz)).xzw;
    // 82: movc r3.xyz, r3.yyyy, l(0,0,0,0), r3.xzwx
    r3.xyz = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xzwx)).xyz;
    // 83: add r3.xyz, r3.xyzx, cb0[2].xyzx
    r3.xyz = ((r3.xyzx)+(source[2].xyzx)).xyz;
    // 84: mul r4.xy, r14.yxyy, cb0[10].ywyy
    r4.xy = ((r14.yxyy)*(source[10].ywyy)).xy;
    // 85: lt r8.xy, |r4.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r8.xy = (asfloat((uint4)((abs(r4.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 86: log r4.xy, |r4.xyxx|
    r4.xy = (log2(abs(r4.xyxx))).xy;
    // 87: mul r3.w, r4.x, cb0[10].z
    r3.w = ((r4.xxxx)*(source[10].zzzz)).w;
    // 88: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 89: movc r3.w, r8.x, l(0), r3.w
    r3.w = ((asuint(r8.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 90: max r3.w, r3.w, cb0[0].w
    r3.w = (max(r3.wwww,source[0].wwww)).w;
    // 91: min r8.z, r3.w, l(1.000000)
    r8.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 92: mul_sat r8.w, r4.z, cb2[3].w
    r8.w = (saturate((r4.zzzz)*(passValues[3].wwww))).w;
    // 93: mov_sat r7.w, cb0[9].z
    r7.w = (saturate(source[9].zzzz)).w;
    // 94: mul r3.w, r4.y, cb0[11].x
    r3.w = ((r4.yyyy)*(source[11].xxxx)).w;
    // 95: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 96: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: movc r3.w, r8.y, l(0), r3.w
    r3.w = ((asuint(r8.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 98: dp2_sat r4.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 99: dp3_sat r4.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 100: dp3_sat r4.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 101: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 102: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t6.xyzw, s4
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 103: mul r10.xyz, r10.xyzx, cb0[25].xyzx
    r10.xyz = ((r10.xyzx)*(source[25].xyzx)).xyz;
    // 104: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t7.xyzw, s4
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 105: mul r11.xyz, r11.xyzx, cb0[26].xyzx
    r11.xyz = ((r11.xyzx)*(source[26].xyzx)).xyz;
    // 106: dp3 r4.x, r11.xyzx, r4.xyzx
    r4.x = (dot((r11.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 107: mul r4.yzw, r4.xxxx, r10.xxyz
    r4.yzw = ((r4.xxxx)*(r10.xxyz)).yzw;
    // 108: dp3 r5.w, v7.xyzx, v7.xyzx
    r5.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 109: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 110: mul r11.xyz, r5.wwww, v7.xyzx
    r11.xyz = ((r5.wwww)*(v7.xyzx)).xyz;
    // 111: dp3 r5.w, r11.xyzx, r5.xyzx
    r5.w = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 112: mad r8.xy, r5.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r5.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 113: mul r8.xy, r8.xyxx, r8.xyxx
    r8.xy = ((r8.xyxx)*(r8.xyxx)).xy;
    // 114: mul r11.xyz, r8.yyyy, cb0[23].xyzx
    r11.xyz = ((r8.yyyy)*(source[23].xyzx)).xyz;
    // 115: mad r11.xyz, r8.xxxx, cb0[22].xyzx, r11.xyzx
    r11.xyz = ((r8.xxxx)*(source[22].xyzx)+(r11.xyzx)).xyz;
    // 116: mul r11.xyz, r11.xyzx, cb0[24].wwww
    r11.xyz = ((r11.xyzx)*(source[24].wwww)).xyz;
    // 117: mul r12.xyz, r7.xyzx, r11.xyzx
    r12.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 118: mad r12.xyz, r7.xyzx, r4.yzwy, r12.xyzx
    r12.xyz = ((r7.xyzx)*(r4.yzwy)+(r12.xyzx)).xyz;
    // 119: mad r10.xyz, r10.xyzx, r4.xxxx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r4.xxxx)+(r11.xyzx)).xyz;
    // 120: mul r4.x, r7.w, l(0.080000)
    r4.x = ((r7.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 121: mad r11.xyz, -r7.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r7.xyzx
    r11.xyz = ((-(r7.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r7.xyzx)).xyz;
    // 122: mad r11.xyz, r8.wwww, r11.xyzx, r4.xxxx
    r11.xyz = ((r8.wwww)*(r11.xyzx)+(r4.xxxx)).xyz;
    // 123: deriv_rtx_coarse r8.x, r0.w
    r8.x = (ddx_coarse(r0.wwww)).x;
    // 124: deriv_rty_coarse r8.y, r0.w
    r8.y = (ddy_coarse(r0.wwww)).y;
    // 125: dp2 r4.x, r8.xyxx, r8.xyxx
    r4.x = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 126: sqrt r4.x, r4.x
    r4.x = (sqrt(r4.xxxx)).x;
    // 127: mad_sat r8.y, r4.x, l(0.300000), r8.z
    r8.y = (saturate((r4.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 128: add r4.x, r6.z, l(1.000000)
    r4.x = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 129: min r4.x, r4.x, l(1.000000)
    r4.x = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 130: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: add_sat r8.x, -r4.x, r0.w
    r8.x = (saturate((-(r4.xxxx))+(r0.wwww))).x;
    // 132: sample_indexable(texture2d)(float,float,float,float) r9.zw, r8.xyxx, t4.zwxy, s6
    r9.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 133: add r0.w, -r8.y, l(1.000000)
    r0.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: max r13.xyz, r11.xyzx, r0.wwww
    r13.xyz = (max(r11.xyzx,r0.wwww)).xyz;
    // 135: add r13.xyz, -r11.xyzx, r13.xyzx
    r13.xyz = ((-(r11.xyzx))+(r13.xyzx)).xyz;
    // 136: mul_sat r0.w, r11.y, l(50.000000)
    r0.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 137: mul r13.xyz, r0.wwww, r13.xyzx
    r13.xyz = ((r0.wwww)*(r13.xyzx)).xyz;
    // 138: mul r14.xyz, r9.wwww, r11.xyzx
    r14.xyz = ((r9.wwww)*(r11.xyzx)).xyz;
    // 139: mad r13.xyz, r13.xyzx, r9.zzzz, r14.xyzx
    r13.xyz = ((r13.xyzx)*(r9.zzzz)+(r14.xyzx)).xyz;
    // 140: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r9.w
    r0.w = r9.w != 0.f ? 1.f / r9.w : 0.f;
    // 141: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 142: mad r14.xyz, r11.xyzx, r0.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((r11.xyzx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 143: mul r15.xyz, r13.xyzx, r14.xyzx
    r15.xyz = ((r13.xyzx)*(r14.xyzx)).xyz;
    // 144: dp3 r6.y, r0.xyzx, r6.xyzx
    r6.y = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 145: mul r0.w, r8.y, l(5.000000)
    r0.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 146: mul r9.zw, cb0[13].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r9.zw = ((source[13].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 147: dp2 r6.x, r9.xyxx, r9.zwzz
    r6.x = (dot((r9.xyxx).xy,(r9.zwzz).xy).xxxx).x;
    // 148: dp2 r6.z, r9.xyxx, cb0[13].xyxx
    r6.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 149: sample_l_indexable(texturecube)(float,float,float,float) r6.xyzw, r6.xyzx, t5.xyzw, s5, r0.w
    r6.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r0.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 150: mul r6.xyz, r6.xyzx, r6.wwww
    r6.xyz = ((r6.xyzx)*(r6.wwww)).xyz;
    // 151: mul r6.xyz, r6.xyzx, cb0[12].xyzx
    r6.xyz = ((r6.xyzx)*(source[12].xyzx)).xyz;
    // 152: mad r6.xyz, r6.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r6.xyz = ((r6.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 153: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 154: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 155: dp3 r0.y, r0.xyzx, r5.xyzx
    r0.y = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 156: dp2 r0.x, r1.xyxx, r9.zwzz
    r0.x = (dot((r1.xyxx).xy,(r9.zwzz).xy).xxxx).x;
    // 157: dp2 r0.z, r1.xyxx, cb0[13].xyxx
    r0.z = (dot((r1.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 158: mov r0.w, l(1.000000)
    r0.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 159: dp4 r2.x, cb0[14].xyzw, r0.xyzw
    r2.x = (dot((source[14].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).x;
    // 160: dp4 r2.y, cb0[15].xyzw, r0.xyzw
    r2.y = (dot((source[15].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).y;
    // 161: dp4 r2.z, cb0[16].xyzw, r0.xyzw
    r2.z = (dot((source[16].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).z;
    // 162: mul r5.xyzw, r0.yzzx, r0.xyzz
    r5.xyzw = ((r0.yzzx)*(r0.xyzz)).xyzw;
    // 163: dp4 r9.x, cb0[17].xyzw, r5.xyzw
    r9.x = (dot((source[17].xyzw).xyzw,(r5.xyzw).xyzw).xxxx).x;
    // 164: dp4 r9.y, cb0[18].xyzw, r5.xyzw
    r9.y = (dot((source[18].xyzw).xyzw,(r5.xyzw).xyzw).xxxx).y;
    // 165: dp4 r9.z, cb0[19].xyzw, r5.xyzw
    r9.z = (dot((source[19].xyzw).xyzw,(r5.xyzw).xyzw).xxxx).z;
    // 166: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 167: mad r0.x, r0.x, r0.x, -r0.z
    r0.x = ((r0.xxxx)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 168: add r2.xyz, r2.xyzx, r9.xyzx
    r2.xyz = ((r2.xyzx)+(r9.xyzx)).xyz;
    // 169: mad r0.xzw, cb0[20].xxyz, r0.xxxx, r2.xxyz
    r0.xzw = ((source[20].xxyz)*(r0.xxxx)+(r2.xxyz)).xzw;
    // 170: max r0.xzw, r0.xxzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xzw = (max(r0.xxzw,float4(0.000000,0.000000,0.000000,0.000000))).xzw;
    // 171: mul r0.xzw, r0.xxzw, cb0[12].xxyz
    r0.xzw = ((r0.xxzw)*(source[12].xxyz)).xzw;
    // 172: mad r0.xzw, r0.xxzw, l(3.141593, 0.000000, 3.141593, 3.141593), cb0[12].wwww
    r0.xzw = ((r0.xxzw)*(float4(3.141593,0.000000,3.141593,3.141593))+(source[12].wwww)).xzw;
    // 173: mul r2.x, r8.y, r8.y
    r2.x = ((r8.yyyy)*(r8.yyyy)).x;
    // 174: dp3 r2.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 175: add r2.z, r3.w, r8.x
    r2.z = ((r3.wwww)+(r8.xxxx)).z;
    // 176: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 177: mul r2.x, r2.z, r2.x
    r2.x = ((r2.zzzz)*(r2.xxxx)).x;
    // 178: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 179: add r2.x, r3.w, r2.x
    r2.x = ((r3.wwww)+(r2.xxxx)).x;
    // 180: add_sat r2.x, r2.x, l(-1.000000)
    r2.x = (saturate((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 181: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 182: mad r2.y, r2.x, r5.x, r5.y
    r2.y = ((r2.xxxx)*(r5.xxxx)+(r5.yyyy)).y;
    // 183: mad r2.y, r2.y, r2.x, r5.z
    r2.y = ((r2.yyyy)*(r2.xxxx)+(r5.zzzz)).y;
    // 184: mul r2.y, r2.x, r2.y
    r2.y = ((r2.xxxx)*(r2.yyyy)).y;
    // 185: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 186: mad r5.xyz, r7.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r7.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 187: mad r9.xyz, r7.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r7.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 188: mad r11.xyz, r7.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r11.xyz = ((r7.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 189: mad r5.xyz, r3.wwww, r5.xyzx, r9.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(r9.xyzx)).xyz;
    // 190: mad r5.xyz, r5.xyzx, r3.wwww, r11.xyzx
    r5.xyz = ((r5.xyzx)*(r3.wwww)+(r11.xyzx)).xyz;
    // 191: mul r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 192: max r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = (max(r3.wwww,r5.xyzx)).xyz;
    // 193: mul r2.xyz, r2.xxxx, r10.xyzx
    r2.xyz = ((r2.xxxx)*(r10.xyzx)).xyz;
    // 194: mul r5.xyz, r5.xyzx, r12.xyzx
    r5.xyz = ((r5.xyzx)*(r12.xyzx)).xyz;
    // 195: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 196: mul r6.xyz, r15.xyzx, r2.xyzx
    r6.xyz = ((r15.xyzx)*(r2.xyzx)).xyz;
    // 197: mad r9.xyz, -r13.xyzx, r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r13.xyzx))*(r14.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mul r0.xzw, r0.xxzw, r9.xxyz
    r0.xzw = ((r0.xxzw)*(r9.xxyz)).xzw;
    // 199: mul r0.xzw, r0.xxzw, r5.xxyz
    r0.xzw = ((r0.xxzw)*(r5.xxyz)).xzw;
    // 200: mad r0.xzw, -r0.xxzw, r8.wwww, r0.xxzw
    r0.xzw = ((-(r0.xxzw))*(r8.wwww)+(r0.xxzw)).xzw;
    // 201: mad r0.xzw, r2.xxyz, r15.xxyz, r0.xxzw
    r0.xzw = ((r2.xxyz)*(r15.xxyz)+(r0.xxzw)).xzw;
    // 202: add r2.xyz, r0.xzwx, r3.xyzx
    r2.xyz = ((r0.xzwx)+(r3.xyzx)).xyz;
    // 203: mad r2.xyz, r7.xyzx, cb0[24].xyzx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(source[24].xyzx)+(r2.xyzx)).xyz;
    // 204: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 205: mad r3.x, r3.x, l(-0.250000), l(0.400000)
    r3.x = ((r3.xxxx)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).x;
    // 206: eq r3.y, cb0[27].x, l(0.000000)
    r3.y = (asfloat((uint4)((source[27].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 207: not r3.z, r3.y
    r3.z = (asfloat(~asuint(r3.yyyy))).z;
    // 208: lt r4.x, r2.w, r3.x
    r4.x = (asfloat((uint4)((r2.wwww)<(r3.xxxx)) * 0xffffffffu)).x;
    // 209: and r3.z, r3.z, r4.x
    r3.z = (asfloat(asuint(r3.zzzz) & asuint(r4.xxxx))).z;
    // 210: discard_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 211: ge r3.x, r2.w, r3.x
    r3.x = (asfloat((uint4)((r2.wwww)>=(r3.xxxx)) * 0xffffffffu)).x;
    // 212: mad r1.w, r1.w, cb0[1].x, l(-0.900000)
    r1.w = ((r1.wwww)*(source[1].xxxx)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 213: mul_sat r1.w, r1.w, l(9.999998)
    r1.w = (saturate((r1.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 214: mad r3.z, r1.w, l(-2.000000), l(3.000000)
    r3.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 215: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 216: mul r1.w, r1.w, r3.z
    r1.w = ((r1.wwww)*(r3.zzzz)).w;
    // 217: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 218: movc r1.w, r3.x, r1.w, r2.w
    r1.w = ((asuint(r3.xxxx) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 219: movc o0.w, r3.y, r1.w, r2.w
    output.targets[0].w = ((asuint(r3.yyyy) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 220: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 221: mov r1.z, r0.y
    r1.z = (r0.yyyy).z;
    // 222: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 223: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 224: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 225: dp3 r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r0.y = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).y;
    // 226: div r1.xy, r1.xyxx, r0.yyyy
    r1.xy = ((r1.xyxx)/(r0.yyyy)).xy;
    // 227: ge r0.y, l(0.000000), r1.z
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).y;
    // 228: ge r1.zw, r1.xxxy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.zw = (asfloat((uint4)((r1.xxxy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).zw;
    // 229: movc r1.zw, r1.zzzw, l(0,0,1.000000,1.000000), l(0,0,-1.000000,-1.000000)
    r1.zw = ((asuint(r1.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),1.000000,1.000000)) : (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000))).zw;
    // 230: mad r1.zw, -|r1.yyyx|, r1.zzzw, r1.zzzw
    r1.zw = ((-(abs(r1.yyyx)))*(r1.zzzw)+(r1.zzzw)).zw;
    // 231: movc r1.xy, r0.yyyy, r1.zwzz, r1.xyxx
    r1.xy = ((asuint(r0.yyyy) != 0u) ? (r1.zwzz) : (r1.xyxx)).xy;
    // 232: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 233: dp3 o4.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 234: dp3 o4.y, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 235: add r0.yzw, r10.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r10.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 236: div r0.yzw, r4.yyzw, r0.yyzw
    r0.yzw = ((r4.yyzw)/(r0.yyzw)).yzw;
    // 237: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 238: mul o4.z, r0.y, r0.x
    output.targets[4].z = ((r0.yyyy)*(r0.xxxx)).z;
    // 239: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 240: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 241: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 242: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 243: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 244: mov o3.xyzw, r7.xyzw
    output.targets[3].xyzw = (r7.xyzw).xyzw;
    // 245: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 246: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 247: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 248: ret
    return output;
}

// source.character.static-map-native-1107.v1 / source program cdb0edf49388f047a43ad6476f4b91c9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1107(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1107(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[10]=g_SourceCharacterBaseConstants[8];
    source[11]=g_SourceCharacterBaseConstants[9];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
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
    // 10: dp3 r0.w, -v8.xyzx, -v8.xyzx
    r0.w = (dot((-(v8.xyzx)).xyz,(-(v8.xyzx)).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, -v8.xyzx
    r3.xyz = ((r0.wwww)*(-(v8.xyzx))).xyz;
    // 13: mul r4.xyz, r2.xyzx, r3.yyyy
    r4.xyz = ((r2.xyzx)*(r3.yyyy)).xyz;
    // 14: mad r3.xyw, r3.xxxx, r1.xyxz, r4.xyxz
    r3.xyw = ((r3.xxxx)*(r1.xyxz)+(r4.xyxz)).xyw;
    // 15: mad r3.xyz, r3.zzzz, r0.xyzx, r3.xywx
    r3.xyz = ((r3.zzzz)*(r0.xyzx)+(r3.xywx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 19: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 22: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r4.xy, r4.xyxx, cb0[7].xxxx
    r4.xy = ((r4.xyxx)*(source[7].xxxx)).xy;
    // 24: mul r5.xy, r4.xyxx, v6.wwww
    r5.xy = ((r4.xyxx)*(v6.wwww)).xy;
    // 25: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 26: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 27: div r4.xyw, r5.xyxz, r0.wwww
    r4.xyw = ((r5.xyxz)/(r0.wwww)).xyw;
    // 28: dp3 r0.w, r4.xywx, r4.xywx
    r0.w = (dot((r4.xywx).xyz,(r4.xywx).xyz).xxxx).w;
    // 29: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 30: mul r5.xyz, r0.wwww, r4.xywx
    r5.xyz = ((r0.wwww)*(r4.xywx)).xyz;
    // 31: lt r0.w, r5.z, l(0.999850)
    r0.w = (asfloat((uint4)((r5.zzzz)<(float4(0.999850,0.999850,0.999850,0.999850))) * 0xffffffffu)).w;
    // 32: movc r5.xyz, r0.wwww, r5.xyzx, l(0,0,1.000000,0)
    r5.xyz = ((asuint(r0.wwww) != 0u) ? (r5.xyzx) : (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u)))).xyz;
    // 33: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 34: mul r6.xyz, r0.wwww, r5.xyzx
    r6.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 35: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul_sat r1.w, r7.w, cb0[9].w
    r1.w = (saturate((r7.wwww)*(source[9].wwww))).w;
    // 38: mul r2.w, r1.w, cb0[1].x
    r2.w = ((r1.wwww)*(source[1].xxxx)).w;
    // 39: add r8.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 40: dp3 r9.x, r1.xyzx, r6.xyzx
    r9.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 41: dp3 r9.y, r2.xyzx, r6.xyzx
    r9.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 42: mul r9.zw, cb0[0].xxxy, l(0.000000, 0.000000, 0.000300, 0.000300)
    r9.zw = ((source[0].xxxy)*(float4(0.000000,0.000000,0.000300,0.000300))).zw;
    // 43: mad r9.zw, cb0[7].yyyy, r9.xxxy, r9.zzzw
    r9.zw = ((source[7].yyyy)*(r9.xxxy)+(r9.zzzw)).zw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, r9.zwzz, t2.xyzw, s1, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r9.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: mul r10.xyz, r10.xyzx, cb0[4].xyzx
    r10.xyz = ((r10.xyzx)*(source[4].xyzx)).xyz;
    // 46: mad r10.xyz, cb0[7].zzzz, r10.xyzx, r10.xyzx
    r10.xyz = ((source[7].zzzz)*(r10.xyzx)+(r10.xyzx)).xyz;
    // 47: add r10.xyz, r10.xyzx, -cb0[7].zzzz
    r10.xyz = ((r10.xyzx)+(-(source[7].zzzz))).xyz;
    // 48: mov_sat r11.xyz, r10.xyzx
    r11.xyz = (saturate(r10.xyzx)).xyz;
    // 49: mul r3.w, r4.z, cb0[7].w
    r3.w = ((r4.zzzz)*(source[7].wwww)).w;
    // 50: mul r12.xyz, cb0[5].xyzx, cb0[8].xxxx
    r12.xyz = ((source[5].xyzx)*(source[8].xxxx)).xyz;
    // 51: mul r12.xyz, r7.xyzx, r12.xyzx
    r12.xyz = ((r7.xyzx)*(r12.xyzx)).xyz;
    // 52: mul r13.xyz, cb0[6].xyzx, cb0[8].yyyy
    r13.xyz = ((source[6].xyzx)*(source[8].yyyy)).xyz;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r14.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r14.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 54: mul r4.z, r14.z, cb0[8].z
    r4.z = ((r14.zzzz)*(source[8].zzzz)).z;
    // 55: lt r5.w, |r4.z|, l(0.000001)
    r5.w = (asfloat((uint4)((abs(r4.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 56: log r4.z, |r4.z|
    r4.z = (log2(abs(r4.zzzz))).z;
    // 57: mul r4.z, r4.z, cb0[8].w
    r4.z = ((r4.zzzz)*(source[8].wwww)).z;
    // 58: exp r4.z, r4.z
    r4.z = (exp2(r4.zzzz)).z;
    // 59: movc r4.z, r5.w, l(0), r4.z
    r4.z = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.zzzz)).z;
    // 60: min r5.w, r4.z, l(1.000000)
    r5.w = (min(r4.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: mad r7.xyz, r13.xyzx, r7.xyzx, -r12.xyzx
    r7.xyz = ((r13.xyzx)*(r7.xyzx)+(-(r12.xyzx))).xyz;
    // 62: mad r7.xyz, r5.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 63: mad r7.xyz, r3.wwww, r11.xyzx, r7.xyzx
    r7.xyz = ((r3.wwww)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 64: mov_sat r10.xyz, -r10.xyzx
    r10.xyz = (saturate(-(r10.xyzx))).xyz;
    // 65: mad r10.xyz, -r3.wwww, r10.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r3.wwww))*(r10.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 66: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 67: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 68: min r7.xyz, r7.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 69: mul r10.xyz, r7.xyzx, cb0[9].xxxx
    r10.xyz = ((r7.xyzx)*(source[9].xxxx)).xyz;
    // 70: mad r7.xyz, cb0[9].yyyy, r7.xyzx, -r10.xyzx
    r7.xyz = ((source[9].yyyy)*(r7.xyzx)+(-(r10.xyzx))).xyz;
    // 71: mad r7.xyz, r5.wwww, r7.xyzx, r10.xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(r10.xyzx)).xyz;
    // 72: mul r7.xyz, r8.xyzx, r7.xyzx
    r7.xyz = ((r8.xyzx)*(r7.xyzx)).xyz;
    // 73: mad_sat r7.xyz, r7.xyzx, cb2[3].wwww, cb2[3].xyzx
    r7.xyz = (saturate((r7.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 74: dp3 r3.x, r4.xywx, r3.xyzx
    r3.x = (dot((r4.xywx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 75: add r3.xw, -|r3.xxxz|, l(1.000000, 0.000000, 0.000000, 1.000000)
    r3.xw = ((-(abs(r3.xxxz)))+(float4(1.000000,0.000000,0.000000,1.000000))).xw;
    // 76: mul r3.x, r3.x, r3.w
    r3.x = ((r3.xxxx)*(r3.wwww)).x;
    // 77: lt r3.y, |r3.x|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 78: log r3.x, |r3.x|
    r3.x = (log2(abs(r3.xxxx))).x;
    // 79: mul r3.x, r3.x, l(1.500000)
    r3.x = ((r3.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 80: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 81: mul r3.xzw, r3.xxxx, cb0[3].xxyz
    r3.xzw = ((r3.xxxx)*(source[3].xxyz)).xzw;
    // 82: movc r3.xyz, r3.yyyy, l(0,0,0,0), r3.xzwx
    r3.xyz = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xzwx)).xyz;
    // 83: add r3.xyz, r3.xyzx, cb0[2].xyzx
    r3.xyz = ((r3.xyzx)+(source[2].xyzx)).xyz;
    // 84: mul r4.xy, r14.yxyy, cb0[10].ywyy
    r4.xy = ((r14.yxyy)*(source[10].ywyy)).xy;
    // 85: lt r8.xy, |r4.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r8.xy = (asfloat((uint4)((abs(r4.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 86: log r4.xy, |r4.xyxx|
    r4.xy = (log2(abs(r4.xyxx))).xy;
    // 87: mul r3.w, r4.x, cb0[10].z
    r3.w = ((r4.xxxx)*(source[10].zzzz)).w;
    // 88: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 89: movc r3.w, r8.x, l(0), r3.w
    r3.w = ((asuint(r8.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 90: max r3.w, r3.w, cb0[0].w
    r3.w = (max(r3.wwww,source[0].wwww)).w;
    // 91: min r8.z, r3.w, l(1.000000)
    r8.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 92: mul_sat r8.w, r4.z, cb2[3].w
    r8.w = (saturate((r4.zzzz)*(passValues[3].wwww))).w;
    // 93: mov_sat r7.w, cb0[9].z
    r7.w = (saturate(source[9].zzzz)).w;
    // 94: mul r3.w, r4.y, cb0[11].x
    r3.w = ((r4.yyyy)*(source[11].xxxx)).w;
    // 95: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 96: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: movc r3.w, r8.y, l(0), r3.w
    r3.w = ((asuint(r8.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 98: dp3 r4.x, v7.xyzx, v7.xyzx
    r4.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 99: rsq r4.x, r4.x
    r4.x = (rsqrt(r4.xxxx)).x;
    // 100: mul r4.xyz, r4.xxxx, v7.xyzx
    r4.xyz = ((r4.xxxx)*(v7.xyzx)).xyz;
    // 101: dp3 r4.w, r4.xyzx, r5.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 102: mad r8.xy, r4.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r4.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 103: mul r8.xy, r8.xyxx, r8.xyxx
    r8.xy = ((r8.xyxx)*(r8.xyxx)).xy;
    // 104: mul r10.xyz, r8.yyyy, cb0[23].xyzx
    r10.xyz = ((r8.yyyy)*(source[23].xyzx)).xyz;
    // 105: mad r10.xyz, r8.xxxx, cb0[22].xyzx, r10.xyzx
    r10.xyz = ((r8.xxxx)*(source[22].xyzx)+(r10.xyzx)).xyz;
    // 106: mul r10.xyz, r10.xyzx, cb0[24].wwww
    r10.xyz = ((r10.xyzx)*(source[24].wwww)).xyz;
    // 107: mul r10.xyz, r7.xyzx, r10.xyzx
    r10.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 108: dp3 r4.x, r4.xyzx, r6.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 109: mad r4.xy, r4.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r4.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 110: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 111: mul r4.yzw, r4.yyyy, cb0[23].xxyz
    r4.yzw = ((r4.yyyy)*(source[23].xxyz)).yzw;
    // 112: mad r4.xyz, cb0[22].xyzx, r4.xxxx, r4.yzwy
    r4.xyz = ((source[22].xyzx)*(r4.xxxx)+(r4.yzwy)).xyz;
    // 113: mul r4.xyz, r4.xyzx, cb0[24].wwww
    r4.xyz = ((r4.xyzx)*(source[24].wwww)).xyz;
    // 114: mul r4.w, r7.w, l(0.080000)
    r4.w = ((r7.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 115: mad r11.xyz, -r7.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r7.xyzx
    r11.xyz = ((-(r7.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r7.xyzx)).xyz;
    // 116: mad r11.xyz, r8.wwww, r11.xyzx, r4.wwww
    r11.xyz = ((r8.wwww)*(r11.xyzx)+(r4.wwww)).xyz;
    // 117: deriv_rtx_coarse r8.x, r0.w
    r8.x = (ddx_coarse(r0.wwww)).x;
    // 118: deriv_rty_coarse r8.y, r0.w
    r8.y = (ddy_coarse(r0.wwww)).y;
    // 119: dp2 r4.w, r8.xyxx, r8.xyxx
    r4.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 120: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 121: mad_sat r8.y, r4.w, l(0.300000), r8.z
    r8.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 122: add r4.w, r6.z, l(1.000000)
    r4.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: add_sat r8.x, -r4.w, r0.w
    r8.x = (saturate((-(r4.wwww))+(r0.wwww))).x;
    // 126: sample_indexable(texture2d)(float,float,float,float) r9.zw, r8.xyxx, t4.zwxy, s5
    r9.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 127: add r0.w, -r8.y, l(1.000000)
    r0.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: max r12.xyz, r11.xyzx, r0.wwww
    r12.xyz = (max(r11.xyzx,r0.wwww)).xyz;
    // 129: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 130: mul_sat r0.w, r11.y, l(50.000000)
    r0.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 131: mul r12.xyz, r0.wwww, r12.xyzx
    r12.xyz = ((r0.wwww)*(r12.xyzx)).xyz;
    // 132: mul r13.xyz, r9.wwww, r11.xyzx
    r13.xyz = ((r9.wwww)*(r11.xyzx)).xyz;
    // 133: mad r12.xyz, r12.xyzx, r9.zzzz, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r9.zzzz)+(r13.xyzx)).xyz;
    // 134: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r9.w
    r0.w = r9.w != 0.f ? 1.f / r9.w : 0.f;
    // 135: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 136: mad r13.xyz, r11.xyzx, r0.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r11.xyzx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 137: mul r14.xyz, r12.xyzx, r13.xyzx
    r14.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 138: dp3 r6.y, r0.xyzx, r6.xyzx
    r6.y = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 139: mul r0.w, r8.y, l(5.000000)
    r0.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 140: mul r9.zw, cb0[13].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r9.zw = ((source[13].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 141: dp2 r6.x, r9.xyxx, r9.zwzz
    r6.x = (dot((r9.xyxx).xy,(r9.zwzz).xy).xxxx).x;
    // 142: dp2 r6.z, r9.xyxx, cb0[13].xyxx
    r6.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 143: sample_l_indexable(texturecube)(float,float,float,float) r6.xyzw, r6.xyzx, t5.xyzw, s4, r0.w
    r6.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r0.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 144: mul r6.xyz, r6.xyzx, r6.wwww
    r6.xyz = ((r6.xyzx)*(r6.wwww)).xyz;
    // 145: mul r6.xyz, r6.xyzx, cb0[12].xyzx
    r6.xyz = ((r6.xyzx)*(source[12].xyzx)).xyz;
    // 146: mad r6.xyz, r6.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r6.xyz = ((r6.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 147: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 148: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 149: dp3 r0.y, r0.xyzx, r5.xyzx
    r0.y = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 150: dp2 r0.x, r1.xyxx, r9.zwzz
    r0.x = (dot((r1.xyxx).xy,(r9.zwzz).xy).xxxx).x;
    // 151: dp2 r0.z, r1.xyxx, cb0[13].xyxx
    r0.z = (dot((r1.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 152: mov r0.w, l(1.000000)
    r0.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 153: dp4 r2.x, cb0[14].xyzw, r0.xyzw
    r2.x = (dot((source[14].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).x;
    // 154: dp4 r2.y, cb0[15].xyzw, r0.xyzw
    r2.y = (dot((source[15].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).y;
    // 155: dp4 r2.z, cb0[16].xyzw, r0.xyzw
    r2.z = (dot((source[16].xyzw).xyzw,(r0.xyzw).xyzw).xxxx).z;
    // 156: mul r5.xyzw, r0.yzzx, r0.xyzz
    r5.xyzw = ((r0.yzzx)*(r0.xyzz)).xyzw;
    // 157: dp4 r9.x, cb0[17].xyzw, r5.xyzw
    r9.x = (dot((source[17].xyzw).xyzw,(r5.xyzw).xyzw).xxxx).x;
    // 158: dp4 r9.y, cb0[18].xyzw, r5.xyzw
    r9.y = (dot((source[18].xyzw).xyzw,(r5.xyzw).xyzw).xxxx).y;
    // 159: dp4 r9.z, cb0[19].xyzw, r5.xyzw
    r9.z = (dot((source[19].xyzw).xyzw,(r5.xyzw).xyzw).xxxx).z;
    // 160: mul r0.z, r0.y, r0.y
    r0.z = ((r0.yyyy)*(r0.yyyy)).z;
    // 161: mad r0.x, r0.x, r0.x, -r0.z
    r0.x = ((r0.xxxx)*(r0.xxxx)+(-(r0.zzzz))).x;
    // 162: add r2.xyz, r2.xyzx, r9.xyzx
    r2.xyz = ((r2.xyzx)+(r9.xyzx)).xyz;
    // 163: mad r0.xzw, cb0[20].xxyz, r0.xxxx, r2.xxyz
    r0.xzw = ((source[20].xxyz)*(r0.xxxx)+(r2.xxyz)).xzw;
    // 164: max r0.xzw, r0.xxzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xzw = (max(r0.xxzw,float4(0.000000,0.000000,0.000000,0.000000))).xzw;
    // 165: mul r0.xzw, r0.xxzw, cb0[12].xxyz
    r0.xzw = ((r0.xxzw)*(source[12].xxyz)).xzw;
    // 166: mad r0.xzw, r0.xxzw, l(3.141593, 0.000000, 3.141593, 3.141593), cb0[12].wwww
    r0.xzw = ((r0.xxzw)*(float4(3.141593,0.000000,3.141593,3.141593))+(source[12].wwww)).xzw;
    // 167: mul r2.x, r8.y, r8.y
    r2.x = ((r8.yyyy)*(r8.yyyy)).x;
    // 168: dp3 r2.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 169: add r2.z, r3.w, r8.x
    r2.z = ((r3.wwww)+(r8.xxxx)).z;
    // 170: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 171: mul r2.x, r2.z, r2.x
    r2.x = ((r2.zzzz)*(r2.xxxx)).x;
    // 172: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 173: add r2.x, r3.w, r2.x
    r2.x = ((r3.wwww)+(r2.xxxx)).x;
    // 174: add_sat r2.x, r2.x, l(-1.000000)
    r2.x = (saturate((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 175: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 176: mad r2.y, r2.x, r5.x, r5.y
    r2.y = ((r2.xxxx)*(r5.xxxx)+(r5.yyyy)).y;
    // 177: mad r2.y, r2.y, r2.x, r5.z
    r2.y = ((r2.yyyy)*(r2.xxxx)+(r5.zzzz)).y;
    // 178: mul r2.y, r2.x, r2.y
    r2.y = ((r2.xxxx)*(r2.yyyy)).y;
    // 179: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 180: mad r5.xyz, r7.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r7.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 181: mad r9.xyz, r7.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r7.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 182: mad r11.xyz, r7.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r11.xyz = ((r7.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 183: mad r5.xyz, r3.wwww, r5.xyzx, r9.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)+(r9.xyzx)).xyz;
    // 184: mad r5.xyz, r5.xyzx, r3.wwww, r11.xyzx
    r5.xyz = ((r5.xyzx)*(r3.wwww)+(r11.xyzx)).xyz;
    // 185: mul r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = ((r3.wwww)*(r5.xyzx)).xyz;
    // 186: max r5.xyz, r3.wwww, r5.xyzx
    r5.xyz = (max(r3.wwww,r5.xyzx)).xyz;
    // 187: mul r2.xyz, r2.xxxx, r4.xyzx
    r2.xyz = ((r2.xxxx)*(r4.xyzx)).xyz;
    // 188: mul r4.xyz, r5.xyzx, r10.xyzx
    r4.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 189: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 190: mul r5.xyz, r14.xyzx, r2.xyzx
    r5.xyz = ((r14.xyzx)*(r2.xyzx)).xyz;
    // 191: mad r6.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: mul r0.xzw, r0.xxzw, r6.xxyz
    r0.xzw = ((r0.xxzw)*(r6.xxyz)).xzw;
    // 193: mul r0.xzw, r0.xxzw, r4.xxyz
    r0.xzw = ((r0.xxzw)*(r4.xxyz)).xzw;
    // 194: mad r0.xzw, -r0.xxzw, r8.wwww, r0.xxzw
    r0.xzw = ((-(r0.xxzw))*(r8.wwww)+(r0.xxzw)).xzw;
    // 195: mad r0.xzw, r2.xxyz, r14.xxyz, r0.xxzw
    r0.xzw = ((r2.xxyz)*(r14.xxyz)+(r0.xxzw)).xzw;
    // 196: add r2.xyz, r0.xzwx, r3.xyzx
    r2.xyz = ((r0.xzwx)+(r3.xyzx)).xyz;
    // 197: mad r2.xyz, r7.xyzx, cb0[24].xyzx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(source[24].xyzx)+(r2.xyzx)).xyz;
    // 198: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 199: mad r3.x, r3.x, l(-0.250000), l(0.400000)
    r3.x = ((r3.xxxx)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).x;
    // 200: eq r3.y, cb0[25].x, l(0.000000)
    r3.y = (asfloat((uint4)((source[25].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 201: not r3.z, r3.y
    r3.z = (asfloat(~asuint(r3.yyyy))).z;
    // 202: lt r4.x, r2.w, r3.x
    r4.x = (asfloat((uint4)((r2.wwww)<(r3.xxxx)) * 0xffffffffu)).x;
    // 203: and r3.z, r3.z, r4.x
    r3.z = (asfloat(asuint(r3.zzzz) & asuint(r4.xxxx))).z;
    // 204: discard_nz r3.z
    if ((asuint(r3.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 205: ge r3.x, r2.w, r3.x
    r3.x = (asfloat((uint4)((r2.wwww)>=(r3.xxxx)) * 0xffffffffu)).x;
    // 206: mad r1.w, r1.w, cb0[1].x, l(-0.900000)
    r1.w = ((r1.wwww)*(source[1].xxxx)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 207: mul_sat r1.w, r1.w, l(9.999998)
    r1.w = (saturate((r1.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 208: mad r3.z, r1.w, l(-2.000000), l(3.000000)
    r3.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 209: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 210: mul r1.w, r1.w, r3.z
    r1.w = ((r1.wwww)*(r3.zzzz)).w;
    // 211: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 212: movc r1.w, r3.x, r1.w, r2.w
    r1.w = ((asuint(r3.xxxx) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 213: movc o0.w, r3.y, r1.w, r2.w
    output.targets[0].w = ((asuint(r3.yyyy) != 0u) ? (r1.wwww) : (r2.wwww)).w;
    // 214: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 215: mov r1.z, r0.y
    r1.z = (r0.yyyy).z;
    // 216: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 217: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 218: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 219: dp3 r0.y, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r0.y = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).y;
    // 220: div r1.xy, r1.xyxx, r0.yyyy
    r1.xy = ((r1.xyxx)/(r0.yyyy)).xy;
    // 221: ge r0.y, l(0.000000), r1.z
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).y;
    // 222: ge r1.zw, r1.xxxy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.zw = (asfloat((uint4)((r1.xxxy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).zw;
    // 223: movc r1.zw, r1.zzzw, l(0,0,1.000000,1.000000), l(0,0,-1.000000,-1.000000)
    r1.zw = ((asuint(r1.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),1.000000,1.000000)) : (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000))).zw;
    // 224: mad r1.zw, -|r1.yyyx|, r1.zzzw, r1.zzzw
    r1.zw = ((-(abs(r1.yyyx)))*(r1.zzzw)+(r1.zzzw)).zw;
    // 225: movc r1.xy, r0.yyyy, r1.zwzz, r1.xyxx
    r1.xy = ((asuint(r0.yyyy) != 0u) ? (r1.zwzz) : (r1.xyxx)).xy;
    // 226: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 227: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 228: dp3 o4.y, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 229: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 230: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 231: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 232: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 233: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 234: mov o3.xyzw, r7.xyzw
    output.targets[3].xyzw = (r7.xyzw).xyzw;
    // 235: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 236: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 237: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 238: ret
    return output;
}

// source.character.static-map-native-1108.v1 / source program 7b60bfa74188e94fa857e706d04535f0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1108(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[12]=g_SourceCharacterBaseConstants[11];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    source[26]=1.f;
    source[27]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
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
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 11: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: mul r0.w, r3.z, cb0[9].x
    r0.w = ((r3.zzzz)*(source[9].xxxx)).w;
    // 13: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 14: mul r3.xy, r3.xyxx, cb0[8].xxxx
    r3.xy = ((r3.xyxx)*(source[8].xxxx)).xy;
    // 15: mul r3.xy, r3.xyxx, v2.wwww
    r3.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 16: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 18: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 19: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 20: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 23: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 24: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 25: mul r4.xyz, r1.wwww, r3.xyzx
    r4.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 26: dp3 r5.y, r2.xyzx, r4.xyzx
    r5.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 27: dp3 r5.x, r1.xyzx, r4.xyzx
    r5.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 28: dp2 r6.z, r5.xyxx, cb0[14].xyxx
    r6.z = (dot((r5.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 29: mul r7.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 30: dp2 r6.x, r5.xyxx, r7.xyxx
    r6.x = (dot((r5.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 31: dp3 r6.y, r0.xyzx, r4.xyzx
    r6.y = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 32: mov r6.w, l(1.000000)
    r6.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 33: dp4 r8.x, cb0[15].xyzw, r6.xyzw
    r8.x = (dot((source[15].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).x;
    // 34: dp4 r8.y, cb0[16].xyzw, r6.xyzw
    r8.y = (dot((source[16].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).y;
    // 35: dp4 r8.z, cb0[17].xyzw, r6.xyzw
    r8.z = (dot((source[17].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).z;
    // 36: mul r9.xyzw, r6.yzzx, r6.xyzz
    r9.xyzw = ((r6.yzzx)*(r6.xyzz)).xyzw;
    // 37: dp4 r10.x, cb0[18].xyzw, r9.xyzw
    r10.x = (dot((source[18].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 38: dp4 r10.y, cb0[19].xyzw, r9.xyzw
    r10.y = (dot((source[19].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 39: dp4 r10.z, cb0[20].xyzw, r9.xyzw
    r10.z = (dot((source[20].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 40: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 41: mul r1.w, r6.y, r6.y
    r1.w = ((r6.yyyy)*(r6.yyyy)).w;
    // 42: mov r5.z, r6.y
    r5.z = (r6.yyyy).z;
    // 43: mad r1.w, r6.x, r6.x, -r1.w
    r1.w = ((r6.xxxx)*(r6.xxxx)+(-(r1.wwww))).w;
    // 44: mad r6.xyz, cb0[21].xyzx, r1.wwww, r8.xyzx
    r6.xyz = ((source[21].xyzx)*(r1.wwww)+(r8.xyzx)).xyz;
    // 45: max r6.xyz, r6.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r6.xyz = (max(r6.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 46: mul r6.xyz, r6.xyzx, cb0[13].xyzx
    r6.xyz = ((r6.xyzx)*(source[13].xyzx)).xyz;
    // 47: mad r6.xyz, r6.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r6.xyz = ((r6.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 48: mul r8.xyz, cb0[7].xyzx, cb0[9].zzzz
    r8.xyz = ((source[7].xyzx)*(source[9].zzzz)).xyz;
    // 49: mul r9.xyz, cb0[6].xyzx, cb0[9].yyyy
    r9.xyz = ((source[6].xyzx)*(source[9].yyyy)).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 52: mad r8.xyz, r8.xyzx, r10.xyzx, -r9.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)+(-(r9.xyzx))).xyz;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 54: mul r1.w, r10.z, cb0[9].w
    r1.w = ((r10.zzzz)*(source[9].wwww)).w;
    // 55: mul r7.zw, r10.yyyx, cb0[11].yyyw
    r7.zw = ((r10.yyyx)*(source[11].yyyw)).zw;
    // 56: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 57: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 58: mul r2.w, r2.w, cb0[10].x
    r2.w = ((r2.wwww)*(source[10].xxxx)).w;
    // 59: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 60: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 61: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 62: mul_sat r10.w, r1.w, cb2[3].w
    r10.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 63: mad r8.xyz, r2.wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 64: mul r9.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r9.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 65: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 66: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 67: mul r11.xyz, r1.wwww, v5.xyzx
    r11.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 68: dp3 r1.w, r4.xyzx, r11.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 69: mul r12.xyz, r1.wwww, r4.xyzx
    r12.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 70: mad r12.xyz, r12.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r12.xyz = ((r12.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 71: dp3 r2.y, r2.xyzx, r12.xyzx
    r2.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 72: dp3 r2.x, r1.xyzx, r12.xyzx
    r2.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 73: mad r1.xy, cb0[8].zzzz, r2.xyxx, r9.xyxx
    r1.xy = ((source[8].zzzz)*(r2.xyxx)+(r9.xyxx)).xy;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 75: mul r1.xyz, r1.xyzx, cb0[5].xyzx
    r1.xyz = ((r1.xyzx)*(source[5].xyzx)).xyz;
    // 76: mad r1.xyz, cb0[8].wwww, r1.xyzx, r1.xyzx
    r1.xyz = ((source[8].wwww)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 77: add r1.xyz, r1.xyzx, -cb0[8].wwww
    r1.xyz = ((r1.xyzx)+(-(source[8].wwww))).xyz;
    // 78: mov_sat r9.xyz, r1.xyzx
    r9.xyz = (saturate(r1.xyzx)).xyz;
    // 79: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 80: mad r1.xyz, -r0.wwww, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.wwww))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 81: mad r8.xyz, r0.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r0.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 82: mul r1.xyz, r1.xyzx, r8.xyzx
    r1.xyz = ((r1.xyzx)*(r8.xyzx)).xyz;
    // 83: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 84: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 85: mul r8.xyz, r1.xyzx, cb0[10].yyyy
    r8.xyz = ((r1.xyzx)*(source[10].yyyy)).xyz;
    // 86: mad r1.xyz, cb0[10].zzzz, r1.xyzx, -r8.xyzx
    r1.xyz = ((source[10].zzzz)*(r1.xyzx)+(-(r8.xyzx))).xyz;
    // 87: mad r1.xyz, r2.wwww, r1.xyzx, r8.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r8.xyzx)).xyz;
    // 88: add r8.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 89: mul r1.xyz, r1.xyzx, r8.xyzx
    r1.xyz = ((r1.xyzx)*(r8.xyzx)).xyz;
    // 90: mad_sat r8.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 91: mov_sat r8.w, cb0[10].w
    r8.w = (saturate(source[10].wwww)).w;
    // 92: mad r1.xyz, -r8.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r8.xyzx
    r1.xyz = ((-(r8.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r8.xyzx)).xyz;
    // 93: mul r0.w, r8.w, l(0.080000)
    r0.w = ((r8.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 94: mov o3.xyzw, r8.xyzw
    output.targets[3].xyzw = (r8.xyzw).xyzw;
    // 95: mad r1.xyz, r10.wwww, r1.xyzx, r0.wwww
    r1.xyz = ((r10.wwww)*(r1.xyzx)+(r0.wwww)).xyz;
    // 96: deriv_rtx_coarse r9.x, r1.w
    r9.x = (ddx_coarse(r1.wwww)).x;
    // 97: deriv_rty_coarse r9.y, r1.w
    r9.y = (ddy_coarse(r1.wwww)).y;
    // 98: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 100: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 101: log r2.zw, |r7.zzzw|
    r2.zw = (log2(abs(r7.zzzw))).zw;
    // 102: lt r7.zw, |r7.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r7.zw = (asfloat((uint4)((abs(r7.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 103: mul r2.z, r2.z, cb0[11].z
    r2.z = ((r2.zzzz)*(source[11].zzzz)).z;
    // 104: mul r2.w, r2.w, cb0[12].x
    r2.w = ((r2.wwww)*(source[12].xxxx)).w;
    // 105: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 106: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 108: movc r2.zw, r7.zzzw, l(0,0,0,0), r2.zzzw
    r2.zw = ((asuint(r7.zzzw) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzw)).zw;
    // 109: max r2.z, r2.z, cb0[0].w
    r2.z = (max(r2.zzzz,source[0].wwww)).z;
    // 110: min r10.z, r2.z, l(1.000000)
    r10.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 111: mad_sat r9.y, r1.w, l(0.300000), r10.z
    r9.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r10.zzzz))).y;
    // 112: add r1.w, -r9.y, l(1.000000)
    r1.w = ((-(r9.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: max r13.xyz, r1.xyzx, r1.wwww
    r13.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 114: add r13.xyz, -r1.xyzx, r13.xyzx
    r13.xyz = ((-(r1.xyzx))+(r13.xyzx)).xyz;
    // 115: mul_sat r1.w, r1.y, l(50.000000)
    r1.w = (saturate((r1.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 116: mul r13.xyz, r1.wwww, r13.xyzx
    r13.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 117: add r1.w, r12.z, l(1.000000)
    r1.w = ((r12.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: add_sat r9.x, r0.w, -r1.w
    r9.x = (saturate((r0.wwww)+(-(r1.wwww)))).x;
    // 120: sample_indexable(texture2d)(float,float,float,float) r7.zw, r9.xyxx, t5.zwxy, s7
    r7.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 121: add r0.w, r2.w, r9.x
    r0.w = ((r2.wwww)+(r9.xxxx)).w;
    // 122: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 123: mul r9.xzw, r1.xxyz, r7.wwww
    r9.xzw = ((r1.xxyz)*(r7.wwww)).xzw;
    // 124: mad r9.xzw, r13.xxyz, r7.zzzz, r9.xxzw
    r9.xzw = ((r13.xxyz)*(r7.zzzz)+(r9.xxzw)).xzw;
    // 125: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r7.w
    r1.w = r7.w != 0.f ? 1.f / r7.w : 0.f;
    // 126: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 127: mad r13.xyz, r1.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r1.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 128: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 129: mad r1.xyz, r1.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r1.xyz = ((r1.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 130: mad r14.xyz, -r9.xzwx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r9.xzwx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 131: mul r9.xzw, r9.xxzw, r13.xxyz
    r9.xzw = ((r9.xxzw)*(r13.xxyz)).xzw;
    // 132: mul r6.xyz, r6.xyzx, r14.xyzx
    r6.xyz = ((r6.xyzx)*(r14.xyzx)).xyz;
    // 133: dp2_sat r13.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r13.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 134: dp3_sat r13.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r13.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 135: dp3_sat r13.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r13.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 136: mul r13.xyz, r13.xyzx, r13.xyzx
    r13.xyz = ((r13.xyzx)*(r13.xyzx)).xyz;
    // 137: sample_indexable(texture2d)(float,float,float,float) r14.xyz, v3.zwzz, t8.xyzw, s5
    r14.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 138: mul r14.xyz, r14.xyzx, cb0[27].xyzx
    r14.xyz = ((r14.xyzx)*(source[27].xyzx)).xyz;
    // 139: dp3 r1.w, r14.xyzx, r13.xyzx
    r1.w = (dot((r14.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 140: sample_indexable(texture2d)(float,float,float,float) r13.xyz, v3.zwzz, t7.xyzw, s5
    r13.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 141: mul r13.xyz, r13.xyzx, cb0[26].xyzx
    r13.xyz = ((r13.xyzx)*(source[26].xyzx)).xyz;
    // 142: mul r15.xyz, r1.wwww, r13.xyzx
    r15.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 143: dp3 r2.z, v6.xyzx, v6.xyzx
    r2.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 144: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 145: mul r16.xyz, r2.zzzz, v6.xyzx
    r16.xyz = ((r2.zzzz)*(v6.xyzx)).xyz;
    // 146: dp3 r2.z, r16.xyzx, r4.xyzx
    r2.z = (dot((r16.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 147: mad r4.xy, r2.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r2.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 148: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 149: mul r4.yzw, r4.yyyy, cb0[24].xxyz
    r4.yzw = ((r4.yyyy)*(source[24].xxyz)).yzw;
    // 150: mad r4.xyz, r4.xxxx, cb0[23].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[23].xyzx)+(r4.yzwy)).xyz;
    // 151: mul r4.xyz, r4.xyzx, cb0[25].wwww
    r4.xyz = ((r4.xyzx)*(source[25].wwww)).xyz;
    // 152: mul r16.xyz, r8.xyzx, r4.xyzx
    r16.xyz = ((r8.xyzx)*(r4.xyzx)).xyz;
    // 153: mad r15.xyz, r8.xyzx, r15.xyzx, r16.xyzx
    r15.xyz = ((r8.xyzx)*(r15.xyzx)+(r16.xyzx)).xyz;
    // 154: mad r16.xyz, r8.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r16.xyz = ((r8.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 155: mad r17.xyz, r8.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r17.xyz = ((r8.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 156: mad r18.xyz, r8.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r18.xyz = ((r8.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 157: mad r17.xyz, r2.wwww, r17.xyzx, r18.xyzx
    r17.xyz = ((r2.wwww)*(r17.xyzx)+(r18.xyzx)).xyz;
    // 158: mad r16.xyz, r17.xyzx, r2.wwww, r16.xyzx
    r16.xyz = ((r17.xyzx)*(r2.wwww)+(r16.xyzx)).xyz;
    // 159: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 160: max r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = (max(r2.wwww,r16.xyzx)).xyz;
    // 161: mul r15.xyz, r15.xyzx, r16.xyzx
    r15.xyz = ((r15.xyzx)*(r16.xyzx)).xyz;
    // 162: mul r6.xyz, r6.xyzx, r15.xyzx
    r6.xyz = ((r6.xyzx)*(r15.xyzx)).xyz;
    // 163: mad r6.xyz, -r6.xyzx, r10.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r10.wwww)+(r6.xyzx)).xyz;
    // 164: mov o2.zw, r10.zzzw
    output.targets[2].zw = (r10.zzzw).zw;
    // 165: mul r2.z, r9.y, l(5.000000)
    r2.z = ((r9.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 166: mul r3.w, r9.y, r9.y
    r3.w = ((r9.yyyy)*(r9.yyyy)).w;
    // 167: mul r0.w, r0.w, r3.w
    r0.w = ((r0.wwww)*(r3.wwww)).w;
    // 168: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 169: add r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)+(r0.wwww)).w;
    // 170: mov o5.y, r2.w
    output.targets[5].y = (r2.wwww).y;
    // 171: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 172: dp3 r0.y, r0.xyzx, r12.xyzx
    r0.y = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 173: dp2 r0.x, r2.xyxx, r7.xyxx
    r0.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 174: dp2 r0.z, r2.xyxx, cb0[14].xyxx
    r0.z = (dot((r2.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 175: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r0.xyzx, t6.xyzw, s6, r2.z
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r0.xyzx).xyz, (r2.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 176: mul r0.xyz, r2.xyzx, r2.wwww
    r0.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 177: mul r0.xyz, r0.xyzx, cb0[13].xyzx
    r0.xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 178: mad r0.xyz, r0.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r0.xyz = ((r0.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 179: dp2_sat r2.x, r12.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r12.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 180: dp3_sat r2.y, r12.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r12.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 181: dp3_sat r2.z, r12.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r12.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 182: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 183: dp3 r2.x, r14.xyzx, r2.xyzx
    r2.x = (dot((r14.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 184: add r1.w, r1.w, -r2.x
    r1.w = ((r1.wwww)+(-(r2.xxxx))).w;
    // 185: mad r1.w, r10.z, r1.w, r2.x
    r1.w = ((r10.zzzz)*(r1.wwww)+(r2.xxxx)).w;
    // 186: mad r2.xyz, r13.xyzx, r1.wwww, r4.xyzx
    r2.xyz = ((r13.xyzx)*(r1.wwww)+(r4.xyzx)).xyz;
    // 187: mul r4.xyz, r1.wwww, r13.xyzx
    r4.xyz = ((r1.wwww)*(r13.xyzx)).xyz;
    // 188: mad r1.x, r0.w, r1.x, r1.y
    r1.x = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).x;
    // 189: mad r1.x, r1.x, r0.w, r1.z
    r1.x = ((r1.xxxx)*(r0.wwww)+(r1.zzzz)).x;
    // 190: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 191: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 192: mul r1.xyz, r0.wwww, r2.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 193: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 194: div r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)/(r2.xyzx)).xyz;
    // 195: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 196: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 197: mad r1.xyz, r0.xyzx, r9.xzwx, r6.xyzx
    r1.xyz = ((r0.xyzx)*(r9.xzwx)+(r6.xyzx)).xyz;
    // 198: mul r0.xyz, r9.xzwx, r0.xyzx
    r0.xyz = ((r9.xzwx)*(r0.xyzx)).xyz;
    // 199: dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 200: dp3 r0.x, r3.xyzx, r11.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 201: add r0.y, -|r11.z|, l(1.000000)
    r0.y = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 202: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 203: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 204: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 205: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 206: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 207: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 208: mul r2.xyz, r0.xxxx, cb0[2].xyzx
    r2.xyz = ((r0.xxxx)*(source[2].xyzx)).xyz;
    // 209: movc r0.xyz, r0.yyyy, l(0,0,0,0), r2.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 210: mul r2.xy, v4.xyxx, cb0[3].xyxx
    r2.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 211: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 212: mul r3.xyz, cb0[4].xyzx, cb0[8].yyyy
    r3.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 213: mad r0.xyz, r2.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 214: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 215: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 216: mad o0.xyz, r8.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r8.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 217: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 218: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 219: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 220: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 221: ge r1.w, l(0.000000), r0.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 222: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 223: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 224: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 225: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 226: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 227: movc r0.xy, r1.wwww, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 228: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 229: mul o4.z, r0.w, r1.x
    output.targets[4].z = ((r0.wwww)*(r1.xxxx)).z;
    // 230: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 231: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 232: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
    // 233: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 234: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 235: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 236: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 237: ret
    return output;
}

// source.character.static-map-native-1108.v1 / source program ba114f26028fdf41b7170a637f7ba2fc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1108(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1108(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[12]=g_SourceCharacterBaseConstants[11];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: mul r0.z, r0.z, cb0[9].x
    r0.z = ((r0.zzzz)*(source[9].xxxx)).z;
    // 4: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 5: mul r0.xy, r0.xyxx, cb0[8].xxxx
    r0.xy = ((r0.xyxx)*(source[8].xxxx)).xy;
    // 6: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 7: add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 9: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 10: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 12: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 13: div r0.xyw, r1.xyxz, r0.xxxx
    r0.xyw = ((r1.xyxz)/(r0.xxxx)).xyw;
    // 14: dp3 r1.x, r0.xywx, r0.xywx
    r1.x = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).x;
    // 15: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 16: mul r1.xyz, r0.xywx, r1.xxxx
    r1.xyz = ((r0.xywx)*(r1.xxxx)).xyz;
    // 17: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 18: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 19: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 20: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 22: mul r3.xyz, r1.wwww, v0.xyzx
    r3.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 23: mul r4.xyz, r2.zxyz, r3.yzxy
    r4.xyz = ((r2.zxyz)*(r3.yzxy)).xyz;
    // 24: mad r4.xyz, r2.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r2.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 25: mul r4.xyz, r4.xyzx, v1.wwww
    r4.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 26: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 27: dp3 r5.x, r3.xyzx, r1.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 28: dp2 r6.z, r5.xyxx, cb0[14].xyxx
    r6.z = (dot((r5.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 29: mul r7.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 30: dp2 r6.x, r5.xyxx, r7.xyxx
    r6.x = (dot((r5.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 31: dp3 r6.y, r2.xyzx, r1.xyzx
    r6.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 32: mov r6.w, l(1.000000)
    r6.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 33: dp4 r8.x, cb0[15].xyzw, r6.xyzw
    r8.x = (dot((source[15].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).x;
    // 34: dp4 r8.y, cb0[16].xyzw, r6.xyzw
    r8.y = (dot((source[16].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).y;
    // 35: dp4 r8.z, cb0[17].xyzw, r6.xyzw
    r8.z = (dot((source[17].xyzw).xyzw,(r6.xyzw).xyzw).xxxx).z;
    // 36: mul r9.xyzw, r6.yzzx, r6.xyzz
    r9.xyzw = ((r6.yzzx)*(r6.xyzz)).xyzw;
    // 37: dp4 r10.x, cb0[18].xyzw, r9.xyzw
    r10.x = (dot((source[18].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 38: dp4 r10.y, cb0[19].xyzw, r9.xyzw
    r10.y = (dot((source[19].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 39: dp4 r10.z, cb0[20].xyzw, r9.xyzw
    r10.z = (dot((source[20].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 40: add r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)+(r10.xyzx)).xyz;
    // 41: mul r1.w, r6.y, r6.y
    r1.w = ((r6.yyyy)*(r6.yyyy)).w;
    // 42: mov r5.z, r6.y
    r5.z = (r6.yyyy).z;
    // 43: mad r1.w, r6.x, r6.x, -r1.w
    r1.w = ((r6.xxxx)*(r6.xxxx)+(-(r1.wwww))).w;
    // 44: mad r6.xyz, cb0[21].xyzx, r1.wwww, r8.xyzx
    r6.xyz = ((source[21].xyzx)*(r1.wwww)+(r8.xyzx)).xyz;
    // 45: max r6.xyz, r6.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r6.xyz = (max(r6.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 46: mul r6.xyz, r6.xyzx, cb0[13].xyzx
    r6.xyz = ((r6.xyzx)*(source[13].xyzx)).xyz;
    // 47: mad r6.xyz, r6.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r6.xyz = ((r6.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 48: mul r8.xyz, cb0[7].xyzx, cb0[9].zzzz
    r8.xyz = ((source[7].xyzx)*(source[9].zzzz)).xyz;
    // 49: mul r9.xyz, cb0[6].xyzx, cb0[9].yyyy
    r9.xyz = ((source[6].xyzx)*(source[9].yyyy)).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 52: mad r8.xyz, r8.xyzx, r10.xyzx, -r9.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)+(-(r9.xyzx))).xyz;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 54: mul r1.w, r10.z, cb0[9].w
    r1.w = ((r10.zzzz)*(source[9].wwww)).w;
    // 55: mul r7.zw, r10.yyyx, cb0[11].yyyw
    r7.zw = ((r10.yyyx)*(source[11].yyyw)).zw;
    // 56: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 57: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 58: mul r2.w, r2.w, cb0[10].x
    r2.w = ((r2.wwww)*(source[10].xxxx)).w;
    // 59: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 60: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 61: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 62: mul_sat r10.w, r1.w, cb2[3].w
    r10.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 63: mad r8.xyz, r2.wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 64: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 65: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 66: mul r9.xyz, r1.wwww, v5.xyzx
    r9.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 67: dp3 r1.w, r1.xyzx, r9.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 68: mul r11.xyz, r1.wwww, r1.xyzx
    r11.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 69: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 70: dp3 r4.y, r4.xyzx, r11.xyzx
    r4.y = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 71: dp3 r4.x, r3.xyzx, r11.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 72: mul r3.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r3.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 73: mad r3.xy, cb0[8].zzzz, r4.xyxx, r3.xyxx
    r3.xy = ((source[8].zzzz)*(r4.xyxx)+(r3.xyxx)).xy;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 75: mul r3.xyz, r3.xyzx, cb0[5].xyzx
    r3.xyz = ((r3.xyzx)*(source[5].xyzx)).xyz;
    // 76: mad r3.xyz, cb0[8].wwww, r3.xyzx, r3.xyzx
    r3.xyz = ((source[8].wwww)*(r3.xyzx)+(r3.xyzx)).xyz;
    // 77: add r3.xyz, r3.xyzx, -cb0[8].wwww
    r3.xyz = ((r3.xyzx)+(-(source[8].wwww))).xyz;
    // 78: mov_sat r12.xyz, r3.xyzx
    r12.xyz = (saturate(r3.xyzx)).xyz;
    // 79: mov_sat r3.xyz, -r3.xyzx
    r3.xyz = (saturate(-(r3.xyzx))).xyz;
    // 80: mad r3.xyz, -r0.zzzz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r0.zzzz))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 81: mad r8.xyz, r0.zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((r0.zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 82: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 83: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 84: min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 85: mul r8.xyz, r3.xyzx, cb0[10].yyyy
    r8.xyz = ((r3.xyzx)*(source[10].yyyy)).xyz;
    // 86: mad r3.xyz, cb0[10].zzzz, r3.xyzx, -r8.xyzx
    r3.xyz = ((source[10].zzzz)*(r3.xyzx)+(-(r8.xyzx))).xyz;
    // 87: mad r3.xyz, r2.wwww, r3.xyzx, r8.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r8.xyzx)).xyz;
    // 88: add r8.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 89: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 90: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 91: mov_sat r3.w, cb0[10].w
    r3.w = (saturate(source[10].wwww)).w;
    // 92: mad r8.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r8.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 93: mul r0.z, r3.w, l(0.080000)
    r0.z = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 94: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 95: mad r8.xyz, r10.wwww, r8.xyzx, r0.zzzz
    r8.xyz = ((r10.wwww)*(r8.xyzx)+(r0.zzzz)).xyz;
    // 96: deriv_rtx_coarse r10.x, r1.w
    r10.x = (ddx_coarse(r1.wwww)).x;
    // 97: deriv_rty_coarse r10.y, r1.w
    r10.y = (ddy_coarse(r1.wwww)).y;
    // 98: add r0.z, r1.w, l(1.000000)
    r0.z = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 99: dp2 r1.w, r10.xyxx, r10.xyxx
    r1.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 100: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 101: log r4.zw, |r7.zzzw|
    r4.zw = (log2(abs(r7.zzzw))).zw;
    // 102: lt r7.zw, |r7.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r7.zw = (asfloat((uint4)((abs(r7.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 103: mul r2.w, r4.z, cb0[11].z
    r2.w = ((r4.zzzz)*(source[11].zzzz)).w;
    // 104: mul r3.w, r4.w, cb0[12].x
    r3.w = ((r4.wwww)*(source[12].xxxx)).w;
    // 105: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 106: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: movc r3.w, r7.w, l(0), r3.w
    r3.w = ((asuint(r7.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 108: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 109: movc r2.w, r7.z, l(0), r2.w
    r2.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 110: max r2.w, r2.w, cb0[0].w
    r2.w = (max(r2.wwww,source[0].wwww)).w;
    // 111: min r10.z, r2.w, l(1.000000)
    r10.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 112: mad_sat r10.y, r1.w, l(0.300000), r10.z
    r10.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r10.zzzz))).y;
    // 113: mov o2.zw, r10.zzzw
    output.targets[2].zw = (r10.zzzw).zw;
    // 114: add r1.w, -r10.y, l(1.000000)
    r1.w = ((-(r10.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: max r12.xyz, r8.xyzx, r1.wwww
    r12.xyz = (max(r8.xyzx,r1.wwww)).xyz;
    // 116: add r12.xyz, -r8.xyzx, r12.xyzx
    r12.xyz = ((-(r8.xyzx))+(r12.xyzx)).xyz;
    // 117: mul_sat r1.w, r8.y, l(50.000000)
    r1.w = (saturate((r8.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 118: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 119: add r1.w, r11.z, l(1.000000)
    r1.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: add_sat r10.x, r0.z, -r1.w
    r10.x = (saturate((r0.zzzz)+(-(r1.wwww)))).x;
    // 122: sample_indexable(texture2d)(float,float,float,float) r4.zw, r10.xyxx, t5.zwxy, s6
    r4.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 123: add r0.z, r3.w, r10.x
    r0.z = ((r3.wwww)+(r10.xxxx)).z;
    // 124: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 125: mul r13.xyz, r4.wwww, r8.xyzx
    r13.xyz = ((r4.wwww)*(r8.xyzx)).xyz;
    // 126: mad r12.xyz, r12.xyzx, r4.zzzz, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r4.zzzz)+(r13.xyzx)).xyz;
    // 127: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r4.w
    r1.w = r4.w != 0.f ? 1.f / r4.w : 0.f;
    // 128: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 129: mad r13.xyz, r8.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r8.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 130: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 131: mad r8.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r8.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 132: mad r14.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 133: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 134: mul r6.xyz, r6.xyzx, r14.xyzx
    r6.xyz = ((r6.xyzx)*(r14.xyzx)).xyz;
    // 135: mad r13.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 136: mad r14.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r14.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 137: mad r15.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r15.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 138: mad r14.xyz, r3.wwww, r14.xyzx, r15.xyzx
    r14.xyz = ((r3.wwww)*(r14.xyzx)+(r15.xyzx)).xyz;
    // 139: mad r13.xyz, r14.xyzx, r3.wwww, r13.xyzx
    r13.xyz = ((r14.xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 140: mul r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = ((r3.wwww)*(r13.xyzx)).xyz;
    // 141: max r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = (max(r3.wwww,r13.xyzx)).xyz;
    // 142: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 143: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 144: mul r14.xyz, r1.wwww, v6.xyzx
    r14.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 145: dp3 r1.x, r14.xyzx, r1.xyzx
    r1.x = (dot((r14.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 146: dp3 r1.y, r14.xyzx, r11.xyzx
    r1.y = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 147: dp3 r2.y, r2.xyzx, r11.xyzx
    r2.y = (dot((r2.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 148: mad r1.yz, r1.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r1.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 149: mad r1.xw, r1.xxxx, l(0.500000, 0.000000, 0.000000, -0.500000), l(0.500000, 0.000000, 0.000000, 0.500000)
    r1.xw = ((r1.xxxx)*(float4(0.500000,0.000000,0.000000,-0.500000))+(float4(0.500000,0.000000,0.000000,0.500000))).xw;
    // 150: mul r1.xyzw, r1.xyzw, r1.xyzw
    r1.xyzw = ((r1.xyzw)*(r1.xyzw)).xyzw;
    // 151: mul r11.xyz, r1.wwww, cb0[24].xyzx
    r11.xyz = ((r1.wwww)*(source[24].xyzx)).xyz;
    // 152: mad r11.xyz, r1.xxxx, cb0[23].xyzx, r11.xyzx
    r11.xyz = ((r1.xxxx)*(source[23].xyzx)+(r11.xyzx)).xyz;
    // 153: mul r11.xyz, r11.xyzx, cb0[25].wwww
    r11.xyz = ((r11.xyzx)*(source[25].wwww)).xyz;
    // 154: mul r11.xyz, r3.xyzx, r11.xyzx
    r11.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 155: mul r11.xyz, r13.xyzx, r11.xyzx
    r11.xyz = ((r13.xyzx)*(r11.xyzx)).xyz;
    // 156: mul r6.xyz, r6.xyzx, r11.xyzx
    r6.xyz = ((r6.xyzx)*(r11.xyzx)).xyz;
    // 157: mad r6.xyz, -r6.xyzx, r10.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r10.wwww)+(r6.xyzx)).xyz;
    // 158: dp2 r2.x, r4.xyxx, r7.xyxx
    r2.x = (dot((r4.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 159: dp2 r2.z, r4.xyxx, cb0[14].xyxx
    r2.z = (dot((r4.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 160: mul r1.x, r10.y, l(5.000000)
    r1.x = ((r10.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 161: mul r1.w, r10.y, r10.y
    r1.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 162: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 163: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 164: add r0.z, r3.w, r0.z
    r0.z = ((r3.wwww)+(r0.zzzz)).z;
    // 165: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 166: add_sat r0.z, r0.z, l(-1.000000)
    r0.z = (saturate((r0.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).z;
    // 167: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r2.xyzx, t6.xyzw, s5, r1.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r2.xyzx).xyz, (r1.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 168: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 169: mul r2.xyz, r2.xyzx, cb0[13].xyzx
    r2.xyz = ((r2.xyzx)*(source[13].xyzx)).xyz;
    // 170: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 171: mad r1.x, r0.z, r8.x, r8.y
    r1.x = ((r0.zzzz)*(r8.xxxx)+(r8.yyyy)).x;
    // 172: mad r1.x, r1.x, r0.z, r8.z
    r1.x = ((r1.xxxx)*(r0.zzzz)+(r8.zzzz)).x;
    // 173: mul r1.x, r0.z, r1.x
    r1.x = ((r0.zzzz)*(r1.xxxx)).x;
    // 174: max r0.z, r0.z, r1.x
    r0.z = (max(r0.zzzz,r1.xxxx)).z;
    // 175: mul r1.xzw, r1.zzzz, cb0[24].xxyz
    r1.xzw = ((r1.zzzz)*(source[24].xxyz)).xzw;
    // 176: mad r1.xyz, cb0[23].xyzx, r1.yyyy, r1.xzwx
    r1.xyz = ((source[23].xyzx)*(r1.yyyy)+(r1.xzwx)).xyz;
    // 177: mul r1.xyz, r1.xyzx, cb0[25].wwww
    r1.xyz = ((r1.xyzx)*(source[25].wwww)).xyz;
    // 178: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 179: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 180: mad r2.xyz, r1.xyzx, r12.xyzx, r6.xyzx
    r2.xyz = ((r1.xyzx)*(r12.xyzx)+(r6.xyzx)).xyz;
    // 181: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 182: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 183: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 184: add r0.y, -|r9.z|, l(1.000000)
    r0.y = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 185: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 186: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 187: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 188: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 189: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 190: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 191: mul r0.xzw, r0.xxxx, cb0[2].xxyz
    r0.xzw = ((r0.xxxx)*(source[2].xxyz)).xzw;
    // 192: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 193: mul r1.xy, v4.xyxx, cb0[3].xyxx
    r1.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 194: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 195: mul r4.xyz, cb0[4].xyzx, cb0[8].yyyy
    r4.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 196: mad r0.xyz, r1.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 197: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 198: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 199: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 200: mad o0.xyz, r3.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 201: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 202: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 203: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 204: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 205: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 206: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 207: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 208: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 209: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 210: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 211: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 212: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 213: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 214: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
    // 215: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 216: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 217: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 218: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 219: ret
    return output;
}

// source.character.static-map-native-1109.v1 / source program 545e58024ab525468468823e72f26d62
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1109(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    source[26]=1.f;
    source[27]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f, r19=0.f;
    // 1: mul r0.xyz, cb0[6].xyzx, cb0[9].xxxx
    r0.xyz = ((source[6].xyzx)*(source[9].xxxx)).xyz;
    // 2: mul r1.xyz, cb0[5].xyzx, cb0[8].wwww
    r1.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r4.xyz, cb0[8].zzzz, r3.xyzx, r2.xyzx
    r4.xyz = ((source[8].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mad r2.xyz, cb0[9].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[9].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 8: add r3.xy, r2.wwww, cb0[11].wyww
    r3.xy = ((r2.wwww)+(source[11].wyww)).xy;
    // 9: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 10: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 12: mul r0.w, r2.z, cb0[9].w
    r0.w = ((r2.zzzz)*(source[9].wwww)).w;
    // 13: mul r2.xy, r2.yxyy, cb0[12].ywyy
    r2.xy = ((r2.yxyy)*(source[12].ywyy)).xy;
    // 14: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 15: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 16: mul r1.w, r1.w, cb0[10].x
    r1.w = ((r1.wwww)*(source[10].xxxx)).w;
    // 17: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 18: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 19: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 21: mul r1.xyz, r0.xyzx, cb0[10].yyyy
    r1.xyz = ((r0.xyzx)*(source[10].yyyy)).xyz;
    // 22: mad r0.xyz, cb0[10].zzzz, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[10].zzzz)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 23: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 24: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 25: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 26: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 27: mad r1.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 28: mad r4.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 29: mad r5.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 30: log r2.zw, |r2.xxxy|
    r2.zw = (log2(abs(r2.xxxy))).zw;
    // 31: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 32: mul r1.w, r2.w, cb0[13].x
    r1.w = ((r2.wwww)*(source[13].xxxx)).w;
    // 33: mul r2.z, r2.z, cb0[12].z
    r2.z = ((r2.zzzz)*(source[12].zzzz)).z;
    // 34: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 35: movc r2.x, r2.x, l(0), r2.z
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).x;
    // 36: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 37: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 38: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 39: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: movc r1.w, r2.y, l(0), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 41: mad r2.yzw, r1.wwww, r4.xxyz, r5.xxyz
    r2.yzw = ((r1.wwww)*(r4.xxyz)+(r5.xxyz)).yzw;
    // 42: mad r1.xyz, r2.yzwy, r1.wwww, r1.xyzx
    r1.xyz = ((r2.yzwy)*(r1.wwww)+(r1.xyzx)).xyz;
    // 43: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 44: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 45: dp3 r2.y, v7.xyzx, v7.xyzx
    r2.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 46: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 47: mul r2.yzw, r2.yyyy, v7.xxyz
    r2.yzw = ((r2.yyyy)*(v7.xxyz)).yzw;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 49: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 50: dp2 r4.x, r3.zwzz, r3.zwzz
    r4.x = (dot((r3.zwzz).xy,(r3.zwzz).xy).xxxx).x;
    // 51: mul r3.zw, r3.zzzw, cb0[8].xxxx
    r3.zw = ((r3.zzzw)*(source[8].xxxx)).zw;
    // 52: mul r5.xy, r3.zwzz, v2.wwww
    r5.xy = ((r3.zwzz)*(v2.wwww)).xy;
    // 53: add r3.z, -r4.x, l(1.000000)
    r3.z = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 54: max r3.z, r3.z, l(0.000000)
    r3.z = (max(r3.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 55: sqrt r3.z, r3.z
    r3.z = (sqrt(r3.zzzz)).z;
    // 56: add r5.z, r3.z, l(0.000010)
    r5.z = ((r3.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 57: dp3 r3.z, r5.xyzx, r5.xyzx
    r3.z = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 58: sqrt r3.z, r3.z
    r3.z = (sqrt(r3.zzzz)).z;
    // 59: div r4.xyz, r5.xyzx, r3.zzzz
    r4.xyz = ((r5.xyzx)/(r3.zzzz)).xyz;
    // 60: dp3 r3.z, r4.xyzx, r4.xyzx
    r3.z = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 61: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 62: mul r5.xyz, r3.zzzz, r4.xyzx
    r5.xyz = ((r3.zzzz)*(r4.xyzx)).xyz;
    // 63: dp3 r3.z, r2.yzwy, r5.xyzx
    r3.z = (dot((r2.yzwy).xyz,(r5.xyzx).xyz).xxxx).z;
    // 64: dp3 r2.y, -r2.yzwy, r5.xyzx
    r2.y = (dot((-(r2.yzwy)).xyz,(r5.xyzx).xyz).xxxx).y;
    // 65: mad r2.yz, r2.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r2.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 66: mad r3.zw, r3.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r3.zw = ((r3.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 67: mul r3.zw, r3.zzzw, r3.zzzw
    r3.zw = ((r3.zzzw)*(r3.zzzw)).zw;
    // 68: mul r6.xyz, r3.wwww, cb0[24].xyzx
    r6.xyz = ((r3.wwww)*(source[24].xyzx)).xyz;
    // 69: mad r6.xyz, r3.zzzz, cb0[23].xyzx, r6.xyzx
    r6.xyz = ((r3.zzzz)*(source[23].xyzx)+(r6.xyzx)).xyz;
    // 70: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 71: mul r7.xyz, r0.xyzx, r6.xyzx
    r7.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 72: dp2_sat r8.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 73: dp3_sat r8.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 74: dp3_sat r8.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 75: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 76: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t8.xyzw, s5
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 77: mul r9.xyz, r9.xyzx, cb0[27].xyzx
    r9.xyz = ((r9.xyzx)*(source[27].xyzx)).xyz;
    // 78: dp3 r2.w, r9.xyzx, r8.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 79: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t7.xyzw, s5
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 80: mul r8.xyz, r8.xyzx, cb0[26].xyzx
    r8.xyz = ((r8.xyzx)*(source[26].xyzx)).xyz;
    // 81: mul r10.xyz, r2.wwww, r8.xyzx
    r10.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 82: mad r7.xyz, r0.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r0.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 83: mul r7.xyz, r1.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 84: dp3 r3.z, v1.xyzx, v1.xyzx
    r3.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 85: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 86: mul r10.xyz, r3.zzzz, v1.xyzx
    r10.xyz = ((r3.zzzz)*(v1.xyzx)).xyz;
    // 87: dp3 r3.z, v0.xyzx, v0.xyzx
    r3.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 88: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 89: mul r11.xyz, r3.zzzz, v0.xyzx
    r11.xyz = ((r3.zzzz)*(v0.xyzx)).xyz;
    // 90: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 91: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 92: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 93: dp3 r13.y, r12.xyzx, r5.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 94: dp3 r13.x, r11.xyzx, r5.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 95: dp2 r14.z, r13.xyxx, cb0[15].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 96: mul r3.zw, cb0[15].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r3.zw = ((source[15].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 97: dp2 r14.x, r13.xyxx, r3.zwzz
    r14.x = (dot((r13.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 98: dp3 r14.y, r10.xyzx, r5.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 99: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 100: dp4 r13.x, cb0[16].xyzw, r14.xyzw
    r13.x = (dot((source[16].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 101: dp4 r13.y, cb0[17].xyzw, r14.xyzw
    r13.y = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 102: dp4 r13.z, cb0[18].xyzw, r14.xyzw
    r13.z = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 103: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 104: mul r4.w, r14.y, r14.y
    r4.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 105: mad r4.w, r14.x, r14.x, -r4.w
    r4.w = ((r14.xxxx)*(r14.xxxx)+(-(r4.wwww))).w;
    // 106: dp4 r14.x, cb0[19].xyzw, r15.xyzw
    r14.x = (dot((source[19].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 107: dp4 r14.y, cb0[20].xyzw, r15.xyzw
    r14.y = (dot((source[20].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 108: dp4 r14.z, cb0[21].xyzw, r15.xyzw
    r14.z = (dot((source[21].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 109: add r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)+(r14.xyzx)).xyz;
    // 110: mad r13.xyz, cb0[22].xyzx, r4.wwww, r13.xyzx
    r13.xyz = ((source[22].xyzx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 111: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 112: mul r13.xyz, r13.xyzx, cb0[14].xyzx
    r13.xyz = ((r13.xyzx)*(source[14].xyzx)).xyz;
    // 113: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 114: dp3 r4.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 115: dp3 r5.w, v6.xyzx, v6.xyzx
    r5.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 116: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 117: mul r14.xyz, r5.wwww, v6.xyzx
    r14.xyz = ((r5.wwww)*(v6.xyzx)).xyz;
    // 118: dp3 r5.w, r5.xyzx, r14.xyzx
    r5.w = (dot((r5.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 119: mul r5.xyz, r5.wwww, r5.xyzx
    r5.xyz = ((r5.wwww)*(r5.xyzx)).xyz;
    // 120: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 121: deriv_rtx_coarse r15.x, r5.w
    r15.x = (ddx_coarse(r5.wwww)).x;
    // 122: deriv_rty_coarse r15.y, r5.w
    r15.y = (ddy_coarse(r5.wwww)).y;
    // 123: dp2 r6.w, r15.xyxx, r15.xyxx
    r6.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 124: sqrt r6.w, r6.w
    r6.w = (sqrt(r6.wwww)).w;
    // 125: mad_sat r15.y, r6.w, l(0.300000), r2.x
    r15.y = (saturate((r6.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.xxxx))).y;
    // 126: mad r6.w, r15.y, l(0.200000), l(0.200000)
    r6.w = ((r15.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 127: div r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)/(r6.wwww)).w;
    // 128: dp3 r7.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r7.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: mad r4.w, r7.w, l(5.000000), r4.w
    r4.w = ((r7.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r4.wwww)).w;
    // 130: sample_b_indexable(texture2d)(float,float,float,float) r8.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r8.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 131: mad r3.x, r8.w, -r3.x, r3.x
    r3.x = ((r8.wwww)*(-(r3.xxxx))+(r3.xxxx)).x;
    // 132: add_sat r0.w, r0.w, r3.x
    r0.w = (saturate((r0.wwww)+(r3.xxxx))).w;
    // 133: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 134: add_sat r3.x, r0.w, r4.w
    r3.x = (saturate((r0.wwww)+(r4.wwww))).x;
    // 135: mad r4.w, r3.x, l(-2.000000), l(3.000000)
    r4.w = ((r3.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 136: mul r3.x, r3.x, r3.x
    r3.x = ((r3.xxxx)*(r3.xxxx)).x;
    // 137: mul r3.x, r3.x, r4.w
    r3.x = ((r3.xxxx)*(r4.wwww)).x;
    // 138: log r3.x, r3.x
    r3.x = (log2(r3.xxxx)).x;
    // 139: mul r3.x, r3.x, l(1.500000)
    r3.x = ((r3.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 140: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 141: mul r13.xyz, r3.xxxx, r13.xyzx
    r13.xyz = ((r3.xxxx)*(r13.xyzx)).xyz;
    // 142: mul r7.xyz, r7.xyzx, r13.xyzx
    r7.xyz = ((r7.xyzx)*(r13.xyzx)).xyz;
    // 143: add r3.x, -r3.y, l(1.000000)
    r3.x = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 144: mad_sat r3.x, r8.w, r3.x, r3.y
    r3.x = (saturate((r8.wwww)*(r3.xxxx)+(r3.yyyy))).x;
    // 145: mad r3.x, -r3.x, cb0[2].x, l(1.000000)
    r3.x = ((-(r3.xxxx))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 146: mul r3.y, r15.y, r15.y
    r3.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 147: mad r4.w, r3.y, l(0.350000), l(1.000000)
    r4.w = ((r3.yyyy)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: div_sat r3.x, r3.x, r4.w
    r3.x = (saturate((r3.xxxx)/(r4.wwww))).x;
    // 149: mov_sat r4.w, cb0[10].w
    r4.w = (saturate(source[10].wwww)).w;
    // 150: mad r16.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r16.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 151: mul r4.w, r4.w, l(0.080000)
    r4.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 152: mad r16.xyz, r0.wwww, r16.xyzx, r4.wwww
    r16.xyz = ((r0.wwww)*(r16.xyzx)+(r4.wwww)).xyz;
    // 153: add r4.w, r5.w, l(1.000000)
    r4.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: mov_sat r5.w, r5.w
    r5.w = (saturate(r5.wwww)).w;
    // 155: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 156: mul r5.w, r5.w, cb0[1].y
    r5.w = ((r5.wwww)*(source[1].yyyy)).w;
    // 157: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 158: mad_sat r5.w, r5.w, cb0[1].w, cb0[1].z
    r5.w = (saturate((r5.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 159: add r8.w, r5.z, l(1.000000)
    r8.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: min r8.w, r8.w, l(1.000000)
    r8.w = (min(r8.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: add_sat r15.x, r4.w, -r8.w
    r15.x = (saturate((r4.wwww)+(-(r8.wwww)))).x;
    // 162: add r15.zw, -r15.yyyx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r15.zw = ((-(r15.yyyx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 163: max r17.xyz, r16.xyzx, r15.zzzz
    r17.xyz = (max(r16.xyzx,r15.zzzz)).xyz;
    // 164: add r17.xyz, -r16.xyzx, r17.xyzx
    r17.xyz = ((-(r16.xyzx))+(r17.xyzx)).xyz;
    // 165: mul_sat r4.w, r16.y, l(50.000000)
    r4.w = (saturate((r16.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 166: mul r17.xyz, r4.wwww, r17.xyzx
    r17.xyz = ((r4.wwww)*(r17.xyzx)).xyz;
    // 167: sample_indexable(texture2d)(float,float,float,float) r18.xy, r15.xyxx, t5.xyzw, s7
    r18.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 168: mul r8.w, r15.y, l(5.000000)
    r8.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 169: add r9.w, r1.w, r15.x
    r9.w = ((r1.wwww)+(r15.xxxx)).w;
    // 170: log r9.w, r9.w
    r9.w = (log2(r9.wwww)).w;
    // 171: mul r3.y, r3.y, r9.w
    r3.y = ((r3.yyyy)*(r9.wwww)).y;
    // 172: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 173: add r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)+(r3.yyyy)).w;
    // 174: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 175: mul r15.xyz, r16.xyzx, r18.yyyy
    r15.xyz = ((r16.xyzx)*(r18.yyyy)).xyz;
    // 176: mad r15.xyz, r17.xyzx, r18.xxxx, r15.xyzx
    r15.xyz = ((r17.xyzx)*(r18.xxxx)+(r15.xyzx)).xyz;
    // 177: div r3.y, l(1.000000, 1.000000, 1.000000, 1.000000), r18.y
    r3.y = r18.y != 0.f ? 1.f / r18.y : 0.f;
    // 178: add r3.y, r3.y, l(-1.000000)
    r3.y = ((r3.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 179: mad r17.xyz, r16.xyzx, r3.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r16.xyzx)*(r3.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: mad r18.xyz, -r15.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r15.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: mul r15.xyz, r15.xyzx, r17.xyzx
    r15.xyz = ((r15.xyzx)*(r17.xyzx)).xyz;
    // 182: mul r3.y, r15.w, r15.w
    r3.y = ((r15.wwww)*(r15.wwww)).y;
    // 183: mul r3.y, r3.y, r3.y
    r3.y = ((r3.yyyy)*(r3.yyyy)).y;
    // 184: mad r9.w, -r3.y, r15.w, l(1.000000)
    r9.w = ((-(r3.yyyy))*(r15.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 185: mul r3.y, r15.w, r3.y
    r3.y = ((r15.wwww)*(r3.yyyy)).y;
    // 186: mul r17.xyz, r16.xyzx, r9.wwww
    r17.xyz = ((r16.xyzx)*(r9.wwww)).xyz;
    // 187: dp3 r9.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r9.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 188: mad r16.xyz, r9.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r16.xyz = ((r9.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 189: mad r17.xyz, r4.wwww, r3.yyyy, r17.xyzx
    r17.xyz = ((r4.wwww)*(r3.yyyy)+(r17.xyzx)).xyz;
    // 190: add r17.xyz, -r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r17.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 191: mul r17.xyz, r17.xyzx, r17.xyzx
    r17.xyz = ((r17.xyzx)*(r17.xyzx)).xyz;
    // 192: mad r19.xyz, -r3.xxxx, r17.xyzx, r18.xyzx
    r19.xyz = ((-(r3.xxxx))*(r17.xyzx)+(r18.xyzx)).xyz;
    // 193: mul r18.xyz, r0.xyzx, r18.xyzx
    r18.xyz = ((r0.xyzx)*(r18.xyzx)).xyz;
    // 194: mul r17.xyz, r3.xxxx, r17.xyzx
    r17.xyz = ((r3.xxxx)*(r17.xyzx)).xyz;
    // 195: mul r17.xyz, r0.xyzx, r17.xyzx
    r17.xyz = ((r0.xyzx)*(r17.xyzx)).xyz;
    // 196: mad r17.xyz, -r17.xyzx, r0.wwww, r17.xyzx
    r17.xyz = ((-(r17.xyzx))*(r0.wwww)+(r17.xyzx)).xyz;
    // 197: mul r7.xyz, r7.xyzx, r19.xyzx
    r7.xyz = ((r7.xyzx)*(r19.xyzx)).xyz;
    // 198: mad r7.xyz, -r7.xyzx, r0.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r0.wwww)+(r7.xyzx)).xyz;
    // 199: dp2_sat r19.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r19.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 200: dp3_sat r19.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r19.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 201: dp3_sat r19.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r19.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 202: mul r19.xyz, r19.xyzx, r19.xyzx
    r19.xyz = ((r19.xyzx)*(r19.xyzx)).xyz;
    // 203: dp3 r3.y, r9.xyzx, r19.xyzx
    r3.y = (dot((r9.xyzx).xyz,(r19.xyzx).xyz).xxxx).y;
    // 204: add r2.w, r2.w, -r3.y
    r2.w = ((r2.wwww)+(-(r3.yyyy))).w;
    // 205: mad r2.x, r2.x, r2.w, r3.y
    r2.x = ((r2.xxxx)*(r2.wwww)+(r3.yyyy)).x;
    // 206: mad r6.xyz, r8.xyzx, r2.xxxx, r6.xyzx
    r6.xyz = ((r8.xyzx)*(r2.xxxx)+(r6.xyzx)).xyz;
    // 207: mad r2.x, r1.w, r16.x, r16.y
    r2.x = ((r1.wwww)*(r16.xxxx)+(r16.yyyy)).x;
    // 208: mad r2.x, r2.x, r1.w, r16.z
    r2.x = ((r2.xxxx)*(r1.wwww)+(r16.zzzz)).x;
    // 209: mul r2.x, r1.w, r2.x
    r2.x = ((r1.wwww)*(r2.xxxx)).x;
    // 210: max r1.w, r1.w, r2.x
    r1.w = (max(r1.wwww,r2.xxxx)).w;
    // 211: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 212: dp3 r11.x, r11.xyzx, r5.xyzx
    r11.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 213: dp3 r11.y, r12.xyzx, r5.xyzx
    r11.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 214: dp3 r5.y, r10.xyzx, r5.xyzx
    r5.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 215: dp2 r5.x, r11.xyxx, r3.zwzz
    r5.x = (dot((r11.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 216: dp2 r5.z, r11.xyxx, cb0[15].xyxx
    r5.z = (dot((r11.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 217: sample_l_indexable(texturecube)(float,float,float,float) r10.xyzw, r5.xyzx, t6.xyzw, s6, r8.w
    r10.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r8.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 218: mul r3.yzw, r10.xxyz, r10.wwww
    r3.yzw = ((r10.xxyz)*(r10.wwww)).yzw;
    // 219: mul r3.yzw, r3.yyzw, cb0[14].xxyz
    r3.yzw = ((r3.yyzw)*(source[14].xxyz)).yzw;
    // 220: mad r3.yzw, r3.yyzw, l(0.000000, 6.000000, 6.000000, 6.000000), cb0[14].wwww
    r3.yzw = ((r3.yyzw)*(float4(0.000000,6.000000,6.000000,6.000000))+(source[14].wwww)).yzw;
    // 221: dp3 r2.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 222: div r2.x, r2.x, r6.w
    r2.x = ((r2.xxxx)/(r6.wwww)).x;
    // 223: mad r2.x, r7.w, l(5.000000), r2.x
    r2.x = ((r7.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.xxxx)).x;
    // 224: add_sat r2.x, r0.w, r2.x
    r2.x = (saturate((r0.wwww)+(r2.xxxx))).x;
    // 225: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 226: mad r2.w, r2.x, l(-2.000000), l(3.000000)
    r2.w = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 227: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 228: mul r2.xyz, r2.xyzx, r2.wyzw
    r2.xyz = ((r2.xyzx)*(r2.wyzw)).xyz;
    // 229: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 230: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 231: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 232: mul r3.yzw, r2.xxxx, r3.yyzw
    r3.yzw = ((r2.xxxx)*(r3.yyzw)).yzw;
    // 233: mul r5.xyz, r6.xyzx, r3.yzwy
    r5.xyz = ((r6.xyzx)*(r3.yzwy)).xyz;
    // 234: mul r3.yzw, r3.yyzw, r15.xxyz
    r3.yzw = ((r3.yyzw)*(r15.xxyz)).yzw;
    // 235: mad r5.xyz, r5.xyzx, r15.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r15.xyzx)+(r7.xyzx)).xyz;
    // 236: mul r2.xzw, r2.zzzz, cb0[24].xxyz
    r2.xzw = ((r2.zzzz)*(source[24].xxyz)).xzw;
    // 237: mad r2.xyz, r2.yyyy, cb0[23].xyzx, r2.xzwx
    r2.xyz = ((r2.yyyy)*(source[23].xyzx)+(r2.xzwx)).xyz;
    // 238: mul r2.xyz, r2.xyzx, cb0[25].wwww
    r2.xyz = ((r2.xyzx)*(source[25].wwww)).xyz;
    // 239: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 240: add_sat r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (saturate((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 241: add_sat r6.xyz, r6.xyzx, cb0[13].zzzz
    r6.xyz = (saturate((r6.xyzx)+(source[13].zzzz))).xyz;
    // 242: mul r2.w, r6.x, cb0[13].w
    r2.w = ((r6.xxxx)*(source[13].wwww)).w;
    // 243: mul_sat r6.xyz, r6.xyzx, cb0[7].xyzx
    r6.xyz = (saturate((r6.xyzx)*(source[7].xyzx))).xyz;
    // 244: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 245: mul r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 246: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 247: mul r7.xyz, r0.wwww, r18.xyzx
    r7.xyz = ((r0.wwww)*(r18.xyzx)).xyz;
    // 248: mul r7.xyz, r13.xyzx, r7.xyzx
    r7.xyz = ((r13.xyzx)*(r7.xyzx)).xyz;
    // 249: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 250: mad r1.xyz, r3.yzwy, r1.wwww, r1.xyzx
    r1.xyz = ((r3.yzwy)*(r1.wwww)+(r1.xyzx)).xyz;
    // 251: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 252: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 253: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 254: dp3 r0.w, r9.xyzx, r0.wwww
    r0.w = (dot((r9.xyzx).xyz,(r0.wwww).xyz).xxxx).w;
    // 255: mul r3.yzw, r0.wwww, r8.xxyz
    r3.yzw = ((r0.wwww)*(r8.xxyz)).yzw;
    // 256: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 257: mad r0.xyz, r0.xyzx, r3.yzwy, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r3.yzwy)+(r2.xyzx)).xyz;
    // 258: dp3 r0.w, r4.xyzx, r14.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 259: add r1.w, -|r14.z|, l(1.000000)
    r1.w = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 260: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 261: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 262: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 263: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 264: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 265: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 266: mul r2.xyz, r0.wwww, cb0[4].xyzx
    r2.xyz = ((r0.wwww)*(source[4].xyzx)).xyz;
    // 267: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 268: add r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(source[3].xyzx)).xyz;
    // 269: add r0.w, -r3.x, l(1.000000)
    r0.w = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 270: mad r2.xyz, r1.xyzx, r0.wwww, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 271: mul r1.xyz, r3.xxxx, r1.xyzx
    r1.xyz = ((r3.xxxx)*(r1.xyzx)).xyz;
    // 272: mul r3.xyz, r3.xxxx, r0.xyzx
    r3.xyz = ((r3.xxxx)*(r0.xyzx)).xyz;
    // 273: mad r0.xyz, r0.xyzx, r0.wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 274: add r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)+(r5.xyzx)).xyz;
    // 275: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 276: mad r0.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r17.xyzx
    r0.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r17.xyzx)).xyz;
    // 277: mad r0.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 278: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 279: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 280: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 281: ret
    return output;
}

// source.character.static-map-native-1109.v1 / source program 4cb32a2c995b594e89cd7046c24840ea
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1109(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1109(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: mul r0.xyz, cb0[6].xyzx, cb0[9].xxxx
    r0.xyz = ((source[6].xyzx)*(source[9].xxxx)).xyz;
    // 2: mul r1.xyz, cb0[5].xyzx, cb0[8].wwww
    r1.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r4.xyz, cb0[8].zzzz, r3.xyzx, r2.xyzx
    r4.xyz = ((source[8].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mad r2.xyz, cb0[9].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[9].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 8: add r3.xy, r2.wwww, cb0[11].wyww
    r3.xy = ((r2.wwww)+(source[11].wyww)).xy;
    // 9: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 10: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 12: mul r0.w, r2.z, cb0[9].w
    r0.w = ((r2.zzzz)*(source[9].wwww)).w;
    // 13: mul r2.xy, r2.yxyy, cb0[12].ywyy
    r2.xy = ((r2.yxyy)*(source[12].ywyy)).xy;
    // 14: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 15: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 16: mul r1.w, r1.w, cb0[10].x
    r1.w = ((r1.wwww)*(source[10].xxxx)).w;
    // 17: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 18: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 19: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 21: mul r1.xyz, r0.xyzx, cb0[10].yyyy
    r1.xyz = ((r0.xyzx)*(source[10].yyyy)).xyz;
    // 22: mad r0.xyz, cb0[10].zzzz, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[10].zzzz)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 23: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 24: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 25: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 26: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 27: mad r1.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 28: mad r4.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 29: mad r5.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 30: log r2.zw, |r2.xxxy|
    r2.zw = (log2(abs(r2.xxxy))).zw;
    // 31: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 32: mul r1.w, r2.w, cb0[13].x
    r1.w = ((r2.wwww)*(source[13].xxxx)).w;
    // 33: mul r2.z, r2.z, cb0[12].z
    r2.z = ((r2.zzzz)*(source[12].zzzz)).z;
    // 34: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 35: movc r2.x, r2.x, l(0), r2.z
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).x;
    // 36: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 37: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 38: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 39: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: movc r1.w, r2.y, l(0), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 41: mad r2.yzw, r1.wwww, r4.xxyz, r5.xxyz
    r2.yzw = ((r1.wwww)*(r4.xxyz)+(r5.xxyz)).yzw;
    // 42: mad r1.xyz, r2.yzwy, r1.wwww, r1.xyzx
    r1.xyz = ((r2.yzwy)*(r1.wwww)+(r1.xyzx)).xyz;
    // 43: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 44: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 46: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 47: dp2 r2.w, r2.yzyy, r2.yzyy
    r2.w = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).w;
    // 48: mul r2.yz, r2.yyzy, cb0[8].xxxx
    r2.yz = ((r2.yyzy)*(source[8].xxxx)).yz;
    // 49: mul r4.xy, r2.yzyy, v2.wwww
    r4.xy = ((r2.yzyy)*(v2.wwww)).xy;
    // 50: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 52: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 53: add r4.z, r2.y, l(0.000010)
    r4.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 54: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 55: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 56: div r2.yzw, r4.xxyz, r2.yyyy
    r2.yzw = ((r4.xxyz)/(r2.yyyy)).yzw;
    // 57: dp3 r3.z, r2.yzwy, r2.yzwy
    r3.z = (dot((r2.yzwy).xyz,(r2.yzwy).xyz).xxxx).z;
    // 58: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 59: mul r4.xyz, r2.yzwy, r3.zzzz
    r4.xyz = ((r2.yzwy)*(r3.zzzz)).xyz;
    // 60: dp3 r3.z, v7.xyzx, v7.xyzx
    r3.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 61: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 62: mul r5.xyz, r3.zzzz, v7.xyzx
    r5.xyz = ((r3.zzzz)*(v7.xyzx)).xyz;
    // 63: dp3 r3.z, r5.xyzx, r4.xyzx
    r3.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 64: mad r3.zw, r3.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r3.zw = ((r3.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 65: mul r3.zw, r3.zzzw, r3.zzzw
    r3.zw = ((r3.zzzw)*(r3.zzzw)).zw;
    // 66: mul r6.xyz, r3.wwww, cb0[24].xyzx
    r6.xyz = ((r3.wwww)*(source[24].xyzx)).xyz;
    // 67: mad r6.xyz, r3.zzzz, cb0[23].xyzx, r6.xyzx
    r6.xyz = ((r3.zzzz)*(source[23].xyzx)+(r6.xyzx)).xyz;
    // 68: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 69: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 70: mul r6.xyz, r1.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 71: dp3 r3.z, v1.xyzx, v1.xyzx
    r3.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 72: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 73: mul r7.xyz, r3.zzzz, v1.xyzx
    r7.xyz = ((r3.zzzz)*(v1.xyzx)).xyz;
    // 74: dp3 r3.z, v0.xyzx, v0.xyzx
    r3.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 75: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 76: mul r8.xyz, r3.zzzz, v0.xyzx
    r8.xyz = ((r3.zzzz)*(v0.xyzx)).xyz;
    // 77: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 78: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 79: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 80: dp3 r10.y, r9.xyzx, r4.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 81: dp3 r10.x, r8.xyzx, r4.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 82: dp2 r11.z, r10.xyxx, cb0[15].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 83: mul r3.zw, cb0[15].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r3.zw = ((source[15].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 84: dp2 r11.x, r10.xyxx, r3.zwzz
    r11.x = (dot((r10.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 85: dp3 r11.y, r7.xyzx, r4.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 86: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 87: dp4 r10.x, cb0[16].xyzw, r11.xyzw
    r10.x = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 88: dp4 r10.y, cb0[17].xyzw, r11.xyzw
    r10.y = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 89: dp4 r10.z, cb0[18].xyzw, r11.xyzw
    r10.z = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 90: mul r12.xyzw, r11.yzzx, r11.xyzz
    r12.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 91: mul r4.w, r11.y, r11.y
    r4.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 92: mad r4.w, r11.x, r11.x, -r4.w
    r4.w = ((r11.xxxx)*(r11.xxxx)+(-(r4.wwww))).w;
    // 93: dp4 r11.x, cb0[19].xyzw, r12.xyzw
    r11.x = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 94: dp4 r11.y, cb0[20].xyzw, r12.xyzw
    r11.y = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 95: dp4 r11.z, cb0[21].xyzw, r12.xyzw
    r11.z = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 96: add r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)+(r11.xyzx)).xyz;
    // 97: mad r10.xyz, cb0[22].xyzx, r4.wwww, r10.xyzx
    r10.xyz = ((source[22].xyzx)*(r4.wwww)+(r10.xyzx)).xyz;
    // 98: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 99: mul r10.xyz, r10.xyzx, cb0[14].xyzx
    r10.xyz = ((r10.xyzx)*(source[14].xyzx)).xyz;
    // 100: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 101: dp3 r4.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: dp3 r5.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: dp3 r6.w, v6.xyzx, v6.xyzx
    r6.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 104: rsq r6.w, r6.w
    r6.w = (rsqrt(r6.wwww)).w;
    // 105: mul r11.xyz, r6.wwww, v6.xyzx
    r11.xyz = ((r6.wwww)*(v6.xyzx)).xyz;
    // 106: dp3 r6.w, r4.xyzx, r11.xyzx
    r6.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 107: deriv_rtx_coarse r12.x, r6.w
    r12.x = (ddx_coarse(r6.wwww)).x;
    // 108: deriv_rty_coarse r12.y, r6.w
    r12.y = (ddy_coarse(r6.wwww)).y;
    // 109: dp2 r7.w, r12.xyxx, r12.xyxx
    r7.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 110: sqrt r7.w, r7.w
    r7.w = (sqrt(r7.wwww)).w;
    // 111: mad_sat r12.y, r7.w, l(0.300000), r2.x
    r12.y = (saturate((r7.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.xxxx))).y;
    // 112: mad r2.x, r12.y, l(0.200000), l(0.200000)
    r2.x = ((r12.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 113: div r5.w, r5.w, r2.x
    r5.w = ((r5.wwww)/(r2.xxxx)).w;
    // 114: mad r5.w, r4.w, l(5.000000), r5.w
    r5.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r5.wwww)).w;
    // 115: sample_b_indexable(texture2d)(float,float,float,float) r7.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r7.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 116: mad r3.x, r7.w, -r3.x, r3.x
    r3.x = ((r7.wwww)*(-(r3.xxxx))+(r3.xxxx)).x;
    // 117: add_sat r0.w, r0.w, r3.x
    r0.w = (saturate((r0.wwww)+(r3.xxxx))).w;
    // 118: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 119: add_sat r3.x, r0.w, r5.w
    r3.x = (saturate((r0.wwww)+(r5.wwww))).x;
    // 120: mad r5.w, r3.x, l(-2.000000), l(3.000000)
    r5.w = ((r3.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 121: mul r3.x, r3.x, r3.x
    r3.x = ((r3.xxxx)*(r3.xxxx)).x;
    // 122: mul r3.x, r3.x, r5.w
    r3.x = ((r3.xxxx)*(r5.wwww)).x;
    // 123: log r3.x, r3.x
    r3.x = (log2(r3.xxxx)).x;
    // 124: mul r3.x, r3.x, l(1.500000)
    r3.x = ((r3.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 125: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 126: mul r10.xyz, r3.xxxx, r10.xyzx
    r10.xyz = ((r3.xxxx)*(r10.xyzx)).xyz;
    // 127: mul r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 128: mul r13.xyz, r4.xyzx, r6.wwww
    r13.xyz = ((r4.xyzx)*(r6.wwww)).xyz;
    // 129: dp3 r3.x, -r5.xyzx, r4.xyzx
    r3.x = (dot((-(r5.xyzx)).xyz,(r4.xyzx).xyz).xxxx).x;
    // 130: mad r4.xy, r3.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r3.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 131: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 132: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 133: add r3.x, r13.z, l(1.000000)
    r3.x = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 134: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: add r4.z, r6.w, l(1.000000)
    r4.z = ((r6.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 136: mov_sat r6.w, r6.w
    r6.w = (saturate(r6.wwww)).w;
    // 137: log r5.w, r6.w
    r5.w = (log2(r6.wwww)).w;
    // 138: mul r5.w, r5.w, cb0[1].y
    r5.w = ((r5.wwww)*(source[1].yyyy)).w;
    // 139: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 140: mad_sat r5.w, r5.w, cb0[1].w, cb0[1].z
    r5.w = (saturate((r5.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 141: add_sat r12.x, -r3.x, r4.z
    r12.x = (saturate((-(r3.xxxx))+(r4.zzzz))).x;
    // 142: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t5.zwxy, s6
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 143: add r14.xy, -r12.yxyy, l(1.000000, 1.000000, 0.000000, 0.000000)
    r14.xy = ((-(r12.yxyy))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 144: add r3.x, r1.w, r12.x
    r3.x = ((r1.wwww)+(r12.xxxx)).x;
    // 145: log r3.x, r3.x
    r3.x = (log2(r3.xxxx)).x;
    // 146: mov_sat r4.z, cb0[10].w
    r4.z = (saturate(source[10].wwww)).z;
    // 147: mad r15.xyz, -r4.zzzz, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r15.xyz = ((-(r4.zzzz))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 148: mul r4.z, r4.z, l(0.080000)
    r4.z = ((r4.zzzz)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 149: mad r15.xyz, r0.wwww, r15.xyzx, r4.zzzz
    r15.xyz = ((r0.wwww)*(r15.xyzx)+(r4.zzzz)).xyz;
    // 150: max r14.xzw, r14.xxxx, r15.xxyz
    r14.xzw = (max(r14.xxxx,r15.xxyz)).xzw;
    // 151: add r14.xzw, -r15.xxyz, r14.xxzw
    r14.xzw = ((-(r15.xxyz))+(r14.xxzw)).xzw;
    // 152: mul_sat r4.z, r15.y, l(50.000000)
    r4.z = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 153: mul r14.xzw, r4.zzzz, r14.xxzw
    r14.xzw = ((r4.zzzz)*(r14.xxzw)).xzw;
    // 154: mul r16.xyz, r12.wwww, r15.xyzx
    r16.xyz = ((r12.wwww)*(r15.xyzx)).xyz;
    // 155: mad r14.xzw, r14.xxzw, r12.zzzz, r16.xxyz
    r14.xzw = ((r14.xxzw)*(r12.zzzz)+(r16.xxyz)).xzw;
    // 156: div r6.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r6.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 157: add r6.w, r6.w, l(-1.000000)
    r6.w = ((r6.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 158: mad r12.xzw, r15.xxyz, r6.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r15.xxyz)*(r6.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 159: mad r16.xyz, -r14.xzwx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xzwx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 160: mul r12.xzw, r12.xxzw, r14.xxzw
    r12.xzw = ((r12.xxzw)*(r14.xxzw)).xzw;
    // 161: mul r6.w, r14.y, r14.y
    r6.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 162: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 163: mul r8.w, r14.y, r6.w
    r8.w = ((r14.yyyy)*(r6.wwww)).w;
    // 164: mad r6.w, -r6.w, r14.y, l(1.000000)
    r6.w = ((-(r6.wwww))*(r14.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: mul r14.xyz, r15.xyzx, r6.wwww
    r14.xyz = ((r15.xyzx)*(r6.wwww)).xyz;
    // 166: dp3 r6.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 167: mad r15.xyz, r6.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r6.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 168: mad r14.xyz, r4.zzzz, r8.wwww, r14.xyzx
    r14.xyz = ((r4.zzzz)*(r8.wwww)+(r14.xyzx)).xyz;
    // 169: add r14.xyz, -r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r14.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mul r14.xyz, r14.xyzx, r14.xyzx
    r14.xyz = ((r14.xyzx)*(r14.xyzx)).xyz;
    // 171: add r4.z, -r3.y, l(1.000000)
    r4.z = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 172: mad_sat r3.y, r7.w, r4.z, r3.y
    r3.y = (saturate((r7.wwww)*(r4.zzzz)+(r3.yyyy))).y;
    // 173: mad r3.y, -r3.y, cb0[2].x, l(1.000000)
    r3.y = ((-(r3.yyyy))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 174: mul r4.z, r12.y, r12.y
    r4.z = ((r12.yyyy)*(r12.yyyy)).z;
    // 175: mul r6.w, r12.y, l(5.000000)
    r6.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 176: mad r7.w, r4.z, l(0.350000), l(1.000000)
    r7.w = ((r4.zzzz)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r3.x, r3.x, r4.z
    r3.x = ((r3.xxxx)*(r4.zzzz)).x;
    // 178: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 179: add r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)+(r3.xxxx)).w;
    // 180: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 181: div_sat r3.x, r3.y, r7.w
    r3.x = (saturate((r3.yyyy)/(r7.wwww))).x;
    // 182: mad r17.xyz, -r3.xxxx, r14.xyzx, r16.xyzx
    r17.xyz = ((-(r3.xxxx))*(r14.xyzx)+(r16.xyzx)).xyz;
    // 183: mul r16.xyz, r0.xyzx, r16.xyzx
    r16.xyz = ((r0.xyzx)*(r16.xyzx)).xyz;
    // 184: mul r14.xyz, r14.xyzx, r3.xxxx
    r14.xyz = ((r14.xyzx)*(r3.xxxx)).xyz;
    // 185: mul r14.xyz, r0.xyzx, r14.xyzx
    r14.xyz = ((r0.xyzx)*(r14.xyzx)).xyz;
    // 186: mad r14.xyz, -r14.xyzx, r0.wwww, r14.xyzx
    r14.xyz = ((-(r14.xyzx))*(r0.wwww)+(r14.xyzx)).xyz;
    // 187: mul r6.xyz, r6.xyzx, r17.xyzx
    r6.xyz = ((r6.xyzx)*(r17.xyzx)).xyz;
    // 188: mad r6.xyz, -r6.xyzx, r0.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r0.wwww)+(r6.xyzx)).xyz;
    // 189: dp3 r8.x, r8.xyzx, r13.xyzx
    r8.x = (dot((r8.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 190: dp3 r8.y, r9.xyzx, r13.xyzx
    r8.y = (dot((r9.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 191: dp2 r9.x, r8.xyxx, r3.zwzz
    r9.x = (dot((r8.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 192: dp2 r9.z, r8.xyxx, cb0[15].xyxx
    r9.z = (dot((r8.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 193: dp3 r9.y, r7.xyzx, r13.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 194: dp3 r3.y, r5.xyzx, r13.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 195: mad r3.yz, r3.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r3.yz = ((r3.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 196: mul r3.yz, r3.yyzy, r3.yyzy
    r3.yz = ((r3.yyzy)*(r3.yyzy)).yz;
    // 197: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r9.xyzx, t6.xyzw, s5, r6.w
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r6.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 198: mul r5.xyz, r7.xyzx, r7.wwww
    r5.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 199: mul r5.xyz, r5.xyzx, cb0[14].xyzx
    r5.xyz = ((r5.xyzx)*(source[14].xyzx)).xyz;
    // 200: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 201: dp3 r3.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 202: div r2.x, r3.w, r2.x
    r2.x = ((r3.wwww)/(r2.xxxx)).x;
    // 203: mad r2.x, r4.w, l(5.000000), r2.x
    r2.x = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.xxxx)).x;
    // 204: add_sat r2.x, r0.w, r2.x
    r2.x = (saturate((r0.wwww)+(r2.xxxx))).x;
    // 205: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 206: mad r3.w, r2.x, l(-2.000000), l(3.000000)
    r3.w = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 207: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 208: mul r2.x, r2.x, r3.w
    r2.x = ((r2.xxxx)*(r3.wwww)).x;
    // 209: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 210: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 211: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 212: mul r5.xyz, r2.xxxx, r5.xyzx
    r5.xyz = ((r2.xxxx)*(r5.xyzx)).xyz;
    // 213: mad r2.x, r1.w, r15.x, r15.y
    r2.x = ((r1.wwww)*(r15.xxxx)+(r15.yyyy)).x;
    // 214: mad r2.x, r2.x, r1.w, r15.z
    r2.x = ((r2.xxxx)*(r1.wwww)+(r15.zzzz)).x;
    // 215: mul r2.x, r1.w, r2.x
    r2.x = ((r1.wwww)*(r2.xxxx)).x;
    // 216: max r1.w, r1.w, r2.x
    r1.w = (max(r1.wwww,r2.xxxx)).w;
    // 217: mul r7.xyz, r3.zzzz, cb0[24].xyzx
    r7.xyz = ((r3.zzzz)*(source[24].xyzx)).xyz;
    // 218: mad r3.yzw, cb0[23].xxyz, r3.yyyy, r7.xxyz
    r3.yzw = ((source[23].xxyz)*(r3.yyyy)+(r7.xxyz)).yzw;
    // 219: mul r3.yzw, r3.yyzw, cb0[25].wwww
    r3.yzw = ((r3.yyzw)*(source[25].wwww)).yzw;
    // 220: mul r3.yzw, r1.wwww, r3.yyzw
    r3.yzw = ((r1.wwww)*(r3.yyzw)).yzw;
    // 221: mul r3.yzw, r3.yyzw, r5.xxyz
    r3.yzw = ((r3.yyzw)*(r5.xxyz)).yzw;
    // 222: mul r5.xyz, r5.xyzx, r12.xzwx
    r5.xyz = ((r5.xyzx)*(r12.xzwx)).xyz;
    // 223: mad r3.yzw, r3.yyzw, r12.xxzw, r6.xxyz
    r3.yzw = ((r3.yyzw)*(r12.xxzw)+(r6.xxyz)).yzw;
    // 224: dp3 r2.x, r2.yzwy, r11.xyzx
    r2.x = (dot((r2.yzwy).xyz,(r11.xyzx).xyz).xxxx).x;
    // 225: add r2.y, -|r11.z|, l(1.000000)
    r2.y = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 226: add r2.x, -|r2.x|, l(1.000000)
    r2.x = ((-(abs(r2.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 227: mul r2.x, r2.x, r2.y
    r2.x = ((r2.xxxx)*(r2.yyyy)).x;
    // 228: lt r2.y, |r2.x|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 229: log r2.x, |r2.x|
    r2.x = (log2(abs(r2.xxxx))).x;
    // 230: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 231: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 232: mul r2.xzw, r2.xxxx, cb0[4].xxyz
    r2.xzw = ((r2.xxxx)*(source[4].xxyz)).xzw;
    // 233: movc r2.xyz, r2.yyyy, l(0,0,0,0), r2.xzwx
    r2.xyz = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xzwx)).xyz;
    // 234: add r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(source[3].xyzx)).xyz;
    // 235: mul r6.xyz, r0.wwww, r16.xyzx
    r6.xyz = ((r0.wwww)*(r16.xyzx)).xyz;
    // 236: mul r6.xyz, r10.xyzx, r6.xyzx
    r6.xyz = ((r10.xyzx)*(r6.xyzx)).xyz;
    // 237: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 238: mad r1.xyz, r5.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r5.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 239: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 240: add r1.w, -r3.x, l(1.000000)
    r1.w = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 241: mad r2.xyz, r1.xyzx, r1.wwww, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 242: mul r1.xyz, r3.xxxx, r1.xyzx
    r1.xyz = ((r3.xxxx)*(r1.xyzx)).xyz;
    // 243: mad r1.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r14.xyzx)).xyz;
    // 244: mul r4.yzw, r4.yyyy, cb0[24].xxyz
    r4.yzw = ((r4.yyyy)*(source[24].xxyz)).yzw;
    // 245: mad r4.xyz, r4.xxxx, cb0[23].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[23].xyzx)+(r4.yzwy)).xyz;
    // 246: mul r4.xyz, r4.xyzx, cb0[25].wwww
    r4.xyz = ((r4.xyzx)*(source[25].wwww)).xyz;
    // 247: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 248: add_sat r5.xyz, -r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (saturate((-(r5.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 249: add_sat r5.xyz, r5.xyzx, cb0[13].zzzz
    r5.xyz = (saturate((r5.xyzx)+(source[13].zzzz))).xyz;
    // 250: mul r2.w, r5.x, cb0[13].w
    r2.w = ((r5.xxxx)*(source[13].wwww)).w;
    // 251: mul_sat r5.xyz, r5.xyzx, cb0[7].xyzx
    r5.xyz = (saturate((r5.xyzx)*(source[7].xyzx))).xyz;
    // 252: mul r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)*(r5.wwww)).w;
    // 253: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 254: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 255: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 256: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 257: mad r2.xyz, r0.xyzx, r1.wwww, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 258: mul r0.xyz, r3.xxxx, r0.xyzx
    r0.xyz = ((r3.xxxx)*(r0.xyzx)).xyz;
    // 259: mad r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 260: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 261: add r0.xyz, r2.xyzx, r3.yzwy
    r0.xyz = ((r2.xyzx)+(r3.yzwy)).xyz;
    // 262: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 263: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 264: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 265: ret
    return output;
}

// source.character.static-map-native-1110.v1 / source program d74cffcab2f1ff4a97d6f1b214a373a9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1110(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[8]=1.f;
    source[9]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[3].yyyy
    r0.xy = ((v4.xyxx)*(source[3].yyyy)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: mul r0.xy, r0.xyxx, cb0[3].zzzz
    r0.xy = ((r0.xyxx)*(source[3].zzzz)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 7: mad r0.xy, cb0[3].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[3].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 8: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 9: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 10: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 11: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 12: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 14: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 15: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 16: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 17: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 18: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 19: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 20: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 23: dp3 r0.w, r0.xyzx, r1.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 24: mul r2.xyz, r0.wwww, r0.xyzx
    r2.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 25: mad r1.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 26: dp2_sat r2.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 27: dp3_sat r2.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 28: dp3_sat r2.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 29: log r1.xyz, r2.xyzx
    r1.xyz = (log2(r2.xyzx)).xyz;
    // 30: add r0.w, cb0[4].y, l(1.000000)
    r0.w = ((source[4].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 32: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 33: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t4.xyzw, s3
    r2.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 34: mul r2.xyz, r2.xyzx, cb0[9].xyzx
    r2.xyz = ((r2.xyzx)*(source[9].xyzx)).xyz;
    // 35: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 36: dp2_sat r1.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r1.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 37: dp3_sat r1.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r1.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 38: dp3_sat r1.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r1.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 39: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 40: dp3 r1.x, r2.xyzx, r1.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 41: sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t3.wxyz, s3
    r1.yzw = ((float4(input.bakedAverage,1.f)).wxyz).yzw;
    // 42: mul r1.yzw, r1.yyzw, cb0[8].xxyz
    r1.yzw = ((r1.yyzw)*(source[8].xxyz)).yzw;
    // 43: mul r2.xyz, r1.xxxx, r1.yzwy
    r2.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 44: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 45: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 46: mul r3.xyz, r2.wwww, v6.xyzx
    r3.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 47: dp3 r2.w, r3.xyzx, r0.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 48: mad r3.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 49: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 50: mul r3.yzw, r3.yyyy, cb0[6].xxyz
    r3.yzw = ((r3.yyyy)*(source[6].xxyz)).yzw;
    // 51: mad r3.xyz, r3.xxxx, cb0[5].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[5].xyzx)+(r3.yzwy)).xyz;
    // 52: mul r3.xyz, r3.xyzx, cb0[7].wwww
    r3.xyz = ((r3.xyzx)*(source[7].wwww)).xyz;
    // 53: mul r4.xyz, cb0[1].xyzx, cb0[3].wwww
    r4.xyz = ((source[1].xyzx)*(source[3].wwww)).xyz;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 55: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 56: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 57: mul r6.xyz, r3.xyzx, r4.xyzx
    r6.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 58: mad r3.xyz, r1.yzwy, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.yzwy)*(r1.xxxx)+(r3.xyzx)).xyz;
    // 59: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 60: div r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)/(r3.xyzx)).xyz;
    // 61: mad r2.xyz, r4.xyzx, r2.xyzx, r6.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 62: dp3 r1.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 63: mul r3.xyz, cb0[2].xyzx, cb0[4].xxxx
    r3.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // 64: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 65: mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 66: mul r1.yzw, r1.yyzw, r3.xxyz
    r1.yzw = ((r1.yyzw)*(r3.xxyz)).yzw;
    // 67: mad r2.xyz, r1.yzwy, r0.wwww, r2.xyzx
    r2.xyz = ((r1.yzwy)*(r0.wwww)+(r2.xyzx)).xyz;
    // 68: mul r1.yzw, r0.wwww, r1.yyzw
    r1.yzw = ((r0.wwww)*(r1.yyzw)).yzw;
    // 69: dp3 o4.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 70: add r1.yzw, r2.xxyz, cb0[0].xxyz
    r1.yzw = ((r2.xxyz)+(source[0].xxyz)).yzw;
    // 71: mad o0.xyz, r4.xyzx, cb0[7].xyzx, r1.yzwy
    output.targets[0].xyz = ((r4.xyzx)*(source[7].xyzx)+(r1.yzwy)).xyz;
    // 72: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 73: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 74: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 75: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 76: mul r1.yzw, r0.wwww, v1.xxyz
    r1.yzw = ((r0.wwww)*(v1.xxyz)).yzw;
    // 77: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 78: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 79: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 80: mul r4.xyz, r1.wyzw, r3.yzxy
    r4.xyz = ((r1.wyzw)*(r3.yzxy)).xyz;
    // 81: mad r4.xyz, r1.zwyz, r3.zxyz, -r4.xyzx
    r4.xyz = ((r1.zwyz)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 82: dp3 r5.z, r1.yzwy, r0.xyzx
    r5.z = (dot((r1.yzwy).xyz,(r0.xyzx).xyz).xxxx).z;
    // 83: dp3 r5.x, r3.xyzx, r0.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 84: mul r1.yzw, r4.xxyz, v1.wwww
    r1.yzw = ((r4.xxyz)*(v1.wwww)).yzw;
    // 85: dp3 r5.y, r1.yzwy, r0.xyzx
    r5.y = (dot((r1.yzwy).xyz,(r0.xyzx).xyz).xxxx).y;
    // 86: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 87: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 88: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 89: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 90: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 91: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 92: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 93: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 94: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 95: movc r0.xy, r0.wwww, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 96: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 97: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 98: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 99: mul o4.z, r1.x, r2.x
    output.targets[4].z = ((r1.xxxx)*(r2.xxxx)).z;
    // 100: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 101: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 102: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 103: ret
    return output;
}

// source.character.static-map-native-1110.v1 / source program 67dc323a84899d488b6e16be353296a3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1110(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1110(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[3];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[2].yyyy
    r0.xy = ((v4.xyxx)*(source[2].yyyy)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: mul r0.xy, r0.xyxx, cb0[2].zzzz
    r0.xy = ((r0.xyxx)*(source[2].zzzz)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 7: mad r0.xy, cb0[2].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[2].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 8: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 9: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 10: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 11: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 12: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 14: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 15: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 16: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 17: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 18: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 19: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 20: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 23: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 24: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 26: mul r1.yzw, r1.yyyy, cb0[4].xxyz
    r1.yzw = ((r1.yyyy)*(source[4].xxyz)).yzw;
    // 27: mad r1.xyz, r1.xxxx, cb0[3].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[3].xyzx)+(r1.yzwy)).xyz;
    // 28: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 30: mul r3.xyz, cb0[1].xyzx, cb0[2].wwww
    r3.xyz = ((source[1].xyzx)*(source[2].wwww)).xyz;
    // 31: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 32: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 33: mad r3.xyz, r1.xyzx, r2.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)+(source[0].xyzx)).xyz;
    // 34: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 35: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 36: mad o0.xyz, r2.xyzx, cb0[5].xyzx, r3.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[5].xyzx)+(r3.xyzx)).xyz;
    // 37: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 38: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 39: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 40: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 41: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 42: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 43: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 44: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 45: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 46: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 47: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 48: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 49: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 50: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 51: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 52: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 53: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 54: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 55: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 56: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 57: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 58: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 59: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 60: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 61: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 62: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 63: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 64: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 65: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 66: ret
    return output;
}

// source.character.static-map-native-1111.v1 / source program 4b3a261b5c8ed1438620dbbcdac20e3e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1111(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[8]=1.f;
    source[9]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 7: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t2.zwxy, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).xy;
    // 9: mul r0.w, r2.y, cb0[3].z
    r0.w = ((r2.yyyy)*(source[3].zzzz)).w;
    // 10: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 11: mad r1.xyz, cb0[3].wwww, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r1.xyz = ((source[3].wwww)*(source[1].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 12: mad r1.xyz, r2.yyyy, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((r2.yyyy)*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 13: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 14: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 15: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 16: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 17: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 19: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 20: dp2 r0.w, r2.yzyy, r2.yzyy
    r0.w = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).w;
    // 21: mul r3.xy, r2.yzyy, cb0[3].xxxx
    r3.xy = ((r2.yzyy)*(source[3].xxxx)).xy;
    // 22: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 25: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 27: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 28: div r2.yzw, r3.xxyz, r0.wwww
    r2.yzw = ((r3.xxyz)/(r0.wwww)).yzw;
    // 29: dp3 r0.w, r2.yzwy, r2.yzwy
    r0.w = (dot((r2.yzwy).xyz,(r2.yzwy).xyz).xxxx).w;
    // 30: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 31: mul r2.yzw, r0.wwww, r2.yyzw
    r2.yzw = ((r0.wwww)*(r2.yyzw)).yzw;
    // 32: dp3 r0.w, r1.xyzx, r2.yzwy
    r0.w = (dot((r1.xyzx).xyz,(r2.yzwy).xyz).xxxx).w;
    // 33: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 34: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 35: mul r3.xyz, cb0[2].xyzx, cb0[4].xxxx
    r3.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // 36: mad r4.xyz, r2.xxxx, r3.xyzx, -r1.yyyy
    r4.xyz = ((r2.xxxx)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 37: mul r5.xyz, r2.xxxx, r3.xyzx
    r5.xyz = ((r2.xxxx)*(r3.xyzx)).xyz;
    // 38: mad r1.yzw, r5.xxyz, r4.xxyz, r1.yyyy
    r1.yzw = ((r5.xxyz)*(r4.xxyz)+(r1.yyyy)).yzw;
    // 39: mul r1.yzw, r1.yyzw, cb0[6].xxyz
    r1.yzw = ((r1.yyzw)*(source[6].xxyz)).yzw;
    // 40: mad r4.xyz, r2.xxxx, r3.xyzx, -r1.xxxx
    r4.xyz = ((r2.xxxx)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 41: mad r4.xyz, r5.xyzx, r4.xyzx, r1.xxxx
    r4.xyz = ((r5.xyzx)*(r4.xyzx)+(r1.xxxx)).xyz;
    // 42: mad r1.xyz, r4.xyzx, cb0[5].xyzx, r1.yzwy
    r1.xyz = ((r4.xyzx)*(source[5].xyzx)+(r1.yzwy)).xyz;
    // 43: mul r1.xyz, r1.xyzx, cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(source[7].wwww)).xyz;
    // 44: mad r3.xyz, -r2.xxxx, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r2.xxxx))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 45: mul r4.xyz, r0.xyzx, r1.xyzx
    r4.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 46: dp2_sat r6.x, r2.zwzz, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r2.zwzz).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 47: dp3_sat r6.y, r2.yzwy, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r2.yzwy).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 48: dp3_sat r6.z, r2.yzwy, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r2.yzwy).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 49: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 50: mad r3.xyz, r6.xyzx, r3.xyzx, r5.xyzx
    r3.xyz = ((r6.xyzx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 51: sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t4.xyzw, s3
    r5.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 52: mul r5.xyz, r5.xyzx, cb0[9].xyzx
    r5.xyz = ((r5.xyzx)*(source[9].xyzx)).xyz;
    // 53: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 54: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t3.xyzw, s3
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 55: mul r3.xyz, r3.xyzx, cb0[8].xyzx
    r3.xyz = ((r3.xyzx)*(source[8].xyzx)).xyz;
    // 56: mul r6.xyz, r0.wwww, r3.xyzx
    r6.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 57: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 58: mul r3.xyz, r3.xyzx, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].xyzx)).xyz;
    // 59: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 60: div r1.xyz, r6.xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)/(r1.xyzx)).xyz;
    // 61: mad r4.xyz, r0.xyzx, r6.xyzx, r4.xyzx
    r4.xyz = ((r0.xyzx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 62: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 63: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 64: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 65: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 66: dp3 r1.w, r2.yzwy, r1.xyzx
    r1.w = (dot((r2.yzwy).xyz,(r1.xyzx).xyz).xxxx).w;
    // 67: mul r6.xyz, r1.wwww, r2.yzwy
    r6.xyz = ((r1.wwww)*(r2.yzwy)).xyz;
    // 68: mad r1.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 69: dp2_sat r6.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 70: dp3_sat r6.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 71: dp3_sat r6.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 72: mul r1.xyz, r6.xyzx, r6.xyzx
    r1.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 73: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 74: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 75: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 76: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 77: mad r1.yzw, r3.xxyz, r1.xxxx, r4.xxyz
    r1.yzw = ((r3.xxyz)*(r1.xxxx)+(r4.xxyz)).yzw;
    // 78: mul r3.xyz, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 79: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 80: add r3.xyz, r1.yzwy, cb0[0].xyzx
    r3.xyz = ((r1.yzwy)+(source[0].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, cb0[7].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)+(r3.xyzx)).xyz;
    // 82: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 83: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 84: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 85: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 86: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 87: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 88: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 89: mul r3.xyz, r1.xxxx, v0.xyzx
    r3.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 90: mul r4.xyz, r0.zxyz, r3.yzxy
    r4.xyz = ((r0.zxyz)*(r3.yzxy)).xyz;
    // 91: mad r4.xyz, r0.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 92: dp3 r0.z, r0.xyzx, r2.yzwy
    r0.z = (dot((r0.xyzx).xyz,(r2.yzwy).xyz).xxxx).z;
    // 93: dp3 r0.x, r3.xyzx, r2.yzwy
    r0.x = (dot((r3.xyzx).xyz,(r2.yzwy).xyz).xxxx).x;
    // 94: mul r3.xyz, r4.xyzx, v1.wwww
    r3.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 95: dp3 r0.y, r3.xyzx, r2.yzwy
    r0.y = (dot((r3.xyzx).xyz,(r2.yzwy).xyz).xxxx).y;
    // 96: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 97: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 98: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 99: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 100: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 101: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 102: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 103: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 104: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 105: movc r0.xy, r1.xxxx, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 106: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 107: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 108: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 109: mul o4.z, r0.w, r1.y
    output.targets[4].z = ((r0.wwww)*(r1.yyyy)).z;
    // 110: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 111: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 112: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 113: ret
    return output;
}

// source.character.static-map-native-1111.v1 / source program e63037d563a708418bc9ee84f27980e5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1111(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1111(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 7: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 8: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 10: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: mul r2.xy, r2.xyxx, cb0[3].xxxx
    r2.xy = ((r2.xyxx)*(source[3].xxxx)).xy;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 23: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 24: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 26: mul r3.xyz, cb0[2].xyzx, cb0[4].xxxx
    r3.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).zw;
    // 28: mad r4.xyz, r1.zzzz, r3.xyzx, -r1.yyyy
    r4.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 29: mul r5.xyz, r3.xyzx, r1.zzzz
    r5.xyz = ((r3.xyzx)*(r1.zzzz)).xyz;
    // 30: mad r3.xyz, r1.zzzz, r3.xyzx, -r1.xxxx
    r3.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 31: mad r3.xyz, r5.xyzx, r3.xyzx, r1.xxxx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r1.xxxx)).xyz;
    // 32: mad r1.xyz, r5.xyzx, r4.xyzx, r1.yyyy
    r1.xyz = ((r5.xyzx)*(r4.xyzx)+(r1.yyyy)).xyz;
    // 33: mul r1.xyz, r1.xyzx, cb0[6].xyzx
    r1.xyz = ((r1.xyzx)*(source[6].xyzx)).xyz;
    // 34: mad r1.xyz, r3.xyzx, cb0[5].xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(source[5].xyzx)+(r1.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(source[7].wwww)).xyz;
    // 36: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 38: mul r0.w, r1.w, cb0[3].z
    r0.w = ((r1.wwww)*(source[3].zzzz)).w;
    // 39: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 40: mad r3.xyz, cb0[3].wwww, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r3.xyz = ((source[3].wwww)*(source[1].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 41: mad r3.xyz, r1.wwww, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 42: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 43: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 44: mad r3.xyz, r1.xyzx, r0.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.xyzx)*(r0.xyzx)+(source[0].xyzx)).xyz;
    // 45: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 46: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 47: mad o0.xyz, r0.xyzx, cb0[7].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)+(r3.xyzx)).xyz;
    // 48: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 49: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 50: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 51: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 52: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 53: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 54: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 55: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 56: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 57: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 58: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 59: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 60: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 61: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 62: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 63: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 64: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 65: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 66: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 67: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 68: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 69: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 70: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 71: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 72: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 74: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 75: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 76: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 77: ret
    return output;
}

// source.character.static-map-native-1112.v1 / source program be3f2d4488ef3e43b9a3f21f0cac9489
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1112(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[6]=1.f;
    source[7]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
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
    // 15: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t2.xyzw, s1
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 17: mul r3.xyz, r3.xyzx, cb0[7].xyzx
    r3.xyz = ((r3.xyzx)*(source[7].xyzx)).xyz;
    // 18: dp3 r0.w, r3.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // 19: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t1.xyzw, s1
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 20: mul r3.xyz, r3.xyzx, cb0[6].xyzx
    r3.xyz = ((r3.xyzx)*(source[6].xyzx)).xyz;
    // 21: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 22: mad r0.xyz, r3.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 23: add r0.xyz, r0.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r0.xyz = ((r0.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 24: div r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)/(r0.xyzx)).xyz;
    // 25: mad r2.xyz, r1.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 26: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 27: mul o4.z, r0.x, r2.x
    output.targets[4].z = ((r0.xxxx)*(r2.xxxx)).z;
    // 28: add r0.xyz, r2.xyzx, cb0[0].xyzx
    r0.xyz = ((r2.xyzx)+(source[0].xyzx)).xyz;
    // 29: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 30: mad o0.xyz, r1.xyzx, cb0[5].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[5].xyzx)+(r0.xyzx)).xyz;
    // 31: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 32: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 33: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 34: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 35: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 36: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 37: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 38: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 39: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 40: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 41: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 42: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 43: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 44: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 45: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 46: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 47: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 48: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 49: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 50: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 51: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 52: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 53: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 54: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 55: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 56: mov o4.xw, l(0,0,0,0)
    output.targets[4].xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 57: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 58: ret
    return output;
}

// source.character.static-map-native-1112.v1 / source program 52d02377bfee2249bb1a7fb96f803686
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1112(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1112(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
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

// source.character.static-map-native-1113.v1 / source program 7ef8b0897718f64daa8ebb92a01e92ce
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1113(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[11].x=(g_SourceCharacterTime.xxxx).x;
    source[12]=g_SourceCharacterBaseConstants[11];
    source[13]=g_SourceCharacterBaseConstants[12];
    source[14]=g_SourceCharacterBaseConstants[13];
    source[15]=g_SourceCharacterBaseConstants[14];
    source[16]=g_SourceCharacterBaseConstants[15];
    source[21]=1.f;
    source[22]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: mul r0.xy, cb0[2].zwzz, cb0[11].xxxx
    r0.xy = ((source[2].zwzz)*(source[11].xxxx)).xy;
    // 2: add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 3: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.010000, 0.010000), r0.xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,0.010000,0.010000))+(r0.xxxy)).zw;
    // 4: mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: dp2 r0.z, r2.xyxx, r2.xyxx
    r0.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 8: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 9: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 10: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 11: add r0.z, r0.z, l(0.000010)
    r0.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 12: mul r3.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000)
    r3.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))).xy;
    // 13: mad r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000), r3.xyxx
    r0.xy = ((r0.xyxx)*(float4(-0.500000,-0.500000,0.000000,0.000000))+(r3.xyxx)).xy;
    // 14: mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // 15: mul r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 17: mad r4.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r0.x, r4.xyxx, r4.xyxx
    r0.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 19: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 21: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 22: add r4.z, r0.z, r0.x
    r4.z = ((r0.zzzz)+(r0.xxxx)).z;
    // 23: mov r2.z, l(0.000010)
    r2.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // 24: add r0.xyz, r2.xyzx, r4.xyzx
    r0.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 25: mul r2.xyzw, r0.xyxy, cb0[11].yyzz
    r2.xyzw = ((r0.xyxy)*(source[11].yyzz)).xyzw;
    // 26: mul r3.zw, cb0[3].zzzw, cb0[11].xxxx
    r3.zw = ((source[3].zzzw)*(source[11].xxxx)).zw;
    // 27: mad r4.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000), r3.zwzz
    r4.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))+(r3.zwzz)).xy;
    // 28: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000), r3.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,-0.500000,-0.500000))+(r3.xxxy)).zw;
    // 29: mul r3.zw, r3.zzzw, cb0[3].xxxy
    r3.zw = ((r3.zzzw)*(source[3].xxxy)).zw;
    // 30: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), r2.zzzw
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(r2.zzzw)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, r3.zwzz, t1.zwxy, s2, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 32: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 33: mad r2.zw, r4.xxxy, cb0[3].xxxy, r2.zzzw
    r2.zw = ((r4.xxxy)*(source[3].xxxy)+(r2.zzzw)).zw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.zwzz, t1.zwxy, s2, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 35: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 36: mad r2.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), r2.zzzw
    r2.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(r2.zzzw)).zw;
    // 37: add r2.zw, r2.zzzw, l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 38: mad r2.xy, cb0[11].wwww, r2.zwzz, r2.xyxx
    r2.xy = ((source[11].wwww)*(r2.zwzz)+(r2.xyxx)).xy;
    // 39: mov r2.z, r0.z
    r2.z = (r0.zzzz).z;
    // 40: mad r0.xy, cb0[14].yyyy, r0.xyxx, v4.xyxx
    r0.xy = ((source[14].yyyy)*(r0.xyxx)+(v4.xyxx)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s7, l(0.000000)
    r0.x = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 42: add r0.yzw, -r2.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r0.yzw = ((-(r2.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // 43: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 44: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 45: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 46: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 48: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 49: mul r1.x, r1.x, cb0[12].x
    r1.x = ((r1.xxxx)*(source[12].xxxx)).x;
    // 50: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 51: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 52: mad r0.yzw, r1.xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 53: mul_sat r1.x, r1.x, cb0[14].x
    r1.x = (saturate((r1.xxxx)*(source[14].xxxx))).x;
    // 54: dp3 r1.y, r0.yzwy, r0.yzwy
    r1.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 55: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 56: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 57: mul r2.xyz, r0.yzwy, r1.yyyy
    r2.xyz = ((r0.yzwy)*(r1.yyyy)).xyz;
    // 58: div r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)/(r1.zzzz)).yzw;
    // 59: mul r1.yz, r0.yyzy, cb0[12].yyyy
    r1.yz = ((r0.yyzy)*(source[12].yyyy)).yz;
    // 60: mad r3.zw, cb0[11].xxxx, cb0[10].zzzw, r3.xxxy
    r3.zw = ((source[11].xxxx)*(source[10].zzzw)+(r3.xxxy)).zw;
    // 61: mad r3.xy, cb0[11].xxxx, cb0[5].zwzz, r3.xyxx
    r3.xy = ((source[11].xxxx)*(source[5].zwzz)+(r3.xyxx)).xy;
    // 62: mad r3.xy, r3.xyxx, cb0[5].xyxx, r1.yzyy
    r3.xy = ((r3.xyxx)*(source[5].xyxx)+(r1.yzyy)).xy;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t7.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 64: mul r4.xyz, r4.xyzx, cb0[4].xyzx
    r4.xyz = ((r4.xyzx)*(source[4].xyzx)).xyz;
    // 65: mul r3.xy, r3.zwzz, cb0[10].xyxx
    r3.xy = ((r3.zwzz)*(source[10].xyxx)).xy;
    // 66: mad r3.zw, r3.zzzw, cb0[10].xxxy, r1.yyyz
    r3.zw = ((r3.zzzw)*(source[10].xxxy)+(r1.yyyz)).zw;
    // 67: mad r1.yz, r3.xxyx, l(0.000000, 2.000000, 2.000000, 0.000000), r1.yyzy
    r1.yz = ((r3.xxyx)*(float4(0.000000,2.000000,2.000000,0.000000))+(r1.yyzy)).yz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t6.wxyz, s6, l(0.000000)
    r1.yzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.zwzz, t6.xyzw, s6, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 70: mad r1.yzw, -r3.xxyz, l(0.000000, 2.000000, 2.000000, 2.000000), r1.yyzw
    r1.yzw = ((-(r3.xxyz))*(float4(0.000000,2.000000,2.000000,2.000000))+(r1.yyzw)).yzw;
    // 71: add r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)+(r3.xyzx)).xyz;
    // 72: mad r1.yzw, r0.xxxx, r1.yyzw, r3.xxyz
    r1.yzw = ((r0.xxxx)*(r1.yyzw)+(r3.xxyz)).yzw;
    // 73: dp3 r2.w, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: add r3.xyz, -r1.yzwy, r2.wwww
    r3.xyz = ((-(r1.yzwy))+(r2.wwww)).xyz;
    // 75: mad r1.yzw, cb0[14].wwww, r3.xxyz, r1.yyzw
    r1.yzw = ((source[14].wwww)*(r3.xxyz)+(r1.yyzw)).yzw;
    // 76: mul r1.yzw, r1.yyzw, cb0[9].xxyz
    r1.yzw = ((r1.yyzw)*(source[9].xxyz)).yzw;
    // 77: mul r1.yzw, r0.xxxx, r1.yyzw
    r1.yzw = ((r0.xxxx)*(r1.yyzw)).yzw;
    // 78: add r3.xyz, r0.yzwy, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r3.xyz = ((r0.yzwy)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // 79: mad r3.xyz, cb0[13].xxxx, r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r3.xyz = ((source[13].xxxx)*(r3.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 80: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 81: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 82: mul r5.xyz, r2.wwww, v6.xyzx
    r5.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 83: dp3 r2.w, r3.xyzx, r5.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 84: mul r3.xy, r3.xyxx, r2.wwww
    r3.xy = ((r3.xyxx)*(r2.wwww)).xy;
    // 85: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r5.xyxx
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r5.xyxx))).xy;
    // 86: mul r3.xy, r3.xyxx, cb0[7].xyxx
    r3.xy = ((r3.xyxx)*(source[7].xyxx)).xy;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 88: mul r3.xyz, r3.xyzx, cb0[6].xyzx
    r3.xyz = ((r3.xyzx)*(source[6].xyzx)).xyz;
    // 89: dp3 r0.y, r0.yzwy, r5.xyzx
    r0.y = (dot((r0.yzwy).xyz,(r5.xyzx).xyz).xxxx).y;
    // 90: add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 91: mad r0.z, -r0.y, l(0.500000), l(1.000000)
    r0.z = ((-(r0.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 92: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 93: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 94: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 95: mul r0.w, r0.w, cb0[13].y
    r0.w = ((r0.wwww)*(source[13].yyyy)).w;
    // 96: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 97: mul r0.w, r0.w, cb0[13].z
    r0.w = ((r0.wwww)*(source[13].zzzz)).w;
    // 98: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 99: dp3 r0.w, r2.xyzx, r5.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 100: mul r6.xyz, r0.wwww, r2.xyzx
    r6.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 101: mad r5.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xyzx
    r5.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xyzx))).xyz;
    // 102: add r6.xy, r5.xyxx, -v4.xyxx
    r6.xy = ((r5.xyxx)+(-(v4.xyxx))).xy;
    // 103: mad r6.xy, r6.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000), v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 104: mul r6.xy, r6.xyxx, cb0[13].wwww
    r6.xy = ((r6.xyxx)*(source[13].wwww)).xy;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 106: mul r6.xyz, r6.xyzx, cb0[8].xyzx
    r6.xyz = ((r6.xyzx)*(source[8].xyzx)).xyz;
    // 107: mul r6.xyz, r1.xxxx, r6.xyzx
    r6.xyz = ((r1.xxxx)*(r6.xyzx)).xyz;
    // 108: mad r3.xyz, r0.zzzz, r3.xyzx, r6.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 109: add r6.xyz, -r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r3.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 110: mad r1.xyz, r1.yzwy, r6.xyzx, r3.xyzx
    r1.xyz = ((r1.yzwy)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 111: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 112: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 113: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 114: dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 115: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 116: mul r3.xyz, r0.zzzz, v7.xyzx
    r3.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // 117: dp3 r0.z, r3.xyzx, r2.xyzx
    r0.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 118: mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 119: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 120: mul r3.xyz, r0.wwww, cb0[19].xyzx
    r3.xyz = ((r0.wwww)*(source[19].xyzx)).xyz;
    // 121: mad r3.xyz, r0.zzzz, cb0[18].xyzx, r3.xyzx
    r3.xyz = ((r0.zzzz)*(source[18].xyzx)+(r3.xyzx)).xyz;
    // 122: mul r3.xyz, r3.xyzx, cb0[20].wwww
    r3.xyz = ((r3.xyzx)*(source[20].wwww)).xyz;
    // 123: mul r6.xyz, r1.xyzx, r3.xyzx
    r6.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 124: dp2_sat r7.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 125: dp3_sat r7.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 126: dp3_sat r7.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 127: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 128: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t9.xyzw, s8
    r8.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 129: mul r8.xyz, r8.xyzx, cb0[22].xyzx
    r8.xyz = ((r8.xyzx)*(source[22].xyzx)).xyz;
    // 130: dp3 r0.z, r8.xyzx, r7.xyzx
    r0.z = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 131: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t8.xyzw, s8
    r7.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 132: mul r7.xyz, r7.xyzx, cb0[21].xyzx
    r7.xyz = ((r7.xyzx)*(source[21].xyzx)).xyz;
    // 133: mul r9.xyz, r0.zzzz, r7.xyzx
    r9.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 134: mad r3.xyz, r7.xyzx, r0.zzzz, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r0.zzzz)+(r3.xyzx)).xyz;
    // 135: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 136: div r3.xyz, r9.xyzx, r3.xyzx
    r3.xyz = ((r9.xyzx)/(r3.xyzx)).xyz;
    // 137: mad r6.xyz, r1.xyzx, r9.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 138: dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 139: dp2_sat r3.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 140: dp3_sat r3.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 141: dp3_sat r3.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 142: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 143: add r0.w, cb0[15].y, l(1.000000)
    r0.w = ((source[15].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mul r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 145: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 146: dp3 r0.w, r8.xyzx, r3.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 147: mad r3.xyz, cb0[15].xxxx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((source[15].xxxx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 148: mul r5.xyz, r7.xyzx, r3.xyzx
    r5.xyz = ((r7.xyzx)*(r3.xyzx)).xyz;
    // 149: mul_sat r3.xyz, r3.xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r3.xyz = (saturate((r3.xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 150: sqrt o5.xyz, r3.xyzx
    output.targets[5].xyz = (sqrt(r3.xyzx)).xyz;
    // 151: mad r3.xyz, r5.xyzx, r0.wwww, r6.xyzx
    r3.xyz = ((r5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 152: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 153: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 154: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 155: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 156: mul r0.w, r0.w, cb0[12].z
    r0.w = ((r0.wwww)*(source[12].zzzz)).w;
    // 157: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 158: mul r0.w, r0.w, cb0[12].w
    r0.w = ((r0.wwww)*(source[12].wwww)).w;
    // 159: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 160: mad r4.xyz, r0.yyyy, r4.xyzx, cb0[1].xyzx
    r4.xyz = ((r0.yyyy)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // 161: add r4.xyz, r3.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 162: mad r4.xyz, r1.xyzx, cb0[20].xyzx, r4.xyzx
    r4.xyz = ((r1.xyzx)*(source[20].xyzx)+(r4.xyzx)).xyz;
    // 163: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 164: mad o0.xyz, r4.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 165: mul r1.xyz, v8.yyyy, cb1[1].xywx
    r1.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 166: mad r1.xyz, cb1[0].xywx, v8.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v8.xxxx)+(r1.xyzx)).xyz;
    // 167: mad r1.xyz, cb1[2].xywx, v8.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v8.zzzz)+(r1.xyzx)).xyz;
    // 168: mad r1.xyz, cb1[3].xywx, v8.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v8.wwww)+(r1.xyzx)).xyz;
    // 169: div r0.yw, r1.xxxy, r1.zzzz
    r0.yw = ((r1.xxxy)/(r1.zzzz)).yw;
    // 170: mad r0.yw, r0.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r0.yw = ((r0.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 171: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t2.yxzw, s0, l(0.000000)
    r0.y = ((float4(0.0,0.0,0.0,0.0)).yxzw).y;
    // 172: min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // 173: mad r0.w, r0.y, cb2[1].z, -cb2[1].w
    r0.w = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).w;
    // 174: mad r0.y, r0.y, cb2[1].x, cb2[1].y
    r0.y = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // 175: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 176: add r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)+(r0.yyyy)).y;
    // 177: add r0.y, -r1.z, r0.y
    r0.y = ((-(r1.zzzz))+(r0.yyyy)).y;
    // 178: add r0.w, -cb0[15].z, l(1.000000)
    r0.w = ((-(source[15].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 180: div_sat r0.y, r0.y, r0.w
    r0.y = (saturate((r0.yyyy)/(r0.wwww))).y;
    // 181: mul r0.y, r0.y, cb0[15].w
    r0.y = ((r0.yyyy)*(source[15].wwww)).y;
    // 182: log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // 183: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 184: mul r0.w, r0.w, cb0[16].x
    r0.w = ((r0.wwww)*(source[16].xxxx)).w;
    // 185: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 186: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 187: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 188: add r0.w, -r0.y, l(1.000000)
    r0.w = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mad r0.x, r0.x, r0.w, r0.y
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.yyyy)).x;
    // 190: mul o0.w, r0.x, cb0[0].w
    output.targets[0].w = ((r0.xxxx)*(source[0].wwww)).w;
    // 191: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 192: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 193: mul r0.xyw, r0.xxxx, v1.xyxz
    r0.xyw = ((r0.xxxx)*(v1.xyxz)).xyw;
    // 194: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 195: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 196: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 197: mul r4.xyz, r0.wxyw, r1.yzxy
    r4.xyz = ((r0.wxyw)*(r1.yzxy)).xyz;
    // 198: mad r4.xyz, r0.ywxy, r1.zxyz, -r4.xyzx
    r4.xyz = ((r0.ywxy)*(r1.zxyz)+(-(r4.xyzx))).xyz;
    // 199: dp3 r5.z, r0.xywx, r2.xyzx
    r5.z = (dot((r0.xywx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 200: dp3 r5.x, r1.xyzx, r2.xyzx
    r5.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 201: mul r0.xyw, r4.xyxz, v1.wwww
    r0.xyw = ((r4.xyxz)*(v1.wwww)).xyw;
    // 202: dp3 r5.y, r0.xywx, r2.xyzx
    r5.y = (dot((r0.xywx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 203: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 204: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 205: mul r0.xyw, r0.xxxx, r5.xyxz
    r0.xyw = ((r0.xxxx)*(r5.xyxz)).xyw;
    // 206: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 207: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xywx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xywx)).xyz).xxxx).w;
    // 208: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 209: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 210: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 211: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 212: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 213: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 214: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 215: mul_sat o3.w, cb0[15].y, l(0.002000)
    output.targets[3].w = (saturate((source[15].yyyy)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // 216: mul o4.z, r0.z, r3.x
    output.targets[4].z = ((r0.zzzz)*(r3.xxxx)).z;
    // 217: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 218: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 219: ftou r0.x, cb0[17].z
    r0.x = (asfloat((uint4)(source[17].zzzz))).x;
    // 220: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 221: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 222: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 223: ret
    return output;
}

// source.character.static-map-native-1113.v1 / source program 97b4376d3820d54fabb6cab16439a3af
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1113(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1113(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[11].x=(g_SourceCharacterTime.xxxx).x;
    source[12]=g_SourceCharacterBaseConstants[11];
    source[13]=g_SourceCharacterBaseConstants[12];
    source[14]=g_SourceCharacterBaseConstants[13];
    source[15]=g_SourceCharacterBaseConstants[14];
    source[16]=g_SourceCharacterBaseConstants[15];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: mul r0.xy, cb0[2].zwzz, cb0[11].xxxx
    r0.xy = ((source[2].zwzz)*(source[11].xxxx)).xy;
    // 2: add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 3: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.010000, 0.010000), r0.xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,0.010000,0.010000))+(r0.xxxy)).zw;
    // 4: mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: dp2 r0.z, r2.xyxx, r2.xyxx
    r0.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 8: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 9: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 10: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 11: add r0.z, r0.z, l(0.000010)
    r0.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 12: mul r3.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000)
    r3.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))).xy;
    // 13: mad r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000), r3.xyxx
    r0.xy = ((r0.xyxx)*(float4(-0.500000,-0.500000,0.000000,0.000000))+(r3.xyxx)).xy;
    // 14: mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // 15: mul r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 17: mad r4.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r0.x, r4.xyxx, r4.xyxx
    r0.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 19: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 21: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 22: add r4.z, r0.z, r0.x
    r4.z = ((r0.zzzz)+(r0.xxxx)).z;
    // 23: mov r2.z, l(0.000010)
    r2.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // 24: add r0.xyz, r2.xyzx, r4.xyzx
    r0.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 25: mul r2.xyzw, r0.xyxy, cb0[11].yyzz
    r2.xyzw = ((r0.xyxy)*(source[11].yyzz)).xyzw;
    // 26: mul r3.zw, cb0[3].zzzw, cb0[11].xxxx
    r3.zw = ((source[3].zzzw)*(source[11].xxxx)).zw;
    // 27: mad r4.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000), r3.zwzz
    r4.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))+(r3.zwzz)).xy;
    // 28: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000), r3.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,-0.500000,-0.500000))+(r3.xxxy)).zw;
    // 29: mul r3.zw, r3.zzzw, cb0[3].xxxy
    r3.zw = ((r3.zzzw)*(source[3].xxxy)).zw;
    // 30: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), r2.zzzw
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(r2.zzzw)).zw;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, r3.zwzz, t1.zwxy, s2, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 32: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 33: mad r2.zw, r4.xxxy, cb0[3].xxxy, r2.zzzw
    r2.zw = ((r4.xxxy)*(source[3].xxxy)+(r2.zzzw)).zw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.zwzz, t1.zwxy, s2, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 35: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 36: mad r2.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), r2.zzzw
    r2.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(r2.zzzw)).zw;
    // 37: add r2.zw, r2.zzzw, l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 38: mad r2.xy, cb0[11].wwww, r2.zwzz, r2.xyxx
    r2.xy = ((source[11].wwww)*(r2.zwzz)+(r2.xyxx)).xy;
    // 39: mov r2.z, r0.z
    r2.z = (r0.zzzz).z;
    // 40: mad r0.xy, cb0[14].yyyy, r0.xyxx, v4.xyxx
    r0.xy = ((source[14].yyyy)*(r0.xyxx)+(v4.xyxx)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s7, l(0.000000)
    r0.x = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 42: add r0.yzw, -r2.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r0.yzw = ((-(r2.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // 43: dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 44: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 45: div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // 46: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 48: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 49: mul r1.x, r1.x, cb0[12].x
    r1.x = ((r1.xxxx)*(source[12].xxxx)).x;
    // 50: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 51: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 52: mad r0.yzw, r1.xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // 53: mul_sat r1.x, r1.x, cb0[14].x
    r1.x = (saturate((r1.xxxx)*(source[14].xxxx))).x;
    // 54: dp3 r1.y, r0.yzwy, r0.yzwy
    r1.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 55: sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // 56: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 57: mul r2.xyz, r0.yzwy, r1.yyyy
    r2.xyz = ((r0.yzwy)*(r1.yyyy)).xyz;
    // 58: div r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)/(r1.zzzz)).yzw;
    // 59: add r1.yzw, r0.yyzw, l(0.000000, -0.000000, -0.000000, -1.000000)
    r1.yzw = ((r0.yyzw)+(float4(0.000000,-0.000000,-0.000000,-1.000000))).yzw;
    // 60: mad r1.yzw, cb0[13].xxxx, r1.yyzw, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.yzw = ((source[13].xxxx)*(r1.yyzw)+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // 61: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 62: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 63: mul r4.xyz, r2.wwww, v6.xyzx
    r4.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 64: dp3 r1.w, r1.yzwy, r4.xyzx
    r1.w = (dot((r1.yzwy).xyz,(r4.xyzx).xyz).xxxx).w;
    // 65: mul r1.yz, r1.yyzy, r1.wwww
    r1.yz = ((r1.yyzy)*(r1.wwww)).yz;
    // 66: mad r1.yz, r1.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), -r4.xxyx
    r1.yz = ((r1.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(-(r4.xxyx))).yz;
    // 67: mul r1.yz, r1.yyzy, cb0[7].xxyx
    r1.yz = ((r1.yyzy)*(source[7].xxyx)).yz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t4.wxyz, s4, l(0.000000)
    r1.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 69: mul r1.yzw, r1.yyzw, cb0[6].xxyz
    r1.yzw = ((r1.yyzw)*(source[6].xxyz)).yzw;
    // 70: dp3 r2.w, r2.xyzx, r4.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 71: mul r3.zw, r2.wwww, r2.xxxy
    r3.zw = ((r2.wwww)*(r2.xxxy)).zw;
    // 72: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), -r4.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(-(r4.xxxy))).zw;
    // 73: dp3 r0.w, r0.yzwy, r4.xyzx
    r0.w = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).w;
    // 74: mul r0.yz, r0.yyzy, cb0[12].yyyy
    r0.yz = ((r0.yyzy)*(source[12].yyyy)).yz;
    // 75: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: add r3.zw, r3.zzzw, -v4.xxxy
    r3.zw = ((r3.zzzw)+(-(v4.xxxy))).zw;
    // 77: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 0.050000, 0.050000), v4.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,0.050000,0.050000))+(v4.xxxy)).zw;
    // 78: mul r3.zw, r3.zzzw, cb0[13].wwww
    r3.zw = ((r3.zzzw)*(source[13].wwww)).zw;
    // 79: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.zwzz, t5.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 80: mul r4.xyz, r4.xyzx, cb0[8].xyzx
    r4.xyz = ((r4.xyzx)*(source[8].xyzx)).xyz;
    // 81: mul r4.xyz, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // 82: mad r1.x, -r0.w, l(0.500000), l(1.000000)
    r1.x = ((-(r0.wwww))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 84: log r2.w, |r1.x|
    r2.w = (log2(abs(r1.xxxx))).w;
    // 85: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 86: mul r2.w, r2.w, cb0[13].y
    r2.w = ((r2.wwww)*(source[13].yyyy)).w;
    // 87: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 88: mul r2.w, r2.w, cb0[13].z
    r2.w = ((r2.wwww)*(source[13].zzzz)).w;
    // 89: movc r1.x, r1.x, l(0), r2.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).x;
    // 90: mad r1.xyz, r1.xxxx, r1.yzwy, r4.xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(r4.xyzx)).xyz;
    // 91: add r4.xyz, -r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mad r3.zw, cb0[11].xxxx, cb0[10].zzzw, r3.xxxy
    r3.zw = ((source[11].xxxx)*(source[10].zzzw)+(r3.xxxy)).zw;
    // 93: mad r3.xy, cb0[11].xxxx, cb0[5].zwzz, r3.xyxx
    r3.xy = ((source[11].xxxx)*(source[5].zwzz)+(r3.xyxx)).xy;
    // 94: mad r3.xy, r3.xyxx, cb0[5].xyxx, r0.yzyy
    r3.xy = ((r3.xyxx)*(source[5].xyxx)+(r0.yzyy)).xy;
    // 95: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r3.xyxx, t7.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 96: mul r5.xyz, r5.xyzx, cb0[4].xyzx
    r5.xyz = ((r5.xyzx)*(source[4].xyzx)).xyz;
    // 97: mul r3.xy, r3.zwzz, cb0[10].xyxx
    r3.xy = ((r3.zwzz)*(source[10].xyxx)).xy;
    // 98: mad r3.zw, r3.zzzw, cb0[10].xxxy, r0.yyyz
    r3.zw = ((r3.zzzw)*(source[10].xxxy)+(r0.yyyz)).zw;
    // 99: mad r0.yz, r3.xxyx, l(0.000000, 2.000000, 2.000000, 0.000000), r0.yyzy
    r0.yz = ((r3.xxyx)*(float4(0.000000,2.000000,2.000000,0.000000))+(r0.yyzy)).yz;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r0.yzyy, t6.xyzw, s6, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 101: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.zwzz, t6.xyzw, s6, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 102: mad r6.xyz, -r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), r6.xyzx
    r6.xyz = ((-(r3.xyzx))*(float4(2.000000,2.000000,2.000000,0.000000))+(r6.xyzx)).xyz;
    // 103: add r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)+(r3.xyzx)).xyz;
    // 104: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 105: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 106: add r6.xyz, -r3.xyzx, r0.yyyy
    r6.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 107: mad r3.xyz, cb0[14].wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((source[14].wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 108: mul r3.xyz, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r3.xyzx)*(source[9].xyzx)).xyz;
    // 109: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 110: mad r1.xyz, r3.xyzx, r4.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 111: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 112: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 113: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 114: log r0.y, |r0.w|
    r0.y = (log2(abs(r0.wwww))).y;
    // 115: lt r0.z, |r0.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 116: mul r0.y, r0.y, cb0[12].z
    r0.y = ((r0.yyyy)*(source[12].zzzz)).y;
    // 117: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 118: mul r0.y, r0.y, cb0[12].w
    r0.y = ((r0.yyyy)*(source[12].wwww)).y;
    // 119: movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 120: mad r0.yzw, r0.yyyy, r5.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(r5.xxyz)+(source[1].xxyz)).yzw;
    // 121: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 122: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 123: mul r3.xyz, r1.wwww, v7.xyzx
    r3.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 124: dp3 r1.w, r3.xyzx, r2.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 125: mad r3.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 126: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 127: mul r3.yzw, r3.yyyy, cb0[19].xxyz
    r3.yzw = ((r3.yyyy)*(source[19].xxyz)).yzw;
    // 128: mad r3.xyz, r3.xxxx, cb0[18].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[18].xyzx)+(r3.yzwy)).xyz;
    // 129: mul r3.xyz, r3.xyzx, cb0[20].wwww
    r3.xyz = ((r3.xyzx)*(source[20].wwww)).xyz;
    // 130: mad r0.yzw, r3.xxyz, r1.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 131: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 132: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 133: mad r0.yzw, r1.xxyz, cb0[20].xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(source[20].xxyz)+(r0.yyzw)).yzw;
    // 134: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 135: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 136: mul r0.yzw, v8.yyyy, cb1[1].xxyw
    r0.yzw = ((v8.yyyy)*(projection[1].xxyw)).yzw;
    // 137: mad r0.yzw, cb1[0].xxyw, v8.xxxx, r0.yyzw
    r0.yzw = ((projection[0].xxyw)*(v8.xxxx)+(r0.yyzw)).yzw;
    // 138: mad r0.yzw, cb1[2].xxyw, v8.zzzz, r0.yyzw
    r0.yzw = ((projection[2].xxyw)*(v8.zzzz)+(r0.yyzw)).yzw;
    // 139: mad r0.yzw, cb1[3].xxyw, v8.wwww, r0.yyzw
    r0.yzw = ((projection[3].xxyw)*(v8.wwww)+(r0.yyzw)).yzw;
    // 140: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 141: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 142: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s0, l(0.000000)
    r0.y = ((float4(0.0,0.0,0.0,0.0)).yxzw).y;
    // 143: min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // 144: mad r0.z, r0.y, cb2[1].z, -cb2[1].w
    r0.z = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).z;
    // 145: mad r0.y, r0.y, cb2[1].x, cb2[1].y
    r0.y = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // 146: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 147: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 148: add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // 149: add r0.z, -cb0[15].z, l(1.000000)
    r0.z = ((-(source[15].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 150: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 151: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 152: mul r0.y, r0.y, cb0[15].w
    r0.y = ((r0.yyyy)*(source[15].wwww)).y;
    // 153: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 154: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 155: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 156: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 157: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 158: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 159: add r0.z, -r0.y, l(1.000000)
    r0.z = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 160: mad r0.x, r0.x, r0.z, r0.y
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).x;
    // 161: mul o0.w, r0.x, cb0[0].w
    output.targets[0].w = ((r0.xxxx)*(source[0].wwww)).w;
    // 162: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 163: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 164: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 165: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 166: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 167: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 168: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 169: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 170: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 171: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 172: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 173: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 174: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 175: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 176: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 177: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 178: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 179: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 180: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 181: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 182: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 183: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 184: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 185: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 186: mul_sat o3.w, cb0[15].y, l(0.002000)
    output.targets[3].w = (saturate((source[15].yyyy)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // 187: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 188: ftou r0.x, cb0[17].z
    r0.x = (asfloat((uint4)(source[17].zzzz))).x;
    // 189: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 190: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 191: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 192: mad r0.xyz, cb0[15].xxxx, cb2[4].wwww, cb2[4].xyzx
    r0.xyz = ((source[15].xxxx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 193: mul_sat r0.xyz, r0.xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((r0.xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 194: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 195: ret
    return output;
}

// source.character.static-map-native-1114.v1 / source program 903b0e5937b8474bb76c3dba0238076e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1114(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[12]=1.f;
    source[13]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xyz, v8.xyzx, cb0[0].xyzx
    r0.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 2: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 3: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 4: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 5: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 6: dp3 r0.w, cb0[5].xyzx, cb0[5].xyzx
    r0.w = (dot((source[5].xyzx).xyz,(source[5].xyzx).xyz).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: div r1.xyz, cb0[5].xyzx, r0.wwww
    r1.xyz = ((source[5].xyzx)/(r0.wwww)).xyz;
    // 9: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 10: mad r0.x, -r0.x, l(0.499000), l(0.500000)
    r0.x = ((-(r0.xxxx))*(float4(0.499000,0.499000,0.499000,0.499000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 11: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 12: mul r0.y, cb0[8].x, l(0.017453)
    r0.y = ((source[8].xxxx)*(float4(0.017453,0.017453,0.017453,0.017453))).y;
    // 13: sincos null, r0.y, r0.y
    r0.y = (cos(r0.yyyy)).y;
    // 14: max r0.y, r0.y, l(-0.999990)
    r0.y = (max(r0.yyyy,float4(-0.999990,-0.999990,-0.999990,-0.999990))).y;
    // 15: min r0.y, r0.y, l(0.999990)
    r0.y = (min(r0.yyyy,float4(0.999990,0.999990,0.999990,0.999990))).y;
    // 16: mad r0.y, -r0.y, l(0.500000), l(0.500000)
    r0.y = ((-(r0.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 17: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 18: mul r0.y, r0.y, l(0.693147)
    r0.y = ((r0.yyyy)*(float4(0.693147,0.693147,0.693147,0.693147))).y;
    // 19: div r0.y, l(-0.301030), r0.y
    r0.y = ((float4(-0.301030,-0.301030,-0.301030,-0.301030))/(r0.yyyy)).y;
    // 20: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 21: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 22: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mul r0.y, r0.x, cb0[8].z
    r0.y = ((r0.xxxx)*(source[8].zzzz)).y;
    // 24: mov r1.x, v4.y
    r1.x = (v4.yyyy).x;
    // 25: mov r1.y, cb0[6].z
    r1.y = (source[6].zzzz).y;
    // 26: add r0.zw, -r1.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r1.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 27: ge r1.x, v4.y, cb0[6].z
    r1.x = (asfloat((uint4)((v4.yyyy)>=(source[6].zzzz)) * 0xffffffffu)).x;
    // 28: movc r0.z, r1.x, r0.z, v4.y
    r0.z = ((asuint(r1.xxxx) != 0u) ? (r0.zzzz) : (v4.yyyy)).z;
    // 29: movc r0.w, r1.x, r0.w, cb0[6].z
    r0.w = ((asuint(r1.xxxx) != 0u) ? (r0.wwww) : (source[6].zzzz)).w;
    // 30: movc_sat r1.x, r1.x, cb0[6].w, cb0[7].x
    r1.x = (saturate((asuint(r1.xxxx) != 0u) ? (source[6].wwww) : (source[7].xxxx))).x;
    // 31: div r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)/(r0.wwww)).z;
    // 32: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 34: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 35: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 36: mul_sat r0.z, r0.z, cb0[7].y
    r0.z = (saturate((r0.zzzz)*(source[7].yyyy))).z;
    // 37: mad r0.w, -cb0[8].z, r0.x, r0.z
    r0.w = ((-(source[8].zzzz))*(r0.xxxx)+(r0.zzzz)).w;
    // 38: mad r0.y, cb0[8].z, r0.w, r0.y
    r0.y = ((source[8].zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 39: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 40: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 41: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 42: mad r2.xyz, r2.xyzx, r0.yyyy, -r1.xyzx
    r2.xyz = ((r2.xyzx)*(r0.yyyy)+(-(r1.xyzx))).xyz;
    // 43: mad r0.xyw, r0.xxxx, r2.xyxz, r1.xyxz
    r0.xyw = ((r0.xxxx)*(r2.xyxz)+(r1.xyxz)).xyw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 45: mul r2.xyz, r1.xyzx, cb0[2].xyzx
    r2.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 46: mad r1.xyz, -r1.xyzx, cb0[2].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[2].xyzx)+(r1.xyzx)).xyz;
    // 47: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 48: add r0.xyw, r0.xyxw, -r1.xyxz
    r0.xyw = ((r0.xyxw)+(-(r1.xyxz))).xyw;
    // 49: add r2.x, -r0.z, l(1.000000)
    r2.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: mad_sat r0.z, -r2.x, r1.w, r0.z
    r0.z = (saturate((-(r2.xxxx))*(r1.wwww)+(r0.zzzz))).z;
    // 51: mad r0.xyz, r0.zzzz, r0.xywx, r1.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r1.xyzx)).xyz;
    // 52: mul r0.xyz, r0.xyzx, cb0[8].wwww
    r0.xyz = ((r0.xyzx)*(source[8].wwww)).xyz;
    // 53: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 54: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 55: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 56: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 57: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 59: mul r1.yzw, r1.yyyy, cb0[10].xxyz
    r1.yzw = ((r1.yyyy)*(source[10].xxyz)).yzw;
    // 60: mad r1.xyz, r1.xxxx, cb0[9].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[9].xyzx)+(r1.yzwy)).xyz;
    // 61: mul r1.xyz, r1.xyzx, cb0[11].wwww
    r1.xyz = ((r1.xyzx)*(source[11].wwww)).xyz;
    // 62: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 63: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t2.xyzw, s1
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 64: mul r3.xyz, r3.xyzx, cb0[13].xyzx
    r3.xyz = ((r3.xyzx)*(source[13].xyzx)).xyz;
    // 65: dp3 r0.w, r3.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // 66: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t1.xyzw, s1
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 67: mul r3.xyz, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((r3.xyzx)*(source[12].xyzx)).xyz;
    // 68: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 69: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 70: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 71: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 72: mad r2.xyz, r0.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 73: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 75: add r1.xyz, r2.xyzx, cb0[1].xyzx
    r1.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 76: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 77: mad r1.xyz, r0.xyzx, cb0[11].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[11].xyzx)+(r1.xyzx)).xyz;
    // 78: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 79: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 80: mov o0.w, cb0[0].w
    output.targets[0].w = (source[0].wwww).w;
    // 81: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 82: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 83: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 84: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 85: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 86: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 87: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 88: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 89: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 90: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 91: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 92: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 93: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 94: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 95: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 96: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 97: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 98: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 99: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 100: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 101: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 102: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 103: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 104: mov o4.xw, l(0,0,0,0)
    output.targets[4].xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 105: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 106: ret
    return output;
}

// source.character.static-map-native-1114.v1 / source program b2a6227f97c08940ba56bb78a8c1b3bb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1114(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1114(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xyz, v8.xyzx, cb0[0].xyzx
    r0.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 2: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 3: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 4: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 5: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 6: dp3 r0.w, cb0[5].xyzx, cb0[5].xyzx
    r0.w = (dot((source[5].xyzx).xyz,(source[5].xyzx).xyz).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: div r1.xyz, cb0[5].xyzx, r0.wwww
    r1.xyz = ((source[5].xyzx)/(r0.wwww)).xyz;
    // 9: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 10: mad r0.x, -r0.x, l(0.499000), l(0.500000)
    r0.x = ((-(r0.xxxx))*(float4(0.499000,0.499000,0.499000,0.499000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 11: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 12: mul r0.y, cb0[8].x, l(0.017453)
    r0.y = ((source[8].xxxx)*(float4(0.017453,0.017453,0.017453,0.017453))).y;
    // 13: sincos null, r0.y, r0.y
    r0.y = (cos(r0.yyyy)).y;
    // 14: max r0.y, r0.y, l(-0.999990)
    r0.y = (max(r0.yyyy,float4(-0.999990,-0.999990,-0.999990,-0.999990))).y;
    // 15: min r0.y, r0.y, l(0.999990)
    r0.y = (min(r0.yyyy,float4(0.999990,0.999990,0.999990,0.999990))).y;
    // 16: mad r0.y, -r0.y, l(0.500000), l(0.500000)
    r0.y = ((-(r0.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 17: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 18: mul r0.y, r0.y, l(0.693147)
    r0.y = ((r0.yyyy)*(float4(0.693147,0.693147,0.693147,0.693147))).y;
    // 19: div r0.y, l(-0.301030), r0.y
    r0.y = ((float4(-0.301030,-0.301030,-0.301030,-0.301030))/(r0.yyyy)).y;
    // 20: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 21: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 22: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mul r0.y, r0.x, cb0[8].z
    r0.y = ((r0.xxxx)*(source[8].zzzz)).y;
    // 24: mov r1.x, v4.y
    r1.x = (v4.yyyy).x;
    // 25: mov r1.y, cb0[6].z
    r1.y = (source[6].zzzz).y;
    // 26: add r0.zw, -r1.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r1.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 27: ge r1.x, v4.y, cb0[6].z
    r1.x = (asfloat((uint4)((v4.yyyy)>=(source[6].zzzz)) * 0xffffffffu)).x;
    // 28: movc r0.z, r1.x, r0.z, v4.y
    r0.z = ((asuint(r1.xxxx) != 0u) ? (r0.zzzz) : (v4.yyyy)).z;
    // 29: movc r0.w, r1.x, r0.w, cb0[6].z
    r0.w = ((asuint(r1.xxxx) != 0u) ? (r0.wwww) : (source[6].zzzz)).w;
    // 30: movc_sat r1.x, r1.x, cb0[6].w, cb0[7].x
    r1.x = (saturate((asuint(r1.xxxx) != 0u) ? (source[6].wwww) : (source[7].xxxx))).x;
    // 31: div r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)/(r0.wwww)).z;
    // 32: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 34: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 35: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 36: mul_sat r0.z, r0.z, cb0[7].y
    r0.z = (saturate((r0.zzzz)*(source[7].yyyy))).z;
    // 37: mad r0.w, -cb0[8].z, r0.x, r0.z
    r0.w = ((-(source[8].zzzz))*(r0.xxxx)+(r0.zzzz)).w;
    // 38: mad r0.y, cb0[8].z, r0.w, r0.y
    r0.y = ((source[8].zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 39: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 40: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 41: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 42: mad r2.xyz, r2.xyzx, r0.yyyy, -r1.xyzx
    r2.xyz = ((r2.xyzx)*(r0.yyyy)+(-(r1.xyzx))).xyz;
    // 43: mad r0.xyw, r0.xxxx, r2.xyxz, r1.xyxz
    r0.xyw = ((r0.xxxx)*(r2.xyxz)+(r1.xyxz)).xyw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 45: mul r2.xyz, r1.xyzx, cb0[2].xyzx
    r2.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 46: mad r1.xyz, -r1.xyzx, cb0[2].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[2].xyzx)+(r1.xyzx)).xyz;
    // 47: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 48: add r0.xyw, r0.xyxw, -r1.xyxz
    r0.xyw = ((r0.xyxw)+(-(r1.xyxz))).xyw;
    // 49: add r2.x, -r0.z, l(1.000000)
    r2.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: mad_sat r0.z, -r2.x, r1.w, r0.z
    r0.z = (saturate((-(r2.xxxx))*(r1.wwww)+(r0.zzzz))).z;
    // 51: mad r0.xyz, r0.zzzz, r0.xywx, r1.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r1.xyzx)).xyz;
    // 52: mul r0.xyz, r0.xyzx, cb0[8].wwww
    r0.xyz = ((r0.xyzx)*(source[8].wwww)).xyz;
    // 53: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 54: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 55: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 56: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 57: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 59: mul r1.yzw, r1.yyyy, cb0[10].xxyz
    r1.yzw = ((r1.yyyy)*(source[10].xxyz)).yzw;
    // 60: mad r1.xyz, r1.xxxx, cb0[9].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[9].xyzx)+(r1.yzwy)).xyz;
    // 61: mul r1.xyz, r1.xyzx, cb0[11].wwww
    r1.xyz = ((r1.xyzx)*(source[11].wwww)).xyz;
    // 62: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 63: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 64: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 65: mad r1.xyz, r0.xyzx, cb0[11].xyzx, r2.xyzx
    r1.xyz = ((r0.xyzx)*(source[11].xyzx)+(r2.xyzx)).xyz;
    // 66: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 67: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 68: mov o0.w, cb0[0].w
    output.targets[0].w = (source[0].wwww).w;
    // 69: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 70: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 71: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 72: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 73: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 74: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 75: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 76: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 77: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 78: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 79: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 80: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 81: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 82: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 83: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 84: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 85: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 86: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 87: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 88: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 89: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 90: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 91: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 92: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 93: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 94: ret
    return output;
}

// source.character.static-map-native-1115.v1 / source program ff89c92438a5ad4bb105a31872bd76e8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1115(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[9]=1.f;
    source[10]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xyz, v7.xyzx, cb0[0].xyzx
    r0.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 2: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 3: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 4: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 5: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 6: dp3 r0.w, cb0[4].xyzx, cb0[4].xyzx
    r0.w = (dot((source[4].xyzx).xyz,(source[4].xyzx).xyz).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: div r1.xyz, cb0[4].xyzx, r0.wwww
    r1.xyz = ((source[4].xyzx)/(r0.wwww)).xyz;
    // 9: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 10: add r0.x, -r0.x, l(-0.999500)
    r0.x = ((-(r0.xxxx))+(float4(-0.999500,-0.999500,-0.999500,-0.999500))).x;
    // 11: mul_sat r0.x, r0.x, l(2000.000000)
    r0.x = (saturate((r0.xxxx)*(float4(2000.000000,2000.000000,2000.000000,2000.000000)))).x;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 13: mul r0.yzw, r1.xxyz, cb0[2].xxyz
    r0.yzw = ((r1.xxyz)*(source[2].xxyz)).yzw;
    // 14: mad r1.xyz, -r1.xyzx, cb0[2].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[2].xyzx)+(r1.xyzx)).xyz;
    // 15: mad r0.yzw, r1.wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 16: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 17: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 18: mul r0.xyz, r0.xyzx, cb0[5].xxxx
    r0.xyz = ((r0.xyzx)*(source[5].xxxx)).xyz;
    // 19: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 20: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 23: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 24: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 25: mul r1.yzw, r1.yyyy, cb0[7].xxyz
    r1.yzw = ((r1.yyyy)*(source[7].xxyz)).yzw;
    // 26: mad r1.xyz, r1.xxxx, cb0[6].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[6].xyzx)+(r1.yzwy)).xyz;
    // 27: mul r1.xyz, r1.xyzx, cb0[8].wwww
    r1.xyz = ((r1.xyzx)*(source[8].wwww)).xyz;
    // 28: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 29: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t2.xyzw, s1
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 30: mul r3.xyz, r3.xyzx, cb0[10].xyzx
    r3.xyz = ((r3.xyzx)*(source[10].xyzx)).xyz;
    // 31: dp3 r0.w, r3.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // 32: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t1.xyzw, s1
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 33: mul r3.xyz, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r3.xyzx)*(source[9].xyzx)).xyz;
    // 34: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 35: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 36: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 37: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 38: mad r2.xyz, r0.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 39: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 41: add r1.xyz, r2.xyzx, cb0[1].xyzx
    r1.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 42: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 43: mad o0.xyz, r0.xyzx, cb0[8].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)+(r1.xyzx)).xyz;
    // 44: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 45: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 46: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 47: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 48: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 49: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 50: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 51: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 52: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 53: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 54: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 55: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 56: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 57: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 58: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 59: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 60: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 61: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 62: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 63: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 64: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 65: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 66: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 68: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 69: mov o4.xw, l(0,0,0,0)
    output.targets[4].xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 70: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 71: ret
    return output;
}

// source.character.static-map-native-1115.v1 / source program fc9b88dd02288f42812293bc75805170
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1115(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1115(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xyz, v7.xyzx, cb0[0].xyzx
    r0.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 2: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 3: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 4: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 5: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 6: dp3 r0.w, cb0[4].xyzx, cb0[4].xyzx
    r0.w = (dot((source[4].xyzx).xyz,(source[4].xyzx).xyz).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: div r1.xyz, cb0[4].xyzx, r0.wwww
    r1.xyz = ((source[4].xyzx)/(r0.wwww)).xyz;
    // 9: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 10: add r0.x, -r0.x, l(-0.999500)
    r0.x = ((-(r0.xxxx))+(float4(-0.999500,-0.999500,-0.999500,-0.999500))).x;
    // 11: mul_sat r0.x, r0.x, l(2000.000000)
    r0.x = (saturate((r0.xxxx)*(float4(2000.000000,2000.000000,2000.000000,2000.000000)))).x;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 13: mul r0.yzw, r1.xxyz, cb0[2].xxyz
    r0.yzw = ((r1.xxyz)*(source[2].xxyz)).yzw;
    // 14: mad r1.xyz, -r1.xyzx, cb0[2].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[2].xyzx)+(r1.xyzx)).xyz;
    // 15: mad r0.yzw, r1.wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 16: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 17: mad r0.xyz, r0.xxxx, r1.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r1.xyzx)+(r0.yzwy)).xyz;
    // 18: mul r0.xyz, r0.xyzx, cb0[5].xxxx
    r0.xyz = ((r0.xyzx)*(source[5].xxxx)).xyz;
    // 19: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 20: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 23: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 24: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 25: mul r1.yzw, r1.yyyy, cb0[7].xxyz
    r1.yzw = ((r1.yyyy)*(source[7].xxyz)).yzw;
    // 26: mad r1.xyz, r1.xxxx, cb0[6].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[6].xyzx)+(r1.yzwy)).xyz;
    // 27: mul r1.xyz, r1.xyzx, cb0[8].wwww
    r1.xyz = ((r1.xyzx)*(source[8].wwww)).xyz;
    // 28: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 29: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 30: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 31: mad o0.xyz, r0.xyzx, cb0[8].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)+(r2.xyzx)).xyz;
    // 32: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 33: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 34: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 35: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 36: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 37: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 38: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 39: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 40: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 41: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 42: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 43: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 44: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 45: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 46: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 47: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 48: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 49: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 50: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 51: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 52: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 53: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 54: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 55: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 56: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 57: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 58: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 59: ret
    return output;
}

// source.character.static-map-native-1116.v1 / source program 5cbaa8f6c8e79c4eba0ff605e11af475
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1116(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[6]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[11].y=(g_SourceCharacterTime.xxxx).x;
    source[11].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[12]=g_SourceCharacterBaseConstants[13];
    source[12].x=((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))))).x;
    source[16]=1.f;
    source[17]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xyz, v7.xyzx, cb0[0].xyzx
    r0.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 2: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 3: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 4: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 5: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 6: dp3 r0.w, cb0[3].xyzx, cb0[3].xyzx
    r0.w = (dot((source[3].xyzx).xyz,(source[3].xyzx).xyz).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: div r1.xyz, cb0[3].xyzx, r0.wwww
    r1.xyz = ((source[3].xyzx)/(r0.wwww)).xyz;
    // 9: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 10: mov_sat r0.y, -r0.x
    r0.y = (saturate(-(r0.xxxx))).y;
    // 11: add r0.x, -r0.x, l(-0.999500)
    r0.x = ((-(r0.xxxx))+(float4(-0.999500,-0.999500,-0.999500,-0.999500))).x;
    // 12: mul_sat r0.x, r0.x, l(2000.000000)
    r0.x = (saturate((r0.xxxx)*(float4(2000.000000,2000.000000,2000.000000,2000.000000)))).x;
    // 13: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 14: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 15: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 16: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 17: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 18: mul r0.zw, v4.xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[4].xxxy)).zw;
    // 19: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, -1.000000, 2.000000), cb0[6].xxxy
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,-1.000000,2.000000))+(source[6].xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r0.zwzz, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 22: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 23: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 24: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 26: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 27: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 28: mov r3.xw, l(1.000000,0,0,2.000000)
    r3.xw = (float4(1.000000,asfloat(0u),asfloat(0u),2.000000)).xw;
    // 29: mov r3.yz, cb0[4].yyxy
    r3.yz = (source[4].yyxy).yz;
    // 30: mul r0.zw, r3.xxxy, v4.xxxy
    r0.zw = ((r3.xxxy)*(v4.xxxy)).zw;
    // 31: mad r3.xy, r3.zwzz, r0.zwzz, cb0[5].xyxx
    r3.xy = ((r3.zwzz)*(r0.zwzz)+(source[5].xyxx)).xy;
    // 32: mad r0.zw, r3.zzzw, r0.zzzw, cb0[9].xxxy
    r0.zw = ((r3.zzzw)*(r0.zzzw)+(source[9].xxxy)).zw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r3.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 36: mad r2.yzw, r3.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r2.xxyz
    r2.yzw = ((r3.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r2.xxyz)).yzw;
    // 37: mul r2.yzw, r2.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000)
    r2.yzw = ((r2.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // 38: mad_sat r2.yzw, r4.xxyz, r2.yyzw, r2.yyzw
    r2.yzw = (saturate((r4.xxyz)*(r2.yyzw)+(r2.yyzw))).yzw;
    // 39: mad r3.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: dp2 r0.z, r3.xyxx, r3.xyxx
    r0.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 41: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 43: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 44: add r3.z, r0.z, l(0.000010)
    r3.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 45: add r0.z, r1.z, -r3.z
    r0.z = ((r1.zzzz)+(-(r3.zzzz))).z;
    // 46: add r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)+(r3.xyzx)).xyz;
    // 47: mad r0.z, r2.x, r0.z, r3.z
    r0.z = ((r2.xxxx)*(r0.zzzz)+(r3.zzzz)).z;
    // 48: add r0.w, -r2.x, l(1.000000)
    r0.w = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 51: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 52: mul r2.xzw, r2.yyzw, r0.zzzz
    r2.xzw = ((r2.yyzw)*(r0.zzzz)).xzw;
    // 53: mul r0.z, r2.y, cb0[12].y
    r0.z = ((r2.yyyy)*(source[12].yyyy)).z;
    // 54: add r1.w, cb0[8].w, l(-0.030000)
    r1.w = ((source[8].wwww)+(float4(-0.030000,-0.030000,-0.030000,-0.030000))).w;
    // 55: mad r2.xyz, r2.xzwx, r1.wwww, l(0.030000, 0.030000, 0.030000, 0.000000)
    r2.xyz = ((r2.xzwx)*(r1.wwww)+(float4(0.030000,0.030000,0.030000,0.000000))).xyz;
    // 56: mul r2.xyz, r2.xyzx, cb0[8].xyzx
    r2.xyz = ((r2.xyzx)*(source[8].xyzx)).xyz;
    // 57: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 58: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 59: div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // 60: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 61: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 62: mul r3.xyz, r1.wwww, v5.xyzx
    r3.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 63: dp3 r1.x, r3.xyzx, r1.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 64: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: mad r1.x, -r1.x, l(0.500000), l(1.000000)
    r1.x = ((-(r1.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: mul r1.xyz, r1.xxxx, cb0[7].xyzx
    r1.xyz = ((r1.xxxx)*(source[7].xyzx)).xyz;
    // 67: mul r1.xyz, r1.xyzx, cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(source[7].wwww)).xyz;
    // 68: mad r1.xyz, r0.yyyy, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 70: mul r3.xyz, r2.xyzx, cb0[2].xyzx
    r3.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 71: mad r2.xyz, -r2.xyzx, cb0[2].xyzx, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(source[2].xyzx)+(r2.xyzx)).xyz;
    // 72: mad r2.xyz, r2.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 73: add r0.y, -r2.w, l(0.200000)
    r0.y = ((-(r2.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 74: mul_sat r0.y, r0.y, l(5.000000)
    r0.y = (saturate((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000)))).y;
    // 75: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 76: add r1.xyz, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)+(-(r2.xyzx))).xyz;
    // 77: mad r1.xyz, r0.yyyy, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 78: mul r2.xyz, cb0[10].xyzx, cb0[10].wwww
    r2.xyz = ((source[10].xyzx)*(source[10].wwww)).xyz;
    // 79: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 80: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 81: mul r0.xyz, r0.xyzx, cb0[12].zzzz
    r0.xyz = ((r0.xyzx)*(source[12].zzzz)).xyz;
    // 82: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 83: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 84: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 85: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 86: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 87: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 88: mul r1.yzw, r1.yyyy, cb0[14].xxyz
    r1.yzw = ((r1.yyyy)*(source[14].xxyz)).yzw;
    // 89: mad r1.xyz, r1.xxxx, cb0[13].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[13].xyzx)+(r1.yzwy)).xyz;
    // 90: mul r1.xyz, r1.xyzx, cb0[15].wwww
    r1.xyz = ((r1.xyzx)*(source[15].wwww)).xyz;
    // 91: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 92: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t4.xyzw, s3
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 93: mul r3.xyz, r3.xyzx, cb0[17].xyzx
    r3.xyz = ((r3.xyzx)*(source[17].xyzx)).xyz;
    // 94: dp3 r0.w, r3.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // 95: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t3.xyzw, s3
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 96: mul r3.xyz, r3.xyzx, cb0[16].xyzx
    r3.xyz = ((r3.xyzx)*(source[16].xyzx)).xyz;
    // 97: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 98: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 99: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 100: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 101: mad r2.xyz, r0.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 102: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 104: add r1.xyz, r2.xyzx, cb0[1].xyzx
    r1.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 105: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 106: mad o0.xyz, r0.xyzx, cb0[15].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[15].xyzx)+(r1.xyzx)).xyz;
    // 107: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 108: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 109: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 110: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 111: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 112: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 113: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 114: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 115: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 116: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 117: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 118: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 119: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 120: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 121: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 122: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 123: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 124: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 125: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 126: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 127: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 128: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 129: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 130: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 131: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 132: mov o4.xw, l(0,0,0,0)
    output.targets[4].xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 133: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 134: ret
    return output;
}

// source.character.static-map-native-1116.v1 / source program c442dd42a645c040afccda295a6a8135
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1116(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1116(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[6]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[11].y=(g_SourceCharacterTime.xxxx).x;
    source[11].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[12]=g_SourceCharacterBaseConstants[13];
    source[12].x=((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))))).x;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xyz, v7.xyzx, cb0[0].xyzx
    r0.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 2: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 3: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 4: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 5: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 6: dp3 r0.w, cb0[3].xyzx, cb0[3].xyzx
    r0.w = (dot((source[3].xyzx).xyz,(source[3].xyzx).xyz).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: div r1.xyz, cb0[3].xyzx, r0.wwww
    r1.xyz = ((source[3].xyzx)/(r0.wwww)).xyz;
    // 9: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 10: mov_sat r0.y, -r0.x
    r0.y = (saturate(-(r0.xxxx))).y;
    // 11: add r0.x, -r0.x, l(-0.999500)
    r0.x = ((-(r0.xxxx))+(float4(-0.999500,-0.999500,-0.999500,-0.999500))).x;
    // 12: mul_sat r0.x, r0.x, l(2000.000000)
    r0.x = (saturate((r0.xxxx)*(float4(2000.000000,2000.000000,2000.000000,2000.000000)))).x;
    // 13: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 14: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 15: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 16: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 17: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 18: mul r0.zw, v4.xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)*(source[4].xxxy)).zw;
    // 19: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, -1.000000, 2.000000), cb0[6].xxxy
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,-1.000000,2.000000))+(source[6].xxxy)).zw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r0.zwzz, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 22: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 23: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 24: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 26: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 27: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 28: mov r3.xw, l(1.000000,0,0,2.000000)
    r3.xw = (float4(1.000000,asfloat(0u),asfloat(0u),2.000000)).xw;
    // 29: mov r3.yz, cb0[4].yyxy
    r3.yz = (source[4].yyxy).yz;
    // 30: mul r0.zw, r3.xxxy, v4.xxxy
    r0.zw = ((r3.xxxy)*(v4.xxxy)).zw;
    // 31: mad r3.xy, r3.zwzz, r0.zwzz, cb0[5].xyxx
    r3.xy = ((r3.zwzz)*(r0.zwzz)+(source[5].xyxx)).xy;
    // 32: mad r0.zw, r3.zzzw, r0.zzzw, cb0[9].xxxy
    r0.zw = ((r3.zzzw)*(r0.zzzw)+(source[9].xxxy)).zw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r3.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 36: mad r2.yzw, r3.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r2.xxyz
    r2.yzw = ((r3.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r2.xxyz)).yzw;
    // 37: mul r2.yzw, r2.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000)
    r2.yzw = ((r2.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // 38: mad_sat r2.yzw, r4.xxyz, r2.yyzw, r2.yyzw
    r2.yzw = (saturate((r4.xxyz)*(r2.yyzw)+(r2.yyzw))).yzw;
    // 39: mad r3.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: dp2 r0.z, r3.xyxx, r3.xyxx
    r0.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 41: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 43: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 44: add r3.z, r0.z, l(0.000010)
    r3.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 45: add r0.z, r1.z, -r3.z
    r0.z = ((r1.zzzz)+(-(r3.zzzz))).z;
    // 46: add r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)+(r3.xyzx)).xyz;
    // 47: mad r0.z, r2.x, r0.z, r3.z
    r0.z = ((r2.xxxx)*(r0.zzzz)+(r3.zzzz)).z;
    // 48: add r0.w, -r2.x, l(1.000000)
    r0.w = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 51: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 52: mul r2.xzw, r2.yyzw, r0.zzzz
    r2.xzw = ((r2.yyzw)*(r0.zzzz)).xzw;
    // 53: mul r0.z, r2.y, cb0[12].y
    r0.z = ((r2.yyyy)*(source[12].yyyy)).z;
    // 54: add r1.w, cb0[8].w, l(-0.030000)
    r1.w = ((source[8].wwww)+(float4(-0.030000,-0.030000,-0.030000,-0.030000))).w;
    // 55: mad r2.xyz, r2.xzwx, r1.wwww, l(0.030000, 0.030000, 0.030000, 0.000000)
    r2.xyz = ((r2.xzwx)*(r1.wwww)+(float4(0.030000,0.030000,0.030000,0.000000))).xyz;
    // 56: mul r2.xyz, r2.xyzx, cb0[8].xyzx
    r2.xyz = ((r2.xyzx)*(source[8].xyzx)).xyz;
    // 57: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 58: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 59: div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // 60: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 61: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 62: mul r3.xyz, r1.wwww, v5.xyzx
    r3.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 63: dp3 r1.x, r3.xyzx, r1.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 64: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: mad r1.x, -r1.x, l(0.500000), l(1.000000)
    r1.x = ((-(r1.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: mul r1.xyz, r1.xxxx, cb0[7].xyzx
    r1.xyz = ((r1.xxxx)*(source[7].xyzx)).xyz;
    // 67: mul r1.xyz, r1.xyzx, cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(source[7].wwww)).xyz;
    // 68: mad r1.xyz, r0.yyyy, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 70: mul r3.xyz, r2.xyzx, cb0[2].xyzx
    r3.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 71: mad r2.xyz, -r2.xyzx, cb0[2].xyzx, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(source[2].xyzx)+(r2.xyzx)).xyz;
    // 72: mad r2.xyz, r2.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 73: add r0.y, -r2.w, l(0.200000)
    r0.y = ((-(r2.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 74: mul_sat r0.y, r0.y, l(5.000000)
    r0.y = (saturate((r0.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000)))).y;
    // 75: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 76: add r1.xyz, r1.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)+(-(r2.xyzx))).xyz;
    // 77: mad r1.xyz, r0.yyyy, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 78: mul r2.xyz, cb0[10].xyzx, cb0[10].wwww
    r2.xyz = ((source[10].xyzx)*(source[10].wwww)).xyz;
    // 79: mul r0.xyz, r0.xxxx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 80: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 81: mul r0.xyz, r0.xyzx, cb0[12].zzzz
    r0.xyz = ((r0.xyzx)*(source[12].zzzz)).xyz;
    // 82: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 83: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 84: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 85: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 86: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 87: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 88: mul r1.yzw, r1.yyyy, cb0[14].xxyz
    r1.yzw = ((r1.yyyy)*(source[14].xxyz)).yzw;
    // 89: mad r1.xyz, r1.xxxx, cb0[13].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[13].xyzx)+(r1.yzwy)).xyz;
    // 90: mul r1.xyz, r1.xyzx, cb0[15].wwww
    r1.xyz = ((r1.xyzx)*(source[15].wwww)).xyz;
    // 91: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 92: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 93: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 94: mad o0.xyz, r0.xyzx, cb0[15].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[15].xyzx)+(r2.xyzx)).xyz;
    // 95: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 96: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 97: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 98: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 99: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 100: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 101: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 102: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 103: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 104: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 105: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 106: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 107: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 108: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 109: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 110: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 111: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 112: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 113: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 114: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 115: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 116: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 117: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 118: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 119: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 120: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 121: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 122: ret
    return output;
}

// source.character.static-map-native-1117.v1 / source program 52512f4923559e46821fff16e12318c8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1117(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[9]=1.f;
    source[10]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 7: mul r1.xy, r1.xyxx, cb0[4].xxxx
    r1.xy = ((r1.xyxx)*(source[4].xxxx)).xy;
    // 8: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 9: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 11: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 12: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 16: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 19: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 20: mul r2.xyz, r0.wwww, r1.xyzx
    r2.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 21: mad r0.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 22: dp2_sat r2.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 23: dp3_sat r2.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 24: dp3_sat r2.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 25: log r0.xyz, r2.xyzx
    r0.xyz = (log2(r2.xyzx)).xyz;
    // 26: add r0.w, cb0[4].w, l(1.000000)
    r0.w = ((source[4].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 28: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 29: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t3.xyzw, s2
    r2.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 30: mul r2.xyz, r2.xyzx, cb0[10].xyzx
    r2.xyz = ((r2.xyzx)*(source[10].xyzx)).xyz;
    // 31: dp3 r0.x, r2.xyzx, r0.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 32: dp2_sat r3.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 33: dp3_sat r3.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 34: dp3_sat r3.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 35: mul r0.yzw, r3.xxyz, r3.xxyz
    r0.yzw = ((r3.xxyz)*(r3.xxyz)).yzw;
    // 36: dp3 r0.y, r2.xyzx, r0.yzwy
    r0.y = (dot((r2.xyzx).xyz,(r0.yzwy).xyz).xxxx).y;
    // 37: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t2.xyzw, s2
    r2.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 38: mul r2.xyz, r2.xyzx, cb0[9].xyzx
    r2.xyz = ((r2.xyzx)*(source[9].xyzx)).xyz;
    // 39: mul r3.xyz, r0.yyyy, r2.xyzx
    r3.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 40: dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 41: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 42: mul r4.xyz, r0.zzzz, v7.xyzx
    r4.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // 43: dp3 r0.z, r4.xyzx, r1.xyzx
    r0.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 44: mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 45: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 46: mul r4.xyz, r0.wwww, cb0[7].xyzx
    r4.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 47: mad r4.xyz, r0.zzzz, cb0[6].xyzx, r4.xyzx
    r4.xyz = ((r0.zzzz)*(source[6].xyzx)+(r4.xyzx)).xyz;
    // 48: mul r4.xyz, r4.xyzx, cb0[8].wwww
    r4.xyz = ((r4.xyzx)*(source[8].wwww)).xyz;
    // 49: mul r5.xyz, cb0[2].xyzx, cb0[4].yyyy
    r5.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 51: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 52: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 53: mul r7.xyz, r4.xyzx, r5.xyzx
    r7.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 54: mad r0.yzw, r2.xxyz, r0.yyyy, r4.xxyz
    r0.yzw = ((r2.xxyz)*(r0.yyyy)+(r4.xxyz)).yzw;
    // 55: add r0.yzw, r0.yyzw, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r0.yyzw)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 56: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 57: mad r3.xyz, r5.xyzx, r3.xyzx, r7.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 58: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 59: mul r4.xyz, cb0[3].xyzx, cb0[4].zzzz
    r4.xyz = ((source[3].xyzx)*(source[4].zzzz)).xyz;
    // 60: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 61: mul_sat r0.z, r6.w, cb0[5].x
    r0.z = (saturate((r6.wwww)*(source[5].xxxx))).z;
    // 62: mul o0.w, r0.z, cb0[0].x
    output.targets[0].w = ((r0.zzzz)*(source[0].xxxx)).w;
    // 63: mad r4.xyz, r4.xyzx, cb2[4].wwww, cb2[4].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 64: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 65: mad r3.xyz, r2.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 66: mul r0.xzw, r0.xxxx, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)).xzw;
    // 67: dp3 o4.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 68: add r0.xzw, r3.xxyz, cb0[1].xxyz
    r0.xzw = ((r3.xxyz)+(source[1].xxyz)).xzw;
    // 69: mad r0.xzw, r5.xxyz, cb0[8].xxyz, r0.xxzw
    r0.xzw = ((r5.xxyz)*(source[8].xxyz)+(r0.xxzw)).xzw;
    // 70: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 71: mad o0.xyz, r0.xzwx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xzwx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 72: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 73: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 74: mul r0.xzw, r0.xxxx, v1.xxyz
    r0.xzw = ((r0.xxxx)*(v1.xxyz)).xzw;
    // 75: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 76: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 77: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 78: mul r4.xyz, r0.wxzw, r2.yzxy
    r4.xyz = ((r0.wxzw)*(r2.yzxy)).xyz;
    // 79: mad r4.xyz, r0.zwxz, r2.zxyz, -r4.xyzx
    r4.xyz = ((r0.zwxz)*(r2.zxyz)+(-(r4.xyzx))).xyz;
    // 80: dp3 r5.z, r0.xzwx, r1.xyzx
    r5.z = (dot((r0.xzwx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 81: dp3 r5.x, r2.xyzx, r1.xyzx
    r5.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 82: mul r0.xzw, r4.xxyz, v1.wwww
    r0.xzw = ((r4.xxyz)*(v1.wwww)).xzw;
    // 83: dp3 r5.y, r0.xzwx, r1.xyzx
    r5.y = (dot((r0.xzwx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 84: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 85: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 86: mul r0.xzw, r0.xxxx, r5.xxyz
    r0.xzw = ((r0.xxxx)*(r5.xxyz)).xzw;
    // 87: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 88: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xzwx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xzwx)).xyz).xxxx).w;
    // 89: div r0.xz, r0.xxzx, r0.wwww
    r0.xz = ((r0.xxzx)/(r0.wwww)).xz;
    // 90: ge r1.yz, r0.xxzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxzx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 91: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 92: mad r1.yz, -|r0.zzxz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzxz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 93: movc r0.xz, r1.xxxx, r1.yyzy, r0.xxzx
    r0.xz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.xxzx)).xz;
    // 94: mad o2.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xzxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 95: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 96: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 97: mul o4.z, r0.y, r3.x
    output.targets[4].z = ((r0.yyyy)*(r3.xxxx)).z;
    // 98: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 99: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 100: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 101: ret
    return output;
}

// source.character.static-map-native-1117.v1 / source program c40ef804fec1a444909b4d38292e092d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1117(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1117(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: mul r0.xy, r0.xyxx, cb0[3].xxxx
    r0.xy = ((r0.xyxx)*(source[3].xxxx)).xy;
    // 5: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 6: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 9: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 11: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 12: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 13: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 16: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r1.xyz, r0.wwww, v7.xyzx
    r1.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 19: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 20: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 22: mul r1.yzw, r1.yyyy, cb0[6].xxyz
    r1.yzw = ((r1.yyyy)*(source[6].xxyz)).yzw;
    // 23: mad r1.xyz, r1.xxxx, cb0[5].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[5].xyzx)+(r1.yzwy)).xyz;
    // 24: mul r1.xyz, r1.xyzx, cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(source[7].wwww)).xyz;
    // 25: mul r2.xyz, cb0[2].xyzx, cb0[3].yyyy
    r2.xyz = ((source[2].xyzx)*(source[3].yyyy)).xyz;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 27: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 28: mul_sat r0.w, r3.w, cb0[4].x
    r0.w = (saturate((r3.wwww)*(source[4].xxxx))).w;
    // 29: mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // 30: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 31: mad r3.xyz, r1.xyzx, r2.xyzx, cb0[1].xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)+(source[1].xyzx)).xyz;
    // 32: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 33: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 34: mad r1.xyz, r2.xyzx, cb0[7].xyzx, r3.xyzx
    r1.xyz = ((r2.xyzx)*(source[7].xyzx)+(r3.xyzx)).xyz;
    // 35: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 36: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 37: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 38: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 39: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 40: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 41: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 42: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 43: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 44: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 45: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 46: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 47: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 48: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 49: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 50: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 51: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 52: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 53: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 54: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 55: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 56: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 57: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 58: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 59: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 60: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 61: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 62: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 63: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 64: ret
    return output;
}

// source.character.static-map-native-1118.v1 / source program b787a93a3b67bb40a99e6a744d009fe2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1118(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[10]=g_SourceCharacterBaseConstants[8];
    source[11]=g_SourceCharacterBaseConstants[9];
    source[12]=g_SourceCharacterBaseConstants[10];
    source[13]=g_SourceCharacterBaseConstants[11];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    source[27]=1.f;
    source[28]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: mul r0.xy, cb0[0].xyxx, cb0[8].wwww
    r0.xy = ((source[0].xyxx)*(source[8].wwww)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 8: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 10: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 11: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 12: mul r1.zw, v4.xxxy, cb0[3].xxxy
    r1.zw = ((v4.xxxy)*(source[3].xxxy)).zw;
    // 13: mul r3.xy, r1.zwzz, cb0[8].xxxx
    r3.xy = ((r1.zwzz)*(source[8].xxxx)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 15: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mul r3.xy, r3.xyxx, cb0[8].yyyy
    r3.xy = ((r3.xyxx)*(source[8].yyyy)).xy;
    // 17: mad r1.xy, cb0[7].xxxx, r1.xyxx, r3.xyxx
    r1.xy = ((source[7].xxxx)*(r1.xyxx)+(r3.xyxx)).xy;
    // 18: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 19: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 21: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 22: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 25: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 26: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 27: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 28: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 29: dp3 r4.z, r5.xyzx, r2.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 30: mul r6.xyz, r3.yzxy, r5.zxyz
    r6.xyz = ((r3.yzxy)*(r5.zxyz)).xyz;
    // 31: mad r6.xyz, r5.yzxy, r3.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r3.zxyz)+(-(r6.xyzx))).xyz;
    // 32: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 33: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 34: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[9].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).x;
    // 37: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.zwzz, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 41: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 42: mul r0.zw, v4.xxxy, cb0[9].wwww
    r0.zw = ((v4.xxxy)*(source[9].wwww)).zw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 45: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 46: max r0.z, cb0[8].z, l(0.000000)
    r0.z = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 47: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 48: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 49: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 50: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 51: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 53: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 54: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 55: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 56: mul r0.yzw, cb0[6].xxyz, cb0[10].yyyy
    r0.yzw = ((source[6].xxyz)*(source[10].yyyy)).yzw;
    // 57: mul r8.xyz, r7.xyzx, r0.yzwy
    r8.xyz = ((r7.xyzx)*(r0.yzwy)).xyz;
    // 58: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 59: mad r0.yzw, -r0.yyzw, r7.xxyz, r1.wwww
    r0.yzw = ((-(r0.yyzw))*(r7.xxyz)+(r1.wwww)).yzw;
    // 60: mad r0.yzw, cb0[10].wwww, r0.yyzw, r8.xxyz
    r0.yzw = ((source[10].wwww)*(r0.yyzw)+(r8.xxyz)).yzw;
    // 61: mul r7.xyz, cb0[5].xyzx, cb0[10].xxxx
    r7.xyz = ((source[5].xyzx)*(source[10].xxxx)).xyz;
    // 62: mad r0.yzw, -r4.xxyz, r7.xxyz, r0.yyzw
    r0.yzw = ((-(r4.xxyz))*(r7.xxyz)+(r0.yyzw)).yzw;
    // 63: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 64: mad r0.yzw, r0.xxxx, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.xxxx)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 65: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 66: mul r4.xyz, r0.yzwy, cb0[11].xxxx
    r4.xyz = ((r0.yzwy)*(source[11].xxxx)).xyz;
    // 67: mad r0.yzw, cb0[11].yyyy, r0.yyzw, -r4.xxyz
    r0.yzw = ((source[11].yyyy)*(r0.yyzw)+(-(r4.xxyz))).yzw;
    // 68: mul r1.z, r1.z, cb0[11].z
    r1.z = ((r1.zzzz)*(source[11].zzzz)).z;
    // 69: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 70: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 71: mul r1.w, r1.w, cb0[11].w
    r1.w = ((r1.wwww)*(source[11].wwww)).w;
    // 72: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 73: movc r1.z, r1.z, l(0), r1.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).z;
    // 74: min r1.w, r1.z, l(1.000000)
    r1.w = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: mul_sat r7.w, r1.z, cb2[3].w
    r7.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 76: mad r0.yzw, r1.wwww, r0.yyzw, r4.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 77: add r4.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 78: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 79: mad_sat r4.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 80: mad r0.yzw, r4.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r4.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 81: mad r8.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 82: mad r9.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 83: mul r1.x, r1.x, cb0[13].x
    r1.x = ((r1.xxxx)*(source[13].xxxx)).x;
    // 84: mul r1.y, r1.y, cb0[12].z
    r1.y = ((r1.yyyy)*(source[12].zzzz)).y;
    // 85: log r1.z, |r1.x|
    r1.z = (log2(abs(r1.xxxx))).z;
    // 86: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 87: mul r1.z, r1.z, cb0[13].y
    r1.z = ((r1.zzzz)*(source[13].yyyy)).z;
    // 88: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 89: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 90: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 91: mad r8.xyz, r1.xxxx, r8.xyzx, r9.xyzx
    r8.xyz = ((r1.xxxx)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 92: mad r0.yzw, r8.xxyz, r1.xxxx, r0.yyzw
    r0.yzw = ((r8.xxyz)*(r1.xxxx)+(r0.yyzw)).yzw;
    // 93: mul r0.yzw, r1.xxxx, r0.yyzw
    r0.yzw = ((r1.xxxx)*(r0.yyzw)).yzw;
    // 94: max r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = (max(r0.yyzw,r1.xxxx)).yzw;
    // 95: add r8.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 96: mad r2.xyz, r0.xxxx, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 97: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 98: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 99: mul r8.xyz, r0.xxxx, r2.xyzx
    r8.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 100: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 101: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 102: mul r9.xyz, r0.xxxx, v6.xyzx
    r9.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 103: dp3 r0.x, r9.xyzx, r8.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 104: mad r1.zw, r0.xxxx, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r0.xxxx)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 105: mul r1.zw, r1.zzzw, r1.zzzw
    r1.zw = ((r1.zzzw)*(r1.zzzw)).zw;
    // 106: mul r9.xyz, r1.wwww, cb0[25].xyzx
    r9.xyz = ((r1.wwww)*(source[25].xyzx)).xyz;
    // 107: mad r9.xyz, r1.zzzz, cb0[24].xyzx, r9.xyzx
    r9.xyz = ((r1.zzzz)*(source[24].xyzx)+(r9.xyzx)).xyz;
    // 108: mul r9.xyz, r9.xyzx, cb0[26].wwww
    r9.xyz = ((r9.xyzx)*(source[26].wwww)).xyz;
    // 109: mul r10.xyz, r4.xyzx, r9.xyzx
    r10.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 110: dp2_sat r11.x, r8.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r11.x = (saturate(dot((r8.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 111: dp3_sat r11.y, r8.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r11.y = (saturate(dot((r8.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 112: dp3_sat r11.z, r8.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r11.z = (saturate(dot((r8.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 113: mul r11.xyz, r11.xyzx, r11.xyzx
    r11.xyz = ((r11.xyzx)*(r11.xyzx)).xyz;
    // 114: sample_indexable(texture2d)(float,float,float,float) r12.xyz, v3.zwzz, t8.xyzw, s5
    r12.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 115: mul r12.xyz, r12.xyzx, cb0[28].xyzx
    r12.xyz = ((r12.xyzx)*(source[28].xyzx)).xyz;
    // 116: dp3 r0.x, r12.xyzx, r11.xyzx
    r0.x = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 117: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t7.xyzw, s5
    r11.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 118: mul r11.xyz, r11.xyzx, cb0[27].xyzx
    r11.xyz = ((r11.xyzx)*(source[27].xyzx)).xyz;
    // 119: mul r13.xyz, r0.xxxx, r11.xyzx
    r13.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 120: mad r10.xyz, r4.xyzx, r13.xyzx, r10.xyzx
    r10.xyz = ((r4.xyzx)*(r13.xyzx)+(r10.xyzx)).xyz;
    // 121: mul r0.yzw, r0.yyzw, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r10.xxyz)).yzw;
    // 122: dp3 r10.x, r3.xyzx, r8.xyzx
    r10.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 123: dp3 r10.y, r6.xyzx, r8.xyzx
    r10.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 124: dp2 r13.z, r10.xyxx, cb0[15].xyxx
    r13.z = (dot((r10.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 125: dp3 r13.y, r5.xyzx, r8.xyzx
    r13.y = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 126: mul r1.zw, cb0[15].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r1.zw = ((source[15].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 127: dp2 r13.x, r10.xyxx, r1.zwzz
    r13.x = (dot((r10.xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 128: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 129: dp4 r14.x, cb0[16].xyzw, r13.xyzw
    r14.x = (dot((source[16].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 130: dp4 r14.y, cb0[17].xyzw, r13.xyzw
    r14.y = (dot((source[17].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 131: dp4 r14.z, cb0[18].xyzw, r13.xyzw
    r14.z = (dot((source[18].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 132: mul r15.xyzw, r13.yzzx, r13.xyzz
    r15.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 133: dp4 r16.x, cb0[19].xyzw, r15.xyzw
    r16.x = (dot((source[19].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 134: dp4 r16.y, cb0[20].xyzw, r15.xyzw
    r16.y = (dot((source[20].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 135: dp4 r16.z, cb0[21].xyzw, r15.xyzw
    r16.z = (dot((source[21].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 136: add r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)+(r16.xyzx)).xyz;
    // 137: mul r2.w, r13.y, r13.y
    r2.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 138: mov r10.z, r13.y
    r10.z = (r13.yyyy).z;
    // 139: mad r2.w, r13.x, r13.x, -r2.w
    r2.w = ((r13.xxxx)*(r13.xxxx)+(-(r2.wwww))).w;
    // 140: mad r13.xyz, cb0[22].xyzx, r2.wwww, r14.xyzx
    r13.xyz = ((source[22].xyzx)*(r2.wwww)+(r14.xyzx)).xyz;
    // 141: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 142: mul r13.xyz, r13.xyzx, cb0[14].xyzx
    r13.xyz = ((r13.xyzx)*(source[14].xyzx)).xyz;
    // 143: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 144: mov_sat r4.w, cb0[12].x
    r4.w = (saturate(source[12].xxxx)).w;
    // 145: mad r14.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r14.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 146: mul r2.w, r4.w, l(0.080000)
    r2.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 147: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 148: mad r14.xyz, r7.wwww, r14.xyzx, r2.wwww
    r14.xyz = ((r7.wwww)*(r14.xyzx)+(r2.wwww)).xyz;
    // 149: mul_sat r2.w, r14.y, l(50.000000)
    r2.w = (saturate((r14.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 150: log r3.w, |r1.y|
    r3.w = (log2(abs(r1.yyyy))).w;
    // 151: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 152: mul r3.w, r3.w, cb0[12].w
    r3.w = ((r3.wwww)*(source[12].wwww)).w;
    // 153: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 154: movc r1.y, r1.y, l(0), r3.w
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).y;
    // 155: max r1.y, r1.y, cb0[1].x
    r1.y = (max(r1.yyyy,source[1].xxxx)).y;
    // 156: min r7.z, r1.y, l(1.000000)
    r7.z = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 157: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 158: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 159: mul r15.xyz, r1.yyyy, v5.xyzx
    r15.xyz = ((r1.yyyy)*(v5.xyzx)).xyz;
    // 160: dp3 r1.y, r8.xyzx, r15.xyzx
    r1.y = (dot((r8.xyzx).xyz,(r15.xyzx).xyz).xxxx).y;
    // 161: mul r8.xyz, r1.yyyy, r8.xyzx
    r8.xyz = ((r1.yyyy)*(r8.xyzx)).xyz;
    // 162: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r15.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r15.xyzx))).xyz;
    // 163: deriv_rtx_coarse r7.x, r1.y
    r7.x = (ddx_coarse(r1.yyyy)).x;
    // 164: deriv_rty_coarse r7.y, r1.y
    r7.y = (ddy_coarse(r1.yyyy)).y;
    // 165: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 166: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 167: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 168: mad_sat r7.y, r3.w, l(0.300000), r7.z
    r7.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 169: add r3.w, -r7.y, l(1.000000)
    r3.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: max r16.xyz, r14.xyzx, r3.wwww
    r16.xyz = (max(r14.xyzx,r3.wwww)).xyz;
    // 171: add r16.xyz, -r14.xyzx, r16.xyzx
    r16.xyz = ((-(r14.xyzx))+(r16.xyzx)).xyz;
    // 172: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 173: add r2.w, r8.z, l(1.000000)
    r2.w = ((r8.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: add_sat r7.x, r1.y, -r2.w
    r7.x = (saturate((r1.yyyy)+(-(r2.wwww)))).x;
    // 176: sample_indexable(texture2d)(float,float,float,float) r17.xy, r7.xyxx, t5.xyzw, s7
    r17.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 177: add r1.y, r1.x, r7.x
    r1.y = ((r1.xxxx)+(r7.xxxx)).y;
    // 178: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 179: mul r18.xyz, r14.xyzx, r17.yyyy
    r18.xyz = ((r14.xyzx)*(r17.yyyy)).xyz;
    // 180: mad r16.xyz, r16.xyzx, r17.xxxx, r18.xyzx
    r16.xyz = ((r16.xyzx)*(r17.xxxx)+(r18.xyzx)).xyz;
    // 181: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r17.y
    r2.w = r17.y != 0.f ? 1.f / r17.y : 0.f;
    // 182: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 183: mad r17.xyz, r14.xyzx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r14.xyzx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: dp3 r2.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 185: mad r14.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r14.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 186: mad r18.xyz, -r16.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r16.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mul r16.xyz, r16.xyzx, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r17.xyzx)).xyz;
    // 188: mul r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)*(r18.xyzx)).xyz;
    // 189: mul r0.yzw, r0.yyzw, r13.xxyz
    r0.yzw = ((r0.yyzw)*(r13.xxyz)).yzw;
    // 190: mad r0.yzw, -r0.yyzw, r7.wwww, r0.yyzw
    r0.yzw = ((-(r0.yyzw))*(r7.wwww)+(r0.yyzw)).yzw;
    // 191: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 192: dp3 r3.x, r3.xyzx, r8.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 193: dp3 r3.y, r6.xyzx, r8.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 194: dp2 r6.x, r3.xyxx, r1.zwzz
    r6.x = (dot((r3.xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 195: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 196: mul r1.z, r7.y, l(5.000000)
    r1.z = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 197: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 198: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 199: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 200: add r1.y, r1.x, r1.y
    r1.y = ((r1.xxxx)+(r1.yyyy)).y;
    // 201: mov o5.y, r1.x
    output.targets[5].y = (r1.xxxx).y;
    // 202: add_sat r1.x, r1.y, l(-1.000000)
    r1.x = (saturate((r1.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 203: dp3 r6.y, r5.xyzx, r8.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 204: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t6.xyzw, s6, r1.z
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r1.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 205: mul r1.yzw, r3.xxyz, r3.wwww
    r1.yzw = ((r3.xxyz)*(r3.wwww)).yzw;
    // 206: mul r1.yzw, r1.yyzw, cb0[14].xxyz
    r1.yzw = ((r1.yyzw)*(source[14].xxyz)).yzw;
    // 207: mad r1.yzw, r1.yyzw, l(0.000000, 6.000000, 6.000000, 6.000000), cb0[14].wwww
    r1.yzw = ((r1.yyzw)*(float4(0.000000,6.000000,6.000000,6.000000))+(source[14].wwww)).yzw;
    // 208: dp2_sat r3.x, r8.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r8.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 209: dp3_sat r3.y, r8.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r8.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 210: dp3_sat r3.z, r8.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r8.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 211: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 212: dp3 r2.w, r12.xyzx, r3.xyzx
    r2.w = (dot((r12.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 213: add r0.x, r0.x, -r2.w
    r0.x = ((r0.xxxx)+(-(r2.wwww))).x;
    // 214: mad r0.x, r7.z, r0.x, r2.w
    r0.x = ((r7.zzzz)*(r0.xxxx)+(r2.wwww)).x;
    // 215: mad r3.xyz, r11.xyzx, r0.xxxx, r9.xyzx
    r3.xyz = ((r11.xyzx)*(r0.xxxx)+(r9.xyzx)).xyz;
    // 216: mul r5.xyz, r0.xxxx, r11.xyzx
    r5.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 217: mad r0.x, r1.x, r14.x, r14.y
    r0.x = ((r1.xxxx)*(r14.xxxx)+(r14.yyyy)).x;
    // 218: mad r0.x, r0.x, r1.x, r14.z
    r0.x = ((r0.xxxx)*(r1.xxxx)+(r14.zzzz)).x;
    // 219: mul r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)*(r0.xxxx)).x;
    // 220: max r0.x, r0.x, r1.x
    r0.x = (max(r0.xxxx,r1.xxxx)).x;
    // 221: mul r6.xyz, r0.xxxx, r3.xyzx
    r6.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 222: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 223: div r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)/(r3.xyzx)).xyz;
    // 224: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 225: mul r1.xyz, r1.yzwy, r6.xyzx
    r1.xyz = ((r1.yzwy)*(r6.xyzx)).xyz;
    // 226: mad r0.yzw, r1.xxyz, r16.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(r16.xxyz)+(r0.yyzw)).yzw;
    // 227: mul r1.xyz, r16.xyzx, r1.xyzx
    r1.xyz = ((r16.xyzx)*(r1.xyzx)).xyz;
    // 228: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 229: dp3 r1.x, r2.xyzx, r15.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 230: add r1.y, -|r15.z|, l(1.000000)
    r1.y = ((-(abs(r15.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 231: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 232: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 233: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 234: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 235: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 236: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 237: mul r1.xzw, r1.xxxx, cb0[4].xxyz
    r1.xzw = ((r1.xxxx)*(source[4].xxyz)).xzw;
    // 238: movc r1.xyz, r1.yyyy, l(0,0,0,0), r1.xzwx
    r1.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xzwx)).xyz;
    // 239: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 240: add r1.xyz, r0.yzwy, r1.xyzx
    r1.xyz = ((r0.yzwy)+(r1.xyzx)).xyz;
    // 241: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 242: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 243: dp3 r1.x, r10.xyzx, r10.xyzx
    r1.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 244: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 245: mul r1.xyz, r1.xxxx, r10.xyzx
    r1.xyz = ((r1.xxxx)*(r10.xyzx)).xyz;
    // 246: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 247: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 248: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 249: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 250: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 251: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 252: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 253: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 254: mul o4.z, r0.x, r0.y
    output.targets[4].z = ((r0.xxxx)*(r0.yyyy)).z;
    // 255: dp3 o4.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 256: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 257: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 258: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 259: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 260: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 261: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 262: ret
    return output;
}

// source.character.static-map-native-1118.v1 / source program eaa4ddb30de4e248b27579802e6013ea
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1118(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1118(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[10]=g_SourceCharacterBaseConstants[8];
    source[11]=g_SourceCharacterBaseConstants[9];
    source[12]=g_SourceCharacterBaseConstants[10];
    source[13]=g_SourceCharacterBaseConstants[11];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f;
    // 1: mul r0.xy, cb0[0].xyxx, cb0[8].wwww
    r0.xy = ((source[0].xyxx)*(source[8].wwww)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 8: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 10: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 11: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 12: mul r1.zw, v4.xxxy, cb0[3].xxxy
    r1.zw = ((v4.xxxy)*(source[3].xxxy)).zw;
    // 13: mul r3.xy, r1.zwzz, cb0[8].xxxx
    r3.xy = ((r1.zwzz)*(source[8].xxxx)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 15: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mul r3.xy, r3.xyxx, cb0[8].yyyy
    r3.xy = ((r3.xyxx)*(source[8].yyyy)).xy;
    // 17: mad r1.xy, cb0[7].xxxx, r1.xyxx, r3.xyxx
    r1.xy = ((source[7].xxxx)*(r1.xyxx)+(r3.xyxx)).xy;
    // 18: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 19: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 21: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 22: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 25: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 26: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 27: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 28: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 29: dp3 r4.z, r5.xyzx, r2.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 30: mul r6.xyz, r3.yzxy, r5.zxyz
    r6.xyz = ((r3.yzxy)*(r5.zxyz)).xyz;
    // 31: mad r6.xyz, r5.yzxy, r3.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r3.zxyz)+(-(r6.xyzx))).xyz;
    // 32: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 33: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 34: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[9].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).x;
    // 37: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.zwzz, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 41: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 42: mul r0.zw, v4.xxxy, cb0[9].wwww
    r0.zw = ((v4.xxxy)*(source[9].wwww)).zw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 45: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 46: max r0.z, cb0[8].z, l(0.000000)
    r0.z = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 47: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 48: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 49: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 50: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 51: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 53: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 54: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 55: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 56: mul r0.yzw, cb0[6].xxyz, cb0[10].yyyy
    r0.yzw = ((source[6].xxyz)*(source[10].yyyy)).yzw;
    // 57: mul r8.xyz, r7.xyzx, r0.yzwy
    r8.xyz = ((r7.xyzx)*(r0.yzwy)).xyz;
    // 58: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 59: mad r0.yzw, -r0.yyzw, r7.xxyz, r1.wwww
    r0.yzw = ((-(r0.yyzw))*(r7.xxyz)+(r1.wwww)).yzw;
    // 60: mad r0.yzw, cb0[10].wwww, r0.yyzw, r8.xxyz
    r0.yzw = ((source[10].wwww)*(r0.yyzw)+(r8.xxyz)).yzw;
    // 61: mul r7.xyz, cb0[5].xyzx, cb0[10].xxxx
    r7.xyz = ((source[5].xyzx)*(source[10].xxxx)).xyz;
    // 62: mad r0.yzw, -r4.xxyz, r7.xxyz, r0.yyzw
    r0.yzw = ((-(r4.xxyz))*(r7.xxyz)+(r0.yyzw)).yzw;
    // 63: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 64: mad r0.yzw, r0.xxxx, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.xxxx)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 65: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 66: mul r4.xyz, r0.yzwy, cb0[11].xxxx
    r4.xyz = ((r0.yzwy)*(source[11].xxxx)).xyz;
    // 67: mad r0.yzw, cb0[11].yyyy, r0.yyzw, -r4.xxyz
    r0.yzw = ((source[11].yyyy)*(r0.yyzw)+(-(r4.xxyz))).yzw;
    // 68: mul r1.z, r1.z, cb0[11].z
    r1.z = ((r1.zzzz)*(source[11].zzzz)).z;
    // 69: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 70: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 71: mul r1.w, r1.w, cb0[11].w
    r1.w = ((r1.wwww)*(source[11].wwww)).w;
    // 72: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 73: movc r1.z, r1.z, l(0), r1.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).z;
    // 74: min r1.w, r1.z, l(1.000000)
    r1.w = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: mul_sat r7.w, r1.z, cb2[3].w
    r7.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 76: mad r0.yzw, r1.wwww, r0.yyzw, r4.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 77: add r4.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 78: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 79: mad_sat r4.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 80: mad r0.yzw, r4.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r4.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 81: mad r8.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 82: mad r9.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 83: mul r1.x, r1.x, cb0[13].x
    r1.x = ((r1.xxxx)*(source[13].xxxx)).x;
    // 84: mul r1.y, r1.y, cb0[12].z
    r1.y = ((r1.yyyy)*(source[12].zzzz)).y;
    // 85: log r1.z, |r1.x|
    r1.z = (log2(abs(r1.xxxx))).z;
    // 86: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 87: mul r1.z, r1.z, cb0[13].y
    r1.z = ((r1.zzzz)*(source[13].yyyy)).z;
    // 88: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 89: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 90: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 91: mad r8.xyz, r1.xxxx, r8.xyzx, r9.xyzx
    r8.xyz = ((r1.xxxx)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 92: mad r0.yzw, r8.xxyz, r1.xxxx, r0.yyzw
    r0.yzw = ((r8.xxyz)*(r1.xxxx)+(r0.yyzw)).yzw;
    // 93: mul r0.yzw, r1.xxxx, r0.yyzw
    r0.yzw = ((r1.xxxx)*(r0.yyzw)).yzw;
    // 94: max r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = (max(r0.yyzw,r1.xxxx)).yzw;
    // 95: add r8.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 96: mad r2.xyz, r0.xxxx, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 97: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 98: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 99: mul r8.xyz, r0.xxxx, r2.xyzx
    r8.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 100: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 101: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 102: mul r9.xyz, r0.xxxx, v6.xyzx
    r9.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 103: dp3 r0.x, r9.xyzx, r8.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 104: mad r1.zw, r0.xxxx, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r0.xxxx)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 105: mul r1.zw, r1.zzzw, r1.zzzw
    r1.zw = ((r1.zzzw)*(r1.zzzw)).zw;
    // 106: mul r10.xyz, r1.wwww, cb0[25].xyzx
    r10.xyz = ((r1.wwww)*(source[25].xyzx)).xyz;
    // 107: mad r10.xyz, r1.zzzz, cb0[24].xyzx, r10.xyzx
    r10.xyz = ((r1.zzzz)*(source[24].xyzx)+(r10.xyzx)).xyz;
    // 108: mul r10.xyz, r10.xyzx, cb0[26].wwww
    r10.xyz = ((r10.xyzx)*(source[26].wwww)).xyz;
    // 109: mul r10.xyz, r4.xyzx, r10.xyzx
    r10.xyz = ((r4.xyzx)*(r10.xyzx)).xyz;
    // 110: mul r0.xyz, r0.yzwy, r10.xyzx
    r0.xyz = ((r0.yzwy)*(r10.xyzx)).xyz;
    // 111: dp3 r10.x, r3.xyzx, r8.xyzx
    r10.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 112: dp3 r10.y, r6.xyzx, r8.xyzx
    r10.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 113: dp2 r11.z, r10.xyxx, cb0[15].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 114: dp3 r11.y, r5.xyzx, r8.xyzx
    r11.y = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 115: mul r1.zw, cb0[15].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r1.zw = ((source[15].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 116: dp2 r11.x, r10.xyxx, r1.zwzz
    r11.x = (dot((r10.xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 117: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 118: dp4 r12.x, cb0[16].xyzw, r11.xyzw
    r12.x = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 119: dp4 r12.y, cb0[17].xyzw, r11.xyzw
    r12.y = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 120: dp4 r12.z, cb0[18].xyzw, r11.xyzw
    r12.z = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 121: mul r13.xyzw, r11.yzzx, r11.xyzz
    r13.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 122: dp4 r14.x, cb0[19].xyzw, r13.xyzw
    r14.x = (dot((source[19].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 123: dp4 r14.y, cb0[20].xyzw, r13.xyzw
    r14.y = (dot((source[20].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 124: dp4 r14.z, cb0[21].xyzw, r13.xyzw
    r14.z = (dot((source[21].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 125: add r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)+(r14.xyzx)).xyz;
    // 126: mul r0.w, r11.y, r11.y
    r0.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 127: mov r10.z, r11.y
    r10.z = (r11.yyyy).z;
    // 128: mad r0.w, r11.x, r11.x, -r0.w
    r0.w = ((r11.xxxx)*(r11.xxxx)+(-(r0.wwww))).w;
    // 129: mad r11.xyz, cb0[22].xyzx, r0.wwww, r12.xyzx
    r11.xyz = ((source[22].xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 130: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 131: mul r11.xyz, r11.xyzx, cb0[14].xyzx
    r11.xyz = ((r11.xyzx)*(source[14].xyzx)).xyz;
    // 132: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 133: mov_sat r4.w, cb0[12].x
    r4.w = (saturate(source[12].xxxx)).w;
    // 134: mad r12.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r12.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 135: mul r0.w, r4.w, l(0.080000)
    r0.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 136: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 137: mad r12.xyz, r7.wwww, r12.xyzx, r0.wwww
    r12.xyz = ((r7.wwww)*(r12.xyzx)+(r0.wwww)).xyz;
    // 138: mul_sat r0.w, r12.y, l(50.000000)
    r0.w = (saturate((r12.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 139: log r2.w, |r1.y|
    r2.w = (log2(abs(r1.yyyy))).w;
    // 140: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 141: mul r2.w, r2.w, cb0[12].w
    r2.w = ((r2.wwww)*(source[12].wwww)).w;
    // 142: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 143: movc r1.y, r1.y, l(0), r2.w
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 144: max r1.y, r1.y, cb0[1].x
    r1.y = (max(r1.yyyy,source[1].xxxx)).y;
    // 145: min r7.z, r1.y, l(1.000000)
    r7.z = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 147: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 148: mul r13.xyz, r1.yyyy, v5.xyzx
    r13.xyz = ((r1.yyyy)*(v5.xyzx)).xyz;
    // 149: dp3 r1.y, r8.xyzx, r13.xyzx
    r1.y = (dot((r8.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 150: mul r8.xyz, r1.yyyy, r8.xyzx
    r8.xyz = ((r1.yyyy)*(r8.xyzx)).xyz;
    // 151: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r13.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r13.xyzx))).xyz;
    // 152: deriv_rtx_coarse r7.x, r1.y
    r7.x = (ddx_coarse(r1.yyyy)).x;
    // 153: deriv_rty_coarse r7.y, r1.y
    r7.y = (ddy_coarse(r1.yyyy)).y;
    // 154: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 155: dp2 r2.w, r7.xyxx, r7.xyxx
    r2.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 156: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 157: mad_sat r7.y, r2.w, l(0.300000), r7.z
    r7.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 158: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 159: add r2.w, -r7.y, l(1.000000)
    r2.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: max r14.xyz, r12.xyzx, r2.wwww
    r14.xyz = (max(r12.xyzx,r2.wwww)).xyz;
    // 161: add r14.xyz, -r12.xyzx, r14.xyzx
    r14.xyz = ((-(r12.xyzx))+(r14.xyzx)).xyz;
    // 162: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 163: add r0.w, r8.z, l(1.000000)
    r0.w = ((r8.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: add_sat r7.x, -r0.w, r1.y
    r7.x = (saturate((-(r0.wwww))+(r1.yyyy))).x;
    // 166: sample_indexable(texture2d)(float,float,float,float) r15.xy, r7.xyxx, t5.xyzw, s6
    r15.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 167: add r0.w, r1.x, r7.x
    r0.w = ((r1.xxxx)+(r7.xxxx)).w;
    // 168: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 169: mul r16.xyz, r12.xyzx, r15.yyyy
    r16.xyz = ((r12.xyzx)*(r15.yyyy)).xyz;
    // 170: mad r14.xyz, r14.xyzx, r15.xxxx, r16.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xxxx)+(r16.xyzx)).xyz;
    // 171: div r1.y, l(1.000000, 1.000000, 1.000000, 1.000000), r15.y
    r1.y = r15.y != 0.f ? 1.f / r15.y : 0.f;
    // 172: add r1.y, r1.y, l(-1.000000)
    r1.y = ((r1.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 173: mad r15.xyz, r12.xyzx, r1.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r12.xyzx)*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 174: dp3 r1.y, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 175: mad r12.xyz, r1.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r12.xyz = ((r1.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 176: mad r16.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 178: mul r11.xyz, r11.xyzx, r16.xyzx
    r11.xyz = ((r11.xyzx)*(r16.xyzx)).xyz;
    // 179: mul r0.xyz, r0.xyzx, r11.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 180: mad r0.xyz, -r0.xyzx, r7.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r7.wwww)+(r0.xyzx)).xyz;
    // 181: dp3 r3.x, r3.xyzx, r8.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 182: dp3 r3.y, r6.xyzx, r8.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 183: dp2 r6.x, r3.xyxx, r1.zwzz
    r6.x = (dot((r3.xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 184: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 185: mul r1.y, r7.y, l(5.000000)
    r1.y = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 186: mul r1.z, r7.y, r7.y
    r1.z = ((r7.yyyy)*(r7.yyyy)).z;
    // 187: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 188: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 189: add r0.w, r1.x, r0.w
    r0.w = ((r1.xxxx)+(r0.wwww)).w;
    // 190: mov o5.y, r1.x
    output.targets[5].y = (r1.xxxx).y;
    // 191: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 192: dp3 r6.y, r5.xyzx, r8.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 193: dp3 r1.x, r9.xyzx, r8.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 194: mad r1.xz, r1.xxxx, l(0.500000, 0.000000, -0.500000, 0.000000), l(0.500000, 0.000000, 0.500000, 0.000000)
    r1.xz = ((r1.xxxx)*(float4(0.500000,0.000000,-0.500000,0.000000))+(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 195: mul r1.xz, r1.xxzx, r1.xxzx
    r1.xz = ((r1.xxzx)*(r1.xxzx)).xz;
    // 196: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t6.xyzw, s5, r1.y
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r1.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 197: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 198: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 199: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 200: mad r1.y, r0.w, r12.x, r12.y
    r1.y = ((r0.wwww)*(r12.xxxx)+(r12.yyyy)).y;
    // 201: mad r1.y, r1.y, r0.w, r12.z
    r1.y = ((r1.yyyy)*(r0.wwww)+(r12.zzzz)).y;
    // 202: mul r1.y, r0.w, r1.y
    r1.y = ((r0.wwww)*(r1.yyyy)).y;
    // 203: max r0.w, r0.w, r1.y
    r0.w = (max(r0.wwww,r1.yyyy)).w;
    // 204: mul r1.yzw, r1.zzzz, cb0[25].xxyz
    r1.yzw = ((r1.zzzz)*(source[25].xxyz)).yzw;
    // 205: mad r1.xyz, cb0[24].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[24].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 206: mul r1.xyz, r1.xyzx, cb0[26].wwww
    r1.xyz = ((r1.xyzx)*(source[26].wwww)).xyz;
    // 207: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 208: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 209: mad r0.xyz, r1.xyzx, r14.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r14.xyzx)+(r0.xyzx)).xyz;
    // 210: mul r1.xyz, r14.xyzx, r1.xyzx
    r1.xyz = ((r14.xyzx)*(r1.xyzx)).xyz;
    // 211: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 212: dp3 r0.w, r2.xyzx, r13.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 213: add r1.x, -|r13.z|, l(1.000000)
    r1.x = ((-(abs(r13.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 214: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 215: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 216: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 217: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 218: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 219: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 220: mul r1.yzw, r0.wwww, cb0[4].xxyz
    r1.yzw = ((r0.wwww)*(source[4].xxyz)).yzw;
    // 221: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 222: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 223: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 224: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 225: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 226: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 227: dp3 r0.x, r10.xyzx, r10.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 228: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 229: mul r0.xyz, r0.xxxx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 230: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 231: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 232: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 233: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 234: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 235: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 236: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 237: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 238: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 239: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 240: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 241: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 242: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 243: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 244: ret
    return output;
}

// source.character.static-map-native-1119.v1 / source program 66df9a0385e9e0478ff7c132a2fac7ac
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1119(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[10];
    source[8].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[8].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[8].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[9]=g_SourceCharacterBaseConstants[11];
    source[10]=g_SourceCharacterBaseConstants[12];
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    source[26]=1.f;
    source[27]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 7: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 9: mul r1.xy, r1.xyxx, cb0[6].xxxx
    r1.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 10: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 18: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r2.xyz, r0.wwww, v5.xyzx
    r2.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 21: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 22: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: add r1.w, -|r2.z|, l(1.000000)
    r1.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 25: mad_sat r1.w, r0.w, cb0[6].w, -cb0[7].x
    r1.w = (saturate((r0.wwww)*(source[6].wwww)+(-(source[7].xxxx)))).w;
    // 26: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 27: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 28: mul r2.w, r2.w, cb0[7].y
    r2.w = ((r2.wwww)*(source[7].yyyy)).w;
    // 29: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 30: mul r3.xyz, r2.wwww, cb0[3].xyzx
    r3.xyz = ((r2.wwww)*(source[3].xyzx)).xyz;
    // 31: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 32: add r3.xyz, r3.xyzx, -cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[3].xyzx))).xyz;
    // 33: mad r3.xyz, cb0[3].wwww, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((source[3].wwww)*(r3.xyzx)+(source[3].xyzx)).xyz;
    // 34: mul r4.xyz, cb0[4].xyzx, cb0[7].wwww
    r4.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 35: mul r4.xyz, r4.xyzx, cb0[8].zzzz
    r4.xyz = ((r4.xyzx)*(source[8].zzzz)).xyz;
    // 36: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 37: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 38: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 39: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 40: mul r4.xyz, r4.xyzx, cb0[8].wwww
    r4.xyz = ((r4.xyzx)*(source[8].wwww)).xyz;
    // 41: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 42: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 44: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 45: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 46: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 47: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 48: mul r4.xyz, r1.wwww, cb0[2].xyzx
    r4.xyz = ((r1.wwww)*(source[2].xyzx)).xyz;
    // 49: movc r4.xyz, r0.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 50: mad r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r4.xyzx
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r4.xyzx)).xyz;
    // 51: add r3.xyz, r3.xyzx, cb0[1].xyzx
    r3.xyz = ((r3.xyzx)+(source[1].xyzx)).xyz;
    // 52: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 53: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 54: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 55: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 56: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 57: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 58: dp3 r0.w, r4.xyzx, r1.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 59: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 60: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 61: mul r4.yzw, r4.yyyy, cb0[24].xxyz
    r4.yzw = ((r4.yyyy)*(source[24].xxyz)).yzw;
    // 62: mad r4.xyz, r4.xxxx, cb0[23].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[23].xyzx)+(r4.yzwy)).xyz;
    // 63: mul r4.xyz, r4.xyzx, cb0[25].wwww
    r4.xyz = ((r4.xyzx)*(source[25].wwww)).xyz;
    // 64: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r5.xyz, -r0.xyzx, r0.wwww
    r5.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 66: mad r0.xyz, cb0[9].yyyy, r5.xyzx, r0.xyzx
    r0.xyz = ((source[9].yyyy)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 67: mul r5.xyz, cb0[5].xyzx, cb0[9].zzzz
    r5.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 68: mul r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 69: mul r5.xyz, r0.xyzx, cb0[9].wwww
    r5.xyz = ((r0.xyzx)*(source[9].wwww)).xyz;
    // 70: mad r0.xyz, cb0[10].xxxx, r0.xyzx, -r5.xyzx
    r0.xyz = ((source[10].xxxx)*(r0.xyzx)+(-(r5.xyzx))).xyz;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 72: mul r0.w, r6.z, cb0[10].y
    r0.w = ((r6.zzzz)*(source[10].yyyy)).w;
    // 73: mul r6.xy, r6.yxyy, cb0[11].ywyy
    r6.xy = ((r6.yxyy)*(source[11].ywyy)).xy;
    // 74: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 75: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 76: mul r1.w, r1.w, cb0[10].z
    r1.w = ((r1.wwww)*(source[10].zzzz)).w;
    // 77: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 78: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 79: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 80: mul_sat r6.w, r0.w, cb2[3].w
    r6.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 81: mad r0.xyz, r1.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 82: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 83: mul r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 84: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 85: mul r5.xyz, r0.xyzx, r4.xyzx
    r5.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 86: dp2_sat r7.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 87: dp3_sat r7.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 88: dp3_sat r7.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 89: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 90: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t6.xyzw, s3
    r8.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 91: mul r8.xyz, r8.xyzx, cb0[27].xyzx
    r8.xyz = ((r8.xyzx)*(source[27].xyzx)).xyz;
    // 92: dp3 r1.w, r8.xyzx, r7.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 93: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t5.xyzw, s3
    r7.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 94: mul r7.xyz, r7.xyzx, cb0[26].xyzx
    r7.xyz = ((r7.xyzx)*(source[26].xyzx)).xyz;
    // 95: mul r9.xyz, r1.wwww, r7.xyzx
    r9.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // 96: mad r5.xyz, r0.xyzx, r9.xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(r9.xyzx)+(r5.xyzx)).xyz;
    // 97: mad r9.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r9.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 98: mad r10.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 99: mad r11.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 100: log r12.xy, |r6.xyxx|
    r12.xy = (log2(abs(r6.xyxx))).xy;
    // 101: lt r6.xy, |r6.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r6.xy = (asfloat((uint4)((abs(r6.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 102: mul r2.w, r12.y, cb0[12].x
    r2.w = ((r12.yyyy)*(source[12].xxxx)).w;
    // 103: mul r3.w, r12.x, cb0[11].z
    r3.w = ((r12.xxxx)*(source[11].zzzz)).w;
    // 104: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 105: movc r3.w, r6.x, l(0), r3.w
    r3.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 106: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 107: min r6.z, r3.w, l(1.000000)
    r6.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 108: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 109: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: movc r2.w, r6.y, l(0), r2.w
    r2.w = ((asuint(r6.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 111: mad r10.xyz, r2.wwww, r10.xyzx, r11.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)+(r11.xyzx)).xyz;
    // 112: mad r9.xyz, r10.xyzx, r2.wwww, r9.xyzx
    r9.xyz = ((r10.xyzx)*(r2.wwww)+(r9.xyzx)).xyz;
    // 113: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 114: max r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = (max(r2.wwww,r9.xyzx)).xyz;
    // 115: mul r5.xyz, r5.xyzx, r9.xyzx
    r5.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // 116: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 117: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 118: mul r9.xyz, r3.wwww, v1.xyzx
    r9.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 119: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 120: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 121: mul r10.xyz, r3.wwww, v0.xyzx
    r10.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 122: mul r11.xyz, r9.zxyz, r10.yzxy
    r11.xyz = ((r9.zxyz)*(r10.yzxy)).xyz;
    // 123: mad r11.xyz, r9.yzxy, r10.zxyz, -r11.xyzx
    r11.xyz = ((r9.yzxy)*(r10.zxyz)+(-(r11.xyzx))).xyz;
    // 124: mul r11.xyz, r11.xyzx, v1.wwww
    r11.xyz = ((r11.xyzx)*(v1.wwww)).xyz;
    // 125: dp3 r12.y, r11.xyzx, r1.xyzx
    r12.y = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 126: dp3 r12.x, r10.xyzx, r1.xyzx
    r12.x = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 127: dp2 r13.z, r12.xyxx, cb0[14].xyxx
    r13.z = (dot((r12.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 128: mul r6.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 129: dp2 r13.x, r12.xyxx, r6.xyxx
    r13.x = (dot((r12.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 130: dp3 r13.y, r9.xyzx, r1.xyzx
    r13.y = (dot((r9.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 131: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 132: dp4 r14.x, cb0[15].xyzw, r13.xyzw
    r14.x = (dot((source[15].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 133: dp4 r14.y, cb0[16].xyzw, r13.xyzw
    r14.y = (dot((source[16].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 134: dp4 r14.z, cb0[17].xyzw, r13.xyzw
    r14.z = (dot((source[17].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 135: mul r15.xyzw, r13.yzzx, r13.xyzz
    r15.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 136: dp4 r16.x, cb0[18].xyzw, r15.xyzw
    r16.x = (dot((source[18].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 137: dp4 r16.y, cb0[19].xyzw, r15.xyzw
    r16.y = (dot((source[19].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 138: dp4 r16.z, cb0[20].xyzw, r15.xyzw
    r16.z = (dot((source[20].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 139: add r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)+(r16.xyzx)).xyz;
    // 140: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 141: mov r12.z, r13.y
    r12.z = (r13.yyyy).z;
    // 142: mad r3.w, r13.x, r13.x, -r3.w
    r3.w = ((r13.xxxx)*(r13.xxxx)+(-(r3.wwww))).w;
    // 143: mad r13.xyz, cb0[21].xyzx, r3.wwww, r14.xyzx
    r13.xyz = ((source[21].xyzx)*(r3.wwww)+(r14.xyzx)).xyz;
    // 144: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 145: mul r13.xyz, r13.xyzx, cb0[13].xyzx
    r13.xyz = ((r13.xyzx)*(source[13].xyzx)).xyz;
    // 146: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 147: dp3 r3.w, r1.xyzx, r2.xyzx
    r3.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 148: mul r1.xyz, r1.xyzx, r3.wwww
    r1.xyz = ((r1.xyzx)*(r3.wwww)).xyz;
    // 149: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 150: deriv_rtx_coarse r2.x, r3.w
    r2.x = (ddx_coarse(r3.wwww)).x;
    // 151: deriv_rty_coarse r2.y, r3.w
    r2.y = (ddy_coarse(r3.wwww)).y;
    // 152: add r2.z, r3.w, l(1.000000)
    r2.z = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 153: dp2 r2.x, r2.xyxx, r2.xyxx
    r2.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 154: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 155: mad_sat r2.y, r2.x, l(0.300000), r6.z
    r2.y = (saturate((r2.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz))).y;
    // 156: add r3.w, -r2.y, l(1.000000)
    r3.w = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: mov_sat r0.w, cb0[10].w
    r0.w = (saturate(source[10].wwww)).w;
    // 158: mad r14.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r14.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 159: mul r4.w, r0.w, l(0.080000)
    r4.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 160: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 161: mad r14.xyz, r6.wwww, r14.xyzx, r4.wwww
    r14.xyz = ((r6.wwww)*(r14.xyzx)+(r4.wwww)).xyz;
    // 162: max r15.xyz, r3.wwww, r14.xyzx
    r15.xyz = (max(r3.wwww,r14.xyzx)).xyz;
    // 163: add r15.xyz, -r14.xyzx, r15.xyzx
    r15.xyz = ((-(r14.xyzx))+(r15.xyzx)).xyz;
    // 164: mul_sat r0.w, r14.y, l(50.000000)
    r0.w = (saturate((r14.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 165: mul r15.xyz, r0.wwww, r15.xyzx
    r15.xyz = ((r0.wwww)*(r15.xyzx)).xyz;
    // 166: add r0.w, r1.z, l(1.000000)
    r0.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: add_sat r2.x, -r0.w, r2.z
    r2.x = (saturate((-(r0.wwww))+(r2.zzzz))).x;
    // 169: sample_indexable(texture2d)(float,float,float,float) r16.xy, r2.xyxx, t3.xyzw, s5
    r16.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 170: add r0.w, r2.w, r2.x
    r0.w = ((r2.wwww)+(r2.xxxx)).w;
    // 171: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 172: mul r17.xyz, r14.xyzx, r16.yyyy
    r17.xyz = ((r14.xyzx)*(r16.yyyy)).xyz;
    // 173: mad r15.xyz, r15.xyzx, r16.xxxx, r17.xyzx
    r15.xyz = ((r15.xyzx)*(r16.xxxx)+(r17.xyzx)).xyz;
    // 174: div r2.x, l(1.000000, 1.000000, 1.000000, 1.000000), r16.y
    r2.x = r16.y != 0.f ? 1.f / r16.y : 0.f;
    // 175: add r2.x, r2.x, l(-1.000000)
    r2.x = ((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 176: mad r16.xyz, r14.xyzx, r2.xxxx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((r14.xyzx)*(r2.xxxx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: dp3 r2.x, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 178: mad r14.xyz, r2.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r14.xyz = ((r2.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 179: mad r17.xyz, -r15.xyzx, r16.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r15.xyzx))*(r16.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: mul r15.xyz, r15.xyzx, r16.xyzx
    r15.xyz = ((r15.xyzx)*(r16.xyzx)).xyz;
    // 181: mul r13.xyz, r13.xyzx, r17.xyzx
    r13.xyz = ((r13.xyzx)*(r17.xyzx)).xyz;
    // 182: mul r5.xyz, r5.xyzx, r13.xyzx
    r5.xyz = ((r5.xyzx)*(r13.xyzx)).xyz;
    // 183: mad r5.xyz, -r5.xyzx, r6.wwww, r5.xyzx
    r5.xyz = ((-(r5.xyzx))*(r6.wwww)+(r5.xyzx)).xyz;
    // 184: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 185: mul r2.x, r2.y, l(5.000000)
    r2.x = ((r2.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 186: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 187: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 188: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 189: add r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)+(r0.wwww)).w;
    // 190: mov o5.y, r2.w
    output.targets[5].y = (r2.wwww).y;
    // 191: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 192: dp3 r10.x, r10.xyzx, r1.xyzx
    r10.x = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 193: dp3 r10.y, r11.xyzx, r1.xyzx
    r10.y = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 194: dp2 r11.x, r10.xyxx, r6.xyxx
    r11.x = (dot((r10.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 195: dp2 r11.z, r10.xyxx, cb0[14].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 196: dp3 r11.y, r9.xyzx, r1.xyzx
    r11.y = (dot((r9.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 197: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r11.xyzx, t4.xyzw, s4, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r11.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 198: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 199: mul r2.xyz, r2.xyzx, cb0[13].xyzx
    r2.xyz = ((r2.xyzx)*(source[13].xyzx)).xyz;
    // 200: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 201: dp2_sat r9.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 202: dp3_sat r9.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 203: dp3_sat r9.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 204: mul r1.xyz, r9.xyzx, r9.xyzx
    r1.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 205: dp3 r1.x, r8.xyzx, r1.xyzx
    r1.x = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 206: add r1.y, -r1.x, r1.w
    r1.y = ((-(r1.xxxx))+(r1.wwww)).y;
    // 207: mad r1.x, r6.z, r1.y, r1.x
    r1.x = ((r6.zzzz)*(r1.yyyy)+(r1.xxxx)).x;
    // 208: mad r1.yzw, r7.xxyz, r1.xxxx, r4.xxyz
    r1.yzw = ((r7.xxyz)*(r1.xxxx)+(r4.xxyz)).yzw;
    // 209: mul r4.xyz, r1.xxxx, r7.xyzx
    r4.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // 210: mad r1.x, r0.w, r14.x, r14.y
    r1.x = ((r0.wwww)*(r14.xxxx)+(r14.yyyy)).x;
    // 211: mad r1.x, r1.x, r0.w, r14.z
    r1.x = ((r1.xxxx)*(r0.wwww)+(r14.zzzz)).x;
    // 212: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 213: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 214: mul r6.xyz, r0.wwww, r1.yzwy
    r6.xyz = ((r0.wwww)*(r1.yzwy)).xyz;
    // 215: add r1.xyz, r1.yzwy, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.yzwy)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 216: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 217: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 218: mul r1.xyz, r2.xyzx, r6.xyzx
    r1.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 219: mad r2.xyz, r1.xyzx, r15.xyzx, r5.xyzx
    r2.xyz = ((r1.xyzx)*(r15.xyzx)+(r5.xyzx)).xyz;
    // 220: mul r1.xyz, r15.xyzx, r1.xyzx
    r1.xyz = ((r15.xyzx)*(r1.xyzx)).xyz;
    // 221: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 222: add r1.xyz, r2.xyzx, r3.xyzx
    r1.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 223: mad o0.xyz, r0.xyzx, cb0[25].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)+(r1.xyzx)).xyz;
    // 224: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 225: dp3 r0.x, r12.xyzx, r12.xyzx
    r0.x = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 226: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 227: mul r0.xyz, r0.xxxx, r12.xyzx
    r0.xyz = ((r0.xxxx)*(r12.xyzx)).xyz;
    // 228: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 229: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 230: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 231: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 232: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 233: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 234: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 235: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 236: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 237: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 238: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 239: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
    // 240: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 241: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 242: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 243: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 244: ret
    return output;
}

// source.character.static-map-native-1119.v1 / source program f6595c514c46a140b628c791134925b1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1119(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1119(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[10];
    source[8].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[8].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[8].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[9]=g_SourceCharacterBaseConstants[11];
    source[10]=g_SourceCharacterBaseConstants[12];
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 7: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 8: mad r0.xyz, cb0[9].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 9: mul r1.xyz, cb0[5].xyzx, cb0[9].zzzz
    r1.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 10: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 11: mul r1.xyz, r0.xyzx, cb0[9].wwww
    r1.xyz = ((r0.xyzx)*(source[9].wwww)).xyz;
    // 12: mad r0.xyz, cb0[10].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[10].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 14: mul r0.w, r2.z, cb0[10].y
    r0.w = ((r2.zzzz)*(source[10].yyyy)).w;
    // 15: mul r2.xy, r2.yxyy, cb0[11].ywyy
    r2.xy = ((r2.yxyy)*(source[11].ywyy)).xy;
    // 16: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 17: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 18: mul r1.w, r1.w, cb0[10].z
    r1.w = ((r1.wwww)*(source[10].zzzz)).w;
    // 19: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 20: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 21: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 22: mul_sat r2.w, r0.w, cb2[3].w
    r2.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 23: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 24: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 25: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 26: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 27: mad r1.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 28: mad r3.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 29: mad r4.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 30: log r5.xy, |r2.xyxx|
    r5.xy = (log2(abs(r2.xyxx))).xy;
    // 31: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 32: mul r1.w, r5.y, cb0[12].x
    r1.w = ((r5.yyyy)*(source[12].xxxx)).w;
    // 33: mul r3.w, r5.x, cb0[11].z
    r3.w = ((r5.xxxx)*(source[11].zzzz)).w;
    // 34: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 35: movc r2.x, r2.x, l(0), r3.w
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).x;
    // 36: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 37: min r2.z, r2.x, l(1.000000)
    r2.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 38: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 39: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: movc r1.w, r2.y, l(0), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 41: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 42: mad r1.xyz, r3.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 43: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 44: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 46: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 47: dp2 r3.x, r2.xyxx, r2.xyxx
    r3.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 48: mul r2.xy, r2.xyxx, cb0[6].xxxx
    r2.xy = ((r2.xyxx)*(source[6].xxxx)).xy;
    // 49: mul r4.xy, r2.xyxx, v2.wwww
    r4.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 50: add r2.x, -r3.x, l(1.000000)
    r2.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 52: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 53: add r4.z, r2.x, l(0.000010)
    r4.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 54: dp3 r2.x, r4.xyzx, r4.xyzx
    r2.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 55: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 56: div r3.xyz, r4.xyzx, r2.xxxx
    r3.xyz = ((r4.xyzx)/(r2.xxxx)).xyz;
    // 57: dp3 r2.x, r3.xyzx, r3.xyzx
    r2.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 58: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 59: mul r4.xyz, r2.xxxx, r3.xyzx
    r4.xyz = ((r2.xxxx)*(r3.xyzx)).xyz;
    // 60: dp3 r2.x, v6.xyzx, v6.xyzx
    r2.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 61: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 62: mul r5.xyz, r2.xxxx, v6.xyzx
    r5.xyz = ((r2.xxxx)*(v6.xyzx)).xyz;
    // 63: dp3 r2.x, r5.xyzx, r4.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 64: mad r2.xy, r2.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 65: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 66: mul r6.xyz, r2.yyyy, cb0[24].xyzx
    r6.xyz = ((r2.yyyy)*(source[24].xyzx)).xyz;
    // 67: mad r6.xyz, r2.xxxx, cb0[23].xyzx, r6.xyzx
    r6.xyz = ((r2.xxxx)*(source[23].xyzx)+(r6.xyzx)).xyz;
    // 68: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 69: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 70: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 71: dp3 r2.x, v1.xyzx, v1.xyzx
    r2.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 72: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 73: mul r6.xyz, r2.xxxx, v1.xyzx
    r6.xyz = ((r2.xxxx)*(v1.xyzx)).xyz;
    // 74: dp3 r2.x, v0.xyzx, v0.xyzx
    r2.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 75: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 76: mul r7.xyz, r2.xxxx, v0.xyzx
    r7.xyz = ((r2.xxxx)*(v0.xyzx)).xyz;
    // 77: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 78: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 79: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 80: dp3 r9.y, r8.xyzx, r4.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 81: dp3 r9.x, r7.xyzx, r4.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 82: dp2 r10.z, r9.xyxx, cb0[14].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 83: mul r2.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 84: dp2 r10.x, r9.xyxx, r2.xyxx
    r10.x = (dot((r9.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 85: dp3 r10.y, r6.xyzx, r4.xyzx
    r10.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 86: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 87: dp4 r11.x, cb0[15].xyzw, r10.xyzw
    r11.x = (dot((source[15].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 88: dp4 r11.y, cb0[16].xyzw, r10.xyzw
    r11.y = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 89: dp4 r11.z, cb0[17].xyzw, r10.xyzw
    r11.z = (dot((source[17].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 90: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 91: dp4 r13.x, cb0[18].xyzw, r12.xyzw
    r13.x = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 92: dp4 r13.y, cb0[19].xyzw, r12.xyzw
    r13.y = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 93: dp4 r13.z, cb0[20].xyzw, r12.xyzw
    r13.z = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 94: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 95: mul r3.w, r10.y, r10.y
    r3.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 96: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 97: mad r3.w, r10.x, r10.x, -r3.w
    r3.w = ((r10.xxxx)*(r10.xxxx)+(-(r3.wwww))).w;
    // 98: mad r10.xyz, cb0[21].xyzx, r3.wwww, r11.xyzx
    r10.xyz = ((source[21].xyzx)*(r3.wwww)+(r11.xyzx)).xyz;
    // 99: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 100: mul r10.xyz, r10.xyzx, cb0[13].xyzx
    r10.xyz = ((r10.xyzx)*(source[13].xyzx)).xyz;
    // 101: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 102: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 103: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 104: mul r11.xyz, r3.wwww, v5.xyzx
    r11.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 105: dp3 r3.w, r4.xyzx, r11.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 106: mul r4.xyz, r3.wwww, r4.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 107: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 108: deriv_rtx_coarse r12.x, r3.w
    r12.x = (ddx_coarse(r3.wwww)).x;
    // 109: deriv_rty_coarse r12.y, r3.w
    r12.y = (ddy_coarse(r3.wwww)).y;
    // 110: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: dp2 r4.w, r12.xyxx, r12.xyxx
    r4.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 112: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 113: mad_sat r12.y, r4.w, l(0.300000), r2.z
    r12.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 114: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 115: add r2.z, -r12.y, l(1.000000)
    r2.z = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 116: mov_sat r0.w, cb0[10].w
    r0.w = (saturate(source[10].wwww)).w;
    // 117: mad r13.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r13.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 118: mul r4.w, r0.w, l(0.080000)
    r4.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 119: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 120: mad r13.xyz, r2.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r2.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 121: max r14.xyz, r2.zzzz, r13.xyzx
    r14.xyz = (max(r2.zzzz,r13.xyzx)).xyz;
    // 122: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 123: mul_sat r0.w, r13.y, l(50.000000)
    r0.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 124: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 125: add r0.w, r4.z, l(1.000000)
    r0.w = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: add_sat r12.x, -r0.w, r3.w
    r12.x = (saturate((-(r0.wwww))+(r3.wwww))).x;
    // 128: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t3.zwxy, s4
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 129: add r0.w, r1.w, r12.x
    r0.w = ((r1.wwww)+(r12.xxxx)).w;
    // 130: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 131: mul r15.xyz, r12.wwww, r13.xyzx
    r15.xyz = ((r12.wwww)*(r13.xyzx)).xyz;
    // 132: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 133: div r2.z, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r2.z = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 134: add r2.z, r2.z, l(-1.000000)
    r2.z = ((r2.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 135: mad r12.xzw, r13.xxyz, r2.zzzz, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r13.xxyz)*(r2.zzzz)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 136: dp3 r2.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 137: mad r13.xyz, r2.zzzz, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r2.zzzz)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 138: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 139: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 140: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 141: mul r1.xyz, r1.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 142: mad r1.xyz, -r1.xyzx, r2.wwww, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(r2.wwww)+(r1.xyzx)).xyz;
    // 143: mul r2.z, r12.y, l(5.000000)
    r2.z = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 144: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 145: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 146: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 147: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 148: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 149: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 150: dp3 r7.x, r7.xyzx, r4.xyzx
    r7.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 151: dp3 r7.y, r8.xyzx, r4.xyzx
    r7.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 152: dp2 r8.x, r7.xyxx, r2.xyxx
    r8.x = (dot((r7.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 153: dp2 r8.z, r7.xyxx, cb0[14].xyxx
    r8.z = (dot((r7.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 154: dp3 r8.y, r6.xyzx, r4.xyzx
    r8.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 155: dp3 r1.w, r5.xyzx, r4.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 156: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 157: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 158: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r8.xyzx, t4.xyzw, s3, r2.z
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r8.xyzx).xyz, (r2.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 159: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 160: mul r4.xyz, r4.xyzx, cb0[13].xyzx
    r4.xyz = ((r4.xyzx)*(source[13].xyzx)).xyz;
    // 161: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 162: mad r1.w, r0.w, r13.x, r13.y
    r1.w = ((r0.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 163: mad r1.w, r1.w, r0.w, r13.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r13.zzzz)).w;
    // 164: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 165: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 166: mul r2.yzw, r2.yyyy, cb0[24].xxyz
    r2.yzw = ((r2.yyyy)*(source[24].xxyz)).yzw;
    // 167: mad r2.xyz, cb0[23].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[23].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 168: mul r2.xyz, r2.xyzx, cb0[25].wwww
    r2.xyz = ((r2.xyzx)*(source[25].wwww)).xyz;
    // 169: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 170: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 171: mad r1.xyz, r2.xyzx, r12.xzwx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r12.xzwx)+(r1.xyzx)).xyz;
    // 172: mul r2.xyz, r12.xzwx, r2.xyzx
    r2.xyz = ((r12.xzwx)*(r2.xyzx)).xyz;
    // 173: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 174: dp3 r0.w, r3.xyzx, r11.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 175: add r1.w, -|r11.z|, l(1.000000)
    r1.w = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 178: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 179: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 180: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 181: mul r2.xyz, r1.wwww, cb0[2].xyzx
    r2.xyz = ((r1.wwww)*(source[2].xyzx)).xyz;
    // 182: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 183: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 184: mad_sat r1.w, r0.w, cb0[6].w, -cb0[7].x
    r1.w = (saturate((r0.wwww)*(source[6].wwww)+(-(source[7].xxxx)))).w;
    // 185: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 186: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 187: mul r2.w, r2.w, cb0[7].y
    r2.w = ((r2.wwww)*(source[7].yyyy)).w;
    // 188: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 189: mul r3.xyz, r2.wwww, cb0[3].xyzx
    r3.xyz = ((r2.wwww)*(source[3].xyzx)).xyz;
    // 190: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 191: add r3.xyz, r3.xyzx, -cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[3].xyzx))).xyz;
    // 192: mad r3.xyz, cb0[3].wwww, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((source[3].wwww)*(r3.xyzx)+(source[3].xyzx)).xyz;
    // 193: mul r4.xyz, cb0[4].xyzx, cb0[7].wwww
    r4.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 194: mul r4.xyz, r4.xyzx, cb0[8].zzzz
    r4.xyz = ((r4.xyzx)*(source[8].zzzz)).xyz;
    // 195: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 196: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 197: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 198: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 199: mul r4.xyz, r4.xyzx, cb0[8].wwww
    r4.xyz = ((r4.xyzx)*(source[8].wwww)).xyz;
    // 200: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 201: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 202: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 203: mad r2.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r2.xyzx)).xyz;
    // 204: add r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 205: add r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 206: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 207: mad o0.xyz, r0.xyzx, cb0[25].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)+(r2.xyzx)).xyz;
    // 208: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 209: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 210: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 211: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 212: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 213: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 214: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 215: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 216: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 217: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 218: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 219: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 220: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 221: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
    // 222: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 223: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 224: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 225: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 226: ret
    return output;
}

// source.character.static-map-native-1120.v1 / source program 7ab1ba783e1a0f418ff186a3a45ee040
#else // SOURCE_CHARACTER_BASE_DISPATCH_CASES
    case 1104u: return SourceCharacterBase1104(input);
    case 1105u: return SourceCharacterBase1105(input);
    case 1106u: return SourceCharacterBase1106(input);
    case 1107u: return SourceCharacterBase1107(input);
    case 1108u: return SourceCharacterBase1108(input);
    case 1109u: return SourceCharacterBase1109(input);
    case 1110u: return SourceCharacterBase1110(input);
    case 1111u: return SourceCharacterBase1111(input);
    case 1112u: return SourceCharacterBase1112(input);
    case 1113u: return SourceCharacterBase1113(input);
    case 1114u: return SourceCharacterBase1114(input);
    case 1115u: return SourceCharacterBase1115(input);
    case 1116u: return SourceCharacterBase1116(input);
    case 1117u: return SourceCharacterBase1117(input);
    case 1118u: return SourceCharacterBase1118(input);
    case 1119u: return SourceCharacterBase1119(input);
#endif // SOURCE_CHARACTER_BASE_DISPATCH_CASES
