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
static const uint RUNTIME_MATERIAL_V2_UE3_FLUID01_SPRITE_W_FD_01_3 = 17u;
static const uint
    RUNTIME_MATERIAL_V2_UE3_RIBBONLIQUID01_PARENT_DEFAULT = 20u;
static const uint
    RUNTIME_MATERIAL_V2_PROJECT_BASE_COVERAGE_EMISSIVE_DISSOLVE_RECT = 21u;
static const uint
    RUNTIME_MATERIAL_V2_UE3_WPO_SINWAVE_ELECTRIC_RT0_MESH = 22u;
static const uint
    RUNTIME_MATERIAL_V2_PROJECT_TUNED_BASE_COVERAGE_SRGB = 1001u;
static const uint
    RUNTIME_MATERIAL_V2_PROJECT_TUNED_BASE_COVERAGE_LINEAR = 1002u;
static const uint
    RUNTIME_MATERIAL_V2_PROJECT_TUNED_WATER_DROPLET_BURST = 1003u;
static const uint
    RUNTIME_MATERIAL_V2_PROJECT_TUNED_GLASS_MESH_V1 = 1004u;

bool EffectUe3RibbonLiquid01ParentDefaultPacketIsValid()
{
    return g_RuntimeMaterialV2Enabled == 1u &&
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_RIBBONLIQUID01_PARENT_DEFAULT &&
        g_RuntimeMaterialV2TextureLaneCount == 4u &&
        g_RuntimeMaterialV2TextureMask == 0xfu &&
        g_SourceTextureMask == 0xfu &&
        g_RuntimeMaterialV2DynamicConsumedMask == 0xfu &&
        g_RuntimeMaterialV2DynamicSuppressedMask == 0u &&
        g_RuntimeMaterialV2ParticleColorPolicy == 2u &&
        g_RuntimeMaterialV2ParticleColorConsumedMask == 0x8u &&
        g_RuntimeMaterialV2ParticleColorSuppressedMask == 0x7u &&
        g_RuntimeMaterialV2ScalarCount == 12u &&
        g_RuntimeMaterialV2VectorCount == 1u &&
        g_RuntimeMaterialV2InputCount == 17u &&
        all(g_RuntimeMaterialV2InputConsumedMask ==
            uint2(0x1ff7fu, 0u)) &&
        all(g_RuntimeMaterialV2InputSuppressedMask == uint2(0x80u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentConsumedMask ==
            uint3(0xfu, 0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentSuppressedMask ==
            uint3(0u, 0u, 0u)) &&
        g_RuntimeMaterialV2StaticInputCount == 0u &&
        g_RuntimeMaterialV2StaticSelectedMask == 0u &&
        g_RuntimeMaterialV2StaticConsumedMask == 0u &&
        g_RuntimeMaterialV2StaticSuppressedMask == 0u &&
        g_RuntimeMaterialV2RenderInputCount == 6u &&
        g_RuntimeMaterialV2RenderConsumedMask == 0x2fu &&
        g_RuntimeMaterialV2RenderSuppressedMask == 0x10u;
}

EFFECT_PS_OUT Shade_EffectUe3RibbonLiquid01ParentDefault(
    float2 ribbonUV,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (!EffectUe3RibbonLiquid01ParentDefaultPacketIsValid())
    {
        clip(-1.f);
        return output;
    }

    /* The packet is the effective child value set evaluated against the
       exact parent-default map.  Its raw BasePass DXBC and binding wires are
       evidence; this HLSL is a bounded typed replay because the original
       beam/trail VF reflection basis and UE3 CDO sampler defaults are not
       native-executed here. */
    const float4 p0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 p1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 p2 = g_RuntimeMaterialV2ScalarBlocks[2];
    const float4 reflectColor = g_RuntimeMaterialV2Vectors[0];
    if (!all(isfinite(ribbonUV)) || !all(isfinite(particleColor)) ||
        !all(isfinite(dynamicParameter)) || !all(isfinite(p0)) ||
        !all(isfinite(p1)) || !all(isfinite(p2)) ||
        !all(isfinite(reflectColor)))
    {
        clip(-1.f);
        return output;
    }

    const float normalStrength = p0.x;
    const float alphaStrength = p0.y;
    const float reflectionUvScale = p0.z;
    const float2 normalUvScale = max(abs(p1.xy), float2(0.01f, 0.01f));
    const float2 alphaUvScale = max(abs(p1.zw), float2(0.01f, 0.01f));
    const float2 normalPan = p2.xy;
    const float2 alphaPan = p2.zw;

    const float2 distortionUv = ribbonUV * normalUvScale +
        normalPan * g_EffectLocalTime;
    const float2 distortionNormal = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, distortionUv).rg * 2.f - 1.f;
    const float uvDistortion = dynamicParameter.w - 1.f;
    float2 warpedUv = ribbonUV;
    warpedUv.x += distortionNormal.x * uvDistortion;
    warpedUv.y = 0.5f + (ribbonUV.y - 0.5f) * dynamicParameter.x +
        distortionNormal.y * uvDistortion;

    const float2 alphaUv = warpedUv * alphaUvScale +
        alphaPan * g_EffectLocalTime;
    /* Parent expression 01.alphatex is bound to native t2/s3 and resolves to
       fx_k_auraline_14_ycl.  Keep this semantic mapping explicit instead of
       inferring a role from the texture's filename. */
    const float3 alphaTexture = g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, alphaUv).rgb;
    const float alphaCarrier = dot(alphaTexture,
        float3(0.33333334f, 0.33333334f, 0.33333334f));
    const float dissolveThreshold = 1.f - saturate(dynamicParameter.y);
    const float coverage = saturate(
        (alphaCarrier - dissolveThreshold) * max(alphaStrength, 0.f)) *
        saturate(particleColor.a);

    const float2 surfaceNormal = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, warpedUv * normalUvScale +
            normalPan * g_EffectLocalTime).rg * 2.f - 1.f;
    const float2 reflectionUv = float2(
        ribbonUV.x * max(abs(reflectionUvScale), 0.01f),
        saturate(0.5f + surfaceNormal.y * normalStrength * 0.5f));
    /* Parent expression 01.reflectiontex is bound to native t3/s2 and
       resolves to fx_a_fluid_003.  The native beam/trail reflection basis is
       not available on the typed carrier; reflectionUv is the bounded axis
       reconstruction and is intentionally reported as partial fidelity. */
    const float3 reflectionTexture = g_SourceTexture3.Sample(
        g_RuntimeMaterialV2Sampler3, reflectionUv).rgb;
    const float reflectionCarrier = dot(reflectionTexture,
        float3(0.33333334f, 0.33333334f, 0.33333334f));
    const float reflectionEnergy = reflectionCarrier * reflectionCarrier;
    const float3 radiance = max(reflectColor.rgb, 0.f) *
        max(reflectColor.w, 0.f) * reflectionEnergy *
        max(g_EmissiveIntensity, 0.f);

    if (!all(isfinite(float4(radiance, coverage))))
    {
        clip(-1.f);
        return output;
    }
    output.SceneColor = float4(
        radiance * max((g_ColorMultiply + g_ColorOffset).rgb, 0.f),
        coverage);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    clip(output.SceneColor.a - g_ColorClip);
    return output;
}

bool EffectUe3Fluid01SpriteWFd013PacketIsValid()
{
    return g_RuntimeMaterialV2Enabled == 1u &&
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_FLUID01_SPRITE_W_FD_01_3 &&
        g_RuntimeMaterialV2TextureLaneCount == 4u &&
        g_RuntimeMaterialV2TextureMask == 0xfu &&
        g_SourceTextureMask == 0xfu &&
        g_RuntimeMaterialV2DynamicConsumedMask == 0x7u &&
        g_RuntimeMaterialV2DynamicSuppressedMask == 0x8u &&
        g_RuntimeMaterialV2ParticleColorPolicy == 2u &&
        g_RuntimeMaterialV2ParticleColorConsumedMask == 0xfu &&
        g_RuntimeMaterialV2ParticleColorSuppressedMask == 0u &&
        g_RuntimeMaterialV2ScalarCount == 22u &&
        g_RuntimeMaterialV2VectorCount == 0u &&
        g_RuntimeMaterialV2InputCount == 22u &&
        all(g_RuntimeMaterialV2InputConsumedMask ==
            uint2(0x003fffffu, 0u)) &&
        all(g_RuntimeMaterialV2InputSuppressedMask == uint2(0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentConsumedMask ==
            uint3(0u, 0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentSuppressedMask ==
            uint3(0u, 0u, 0u)) &&
        g_RuntimeMaterialV2StaticInputCount == 0u &&
        g_RuntimeMaterialV2StaticSelectedMask == 0u &&
        g_RuntimeMaterialV2StaticConsumedMask == 0u &&
        g_RuntimeMaterialV2StaticSuppressedMask == 0u &&
        g_RuntimeMaterialV2RenderInputCount == 0u &&
        g_RuntimeMaterialV2RenderConsumedMask == 0u &&
        g_RuntimeMaterialV2RenderSuppressedMask == 0u;
}

EFFECT_PS_OUT Shade_EffectUe3Fluid01SpriteWFd013Particle(
    float2 localUV,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (!EffectUe3Fluid01SpriteWFd013PacketIsValid())
    {
        clip(-1.f);
        return output;
    }

    // Exact source scalar packet for fx_w_pa_fd_01_3_tr.  This is the
    // class-neutral typed successor of legacy bounded profile 34; it is a
    // source-evidenced reconstruction, not a native cooked-DXBC claim.
    const float4 p0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 p1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 p2 = g_RuntimeMaterialV2ScalarBlocks[2];
    const float4 p3 = g_RuntimeMaterialV2ScalarBlocks[3];
    const float4 p4 = g_RuntimeMaterialV2ScalarBlocks[4];
    const float4 p5 = g_RuntimeMaterialV2ScalarBlocks[5];

    const float2 noise01UV =
        localUV * max(abs(p1.w), 0.01f) +
        float2(p2.y, p2.x) * g_EffectLocalTime;
    const float2 noise02UV =
        localUV * max(abs(p2.w), 0.01f) +
        float2(p3.y, p3.x) * g_EffectLocalTime;
    const float2 noise01 =
        (g_SourceTexture2.Sample(
            g_RuntimeMaterialV2Sampler2, noise01UV).rg * 2.f - 1.f) * p2.z;
    const float2 noise02 =
        (g_SourceTexture3.Sample(
            g_RuntimeMaterialV2Sampler3, noise02UV).rg * 2.f - 1.f) * p3.z;
    const float2 uvNoise = clamp(noise01 + noise02, -0.25f, 0.25f);

    const float2 transitionUV =
        localUV * max(abs(p0.z), 0.01f) +
        float2(p1.x, p0.w) * g_EffectLocalTime + uvNoise;
    const float3 transitionColor = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, transitionUV).rgb;
    const float transitionCarrier = saturate(dot(
        transitionColor, float3(0.299f, 0.587f, 0.114f)));
    const float threshold = saturate(
        abs(dynamicParameter.x) / 1.5f * 0.8f + p0.y * 0.1f);
    const float transitionSoftness = max(abs(p0.x), 1.f / 255.f);
    const float fillGate = smoothstep(
        threshold - transitionSoftness,
        threshold + transitionSoftness, transitionCarrier);
    const float lineWidth = max(
        abs(p1.z) * transitionSoftness, 1.f / 255.f);
    const float lineGate = saturate(
        1.f - abs(transitionCarrier - threshold) / lineWidth);

    const float2 centeredUV = localUV - float2(0.5f, 0.5f);
    const float fresnelPower = max(
        abs(dynamicParameter.z) * max(abs(p4.w), 0.01f), 0.01f);
    const float radialEnvelope = pow(saturate(
        1.f - smoothstep(0.38f, 0.5f, length(centeredUV))),
        fresnelPower);
    const float alphaPower = max(abs(dynamicParameter.y), 0.01f);
    const float shape = pow(
        saturate(max(fillGate, lineGate)), alphaPower) * radialEnvelope;

    const float2 emissiveUV =
        localUV * max(abs(p4.yz), float2(0.01f, 0.01f)) + uvNoise;
    const float3 emissiveSample = Desaturate_SourceColor(
        g_SourceTexture1.Sample(
            g_RuntimeMaterialV2Sampler1, emissiveUV).rgb,
        saturate(p4.x));
    const float4 color = (g_ColorMultiply + g_ColorOffset) * particleColor;
    output.SceneColor.rgb =
        (emissiveSample * max(p3.w, 0.f) +
         transitionColor * lineGate * max(p1.y, 0.f)) *
        max(p5.y, 0.f) * color.rgb * max(g_EmissiveIntensity, 0.f);
    const float peakRadiance = max(output.SceneColor.r,
        max(output.SceneColor.g, output.SceneColor.b));
    output.SceneColor.rgb /= 1.f + peakRadiance;
    output.SceneColor.a = saturate(color.a * shape);

    output.Distortion.xy = uvNoise *
        clamp(p5.x * 0.01f, -0.25f, 0.25f) * shape;
    clip(output.SceneColor.a - g_ColorClip);
    return output;
}

static const uint
    RUNTIME_MATERIAL_V2_UE3_SPRITEWAVE_AD_NO_EMISSIVE = 18u;
static const uint RUNTIME_MATERIAL_V2_UE3_DRAGON_PH_MASKED_MESH = 19u;

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

bool EffectUe3DragonPhMaskedPacketIsValid()
{
    return g_RuntimeMaterialV2Enabled == 1u &&
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_DRAGON_PH_MASKED_MESH &&
        g_RuntimeMaterialV2TextureLaneCount == 5u &&
        g_RuntimeMaterialV2TextureMask == 0x1fu &&
        g_SourceTextureMask == 0x1fu &&
        g_RuntimeMaterialV2DynamicConsumedMask == 0x08u &&
        g_RuntimeMaterialV2DynamicSuppressedMask == 0x07u &&
        g_RuntimeMaterialV2ParticleColorPolicy == 2u &&
        g_RuntimeMaterialV2ParticleColorConsumedMask == 0x0fu &&
        g_RuntimeMaterialV2ParticleColorSuppressedMask == 0u &&
        g_RuntimeMaterialV2ScalarCount == 25u &&
        g_RuntimeMaterialV2VectorCount == 3u &&
        g_RuntimeMaterialV2InputCount == 25u &&
        all(g_RuntimeMaterialV2InputConsumedMask ==
            uint2(0x01ffffffu, 0u)) &&
        all(g_RuntimeMaterialV2InputSuppressedMask == uint2(0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentConsumedMask ==
            uint3(0x07u, 0x07u, 0x07u)) &&
        all(g_RuntimeMaterialV2VectorComponentSuppressedMask ==
            uint3(0x08u, 0x08u, 0x08u)) &&
        g_RuntimeMaterialV2StaticInputCount == 23u &&
        g_RuntimeMaterialV2StaticSelectedMask == 0x0013b74fu &&
        g_RuntimeMaterialV2StaticConsumedMask == 0x007fffffu &&
        g_RuntimeMaterialV2StaticSuppressedMask == 0u &&
        g_RuntimeMaterialV2RenderInputCount == 6u &&
        g_RuntimeMaterialV2RenderConsumedMask == 0x2fu &&
        g_RuntimeMaterialV2RenderSuppressedMask == 0x10u;
}

EFFECT_PS_OUT Shade_EffectUe3DragonPhMaskedMesh(
    float2 localUV,
    float3 worldPosition,
    float3 geometryNormal,
    float3 tangent,
    float3 binormal,
    float3 cameraPosition,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (!EffectUe3DragonPhMaskedPacketIsValid())
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
    const float4 b6 = g_RuntimeMaterialV2ScalarBlocks[6];
    const float4 diffuseColor = g_RuntimeMaterialV2Vectors[0];
    const float4 specularColor = g_RuntimeMaterialV2Vectors[1];
    const float4 emissionColor = g_RuntimeMaterialV2Vectors[2];
    if (!all(isfinite(float4(localUV, g_EffectLocalTime,
            g_RuntimeMaterialV2NormalizedLife))) ||
        !all(isfinite(float4(worldPosition, 1.f))) ||
        !all(isfinite(float4(geometryNormal, 1.f))) ||
        !all(isfinite(float4(tangent, 1.f))) ||
        !all(isfinite(float4(binormal, 1.f))) ||
        !all(isfinite(float4(cameraPosition, 1.f))) ||
        !all(isfinite(particleColor)) || !all(isfinite(dynamicParameter)) ||
        !all(isfinite(b0)) || !all(isfinite(b1)) || !all(isfinite(b2)) ||
        !all(isfinite(b3)) || !all(isfinite(b4)) || !all(isfinite(b5)) ||
        !all(isfinite(b6)) || !all(isfinite(diffuseColor)) ||
        !all(isfinite(specularColor)) || !all(isfinite(emissionColor)))
    {
        clip(-1.f);
        return output;
    }

    /* The recovered t1/s4 alpha lane is the only moving UV.  Normal,
       emission, diffuse and specular keep their independent stationary UVs;
       they are never folded into Detail.UV's single pan. */
    const float2 normalUV = localUV * max(abs(b0.xy), 0.0001f) +
        b0.zw * g_EffectLocalTime;
    const float2 alphaUV = localUV * max(abs(b1.yz), 0.0001f) +
        float2(b1.w, b2.x) * g_EffectLocalTime;
    const float2 emissionUV = localUV * max(abs(float2(b2.w, b3.x)), 0.0001f);
    const float2 diffuseUV = localUV * max(abs(b3.zw), 0.0001f);
    const float2 specularUV = localUV * max(abs(b4.zw), 0.0001f);

    const float2 encodedNormal = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, normalUV).rg * 2.f - 1.f;
    const float2 normalXY = clamp(encodedNormal * b1.x, -0.999f, 0.999f);
    const float normalZ = sqrt(saturate(1.f - dot(normalXY, normalXY)));
    const float3 mappedNormal = normalize(
        normalize(tangent) * normalXY.x +
        normalize(binormal) * normalXY.y +
        normalize(geometryNormal) * normalZ);

    /* Static switch 21.invert is sealed in the packet mask.  Dynamic W is a
       dissolve threshold; ParticleColor alpha remains a separate lifetime
       envelope and normalized life is deliberately not folded into either. */
    const float alphaR = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, alphaUV).r;
    const float invertedAlpha = pow(saturate(1.f - alphaR),
        max(abs(b2.z), 0.0001f));
    const float dissolveThreshold = 1.f - saturate(dynamicParameter.w);
    const float dissolveGate = saturate(
        (invertedAlpha - dissolveThreshold) * max(abs(b2.y), 0.0001f));
    const float coverage = dissolveGate * saturate(particleColor.a);

    const float3 diffuseSample = Desaturate_SourceColor(
        max(g_SourceTexture3.Sample(
            g_RuntimeMaterialV2Sampler3, diffuseUV).rgb, 0.f), b4.x);
    const float3 emissionSample = max(g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, emissionUV).rgb, 0.f);
    const float3 specularSample = Desaturate_SourceColor(
        max(g_SourceTexture4.Sample(
            g_RuntimeMaterialV2Sampler4, specularUV).rgb, 0.f), b5.y);

    const float3 sourceLightDirection = normalize(float3(0.35f, 0.55f, 0.76f));
    const float fakeLight = pow(saturate(dot(mappedNormal,
        sourceLightDirection)), max(abs(b5.w), 0.0001f)) * max(b6.x, 0.f);
    const float3 viewDirection = normalize(cameraPosition - worldPosition);
    const float3 halfDirection = normalize(sourceLightDirection + viewDirection);
    const float specularLobe = pow(saturate(dot(mappedNormal, halfDirection)),
        max(abs(b5.z) * 8.f, 0.0001f));

    const float3 diffuseRadiance = diffuseSample * max(diffuseColor.rgb, 0.f) *
        max(b4.y, 0.f) * (0.2f + fakeLight);
    const float emissionLuminance = dot(
        emissionSample, float3(0.299f, 0.587f, 0.114f));
    const float3 emissionRadiance = emissionLuminance *
        max(emissionColor.rgb, 0.f) * max(b3.y, 0.f);
    const float specularMask = pow(saturate(dot(specularSample,
        float3(0.299f, 0.587f, 0.114f))), max(abs(b5.z), 0.0001f));
    const float3 specularRadiance = max(specularColor.rgb, 0.f) *
        max(b5.x, 0.f) * specularMask * specularLobe;

    const float3 particleMagnitude = abs(particleColor.rgb);
    const float peakParticleMagnitude = max(particleMagnitude.r,
        max(particleMagnitude.g, particleMagnitude.b));
    const float3 particleModulator = peakParticleMagnitude > 0.0001f ?
        particleMagnitude : float3(1.f, 1.f, 1.f);
    float3 radiance = particleModulator *
        (diffuseRadiance + emissionRadiance + specularRadiance);
    const float peakRadiance = max(radiance.r, max(radiance.g, radiance.b));
    radiance /= 1.f + peakRadiance;

    if (!all(isfinite(float4(radiance, coverage))))
    {
        clip(-1.f);
        return output;
    }
    clip(coverage - max(g_ColorClip, 1.f / 255.f));
    output.SceneColor = float4(radiance, coverage);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

bool EffectProjectBaseCoverageEmissiveDissolveRectPacketIsValid()
{
    return g_RuntimeMaterialV2Enabled == 1u &&
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_PROJECT_BASE_COVERAGE_EMISSIVE_DISSOLVE_RECT &&
        g_RuntimeMaterialV2TextureLaneCount == 4u &&
        g_RuntimeMaterialV2TextureMask == 0x0fu &&
        g_SourceTextureMask == 0x0fu &&
        g_RuntimeMaterialV2DynamicConsumedMask == 0u &&
        g_RuntimeMaterialV2DynamicSuppressedMask == 0u &&
        g_RuntimeMaterialV2ParticleColorPolicy == 0u &&
        g_RuntimeMaterialV2ParticleColorConsumedMask == 0u &&
        g_RuntimeMaterialV2ParticleColorSuppressedMask == 0u &&
        g_RuntimeMaterialV2ScalarCount == 0u &&
        g_RuntimeMaterialV2VectorCount == 0u &&
        g_RuntimeMaterialV2InputCount == 0u &&
        all(g_RuntimeMaterialV2InputConsumedMask == uint2(0u, 0u)) &&
        all(g_RuntimeMaterialV2InputSuppressedMask == uint2(0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentConsumedMask ==
            uint3(0u, 0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentSuppressedMask ==
            uint3(0u, 0u, 0u)) &&
        g_RuntimeMaterialV2StaticInputCount == 0u &&
        g_RuntimeMaterialV2StaticSelectedMask == 0u &&
        g_RuntimeMaterialV2StaticConsumedMask == 0u &&
        g_RuntimeMaterialV2StaticSuppressedMask == 0u &&
        g_RuntimeMaterialV2RenderInputCount == 0u &&
        g_RuntimeMaterialV2RenderConsumedMask == 0u &&
        g_RuntimeMaterialV2RenderSuppressedMask == 0u;
}

EFFECT_PS_OUT Shade_EffectProjectBaseCoverageEmissiveDissolveRect(float2 uv)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (!EffectProjectBaseCoverageEmissiveDissolveRectPacketIsValid() ||
        !all(isfinite(uv)))
    {
        clip(-1.f);
        return output;
    }

    const float4 base = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, uv);
    const float coverage = saturate(g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, uv).r);
    const float3 emissive = max(g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, uv).rgb, float3(0.f, 0.f, 0.f));
    const float dissolve = saturate(g_SourceTexture3.Sample(
        g_RuntimeMaterialV2Sampler3, uv).r);
    if (!all(isfinite(base)) || !isfinite(coverage) ||
        !all(isfinite(emissive)) || !isfinite(dissolve))
    {
        clip(-1.f);
        return output;
    }

    const float4 coloredBase = base * g_ColorMultiply + g_ColorOffset;
    const float dissolveGate = step(saturate(g_DissolveAmount), dissolve);
    const float alpha = saturate(coloredBase.a) * coverage * dissolveGate;
    const float3 radiance = max(coloredBase.rgb, float3(0.f, 0.f, 0.f)) +
        emissive * max(g_EmissiveIntensity, 0.f);
    if (!all(isfinite(float4(radiance, alpha))))
    {
        clip(-1.f);
        return output;
    }

    clip(alpha - max(g_ColorClip, 1.f / 255.f));
    output.SceneColor = float4(radiance, alpha);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

bool EffectUe3WpoSinWaveElectricRt0MeshPacketIsValid()
{
    return g_RuntimeMaterialV2Enabled == 1u &&
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_WPO_SINWAVE_ELECTRIC_RT0_MESH &&
        g_RuntimeMaterialV2TextureLaneCount == 2u &&
        g_RuntimeMaterialV2TextureMask == 0x03u &&
        g_SourceTextureMask == 0x03u &&
        g_RuntimeMaterialV2DynamicConsumedMask == 0x02u &&
        g_RuntimeMaterialV2DynamicSuppressedMask == 0x0du &&
        g_RuntimeMaterialV2ParticleColorPolicy == 2u &&
        g_RuntimeMaterialV2ParticleColorConsumedMask == 0x0fu &&
        g_RuntimeMaterialV2ParticleColorSuppressedMask == 0u &&
        g_RuntimeMaterialV2ScalarCount == 10u &&
        g_RuntimeMaterialV2VectorCount == 1u &&
        g_RuntimeMaterialV2InputCount == 10u &&
        all(g_RuntimeMaterialV2InputConsumedMask == uint2(0x03ffu, 0u)) &&
        all(g_RuntimeMaterialV2InputSuppressedMask == uint2(0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentConsumedMask ==
            uint3(0x07u, 0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentSuppressedMask ==
            uint3(0x08u, 0u, 0u)) &&
        g_RuntimeMaterialV2StaticInputCount == 0u &&
        g_RuntimeMaterialV2StaticSelectedMask == 0u &&
        g_RuntimeMaterialV2StaticConsumedMask == 0u &&
        g_RuntimeMaterialV2StaticSuppressedMask == 0u &&
        g_RuntimeMaterialV2RenderInputCount == 6u &&
        g_RuntimeMaterialV2RenderConsumedMask == 0x2fu &&
        g_RuntimeMaterialV2RenderSuppressedMask == 0x10u;
}

EFFECT_PS_OUT Shade_EffectUe3WpoSinWaveElectricRt0Mesh(
    float2 localUV,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (!EffectUe3WpoSinWaveElectricRt0MeshPacketIsValid() ||
        !all(isfinite(localUV)) || !all(isfinite(particleColor)) ||
        !all(isfinite(dynamicParameter)))
    {
        clip(-1.f);
        return output;
    }

    /* PROJECT_RECONSTRUCTED RT0 base.  Source evidence resolves only
       21.map_c to the alpha family and 02.map_e to the emission family.
       Parent 06.map/12.map_f/dependency lanes and the vertex WPO equation
       remain PENDING_EVIDENCE and are deliberately absent from this packet. */
    const float4 p0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 p1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 p2 = g_RuntimeMaterialV2ScalarBlocks[2];
    const float4 emissionColor = g_RuntimeMaterialV2Vectors[0];
    if (!all(isfinite(p0)) || !all(isfinite(p1)) ||
        !all(isfinite(p2)) || !all(isfinite(emissionColor)))
    {
        clip(-1.f);
        return output;
    }

    const float alphaPower = max(abs(p0.x), 1.0e-4f);
    const float alphaStrength = max(abs(p0.y), 1.0e-4f);
    const float2 alphaUvScale = max(abs(p0.zw), float2(1.0e-4f, 1.0e-4f));
    const float emissionPower = max(abs(p1.x), 1.0e-4f);
    const float emissionDesaturation = saturate(p1.y);
    const float2 emissionUvScale = max(abs(p1.zw), float2(1.0e-4f, 1.0e-4f));
    const float2 emissionPan = p2.xy;

    const float alphaMask = saturate(g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, localUV * alphaUvScale).r);
    const float poweredMask = pow(alphaMask, alphaPower);
    const float dissolveThreshold = saturate(dynamicParameter.y);
    const float dissolveGate = saturate(
        (poweredMask - dissolveThreshold) * alphaStrength);
    const float coverage = dissolveGate * saturate(particleColor.a);

    const float3 emissionSample = max(g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1,
        localUV * emissionUvScale + emissionPan * g_EffectLocalTime).rgb, 0.f);
    const float emissionLuma = dot(
        emissionSample, float3(0.299f, 0.587f, 0.114f));
    const float3 emissionBase = lerp(
        emissionSample, emissionLuma.xxx, emissionDesaturation);
    const float3 poweredEmission = pow(
        max(emissionBase, 1.0e-6f), emissionPower);
    const float3 particleMagnitude = abs(particleColor.rgb);
    const float particlePeak = max(
        particleMagnitude.r, max(particleMagnitude.g, particleMagnitude.b));
    const float3 particleModulator = particlePeak > 1.0e-4f ?
        particleMagnitude : float3(1.f, 1.f, 1.f);
    const float3 authoredColor = max(
        (g_ColorMultiply + g_ColorOffset).rgb, float3(0.f, 0.f, 0.f));
    const float3 radiance = poweredEmission * max(emissionColor.rgb, 0.f) *
        particleModulator * authoredColor * max(g_EmissiveIntensity, 0.f);

    if (!all(isfinite(float4(radiance, coverage))))
    {
        clip(-1.f);
        return output;
    }
    output.SceneColor = float4(radiance, coverage);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

bool EffectProjectTunedBaseCoveragePacketIsValid()
{
    const bool supportedOpcode =
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_PROJECT_TUNED_BASE_COVERAGE_SRGB ||
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_PROJECT_TUNED_BASE_COVERAGE_LINEAR;
    return g_RuntimeMaterialV2Enabled == 1u && supportedOpcode &&
        g_RuntimeMaterialV2TextureLaneCount == 1u &&
        g_RuntimeMaterialV2TextureMask == 0x01u &&
        g_SourceTextureMask == 0x01u &&
        g_RuntimeMaterialV2DynamicConsumedMask == 0u &&
        g_RuntimeMaterialV2DynamicSuppressedMask == 0x0fu &&
        g_RuntimeMaterialV2ParticleColorPolicy == 2u &&
        g_RuntimeMaterialV2ParticleColorConsumedMask == 0x0fu &&
        g_RuntimeMaterialV2ParticleColorSuppressedMask == 0u &&
        g_RuntimeMaterialV2ScalarCount == 1u &&
        g_RuntimeMaterialV2VectorCount == 0u &&
        g_RuntimeMaterialV2InputCount == 1u &&
        all(g_RuntimeMaterialV2InputConsumedMask == uint2(1u, 0u)) &&
        all(g_RuntimeMaterialV2InputSuppressedMask == uint2(0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentConsumedMask ==
            uint3(0u, 0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentSuppressedMask ==
            uint3(0u, 0u, 0u)) &&
        g_RuntimeMaterialV2StaticInputCount == 0u &&
        g_RuntimeMaterialV2StaticSelectedMask == 0u &&
        g_RuntimeMaterialV2StaticConsumedMask == 0u &&
        g_RuntimeMaterialV2StaticSuppressedMask == 0u &&
        g_RuntimeMaterialV2RenderInputCount == 6u &&
        g_RuntimeMaterialV2RenderConsumedMask == 0x2fu &&
        g_RuntimeMaterialV2RenderSuppressedMask == 0x10u;
}

EFFECT_PS_OUT Shade_EffectProjectTunedBaseCoverage(
    float2 carrierUV,
    float2 carrierUVNext,
    float subUVBlend,
    float4 carrierColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (!EffectProjectTunedBaseCoveragePacketIsValid() ||
        !all(isfinite(carrierUV)) || !all(isfinite(carrierUVNext)) ||
        !isfinite(subUVBlend) || !all(isfinite(carrierColor)))
    {
        clip(-1.f);
        return output;
    }

    /* PROJECT_TUNED_APPROX is deliberately a small RT0 category, not a
       source-exact UE3 equation.  The SRGB/LINEAR opcode split owns the SRV
       interpretation while both variants share the same one-texture ABI. */
    const float4 baseCoverageCurrent = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, carrierUV);
    const float4 baseCoverageNext = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, carrierUVNext);
    const float4 baseCoverage = lerp(
        baseCoverageCurrent, baseCoverageNext, saturate(subUVBlend));
    const float coverageSelector = g_RuntimeMaterialV2ScalarBlocks[0].x;
    if (!isfinite(coverageSelector) || coverageSelector < 0.f ||
        coverageSelector > 2.f ||
        abs(coverageSelector - round(coverageSelector)) > 1.0e-5f)
    {
        clip(-1.f);
        return output;
    }
    const float sampledCoverage = coverageSelector < 0.5f ? baseCoverage.a :
        (coverageSelector < 1.5f ? baseCoverage.r :
            dot(baseCoverage.rgb, float3(0.299f, 0.587f, 0.114f)));
    const float4 combinedCarrier =
        (g_ColorMultiply + g_ColorOffset) * carrierColor;
    const float3 radiance = max(baseCoverage.rgb * combinedCarrier.rgb, 0.f) *
        max(g_EmissiveIntensity, 0.f);
    const float alpha = saturate(sampledCoverage * combinedCarrier.a);
    if (!all(isfinite(baseCoverage)) ||
        !all(isfinite(float4(radiance, alpha))))
    {
        clip(-1.f);
        return output;
    }

    output.SceneColor = float4(radiance, alpha);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

bool EffectProjectTunedWaterDropletBurstPacketIsValid()
{
    return g_RuntimeMaterialV2Enabled == 1u &&
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_PROJECT_TUNED_WATER_DROPLET_BURST &&
        g_RuntimeMaterialV2TextureLaneCount == 2u &&
        g_RuntimeMaterialV2TextureMask == 0x03u &&
        g_SourceTextureMask == 0x03u &&
        g_RuntimeMaterialV2DynamicConsumedMask == 0u &&
        g_RuntimeMaterialV2DynamicSuppressedMask == 0x0fu &&
        g_RuntimeMaterialV2ParticleColorPolicy == 2u &&
        g_RuntimeMaterialV2ParticleColorConsumedMask == 0x08u &&
        g_RuntimeMaterialV2ParticleColorSuppressedMask == 0x07u &&
        g_RuntimeMaterialV2ScalarCount == 16u &&
        g_RuntimeMaterialV2VectorCount == 2u &&
        g_RuntimeMaterialV2InputCount == 16u &&
        all(g_RuntimeMaterialV2InputConsumedMask == uint2(0xffffu, 0u)) &&
        all(g_RuntimeMaterialV2InputSuppressedMask == uint2(0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentConsumedMask ==
            uint3(0x0fu, 0x0fu, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentSuppressedMask ==
            uint3(0u, 0u, 0u)) &&
        g_RuntimeMaterialV2StaticInputCount == 0u &&
        g_RuntimeMaterialV2StaticSelectedMask == 0u &&
        g_RuntimeMaterialV2StaticConsumedMask == 0u &&
        g_RuntimeMaterialV2StaticSuppressedMask == 0u &&
        g_RuntimeMaterialV2RenderInputCount == 6u &&
        g_RuntimeMaterialV2RenderConsumedMask == 0x2fu &&
        g_RuntimeMaterialV2RenderSuppressedMask == 0x10u;
}

EFFECT_PS_OUT Shade_EffectProjectTunedWaterDropletBurst(
    float2 carrierUV,
    float4 carrierColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    const float4 p0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 p1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 p2 = g_RuntimeMaterialV2ScalarBlocks[2];
    const float4 p3 = g_RuntimeMaterialV2ScalarBlocks[3];
    const float4 bodyColor = g_RuntimeMaterialV2Vectors[0];
    const float4 rimColor = g_RuntimeMaterialV2Vectors[1];
    if (!EffectProjectTunedWaterDropletBurstPacketIsValid() ||
        !all(isfinite(carrierUV)) || !all(isfinite(carrierColor)) ||
        !all(isfinite(p0)) || !all(isfinite(p1)) || !all(isfinite(p2)) ||
        !all(isfinite(p3)) || !all(isfinite(bodyColor)) ||
        !all(isfinite(rimColor)) || !isfinite(g_EffectLocalTime))
    {
        clip(-1.f);
        return output;
    }

    const float noiseTiling = p0.x;
    const float2 noisePan = p0.yz;
    const float secondOctaveScale = p0.w;
    const float flowWarp = p1.x;
    const float maskThreshold = p1.y;
    const float edgeSoftness = p1.z;
    const float rimWidth = p1.w;
    const float coveragePower = p2.x;
    const float bodyStrength = p2.y;
    const float rimStrength = p2.z;
    const float distortionStrength = p2.w;
    const float alphaGain = p3.x;
    const float fadeStart = p3.y;
    const float fadeEnd = p3.z;
    const float cardFeather = p3.w;
    if (noiseTiling < 0.01f || noiseTiling > 16.f ||
        any(noisePan < -8.f) || any(noisePan > 8.f) ||
        secondOctaveScale < 0.5f || secondOctaveScale > 8.f ||
        flowWarp < 0.f || flowWarp > 0.25f ||
        maskThreshold < 0.f || maskThreshold > 1.f ||
        edgeSoftness < 0.1f || edgeSoftness > 8.f ||
        rimWidth < 0.001f || rimWidth > 0.5f ||
        coveragePower < 0.1f || coveragePower > 4.f ||
        bodyStrength < 0.f || bodyStrength > 8.f ||
        rimStrength < 0.f || rimStrength > 8.f ||
        abs(distortionStrength) > 0.025f ||
        alphaGain < 0.f || alphaGain > 4.f ||
        fadeStart < 0.f || fadeEnd <= fadeStart || fadeEnd > 5.f ||
        cardFeather < 0.001f || cardFeather > 0.49f ||
        any(bodyColor.rgb < 0.f) || any(bodyColor.rgb > 4.f) ||
        bodyColor.a < 0.f || bodyColor.a > 1.f ||
        any(rimColor.rgb < 0.f) || any(rimColor.rgb > 8.f) ||
        rimColor.a < 0.f || rimColor.a > 1.f)
    {
        clip(-1.f);
        return output;
    }

    /* The Valtan noise is dark-biased, so octave subtraction forms a signed
       flow field without the large negative bias produced by 2*n-1. */
    const float2 noiseUV = carrierUV * noiseTiling +
        noisePan * g_EffectLocalTime;
    const float2 noiseA = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, noiseUV).rg;
    const float2 noiseB = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0,
        noiseUV * secondOctaveScale + float2(0.37f, 0.61f)).gr;
    const float2 signedFlow = noiseA - noiseB;
    const float2 warpedUV = carrierUV + signedFlow * flowWarp;
    const float4 fluid = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, warpedUV);
    const float maskAA = max(fwidth(fluid.a) * edgeSoftness, 1.0e-4f);
    const float outer = smoothstep(
        maskThreshold - maskAA, maskThreshold + maskAA, fluid.a);
    const float inner = smoothstep(
        maskThreshold + rimWidth - maskAA,
        maskThreshold + rimWidth + maskAA, fluid.a);
    const float textureRim = saturate(outer - inner);

    const float2 centered = carrierUV * 2.f - 1.f;
    const float hemisphereZ = sqrt(saturate(1.f - dot(centered, centered)));
    const float fresnel = pow(saturate(1.f - hemisphereZ), 2.5f) * outer;
    const float rim = max(textureRim, fresnel);
    const float body = pow(saturate(outer), coveragePower);
    const float cardDistance = min(
        min(carrierUV.x, 1.f - carrierUV.x),
        min(carrierUV.y, 1.f - carrierUV.y));
    const float cardGate = smoothstep(0.f, cardFeather, cardDistance);
    const float spawnGate = smoothstep(0.f, 0.035f, g_EffectLocalTime);
    const float lifeGate = 1.f - smoothstep(
        fadeStart, fadeEnd, g_EffectLocalTime);
    const float authoredAlpha = saturate(
        (g_ColorMultiply + g_ColorOffset).a * carrierColor.a * alphaGain);
    const float coverage = saturate(
        body * cardGate * spawnGate * lifeGate * authoredAlpha);

    const float fluidLuminance = dot(
        fluid.rgb, float3(0.299f, 0.587f, 0.114f));
    float3 radiance = max(bodyColor.rgb, 0.f) * bodyColor.a *
        bodyStrength * body *
        (0.65f + 0.35f * saturate(fluidLuminance)) +
        max(rimColor.rgb, 0.f) * rimColor.a * rimStrength * rim;
    radiance *= max(g_EmissiveIntensity, 0.f) * cardGate * lifeGate;
    const float peak = max(radiance.r, max(radiance.g, radiance.b));
    radiance /= 1.f + peak * 0.35f;
    const float2 distortion = signedFlow * distortionStrength * coverage;
    if (!all(isfinite(float4(radiance, coverage))) ||
        !all(isfinite(distortion)))
    {
        clip(-1.f);
        return output;
    }

    output.SceneColor = float4(radiance, coverage);
    output.Distortion = float4(distortion, 0.f, 0.f);
    clip(output.SceneColor.a - g_ColorClip);
    return output;
}

bool EffectProjectTunedGlassMeshV1PacketIsValid()
{
    return g_RuntimeMaterialV2Enabled == 1u &&
        g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_PROJECT_TUNED_GLASS_MESH_V1 &&
        g_RuntimeMaterialV2TextureLaneCount == 1u &&
        g_RuntimeMaterialV2TextureMask == 0x01u &&
        g_SourceTextureMask == 0x01u &&
        g_RuntimeMaterialV2DynamicConsumedMask == 0u &&
        g_RuntimeMaterialV2DynamicSuppressedMask == 0x0fu &&
        g_RuntimeMaterialV2ParticleColorPolicy == 2u &&
        g_RuntimeMaterialV2ParticleColorConsumedMask == 0x08u &&
        g_RuntimeMaterialV2ParticleColorSuppressedMask == 0x07u &&
        g_RuntimeMaterialV2ScalarCount == 8u &&
        g_RuntimeMaterialV2VectorCount == 2u &&
        g_RuntimeMaterialV2InputCount == 8u &&
        all(g_RuntimeMaterialV2InputConsumedMask == uint2(0xffu, 0u)) &&
        all(g_RuntimeMaterialV2InputSuppressedMask == uint2(0u, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentConsumedMask ==
            uint3(0x0fu, 0x0fu, 0u)) &&
        all(g_RuntimeMaterialV2VectorComponentSuppressedMask ==
            uint3(0u, 0u, 0u)) &&
        g_RuntimeMaterialV2StaticInputCount == 0u &&
        g_RuntimeMaterialV2StaticSelectedMask == 0u &&
        g_RuntimeMaterialV2StaticConsumedMask == 0u &&
        g_RuntimeMaterialV2StaticSuppressedMask == 0u &&
        g_RuntimeMaterialV2RenderInputCount == 6u &&
        g_RuntimeMaterialV2RenderConsumedMask == 0x2fu &&
        g_RuntimeMaterialV2RenderSuppressedMask == 0x10u;
}

bool EffectProjectTunedGlassFinite1(float value)
{
    return (asuint(value) & 0x7f800000u) != 0x7f800000u;
}

bool EffectProjectTunedGlassFinite2(float2 value)
{
    return all((asuint(value) & 0x7f800000u) != 0x7f800000u);
}

bool EffectProjectTunedGlassFinite3(float3 value)
{
    return all((asuint(value) & 0x7f800000u) != 0x7f800000u);
}

bool EffectProjectTunedGlassFinite4(float4 value)
{
    return all((asuint(value) & 0x7f800000u) != 0x7f800000u);
}

EFFECT_PS_OUT Shade_EffectProjectTunedGlassMeshV1(
    float2 carrierUV,
    float3 worldPosition,
    float3 worldNormal,
    float2 viewNormal,
    float3 cameraPosition,
    float4 carrierColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    const float4 p0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 p1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 bodyTintLinear = g_RuntimeMaterialV2Vectors[0];
    const float4 edgeTintLinear = g_RuntimeMaterialV2Vectors[1];
    if (!EffectProjectTunedGlassMeshV1PacketIsValid() ||
        !EffectProjectTunedGlassFinite2(carrierUV) ||
        !EffectProjectTunedGlassFinite3(worldPosition) ||
        !EffectProjectTunedGlassFinite3(worldNormal) ||
        !EffectProjectTunedGlassFinite2(viewNormal) ||
        !EffectProjectTunedGlassFinite3(cameraPosition) ||
        !EffectProjectTunedGlassFinite1(carrierColor.a) ||
        !EffectProjectTunedGlassFinite4(p0) ||
        !EffectProjectTunedGlassFinite4(p1) ||
        !EffectProjectTunedGlassFinite4(bodyTintLinear) ||
        !EffectProjectTunedGlassFinite4(edgeTintLinear))
    {
        clip(-1.f);
        return output;
    }

    const float coverageGain = p0.x;
    const float bodyOpacity = p0.y;
    const float fresnelPower = p0.z;
    const float edgeGain = p0.w;
    const float crackGain = p1.x;
    const float refractionStrength = p1.y;
    const float distortionClamp = p1.z;
    const float emissionGain = p1.w;
    const bool scalarBoundsValid =
        coverageGain >= 0.f && coverageGain <= 4.f &&
        bodyOpacity >= 0.f && bodyOpacity <= 1.f &&
        fresnelPower >= 0.25f && fresnelPower <= 16.f &&
        edgeGain >= 0.f && edgeGain <= 8.f &&
        crackGain >= 0.f && crackGain <= 4.f &&
        abs(refractionStrength) <= 0.025f &&
        distortionClamp >= 0.f && distortionClamp <= 0.025f &&
        emissionGain >= 0.f && emissionGain <= 8.f;
    const bool colorBoundsValid =
        all(bodyTintLinear.rgb >= 0.f) &&
        all(bodyTintLinear.rgb <= 4.f) &&
        bodyTintLinear.a >= 0.f && bodyTintLinear.a <= 1.f &&
        all(edgeTintLinear.rgb >= 0.f) &&
        all(edgeTintLinear.rgb <= 8.f) &&
        edgeTintLinear.a >= 0.f && edgeTintLinear.a <= 1.f;
    const float normalLengthSquared = dot(worldNormal, worldNormal);
    const float3 toCamera = cameraPosition - worldPosition;
    const float viewLengthSquared = dot(toCamera, toCamera);
    if (!scalarBoundsValid || !colorBoundsValid ||
        !EffectProjectTunedGlassFinite1(normalLengthSquared) ||
        !EffectProjectTunedGlassFinite1(viewLengthSquared) ||
        normalLengthSquared <= 1.0e-8f || viewLengthSquared <= 1.0e-8f)
    {
        clip(-1.f);
        return output;
    }

    const float4 crackSample = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, carrierUV);
    if (!EffectProjectTunedGlassFinite4(crackSample))
    {
        clip(-1.f);
        return output;
    }

    const float3 normal = worldNormal * rsqrt(normalLengthSquared);
    const float3 viewDirection = toCamera * rsqrt(viewLengthSquared);
    const float fresnel = pow(
        saturate(1.f - abs(dot(normal, viewDirection))), fresnelPower);
    const float crackLuminance = saturate(dot(
        crackSample.rgb, float3(0.299f, 0.587f, 0.114f)));
    const float crackAccent = saturate(crackLuminance * crackGain);
    const float edge = saturate(fresnel * edgeGain + crackAccent);
    const float authoredAlpha = saturate(carrierColor.a);
    const float bodyCoverage = bodyOpacity * bodyTintLinear.a;
    const float edgeCoverage = edge * edgeTintLinear.a;
    const float coverage = saturate(
        max(bodyCoverage, edgeCoverage) * coverageGain * authoredAlpha);

    const float bodyShape = 0.35f + 0.65f * (1.f - crackAccent);
    const float colorWeight = bodyCoverage + edgeCoverage;
    const float3 unassociatedTint = colorWeight > 1.0e-6f ?
        (bodyTintLinear.rgb * bodyCoverage * bodyShape +
            edgeTintLinear.rgb * edgeCoverage) / colorWeight :
        float3(0.f, 0.f, 0.f);
    const float3 radiance = max(
        unassociatedTint * emissionGain *
            max(g_EmissiveIntensity, 0.f),
        float3(0.f, 0.f, 0.f));

    const float2 signedCrack = crackSample.rg * 2.f - 1.f;
    const float2 normalSignal = clamp(
        viewNormal + signedCrack * (0.25f * crackAccent), -1.f, 1.f);
    const float2 distortion = clamp(
        normalSignal * refractionStrength,
        -distortionClamp.xx, distortionClamp.xx) * coverage;
    if (!all(isfinite(float4(radiance, coverage))) ||
        !all(isfinite(distortion)))
    {
        clip(-1.f);
        return output;
    }

    output.SceneColor = float4(radiance, coverage);
    output.Distortion = float4(distortion, 0.f, 0.f);
    clip(output.SceneColor.a - g_ColorClip);
    return output;
}

#endif
