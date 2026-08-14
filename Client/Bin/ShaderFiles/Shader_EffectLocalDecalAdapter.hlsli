#ifndef LOSTARK_EFFECT_LOCAL_DECAL_ADAPTER_HLSLI
#define LOSTARK_EFFECT_LOCAL_DECAL_ADAPTER_HLSLI

/* Stable adapter ID materialized by the generic visual-program packet. */
static const uint EFFECT_RUNTIME_ADAPTER_LOCAL_DECAL_RT0_SIX_SRV_V1 = 14u;

float3 EffectLocalDecal_DecodeBC5Normal(float2 encodedNormal)
{
    const float2 xy = encodedNormal * 2.f - 1.f;
    return normalize(float3(xy, sqrt(saturate(1.f - dot(xy, xy)))));
}

EFFECT_PS_OUT Shade_EffectLocalDecalRt0SixSrvV1(float2 localUV)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 6u ||
        g_RuntimeMaterialV2TextureMask != 0x3fu ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0u ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorPolicy != 0u ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0u ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
        g_RuntimeMaterialV2ScalarCount != 22u ||
        g_RuntimeMaterialV2VectorCount != 3u ||
        g_RuntimeMaterialV2InputCount != 33u ||
        any(g_RuntimeMaterialV2InputConsumedMask !=
            uint2(0x820ec1ffu, 0x00000001u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask !=
            uint2(0x7df13e00u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 18u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0x3fffbu ||
        g_RuntimeMaterialV2StaticConsumedMask != 0x3ffffu ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x03u ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x3cu ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x0fu, 0x0fu, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0u, 0u, 0x0fu)))
    {
        clip(-1.f);
        return output;
    }

    const float4 b0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 b1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 b2 = g_RuntimeMaterialV2ScalarBlocks[2];
    const float4 b3 = g_RuntimeMaterialV2ScalarBlocks[3];
    const float4 b4 = g_RuntimeMaterialV2ScalarBlocks[4];
    const float4 b5 = g_RuntimeMaterialV2ScalarBlocks[5];
    const float4 diffuseColor = g_RuntimeMaterialV2Vectors[0];
    const float4 emissionColor = g_RuntimeMaterialV2Vectors[1];
    if (!all(isfinite(localUV)) ||
        !all(isfinite(b0)) || !all(isfinite(b1)) ||
        !all(isfinite(b2)) || !all(isfinite(b3)) ||
        !all(isfinite(b4)) || !all(isfinite(b5)) ||
        !all(isfinite(diffuseColor)) || !all(isfinite(emissionColor)) ||
        !isfinite(g_RuntimeMaterialV2NormalizedLife))
    {
        clip(-1.f);
        return output;
    }

    const float2 diffuseUV = localUV * b0.ww;
    const float4 diffuseSample = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, diffuseUV);
    const float2 normalEncoded = g_SourceTexture3.Sample(
        g_RuntimeMaterialV2Sampler3, localUV * b5.yy).rg;
    float3 tangentNormal = EffectLocalDecal_DecodeBC5Normal(normalEncoded);
    tangentNormal.xy *= b5.x;
    tangentNormal = normalize(tangentNormal);

    const float2 emissionUV = (localUV - float2(0.5f, 0.5f)) * b1.yz +
        float2(0.5f, 0.5f);
    const float emissionMask = g_SourceTexture5.Sample(
        g_RuntimeMaterialV2Sampler5, emissionUV).r;
    const float diffuseLuminance = dot(diffuseSample.rgb,
        float3(0.3f, 0.59f, 0.11f));
    float3 diffuse = lerp(diffuseSample.rgb, diffuseLuminance.xxx, b1.x) *
        diffuseColor.rgb * diffuseColor.a * saturate(tangentNormal.z);
    diffuse *= (1.f - emissionMask) * (1.f - emissionMask);

    const float2 animatedUV = diffuseUV + 0.2f * diffuseSample.rg +
        float2(g_EffectLocalTime * b2.x, 0.f);
    const float animatedDiffuse = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, animatedUV).r;
    const float emissionBase = animatedDiffuse *
        (animatedDiffuse + emissionMask) * emissionMask;
    const float poweredEmission = abs(emissionBase) < 1.0e-6f ? 0.f :
        pow(abs(emissionBase), b1.w);
    const float3 emission = poweredEmission * emissionColor.rgb *
        emissionColor.a;

    const float rawAlphaBase = abs(diffuseSample.a * b0.x);
    const float rawAlpha = rawAlphaBase < 1.0e-6f ? 0.f :
        pow(rawAlphaBase, b0.y);
    const float dissolveMask = g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, localUV * b4.x).g;
    const float life = saturate(g_RuntimeMaterialV2NormalizedLife);
    const float coverage = saturate(
        rawAlpha - 10.f * dissolveMask * (1.f - life)) * life;

    /*
     * The source cache owns six SRV roles and visible material arithmetic.
     * The common typed projector owns volume/depth rejection. Native tangent
     * parallax, orientation fade, fog/custom-light CBs and MRT2-5 are absent
     * from this RT0 adapter and remain explicitly suppressed.
     */
    output.SceneColor = float4(0.15f * diffuse + emission, coverage);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

#endif
