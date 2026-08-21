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
static const uint RUNTIME_MATERIAL_V2_UE3_FLUID01_SPRITE_W_FD_01_3 = 17u;
static const uint
    RUNTIME_MATERIAL_V2_UE3_RIBBONLIQUID01_PARENT_DEFAULT = 20u;

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

/* Opcodes 16 and 17 are reserved for the bounded Glasshole and Fluid family
   slices.  Artist D uses its own additive SpriteWave child contract. */
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

#endif
