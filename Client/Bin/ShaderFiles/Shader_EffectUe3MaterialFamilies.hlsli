#ifndef LOSTARK_SHADER_EFFECT_UE3_MATERIAL_FAMILIES_HLSLI
#define LOSTARK_SHADER_EFFECT_UE3_MATERIAL_FAMILIES_HLSLI

/*
 * Shared UE3 Material-family dispatch.
 *
 * Runtime documents select only a fixed opcode and a validated
 * RuntimeMaterialV2 packet.  No runtime-authored HLSL or graph bytecode is
 * accepted here.  The first cross-class family reuses the recovered
 * fx_m_pa_spritewave_01_tr equation that was originally admitted for Artist
 * 31470 active-011.
 */
#include "Shader_Artist31470Active011OuterMaterial.hlsli"

static const uint RUNTIME_MATERIAL_V2_UE3_SPRITEWAVE_TR = 15u;
static const uint RUNTIME_MATERIAL_V2_UE3_GLASSHOLE02_SPRITE_K01 = 16u;

bool EffectUe3Glasshole02K01PacketIsValid()
{
    return g_RuntimeMaterialV2Enabled == 1u &&
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_GLASSHOLE02_SPRITE_K01 &&
        g_RuntimeMaterialV2TextureLaneCount == 3u &&
        g_RuntimeMaterialV2TextureMask == 0x7u &&
        g_SourceTextureMask == 0x7u &&
        g_RuntimeMaterialV2DynamicConsumedMask == 0x1u &&
        g_RuntimeMaterialV2DynamicSuppressedMask == 0xeu &&
        g_RuntimeMaterialV2ParticleColorPolicy == 2u &&
        g_RuntimeMaterialV2ParticleColorConsumedMask == 0xfu &&
        g_RuntimeMaterialV2ParticleColorSuppressedMask == 0u &&
        g_RuntimeMaterialV2ScalarCount == 32u &&
        g_RuntimeMaterialV2VectorCount == 2u &&
        g_RuntimeMaterialV2InputCount == 34u &&
        all(g_RuntimeMaterialV2InputConsumedMask ==
            uint2(0x77ff8cbfu, 0x3u)) &&
        all(g_RuntimeMaterialV2InputSuppressedMask ==
            uint2(0x88007340u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentConsumedMask ==
            uint3(0x7u, 0x7u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentSuppressedMask ==
            uint3(0x8u, 0x8u, 0u)) &&
        g_RuntimeMaterialV2StaticInputCount == 6u &&
        g_RuntimeMaterialV2StaticSelectedMask == 0x24u &&
        g_RuntimeMaterialV2StaticConsumedMask == 0x3fu &&
        g_RuntimeMaterialV2StaticSuppressedMask == 0u &&
        g_RuntimeMaterialV2RenderInputCount == 6u &&
        g_RuntimeMaterialV2RenderConsumedMask == 0x2fu &&
        g_RuntimeMaterialV2RenderSuppressedMask == 0x10u;
}

EFFECT_PS_OUT Shade_EffectUe3Glasshole02K01Particle(
    float2 localUV,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (!EffectUe3Glasshole02K01PacketIsValid())
    {
        clip(-1.f);
        return output;
    }

    const float4 p0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 p1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 p2 = g_RuntimeMaterialV2ScalarBlocks[2];
    const float4 p3 = g_RuntimeMaterialV2ScalarBlocks[3];
    const float4 p4 = g_RuntimeMaterialV2ScalarBlocks[4];
    const float4 p5 = g_RuntimeMaterialV2ScalarBlocks[5];
    const float4 p6 = g_RuntimeMaterialV2ScalarBlocks[6];
    const float4 p7 = g_RuntimeMaterialV2ScalarBlocks[7];

    const float radialSize = max(abs(dynamicParameter.x), 0.0001f);
    const float2 glassCentered =
        (localUV - float2(0.5f, 0.5f)) / radialSize;
    const float glassRadius = length(glassCentered * 2.f);
    const float glassEdge = pow(saturate(
        1.f - glassRadius / max(abs(p7.y), 0.01f)),
        max(abs(p7.x), 1.f));

    const float2 normalUV = localUV * max(
        abs(p6.xy), float2(0.01f, 0.01f)) +
        float2(p7.z, p2.w) * g_EffectLocalTime;
    const float2 crackNormal = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, normalUV).rg * 2.f - 1.f;
    const float normalStrength = clamp(p6.z * 0.0025f, -0.1f, 0.1f);
    const float twist = p1.w * saturate(glassRadius);
    float2 auraUV = Rotate_LinearFlowUV(localUV, twist);
    auraUV = (auraUV - float2(0.5f, 0.5f)) * max(
        abs(p0.xy), float2(0.01f, 0.01f)) +
        float2(0.5f, 0.5f) +
        (p0.zw - float2(0.5f, 0.5f)) +
        p2.zw * g_EffectLocalTime + crackNormal * normalStrength;
    const float4 aura = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, auraUV);

    const float2 innerUV = localUV * max(abs(p5.w), 0.01f) +
        p4.xy * g_EffectLocalTime + crackNormal * p3.w * 0.01f;
    const float3 innerRgb = Desaturate_SourceColor(
        g_SourceTexture2.Sample(g_RuntimeMaterialV2Sampler2, innerUV).rgb,
        p5.x);
    const float inner = pow(saturate(dot(
        innerRgb, float3(0.299f, 0.587f, 0.114f))),
        max(abs(p4.z), 0.01f)) * max(p4.w, 0.f);
    const float auraAlpha = pow(saturate(aura.a),
        max(abs(p1.y), 0.01f)) * max(p1.x, 0.f);
    const float cardDistance = min(
        min(localUV.x, 1.f - localUV.x),
        min(localUV.y, 1.f - localUV.y));
    const float shape = saturate(auraAlpha) * glassEdge *
        saturate(cardDistance * 64.f);

    const float4 color = (g_ColorMultiply + g_ColorOffset) * particleColor;
    output.SceneColor.rgb =
        (aura.rgb * g_RuntimeMaterialV2Vectors[0].rgb * max(p1.x, 0.f) +
         innerRgb * g_RuntimeMaterialV2Vectors[1].rgb * inner) *
        color.rgb * max(g_EmissiveIntensity, 0.f);
    const float peakRadiance = max(output.SceneColor.r,
        max(output.SceneColor.g, output.SceneColor.b));
    output.SceneColor.rgb /= 1.f + peakRadiance;
    output.SceneColor.a = saturate(color.a * shape);

    const float glassDistortion = clamp(p5.z * 0.001f, -0.25f, 0.25f);
    output.Distortion.xy = crackNormal * glassDistortion *
        saturate(pow(max(inner, 0.0001f),
            rcp(max(abs(p5.y), 1.f)))) * shape;
    clip(output.SceneColor.a - g_ColorClip);
    return output;
}

EFFECT_PS_OUT Shade_EffectUe3SpriteWaveTrParticle(
    float2 localUV,
    float4 particleColor,
    float4 dynamicParameter)
{
    return Shade_Artist31470Active011OuterMaterial(
        localUV,
        float3(0.f, 0.f, 0.f),
        float3(0.f, 1.f, 0.f),
        float3(0.f, 0.f, 0.f),
        particleColor,
        dynamicParameter);
}

#endif
