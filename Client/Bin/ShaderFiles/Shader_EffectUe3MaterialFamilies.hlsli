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

/* Opcodes 16 and 17 are reserved for the bounded Glasshole and Fluid family
   slices.  Artist D uses its own additive SpriteWave child contract. */
static const uint
    RUNTIME_MATERIAL_V2_UE3_SPRITEWAVE_AD_NO_EMISSIVE = 18u;

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

EFFECT_PS_OUT Shade_EffectUe3SpriteWaveAdditiveNoEmissiveParticle(
    float2 localUV,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    const bool child5 = g_RuntimeMaterialV2ScalarCount == 28u;
    const bool child6 = g_RuntimeMaterialV2ScalarCount == 24u;
    const uint expectedInputMask = child5 ? 0x0fffffffu : 0x00ffffffu;
    if ((!child5 && !child6) ||
        g_RuntimeMaterialV2TextureLaneCount != 3u ||
        g_RuntimeMaterialV2TextureMask != 0x07u ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0u ||
        g_RuntimeMaterialV2ParticleColorPolicy != 2u ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
        g_RuntimeMaterialV2VectorCount != 1u ||
        g_RuntimeMaterialV2InputCount != g_RuntimeMaterialV2ScalarCount ||
        any(g_RuntimeMaterialV2InputConsumedMask !=
            uint2(expectedInputMask, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask != uint2(0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x07u, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0x08u, 0u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 0u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0u ||
        g_RuntimeMaterialV2StaticConsumedMask != 0u ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u)
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
    const float4 b6 = child5 ? g_RuntimeMaterialV2ScalarBlocks[6] : 0.f;
    const float4 edgeColor = g_RuntimeMaterialV2Vectors[0];
    if (!all(isfinite(float4(localUV, g_EffectLocalTime,
            g_RuntimeMaterialV2NormalizedLife))) ||
        !all(isfinite(particleColor)) || !all(isfinite(dynamicParameter)) ||
        !all(isfinite(b0)) || !all(isfinite(b1)) || !all(isfinite(b2)) ||
        !all(isfinite(b3)) || !all(isfinite(b4)) || !all(isfinite(b5)) ||
        (child5 && !all(isfinite(b6))) || !all(isfinite(edgeColor)))
    {
        clip(-1.f);
        return output;
    }

    const float2 dissolvePan = b0.xy;
    const float2 dissolveTile = b0.zw;
    const float dissolveHardness = max(abs(b1.x), 1.0e-4f);
    const float edgeThin = b1.y;
    const float emissiveBase = b1.z;
    const float emissiveCorePower = b1.w;
    const float emissiveCoreStrength = b2.x;
    const float alphaStrength = b2.y;

    float2 mainDynamicPan = 0.f;
    float2 mainMove = 0.f;
    float2 mainPan = 0.f;
    float mainRotator = 0.f;
    float2 mainTile = 1.f;
    float sphereStrength = 0.f;
    float sphereStrengthMax = 0.f;
    float uvNoiseStrength = 0.f;
    float2 noisePan = 0.f;
    float2 noiseTile = 1.f;
    float2 noiseMove = 0.f;
    if (child5)
    {
        mainDynamicPan = b2.zw;
        mainMove = b3.xy;
        mainPan = b3.zw;
        mainRotator = b4.x;
        mainTile = b4.yz;
        sphereStrength = b4.w;
        sphereStrengthMax = b5.x;
        uvNoiseStrength = b5.y;
        noisePan = b5.zw;
        noiseTile = b6.xy;
        noiseMove = b6.zw;
    }
    else
    {
        mainMove = b2.zw;
        mainPan = b3.xy;
        mainRotator = b3.z;
        mainTile = float2(b3.w, b4.x);
        uvNoiseStrength = b4.y;
        noisePan = b4.zw;
        noiseTile = b5.xy;
        noiseMove = b5.zw;
    }

    const float2 noiseUV = localUV * noiseTile + noiseMove +
        noisePan * g_EffectLocalTime + float2(dynamicParameter.w * 0.01f, 0.f);
    /* Explicit lane contract: only noise.rg is UV displacement. */
    const float2 noiseRG = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, noiseUV).rg;
    const float2 warp = (noiseRG * 2.f - 1.f) *
        (uvNoiseStrength * dynamicParameter.z);

    const float2 rotatedUV = RuntimeMaterialV2_RotateUV(
        localUV, mainRotator * 0.125f);
    float2 mainUV = rotatedUV * mainTile + mainMove +
        mainPan * g_EffectLocalTime + warp;
    mainUV += child5 ? mainDynamicPan * dynamicParameter.x :
        float2(dynamicParameter.x * 0.01f, 0.f);
    const float4 mainSample = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, mainUV);

    const float2 dissolveUV = localUV * dissolveTile +
        dissolvePan * g_EffectLocalTime;
    /* Explicit lane contract: dissolve.gba never participates in coverage. */
    const float dissolveR = g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, dissolveUV).r;
    const float dissolveGate = saturate(
        (dissolveR + edgeThin - dynamicParameter.y) * dissolveHardness);
    const float dissolveEdge = saturate(1.f - abs(
        dissolveR + edgeThin - dynamicParameter.y) * dissolveHardness);

    float sphereGate = 1.f;
    if (child5)
    {
        const float radius = length(localUV - float2(0.5f, 0.5f));
        sphereGate = saturate(
            sphereStrength + (0.5f - radius) * sphereStrengthMax);
    }

    /* DXT1 main alpha is opaque padding.  main.r is the only silhouette
       coverage source; main.rgb remains unpremultiplied linear radiance. */
    const float baseCoverage = saturate(mainSample.r * alphaStrength);
    const float core = pow(max(abs(mainSample.r), 1.0e-6f),
        max(emissiveCorePower, 0.f));
    const float3 baseRadiance = max(mainSample.rgb, 0.f) *
        (max(emissiveBase, 0.f) + core * max(emissiveCoreStrength, 0.f));
    const float3 edgeRadiance = max(edgeColor.rgb, 0.f) * dissolveEdge;
    /* This is the same source-owned magnitude policy already sealed for the
       ParticleMaster/SpriteWave profiles in Shader_EffectCommon.  Cooked
       ParticleColor curves can be signed (Artist D child5 is exactly -0.5)
       or cross zero.  The texture/vector lanes own chroma, so a signed curve
       is a magnitude modulator; an exact zero curve is neutral. */
    const float3 particleColorMagnitude = abs(particleColor.rgb);
    const float peakParticleColorMagnitude = max(particleColorMagnitude.r,
        max(particleColorMagnitude.g, particleColorMagnitude.b));
    const float3 particleColorModulator =
        peakParticleColorMagnitude > 0.0001f ?
            particleColorMagnitude : float3(1.f, 1.f, 1.f);
    const float3 radiance = particleColorModulator *
        (baseRadiance + edgeRadiance);

    /* Source-recipe ParticleColor.a already carries the particle/lifetime
       envelope.  It is multiplied once, independently of the dissolve gate;
       normalized life is deliberately not folded into dissolve. */
    const float coverage = baseCoverage * dissolveGate * sphereGate *
        saturate(particleColor.a);
    if (!all(isfinite(noiseRG)) || !all(isfinite(mainSample)) ||
        !all(isfinite(float4(radiance, coverage))))
    {
        clip(-1.f);
        return output;
    }

    /* BS_EffectAdditive applies SrcAlpha to SceneColor.rgb exactly once. */
    output.SceneColor = float4(radiance, coverage);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

#endif
