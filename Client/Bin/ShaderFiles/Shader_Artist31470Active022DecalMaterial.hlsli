#ifndef SHADER_ARTIST31470_ACTIVE022_DECAL_MATERIAL_HLSLI
#define SHADER_ARTIST31470_ACTIVE022_DECAL_MATERIAL_HLSLI

/*
 * Artist F source-active-022 bounded Decal material reconstruction.
 *
 * The cooked material graph is incomplete.  This helper consumes only the
 * one admitted SRGB texture provider and the inputs named by the frozen v2
 * packet; every unresolved dissolve/noise/mask lane remains suppressed.
 */
EFFECT_PS_OUT Shade_Artist31470RuntimeMaterialV2Active022Decal(
    float2 decalUV,
    float4 particleColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 1u ||
        g_RuntimeMaterialV2TextureMask != 0x01u ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0u ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorPolicy !=
            RUNTIME_MATERIAL_V2_BLOCKED_COLOROVERLIFE_RGB_SUPPRESSED_V1 ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x08u ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0x07u ||
        g_RuntimeMaterialV2ScalarCount != 21u ||
        g_RuntimeMaterialV2VectorCount != 2u ||
        g_RuntimeMaterialV2InputCount != 29u ||
        any(g_RuntimeMaterialV2InputConsumedMask !=
            uint2(0x00001f87u, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask !=
            uint2(0x1fffe078u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 0u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0u ||
        g_RuntimeMaterialV2StaticConsumedMask != 0u ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x0fu, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0u, 0x0fu, 0u)))
    {
        clip(-1.f);
        return output;
    }

    /* Sequential scalar ABI: block 0 preserves input rows 0,3,4,5;
       block 1 preserves rows 7..10; block 2.x/y preserve rows 11/12.
       Only rows 0,7..12 are consumed by this bounded reconstruction. */
    const float desaturation = g_RuntimeMaterialV2ScalarBlocks[0].x;
    const float alphaIntensity = g_RuntimeMaterialV2ScalarBlocks[1].x;
    const float emissivePower = g_RuntimeMaterialV2ScalarBlocks[1].y;
    const float uvScale = g_RuntimeMaterialV2ScalarBlocks[1].z;
    const float rotationSpeedTurns = g_RuntimeMaterialV2ScalarBlocks[1].w;
    const float panY = g_RuntimeMaterialV2ScalarBlocks[2].x;
    const float panX = g_RuntimeMaterialV2ScalarBlocks[2].y;
    const float4 tintAndIntensity = g_RuntimeMaterialV2Vectors[0];

    if (!all(isfinite(float4(decalUV, g_EffectLocalTime, uvScale))) ||
        !all(isfinite(particleColor)) ||
        !all(isfinite(g_RuntimeMaterialV2ScalarBlocks[0])) ||
        !all(isfinite(g_RuntimeMaterialV2ScalarBlocks[1])) ||
        !all(isfinite(g_RuntimeMaterialV2ScalarBlocks[2])) ||
        !all(isfinite(g_RuntimeMaterialV2ScalarBlocks[3])) ||
        !all(isfinite(g_RuntimeMaterialV2ScalarBlocks[4])) ||
        !all(isfinite(g_RuntimeMaterialV2ScalarBlocks[5])) ||
        !all(isfinite(g_RuntimeMaterialV2Vectors[0])) ||
        !all(isfinite(g_RuntimeMaterialV2Vectors[1])))
    {
        clip(-1.f);
        return output;
    }

    float2 sourceUV = decalUV * uvScale;
    sourceUV = RuntimeMaterialV2_RotateUV(
        sourceUV, rotationSpeedTurns * g_EffectLocalTime);
    sourceUV += float2(panX, panY) * g_EffectLocalTime;

    /* The SRGB SRV performs the texture decode before this color equation. */
    const float4 decalSample = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, sourceUV);
    const float luminance = dot(decalSample.rgb,
        float3(0.299f, 0.587f, 0.114f));
    const float3 desaturated = lerp(
        decalSample.rgb, luminance.xxx, saturate(desaturation));
    const float3 powered = sign(desaturated) * pow(
        abs(desaturated), max(abs(emissivePower), 0.001f));
    const float3 rgb = powered * max(tintAndIntensity.rgb, 0.f) *
        max(tintAndIntensity.a, 0.f);
    const float alpha = saturate(decalSample.a *
        max(alphaIntensity, 0.f) * max(particleColor.a, 0.f));

    if (!all(isfinite(decalSample)) ||
        !all(isfinite(float4(rgb, alpha))))
    {
        clip(-1.f);
        return output;
    }

    output.SceneColor = float4(clamp(rgb,
        float3(0.f, 0.f, 0.f), float3(16.f, 16.f, 16.f)), alpha);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

#endif
