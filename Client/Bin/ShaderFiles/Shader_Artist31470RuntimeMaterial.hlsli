/*
 * Production-only Artist 31470 reconstructed material lane.
 *
 * The diagnostic evaluator is a frozen numeric oracle.  This file is kept
 * separate so production UV, mask, normal and dynamic-parameter semantics do
 * not silently change the diagnostic baseline.
 */
#include "Shader_EffectRuntimeMaterialPacket.hlsli"

static const uint RUNTIME_MATERIAL_V2_ACTIVE004_MASKED_MESH = 1u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE023_PROCEDURAL_GLOW = 2u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE009_010_INK_CORE = 3u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE016_LAYERED_SUBUV = 4u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE030_LAYERED_SUBUV = 5u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE031_WIND_SPARKLE = 6u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE022_DECAL = 7u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE011_OUTER_MESH = 8u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE003_RIBBON = 9u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE005_006_SMOKE_SQUARE = 10u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE024_NOISE_HIT = 11u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE028_FLOW_MASK = 12u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE027_RADIAL_EMISSIVE = 13u;
static const uint
    RUNTIME_MATERIAL_V2_BLOCKED_COLOROVERLIFE_RGB_SUPPRESSED_V1 = 1u;
static const uint RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1 = 2u;
static const uint RUNTIME_MATERIAL_V2_ACTIVE031_HDR_NORMALIZED_V1 = 3u;

float3 RuntimeMaterialV2_DecodeBC5Normal(float2 encodedNormal)
{
    const float2 xy = encodedNormal * 2.f - 1.f;
    return normalize(float3(xy, sqrt(saturate(1.f - dot(xy, xy)))));
}

float2 RuntimeMaterialV2_RotateUV(float2 uv, float turns)
{
    const float radians = turns * 6.28318530717958647692f;
    const float sineValue = sin(radians);
    const float cosineValue = cos(radians);
    const float2 centered = uv - float2(0.5f, 0.5f);
    return float2(
        centered.x * cosineValue - centered.y * sineValue,
        centered.x * sineValue + centered.y * cosineValue) +
        float2(0.5f, 0.5f);
}

EFFECT_PS_OUT Shade_RuntimeMaterialV2Active009010(
    float2 sourceUV,
    float3 worldPosition,
    float3 worldNormal,
    float3 cameraPosition,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 2u ||
        g_RuntimeMaterialV2TextureMask != 0x03u ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0u ||
        g_RuntimeMaterialV2ParticleColorPolicy !=
            RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1 ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
        g_RuntimeMaterialV2ScalarCount != 29u ||
        g_RuntimeMaterialV2VectorCount != 1u ||
        g_RuntimeMaterialV2InputCount != 32u ||
        any(g_RuntimeMaterialV2InputConsumedMask !=
            uint2(0xcffffff7u, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask !=
            uint2(0x30000008u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 14u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0x33ffu ||
        g_RuntimeMaterialV2StaticConsumedMask != 0x3fffu ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0u, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0x0fu, 0u, 0u)))
    {
        clip(-1.f);
        return output;
    }

    /* SOURCE_SHADERMAP_DXBC_REPLAYED_CANDIDATE.

       Blocks 0..5 are the active MIC's first 24 scalar rows.  Block 6 is
       [outalpha_falloff, disslove_hardness, depthbias, desaturation] and
       block 7.x is alpha_strength.  Source0 is original t2/maintex and
       Source1 is original t0/uv_noise_tex.  Original t1 is normal-only MRT2
       data and is not read by this recovered RT0 path.  c0.x remains an
       unresolved external opacity carrier and is held at the explicit neutral
       value one; particleColor is the recovered c1 renderer carrier and
       dynamicParameter is exact uniform vector expression c3. */
    const float4 b0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 b1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 b2 = g_RuntimeMaterialV2ScalarBlocks[2];
    const float4 b3 = g_RuntimeMaterialV2ScalarBlocks[3];
    const float4 b4 = g_RuntimeMaterialV2ScalarBlocks[4];
    const float4 b5 = g_RuntimeMaterialV2ScalarBlocks[5];
    const float4 b6 = g_RuntimeMaterialV2ScalarBlocks[6];
    const float alphaStrength = g_RuntimeMaterialV2ScalarBlocks[7].x;

    if (!all(isfinite(float4(sourceUV, g_EffectLocalTime, alphaStrength))) ||
        !all(isfinite(worldPosition)) || !all(isfinite(worldNormal)) ||
        !all(isfinite(cameraPosition)) || !all(isfinite(particleColor)) ||
        !all(isfinite(dynamicParameter)) || !all(isfinite(b0)) ||
        !all(isfinite(b1)) || !all(isfinite(b2)) || !all(isfinite(b3)) ||
        !all(isfinite(b4)) || !all(isfinite(b5)) || !all(isfinite(b6)))
    {
        clip(-1.f);
        return output;
    }

    const float mainAngle = 3.14000010490417480469f * b3.z * 0.25f;
    const float mainSine = sin(mainAngle);
    const float mainCosine = cos(mainAngle);
    const float2 centeredSource = sourceUV - float2(0.5f, 0.5f);
    const float2 rotatedSource = float2(
        dot(float2(mainCosine, -mainSine), centeredSource),
        dot(float2(mainSine, mainCosine), centeredSource)) +
        float2(0.5f, 0.5f);

    const float2 noiseUV = float2(
        rotatedSource.x * b5.y + g_EffectLocalTime * b4.w +
            dynamicParameter.y * b4.y,
        rotatedSource.y * b5.z + g_EffectLocalTime * b5.x +
            dynamicParameter.y * b4.z);
    const float4 noiseSample = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, noiseUV);

    float2 mainUV = float2(
        rotatedSource.x * b3.w + g_EffectLocalTime * b1.y + b2.z,
        rotatedSource.y * b4.x + g_EffectLocalTime * b1.z + b2.w);
    mainUV += noiseSample.rg * (dynamicParameter.w + b5.w - 1.f);
    mainUV += (dynamicParameter.x - 1.f) * float2(b3.x, b3.y);

    const float4 mainSample = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, mainUV);

    const float2 dissolvePreUV = float2(
        rotatedSource.x * b0.y,
        rotatedSource.y * b0.z + dynamicParameter.y * b0.x);
    const float2 dissolveCentered = dissolvePreUV - float2(0.5f, 0.5f);
    /* The source uniform-expression graph evaluates pi*0.25 on the CPU. */
    const float dissolveCosine = 0.00079627428211569786f;
    const float dissolveSine = 0.99999970197677612305f;
    const float2 dissolveUV = float2(
        dot(float2(dissolveCosine, -dissolveSine), dissolveCentered),
        dot(float2(dissolveSine, dissolveCosine), dissolveCentered)) +
        float2(0.5f, 0.5f);
    const float dissolveMask = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, dissolveUV).r;
    const float dissolve = saturate(
        (dissolveMask + 1.f - dynamicParameter.z) * b6.y);

    const float boundary = saturate(
        sourceUV.x * (1.f - sourceUV.x) *
        sourceUV.y * (1.f - sourceUV.y) * b6.x);
    const float3 viewVector = cameraPosition - worldPosition;
    const float viewLengthSquared = dot(viewVector, viewVector);
    const float normalLengthSquared = dot(worldNormal, worldNormal);
    float viewNormalZ = 0.f;
    if (viewLengthSquared > 1.0e-8f && normalLengthSquared > 1.0e-8f)
    {
        viewNormalZ = abs(dot(viewVector, worldNormal) * rsqrt(
            viewLengthSquared * normalLengthSquared));
    }
    const float fresnel = viewNormalZ < 1.0e-6f ? 0.f :
        pow(viewNormalZ, b1.x);
    const float alpha = saturate(fresnel * dissolve *
        (mainSample.r * particleColor.a) * boundary * alphaStrength);

    const float3 coloredMain = mainSample.rgb * particleColor.rgb;
    const float luminance = dot(coloredMain,
        float3(0.3f, 0.59f, 0.11f));
    const float3 desaturated = coloredMain +
        b6.w * (luminance.xxx - coloredMain);
    const float3 powered = pow(max(abs(desaturated),
        float3(1.0e-6f, 1.0e-6f, 1.0e-6f)), b2.x);
    const float3 rgb = b2.y * powered + b1.w * desaturated;

    if (!all(isfinite(noiseSample)) || !all(isfinite(mainSample)) ||
        !all(isfinite(float4(rgb, alpha))))
    {
        clip(-1.f);
        return output;
    }

    output.SceneColor = float4(rgb, alpha);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

EFFECT_PS_OUT Shade_RuntimeMaterialV2Mesh(
    float2 sourceUV,
    float3 worldPosition,
    float3 worldNormal,
    float3 worldTangent,
    float3 worldBinormal,
    float3 cameraPosition,
    float4 vertexColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2Opcode ==
        RUNTIME_MATERIAL_V2_ACTIVE009_010_INK_CORE)
    {
        return Shade_RuntimeMaterialV2Active009010(
            sourceUV, worldPosition, worldNormal, cameraPosition,
            vertexColor, dynamicParameter);
    }
    if (g_RuntimeMaterialV2Opcode !=
        RUNTIME_MATERIAL_V2_ACTIVE004_MASKED_MESH ||
        g_RuntimeMaterialV2TextureLaneCount != 4u ||
        (g_RuntimeMaterialV2TextureMask & 0x0fu) != 0x0fu ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x01u ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0x0eu ||
        g_RuntimeMaterialV2ScalarCount != 6u ||
        g_RuntimeMaterialV2VectorCount != 1u ||
        g_RuntimeMaterialV2InputCount != 11u ||
        any(g_RuntimeMaterialV2InputConsumedMask != uint2(0x7ffu, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask != uint2(0u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 0u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0u ||
        g_RuntimeMaterialV2StaticConsumedMask != 0u ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x3fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0u ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x0fu, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0u, 0u, 0u)))
    {
        clip(-1.f);
        return output;
    }

    const float4 diffuseSample = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, sourceUV);
    const float3 tangentNormal = RuntimeMaterialV2_DecodeBC5Normal(
        g_SourceTexture1.Sample(g_RuntimeMaterialV2Sampler1, sourceUV).rg);
    const float specularMask = g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, sourceUV).r;
    const float dissolveMask = g_SourceTexture3.Sample(
        g_RuntimeMaterialV2Sampler3, sourceUV).r;

    const float3x3 tangentToWorld = float3x3(
        normalize(worldTangent), normalize(worldBinormal),
        normalize(worldNormal));
    const float3 normal = normalize(mul(tangentNormal, tangentToWorld));
    const float3 view = normalize(cameraPosition - worldPosition);

    const float specularPower = max(g_RuntimeMaterialV2ScalarBlocks[0].x, 0.001f);
    const float specularIntensity = max(g_RuntimeMaterialV2ScalarBlocks[0].y, 0.f);
    const float dissolveLineThickness =
        max(g_RuntimeMaterialV2ScalarBlocks[0].z, 0.001f);
    const float transitionArea =
        max(g_RuntimeMaterialV2ScalarBlocks[0].w, 0.001f);
    const float emissiveIntensity = max(g_RuntimeMaterialV2ScalarBlocks[1].x, 0.f);
    const float fresnelIntensity = max(g_RuntimeMaterialV2ScalarBlocks[1].y, 0.f);
    const float opacityClip = saturate(g_RuntimeMaterialV2ScalarBlocks[1].z);
    const float4 diffuseColor = g_RuntimeMaterialV2Vectors[0];
    const float4 particleColor = max(vertexColor, 0.f);

    /* alpha is the exact ParameterDynamic channel name carried by this
       occurrence.  Values above one intentionally saturate, matching its
       source-authored fade-in/hold/fade-out envelope without inventing a new
       wall-clock curve. */
    const float dynamicAlpha = saturate(dynamicParameter.x);
    const float transition = saturate(
        (dissolveMask - (1.f - dynamicAlpha)) *
        dissolveLineThickness / transitionArea);
    /* The selected Program carries StartAlpha through an unresolved cooked
       default and currently materializes it as zero.  This opcode therefore
       assigns opacity solely to the exact named dynamic `alpha` lane plus the
       admitted diffuse/dissolve masks; particle alpha is deliberately not an
       input to this bounded reconstruction. */
    const float opacityMask = min(diffuseSample.a, transition) *
        diffuseColor.a;
    clip(opacityMask - opacityClip);

    /* The cooked graph preserved a Fresnel intensity but not its exponent.
       Schlick's power-five curve is a versioned reconstructed policy; the
       source value remains an intensity and is never relabelled as an
       exponent. */
    const float fresnel = pow(
        saturate(1.f - dot(normal, view)), 5.f) * fresnelIntensity;
    const float specular = pow(
        saturate(dot(normal, view)), specularPower) *
        specularMask * specularIntensity;
    output.SceneColor.rgb =
        diffuseSample.rgb * diffuseColor.rgb * emissiveIntensity +
        specular.xxx + fresnel.xxx * specularMask;
    output.SceneColor.a = saturate(opacityMask);
    output.SceneColor.rgb *= particleColor.rgb;
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

EFFECT_PS_OUT Shade_RuntimeMaterialV2Active016(
    float2 localUV,
    float4 subUVTransform,
    float4 subUVTransformNext,
    float subUVBlend,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 5u ||
        g_RuntimeMaterialV2TextureMask != 0x1fu ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0u ||
        g_RuntimeMaterialV2ParticleColorPolicy !=
            RUNTIME_MATERIAL_V2_BLOCKED_COLOROVERLIFE_RGB_SUPPRESSED_V1 ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x08u ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0x07u ||
        g_RuntimeMaterialV2ScalarCount != 44u ||
        g_RuntimeMaterialV2VectorCount != 3u ||
        g_RuntimeMaterialV2InputCount != 55u ||
        any(g_RuntimeMaterialV2InputConsumedMask !=
            uint2(0x3fff7fffu, 0x00001800u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask !=
            uint2(0xc0008000u, 0x007fe7ffu)) ||
        g_RuntimeMaterialV2StaticInputCount != 7u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0x7bu ||
        g_RuntimeMaterialV2StaticConsumedMask != 0x6bu ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0x14u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x07u, 0x07u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0x08u, 0x08u, 0x0fu)))
    {
        clip(-1.f);
        return output;
    }

    if (!all(isfinite(float4(localUV, g_EffectLocalTime, subUVBlend))) ||
        !all(isfinite(subUVTransform)) ||
        !all(isfinite(subUVTransformNext)) ||
        !all(isfinite(particleColor)) || !all(isfinite(dynamicParameter)) ||
        !all(isfinite(g_RuntimeMaterialV2Vectors[0])) ||
        !all(isfinite(g_RuntimeMaterialV2Vectors[1])) ||
        !all(isfinite(g_RuntimeMaterialV2Vectors[2])) ||
        abs(subUVBlend) > 0.000001f ||
        any(abs(abs(subUVTransform.xy) - float2(0.5f, 0.5f)) >
            float2(0.000001f, 0.000001f)))
    {
        clip(-1.f);
        return output;
    }

    /* Sequential scalar ABI for recipe 4070... .  Scalars 0..20 precede
       vectors 0/1 and the five selected texture rows. Scalars 21..35 are
       recipe rows 28,29,31,32,34..44; scalars 36..38 are rows 46..48; vector
       2 and scalars 39..43 are preserved-but-suppressed. */
    const float maskStrength = g_RuntimeMaterialV2ScalarBlocks[0].x;
    const float maskPower = g_RuntimeMaterialV2ScalarBlocks[0].y;
    const float2 noiseScale = g_RuntimeMaterialV2ScalarBlocks[0].zw;
    const float2 noisePan = g_RuntimeMaterialV2ScalarBlocks[1].xy;
    const float2 maskScale = g_RuntimeMaterialV2ScalarBlocks[1].zw;
    const float2 maskPan = g_RuntimeMaterialV2ScalarBlocks[2].xy;
    const float mainDesaturation = g_RuntimeMaterialV2ScalarBlocks[2].z;
    const float emissionPower = g_RuntimeMaterialV2ScalarBlocks[2].w;
    const float specStrength = g_RuntimeMaterialV2ScalarBlocks[3].x;
    const float specDesaturation = g_RuntimeMaterialV2ScalarBlocks[3].y;
    const float specPower = g_RuntimeMaterialV2ScalarBlocks[3].z;
    const float distortionStrength = g_RuntimeMaterialV2ScalarBlocks[4].x;
    const float2 baseScale = g_RuntimeMaterialV2ScalarBlocks[4].yz;
    const float2 basePan = float2(
        g_RuntimeMaterialV2ScalarBlocks[4].w,
        g_RuntimeMaterialV2ScalarBlocks[5].x);
    /* The frozen recipe packs these two scale pairs as Y then X.  Spell out
       the swizzle so equal current values cannot hide an axis-ABI reversal. */
    const float2 emissiveScale = float2(
        g_RuntimeMaterialV2ScalarBlocks[5].z,
        g_RuntimeMaterialV2ScalarBlocks[5].y);
    const float2 specScale = float2(
        g_RuntimeMaterialV2ScalarBlocks[8].w,
        g_RuntimeMaterialV2ScalarBlocks[8].z);
    const float3 emissionColor = g_RuntimeMaterialV2Vectors[0].rgb;
    const float3 specColor = g_RuntimeMaterialV2Vectors[1].rgb;

    const float panScale = dynamicParameter.y * g_EffectLocalTime;
    const float2 noiseUV = frac(localUV * noiseScale + noisePan * panScale);
    const float2 maskUV = frac(localUV * maskScale + maskPan * panScale);
    const float2 emissiveUV = frac(localUV * emissiveScale);
    const float2 specUV = frac(localUV * specScale);
    const float4 noiseSample = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, noiseUV);
    const float2 warp = clamp((noiseSample.rg * 2.f - 1.f) *
        distortionStrength * dynamicParameter.w,
        float2(-0.25f, -0.25f), float2(0.25f, 0.25f));
    const float2 baseLocalUV = frac(localUV * baseScale +
        basePan * panScale + warp);
    const float2 baseUV = baseLocalUV * subUVTransform.xy +
        subUVTransform.zw;

    const float4 emissiveSample = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, emissiveUV);
    const float4 maskSample = g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, maskUV);
    const float4 specSample = g_SourceTexture3.Sample(
        g_RuntimeMaterialV2Sampler3, specUV);
    const float4 baseSample = g_SourceTexture4.Sample(
        g_RuntimeMaterialV2Sampler4, baseUV);

    /* The selected 31.mapch.r branch owns coverage for this recipe.  The
       source DDS has an opaque container alpha, and averaging RGBA made that
       storage alpha contribute at least 0.25 coverage to every atlas texel.
       Keep storage alpha and unselected color channels out of the coverage
       ABI: only the admitted red mask channel may reveal the sprite. */
    const float selectedMask = saturate(maskSample.r);
    const float maskShape = pow(saturate(selectedMask * maskStrength),
        clamp(maskPower, 0.001f, 64.f));
    const float baseLuminance = dot(baseSample.rgb,
        float3(0.299f, 0.587f, 0.114f));
    const float3 baseShaped = lerp(baseSample.rgb, baseLuminance.xxx,
        saturate(mainDesaturation));
    const float3 noiseMod = float3(0.5f, 0.5f, 0.5f) +
        0.5f * noiseSample.rgb;
    const float specLuminance = dot(specSample.rgb,
        float3(0.299f, 0.587f, 0.114f));
    const float3 specShaped = lerp(specSample.rgb, specLuminance.xxx,
        saturate(specDesaturation));
    const float3 specTerm = pow(saturate(specShaped),
        max(specPower, 0.001f)) * max(specStrength, 0.f) * specColor;
    const float3 edge = saturate(maskShape * max(dynamicParameter.z, 0.f)) *
        emissionColor;
    /* The selected ColorOverLife RGB distribution is a null cooked default
       that the current Program materializes as zero.  The stripped material
       graph does not prove a ParticleColor RGB edge, so this bounded opcode
       suppresses that unresolved lane instead of inventing an identity
       default or multiplying all admitted texture work to black.  The
       explicit AlphaOverLife payload remains a separate opacity envelope. */
    const float3 rgb = baseShaped * noiseMod +
        emissiveSample.rgb * emissionColor * max(emissionPower, 0.f) +
        specTerm + edge;
    /* 00.use_mapa is not in the admitted active static slice.  In particular,
       the base DDS alpha is an opaque storage channel, not source coverage. */
    const float alpha = saturate(maskShape * saturate(dynamicParameter.x) *
        max(particleColor.a, 0.f));

    output.SceneColor = float4(clamp(rgb,
        float3(0.f, 0.f, 0.f), float3(16.f, 16.f, 16.f)), alpha);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

EFFECT_PS_OUT Shade_RuntimeMaterialV2Active030(
    float2 localUV,
    float4 subUVTransform,
    float4 subUVTransformNext,
    float subUVBlend,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 2u ||
        g_RuntimeMaterialV2TextureMask != 0x03u ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0u ||
        g_RuntimeMaterialV2ParticleColorPolicy !=
            RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1 ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
        g_RuntimeMaterialV2ScalarCount != 43u ||
        g_RuntimeMaterialV2VectorCount != 3u ||
        g_RuntimeMaterialV2InputCount != 54u ||
        any(g_RuntimeMaterialV2InputConsumedMask !=
            uint2(0x001c353fu, 0x00000001u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask !=
            uint2(0xffe3cac0u, 0x003ffffeu)) ||
        g_RuntimeMaterialV2StaticInputCount != 7u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0x7bu ||
        g_RuntimeMaterialV2StaticConsumedMask != 0x6bu ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0x14u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x0fu, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0u, 0x0fu, 0x0fu)))
    {
        clip(-1.f);
        return output;
    }
    if (!all(isfinite(float4(localUV, g_EffectLocalTime, subUVBlend))) ||
        !all(isfinite(subUVTransform)) ||
        !all(isfinite(subUVTransformNext)) ||
        !all(isfinite(particleColor)) || !all(isfinite(dynamicParameter)) ||
        !all(isfinite(g_RuntimeMaterialV2Vectors[0])) ||
        abs(subUVBlend) > 0.000001f ||
        any(abs(abs(subUVTransform.xy) - float2(0.5f, 0.5f)) >
            float2(0.000001f, 0.000001f)))
    {
        clip(-1.f);
        return output;
    }

    /* This is a bounded two-provider reconstruction for recipe 9c82..., not
       a recovered source graph.  Input 9 (map_f_panning_y) remains staged and
       suppressed because the frozen evaluator never consumed panAux.w. */
    const float maskStrength = g_RuntimeMaterialV2ScalarBlocks[0].x;
    const float maskPower = g_RuntimeMaterialV2ScalarBlocks[0].y;
    const float2 noiseScale = g_RuntimeMaterialV2ScalarBlocks[0].zw;
    const float2 noisePan = g_RuntimeMaterialV2ScalarBlocks[1].xy;
    const float phase = g_RuntimeMaterialV2ScalarBlocks[2].x;
    const float desaturation = g_RuntimeMaterialV2ScalarBlocks[2].z;
    const float alphaBiasGain = g_RuntimeMaterialV2ScalarBlocks[3].x;
    const float distortionStrength = g_RuntimeMaterialV2ScalarBlocks[3].y;
    const float fresnelStrength = g_RuntimeMaterialV2ScalarBlocks[6].y;
    const float4 emissionColor = g_RuntimeMaterialV2Vectors[0];

    const float panScale = dynamicParameter.y * g_EffectLocalTime;
    const float2 noiseUV = frac(localUV * noiseScale +
        noisePan * panScale + phase.xx);
    const float4 noiseSample = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, noiseUV);
    const float2 warp = clamp((noiseSample.rg * 2.f - 1.f) *
        distortionStrength * dynamicParameter.w,
        float2(-0.25f, -0.25f), float2(0.25f, 0.25f));
    const float2 baseLocalUV = frac(localUV + warp);
    const float2 baseUV = baseLocalUV * subUVTransform.xy +
        subUVTransform.zw;
    const float4 baseSample = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, baseUV);
    const float baseLuminance = dot(baseSample.rgb,
        float3(0.299f, 0.587f, 0.114f));
    const float maskShape = pow(saturate(baseLuminance * maskStrength),
        max(abs(maskPower), 0.001f));
    const float3 baseShaped = lerp(baseSample.rgb, baseLuminance.xxx,
        saturate(desaturation));
    const float3 noiseMod = 0.5f + 0.5f * noiseSample.rgb;
    const float radial = saturate(length(localUV - float2(0.5f, 0.5f)) * 2.f);
    const float3 edge = pow(radial, max(abs(maskPower), 0.001f)) *
        max(fresnelStrength, 0.f) * saturate(dynamicParameter.z);
    const float3 rgb = (baseShaped * noiseMod + edge) *
        max(emissionColor.rgb, 0.f) * max(emissionColor.a, 0.f) *
        max(particleColor.rgb, 0.f);
    const float alpha = saturate(baseSample.a * maskShape *
        max(abs(alphaBiasGain), 0.001f) * saturate(dynamicParameter.x) *
        max(particleColor.a, 0.f));
    output.SceneColor = float4(clamp(rgb,
        float3(0.f, 0.f, 0.f), float3(16.f, 16.f, 16.f)), alpha);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

EFFECT_PS_OUT Shade_RuntimeMaterialV2Active031(
    float2 localUV,
    float4 subUVTransform,
    float4 subUVTransformNext,
    float subUVBlend,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 2u ||
        g_RuntimeMaterialV2TextureMask != 0x03u ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0u ||
        g_RuntimeMaterialV2ParticleColorPolicy !=
            RUNTIME_MATERIAL_V2_ACTIVE031_HDR_NORMALIZED_V1 ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
        g_RuntimeMaterialV2ScalarCount != 3u ||
        g_RuntimeMaterialV2VectorCount != 0u ||
        g_RuntimeMaterialV2InputCount != 6u ||
        any(g_RuntimeMaterialV2InputConsumedMask != uint2(0x37u, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask != uint2(0x08u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 0u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0u ||
        g_RuntimeMaterialV2StaticConsumedMask != 0u ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0u, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0u, 0u, 0u)))
    {
        clip(-1.f);
        return output;
    }

    const float4 identitySubUV = float4(1.f, 1.f, 0.f, 0.f);
    if (!all(isfinite(float4(localUV, g_EffectLocalTime, subUVBlend))) ||
        !all(isfinite(subUVTransform)) ||
        !all(isfinite(subUVTransformNext)) ||
        !all(isfinite(particleColor)) || !all(isfinite(dynamicParameter)) ||
        abs(subUVBlend) > 0.000001f ||
        any(abs(subUVTransform - identitySubUV) >
            float4(0.000001f, 0.000001f, 0.000001f, 0.000001f)) ||
        any(abs(subUVTransformNext - identitySubUV) >
            float4(0.000001f, 0.000001f, 0.000001f, 0.000001f)))
    {
        clip(-1.f);
        return output;
    }

    /* The source recipe preserves two logical textures and three scalar
       operands but not the cooked graph edges.  This opcode is a bounded,
       versioned reconstruction: it restores textured coverage and preserves
       the authored blue hue without multiplying raw HDR [20,20,60] into a
       full rectangular carrier.  Required's 8x4 metadata is suppressed
       because this emitter has no ParticleModuleSubUV. */
    const float intensity = max(g_RuntimeMaterialV2ScalarBlocks[0].x, 0.f);
    const float tiling = max(abs(g_RuntimeMaterialV2ScalarBlocks[0].y),
        0.0001f);
    const float panning = g_RuntimeMaterialV2ScalarBlocks[0].z;
    const float transition = saturate(dynamicParameter.x);
    const float alphaPower = clamp(abs(dynamicParameter.y), 0.001f, 64.f);
    const float uvNoise01 = saturate(max(dynamicParameter.z, 0.f) * 0.5f);
    const float smokePan = clamp(dynamicParameter.w, -8.f, 8.f);
    const float panPhase = panning * smokePan * g_EffectLocalTime;

    const float2 edgeUV = frac(localUV * tiling +
        float2(panPhase, -panPhase));
    const float4 edgeSample = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, edgeUV);
    const float2 warp = clamp((edgeSample.rg * 2.f - 1.f) *
        (0.05f * uvNoise01), float2(-0.05f, -0.05f),
        float2(0.05f, 0.05f));
    const float2 sparkleUV = frac(localUV * tiling +
        float2(panPhase, 0.f) + warp);
    const float4 sparkleSample = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, sparkleUV);

    const float3 combined = saturate(sparkleSample.rgb *
        (0.5f + 0.5f * edgeSample.rgb));
    const float carrier = dot(combined,
        float3(0.299f, 0.587f, 0.114f));
    const float gate = saturate((carrier - transition) * intensity);
    const float coverage = pow(gate, alphaPower);
    const float3 positiveParticleRgb = max(particleColor.rgb, 0.f);
    const float particlePeak = max(max(positiveParticleRgb.r,
        max(positiveParticleRgb.g, positiveParticleRgb.b)), 1.f);
    const float3 particleTint = saturate(positiveParticleRgb / particlePeak);
    const float particleAlpha = saturate(max(particleColor.a, 0.f) / 50.f);

    output.SceneColor.rgb = clamp(combined * intensity * particleTint,
        float3(0.f, 0.f, 0.f), float3(16.f, 16.f, 16.f));
    output.SceneColor.a = saturate(coverage * particleAlpha);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

EFFECT_PS_OUT Shade_RuntimeMaterialV2Active027(
    float2 localUV,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 6u ||
        g_RuntimeMaterialV2TextureMask != 0x3fu ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x01u ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0x0eu ||
        g_RuntimeMaterialV2ParticleColorPolicy !=
            RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1 ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
        g_RuntimeMaterialV2ScalarCount != 4u ||
        g_RuntimeMaterialV2VectorCount != 3u ||
        g_RuntimeMaterialV2InputCount != 14u ||
        any(g_RuntimeMaterialV2InputConsumedMask != uint2(0x3fdcu, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask != uint2(0x0023u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 3u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0x06u ||
        g_RuntimeMaterialV2StaticConsumedMask != 0x07u ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x07u ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x38u ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x03u, 0x0fu, 0x0fu)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0x0cu, 0u, 0u)))
    {
        clip(-1.f);
        return output;
    }

    const float4 materialScalars = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 alphaTex02UVPos = g_RuntimeMaterialV2Vectors[0];
    const float4 outerColorAndIntensity = g_RuntimeMaterialV2Vectors[1];
    const float4 innerColorAndIntensity = g_RuntimeMaterialV2Vectors[2];
    if (!all(isfinite(localUV)) || !all(isfinite(particleColor)) ||
        !all(isfinite(dynamicParameter)) ||
        !all(isfinite(materialScalars)) ||
        !all(isfinite(alphaTex02UVPos)) ||
        !all(isfinite(outerColorAndIntensity)) ||
        !all(isfinite(innerColorAndIntensity)) ||
        any(abs(materialScalars - float4(0.5f, 5.f,
            0.1000000015f, 0.0099999998f)) > 0.00001f) ||
        any(abs(alphaTex02UVPos - float4(0.f, 0.f, 0.f, 1.f)) >
            0.00001f) ||
        any(abs(outerColorAndIntensity -
            float4(15.f, 0.1000000015f, 0.f, 1.f)) > 0.00001f) ||
        any(abs(innerColorAndIntensity -
            float4(4.f, 0.5f, 0.1000000015f, 0.8500000238f)) >
            0.00001f))
    {
        clip(-1.f);
        return output;
    }

    /* Bounded recovered alpha replay. The six staged roles are immutable:
       t0 noise RG, t1 alpha-01 G, t2 moving alpha G, t3 alpha-02 A,
       t4 alpha-02 mask G, and t5 emissive RGB. dynamic.x is the age-aligned
       dissolve carrier; dynamic.yzw are present but explicitly suppressed.
       The recovered channel/arithmetic chain and cb7 mask pan are preserved,
       but the unavailable native CB packing for perturbed p keeps this bounded. */
    const float2 noisePan = float2(frac(g_EffectLocalTime * 0.1f), 0.f);
    const float2 maskPan = float2(frac(g_EffectLocalTime * -0.25f), 0.f);
    const float2 noise = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, localUV + noisePan).rg;
    const float2 flowUV = localUV * float2(0.85f, 1.f) +
        materialScalars.x * noise;
    const float alpha01 = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, flowUV).g;
    const float alpha02 = g_SourceTexture3.Sample(
        g_RuntimeMaterialV2Sampler3, flowUV + alphaTex02UVPos.xy).a;
    const float alpha02Mask = g_SourceTexture4.Sample(
        g_RuntimeMaterialV2Sampler4, localUV + maskPan).g;
    const float shift = dynamicParameter.x * 2.8f - 1.9f;
    const float2 movingAlphaUV = float2(
        localUV.x * 1.5f + shift, localUV.y * 1.5f);
    const float movingAlpha = g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, movingAlphaUV).g;
    const float movingAlphaOffset = g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, movingAlphaUV + alpha01.xx).g;
    const float secondaryCoverage = 20.f * alpha02 * alpha02Mask *
        movingAlpha;
    const float alpha01Gate = saturate(alpha01 * materialScalars.y);
    const float coverage = saturate(saturate(
        movingAlphaOffset * alpha01Gate + secondaryCoverage) *
        particleColor.a);

    /* Bounded RGB replay: recovered radial inner/outer emissive and t5
       luminance/black-area terms are preserved before the unavailable native
       normal/spec (t6/t7), selection/fog and auxiliary MRT carriers. No
       replacement lighting or neutral texture is synthesized. */
    const float radius = length(localUV - float2(0.5f, 0.5f));
    const float innerWeight = max(1.f - radius, 0.f);
    const float3 outerColor = outerColorAndIntensity.rgb *
        outerColorAndIntensity.a;
    const float3 innerColor = innerColorAndIntensity.rgb *
        innerColorAndIntensity.a;
    const float3 radialColor = (innerWeight * innerColor +
        radius * outerColor) * materialScalars.z;
    const float3 emissiveSample = g_SourceTexture5.Sample(
        g_RuntimeMaterialV2Sampler5, flowUV).rgb;
    const float emissiveLuminance = dot(emissiveSample,
        float3(0.3f, 0.59f, 0.11f)) + materialScalars.w;

    output.SceneColor.rgb = radialColor * emissiveLuminance * particleColor.rgb;
    /* External opacity is unavailable and explicitly held at neutral one. */
    output.SceneColor.a = coverage;
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

EFFECT_PS_OUT Shade_RuntimeMaterialV2Active028(
    float2 localUV,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 5u ||
        g_RuntimeMaterialV2TextureMask != 0x1fu ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0u ||
        g_RuntimeMaterialV2ParticleColorPolicy !=
            RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1 ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
        g_RuntimeMaterialV2ScalarCount != 18u ||
        g_RuntimeMaterialV2VectorCount != 1u ||
        g_RuntimeMaterialV2InputCount != 24u ||
        any(g_RuntimeMaterialV2InputConsumedMask !=
            uint2(0x00c7ffffu, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask !=
            uint2(0x00380000u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 0u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0u ||
        g_RuntimeMaterialV2StaticConsumedMask != 0u ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x07u ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x38u ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x0fu, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0u, 0u, 0u)))
    {
        clip(-1.f);
        return output;
    }

    const float4 b0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 b1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 b2 = g_RuntimeMaterialV2ScalarBlocks[2];
    const float4 b3 = g_RuntimeMaterialV2ScalarBlocks[3];
    const float4 b4 = g_RuntimeMaterialV2ScalarBlocks[4];
    const float4 backColor = g_RuntimeMaterialV2Vectors[0];
    if (!all(isfinite(localUV)) || !all(isfinite(particleColor)) ||
        !all(isfinite(dynamicParameter)) || !all(isfinite(b0)) ||
        !all(isfinite(b1)) || !all(isfinite(b2)) ||
        !all(isfinite(b3)) || !all(isfinite(b4)) ||
        !all(isfinite(backColor)) ||
        any(abs(b0 - float4(1.f, 1.f, 1.f, 2.f)) > 0.00001f) ||
        any(abs(b1 - float4(0.8000000119f, 1.1000000238f,
            200.f, 1.f)) > 0.00001f) ||
        any(abs(b2 - float4(1.f, 20.f, 1.f, 1.f)) > 0.00001f) ||
        any(abs(b3 - float4(0.5f, 0.200000003f, 2.f, 1.f)) >
            0.00001f) ||
        any(abs(b4 - float4(0.8000000119f, 1.f, 1.f,
            0.0078125f)) > 0.00001f) ||
        any(abs(backColor - float4(0.f, 0.f, 0.f, 1.f)) > 0.00001f))
    {
        clip(-1.f);
        return output;
    }

    /* BOUNDED_EXPLICIT recovered base-PS replay for source-active-028.
       t0 is flow fx_d_noise_002 (R base/Y-gradient, G X-gradient), t1 is
       smoke mask G, t2 is opacity G, and t3/t4 are the two RGB diffuse
       roles. The recipe color_tex alias is deliberately suppressed because
       this equation never reads it. The current particle dynamic.xyzw carrier
       is sampled on the same particle-age clock by the emitter executor. */
    const float2 flowTile = float2(b1.w, b2.x);
    const float2 flowUV = localUV * flowTile;
    const float flowBase = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0, flowUV).r;
    const float flowNeighborX = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0,
        flowUV + float2(b4.w, 0.f)).g;
    const float flowNeighborY = g_SourceTexture0.Sample(
        g_RuntimeMaterialV2Sampler0,
        flowUV + float2(0.f, b4.w)).r;
    float3 flow = normalize(float3(
        -8.f * (flowNeighborY - flowBase),
        -8.f * (flowNeighborX - flowBase), 1.f));
    flow = (flow * 0.5f + 0.5f) * dynamicParameter.x;

    const float2 centered = localUV - float2(0.5f, 0.5f);
    const float radius = saturate(2.f *
        (1.f - length(centered) / max(dynamicParameter.z, 1.0e-6f)));
    flow *= radius;
    flow.xy *= float2(b3.y, 1.f);

    float mask = g_SourceTexture1.Sample(g_RuntimeMaterialV2Sampler1,
        localUV + flow.xy * float2(b3.x * b3.y, b3.x)).g;
    const float3 diffuse1 = g_SourceTexture3.Sample(
        g_RuntimeMaterialV2Sampler3,
        localUV * b0.xy + flow.xy + float2(0.f, dynamicParameter.w)).rgb;
    const float3 diffuse2 = g_SourceTexture4.Sample(
        g_RuntimeMaterialV2Sampler4,
        localUV * b0.zw + flow.xy).rgb;
    const float3 product = diffuse1 * diffuse2;
    const float luminance = dot(product, float3(0.3f, 0.59f, 0.11f));
    const float3 colorBase = lerp(product, luminance.xxx, b1.x);
    const float3 rgbCore = pow(abs(colorBase), b1.y) * b1.z +
        backColor.rgb;

    const float alphaRadius = saturate(2.f *
        (1.f - length(centered) / max(dynamicParameter.z + 0.1f,
            1.0e-6f)));
    mask *= alphaRadius;
    const float2 opacityCentered = localUV * b2.zw -
        float2(0.5f, 0.5f);
    const float2 opacityUV = opacityCentered +
        float2(0.f, dynamicParameter.y) + float2(0.5f, 0.5f) +
        b4.y * flow.xy;
    const float opacity = g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, opacityUV).g;
    const float coverage = saturate(
        pow(abs(opacity * mask), b4.x) * b2.y);

    /* Native selection/fog, auxiliary MRT carriers and the external opacity
       carrier are unavailable in this ABI. They are explicitly omitted, with
       external opacity held at the b4.z neutral one. RT1 must remain zero. */
    output.SceneColor.rgb = particleColor.rgb * radius * rgbCore;
    output.SceneColor.a = b4.z * particleColor.a * coverage;
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

EFFECT_PS_OUT Shade_RuntimeMaterialV2Particle(
    float2 localUV,
    float4 subUVTransform,
    float4 subUVTransformNext,
    float subUVBlend,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
	if (g_RuntimeMaterialV2Opcode ==
		RUNTIME_MATERIAL_V2_ACTIVE027_RADIAL_EMISSIVE)
	{
		return Shade_RuntimeMaterialV2Active027(
			localUV, particleColor, dynamicParameter);
	}
	if (g_RuntimeMaterialV2Opcode ==
		RUNTIME_MATERIAL_V2_ACTIVE028_FLOW_MASK)
    {
        return Shade_RuntimeMaterialV2Active028(
            localUV, particleColor, dynamicParameter);
    }
    if (g_RuntimeMaterialV2Opcode ==
        RUNTIME_MATERIAL_V2_ACTIVE024_NOISE_HIT)
    {
        if (g_RuntimeMaterialV2TextureLaneCount != 3u ||
            g_RuntimeMaterialV2TextureMask != 0x07u ||
            g_RuntimeMaterialV2DynamicConsumedMask != 0x03u ||
            g_RuntimeMaterialV2DynamicSuppressedMask != 0x0cu ||
            g_RuntimeMaterialV2ParticleColorPolicy !=
                RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1 ||
            g_RuntimeMaterialV2ParticleColorConsumedMask != 0x0fu ||
            g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
            g_RuntimeMaterialV2ScalarCount != 8u ||
            g_RuntimeMaterialV2VectorCount != 0u ||
            g_RuntimeMaterialV2InputCount != 29u ||
            any(g_RuntimeMaterialV2InputConsumedMask !=
                uint2(0x00000dffu, 0u)) ||
            any(g_RuntimeMaterialV2InputSuppressedMask !=
                uint2(0x1ffff200u, 0u)) ||
            g_RuntimeMaterialV2StaticInputCount != 1u ||
            g_RuntimeMaterialV2StaticSelectedMask != 0x01u ||
            g_RuntimeMaterialV2StaticConsumedMask != 0x01u ||
            g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
            g_RuntimeMaterialV2RenderInputCount != 6u ||
            g_RuntimeMaterialV2RenderConsumedMask != 0x07u ||
            g_RuntimeMaterialV2RenderSuppressedMask != 0x38u ||
            any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
                uint3(0u, 0u, 0u)) ||
            any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
                uint3(0u, 0u, 0u)))
        {
            clip(-1.f);
            return output;
        }

        const float4 noiseTransform = g_RuntimeMaterialV2ScalarBlocks[0];
        const float4 materialParameters = g_RuntimeMaterialV2ScalarBlocks[1];
        if (!all(isfinite(float4(localUV, g_EffectLocalTime, subUVBlend))) ||
            !all(isfinite(subUVTransform)) ||
            !all(isfinite(subUVTransformNext)) ||
            !all(isfinite(particleColor)) || !all(isfinite(dynamicParameter)) ||
            !all(isfinite(noiseTransform)) ||
            !all(isfinite(materialParameters)))
        {
            clip(-1.f);
            return output;
        }

        const float2 noiseUV = localUV * noiseTransform.zw +
            g_EffectLocalTime * noiseTransform.xy;
        const float2 noise = g_SourceTexture0.Sample(
            g_RuntimeMaterialV2Sampler0, noiseUV).rg;
        const float2 warpedUV = localUV + materialParameters.x * noise;
        const float scale = dynamicParameter.x * materialParameters.y;
        const float2 mainUV = (warpedUV - float2(0.5f, 0.5f)) * scale +
            float2(0.5f, 0.5f);

        /* Recovered DXBC swizzle authority: t1 is alpha_tex.R, never G/A;
           t2 is the duplicate physical noise DDS in its distinct emissive
           sampler role and contributes RGB. */
        const float alphaTex = g_SourceTexture1.Sample(
            g_RuntimeMaterialV2Sampler1, mainUV).r;
        const float3 emissiveTex = g_SourceTexture2.Sample(
            g_RuntimeMaterialV2Sampler2, mainUV).rgb;
        const float emissivePower = materialParameters.w;
        const float poweredAlpha = abs(alphaTex) < 1.0e-6f ? 0.f :
            pow(abs(alphaTex), emissivePower);
        const float coverageBase = poweredAlpha * particleColor.a;
        const float coverage = abs(coverageBase) < 1.0e-6f ? 0.f :
            pow(abs(coverageBase), dynamicParameter.y);

        const float3 lit = emissiveTex * materialParameters.z;
        const float luminance = dot(lit, float3(0.3f, 0.59f, 0.11f));
        /* emissive_desaturation is the compiled parent default zero. */
        const float3 desaturated = lerp(lit, luminance.xxx, 0.f);
        const float3 absDesaturated = abs(desaturated);
        const float3 rgb = float3(
            absDesaturated.x < 1.0e-6f ? 0.f :
                pow(absDesaturated.x, emissivePower),
            absDesaturated.y < 1.0e-6f ? 0.f :
                pow(absDesaturated.y, emissivePower),
            absDesaturated.z < 1.0e-6f ? 0.f :
                pow(absDesaturated.z, emissivePower));

        /* Native selection/fog and auxiliary MRT2/3 carriers are not exposed
           by this runtime ABI. External opacity is therefore the explicitly
           bounded no-fade neutral one; no replacement lighting is invented. */
        output.SceneColor.rgb = rgb * particleColor.rgb;
        output.SceneColor.a = min(coverage, 1.f);
        output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
        return output;
    }
    if (g_RuntimeMaterialV2Opcode ==
        RUNTIME_MATERIAL_V2_ACTIVE005_006_SMOKE_SQUARE)
    {
        if (g_RuntimeMaterialV2TextureLaneCount != 1u ||
            g_RuntimeMaterialV2TextureMask != 0x01u ||
            g_RuntimeMaterialV2DynamicConsumedMask != 0x02u ||
            g_RuntimeMaterialV2DynamicSuppressedMask != 0x0du ||
            g_RuntimeMaterialV2ParticleColorPolicy !=
                RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1 ||
            g_RuntimeMaterialV2ParticleColorConsumedMask != 0x0fu ||
            g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
            g_RuntimeMaterialV2ScalarCount != 5u ||
            g_RuntimeMaterialV2VectorCount != 0u ||
            g_RuntimeMaterialV2InputCount != 6u ||
            any(g_RuntimeMaterialV2InputConsumedMask != uint2(0x0fu, 0u)) ||
            any(g_RuntimeMaterialV2InputSuppressedMask != uint2(0x30u, 0u)) ||
            g_RuntimeMaterialV2StaticInputCount != 2u ||
            g_RuntimeMaterialV2StaticSelectedMask != 0x03u ||
            g_RuntimeMaterialV2StaticConsumedMask != 0x03u ||
            g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
            g_RuntimeMaterialV2RenderInputCount != 6u ||
            g_RuntimeMaterialV2RenderConsumedMask != 0x03u ||
            g_RuntimeMaterialV2RenderSuppressedMask != 0x3cu ||
            any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
                uint3(0u, 0u, 0u)) ||
            any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
                uint3(0u, 0u, 0u)))
        {
            clip(-1.f);
            return output;
        }

        const float lod = g_RuntimeMaterialV2ScalarBlocks[0].x;
        const float uvY = g_RuntimeMaterialV2ScalarBlocks[0].y;
        const float preservedUvX = g_RuntimeMaterialV2ScalarBlocks[0].z;
        const float shadowStrength = g_RuntimeMaterialV2ScalarBlocks[0].w;
        const float desaturation = g_RuntimeMaterialV2ScalarBlocks[1].x;
        const float externalOpacity = g_RuntimeMaterialV2ScalarBlocks[1].y;
        const float2 currentScale = subUVTransform.xy;
        const float expectedSubUvScale = 1.f / 6.f;
        if (!all(isfinite(float4(lod, uvY, preservedUvX, shadowStrength))) ||
            !all(isfinite(float4(desaturation, externalOpacity,
                subUVBlend, dynamicParameter.y))) ||
            !all(isfinite(particleColor)) ||
            !all(isfinite(subUVTransform)) ||
            !all(isfinite(subUVTransformNext)) ||
            abs(lod + 1.f) > 0.00001f || abs(uvY - 6.f) > 0.00001f ||
            abs(preservedUvX - 6.f) > 0.00001f ||
            abs(shadowStrength - 0.0299999993f) > 0.00001f ||
            abs(desaturation - 0.5f) > 0.00001f ||
            abs(externalOpacity - 1.f) > 0.00001f ||
            abs(dynamicParameter.y - 1.f) > 0.00001f ||
            any(abs(abs(currentScale) - expectedSubUvScale.xx) > 0.00001f) ||
            subUVBlend < 0.f || subUVBlend > 1.f)
        {
            clip(-1.f);
            return output;
        }

        const float2 currentUV = localUV * currentScale + subUVTransform.zw;
        const float4 smokeSample = g_SourceTexture0.SampleLevel(
            g_RuntimeMaterialV2Sampler0, currentUV, lod);
        /* Recovered PS 3e38be... samples t0 exactly once from TEXCOORD0.
           The candidate SubUV VFs share recovered no-density VS 881a765f...,
           which forwards the current atlas coordinate to that semantic; the
           cache receipt does not yet admit one exact native VF variant.
           Cascade's linear-blend setting therefore does not authorize a
           second texture sample or a color cross-fade for these occurrences.
           The CPU still resolves frame/flip and this core consumes current UV. */
        const float stripe = 1.f - frac(currentUV.y * uvY);
        const float3 raw = smokeSample.rgb + shadowStrength * stripe.xxx;
        const float luminance = dot(raw, float3(0.3f, 0.59f, 0.11f));
        const float3 smokeRgb = lerp(raw, luminance.xxx, desaturation);

        /* The recovered PS adds selection color and applies the UE3 fog MAD
           after particle modulation. Those engine carriers are not exposed by
           this reconstructed particle ABI, so the exact texture/SubUV/color
           core is replayed and no replacement lighting term is invented. */
        output.SceneColor.rgb = particleColor.rgb * smokeRgb;
        const float lifeCut = 1.f - saturate(dynamicParameter.y);
        const float sourceAlpha = saturate(smokeSample.a - lifeCut);
        output.SceneColor.a = externalOpacity *
            saturate(sourceAlpha * particleColor.a);
        output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
        return output;
    }
    if (g_RuntimeMaterialV2Opcode ==
        RUNTIME_MATERIAL_V2_ACTIVE031_WIND_SPARKLE)
    {
        return Shade_RuntimeMaterialV2Active031(localUV, subUVTransform,
            subUVTransformNext, subUVBlend, particleColor, dynamicParameter);
    }
    if (g_RuntimeMaterialV2Opcode ==
        RUNTIME_MATERIAL_V2_ACTIVE030_LAYERED_SUBUV)
    {
        return Shade_RuntimeMaterialV2Active030(localUV, subUVTransform,
            subUVTransformNext, subUVBlend, particleColor, dynamicParameter);
    }
    if (g_RuntimeMaterialV2Opcode ==
        RUNTIME_MATERIAL_V2_ACTIVE016_LAYERED_SUBUV)
    {
        return Shade_RuntimeMaterialV2Active016(localUV, subUVTransform,
            subUVTransformNext, subUVBlend, particleColor, dynamicParameter);
    }
    if (g_RuntimeMaterialV2Opcode !=
        RUNTIME_MATERIAL_V2_ACTIVE023_PROCEDURAL_GLOW ||
        g_RuntimeMaterialV2TextureLaneCount != 0u ||
        g_RuntimeMaterialV2TextureMask != 0u ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x04u ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0x0bu ||
        g_RuntimeMaterialV2ScalarCount != 5u ||
        g_RuntimeMaterialV2VectorCount != 0u ||
        g_RuntimeMaterialV2InputCount != 5u ||
        any(g_RuntimeMaterialV2InputConsumedMask != uint2(0x1eu, 0u)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask != uint2(0x01u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 6u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0x3fu ||
        g_RuntimeMaterialV2StaticConsumedMask != 0x3fu ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0u, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0u, 0u, 0u)))
    {
        clip(-1.f);
        return output;
    }

    const float radius = g_RuntimeMaterialV2ScalarBlocks[0].x;
    const float spherePower = g_RuntimeMaterialV2ScalarBlocks[0].y;
    const float sphereStrength = g_RuntimeMaterialV2ScalarBlocks[0].z;
    const float fresnelPower = g_RuntimeMaterialV2ScalarBlocks[0].w;
    const float preservedDepthAlphaBias =
        g_RuntimeMaterialV2ScalarBlocks[1].x;
    if (!all(isfinite(float4(radius, spherePower,
        sphereStrength, fresnelPower))) ||
        !isfinite(preservedDepthAlphaBias) ||
        !all(isfinite(dynamicParameter)) ||
        !all(isfinite(particleColor)))
    {
        clip(-1.f);
        return output;
    }

    /* active023's cooked parent graph lost its sphere/twirl equations.  This
       opcode is therefore a bounded reconstructed visibility policy, not a
       source-exact material graph: the exact radius/power/strength inputs and
       exact named `uv_sphery[1-x]` dynamic lane shape a textureless radial
       carrier.  depth alpha bias, y-pan, UV distortion and twirl remain
       explicitly staged-but-suppressed until their equations are recovered. */
    const float normalizedRadius = length(localUV - float2(0.5f, 0.5f)) /
        max(radius, 0.0001f);
    const float carrier = saturate(1.f - normalizedRadius);
    const float center = pow(carrier, max(spherePower, 0.0001f)) *
        max(sphereStrength, 0.f);
    const float rim = pow(saturate(normalizedRadius),
        max(fresnelPower, 0.0001f)) * carrier;
    const float sphereEnvelope = saturate(dynamicParameter.z);
    const float shape = saturate(center + rim) * sphereEnvelope;

    /* BS_EffectAdditive already multiplies SceneColor.rgb by SrcAlpha.  Keep
       the radial carrier in alpha only so the same envelope is not squared
       once in the shader and a second time by the blend state. */
    output.SceneColor.rgb = max(particleColor.rgb, 0.f);
    output.SceneColor.a = saturate(max(particleColor.a, 0.f) * shape);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}
