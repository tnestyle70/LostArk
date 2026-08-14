#ifndef SHADER_ARTIST31470_ACTIVE003_RIBBON_MATERIAL_HLSLI
#define SHADER_ARTIST31470_ACTIVE003_RIBBON_MATERIAL_HLSLI

/*
 * Bounded reconstructed material for Artist F source-active-003.
 *
 * The cooked parent graph is incomplete.  This function therefore keeps RGB
 * as an explicitly bounded two-texture carrier, but derives coverage from the
 * strictly pinned use_gra_r_channel=true lane and gra_pow=1.5 source rows.
 * Neither opaque DDS container alpha is a coverage source.  All four
 * DynamicParameter lanes and RT1 remain explicitly suppressed.
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
            uint2(0x00010481u, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask !=
            uint2(0x0000fb7eu, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x0fu, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0u, 0u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 1u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0x01u ||
        g_RuntimeMaterialV2StaticConsumedMask != 0x01u ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u)
    {
        clip(-1.f);
        return output;
    }

    const float signedPower = g_RuntimeMaterialV2ScalarBlocks[3].y;
    const float4 color = g_RuntimeMaterialV2Vectors[0];
    if (!all(isfinite(sourceUV)) || !isfinite(signedPower) ||
        !all(isfinite(vertexColor)) || !all(isfinite(color)) ||
        signedPower <= 0.f)
    {
        clip(-1.f);
        return output;
    }

    const float4 texture0 = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, sourceUV);
    const float4 texture1 = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, sourceUV);
    if (!all(isfinite(texture0)) || !all(isfinite(texture1)))
    {
        clip(-1.f);
        return output;
    }

    /* The missing cooked topology prevents an exact emissive reconstruction.
       Keep the admitted two source textures as bounded RGB only.  Coverage is
       the source-selected tex_main red lane shaped by gra_pow; it never reads
       the opaque BC container alpha. */
    const float3 boundedRgb = texture0.rgb *
        (0.5f + 0.5f * texture1.rgb) * color.rgb * max(color.a, 0.f);
    const float coverage = pow(saturate(texture0.r), signedPower) *
        max(vertexColor.a, 0.f);

    output.SceneColor = float4(clamp(boundedRgb,
        float3(0.f, 0.f, 0.f), float3(16.f, 16.f, 16.f)),
        saturate(coverage));
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

#endif
