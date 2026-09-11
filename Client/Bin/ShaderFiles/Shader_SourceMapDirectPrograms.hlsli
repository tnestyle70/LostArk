// Recovered native forward direct-light programs. Scene light selection is a project adapter.
#ifndef LOSTARK_SOURCE_MAP_DIRECT_PROGRAMS
#define LOSTARK_SOURCE_MAP_DIRECT_PROGRAMS
float4 g_SourceCharacterLightConstants[64];
SamplerState SourceMapDirectSkySampler{Filter=MIN_MAG_MIP_LINEAR;AddressU=Wrap;AddressV=Clamp;};

// dad0c6b258a72a4ca16fdcb5ab7e94ce
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect40(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[10]=0.f;
    source[10]=float4(input.lightColor,1.f);
    source[6].y=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // mul r0.xy, cb0[1].zwzz, cb0[6].yyyy
    r0.xy = ((source[1].zwzz)*(source[6].yyyy)).xy;
    // mad r0.zw, cb0[6].xxxx, v2.xxxy, r0.xxxy
    r0.zw = ((source[6].xxxx)*(v2.xxxy)+(r0.xxxy)).zw;
    // mul r0.zw, r0.zzzw, cb0[1].xxxy
    r0.zw = ((r0.zzzw)*(source[1].xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r1.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // add r0.z, r0.z, l(0.000010)
    r0.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r2.xy, v2.xyxx, cb0[6].xxxx
    r2.xy = ((v2.xyxx)*(source[6].xxxx)).xy;
    // mad r0.xy, r0.xyxx, l(-0.300000, -0.300000, 0.000000, 0.000000), r2.xyxx
    r0.xy = ((r0.xyxx)*(float4(-0.300000,-0.300000,0.000000,0.000000))+(r2.xyxx)).xy;
    // mad r2.xy, cb0[6].yyyy, cb0[2].zwzz, r2.xyxx
    r2.xy = ((source[6].yyyy)*(source[2].zwzz)+(r2.xyxx)).xy;
    // mul r0.xy, r0.xyxx, cb0[1].xyxx
    r0.xy = ((r0.xyxx)*(source[1].xyxx)).xy;
    // mul r0.xy, r0.xyxx, l(0.300000, 0.300000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.300000,0.300000,0.000000,0.000000))).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.x, r3.xyxx, r3.xyxx
    r0.x = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // add r3.z, r0.z, r0.x
    r3.z = ((r0.zzzz)+(r0.xxxx)).z;
    // mov r1.z, l(0.000010)
    r1.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // add r0.xyz, r1.xyzx, r3.xyzx
    r0.xyz = ((r1.xyzx)+(r3.xyzx)).xyz;
    // mul r1.xyzw, r0.xyxy, cb0[6].zzww
    r1.xyzw = ((r0.xyxy)*(source[6].zzww)).xyzw;
    // mad r1.zw, r2.xxxy, cb0[2].xxxy, r1.zzzw
    r1.zw = ((r2.xxxy)*(source[2].xxxy)+(r1.zzzw)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s2, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mad r2.x, -v5.z, r0.w, l(1.000000)
    r2.x = ((-(v5.zzzz))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r2.yzw, r0.wwww, v5.xxyz
    r2.yzw = ((r0.wwww)*(v5.xxyz)).yzw;
    // log r0.w, |r2.x|
    r0.w = (log2(abs(r2.xxxx))).w;
    // lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // mul r0.w, r0.w, cb0[7].x
    r0.w = ((r0.wwww)*(source[7].xxxx)).w;
    // exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // mul_sat r0.w, r0.w, cb0[7].y
    r0.w = (saturate((r0.wwww)*(source[7].yyyy))).w;
    // movc r0.w, r2.x, l(0), r0.w
    r0.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // add r2.x, -r0.w, l(1.000000)
    r2.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r2.x, r2.x, cb0[7].z
    r2.x = ((r2.xxxx)*(source[7].zzzz)).x;
    // mad r0.xy, r2.xxxx, r1.zwzz, r1.xyxx
    r0.xy = ((r2.xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // rsq r1.y, r1.x
    r1.y = (rsqrt(r1.xxxx)).y;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // div r1.xzw, r0.xxyz, r1.xxxx
    r1.xzw = ((r0.xxyz)/(r1.xxxx)).xzw;
    // mul r3.xyz, r0.xyzx, r1.yyyy
    r3.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // dp3 r1.y, r3.xyzx, r2.yzwy
    r1.y = (dot((r3.xyzx).xyz,(r2.yzwy).xyz).xxxx).y;
    // mul r3.xy, r1.yyyy, r3.xyxx
    r3.xy = ((r1.yyyy)*(r3.xyxx)).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r2.yzyy
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r2.yzyy))).xy;
    // add r3.xy, r3.xyxx, -v2.xyxx
    r3.xy = ((r3.xyxx)+(-(v2.xyxx))).xy;
    // mad r3.xy, r3.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000), v2.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v2.xyxx)).xy;
    // mul r3.xy, r3.xyxx, cb0[8].wwww
    r3.xy = ((r3.xyxx)*(source[8].wwww)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t2.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)*(source[3].xyzx)).xyz;
    // add r4.xyz, r1.xzwx, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r4.xyz = ((r1.xzwx)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r4.xyz, cb0[9].xxxx, r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((source[9].xxxx)*(r4.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // dp3 r1.y, r4.xyzx, r2.yzwy
    r1.y = (dot((r4.xyzx).xyz,(r2.yzwy).xyz).xxxx).y;
    // mul r4.xy, r4.xyxx, r1.yyyy
    r4.xy = ((r4.xyxx)*(r1.yyyy)).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r2.yzyy
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r2.yzyy))).xy;
    // mul r4.xy, r4.xyxx, cb0[5].xyxx
    r4.xy = ((r4.xyxx)*(source[5].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t3.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r4.xyz, r4.xyzx, cb0[4].xyzx
    r4.xyz = ((r4.xyzx)*(source[4].xyzx)).xyz;
    // mad r4.xyz, cb0[9].yyyy, r4.xyzx, -r3.xyzx
    r4.xyz = ((source[9].yyyy)*(r4.xyzx)+(-(r3.xyzx))).xyz;
    // mad r3.xyz, r0.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mad r2.xyz, v3.xyzx, r0.wwww, r2.yzwy
    r2.xyz = ((v3.xyzx)*(r0.wwww)+(r2.yzwy)).xyz;
    // mul r4.xyz, r0.wwww, v3.xyzx
    r4.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.w, r2.xyzx, r1.xzwx
    r0.w = (dot((r2.xyzx).xyz,(r1.xzwx).xyz).xxxx).w;
    // dp3 r1.x, r4.xyzx, r1.xzwx
    r1.x = (dot((r4.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // mul r0.xyz, r0.xxxx, cb2[3].xyzx
    r0.xyz = ((r0.xxxx)*(passValues[3].xyzx)).xyz;
    // log r1.y, |r0.w|
    r1.y = (log2(abs(r0.wwww))).y;
    // lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r1.y, r1.y, cb0[9].w
    r1.y = ((r1.yyyy)*(source[9].wwww)).y;
    // exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mul r1.y, r1.y, cb0[9].z
    r1.y = ((r1.yyyy)*(source[9].zzzz)).y;
    // movc r0.w, r0.w, l(0), r1.y
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).w;
    // mad r1.xyz, r1.xxxx, r3.xyzx, r0.wwww
    r1.xyz = ((r1.xxxx)*(r3.xyzx)+(r0.wwww)).xyz;
    // div r2.xy, v6.xyxx, v6.wwww
    r2.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t4.yzwx, s0, l(0.000000)
    r0.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // min r0.w, r0.w, l(0.999000)
    r0.w = (min(r0.wwww,float4(0.999000,0.999000,0.999000,0.999000))).w;
    // mad r1.w, r0.w, cb2[1].z, -cb2[1].w
    r1.w = ((r0.wwww)*(passValues[1].zzzz)+(-(passValues[1].wwww))).w;
    // mad r0.w, r0.w, cb2[1].x, cb2[1].y
    r0.w = ((r0.wwww)*(passValues[1].xxxx)+(passValues[1].yyyy)).w;
    // div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // add r0.w, r0.w, -v6.w
    r0.w = ((r0.wwww)+(-(v6.wwww))).w;
    // add r1.w, -cb0[8].x, l(1.000000)
    r1.w = ((-(source[8].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // div_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)/(r1.wwww))).w;
    // log r1.w, r0.w
    r1.w = (log2(r0.wwww)).w;
    // lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r1.w, r1.w, cb0[8].y
    r1.w = ((r1.wwww)*(source[8].yyyy)).w;
    // exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // mul_sat r1.w, r0.w, cb0[8].z
    r1.w = (saturate((r0.wwww)*(source[8].zzzz))).w;
    // mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // mad r0.xyz, r1.xyzx, cb2[3].wwww, r0.xyzx
    r0.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r0.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 1e47719ced94b6478986989ad90d8b7d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect41(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[10]=0.f;
    source[10]=float4(input.lightColor,1.f);
    source[6].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // add r0.xy, v6.xyxx, cb0[0].xyxx
    r0.xy = ((v6.xyxx)+(source[0].xyxx)).xy;
    // mul r0.zw, cb0[1].zzzw, cb0[6].xxxx
    r0.zw = ((source[1].zzzw)*(source[6].xxxx)).zw;
    // mad r1.xy, r0.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000), r0.zwzz
    r1.xy = ((r0.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))+(r0.zwzz)).xy;
    // mul r0.xy, r0.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))).xy;
    // mad r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.300000, -0.300000), r0.xxxy
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,-0.300000,-0.300000))+(r0.xxxy)).zw;
    // mad r0.xy, cb0[6].xxxx, cb0[2].zwzz, r0.xyxx
    r0.xy = ((source[6].xxxx)*(source[2].zwzz)+(r0.xyxx)).xy;
    // mul r0.zw, r0.zzzw, cb0[1].xxxy
    r0.zw = ((r0.zzzw)*(source[1].xxxy)).zw;
    // mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.300000, 0.300000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.300000,0.300000))).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r0.zw, r1.xxxy, cb0[1].xxxy
    r0.zw = ((r1.xxxy)*(source[1].xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r1.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // add r0.z, r0.z, l(0.000010)
    r0.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // add r2.z, r0.z, r0.w
    r2.z = ((r0.zzzz)+(r0.wwww)).z;
    // mov r1.z, l(0.000010)
    r1.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // mul r2.xyzw, r1.xyxy, cb0[6].yyzz
    r2.xyzw = ((r1.xyxy)*(source[6].yyzz)).xyzw;
    // mad r0.xy, r0.xyxx, cb0[2].xyxx, r2.zwzz
    r0.xy = ((r0.xyxx)*(source[2].xyxx)+(r2.zwzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mad r0.w, -v5.z, r0.z, l(1.000000)
    r0.w = ((-(v5.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r3.xyz, r0.zzzz, v5.xyzx
    r3.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // log r0.z, |r0.w|
    r0.z = (log2(abs(r0.wwww))).z;
    // lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r0.z, r0.z, cb0[6].w
    r0.z = ((r0.zzzz)*(source[6].wwww)).z;
    // exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // mul_sat r0.z, r0.z, cb0[7].x
    r0.z = (saturate((r0.zzzz)*(source[7].xxxx))).z;
    // movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.w, r0.w, cb0[7].y
    r0.w = ((r0.wwww)*(source[7].yyyy)).w;
    // mad r1.xy, r0.wwww, r0.xyxx, r2.xyxx
    r1.xy = ((r0.wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // rsq r0.y, r0.x
    r0.y = (rsqrt(r0.xxxx)).y;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // div r2.xyz, r1.xyzx, r0.xxxx
    r2.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // mul r0.xyw, r0.yyyy, r1.xyxz
    r0.xyw = ((r0.yyyy)*(r1.xyxz)).xyw;
    // dp3 r0.w, r0.xywx, r3.xyzx
    r0.w = (dot((r0.xywx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r0.xy, r0.wwww, r0.xyxx
    r0.xy = ((r0.wwww)*(r0.xyxx)).xy;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r3.xyxx
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r3.xyxx))).xy;
    // add r0.xy, r0.xyxx, -v2.xyxx
    r0.xy = ((r0.xyxx)+(-(v2.xyxx))).xy;
    // mad r0.xy, r0.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000), v2.xyxx
    r0.xy = ((r0.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v2.xyxx)).xy;
    // mul r0.xy, r0.xyxx, cb0[8].zzzz
    r0.xy = ((r0.xyxx)*(source[8].zzzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t2.xywz, s3, l(0.000000)
    r0.xyw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyw;
    // mul r0.xyw, r0.xyxw, cb0[3].xyxz
    r0.xyw = ((r0.xyxw)*(source[3].xyxz)).xyw;
    // add r4.xyz, r2.xyzx, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r4.xyz = ((r2.xyzx)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r4.xyz, cb0[8].wwww, r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((source[8].wwww)*(r4.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r4.xy, r4.xyxx, r1.wwww
    r4.xy = ((r4.xyxx)*(r1.wwww)).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r3.xyxx
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r3.xyxx))).xy;
    // mul r4.xy, r4.xyxx, cb0[5].xyxx
    r4.xy = ((r4.xyxx)*(source[5].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t3.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r4.xyz, r4.xyzx, cb0[4].xyzx
    r4.xyz = ((r4.xyzx)*(source[4].xyzx)).xyz;
    // mad r4.xyz, cb0[9].xxxx, r4.xyzx, -r0.xywx
    r4.xyz = ((source[9].xxxx)*(r4.xyzx)+(-(r0.xywx))).xyz;
    // mad r0.xyz, r0.zzzz, r4.xyzx, r0.xywx
    r0.xyz = ((r0.zzzz)*(r4.xyzx)+(r0.xywx)).xyz;
    // dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mad r3.xyz, v3.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v3.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // mul r4.xyz, r0.wwww, v3.xyzx
    r4.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // dp3 r1.w, r4.xyzx, r2.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // mul r1.xyz, r1.xxxx, cb2[3].xyzx
    r1.xyz = ((r1.xxxx)*(passValues[3].xyzx)).xyz;
    // log r2.x, |r0.w|
    r2.x = (log2(abs(r0.wwww))).x;
    // lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r2.x, r2.x, cb0[9].z
    r2.x = ((r2.xxxx)*(source[9].zzzz)).x;
    // exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r2.x, r2.x, cb0[9].y
    r2.x = ((r2.xxxx)*(source[9].yyyy)).x;
    // movc r0.w, r0.w, l(0), r2.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // mad r0.xyz, r1.wwww, r0.xyzx, r0.wwww
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r0.wwww)).xyz;
    // mul r2.xyz, v6.yyyy, cb1[1].xywx
    r2.xyz = ((v6.yyyy)*(projection[1].xywx)).xyz;
    // mad r2.xyz, cb1[0].xywx, v6.xxxx, r2.xyzx
    r2.xyz = ((projection[0].xywx)*(v6.xxxx)+(r2.xyzx)).xyz;
    // mad r2.xyz, cb1[2].xywx, v6.zzzz, r2.xyzx
    r2.xyz = ((projection[2].xywx)*(v6.zzzz)+(r2.xyzx)).xyz;
    // mad r2.xyz, cb1[3].xywx, v6.wwww, r2.xyzx
    r2.xyz = ((projection[3].xywx)*(v6.wwww)+(r2.xyzx)).xyz;
    // div r2.xy, r2.xyxx, r2.zzzz
    r2.xy = ((r2.xyxx)/(r2.zzzz)).xy;
    // mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.w, r2.xyxx, t4.yzwx, s0, l(0.000000)
    r0.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // min r0.w, r0.w, l(0.999000)
    r0.w = (min(r0.wwww,float4(0.999000,0.999000,0.999000,0.999000))).w;
    // mad r1.w, r0.w, cb2[1].z, -cb2[1].w
    r1.w = ((r0.wwww)*(passValues[1].zzzz)+(-(passValues[1].wwww))).w;
    // mad r0.w, r0.w, cb2[1].x, cb2[1].y
    r0.w = ((r0.wwww)*(passValues[1].xxxx)+(passValues[1].yyyy)).w;
    // div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // add r0.w, -r2.z, r0.w
    r0.w = ((-(r2.zzzz))+(r0.wwww)).w;
    // add r1.w, -cb0[7].w, l(1.000000)
    r1.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // div_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)/(r1.wwww))).w;
    // log r1.w, r0.w
    r1.w = (log2(r0.wwww)).w;
    // lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r1.w, r1.w, cb0[8].x
    r1.w = ((r1.wwww)*(source[8].xxxx)).w;
    // exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // mul_sat r1.w, r0.w, cb0[8].y
    r1.w = (saturate((r0.wwww)*(source[8].yyyy))).w;
    // mul o0.w, r0.w, cb0[0].w
    output.targets[0].w = ((r0.wwww)*(source[0].wwww)).w;
    // mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 36e8eb53f5f0f1428aefb794851d27d5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect42(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[15]=0.f;
    source[15]=float4(input.lightColor,1.f);
    source[9].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f;
    // mul r0.xy, cb0[2].zwzz, cb0[9].xxxx
    r0.xy = ((source[2].zwzz)*(source[9].xxxx)).xy;
    // add r1.xyz, v6.xyzx, cb0[0].xyzx
    r1.xyz = ((v6.xyzx)+(source[0].xyzx)).xyz;
    // mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.010000, 0.010000), r0.xxxy
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,0.010000,0.010000))+(r0.xxxy)).zw;
    // mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.z, r2.xyxx, r2.xyxx
    r0.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // add r0.z, r0.z, l(0.000010)
    r0.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r3.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000)
    r3.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))).xy;
    // mad r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000), r3.xyxx
    r0.xy = ((r0.xyxx)*(float4(-0.500000,-0.500000,0.000000,0.000000))+(r3.xyxx)).xy;
    // mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // mul r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.x, r4.xyxx, r4.xyxx
    r0.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // add r4.z, r0.z, r0.x
    r4.z = ((r0.zzzz)+(r0.xxxx)).z;
    // mov r2.z, l(0.000010)
    r2.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // add r0.xyz, r2.xyzx, r4.xyzx
    r0.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // mul r2.xyzw, r0.xyxy, cb0[9].yyzz
    r2.xyzw = ((r0.xyxy)*(source[9].yyzz)).xyzw;
    // mul r3.zw, cb0[3].zzzw, cb0[9].xxxx
    r3.zw = ((source[3].zzzw)*(source[9].xxxx)).zw;
    // mad r4.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000), r3.zwzz
    r4.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))+(r3.zwzz)).xy;
    // mad r3.zw, r3.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000), r3.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,-0.500000,-0.500000))+(r3.xxxy)).zw;
    // mad r3.xy, cb0[9].xxxx, cb0[8].zwzz, r3.xyxx
    r3.xy = ((source[9].xxxx)*(source[8].zwzz)+(r3.xyxx)).xy;
    // mul r3.zw, r3.zzzw, cb0[3].xxxy
    r3.zw = ((r3.zzzw)*(source[3].xxxy)).zw;
    // mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), r2.zzzw
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(r2.zzzw)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.zw, r3.zwzz, t1.zwxy, s2, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // mad r2.zw, r4.xxxy, cb0[3].xxxy, r2.zzzw
    r2.zw = ((r4.xxxy)*(source[3].xxxy)+(r2.zzzw)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.zwzz, t1.zwxy, s2, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // mad r2.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), r2.zzzw
    r2.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(r2.zzzw)).zw;
    // add r2.zw, r2.zzzw, l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // mad r2.xy, cb0[9].wwww, r2.zwzz, r2.xyxx
    r2.xy = ((source[9].wwww)*(r2.zwzz)+(r2.xyxx)).xy;
    // mov r2.z, r0.z
    r2.z = (r0.zzzz).z;
    // mad r0.xy, cb0[12].yyyy, r0.xyxx, v2.xyxx
    r0.xy = ((source[12].yyyy)*(r0.xyxx)+(v2.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t5.xyzw, s6, l(0.000000)
    r0.x = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // add r0.yzw, -r2.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r0.yzw = ((-(r2.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // dp3 r1.x, r1.xyzx, r1.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // div r1.x, r1.z, r1.x
    r1.x = ((r1.zzzz)/(r1.xxxx)).x;
    // add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // mul r1.x, r1.x, cb0[10].x
    r1.x = ((r1.xxxx)*(source[10].xxxx)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mad r0.yzw, r1.xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // mul_sat r1.x, r1.x, cb0[12].x
    r1.x = (saturate((r1.xxxx)*(source[12].xxxx))).x;
    // dp3 r1.y, r0.yzwy, r0.yzwy
    r1.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // rsq r1.z, r1.y
    r1.z = (rsqrt(r1.yyyy)).z;
    // sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // div r2.xyz, r0.yzwy, r1.yyyy
    r2.xyz = ((r0.yzwy)/(r1.yyyy)).xyz;
    // mul r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)*(r1.zzzz)).yzw;
    // dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r1.yzw, r1.yyyy, v5.xxyz
    r1.yzw = ((r1.yyyy)*(v5.xxyz)).yzw;
    // dp3 r2.w, r0.yzwy, r1.yzwy
    r2.w = (dot((r0.yzwy).xyz,(r1.yzwy).xyz).xxxx).w;
    // mul r4.xyz, r0.yzwy, r2.wwww
    r4.xyz = ((r0.yzwy)*(r2.wwww)).xyz;
    // mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.yzwy
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.yzwy))).xyz;
    // add r3.zw, r4.xxxy, -v2.xxxy
    r3.zw = ((r4.xxxy)+(-(v2.xxxy))).zw;
    // mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 0.050000, 0.050000), v2.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,0.050000,0.050000))+(v2.xxxy)).zw;
    // mul r3.zw, r3.zzzw, cb0[11].wwww
    r3.zw = ((r3.zzzw)*(source[11].wwww)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r3.zwzz, t3.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, r5.xyzx, cb0[6].xyzx
    r5.xyz = ((r5.xyzx)*(source[6].xyzx)).xyz;
    // mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // add r6.xyz, r2.xyzx, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r6.xyz = ((r2.xyzx)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r6.xyz, cb0[11].xxxx, r6.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((source[11].xxxx)*(r6.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // dp3 r1.x, r6.xyzx, r1.yzwy
    r1.x = (dot((r6.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // mul r3.zw, r6.xxxy, r1.xxxx
    r3.zw = ((r6.xxxy)*(r1.xxxx)).zw;
    // mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), -r1.yyyz
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(-(r1.yyyz))).zw;
    // dp3 r1.x, r2.xyzx, r1.yzwy
    r1.x = (dot((r2.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // mul r1.yz, r2.xxyx, cb0[10].yyyy
    r1.yz = ((r2.xxyx)*(source[10].yyyy)).yz;
    // add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mad r1.x, -r1.x, l(0.500000), l(1.000000)
    r1.x = ((-(r1.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r2.xy, r3.zwzz, cb0[5].xyxx
    r2.xy = ((r3.zwzz)*(source[5].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xyz, r2.xyzx, cb0[4].xyzx
    r2.xyz = ((r2.xyzx)*(source[4].xyzx)).xyz;
    // log r1.w, |r1.x|
    r1.w = (log2(abs(r1.xxxx))).w;
    // lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // mul r1.w, r1.w, cb0[11].y
    r1.w = ((r1.wwww)*(source[11].yyyy)).w;
    // exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // mul r1.w, r1.w, cb0[11].z
    r1.w = ((r1.wwww)*(source[11].zzzz)).w;
    // movc r1.x, r1.x, l(0), r1.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).x;
    // mad r2.xyz, r1.xxxx, r2.xyzx, r5.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)+(r5.xyzx)).xyz;
    // add r5.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r1.xw, r3.xxxy, cb0[8].xxxy
    r1.xw = ((r3.xxxy)*(source[8].xxxy)).xw;
    // mad r3.xy, r3.xyxx, cb0[8].xyxx, r1.yzyy
    r3.xy = ((r3.xyxx)*(source[8].xyxx)+(r1.yzyy)).xy;
    // mad r1.xy, r1.xwxx, l(2.000000, 2.000000, 0.000000, 0.000000), r1.yzyy
    r1.xy = ((r1.xwxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r1.yzyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t4.xyzw, s5, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r1.xyz, -r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), r1.xyzx
    r1.xyz = ((-(r3.xyzx))*(float4(2.000000,2.000000,2.000000,0.000000))+(r1.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)+(r3.xyzx)).xyz;
    // mad r1.xyz, r0.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r3.xyz, -r1.xyzx, r1.wwww
    r3.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // mad r1.xyz, cb0[12].wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((source[12].wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, r5.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r1.w, v3.xyzx, v3.xyzx
    r1.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v3.xyzx
    r2.xyz = ((r1.wwww)*(v3.xyzx)).xyz;
    // dp3_sat r0.y, r0.yzwy, r2.xyzx
    r0.y = (saturate(dot((r0.yzwy).xyz,(r2.xyzx).xyz).xxxx)).y;
    // dp3_sat r0.z, r4.xyzx, r2.xyzx
    r0.z = (saturate(dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx)).z;
    // lt r0.w, r0.y, l(0.000001)
    r0.w = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // log r0.w, r0.z
    r0.w = (log2(r0.zzzz)).w;
    // lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // mul r0.w, r0.w, cb0[13].y
    r0.w = ((r0.wwww)*(source[13].yyyy)).w;
    // exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // mad r2.xyz, cb0[13].xxxx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((source[13].xxxx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r2.xyz, r0.zzzz, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)).xyz;
    // mad r0.yzw, r1.xxyz, r0.yyyy, r2.xxyz
    r0.yzw = ((r1.xxyz)*(r0.yyyy)+(r2.xxyz)).yzw;
    // mul o0.xyz, r0.yzwy, cb0[15].xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[15].xyzx)).xyz;
    // mul r0.yzw, v6.yyyy, cb1[1].xxyw
    r0.yzw = ((v6.yyyy)*(projection[1].xxyw)).yzw;
    // mad r0.yzw, cb1[0].xxyw, v6.xxxx, r0.yyzw
    r0.yzw = ((projection[0].xxyw)*(v6.xxxx)+(r0.yyzw)).yzw;
    // mad r0.yzw, cb1[2].xxyw, v6.zzzz, r0.yyzw
    r0.yzw = ((projection[2].xxyw)*(v6.zzzz)+(r0.yyzw)).yzw;
    // mad r0.yzw, cb1[3].xxyw, v6.wwww, r0.yyzw
    r0.yzw = ((projection[3].xxyw)*(v6.wwww)+(r0.yyzw)).yzw;
    // div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t6.yxzw, s0, l(0.000000)
    r0.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.yzyy).xy,0.f).rrrr).yxzw).y;
    // min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // mad r0.z, r0.y, cb2[1].z, -cb2[1].w
    r0.z = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).z;
    // mad r0.y, r0.y, cb2[1].x, cb2[1].y
    r0.y = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // add r0.y, -r0.w, r0.y
    r0.y = ((-(r0.wwww))+(r0.yyyy)).y;
    // add r0.z, -cb0[13].z, l(1.000000)
    r0.z = ((-(source[13].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // mul r0.y, r0.y, cb0[13].w
    r0.y = ((r0.yyyy)*(source[13].wwww)).y;
    // log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // mul r0.z, r0.z, cb0[14].x
    r0.z = ((r0.zzzz)*(source[14].xxxx)).z;
    // exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // add r0.z, -r0.y, l(1.000000)
    r0.z = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mad r0.x, r0.x, r0.z, r0.y
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.yyyy)).x;
    // mul o0.w, r0.x, cb0[0].w
    output.targets[0].w = ((r0.xxxx)*(source[0].wwww)).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 5955baa70cf4d048a51cd11a57d9a23e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect43(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[12]=0.f;
    source[12]=float4(input.lightColor,1.f);
    source[10].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f,r10=0.f;
    // add r0.xy, v7.xyxx, cb0[0].xyxx
    r0.xy = ((v7.xyxx)+(source[0].xyxx)).xy;
    // mul r0.xy, r0.xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // mad r0.zw, cb0[10].xxxx, l(0.000000, 0.000000, 0.090000, 0.090000), r0.xxxy
    r0.zw = ((source[10].xxxx)*(float4(0.000000,0.000000,0.090000,0.090000))+(r0.xxxy)).zw;
    // mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 6.283185, 6.283185)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,6.283185,6.283185))).zw;
    // sincos r0.z, null, r0.z
    r0.z = (sin(r0.zzzz)).z;
    // sincos null, r0.w, r0.w
    r0.w = (cos(r0.wwww)).w;
    // mad r1.y, r0.w, l(0.005000), r0.y
    r1.y = ((r0.wwww)*(float4(0.005000,0.005000,0.005000,0.005000))+(r0.yyyy)).y;
    // mad r1.x, r0.z, l(0.005000), r0.x
    r1.x = ((r0.zzzz)*(float4(0.005000,0.005000,0.005000,0.005000))+(r0.xxxx)).x;
    // mul r0.xyzw, cb0[1].zwzw, cb0[10].xxxx
    r0.xyzw = ((source[1].zwzw)*(source[10].xxxx)).xyzw;
    // mul r2.xyzw, r0.zwzw, l(1.000000, 0.000000, -1.000000, 0.000000)
    r2.xyzw = ((r0.zwzw)*(float4(1.000000,0.000000,-1.000000,0.000000))).xyzw;
    // mul r0.xyzw, r0.xyzw, l(0.000000, 1.000000, 0.000000, -1.000000)
    r0.xyzw = ((r0.xyzw)*(float4(0.000000,1.000000,0.000000,-1.000000))).xyzw;
    // mad r1.zw, r1.xxxy, cb0[1].xxxy, r2.xxxy
    r1.zw = ((r1.xxxy)*(source[1].xxxy)+(r2.xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.zwzz, t0.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mad r4.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mad r3.xyzw, r3.xyzw, l(2.000000, 2.000000, 2.000000, 1.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r3.xyzw = ((r3.xyzw)*(float4(2.000000,2.000000,2.000000,1.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyzw;
    // mul r1.zw, r1.xxxy, cb0[1].xxxy
    r1.zw = ((r1.xxxy)*(source[1].xxxy)).zw;
    // mad r2.xy, r1.zwzz, l(1.300000, 1.300000, 0.000000, 0.000000), r2.zwzz
    r2.xy = ((r1.zwzz)*(float4(1.300000,1.300000,0.000000,0.000000))+(r2.zwzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mad r5.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mad r2.xyzw, r2.xyzw, l(2.000000, 2.000000, 2.000000, 1.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r2.xyzw = ((r2.xyzw)*(float4(2.000000,2.000000,2.000000,1.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyzw;
    // mad r2.xyzw, r2.xyzw, l(0.875000, 0.875000, 0.875000, 0.875000), r3.xyzw
    r2.xyzw = ((r2.xyzw)*(float4(0.875000,0.875000,0.875000,0.875000))+(r3.xyzw)).xyzw;
    // mad r3.xyz, r5.xyzx, l(0.875000, 0.875000, 0.875000, 0.000000), r4.xyzx
    r3.xyz = ((r5.xyzx)*(float4(0.875000,0.875000,0.875000,0.000000))+(r4.xyzx)).xyz;
    // mad r0.zw, r1.zzzw, l(0.000000, 0.000000, 2.300000, 2.300000), r0.zzzw
    r0.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.300000,2.300000))+(r0.zzzw)).zw;
    // mad r0.xy, r1.zwzz, l(1.700000, 1.700000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r1.zwzz)*(float4(1.700000,1.700000,0.000000,0.000000))+(r0.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r0.zwzz, t1.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mad r5.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mad r4.xyzw, r4.xyzw, l(2.000000, 2.000000, 2.000000, 1.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyzw = ((r4.xyzw)*(float4(2.000000,2.000000,2.000000,1.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyzw;
    // mul r4.xyzw, r4.xyzw, l(0.669922, 0.669922, 0.669922, 0.669922)
    r4.xyzw = ((r4.xyzw)*(float4(0.669922,0.669922,0.669922,0.669922))).xyzw;
    // mul r5.xyz, r5.xyzx, l(0.669922, 0.669922, 0.669922, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.669922,0.669922,0.669922,0.000000))).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mad r7.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mad r6.xyzw, r6.xyzw, l(2.000000, 2.000000, 2.000000, 1.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r6.xyzw = ((r6.xyzw)*(float4(2.000000,2.000000,2.000000,1.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyzw;
    // mad r4.xyzw, r6.xyzw, l(0.765625, 0.765625, 0.765625, 0.765625), r4.xyzw
    r4.xyzw = ((r6.xyzw)*(float4(0.765625,0.765625,0.765625,0.765625))+(r4.xyzw)).xyzw;
    // add r4.xyzw, r2.xyzw, r4.xyzw
    r4.xyzw = ((r2.xyzw)+(r4.xyzw)).xyzw;
    // mad r5.xyz, r7.xyzx, l(0.765625, 0.765625, 0.765625, 0.000000), r5.xyzx
    r5.xyz = ((r7.xyzx)*(float4(0.765625,0.765625,0.765625,0.000000))+(r5.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)+(r5.xyzx)).xyz;
    // add r1.zw, r2.xxxy, r5.xxxy
    r1.zw = ((r2.xxxy)+(r5.xxxy)).zw;
    // mul r1.zw, r1.zzzw, cb0[2].xxxy
    r1.zw = ((r1.zzzw)*(source[2].xxxy)).zw;
    // mul r2.xyz, r3.xyzx, cb0[2].xyzx
    r2.xyz = ((r3.xyzx)*(source[2].xyzx)).xyz;
    // dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r3.xyz, r2.wwww, v6.xyzx
    r3.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // mad r2.w, -v6.z, r2.w, l(1.000000)
    r2.w = ((-(v6.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r5.xyz, r2.xyzx, r3.zzzz
    r5.xyz = ((r2.xyzx)*(r3.zzzz)).xyz;
    // mul r5.xyz, r5.xyzx, l(0.670000, 0.670000, 0.000000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.670000,0.670000,0.000000,0.000000))).xyz;
    // mad r2.xyz, r2.xyzx, l(0.330000, 0.330000, 1.000000, 0.000000), r5.xyzx
    r2.xyz = ((r2.xyzx)*(float4(0.330000,0.330000,1.000000,0.000000))+(r5.xyzx)).xyz;
    // add r2.xyz, r2.xyzx, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r2.xyz, v2.yyyy, r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((v2.yyyy)*(r2.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // dp3 r3.w, r2.xyzx, r2.xyzx
    r3.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r5.xyz, r2.xyzx, r3.wwww
    r5.xyz = ((r2.xyzx)*(r3.wwww)).xyz;
    // dp3 r3.w, r5.xyzx, r3.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r5.xy, r3.wwww, r5.xyxx
    r5.xy = ((r3.wwww)*(r5.xyxx)).xy;
    // mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r3.xyxx
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r3.xyxx))).xy;
    // dp2 r6.x, cb0[3].xyxx, r5.xyxx
    r6.x = (dot((source[3].xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // dp2 r6.y, cb0[4].xyxx, r5.xyxx
    r6.y = (dot((source[4].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // mad r5.xy, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r6.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r4.xyz, r4.xyzx, cb0[2].xyzx
    r4.xyz = ((r4.xyzx)*(source[2].xyzx)).xyz;
    // mul r3.w, r4.w, r4.w
    r3.w = ((r4.wwww)*(r4.wwww)).w;
    // mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // mul r6.xyz, r3.zzzz, r4.xyzx
    r6.xyz = ((r3.zzzz)*(r4.xyzx)).xyz;
    // mul r6.xyz, r6.xyzx, l(0.670000, 0.670000, 0.000000, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.670000,0.670000,0.000000,0.000000))).xyz;
    // mad r4.xyz, r4.xyzx, l(0.330000, 0.330000, 1.000000, 0.000000), r6.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.330000,0.330000,1.000000,0.000000))+(r6.xyzx)).xyz;
    // add r4.xyz, r4.xyzx, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r4.xyz, v2.yyyy, r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((v2.yyyy)*(r4.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // mul r5.zw, r4.xxxy, cb0[10].wwww
    r5.zw = ((r4.xxxy)*(source[10].wwww)).zw;
    // mad r5.zw, r5.xxxy, l(0.000000, 0.000000, 7.000000, 7.000000), r5.zzzw
    r5.zw = ((r5.xxxy)*(float4(0.000000,0.000000,7.000000,7.000000))+(r5.zzzw)).zw;
    // mad r5.xy, cb0[10].wwww, r4.xyxx, r5.xyxx
    r5.xy = ((source[10].wwww)*(r4.xyxx)+(r5.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r5.xyxx, t3.xyzw, s6, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterLookupSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // add_sat r6.xyz, r6.xyzx, cb0[11].wwww
    r6.xyz = (saturate((r6.xyzx)+(source[11].wwww))).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.zwzz, t2.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // add r4.w, r5.y, r5.x
    r4.w = ((r5.yyyy)+(r5.xxxx)).w;
    // add r4.w, r5.z, r4.w
    r4.w = ((r5.zzzz)+(r4.wwww)).w;
    // mad r4.w, r4.w, l(0.333330), l(-0.050000)
    r4.w = ((r4.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))+(float4(-0.050000,-0.050000,-0.050000,-0.050000))).w;
    // dp3_sat r4.z, cb0[5].xyzx, r4.xyzx
    r4.z = (saturate(dot((source[5].xyzx).xyz,(r4.xyzx).xyz).xxxx)).z;
    // mul r3.w, r3.w, r4.z
    r3.w = ((r3.wwww)*(r4.zzzz)).w;
    // mul_sat r3.w, r3.w, r4.w
    r3.w = (saturate((r3.wwww)*(r4.wwww))).w;
    // mad_sat r5.xyz, r3.wwww, cb0[11].zzzz, r6.xyzx
    r5.xyz = (saturate((r3.wwww)*(source[11].zzzz)+(r6.xyzx))).xyz;
    // mul r6.xyz, cb0[6].xyzx, cb0[6].wwww
    r6.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r7.xyz, -cb0[6].wwww, cb0[6].xyzx, r3.wwww
    r7.xyz = ((-(source[6].wwww))*(source[6].xyzx)+(r3.wwww)).xyz;
    // mad r6.xyz, r7.xyzx, l(0.660000, 0.660000, 0.660000, 0.000000), r6.xyzx
    r6.xyz = ((r7.xyzx)*(float4(0.660000,0.660000,0.660000,0.000000))+(r6.xyzx)).xyz;
    // dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r7.xyz, r3.wwww, v1.xyzx
    r7.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // mul r8.xyz, r3.wwww, v0.xyzx
    r8.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // mul r9.xyz, r9.xzyx, v1.wwww
    r9.xyz = ((r9.xzyx)*(v1.wwww)).xyz;
    // dp3 r10.y, r9.xzyx, r3.xyzx
    r10.y = (dot((r9.xzyx).xyz,(r3.xyzx).xyz).xxxx).y;
    // dp3 r10.x, r8.xyzx, r3.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // mov r9.x, r8.z
    r9.x = (r8.zzzz).x;
    // dp3 r10.z, r7.xyzx, r3.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // mov r9.z, r7.z
    r9.z = (r7.zzzz).z;
    // mul r3.xy, r1.zwzz, r3.zzzz
    r3.xy = ((r1.zwzz)*(r3.zzzz)).xy;
    // mul r3.xy, r3.xyxx, l(0.670000, 0.670000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(0.670000,0.670000,0.000000,0.000000))).xy;
    // mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.330000, 0.330000), r3.xxxy
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.330000,0.330000))+(r3.xxxy)).zw;
    // mul r1.zw, r1.zzzw, v2.yyyy
    r1.zw = ((r1.zzzw)*(v2.yyyy)).zw;
    // mul r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.025000, 0.025000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.025000,0.025000))).zw;
    // dp3 r3.x, r9.xyzx, r10.xyzx
    r3.x = (dot((r9.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // mul r3.xyz, r9.xyzx, r3.xxxx
    r3.xyz = ((r9.xyzx)*(r3.xxxx)).xyz;
    // mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // mov r3.w, -r3.x
    r3.w = (-(r3.xxxx)).w;
    // dp2 r3.x, r3.ywyy, r3.ywyy
    r3.x = (dot((r3.ywyy).xy,(r3.ywyy).xy).xxxx).x;
    // sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // div r3.xy, r3.ywyy, r3.xxxx
    r3.xy = ((r3.ywyy)/(r3.xxxx)).xy;
    // mad r3.z, -r3.z, l(0.250000), l(0.250000)
    r3.z = ((-(r3.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // mad r3.xy, r3.zzzz, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r3.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), r3.xyxx
    r3.xy = ((r4.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(r3.xyxx)).xy;
    // add r3.xy, r3.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // dp2 r4.x, cb0[7].xyxx, r3.xyxx
    r4.x = (dot((source[7].xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // dp2 r4.y, cb0[8].xyxx, r3.xyxx
    r4.y = (dot((source[8].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // add r3.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r4.xyz, v7.yyyy, cb1[1].xywx
    r4.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // mad r4.xyz, cb1[0].xywx, v7.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v7.xxxx)+(r4.xyzx)).xyz;
    // mad r4.xyz, cb1[2].xywx, v7.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v7.zzzz)+(r4.xyzx)).xyz;
    // mad r4.xyz, cb1[3].xywx, v7.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v7.wwww)+(r4.xyzx)).xyz;
    // div r3.zw, r4.xxxy, r4.zzzz
    r3.zw = ((r4.xxxy)/(r4.zzzz)).zw;
    // mad r3.zw, r3.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r3.zw = ((r3.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // mul r4.xy, r3.zwzz, l(512.000000, 512.000000, 0.000000, 0.000000)
    r4.xy = ((r3.zwzz)*(float4(512.000000,512.000000,0.000000,0.000000))).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r3.z, r3.zwzz, t5.yzxw, s0, l(0.000000)
    r3.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r3.zwzz).xy,0.f).rrrr).yzxw).z;
    // min r3.z, r3.z, l(0.999000)
    r3.z = (min(r3.zzzz,float4(0.999000,0.999000,0.999000,0.999000))).z;
    // deriv_rtx_coarse r7.xy, r4.xyxx
    r7.xy = (ddx_coarse(r4.xyxx)).xy;
    // deriv_rty_coarse r4.xy, r4.xyxx
    r4.xy = (ddy_coarse(r4.xyxx)).xy;
    // dp2 r3.w, r4.xyxx, r4.xyxx
    r3.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // dp2 r4.x, r7.xyxx, r7.xyxx
    r4.x = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // max r3.w, r3.w, r4.x
    r3.w = (max(r3.wwww,r4.xxxx)).w;
    // sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // sample_l_indexable(texture2d)(float,float,float,float) r3.xyw, r3.xyxx, t4.xywz, s7, r3.w
    r3.xyw = ((g_SourceCharacterTexture6.SampleLevel(SourceCharacterSampler, (r3.xyxx).xy, (r3.wwww).x)).xywz).xyw;
    // log r4.xyw, r3.xyxw
    r4.xyw = (log2(r3.xyxw)).xyw;
    // mul r7.xyz, r4.xywx, l(0.909091, 0.909091, 0.909091, 0.000000)
    r7.xyz = ((r4.xywx)*(float4(0.909091,0.909091,0.909091,0.000000))).xyz;
    // mul r4.xyw, r4.xyxw, l(1.100000, 1.100000, 0.000000, 1.100000)
    r4.xyw = ((r4.xyxw)*(float4(1.100000,1.100000,0.000000,1.100000))).xyw;
    // exp r4.xyw, r4.xyxw
    r4.xyw = (exp2(r4.xyxw)).xyw;
    // exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // mul r7.xyz, r7.xyzx, l(0.909091, 0.909091, 0.909091, 0.000000)
    r7.xyz = ((r7.xyzx)*(float4(0.909091,0.909091,0.909091,0.000000))).xyz;
    // mad r4.xyw, r4.xyxw, l(1.100000, 1.100000, 0.000000, 1.100000), r7.xyxz
    r4.xyw = ((r4.xyxw)*(float4(1.100000,1.100000,0.000000,1.100000))+(r7.xyxz)).xyw;
    // add r3.xyw, r3.xyxw, r4.xyxw
    r3.xyw = ((r3.xyxw)+(r4.xyxw)).xyw;
    // mov_sat r4.x, r2.w
    r4.x = (saturate(r2.wwww)).x;
    // mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // mul r4.xyw, r3.xyxw, r4.xxxx
    r4.xyw = ((r3.xyxw)*(r4.xxxx)).xyw;
    // mul r4.xyw, r4.xyxw, l(0.629999, 0.629999, 0.000000, 0.629999)
    r4.xyw = ((r4.xyxw)*(float4(0.629999,0.629999,0.000000,0.629999))).xyw;
    // mad r3.xyw, r3.xyxw, l(0.070000, 0.070000, 0.000000, 0.070000), r4.xyxw
    r3.xyw = ((r3.xyxw)*(float4(0.070000,0.070000,0.000000,0.070000))+(r4.xyxw)).xyw;
    // mul r3.xyw, r3.xyxw, r3.xyxw
    r3.xyw = ((r3.xyxw)*(r3.xyxw)).xyw;
    // mul r4.xyw, cb0[9].xyxz, cb0[9].wwww
    r4.xyw = ((source[9].xyxz)*(source[9].wwww)).xyw;
    // mul r3.xyw, r3.xyxw, r4.xyxw
    r3.xyw = ((r3.xyxw)*(r4.xyxw)).xyw;
    // mad r3.xyw, r5.xyxz, r6.xyxz, r3.xyxw
    r3.xyw = ((r5.xyxz)*(r6.xyxz)+(r3.xyxw)).xyw;
    // dp3 r4.x, v4.xyzx, v4.xyzx
    r4.x = (dot((v4.xyzx).xyz,(v4.xyzx).xyz).xxxx).x;
    // rsq r4.x, r4.x
    r4.x = (rsqrt(r4.xxxx)).x;
    // mul r4.xyw, r4.xxxx, v4.xyxz
    r4.xyw = ((r4.xxxx)*(v4.xyxz)).xyw;
    // dp3 r2.x, r2.xyzx, r4.xywx
    r2.x = (dot((r2.xyzx).xyz,(r4.xywx).xyz).xxxx).x;
    // max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // mul r2.xyz, r2.xxxx, cb2[3].xyzx
    r2.xyz = ((r2.xxxx)*(passValues[3].xyzx)).xyz;
    // mad r2.xyz, r3.xywx, cb2[3].wwww, r2.xyzx
    r2.xyz = ((r3.xywx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // mul o0.xyz, r2.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[12].xyzx)).xyz;
    // mul r2.x, |r2.w|, |r2.w|
    r2.x = ((abs(r2.wwww))*(abs(r2.wwww))).x;
    // lt r2.y, |r2.w|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // movc r2.x, r2.y, l(0), r2.x
    r2.x = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // mad r2.y, r3.z, cb2[1].z, -cb2[1].w
    r2.y = ((r3.zzzz)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // mad r2.z, r3.z, cb2[1].x, cb2[1].y
    r2.z = ((r3.zzzz)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.y
    r2.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.yyyy)).y;
    // add r2.y, r2.y, r2.z
    r2.y = ((r2.yyyy)+(r2.zzzz)).y;
    // add r2.y, -r4.z, r2.y
    r2.y = ((-(r4.zzzz))+(r2.yyyy)).y;
    // add r2.z, -cb0[10].y, l(1.000000)
    r2.z = ((-(source[10].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // max r2.z, -r2.z, l(0.001000)
    r2.z = (max(-(r2.zzzz),float4(0.001000,0.001000,0.001000,0.001000))).z;
    // div_sat r2.z, r2.y, r2.z
    r2.z = (saturate((r2.yyyy)/(r2.zzzz))).z;
    // mul_sat r3.xyz, r2.yyyy, l(0.034483, 0.020408, 0.005025, 0.000000)
    r3.xyz = (saturate((r2.yyyy)*(float4(0.034483,0.020408,0.005025,0.000000)))).xyz;
    // add r2.x, r2.z, r2.x
    r2.x = ((r2.zzzz)+(r2.xxxx)).x;
    // mad r0.xy, r0.xyxx, l(0.300000, 0.340000, 0.000000, 0.000000), r1.zwzz
    r0.xy = ((r0.xyxx)*(float4(0.300000,0.340000,0.000000,0.000000))+(r1.zwzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.yzw, r0.xyxx, t7.wxyz, s4, l(0.000000)
    r2.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // mad r0.xy, r0.zwzz, l(0.700000, 0.500000, 0.000000, 0.000000), r1.zwzz
    r0.xy = ((r0.zwzz)*(float4(0.700000,0.500000,0.000000,0.000000))+(r1.zwzz)).xy;
    // mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 6.000000, 6.000000), r1.zzzw
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,6.000000,6.000000))+(r1.zzzw)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.zwzz, t6.xyzw, s3, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s4, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // add r0.xyz, r0.xyzx, r2.yzwy
    r0.xyz = ((r0.xyzx)+(r2.yzwy)).xyz;
    // add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // add r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)+(r0.xxxx)).x;
    // mul r0.x, r0.x, v2.x
    r0.x = ((r0.xxxx)*(v2.xxxx)).x;
    // add r0.yz, -r3.yyzy, l(0.000000, 1.000000, 1.000000, 0.000000)
    r0.yz = ((-(r3.yyzy))+(float4(0.000000,1.000000,1.000000,0.000000))).yz;
    // mul r0.yz, r0.yyzy, r0.yyzy
    r0.yz = ((r0.yyzy)*(r0.yyzy)).yz;
    // mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // add r0.z, r1.y, r1.x
    r0.z = ((r1.yyyy)+(r1.xxxx)).z;
    // add r0.z, r1.z, r0.z
    r0.z = ((r1.zzzz)+(r0.zzzz)).z;
    // mul r0.xw, r0.xxxz, l(0.333330, 0.000000, 0.000000, 0.333330)
    r0.xw = ((r0.xxxz)*(float4(0.333330,0.000000,0.000000,0.333330))).xw;
    // mad_sat r0.z, r0.z, l(0.083333), r3.x
    r0.z = (saturate((r0.zzzz)*(float4(0.083333,0.083333,0.083333,0.083333))+(r3.xxxx))).z;
    // mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // mad_sat r0.x, r0.y, l(2.500000), r0.x
    r0.x = (saturate((r0.yyyy)*(float4(2.500000,2.500000,2.500000,2.500000))+(r0.xxxx))).x;
    // log r0.y, r0.z
    r0.y = (log2(r0.zzzz)).y;
    // lt r0.z, r0.z, l(0.000001)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // mul r0.y, r0.y, l(50.000000)
    r0.y = ((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000))).y;
    // exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // mad r0.x, r2.x, r0.y, r0.x
    r0.x = ((r2.xxxx)*(r0.yyyy)+(r0.xxxx)).x;
    // min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r0.x, r0.x, v2.w
    r0.x = ((r0.xxxx)*(v2.wwww)).x;
    // mul o0.w, r0.x, cb0[0].w
    output.targets[0].w = ((r0.xxxx)*(source[0].wwww)).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 7cbb3ce02becff45bd1951283545399a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect44(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;
    source[7]=0.f;
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[6]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v3.xyzx
    r1.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r2.xy, r2.xyxx, cb0[4].xxxx
    r2.xy = ((r2.xyxx)*(source[4].xxxx)).xy;
    // mul r3.xy, r2.xyxx, v0.wwww
    r3.xy = ((r2.xyxx)*(v0.wwww)).xy;
    // dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r2.xyw, r3.xyxz, r1.wwww
    r2.xyw = ((r3.xyxz)/(r1.wwww)).xyw;
    // dp3 r1.w, r2.xywx, r2.xywx
    r1.w = (dot((r2.xywx).xyz,(r2.xywx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, r2.xywx
    r3.xyz = ((r1.wwww)*(r2.xywx)).xyz;
    // dp3 r1.w, r3.xyzx, r0.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // mul r3.xy, r1.wwww, r3.xyxx
    r3.xy = ((r1.wwww)*(r3.xyxx)).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r0.xyxx
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r0.xyxx))).xy;
    // ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].y
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].yyyy)) * 0xffffffffu)).w;
    // if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // mul r4.xyzw, v6.yyyy, cb0[8].xyzw
    r4.xyzw = ((v6.yyyy)*(source[8].xyzw)).xyzw;
    // mad r4.xyzw, cb0[7].xyzw, v6.xxxx, r4.xyzw
    r4.xyzw = ((source[7].xyzw)*(v6.xxxx)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[9].xyzw, v6.zzzz, r4.xyzw
    r4.xyzw = ((source[9].xyzw)*(v6.zzzz)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[10].xyzw, v6.wwww, r4.xyzw
    r4.xyzw = ((source[10].xyzw)*(v6.wwww)+(r4.xyzw)).xyzw;
    // div r3.zw, r4.xxxy, r4.wwww
    r3.zw = ((r4.xxxy)/(r4.wwww)).zw;
    // sample_indexable(texture2d)(float,float,float,float) r5.x, r3.zwzz, t1.xyzw, s3
    r5.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r3.zwzz).xy,0.f).rrrr).xyzw).x;
    // mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r6.yz, cb0[11].wwzw
    r6.yz = (source[11].wwzw).yz;
    // add r6.xyzw, r3.zwzw, r6.xyzw
    r6.xyzw = ((r3.zwzw)+(r6.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r5.y, r6.xyxx, t1.yxzw, s3
    r5.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r5.z, r6.zwzz, t1.yzxw, s3
    r5.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r4.xy, r3.zwzz, cb0[11].zwzz
    r4.xy = ((r3.zwzz)+(source[11].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r5.w, r4.xyxx, t1.yzwx, s3
    r5.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r4.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r4.xyzw, r4.zzzz, r5.xyzw
    r4.xyzw = (asfloat((uint4)((r4.zzzz)<(r5.xyzw)) * 0xffffffffu)).xyzw;
    // and r5.xyzw, r4.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r5.xyzw = (asfloat(asuint(r4.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r3.zw, r3.zzzw, cb0[11].xxxy
    r3.zw = ((r3.zzzw)*(source[11].xxxy)).zw;
    // frc r3.zw, r3.zzzw
    r3.zw = (frac(r3.zzzw)).zw;
    // movc r4.xy, r4.xyxx, l(-1.000000,-1.000000,0,0), l(-0.000000,-0.000000,0,0)
    r4.xy = ((asuint(r4.xyxx) != 0u) ? (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u))) : (float4(-0.000000,-0.000000,asfloat(0u),asfloat(0u)))).xy;
    // add r4.xy, r4.xyxx, r5.zwzz
    r4.xy = ((r4.xyxx)+(r5.zwzz)).xy;
    // mad r4.xy, r3.zzzz, r4.xyxx, r5.xyxx
    r4.xy = ((r3.zzzz)*(r4.xyxx)+(r5.xyxx)).xy;
    // add r1.w, -r4.x, r4.y
    r1.w = ((-(r4.xxxx))+(r4.yyyy)).w;
    // mad r1.w, r3.w, r1.w, r4.x
    r1.w = ((r3.wwww)*(r1.wwww)+(r4.xxxx)).w;
    // mul r4.xyz, r1.wwww, cb0[12].xxxx
    r4.xyz = ((r1.wwww)*(source[12].xxxx)).xyz;
    // else
    } else {
    // mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v2.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v2.xyxx))).xy;
    // mad r3.xy, r3.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v2.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v2.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[1].xyzx
    r3.xyz = ((r3.xyzx)*(source[1].xyzx)).xyz;
    // mad r3.xyz, cb0[4].yyyy, r3.xyzx, r3.xyzx
    r3.xyz = ((source[4].yyyy)*(r3.xyzx)+(r3.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, -cb0[4].yyyy
    r3.xyz = ((r3.xyzx)+(-(source[4].yyyy))).xyz;
    // mov_sat r5.xyz, r3.xyzx
    r5.xyz = (saturate(r3.xyzx)).xyz;
    // mul r1.w, r2.z, cb0[4].z
    r1.w = ((r2.zzzz)*(source[4].zzzz)).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r7.xyz, cb0[2].xyzx, cb0[4].wwww
    r7.xyz = ((source[2].xyzx)*(source[4].wwww)).xyz;
    // mul r7.xyz, r6.xyzx, r7.xyzx
    r7.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // mad r5.xyz, r1.wwww, r5.xyzx, r7.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r7.xyzx)).xyz;
    // mov_sat r3.xyz, -r3.xyzx
    r3.xyz = (saturate(-(r3.xyzx))).xyz;
    // mad r3.xyz, -r1.wwww, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r1.wwww))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // dp3 r1.x, r2.xywx, r1.xyzx
    r1.x = (dot((r2.xywx).xyz,(r1.xyzx).xyz).xxxx).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // min r1.y, r1.x, l(1.000000)
    r1.y = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mul r5.xyz, cb0[3].xyzx, cb0[5].xxxx
    r5.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // mad r0.xyz, v3.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((v3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.x, r0.xyzx, r2.xywx
    r0.x = (dot((r0.xyzx).xyz,(r2.xywx).xyz).xxxx).x;
    // lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // mul r0.x, r0.x, cb0[5].y
    r0.x = ((r0.xxxx)*(source[5].yyyy)).x;
    // exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // mul r0.xyz, r5.xyzx, r0.xxxx
    r0.xyz = ((r5.xyzx)*(r0.xxxx)).xyz;
    // mad r0.xyz, r1.yyyy, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.yyyy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // mul r1.xyz, r1.xxxx, cb2[3].xyzx
    r1.xyz = ((r1.xxxx)*(passValues[3].xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[6].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[6].xyzx)).xyz;
    // mul_sat r0.x, r6.w, cb0[5].z
    r0.x = (saturate((r6.wwww)*(source[5].zzzz))).x;
    // mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// bd2d9d76c2e71846a4c52e67e1996291
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect45(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[5]=0.f;
    source[6]=0.f;
    source[7]=0.f;
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[5]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].yyyy)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // mul r0.xyzw, v6.yyyy, cb0[7].xyzw
    r0.xyzw = ((v6.yyyy)*(source[7].xyzw)).xyzw;
    // mad r0.xyzw, cb0[6].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[6].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[8].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[8].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[9].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[9].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t1.xyzw, s3
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r2.yz, cb0[10].wwzw
    r2.yz = (source[10].wwzw).yz;
    // add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s3
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s3
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r2.xy, r0.xyxx, cb0[10].zwzz
    r2.xy = ((r0.xyxx)+(source[10].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t1.yzwx, s3
    r1.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r0.xy, r0.xyxx, cb0[10].xyxx
    r0.xy = ((r0.xyxx)*(source[10].xyxx)).xy;
    // frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, cb0[11].xxxx
    r0.xyz = ((r0.xxxx)*(source[11].xxxx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r0.w, -v0.w, l(1.000000)
    r0.w = ((-(v0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // add r0.w, r0.w, cb0[4].y
    r0.w = ((r0.wwww)+(source[4].yyyy)).w;
    // add_sat r0.w, r0.w, cb0[4].x
    r0.w = (saturate((r0.wwww)+(source[4].xxxx))).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // dp3 r1.y, v3.xyzx, v3.xyzx
    r1.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r1.yzw, r1.yyyy, v3.xxyz
    r1.yzw = ((r1.yyyy)*(v3.xxyz)).yzw;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t3.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, cb0[1].xyzx, cb0[3].yyyy
    r3.xyz = ((source[1].xyzx)*(source[3].yyyy)).xyz;
    // mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r2.w, r3.xyxx, r3.xyxx
    r2.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // add r4.z, r2.w, l(0.000010)
    r4.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r3.xy, r3.xyxx, cb0[3].xxxx
    r3.xy = ((r3.xyxx)*(source[3].xxxx)).xy;
    // mul r4.xy, r3.xyxx, v0.wwww
    r4.xy = ((r3.xyxx)*(v0.wwww)).xy;
    // dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // div r3.xyz, r4.xyzx, r2.wwww
    r3.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // dp3 r2.w, r3.xyzx, r1.yzwy
    r2.w = (dot((r3.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, cb0[2].xyzx, cb0[3].zzzz
    r5.xyz = ((source[2].xyzx)*(source[3].zzzz)).xyz;
    // mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r1.xyz, v5.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((v5.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // mul r1.x, r1.x, cb0[3].w
    r1.x = ((r1.xxxx)*(source[3].wwww)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mul r1.xyz, r4.xyzx, r1.xxxx
    r1.xyz = ((r4.xyzx)*(r1.xxxx)).xyz;
    // mad r1.xyz, r3.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // mul r2.xyz, r2.wwww, cb2[3].xyzx
    r2.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// e8197db6330e7c4f8eb9568ec71f5ae7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect46(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;
    source[7]=0.f;
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[6]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f;
    // dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v3.zxyz
    r1.xyz = ((r0.wwww)*(v3.zxyz)).xyz;
    // ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].y
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].yyyy)) * 0xffffffffu)).w;
    // if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // mul r2.xyzw, v6.yyyy, cb0[8].xyzw
    r2.xyzw = ((v6.yyyy)*(source[8].xyzw)).xyzw;
    // mad r2.xyzw, cb0[7].xyzw, v6.xxxx, r2.xyzw
    r2.xyzw = ((source[7].xyzw)*(v6.xxxx)+(r2.xyzw)).xyzw;
    // mad r2.xyzw, cb0[9].xyzw, v6.zzzz, r2.xyzw
    r2.xyzw = ((source[9].xyzw)*(v6.zzzz)+(r2.xyzw)).xyzw;
    // mad r2.xyzw, cb0[10].xyzw, v6.wwww, r2.xyzw
    r2.xyzw = ((source[10].xyzw)*(v6.wwww)+(r2.xyzw)).xyzw;
    // div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t1.xyzw, s2
    r3.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r4.yz, cb0[11].wwzw
    r4.yz = (source[11].wwzw).yz;
    // add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t1.yxzw, s2
    r3.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r4.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t1.yzxw, s2
    r3.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r4.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r4.xy, r2.xyxx, cb0[11].zwzz
    r4.xy = ((r2.xyxx)+(source[11].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t1.yzwx, s2
    r3.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r4.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r2.xy, r2.xyxx, cb0[11].xyxx
    r2.xy = ((r2.xyxx)*(source[11].xyxx)).xy;
    // frc r2.xy, r2.xyxx
    r2.xy = (frac(r2.xyxx)).xy;
    // movc r2.zw, r3.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r2.zw = ((asuint(r3.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r2.zw, r2.zzzw, r4.zzzw
    r2.zw = ((r2.zzzw)+(r4.zzzw)).zw;
    // mad r2.xz, r2.xxxx, r2.zzwz, r4.xxyx
    r2.xz = ((r2.xxxx)*(r2.zzwz)+(r4.xxyx)).xz;
    // add r1.w, -r2.x, r2.z
    r1.w = ((-(r2.xxxx))+(r2.zzzz)).w;
    // mad r1.w, r2.y, r1.w, r2.x
    r1.w = ((r2.yyyy)*(r1.wwww)+(r2.xxxx)).w;
    // mul r2.xyz, r1.wwww, cb0[12].xxxx
    r2.xyz = ((r1.wwww)*(source[12].xxxx)).xyz;
    // else
    } else {
    // mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v2.xyxx, t2.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.xyz, cb0[1].xyzx, cb0[4].zzzz
    r4.xyz = ((source[1].xyzx)*(source[4].zzzz)).xyz;
    // mul r4.xyz, r3.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    // mul r6.xy, r5.xyxx, v0.wwww
    r6.xy = ((r5.xyxx)*(v0.wwww)).xy;
    // dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.y, r5.zxyz, r1.xyzx
    r1.y = (dot((r5.zxyz).xyz,(r1.xyzx).xyz).xxxx).y;
    // max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // min r1.z, r1.y, l(1.000000)
    r1.z = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mul r6.xyz, cb0[2].xyzx, cb0[4].wwww
    r6.xyz = ((source[2].xyzx)*(source[4].wwww)).xyz;
    // mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // mad r6.xyz, v3.xyzx, r0.wwww, r0.xyzx
    r6.xyz = ((v3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r6.xyz, r6.xyzx, r0.wwww
    r6.xyz = ((r6.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // mul r0.w, r0.w, cb0[5].x
    r0.w = ((r0.wwww)*(source[5].xxxx)).w;
    // exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // mul r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // mad r3.xyz, r1.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // mov_sat r1.x, -r1.x
    r1.x = (saturate(-(r1.xxxx))).x;
    // mul r4.xyz, cb0[3].xyzx, cb0[5].zzzz
    r4.xyz = ((source[3].xyzx)*(source[5].zzzz)).xyz;
    // dp3 r0.x, r5.xyzx, r0.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // add r0.xw, -|r0.xxxz|, l(1.000000, 0.000000, 0.000000, 1.000000)
    r0.xw = ((-(abs(r0.xxxz)))+(float4(1.000000,0.000000,0.000000,1.000000))).xw;
    // mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // mul r0.x, r0.x, cb0[5].w
    r0.x = ((r0.xxxx)*(source[5].wwww)).x;
    // exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // mul r0.xzw, r4.xxyz, r0.xxxx
    r0.xzw = ((r4.xxyz)*(r0.xxxx)).xzw;
    // mul r0.xzw, r0.xxzw, r1.xxxx
    r0.xzw = ((r0.xxzw)*(r1.xxxx)).xzw;
    // movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // mul r1.xyz, r1.yyyy, cb2[3].xyzx
    r1.xyz = ((r1.yyyy)*(passValues[3].xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[6].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[6].xyzx)).xyz;
    // mul_sat r0.x, r3.w, cb0[5].y
    r0.x = (saturate((r3.wwww)*(source[5].yyyy))).x;
    // mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 7421b8eebd12b04cabf0c6f66494d95c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect48(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[6]=0.f;
    source[7]=0.f;
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[6]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v3.xyzx
    r1.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r2.xy, r2.xyxx, cb0[4].xxxx
    r2.xy = ((r2.xyxx)*(source[4].xxxx)).xy;
    // mul r3.xy, r2.xyxx, v0.wwww
    r3.xy = ((r2.xyxx)*(v0.wwww)).xy;
    // dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r2.xyw, r3.xyxz, r1.wwww
    r2.xyw = ((r3.xyxz)/(r1.wwww)).xyw;
    // dp3 r1.w, r2.xywx, r2.xywx
    r1.w = (dot((r2.xywx).xyz,(r2.xywx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, r2.xywx
    r3.xyz = ((r1.wwww)*(r2.xywx)).xyz;
    // dp3 r1.w, r3.xyzx, r0.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // mul r3.xy, r1.wwww, r3.xyxx
    r3.xy = ((r1.wwww)*(r3.xyxx)).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r0.xyxx
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r0.xyxx))).xy;
    // ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].y
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].yyyy)) * 0xffffffffu)).w;
    // if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // mul r4.xyzw, v6.yyyy, cb0[8].xyzw
    r4.xyzw = ((v6.yyyy)*(source[8].xyzw)).xyzw;
    // mad r4.xyzw, cb0[7].xyzw, v6.xxxx, r4.xyzw
    r4.xyzw = ((source[7].xyzw)*(v6.xxxx)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[9].xyzw, v6.zzzz, r4.xyzw
    r4.xyzw = ((source[9].xyzw)*(v6.zzzz)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[10].xyzw, v6.wwww, r4.xyzw
    r4.xyzw = ((source[10].xyzw)*(v6.wwww)+(r4.xyzw)).xyzw;
    // div r3.zw, r4.xxxy, r4.wwww
    r3.zw = ((r4.xxxy)/(r4.wwww)).zw;
    // sample_indexable(texture2d)(float,float,float,float) r5.x, r3.zwzz, t1.xyzw, s4
    r5.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r3.zwzz).xy,0.f).rrrr).xyzw).x;
    // mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r6.yz, cb0[11].wwzw
    r6.yz = (source[11].wwzw).yz;
    // add r6.xyzw, r3.zwzw, r6.xyzw
    r6.xyzw = ((r3.zwzw)+(r6.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r5.y, r6.xyxx, t1.yxzw, s4
    r5.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r5.z, r6.zwzz, t1.yzxw, s4
    r5.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r4.xy, r3.zwzz, cb0[11].zwzz
    r4.xy = ((r3.zwzz)+(source[11].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r5.w, r4.xyxx, t1.yzwx, s4
    r5.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r4.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r4.xyzw, r4.zzzz, r5.xyzw
    r4.xyzw = (asfloat((uint4)((r4.zzzz)<(r5.xyzw)) * 0xffffffffu)).xyzw;
    // and r5.xyzw, r4.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r5.xyzw = (asfloat(asuint(r4.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r3.zw, r3.zzzw, cb0[11].xxxy
    r3.zw = ((r3.zzzw)*(source[11].xxxy)).zw;
    // frc r3.zw, r3.zzzw
    r3.zw = (frac(r3.zzzw)).zw;
    // movc r4.xy, r4.xyxx, l(-1.000000,-1.000000,0,0), l(-0.000000,-0.000000,0,0)
    r4.xy = ((asuint(r4.xyxx) != 0u) ? (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u))) : (float4(-0.000000,-0.000000,asfloat(0u),asfloat(0u)))).xy;
    // add r4.xy, r4.xyxx, r5.zwzz
    r4.xy = ((r4.xyxx)+(r5.zwzz)).xy;
    // mad r4.xy, r3.zzzz, r4.xyxx, r5.xyxx
    r4.xy = ((r3.zzzz)*(r4.xyxx)+(r5.xyxx)).xy;
    // add r1.w, -r4.x, r4.y
    r1.w = ((-(r4.xxxx))+(r4.yyyy)).w;
    // mad r1.w, r3.w, r1.w, r4.x
    r1.w = ((r3.wwww)*(r1.wwww)+(r4.xxxx)).w;
    // mul r4.xyz, r1.wwww, cb0[12].xxxx
    r4.xyz = ((r1.wwww)*(source[12].xxxx)).xyz;
    // else
    } else {
    // mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v2.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v2.xyxx))).xy;
    // mad r3.xy, r3.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v2.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v2.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[1].xyzx
    r3.xyz = ((r3.xyzx)*(source[1].xyzx)).xyz;
    // mad r3.xyz, cb0[4].yyyy, r3.xyzx, r3.xyzx
    r3.xyz = ((source[4].yyyy)*(r3.xyzx)+(r3.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, -cb0[4].yyyy
    r3.xyz = ((r3.xyzx)+(-(source[4].yyyy))).xyz;
    // mov_sat r5.xyz, r3.xyzx
    r5.xyz = (saturate(r3.xyzx)).xyz;
    // mul r1.w, r2.z, cb0[4].z
    r1.w = ((r2.zzzz)*(source[4].zzzz)).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v2.xyxx, t4.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r7.xyz, cb0[2].xyzx, cb0[4].wwww
    r7.xyz = ((source[2].xyzx)*(source[4].wwww)).xyz;
    // mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // mad r5.xyz, r1.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // mov_sat r3.xyz, -r3.xyzx
    r3.xyz = (saturate(-(r3.xyzx))).xyz;
    // mad r3.xyz, -r1.wwww, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r1.wwww))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // dp3 r1.x, r2.xywx, r1.xyzx
    r1.x = (dot((r2.xywx).xyz,(r1.xyzx).xyz).xxxx).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // min r1.y, r1.x, l(1.000000)
    r1.y = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v2.xyxx, t2.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r6.xyz, cb0[3].xyzx, cb0[5].xxxx
    r6.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // mad r0.xyz, v3.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((v3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.x, r0.xyzx, r2.xywx
    r0.x = (dot((r0.xyzx).xyz,(r2.xywx).xyz).xxxx).x;
    // lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // mul r0.x, r0.x, cb0[5].y
    r0.x = ((r0.xxxx)*(source[5].yyyy)).x;
    // exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // mul r0.xyz, r5.xyzx, r0.xxxx
    r0.xyz = ((r5.xyzx)*(r0.xxxx)).xyz;
    // mad r0.xyz, r1.yyyy, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.yyyy)*(r3.xyzx)+(r0.xyzx)).xyz;
    // mul r1.xyz, r1.xxxx, cb2[3].xyzx
    r1.xyz = ((r1.xxxx)*(passValues[3].xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[6].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[6].xyzx)).xyz;
    // mul_sat r0.x, r6.w, cb0[5].z
    r0.x = (saturate((r6.wwww)*(source[5].zzzz))).x;
    // mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 2ed53d75b3a338459a62b60ee2ecfd25
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect49(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[13]=0.f;
    source[14]=0.f;
    source[8]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[14].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[14].yyyy)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // mul r0.xyzw, v6.yyyy, cb0[10].xyzw
    r0.xyzw = ((v6.yyyy)*(source[10].xyzw)).xyzw;
    // mad r0.xyzw, cb0[9].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[9].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[11].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[11].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[12].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[12].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t4.xyzw, s4
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r2.yz, cb0[13].wwzw
    r2.yz = (source[13].wwzw).yz;
    // add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t4.yxzw, s4
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t4.yzxw, s4
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r2.xy, r0.xyxx, cb0[13].zwzz
    r2.xy = ((r0.xyxx)+(source[13].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t4.yzwx, s4
    r1.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r0.xy, r0.xyxx, cb0[13].xyxx
    r0.xy = ((r0.xyxx)*(source[13].xyxx)).xy;
    // frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, cb0[14].xxxx
    r0.xyz = ((r0.xxxx)*(source[14].xxxx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r0.w, -v0.w, cb0[7].x
    r0.w = ((-(v0.wwww))+(source[7].xxxx)).w;
    // add r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)+(source[6].wwww)).w;
    // add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // dp3 r1.y, v3.xyzx, v3.xyzx
    r1.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r1.yzw, r1.yyyy, v3.xxyz
    r1.yzw = ((r1.yyyy)*(v3.xxyz)).yzw;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r3.xyz, cb0[1].xyzx, cb0[5].xxxx
    r3.xyz = ((source[1].xyzx)*(source[5].xxxx)).xyz;
    // mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // mul r5.xyz, cb0[2].xyzx, cb0[5].yyyy
    r5.xyz = ((source[2].xyzx)*(source[5].yyyy)).xyz;
    // mul r6.xy, v2.xyxx, cb0[4].yyyy
    r6.xy = ((v2.xyxx)*(source[4].yyyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // dp3 r3.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r5.xyz, -r5.xyzx, r7.xyzx, r3.wwww
    r5.xyz = ((-(r5.xyzx))*(r7.xyzx)+(r3.wwww)).xyz;
    // mad r5.xyz, cb0[5].wwww, r5.xyzx, r8.xyzx
    r5.xyz = ((source[5].wwww)*(r5.xyzx)+(r8.xyzx)).xyz;
    // max r3.w, cb0[4].w, l(0.000000)
    r3.w = (max(source[4].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r3.w, r3.w, l(0.990000)
    r3.w = (min(r3.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r4.w
    r4.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r4.wwww)).w;
    // add r5.w, -v0.x, l(1.000000)
    r5.w = ((-(v0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.zw, v2.xyxx, t0.zwxy, s0, l(0.000000)
    r6.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r6.zw, r6.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r6.zw = ((r6.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r8.x, r6.zwzz, r6.zwzz
    r8.x = (dot((r6.zwzz).xy,(r6.zwzz).xy).xxxx).x;
    // add r8.x, -r8.x, l(1.000000)
    r8.x = ((-(r8.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r8.x, r8.x, l(0.000000)
    r8.x = (max(r8.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r8.x, r8.x
    r8.x = (sqrt(r8.xxxx)).x;
    // add r8.z, r8.x, l(0.000010)
    r8.z = ((r8.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r6.zw, r6.zzzw, cb0[4].xxxx
    r6.zw = ((r6.zzzw)*(source[4].xxxx)).zw;
    // mul r8.xy, r6.zwzz, v0.wwww
    r8.xy = ((r6.zwzz)*(v0.wwww)).xy;
    // dp3 r6.z, r8.xyzx, r8.xyzx
    r6.z = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // sqrt r6.z, r6.z
    r6.z = (sqrt(r6.zzzz)).z;
    // div r8.xyz, r8.xyzx, r6.zzzz
    r8.xyz = ((r8.xyzx)/(r6.zzzz)).xyz;
    // mul r6.z, r8.z, r8.z
    r6.z = ((r8.zzzz)*(r8.zzzz)).z;
    // mul_sat r2.w, r2.w, r6.z
    r2.w = (saturate((r2.wwww)*(r6.zzzz))).w;
    // add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r6.z, r7.w, r7.w
    r6.z = ((r7.wwww)*(r7.wwww)).z;
    // mul r2.w, r2.w, r6.z
    r2.w = ((r2.wwww)*(r6.zzzz)).w;
    // mul r6.z, r2.w, r3.w
    r6.z = ((r2.wwww)*(r3.wwww)).z;
    // mad r5.w, r5.w, r6.z, r5.w
    r5.w = ((r5.wwww)*(r6.zzzz)+(r5.wwww)).w;
    // add r3.w, -r3.w, r5.w
    r3.w = ((-(r3.wwww))+(r5.wwww)).w;
    // mul r6.z, r3.w, r4.w
    r6.z = ((r3.wwww)*(r4.wwww)).z;
    // mad r3.w, -r4.w, r3.w, r5.w
    r3.w = ((-(r4.wwww))*(r3.wwww)+(r5.wwww)).w;
    // mad_sat r2.w, r2.w, r3.w, r6.z
    r2.w = (saturate((r2.wwww)*(r3.wwww)+(r6.zzzz))).w;
    // mad r3.xyz, -r2.xyzx, r3.xyzx, r5.xyzx
    r3.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r5.xyzx)).xyz;
    // mad r3.xyz, r2.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r6.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r3.w, r4.xyxx, r4.xyxx
    r3.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // add r5.z, r3.w, l(0.000010)
    r5.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r5.xy, r4.xyxx, cb0[4].zzzz
    r5.xy = ((r4.xyxx)*(source[4].zzzz)).xy;
    // mul r3.w, r2.w, l(0.650000)
    r3.w = ((r2.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // add r4.xyz, -r8.xyzx, r5.xyzx
    r4.xyz = ((-(r8.xyzx))+(r5.xyzx)).xyz;
    // mad r4.xyz, r3.wwww, r4.xyzx, r8.xyzx
    r4.xyz = ((r3.wwww)*(r4.xyzx)+(r8.xyzx)).xyz;
    // dp3 r3.w, r4.xyzx, r1.yzwy
    r3.w = (dot((r4.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r4.w, r3.w, l(1.000000)
    r4.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r5.xyz, cb0[3].xyzx, cb0[6].xxxx
    r5.xyz = ((source[3].xyzx)*(source[6].xxxx)).xyz;
    // mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // mad r5.xyz, cb0[6].yyyy, r7.xyzx, -r2.xyzx
    r5.xyz = ((source[6].yyyy)*(r7.xyzx)+(-(r2.xyzx))).xyz;
    // mad r2.xyz, r2.wwww, r5.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r5.xyzx)+(r2.xyzx)).xyz;
    // mad r1.xyz, v5.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((v5.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // mul r1.x, r1.x, cb0[6].z
    r1.x = ((r1.xxxx)*(source[6].zzzz)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mul r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)*(r1.xxxx)).xyz;
    // mad r1.xyz, r4.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r4.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // mul r2.xyz, r3.wwww, cb2[3].xyzx
    r2.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 80e1973ffc1ea8488260393067a803a7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect50(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[13]=0.f;
    source[14]=0.f;
    source[15]=0.f;
    source[9]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[15].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[15].yyyy)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // mul r0.xyzw, v6.yyyy, cb0[11].xyzw
    r0.xyzw = ((v6.yyyy)*(source[11].xyzw)).xyzw;
    // mad r0.xyzw, cb0[10].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[10].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[12].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[12].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[13].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[13].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t4.xyzw, s4
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r2.yz, cb0[14].wwzw
    r2.yz = (source[14].wwzw).yz;
    // add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t4.yxzw, s4
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t4.yzxw, s4
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r2.xy, r0.xyxx, cb0[14].zwzz
    r2.xy = ((r0.xyxx)+(source[14].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t4.yzwx, s4
    r1.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r0.xy, r0.xyxx, cb0[14].xyxx
    r0.xy = ((r0.xyxx)*(source[14].xyxx)).xy;
    // frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, cb0[15].xxxx
    r0.xyz = ((r0.xxxx)*(source[15].xxxx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r0.w, -v0.w, cb0[8].w
    r0.w = ((-(v0.wwww))+(source[8].wwww)).w;
    // add r0.w, r0.w, cb0[8].z
    r0.w = ((r0.wwww)+(source[8].zzzz)).w;
    // add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // dp3 r1.y, v3.xyzx, v3.xyzx
    r1.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r1.yzw, r1.yyyy, v3.xxyz
    r1.yzw = ((r1.yyyy)*(v3.xxyz)).yzw;
    // mul r2.xy, v2.xyxx, cb0[1].xyxx
    r2.xy = ((v2.xyxx)*(source[1].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r4.xyz, cb0[2].xyzx, cb0[6].wwww
    r4.xyz = ((source[2].xyzx)*(source[6].wwww)).xyz;
    // mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // mul r6.xyz, cb0[3].xyzx, cb0[7].xxxx
    r6.xyz = ((source[3].xyzx)*(source[7].xxxx)).xyz;
    // mul r2.zw, v2.xxxy, cb0[6].xxxx
    r2.zw = ((v2.xxxy)*(source[6].xxxx)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r2.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // dp3 r4.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r6.xyz, -r6.xyzx, r7.xyzx, r4.wwww
    r6.xyz = ((-(r6.xyzx))*(r7.xyzx)+(r4.wwww)).xyz;
    // mad r6.xyz, cb0[7].zzzz, r6.xyzx, r8.xyzx
    r6.xyz = ((source[7].zzzz)*(r6.xyzx)+(r8.xyzx)).xyz;
    // max r4.w, cb0[6].z, l(0.000000)
    r4.w = (max(source[6].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r4.w, r4.w, l(0.990000)
    r4.w = (min(r4.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // add r5.w, -r4.w, l(1.000000)
    r5.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // div r5.w, l(1.000000, 1.000000, 1.000000, 1.000000), r5.w
    r5.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r5.wwww)).w;
    // add r6.w, -v0.x, l(1.000000)
    r6.w = ((-(v0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r8.x, r2.xyxx, r2.xyxx
    r8.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // add r8.x, -r8.x, l(1.000000)
    r8.x = ((-(r8.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r8.x, r8.x, l(0.000000)
    r8.x = (max(r8.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r8.x, r8.x
    r8.x = (sqrt(r8.xxxx)).x;
    // add r8.z, r8.x, l(0.000010)
    r8.z = ((r8.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r2.xy, r2.xyxx, cb0[5].wwww
    r2.xy = ((r2.xyxx)*(source[5].wwww)).xy;
    // mul r8.xy, r2.xyxx, v0.wwww
    r8.xy = ((r2.xyxx)*(v0.wwww)).xy;
    // dp3 r2.x, r8.xyzx, r8.xyzx
    r2.x = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // div r8.xyz, r8.xyzx, r2.xxxx
    r8.xyz = ((r8.xyzx)/(r2.xxxx)).xyz;
    // mul r2.x, r8.z, r8.z
    r2.x = ((r8.zzzz)*(r8.zzzz)).x;
    // mul_sat r2.x, r2.x, r3.w
    r2.x = (saturate((r2.xxxx)*(r3.wwww))).x;
    // add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r2.y, r7.w, r7.w
    r2.y = ((r7.wwww)*(r7.wwww)).y;
    // mul r2.x, r2.y, r2.x
    r2.x = ((r2.yyyy)*(r2.xxxx)).x;
    // mul r2.y, r2.x, r4.w
    r2.y = ((r2.xxxx)*(r4.wwww)).y;
    // mad r2.y, r6.w, r2.y, r6.w
    r2.y = ((r6.wwww)*(r2.yyyy)+(r6.wwww)).y;
    // add r3.w, -r4.w, r2.y
    r3.w = ((-(r4.wwww))+(r2.yyyy)).w;
    // mul r4.w, r3.w, r5.w
    r4.w = ((r3.wwww)*(r5.wwww)).w;
    // mad r2.y, -r5.w, r3.w, r2.y
    r2.y = ((-(r5.wwww))*(r3.wwww)+(r2.yyyy)).y;
    // mad_sat r2.x, r2.x, r2.y, r4.w
    r2.x = (saturate((r2.xxxx)*(r2.yyyy)+(r4.wwww))).x;
    // mad r4.xyz, -r3.xyzx, r4.xyzx, r6.xyzx
    r4.xyz = ((-(r3.xyzx))*(r4.xyzx)+(r6.xyzx)).xyz;
    // mad r4.xyz, r2.xxxx, r4.xyzx, r5.xyzx
    r4.xyz = ((r2.xxxx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.yz, r2.zwzz, t1.zxyw, s1, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // dp2 r2.w, r2.yzyy, r2.yzyy
    r2.w = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).w;
    // add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // add r5.z, r2.w, l(0.000010)
    r5.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r5.xy, r2.yzyy, cb0[6].yyyy
    r5.xy = ((r2.yzyy)*(source[6].yyyy)).xy;
    // mul r2.y, r2.x, l(0.650000)
    r2.y = ((r2.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // add r5.xyz, -r8.xyzx, r5.xyzx
    r5.xyz = ((-(r8.xyzx))+(r5.xyzx)).xyz;
    // mad r2.yzw, r2.yyyy, r5.xxyz, r8.xxyz
    r2.yzw = ((r2.yyyy)*(r5.xxyz)+(r8.xxyz)).yzw;
    // dp3 r3.w, r2.yzwy, r1.yzwy
    r3.w = (dot((r2.yzwy).xyz,(r1.yzwy).xyz).xxxx).w;
    // max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r4.w, r3.w, l(1.000000)
    r4.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r5.xyz, cb0[4].xyzx, cb0[7].wwww
    r5.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // mad r5.xyz, cb0[8].xxxx, r7.xyzx, -r3.xyzx
    r5.xyz = ((source[8].xxxx)*(r7.xyzx)+(-(r3.xyzx))).xyz;
    // mad r3.xyz, r2.xxxx, r5.xyzx, r3.xyzx
    r3.xyz = ((r2.xxxx)*(r5.xyzx)+(r3.xyzx)).xyz;
    // mad r1.xyz, v5.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((v5.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.x, r1.xyzx, r2.yzwy
    r1.x = (dot((r1.xyzx).xyz,(r2.yzwy).xyz).xxxx).x;
    // lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // mul r1.x, r1.x, cb0[8].y
    r1.x = ((r1.xxxx)*(source[8].yyyy)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mul r1.xyz, r3.xyzx, r1.xxxx
    r1.xyz = ((r3.xyzx)*(r1.xxxx)).xyz;
    // mad r1.xyz, r4.wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // mul r2.xyz, r3.wwww, cb2[3].xyzx
    r2.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// db2e4dedb45e9a4b88b2a0650e21e495
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect51(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[7]=0.f;
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[13]=0.f;
    source[7]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f;
    // dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v3.zxyz
    r1.xyz = ((r0.wwww)*(v3.zxyz)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r2.xy, r2.xyxx, cb0[5].xxxx
    r2.xy = ((r2.xyxx)*(source[5].xxxx)).xy;
    // mul r3.xy, r2.xyxx, v0.wwww
    r3.xy = ((r2.xyxx)*(v0.wwww)).xy;
    // dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r2.xyw, r3.xyxz, r1.wwww
    r2.xyw = ((r3.xyxz)/(r1.wwww)).xyw;
    // dp3 r1.w, r2.xywx, r2.xywx
    r1.w = (dot((r2.xywx).xyz,(r2.xywx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, r2.xywx
    r3.xyz = ((r1.wwww)*(r2.xywx)).xyz;
    // dp3 r1.w, r3.xyzx, r0.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // mul r3.xy, r1.wwww, r3.xyxx
    r3.xy = ((r1.wwww)*(r3.xyxx)).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r0.xyxx
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r0.xyxx))).xy;
    // ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].y
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].yyyy)) * 0xffffffffu)).w;
    // if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // mul r4.xyzw, v6.yyyy, cb0[9].xyzw
    r4.xyzw = ((v6.yyyy)*(source[9].xyzw)).xyzw;
    // mad r4.xyzw, cb0[8].xyzw, v6.xxxx, r4.xyzw
    r4.xyzw = ((source[8].xyzw)*(v6.xxxx)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[10].xyzw, v6.zzzz, r4.xyzw
    r4.xyzw = ((source[10].xyzw)*(v6.zzzz)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[11].xyzw, v6.wwww, r4.xyzw
    r4.xyzw = ((source[11].xyzw)*(v6.wwww)+(r4.xyzw)).xyzw;
    // div r3.zw, r4.xxxy, r4.wwww
    r3.zw = ((r4.xxxy)/(r4.wwww)).zw;
    // sample_indexable(texture2d)(float,float,float,float) r5.x, r3.zwzz, t1.xyzw, s3
    r5.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r3.zwzz).xy,0.f).rrrr).xyzw).x;
    // mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r6.yz, cb0[12].wwzw
    r6.yz = (source[12].wwzw).yz;
    // add r6.xyzw, r3.zwzw, r6.xyzw
    r6.xyzw = ((r3.zwzw)+(r6.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r5.y, r6.xyxx, t1.yxzw, s3
    r5.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r5.z, r6.zwzz, t1.yzxw, s3
    r5.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r4.xy, r3.zwzz, cb0[12].zwzz
    r4.xy = ((r3.zwzz)+(source[12].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r5.w, r4.xyxx, t1.yzwx, s3
    r5.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r4.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r4.xyzw, r4.zzzz, r5.xyzw
    r4.xyzw = (asfloat((uint4)((r4.zzzz)<(r5.xyzw)) * 0xffffffffu)).xyzw;
    // and r5.xyzw, r4.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r5.xyzw = (asfloat(asuint(r4.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r3.zw, r3.zzzw, cb0[12].xxxy
    r3.zw = ((r3.zzzw)*(source[12].xxxy)).zw;
    // frc r3.zw, r3.zzzw
    r3.zw = (frac(r3.zzzw)).zw;
    // movc r4.xy, r4.xyxx, l(-1.000000,-1.000000,0,0), l(-0.000000,-0.000000,0,0)
    r4.xy = ((asuint(r4.xyxx) != 0u) ? (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u))) : (float4(-0.000000,-0.000000,asfloat(0u),asfloat(0u)))).xy;
    // add r4.xy, r4.xyxx, r5.zwzz
    r4.xy = ((r4.xyxx)+(r5.zwzz)).xy;
    // mad r4.xy, r3.zzzz, r4.xyxx, r5.xyxx
    r4.xy = ((r3.zzzz)*(r4.xyxx)+(r5.xyxx)).xy;
    // add r1.w, -r4.x, r4.y
    r1.w = ((-(r4.xxxx))+(r4.yyyy)).w;
    // mad r1.w, r3.w, r1.w, r4.x
    r1.w = ((r3.wwww)*(r1.wwww)+(r4.xxxx)).w;
    // mul r4.xyz, r1.wwww, cb0[13].xxxx
    r4.xyz = ((r1.wwww)*(source[13].xxxx)).xyz;
    // else
    } else {
    // mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v2.xyxx, t2.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r6.xyz, cb0[1].xyzx, cb0[5].yyyy
    r6.xyz = ((source[1].xyzx)*(source[5].yyyy)).xyz;
    // mul r6.xyz, r5.xyzx, r6.xyzx
    r6.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // add r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mad r3.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v2.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v2.xyxx))).xy;
    // mad r3.xy, r3.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v2.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v2.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r1.w, r2.z, cb0[5].z
    r1.w = ((r2.zzzz)*(source[5].zzzz)).w;
    // mad r3.xyz, r3.xyzx, cb0[2].xyzx, -r6.xyzx
    r3.xyz = ((r3.xyzx)*(source[2].xyzx)+(-(r6.xyzx))).xyz;
    // mad r3.xyz, r1.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // dp3 r1.y, r2.wxyw, r1.xyzx
    r1.y = (dot((r2.wxyw).xyz,(r1.xyzx).xyz).xxxx).y;
    // max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // min r1.z, r1.y, l(1.000000)
    r1.z = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mul r6.xyz, cb0[3].xyzx, cb0[5].wwww
    r6.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // mad r6.xyz, v3.xyzx, r0.wwww, r0.xyzx
    r6.xyz = ((v3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r6.xyz, r6.xyzx, r0.wwww
    r6.xyz = ((r6.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.w, r6.xyzx, r2.xywx
    r0.w = (dot((r6.xyzx).xyz,(r2.xywx).xyz).xxxx).w;
    // lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // mul r0.w, r0.w, cb0[6].x
    r0.w = ((r0.wwww)*(source[6].xxxx)).w;
    // exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // mul r5.xyz, r5.xyzx, r0.wwww
    r5.xyz = ((r5.xyzx)*(r0.wwww)).xyz;
    // mad r3.xyz, r1.zzzz, r3.xyzx, r5.xyzx
    r3.xyz = ((r1.zzzz)*(r3.xyzx)+(r5.xyzx)).xyz;
    // mov_sat r1.x, -r1.x
    r1.x = (saturate(-(r1.xxxx))).x;
    // mul r5.xyz, cb0[4].xyzx, cb0[6].zzzz
    r5.xyz = ((source[4].xyzx)*(source[6].zzzz)).xyz;
    // dp3 r0.x, r2.xywx, r0.xyzx
    r0.x = (dot((r2.xywx).xyz,(r0.xyzx).xyz).xxxx).x;
    // add r0.xw, -|r0.xxxz|, l(1.000000, 0.000000, 0.000000, 1.000000)
    r0.xw = ((-(abs(r0.xxxz)))+(float4(1.000000,0.000000,0.000000,1.000000))).xw;
    // mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // mul r0.x, r0.x, cb0[6].w
    r0.x = ((r0.xxxx)*(source[6].wwww)).x;
    // exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // mul r0.xzw, r5.xxyz, r0.xxxx
    r0.xzw = ((r5.xxyz)*(r0.xxxx)).xzw;
    // mul r0.xzw, r0.xxzw, r1.xxxx
    r0.xzw = ((r0.xxzw)*(r1.xxxx)).xzw;
    // movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // mul r1.xyz, r1.yyyy, cb2[3].xyzx
    r1.xyz = ((r1.yyyy)*(passValues[3].xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // mul_sat r0.x, r5.w, cb0[6].y
    r0.x = (saturate((r5.wwww)*(source[6].yyyy))).x;
    // mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 2a21e12efa02ed4382abed4db0b3b9f2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect52(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[13]=0.f;
    source[14]=0.f;
    source[15]=0.f;
    source[9]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v3.zxyz
    r1.xyz = ((r0.wwww)*(v3.zxyz)).xyz;
    // mul r2.xy, v2.xyxx, cb0[1].xyxx
    r2.xy = ((v2.xyxx)*(source[1].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t0.xywz, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r2.zw, r3.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r3.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r2.zw, r2.zzzw, cb0[6].wwww
    r2.zw = ((r2.zzzw)*(source[6].wwww)).zw;
    // mul r4.xy, r2.zwzz, v0.wwww
    r4.xy = ((r2.zwzz)*(v0.wwww)).xy;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r3.xyw, r4.xyxz, r1.wwww
    r3.xyw = ((r4.xyxz)/(r1.wwww)).xyw;
    // dp3 r1.w, r3.xywx, r3.xywx
    r1.w = (dot((r3.xywx).xyz,(r3.xywx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, r3.xywx
    r4.xyz = ((r1.wwww)*(r3.xywx)).xyz;
    // dp3 r1.w, r4.xyzx, r0.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // mul r2.zw, r1.wwww, r4.xxxy
    r2.zw = ((r1.wwww)*(r4.xxxy)).zw;
    // mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), -r0.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(-(r0.xxxy))).zw;
    // ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[15].y
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[15].yyyy)) * 0xffffffffu)).w;
    // if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // mul r4.xyzw, v6.yyyy, cb0[11].xyzw
    r4.xyzw = ((v6.yyyy)*(source[11].xyzw)).xyzw;
    // mad r4.xyzw, cb0[10].xyzw, v6.xxxx, r4.xyzw
    r4.xyzw = ((source[10].xyzw)*(v6.xxxx)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[12].xyzw, v6.zzzz, r4.xyzw
    r4.xyzw = ((source[12].xyzw)*(v6.zzzz)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[13].xyzw, v6.wwww, r4.xyzw
    r4.xyzw = ((source[13].xyzw)*(v6.wwww)+(r4.xyzw)).xyzw;
    // div r4.xy, r4.xyxx, r4.wwww
    r4.xy = ((r4.xyxx)/(r4.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r5.x, r4.xyxx, t1.xyzw, s3
    r5.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r4.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r6.yz, cb0[14].wwzw
    r6.yz = (source[14].wwzw).yz;
    // add r6.xyzw, r4.xyxy, r6.xyzw
    r6.xyzw = ((r4.xyxy)+(r6.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r5.y, r6.xyxx, t1.yxzw, s3
    r5.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r5.z, r6.zwzz, t1.yzxw, s3
    r5.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r6.xy, r4.xyxx, cb0[14].zwzz
    r6.xy = ((r4.xyxx)+(source[14].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r5.w, r6.xyxx, t1.yzwx, s3
    r5.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r5.xyzw, r4.zzzz, r5.xyzw
    r5.xyzw = (asfloat((uint4)((r4.zzzz)<(r5.xyzw)) * 0xffffffffu)).xyzw;
    // and r6.xyzw, r5.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r6.xyzw = (asfloat(asuint(r5.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r4.xy, r4.xyxx, cb0[14].xyxx
    r4.xy = ((r4.xyxx)*(source[14].xyxx)).xy;
    // frc r4.xy, r4.xyxx
    r4.xy = (frac(r4.xyxx)).xy;
    // movc r4.zw, r5.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r4.zw = ((asuint(r5.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r4.zw, r4.zzzw, r6.zzzw
    r4.zw = ((r4.zzzw)+(r6.zzzw)).zw;
    // mad r4.xz, r4.xxxx, r4.zzwz, r6.xxyx
    r4.xz = ((r4.xxxx)*(r4.zzwz)+(r6.xxyx)).xz;
    // add r1.w, -r4.x, r4.z
    r1.w = ((-(r4.xxxx))+(r4.zzzz)).w;
    // mad r1.w, r4.y, r1.w, r4.x
    r1.w = ((r4.yyyy)*(r1.wwww)+(r4.xxxx)).w;
    // mul r4.xyz, r1.wwww, cb0[15].xxxx
    r4.xyz = ((r1.wwww)*(source[15].xxxx)).xyz;
    // else
    } else {
    // mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r2.zw, r2.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r2.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), -v2.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(-(v2.xxxy))).zw;
    // mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 0.750000, 0.750000), v2.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,0.750000,0.750000))+(v2.xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r2.zwzz, t3.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r5.xyzx)*(source[2].xyzx)).xyz;
    // mad r5.xyz, cb0[7].xxxx, r5.xyzx, r5.xyzx
    r5.xyz = ((source[7].xxxx)*(r5.xyzx)+(r5.xyzx)).xyz;
    // add r5.xyz, r5.xyzx, -cb0[7].xxxx
    r5.xyz = ((r5.xyzx)+(-(source[7].xxxx))).xyz;
    // mov_sat r6.xyz, r5.xyzx
    r6.xyz = (saturate(r5.xyzx)).xyz;
    // mul r1.w, r3.z, cb0[7].y
    r1.w = ((r3.zzzz)*(source[7].yyyy)).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r7.xyz, cb0[3].xyzx, cb0[7].zzzz
    r7.xyz = ((source[3].xyzx)*(source[7].zzzz)).xyz;
    // mul r7.xyz, r2.xyzx, r7.xyzx
    r7.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // mad r6.xyz, r1.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // mad r5.xyz, -r1.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r1.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // max r5.xyz, r5.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r5.xyz = (max(r5.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r5.xyz, r5.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // dp3 r1.y, r3.wxyw, r1.xyzx
    r1.y = (dot((r3.wxyw).xyz,(r1.xyzx).xyz).xxxx).y;
    // max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // min r1.z, r1.y, l(1.000000)
    r1.z = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mul r6.xyz, cb0[4].xyzx, cb0[7].wwww
    r6.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // mad r6.xyz, v3.xyzx, r0.wwww, r0.xyzx
    r6.xyz = ((v3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r6.xyz, r6.xyzx, r0.wwww
    r6.xyz = ((r6.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.w, r6.xyzx, r3.xywx
    r0.w = (dot((r6.xyzx).xyz,(r3.xywx).xyz).xxxx).w;
    // lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // mul r0.w, r0.w, cb0[8].x
    r0.w = ((r0.wwww)*(source[8].xxxx)).w;
    // exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // mul r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // mad r2.xyz, r1.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // mov_sat r1.x, -r1.x
    r1.x = (saturate(-(r1.xxxx))).x;
    // mul r5.xyz, cb0[5].xyzx, cb0[8].zzzz
    r5.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // dp3 r0.x, r3.xywx, r0.xyzx
    r0.x = (dot((r3.xywx).xyz,(r0.xyzx).xyz).xxxx).x;
    // add r0.xw, -|r0.xxxz|, l(1.000000, 0.000000, 0.000000, 1.000000)
    r0.xw = ((-(abs(r0.xxxz)))+(float4(1.000000,0.000000,0.000000,1.000000))).xw;
    // mul r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)).x;
    // lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // mul r0.x, r0.x, cb0[8].w
    r0.x = ((r0.xxxx)*(source[8].wwww)).x;
    // exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // mul r0.xzw, r5.xxyz, r0.xxxx
    r0.xzw = ((r5.xxyz)*(r0.xxxx)).xzw;
    // mul r0.xzw, r0.xxzw, r1.xxxx
    r0.xzw = ((r0.xxzw)*(r1.xxxx)).xzw;
    // movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // mul r1.xyz, r1.yyyy, cb2[3].xyzx
    r1.xyz = ((r1.yyyy)*(passValues[3].xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // mul_sat r0.x, r2.w, cb0[8].y
    r0.x = (saturate((r2.wwww)*(source[8].yyyy))).x;
    // mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 6e77c0e0119e534c968352aa0afd2372
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect54(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[5]=0.f;
    source[6]=0.f;
    source[7]=0.f;
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[5]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].yyyy)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // mul r0.xyzw, v6.yyyy, cb0[7].xyzw
    r0.xyzw = ((v6.yyyy)*(source[7].xyzw)).xyzw;
    // mad r0.xyzw, cb0[6].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[6].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[8].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[8].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[9].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[9].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t1.xyzw, s3
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r2.yz, cb0[10].wwzw
    r2.yz = (source[10].wwzw).yz;
    // add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s3
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s3
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r2.xy, r0.xyxx, cb0[10].zwzz
    r2.xy = ((r0.xyxx)+(source[10].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t1.yzwx, s3
    r1.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r0.xy, r0.xyxx, cb0[10].xyxx
    r0.xy = ((r0.xyxx)*(source[10].xyxx)).xy;
    // frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, cb0[11].xxxx
    r0.xyz = ((r0.xxxx)*(source[11].xxxx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul_sat r0.w, r1.w, cb0[4].x
    r0.w = (saturate((r1.wwww)*(source[4].xxxx))).w;
    // add r1.w, -v0.w, l(1.000000)
    r1.w = ((-(v0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // add r1.w, r1.w, cb0[4].z
    r1.w = ((r1.wwww)+(source[4].zzzz)).w;
    // add_sat r1.w, r1.w, cb0[4].y
    r1.w = (saturate((r1.wwww)+(source[4].yyyy))).w;
    // mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.w, r0.w, l(0.003000)
    r1.w = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).w;
    // if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // dp3 r2.x, v3.xyzx, v3.xyzx
    r2.x = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).x;
    // rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // mul r2.xyz, r2.xxxx, v3.xyzx
    r2.xyz = ((r2.xxxx)*(v3.xyzx)).xyz;
    // mul r3.xyz, cb0[1].xyzx, cb0[3].yyyy
    r3.xyz = ((source[1].xyzx)*(source[3].yyyy)).xyz;
    // mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r2.w, r3.xyxx, r3.xyxx
    r2.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // add r4.z, r2.w, l(0.000010)
    r4.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r3.xy, r3.xyxx, cb0[3].xxxx
    r3.xy = ((r3.xyxx)*(source[3].xxxx)).xy;
    // mul r4.xy, r3.xyxx, v0.wwww
    r4.xy = ((r3.xyxx)*(v0.wwww)).xy;
    // dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // div r3.xyz, r4.xyzx, r2.wwww
    r3.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // dp3 r2.w, r3.xyzx, r2.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, cb0[2].xyzx, cb0[3].zzzz
    r5.xyz = ((source[2].xyzx)*(source[3].zzzz)).xyz;
    // mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // mad r2.xyz, v5.xyzx, r1.wwww, r2.xyzx
    r2.xyz = ((v5.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r2.xyzx, r3.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // lt r2.x, |r1.w|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // mul r1.w, r1.w, cb0[3].w
    r1.w = ((r1.wwww)*(source[3].wwww)).w;
    // exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // movc r1.w, r2.x, l(0), r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // mul r2.xyz, r4.xyzx, r1.wwww
    r2.xyz = ((r4.xyzx)*(r1.wwww)).xyz;
    // mad r1.xyz, r3.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // mul r2.xyz, r2.wwww, cb2[3].xyzx
    r2.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 2cf61135d30a0348afb60d4bfc1966ef
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect56(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[5]=0.f;
    source[6]=0.f;
    source[7]=0.f;
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[5]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].yyyy)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // mul r0.xyzw, v6.yyyy, cb0[7].xyzw
    r0.xyzw = ((v6.yyyy)*(source[7].xyzw)).xyzw;
    // mad r0.xyzw, cb0[6].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[6].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[8].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[8].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[9].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[9].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t1.xyzw, s2
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r2.yz, cb0[10].wwzw
    r2.yz = (source[10].wwzw).yz;
    // add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s2
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s2
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r2.xy, r0.xyxx, cb0[10].zwzz
    r2.xy = ((r0.xyxx)+(source[10].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t1.yzwx, s2
    r1.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r0.xy, r0.xyxx, cb0[10].xyxx
    r0.xy = ((r0.xyxx)*(source[10].xyxx)).xy;
    // frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, cb0[11].xxxx
    r0.xyz = ((r0.xxxx)*(source[11].xxxx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r0.w, -v0.w, cb0[4].y
    r0.w = ((-(v0.wwww))+(source[4].yyyy)).w;
    // add r0.w, r0.w, cb0[4].x
    r0.w = ((r0.wwww)+(source[4].xxxx)).w;
    // add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // dp3 r1.y, v3.xyzx, v3.xyzx
    r1.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r1.yzw, r1.yyyy, v3.xxyz
    r1.yzw = ((r1.yyyy)*(v3.xxyz)).yzw;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, cb0[1].xyzx, cb0[3].yyyy
    r3.xyz = ((source[1].xyzx)*(source[3].yyyy)).xyz;
    // mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // add r5.z, r2.w, l(0.000010)
    r5.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r4.xyxx, cb0[3].xxxx
    r4.xy = ((r4.xyxx)*(source[3].xxxx)).xy;
    // mul r5.xy, r4.xyxx, v0.wwww
    r5.xy = ((r4.xyxx)*(v0.wwww)).xy;
    // dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // div r4.xyz, r5.xyzx, r2.wwww
    r4.xyz = ((r5.xyzx)/(r2.wwww)).xyz;
    // dp3 r2.w, r4.xyzx, r1.yzwy
    r2.w = (dot((r4.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r5.xyz, cb0[2].xyzx, cb0[3].zzzz
    r5.xyz = ((source[2].xyzx)*(source[3].zzzz)).xyz;
    // mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // mad r1.xyz, v5.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((v5.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // mul r1.x, r1.x, cb0[3].w
    r1.x = ((r1.xxxx)*(source[3].wwww)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mul r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)*(r1.xxxx)).xyz;
    // mad r1.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // mul r2.xyz, r2.wwww, cb2[3].xyzx
    r2.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 5fc881d5533f9b40a806ff9878a79083
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect57(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[13]=0.f;
    source[14]=0.f;
    source[8]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[14].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[14].yyyy)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // mul r0.xyzw, v6.yyyy, cb0[10].xyzw
    r0.xyzw = ((v6.yyyy)*(source[10].xyzw)).xyzw;
    // mad r0.xyzw, cb0[9].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[9].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[11].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[11].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[12].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[12].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t4.xyzw, s5
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r2.yz, cb0[13].wwzw
    r2.yz = (source[13].wwzw).yz;
    // add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t4.yxzw, s5
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t4.yzxw, s5
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r2.xy, r0.xyxx, cb0[13].zwzz
    r2.xy = ((r0.xyxx)+(source[13].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t4.yzwx, s5
    r1.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r0.xy, r0.xyxx, cb0[13].xyxx
    r0.xy = ((r0.xyxx)*(source[13].xyxx)).xy;
    // frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, cb0[14].xxxx
    r0.xyz = ((r0.xxxx)*(source[14].xxxx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r0.w, -v0.w, cb0[7].x
    r0.w = ((-(v0.wwww))+(source[7].xxxx)).w;
    // add r0.w, r0.w, cb0[6].w
    r0.w = ((r0.wwww)+(source[6].wwww)).w;
    // add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // dp3 r1.y, v3.xyzx, v3.xyzx
    r1.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r1.yzw, r1.yyyy, v3.xxyz
    r1.yzw = ((r1.yyyy)*(v3.xxyz)).yzw;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r3.xyz, cb0[1].xyzx, cb0[5].xxxx
    r3.xyz = ((source[1].xyzx)*(source[5].xxxx)).xyz;
    // mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // mul r5.xyz, cb0[2].xyzx, cb0[5].yyyy
    r5.xyz = ((source[2].xyzx)*(source[5].yyyy)).xyz;
    // mul r6.xy, v2.xyxx, cb0[4].yyyy
    r6.xy = ((v2.xyxx)*(source[4].yyyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // dp3 r3.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // mad r5.xyz, -r5.xyzx, r7.xyzx, r3.wwww
    r5.xyz = ((-(r5.xyzx))*(r7.xyzx)+(r3.wwww)).xyz;
    // mad r5.xyz, cb0[5].wwww, r5.xyzx, r8.xyzx
    r5.xyz = ((source[5].wwww)*(r5.xyzx)+(r8.xyzx)).xyz;
    // max r3.w, cb0[4].w, l(0.000000)
    r3.w = (max(source[4].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r3.w, r3.w, l(0.990000)
    r3.w = (min(r3.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r4.w
    r4.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r4.wwww)).w;
    // add r5.w, -v0.x, l(1.000000)
    r5.w = ((-(v0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.zw, v2.xyxx, t0.zwxy, s0, l(0.000000)
    r6.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r6.zw, r6.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r6.zw = ((r6.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r8.x, r6.zwzz, r6.zwzz
    r8.x = (dot((r6.zwzz).xy,(r6.zwzz).xy).xxxx).x;
    // add r8.x, -r8.x, l(1.000000)
    r8.x = ((-(r8.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r8.x, r8.x, l(0.000000)
    r8.x = (max(r8.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r8.x, r8.x
    r8.x = (sqrt(r8.xxxx)).x;
    // add r8.z, r8.x, l(0.000010)
    r8.z = ((r8.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r6.zw, r6.zzzw, cb0[4].xxxx
    r6.zw = ((r6.zzzw)*(source[4].xxxx)).zw;
    // mul r8.xy, r6.zwzz, v0.wwww
    r8.xy = ((r6.zwzz)*(v0.wwww)).xy;
    // dp3 r6.z, r8.xyzx, r8.xyzx
    r6.z = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // sqrt r6.z, r6.z
    r6.z = (sqrt(r6.zzzz)).z;
    // div r8.xyz, r8.xyzx, r6.zzzz
    r8.xyz = ((r8.xyzx)/(r6.zzzz)).xyz;
    // mul r6.z, r8.z, r8.z
    r6.z = ((r8.zzzz)*(r8.zzzz)).z;
    // mul_sat r2.w, r2.w, r6.z
    r2.w = (saturate((r2.wwww)*(r6.zzzz))).w;
    // add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r6.z, r7.w, r7.w
    r6.z = ((r7.wwww)*(r7.wwww)).z;
    // mul r2.w, r2.w, r6.z
    r2.w = ((r2.wwww)*(r6.zzzz)).w;
    // mul r6.z, r2.w, r3.w
    r6.z = ((r2.wwww)*(r3.wwww)).z;
    // mad r5.w, r5.w, r6.z, r5.w
    r5.w = ((r5.wwww)*(r6.zzzz)+(r5.wwww)).w;
    // add r3.w, -r3.w, r5.w
    r3.w = ((-(r3.wwww))+(r5.wwww)).w;
    // mul r6.z, r3.w, r4.w
    r6.z = ((r3.wwww)*(r4.wwww)).z;
    // mad r3.w, -r4.w, r3.w, r5.w
    r3.w = ((-(r4.wwww))*(r3.wwww)+(r5.wwww)).w;
    // mad_sat r2.w, r2.w, r3.w, r6.z
    r2.w = (saturate((r2.wwww)*(r3.wwww)+(r6.zzzz))).w;
    // mad r2.xyz, -r2.xyzx, r3.xyzx, r5.xyzx
    r2.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r5.xyzx)).xyz;
    // mad r2.xyz, r2.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r6.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r3.z, r3.xyxx, r3.xyxx
    r3.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // add r3.z, -r3.z, l(1.000000)
    r3.z = ((-(r3.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // max r3.z, r3.z, l(0.000000)
    r3.z = (max(r3.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // sqrt r3.z, r3.z
    r3.z = (sqrt(r3.zzzz)).z;
    // add r4.z, r3.z, l(0.000010)
    r4.z = ((r3.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r3.xyxx, cb0[4].zzzz
    r4.xy = ((r3.xyxx)*(source[4].zzzz)).xy;
    // mul r3.x, r2.w, l(0.650000)
    r3.x = ((r2.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // add r3.yzw, -r8.xxyz, r4.xxyz
    r3.yzw = ((-(r8.xxyz))+(r4.xxyz)).yzw;
    // mad r3.xyz, r3.xxxx, r3.yzwy, r8.xyzx
    r3.xyz = ((r3.xxxx)*(r3.yzwy)+(r8.xyzx)).xyz;
    // dp3 r3.w, r3.xyzx, r1.yzwy
    r3.w = (dot((r3.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r4.x, r3.w, l(1.000000)
    r4.x = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.yzw, v2.xyxx, t5.wxyz, s4, l(0.000000)
    r4.yzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // mul r5.xyz, cb0[3].xyzx, cb0[6].xxxx
    r5.xyz = ((source[3].xyzx)*(source[6].xxxx)).xyz;
    // mul r4.yzw, r4.yyzw, r5.xxyz
    r4.yzw = ((r4.yyzw)*(r5.xxyz)).yzw;
    // mad r5.xyz, cb0[6].yyyy, r7.xyzx, -r4.yzwy
    r5.xyz = ((source[6].yyyy)*(r7.xyzx)+(-(r4.yzwy))).xyz;
    // mad r4.yzw, r2.wwww, r5.xxyz, r4.yyzw
    r4.yzw = ((r2.wwww)*(r5.xxyz)+(r4.yyzw)).yzw;
    // mad r1.xyz, v5.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((v5.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // mul r1.x, r1.x, cb0[6].z
    r1.x = ((r1.xxxx)*(source[6].zzzz)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mul r1.xyz, r4.yzwy, r1.xxxx
    r1.xyz = ((r4.yzwy)*(r1.xxxx)).xyz;
    // mad r1.xyz, r4.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r4.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // mul r2.xyz, r3.wwww, cb2[3].xyzx
    r2.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// caef2c98eace3241ae73dcba15f36599
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect58(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[5]=0.f;
    source[6]=0.f;
    source[7]=0.f;
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[5]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].yyyy)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // mul r0.xyzw, v6.yyyy, cb0[7].xyzw
    r0.xyzw = ((v6.yyyy)*(source[7].xyzw)).xyzw;
    // mad r0.xyzw, cb0[6].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[6].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[8].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[8].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[9].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[9].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t1.xyzw, s2
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r2.yz, cb0[10].wwzw
    r2.yz = (source[10].wwzw).yz;
    // add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s2
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s2
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r2.xy, r0.xyxx, cb0[10].zwzz
    r2.xy = ((r0.xyxx)+(source[10].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t1.yzwx, s2
    r1.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r0.xy, r0.xyxx, cb0[10].xyxx
    r0.xy = ((r0.xyxx)*(source[10].xyxx)).xy;
    // frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, cb0[11].xxxx
    r0.xyz = ((r0.xxxx)*(source[11].xxxx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r0.w, -v0.w, cb0[4].w
    r0.w = ((-(v0.wwww))+(source[4].wwww)).w;
    // add r0.w, r0.w, cb0[4].z
    r0.w = ((r0.wwww)+(source[4].zzzz)).w;
    // add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // dp3 r1.y, v3.xyzx, v3.xyzx
    r1.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r1.yzw, r1.yyyy, v3.xxyz
    r1.yzw = ((r1.yyyy)*(v3.xxyz)).yzw;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, cb0[2].xyzx, cb0[4].xxxx
    r3.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // add r5.z, r2.w, l(0.000010)
    r5.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r4.xy, r4.xyxx, cb0[3].xxxx
    r4.xy = ((r4.xyxx)*(source[3].xxxx)).xy;
    // mul r5.xy, r4.xyxx, v0.wwww
    r5.xy = ((r4.xyxx)*(v0.wwww)).xy;
    // dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // div r4.xyz, r5.xyzx, r2.wwww
    r4.xyz = ((r5.xyzx)/(r2.wwww)).xyz;
    // dp3 r2.w, r4.xyzx, r1.yzwy
    r2.w = (dot((r4.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r5.xyz, cb0[1].xyzx, cb0[3].yyyy
    r5.xyz = ((source[1].xyzx)*(source[3].yyyy)).xyz;
    // mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // mad r1.xyz, v5.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((v5.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // mul r1.x, r1.x, cb0[4].y
    r1.x = ((r1.xxxx)*(source[4].yyyy)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mul r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)*(r1.xxxx)).xyz;
    // mad r1.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // mul r2.xyz, r2.wwww, cb2[3].xyzx
    r2.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// e4f30a099c13d740bd43b3575f7202cb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect59(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[10]=0.f;
    source[11]=0.f;
    source[10]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].xxxx)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1
    r0.xyz = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).xyz;
    // mul r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)*(r0.xyzx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t2.yzwx, s4, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // div r1.xy, v6.xyxx, v6.wwww
    r1.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s0, l(0.000000)
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r1.xyxx).xy,0.f).rrrr).xyzw).x;
    // min r1.x, r1.x, l(0.999000)
    r1.x = (min(r1.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // mad r1.y, r1.x, cb2[1].x, cb2[1].y
    r1.y = ((r1.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // mad r1.x, r1.x, cb2[1].z, -cb2[1].w
    r1.x = ((r1.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).x;
    // div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // add r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)+(r1.yyyy)).x;
    // add r1.y, -cb0[9].y, l(1.000000)
    r1.y = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // add r1.x, r1.x, -v6.w
    r1.x = ((r1.xxxx)+(-(v6.wwww))).x;
    // max r1.y, r1.y, l(0.001000)
    r1.y = (max(r1.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // dp3 r1.w, v3.xyzx, v3.xyzx
    r1.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v3.xyzx
    r2.xyz = ((r1.wwww)*(v3.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v2.xyxx, t0.xyzw, s2, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    // dp3 r1.w, r3.xyzx, r1.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // mul r4.xyz, r1.wwww, r3.xyzx
    r4.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // mad r1.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // add r4.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r4.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // dp2 r5.x, cb0[2].xyxx, r4.xyxx
    r5.x = (dot((source[2].xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // dp2 r5.y, cb0[3].xyxx, r4.xyxx
    r5.y = (dot((source[3].xyxx).xy,(r4.xyxx).xy).xxxx).y;
    // add r4.xy, r5.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r5.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r4.xy, r4.xyxx, cb0[4].xyxx
    r4.xy = ((r4.xyxx)*(source[4].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t1.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r5.xyz, -r4.xyzx, r1.wwww
    r5.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // mad r4.xyz, cb0[8].yyyy, r5.xyzx, r4.xyzx
    r4.xyz = ((source[8].yyyy)*(r5.xyzx)+(r4.xyzx)).xyz;
    // mul r5.xyz, r4.xyzx, cb0[8].wwww
    r5.xyz = ((r4.xyzx)*(source[8].wwww)).xyz;
    // mul r5.xyz, r5.xyzx, cb0[6].xyzx
    r5.xyz = ((r5.xyzx)*(source[6].xyzx)).xyz;
    // mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r4.xyz, r4.xyzx, cb0[8].zzzz
    r4.xyz = ((r4.xyzx)*(source[8].zzzz)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[5].xyzx
    r4.xyz = ((r4.xyzx)*(source[5].xyzx)).xyz;
    // mul r4.xyz, r6.xyzx, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r4.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3_sat r1.w, r3.xyzx, r2.xyzx
    r1.w = (saturate(dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx)).w;
    // lt r2.w, r1.w, l(0.000001)
    r2.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // dp3_sat r1.x, r1.xyzx, r2.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx)).x;
    // lt r1.y, r1.x, l(0.000001)
    r1.y = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // mul r1.x, r1.x, cb0[9].x
    r1.x = ((r1.xxxx)*(source[9].xxxx)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mul r1.xyz, r5.xyzx, r1.xxxx
    r1.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // mad r1.xyz, r4.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r4.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 6d4c725e5426fc46880553bd35496678
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect60(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[7]=0.f;
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[13]=0.f;
    source[7]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].yyyy)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // mul r0.xyzw, v8.yyyy, cb0[9].xyzw
    r0.xyzw = ((v8.yyyy)*(source[9].xyzw)).xyzw;
    // mad r0.xyzw, cb0[8].xyzw, v8.xxxx, r0.xyzw
    r0.xyzw = ((source[8].xyzw)*(v8.xxxx)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[10].xyzw, v8.zzzz, r0.xyzw
    r0.xyzw = ((source[10].xyzw)*(v8.zzzz)+(r0.xyzw)).xyzw;
    // mad r0.xyzw, cb0[11].xyzw, v8.wwww, r0.xyzw
    r0.xyzw = ((source[11].xyzw)*(v8.wwww)+(r0.xyzw)).xyzw;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t1.xyzw, s4
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r2.yz, cb0[12].wwzw
    r2.yz = (source[12].wwzw).yz;
    // add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s4
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s4
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r2.xy, r0.xyxx, cb0[12].zwzz
    r2.xy = ((r0.xyxx)+(source[12].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t1.yzwx, s4
    r1.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r0.xy, r0.xyxx, cb0[12].xyxx
    r0.xy = ((r0.xyxx)*(source[12].xyxx)).xy;
    // frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, cb0[13].xxxx
    r0.xyz = ((r0.xxxx)*(source[13].xxxx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t4.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul_sat r0.w, r1.w, cb0[5].w
    r0.w = (saturate((r1.wwww)*(source[5].wwww))).w;
    // add r1.w, -v2.w, cb0[6].y
    r1.w = ((-(v2.wwww))+(source[6].yyyy)).w;
    // add r1.w, r1.w, cb0[6].x
    r1.w = ((r1.wwww)+(source[6].xxxx)).w;
    // add_sat r1.w, r1.w, l(1.000000)
    r1.w = (saturate((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // mul r0.w, r0.w, cb0[0].w
    r0.w = ((r0.wwww)*(source[0].wwww)).w;
    // lt r1.w, r0.w, l(0.003000)
    r1.w = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).w;
    // if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v1.zxyz
    r2.xyz = ((r1.wwww)*(v1.zxyz)).xyz;
    // dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v0.xyzx
    r3.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // mul r4.xyz, r2.xyzx, r3.yzxy
    r4.xyz = ((r2.xyzx)*(r3.yzxy)).xyz;
    // mad r2.xyz, r2.zxyz, r3.zxyz, -r4.xyzx
    r2.xyz = ((r2.zxyz)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, v7.xyzx
    r4.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r5.xyz, r1.wwww, v5.xyzx
    r5.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // add r7.z, r2.w, l(0.000010)
    r7.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r6.xy, r6.xyxx, cb0[4].xxxx
    r6.xy = ((r6.xyxx)*(source[4].xxxx)).xy;
    // mul r7.xy, r6.xyxx, v2.wwww
    r7.xy = ((r6.xyxx)*(v2.wwww)).xy;
    // dp3 r2.w, r7.xyzx, r7.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // div r6.xyw, r7.xyxz, r2.wwww
    r6.xyw = ((r7.xyxz)/(r2.wwww)).xyw;
    // dp3 r2.w, r6.xywx, r6.xywx
    r2.w = (dot((r6.xywx).xyz,(r6.xywx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r7.xyz, r2.wwww, r6.xywx
    r7.xyz = ((r2.wwww)*(r6.xywx)).xyz;
    // dp3 r2.w, r7.xyzx, r4.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // dp3 r3.x, r3.xyzx, r7.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // dp3 r3.y, r2.xyzx, r7.xyzx
    r3.y = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // mul r2.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r2.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // mad r2.xy, cb0[4].yyyy, r3.xyxx, r2.xyxx
    r2.xy = ((source[4].yyyy)*(r3.xyxx)+(r2.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // mad r2.xyz, cb0[4].zzzz, r2.xyzx, r2.xyzx
    r2.xyz = ((source[4].zzzz)*(r2.xyzx)+(r2.xyzx)).xyz;
    // add r2.xyz, r2.xyzx, -cb0[4].zzzz
    r2.xyz = ((r2.xyzx)+(-(source[4].zzzz))).xyz;
    // mov_sat r3.xyz, r2.xyzx
    r3.xyz = (saturate(r2.xyzx)).xyz;
    // mul r2.w, r6.z, cb0[4].w
    r2.w = ((r6.zzzz)*(source[4].wwww)).w;
    // mul r7.xyz, cb0[2].xyzx, cb0[5].xxxx
    r7.xyz = ((source[2].xyzx)*(source[5].xxxx)).xyz;
    // mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // mad r1.xyz, r2.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r2.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // mov_sat r2.xyz, -r2.xyzx
    r2.xyz = (saturate(-(r2.xyzx))).xyz;
    // mad r2.xyz, -r2.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // dp3 r2.x, r6.xywx, r5.xyzx
    r2.x = (dot((r6.xywx).xyz,(r5.xyzx).xyz).xxxx).x;
    // max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // min r2.y, r2.x, l(1.000000)
    r2.y = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, cb0[3].xyzx, cb0[5].yyyy
    r5.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // mad r4.xyz, v5.xyzx, r1.wwww, r4.xyzx
    r4.xyz = ((v5.xyzx)*(r1.wwww)+(r4.xyzx)).xyz;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r4.xyzx, r6.xywx
    r1.w = (dot((r4.xyzx).xyz,(r6.xywx).xyz).xxxx).w;
    // lt r2.z, |r1.w|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // mul r1.w, r1.w, cb0[5].z
    r1.w = ((r1.wwww)*(source[5].zzzz)).w;
    // exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // min r1.xyzw, r1.xyzw, l(999.000000, 999.000000, 999.000000, 1.000000)
    r1.xyzw = (min(r1.xyzw,float4(999.000000,999.000000,999.000000,1.000000))).xyzw;
    // movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // mul r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)*(r1.wwww)).xyz;
    // mad r1.xyz, r2.yyyy, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.yyyy)*(r1.xyzx)+(r3.xyzx)).xyz;
    // mul r2.xyz, r2.xxxx, cb2[3].xyzx
    r2.xyz = ((r2.xxxx)*(passValues[3].xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// 89e005baf2276249963f1a74fd7c7948
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect61(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[7]=0.f;
    source[8]=0.f;
    source[7]=float4(input.lightColor,1.f);
    source[0]=SourceCharacterAppend(frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterLightConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(0.00100000005f,0.f,0.f,0.f))),frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterLightConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(0.f,0.f,0.f,0.f))),1u);
    source[1]=SourceCharacterAppend(frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterLightConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(-0.00200000009f,0.f,0.f,0.f))),frac(((float4(g_SourceCharacterTime,0.f,0.f,0.f)*(g_SourceCharacterLightConstants[60]*float4(4.f,0.f,0.f,0.f)))*float4(0.f,0.f,0.f,0.f))),1u);
    source[2]=SourceCharacterAppend(frac(((g_SourceCharacterLightConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(-0.0199999996f,0.f,0.f,0.f))),frac(((g_SourceCharacterLightConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(0.f,0.f,0.f,0.f))),1u);
    source[5].x=(((g_SourceCharacterLightConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(0.f,0.f,0.f,0.f))).x;
    source[5].y=(frac(((g_SourceCharacterLightConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(0.f,0.f,0.f,0.f)))).x;
    source[5].z=(frac(((g_SourceCharacterLightConstants[60]*float4(g_SourceCharacterTime,0.f,0.f,0.f))*float4(-0.0199999996f,0.f,0.f,0.f)))).x;
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // dp3 r0.w, v4.xyzx, v4.xyzx
    r0.w = (dot((v4.xyzx).xyz,(v4.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v4.xyzx
    r1.xyz = ((r0.wwww)*(v4.xyzx)).xyz;
    // ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[8].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[8].xxxx)) * 0xffffffffu)).w;
    // if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // div r2.xy, v7.xyxx, v7.wwww
    r2.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s0
    r2.xyz = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r2.xyxx).xy,0.f).rrrr).xyzw).xyz;
    // mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // else
    } else {
    // mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r3.xy, v2.xyxx, cb0[0].xyxx
    r3.xy = ((v2.xyxx)+(source[0].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r3.zw, v2.xxxy, l(0.000000, 0.000000, -1.000000, 1.000000), cb0[1].xxxy
    r3.zw = ((v2.xxxy)*(float4(0.000000,0.000000,-1.000000,1.000000))+(source[1].xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r3.zwzz, t0.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r5.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r4.xyzx)).xyz;
    // mul r5.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // mad r6.xy, v2.xyxx, l(-1.000000, 1.000000, 0.000000, 0.000000), cb0[2].xyxx
    r6.xy = ((v2.xyxx)*(float4(-1.000000,1.000000,0.000000,0.000000))+(source[2].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t0.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceMapSkyCloudSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad_sat r5.xyz, r6.xyzx, r5.xyzx, r5.xyzx
    r5.xyz = (saturate((r6.xyzx)*(r5.xyzx)+(r5.xyzx))).xyz;
    // mul r6.xyz, r5.xyzx, cb0[5].wwww
    r6.xyz = ((r5.xyzx)*(source[5].wwww)).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.zw, r3.zwzz, t1.zwxy, s2, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture2.SampleBias(SourceMapDirectSkySampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r7.xy, r3.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r3.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.w, r7.xyxx, r7.xyxx
    r0.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // add r7.z, r0.w, l(0.000010)
    r7.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceMapDirectSkySampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // add r8.xyz, -r7.xyzx, r3.xyzx
    r8.xyz = ((-(r7.xyzx))+(r3.xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, r8.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)+(r7.xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, r1.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // add r1.w, -cb0[6].z, cb0[6].y
    r1.w = ((-(source[6].zzzz))+(source[6].yyyy)).w;
    // mad r0.w, r0.w, r1.w, cb0[6].z
    r0.w = ((r0.wwww)*(r1.wwww)+(source[6].zzzz)).w;
    // add r3.xyz, r7.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)+(r3.xyzx)).xyz;
    // dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // dp3 r1.w, r3.xyzx, r0.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r1.w, r1.w, cb0[6].w
    r1.w = ((r1.wwww)*(source[6].wwww)).w;
    // dp3 r0.x, r0.xyzx, r1.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // mov_sat r0.x, -r0.x
    r0.x = (saturate(-(r0.xxxx))).x;
    // lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // mul r0.z, r0.z, l(10.000000)
    r0.z = ((r0.zzzz)*(float4(10.000000,10.000000,10.000000,10.000000))).z;
    // exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // mul r1.xyw, cb0[3].xyxz, cb0[3].wwww
    r1.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // mul r1.xyw, r0.yyyy, r1.xyxw
    r1.xyw = ((r0.yyyy)*(r1.xyxw)).xyw;
    // mul r1.xyw, r1.xyxw, r6.xyxz
    r1.xyw = ((r1.xyxw)*(r6.xyxz)).xyw;
    // mad r0.yzw, r0.wwww, r6.xxyz, r1.xxyw
    r0.yzw = ((r0.wwww)*(r6.xxyz)+(r1.xxyw)).yzw;
    // mad r1.xyw, -cb0[5].wwww, r5.xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r1.xyw = ((-(source[5].wwww))*(r5.xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // add r0.x, r0.x, l(-1.000000)
    r0.x = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // mad r0.x, -r0.x, l(-499.999969), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(-499.999969,-499.999969,-499.999969,-499.999969))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // mul r3.xyz, cb0[4].xyzx, cb0[4].wwww
    r3.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // mad r0.xyz, r1.xywx, r3.xyzx, r0.yzwy
    r0.xyz = ((r1.xywx)*(r3.xyzx)+(r0.yzwy)).xyz;
    // mul r0.xyz, r0.xyzx, cb0[6].xxxx
    r0.xyz = ((r0.xyzx)*(source[6].xxxx)).xyz;
    // max r0.w, r1.z, l(0.000000)
    r0.w = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// e9aba0e747c157419724e12633e4e3b2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect62(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[9]=0.f;
    source[10]=0.f;
    source[9]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f;
    // ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).x;
    // if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1
    r0.xyz = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).xyz;
    // mul r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)*(r0.xyzx)).xyz;
    // else
    } else {
    // mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t1.yzwx, s3, l(0.000000)
    r0.w = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // div r1.xy, v6.xyxx, v6.wwww
    r1.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s0, l(0.000000)
    r1.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r1.xyxx).xy,0.f).rrrr).xyzw).x;
    // min r1.x, r1.x, l(0.999000)
    r1.x = (min(r1.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // mad r1.y, r1.x, cb2[1].x, cb2[1].y
    r1.y = ((r1.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // mad r1.x, r1.x, cb2[1].z, -cb2[1].w
    r1.x = ((r1.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).x;
    // div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // add r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)+(r1.yyyy)).x;
    // add r1.y, -cb0[8].x, l(1.000000)
    r1.y = ((-(source[8].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // add r1.x, r1.x, -v6.w
    r1.x = ((r1.xxxx)+(-(v6.wwww))).x;
    // max r1.y, r1.y, l(0.001000)
    r1.y = (max(r1.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // endif
    }
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // dp3 r1.w, v3.xyzx, v3.xyzx
    r1.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r2.xyz, r1.wwww, v3.xyzx
    r2.xyz = ((r1.wwww)*(v3.xyzx)).xyz;
    // mad r1.xyz, r1.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r1.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // add r3.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // dp2 r4.x, cb0[2].xyxx, r3.xyxx
    r4.x = (dot((source[2].xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // dp2 r4.y, cb0[3].xyxx, r3.xyxx
    r4.y = (dot((source[3].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // add r3.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r3.xy, r3.xyxx, cb0[4].xyxx
    r3.xy = ((r3.xyxx)*(source[4].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t0.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r4.xyz, -r3.xyzx, r1.wwww
    r4.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // mad r3.xyz, cb0[7].xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((source[7].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // mul r4.xyz, r3.xyzx, cb0[7].zzzz
    r4.xyz = ((r3.xyzx)*(source[7].zzzz)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[6].xyzx
    r4.xyz = ((r4.xyzx)*(source[6].xyzx)).xyz;
    // mad r4.xyz, r4.xyzx, cb2[4].wwww, cb2[4].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r3.xyz, r3.xyzx, cb0[7].yyyy
    r3.xyz = ((r3.xyzx)*(source[7].yyyy)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[5].xyzx
    r3.xyz = ((r3.xyzx)*(source[5].xyzx)).xyz;
    // mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // mov_sat r1.w, r2.z
    r1.w = (saturate(r2.zzzz)).w;
    // lt r2.w, r1.w, l(0.000001)
    r2.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // dp3_sat r1.x, r1.xyzx, r2.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx)).x;
    // lt r1.y, r1.x, l(0.000001)
    r1.y = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // mul r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)*(source[7].wwww)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mul r1.xyz, r4.xyzx, r1.xxxx
    r1.xyz = ((r4.xyzx)*(r1.xxxx)).xyz;
    // mad r1.xyz, r3.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// cc1d22b3b4a7d84694f178861bdac555
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapDirect63(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    [unroll]for(uint target=0u;target<6u;++target)output.targets[target]=0.f;
    float4 source[64];[unroll]for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterLightConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[8]=0.f;
    source[9]=0.f;
    source[10]=0.f;
    source[11]=0.f;
    source[12]=0.f;
    source[13]=0.f;
    source[14]=0.f;
    source[8]=float4(input.lightColor,1.f);
    float4 projection[4];[unroll]for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, v3.xyzx
    r1.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // mul r2.xy, v2.xyxx, cb0[1].xyxx
    r2.xy = ((v2.xyxx)*(source[1].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t0.xywz, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // mad r2.zw, r3.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r3.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r2.zw, r2.zzzw, cb0[5].wwww
    r2.zw = ((r2.zzzw)*(source[5].wwww)).zw;
    // mul r4.xy, r2.zwzz, v0.wwww
    r4.xy = ((r2.zwzz)*(v0.wwww)).xy;
    // dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // div r3.xyw, r4.xyxz, r1.wwww
    r3.xyw = ((r4.xyxz)/(r1.wwww)).xyw;
    // dp3 r1.w, r3.xywx, r3.xywx
    r1.w = (dot((r3.xywx).xyz,(r3.xywx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r4.xyz, r1.wwww, r3.xywx
    r4.xyz = ((r1.wwww)*(r3.xywx)).xyz;
    // dp3 r1.w, r4.xyzx, r0.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // mul r2.zw, r1.wwww, r4.xxxy
    r2.zw = ((r1.wwww)*(r4.xxxy)).zw;
    // mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), -r0.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(-(r0.xxxy))).zw;
    // ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[14].y
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[14].yyyy)) * 0xffffffffu)).w;
    // if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // mul r4.xyzw, v6.yyyy, cb0[10].xyzw
    r4.xyzw = ((v6.yyyy)*(source[10].xyzw)).xyzw;
    // mad r4.xyzw, cb0[9].xyzw, v6.xxxx, r4.xyzw
    r4.xyzw = ((source[9].xyzw)*(v6.xxxx)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[11].xyzw, v6.zzzz, r4.xyzw
    r4.xyzw = ((source[11].xyzw)*(v6.zzzz)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb0[12].xyzw, v6.wwww, r4.xyzw
    r4.xyzw = ((source[12].xyzw)*(v6.wwww)+(r4.xyzw)).xyzw;
    // div r4.xy, r4.xyxx, r4.wwww
    r4.xy = ((r4.xyxx)/(r4.wwww)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r5.x, r4.xyxx, t1.xyzw, s4
    r5.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r4.xyxx).xy,0.f).rrrr).xyzw).x;
    // mov r6.xw, l(0,0,0,0)
    r6.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // mov r6.yz, cb0[13].wwzw
    r6.yz = (source[13].wwzw).yz;
    // add r6.xyzw, r4.xyxy, r6.xyzw
    r6.xyzw = ((r4.xyxy)+(r6.xyzw)).xyzw;
    // sample_indexable(texture2d)(float,float,float,float) r5.y, r6.xyxx, t1.yxzw, s4
    r5.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.xyxx).xy,0.f).rrrr).yxzw).y;
    // sample_indexable(texture2d)(float,float,float,float) r5.z, r6.zwzz, t1.yzxw, s4
    r5.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.zwzz).xy,0.f).rrrr).yzxw).z;
    // add r6.xy, r4.xyxx, cb0[13].zwzz
    r6.xy = ((r4.xyxx)+(source[13].zwzz)).xy;
    // sample_indexable(texture2d)(float,float,float,float) r5.w, r6.xyxx, t1.yzwx, s4
    r5.w = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r6.xyxx).xy,0.f).rrrr).yzwx).w;
    // lt r5.xyzw, r4.zzzz, r5.xyzw
    r5.xyzw = (asfloat((uint4)((r4.zzzz)<(r5.xyzw)) * 0xffffffffu)).xyzw;
    // and r6.xyzw, r5.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r6.xyzw = (asfloat(asuint(r5.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // mul r4.xy, r4.xyxx, cb0[13].xyxx
    r4.xy = ((r4.xyxx)*(source[13].xyxx)).xy;
    // frc r4.xy, r4.xyxx
    r4.xy = (frac(r4.xyxx)).xy;
    // movc r4.zw, r5.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r4.zw = ((asuint(r5.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // add r4.zw, r4.zzzw, r6.zzzw
    r4.zw = ((r4.zzzw)+(r6.zzzw)).zw;
    // mad r4.xz, r4.xxxx, r4.zzwz, r6.xxyx
    r4.xz = ((r4.xxxx)*(r4.zzwz)+(r6.xxyx)).xz;
    // add r1.w, -r4.x, r4.z
    r1.w = ((-(r4.xxxx))+(r4.zzzz)).w;
    // mad r1.w, r4.y, r1.w, r4.x
    r1.w = ((r4.yyyy)*(r1.wwww)+(r4.xxxx)).w;
    // mul r4.xyz, r1.wwww, cb0[14].xxxx
    r4.xyz = ((r1.wwww)*(source[14].xxxx)).xyz;
    // else
    } else {
    // mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // endif
    }
    // add r2.zw, r2.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r2.zzzw)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 0.500000, 0.500000), -v2.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,0.500000,0.500000))+(-(v2.xxxy))).zw;
    // mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 0.750000, 0.750000), v2.xxxy
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,0.750000,0.750000))+(v2.xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r2.zwzz, t3.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r5.xyzx)*(source[2].xyzx)).xyz;
    // mad r5.xyz, cb0[6].xxxx, r5.xyzx, r5.xyzx
    r5.xyz = ((source[6].xxxx)*(r5.xyzx)+(r5.xyzx)).xyz;
    // add r5.xyz, r5.xyzx, -cb0[6].xxxx
    r5.xyz = ((r5.xyzx)+(-(source[6].xxxx))).xyz;
    // mov_sat r6.xyz, r5.xyzx
    r6.xyz = (saturate(r5.xyzx)).xyz;
    // mul r1.w, r3.z, cb0[6].y
    r1.w = ((r3.zzzz)*(source[6].yyyy)).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r2.xyxx, t4.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // mul r8.xyz, cb0[3].xyzx, cb0[6].zzzz
    r8.xyz = ((source[3].xyzx)*(source[6].zzzz)).xyz;
    // mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // mad r6.xyz, r1.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // mad r5.xyz, -r1.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r1.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // max r5.xyz, r5.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r5.xyz = (max(r5.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // min r5.xyz, r5.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // dp3 r1.x, r3.xywx, r1.xyzx
    r1.x = (dot((r3.xywx).xyz,(r1.xyzx).xyz).xxxx).x;
    // max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // min r1.y, r1.x, l(1.000000)
    r1.y = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r6.xyz, cb0[4].xyzx, cb0[6].wwww
    r6.xyz = ((source[4].xyzx)*(source[6].wwww)).xyz;
    // mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // mad r0.xyz, v3.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((v3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // dp3 r0.x, r0.xyzx, r3.xywx
    r0.x = (dot((r0.xyzx).xyz,(r3.xywx).xyz).xxxx).x;
    // lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // mul r0.x, r0.x, cb0[7].x
    r0.x = ((r0.xxxx)*(source[7].xxxx)).x;
    // exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // mul r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // mad r0.xyz, r1.yyyy, r5.xyzx, r0.xyzx
    r0.xyz = ((r1.yyyy)*(r5.xyzx)+(r0.xyzx)).xyz;
    // mul r1.xyz, r1.xxxx, cb2[3].xyzx
    r1.xyz = ((r1.xxxx)*(passValues[3].xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // mul_sat r0.x, r7.w, cb0[7].y
    r0.x = (saturate((r7.wwww)*(source[7].yyyy))).x;
    // mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

#endif
