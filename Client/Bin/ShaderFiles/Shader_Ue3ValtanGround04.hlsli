#ifndef SHADER_UE3_VALTAN_GROUND04_HLSLI
#define SHADER_UE3_VALTAN_GROUND04_HLSLI

/*
 * Client-owned register wrapper around the generated instruction-order
 * translation of Shade_Ue3_fx_d_de_ground_04_tr.hlsli.
 *
 * Source ABI: cb0[20], cb2[4], t0..t5 and s0..s5.
 * The equation body below is copied verbatim from the generated evidence file;
 * only its declarations are replaced by named, explicitly registered runtime
 * ABI variables.
 */
cbuffer Ue3ValtanGround04ConstantsCB0 : register(b0)
{
    float4 g_Ue3ValtanGround04CB0[20];
};

cbuffer Ue3ValtanGround04ConstantsCB2 : register(b2)
{
    float4 g_Ue3ValtanGround04CB2[4];
};

Texture2D<float4> g_Ue3ValtanGround04Texture0 : register(t0);
Texture2D<float4> g_Ue3ValtanGround04Texture1 : register(t1);
Texture2D<float4> g_Ue3ValtanGround04Texture2 : register(t2);
Texture2D<float4> g_Ue3ValtanGround04Texture3 : register(t3);
Texture2D<float4> g_Ue3ValtanGround04Texture4 : register(t4);
Texture2D<float4> g_Ue3ValtanGround04Texture5 : register(t5);

SamplerState g_Ue3ValtanGround04Sampler0 : register(s0);
SamplerState g_Ue3ValtanGround04Sampler1 : register(s1);
SamplerState g_Ue3ValtanGround04Sampler2 : register(s2);
SamplerState g_Ue3ValtanGround04Sampler3 : register(s3);
SamplerState g_Ue3ValtanGround04Sampler4 : register(s4);
SamplerState g_Ue3ValtanGround04Sampler5 : register(s5);

#define cb0 g_Ue3ValtanGround04CB0
#define cb2 g_Ue3ValtanGround04CB2
#define t0 g_Ue3ValtanGround04Texture0
#define t1 g_Ue3ValtanGround04Texture1
#define t2 g_Ue3ValtanGround04Texture2
#define t3 g_Ue3ValtanGround04Texture3
#define t4 g_Ue3ValtanGround04Texture4
#define t5 g_Ue3ValtanGround04Texture5
#define s0 g_Ue3ValtanGround04Sampler0
#define s1 g_Ue3ValtanGround04Sampler1
#define s2 g_Ue3ValtanGround04Sampler2
#define s3 g_Ue3ValtanGround04Sampler3
#define s4 g_Ue3ValtanGround04Sampler4
#define s5 g_Ue3ValtanGround04Sampler5

struct Shade_Ue3_fx_d_de_ground_04_tr_INPUT
{
    float4 v0;
    float4 v4;
    float4 v5;
    float4 v6;
    float4 v7;
};

struct Shade_Ue3_fx_d_de_ground_04_tr_OUTPUT
{
    float4 o0;
    float4 o2;
    float4 o3;
    float4 o4;
    float4 o5;
};

Shade_Ue3_fx_d_de_ground_04_tr_OUTPUT Shade_Ue3_fx_d_de_ground_04_tr(Shade_Ue3_fx_d_de_ground_04_tr_INPUT stage)
{
    float4 v0 = stage.v0;
    float4 v4 = stage.v4;
    float4 v5 = stage.v5;
    float4 v6 = stage.v6;
    float4 v7 = stage.v7;
    float4 r0 = (float4)0;
    float4 r1 = (float4)0;
    float4 r2 = (float4)0;
    float4 r3 = (float4)0;
    float4 r4 = (float4)0;
    float4 o0 = (float4)0;
    float4 o2 = (float4)0;
    float4 o3 = (float4)0;
    float4 o4 = (float4)0;
    float4 o5 = (float4)0;

    // add r0.xy, -v4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    float stage1_x = -v4.x + 1.0f;
    float stage1_y = -v4.y + 1.0f;
    r0.x = stage1_x;
    r0.y = stage1_y;
    // mul r0.xy, r0.xyxx, v4.xyxx
    float stage2_x = r0.x * v4.x;
    float stage2_y = r0.y * v4.y;
    r0.x = stage2_x;
    r0.y = stage2_y;
    // add r1.x, v4.w, -cb0[0].x
    r1.x = v4.w + -cb0[0].x;
    // add r0.z, r1.x, l(0.001000)
    r0.z = r1.x + 0.001f;
    // add r1.x, -v4.w, cb0[0].y
    r1.x = -v4.w + cb0[0].y;
    // add r0.w, r1.x, l(0.001000)
    r0.w = r1.x + 0.001f;
    // lt r0.xyzw, r0.xyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    float stage3_x = asfloat((r0.x < 0.0f) ? 0xffffffffu : 0u);
    float stage3_y = asfloat((r0.y < 0.0f) ? 0xffffffffu : 0u);
    float stage3_z = asfloat((r0.z < 0.0f) ? 0xffffffffu : 0u);
    float stage3_w = asfloat((r0.w < 0.0f) ? 0xffffffffu : 0u);
    r0.x = stage3_x;
    r0.y = stage3_y;
    r0.z = stage3_z;
    r0.w = stage3_w;
    // or r0.xy, r0.zwzz, r0.xyxx
    float stage4_x = asfloat(asuint(r0.z) | asuint(r0.x));
    float stage4_y = asfloat(asuint(r0.w) | asuint(r0.y));
    r0.x = stage4_x;
    r0.y = stage4_y;
    // or r0.x, r0.y, r0.x
    r0.x = asfloat(asuint(r0.y) | asuint(r0.x));
    // discard_nz r0.x
    if (asuint(r0.x) != 0u) discard;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.zxyw, s0, l(0.000000)
    float4 fetch5 = t0.SampleBias(s0, v4.xy, 0.0f);
    r0.x = fetch5.z;
    // add r0.x, r0.x, -cb0[8].x
    r0.x = r0.x + -cb0[8].x;
    // mul r0.x, r0.x, cb0[8].y
    r0.x = r0.x * cb0[8].y;
    // mad r0.x, r0.x, l(0.050000), l(-0.025000)
    r0.x = r0.x * 0.05f + -0.025f;
    // dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = dot(v6.xyz, v6.xyz);
    // rsq r0.y, r0.y
    r0.y = rsqrt(r0.y);
    // mul r0.yz, r0.yyyy, v6.xxyx
    float stage6_y = r0.y * v6.x;
    float stage6_z = r0.y * v6.y;
    r0.y = stage6_y;
    r0.z = stage6_z;
    // mad r0.xy, r0.xxxx, r0.yzyy, v4.xyxx
    float stage7_x = r0.x * r0.y + v4.x;
    float stage7_y = r0.x * r0.z + v4.y;
    r0.x = stage7_x;
    r0.y = stage7_y;
    // add r0.zw, r0.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    float stage8_z = r0.x + -0.5f;
    float stage8_w = r0.y + -0.5f;
    r0.z = stage8_z;
    r0.w = stage8_w;
    // mad r0.zw, r0.zzzw, cb0[4].xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    float stage9_z = r0.z * cb0[4].x + 0.5f;
    float stage9_w = r0.w * cb0[4].y + 0.5f;
    r0.z = stage9_z;
    r0.w = stage9_w;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t4.xzyw, s1, l(0.000000)
    float4 fetch10 = t4.SampleBias(s1, r0.zw, 0.0f);
    r0.z = fetch10.y;
    // mov_sat r1.xy, cb0[1].xyxx
    float stage11_x = saturate(cb0[1].x);
    float stage11_y = saturate(cb0[1].y);
    r1.x = stage11_x;
    r1.y = stage11_y;
    // add r1.xy, -r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    float stage12_x = -r1.x + 1.0f;
    float stage12_y = -r1.y + 1.0f;
    r1.x = stage12_x;
    r1.y = stage12_y;
    // add r0.z, r0.z, -r1.y
    r0.z = r0.z + -r1.y;
    // mul_sat r0.z, r0.z, cb0[9].x
    r0.z = saturate(r0.z * cb0[9].x);
    // log r0.w, r0.z
    r0.w = log2(r0.z);
    // lt r0.z, r0.z, l(0.000001)
    r0.z = asfloat((r0.z < 1e-06f) ? 0xffffffffu : 0u);
    // mul r0.w, r0.w, cb0[9].y
    r0.w = r0.w * cb0[9].y;
    // exp r0.w, r0.w
    r0.w = exp2(r0.w);
    // movc r0.z, r0.z, l(0), r0.w
    r0.z = asuint(r0.z) != 0u ? asfloat(0u) : r0.w;
    // mad r1.yz, cb0[10].wwww, r0.xxyx, cb0[5].xxyx
    float stage13_y = cb0[10].w * r0.x + cb0[5].x;
    float stage13_z = cb0[10].w * r0.y + cb0[5].y;
    r1.y = stage13_y;
    r1.z = stage13_z;
    // sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t5.wxyz, s2, l(0.000000)
    float4 fetch14 = t5.SampleBias(s2, r1.yz, 0.0f);
    r1.y = fetch14.x;
    r1.z = fetch14.y;
    r1.w = fetch14.z;
    // mul r2.xyz, cb0[6].xyzx, cb0[6].wwww
    float stage15_x = cb0[6].x * cb0[6].w;
    float stage15_y = cb0[6].y * cb0[6].w;
    float stage15_z = cb0[6].z * cb0[6].w;
    r2.x = stage15_x;
    r2.y = stage15_y;
    r2.z = stage15_z;
    // mul r1.yzw, r1.yyzw, r2.xxyz
    float stage16_y = r1.y * r2.x;
    float stage16_z = r1.z * r2.y;
    float stage16_w = r1.w * r2.z;
    r1.y = stage16_y;
    r1.z = stage16_z;
    r1.w = stage16_w;
    // mad r1.yzw, r0.zzzz, r1.yyzw, cb0[3].xxyz
    float stage17_y = r0.z * r1.y + cb0[3].x;
    float stage17_z = r0.z * r1.z + cb0[3].y;
    float stage17_w = r0.z * r1.w + cb0[3].z;
    r1.y = stage17_y;
    r1.z = stage17_z;
    r1.w = stage17_w;
    // dp3 r0.z, v7.xyzx, v7.xyzx
    r0.z = dot(v7.xyz, v7.xyz);
    // rsq r0.z, r0.z
    r0.z = rsqrt(r0.z);
    // mul r0.z, r0.z, v7.z
    r0.z = r0.z * v7.z;
    // mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    float stage18_z = r0.z * 0.5f + 0.5f;
    float stage18_w = r0.z * -0.5f + 0.5f;
    r0.z = stage18_z;
    r0.w = stage18_w;
    // mul r0.zw, r0.zzzw, r0.zzzw
    float stage19_z = r0.z * r0.z;
    float stage19_w = r0.w * r0.w;
    r0.z = stage19_z;
    r0.w = stage19_w;
    // mul r2.xyz, r0.wwww, cb0[18].xyzx
    float stage20_x = r0.w * cb0[18].x;
    float stage20_y = r0.w * cb0[18].y;
    float stage20_z = r0.w * cb0[18].z;
    r2.x = stage20_x;
    r2.y = stage20_y;
    r2.z = stage20_z;
    // mad r2.xyz, r0.zzzz, cb0[17].xyzx, r2.xyzx
    float stage21_x = r0.z * cb0[17].x + r2.x;
    float stage21_y = r0.z * cb0[17].y + r2.y;
    float stage21_z = r0.z * cb0[17].z + r2.z;
    r2.x = stage21_x;
    r2.y = stage21_y;
    r2.z = stage21_z;
    // mul r2.xyz, r2.xyzx, cb0[19].wwww
    float stage22_x = r2.x * cb0[19].w;
    float stage22_y = r2.y * cb0[19].w;
    float stage22_z = r2.z * cb0[19].w;
    r2.x = stage22_x;
    r2.y = stage22_y;
    r2.z = stage22_z;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t2.zwxy, s4, l(0.000000)
    float4 fetch23 = t2.SampleBias(s4, r0.xy, 0.0f);
    r0.z = fetch23.x;
    r0.w = fetch23.y;
    // mad r3.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    float stage24_x = r0.z * 2.0f + -1.0f;
    float stage24_y = r0.w * 2.0f + -1.0f;
    r3.x = stage24_x;
    r3.y = stage24_y;
    // dp2 r0.z, r3.xyxx, r3.xyxx
    r0.z = dot(r3.xy, r3.xy);
    // add r0.z, -r0.z, l(1.000000)
    r0.z = -r0.z + 1.0f;
    // max r0.z, r0.z, l(0.000000)
    r0.z = max(r0.z, 0.0f);
    // sqrt r0.z, r0.z
    r0.z = sqrt(r0.z);
    // add r3.z, r0.z, l(0.000010)
    r3.z = r0.z + 1e-05f;
    // dp3 r0.z, r3.xyzx, r3.xyzx
    r0.z = dot(r3.xyz, r3.xyz);
    // sqrt r0.z, r0.z
    r0.z = sqrt(r0.z);
    // div r0.z, r3.z, r0.z
    r0.z = r3.z / r0.z;
    // max r0.z, r0.z, l(0.200000)
    r0.z = max(r0.z, 0.2f);
    // min r0.z, r0.z, l(1.000000)
    r0.z = min(r0.z, 1.0f);
    // sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.xyxx, t3.xyzw, s5, l(0.000000)
    float4 fetch25 = t3.SampleBias(s5, r0.xy, 0.0f);
    r3.x = fetch25.x;
    r3.y = fetch25.y;
    r3.z = fetch25.z;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(0.000000)
    float4 fetch26 = t0.SampleBias(s0, r0.xy, 0.0f);
    r0.x = fetch26.x;
    // mul_sat r0.x, r0.x, cb0[11].z
    r0.x = saturate(r0.x * cb0[11].z);
    // dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = dot(r3.xyz, float3(0.3f, 0.59f, 0.11f));
    // add r4.xyz, -r3.xyzx, r0.yyyy
    float stage27_x = -r3.x + r0.y;
    float stage27_y = -r3.y + r0.y;
    float stage27_z = -r3.z + r0.y;
    r4.x = stage27_x;
    r4.y = stage27_y;
    r4.z = stage27_z;
    // mad r3.xyz, cb0[14].xxxx, r4.xyzx, r3.xyzx
    float stage28_x = cb0[14].x * r4.x + r3.x;
    float stage28_y = cb0[14].x * r4.y + r3.y;
    float stage28_z = cb0[14].x * r4.z + r3.z;
    r3.x = stage28_x;
    r3.y = stage28_y;
    r3.z = stage28_z;
    // mul r4.xyz, cb0[7].xyzx, cb0[7].wwww
    float stage29_x = cb0[7].x * cb0[7].w;
    float stage29_y = cb0[7].y * cb0[7].w;
    float stage29_z = cb0[7].z * cb0[7].w;
    r4.x = stage29_x;
    r4.y = stage29_y;
    r4.z = stage29_z;
    // mul r3.xyz, r3.xyzx, r4.xyzx
    float stage30_x = r3.x * r4.x;
    float stage30_y = r3.y * r4.y;
    float stage30_z = r3.z * r4.z;
    r3.x = stage30_x;
    r3.y = stage30_y;
    r3.z = stage30_z;
    // mul r0.yzw, r0.zzzz, r3.xxyz
    float stage31_y = r0.z * r3.x;
    float stage31_z = r0.z * r3.y;
    float stage31_w = r0.z * r3.z;
    r0.y = stage31_y;
    r0.z = stage31_z;
    r0.w = stage31_w;
    // mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    float stage32_y = r0.y * cb2[3].w + cb2[3].x;
    float stage32_z = r0.z * cb2[3].w + cb2[3].y;
    float stage32_w = r0.w * cb2[3].w + cb2[3].z;
    r0.y = stage32_y;
    r0.z = stage32_z;
    r0.w = stage32_w;
    // mad r1.yzw, r2.xxyz, r0.yyzw, r1.yyzw
    float stage33_y = r2.x * r0.y + r1.y;
    float stage33_z = r2.y * r0.z + r1.z;
    float stage33_w = r2.z * r0.w + r1.w;
    r1.y = stage33_y;
    r1.z = stage33_z;
    r1.w = stage33_w;
    // mul r2.xyz, r0.yzwy, r2.xyzx
    float stage34_x = r0.y * r2.x;
    float stage34_y = r0.z * r2.y;
    float stage34_z = r0.w * r2.z;
    r2.x = stage34_x;
    r2.y = stage34_y;
    r2.z = stage34_z;
    // dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    o4.y = dot(r2.xyz, float3(0.3f, 0.59f, 0.11f));
    // mad r1.yzw, r0.yyzw, cb0[19].xxyz, r1.yyzw
    float stage35_y = r0.y * cb0[19].x + r1.y;
    float stage35_z = r0.z * cb0[19].y + r1.z;
    float stage35_w = r0.w * cb0[19].z + r1.w;
    r1.y = stage35_y;
    r1.z = stage35_z;
    r1.w = stage35_w;
    // mov o3.xyz, r0.yzwy
    float stage36_x = r0.y;
    float stage36_y = r0.z;
    float stage36_z = r0.w;
    o3.x = stage36_x;
    o3.y = stage36_y;
    o3.z = stage36_z;
    // mad o0.xyz, r1.yzwy, v5.wwww, v5.xyzx
    float stage37_x = r1.y * v5.w + v5.x;
    float stage37_y = r1.z * v5.w + v5.y;
    float stage37_z = r1.w * v5.w + v5.z;
    o0.x = stage37_x;
    o0.y = stage37_y;
    o0.z = stage37_z;
    // add r2.xyzw, v4.xyxy, l(-0.500000, -0.500000, -0.500000, -0.500000)
    float stage38_x = v4.x + -0.5f;
    float stage38_y = v4.y + -0.5f;
    float stage38_z = v4.x + -0.5f;
    float stage38_w = v4.y + -0.5f;
    r2.x = stage38_x;
    r2.y = stage38_y;
    r2.z = stage38_z;
    r2.w = stage38_w;
    // dp4 r0.y, r2.xyzw, r2.xyzw
    r0.y = dot(r2.xyzw, r2.xyzw);
    // min r0.y, r0.y, l(1.000000)
    r0.y = min(r0.y, 1.0f);
    // add r0.y, -r0.y, l(1.000000)
    r0.y = -r0.y + 1.0f;
    // mul r0.zw, v4.xxxy, cb0[11].wwww
    float stage39_z = v4.x * cb0[11].w;
    float stage39_w = v4.y * cb0[11].w;
    r0.z = stage39_z;
    r0.w = stage39_w;
    // sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t1.yzxw, s3, l(0.000000)
    float4 fetch40 = t1.SampleBias(s3, r0.zw, 0.0f);
    r0.z = fetch40.x;
    // mul r0.z, r0.z, cb0[12].x
    r0.z = r0.z * cb0[12].x;
    // max r0.z, r0.z, l(0.010000)
    r0.z = max(r0.z, 0.01f);
    // min r0.z, r0.z, l(0.950000)
    r0.z = min(r0.z, 0.95f);
    // add r0.y, -r0.z, r0.y
    r0.y = -r0.z + r0.y;
    // mad r0.y, cb0[12].y, r0.y, r0.z
    r0.y = cb0[12].y * r0.y + r0.z;
    // add r0.y, -r1.x, r0.y
    r0.y = -r1.x + r0.y;
    // mul_sat r0.y, r0.y, cb0[12].z
    r0.y = saturate(r0.y * cb0[12].z);
    // log r0.z, r0.y
    r0.z = log2(r0.y);
    // lt r0.y, r0.y, l(0.000001)
    r0.y = asfloat((r0.y < 1e-06f) ? 0xffffffffu : 0u);
    // mul r0.z, r0.z, cb0[12].w
    r0.z = r0.z * cb0[12].w;
    // exp r0.z, r0.z
    r0.z = exp2(r0.z);
    // movc r0.y, r0.y, l(0), r0.z
    r0.y = asuint(r0.y) != 0u ? asfloat(0u) : r0.z;
    // mul r0.x, r0.y, r0.x
    r0.x = r0.y * r0.x;
    // mad_sat r0.x, r0.x, cb0[1].w, cb0[13].w
    r0.x = saturate(r0.x * cb0[1].w + cb0[13].w);
    // mul r0.x, r0.x, cb0[2].x
    r0.x = r0.x * cb0[2].x;
    // add r0.y, -|v4.w|, cb0[0].y
    r0.y = -(abs(v4.w)) + cb0[0].y;
    // mul r0.y, r0.y, l(5.000000)
    r0.y = r0.y * 5.0f;
    // div_sat r0.y, r0.y, cb0[0].y
    r0.y = saturate(r0.y / cb0[0].y);
    // mul r0.y, r0.y, v4.z
    r0.y = r0.y * v4.z;
    // mul o0.w, r0.y, r0.x
    o0.w = r0.y * r0.x;
    // dp3 r0.x, v0.xyzx, v0.xyzx
    r0.x = dot(v0.xyz, v0.xyz);
    // rsq r0.x, r0.x
    r0.x = rsqrt(r0.x);
    // mul r0.xyz, r0.xxxx, v0.xyzx
    float stage41_x = r0.x * v0.x;
    float stage41_y = r0.x * v0.y;
    float stage41_z = r0.x * v0.z;
    r0.x = stage41_x;
    r0.y = stage41_y;
    r0.z = stage41_z;
    // dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = dot(float3(1.0f, 1.0f, 1.0f), abs(r0.xyz));
    // div r0.xy, r0.xyxx, r0.wwww
    float stage42_x = r0.x / r0.w;
    float stage42_y = r0.y / r0.w;
    r0.x = stage42_x;
    r0.y = stage42_y;
    // ge r0.z, l(0.000000), r0.z
    r0.z = asfloat((0.0f >= r0.z) ? 0xffffffffu : 0u);
    // ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    float stage43_x = asfloat((r0.x >= 0.0f) ? 0xffffffffu : 0u);
    float stage43_y = asfloat((r0.y >= 0.0f) ? 0xffffffffu : 0u);
    r1.x = stage43_x;
    r1.y = stage43_y;
    // movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    float stage44_x = asuint(r1.x) != 0u ? 1.0f : -1.0f;
    float stage44_y = asuint(r1.y) != 0u ? 1.0f : -1.0f;
    r1.x = stage44_x;
    r1.y = stage44_y;
    // mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    float stage45_x = -(abs(r0.y)) * r1.x + r1.x;
    float stage45_y = -(abs(r0.x)) * r1.y + r1.y;
    r1.x = stage45_x;
    r1.y = stage45_y;
    // movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    float stage46_x = asuint(r0.z) != 0u ? r1.x : r0.x;
    float stage46_y = asuint(r0.z) != 0u ? r1.y : r0.y;
    r0.x = stage46_x;
    r0.y = stage46_y;
    // mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    float stage47_x = r0.x * 0.5f + 0.5f;
    float stage47_y = r0.y * 0.5f + 0.5f;
    o2.x = stage47_x;
    o2.y = stage47_y;
    // mov o2.zw, l(0,0,1.000000,0)
    float stage48_z = 1.0f;
    float stage48_w = asfloat(0u);
    o2.z = stage48_z;
    o2.w = stage48_w;
    // mov o3.w, l(0)
    o3.w = asfloat(0u);
    // mov o4.xzw, l(0,0,0,0)
    float stage49_x = asfloat(0u);
    float stage49_z = asfloat(0u);
    float stage49_w = asfloat(0u);
    o4.x = stage49_x;
    o4.z = stage49_z;
    o4.w = stage49_w;
    // mov o5.xyzw, l(0,0,0,0)
    float stage50_x = asfloat(0u);
    float stage50_y = asfloat(0u);
    float stage50_z = asfloat(0u);
    float stage50_w = asfloat(0u);
    o5.x = stage50_x;
    o5.y = stage50_y;
    o5.z = stage50_z;
    o5.w = stage50_w;

    Shade_Ue3_fx_d_de_ground_04_tr_OUTPUT result;
    result.o0 = o0;
    result.o2 = o2;
    result.o3 = o3;
    result.o4 = o4;
    result.o5 = o5;
    return result;
}

#undef cb0
#undef cb2
#undef t0
#undef t1
#undef t2
#undef t3
#undef t4
#undef t5
#undef s0
#undef s1
#undef s2
#undef s3
#undef s4
#undef s5

#endif
