#ifndef SHADER_ARTIST31470_ACTIVE003_RIBBON_MATERIAL_HLSLI
#define SHADER_ARTIST31470_ACTIVE003_RIBBON_MATERIAL_HLSLI

/*
 * Bounded reconstructed material for Artist F source-active-003.
 *
 * The cooked parent graph is incomplete.  This function therefore executes
 * only the already-approved two-texture numeric evaluator and keeps all four
 * DynamicParameter lanes explicitly suppressed.  It must be included after
 * Shader_Artist31470RuntimeMaterial.hlsli has declared the immutable packet
 * ABI, source textures, sampler lanes and g_EffectLocalTime.
 */
EFFECT_PS_OUT Shade_Artist31470Active003RibbonMaterial(
    float2 sourceUV,
    float4 vertexColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 2u ||
        g_RuntimeMaterialV2TextureMask != 0x03u ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0u ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorPolicy !=
            RUNTIME_MATERIAL_V2_BLOCKED_COLOROVERLIFE_RGB_SUPPRESSED_V1 ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x08u ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0x07u ||
        g_RuntimeMaterialV2ScalarCount != 14u ||
        g_RuntimeMaterialV2VectorCount != 1u ||
        g_RuntimeMaterialV2InputCount != 17u ||
        any(g_RuntimeMaterialV2InputConsumedMask !=
            uint2(0x0001849du, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask !=
            uint2(0x00007b62u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x0fu, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0u, 0u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 1u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0x01u ||
        g_RuntimeMaterialV2StaticConsumedMask != 0u ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0x01u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u)
    {
        clip(-1.f);
        return output;
    }

    const float2 uvScale = float2(
        g_RuntimeMaterialV2ScalarBlocks[0].y,
        g_RuntimeMaterialV2ScalarBlocks[0].w);
    const float2 pan = float2(
        g_RuntimeMaterialV2ScalarBlocks[0].z,
        g_RuntimeMaterialV2ScalarBlocks[3].x);
    const float signedPower = g_RuntimeMaterialV2ScalarBlocks[3].y;
    const float4 color = g_RuntimeMaterialV2Vectors[0];
    if (!all(isfinite(float4(sourceUV, g_EffectLocalTime, signedPower))) ||
        !all(isfinite(vertexColor)) || !all(isfinite(uvScale)) ||
        !all(isfinite(pan)) || !all(isfinite(color)))
    {
        clip(-1.f);
        return output;
    }

    const float4 texture0 = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, sourceUV);
    const float4 texture1 = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, sourceUV);
    float4 result = texture0 * (0.5f + 0.5f * texture1);

    const float2 oracleUV = float2(0.375f, 0.625f) * uvScale +
        pan * g_EffectLocalTime;
    result.rgb += (frac(oracleUV.x) + frac(oracleUV.y)) * 0.03125f;
    result.rgb += frac(g_EffectLocalTime * 0.125f) * 0.015625f;
    result.rgb *= color.rgb * max(color.a, 0.f);
    result.rgb = sign(result.rgb) * pow(
        abs(result.rgb),
        max(abs(signedPower), 0.001f));

    output.SceneColor = float4(clamp(result.rgb,
        float3(0.f, 0.f, 0.f), float3(16.f, 16.f, 16.f)),
        saturate(result.a * max(vertexColor.a, 0.f)));
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

#endif
