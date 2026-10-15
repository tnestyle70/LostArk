// 정적 shadow texture의 거리 값을 전송 곡선으로 바꿔 빛의 통과 비율을 계산한다.
// PickPos.w에는 기존 법선 비트를 보존하며 shadow 채널 번호를 함께 기록한다.
#ifndef LOSTARK_STATIC_SHADOW_MAP_INCLUDED
#define LOSTARK_STATIC_SHADOW_MAP_INCLUDED

Texture2D g_StaticShadowTexture;
uint g_HasStaticShadow = 0u;
uint g_StaticShadowChannel = 0u;
float4 g_StaticShadowTransfer = float4(0.f, 1.f, 1.f, 0.f);
float4 g_StaticShadowScaleBias = 0.f;
SamplerState SourceStaticShadowSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

// PickPos is RGBA32_FLOAT. Preserve the 23 normal/RNM mantissa bits and
// use a finite positive exponent for the per-pixel shadow channel (0..15).
// CPicking reads XYZ and restores W=1; normal decoding reads only mantissa.
float EncodeMapStaticShadowChannel(float packedW)
{
    if (g_HasStaticShadow == 0u) return packedW;
    return asfloat((asuint(packedW) & 0x007fffffu) | ((127u + g_StaticShadowChannel) << 23u));
}

// Original signed-distance PS transfer; atlas texels, UVs and exponent come
// from source data. CPU bias/scale use the explicit PROJECT_ADAPTER width.
// Callers supply UV1 transformed by this placement's original shadow atlas UV.
float EvaluateMapStaticShadowDistance(float distance)
{
    return pow(saturate((distance + g_StaticShadowTransfer.x) * g_StaticShadowTransfer.y),
        g_StaticShadowTransfer.z);
}

float EvaluateMapStaticShadow(float2 shadowUV)
{
    if (g_HasStaticShadow == 0u) return 1.f;
    return EvaluateMapStaticShadowDistance(g_StaticShadowTexture.Sample(SourceStaticShadowSampler, shadowUV).r);
}

#endif
