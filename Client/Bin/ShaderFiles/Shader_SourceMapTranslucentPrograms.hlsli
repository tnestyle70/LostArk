// Native Bern translucent, additive and sky material programs.
#ifndef LOSTARK_SOURCE_MAP_TRANSLUCENT_PROGRAMS
#define LOSTARK_SOURCE_MAP_TRANSLUCENT_PROGRAMS

// 358c2856c67987448932b6a9de9de9ff
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent44Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;source[7]=0.f;source[8]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // mul r0.xyz, r0.xyzx, cb0[4].xxzx
    r0.xyz = ((r0.xyzx)*(source[4].xxzx)).xyz;
    // mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // div r0.xyw, r1.xyxz, r0.xxxx
    r0.xyw = ((r1.xyxz)/(r0.xxxx)).xyw;
    // dp3 r1.x, r0.xywx, r0.xywx
    r1.x = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r0.xyw, r0.xyxw, r1.xxxx
    r0.xyw = ((r0.xyxw)*(r1.xxxx)).xyw;
    // dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v6.xyzx
    r1.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // dp3 r1.z, r0.xywx, r1.xyzx
    r1.z = (dot((r0.xywx).xyz,(r1.xyzx).xyz).xxxx).z;
    // mul r1.zw, r0.xxxy, r1.zzzz
    r1.zw = ((r0.xxxy)*(r1.zzzz)).zw;
    // mad r1.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), -r1.xyxx
    r1.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r1.xyxx))).xy;
    // add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r1.xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // mad r1.xy, r1.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r1.xy = ((r1.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // mad r1.xyz, cb0[4].yyyy, r1.xyzx, r1.xyzx
    r1.xyz = ((source[4].yyyy)*(r1.xyzx)+(r1.xyzx)).xyz;
    // add r1.xyz, r1.xyzx, -cb0[4].yyyy
    r1.xyz = ((r1.xyzx)+(-(source[4].yyyy))).xyz;
    // mov_sat r2.xyz, -r1.xyzx
    r2.xyz = (saturate(-(r1.xyzx))).xyz;
    // mov_sat r1.xyz, r1.xyzx
    r1.xyz = (saturate(r1.xyzx)).xyz;
    // mad r2.xyz, -r0.zzzz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r0.zzzz))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r3.xyz, cb0[3].xyzx, cb0[4].wwww
    r3.xyz = ((source[3].xyzx)*(source[4].wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // mul_sat r1.w, r4.w, cb0[5].z
    r1.w = (saturate((r4.wwww)*(source[5].zzzz))).w;
    // mul o0.w, r1.w, cb0[0].x
    output.targets[0].w = ((r1.wwww)*(source[0].xxxx)).w;
    // mad r1.xyz, r0.zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r2.xyz, r0.zzzz, v7.xyzx
    r2.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // dp3 r0.z, r2.xyzx, r0.xywx
    r0.z = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // mad r2.xy, r0.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // mul r2.yzw, r2.yyyy, cb0[7].xxyz
    r2.yzw = ((r2.yyyy)*(source[7].xxyz)).yzw;
    // mad r2.xyz, r2.xxxx, cb0[6].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[6].xyzx)+(r2.yzwy)).xyz;
    // mul r2.xyz, r2.xyzx, cb0[8].wwww
    r2.xyz = ((r2.xyzx)*(source[8].wwww)).xyz;
    // mad r3.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r2.xyz, r1.xyzx, cb0[8].xyzx, r3.xyzx
    r2.xyz = ((r1.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.z, v1.xyzx, v1.xyzx
    r0.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r1.xyz, r0.zzzz, v1.xyzx
    r1.xyz = ((r0.zzzz)*(v1.xyzx)).xyz;
    // dp3 r0.z, v0.xyzx, v0.xyzx
    r0.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r2.xyz, r0.zzzz, v0.xyzx
    r2.xyz = ((r0.zzzz)*(v0.xyzx)).xyz;
    // mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r1.z, r1.xyzx, r0.xywx
    r1.z = (dot((r1.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // dp3 r1.x, r2.xyzx, r0.xywx
    r1.x = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.y, r2.xyzx, r0.xywx
    r1.y = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).y;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 11c859433b7f0940804c4bacfd49cec0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent44Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[7]=0.f;source[8]=0.f;source[9]=float4(0.f,0.f,0.f,0.f);
    source[10]=g_LightmapAverageScale;source[11]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r0.w, r1.z, cb0[5].z
    r0.w = ((r1.zzzz)*(source[5].zzzz)).w;
    // dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // mul r1.xy, r1.xyxx, cb0[5].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // dp3 r1.w, r1.xyzx, r0.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // mul r2.xyz, r1.wwww, r1.xyzx
    r2.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // mad r0.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // add r2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // mad r2.xy, r2.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // mad r2.xyz, cb0[5].yyyy, r2.xyzx, r2.xyzx
    r2.xyz = ((source[5].yyyy)*(r2.xyzx)+(r2.xyzx)).xyz;
    // add r2.xyz, r2.xyzx, -cb0[5].yyyy
    r2.xyz = ((r2.xyzx)+(-(source[5].yyyy))).xyz;
    // mov_sat r3.xyz, -r2.xyzx
    r3.xyz = (saturate(-(r2.xyzx))).xyz;
    // mov_sat r2.xyz, r2.xyzx
    r2.xyz = (saturate(r2.xyzx)).xyz;
    // mad r3.xyz, -r0.wwww, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r0.wwww))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r4.xyz, cb0[3].xyzx, cb0[5].wwww
    r4.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r2.xyz, r0.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // max r2.xyz, r2.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r2.xyz, r2.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // mul r3.yzw, r3.yyyy, cb0[8].xxyz
    r3.yzw = ((r3.yyyy)*(source[8].xxyz)).yzw;
    // mad r3.xyz, r3.xxxx, cb0[7].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[7].xyzx)+(r3.yzwy)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[9].wwww
    r3.xyz = ((r3.xyzx)*(source[9].wwww)).xyz;
    // mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // dp2_sat r6.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r6.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r6.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t4.xyzw, s3
    r7.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r7.xyz, r7.xyzx, cb0[11].xyzx
    r7.xyz = ((r7.xyzx)*(source[11].xyzx)).xyz;
    // dp3 r0.w, r7.xyzx, r6.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t3.xyzw, s3
    r6.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r6.xyzx)*(source[10].xyzx)).xyz;
    // mul r8.xyz, r0.wwww, r6.xyzx
    r8.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // mad r3.xyz, r6.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r3.xyz, r8.xyzx, r3.xyzx
    r3.xyz = ((r8.xyzx)/(r3.xyzx)).xyz;
    // mad r4.xyz, r2.xyzx, r8.xyzx, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r8.xyzx)+(r4.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // dp2_sat r3.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r3.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r3.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // log r0.xyz, r3.xyzx
    r0.xyz = (log2(r3.xyzx)).xyz;
    // add r1.w, cb0[6].y, l(1.000000)
    r1.w = ((source[6].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // dp3 r0.x, r7.xyzx, r0.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // mul r3.xyz, cb0[4].xyzx, cb0[6].xxxx
    r3.xyz = ((source[4].xyzx)*(source[6].xxxx)).xyz;
    // mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // mul_sat r0.y, r5.w, cb0[6].z
    r0.y = (saturate((r5.wwww)*(source[6].zzzz))).y;
    // mul o0.w, r0.y, cb0[0].x
    output.targets[0].w = ((r0.yyyy)*(source[0].xxxx)).w;
    // mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r3.xyz, r6.xyzx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // mad r4.xyz, r3.xyzx, r0.xxxx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(r0.xxxx)+(r4.xyzx)).xyz;
    // mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r0.xyz, r4.xyzx, cb0[1].xyzx
    r0.xyz = ((r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r0.xyz, r2.xyzx, cb0[9].xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(source[9].xyzx)+(r0.xyzx)).xyz;
    // mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mul o4.z, r0.w, r4.x
    output.targets[4].z = ((r0.wwww)*(r4.xxxx)).z;
    // dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// e1cb414df669fe4388b0d863864db470
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent45Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[5]=0.f;source[6]=0.f;source[7]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f;
    // add r0.x, -v2.w, cb0[4].y
    r0.x = ((-(v2.wwww))+(source[4].yyyy)).x;
    // add r0.x, r0.x, cb0[4].x
    r0.x = ((r0.xxxx)+(source[4].xxxx)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r3.xy, r3.xyxx, cb0[3].xxxx
    r3.xy = ((r3.xyxx)*(source[3].xxxx)).xy;
    // mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, cb0[2].xyzx, cb0[3].yyyy
    r5.xyz = ((source[2].xyzx)*(source[3].yyyy)).xyz;
    // mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // mul r5.yzw, r5.yyyy, cb0[6].xxyz
    r5.yzw = ((r5.yyyy)*(source[6].xxyz)).yzw;
    // mad r5.xyz, r5.xxxx, cb0[5].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[5].xyzx)+(r5.yzwy)).xyz;
    // mul r5.xyz, r5.xyzx, cb0[7].wwww
    r5.xyz = ((r5.xyzx)*(source[7].wwww)).xyz;
    // mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r5.xyz, r4.xyzx, cb0[7].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[7].xyzx)+(r5.xyzx)).xyz;
    // mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 63e66660d04ed94ebd19c0377a126a16
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent45Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;source[7]=0.f;source[8]=float4(0.f,0.f,0.f,0.f);
    source[9]=g_LightmapAverageScale;source[10]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f;
    // add r0.x, -v2.w, cb0[5].y
    r0.x = ((-(v2.wwww))+(source[5].yyyy)).x;
    // add r0.x, r0.x, cb0[5].x
    r0.x = ((r0.xxxx)+(source[5].xxxx)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r4.xyxx, cb0[4].xxxx
    r4.xy = ((r4.xyxx)*(source[4].xxxx)).xy;
    // mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // mad r3.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r6.xyz, cb0[2].xyzx, cb0[4].yyyy
    r6.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r7.xyz, cb0[3].xyzx, cb0[4].zzzz
    r7.xyz = ((source[3].xyzx)*(source[4].zzzz)).xyz;
    // mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // add r1.w, cb0[4].w, l(1.000000)
    r1.w = ((source[4].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t3.xyzw, s3
    r8.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r8.xyz, r8.xyzx, cb0[9].xyzx
    r8.xyz = ((r8.xyzx)*(source[9].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t4.xyzw, s3
    r9.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r9.xyz, r9.xyzx, cb0[10].xyzx
    r9.xyz = ((r9.xyzx)*(source[10].xyzx)).xyz;
    // dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // mul r7.xyz, r2.wwww, r6.xyzx
    r7.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // mul r9.yzw, r9.yyyy, cb0[7].xxyz
    r9.yzw = ((r9.yyyy)*(source[7].xxyz)).yzw;
    // mad r9.xyz, r9.xxxx, cb0[6].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[6].xyzx)+(r9.yzwy)).xyz;
    // mul r9.xyz, r9.xyzx, cb0[8].wwww
    r9.xyz = ((r9.xyzx)*(source[8].wwww)).xyz;
    // mul r10.xyz, r5.xyzx, r9.xyzx
    r10.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // mad r10.xyz, r5.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r5.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, r2.wwww, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // add r9.xyz, r6.xyzx, cb0[1].xyzx
    r9.xyz = ((r6.xyzx)+(source[1].xyzx)).xyz;
    // mad r9.xyz, r5.xyzx, cb0[8].xyzx, r9.xyzx
    r9.xyz = ((r5.xyzx)*(source[8].xyzx)+(r9.xyzx)).xyz;
    // mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mul o4.z, r0.y, r6.x
    output.targets[4].z = ((r0.yyyy)*(r6.xxxx)).z;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 2a6f65c31375d747ad24127d09857677
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent46Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[7]=0.f;source[8]=0.f;source[9]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r1.xyz, cb0[3].xyzx, cb0[5].yyyy
    r1.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // mul r1.xy, r1.xyxx, cb0[5].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // mul r2.yzw, r2.yyyy, cb0[8].xxyz
    r2.yzw = ((r2.yyyy)*(source[8].xxyz)).yzw;
    // mad r2.xyz, r2.xxxx, cb0[7].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[7].xyzx)+(r2.yzwy)).xyz;
    // mul r2.xyz, r2.xyzx, cb0[9].wwww
    r2.xyz = ((r2.xyzx)*(source[9].wwww)).xyz;
    // mul r3.xyz, cb0[4].xyzx, cb0[5].zzzz
    r3.xyz = ((source[4].xyzx)*(source[5].zzzz)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // mul_sat r0.w, r4.w, cb0[6].y
    r0.w = (saturate((r4.wwww)*(source[6].yyyy))).w;
    // mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mad r0.xyz, r2.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r0.xyz, r3.xyzx, cb0[9].xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(source[9].xyzx)+(r0.xyzx)).xyz;
    // mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// cf7f4c7f8f54de419147757d22accb61
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent46Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[8]=0.f;source[9]=0.f;source[10]=float4(0.f,0.f,0.f,0.f);
    source[11]=g_LightmapAverageScale;source[12]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r1.xyz, cb0[3].xyzx, cb0[6].yyyy
    r1.xyz = ((source[3].xyzx)*(source[6].yyyy)).xyz;
    // mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // mul r2.xy, r2.xyxx, cb0[6].xxxx
    r2.xy = ((r2.xyxx)*(source[6].xxxx)).xy;
    // mul r2.xy, r2.xyxx, v2.wwww
    r2.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r0.wwww, r2.xyzx
    r3.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // mad r1.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // dp2_sat r3.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r3.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r3.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // log r1.xyz, r3.xyzx
    r1.xyz = (log2(r3.xyzx)).xyz;
    // add r0.w, cb0[7].x, l(1.000000)
    r0.w = ((source[7].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t4.xyzw, s3
    r3.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((r3.xyzx)*(source[12].xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // dp2_sat r1.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r1.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r1.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r1.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r1.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r1.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // dp3 r1.x, r3.xyzx, r1.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t3.wxyz, s3
    r1.yzw = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).wxyz).yzw;
    // mul r1.yzw, r1.yyzw, cb0[11].xxyz
    r1.yzw = ((r1.yyzw)*(source[11].xxyz)).yzw;
    // mul r3.xyz, r1.xxxx, r1.yzwy
    r3.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r4.xyz, r2.wwww, v7.xyzx
    r4.xyz = ((r2.wwww)*(v7.xyzx)).xyz;
    // dp3 r2.w, r4.xyzx, r2.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // mad r4.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // mul r4.yzw, r4.yyyy, cb0[9].xxyz
    r4.yzw = ((r4.yyyy)*(source[9].xxyz)).yzw;
    // mad r4.xyz, r4.xxxx, cb0[8].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[8].xyzx)+(r4.yzwy)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[10].wwww
    r4.xyz = ((r4.xyzx)*(source[10].wwww)).xyz;
    // mul r5.xyz, cb0[4].xyzx, cb0[6].zzzz
    r5.xyz = ((source[4].xyzx)*(source[6].zzzz)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r7.xyz, r4.xyzx, r5.xyzx
    r7.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r4.xyz, r1.yzwy, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.yzwy)*(r1.xxxx)+(r4.xyzx)).xyz;
    // add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r4.xyz, r3.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)/(r4.xyzx)).xyz;
    // mad r3.xyz, r5.xyzx, r3.xyzx, r7.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r7.xyzx)).xyz;
    // dp3 r1.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // mul r4.xyz, cb0[5].xyzx, cb0[6].wwww
    r4.xyz = ((source[5].xyzx)*(source[6].wwww)).xyz;
    // mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // mul_sat r2.w, r6.w, cb0[7].y
    r2.w = (saturate((r6.wwww)*(source[7].yyyy))).w;
    // mul o0.w, r2.w, cb0[0].x
    output.targets[0].w = ((r2.wwww)*(source[0].xxxx)).w;
    // mad r4.xyz, r4.xyzx, cb2[4].wwww, cb2[4].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r1.yzw, r1.yyzw, r4.xxyz
    r1.yzw = ((r1.yyzw)*(r4.xxyz)).yzw;
    // mad r3.xyz, r1.yzwy, r0.wwww, r3.xyzx
    r3.xyz = ((r1.yzwy)*(r0.wwww)+(r3.xyzx)).xyz;
    // mul r1.yzw, r0.wwww, r1.yyzw
    r1.yzw = ((r0.wwww)*(r1.yyzw)).yzw;
    // dp3 o4.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // mad r0.xyz, r5.xyzx, cb0[10].xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(source[10].xyzx)+(r0.xyzx)).xyz;
    // mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.yzw, r0.wwww, v0.xxyz
    r1.yzw = ((r0.wwww)*(v0.xxyz)).yzw;
    // mul r4.xyz, r0.zxyz, r1.zwyz
    r4.xyz = ((r0.zxyz)*(r1.zwyz)).xyz;
    // mad r4.xyz, r0.yzxy, r1.wyzw, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r1.wyzw)+(-(r4.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // dp3 r0.x, r1.yzwy, r2.xyzx
    r0.x = (dot((r1.yzwy).xyz,(r2.xyzx).xyz).xxxx).x;
    // mul r1.yzw, r4.xxyz, v1.wwww
    r1.yzw = ((r4.xxyz)*(v1.wwww)).yzw;
    // dp3 r0.y, r1.yzwy, r2.xyzx
    r0.y = (dot((r1.yzwy).xyz,(r2.xyzx).xyz).xxxx).y;
    // dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // movc r0.xy, r0.wwww, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mul o4.z, r1.x, r3.x
    output.targets[4].z = ((r1.xxxx)*(r3.xxxx)).z;
    // dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 66d3e1fbb90e89489857bce6562d76ad
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent47Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[8].z=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f;
    // add r0.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // mad r1.x, r0.y, l(0.159155), l(0.500000)
    r1.x = ((r0.yyyy)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // add r0.y, r0.x, r0.x
    r0.y = ((r0.xxxx)+(r0.xxxx)).y;
    // log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // lt r0.z, r0.x, l(0.000000)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // movc r1.y, r0.z, l(0), r0.y
    r1.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // mul r0.yz, r1.xxyx, cb0[9].xxyx
    r0.yz = ((r1.xxyx)*(source[9].xxyx)).yz;
    // mul r0.w, cb0[8].y, cb0[8].z
    r0.w = ((source[8].yyyy)*(source[8].zzzz)).w;
    // mad r1.x, r0.w, cb0[8].w, r0.y
    r1.x = ((r0.wwww)*(source[8].wwww)+(r0.yyyy)).x;
    // mad r1.y, r0.w, cb0[9].w, r0.z
    r1.y = ((r0.wwww)*(source[9].wwww)+(r0.zzzz)).y;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t1.wxyz, s1, l(-1.000000)
    r0.yzw = ((g_SourceCharacterTexture0.SampleLevel(SourceCharacterSampler, (r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x)).wxyz).yzw;
    // add r1.x, r0.z, r0.y
    r1.x = ((r0.zzzz)+(r0.yyyy)).x;
    // add r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)+(r1.xxxx)).x;
    // mad r1.y, -r0.x, cb0[5].x, l(1.000000)
    r1.y = ((-(r0.xxxx))*(source[5].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mad r0.x, -r0.x, cb0[6].w, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[6].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // mul_sat r1.y, r1.y, cb0[6].x
    r1.y = (saturate((r1.yyyy)*(source[6].xxxx))).y;
    // add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mul r0.x, r0.x, r1.y
    r0.x = ((r0.xxxx)*(r1.yyyy)).x;
    // lt r1.y, r0.x, l(0.000001)
    r1.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // mul r0.x, r0.x, cb0[8].x
    r0.x = ((r0.xxxx)*(source[8].xxxx)).x;
    // exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // movc r0.x, r1.y, l(0), r0.x
    r0.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // mad r1.x, r1.x, l(0.333330), -r0.x
    r1.x = ((r1.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))+(-(r0.xxxx))).x;
    // mad r1.x, cb0[10].x, r1.x, r0.x
    r1.x = ((source[10].xxxx)*(r1.xxxx)+(r0.xxxx)).x;
    // mul_sat r0.x, r0.x, cb0[11].z
    r0.x = (saturate((r0.xxxx)*(source[11].zzzz))).x;
    // mul r1.x, r1.x, cb0[10].y
    r1.x = ((r1.xxxx)*(source[10].yyyy)).x;
    // dp3 r1.y, v6.xyzx, v6.xyzx
    r1.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r1.yzw, r1.yyyy, v6.xxyz
    r1.yzw = ((r1.yyyy)*(v6.xxyz)).yzw;
    // mad r1.yz, r1.yyzy, l(0.000000, -0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r1.yyzy)*(float4(0.000000,-0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // mad r1.xy, r1.yzyy, cb0[2].xyxx, r1.xxxx
    r1.xy = ((r1.yzyy)*(source[2].xyxx)+(r1.xxxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // dp3 r2.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r2.xyz, -r1.xyzx, r2.xxxx
    r2.xyz = ((-(r1.xyzx))+(r2.xxxx)).xyz;
    // mad r1.xyz, cb0[10].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[10].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // dp3 r2.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r2.xyz, -r0.yzwy, r2.xxxx
    r2.xyz = ((-(r0.yzwy))+(r2.xxxx)).xyz;
    // mad r0.yzw, cb0[11].xxxx, r2.xxyz, r0.yyzw
    r0.yzw = ((source[11].xxxx)*(r2.xxyz)+(r0.yyzw)).yzw;
    // max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // mul r0.yzw, r0.yyzw, cb0[11].yyyy
    r0.yzw = ((r0.yyzw)*(source[11].yyyy)).yzw;
    // exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // mad r0.yzw, r1.wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((r1.wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // mad r0.yzw, r0.yyzw, v2.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v2.xxyz)+(source[1].xxyz)).yzw;
    // mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // log r1.x, r0.x
    r1.x = (log2(r0.xxxx)).x;
    // lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // mul r1.x, r1.x, cb0[11].w
    r1.x = ((r1.xxxx)*(source[11].wwww)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // movc r0.x, r0.x, l(0), r1.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // div r1.xy, v7.xyxx, v7.wwww
    r1.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r1.xyxx).xy,0.f).rrrr).xyzw).x;
    // min r1.x, r1.x, l(0.999000)
    r1.x = (min(r1.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // mad r1.y, r1.x, cb2[1].z, -cb2[1].w
    r1.y = ((r1.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // mad r1.x, r1.x, cb2[1].x, cb2[1].y
    r1.x = ((r1.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).x;
    // div r1.y, l(1.000000, 1.000000, 1.000000, 1.000000), r1.y
    r1.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.yyyy)).y;
    // add r1.x, r1.y, r1.x
    r1.x = ((r1.yyyy)+(r1.xxxx)).x;
    // add r1.x, r1.x, -v7.w
    r1.x = ((r1.xxxx)+(-(v7.wwww))).x;
    // add r1.y, -cb0[12].x, l(1.000000)
    r1.y = ((-(source[12].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // max r1.y, -r1.y, l(0.001000)
    r1.y = (max(-(r1.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // mul_sat r0.x, r0.x, v2.w
    r0.x = (saturate((r0.xxxx)*(v2.wwww))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // mul o0.xyz, r0.xxxx, r0.yzwy
    output.targets[0].xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // ret
    return output;
}

// 358c2856c67987448932b6a9de9de9ff
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent48Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;source[7]=0.f;source[8]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // mul r0.xyz, r0.xyzx, cb0[4].xxzx
    r0.xyz = ((r0.xyzx)*(source[4].xxzx)).xyz;
    // mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // div r0.xyw, r1.xyxz, r0.xxxx
    r0.xyw = ((r1.xyxz)/(r0.xxxx)).xyw;
    // dp3 r1.x, r0.xywx, r0.xywx
    r1.x = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r0.xyw, r0.xyxw, r1.xxxx
    r0.xyw = ((r0.xyxw)*(r1.xxxx)).xyw;
    // dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v6.xyzx
    r1.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // dp3 r1.z, r0.xywx, r1.xyzx
    r1.z = (dot((r0.xywx).xyz,(r1.xyzx).xyz).xxxx).z;
    // mul r1.zw, r0.xxxy, r1.zzzz
    r1.zw = ((r0.xxxy)*(r1.zzzz)).zw;
    // mad r1.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), -r1.xyxx
    r1.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r1.xyxx))).xy;
    // add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r1.xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // mad r1.xy, r1.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r1.xy = ((r1.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // mad r1.xyz, cb0[4].yyyy, r1.xyzx, r1.xyzx
    r1.xyz = ((source[4].yyyy)*(r1.xyzx)+(r1.xyzx)).xyz;
    // add r1.xyz, r1.xyzx, -cb0[4].yyyy
    r1.xyz = ((r1.xyzx)+(-(source[4].yyyy))).xyz;
    // mov_sat r2.xyz, -r1.xyzx
    r2.xyz = (saturate(-(r1.xyzx))).xyz;
    // mov_sat r1.xyz, r1.xyzx
    r1.xyz = (saturate(r1.xyzx)).xyz;
    // mad r2.xyz, -r0.zzzz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r0.zzzz))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r3.xyz, cb0[3].xyzx, cb0[4].wwww
    r3.xyz = ((source[3].xyzx)*(source[4].wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // mul_sat r1.w, r4.w, cb0[5].z
    r1.w = (saturate((r4.wwww)*(source[5].zzzz))).w;
    // mul o0.w, r1.w, cb0[0].x
    output.targets[0].w = ((r1.wwww)*(source[0].xxxx)).w;
    // mad r1.xyz, r0.zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r2.xyz, r0.zzzz, v7.xyzx
    r2.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // dp3 r0.z, r2.xyzx, r0.xywx
    r0.z = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // mad r2.xy, r0.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // mul r2.yzw, r2.yyyy, cb0[7].xxyz
    r2.yzw = ((r2.yyyy)*(source[7].xxyz)).yzw;
    // mad r2.xyz, r2.xxxx, cb0[6].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[6].xyzx)+(r2.yzwy)).xyz;
    // mul r2.xyz, r2.xyzx, cb0[8].wwww
    r2.xyz = ((r2.xyzx)*(source[8].wwww)).xyz;
    // mad r3.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r2.xyz, r1.xyzx, cb0[8].xyzx, r3.xyzx
    r2.xyz = ((r1.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.z, v1.xyzx, v1.xyzx
    r0.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r1.xyz, r0.zzzz, v1.xyzx
    r1.xyz = ((r0.zzzz)*(v1.xyzx)).xyz;
    // dp3 r0.z, v0.xyzx, v0.xyzx
    r0.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r2.xyz, r0.zzzz, v0.xyzx
    r2.xyz = ((r0.zzzz)*(v0.xyzx)).xyz;
    // mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r1.z, r1.xyzx, r0.xywx
    r1.z = (dot((r1.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // dp3 r1.x, r2.xyzx, r0.xywx
    r1.x = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.y, r2.xyzx, r0.xywx
    r1.y = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).y;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// b47cefb6363be94f8cb54e0a240c947a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent48Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[7]=0.f;source[8]=0.f;source[9]=float4(0.f,0.f,0.f,0.f);
    source[10]=g_LightmapAverageScale;source[11]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r0.w, r1.z, cb0[5].z
    r0.w = ((r1.zzzz)*(source[5].zzzz)).w;
    // dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // mul r1.xy, r1.xyxx, cb0[5].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // dp3 r1.w, r1.xyzx, r0.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // mul r2.xyz, r1.wwww, r1.xyzx
    r2.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // mad r0.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // add r2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // mad r2.xy, r2.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // mad r2.xyz, cb0[5].yyyy, r2.xyzx, r2.xyzx
    r2.xyz = ((source[5].yyyy)*(r2.xyzx)+(r2.xyzx)).xyz;
    // add r2.xyz, r2.xyzx, -cb0[5].yyyy
    r2.xyz = ((r2.xyzx)+(-(source[5].yyyy))).xyz;
    // mov_sat r3.xyz, r2.xyzx
    r3.xyz = (saturate(r2.xyzx)).xyz;
    // mov_sat r2.xyz, -r2.xyzx
    r2.xyz = (saturate(-(r2.xyzx))).xyz;
    // mad r2.xyz, -r0.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r0.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r4.xyz, cb0[3].xyzx, cb0[5].wwww
    r4.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mul_sat r1.w, r5.w, cb0[6].z
    r1.w = (saturate((r5.wwww)*(source[6].zzzz))).w;
    // mul o0.w, r1.w, cb0[0].x
    output.targets[0].w = ((r1.wwww)*(source[0].xxxx)).w;
    // mad r3.xyz, r0.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // max r2.xyz, r2.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r2.xyz, r2.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // mul r3.yzw, r3.yyyy, cb0[8].xxyz
    r3.yzw = ((r3.yyyy)*(source[8].xxyz)).yzw;
    // mad r3.xyz, r3.xxxx, cb0[7].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[7].xyzx)+(r3.yzwy)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[9].wwww
    r3.xyz = ((r3.xyzx)*(source[9].wwww)).xyz;
    // mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // dp2_sat r5.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r5.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r5.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t5.xyzw, s4
    r6.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[11].xyzx
    r6.xyz = ((r6.xyzx)*(source[11].xyzx)).xyz;
    // dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t4.xyzw, s4
    r5.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r5.xyz, r5.xyzx, cb0[10].xyzx
    r5.xyz = ((r5.xyzx)*(source[10].xyzx)).xyz;
    // mul r7.xyz, r0.wwww, r5.xyzx
    r7.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // mad r3.xyz, r5.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r3.xyz, r7.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)/(r3.xyzx)).xyz;
    // mad r4.xyz, r2.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // dp2_sat r3.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r3.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r3.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // log r0.xyz, r3.xyzx
    r0.xyz = (log2(r3.xyzx)).xyz;
    // add r1.w, cb0[6].y, l(1.000000)
    r1.w = ((source[6].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // dp3 r0.x, r6.xyzx, r0.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r6.xyz, cb0[4].xyzx, cb0[6].xxxx
    r6.xyz = ((source[4].xyzx)*(source[6].xxxx)).xyz;
    // mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // mad r4.xyz, r3.xyzx, r0.xxxx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(r0.xxxx)+(r4.xyzx)).xyz;
    // mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r0.xyz, r4.xyzx, cb0[1].xyzx
    r0.xyz = ((r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r0.xyz, r2.xyzx, cb0[9].xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(source[9].xyzx)+(r0.xyzx)).xyz;
    // mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mul o4.z, r0.w, r4.x
    output.targets[4].z = ((r0.wwww)*(r4.xxxx)).z;
    // dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// c2084cc20047044da5cf38d552cbcf61
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent49Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[8]=0.f;source[9]=0.f;source[10]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f;
    // add r0.x, -v2.w, cb0[7].x
    r0.x = ((-(v2.wwww))+(source[7].xxxx)).x;
    // add r0.x, r0.x, cb0[6].w
    r0.x = ((r0.xxxx)+(source[6].wwww)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r3.xy, r3.xyxx, cb0[4].xxxx
    r3.xy = ((r3.xyxx)*(source[4].xxxx)).xy;
    // mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // mul r4.xy, v4.xyxx, cb0[4].yyyy
    r4.xy = ((v4.xyxx)*(source[4].yyyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.zw, r4.xyxx, t1.zwxy, s1, l(0.000000)
    r4.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r4.zw, r4.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r4.zw = ((r4.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r1.w, r4.zwzz, r4.zwzz
    r1.w = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r5.xy, r4.zwzz, cb0[4].zzzz
    r5.xy = ((r4.zwzz)*(source[4].zzzz)).xy;
    // max r1.w, cb0[4].w, l(0.000000)
    r1.w = (max(source[4].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.z, r3.z, r3.z
    r4.z = ((r3.zzzz)*(r3.zzzz)).z;
    // mul_sat r4.z, r4.z, r6.w
    r4.z = (saturate((r4.zzzz)*(r6.wwww))).z;
    // add r4.z, -r4.z, l(1.000000)
    r4.z = ((-(r4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.x, r7.w, r7.w
    r4.x = ((r7.wwww)*(r7.wwww)).x;
    // mul r4.x, r4.x, r4.z
    r4.x = ((r4.xxxx)*(r4.zzzz)).x;
    // mul r4.y, r1.w, r4.x
    r4.y = ((r1.wwww)*(r4.xxxx)).y;
    // mad r3.w, r3.w, r4.y, r3.w
    r3.w = ((r3.wwww)*(r4.yyyy)+(r3.wwww)).w;
    // add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // mul r4.y, r1.w, r2.w
    r4.y = ((r1.wwww)*(r2.wwww)).y;
    // mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // mad_sat r1.w, r4.x, r1.w, r4.y
    r1.w = (saturate((r4.xxxx)*(r1.wwww)+(r4.yyyy))).w;
    // mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // add r4.xyz, -r3.xyzx, r5.xyzx
    r4.xyz = ((-(r3.xyzx))+(r5.xyzx)).xyz;
    // mad r3.xyz, r2.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // mul r4.xyz, cb0[2].xyzx, cb0[5].xxxx
    r4.xyz = ((source[2].xyzx)*(source[5].xxxx)).xyz;
    // mul r5.xyz, r4.xyzx, r6.xyzx
    r5.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // mul r8.xyz, cb0[3].xyzx, cb0[5].yyyy
    r8.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r7.xyz, -r8.xyzx, r7.xyzx, r2.wwww
    r7.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r2.wwww)).xyz;
    // mad r7.xyz, cb0[5].wwww, r7.xyzx, r9.xyzx
    r7.xyz = ((source[5].wwww)*(r7.xyzx)+(r9.xyzx)).xyz;
    // mad r4.xyz, -r6.xyzx, r4.xyzx, r7.xyzx
    r4.xyz = ((-(r6.xyzx))*(r4.xyzx)+(r7.xyzx)).xyz;
    // mad r4.xyz, r1.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // mul r5.yzw, r5.yyyy, cb0[9].xxyz
    r5.yzw = ((r5.yyyy)*(source[9].xxyz)).yzw;
    // mad r5.xyz, r5.xxxx, cb0[8].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[8].xyzx)+(r5.yzwy)).xyz;
    // mul r5.xyz, r5.xyzx, cb0[10].wwww
    r5.xyz = ((r5.xyzx)*(source[10].wwww)).xyz;
    // mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r5.xyz, r4.xyzx, cb0[10].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[10].xyzx)+(r5.xyzx)).xyz;
    // mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 365fe90d3524dc4c89c47ed440270b3d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent49Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[9]=0.f;source[10]=0.f;source[11]=float4(0.f,0.f,0.f,0.f);
    source[12]=g_LightmapAverageScale;source[13]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f;
    // add r0.x, -v2.w, cb0[8].x
    r0.x = ((-(v2.wwww))+(source[8].xxxx)).x;
    // add r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)+(source[7].wwww)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r4.xyxx, cb0[5].xxxx
    r4.xy = ((r4.xyxx)*(source[5].xxxx)).xy;
    // mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // mul r5.xy, v4.xyxx, cb0[5].yyyy
    r5.xy = ((v4.xyxx)*(source[5].yyyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.xyxx, t1.zwxy, s1, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r6.xy, r5.zwzz, cb0[5].zzzz
    r6.xy = ((r5.zwzz)*(source[5].zzzz)).xy;
    // max r1.w, cb0[5].w, l(0.000000)
    r1.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.w, r4.z, r4.z
    r4.w = ((r4.zzzz)*(r4.zzzz)).w;
    // mul_sat r4.w, r4.w, r7.w
    r4.w = (saturate((r4.wwww)*(r7.wwww))).w;
    // add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // add r6.xyz, -r4.xyzx, r6.xyzx
    r6.xyz = ((-(r4.xyzx))+(r6.xyzx)).xyz;
    // mad r4.xyz, r2.wwww, r6.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r6.xyzx)+(r4.xyzx)).xyz;
    // dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // dp3 r2.w, r4.xyzx, r3.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r6.xyz, r2.wwww, r4.xyzx
    r6.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // mad r3.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // mul r6.xyz, cb0[2].xyzx, cb0[6].xxxx
    r6.xyz = ((source[2].xyzx)*(source[6].xxxx)).xyz;
    // mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // mul r9.xyz, cb0[3].xyzx, cb0[6].yyyy
    r9.xyz = ((source[3].xyzx)*(source[6].yyyy)).xyz;
    // mul r10.xyz, r5.xyzx, r9.xyzx
    r10.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r9.xyz, -r9.xyzx, r5.xyzx, r2.wwww
    r9.xyz = ((-(r9.xyzx))*(r5.xyzx)+(r2.wwww)).xyz;
    // mad r9.xyz, cb0[6].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[6].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // mad r6.xyz, -r7.xyzx, r6.xyzx, r9.xyzx
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r9.xyzx)).xyz;
    // mad r6.xyz, r1.wwww, r6.xyzx, r8.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r8.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, cb2[3].wwww, cb2[3].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r8.xyz, cb0[4].xyzx, cb0[7].xxxx
    r8.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // mad r5.xyz, cb0[7].yyyy, r5.xyzx, -r7.xyzx
    r5.xyz = ((source[7].yyyy)*(r5.xyzx)+(-(r7.xyzx))).xyz;
    // mad r5.xyz, r1.wwww, r5.xyzx, r7.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r7.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // add r1.w, cb0[7].z, l(1.000000)
    r1.w = ((source[7].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t4.xyzw, s4
    r8.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r8.xyz, r8.xyzx, cb0[12].xyzx
    r8.xyz = ((r8.xyzx)*(source[12].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t5.xyzw, s4
    r9.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r9.xyz, r9.xyzx, cb0[13].xyzx
    r9.xyz = ((r9.xyzx)*(source[13].xyzx)).xyz;
    // dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // mul r7.xyz, r2.wwww, r5.xyzx
    r7.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // mul r9.yzw, r9.yyyy, cb0[10].xxyz
    r9.yzw = ((r9.yyyy)*(source[10].xxyz)).yzw;
    // mad r9.xyz, r9.xxxx, cb0[9].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[9].xyzx)+(r9.yzwy)).xyz;
    // mul r9.xyz, r9.xyzx, cb0[11].wwww
    r9.xyz = ((r9.xyzx)*(source[11].wwww)).xyz;
    // mul r10.xyz, r6.xyzx, r9.xyzx
    r10.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // mad r10.xyz, r6.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r6.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, r2.wwww, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // add r9.xyz, r5.xyzx, cb0[1].xyzx
    r9.xyz = ((r5.xyzx)+(source[1].xyzx)).xyz;
    // mad r9.xyz, r6.xyzx, cb0[11].xyzx, r9.xyzx
    r9.xyz = ((r6.xyzx)*(source[11].xyzx)+(r9.xyzx)).xyz;
    // mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mul o4.z, r0.y, r5.x
    output.targets[4].z = ((r0.yyyy)*(r5.xxxx)).z;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r6.xyzx
    output.targets[3].xyz = (r6.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 6a80f3968aff2144a18ad6f590256629
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent50Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[9]=0.f;source[10]=0.f;source[11]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f;
    // add r0.x, -v2.w, cb0[8].w
    r0.x = ((-(v2.wwww))+(source[8].wwww)).x;
    // add r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)+(source[8].zzzz)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // mul r3.xy, v4.xyxx, cb0[2].xyxx
    r3.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.zw, r3.xyxx, t0.zwxy, s0, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r1.w, r3.zwzz, r3.zwzz
    r1.w = (dot((r3.zwzz).xy,(r3.zwzz).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r3.zw, r3.zzzw, cb0[5].wwww
    r3.zw = ((r3.zzzw)*(source[5].wwww)).zw;
    // mul r4.xy, r3.zwzz, v2.wwww
    r4.xy = ((r3.zwzz)*(v2.wwww)).xy;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // mul r3.zw, v4.xxxy, cb0[6].xxxx
    r3.zw = ((v4.xxxy)*(source[6].xxxx)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r3.zwzz, t1.xyzw, s1, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r6.xy, r5.xyxx, cb0[6].yyyy
    r6.xy = ((r5.xyxx)*(source[6].yyyy)).xy;
    // max r1.w, cb0[6].z, l(0.000000)
    r1.w = (max(source[6].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // add r4.w, -v2.x, l(1.000000)
    r4.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r3.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r3.x, r4.z, r4.z
    r3.x = ((r4.zzzz)*(r4.zzzz)).x;
    // mul_sat r3.x, r3.x, r5.w
    r3.x = (saturate((r3.xxxx)*(r5.wwww))).x;
    // add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r3.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r3.y, r7.w, r7.w
    r3.y = ((r7.wwww)*(r7.wwww)).y;
    // mul r3.x, r3.y, r3.x
    r3.x = ((r3.yyyy)*(r3.xxxx)).x;
    // mul r3.y, r1.w, r3.x
    r3.y = ((r1.wwww)*(r3.xxxx)).y;
    // mad r3.y, r4.w, r3.y, r4.w
    r3.y = ((r4.wwww)*(r3.yyyy)+(r4.wwww)).y;
    // add r1.w, -r1.w, r3.y
    r1.w = ((-(r1.wwww))+(r3.yyyy)).w;
    // mul r3.z, r1.w, r2.w
    r3.z = ((r1.wwww)*(r2.wwww)).z;
    // mad r1.w, -r2.w, r1.w, r3.y
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.yyyy)).w;
    // mad_sat r1.w, r3.x, r1.w, r3.z
    r1.w = (saturate((r3.xxxx)*(r1.wwww)+(r3.zzzz))).w;
    // mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // add r3.xyz, -r4.xyzx, r6.xyzx
    r3.xyz = ((-(r4.xyzx))+(r6.xyzx)).xyz;
    // mad r3.xyz, r2.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // mul r4.xyz, cb0[3].xyzx, cb0[6].wwww
    r4.xyz = ((source[3].xyzx)*(source[6].wwww)).xyz;
    // mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mul r8.xyz, cb0[4].xyzx, cb0[7].xxxx
    r8.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r7.xyz, -r8.xyzx, r7.xyzx, r2.wwww
    r7.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r2.wwww)).xyz;
    // mad r7.xyz, cb0[7].zzzz, r7.xyzx, r9.xyzx
    r7.xyz = ((source[7].zzzz)*(r7.xyzx)+(r9.xyzx)).xyz;
    // mad r4.xyz, -r5.xyzx, r4.xyzx, r7.xyzx
    r4.xyz = ((-(r5.xyzx))*(r4.xyzx)+(r7.xyzx)).xyz;
    // mad r4.xyz, r1.wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // mul r5.yzw, r5.yyyy, cb0[10].xxyz
    r5.yzw = ((r5.yyyy)*(source[10].xxyz)).yzw;
    // mad r5.xyz, r5.xxxx, cb0[9].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[9].xyzx)+(r5.yzwy)).xyz;
    // mul r5.xyz, r5.xyzx, cb0[11].wwww
    r5.xyz = ((r5.xyzx)*(source[11].wwww)).xyz;
    // mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r5.xyz, r4.xyzx, cb0[11].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[11].xyzx)+(r5.xyzx)).xyz;
    // mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 6d1984e064c05545b1ee436b70dec949
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent50Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[10]=0.f;source[11]=0.f;source[12]=float4(0.f,0.f,0.f,0.f);
    source[13]=g_LightmapAverageScale;source[14]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f;
    // add r0.x, -v2.w, cb0[9].w
    r0.x = ((-(v2.wwww))+(source[9].wwww)).x;
    // add r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)+(source[9].zzzz)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // mul r4.xy, v4.xyxx, cb0[2].xyxx
    r4.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.zw, r4.xyxx, t0.zwxy, s0, l(0.000000)
    r4.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r4.zw, r4.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r4.zw = ((r4.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r1.w, r4.zwzz, r4.zwzz
    r1.w = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.zw, r4.zzzw, cb0[6].wwww
    r4.zw = ((r4.zzzw)*(source[6].wwww)).zw;
    // mul r5.xy, r4.zwzz, v2.wwww
    r5.xy = ((r4.zwzz)*(v2.wwww)).xy;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r5.xyz, r5.xyzx, r1.wwww
    r5.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // mul r4.zw, v4.xxxy, cb0[7].xxxx
    r4.zw = ((v4.xxxy)*(source[7].xxxx)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r4.zwzz, t1.xyzw, s1, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r6.xyxx, r6.xyxx
    r1.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r7.xy, r6.xyxx, cb0[7].yyyy
    r7.xy = ((r6.xyxx)*(source[7].yyyy)).xy;
    // max r1.w, cb0[7].z, l(0.000000)
    r1.w = (max(source[7].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.x, r5.z, r5.z
    r4.x = ((r5.zzzz)*(r5.zzzz)).x;
    // mul_sat r4.x, r4.x, r6.w
    r4.x = (saturate((r4.xxxx)*(r6.wwww))).x;
    // add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r4.zwzz, t3.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.y, r8.w, r8.w
    r4.y = ((r8.wwww)*(r8.wwww)).y;
    // mul r4.x, r4.y, r4.x
    r4.x = ((r4.yyyy)*(r4.xxxx)).x;
    // mul r4.y, r1.w, r4.x
    r4.y = ((r1.wwww)*(r4.xxxx)).y;
    // mad r3.w, r3.w, r4.y, r3.w
    r3.w = ((r3.wwww)*(r4.yyyy)+(r3.wwww)).w;
    // add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // mul r4.y, r1.w, r2.w
    r4.y = ((r1.wwww)*(r2.wwww)).y;
    // mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // mad_sat r1.w, r4.x, r1.w, r4.y
    r1.w = (saturate((r4.xxxx)*(r1.wwww)+(r4.yyyy))).w;
    // mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // add r4.xyz, -r5.xyzx, r7.xyzx
    r4.xyz = ((-(r5.xyzx))+(r7.xyzx)).xyz;
    // mad r4.xyz, r2.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // dp3 r2.w, r4.xyzx, r3.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r5.xyz, r2.wwww, r4.xyzx
    r5.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // mad r3.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // mul r5.xyz, cb0[3].xyzx, cb0[7].wwww
    r5.xyz = ((source[3].xyzx)*(source[7].wwww)).xyz;
    // mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // mul r9.xyz, cb0[4].xyzx, cb0[8].xxxx
    r9.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // mul r10.xyz, r8.xyzx, r9.xyzx
    r10.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r9.xyz, -r9.xyzx, r8.xyzx, r2.wwww
    r9.xyz = ((-(r9.xyzx))*(r8.xyzx)+(r2.wwww)).xyz;
    // mad r9.xyz, cb0[8].zzzz, r9.xyzx, r10.xyzx
    r9.xyz = ((source[8].zzzz)*(r9.xyzx)+(r10.xyzx)).xyz;
    // mad r5.xyz, -r6.xyzx, r5.xyzx, r9.xyzx
    r5.xyz = ((-(r6.xyzx))*(r5.xyzx)+(r9.xyzx)).xyz;
    // mad r5.xyz, r1.wwww, r5.xyzx, r7.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r7.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r7.xyz, cb0[5].xyzx, cb0[8].wwww
    r7.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // mad r7.xyz, cb0[9].xxxx, r8.xyzx, -r6.xyzx
    r7.xyz = ((source[9].xxxx)*(r8.xyzx)+(-(r6.xyzx))).xyz;
    // mad r6.xyz, r1.wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((r1.wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // add r1.w, cb0[9].y, l(1.000000)
    r1.w = ((source[9].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t4.xyzw, s4
    r8.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r8.xyz, r8.xyzx, cb0[13].xyzx
    r8.xyz = ((r8.xyzx)*(source[13].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t5.xyzw, s4
    r9.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r9.xyz, r9.xyzx, cb0[14].xyzx
    r9.xyz = ((r9.xyzx)*(source[14].xyzx)).xyz;
    // dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // mul r7.xyz, r2.wwww, r6.xyzx
    r7.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // mul r9.yzw, r9.yyyy, cb0[11].xxyz
    r9.yzw = ((r9.yyyy)*(source[11].xxyz)).yzw;
    // mad r9.xyz, r9.xxxx, cb0[10].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[10].xyzx)+(r9.yzwy)).xyz;
    // mul r9.xyz, r9.xyzx, cb0[12].wwww
    r9.xyz = ((r9.xyzx)*(source[12].wwww)).xyz;
    // mul r10.xyz, r5.xyzx, r9.xyzx
    r10.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // mad r10.xyz, r5.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r5.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, r2.wwww, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // add r9.xyz, r6.xyzx, cb0[1].xyzx
    r9.xyz = ((r6.xyzx)+(source[1].xyzx)).xyz;
    // mad r9.xyz, r5.xyzx, cb0[12].xyzx, r9.xyzx
    r9.xyz = ((r5.xyzx)*(source[12].xyzx)+(r9.xyzx)).xyz;
    // mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mul o4.z, r0.y, r6.x
    output.targets[4].z = ((r0.yyyy)*(r6.xxxx)).z;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// c31833e954afaa4f8bee85fe8742c394
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent51Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;source[7]=0.f;source[8]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // mul r0.xyz, r0.xyzx, cb0[4].xxzx
    r0.xyz = ((r0.xyzx)*(source[4].xxzx)).xyz;
    // mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // div r0.xyw, r1.xyxz, r0.xxxx
    r0.xyw = ((r1.xyxz)/(r0.xxxx)).xyw;
    // dp3 r1.x, r0.xywx, r0.xywx
    r1.x = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r0.xyw, r0.xyxw, r1.xxxx
    r0.xyw = ((r0.xyxw)*(r1.xxxx)).xyw;
    // dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v6.xyzx
    r1.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // dp3 r1.z, r0.xywx, r1.xyzx
    r1.z = (dot((r0.xywx).xyz,(r1.xyzx).xyz).xxxx).z;
    // mul r1.zw, r0.xxxy, r1.zzzz
    r1.zw = ((r0.xxxy)*(r1.zzzz)).zw;
    // mad r1.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), -r1.xyxx
    r1.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r1.xyxx))).xy;
    // add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r1.xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // mad r1.xy, r1.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r1.xy = ((r1.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xyz, cb0[2].xyzx, cb0[4].yyyy
    r2.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // mul_sat r1.w, r3.w, cb0[5].y
    r1.w = (saturate((r3.wwww)*(source[5].yyyy))).w;
    // mul o0.w, r1.w, cb0[0].x
    output.targets[0].w = ((r1.wwww)*(source[0].xxxx)).w;
    // mad r1.xyz, r1.xyzx, cb0[3].xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)*(source[3].xyzx)+(-(r2.xyzx))).xyz;
    // mad r1.xyz, r0.zzzz, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r2.xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r2.xyz, r0.zzzz, v7.xyzx
    r2.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // dp3 r0.z, r2.xyzx, r0.xywx
    r0.z = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // mad r2.xy, r0.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // mul r2.yzw, r2.yyyy, cb0[7].xxyz
    r2.yzw = ((r2.yyyy)*(source[7].xxyz)).yzw;
    // mad r2.xyz, r2.xxxx, cb0[6].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[6].xyzx)+(r2.yzwy)).xyz;
    // mul r2.xyz, r2.xyzx, cb0[8].wwww
    r2.xyz = ((r2.xyzx)*(source[8].wwww)).xyz;
    // mad r3.xyz, r2.xyzx, r1.xyzx, cb0[1].xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r2.xyz, r1.xyzx, cb0[8].xyzx, r3.xyzx
    r2.xyz = ((r1.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.z, v1.xyzx, v1.xyzx
    r0.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r1.xyz, r0.zzzz, v1.xyzx
    r1.xyz = ((r0.zzzz)*(v1.xyzx)).xyz;
    // dp3 r0.z, v0.xyzx, v0.xyzx
    r0.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r2.xyz, r0.zzzz, v0.xyzx
    r2.xyz = ((r0.zzzz)*(v0.xyzx)).xyz;
    // mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r1.z, r1.xyzx, r0.xywx
    r1.z = (dot((r1.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // dp3 r1.x, r2.xyzx, r0.xywx
    r1.x = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.y, r2.xyzx, r0.xywx
    r1.y = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).y;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 476e01bb0b4db4419b8c4bfe7dadfeac
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent51Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[7]=0.f;source[8]=0.f;source[9]=float4(0.f,0.f,0.f,0.f);
    source[10]=g_LightmapAverageScale;source[11]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v7.xyzx
    r0.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r0.w, r1.z, cb0[5].z
    r0.w = ((r1.zzzz)*(source[5].zzzz)).w;
    // dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // mul r1.xy, r1.xyxx, cb0[5].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // dp3 r0.x, r0.xyzx, r1.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // mul r2.xyz, r0.yyyy, cb0[8].xyzx
    r2.xyz = ((r0.yyyy)*(source[8].xyzx)).xyz;
    // mad r0.xyz, r0.xxxx, cb0[7].xyzx, r2.xyzx
    r0.xyz = ((r0.xxxx)*(source[7].xyzx)+(r2.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, cb0[9].wwww
    r0.xyz = ((r0.xyzx)*(source[9].wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v6.xyzx
    r2.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // dp3 r1.w, r1.xyzx, r2.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r1.wwww, r1.xyzx
    r3.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // mad r2.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // add r3.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // mad r3.xy, r3.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r4.xyz, cb0[2].xyzx, cb0[5].yyyy
    r4.xyz = ((source[2].xyzx)*(source[5].yyyy)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r3.xyz, r3.xyzx, cb0[3].xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)*(source[3].xyzx)+(-(r4.xyzx))).xyz;
    // mad r3.xyz, r0.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r4.xyz, r0.xyzx, r3.xyzx
    r4.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // dp2_sat r6.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r6.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r6.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t4.xyzw, s3
    r7.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r7.xyz, r7.xyzx, cb0[11].xyzx
    r7.xyz = ((r7.xyzx)*(source[11].xyzx)).xyz;
    // dp3 r0.w, r7.xyzx, r6.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t3.xyzw, s3
    r6.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r6.xyzx)*(source[10].xyzx)).xyz;
    // mul r8.xyz, r0.wwww, r6.xyzx
    r8.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // mad r0.xyz, r6.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // add r0.xyz, r0.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r0.xyz = ((r0.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r0.xyz, r8.xyzx, r0.xyzx
    r0.xyz = ((r8.xyzx)/(r0.xyzx)).xyz;
    // mad r4.xyz, r3.xyzx, r8.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(r8.xyzx)+(r4.xyzx)).xyz;
    // dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp2_sat r8.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r8.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r8.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // log r0.yzw, r8.xxyz
    r0.yzw = (log2(r8.xxyz)).yzw;
    // add r1.w, cb0[6].x, l(1.000000)
    r1.w = ((source[6].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = ((r0.yyzw)*(r1.wwww)).yzw;
    // exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // dp3 r0.y, r7.xyzx, r0.yzwy
    r0.y = (dot((r7.xyzx).xyz,(r0.yzwy).xyz).xxxx).y;
    // mul r2.xyz, cb0[4].xyzx, cb0[5].wwww
    r2.xyz = ((source[4].xyzx)*(source[5].wwww)).xyz;
    // mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // mul_sat r0.z, r5.w, cb0[6].y
    r0.z = (saturate((r5.wwww)*(source[6].yyyy))).z;
    // mul o0.w, r0.z, cb0[0].x
    output.targets[0].w = ((r0.zzzz)*(source[0].xxxx)).w;
    // mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r2.xyz, r6.xyzx, r2.xyzx
    r2.xyz = ((r6.xyzx)*(r2.xyzx)).xyz;
    // mad r4.xyz, r2.xyzx, r0.yyyy, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r0.yyyy)+(r4.xyzx)).xyz;
    // mul r0.yzw, r0.yyyy, r2.xxyz
    r0.yzw = ((r0.yyyy)*(r2.xxyz)).yzw;
    // dp3 o4.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r0.yzw, r4.xxyz, cb0[1].xxyz
    r0.yzw = ((r4.xxyz)+(source[1].xxyz)).yzw;
    // mad r0.yzw, r3.xxyz, cb0[9].xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)*(source[9].xxyz)+(r0.yyzw)).yzw;
    // mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.wyzw, r2.yzxy
    r3.xyz = ((r0.wyzw)*(r2.yzxy)).xyz;
    // mad r3.xyz, r0.zwyz, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.zwyz)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r5.z, r0.yzwy, r1.xyzx
    r5.z = (dot((r0.yzwy).xyz,(r1.xyzx).xyz).xxxx).z;
    // dp3 r5.x, r2.xyzx, r1.xyzx
    r5.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // mul r0.yzw, r3.xxyz, v1.wwww
    r0.yzw = ((r3.xxyz)*(v1.wwww)).yzw;
    // dp3 r5.y, r0.yzwy, r1.xyzx
    r5.y = (dot((r0.yzwy).xyz,(r1.xyzx).xyz).xxxx).y;
    // dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r5.xxyz
    r0.yzw = ((r0.yyyy)*(r5.xxyz)).yzw;
    // ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).w;
    // div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // ge r1.yz, r0.yyzy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.yyzy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // mad r1.yz, -|r0.zzyz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzyz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // movc r0.yz, r1.xxxx, r1.yyzy, r0.yyzy
    r0.yz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mul o4.z, r0.x, r4.x
    output.targets[4].z = ((r0.xxxx)*(r4.xxxx)).z;
    // dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// aeaa8c3eeb66db40a737b6eeff0ffc98
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent52Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[8]=0.f;source[9]=0.f;source[10]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r1.z, r1.z, cb0[6].y
    r1.z = ((r1.zzzz)*(source[6].yyyy)).z;
    // dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // mul r1.xy, r1.xyxx, cb0[5].wwww
    r1.xy = ((r1.xyxx)*(source[5].wwww)).xy;
    // mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // div r1.xyw, r2.xyxz, r1.xxxx
    r1.xyw = ((r2.xyxz)/(r1.xxxx)).xyw;
    // dp3 r2.x, r1.xywx, r1.xywx
    r2.x = (dot((r1.xywx).xyz,(r1.xywx).xyz).xxxx).x;
    // rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // mul r1.xyw, r1.xyxw, r2.xxxx
    r1.xyw = ((r1.xyxw)*(r2.xxxx)).xyw;
    // dp3 r2.x, v6.xyzx, v6.xyzx
    r2.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // mul r2.xyz, r2.xxxx, v6.xyzx
    r2.xyz = ((r2.xxxx)*(v6.xyzx)).xyz;
    // dp3 r2.z, r1.xywx, r2.xyzx
    r2.z = (dot((r1.xywx).xyz,(r2.xyzx).xyz).xxxx).z;
    // mul r2.zw, r1.xxxy, r2.zzzz
    r2.zw = ((r1.xxxy)*(r2.zzzz)).zw;
    // mad r2.xy, r2.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), -r2.xyxx
    r2.xy = ((r2.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r2.xyxx))).xy;
    // add r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // mad r2.xy, r2.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)*(source[3].xyzx)).xyz;
    // mad r2.xyz, cb0[6].xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((source[6].xxxx)*(r2.xyzx)+(r2.xyzx)).xyz;
    // add r2.xyz, r2.xyzx, -cb0[6].xxxx
    r2.xyz = ((r2.xyzx)+(-(source[6].xxxx))).xyz;
    // mov_sat r3.xyz, r2.xyzx
    r3.xyz = (saturate(r2.xyzx)).xyz;
    // mov_sat r2.xyz, -r2.xyzx
    r2.xyz = (saturate(-(r2.xyzx))).xyz;
    // mad r2.xyz, -r1.zzzz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r1.zzzz))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r4.xyz, cb0[4].xyzx, cb0[6].zzzz
    r4.xyz = ((source[4].xyzx)*(source[6].zzzz)).xyz;
    // mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // mul_sat r0.w, r0.w, cb0[7].y
    r0.w = (saturate((r0.wwww)*(source[7].yyyy))).w;
    // mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r2.xyzx, r1.xywx
    r0.w = (dot((r2.xyzx).xyz,(r1.xywx).xyz).xxxx).w;
    // mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // mul r2.yzw, r2.yyyy, cb0[9].xxyz
    r2.yzw = ((r2.yyyy)*(source[9].xxyz)).yzw;
    // mad r2.xyz, r2.xxxx, cb0[8].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[8].xyzx)+(r2.yzwy)).xyz;
    // mul r2.xyz, r2.xyzx, cb0[10].wwww
    r2.xyz = ((r2.xyzx)*(source[10].wwww)).xyz;
    // mad r3.xyz, r2.xyzx, r0.xyzx, cb0[1].xyzx
    r3.xyz = ((r2.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r2.xyz, r0.xyzx, cb0[10].xyzx, r3.xyzx
    r2.xyz = ((r0.xyzx)*(source[10].xyzx)+(r3.xyzx)).xyz;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r1.xywx
    r0.z = (dot((r0.xyzx).xyz,(r1.xywx).xyz).xxxx).z;
    // dp3 r0.x, r2.xyzx, r1.xywx
    r0.x = (dot((r2.xyzx).xyz,(r1.xywx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r2.xyzx, r1.xywx
    r0.y = (dot((r2.xyzx).xyz,(r1.xywx).xyz).xxxx).y;
    // dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// bda4874954f02f488afa85fa1f3466b1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent52Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[9]=0.f;source[10]=0.f;source[11]=float4(0.f,0.f,0.f,0.f);
    source[12]=g_LightmapAverageScale;source[13]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r0.w, r2.z, cb0[7].y
    r0.w = ((r2.zzzz)*(source[7].yyyy)).w;
    // dp2 r2.z, r2.xyxx, r2.xyxx
    r2.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // mul r2.xy, r2.xyxx, cb0[6].wwww
    r2.xy = ((r2.xyxx)*(source[6].wwww)).xy;
    // mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // add r3.z, r2.x, l(0.000010)
    r3.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r2.x, r3.xyzx, r3.xyzx
    r2.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // div r2.xyz, r3.xyzx, r2.xxxx
    r2.xyz = ((r3.xyzx)/(r2.xxxx)).xyz;
    // dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // dp3 r2.w, r2.xyzx, r0.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r2.wwww, r2.xyzx
    r3.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // mad r0.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // add r3.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // mad r3.xy, r3.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t2.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)*(source[3].xyzx)).xyz;
    // mad r3.xyz, cb0[7].xxxx, r3.xyzx, r3.xyzx
    r3.xyz = ((source[7].xxxx)*(r3.xyzx)+(r3.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, -cb0[7].xxxx
    r3.xyz = ((r3.xyzx)+(-(source[7].xxxx))).xyz;
    // mov_sat r4.xyz, r3.xyzx
    r4.xyz = (saturate(r3.xyzx)).xyz;
    // mov_sat r3.xyz, -r3.xyzx
    r3.xyz = (saturate(-(r3.xyzx))).xyz;
    // mad r3.xyz, -r0.wwww, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r0.wwww))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r5.xyz, cb0[4].xyzx, cb0[7].zzzz
    r5.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // mul r5.xyz, r1.xyzx, r5.xyzx
    r5.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // mad r4.xyz, r0.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r4.xyz, r0.wwww, v7.xyzx
    r4.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, r2.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // mul r4.yzw, r4.yyyy, cb0[10].xxyz
    r4.yzw = ((r4.yyyy)*(source[10].xxyz)).yzw;
    // mad r4.xyz, r4.xxxx, cb0[9].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[9].xyzx)+(r4.yzwy)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[11].wwww
    r4.xyz = ((r4.xyzx)*(source[11].wwww)).xyz;
    // mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // dp2_sat r6.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r6.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r6.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t4.xyzw, s3
    r7.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r7.xyz, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((r7.xyzx)*(source[13].xyzx)).xyz;
    // dp3 r0.w, r7.xyzx, r6.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t3.xyzw, s3
    r6.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[12].xyzx
    r6.xyz = ((r6.xyzx)*(source[12].xyzx)).xyz;
    // mul r8.xyz, r0.wwww, r6.xyzx
    r8.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // mad r4.xyz, r6.xyzx, r0.wwww, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r4.xyz, r8.xyzx, r4.xyzx
    r4.xyz = ((r8.xyzx)/(r4.xyzx)).xyz;
    // mad r5.xyz, r3.xyzx, r8.xyzx, r5.xyzx
    r5.xyz = ((r3.xyzx)*(r8.xyzx)+(r5.xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // dp2_sat r4.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r4.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r4.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // log r0.xyz, r4.xyzx
    r0.xyz = (log2(r4.xyzx)).xyz;
    // add r2.w, cb0[8].x, l(1.000000)
    r2.w = ((source[8].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.xyz, r0.xyzx, r2.wwww
    r0.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // dp3 r0.x, r7.xyzx, r0.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // mul r4.xyz, cb0[5].xyzx, cb0[7].wwww
    r4.xyz = ((source[5].xyzx)*(source[7].wwww)).xyz;
    // mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // mul_sat r0.y, r1.w, cb0[8].y
    r0.y = (saturate((r1.wwww)*(source[8].yyyy))).y;
    // mul o0.w, r0.y, cb0[0].x
    output.targets[0].w = ((r0.yyyy)*(source[0].xxxx)).w;
    // mad r1.xyz, r1.xyzx, cb2[4].wwww, cb2[4].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r1.xyz, r6.xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)*(r1.xyzx)).xyz;
    // mad r4.xyz, r1.xyzx, r0.xxxx, r5.xyzx
    r4.xyz = ((r1.xyzx)*(r0.xxxx)+(r5.xyzx)).xyz;
    // mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r0.xyz, r4.xyzx, cb0[1].xyzx
    r0.xyz = ((r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r0.xyz, r3.xyzx, cb0[11].xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(source[11].xyzx)+(r0.xyzx)).xyz;
    // mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mul o4.z, r0.w, r4.x
    output.targets[4].z = ((r0.wwww)*(r4.xxxx)).z;
    // dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// c1248dd942581d45a86992aeeda41d4c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent53Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[1]=float4(1.f,1.f,1.f,1.f); // Static mesh presentation has an identity dynamic multiplier.
    source[6].y=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // mul r0.xy, v4.xyxx, cb0[7].zwzz
    r0.xy = ((v4.xyxx)*(source[7].zwzz)).xy;
    // mul r0.z, cb0[6].x, cb0[6].y
    r0.z = ((source[6].xxxx)*(source[6].yyyy)).z;
    // mad r1.x, r0.z, cb0[7].y, r0.x
    r1.x = ((r0.zzzz)*(source[7].yyyy)+(r0.xxxx)).x;
    // mad r1.y, r0.z, cb0[8].x, r0.y
    r1.y = ((r0.zzzz)*(source[8].xxxx)+(r0.yyyy)).y;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r0.xy, cb0[8].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[8].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // mul r0.w, r0.x, cb0[6].w
    r0.w = ((r0.xxxx)*(source[6].wwww)).w;
    // mad r1.x, r0.z, cb0[6].z, r0.w
    r1.x = ((r0.zzzz)*(source[6].zzzz)+(r0.wwww)).x;
    // mul r0.w, r0.z, cb0[8].w
    r0.w = ((r0.zzzz)*(source[8].wwww)).w;
    // mad r1.y, cb0[7].x, r0.y, r0.w
    r1.y = ((source[7].xxxx)*(r0.yyyy)+(r0.wwww)).y;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xy, r0.xyxx, cb0[9].yzyy
    r2.xy = ((r0.xyxx)*(source[9].yzyy)).xy;
    // add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // mad r0.zw, r0.zzzz, cb0[9].xxxw, r2.xxxy
    r0.zw = ((r0.zzzz)*(source[9].xxxw)+(r2.xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.zwzz, t4.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // mad r1.xyz, -r1.xyzx, r2.xyzx, r0.zzzz
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // mad r1.xyz, cb0[10].xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((source[10].xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, cb0[10].yyyy
    r1.xyz = ((r1.xyzx)*(source[10].yyyy)).xyz;
    // exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // mul r2.xyz, cb0[3].xyzx, cb0[3].wwww
    r2.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, v5.wwww
    r1.xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // mad r0.z, cb0[6].y, cb0[10].w, cb0[11].x
    r0.z = ((source[6].yyyy)*(source[10].wwww)+(source[11].xxxx)).z;
    // sincos r2.x, r3.x, r0.z
    r2.x = (sin(r0.zzzz)).x; r3.x = (cos(r0.zzzz)).x;
    // mov r4.x, -r2.x
    r4.x = (-(r2.xxxx)).x;
    // mov r4.y, r3.x
    r4.y = (r3.xxxx).y;
    // mov r4.z, r2.x
    r4.z = (r2.xxxx).z;
    // dp2 r0.z, r4.zyzz, r0.xyxx
    r0.z = (dot((r4.zyzz).xy,(r0.xyxx).xy).xxxx).z;
    // dp2 r0.x, r4.yxyy, r0.xyxx
    r0.x = (dot((r4.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // mul r2.z, r0.z, cb0[5].y
    r2.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // add r0.y, cb0[4].y, l(-1.000000)
    r0.y = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // mad r2.x, r0.x, cb0[5].x, r0.y
    r2.x = ((r0.xxxx)*(source[5].xxxx)+(r0.yyyy)).x;
    // add r0.xy, r2.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r2.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s4, l(0.000000)
    r0.x = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // add r0.y, -cb0[4].x, l(1.000000)
    r0.y = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // mul_sat r0.x, r0.x, cb0[11].w
    r0.x = (saturate((r0.xxxx)*(source[11].wwww))).x;
    // lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // mul r0.x, r0.x, cb0[12].x
    r0.x = ((r0.xxxx)*(source[12].xxxx)).x;
    // exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t2.yzxw, s0, l(0.000000)
    r0.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.zwzz).xy,0.f).rrrr).yzxw).z;
    // min r0.z, r0.z, l(0.999000)
    r0.z = (min(r0.zzzz,float4(0.999000,0.999000,0.999000,0.999000))).z;
    // mad r0.w, r0.z, cb2[1].z, -cb2[1].w
    r0.w = ((r0.zzzz)*(passValues[1].zzzz)+(-(passValues[1].wwww))).w;
    // mad r0.z, r0.z, cb2[1].x, cb2[1].y
    r0.z = ((r0.zzzz)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // add r0.w, -cb0[12].y, l(1.000000)
    r0.w = ((-(source[12].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r1.w, r1.w, cb0[12].z
    r1.w = ((r1.wwww)*(source[12].zzzz)).w;
    // exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // mul_sat r1.w, r1.w, cb0[12].w
    r1.w = (saturate((r1.wwww)*(source[12].wwww))).w;
    // mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // mul o0.xyz, r0.xxxx, r1.xyzx
    output.targets[0].xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // ret
    return output;
}

// 20706ac084bbce44a6cc27a9ca04625f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent54Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[5]=0.f;source[6]=0.f;source[7]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul_sat r0.w, r0.w, cb0[4].x
    r0.w = (saturate((r0.wwww)*(source[4].xxxx))).w;
    // add r1.x, -v2.w, cb0[4].z
    r1.x = ((-(v2.wwww))+(source[4].zzzz)).x;
    // add r1.x, r1.x, cb0[4].y
    r1.x = ((r1.xxxx)+(source[4].yyyy)).x;
    // add_sat r1.x, r1.x, l(1.000000)
    r1.x = (saturate((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v1.xyzx
    r1.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // mul r3.xyz, r3.xyzx, v1.wwww
    r3.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r4.xyxx, cb0[3].xxxx
    r4.xy = ((r4.xyxx)*(source[3].xxxx)).xy;
    // mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // mul r5.xyz, cb0[2].xyzx, cb0[3].yyyy
    r5.xyz = ((source[2].xyzx)*(source[3].yyyy)).xyz;
    // mul r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r5.xyzx, r4.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // mul r5.yzw, r5.yyyy, cb0[6].xxyz
    r5.yzw = ((r5.yyyy)*(source[6].xxyz)).yzw;
    // mad r5.xyz, r5.xxxx, cb0[5].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[5].xyzx)+(r5.yzwy)).xyz;
    // mul r5.xyz, r5.xyzx, cb0[7].wwww
    r5.xyz = ((r5.xyzx)*(source[7].wwww)).xyz;
    // mul r6.xyz, r0.xyzx, r5.xyzx
    r6.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, r0.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // mad r5.xyz, r0.xyzx, cb0[7].xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(source[7].xyzx)+(r5.xyzx)).xyz;
    // mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r2.x, r2.xyzx, r4.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // dp3 r2.y, r3.xyzx, r4.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // dp3 r2.z, r1.xyzx, r4.xyzx
    r2.z = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, r2.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // dp3 r1.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).w;
    // div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // ge r1.z, l(0.000000), r1.z
    r1.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).z;
    // ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // movc r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.zzzz) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 77ef7bcf1825e14092163ec67f9a89ad
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent54Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;source[7]=0.f;source[8]=float4(0.f,0.f,0.f,0.f);
    source[9]=g_LightmapAverageScale;source[10]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul_sat r0.w, r0.w, cb0[5].x
    r0.w = (saturate((r0.wwww)*(source[5].xxxx))).w;
    // add r1.x, -v2.w, cb0[5].z
    r1.x = ((-(v2.wwww))+(source[5].zzzz)).x;
    // add r1.x, r1.x, cb0[5].y
    r1.x = ((r1.xxxx)+(source[5].yyyy)).x;
    // add_sat r1.x, r1.x, l(1.000000)
    r1.x = (saturate((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v1.xyzx
    r1.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // mul r3.xyz, r3.xyzx, v1.wwww
    r3.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, v6.xyzx
    r4.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r5.xy, r5.xyxx, cb0[4].xxxx
    r5.xy = ((r5.xyxx)*(source[4].xxxx)).xy;
    // mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // dp3 r1.w, r5.xyzx, r4.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // mad r4.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // mul r6.xyz, cb0[2].xyzx, cb0[4].yyyy
    r6.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // mul r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r7.xyz, cb0[3].xyzx, cb0[4].zzzz
    r7.xyz = ((source[3].xyzx)*(source[4].zzzz)).xyz;
    // mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp2_sat r7.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // dp2_sat r8.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r8.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r8.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r4.xyz, r7.xyzx, r7.xyzx
    r4.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // add r1.w, cb0[4].w, l(1.000000)
    r1.w = ((source[4].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t3.xyzw, s3
    r8.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r8.xyz, r8.xyzx, cb0[9].xyzx
    r8.xyz = ((r8.xyzx)*(source[9].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t4.xyzw, s3
    r9.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r9.xyz, r9.xyzx, cb0[10].xyzx
    r9.xyz = ((r9.xyzx)*(source[10].xyzx)).xyz;
    // dp3 r1.w, r9.xyzx, r4.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r4.xyz, r1.wwww, r8.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // mul r7.xyz, r2.wwww, r6.xyzx
    r7.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // dp3 r3.w, r9.xyzx, r5.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // mul r9.yzw, r9.yyyy, cb0[7].xxyz
    r9.yzw = ((r9.yyyy)*(source[7].xxyz)).yzw;
    // mad r9.xyz, r9.xxxx, cb0[6].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[6].xyzx)+(r9.yzwy)).xyz;
    // mul r9.xyz, r9.xyzx, cb0[8].wwww
    r9.xyz = ((r9.xyzx)*(source[8].wwww)).xyz;
    // mul r10.xyz, r0.xyzx, r9.xyzx
    r10.xyz = ((r0.xyzx)*(r9.xyzx)).xyz;
    // mad r10.xyz, r0.xyzx, r4.xyzx, r10.xyzx
    r10.xyz = ((r0.xyzx)*(r4.xyzx)+(r10.xyzx)).xyz;
    // mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, r2.wwww, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // add r9.xyz, r6.xyzx, cb0[1].xyzx
    r9.xyz = ((r6.xyzx)+(source[1].xyzx)).xyz;
    // mad r9.xyz, r0.xyzx, cb0[8].xyzx, r9.xyzx
    r9.xyz = ((r0.xyzx)*(source[8].xyzx)+(r9.xyzx)).xyz;
    // mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r2.x, r2.xyzx, r5.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // dp3 r2.y, r3.xyzx, r5.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // dp3 r2.z, r1.xyzx, r5.xyzx
    r2.z = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, r2.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // dp3 r1.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).w;
    // div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // ge r1.z, l(0.000000), r1.z
    r1.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).z;
    // ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // movc r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.zzzz) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r1.xyz, r8.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r8.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // mul o4.z, r1.x, r6.x
    output.targets[4].z = ((r1.xxxx)*(r6.xxxx)).z;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 1bc1b38f8ff9824fbaeb04ff931f8678
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent55Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[4].y=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f;
    // mul r0.x, v4.x, cb0[4].w
    r0.x = ((v4.xxxx)*(source[4].wwww)).x;
    // mul r0.y, cb0[4].x, cb0[4].y
    r0.y = ((source[4].xxxx)*(source[4].yyyy)).y;
    // mad r1.x, r0.y, cb0[4].z, r0.x
    r1.x = ((r0.yyyy)*(source[4].zzzz)+(r0.xxxx)).x;
    // mul r0.xz, v4.yyxy, cb0[5].xxwx
    r0.xz = ((v4.yyxy)*(source[5].xxwx)).xz;
    // mad r1.yz, r0.yyyy, cb0[5].yyzy, r0.xxzx
    r1.yz = ((r0.yyyy)*(source[5].yyzy)+(r0.xxzx)).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r1.xyxx, t1.xwyz, s0, l(0.000000)
    r0.xzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // mul r1.x, v4.y, cb0[6].x
    r1.x = ((v4.yyyy)*(source[6].xxxx)).x;
    // mad r1.w, r0.y, cb0[6].y, r1.x
    r1.w = ((r0.yyyy)*(source[6].yyyy)+(r1.xxxx)).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t2.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xyz, r0.xzwx, r1.xyzx
    r2.xyz = ((r0.xzwx)*(r1.xyzx)).xyz;
    // dp3 r0.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r0.xyz, -r0.xzwx, r1.xyzx, r0.yyyy
    r0.xyz = ((-(r0.xzwx))*(r1.xyzx)+(r0.yyyy)).xyz;
    // mad r0.xyz, cb0[6].zzzz, r0.xyzx, r2.xyzx
    r0.xyz = ((source[6].zzzz)*(r0.xyzx)+(r2.xyzx)).xyz;
    // max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, cb0[6].wwww
    r0.xyz = ((r0.xyzx)*(source[6].wwww)).xyz;
    // exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, v2.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v2.xyzx)+(source[1].xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // mad r0.w, cb0[4].y, cb0[7].y, cb0[7].z
    r0.w = ((source[4].yyyy)*(source[7].yyyy)+(source[7].zzzz)).w;
    // sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
    // mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // add r1.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r1.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // dp2 r2.y, r3.zyzz, r1.yzyy
    r2.y = (dot((r3.zyzz).xy,(r1.yzyy).xy).xxxx).y;
    // dp2 r2.x, r3.yxyy, r1.yzyy
    r2.x = (dot((r3.yxyy).xy,(r1.yzyy).xy).xxxx).x;
    // mad r1.xy, r2.xyxx, cb0[3].xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(source[3].xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t0.yzwx, s2, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterLookupSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // mul_sat r0.w, r0.w, cb0[8].y
    r0.w = (saturate((r0.wwww)*(source[8].yyyy))).w;
    // log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r1.x, r1.x, cb0[8].z
    r1.x = ((r1.xxxx)*(source[8].zzzz)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // mul_sat r1.x, r1.x, v2.w
    r1.x = (saturate((r1.xxxx)*(v2.wwww))).x;
    // mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // mul o0.xyz, r0.wwww, r0.xyzx
    output.targets[0].xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // ret
    return output;
}

// e1cb414df669fe4388b0d863864db470
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent56Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[5]=0.f;source[6]=0.f;source[7]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f;
    // add r0.x, -v2.w, cb0[4].y
    r0.x = ((-(v2.wwww))+(source[4].yyyy)).x;
    // add r0.x, r0.x, cb0[4].x
    r0.x = ((r0.xxxx)+(source[4].xxxx)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r3.xy, r3.xyxx, cb0[3].xxxx
    r3.xy = ((r3.xyxx)*(source[3].xxxx)).xy;
    // mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, cb0[2].xyzx, cb0[3].yyyy
    r5.xyz = ((source[2].xyzx)*(source[3].yyyy)).xyz;
    // mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // mul r5.yzw, r5.yyyy, cb0[6].xxyz
    r5.yzw = ((r5.yyyy)*(source[6].xxyz)).yzw;
    // mad r5.xyz, r5.xxxx, cb0[5].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[5].xyzx)+(r5.yzwy)).xyz;
    // mul r5.xyz, r5.xyzx, cb0[7].wwww
    r5.xyz = ((r5.xyzx)*(source[7].wwww)).xyz;
    // mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r5.xyz, r4.xyzx, cb0[7].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[7].xyzx)+(r5.xyzx)).xyz;
    // mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 34d101656e12f1449b346b9e5ae789b3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent56Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;source[7]=0.f;source[8]=float4(0.f,0.f,0.f,0.f);
    source[9]=g_LightmapAverageScale;source[10]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f;
    // add r0.x, -v2.w, cb0[5].y
    r0.x = ((-(v2.wwww))+(source[5].yyyy)).x;
    // add r0.x, r0.x, cb0[5].x
    r0.x = ((r0.xxxx)+(source[5].xxxx)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r4.xyxx, cb0[4].xxxx
    r4.xy = ((r4.xyxx)*(source[4].xxxx)).xy;
    // mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // mad r3.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r6.xyz, cb0[2].xyzx, cb0[4].yyyy
    r6.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // mul r6.xyz, r5.xyzx, r6.xyzx
    r6.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, cb2[3].wwww, cb2[3].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r7.xyz, cb0[3].xyzx, cb0[4].zzzz
    r7.xyz = ((source[3].xyzx)*(source[4].zzzz)).xyz;
    // mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // add r1.w, cb0[4].w, l(1.000000)
    r1.w = ((source[4].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t2.xyzw, s2
    r8.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r8.xyz, r8.xyzx, cb0[9].xyzx
    r8.xyz = ((r8.xyzx)*(source[9].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t3.xyzw, s2
    r9.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r9.xyz, r9.xyzx, cb0[10].xyzx
    r9.xyz = ((r9.xyzx)*(source[10].xyzx)).xyz;
    // dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // mul r7.xyz, r2.wwww, r5.xyzx
    r7.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // mul r9.yzw, r9.yyyy, cb0[7].xxyz
    r9.yzw = ((r9.yyyy)*(source[7].xxyz)).yzw;
    // mad r9.xyz, r9.xxxx, cb0[6].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[6].xyzx)+(r9.yzwy)).xyz;
    // mul r9.xyz, r9.xyzx, cb0[8].wwww
    r9.xyz = ((r9.xyzx)*(source[8].wwww)).xyz;
    // mul r10.xyz, r6.xyzx, r9.xyzx
    r10.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // mad r10.xyz, r6.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r6.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, r2.wwww, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // add r9.xyz, r5.xyzx, cb0[1].xyzx
    r9.xyz = ((r5.xyzx)+(source[1].xyzx)).xyz;
    // mad r9.xyz, r6.xyzx, cb0[8].xyzx, r9.xyzx
    r9.xyz = ((r6.xyzx)*(source[8].xyzx)+(r9.xyzx)).xyz;
    // mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mul o4.z, r0.y, r5.x
    output.targets[4].z = ((r0.yyyy)*(r5.xxxx)).z;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r6.xyzx
    output.targets[3].xyz = (r6.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// c2084cc20047044da5cf38d552cbcf61
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent57Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[8]=0.f;source[9]=0.f;source[10]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f;
    // add r0.x, -v2.w, cb0[7].x
    r0.x = ((-(v2.wwww))+(source[7].xxxx)).x;
    // add r0.x, r0.x, cb0[6].w
    r0.x = ((r0.xxxx)+(source[6].wwww)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r3.xy, r3.xyxx, cb0[4].xxxx
    r3.xy = ((r3.xyxx)*(source[4].xxxx)).xy;
    // mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // mul r4.xy, v4.xyxx, cb0[4].yyyy
    r4.xy = ((v4.xyxx)*(source[4].yyyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.zw, r4.xyxx, t1.zwxy, s1, l(0.000000)
    r4.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r4.zw, r4.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r4.zw = ((r4.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r1.w, r4.zwzz, r4.zwzz
    r1.w = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r5.xy, r4.zwzz, cb0[4].zzzz
    r5.xy = ((r4.zwzz)*(source[4].zzzz)).xy;
    // max r1.w, cb0[4].w, l(0.000000)
    r1.w = (max(source[4].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.z, r3.z, r3.z
    r4.z = ((r3.zzzz)*(r3.zzzz)).z;
    // mul_sat r4.z, r4.z, r6.w
    r4.z = (saturate((r4.zzzz)*(r6.wwww))).z;
    // add r4.z, -r4.z, l(1.000000)
    r4.z = ((-(r4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.x, r7.w, r7.w
    r4.x = ((r7.wwww)*(r7.wwww)).x;
    // mul r4.x, r4.x, r4.z
    r4.x = ((r4.xxxx)*(r4.zzzz)).x;
    // mul r4.y, r1.w, r4.x
    r4.y = ((r1.wwww)*(r4.xxxx)).y;
    // mad r3.w, r3.w, r4.y, r3.w
    r3.w = ((r3.wwww)*(r4.yyyy)+(r3.wwww)).w;
    // add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // mul r4.y, r1.w, r2.w
    r4.y = ((r1.wwww)*(r2.wwww)).y;
    // mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // mad_sat r1.w, r4.x, r1.w, r4.y
    r1.w = (saturate((r4.xxxx)*(r1.wwww)+(r4.yyyy))).w;
    // mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // add r4.xyz, -r3.xyzx, r5.xyzx
    r4.xyz = ((-(r3.xyzx))+(r5.xyzx)).xyz;
    // mad r3.xyz, r2.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // mul r4.xyz, cb0[2].xyzx, cb0[5].xxxx
    r4.xyz = ((source[2].xyzx)*(source[5].xxxx)).xyz;
    // mul r5.xyz, r4.xyzx, r6.xyzx
    r5.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // mul r8.xyz, cb0[3].xyzx, cb0[5].yyyy
    r8.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r7.xyz, -r8.xyzx, r7.xyzx, r2.wwww
    r7.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r2.wwww)).xyz;
    // mad r7.xyz, cb0[5].wwww, r7.xyzx, r9.xyzx
    r7.xyz = ((source[5].wwww)*(r7.xyzx)+(r9.xyzx)).xyz;
    // mad r4.xyz, -r6.xyzx, r4.xyzx, r7.xyzx
    r4.xyz = ((-(r6.xyzx))*(r4.xyzx)+(r7.xyzx)).xyz;
    // mad r4.xyz, r1.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // mul r5.yzw, r5.yyyy, cb0[9].xxyz
    r5.yzw = ((r5.yyyy)*(source[9].xxyz)).yzw;
    // mad r5.xyz, r5.xxxx, cb0[8].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[8].xyzx)+(r5.yzwy)).xyz;
    // mul r5.xyz, r5.xyzx, cb0[10].wwww
    r5.xyz = ((r5.xyzx)*(source[10].wwww)).xyz;
    // mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r5.xyz, r4.xyzx, cb0[10].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[10].xyzx)+(r5.xyzx)).xyz;
    // mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 11900cd26ae69e43ab19cd6b65491197
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent57Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[9]=0.f;source[10]=0.f;source[11]=float4(0.f,0.f,0.f,0.f);
    source[12]=g_LightmapAverageScale;source[13]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f;
    // add r0.x, -v2.w, cb0[8].x
    r0.x = ((-(v2.wwww))+(source[8].xxxx)).x;
    // add r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)+(source[7].wwww)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r4.xyxx, cb0[5].xxxx
    r4.xy = ((r4.xyxx)*(source[5].xxxx)).xy;
    // mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // mul r5.xy, v4.xyxx, cb0[5].yyyy
    r5.xy = ((v4.xyxx)*(source[5].yyyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.xyxx, t1.zwxy, s1, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r6.xy, r5.zwzz, cb0[5].zzzz
    r6.xy = ((r5.zwzz)*(source[5].zzzz)).xy;
    // max r1.w, cb0[5].w, l(0.000000)
    r1.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.w, r4.z, r4.z
    r4.w = ((r4.zzzz)*(r4.zzzz)).w;
    // mul_sat r4.w, r4.w, r7.w
    r4.w = (saturate((r4.wwww)*(r7.wwww))).w;
    // add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // add r6.xyz, -r4.xyzx, r6.xyzx
    r6.xyz = ((-(r4.xyzx))+(r6.xyzx)).xyz;
    // mad r4.xyz, r2.wwww, r6.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r6.xyzx)+(r4.xyzx)).xyz;
    // dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // dp3 r2.w, r4.xyzx, r3.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r6.xyz, r2.wwww, r4.xyzx
    r6.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // mad r3.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // mul r6.xyz, cb0[2].xyzx, cb0[6].xxxx
    r6.xyz = ((source[2].xyzx)*(source[6].xxxx)).xyz;
    // mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // mul r9.xyz, cb0[3].xyzx, cb0[6].yyyy
    r9.xyz = ((source[3].xyzx)*(source[6].yyyy)).xyz;
    // mul r10.xyz, r5.xyzx, r9.xyzx
    r10.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r9.xyz, -r9.xyzx, r5.xyzx, r2.wwww
    r9.xyz = ((-(r9.xyzx))*(r5.xyzx)+(r2.wwww)).xyz;
    // mad r9.xyz, cb0[6].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[6].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // mad r6.xyz, -r7.xyzx, r6.xyzx, r9.xyzx
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r9.xyzx)).xyz;
    // mad r6.xyz, r1.wwww, r6.xyzx, r8.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r8.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, cb2[3].wwww, cb2[3].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r8.xyz, cb0[4].xyzx, cb0[7].xxxx
    r8.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // mad r5.xyz, cb0[7].yyyy, r5.xyzx, -r7.xyzx
    r5.xyz = ((source[7].yyyy)*(r5.xyzx)+(-(r7.xyzx))).xyz;
    // mad r5.xyz, r1.wwww, r5.xyzx, r7.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r7.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // add r1.w, cb0[7].z, l(1.000000)
    r1.w = ((source[7].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t5.xyzw, s5
    r8.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r8.xyz, r8.xyzx, cb0[12].xyzx
    r8.xyz = ((r8.xyzx)*(source[12].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t6.xyzw, s5
    r9.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r9.xyz, r9.xyzx, cb0[13].xyzx
    r9.xyz = ((r9.xyzx)*(source[13].xyzx)).xyz;
    // dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // mul r7.xyz, r2.wwww, r5.xyzx
    r7.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // mul r9.yzw, r9.yyyy, cb0[10].xxyz
    r9.yzw = ((r9.yyyy)*(source[10].xxyz)).yzw;
    // mad r9.xyz, r9.xxxx, cb0[9].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[9].xyzx)+(r9.yzwy)).xyz;
    // mul r9.xyz, r9.xyzx, cb0[11].wwww
    r9.xyz = ((r9.xyzx)*(source[11].wwww)).xyz;
    // mul r10.xyz, r6.xyzx, r9.xyzx
    r10.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // mad r10.xyz, r6.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r6.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, r2.wwww, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // add r9.xyz, r5.xyzx, cb0[1].xyzx
    r9.xyz = ((r5.xyzx)+(source[1].xyzx)).xyz;
    // mad r9.xyz, r6.xyzx, cb0[11].xyzx, r9.xyzx
    r9.xyz = ((r6.xyzx)*(source[11].xyzx)+(r9.xyzx)).xyz;
    // mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mul o4.z, r0.y, r5.x
    output.targets[4].z = ((r0.yyyy)*(r5.xxxx)).z;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r6.xyzx
    output.targets[3].xyz = (r6.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 388ba8a3c543114fa97c7799cb45ae5e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent58Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;source[7]=0.f;source[8]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // add r0.x, -v2.w, cb0[5].w
    r0.x = ((-(v2.wwww))+(source[5].wwww)).x;
    // add r0.x, r0.x, cb0[5].z
    r0.x = ((r0.xxxx)+(source[5].zzzz)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r4.xyxx, cb0[4].xxxx
    r4.xy = ((r4.xyxx)*(source[4].xxxx)).xy;
    // mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r7.xyz, cb0[3].xyzx, cb0[5].xxxx
    r7.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // mul r7.xyz, r6.xyzx, r7.xyzx
    r7.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // mad r7.xyz, r7.xyzx, cb2[3].wwww, cb2[3].xyzx
    r7.xyz = ((r7.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r8.xyz, cb0[2].xyzx, cb0[4].yyyy
    r8.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // mul r6.xyz, r6.xyzx, cb0[4].zzzz
    r6.xyz = ((r6.xyzx)*(source[4].zzzz)).xyz;
    // dp3_sat r1.w, r4.xyzx, r3.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx)).w;
    // mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // lt r2.w, r1.w, l(0.000001)
    r2.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // mul r1.w, r1.w, cb0[4].w
    r1.w = ((r1.wwww)*(source[4].wwww)).w;
    // exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // mad r3.xyz, r1.wwww, r6.xyzx, cb0[1].xyzx
    r3.xyz = ((r1.wwww)*(r6.xyzx)+(source[1].xyzx)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, v7.xyzx
    r4.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r4.xyzx, r5.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // mad r4.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // mul r4.yzw, r4.yyyy, cb0[7].xxyz
    r4.yzw = ((r4.yyyy)*(source[7].xxyz)).yzw;
    // mad r4.xyz, r4.xxxx, cb0[6].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[6].xyzx)+(r4.yzwy)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[8].wwww
    r4.xyz = ((r4.xyzx)*(source[8].wwww)).xyz;
    // mul r6.xyz, r7.xyzx, r4.xyzx
    r6.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // mad r3.xyz, r4.xyzx, r7.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r7.xyzx)+(r3.xyzx)).xyz;
    // mad r3.xyz, r7.xyzx, cb0[8].xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // mad o0.xyz, r3.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r5.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r5.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r7.xyzx
    output.targets[3].xyz = (r7.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 29a3a61d99ff3e4f819f7a7e8cb13d93
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent58Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;source[7]=0.f;source[8]=float4(0.f,0.f,0.f,0.f);
    source[9]=g_LightmapAverageScale;source[10]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f,r11=0.f;
    // add r0.x, -v2.w, cb0[5].w
    r0.x = ((-(v2.wwww))+(source[5].wwww)).x;
    // add r0.x, r0.x, cb0[5].z
    r0.x = ((r0.xxxx)+(source[5].zzzz)).x;
    // add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r4.xyxx, cb0[4].xxxx
    r4.xy = ((r4.xyxx)*(source[4].xxxx)).xy;
    // mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r6.xyz, r1.wwww, r5.xyzx
    r6.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r8.xyz, cb0[3].xyzx, cb0[5].xxxx
    r8.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // mul r8.xyz, r7.xyzx, r8.xyzx
    r8.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // mad r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = ((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r9.xyz, cb0[2].xyzx, cb0[4].yyyy
    r9.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // mad r9.xyz, r7.xyzx, cb2[4].wwww, cb2[4].xyzx
    r9.xyz = ((r7.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r7.xyz, r7.xyzx, cb0[4].zzzz
    r7.xyz = ((r7.xyzx)*(source[4].zzzz)).xyz;
    // dp3_sat r1.w, r4.xyzx, r3.xyzx
    r1.w = (saturate(dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx)).w;
    // mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // lt r2.w, r1.w, l(0.000001)
    r2.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // mul r1.w, r1.w, cb0[4].w
    r1.w = ((r1.wwww)*(source[4].wwww)).w;
    // exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // mad r3.xyz, r1.wwww, r7.xyzx, cb0[1].xyzx
    r3.xyz = ((r1.wwww)*(r7.xyzx)+(source[1].xyzx)).xyz;
    // dp2_sat r4.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r4.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r4.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // dp2_sat r7.x, r6.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r6.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r6.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r6.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r6.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r6.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // add r1.w, cb0[5].y, l(1.000000)
    r1.w = ((source[5].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // log r6.xyz, r7.xyzx
    r6.xyz = (log2(r7.xyzx)).xyz;
    // mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t2.xyzw, s2
    r7.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r7.xyz, r7.xyzx, cb0[9].xyzx
    r7.xyz = ((r7.xyzx)*(source[9].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t3.xyzw, s2
    r10.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r10.xyz, r10.xyzx, cb0[10].xyzx
    r10.xyz = ((r10.xyzx)*(source[10].xyzx)).xyz;
    // dp3 r1.w, r10.xyzx, r4.xyzx
    r1.w = (dot((r10.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r4.xyz, r1.wwww, r7.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // mul r9.xyz, r7.xyzx, r9.xyzx
    r9.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // dp3 r2.w, r10.xyzx, r6.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // mul r6.xyz, r2.wwww, r9.xyzx
    r6.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r10.xyz, r3.wwww, v7.xyzx
    r10.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // dp3 r3.w, r10.xyzx, r5.xyzx
    r3.w = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // mad r10.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r10.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r10.xy, r10.xyxx, r10.xyxx
    r10.xy = ((r10.xyxx)*(r10.xyxx)).xy;
    // mul r10.yzw, r10.yyyy, cb0[7].xxyz
    r10.yzw = ((r10.yyyy)*(source[7].xxyz)).yzw;
    // mad r10.xyz, r10.xxxx, cb0[6].xyzx, r10.yzwy
    r10.xyz = ((r10.xxxx)*(source[6].xyzx)+(r10.yzwy)).xyz;
    // mul r10.xyz, r10.xyzx, cb0[8].wwww
    r10.xyz = ((r10.xyzx)*(source[8].wwww)).xyz;
    // mul r11.xyz, r8.xyzx, r10.xyzx
    r11.xyz = ((r8.xyzx)*(r10.xyzx)).xyz;
    // mad r11.xyz, r8.xyzx, r4.xyzx, r11.xyzx
    r11.xyz = ((r8.xyzx)*(r4.xyzx)+(r11.xyzx)).xyz;
    // mad r7.xyz, r7.xyzx, r1.wwww, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r1.wwww)+(r10.xyzx)).xyz;
    // mad r9.xyz, r9.xyzx, r2.wwww, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)+(r9.xyzx)).xyz;
    // mad r3.xyz, r8.xyzx, cb0[8].xyzx, r3.xyzx
    r3.xyz = ((r8.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // mad o0.xyz, r3.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r5.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r5.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r0.yzw, r7.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r7.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // div r0.yzw, r4.xxyz, r0.yyzw
    r0.yzw = ((r4.xxyz)/(r0.yyzw)).yzw;
    // dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mul o4.z, r0.y, r9.x
    output.targets[4].z = ((r0.yyyy)*(r9.xxxx)).z;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r8.xyzx
    output.targets[3].xyz = (r8.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// fff4f25164212f47a9c7d39bbaf4148b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent59Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[11]=0.f;source[12]=0.f;source[13]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s0, l(0.000000)
    r0.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.yzyy).xy,0.f).rrrr).yxzw).y;
    // min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // mad r0.z, r0.y, cb2[1].x, cb2[1].y
    r0.z = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // mad r0.y, r0.y, cb2[1].z, -cb2[1].w
    r0.y = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // add r0.z, -cb0[9].y, l(1.000000)
    r0.z = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // add r0.y, r0.y, -v8.w
    r0.y = ((r0.yyyy)+(-(v8.wwww))).y;
    // max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r3.xyxx, cb0[7].xxxx
    r4.xy = ((r3.xyxx)*(source[7].xxxx)).xy;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, r4.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // add r5.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r5.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // dp2 r6.x, cb0[2].xyxx, r5.xyxx
    r6.x = (dot((source[2].xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // dp2 r6.y, cb0[3].xyxx, r5.xyxx
    r6.y = (dot((source[3].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // add r5.xy, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r5.xy, r5.xyxx, cb0[4].xyxx
    r5.xy = ((r5.xyxx)*(source[4].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r6.xyz, -r5.xyzx, r1.wwww
    r6.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // mad r5.xyz, cb0[8].yyyy, r6.xyzx, r5.xyzx
    r5.xyz = ((source[8].yyyy)*(r6.xyzx)+(r5.xyzx)).xyz;
    // mul r6.xyz, r5.xyzx, cb0[8].zzzz
    r6.xyz = ((r5.xyzx)*(source[8].zzzz)).xyz;
    // mul r6.xyz, r6.xyzx, cb0[5].xyzx
    r6.xyz = ((r6.xyzx)*(source[5].xyzx)).xyz;
    // mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r5.xyz, r5.xyzx, cb0[8].wwww
    r5.xyz = ((r5.xyzx)*(source[8].wwww)).xyz;
    // mul r5.xyz, r5.xyzx, cb0[6].xyzx
    r5.xyz = ((r5.xyzx)*(source[6].xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r6.xyz, r1.wwww, v7.xyzx
    r6.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r6.xyzx, r3.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mad r6.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // mul r6.yzw, r6.yyyy, cb0[12].xxyz
    r6.yzw = ((r6.yyyy)*(source[12].xxyz)).yzw;
    // mad r6.xyz, r6.xxxx, cb0[11].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[11].xyzx)+(r6.yzwy)).xyz;
    // mul r6.xyz, r6.xyzx, cb0[13].wwww
    r6.xyz = ((r6.xyzx)*(source[13].wwww)).xyz;
    // mul r7.xyz, r4.xyzx, r6.xyzx
    r7.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, r4.xyzx, cb0[1].xyzx
    r6.xyz = ((r6.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r6.xyz, r4.xyzx, cb0[13].xyzx, r6.xyzx
    r6.xyz = ((r4.xyzx)*(source[13].xyzx)+(r6.xyzx)).xyz;
    // mad o0.xyz, r6.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r6.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul_sat o3.w, cb0[9].x, l(0.002000)
    output.targets[3].w = (saturate((source[9].xxxx)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // dp3 o4.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // ftou r0.y, cb0[10].z
    r0.y = (asfloat((uint4)(source[10].zzzz))).y;
    // bfi r0.y, l(5), l(0), r0.y, l(32)
    r0.y = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.yyyy),uint4(32u,32u,32u,32u))).y;
    // utof r0.y, r0.y
    r0.y = ((float4)(asuint(r0.yyyy))).y;
    // mul o5.w, r0.y, l(0.003922)
    output.targets[5].w = ((r0.yyyy)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // mul_sat r0.yzw, r5.xxyz, l(0.000000, 0.100000, 0.100000, 0.100000)
    r0.yzw = (saturate((r5.xxyz)*(float4(0.000000,0.100000,0.100000,0.100000)))).yzw;
    // sqrt o5.xyz, r0.yzwy
    output.targets[5].xyz = (sqrt(r0.yzwy)).xyz;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // ret
    return output;
}

// b743d80328550249bd5f49dab27e51cd
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent59Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[11]=0.f;source[12]=0.f;source[13]=float4(0.f,0.f,0.f,0.f);
    source[14]=g_LightmapAverageScale;source[15]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f,r11=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s0, l(0.000000)
    r0.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.yzyy).xy,0.f).rrrr).yxzw).y;
    // min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // mad r0.z, r0.y, cb2[1].x, cb2[1].y
    r0.z = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // mad r0.y, r0.y, cb2[1].z, -cb2[1].w
    r0.y = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // add r0.z, -cb0[9].y, l(1.000000)
    r0.z = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // add r0.y, r0.y, -v8.w
    r0.y = ((r0.yyyy)+(-(v8.wwww))).y;
    // max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r5.xy, r4.xyxx, cb0[7].xxxx
    r5.xy = ((r4.xyxx)*(source[7].xxxx)).xy;
    // dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, r5.xyzx
    r4.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // mad r3.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // dp2 r7.x, cb0[2].xyxx, r6.xyxx
    r7.x = (dot((source[2].xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // dp2 r7.y, cb0[3].xyxx, r6.xyxx
    r7.y = (dot((source[3].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // add r6.xy, r7.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r7.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r6.xy, r6.xyxx, cb0[4].xyxx
    r6.xy = ((r6.xyxx)*(source[4].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // mad r6.xyz, cb0[8].yyyy, r7.xyzx, r6.xyzx
    r6.xyz = ((source[8].yyyy)*(r7.xyzx)+(r6.xyzx)).xyz;
    // mul r7.xyz, r6.xyzx, cb0[8].zzzz
    r7.xyz = ((r6.xyzx)*(source[8].zzzz)).xyz;
    // mul r7.xyz, r7.xyzx, cb0[5].xyzx
    r7.xyz = ((r7.xyzx)*(source[5].xyzx)).xyz;
    // mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r6.xyz, r6.xyzx, cb0[8].wwww
    r6.xyz = ((r6.xyzx)*(source[8].wwww)).xyz;
    // mul r6.xyz, r6.xyzx, cb0[6].xyzx
    r6.xyz = ((r6.xyzx)*(source[6].xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // add r1.w, cb0[9].x, l(1.000000)
    r1.w = ((source[9].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t4.xyzw, s4
    r8.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r8.xyz, r8.xyzx, cb0[14].xyzx
    r8.xyz = ((r8.xyzx)*(source[14].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t5.xyzw, s4
    r9.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r9.xyz, r9.xyzx, cb0[15].xyzx
    r9.xyz = ((r9.xyzx)*(source[15].xyzx)).xyz;
    // dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // mul r10.xyz, r6.xyzx, r8.xyzx
    r10.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // mul r7.xyz, r2.wwww, r10.xyzx
    r7.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // mul r9.yzw, r9.yyyy, cb0[12].xxyz
    r9.yzw = ((r9.yyyy)*(source[12].xxyz)).yzw;
    // mad r9.xyz, r9.xxxx, cb0[11].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[11].xyzx)+(r9.yzwy)).xyz;
    // mul r9.xyz, r9.xyzx, cb0[13].wwww
    r9.xyz = ((r9.xyzx)*(source[13].wwww)).xyz;
    // mul r11.xyz, r5.xyzx, r9.xyzx
    r11.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // mad r11.xyz, r5.xyzx, r3.xyzx, r11.xyzx
    r11.xyz = ((r5.xyzx)*(r3.xyzx)+(r11.xyzx)).xyz;
    // mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // mad r9.xyz, r10.xyzx, r2.wwww, r11.xyzx
    r9.xyz = ((r10.xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // add r10.xyz, r9.xyzx, cb0[1].xyzx
    r10.xyz = ((r9.xyzx)+(source[1].xyzx)).xyz;
    // mad r10.xyz, r5.xyzx, cb0[13].xyzx, r10.xyzx
    r10.xyz = ((r5.xyzx)*(source[13].xyzx)+(r10.xyzx)).xyz;
    // mad o0.xyz, r10.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r10.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul_sat o3.w, cb0[9].x, l(0.002000)
    output.targets[3].w = (saturate((source[9].xxxx)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mul o4.z, r0.y, r9.x
    output.targets[4].z = ((r0.yyyy)*(r9.xxxx)).z;
    // ftou r0.y, cb0[10].z
    r0.y = (asfloat((uint4)(source[10].zzzz))).y;
    // bfi r0.y, l(5), l(0), r0.y, l(32)
    r0.y = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.yyyy),uint4(32u,32u,32u,32u))).y;
    // utof r0.y, r0.y
    r0.y = ((float4)(asuint(r0.yyyy))).y;
    // mul o5.w, r0.y, l(0.003922)
    output.targets[5].w = ((r0.yyyy)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // mul_sat r0.yzw, r6.xxyz, l(0.000000, 0.100000, 0.100000, 0.100000)
    r0.yzw = (saturate((r6.xxyz)*(float4(0.000000,0.100000,0.100000,0.100000)))).yzw;
    // sqrt o5.xyz, r0.yzwy
    output.targets[5].xyz = (sqrt(r0.yzwy)).xyz;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // ret
    return output;
}

// a123e9e92c69c74899f21cc4ce34b9d4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent60Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[7]=0.f;source[8]=0.f;source[9]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul_sat r0.w, r0.w, cb0[5].w
    r0.w = (saturate((r0.wwww)*(source[5].wwww))).w;
    // add r1.x, -v2.w, cb0[6].y
    r1.x = ((-(v2.wwww))+(source[6].yyyy)).x;
    // add r1.x, r1.x, cb0[6].x
    r1.x = ((r1.xxxx)+(source[6].xxxx)).x;
    // add_sat r1.x, r1.x, l(1.000000)
    r1.x = (saturate((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // mul r0.w, r0.w, cb0[0].w
    r0.w = ((r0.wwww)*(source[0].wwww)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v1.xyzx
    r1.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // mul r3.xyz, r3.xyzx, v1.wwww
    r3.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, v6.xyzx
    r4.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r5.xy, r5.xyxx, cb0[4].xxxx
    r5.xy = ((r5.xyxx)*(source[4].xxxx)).xy;
    // mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r5.xyw, r6.xyxz, r1.wwww
    r5.xyw = ((r6.xyxz)/(r1.wwww)).xyw;
    // dp3 r1.w, r5.xywx, r5.xywx
    r1.w = (dot((r5.xywx).xyz,(r5.xywx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyw, r1.wwww, r5.xyxw
    r5.xyw = ((r1.wwww)*(r5.xyxw)).xyw;
    // dp3 r1.w, r5.xywx, r4.xyzx
    r1.w = (dot((r5.xywx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r6.xyz, r1.wwww, r5.xywx
    r6.xyz = ((r1.wwww)*(r5.xywx)).xyz;
    // mad r4.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // dp3 r6.x, r2.xyzx, r4.xyzx
    r6.x = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // dp3 r6.y, r3.xyzx, r4.xyzx
    r6.y = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // mul r4.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r4.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // mad r4.xy, cb0[4].yyyy, r6.xyxx, r4.xyxx
    r4.xy = ((source[4].yyyy)*(r6.xyxx)+(r4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t2.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r4.xyz, r4.xyzx, cb0[2].xyzx
    r4.xyz = ((r4.xyzx)*(source[2].xyzx)).xyz;
    // mad r4.xyz, cb0[4].zzzz, r4.xyzx, r4.xyzx
    r4.xyz = ((source[4].zzzz)*(r4.xyzx)+(r4.xyzx)).xyz;
    // add r4.xyz, r4.xyzx, -cb0[4].zzzz
    r4.xyz = ((r4.xyzx)+(-(source[4].zzzz))).xyz;
    // mov_sat r6.xyz, r4.xyzx
    r6.xyz = (saturate(r4.xyzx)).xyz;
    // mul r1.w, r5.z, cb0[4].w
    r1.w = ((r5.zzzz)*(source[4].wwww)).w;
    // mul r7.xyz, cb0[3].xyzx, cb0[5].xxxx
    r7.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // mul r0.xyz, r0.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r7.xyzx)).xyz;
    // mad r0.xyz, r1.wwww, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r6.xyzx)+(r0.xyzx)).xyz;
    // mov_sat r4.xyz, -r4.xyzx
    r4.xyz = (saturate(-(r4.xyzx))).xyz;
    // mad r4.xyz, -r1.wwww, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.wwww))*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, v7.xyzx
    r4.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r4.xyzx, r5.xywx
    r1.w = (dot((r4.xyzx).xyz,(r5.xywx).xyz).xxxx).w;
    // mad r4.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // mul r4.yzw, r4.yyyy, cb0[8].xxyz
    r4.yzw = ((r4.yyyy)*(source[8].xxyz)).yzw;
    // mad r4.xyz, r4.xxxx, cb0[7].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[7].xyzx)+(r4.yzwy)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[9].wwww
    r4.xyz = ((r4.xyzx)*(source[9].wwww)).xyz;
    // mul r6.xyz, r0.xyzx, r4.xyzx
    r6.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, r0.xyzx, cb0[1].xyzx
    r4.xyz = ((r4.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // mad r4.xyz, r0.xyzx, cb0[9].xyzx, r4.xyzx
    r4.xyz = ((r0.xyzx)*(source[9].xyzx)+(r4.xyzx)).xyz;
    // mad o0.xyz, r4.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r2.x, r2.xyzx, r5.xywx
    r2.x = (dot((r2.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // dp3 r2.y, r3.xyzx, r5.xywx
    r2.y = (dot((r3.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // dp3 r2.z, r1.xyzx, r5.xywx
    r2.z = (dot((r1.xyzx).xyz,(r5.xywx).xyz).xxxx).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, r2.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // dp3 r1.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).w;
    // div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // ge r1.z, l(0.000000), r1.z
    r1.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).z;
    // ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // movc r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.zzzz) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// b79501cee1248946b8560201461f9a20
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent60Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[8]=0.f;source[9]=0.f;source[10]=float4(0.f,0.f,0.f,0.f);
    source[11]=g_LightmapAverageScale;source[12]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul_sat r0.w, r0.w, cb0[6].w
    r0.w = (saturate((r0.wwww)*(source[6].wwww))).w;
    // add r1.x, -v2.w, cb0[7].y
    r1.x = ((-(v2.wwww))+(source[7].yyyy)).x;
    // add r1.x, r1.x, cb0[7].x
    r1.x = ((r1.xxxx)+(source[7].xxxx)).x;
    // add_sat r1.x, r1.x, l(1.000000)
    r1.x = (saturate((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // mul r0.w, r0.w, cb0[0].w
    r0.w = ((r0.wwww)*(source[0].wwww)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v1.xyzx
    r1.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // mul r3.xyz, r3.xyzx, v1.wwww
    r3.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, v6.xyzx
    r4.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r5.xy, r5.xyxx, cb0[5].xxxx
    r5.xy = ((r5.xyxx)*(source[5].xxxx)).xy;
    // mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r5.xyw, r6.xyxz, r1.wwww
    r5.xyw = ((r6.xyxz)/(r1.wwww)).xyw;
    // dp3 r1.w, r5.xywx, r5.xywx
    r1.w = (dot((r5.xywx).xyz,(r5.xywx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyw, r1.wwww, r5.xyxw
    r5.xyw = ((r1.wwww)*(r5.xyxw)).xyw;
    // dp3 r1.w, r5.xywx, r4.xyzx
    r1.w = (dot((r5.xywx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r6.xyz, r1.wwww, r5.xywx
    r6.xyz = ((r1.wwww)*(r5.xywx)).xyz;
    // mad r4.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // dp3 r6.x, r2.xyzx, r4.xyzx
    r6.x = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // dp3 r6.y, r3.xyzx, r4.xyzx
    r6.y = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // mul r6.zw, cb0[0].xxxy, l(0.000000, 0.000000, 0.000300, 0.000300)
    r6.zw = ((source[0].xxxy)*(float4(0.000000,0.000000,0.000300,0.000300))).zw;
    // mad r6.xy, cb0[5].yyyy, r6.xyxx, r6.zwzz
    r6.xy = ((source[5].yyyy)*(r6.xyxx)+(r6.zwzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[2].xyzx
    r6.xyz = ((r6.xyzx)*(source[2].xyzx)).xyz;
    // mad r6.xyz, cb0[5].zzzz, r6.xyzx, r6.xyzx
    r6.xyz = ((source[5].zzzz)*(r6.xyzx)+(r6.xyzx)).xyz;
    // add r6.xyz, r6.xyzx, -cb0[5].zzzz
    r6.xyz = ((r6.xyzx)+(-(source[5].zzzz))).xyz;
    // mov_sat r7.xyz, r6.xyzx
    r7.xyz = (saturate(r6.xyzx)).xyz;
    // mul r1.w, r5.z, cb0[5].w
    r1.w = ((r5.zzzz)*(source[5].wwww)).w;
    // mul r8.xyz, cb0[3].xyzx, cb0[6].xxxx
    r8.xyz = ((source[3].xyzx)*(source[6].xxxx)).xyz;
    // mul r0.xyz, r0.xyzx, r8.xyzx
    r0.xyz = ((r0.xyzx)*(r8.xyzx)).xyz;
    // mad r0.xyz, r1.wwww, r7.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r7.xyzx)+(r0.xyzx)).xyz;
    // mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // mad r6.xyz, -r1.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r1.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r7.xyz, cb0[4].xyzx, cb0[6].yyyy
    r7.xyz = ((source[4].xyzx)*(source[6].yyyy)).xyz;
    // mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp2_sat r7.x, r5.ywyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r5.ywyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r5.xywx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r5.xywx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r5.xywx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r5.xywx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // dp2_sat r8.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r8.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r8.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r4.xyz, r7.xyzx, r7.xyzx
    r4.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // add r1.w, cb0[6].z, l(1.000000)
    r1.w = ((source[6].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t4.xyzw, s4
    r8.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r8.xyz, r8.xyzx, cb0[11].xyzx
    r8.xyz = ((r8.xyzx)*(source[11].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t5.xyzw, s4
    r9.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r9.xyz, r9.xyzx, cb0[12].xyzx
    r9.xyz = ((r9.xyzx)*(source[12].xyzx)).xyz;
    // dp3 r1.w, r9.xyzx, r4.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r4.xyz, r1.wwww, r8.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // mul r7.xyz, r2.wwww, r6.xyzx
    r7.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // dp3 r3.w, r9.xyzx, r5.xywx
    r3.w = (dot((r9.xyzx).xyz,(r5.xywx).xyz).xxxx).w;
    // mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // mul r9.yzw, r9.yyyy, cb0[9].xxyz
    r9.yzw = ((r9.yyyy)*(source[9].xxyz)).yzw;
    // mad r9.xyz, r9.xxxx, cb0[8].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[8].xyzx)+(r9.yzwy)).xyz;
    // mul r9.xyz, r9.xyzx, cb0[10].wwww
    r9.xyz = ((r9.xyzx)*(source[10].wwww)).xyz;
    // mul r10.xyz, r0.xyzx, r9.xyzx
    r10.xyz = ((r0.xyzx)*(r9.xyzx)).xyz;
    // mad r10.xyz, r0.xyzx, r4.xyzx, r10.xyzx
    r10.xyz = ((r0.xyzx)*(r4.xyzx)+(r10.xyzx)).xyz;
    // mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // mad r6.xyz, r6.xyzx, r2.wwww, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // add r9.xyz, r6.xyzx, cb0[1].xyzx
    r9.xyz = ((r6.xyzx)+(source[1].xyzx)).xyz;
    // mad r9.xyz, r0.xyzx, cb0[10].xyzx, r9.xyzx
    r9.xyz = ((r0.xyzx)*(source[10].xyzx)+(r9.xyzx)).xyz;
    // mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r2.x, r2.xyzx, r5.xywx
    r2.x = (dot((r2.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // dp3 r2.y, r3.xyzx, r5.xywx
    r2.y = (dot((r3.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // dp3 r2.z, r1.xyzx, r5.xywx
    r2.z = (dot((r1.xyzx).xyz,(r5.xywx).xyz).xxxx).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, r2.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // dp3 r1.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).w;
    // div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // ge r1.z, l(0.000000), r1.z
    r1.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).z;
    // ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // movc r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.zzzz) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r1.xyz, r8.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r8.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // mul o4.z, r1.x, r6.x
    output.targets[4].z = ((r1.xxxx)*(r6.xxxx)).z;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// ace7ceea9fcb4e4da670deab7a04c5f4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent61Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[12]=0.f;source[13]=0.f;source[14]=float4(g_SourceMapAmbient.rgb,1.f);
    source[1]=SourceCharacterAppend(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.0199999996f,0.f,0.f,0.f))),frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.0199999996f,0.f,0.f,0.f))),1u);
    source[5]=SourceCharacterAppend(frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(0.00100000005f,0.f,0.f,0.f))),frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(0.f,0.f,0.f,0.f))),1u);
    source[6]=SourceCharacterAppend(frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(-0.00200000009f,0.f,0.f,0.f))),frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(0.f,0.f,0.f,0.f))),1u);
    source[7]=SourceCharacterAppend(frac(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(-0.0199999996f,0.f,0.f,0.f))),frac(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(0.f,0.f,0.f,0.f))),1u);
    source[8].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    source[8].y=((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.0199999996f,0.f,0.f,0.f))).x;
    source[8].w=(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.0199999996f,0.f,0.f,0.f)))).x;
    source[9].w=((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))).x;
    source[10].x=(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(0.f,0.f,0.f,0.f))).x;
    source[10].y=(frac(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(0.f,0.f,0.f,0.f)))).x;
    source[10].z=(frac(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(-0.0199999996f,0.f,0.f,0.f)))).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f;
    // mad r0.x, -v4.y, v4.y, l(1.000000)
    r0.x = ((-(v4.yyyy))*(v4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mad r0.x, -r0.x, r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // add r0.yzw, -cb0[3].xxyz, cb0[4].xxyz
    r0.yzw = ((-(source[3].xxyz))+(source[4].xxyz)).yzw;
    // mad r0.xyz, r0.xxxx, r0.yzwy, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[3].xyzx)).xyz;
    // dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // mad r1.xy, cb0[8].zzzz, v5.xyxx, cb0[1].xyxx
    r1.xy = ((source[8].zzzz)*(v5.xyxx)+(source[1].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r1.xyz, r1.xyzx, l(5.000000, 5.000000, 5.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(5.000000,5.000000,5.000000,0.000000))).xyz;
    // mul r2.xy, v5.xyxx, cb0[8].zzzz
    r2.xy = ((v5.xyxx)*(source[8].zzzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r1.xyz, r1.xyzx, r2.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(r2.xyzx)).xyz;
    // mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // add r1.xy, v4.xyxx, cb0[5].xyxx
    r1.xy = ((v4.xyxx)+(source[5].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r2.xy, v4.xyxx, l(-1.000000, 1.000000, 0.000000, 0.000000), cb0[6].xyxx
    r2.xy = ((v4.xyxx)*(float4(-1.000000,1.000000,0.000000,0.000000))+(source[6].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r1.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r1.xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // mad r2.xy, v4.xyxx, l(-1.000000, 1.000000, 0.000000, 0.000000), cb0[7].xyxx
    r2.xy = ((v4.xyxx)*(float4(-1.000000,1.000000,0.000000,0.000000))+(source[7].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad_sat r1.xyz, r2.xyzx, r1.xyzx, r1.xyzx
    r1.xyz = (saturate((r2.xyzx)*(r1.xyzx)+(r1.xyzx))).xyz;
    // mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // mad r0.xyz, r1.xyzx, -r0.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(-(r0.xyzx))+(r0.xyzx)).xyz;
    // mad r0.xyz, cb0[11].xxxx, r0.xyzx, cb0[0].xyzx
    r0.xyz = ((source[11].xxxx)*(r0.xyzx)+(source[0].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // mul r1.yzw, r1.yyyy, cb0[13].xxyz
    r1.yzw = ((r1.yyyy)*(source[13].xxyz)).yzw;
    // mad r1.xyz, r1.xxxx, cb0[12].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[12].xyzx)+(r1.yzwy)).xyz;
    // mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // mad r0.xyz, r1.xyzx, cb2[3].xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(passValues[3].xyzx)+(r0.xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].xyzx)).xyz;
    // dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad o0.xyz, cb2[3].xyzx, cb0[14].xyzx, r0.xyzx
    output.targets[0].xyz = ((passValues[3].xyzx)*(source[14].xyzx)+(r0.xyzx)).xyz;
    // mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// f0a8b9532554564aa35221507bc8f262
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent61Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[12]=0.f;source[13]=0.f;source[14]=float4(0.f,0.f,0.f,0.f);
    source[15]=g_LightmapAverageScale;source[16]=g_LightmapDirectionalScale;
    source[1]=SourceCharacterAppend(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.0199999996f,0.f,0.f,0.f))),frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.0199999996f,0.f,0.f,0.f))),1u);
    source[5]=SourceCharacterAppend(frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(0.00100000005f,0.f,0.f,0.f))),frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(0.f,0.f,0.f,0.f))),1u);
    source[6]=SourceCharacterAppend(frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(-0.00200000009f,0.f,0.f,0.f))),frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(0.f,0.f,0.f,0.f))),1u);
    source[7]=SourceCharacterAppend(frac(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(-0.0199999996f,0.f,0.f,0.f))),frac(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(0.f,0.f,0.f,0.f))),1u);
    source[8].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    source[8].y=((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.0199999996f,0.f,0.f,0.f))).x;
    source[8].w=(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.0199999996f,0.f,0.f,0.f)))).x;
    source[9].w=((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterBaseConstants[60]*float4(4.f,0.f,0.f,0.f)))).x;
    source[10].x=(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(0.f,0.f,0.f,0.f))).x;
    source[10].y=(frac(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(0.f,0.f,0.f,0.f)))).x;
    source[10].z=(frac(((g_SourceCharacterBaseConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(-0.0199999996f,0.f,0.f,0.f)))).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // mad r0.x, -v4.y, v4.y, l(1.000000)
    r0.x = ((-(v4.yyyy))*(v4.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mad r0.x, -r0.x, r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // add r0.yzw, -cb0[3].xxyz, cb0[4].xxyz
    r0.yzw = ((-(source[3].xxyz))+(source[4].xxyz)).yzw;
    // mad r0.xyz, r0.xxxx, r0.yzwy, cb0[3].xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(source[3].xyzx)).xyz;
    // dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // mad r0.xyz, cb0[9].xxxx, r1.xyzx, r0.xyzx
    r0.xyz = ((source[9].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // mad r1.xy, cb0[8].zzzz, v5.xyxx, cb0[1].xyxx
    r1.xy = ((source[8].zzzz)*(v5.xyxx)+(source[1].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r1.xyz, r1.xyzx, l(5.000000, 5.000000, 5.000000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(5.000000,5.000000,5.000000,0.000000))).xyz;
    // mul r2.xy, v5.xyxx, cb0[8].zzzz
    r2.xy = ((v5.xyxx)*(source[8].zzzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r1.xyz, r1.xyzx, r2.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(r2.xyzx)).xyz;
    // mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // add r1.xy, v4.xyxx, cb0[5].xyxx
    r1.xy = ((v4.xyxx)+(source[5].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r2.xy, v4.xyxx, l(-1.000000, 1.000000, 0.000000, 0.000000), cb0[6].xyxx
    r2.xy = ((v4.xyxx)*(float4(-1.000000,1.000000,0.000000,0.000000))+(source[6].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r1.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r1.xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // mad r2.xy, v4.xyxx, l(-1.000000, 1.000000, 0.000000, 0.000000), cb0[7].xyxx
    r2.xy = ((v4.xyxx)*(float4(-1.000000,1.000000,0.000000,0.000000))+(source[7].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad_sat r1.xyz, r2.xyzx, r1.xyzx, r1.xyzx
    r1.xyz = (saturate((r2.xyzx)*(r1.xyzx)+(r1.xyzx))).xyz;
    // mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // mad r0.xyz, r1.xyzx, -r0.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(-(r0.xyzx))+(r0.xyzx)).xyz;
    // mad r0.xyz, cb0[11].xxxx, r0.xyzx, cb0[0].xyzx
    r0.xyz = ((source[11].xxxx)*(r0.xyzx)+(source[0].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // mul r1.yzw, r1.yyyy, cb0[13].xxyz
    r1.yzw = ((r1.yyyy)*(source[13].xxyz)).yzw;
    // mad r1.xyz, r1.xxxx, cb0[12].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[12].xyzx)+(r1.yzwy)).xyz;
    // mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // mul r2.xyz, r1.xyzx, cb2[3].xyzx
    r2.xyz = ((r1.xyzx)*(passValues[3].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t3.xyzw, s2
    r3.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[16].xyzx
    r3.xyz = ((r3.xyzx)*(source[16].xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t2.xyzw, s2
    r3.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[15].xyzx
    r3.xyz = ((r3.xyzx)*(source[15].xyzx)).xyz;
    // mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // mad r2.xyz, cb2[3].xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((passValues[3].xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad o0.xyz, cb2[3].xyzx, cb0[14].xyzx, r0.xyzx
    output.targets[0].xyz = ((passValues[3].xyzx)*(source[14].xyzx)+(r0.xyzx)).xyz;
    // mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xw, l(0,0,0,0)
    output.targets[4].xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// d25977be2346854b9b5bf041418ff66d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent62Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[10]=0.f;source[11]=0.f;source[12]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
    r0.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.yzyy).xy,0.f).rrrr).yxzw).y;
    // min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // mad r0.z, r0.y, cb2[1].x, cb2[1].y
    r0.z = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // mad r0.y, r0.y, cb2[1].z, -cb2[1].w
    r0.y = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // add r0.z, -cb0[8].x, l(1.000000)
    r0.z = ((-(source[8].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // add r0.y, r0.y, -v8.w
    r0.y = ((r0.yyyy)+(-(v8.wwww))).y;
    // max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r1.x, r0.z, r1.x
    r1.x = ((r0.zzzz)*(r1.xxxx)).x;
    // mad r1.x, r0.y, r1.y, -r1.x
    r1.x = ((r0.yyyy)*(r1.yyyy)+(-(r1.xxxx))).x;
    // mul r0.z, r1.x, v1.w
    r0.z = ((r1.xxxx)*(v1.wwww)).z;
    // add r1.xyw, -cb0[1].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r1.xyw = ((-(source[1].xyxz))+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // add r2.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // dp2 r3.x, cb0[2].xyxx, r2.xyxx
    r3.x = (dot((source[2].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // dp2 r3.y, cb0[3].xyxx, r2.xyxx
    r3.y = (dot((source[3].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // add r2.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r2.xy, r2.xyxx, cb0[4].xyxx
    r2.xy = ((r2.xyxx)*(source[4].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // mad r2.xyz, cb0[7].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[7].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // mul r3.xyz, r2.xyzx, cb0[7].yyyy
    r3.xyz = ((r2.xyzx)*(source[7].yyyy)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[5].xyzx
    r3.xyz = ((r3.xyzx)*(source[5].xyzx)).xyz;
    // mul r1.xyw, r1.xyxw, r3.xyxz
    r1.xyw = ((r1.xyxw)*(r3.xyxz)).xyw;
    // mad r1.xyw, r1.xyxw, cb2[3].wwww, cb2[3].xyxz
    r1.xyw = ((r1.xyxw)*(passValues[3].wwww)+(passValues[3].xyxz)).xyw;
    // mul r2.xyz, r2.xyzx, cb0[7].zzzz
    r2.xyz = ((r2.xyzx)*(source[7].zzzz)).xyz;
    // mul r2.xyz, r2.xyzx, cb0[6].xyzx
    r2.xyz = ((r2.xyzx)*(source[6].xyzx)).xyz;
    // mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r2.w, r2.w, v7.z
    r2.w = ((r2.wwww)*(v7.zzzz)).w;
    // mad r3.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // mul r3.yzw, r3.yyyy, cb0[11].xxyz
    r3.yzw = ((r3.yyyy)*(source[11].xxyz)).yzw;
    // mad r3.xyz, r3.xxxx, cb0[10].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[10].xyzx)+(r3.yzwy)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[12].wwww
    r3.xyz = ((r3.xyzx)*(source[12].wwww)).xyz;
    // mul r4.xyz, r1.xywx, r3.xyzx
    r4.xyz = ((r1.xywx)*(r3.xyzx)).xyz;
    // mad r3.xyz, r3.xyzx, r1.xywx, cb0[1].xyzx
    r3.xyz = ((r3.xyzx)*(r1.xywx)+(source[1].xyzx)).xyz;
    // mad r3.xyz, r1.xywx, cb0[12].xyzx, r3.xyzx
    r3.xyz = ((r1.xywx)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // mad o0.xyz, r3.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // mov r0.y, r1.z
    r0.y = (r1.zzzz).y;
    // dp3 r1.z, r0.yzwy, r0.yzwy
    r1.z = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).z;
    // rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // mul r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)*(r1.zzzz)).yzw;
    // dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).z;
    // div r0.yz, r0.yyzy, r1.zzzz
    r0.yz = ((r0.yyzy)/(r1.zzzz)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r3.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r3.xy, r3.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r3.xy = ((asuint(r3.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r3.xy, -|r0.zyzz|, r3.xyxx, r3.xyxx
    r3.xy = ((-(abs(r0.zyzz)))*(r3.xyxx)+(r3.xyxx)).xy;
    // movc r0.yz, r0.wwww, r3.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r3.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul_sat o3.w, cb0[7].w, l(0.002000)
    output.targets[3].w = (saturate((source[7].wwww)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // ftou r0.y, cb0[9].z
    r0.y = (asfloat((uint4)(source[9].zzzz))).y;
    // bfi r0.y, l(5), l(0), r0.y, l(32)
    r0.y = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.yyyy),uint4(32u,32u,32u,32u))).y;
    // utof r0.y, r0.y
    r0.y = ((float4)(asuint(r0.yyyy))).y;
    // mul o5.w, r0.y, l(0.003922)
    output.targets[5].w = ((r0.yyyy)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // mul_sat r0.yzw, r2.xxyz, l(0.000000, 0.100000, 0.100000, 0.100000)
    r0.yzw = (saturate((r2.xxyz)*(float4(0.000000,0.100000,0.100000,0.100000)))).yzw;
    // sqrt o5.xyz, r0.yzwy
    output.targets[5].xyz = (sqrt(r0.yzwy)).xyz;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r1.xywx
    output.targets[3].xyz = (r1.xywx).xyz;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // ret
    return output;
}

// 5c671d30bc73594c8d33cd808f0ea4d5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent62Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[10]=0.f;source[11]=0.f;source[12]=float4(0.f,0.f,0.f,0.f);
    source[13]=g_LightmapAverageScale;source[14]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
    r0.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.yzyy).xy,0.f).rrrr).yxzw).y;
    // min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // mad r0.z, r0.y, cb2[1].x, cb2[1].y
    r0.z = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // mad r0.y, r0.y, cb2[1].z, -cb2[1].w
    r0.y = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // add r0.z, -cb0[8].x, l(1.000000)
    r0.z = ((-(source[8].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // add r0.y, r0.y, -v8.w
    r0.y = ((r0.yyyy)+(-(v8.wwww))).y;
    // max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r1.x, r0.z, r1.x
    r1.x = ((r0.zzzz)*(r1.xxxx)).x;
    // mad r1.x, r0.y, r1.y, -r1.x
    r1.x = ((r0.yyyy)*(r1.yyyy)+(-(r1.xxxx))).x;
    // mul r0.z, r1.x, v1.w
    r0.z = ((r1.xxxx)*(v1.wwww)).z;
    // dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyw, r1.xxxx, v6.xyxz
    r1.xyw = ((r1.xxxx)*(v6.xyxz)).xyw;
    // mad r1.xyw, r1.wwww, l(0.000000, 0.000000, 0.000000, 2.000000), -r1.xyxw
    r1.xyw = ((r1.wwww)*(float4(0.000000,0.000000,0.000000,2.000000))+(-(r1.xyxw))).xyw;
    // add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // add r3.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // dp2 r4.x, cb0[2].xyxx, r3.xyxx
    r4.x = (dot((source[2].xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // dp2 r4.y, cb0[3].xyxx, r3.xyxx
    r4.y = (dot((source[3].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // add r3.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r3.xy, r3.xyxx, cb0[4].xyxx
    r3.xy = ((r3.xyxx)*(source[4].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t2.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // mad r3.xyz, cb0[7].xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((source[7].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // mul r4.xyz, r3.xyzx, cb0[7].yyyy
    r4.xyz = ((r3.xyzx)*(source[7].yyyy)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[5].xyzx
    r4.xyz = ((r4.xyzx)*(source[5].xyzx)).xyz;
    // mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[7].zzzz
    r3.xyz = ((r3.xyzx)*(source[7].zzzz)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[6].xyzx
    r3.xyz = ((r3.xyzx)*(source[6].xyzx)).xyz;
    // mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // dp2_sat r4.x, r1.ywyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r1.ywyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r4.y, r1.xywx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r1.xywx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r4.z, r1.xywx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r1.xywx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // add r1.x, cb0[7].w, l(1.000000)
    r1.x = ((source[7].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // mul r1.xyw, r1.xxxx, r4.xyxz
    r1.xyw = ((r1.xxxx)*(r4.xyxz)).xyw;
    // exp r1.xyw, r1.xyxw
    r1.xyw = (exp2(r1.xyxw)).xyw;
    // sample_indexable(texture2d)(float,float,float,float) r4.xyz, v3.zwzz, t3.xyzw, s3
    r4.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r4.xyz, r4.xyzx, cb0[13].xyzx
    r4.xyz = ((r4.xyzx)*(source[13].xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t4.xyzw, s3
    r5.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r5.xyz, r5.xyzx, cb0[14].xyzx
    r5.xyz = ((r5.xyzx)*(source[14].xyzx)).xyz;
    // dp3 r2.w, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // mul r6.xyz, r2.wwww, r4.xyzx
    r6.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // mul r7.xyz, r3.xyzx, r4.xyzx
    r7.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // dp3 r1.x, r5.xyzx, r1.xywx
    r1.x = (dot((r5.xyzx).xyz,(r1.xywx).xyz).xxxx).x;
    // mul r5.xyz, r1.xxxx, r7.xyzx
    r5.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // dp3 r1.y, v7.xyzx, v7.xyzx
    r1.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r1.y, r1.y, v7.z
    r1.y = ((r1.yyyy)*(v7.zzzz)).y;
    // mad r1.yw, r1.yyyy, l(0.000000, 0.500000, 0.000000, -0.500000), l(0.000000, 0.500000, 0.000000, 0.500000)
    r1.yw = ((r1.yyyy)*(float4(0.000000,0.500000,0.000000,-0.500000))+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // mul r1.yw, r1.yyyw, r1.yyyw
    r1.yw = ((r1.yyyw)*(r1.yyyw)).yw;
    // mul r8.xyz, r1.wwww, cb0[11].xyzx
    r8.xyz = ((r1.wwww)*(source[11].xyzx)).xyz;
    // mad r8.xyz, r1.yyyy, cb0[10].xyzx, r8.xyzx
    r8.xyz = ((r1.yyyy)*(source[10].xyzx)+(r8.xyzx)).xyz;
    // mul r8.xyz, r8.xyzx, cb0[12].wwww
    r8.xyz = ((r8.xyzx)*(source[12].wwww)).xyz;
    // mul r9.xyz, r2.xyzx, r8.xyzx
    r9.xyz = ((r2.xyzx)*(r8.xyzx)).xyz;
    // mad r9.xyz, r2.xyzx, r6.xyzx, r9.xyzx
    r9.xyz = ((r2.xyzx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, r2.wwww, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r2.wwww)+(r8.xyzx)).xyz;
    // mad r1.xyw, r7.xyxz, r1.xxxx, r9.xyxz
    r1.xyw = ((r7.xyxz)*(r1.xxxx)+(r9.xyxz)).xyw;
    // add r7.xyz, r1.xywx, cb0[1].xyzx
    r7.xyz = ((r1.xywx)+(source[1].xyzx)).xyz;
    // mad r7.xyz, r2.xyzx, cb0[12].xyzx, r7.xyzx
    r7.xyz = ((r2.xyzx)*(source[12].xyzx)+(r7.xyzx)).xyz;
    // mad o0.xyz, r7.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r7.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // mov r0.y, r1.z
    r0.y = (r1.zzzz).y;
    // dp3 r1.z, r0.yzwy, r0.yzwy
    r1.z = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).z;
    // rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // mul r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)*(r1.zzzz)).yzw;
    // dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).z;
    // div r0.yz, r0.yyzy, r1.zzzz
    r0.yz = ((r0.yyzy)/(r1.zzzz)).yz;
    // ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // ge r7.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r7.xy, r7.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r7.xy = ((asuint(r7.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r7.xy, -|r0.zyzz|, r7.xyxx, r7.xyxx
    r7.xy = ((-(abs(r0.zyzz)))*(r7.xyxx)+(r7.xyxx)).xy;
    // movc r0.yz, r0.wwww, r7.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r7.xxyx) : (r0.yyzy)).yz;
    // mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul_sat o3.w, cb0[7].w, l(0.002000)
    output.targets[3].w = (saturate((source[7].wwww)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // dp3 o4.y, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r0.yzw, r4.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r4.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // div r0.yzw, r6.xxyz, r0.yyzw
    r0.yzw = ((r6.xxyz)/(r0.yyzw)).yzw;
    // dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mul o4.z, r0.y, r1.x
    output.targets[4].z = ((r0.yyyy)*(r1.xxxx)).z;
    // ftou r0.y, cb0[9].z
    r0.y = (asfloat((uint4)(source[9].zzzz))).y;
    // bfi r0.y, l(5), l(0), r0.y, l(32)
    r0.y = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.yyyy),uint4(32u,32u,32u,32u))).y;
    // utof r0.y, r0.y
    r0.y = ((float4)(asuint(r0.yyyy))).y;
    // mul o5.w, r0.y, l(0.003922)
    output.targets[5].w = ((r0.yyyy)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // mul_sat r0.yzw, r3.xxyz, l(0.000000, 0.100000, 0.100000, 0.100000)
    r0.yzw = (saturate((r3.xxyz)*(float4(0.000000,0.100000,0.100000,0.100000)))).yzw;
    // sqrt o5.xyz, r0.yzwy
    output.targets[5].xyz = (sqrt(r0.yzwy)).xyz;
    // mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // ret
    return output;
}

// aeaa8c3eeb66db40a737b6eeff0ffc98
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent63Base(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+0u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[8]=0.f;source[9]=0.f;source[10]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r1.z, r1.z, cb0[6].y
    r1.z = ((r1.zzzz)*(source[6].yyyy)).z;
    // dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // mul r1.xy, r1.xyxx, cb0[5].wwww
    r1.xy = ((r1.xyxx)*(source[5].wwww)).xy;
    // mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // add r1.x, -r1.w, l(1.000000)
    r1.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // div r1.xyw, r2.xyxz, r1.xxxx
    r1.xyw = ((r2.xyxz)/(r1.xxxx)).xyw;
    // dp3 r2.x, r1.xywx, r1.xywx
    r2.x = (dot((r1.xywx).xyz,(r1.xywx).xyz).xxxx).x;
    // rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // mul r1.xyw, r1.xyxw, r2.xxxx
    r1.xyw = ((r1.xyxw)*(r2.xxxx)).xyw;
    // dp3 r2.x, v6.xyzx, v6.xyzx
    r2.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // mul r2.xyz, r2.xxxx, v6.xyzx
    r2.xyz = ((r2.xxxx)*(v6.xyzx)).xyz;
    // dp3 r2.z, r1.xywx, r2.xyzx
    r2.z = (dot((r1.xywx).xyz,(r2.xyzx).xyz).xxxx).z;
    // mul r2.zw, r1.xxxy, r2.zzzz
    r2.zw = ((r1.xxxy)*(r2.zzzz)).zw;
    // mad r2.xy, r2.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), -r2.xyxx
    r2.xy = ((r2.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r2.xyxx))).xy;
    // add r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r2.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // mad r2.xy, r2.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)*(source[3].xyzx)).xyz;
    // mad r2.xyz, cb0[6].xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((source[6].xxxx)*(r2.xyzx)+(r2.xyzx)).xyz;
    // add r2.xyz, r2.xyzx, -cb0[6].xxxx
    r2.xyz = ((r2.xyzx)+(-(source[6].xxxx))).xyz;
    // mov_sat r3.xyz, r2.xyzx
    r3.xyz = (saturate(r2.xyzx)).xyz;
    // mov_sat r2.xyz, -r2.xyzx
    r2.xyz = (saturate(-(r2.xyzx))).xyz;
    // mad r2.xyz, -r1.zzzz, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r1.zzzz))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r4.xyz, cb0[4].xyzx, cb0[6].zzzz
    r4.xyz = ((source[4].xyzx)*(source[6].zzzz)).xyz;
    // mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // mul_sat r0.w, r0.w, cb0[7].y
    r0.w = (saturate((r0.wwww)*(source[7].yyyy))).w;
    // mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r2.xyzx, r1.xywx
    r0.w = (dot((r2.xyzx).xyz,(r1.xywx).xyz).xxxx).w;
    // mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // mul r2.yzw, r2.yyyy, cb0[9].xxyz
    r2.yzw = ((r2.yyyy)*(source[9].xxyz)).yzw;
    // mad r2.xyz, r2.xxxx, cb0[8].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[8].xyzx)+(r2.yzwy)).xyz;
    // mul r2.xyz, r2.xyzx, cb0[10].wwww
    r2.xyz = ((r2.xyzx)*(source[10].wwww)).xyz;
    // mad r3.xyz, r2.xyzx, r0.xyzx, cb0[1].xyzx
    r3.xyz = ((r2.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r2.xyz, r0.xyzx, cb0[10].xyzx, r3.xyzx
    r2.xyz = ((r0.xyzx)*(source[10].xyzx)+(r3.xyzx)).xyz;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r1.xywx
    r0.z = (dot((r0.xyzx).xyz,(r1.xywx).xyz).xxxx).z;
    // dp3 r0.x, r2.xyzx, r1.xywx
    r0.x = (dot((r2.xyzx).xyz,(r1.xywx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r2.xyzx, r1.xywx
    r0.y = (dot((r2.xyzx).xyz,(r1.xywx).xyz).xxxx).y;
    // dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 8e51d46ed4af154e9d1fb8b65b7f96cb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapTranslucent63Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint zero=32u;zero<64u;++zero)source[zero]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[9]=0.f;source[10]=0.f;source[11]=float4(0.f,0.f,0.f,0.f);
    source[12]=g_LightmapAverageScale;source[13]=g_LightmapDirectionalScale;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r1.zw, r2.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r2.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // mul r0.w, r2.z, cb0[7].y
    r0.w = ((r2.zzzz)*(source[7].yyyy)).w;
    // dp2 r2.x, r1.zwzz, r1.zwzz
    r2.x = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).x;
    // mul r1.zw, r1.zzzw, cb0[6].wwww
    r1.zw = ((r1.zzzw)*(source[6].wwww)).zw;
    // mul r3.xy, r1.zwzz, v2.wwww
    r3.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // add r1.z, -r2.x, l(1.000000)
    r1.z = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // add r3.z, r1.z, l(0.000010)
    r3.z = ((r1.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp3 r1.z, r3.xyzx, r3.xyzx
    r1.z = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // div r2.xyz, r3.xyzx, r1.zzzz
    r2.xyz = ((r3.xyzx)/(r1.zzzz)).xyz;
    // dp3 r1.z, r2.xyzx, r2.xyzx
    r1.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // mul r2.xyz, r1.zzzz, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r2.xyzx)).xyz;
    // dp3 r1.z, r2.xyzx, r0.xyzx
    r1.z = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // mul r3.xyz, r1.zzzz, r2.xyzx
    r3.xyz = ((r1.zzzz)*(r2.xyzx)).xyz;
    // mad r0.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // add r1.zw, r0.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r0.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), -v4.xxxy
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(-(v4.xxxy))).zw;
    // mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.750000, 0.750000), v4.xxxy
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.750000,0.750000))+(v4.xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r1.zwzz, t2.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)*(source[3].xyzx)).xyz;
    // mad r3.xyz, cb0[7].xxxx, r3.xyzx, r3.xyzx
    r3.xyz = ((source[7].xxxx)*(r3.xyzx)+(r3.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, -cb0[7].xxxx
    r3.xyz = ((r3.xyzx)+(-(source[7].xxxx))).xyz;
    // mov_sat r4.xyz, r3.xyzx
    r4.xyz = (saturate(r3.xyzx)).xyz;
    // mov_sat r3.xyz, -r3.xyzx
    r3.xyz = (saturate(-(r3.xyzx))).xyz;
    // mad r3.xyz, -r0.wwww, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r0.wwww))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r5.xyz, cb0[4].xyzx, cb0[7].zzzz
    r5.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r1.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // mul_sat r1.w, r6.w, cb0[8].y
    r1.w = (saturate((r6.wwww)*(source[8].yyyy))).w;
    // mul o0.w, r1.w, cb0[0].x
    output.targets[0].w = ((r1.wwww)*(source[0].xxxx)).w;
    // mad r4.xyz, r0.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r4.xyz, r0.wwww, v7.xyzx
    r4.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, r2.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // mul r4.yzw, r4.yyyy, cb0[10].xxyz
    r4.yzw = ((r4.yyyy)*(source[10].xxyz)).yzw;
    // mad r4.xyz, r4.xxxx, cb0[9].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[9].xyzx)+(r4.yzwy)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[11].wwww
    r4.xyz = ((r4.xyzx)*(source[11].wwww)).xyz;
    // mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // dp2_sat r6.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r6.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r6.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t5.xyzw, s4
    r7.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r7.xyz, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((r7.xyzx)*(source[13].xyzx)).xyz;
    // dp3 r0.w, r7.xyzx, r6.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t4.xyzw, s4
    r6.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[12].xyzx
    r6.xyz = ((r6.xyzx)*(source[12].xyzx)).xyz;
    // mul r8.xyz, r0.wwww, r6.xyzx
    r8.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // mad r4.xyz, r6.xyzx, r0.wwww, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r4.xyz, r8.xyzx, r4.xyzx
    r4.xyz = ((r8.xyzx)/(r4.xyzx)).xyz;
    // mad r5.xyz, r3.xyzx, r8.xyzx, r5.xyzx
    r5.xyz = ((r3.xyzx)*(r8.xyzx)+(r5.xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // dp2_sat r4.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r4.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r4.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // log r0.xyz, r4.xyzx
    r0.xyz = (log2(r4.xyzx)).xyz;
    // add r1.w, cb0[8].x, l(1.000000)
    r1.w = ((source[8].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // dp3 r0.x, r7.xyzx, r0.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // mul r4.xyz, cb0[5].xyzx, cb0[7].wwww
    r4.xyz = ((source[5].xyzx)*(source[7].wwww)).xyz;
    // mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[4].wwww, cb2[4].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r1.xyz, r6.xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)*(r1.xyzx)).xyz;
    // mad r4.xyz, r1.xyzx, r0.xxxx, r5.xyzx
    r4.xyz = ((r1.xyzx)*(r0.xxxx)+(r5.xyzx)).xyz;
    // mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r0.xyz, r4.xyzx, cb0[1].xyzx
    r0.xyz = ((r4.xyzx)+(source[1].xyzx)).xyz;
    // mad r0.xyz, r3.xyzx, cb0[11].xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(source[11].xyzx)+(r0.xyzx)).xyz;
    // mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mul o4.z, r0.w, r4.x
    output.targets[4].z = ((r0.wwww)*(r4.xxxx)).z;
    // dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

#endif
