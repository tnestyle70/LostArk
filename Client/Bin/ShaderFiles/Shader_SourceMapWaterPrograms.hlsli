// Selected Bern source water PS instructions and their native uniform bindings.
#ifndef LOSTARK_SOURCE_MAP_WATER_PROGRAMS
#define LOSTARK_SOURCE_MAP_WATER_PROGRAMS
Texture2D g_SourceMapSceneColor;
SamplerState SourceMapColorSampler { Filter=MIN_MAG_MIP_LINEAR; AddressU=Clamp; AddressV=Clamp; };

// source.map.water-38.v1: 8a7ffc3d78659f41a3aa7a4053b594ad
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater38(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterBaseConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[4]=SourceCharacterAppend(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.100000001f,0.f,0.f,0.f))),frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.100000001f,0.f,0.f,0.f))),1u);
    source[5]=SourceCharacterAppend(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(-0.100000001f,0.f,0.f,0.f))),frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(-0.100000001f,0.f,0.f,0.f))),1u);
    source[6]=SourceCharacterAppend(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.000500000024f,0.f,0.f,0.f))),frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.00499999989f,0.f,0.f,0.f))),1u);
    source[7].z=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // mul r0.x, cb0[7].w, cb0[7].z
    r0.x = ((source[7].wwww)*(source[7].zzzz)).x;
    // mul r0.y, cb0[7].z, cb0[8].x
    r0.y = ((source[7].zzzz)*(source[8].xxxx)).y;
    // add r0.xy, r0.xyxx, v4.xyxx
    r0.xy = ((r0.xyxx)+(v4.xyxx)).xy;
    // mul r0.xy, r0.xyxx, cb0[7].yyyy
    r0.xy = ((r0.xyxx)*(source[7].yyyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t4.yzxw, s4, l(0.000000)
    r0.z = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).z;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r1.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // add r0.xy, r0.zzzz, cb0[6].xyxx
    r0.xy = ((r0.zzzz)+(source[6].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t5.xyzw, s5, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    // mad r2.xyzw, cb0[7].zzzz, cb0[8].yzyz, v4.xyxy
    r2.xyzw = ((source[7].zzzz)*(source[8].yzyz)+(v4.xyxy)).xyzw;
    // mul r2.xyzw, r2.xyzw, cb0[7].xxyy
    r2.xyzw = ((r2.xyzw)*(source[7].xxyy)).xyzw;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.w, r2.zwzz, t4.yzwx, s4, l(0.000000)
    r0.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r0.wwww, t5.zwxy, s5, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.wwww).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r3.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // add r3.z, r0.z, r0.w
    r3.z = ((r0.zzzz)+(r0.wwww)).z;
    // mov r1.z, l(0.000010)
    r1.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // add r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)+(r3.xyzx)).xyz;
    // mad r1.yzw, r1.xxyz, l(0.000000, 0.250000, 0.250000, 0.250000), l(0.000000, -0.000000, -0.000000, -1.000000)
    r1.yzw = ((r1.xxyz)*(float4(0.000000,0.250000,0.250000,0.250000))+(float4(0.000000,-0.000000,-0.000000,-1.000000))).yzw;
    // mul r0.z, r1.x, l(0.250000)
    r0.z = ((r1.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // mad r1.xyz, cb0[8].wwww, r1.yzwy, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((source[8].wwww)*(r1.yzwy)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r3.xyz, r0.wwww, r1.xyzx
    r3.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, r4.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r2.xy, r0.wwww, r3.xyxx
    r2.xy = ((r0.wwww)*(r3.xyxx)).xy;
    // mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r4.xyxx
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r4.xyxx))).xy;
    // dp3 r0.w, r1.xyzx, r4.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mov_sat r1.x, r1.x
    r1.x = (saturate(r1.xxxx)).x;
    // max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r1.yz, r2.xxyx, l(0.000000, 0.600000, 0.600000, 0.000000)
    r1.yz = ((r2.xxyx)*(float4(0.000000,0.600000,0.600000,0.000000))).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r4.xyz, r4.xyzx, cb0[2].xyzx
    r4.xyz = ((r4.xyzx)*(source[2].xyzx)).xyz;
    // mad r0.xy, r0.xyxx, r2.zwzz, r1.yzyy
    r0.xy = ((r0.xyxx)*(r2.zwzz)+(r1.yzyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t6.xyzw, s6, l(0.000000)
    r0.x = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // mul r0.y, r0.y, cb0[10].x
    r0.y = ((r0.yyyy)*(source[10].xxxx)).y;
    // exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // mul r0.y, r0.y, cb0[10].y
    r0.y = ((r0.yyyy)*(source[10].yyyy)).y;
    // movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // log r0.y, r1.x
    r0.y = (log2(r1.xxxx)).y;
    // mul r0.y, r0.y, l(0.400000)
    r0.y = ((r0.yyyy)*(float4(0.400000,0.400000,0.400000,0.400000))).y;
    // exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // lt r1.y, r1.x, l(0.000001)
    r1.y = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // movc r0.y, r1.y, l(0), r0.y
    r0.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // mad r1.yz, v4.xxyx, l(0.000000, 3.000000, 3.000000, 0.000000), cb0[4].xxyx
    r1.yz = ((v4.xxyx)*(float4(0.000000,3.000000,3.000000,0.000000))+(source[4].xxyx)).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t3.wxyz, s3, l(0.000000)
    r1.yzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // mad r2.xy, v4.xyxx, l(5.000000, 5.000000, 0.000000, 0.000000), cb0[5].xyxx
    r2.xy = ((v4.xyxx)*(float4(5.000000,5.000000,0.000000,0.000000))+(source[5].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // add r1.yzw, r1.yyzw, r2.xxyz
    r1.yzw = ((r1.yyzw)+(r2.xxyz)).yzw;
    // mul r0.y, |r0.w|, |r0.w|
    r0.y = ((abs(r0.wwww))*(abs(r0.wwww))).y;
    // mul r0.y, r0.y, |r0.w|
    r0.y = ((r0.yyyy)*(abs(r0.wwww))).y;
    // lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // lt r0.w, r0.y, l(0.000001)
    r0.w = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // mul r0.y, r0.y, cb0[9].y
    r0.y = ((r0.yyyy)*(source[9].yyyy)).y;
    // exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // mul r0.y, r0.y, cb0[9].z
    r0.y = ((r0.yyyy)*(source[9].zzzz)).y;
    // movc r0.y, r0.w, l(0), r0.y
    r0.y = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // mul r2.xyz, r4.xyzx, cb0[9].xxxx
    r2.xyz = ((r4.xyzx)*(source[9].xxxx)).xyz;
    // mad r4.xyz, -cb0[9].xxxx, r4.xyzx, cb0[3].xyzx
    r4.xyz = ((-(source[9].xxxx))*(r4.xyzx)+(source[3].xyzx)).xyz;
    // mad r2.xyz, r0.yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // div r0.yw, v7.xxxy, v7.wwww
    r0.yw = ((v7.xxxy)/(v7.wwww)).yw;
    // mad r0.yw, r0.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r0.yw = ((r0.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t1.yxzw, s0, l(0.000000)
    r0.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.ywyy).xy,0.f).rrrr).yxzw).y;
    // min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // mad r0.w, r0.y, cb2[1].z, -cb2[1].w
    r0.w = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).w;
    // mad r0.y, r0.y, cb2[1].x, cb2[1].y
    r0.y = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // add r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)+(r0.yyyy)).y;
    // add r0.y, r0.y, -v7.w
    r0.y = ((r0.yyyy)+(-(v7.wwww))).y;
    // div_sat r0.w, r0.y, cb0[9].w
    r0.w = (saturate((r0.yyyy)/(source[9].wwww))).w;
    // div_sat r0.y, r0.y, cb0[10].w
    r0.y = (saturate((r0.yyyy)/(source[10].wwww))).y;
    // add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mad r2.xyz, r1.xxxx, r0.wwww, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r0.wwww)+(r2.xyzx)).xyz;
    // mad r1.xyz, r0.xxxx, r1.yzwy, r2.xyzx
    r1.xyz = ((r0.xxxx)*(r1.yzwy)+(r2.xyzx)).xyz;
    // add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // mul r0.x, r0.z, cb0[8].w
    r0.x = ((r0.zzzz)*(source[8].wwww)).x;
    // mad r0.z, cb0[8].w, r0.z, cb0[10].z
    r0.z = ((source[8].wwww)*(r0.zzzz)+(source[10].zzzz)).z;
    // mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // mad r0.x, r0.x, r0.w, r0.y
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.yyyy)).x;
    // mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
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
    // mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r3.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // dp3 r0.x, r1.xyzx, r3.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r1.xyzx, r3.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
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
    // mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// source.map.water-39.v1: 8986e993aac30e4c987be0ae397049db
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater39(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterBaseConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[4]=SourceCharacterAppend(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.00100000005f,0.f,0.f,0.f))),frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.00499999989f,0.f,0.f,0.f))),1u);
    source[5]=SourceCharacterAppend(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.000500000024f,0.f,0.f,0.f))),frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.00499999989f,0.f,0.f,0.f))),1u);
    source[6].z=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    source[8].z=((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.00100000005f,0.f,0.f,0.f))).x;
    source[8].w=((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.00499999989f,0.f,0.f,0.f))).x;
    source[9].x=(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.00499999989f,0.f,0.f,0.f)))).x;
    source[9].y=(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.00100000005f,0.f,0.f,0.f)))).x;
    source[9].w=((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.000500000024f,0.f,0.f,0.f))).x;
    source[10].x=(frac((float4(g_SourceCharacterTime,0.f,0.f,0.f)*float4(0.000500000024f,0.f,0.f,0.f)))).x;
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f;
    // mov r0.y, cb0[7].x
    r0.y = (source[7].xxxx).y;
    // mov r0.xz, l(0,0,0,0)
    r0.xz = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xz;
    // mad r0.xy, cb0[6].zzzz, r0.xyxx, v4.xyxx
    r0.xy = ((source[6].zzzz)*(r0.xyxx)+(v4.xyxx)).xy;
    // mul r0.xy, r0.xyxx, cb0[6].yyyy
    r0.xy = ((r0.xyxx)*(source[6].yyyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r1.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.x, r1.xyxx, r1.xyxx
    r0.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // add r0.x, r0.x, l(0.000010)
    r0.x = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).x;
    // mul r0.w, cb0[6].w, cb0[6].z
    r0.w = ((source[6].wwww)*(source[6].zzzz)).w;
    // add r0.yz, r0.zzwz, v4.xxyx
    r0.yz = ((r0.zzwz)+(v4.xxyx)).yz;
    // mul r0.yz, r0.yyzy, cb0[6].xxxx
    r0.yz = ((r0.yyzy)*(source[6].xxxx)).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t0.zxyw, s1, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // mad r2.xy, r0.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.y, r2.xyxx, r2.xyxx
    r0.y = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // add r2.z, r0.x, r0.y
    r2.z = ((r0.xxxx)+(r0.yyyy)).z;
    // mov r1.z, l(0.000010)
    r1.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // add r0.xyz, r1.xyzx, r2.xyzx
    r0.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // mul r0.w, r0.x, l(0.250000)
    r0.w = ((r0.xxxx)*(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // mad r0.xyz, r0.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), l(-0.000000, -0.000000, -1.000000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r0.xyz, cb0[7].yyyy, r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r0.xyz = ((source[7].yyyy)*(r0.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // mul r1.x, r0.w, cb0[7].y
    r1.x = ((r0.wwww)*(source[7].yyyy)).x;
    // mad r0.w, cb0[7].y, r0.w, cb0[10].w
    r0.w = ((source[7].yyyy)*(r0.wwww)+(source[10].wwww)).w;
    // mov_sat r1.x, r1.x
    r1.x = (saturate(r1.xxxx)).x;
    // div r1.yz, v7.xxyx, v7.wwww
    r1.yz = ((v7.xxyx)/(v7.wwww)).yz;
    // mad r1.yz, r1.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r1.yz = ((r1.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // sample_l_indexable(texture2d)(float,float,float,float) r1.y, r1.yzyy, t1.yxzw, s0, l(0.000000)
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r1.yzyy).xy,0.f).rrrr).yxzw).y;
    // min r1.y, r1.y, l(0.999000)
    r1.y = (min(r1.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // mad r1.z, r1.y, cb2[1].z, -cb2[1].w
    r1.z = ((r1.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).z;
    // mad r1.y, r1.y, cb2[1].x, cb2[1].y
    r1.y = ((r1.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // div r1.z, l(1.000000, 1.000000, 1.000000, 1.000000), r1.z
    r1.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.zzzz)).z;
    // add r1.y, r1.z, r1.y
    r1.y = ((r1.zzzz)+(r1.yyyy)).y;
    // add r1.y, r1.y, -v7.w
    r1.y = ((r1.yyyy)+(-(v7.wwww))).y;
    // div_sat r1.z, r1.y, cb0[11].x
    r1.z = (saturate((r1.yyyy)/(source[11].xxxx))).z;
    // div_sat r1.y, r1.y, cb0[8].y
    r1.y = (saturate((r1.yyyy)/(source[8].yyyy))).y;
    // add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mad r1.x, r1.x, r1.y, r1.z
    r1.x = ((r1.xxxx)*(r1.yyyy)+(r1.zzzz)).x;
    // mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xzw, r0.wwww, v6.xxyz
    r1.xzw = ((r0.wwww)*(v6.xxyz)).xzw;
    // dp3 r0.w, r0.xyzx, r1.xzwx
    r0.w = (dot((r0.xyzx).xyz,(r1.xzwx).xyz).xxxx).w;
    // max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r2.x, |r0.w|, |r0.w|
    r2.x = ((abs(r0.wwww))*(abs(r0.wwww))).x;
    // mul r2.x, |r0.w|, r2.x
    r2.x = ((abs(r0.wwww))*(r2.xxxx)).x;
    // lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // movc r0.w, r0.w, l(0), r2.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // log r2.x, r0.w
    r2.x = (log2(r0.wwww)).x;
    // lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r2.x, r2.x, cb0[7].w
    r2.x = ((r2.xxxx)*(source[7].wwww)).x;
    // exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // mul r2.x, r2.x, cb0[8].x
    r2.x = ((r2.xxxx)*(source[8].xxxx)).x;
    // movc r0.w, r0.w, l(0), r2.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // dp3 r2.x, r0.xyzx, r0.xyzx
    r2.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // mul r2.xyz, r0.xyzx, r2.xxxx
    r2.xyz = ((r0.xyzx)*(r2.xxxx)).xyz;
    // mov_sat r0.x, r0.x
    r0.x = (saturate(r0.xxxx)).x;
    // dp3 r0.y, r2.xyzx, r1.xzwx
    r0.y = (dot((r2.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // mul r0.yz, r0.yyyy, r2.xxyx
    r0.yz = ((r0.yyyy)*(r2.xxyx)).yz;
    // mad r0.yz, r0.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), -r1.xxzx
    r0.yz = ((r0.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(-(r1.xxzx))).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r0.yzyy, t2.xwyz, s2, l(0.000000)
    r1.xzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // dp3 r2.w, r1.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r3.xyz, -r1.xzwx, r2.wwww
    r3.xyz = ((-(r1.xzwx))+(r2.wwww)).xyz;
    // mad r1.xzw, cb0[7].zzzz, r3.xxyz, r1.xxzw
    r1.xzw = ((source[7].zzzz)*(r3.xxyz)+(r1.xxzw)).xzw;
    // mul r3.xyz, r1.xzwx, cb0[2].xyzx
    r3.xyz = ((r1.xzwx)*(source[2].xyzx)).xyz;
    // mad r1.xzw, -cb0[2].xxyz, r1.xxzw, cb0[3].xxyz
    r1.xzw = ((-(source[2].xxyz))*(r1.xxzw)+(source[3].xxyz)).xzw;
    // mad r1.xzw, r0.wwww, r1.xxzw, r3.xxyz
    r1.xzw = ((r0.wwww)*(r1.xxzw)+(r3.xxyz)).xzw;
    // mad r1.xyz, r0.xxxx, r1.yyyy, r1.xzwx
    r1.xyz = ((r0.xxxx)*(r1.yyyy)+(r1.xzwx)).xyz;
    // mul r3.x, cb0[6].z, cb0[9].z
    r3.x = ((source[6].zzzz)*(source[9].zzzz)).x;
    // mul r3.y, cb0[6].z, cb0[7].x
    r3.y = ((source[6].zzzz)*(source[7].xxxx)).y;
    // add r3.xy, r3.xyxx, v4.xyxx
    r3.xy = ((r3.xyxx)+(v4.xyxx)).xy;
    // mul r3.xy, r3.xyxx, cb0[6].yyyy
    r3.xy = ((r3.xyxx)*(source[6].yyyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.w, r3.xyxx, t4.yzwx, s4, l(0.000000)
    r0.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // add r3.xy, r0.wwww, cb0[5].xyxx
    r3.xy = ((r0.wwww)+(source[5].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t5.xyzw, s5, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), r0.wwww
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(r0.wwww)).xy;
    // add r3.xy, r3.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mad r0.yz, r0.yyzy, l(0.000000, 0.600000, 0.600000, 0.000000), r3.xxyx
    r0.yz = ((r0.yyzy)*(float4(0.000000,0.600000,0.600000,0.000000))+(r3.xxyx)).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t3.yxzw, s3, l(0.000000)
    r0.y = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // mul r0.z, r0.z, cb0[10].y
    r0.z = ((r0.zzzz)*(source[10].yyyy)).z;
    // exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // mul r0.z, r0.z, cb0[10].z
    r0.z = ((r0.zzzz)*(source[10].zzzz)).z;
    // movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // add r0.zw, v4.xxxy, cb0[4].xxxy
    r0.zw = ((v4.xxxy)+(source[4].xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t3.yzxw, s3, l(0.000000)
    r0.z = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).z;
    // mul r0.z, r0.z, l(40.000000)
    r0.z = ((r0.zzzz)*(float4(40.000000,40.000000,40.000000,40.000000))).z;
    // mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // mul r0.z, r0.z, l(0.400000)
    r0.z = ((r0.zzzz)*(float4(0.400000,0.400000,0.400000,0.400000))).z;
    // exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // movc r0.x, r0.x, l(0), r0.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).x;
    // mad r0.xyz, r0.yyyy, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xxxx)+(r1.xyzx)).xyz;
    // add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
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
    // mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
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
    // mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

// source.map.water-40.v1: c64d20228d45c94b8884b46b7de315fe
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater40(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterBaseConstants[i];
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[7].y=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    source[11]=0.f;source[12]=0.f;source[13]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // mul r0.xy, cb0[2].zwzz, cb0[7].yyyy
    r0.xy = ((source[2].zwzz)*(source[7].yyyy)).xy;
    // mad r0.zw, cb0[7].xxxx, v4.xxxy, r0.xxxy
    r0.zw = ((source[7].xxxx)*(v4.xxxy)+(r0.xxxy)).zw;
    // mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s2, l(0.000000)
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
    // add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // mul r0.zw, v4.xxxy, cb0[7].xxxx
    r0.zw = ((v4.xxxy)*(source[7].xxxx)).zw;
    // mad r0.xy, r0.xyxx, l(-0.300000, -0.300000, 0.000000, 0.000000), r0.zwzz
    r0.xy = ((r0.xyxx)*(float4(-0.300000,-0.300000,0.000000,0.000000))+(r0.zwzz)).xy;
    // mad r0.zw, cb0[7].yyyy, cb0[3].zzzw, r0.zzzw
    r0.zw = ((source[7].yyyy)*(source[3].zzzw)+(r0.zzzw)).zw;
    // mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // mul r0.xy, r0.xyxx, l(0.300000, 0.300000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.300000,0.300000,0.000000,0.000000))).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s2, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r2.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp2 r0.x, r2.xyxx, r2.xyxx
    r0.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // mul r2.xyzw, r1.xyxy, cb0[7].zzww
    r2.xyzw = ((r1.xyxy)*(source[7].zzww)).xyzw;
    // mad r0.xy, r0.zwzz, cb0[3].xyxx, r2.zwzz
    r0.xy = ((r0.zwzz)*(source[3].xyxx)+(r2.zwzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mad r0.w, -v6.z, r0.z, l(1.000000)
    r0.w = ((-(v6.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r3.xyz, r0.zzzz, v6.xyzx
    r3.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // log r0.z, |r0.w|
    r0.z = (log2(abs(r0.wwww))).z;
    // lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r0.z, r0.z, cb0[8].x
    r0.z = ((r0.zzzz)*(source[8].xxxx)).z;
    // exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // mul_sat r0.z, r0.z, cb0[8].y
    r0.z = (saturate((r0.zzzz)*(source[8].yyyy))).z;
    // movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.w, r0.w, cb0[8].z
    r0.w = ((r0.wwww)*(source[8].zzzz)).w;
    // mad r1.xy, r0.wwww, r0.xyxx, r2.xyxx
    r1.xy = ((r0.wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r2.xyz, r0.xxxx, r1.xyzx
    r2.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // div r0.xyw, r1.xyxz, r0.yyyy
    r0.xyw = ((r1.xyxz)/(r0.yyyy)).xyw;
    // mul r1.xy, r0.xyxx, cb0[8].wwww
    r1.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // add r0.xyw, r0.xyxw, l(-0.000000, -0.000000, 0.000000, -1.000000)
    r0.xyw = ((r0.xyxw)+(float4(-0.000000,-0.000000,0.000000,-1.000000))).xyw;
    // mad r0.xyw, cb0[10].xxxx, r0.xyxw, l(0.000000, 0.000000, 0.000000, 1.000000)
    r0.xyw = ((source[10].xxxx)*(r0.xyxw)+(float4(0.000000,0.000000,0.000000,1.000000))).xyw;
    // div r4.xyz, v8.xyzx, v8.wwww
    r4.xyz = ((v8.xyzx)/(v8.wwww)).xyz;
    // div r1.xy, r1.xyxx, r4.zzzz
    r1.xy = ((r1.xyxx)/(r4.zzzz)).xy;
    // mad r1.zw, r4.xxxy, cb2[0].xxxy, cb2[0].wwwz
    r1.zw = ((r4.xxxy)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t2.yzxw, s0, l(0.000000)
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r1.zwzz).xy,0.f).rrrr).yzxw).z;
    // min r1.z, r1.z, l(0.999000)
    r1.z = (min(r1.zzzz,float4(0.999000,0.999000,0.999000,0.999000))).z;
    // sample_indexable(texture2d)(float,float,float,float) r1.xyw, r1.xyxx, t5.xywz, s1
    r1.xyw = ((g_SourceMapSceneColor.SampleLevel(SourceMapColorSampler,(r1.xyxx).xy,0.f)).xywz).xyw;
    // mad r2.w, r1.z, cb2[1].z, -cb2[1].w
    r2.w = ((r1.zzzz)*(passValues[1].zzzz)+(-(passValues[1].wwww))).w;
    // mad r1.z, r1.z, cb2[1].x, cb2[1].y
    r1.z = ((r1.zzzz)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // add r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)+(r2.wwww)).z;
    // add r1.z, r1.z, -v8.w
    r1.z = ((r1.zzzz)+(-(v8.wwww))).z;
    // add r2.w, -cb0[9].x, l(1.000000)
    r2.w = ((-(source[9].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // div_sat r1.z, r1.z, r2.w
    r1.z = (saturate((r1.zzzz)/(r2.wwww))).z;
    // lt r2.w, r1.z, l(0.000001)
    r2.w = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // mul r1.z, r1.z, cb0[9].y
    r1.z = ((r1.zzzz)*(source[9].yyyy)).z;
    // exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // movc r1.z, r2.w, l(0), r1.z
    r1.z = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // mul_sat r2.w, r1.z, cb0[9].z
    r2.w = (saturate((r1.zzzz)*(source[9].zzzz))).w;
    // mul o0.w, r1.z, cb0[0].x
    output.targets[0].w = ((r1.zzzz)*(source[0].xxxx)).w;
    // add r1.z, -r2.w, l(1.000000)
    r1.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mad r1.xyz, r1.zzzz, r1.xywx, cb0[1].xyzx
    r1.xyz = ((r1.zzzz)*(r1.xywx)+(source[1].xyzx)).xyz;
    // dp3 r0.w, r0.xywx, r3.xyzx
    r0.w = (dot((r0.xywx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)*(r0.wwww)).xy;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r3.xyxx
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r3.xyxx))).xy;
    // mul r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)*(source[6].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t4.xywz, s5, l(0.000000)
    r0.xyw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyw;
    // mul r0.xyw, r0.xyxw, cb0[5].xyxz
    r0.xyw = ((r0.xyxw)*(source[5].xyxz)).xyw;
    // dp3 r1.w, r2.xyzx, r3.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r3.zw, r1.wwww, r2.xxxy
    r3.zw = ((r1.wwww)*(r2.xxxy)).zw;
    // mad r3.xy, r3.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), -r3.xyxx
    r3.xy = ((r3.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r3.xyxx))).xy;
    // add r3.xy, r3.xyxx, -v4.xyxx
    r3.xy = ((r3.xyxx)+(-(v4.xyxx))).xy;
    // mad r3.xy, r3.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000), v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v4.xyxx)).xy;
    // mul r3.xy, r3.xyxx, cb0[9].wwww
    r3.xy = ((r3.xyxx)*(source[9].wwww)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[4].xyzx
    r3.xyz = ((r3.xyzx)*(source[4].xyzx)).xyz;
    // mad r0.xyw, cb0[10].yyyy, r0.xyxw, -r3.xyxz
    r0.xyw = ((source[10].yyyy)*(r0.xyxw)+(-(r3.xyxz))).xyw;
    // mad r0.xyz, r0.zzzz, r0.xywx, r3.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r3.xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // mul r3.yzw, r3.yyyy, cb0[12].xxyz
    r3.yzw = ((r3.yyyy)*(source[12].xxyz)).yzw;
    // mad r3.xyz, r3.xxxx, cb0[11].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[11].xyzx)+(r3.yzwy)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[13].wwww
    r3.xyz = ((r3.xyzx)*(source[13].wwww)).xyz;
    // mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r1.xyz, r0.xyzx, cb0[13].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[13].xyzx)+(r1.xyzx)).xyz;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
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
    // mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
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

// source.map.water-41.v1: 637ea4d18458f4439f47c40d9b5c1de7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater41(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterBaseConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[7].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    source[11]=0.f;source[12]=0.f;source[13]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f;
    // add r0.xy, v8.xyxx, cb0[0].xyxx
    r0.xy = ((v8.xyxx)+(source[0].xyxx)).xy;
    // mul r0.zw, cb0[2].zzzw, cb0[7].xxxx
    r0.zw = ((source[2].zzzw)*(source[7].xxxx)).zw;
    // mad r1.xy, r0.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000), r0.zwzz
    r1.xy = ((r0.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))+(r0.zwzz)).xy;
    // mul r0.xy, r0.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))).xy;
    // mad r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.300000, -0.300000), r0.xxxy
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,-0.300000,-0.300000))+(r0.xxxy)).zw;
    // mad r0.xy, cb0[7].xxxx, cb0[3].zwzz, r0.xyxx
    r0.xy = ((source[7].xxxx)*(source[3].zwzz)+(r0.xyxx)).xy;
    // mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.300000, 0.300000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.300000,0.300000))).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s2, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r0.zw, r1.xxxy, cb0[2].xxxy
    r0.zw = ((r1.xxxy)*(source[2].xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s2, l(0.000000)
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
    // mul r2.xyzw, r1.xyxy, cb0[7].yyzz
    r2.xyzw = ((r1.xyxy)*(source[7].yyzz)).xyzw;
    // mad r0.xy, r0.xyxx, cb0[3].xyxx, r2.zwzz
    r0.xy = ((r0.xyxx)*(source[3].xyxx)+(r2.zwzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mad r0.w, -v6.z, r0.z, l(1.000000)
    r0.w = ((-(v6.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r3.xyz, r0.zzzz, v6.xyzx
    r3.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // log r0.z, |r0.w|
    r0.z = (log2(abs(r0.wwww))).z;
    // lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // mul_sat r0.z, r0.z, cb0[8].x
    r0.z = (saturate((r0.zzzz)*(source[8].xxxx))).z;
    // movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // mad r1.xy, r0.wwww, r0.xyxx, r2.xyxx
    r1.xy = ((r0.wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r2.xyz, r0.xxxx, r1.xyzx
    r2.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // div r0.xyw, r1.xyxz, r0.yyyy
    r0.xyw = ((r1.xyxz)/(r0.yyyy)).xyw;
    // mul r1.xy, r0.xyxx, cb0[8].zzzz
    r1.xy = ((r0.xyxx)*(source[8].zzzz)).xy;
    // add r0.xyw, r0.xyxw, l(-0.000000, -0.000000, 0.000000, -1.000000)
    r0.xyw = ((r0.xyxw)+(float4(-0.000000,-0.000000,0.000000,-1.000000))).xyw;
    // mad r0.xyw, cb0[9].wwww, r0.xyxw, l(0.000000, 0.000000, 0.000000, 1.000000)
    r0.xyw = ((source[9].wwww)*(r0.xyxw)+(float4(0.000000,0.000000,0.000000,1.000000))).xyw;
    // mul r4.xyzw, v8.yyyy, cb1[1].xyzw
    r4.xyzw = ((v8.yyyy)*(projection[1].xyzw)).xyzw;
    // mad r4.xyzw, cb1[0].xyzw, v8.xxxx, r4.xyzw
    r4.xyzw = ((projection[0].xyzw)*(v8.xxxx)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb1[2].xyzw, v8.zzzz, r4.xyzw
    r4.xyzw = ((projection[2].xyzw)*(v8.zzzz)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb1[3].xyzw, v8.wwww, r4.xyzw
    r4.xyzw = ((projection[3].xyzw)*(v8.wwww)+(r4.xyzw)).xyzw;
    // div r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)/(r4.wwww)).xyz;
    // div r1.xy, r1.xyxx, r4.zzzz
    r1.xy = ((r1.xyxx)/(r4.zzzz)).xy;
    // mad r1.zw, r4.xxxy, cb2[0].xxxy, cb2[0].wwwz
    r1.zw = ((r4.xxxy)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t2.yzxw, s0, l(0.000000)
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r1.zwzz).xy,0.f).rrrr).yzxw).z;
    // min r1.z, r1.z, l(0.999000)
    r1.z = (min(r1.zzzz,float4(0.999000,0.999000,0.999000,0.999000))).z;
    // sample_indexable(texture2d)(float,float,float,float) r1.xyw, r1.xyxx, t5.xywz, s1
    r1.xyw = ((g_SourceMapSceneColor.SampleLevel(SourceMapColorSampler,(r1.xyxx).xy,0.f)).xywz).xyw;
    // mad r2.w, r1.z, cb2[1].z, -cb2[1].w
    r2.w = ((r1.zzzz)*(passValues[1].zzzz)+(-(passValues[1].wwww))).w;
    // mad r1.z, r1.z, cb2[1].x, cb2[1].y
    r1.z = ((r1.zzzz)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // add r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)+(r2.wwww)).z;
    // add r1.z, -r4.w, r1.z
    r1.z = ((-(r4.wwww))+(r1.zzzz)).z;
    // add r2.w, -cb0[8].w, l(1.000000)
    r2.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // div_sat r1.z, r1.z, r2.w
    r1.z = (saturate((r1.zzzz)/(r2.wwww))).z;
    // lt r2.w, r1.z, l(0.000001)
    r2.w = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // mul r1.z, r1.z, cb0[9].x
    r1.z = ((r1.zzzz)*(source[9].xxxx)).z;
    // exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // movc r1.z, r2.w, l(0), r1.z
    r1.z = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // mul_sat r2.w, r1.z, cb0[9].y
    r2.w = (saturate((r1.zzzz)*(source[9].yyyy))).w;
    // mul o0.w, r1.z, cb0[0].w
    output.targets[0].w = ((r1.zzzz)*(source[0].wwww)).w;
    // add r1.z, -r2.w, l(1.000000)
    r1.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mad r1.xyz, r1.zzzz, r1.xywx, cb0[1].xyzx
    r1.xyz = ((r1.zzzz)*(r1.xywx)+(source[1].xyzx)).xyz;
    // dp3 r0.w, r0.xywx, r3.xyzx
    r0.w = (dot((r0.xywx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)*(r0.wwww)).xy;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r3.xyxx
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r3.xyxx))).xy;
    // mul r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)*(source[6].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t4.xywz, s5, l(0.000000)
    r0.xyw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyw;
    // mul r0.xyw, r0.xyxw, cb0[5].xyxz
    r0.xyw = ((r0.xyxw)*(source[5].xyxz)).xyw;
    // dp3 r1.w, r2.xyzx, r3.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r3.zw, r1.wwww, r2.xxxy
    r3.zw = ((r1.wwww)*(r2.xxxy)).zw;
    // mad r3.xy, r3.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), -r3.xyxx
    r3.xy = ((r3.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r3.xyxx))).xy;
    // add r3.xy, r3.xyxx, -v4.xyxx
    r3.xy = ((r3.xyxx)+(-(v4.xyxx))).xy;
    // mad r3.xy, r3.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000), v4.xyxx
    r3.xy = ((r3.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v4.xyxx)).xy;
    // mul r3.xy, r3.xyxx, cb0[9].zzzz
    r3.xy = ((r3.xyxx)*(source[9].zzzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[4].xyzx
    r3.xyz = ((r3.xyzx)*(source[4].xyzx)).xyz;
    // mad r0.xyw, cb0[10].xxxx, r0.xyxw, -r3.xyxz
    r0.xyw = ((source[10].xxxx)*(r0.xyxw)+(-(r3.xyxz))).xyw;
    // mad r0.xyz, r0.zzzz, r0.xywx, r3.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r3.xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // mul r3.yzw, r3.yyyy, cb0[12].xxyz
    r3.yzw = ((r3.yyyy)*(source[12].xxyz)).yzw;
    // mad r3.xyz, r3.xxxx, cb0[11].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[11].xyzx)+(r3.yzwy)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[13].wwww
    r3.xyz = ((r3.xyzx)*(source[13].wwww)).xyz;
    // mad r1.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r1.xyz, r0.xyzx, cb0[13].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[13].xyzx)+(r1.xyzx)).xyz;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
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
    // mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
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

// source.map.water-42.v1: 97b4376d3820d54fabb6cab16439a3af
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater42(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterBaseConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[11].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    source[18]=0.f;source[19]=0.f;source[20]=float4(g_SourceMapAmbient.rgb,1.f);
    source[17]=0.f; // Engine view-owned extra directional contribution is unbound.
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f;
    // mul r0.xy, cb0[2].zwzz, cb0[11].xxxx
    r0.xy = ((source[2].zwzz)*(source[11].xxxx)).xy;
    // add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
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
    // mul r2.xyzw, r0.xyxy, cb0[11].yyzz
    r2.xyzw = ((r0.xyxy)*(source[11].yyzz)).xyzw;
    // mul r3.zw, cb0[3].zzzw, cb0[11].xxxx
    r3.zw = ((source[3].zzzw)*(source[11].xxxx)).zw;
    // mad r4.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000), r3.zwzz
    r4.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))+(r3.zwzz)).xy;
    // mad r3.zw, r3.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000), r3.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,-0.500000,-0.500000))+(r3.xxxy)).zw;
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
    // mad r2.xy, cb0[11].wwww, r2.zwzz, r2.xyxx
    r2.xy = ((source[11].wwww)*(r2.zwzz)+(r2.xyxx)).xy;
    // mov r2.z, r0.z
    r2.z = (r0.zzzz).z;
    // mad r0.xy, cb0[14].yyyy, r0.xyxx, v4.xyxx
    r0.xy = ((source[14].yyyy)*(r0.xyxx)+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s7, l(0.000000)
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
    // mul r1.x, r1.x, cb0[12].x
    r1.x = ((r1.xxxx)*(source[12].xxxx)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mad r0.yzw, r1.xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // mul_sat r1.x, r1.x, cb0[14].x
    r1.x = (saturate((r1.xxxx)*(source[14].xxxx))).x;
    // dp3 r1.y, r0.yzwy, r0.yzwy
    r1.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r2.xyz, r0.yzwy, r1.yyyy
    r2.xyz = ((r0.yzwy)*(r1.yyyy)).xyz;
    // div r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)/(r1.zzzz)).yzw;
    // add r1.yzw, r0.yyzw, l(0.000000, -0.000000, -0.000000, -1.000000)
    r1.yzw = ((r0.yyzw)+(float4(0.000000,-0.000000,-0.000000,-1.000000))).yzw;
    // mad r1.yzw, cb0[13].xxxx, r1.yyzw, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.yzw = ((source[13].xxxx)*(r1.yyzw)+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r4.xyz, r2.wwww, v6.xyzx
    r4.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // dp3 r1.w, r1.yzwy, r4.xyzx
    r1.w = (dot((r1.yzwy).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r1.yz, r1.yyzy, r1.wwww
    r1.yz = ((r1.yyzy)*(r1.wwww)).yz;
    // mad r1.yz, r1.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), -r4.xxyx
    r1.yz = ((r1.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(-(r4.xxyx))).yz;
    // mul r1.yz, r1.yyzy, cb0[7].xxyx
    r1.yz = ((r1.yyzy)*(source[7].xxyx)).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t4.wxyz, s4, l(0.000000)
    r1.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // mul r1.yzw, r1.yyzw, cb0[6].xxyz
    r1.yzw = ((r1.yyzw)*(source[6].xxyz)).yzw;
    // dp3 r2.w, r2.xyzx, r4.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r3.zw, r2.wwww, r2.xxxy
    r3.zw = ((r2.wwww)*(r2.xxxy)).zw;
    // mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), -r4.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(-(r4.xxxy))).zw;
    // dp3 r0.w, r0.yzwy, r4.xyzx
    r0.w = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).w;
    // mul r0.yz, r0.yyzy, cb0[12].yyyy
    r0.yz = ((r0.yyzy)*(source[12].yyyy)).yz;
    // add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // add r3.zw, r3.zzzw, -v4.xxxy
    r3.zw = ((r3.zzzw)+(-(v4.xxxy))).zw;
    // mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 0.050000, 0.050000), v4.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,0.050000,0.050000))+(v4.xxxy)).zw;
    // mul r3.zw, r3.zzzw, cb0[13].wwww
    r3.zw = ((r3.zzzw)*(source[13].wwww)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.zwzz, t5.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r4.xyz, r4.xyzx, cb0[8].xyzx
    r4.xyz = ((r4.xyzx)*(source[8].xyzx)).xyz;
    // mul r4.xyz, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // mad r1.x, -r0.w, l(0.500000), l(1.000000)
    r1.x = ((-(r0.wwww))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // log r2.w, |r1.x|
    r2.w = (log2(abs(r1.xxxx))).w;
    // lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // mul r2.w, r2.w, cb0[13].y
    r2.w = ((r2.wwww)*(source[13].yyyy)).w;
    // exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // mul r2.w, r2.w, cb0[13].z
    r2.w = ((r2.wwww)*(source[13].zzzz)).w;
    // movc r1.x, r1.x, l(0), r2.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).x;
    // mad r1.xyz, r1.xxxx, r1.yzwy, r4.xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(r4.xyzx)).xyz;
    // add r4.xyz, -r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r1.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mad r3.zw, cb0[11].xxxx, cb0[10].zzzw, r3.xxxy
    r3.zw = ((source[11].xxxx)*(source[10].zzzw)+(r3.xxxy)).zw;
    // mad r3.xy, cb0[11].xxxx, cb0[5].zwzz, r3.xyxx
    r3.xy = ((source[11].xxxx)*(source[5].zwzz)+(r3.xyxx)).xy;
    // mad r3.xy, r3.xyxx, cb0[5].xyxx, r0.yzyy
    r3.xy = ((r3.xyxx)*(source[5].xyxx)+(r0.yzyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r3.xyxx, t7.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r5.xyz, r5.xyzx, cb0[4].xyzx
    r5.xyz = ((r5.xyzx)*(source[4].xyzx)).xyz;
    // mul r3.xy, r3.zwzz, cb0[10].xyxx
    r3.xy = ((r3.zwzz)*(source[10].xyxx)).xy;
    // mad r3.zw, r3.zzzw, cb0[10].xxxy, r0.yyyz
    r3.zw = ((r3.zzzw)*(source[10].xxxy)+(r0.yyyz)).zw;
    // mad r0.yz, r3.xxyx, l(0.000000, 2.000000, 2.000000, 0.000000), r0.yyzy
    r0.yz = ((r3.xxyx)*(float4(0.000000,2.000000,2.000000,0.000000))+(r0.yyzy)).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r0.yzyy, t6.xyzw, s6, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.zwzz, t6.xyzw, s6, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r6.xyz, -r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), r6.xyzx
    r6.xyz = ((-(r3.xyzx))*(float4(2.000000,2.000000,2.000000,0.000000))+(r6.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)+(r3.xyzx)).xyz;
    // mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // add r6.xyz, -r3.xyzx, r0.yyyy
    r6.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // mad r3.xyz, cb0[14].wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((source[14].wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r3.xyzx)*(source[9].xyzx)).xyz;
    // mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // mad r1.xyz, r3.xyzx, r4.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // log r0.y, |r0.w|
    r0.y = (log2(abs(r0.wwww))).y;
    // lt r0.z, |r0.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // mul r0.y, r0.y, cb0[12].z
    r0.y = ((r0.yyyy)*(source[12].zzzz)).y;
    // exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // mul r0.y, r0.y, cb0[12].w
    r0.y = ((r0.yyyy)*(source[12].wwww)).y;
    // movc r0.y, r0.z, l(0), r0.y
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // mad r0.yzw, r0.yyyy, r5.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyyy)*(r5.xxyz)+(source[1].xxyz)).yzw;
    // dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // mul r3.xyz, r1.wwww, v7.xyzx
    r3.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // dp3 r1.w, r3.xyzx, r2.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // mad r3.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // mul r3.yzw, r3.yyyy, cb0[19].xxyz
    r3.yzw = ((r3.yyyy)*(source[19].xxyz)).yzw;
    // mad r3.xyz, r3.xxxx, cb0[18].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[18].xyzx)+(r3.yzwy)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[20].wwww
    r3.xyz = ((r3.xyzx)*(source[20].wwww)).xyz;
    // mad r0.yzw, r3.xxyz, r1.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r0.yzw, r1.xxyz, cb0[20].xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(source[20].xxyz)+(r0.yyzw)).yzw;
    // mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // mul r0.yzw, v8.yyyy, cb1[1].xxyw
    r0.yzw = ((v8.yyyy)*(projection[1].xxyw)).yzw;
    // mad r0.yzw, cb1[0].xxyw, v8.xxxx, r0.yyzw
    r0.yzw = ((projection[0].xxyw)*(v8.xxxx)+(r0.yyzw)).yzw;
    // mad r0.yzw, cb1[2].xxyw, v8.zzzz, r0.yyzw
    r0.yzw = ((projection[2].xxyw)*(v8.zzzz)+(r0.yyzw)).yzw;
    // mad r0.yzw, cb1[3].xxyw, v8.wwww, r0.yyzw
    r0.yzw = ((projection[3].xxyw)*(v8.wwww)+(r0.yyzw)).yzw;
    // div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s0, l(0.000000)
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
    // add r0.z, -cb0[15].z, l(1.000000)
    r0.z = ((-(source[15].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // mul r0.y, r0.y, cb0[15].w
    r0.y = ((r0.yyyy)*(source[15].wwww)).y;
    // log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
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
    // mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
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
    // mul_sat o3.w, cb0[15].y, l(0.002000)
    output.targets[3].w = (saturate((source[15].yyyy)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // ftou r0.x, cb0[17].z
    r0.x = (asfloat((uint4)(source[17].zzzz))).x;
    // bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // mad r0.xyz, cb0[15].xxxx, cb2[4].wwww, cb2[4].xyzx
    r0.xyz = ((source[15].xxxx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul_sat r0.xyz, r0.xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((r0.xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // ret
    return output;
}

// source.map.water-43.v1: ca1aec15d83f9c4f9d9a9a968ac7ac93
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater43(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i)source[i]=g_SourceCharacterBaseConstants[i];
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[9].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    source[10]=0.f;source[11]=0.f;source[12]=float4(g_SourceMapAmbient.rgb,1.f);
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s0, l(0.000000)
    r0.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // min r0.x, r0.x, l(0.999000)
    r0.x = (min(r0.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // mad r0.y, r0.x, cb2[1].z, -cb2[1].w
    r0.y = ((r0.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // mad r0.x, r0.x, cb2[1].x, cb2[1].y
    r0.x = ((r0.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).x;
    // div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // add r0.x, -r0.z, r0.x
    r0.x = ((-(r0.zzzz))+(r0.xxxx)).x;
    // add r0.y, -cb0[9].y, l(1.000000)
    r0.y = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // div_sat r0.y, r0.x, r0.y
    r0.y = (saturate((r0.xxxx)/(r0.yyyy))).y;
    // mul_sat r0.xzw, r0.xxxx, l(0.034483, 0.000000, 0.020408, 0.005025)
    r0.xzw = (saturate((r0.xxxx)*(float4(0.034483,0.000000,0.020408,0.005025)))).xzw;
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mad r1.y, -v5.z, r1.x, l(1.000000)
    r1.y = ((-(v5.zzzz))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mul r1.x, r1.x, v5.z
    r1.x = ((r1.xxxx)*(v5.zzzz)).x;
    // mul r1.z, |r1.y|, |r1.y|
    r1.z = ((abs(r1.yyyy))*(abs(r1.yyyy))).z;
    // mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mov_sat r1.y, r1.y
    r1.y = (saturate(r1.yyyy)).y;
    // movc r1.z, r1.w, l(0), r1.z
    r1.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // add r0.y, r0.y, r1.z
    r0.y = ((r0.yyyy)+(r1.zzzz)).y;
    // add r1.zw, v7.xxxy, cb0[0].xxxy
    r1.zw = ((v7.xxxy)+(source[0].xxxy)).zw;
    // mul r1.zw, r1.zzzw, l(0.000000, 0.000000, 0.000300, 0.000300)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.000300,0.000300))).zw;
    // mad r2.xy, cb0[9].xxxx, l(0.090000, 0.090000, 0.000000, 0.000000), r1.zwzz
    r2.xy = ((source[9].xxxx)*(float4(0.090000,0.090000,0.000000,0.000000))+(r1.zwzz)).xy;
    // mul r2.xy, r2.xyxx, l(6.283185, 6.283185, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(6.283185,6.283185,0.000000,0.000000))).xy;
    // sincos r2.x, null, r2.x
    r2.x = (sin(r2.xxxx)).x;
    // sincos null, r2.y, r2.y
    r2.y = (cos(r2.yyyy)).y;
    // mad r3.y, r2.y, l(0.005000), r1.w
    r3.y = ((r2.yyyy)*(float4(0.005000,0.005000,0.005000,0.005000))+(r1.wwww)).y;
    // mad r3.x, r2.x, l(0.005000), r1.z
    r3.x = ((r2.xxxx)*(float4(0.005000,0.005000,0.005000,0.005000))+(r1.zzzz)).x;
    // mul r2.xyzw, cb0[2].zwzw, cb0[9].xxxx
    r2.xyzw = ((source[2].zwzw)*(source[9].xxxx)).xyzw;
    // mul r4.xyzw, r2.zwzw, l(1.000000, 0.000000, -1.000000, 0.000000)
    r4.xyzw = ((r2.zwzw)*(float4(1.000000,0.000000,-1.000000,0.000000))).xyzw;
    // mul r2.xyzw, r2.xyzw, l(0.000000, 1.000000, 0.000000, -1.000000)
    r2.xyzw = ((r2.xyzw)*(float4(0.000000,1.000000,0.000000,-1.000000))).xyzw;
    // mad r1.zw, r3.xxxy, cb0[2].xxxy, r4.xxxy
    r1.zw = ((r3.xxxy)*(source[2].xxxy)+(r4.xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r1.zwzz, t0.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mul r1.zw, r3.xxxy, cb0[2].xxxy
    r1.zw = ((r3.xxxy)*(source[2].xxxy)).zw;
    // mad r3.zw, r1.zzzw, l(0.000000, 0.000000, 1.300000, 1.300000), r4.zzzw
    r3.zw = ((r1.zzzw)*(float4(0.000000,0.000000,1.300000,1.300000))+(r4.zzzw)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.zwzz, t1.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mad r4.xyz, r4.xyzx, l(0.875000, 0.875000, 0.875000, 0.000000), r5.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.875000,0.875000,0.875000,0.000000))+(r5.xyzx)).xyz;
    // mad r2.zw, r1.zzzw, l(0.000000, 0.000000, 2.300000, 2.300000), r2.zzzw
    r2.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.300000,2.300000))+(r2.zzzw)).zw;
    // mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 1.700000, 1.700000), r2.xxxy
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,1.700000,1.700000))+(r2.xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r2.zwzz, t1.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mul r5.xyz, r5.xyzx, l(0.669922, 0.669922, 0.669922, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.669922,0.669922,0.669922,0.000000))).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r1.zwzz, t0.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mad r5.xyz, r6.xyzx, l(0.765625, 0.765625, 0.765625, 0.000000), r5.xyzx
    r5.xyz = ((r6.xyzx)*(float4(0.765625,0.765625,0.765625,0.000000))+(r5.xyzx)).xyz;
    // add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[3].xyzx
    r4.xyz = ((r4.xyzx)*(source[3].xyzx)).xyz;
    // mul r5.xyz, r1.xxxx, r4.xyzx
    r5.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // mul r5.xyz, r5.xyzx, l(0.670000, 0.670000, 0.000000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.670000,0.670000,0.000000,0.000000))).xyz;
    // mad r4.xyz, r4.xyzx, l(0.330000, 0.330000, 1.000000, 0.000000), r5.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.330000,0.330000,1.000000,0.000000))+(r5.xyzx)).xyz;
    // mul r2.xy, r4.xyxx, v2.yyyy
    r2.xy = ((r4.xyxx)*(v2.yyyy)).xy;
    // add r4.xyz, r4.xyzx, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r4.xyz, v2.yyyy, r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((v2.yyyy)*(r4.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // mul r2.xy, r2.xyxx, l(0.025000, 0.025000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(0.025000,0.025000,0.000000,0.000000))).xy;
    // mad r3.zw, r1.zzzw, l(0.000000, 0.000000, 0.300000, 0.340000), r2.xxxy
    r3.zw = ((r1.zzzw)*(float4(0.000000,0.000000,0.300000,0.340000))+(r2.xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r3.zw, r2.zzzw, l(0.000000, 0.000000, 0.700000, 0.500000), r2.xxxy
    r3.zw = ((r2.zzzw)*(float4(0.000000,0.000000,0.700000,0.500000))+(r2.xxxy)).zw;
    // mad r2.xy, r3.xyxx, l(6.000000, 6.000000, 0.000000, 0.000000), r2.xyxx
    r2.xy = ((r3.xyxx)*(float4(6.000000,6.000000,0.000000,0.000000))+(r2.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // add r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)+(r7.xyzx)).xyz;
    // add r1.x, r5.y, r5.x
    r1.x = ((r5.yyyy)+(r5.xxxx)).x;
    // add r1.x, r5.z, r1.x
    r1.x = ((r5.zzzz)+(r1.xxxx)).x;
    // mul r1.x, r1.x, v2.x
    r1.x = ((r1.xxxx)*(v2.xxxx)).x;
    // add r0.zw, -r0.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r0.zzzw))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // mul r1.x, r1.x, r0.w
    r1.x = ((r1.xxxx)*(r0.wwww)).x;
    // mul r1.x, r1.x, l(0.333330)
    r1.x = ((r1.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // add r2.x, r6.y, r6.x
    r2.x = ((r6.yyyy)+(r6.xxxx)).x;
    // add r2.x, r6.z, r2.x
    r2.x = ((r6.zzzz)+(r2.xxxx)).x;
    // mul r2.y, r2.x, l(0.333330)
    r2.y = ((r2.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).y;
    // mad_sat r0.x, r2.x, l(0.083333), r0.x
    r0.x = (saturate((r2.xxxx)*(float4(0.083333,0.083333,0.083333,0.083333))+(r0.xxxx))).x;
    // mul r0.z, r0.z, l(2.500000)
    r0.z = ((r0.zzzz)*(float4(2.500000,2.500000,2.500000,2.500000))).z;
    // mad_sat r1.x, r2.y, r0.z, r1.x
    r1.x = (saturate((r2.yyyy)*(r0.zzzz)+(r1.xxxx))).x;
    // log r2.x, r0.x
    r2.x = (log2(r0.xxxx)).x;
    // lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // mul r2.x, r2.x, l(50.000000)
    r2.x = ((r2.xxxx)*(float4(50.000000,50.000000,50.000000,50.000000))).x;
    // exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // movc r0.x, r0.x, l(0), r2.x
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // mul r1.x, r1.x, r0.x
    r1.x = ((r1.xxxx)*(r0.xxxx)).x;
    // mad r0.x, r0.y, r0.x, r1.x
    r0.x = ((r0.yyyy)*(r0.xxxx)+(r1.xxxx)).x;
    // min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r0.x, r0.x, v2.w
    r0.x = ((r0.xxxx)*(v2.wwww)).x;
    // mul o0.w, r0.x, cb0[0].w
    output.targets[0].w = ((r0.xxxx)*(source[0].wwww)).w;
    // mul r0.xy, r4.xyxx, l(0.025000, 0.025000, 0.000000, 0.000000)
    r0.xy = ((r4.xyxx)*(float4(0.025000,0.025000,0.000000,0.000000))).xy;
    // mad r2.xy, r3.xyxx, l(6.000000, 6.000000, 0.000000, 0.000000), r0.xyxx
    r2.xy = ((r3.xyxx)*(float4(6.000000,6.000000,0.000000,0.000000))+(r0.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // add r1.x, r3.y, r3.x
    r1.x = ((r3.yyyy)+(r3.xxxx)).x;
    // add r1.x, r3.z, r1.x
    r1.x = ((r3.zzzz)+(r1.xxxx)).x;
    // mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // mad r1.xz, r1.zzwz, l(0.300000, 0.000000, 0.340000, 0.000000), r0.xxyx
    r1.xz = ((r1.zzwz)*(float4(0.300000,0.000000,0.340000,0.000000))+(r0.xxyx)).xz;
    // mad r0.xy, r2.zwzz, l(0.700000, 0.500000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r2.zwzz)*(float4(0.700000,0.500000,0.000000,0.000000))+(r0.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.xzw, r1.xzxx, t4.xwyz, s4, l(0.000000)
    r1.xzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // add r3.xyz, r2.xyzx, r1.xzwx
    r3.xyz = ((r2.xyzx)+(r1.xzwx)).xyz;
    // add r0.x, r3.y, r3.x
    r0.x = ((r3.yyyy)+(r3.xxxx)).x;
    // add r0.x, r3.z, r0.x
    r0.x = ((r3.zzzz)+(r0.xxxx)).x;
    // mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // mul r0.y, r0.x, v2.x
    r0.y = ((r0.xxxx)*(v2.xxxx)).y;
    // mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // mad r0.w, r1.y, r1.y, r0.w
    r0.w = ((r1.yyyy)*(r1.yyyy)+(r0.wwww)).w;
    // mad_sat r0.y, r0.z, l(0.333330), r0.y
    r0.y = (saturate((r0.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))+(r0.yyyy))).y;
    // mul r1.xy, r2.xyxx, r1.xzxx
    r1.xy = ((r2.xyxx)*(r1.xzxx)).xy;
    // add r0.z, r1.y, r1.x
    r0.z = ((r1.yyyy)+(r1.xxxx)).z;
    // mad r0.z, r1.w, r2.z, r0.z
    r0.z = ((r1.wwww)*(r2.zzzz)+(r0.zzzz)).z;
    // mul r0.z, r0.z, v2.x
    r0.z = ((r0.zzzz)*(v2.xxxx)).z;
    // mad_sat r0.z, r0.z, l(0.333330), l(-0.050000)
    r0.z = (saturate((r0.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))+(float4(-0.050000,-0.050000,-0.050000,-0.050000)))).z;
    // add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // mul r0.y, r0.y, l(1.330000)
    r0.y = ((r0.yyyy)*(float4(1.330000,1.330000,1.330000,1.330000))).y;
    // min r0.yw, r0.yyyw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = (min(r0.yyyw,float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // mul r1.xyz, cb0[6].xyzx, cb0[6].wwww
    r1.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // mul r1.xyz, r1.xyzx, l(0.660000, 0.660000, 0.660000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.660000,0.660000,0.660000,0.000000))).xyz;
    // mad r2.xyz, cb0[7].wwww, cb0[7].xyzx, -r1.xyzx
    r2.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r1.xyzx))).xyz;
    // mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // mul r2.xyz, cb0[8].xyzx, cb0[8].wwww
    r2.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // mad r0.xzw, r0.xxxx, r2.xxyz, -r1.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)+(-(r1.xxyz))).xzw;
    // mad r0.xyz, r0.yyyy, r0.xzwx, r1.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(r1.xyzx)).xyz;
    // mul r1.xyz, r4.xyzx, l(-1.000000, -1.000000, 1.000000, 0.000000)
    r1.xyz = ((r4.xyzx)*(float4(-1.000000,-1.000000,1.000000,0.000000))).xyz;
    // dp3_sat r0.w, cb0[4].xyzx, r1.xyzx
    r0.w = (saturate(dot((source[4].xyzx).xyz,(r1.xyzx).xyz).xxxx)).w;
    // mul r1.xyz, cb0[5].xyzx, cb0[5].wwww
    r1.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // mul r1.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r1.xyz, r0.wwww, r4.xyzx
    r1.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // mul r2.yzw, r2.yyyy, cb0[11].xxyz
    r2.yzw = ((r2.yyyy)*(source[11].xxyz)).yzw;
    // mad r2.xyz, r2.xxxx, cb0[10].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[10].xyzx)+(r2.yzwy)).xyz;
    // mul r2.xyz, r2.xyzx, cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(source[12].wwww)).xyz;
    // mad r3.xyz, r2.xyzx, r0.xyzx, cb0[1].xyzx
    r3.xyz = ((r2.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mad r2.xyz, r0.xyzx, cb0[12].xyzx, r3.xyzx
    r2.xyz = ((r0.xyzx)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mad o0.xyz, r2.xyzx, v4.wwww, v4.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v4.wwww)+(v4.xyzx)).xyz;
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


// Selected native Baked PS. Base/Direct programs above retain their original packing.

// f0d20984ab98d54cbc20069e78326eef
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater40Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint z=32u;z<64u;++z)source[z]=0.f;
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[11]=0.f; // Native baked diffuse already includes scene indirect light.
    source[12]=0.f; // Native baked diffuse already includes scene indirect light.
    source[13]=0.f; // Native baked diffuse already includes scene indirect light.
    source[14]=g_LightmapAverageScale;source[15]=g_LightmapDirectionalScale;
    source[7].y=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // mul r0.xy, cb0[2].zwzz, cb0[7].yyyy
    r0.xy = ((source[2].zwzz)*(source[7].yyyy)).xy;
    // mad r0.zw, cb0[7].xxxx, v4.xxxy, r0.xxxy
    r0.zw = ((source[7].xxxx)*(v4.xxxy)+(r0.xxxy)).zw;
    // mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s2, l(0.000000)
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
    // mul r2.xy, v4.xyxx, cb0[7].xxxx
    r2.xy = ((v4.xyxx)*(source[7].xxxx)).xy;
    // mad r0.xy, r0.xyxx, l(-0.300000, -0.300000, 0.000000, 0.000000), r2.xyxx
    r0.xy = ((r0.xyxx)*(float4(-0.300000,-0.300000,0.000000,0.000000))+(r2.xyxx)).xy;
    // mad r2.xy, cb0[7].yyyy, cb0[3].zwzz, r2.xyxx
    r2.xy = ((source[7].yyyy)*(source[3].zwzz)+(r2.xyxx)).xy;
    // mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // mul r0.xy, r0.xyxx, l(0.300000, 0.300000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.300000,0.300000,0.000000,0.000000))).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s2, l(0.000000)
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
    // mul r1.xyzw, r0.xyxy, cb0[7].zzww
    r1.xyzw = ((r0.xyxy)*(source[7].zzww)).xyzw;
    // mad r1.zw, r2.xxxy, cb0[3].xxxy, r1.zzzw
    r1.zw = ((r2.xxxy)*(source[3].xxxy)+(r1.zzzw)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s3, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mad r2.x, -v6.z, r0.w, l(1.000000)
    r2.x = ((-(v6.zzzz))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r2.yzw, r0.wwww, v6.xxyz
    r2.yzw = ((r0.wwww)*(v6.xxyz)).yzw;
    // log r0.w, |r2.x|
    r0.w = (log2(abs(r2.xxxx))).w;
    // lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // mul r0.w, r0.w, cb0[8].x
    r0.w = ((r0.wwww)*(source[8].xxxx)).w;
    // exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // mul_sat r0.w, r0.w, cb0[8].y
    r0.w = (saturate((r0.wwww)*(source[8].yyyy))).w;
    // movc r0.w, r2.x, l(0), r0.w
    r0.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // add r2.x, -r0.w, l(1.000000)
    r2.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r2.x, r2.x, cb0[8].z
    r2.x = ((r2.xxxx)*(source[8].zzzz)).x;
    // mad r0.xy, r2.xxxx, r1.zwzz, r1.xyxx
    r0.xy = ((r2.xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xzw, r0.xxyz, r1.xxxx
    r1.xzw = ((r0.xxyz)*(r1.xxxx)).xzw;
    // div r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)/(r1.yyyy)).xyz;
    // mul r3.xy, r0.xyxx, cb0[8].wwww
    r3.xy = ((r0.xyxx)*(source[8].wwww)).xy;
    // add r0.xyz, r0.xyzx, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r0.xyz = ((r0.xyzx)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r0.xyz, cb0[10].xxxx, r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r0.xyz = ((source[10].xxxx)*(r0.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // div r4.xyz, v8.xyzx, v8.wwww
    r4.xyz = ((v8.xyzx)/(v8.wwww)).xyz;
    // div r3.xy, r3.xyxx, r4.zzzz
    r3.xy = ((r3.xyxx)/(r4.zzzz)).xy;
    // mad r3.zw, r4.xxxy, cb2[0].xxxy, cb2[0].wwwz
    r3.zw = ((r4.xxxy)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // add r3.xy, r3.zwzz, r3.xyxx
    r3.xy = ((r3.zwzz)+(r3.xyxx)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r1.y, r3.zwzz, t2.yxzw, s0, l(0.000000)
    r1.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r3.zwzz).xy,0.f).rrrr).yxzw).y;
    // min r1.y, r1.y, l(0.999000)
    r1.y = (min(r1.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t5.xyzw, s1
    r3.xyz = ((g_SourceMapSceneColor.SampleLevel(SourceMapColorSampler,(r3.xyxx).xy,0.f)).xyzw).xyz;
    // mad r2.x, r1.y, cb2[1].z, -cb2[1].w
    r2.x = ((r1.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).x;
    // mad r1.y, r1.y, cb2[1].x, cb2[1].y
    r1.y = ((r1.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // div r2.x, l(1.000000, 1.000000, 1.000000, 1.000000), r2.x
    r2.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.xxxx)).x;
    // add r1.y, r1.y, r2.x
    r1.y = ((r1.yyyy)+(r2.xxxx)).y;
    // add r1.y, r1.y, -v8.w
    r1.y = ((r1.yyyy)+(-(v8.wwww))).y;
    // add r2.x, -cb0[9].x, l(1.000000)
    r2.x = ((-(source[9].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // max r2.x, r2.x, l(0.001000)
    r2.x = (max(r2.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // div_sat r1.y, r1.y, r2.x
    r1.y = (saturate((r1.yyyy)/(r2.xxxx))).y;
    // lt r2.x, r1.y, l(0.000001)
    r2.x = (asfloat((uint4)((r1.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // mul r1.y, r1.y, cb0[9].y
    r1.y = ((r1.yyyy)*(source[9].yyyy)).y;
    // exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // movc r1.y, r2.x, l(0), r1.y
    r1.y = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // mul_sat r2.x, r1.y, cb0[9].z
    r2.x = (saturate((r1.yyyy)*(source[9].zzzz))).x;
    // mul o0.w, r1.y, cb0[0].x
    output.targets[0].w = ((r1.yyyy)*(source[0].xxxx)).w;
    // add r1.y, -r2.x, l(1.000000)
    r1.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mad r3.xyz, r1.yyyy, r3.xyzx, cb0[1].xyzx
    r3.xyz = ((r1.yyyy)*(r3.xyzx)+(source[1].xyzx)).xyz;
    // dp3 r0.z, r0.xyzx, r2.yzwy
    r0.z = (dot((r0.xyzx).xyz,(r2.yzwy).xyz).xxxx).z;
    // mul r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)*(r0.zzzz)).xy;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r2.yzyy
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r2.yzyy))).xy;
    // mul r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)*(source[6].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r0.xyz, r0.xyzx, cb0[5].xyzx
    r0.xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // dp3 r1.y, r1.xzwx, r2.yzwy
    r1.y = (dot((r1.xzwx).xyz,(r2.yzwy).xyz).xxxx).y;
    // mul r4.xyz, r1.yyyy, r1.xzwx
    r4.xyz = ((r1.yyyy)*(r1.xzwx)).xyz;
    // mad r2.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.yzwy
    r2.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.yzwy))).xyz;
    // add r4.xy, r2.xyxx, -v4.xyxx
    r4.xy = ((r2.xyxx)+(-(v4.xyxx))).xy;
    // mad r4.xy, r4.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000), v4.xyxx
    r4.xy = ((r4.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v4.xyxx)).xy;
    // mul r4.xy, r4.xyxx, cb0[9].wwww
    r4.xy = ((r4.xyxx)*(source[9].wwww)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t3.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r4.xyz, r4.xyzx, cb0[4].xyzx
    r4.xyz = ((r4.xyzx)*(source[4].xyzx)).xyz;
    // mad r0.xyz, cb0[10].yyyy, r0.xyzx, -r4.xyzx
    r0.xyz = ((source[10].yyyy)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r4.xyz, r0.wwww, v7.xyzx
    r4.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, r1.xzwx
    r0.w = (dot((r4.xyzx).xyz,(r1.xzwx).xyz).xxxx).w;
    // mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // mul r4.yzw, r4.yyyy, cb0[12].xxyz
    r4.yzw = ((r4.yyyy)*(source[12].xxyz)).yzw;
    // mad r4.xyz, r4.xxxx, cb0[11].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[11].xyzx)+(r4.yzwy)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[13].wwww
    r4.xyz = ((r4.xyzx)*(source[13].wwww)).xyz;
    // mul r5.xyz, r0.xyzx, r4.xyzx
    r5.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // dp2_sat r6.x, r1.zwzz, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r1.zwzz).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r6.y, r1.xzwx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r1.xzwx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r6.z, r1.xzwx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r1.xzwx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t7.xyzw, s6
    r7.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r7.xyz, r7.xyzx, cb0[15].xyzx
    r7.xyz = ((r7.xyzx)*(source[15].xyzx)).xyz;
    // dp3 r0.w, r7.xyzx, r6.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t6.xyzw, s6
    r6.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[14].xyzx
    r6.xyz = ((r6.xyzx)*(source[14].xyzx)).xyz;
    // mul r8.xyz, r0.wwww, r6.xyzx
    r8.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // mad r4.xyz, r6.xyzx, r0.wwww, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r4.xyz, r8.xyzx, r4.xyzx
    r4.xyz = ((r8.xyzx)/(r4.xyzx)).xyz;
    // mad r5.xyz, r0.xyzx, r8.xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(r8.xyzx)+(r5.xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // dp2_sat r4.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r4.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r4.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // log r2.xyz, r4.xyzx
    r2.xyz = (log2(r4.xyzx)).xyz;
    // add r1.y, cb0[10].w, l(1.000000)
    r1.y = ((source[10].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mul r2.xyz, r2.xyzx, r1.yyyy
    r2.xyz = ((r2.xyzx)*(r1.yyyy)).xyz;
    // exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // dp3 r1.y, r7.xyzx, r2.xyzx
    r1.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // mad r2.xyz, cb0[10].zzzz, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((source[10].zzzz)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r2.xyz, r6.xyzx, r2.xyzx
    r2.xyz = ((r6.xyzx)*(r2.xyzx)).xyz;
    // mad r4.xyz, r2.xyzx, r1.yyyy, r5.xyzx
    r4.xyz = ((r2.xyzx)*(r1.yyyy)+(r5.xyzx)).xyz;
    // mul r2.xyz, r1.yyyy, r2.xyzx
    r2.xyz = ((r1.yyyy)*(r2.xyzx)).xyz;
    // dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r2.xyz, r3.xyzx, r4.xyzx
    r2.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // mad r2.xyz, r0.xyzx, cb0[13].xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(source[13].xyzx)+(r2.xyzx)).xyz;
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
    // dp3 r1.y, v0.xyzx, v0.xyzx
    r1.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r2.xyz, r1.yyyy, v0.xyzx
    r2.xyz = ((r1.yyyy)*(v0.xyzx)).xyz;
    // mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r1.xzwx
    r0.z = (dot((r0.xyzx).xyz,(r1.xzwx).xyz).xxxx).z;
    // dp3 r0.x, r2.xyzx, r1.xzwx
    r0.x = (dot((r2.xyzx).xyz,(r1.xzwx).xyz).xxxx).x;
    // mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r2.xyzx, r1.xzwx
    r0.y = (dot((r2.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
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

// 38679c063b6cc841a5b31d779fff66a8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater41Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint z=32u;z<64u;++z)source[z]=0.f;
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[11]=0.f; // Native baked diffuse already includes scene indirect light.
    source[12]=0.f; // Native baked diffuse already includes scene indirect light.
    source[13]=0.f; // Native baked diffuse already includes scene indirect light.
    source[14]=g_LightmapAverageScale;source[15]=g_LightmapDirectionalScale;
    source[7].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f;
    // add r0.xy, v8.xyxx, cb0[0].xyxx
    r0.xy = ((v8.xyxx)+(source[0].xyxx)).xy;
    // mul r0.zw, cb0[2].zzzw, cb0[7].xxxx
    r0.zw = ((source[2].zzzw)*(source[7].xxxx)).zw;
    // mad r1.xy, r0.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000), r0.zwzz
    r1.xy = ((r0.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))+(r0.zwzz)).xy;
    // mul r0.xy, r0.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))).xy;
    // mad r0.zw, r0.zzzw, l(0.000000, 0.000000, -0.300000, -0.300000), r0.xxxy
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,-0.300000,-0.300000))+(r0.xxxy)).zw;
    // mad r0.xy, cb0[7].xxxx, cb0[3].zwzz, r0.xyxx
    r0.xy = ((source[7].xxxx)*(source[3].zwzz)+(r0.xyxx)).xy;
    // mul r0.zw, r0.zzzw, cb0[2].xxxy
    r0.zw = ((r0.zzzw)*(source[2].xxxy)).zw;
    // mul r0.zw, r0.zzzw, l(0.000000, 0.000000, 0.300000, 0.300000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,0.300000,0.300000))).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s2, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // mad r2.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // mul r0.zw, r1.xxxy, cb0[2].xxxy
    r0.zw = ((r1.xxxy)*(source[2].xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t0.zwxy, s2, l(0.000000)
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
    // add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // dp2 r0.z, r2.xyxx, r2.xyxx
    r0.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // add r2.z, r0.z, l(0.000010)
    r2.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // mul r2.xyzw, r1.xyxy, cb0[7].yyzz
    r2.xyzw = ((r1.xyxy)*(source[7].yyzz)).xyzw;
    // mad r0.xy, r0.xyxx, cb0[3].xyxx, r2.zwzz
    r0.xy = ((r0.xyxx)*(source[3].xyxx)+(r2.zwzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mad r0.w, -v6.z, r0.z, l(1.000000)
    r0.w = ((-(v6.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r3.xyz, r0.zzzz, v6.xyzx
    r3.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // log r0.z, |r0.w|
    r0.z = (log2(abs(r0.wwww))).z;
    // lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // mul r0.z, r0.z, cb0[7].w
    r0.z = ((r0.zzzz)*(source[7].wwww)).z;
    // exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // mul_sat r0.z, r0.z, cb0[8].x
    r0.z = (saturate((r0.zzzz)*(source[8].xxxx))).z;
    // movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // add r0.w, -r0.z, l(1.000000)
    r0.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // mad r1.xy, r0.wwww, r0.xyxx, r2.xyxx
    r1.xy = ((r0.wwww)*(r0.xyxx)+(r2.xyxx)).xy;
    // dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // sqrt r0.y, r0.x
    r0.y = (sqrt(r0.xxxx)).y;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r2.xyz, r0.xxxx, r1.xyzx
    r2.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // div r0.xyw, r1.xyxz, r0.yyyy
    r0.xyw = ((r1.xyxz)/(r0.yyyy)).xyw;
    // mul r1.xy, r0.xyxx, cb0[8].zzzz
    r1.xy = ((r0.xyxx)*(source[8].zzzz)).xy;
    // add r0.xyw, r0.xyxw, l(-0.000000, -0.000000, 0.000000, -1.000000)
    r0.xyw = ((r0.xyxw)+(float4(-0.000000,-0.000000,0.000000,-1.000000))).xyw;
    // mad r0.xyw, cb0[9].wwww, r0.xyxw, l(0.000000, 0.000000, 0.000000, 1.000000)
    r0.xyw = ((source[9].wwww)*(r0.xyxw)+(float4(0.000000,0.000000,0.000000,1.000000))).xyw;
    // mul r4.xyzw, v8.yyyy, cb1[1].xyzw
    r4.xyzw = ((v8.yyyy)*(projection[1].xyzw)).xyzw;
    // mad r4.xyzw, cb1[0].xyzw, v8.xxxx, r4.xyzw
    r4.xyzw = ((projection[0].xyzw)*(v8.xxxx)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb1[2].xyzw, v8.zzzz, r4.xyzw
    r4.xyzw = ((projection[2].xyzw)*(v8.zzzz)+(r4.xyzw)).xyzw;
    // mad r4.xyzw, cb1[3].xyzw, v8.wwww, r4.xyzw
    r4.xyzw = ((projection[3].xyzw)*(v8.wwww)+(r4.xyzw)).xyzw;
    // div r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)/(r4.wwww)).xyz;
    // div r1.xy, r1.xyxx, r4.zzzz
    r1.xy = ((r1.xyxx)/(r4.zzzz)).xy;
    // mad r1.zw, r4.xxxy, cb2[0].xxxy, cb2[0].wwwz
    r1.zw = ((r4.xxxy)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // add r1.xy, r1.zwzz, r1.xyxx
    r1.xy = ((r1.zwzz)+(r1.xyxx)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r1.z, r1.zwzz, t2.yzxw, s0, l(0.000000)
    r1.z = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r1.zwzz).xy,0.f).rrrr).yzxw).z;
    // min r1.z, r1.z, l(0.999000)
    r1.z = (min(r1.zzzz,float4(0.999000,0.999000,0.999000,0.999000))).z;
    // sample_indexable(texture2d)(float,float,float,float) r1.xyw, r1.xyxx, t5.xywz, s1
    r1.xyw = ((g_SourceMapSceneColor.SampleLevel(SourceMapColorSampler,(r1.xyxx).xy,0.f)).xywz).xyw;
    // mad r2.w, r1.z, cb2[1].z, -cb2[1].w
    r2.w = ((r1.zzzz)*(passValues[1].zzzz)+(-(passValues[1].wwww))).w;
    // mad r1.z, r1.z, cb2[1].x, cb2[1].y
    r1.z = ((r1.zzzz)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // add r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)+(r2.wwww)).z;
    // add r1.z, -r4.w, r1.z
    r1.z = ((-(r4.wwww))+(r1.zzzz)).z;
    // add r2.w, -cb0[8].w, l(1.000000)
    r2.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r2.w, r2.w, l(0.001000)
    r2.w = (max(r2.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // div_sat r1.z, r1.z, r2.w
    r1.z = (saturate((r1.zzzz)/(r2.wwww))).z;
    // lt r2.w, r1.z, l(0.000001)
    r2.w = (asfloat((uint4)((r1.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // mul r1.z, r1.z, cb0[9].x
    r1.z = ((r1.zzzz)*(source[9].xxxx)).z;
    // exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // movc r1.z, r2.w, l(0), r1.z
    r1.z = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // mul_sat r2.w, r1.z, cb0[9].y
    r2.w = (saturate((r1.zzzz)*(source[9].yyyy))).w;
    // mul o0.w, r1.z, cb0[0].w
    output.targets[0].w = ((r1.zzzz)*(source[0].wwww)).w;
    // add r1.z, -r2.w, l(1.000000)
    r1.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mad r1.xyz, r1.zzzz, r1.xywx, cb0[1].xyzx
    r1.xyz = ((r1.zzzz)*(r1.xywx)+(source[1].xyzx)).xyz;
    // dp3 r0.w, r0.xywx, r3.xyzx
    r0.w = (dot((r0.xywx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)*(r0.wwww)).xy;
    // mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r3.xyxx
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r3.xyxx))).xy;
    // mul r0.xy, r0.xyxx, cb0[6].xyxx
    r0.xy = ((r0.xyxx)*(source[6].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t4.xywz, s5, l(0.000000)
    r0.xyw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyw;
    // mul r0.xyw, r0.xyxw, cb0[5].xyxz
    r0.xyw = ((r0.xyxw)*(source[5].xyxz)).xyw;
    // dp3 r1.w, r2.xyzx, r3.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mul r4.xyz, r1.wwww, r2.xyzx
    r4.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // mad r3.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // add r4.xy, r3.xyxx, -v4.xyxx
    r4.xy = ((r3.xyxx)+(-(v4.xyxx))).xy;
    // mad r4.xy, r4.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000), v4.xyxx
    r4.xy = ((r4.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v4.xyxx)).xy;
    // mul r4.xy, r4.xyxx, cb0[9].zzzz
    r4.xy = ((r4.xyxx)*(source[9].zzzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t3.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r4.xyz, r4.xyzx, cb0[4].xyzx
    r4.xyz = ((r4.xyzx)*(source[4].xyzx)).xyz;
    // mad r0.xyw, cb0[10].xxxx, r0.xyxw, -r4.xyxz
    r0.xyw = ((source[10].xxxx)*(r0.xyxw)+(-(r4.xyxz))).xyw;
    // mad r0.xyz, r0.zzzz, r0.xywx, r4.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r4.xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
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
    // mul r4.yzw, r4.yyyy, cb0[12].xxyz
    r4.yzw = ((r4.yyyy)*(source[12].xxyz)).yzw;
    // mad r4.xyz, r4.xxxx, cb0[11].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[11].xyzx)+(r4.yzwy)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[13].wwww
    r4.xyz = ((r4.xyzx)*(source[13].wwww)).xyz;
    // mul r5.xyz, r0.xyzx, r4.xyzx
    r5.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // dp2_sat r6.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r6.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r6.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t7.xyzw, s6
    r7.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r7.xyz, r7.xyzx, cb0[15].xyzx
    r7.xyz = ((r7.xyzx)*(source[15].xyzx)).xyz;
    // dp3 r0.w, r7.xyzx, r6.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t6.xyzw, s6
    r6.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[14].xyzx
    r6.xyz = ((r6.xyzx)*(source[14].xyzx)).xyz;
    // mul r8.xyz, r0.wwww, r6.xyzx
    r8.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // mad r4.xyz, r6.xyzx, r0.wwww, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r4.xyz, r8.xyzx, r4.xyzx
    r4.xyz = ((r8.xyzx)/(r4.xyzx)).xyz;
    // mad r5.xyz, r0.xyzx, r8.xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(r8.xyzx)+(r5.xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // dp2_sat r4.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r4.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r4.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // log r3.xyz, r4.xyzx
    r3.xyz = (log2(r4.xyzx)).xyz;
    // add r1.w, cb0[10].z, l(1.000000)
    r1.w = ((source[10].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)*(r1.wwww)).xyz;
    // exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // dp3 r1.w, r7.xyzx, r3.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mad r3.xyz, cb0[10].yyyy, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((source[10].yyyy)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r3.xyz, r6.xyzx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // mad r4.xyz, r3.xyzx, r1.wwww, r5.xyzx
    r4.xyz = ((r3.xyzx)*(r1.wwww)+(r5.xyzx)).xyz;
    // mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)+(r4.xyzx)).xyz;
    // mad r1.xyz, r0.xyzx, cb0[13].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[13].xyzx)+(r1.xyzx)).xyz;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
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

// 7ef8b0897718f64daa8ebb92a01e92ce
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater42Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint z=32u;z<64u;++z)source[z]=0.f;
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[17]=0.f; // Native baked diffuse already includes scene indirect light.
    source[18]=0.f; // Native baked diffuse already includes scene indirect light.
    source[19]=0.f; // Native baked diffuse already includes scene indirect light.
    source[20]=0.f; // Native baked diffuse already includes scene indirect light.
    source[21]=g_LightmapAverageScale;source[22]=g_LightmapDirectionalScale;
    source[11].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f,r8=0.f,r9=0.f;
    // mul r0.xy, cb0[2].zwzz, cb0[11].xxxx
    r0.xy = ((source[2].zwzz)*(source[11].xxxx)).xy;
    // add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
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
    // mul r2.xyzw, r0.xyxy, cb0[11].yyzz
    r2.xyzw = ((r0.xyxy)*(source[11].yyzz)).xyzw;
    // mul r3.zw, cb0[3].zzzw, cb0[11].xxxx
    r3.zw = ((source[3].zzzw)*(source[11].xxxx)).zw;
    // mad r4.xy, r1.xyxx, l(0.010000, 0.010000, 0.000000, 0.000000), r3.zwzz
    r4.xy = ((r1.xyxx)*(float4(0.010000,0.010000,0.000000,0.000000))+(r3.zwzz)).xy;
    // mad r3.zw, r3.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000), r3.xxxy
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,-0.500000,-0.500000))+(r3.xxxy)).zw;
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
    // mad r2.xy, cb0[11].wwww, r2.zwzz, r2.xyxx
    r2.xy = ((source[11].wwww)*(r2.zwzz)+(r2.xyxx)).xy;
    // mov r2.z, r0.z
    r2.z = (r0.zzzz).z;
    // mad r0.xy, cb0[14].yyyy, r0.xyxx, v4.xyxx
    r0.xy = ((source[14].yyyy)*(r0.xyxx)+(v4.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t3.xyzw, s7, l(0.000000)
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
    // mul r1.x, r1.x, cb0[12].x
    r1.x = ((r1.xxxx)*(source[12].xxxx)).x;
    // exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // mad r0.yzw, r1.xxxx, r0.yyzw, r2.xxyz
    r0.yzw = ((r1.xxxx)*(r0.yyzw)+(r2.xxyz)).yzw;
    // mul_sat r1.x, r1.x, cb0[14].x
    r1.x = (saturate((r1.xxxx)*(source[14].xxxx))).x;
    // dp3 r1.y, r0.yzwy, r0.yzwy
    r1.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // sqrt r1.z, r1.y
    r1.z = (sqrt(r1.yyyy)).z;
    // rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // mul r2.xyz, r0.yzwy, r1.yyyy
    r2.xyz = ((r0.yzwy)*(r1.yyyy)).xyz;
    // div r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)/(r1.zzzz)).yzw;
    // mul r1.yz, r0.yyzy, cb0[12].yyyy
    r1.yz = ((r0.yyzy)*(source[12].yyyy)).yz;
    // mad r3.zw, cb0[11].xxxx, cb0[10].zzzw, r3.xxxy
    r3.zw = ((source[11].xxxx)*(source[10].zzzw)+(r3.xxxy)).zw;
    // mad r3.xy, cb0[11].xxxx, cb0[5].zwzz, r3.xyxx
    r3.xy = ((source[11].xxxx)*(source[5].zwzz)+(r3.xyxx)).xy;
    // mad r3.xy, r3.xyxx, cb0[5].xyxx, r1.yzyy
    r3.xy = ((r3.xyxx)*(source[5].xyxx)+(r1.yzyy)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t7.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r4.xyz, r4.xyzx, cb0[4].xyzx
    r4.xyz = ((r4.xyzx)*(source[4].xyzx)).xyz;
    // mul r3.xy, r3.zwzz, cb0[10].xyxx
    r3.xy = ((r3.zwzz)*(source[10].xyxx)).xy;
    // mad r3.zw, r3.zzzw, cb0[10].xxxy, r1.yyyz
    r3.zw = ((r3.zzzw)*(source[10].xxxy)+(r1.yyyz)).zw;
    // mad r1.yz, r3.xxyx, l(0.000000, 2.000000, 2.000000, 0.000000), r1.yyzy
    r1.yz = ((r3.xxyx)*(float4(0.000000,2.000000,2.000000,0.000000))+(r1.yyzy)).yz;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t6.wxyz, s6, l(0.000000)
    r1.yzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.zwzz, t6.xyzw, s6, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r1.yzw, -r3.xxyz, l(0.000000, 2.000000, 2.000000, 2.000000), r1.yyzw
    r1.yzw = ((-(r3.xxyz))*(float4(0.000000,2.000000,2.000000,2.000000))+(r1.yyzw)).yzw;
    // add r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)+(r3.xyzx)).xyz;
    // mad r1.yzw, r0.xxxx, r1.yyzw, r3.xxyz
    r1.yzw = ((r0.xxxx)*(r1.yyzw)+(r3.xxyz)).yzw;
    // dp3 r2.w, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // add r3.xyz, -r1.yzwy, r2.wwww
    r3.xyz = ((-(r1.yzwy))+(r2.wwww)).xyz;
    // mad r1.yzw, cb0[14].wwww, r3.xxyz, r1.yyzw
    r1.yzw = ((source[14].wwww)*(r3.xxyz)+(r1.yyzw)).yzw;
    // mul r1.yzw, r1.yyzw, cb0[9].xxyz
    r1.yzw = ((r1.yyzw)*(source[9].xxyz)).yzw;
    // mul r1.yzw, r0.xxxx, r1.yyzw
    r1.yzw = ((r0.xxxx)*(r1.yyzw)).yzw;
    // add r3.xyz, r0.yzwy, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r3.xyz = ((r0.yzwy)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r3.xyz, cb0[13].xxxx, r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r3.xyz = ((source[13].xxxx)*(r3.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // mul r5.xyz, r2.wwww, v6.xyzx
    r5.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // dp3 r2.w, r3.xyzx, r5.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // mul r3.xy, r3.xyxx, r2.wwww
    r3.xy = ((r3.xyxx)*(r2.wwww)).xy;
    // mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r5.xyxx
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r5.xyxx))).xy;
    // mul r3.xy, r3.xyxx, cb0[7].xyxx
    r3.xy = ((r3.xyxx)*(source[7].xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r3.xyz, r3.xyzx, cb0[6].xyzx
    r3.xyz = ((r3.xyzx)*(source[6].xyzx)).xyz;
    // dp3 r0.y, r0.yzwy, r5.xyzx
    r0.y = (dot((r0.yzwy).xyz,(r5.xyzx).xyz).xxxx).y;
    // add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mad r0.z, -r0.y, l(0.500000), l(1.000000)
    r0.z = ((-(r0.yyyy))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // mul r0.w, r0.w, cb0[13].y
    r0.w = ((r0.wwww)*(source[13].yyyy)).w;
    // exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // mul r0.w, r0.w, cb0[13].z
    r0.w = ((r0.wwww)*(source[13].zzzz)).w;
    // movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // dp3 r0.w, r2.xyzx, r5.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // mul r6.xyz, r0.wwww, r2.xyzx
    r6.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // mad r5.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xyzx
    r5.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xyzx))).xyz;
    // add r6.xy, r5.xyxx, -v4.xyxx
    r6.xy = ((r5.xyxx)+(-(v4.xyxx))).xy;
    // mad r6.xy, r6.xyxx, l(0.050000, 0.050000, 0.000000, 0.000000), v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.050000,0.050000,0.000000,0.000000))+(v4.xyxx)).xy;
    // mul r6.xy, r6.xyxx, cb0[13].wwww
    r6.xy = ((r6.xyxx)*(source[13].wwww)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[8].xyzx
    r6.xyz = ((r6.xyzx)*(source[8].xyzx)).xyz;
    // mul r6.xyz, r1.xxxx, r6.xyzx
    r6.xyz = ((r1.xxxx)*(r6.xyzx)).xyz;
    // mad r3.xyz, r0.zzzz, r3.xyzx, r6.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)+(r6.xyzx)).xyz;
    // add r6.xyz, -r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r3.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mad r1.xyz, r1.yzwy, r6.xyzx, r3.xyzx
    r1.xyz = ((r1.yzwy)*(r6.xyzx)+(r3.xyzx)).xyz;
    // add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // mul r3.xyz, r0.zzzz, v7.xyzx
    r3.xyz = ((r0.zzzz)*(v7.xyzx)).xyz;
    // dp3 r0.z, r3.xyzx, r2.xyzx
    r0.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // mul r3.xyz, r0.wwww, cb0[19].xyzx
    r3.xyz = ((r0.wwww)*(source[19].xyzx)).xyz;
    // mad r3.xyz, r0.zzzz, cb0[18].xyzx, r3.xyzx
    r3.xyz = ((r0.zzzz)*(source[18].xyzx)+(r3.xyzx)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[20].wwww
    r3.xyz = ((r3.xyzx)*(source[20].wwww)).xyz;
    // mul r6.xyz, r1.xyzx, r3.xyzx
    r6.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // dp2_sat r7.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r7.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r7.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t9.xyzw, s8
    r8.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r8.xyz, r8.xyzx, cb0[22].xyzx
    r8.xyz = ((r8.xyzx)*(source[22].xyzx)).xyz;
    // dp3 r0.z, r8.xyzx, r7.xyzx
    r0.z = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t8.xyzw, s8
    r7.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r7.xyz, r7.xyzx, cb0[21].xyzx
    r7.xyz = ((r7.xyzx)*(source[21].xyzx)).xyz;
    // mul r9.xyz, r0.zzzz, r7.xyzx
    r9.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // mad r3.xyz, r7.xyzx, r0.zzzz, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r0.zzzz)+(r3.xyzx)).xyz;
    // add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r3.xyz, r9.xyzx, r3.xyzx
    r3.xyz = ((r9.xyzx)/(r3.xyzx)).xyz;
    // mad r6.xyz, r1.xyzx, r9.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // dp3 r0.z, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // dp2_sat r3.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r3.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r3.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // add r0.w, cb0[15].y, l(1.000000)
    r0.w = ((source[15].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mul r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // dp3 r0.w, r8.xyzx, r3.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // mad r3.xyz, cb0[15].xxxx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((source[15].xxxx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // mul r5.xyz, r7.xyzx, r3.xyzx
    r5.xyz = ((r7.xyzx)*(r3.xyzx)).xyz;
    // mul_sat r3.xyz, r3.xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r3.xyz = (saturate((r3.xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // sqrt o5.xyz, r3.xyzx
    output.targets[5].xyz = (sqrt(r3.xyzx)).xyz;
    // mad r3.xyz, r5.xyzx, r0.wwww, r6.xyzx
    r3.xyz = ((r5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // mul r0.w, r0.w, cb0[12].z
    r0.w = ((r0.wwww)*(source[12].zzzz)).w;
    // exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // mul r0.w, r0.w, cb0[12].w
    r0.w = ((r0.wwww)*(source[12].wwww)).w;
    // movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // mad r4.xyz, r0.yyyy, r4.xyzx, cb0[1].xyzx
    r4.xyz = ((r0.yyyy)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // add r4.xyz, r3.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // mad r4.xyz, r1.xyzx, cb0[20].xyzx, r4.xyzx
    r4.xyz = ((r1.xyzx)*(source[20].xyzx)+(r4.xyzx)).xyz;
    // mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // mad o0.xyz, r4.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // mul r1.xyz, v8.yyyy, cb1[1].xywx
    r1.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // mad r1.xyz, cb1[0].xywx, v8.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v8.xxxx)+(r1.xyzx)).xyz;
    // mad r1.xyz, cb1[2].xywx, v8.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v8.zzzz)+(r1.xyzx)).xyz;
    // mad r1.xyz, cb1[3].xywx, v8.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v8.wwww)+(r1.xyzx)).xyz;
    // div r0.yw, r1.xxxy, r1.zzzz
    r0.yw = ((r1.xxxy)/(r1.zzzz)).yw;
    // mad r0.yw, r0.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r0.yw = ((r0.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.ywyy, t2.yxzw, s0, l(0.000000)
    r0.y = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.ywyy).xy,0.f).rrrr).yxzw).y;
    // min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // mad r0.w, r0.y, cb2[1].z, -cb2[1].w
    r0.w = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).w;
    // mad r0.y, r0.y, cb2[1].x, cb2[1].y
    r0.y = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // add r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)+(r0.yyyy)).y;
    // add r0.y, -r1.z, r0.y
    r0.y = ((-(r1.zzzz))+(r0.yyyy)).y;
    // add r0.w, -cb0[15].z, l(1.000000)
    r0.w = ((-(source[15].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // max r0.w, r0.w, l(0.001000)
    r0.w = (max(r0.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // div_sat r0.y, r0.y, r0.w
    r0.y = (saturate((r0.yyyy)/(r0.wwww))).y;
    // mul r0.y, r0.y, cb0[15].w
    r0.y = ((r0.yyyy)*(source[15].wwww)).y;
    // log r0.w, |r0.y|
    r0.w = (log2(abs(r0.yyyy))).w;
    // lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // mul r0.w, r0.w, cb0[16].x
    r0.w = ((r0.wwww)*(source[16].xxxx)).w;
    // exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // add r0.w, -r0.y, l(1.000000)
    r0.w = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // mad r0.x, r0.x, r0.w, r0.y
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.yyyy)).x;
    // mul o0.w, r0.x, cb0[0].w
    output.targets[0].w = ((r0.xxxx)*(source[0].wwww)).w;
    // dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyw, r0.xxxx, v1.xyxz
    r0.xyw = ((r0.xxxx)*(v1.xyxz)).xyw;
    // dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r4.xyz, r0.wxyw, r1.yzxy
    r4.xyz = ((r0.wxyw)*(r1.yzxy)).xyz;
    // mad r4.xyz, r0.ywxy, r1.zxyz, -r4.xyzx
    r4.xyz = ((r0.ywxy)*(r1.zxyz)+(-(r4.xyzx))).xyz;
    // dp3 r5.z, r0.xywx, r2.xyzx
    r5.z = (dot((r0.xywx).xyz,(r2.xyzx).xyz).xxxx).z;
    // dp3 r5.x, r1.xyzx, r2.xyzx
    r5.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // mul r0.xyw, r4.xyxz, v1.wwww
    r0.xyw = ((r4.xyxz)*(v1.wwww)).xyw;
    // dp3 r5.y, r0.xywx, r2.xyzx
    r5.y = (dot((r0.xywx).xyz,(r2.xyzx).xyz).xxxx).y;
    // dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // mul r0.xyw, r0.xxxx, r5.xyxz
    r0.xyw = ((r0.xxxx)*(r5.xyxz)).xyw;
    // ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xywx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xywx)).xyz).xxxx).w;
    // div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
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
    // mul_sat o3.w, cb0[15].y, l(0.002000)
    output.targets[3].w = (saturate((source[15].yyyy)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // mul o4.z, r0.z, r3.x
    output.targets[4].z = ((r0.zzzz)*(r3.xxxx)).z;
    // dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // ftou r0.x, cb0[17].z
    r0.x = (asfloat((uint4)(source[17].zzzz))).x;
    // bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // ret
    return output;
}

// 725198d14d9e5d479d4f5700efcb4881
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapWater43Baked(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<32u;++i)source[i]=g_SourceCharacterBaseConstants[i+32u]; [unroll] for(uint z=32u;z<64u;++z)source[z]=0.f;
    source[0]=float4(0.f,0.f,0.f,1.f);
    source[10]=0.f; // Native baked diffuse already includes scene indirect light.
    source[11]=0.f; // Native baked diffuse already includes scene indirect light.
    source[12]=0.f; // Native baked diffuse already includes scene indirect light.
    source[13]=g_LightmapAverageScale;source[14]=g_LightmapDirectionalScale;
    source[9].x=(float4(g_SourceCharacterTime,0.f,0.f,0.f)).x;
    float4 projection[4];[unroll] for(uint p=0u;p<4u;++p)projection[p]=input.projection[p];
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(0.f,0.f,0.f,1.f)};
    float4 v0=input.values[0],v1=input.values[1],v2=input.values[2],v3=input.values[3],v4=input.values[4],v5=input.values[5],v6=input.values[6],v7=input.values[7],v8=input.values[8],v9=input.values[9];
    float4 r0=0.f,r1=0.f,r2=0.f,r3=0.f,r4=0.f,r5=0.f,r6=0.f,r7=0.f;
    // mul r0.xyz, v7.yyyy, cb1[1].xywx
    r0.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // mad r0.xyz, cb1[0].xywx, v7.xxxx, r0.xyzx
    r0.xyz = ((projection[0].xywx)*(v7.xxxx)+(r0.xyzx)).xyz;
    // mad r0.xyz, cb1[2].xywx, v7.zzzz, r0.xyzx
    r0.xyz = ((projection[2].xywx)*(v7.zzzz)+(r0.xyzx)).xyz;
    // mad r0.xyz, cb1[3].xywx, v7.wwww, r0.xyzx
    r0.xyz = ((projection[3].xywx)*(v7.wwww)+(r0.xyzx)).xyz;
    // div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s0, l(0.000000)
    r0.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f).rrrr).xyzw).x;
    // min r0.x, r0.x, l(0.999000)
    r0.x = (min(r0.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // mad r0.y, r0.x, cb2[1].z, -cb2[1].w
    r0.y = ((r0.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // mad r0.x, r0.x, cb2[1].x, cb2[1].y
    r0.x = ((r0.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).x;
    // div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // add r0.x, -r0.z, r0.x
    r0.x = ((-(r0.zzzz))+(r0.xxxx)).x;
    // add r0.y, -cb0[9].y, l(1.000000)
    r0.y = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // div_sat r0.y, r0.x, r0.y
    r0.y = (saturate((r0.xxxx)/(r0.yyyy))).y;
    // mul_sat r0.xzw, r0.xxxx, l(0.034483, 0.000000, 0.020408, 0.005025)
    r0.xzw = (saturate((r0.xxxx)*(float4(0.034483,0.000000,0.020408,0.005025)))).xzw;
    // dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // mad r1.y, -v5.z, r1.x, l(1.000000)
    r1.y = ((-(v5.zzzz))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // mul r1.xzw, r1.xxxx, v5.xxyz
    r1.xzw = ((r1.xxxx)*(v5.xxyz)).xzw;
    // mul r2.x, |r1.y|, |r1.y|
    r2.x = ((abs(r1.yyyy))*(abs(r1.yyyy))).x;
    // mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // lt r2.y, |r1.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // mov_sat r1.y, r1.y
    r1.y = (saturate(r1.yyyy)).y;
    // movc r2.x, r2.y, l(0), r2.x
    r2.x = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // add r0.y, r0.y, r2.x
    r0.y = ((r0.yyyy)+(r2.xxxx)).y;
    // add r2.xy, v7.xyxx, cb0[0].xyxx
    r2.xy = ((v7.xyxx)+(source[0].xyxx)).xy;
    // mul r2.xy, r2.xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // mad r2.zw, cb0[9].xxxx, l(0.000000, 0.000000, 0.090000, 0.090000), r2.xxxy
    r2.zw = ((source[9].xxxx)*(float4(0.000000,0.000000,0.090000,0.090000))+(r2.xxxy)).zw;
    // mul r2.zw, r2.zzzw, l(0.000000, 0.000000, 6.283185, 6.283185)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,6.283185,6.283185))).zw;
    // sincos r2.z, null, r2.z
    r2.z = (sin(r2.zzzz)).z;
    // sincos null, r2.w, r2.w
    r2.w = (cos(r2.wwww)).w;
    // mad r3.y, r2.w, l(0.005000), r2.y
    r3.y = ((r2.wwww)*(float4(0.005000,0.005000,0.005000,0.005000))+(r2.yyyy)).y;
    // mad r3.x, r2.z, l(0.005000), r2.x
    r3.x = ((r2.zzzz)*(float4(0.005000,0.005000,0.005000,0.005000))+(r2.xxxx)).x;
    // mul r2.xyzw, cb0[2].zwzw, cb0[9].xxxx
    r2.xyzw = ((source[2].zwzw)*(source[9].xxxx)).xyzw;
    // mul r4.xyzw, r2.zwzw, l(1.000000, 0.000000, -1.000000, 0.000000)
    r4.xyzw = ((r2.zwzw)*(float4(1.000000,0.000000,-1.000000,0.000000))).xyzw;
    // mul r2.xyzw, r2.xyzw, l(0.000000, 1.000000, 0.000000, -1.000000)
    r2.xyzw = ((r2.xyzw)*(float4(0.000000,1.000000,0.000000,-1.000000))).xyzw;
    // mad r3.zw, r3.xxxy, cb0[2].xxxy, r4.xxxy
    r3.zw = ((r3.xxxy)*(source[2].xxxy)+(r4.xxxy)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r3.zwzz, t0.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mul r3.zw, r3.xxxy, cb0[2].xxxy
    r3.zw = ((r3.xxxy)*(source[2].xxxy)).zw;
    // mad r4.xy, r3.zwzz, l(1.300000, 1.300000, 0.000000, 0.000000), r4.zwzz
    r4.xy = ((r3.zwzz)*(float4(1.300000,1.300000,0.000000,0.000000))+(r4.zwzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t1.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mad r4.xyz, r4.xyzx, l(0.875000, 0.875000, 0.875000, 0.000000), r5.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.875000,0.875000,0.875000,0.000000))+(r5.xyzx)).xyz;
    // mad r2.zw, r3.zzzw, l(0.000000, 0.000000, 2.300000, 2.300000), r2.zzzw
    r2.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.300000,2.300000))+(r2.zzzw)).zw;
    // mad r2.xy, r3.zwzz, l(1.700000, 1.700000, 0.000000, 0.000000), r2.xyxx
    r2.xy = ((r3.zwzz)*(float4(1.700000,1.700000,0.000000,0.000000))+(r2.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r2.zwzz, t1.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mul r5.xyz, r5.xyzx, l(0.669922, 0.669922, 0.669922, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.669922,0.669922,0.669922,0.000000))).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), l(-1.000000, -1.000000, -1.000000, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // mad r5.xyz, r6.xyzx, l(0.765625, 0.765625, 0.765625, 0.000000), r5.xyzx
    r5.xyz = ((r6.xyzx)*(float4(0.765625,0.765625,0.765625,0.000000))+(r5.xyzx)).xyz;
    // add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // mul r4.xyz, r4.xyzx, cb0[3].xyzx
    r4.xyz = ((r4.xyzx)*(source[3].xyzx)).xyz;
    // mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // mul r5.xyz, r5.xyzx, l(0.670000, 0.670000, 0.000000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.670000,0.670000,0.000000,0.000000))).xyz;
    // mad r4.xyz, r4.xyzx, l(0.330000, 0.330000, 1.000000, 0.000000), r5.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.330000,0.330000,1.000000,0.000000))+(r5.xyzx)).xyz;
    // mul r3.zw, r4.xxxy, v2.yyyy
    r3.zw = ((r4.xxxy)*(v2.yyyy)).zw;
    // add r4.xyz, r4.xyzx, l(-0.000000, -0.000000, -1.000000, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(-0.000000,-0.000000,-1.000000,0.000000))).xyz;
    // mad r4.xyz, v2.yyyy, r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((v2.yyyy)*(r4.xyzx)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // mul r3.zw, r3.zzzw, l(0.000000, 0.000000, 0.025000, 0.025000)
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,0.025000,0.025000))).zw;
    // mad r5.xy, r2.xyxx, l(0.300000, 0.340000, 0.000000, 0.000000), r3.zwzz
    r5.xy = ((r2.xyxx)*(float4(0.300000,0.340000,0.000000,0.000000))+(r3.zwzz)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // mad r6.xy, r2.zwzz, l(0.700000, 0.500000, 0.000000, 0.000000), r3.zwzz
    r6.xy = ((r2.zwzz)*(float4(0.700000,0.500000,0.000000,0.000000))+(r3.zwzz)).xy;
    // mad r3.zw, r3.xxxy, l(0.000000, 0.000000, 6.000000, 6.000000), r3.zzzw
    r3.zw = ((r3.xxxy)*(float4(0.000000,0.000000,6.000000,6.000000))+(r3.zzzw)).zw;
    // sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r3.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // add r3.z, r5.y, r5.x
    r3.z = ((r5.yyyy)+(r5.xxxx)).z;
    // add r3.z, r5.z, r3.z
    r3.z = ((r5.zzzz)+(r3.zzzz)).z;
    // mul r3.z, r3.z, v2.x
    r3.z = ((r3.zzzz)*(v2.xxxx)).z;
    // add r0.zw, -r0.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r0.zzzw))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // mul r3.z, r3.z, r0.w
    r3.z = ((r3.zzzz)*(r0.wwww)).z;
    // mul r3.z, r3.z, l(0.333330)
    r3.z = ((r3.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))).z;
    // mul r0.z, r0.z, l(2.500000)
    r0.z = ((r0.zzzz)*(float4(2.500000,2.500000,2.500000,2.500000))).z;
    // add r3.w, r7.y, r7.x
    r3.w = ((r7.yyyy)+(r7.xxxx)).w;
    // add r3.w, r7.z, r3.w
    r3.w = ((r7.zzzz)+(r3.wwww)).w;
    // mul r4.w, r3.w, l(0.333330)
    r4.w = ((r3.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // mad_sat r0.x, r3.w, l(0.083333), r0.x
    r0.x = (saturate((r3.wwww)*(float4(0.083333,0.083333,0.083333,0.083333))+(r0.xxxx))).x;
    // mad_sat r3.z, r4.w, r0.z, r3.z
    r3.z = (saturate((r4.wwww)*(r0.zzzz)+(r3.zzzz))).z;
    // log r3.w, r0.x
    r3.w = (log2(r0.xxxx)).w;
    // lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // mul r3.w, r3.w, l(50.000000)
    r3.w = ((r3.wwww)*(float4(50.000000,50.000000,50.000000,50.000000))).w;
    // exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // movc r0.x, r0.x, l(0), r3.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).x;
    // mul r3.z, r3.z, r0.x
    r3.z = ((r3.zzzz)*(r0.xxxx)).z;
    // mad r0.x, r0.y, r0.x, r3.z
    r0.x = ((r0.yyyy)*(r0.xxxx)+(r3.zzzz)).x;
    // min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // mul r0.x, r0.x, v2.w
    r0.x = ((r0.xxxx)*(v2.wwww)).x;
    // mul o0.w, r0.x, cb0[0].w
    output.targets[0].w = ((r0.xxxx)*(source[0].wwww)).w;
    // mul r0.xy, r4.xyxx, l(0.025000, 0.025000, 0.000000, 0.000000)
    r0.xy = ((r4.xyxx)*(float4(0.025000,0.025000,0.000000,0.000000))).xy;
    // mad r3.xy, r3.xyxx, l(6.000000, 6.000000, 0.000000, 0.000000), r0.xyxx
    r3.xy = ((r3.xyxx)*(float4(6.000000,6.000000,0.000000,0.000000))+(r0.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // add r3.x, r3.y, r3.x
    r3.x = ((r3.yyyy)+(r3.xxxx)).x;
    // add r3.x, r3.z, r3.x
    r3.x = ((r3.zzzz)+(r3.xxxx)).x;
    // mul r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)*(r3.xxxx)).z;
    // mad r2.xy, r2.xyxx, l(0.300000, 0.340000, 0.000000, 0.000000), r0.xyxx
    r2.xy = ((r2.xyxx)*(float4(0.300000,0.340000,0.000000,0.000000))+(r0.xyxx)).xy;
    // mad r0.xy, r2.zwzz, l(0.700000, 0.500000, 0.000000, 0.000000), r0.xyxx
    r0.xy = ((r2.zwzz)*(float4(0.700000,0.500000,0.000000,0.000000))+(r0.xyxx)).xy;
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // add r5.xyz, r3.xyzx, r2.xyzx
    r5.xyz = ((r3.xyzx)+(r2.xyzx)).xyz;
    // add r0.x, r5.y, r5.x
    r0.x = ((r5.yyyy)+(r5.xxxx)).x;
    // add r0.x, r5.z, r0.x
    r0.x = ((r5.zzzz)+(r0.xxxx)).x;
    // mul r0.x, r0.x, l(0.333330)
    r0.x = ((r0.xxxx)*(float4(0.333330,0.333330,0.333330,0.333330))).x;
    // mul r0.y, r0.x, v2.x
    r0.y = ((r0.xxxx)*(v2.xxxx)).y;
    // mul r0.y, r0.w, r0.y
    r0.y = ((r0.wwww)*(r0.yyyy)).y;
    // mad r0.w, r1.y, r1.y, r0.w
    r0.w = ((r1.yyyy)*(r1.yyyy)+(r0.wwww)).w;
    // mad_sat r0.y, r0.z, l(0.333330), r0.y
    r0.y = (saturate((r0.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))+(r0.yyyy))).y;
    // mul r2.xy, r3.xyxx, r2.xyxx
    r2.xy = ((r3.xyxx)*(r2.xyxx)).xy;
    // add r0.z, r2.y, r2.x
    r0.z = ((r2.yyyy)+(r2.xxxx)).z;
    // mad r0.z, r2.z, r3.z, r0.z
    r0.z = ((r2.zzzz)*(r3.zzzz)+(r0.zzzz)).z;
    // mul r0.z, r0.z, v2.x
    r0.z = ((r0.zzzz)*(v2.xxxx)).z;
    // mad_sat r0.z, r0.z, l(0.333330), l(-0.050000)
    r0.z = (saturate((r0.zzzz)*(float4(0.333330,0.333330,0.333330,0.333330))+(float4(-0.050000,-0.050000,-0.050000,-0.050000)))).z;
    // add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // mul r0.y, r0.y, l(1.330000)
    r0.y = ((r0.yyyy)*(float4(1.330000,1.330000,1.330000,1.330000))).y;
    // min r0.yw, r0.yyyw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = (min(r0.yyyw,float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // mul r2.xyz, cb0[6].xyzx, cb0[6].wwww
    r2.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // mul r2.xyz, r2.xyzx, l(0.660000, 0.660000, 0.660000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.660000,0.660000,0.660000,0.000000))).xyz;
    // mad r3.xyz, cb0[7].wwww, cb0[7].xyzx, -r2.xyzx
    r3.xyz = ((source[7].wwww)*(source[7].xyzx)+(-(r2.xyzx))).xyz;
    // mad r2.xyz, r0.wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // mul r3.xyz, cb0[8].xyzx, cb0[8].wwww
    r3.xyz = ((source[8].xyzx)*(source[8].wwww)).xyz;
    // mad r0.xzw, r0.xxxx, r3.xxyz, -r2.xxyz
    r0.xzw = ((r0.xxxx)*(r3.xxyz)+(-(r2.xxyz))).xzw;
    // mad r0.xyz, r0.yyyy, r0.xzwx, r2.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(r2.xyzx)).xyz;
    // mul r2.xyz, r4.xyzx, l(-1.000000, -1.000000, 1.000000, 0.000000)
    r2.xyz = ((r4.xyzx)*(float4(-1.000000,-1.000000,1.000000,0.000000))).xyz;
    // dp3_sat r0.w, cb0[4].xyzx, r2.xyzx
    r0.w = (saturate(dot((source[4].xyzx).xyz,(r2.xyzx).xyz).xxxx)).w;
    // mul r2.xyz, cb0[5].xyzx, cb0[5].wwww
    r2.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // mul r2.xyz, r2.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // mad r0.xyz, r0.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r2.xyz, r0.wwww, r4.xyzx
    r2.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // mul r3.yzw, r3.yyyy, cb0[11].xxyz
    r3.yzw = ((r3.yyyy)*(source[11].xxyz)).yzw;
    // mad r3.xyz, r3.xxxx, cb0[10].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[10].xyzx)+(r3.yzwy)).xyz;
    // mul r3.xyz, r3.xyzx, cb0[12].wwww
    r3.xyz = ((r3.xyzx)*(source[12].wwww)).xyz;
    // mul r4.xyz, r0.xyzx, r3.xyzx
    r4.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // dp2_sat r5.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r5.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r5.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t6.xyzw, s5
    r6.xyz = ((g_BakedDirectionalTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r6.xyz, r6.xyzx, cb0[14].xyzx
    r6.xyz = ((r6.xyzx)*(source[14].xyzx)).xyz;
    // dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t5.xyzw, s5
    r5.xyz = ((g_BakedAverageTexture.SampleLevel(SurfaceLightmapSampler,(v3.zwzz).xy,0.f)).xyzw).xyz;
    // mul r5.xyz, r5.xyzx, cb0[13].xyzx
    r5.xyz = ((r5.xyzx)*(source[13].xyzx)).xyz;
    // mul r7.xyz, r0.wwww, r5.xyzx
    r7.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // mad r3.xyz, r5.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // mul r5.xyz, r5.xyzx, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].xyzx)).xyz;
    // add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // div r3.xyz, r7.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)/(r3.xyzx)).xyz;
    // mad r4.xyz, r0.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xyzx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // dp3 r1.y, r2.xyzx, r1.xzwx
    r1.y = (dot((r2.xyzx).xyz,(r1.xzwx).xyz).xxxx).y;
    // mul r3.xyz, r1.yyyy, r2.xyzx
    r3.xyz = ((r1.yyyy)*(r2.xyzx)).xyz;
    // mad r1.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xzwx
    r1.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xzwx))).xyz;
    // dp2_sat r3.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // dp3_sat r3.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // dp3_sat r3.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // mul r1.xyz, r3.xyzx, r3.xyzx
    r1.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // dp3 r1.x, r6.xyzx, r1.xyzx
    r1.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // mad r1.yzw, r5.xxyz, r1.xxxx, r4.xxyz
    r1.yzw = ((r5.xxyz)*(r1.xxxx)+(r4.xxyz)).yzw;
    // mul r3.xyz, r1.xxxx, r5.xyzx
    r3.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // add r3.xyz, r1.yzwy, cb0[1].xyzx
    r3.xyz = ((r1.yzwy)+(source[1].xyzx)).xyz;
    // mad r3.xyz, r0.xyzx, cb0[12].xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // mad o0.xyz, r3.xyzx, v4.wwww, v4.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(v4.wwww)+(v4.xyzx)).xyz;
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
    // mul r3.xyz, r1.xxxx, v0.xyzx
    r3.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // mul r4.xyz, r0.zxyz, r3.yzxy
    r4.xyz = ((r0.zxyz)*(r3.yzxy)).xyz;
    // mad r4.xyz, r0.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // mul r3.xyz, r4.xyzx, v1.wwww
    r3.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // dp3 r0.y, r3.xyzx, r2.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
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
    // ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // movc r0.xy, r1.xxxx, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mul o4.z, r0.w, r1.y
    output.targets[4].z = ((r0.wwww)*(r1.yyyy)).z;
    // dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // ret
    return output;
}

#endif
