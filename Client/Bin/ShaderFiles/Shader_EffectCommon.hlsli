#include "Engine_Shader_Defines.hlsli"

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;
Texture2D g_Base2Texture;
Texture2D g_Mask2Texture;
Texture2D g_Noise2Texture;
Texture2D g_SourceTexture0;
Texture2D g_SourceTexture1;
Texture2D g_SourceTexture2;
Texture2D g_SourceTexture3;
Texture2D g_SourceTexture4;
Texture2D g_SourceTexture5;
Texture2D g_SourceTexture6;

sampler LinearClampUSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = WRAP;
};

sampler LinearClampVSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = CLAMP;
};

sampler LinearClampUVSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

float2 g_UVScale = float2(1.f, 1.f);
float2 g_UVOffset = float2(0.f, 0.f);
float4 g_ColorOffset = float4(0.f, 0.f, 0.f, 0.f);
float4 g_ColorMultiply = float4(1.f, 1.f, 1.f, 1.f);
float4 g_AuthoredColorMultiply = float4(1.f, 1.f, 1.f, 1.f);
float g_ColorClip = 0.f;
float g_EmissiveIntensity = 1.f;
float g_DistortionIntensity = 0.f;
float g_RadialTime = 0.f;
float g_RadialIntensity = 0.f;
float g_DissolveAmount = 0.f;
uint g_DistortionOnBaseMaterial = 0;
uint g_HasNoise = 0;
uint g_HasMask = 0;
uint g_HasEmissive = 0;
uint g_HasDissolve = 0;
uint g_HasBase2 = 0;
uint g_HasMask2 = 0;
uint g_HasNoise2 = 0;
uint g_SourceMaterialProfile = 0;
float4 g_SourceScalars0 = float4(0.f, 0.f, 0.f, 0.f);
float4 g_SourceScalars1 = float4(0.f, 0.f, 0.f, 0.f);
float4 g_SourceVector0 = float4(0.f, 0.f, 0.f, 0.f);
float4 g_SourceVector1 = float4(0.f, 0.f, 0.f, 0.f);
float4 g_TypedTrailParameters[8];
uint g_SourceTextureMask = 0;
float4 g_LinearFlowParameters[16];
float4 g_LinearFlowMaskAColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_LinearFlowMaskBColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_BlacklineParameters[16];
float4 g_BlacklineDiffuseColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_BlacklineMaskColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_LocalCrackParameters[5];
float4 g_LocalCrackOutColor = float4(0.1f, 0.1f, 0.1f, 1.f);
float4 g_LocalCrackInColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_LocalCrackReflectionColor = float4(1.f, 1.f, 1.f, 1.f);
uint g_SourceTextureClampUMask = 0;
uint g_SourceTextureClampVMask = 0;
float4 g_GroupedUVScalePan = float4(1.f, 1.f, 0.f, 0.f);
float4 g_GroupedAlphaEmissive = float4(1.f, 1.f, 1.f, 1.f);
float4 g_GroupedNoiseDissolve = float4(0.f, 0.f, 0.f, 1.f);
float4 g_GroupedTint = float4(1.f, 1.f, 1.f, 1.f);
uint g_GroupedMaterialFlags = 0;
float g_EffectLocalTime = 0.f;
uint4 g_DynamicParameterSemantics = uint4(0, 0, 0, 0);
uint g_RingFillEnabled = 0u;
uint g_RingFillDirection = 0u;
uint g_RingFillInvert = 0u;
float g_RingFillProgress = 1.f;
float g_RingFillFeather = 0.f;
uint g_LinearRevealEnabled = 0u;
uint g_LinearRevealAxis = 1u;
uint g_LinearRevealInvert = 1u;
float g_LinearRevealStartSeconds = 0.f;
float g_LinearRevealDurationSeconds = 0.55f;
float g_LinearRevealEdgeWidth = 0.045f;
float g_LinearRevealSoftness = 0.03f;
float4 g_LinearRevealEdgeColor = float4(1.f, 1.f, 1.f, 1.f);
float g_LinearRevealEdgeEmissive = 7.f;

#define DEFINE_SOURCE_TEXTURE_SAMPLE(index) \
float4 Sample_SourceTexture##index(float2 uv) \
{ \
    const uint mode = \
        (((g_SourceTextureClampVMask >> index) & 1u) << 1u) | \
        ((g_SourceTextureClampUMask >> index) & 1u); \
    if (1u == mode) \
        return g_SourceTexture##index.Sample(LinearClampUSampler, uv); \
    if (2u == mode) \
        return g_SourceTexture##index.Sample(LinearClampVSampler, uv); \
    if (3u == mode) \
        return g_SourceTexture##index.Sample(LinearClampUVSampler, uv); \
    return g_SourceTexture##index.Sample(LinearSampler, uv); \
}

DEFINE_SOURCE_TEXTURE_SAMPLE(0)
DEFINE_SOURCE_TEXTURE_SAMPLE(1)
DEFINE_SOURCE_TEXTURE_SAMPLE(2)
DEFINE_SOURCE_TEXTURE_SAMPLE(3)
DEFINE_SOURCE_TEXTURE_SAMPLE(4)
DEFINE_SOURCE_TEXTURE_SAMPLE(5)
DEFINE_SOURCE_TEXTURE_SAMPLE(6)

float3 Desaturate_SourceColor(float3 color, float amount)
{
    const float luminance = dot(color, float3(0.299f, 0.587f, 0.114f));
    return lerp(color, luminance.xxx, saturate(amount));
}

struct EFFECT_PS_OUT
{
    float4 SceneColor : SV_TARGET0;
    float4 Distortion : SV_TARGET1;
};

EFFECT_PS_OUT Apply_GenericMeshRingFill(
    EFFECT_PS_OUT output,
    float2 rawCarrierUV)
{
    if (0u == g_RingFillEnabled)
        return output;

    const float progress = saturate(g_RingFillProgress);
    float coverage = 0.f;
    if (progress >= 1.f)
    {
        coverage = 1.f;
    }
    else if (progress > 0.f)
    {
        float radialV = saturate(rawCarrierUV.y);
        if (0u != g_RingFillInvert)
            radialV = 1.f - radialV;
        if (0u != g_RingFillDirection)
            radialV = 1.f - radialV;

        const float feather = clamp(abs(g_RingFillFeather), 0.f, 0.5f);
        coverage = feather > 0.f ?
            1.f - smoothstep(
                progress - feather, progress + feather, radialV) :
            (radialV <= progress ? 1.f : 0.f);
    }

    // Alpha/Additive effect passes both use SrcAlpha. Multiplying RGB here too
    // would apply the feather twice in the framebuffer blend.
    output.SceneColor.a *= coverage;
    output.Distortion.xy *= coverage;
    clip(coverage - (1.f / 255.f));
    return output;
}

EFFECT_PS_OUT Apply_GenericLinearReveal(
    EFFECT_PS_OUT output,
    float2 rawCarrierUV)
{
    if (0u == g_LinearRevealEnabled)
        return output;

    float axisCoordinate = 0u == g_LinearRevealAxis ?
        rawCarrierUV.x : rawCarrierUV.y;
    if (0u != g_LinearRevealInvert)
        axisCoordinate = 1.f - axisCoordinate;
    axisCoordinate = saturate(axisCoordinate);

    const float elapsed = g_EffectLocalTime - g_LinearRevealStartSeconds;
    if (elapsed < 0.f)
    {
        output.SceneColor.a = 0.f;
        output.Distortion.xy = float2(0.f, 0.f);
        clip(-1.f);
        return output;
    }

    const float progress = saturate(
        elapsed / max(g_LinearRevealDurationSeconds, 0.0001f));
    if (progress >= 1.f)
        return output;

    const float softness = clamp(g_LinearRevealSoftness, 0.f, 0.25f);
    const float visibleCoverage = softness > 0.f ?
        1.f - smoothstep(
            progress - softness, progress + softness, axisCoordinate) :
        (axisCoordinate <= progress ? 1.f : 0.f);
    const float edgeWidth = clamp(g_LinearRevealEdgeWidth, 0.f, 0.5f);
    const float edgeDistance = abs(axisCoordinate - progress);
    const float edgeBand = softness > 0.f ?
        1.f - smoothstep(edgeWidth, edgeWidth + softness, edgeDistance) :
        (edgeDistance <= edgeWidth ? 1.f : 0.f);
    const float edgeCoverage = edgeBand * saturate(g_LinearRevealEdgeColor.a);
    const float carrierAlpha = saturate(output.SceneColor.a);

	output.SceneColor.rgb += max(g_LinearRevealEdgeColor.rgb, 0.f) *
		max(g_LinearRevealEdgeEmissive, 0.f) * edgeBand *
		saturate(g_LinearRevealEdgeColor.a);
    output.SceneColor.a = carrierAlpha * max(visibleCoverage, edgeCoverage);
    output.Distortion.xy *= visibleCoverage;
    clip(output.SceneColor.a - max(g_ColorClip, 1.f / 255.f));
    return output;
}

float2 Apply_Radial_UV(float2 uv)
{
    const float2 centered = uv - float2(0.5f, 0.5f);
    const float radius = length(centered);
    const float2 direction = radius > 0.00001f ? centered / radius : float2(0.f, 0.f);
    const float wave = sin((radius - g_RadialTime) * 6.28318530718f);
    return uv + direction * wave * g_RadialIntensity * 0.01f;
}

EFFECT_PS_OUT Shade_Effect(
    float2 sourceUV,
    float3 lighting,
    float4 vertexColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    const float2 uv = Apply_Radial_UV(sourceUV);
    float4 noiseSample = 0 != g_HasNoise ?
        g_NoiseTexture.Sample(LinearSampler, uv) : float4(0.f, 0.f, 0.f, 0.f);
    /* The original materials that carry a second uv_noise map add its
       displacement to the first one rather than replacing it. */
    if (0 != g_HasNoise2)
    {
        const float4 noise2Sample = g_Noise2Texture.Sample(LinearSampler, uv);
        noiseSample = 0 != g_HasNoise ?
            saturate((noiseSample + noise2Sample) * 0.5f) : noise2Sample;
    }
    const float surfaceWarp = (0 != g_HasNoise || 0 != g_HasNoise2) ?
        clamp(g_DistortionIntensity * 0.01f, -0.25f, 0.25f) : 0.f;
    const float2 surfaceUV = uv +
        (noiseSample.rg * 2.f - 1.f) * surfaceWarp;
    float4 base = g_BaseTexture.Sample(LinearSampler, surfaceUV);
    /* diff_tex2 is a second diffuse layer over diff_tex1 in the source
       materials, so it modulates rather than overwrites. */
    if (0 != g_HasBase2)
        base *= g_Base2Texture.Sample(LinearSampler, surfaceUV);
    float mask = 0 != g_HasMask ?
        g_MaskTexture.Sample(LinearSampler, surfaceUV).r : 1.f;
    if (0 != g_HasMask2)
        mask *= g_Mask2Texture.Sample(LinearSampler, surfaceUV).r;

    if (0 != g_HasDissolve)
    {
        const float dissolve =
            g_DissolveTexture.Sample(LinearSampler, surfaceUV).r +
            noiseSample.r * 0.1f;
        clip(dissolve - g_DissolveAmount);
    }

    float4 color = (base * g_ColorMultiply + g_ColorOffset) * vertexColor;
    color.rgb *= lighting;
    color.a *= mask;
    clip(color.a - g_ColorClip);

    if (0 != g_HasEmissive)
    {
        color.rgb += g_EmissiveTexture.Sample(
            LinearSampler, surfaceUV).rgb * g_EmissiveIntensity;
    }

    color.rgb = max(color.rgb, float3(0.f, 0.f, 0.f));
    color.a = saturate(color.a);
    const bool distortionSourceAvailable =
        0 != g_DistortionOnBaseMaterial || 0 != g_HasNoise || 0 != g_HasNoise2;
    const float2 distortionSource = 0 != g_DistortionOnBaseMaterial ?
        base.rg : noiseSample.rg;
    const float2 distortion = distortionSourceAvailable ?
        (distortionSource * 2.f - 1.f) * g_DistortionIntensity * color.a :
        float2(0.f, 0.f);
    output.SceneColor = color;
    output.Distortion = float4(distortion, 0.f, 0.f);
    return output;
}

void Apply_DynamicParameter(
    inout float2 uv,
    inout float opacity,
    inout float emissive,
    inout float distortion,
    inout float radialSize,
    float4 dynamicParameter)
{
    [unroll]
    for (uint channel = 0; channel < 4; ++channel)
    {
        const uint semantic = g_DynamicParameterSemantics[channel];
        const float value = dynamicParameter[channel];
        if (1 == semantic)
            opacity *= saturate(value);
        else if (2 == semantic)
            emissive *= max(value, 0.f);
        else if (3 == semantic)
            opacity *= saturate(1.f - value);
        else if (4 == semantic)
            uv.x += value;
        else if (5 == semantic)
            distortion *= max(value, 0.f);
        else if (6 == semantic)
            radialSize *= max(abs(value), 0.0001f);
    }
}

float Source_DynamicParameterValue(
    float4 dynamicParameter,
    uint semantic,
    float fallbackValue)
{
    [unroll]
    for (uint channel = 0; channel < 4; ++channel)
    {
        if (g_DynamicParameterSemantics[channel] == semantic)
            return dynamicParameter[channel];
    }
    return fallbackValue;
}

float2 Rotate_LinearFlowUV(float2 uv, float angle)
{
    const float sine = sin(angle);
    const float cosine = cos(angle);
    const float2 centered = uv - float2(0.5f, 0.5f);
    return float2(
        centered.x * cosine - centered.y * sine,
        centered.x * sine + centered.y * cosine) + float2(0.5f, 0.5f);
}

float2 Transform_LinearFlowUV(
    float2 uv,
    float2 tile,
    float2 offset,
    float2 pan,
    float rotation,
    float sizeControl)
{
    float2 result = (uv - float2(0.5f, 0.5f)) * tile /
        max(abs(sizeControl), 0.0001f) + float2(0.5f, 0.5f);
    result = Rotate_LinearFlowUV(result, rotation);
    return result + offset + pan * g_EffectLocalTime;
}

float3 Desaturate_LinearFlow(float3 color, float amount)
{
    const float luminance = dot(color, float3(0.299f, 0.587f, 0.114f));
    return lerp(color, luminance.xxx, saturate(amount));
}

EFFECT_PS_OUT Shade_GroupedTranslucent(
    float2 sourceUV,
    float2 carrierUV,
    float3 lighting,
    float4 vertexColor,
    float opacity,
    float emissive,
    float distortionScale)
{
    const uint GROUPED_HAS_ALPHA = 1u << 0u;
    const uint GROUPED_HAS_EMISSIVE = 1u << 1u;
    const uint GROUPED_HAS_NOISE = 1u << 2u;
    const uint GROUPED_HAS_DISTORTION = 1u << 3u;
    const uint GROUPED_HAS_DISSOLVE = 1u << 4u;

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    float2 uv = sourceUV * g_GroupedUVScalePan.xy +
        g_GroupedUVScalePan.zw * g_EffectLocalTime;
    float4 noiseSample = float4(0.f, 0.f, 0.f, 0.f);
    if (0 != g_HasNoise)
    {
        noiseSample = g_NoiseTexture.Sample(LinearSampler, uv);
        if (0 != (g_GroupedMaterialFlags &
            (GROUPED_HAS_NOISE | GROUPED_HAS_DISTORTION)))
        {
            const float noiseStrength = min(
                abs(g_GroupedNoiseDissolve.x) * 0.01f, 0.25f);
            uv += (noiseSample.rg * 2.f - 1.f) * noiseStrength;
        }
    }

    const float4 base = g_BaseTexture.Sample(LinearSampler, uv);
    const float4 emissiveSample = 0 != g_HasEmissive ?
        g_EmissiveTexture.Sample(LinearSampler, uv) :
        float4(0.f, 0.f, 0.f, 0.f);
    float alpha = base.a;
    if (0 != g_HasMask)
        alpha *= g_MaskTexture.Sample(LinearSampler, uv).r;
    else if (0 != g_HasEmissive)
        alpha *= saturate(dot(
            emissiveSample.rgb, float3(0.299f, 0.587f, 0.114f)));
    else
        alpha *= saturate(dot(
            base.rgb, float3(0.299f, 0.587f, 0.114f)));

    if (0 != g_HasDissolve &&
        (0 != (g_GroupedMaterialFlags & GROUPED_HAS_DISSOLVE) ||
            g_DissolveAmount > 0.f))
    {
        const float dissolve =
            g_DissolveTexture.Sample(LinearSampler, uv).r +
            noiseSample.r * 0.1f;
        const float threshold = saturate(max(
            g_DissolveAmount, g_GroupedNoiseDissolve.z));
        const float softness = 1.f / max(
            abs(g_GroupedNoiseDissolve.w), 1.f);
        alpha *= smoothstep(
            threshold - softness, threshold + softness, dissolve);
    }

    if (0 != (g_GroupedMaterialFlags & GROUPED_HAS_ALPHA))
    {
        alpha = pow(
            saturate(alpha * max(g_GroupedAlphaEmissive.x, 0.f)),
            max(abs(g_GroupedAlphaEmissive.y), 0.01f));
    }

    // Feather only the carrier-local border; sampled UVs may pan or wrap freely.
    const float edgeDistance = min(
        min(carrierUV.x, 1.f - carrierUV.x),
        min(carrierUV.y, 1.f - carrierUV.y));
    alpha *= saturate(edgeDistance * 128.f) * opacity;

    float3 materialColor = base.rgb;
    if (0 != g_HasEmissive)
        materialColor += emissiveSample.rgb;
    const float emissivePower = max(
        abs(g_GroupedAlphaEmissive.w), 0.01f);
    const float emissiveStrength = max(
        g_GroupedAlphaEmissive.z, 0.f) * emissive;
    materialColor = pow(saturate(materialColor), emissivePower) *
        emissiveStrength;

    const float4 color =
        (g_ColorMultiply + g_ColorOffset) * vertexColor;
    output.SceneColor.rgb = materialColor * g_GroupedTint.rgb *
        color.rgb * lighting * g_EmissiveIntensity;
    output.SceneColor.a = saturate(alpha * color.a);

    if (0 != g_HasNoise &&
        0 != (g_GroupedMaterialFlags & GROUPED_HAS_DISTORTION))
    {
        const float groupedDistortion = max(
            abs(g_GroupedNoiseDissolve.y) * 0.01f,
            max(g_DistortionIntensity, 0.f));
        output.Distortion.xy = (noiseSample.rg * 2.f - 1.f) *
            groupedDistortion * distortionScale * output.SceneColor.a;
    }

    clip(output.SceneColor.a - max(g_ColorClip, 1.f / 255.f));
    return output;
}

EFFECT_PS_OUT Shade_EffectParticleUV(
    float2 sourceUV,
    float2 localUV,
    float3 lighting,
    float4 vertexColor,
    float4 dynamicParameter)
{
    float2 uv = sourceUV;
    float opacity = 1.f;
    float emissive = 1.f;
    float distortionScale = 1.f;
    float radialSize = 1.f;
    Apply_DynamicParameter(
        uv, opacity, emissive, distortionScale, radialSize,
        dynamicParameter);

    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;

    if (0 == g_SourceMaterialProfile)
    {
        output = Shade_Effect(uv, lighting, vertexColor);
        output.SceneColor.rgb *= emissive;
        output.SceneColor.a *= opacity;
        output.Distortion.xy *= distortionScale;
        clip(output.SceneColor.a - g_ColorClip);
        return output;
    }

    if (6 == g_SourceMaterialProfile)
    {
        return Shade_GroupedTranslucent(
            uv, localUV, lighting, vertexColor, opacity, emissive,
            distortionScale);
    }

    const float2 profileUV = 4 == g_SourceMaterialProfile ? localUV : uv;
    const float2 centered = (profileUV - float2(0.5f, 0.5f)) /
        max(radialSize, 0.0001f);
    const float radius = length(centered) * 2.f;
    float shape = 1.f;

    if (1 == g_SourceMaterialProfile)
    {
        const float spherePower = max(g_SourceScalars0.y, 1.f);
        shape = pow(saturate(1.f - radius), spherePower);
    }
    else if (2 == g_SourceMaterialProfile)
    {
        shape = pow(saturate(1.f - radius), 8.f);
    }
    else if (3 == g_SourceMaterialProfile)
    {
        const float thickness = max(abs(g_SourceScalars0.y), 0.01f);
        const float hardness = max(abs(g_SourceScalars0.z), 1.f);
        const float2 noiseOffset =
            (g_NoiseTexture.Sample(LinearSampler, uv).rg * 2.f - 1.f) * 0.02f;
        const float4 ringTexture = g_BaseTexture.Sample(
            LinearSampler, uv + noiseOffset);
        const float ring = 1.f - abs(radius - 0.65f) / thickness;
        shape = pow(saturate(ring), hardness) * ringTexture.a;
        output.SceneColor.rgb = ringTexture.rgb;
    }
    else if (4 == g_SourceMaterialProfile)
    {
        // The source aura pairs a local radial glow with an 8x4 cloud atlas.
        // Keep both UV domains: sourceUV selects the SubUV cloud frame while
        // localUV preserves the full 0..1 glow carrier.
        const float4 cloud = g_NoiseTexture.Sample(LinearSampler, uv);
        const float cloudMask = dot(
            cloud.rgb, float3(0.299f, 0.587f, 0.114f));
        const float2 auraUV = localUV +
            (cloud.rg * 2.f - 1.f) * 0.03f;
        const float4 aura = g_BaseTexture.Sample(LinearSampler, auraUV);
        const float auraMask = dot(
            aura.rgb, float3(0.299f, 0.587f, 0.114f));
        const float radialFeather = pow(
            saturate(1.f - radius), 0.5f);
        shape = saturate(auraMask * 2.f) *
            saturate(cloudMask * 2.f) * radialFeather;
        output.SceneColor.rgb = aura.rgb * cloud.rgb * 2.f;
    }
    else if (5 == g_SourceMaterialProfile)
    {
        const float2 noiseTiling = max(
            abs(float2(g_SourceScalars0.z, g_SourceScalars1.x)),
            float2(0.01f, 0.01f));
        const float2 noiseUV = uv * noiseTiling +
            float2(0.f, g_SourceScalars0.w * g_EffectLocalTime);
        const float4 noise = g_NoiseTexture.Sample(LinearSampler, noiseUV);
        const float noiseStrength =
            min(abs(g_SourceScalars0.y), 1.f);
        const float2 carrierUV = uv +
            (noise.rg * 2.f - 1.f) * noiseStrength * 0.05f;
        const float4 carrier = g_BaseTexture.Sample(
            LinearSampler, carrierUV);
        const float carrierMask = max(carrier.a, dot(
            carrier.rgb, float3(0.299f, 0.587f, 0.114f)));
        const float edgeDistance = min(
            min(uv.x, 1.f - uv.x), min(uv.y, 1.f - uv.y));
        shape = saturate(carrierMask * 2.f) *
            saturate(edgeDistance * 64.f);
        output.SceneColor.rgb = carrier.rgb;
        // The cooked one-layer recipe carries a signed -40 scalar while the
        // renderer color field is zero. Preserve that typed sign and use the
        // same percent-to-runtime scale as the other reconstructed distortion
        // profiles instead of silently producing a zero vector.
        const float sourceDistortion = clamp(
            g_SourceScalars0.x * 0.01f, -0.5f, 0.5f);
        output.Distortion.xy = (noise.rg * 2.f - 1.f) *
            sourceDistortion * distortionScale;
    }
    else if (7 == g_SourceMaterialProfile)
    {
        const float4 shineBase = g_BaseTexture.Sample(LinearSampler, uv);
        const float4 shineNoise = 0 != g_HasNoise ?
            g_NoiseTexture.Sample(LinearSampler, uv) :
            float4(0.f, 0.f, 0.f, 0.f);
        const float shineMask = 0 != g_HasMask ?
            g_MaskTexture.Sample(LinearSampler, uv).r :
            saturate(dot(shineBase.rgb,
                float3(0.299f, 0.587f, 0.114f)));
        const float2 shineCentered =
            (uv - float2(0.5f, 0.5f)) * float2(1.f, 2.f);
        const float shineFeather = pow(
            saturate(1.f - length(shineCentered) * 2.f),
            max(abs(g_SourceScalars0.y), 1.f));
        const float noisePower = max(abs(g_SourceScalars0.w), 0.01f);
        const float noiseStrength = min(
            abs(g_SourceScalars0.z) * 0.05f, 0.5f);
        shape = pow(saturate(shineBase.a * shineMask), noisePower) *
            shineFeather * max(abs(g_SourceScalars0.x), 0.01f);
        output.SceneColor.rgb =
            (shineBase.rgb + shineNoise.rgb * noiseStrength) *
            max(g_SourceVector0.rgb, float3(0.01f, 0.01f, 0.01f));
    }
    else if (8 == g_SourceMaterialProfile)
    {
        // Bounded reconstruction of fx_j_pa_willowrend_blackline_01_03_add.
        // The cooked parent lost Static Switch defaults and graph edges, so
        // this executor remains explicitly non-exact and consumes only named
        // source textures and MI parameters.
        clip(g_SourceTextureMask == 0x1fu ? 1.f : -1.f);

        const float maskAPan = Source_DynamicParameterValue(
            dynamicParameter, 11u, 0.f);
        const float flowStrength = Source_DynamicParameterValue(
            dynamicParameter, 12u, 1.f);
        const float maskBPan = Source_DynamicParameterValue(
            dynamicParameter, 13u, 0.f);
        const float diffusePan = Source_DynamicParameterValue(
            dynamicParameter, 14u, 0.f);

        const float2 flow01UV = Transform_LinearFlowUV(
            uv, g_BlacklineParameters[9].xy, float2(0.f, 0.f),
            g_BlacklineParameters[9].zw, 0.f, 1.f);
        const float2 flow02UV = Transform_LinearFlowUV(
            uv, g_BlacklineParameters[10].yz, float2(0.f, 0.f),
            float2(g_BlacklineParameters[10].w,
                g_BlacklineParameters[11].x), 0.f, 1.f);
        const float2 flowVector =
            ((Sample_SourceTexture1(flow01UV).rg * 2.f - 1.f) *
                g_BlacklineParameters[10].x +
             (Sample_SourceTexture1(flow02UV).ba * 2.f - 1.f) *
                g_BlacklineParameters[11].y) * flowStrength;

        const float2 diffuseUV = Transform_LinearFlowUV(
            uv, g_BlacklineParameters[0].xy,
            g_BlacklineParameters[0].zw,
            float2(0.f, 0.f), g_BlacklineParameters[1].z, 1.f) +
            g_BlacklineParameters[1].xy * diffusePan +
            flowVector * g_BlacklineParameters[1].w * 0.01f;
        float3 diffuse = Sample_SourceTexture0(diffuseUV).rgb;
        diffuse = Desaturate_LinearFlow(
            diffuse, g_BlacklineParameters[2].z);
        diffuse = pow(saturate(diffuse),
            max(abs(g_BlacklineParameters[2].y), 0.01f)) *
            max(g_BlacklineParameters[2].x, 0.f);

        const float2 maskAUV = Transform_LinearFlowUV(
            uv, g_BlacklineParameters[3].xy,
            g_BlacklineParameters[3].zw,
            float2(0.f, 0.f), g_BlacklineParameters[4].z, 1.f) +
            g_BlacklineParameters[4].xy * maskAPan +
            flowVector * g_BlacklineParameters[4].w * 0.01f;
        float maskA = pow(saturate(Sample_SourceTexture2(maskAUV).r),
            max(abs(g_BlacklineParameters[5].y), 0.01f)) *
            max(g_BlacklineParameters[5].x, 0.f);

        const float2 maskBUV = Transform_LinearFlowUV(
            uv, g_BlacklineParameters[6].xy,
            g_BlacklineParameters[6].zw,
            g_BlacklineParameters[7].xy,
            g_BlacklineParameters[8].x, 1.f) +
            g_BlacklineParameters[7].zw * maskBPan +
            flowVector * g_BlacklineParameters[8].y * 0.01f;
        float maskB = pow(saturate(Sample_SourceTexture3(maskBUV).r),
            max(abs(g_BlacklineParameters[8].w), 0.01f)) *
            max(g_BlacklineParameters[8].z, 0.f);

        const float2 dissolveUV = Transform_LinearFlowUV(
            uv, g_BlacklineParameters[11].zw, float2(0.f, 0.f),
            g_BlacklineParameters[12].xy, 0.f, 1.f);
        const float dissolveValue = Sample_SourceTexture4(dissolveUV).r;
        const float dissolveFeather = rcp(
            max(abs(g_BlacklineParameters[12].z), 1.f));
        const float dissolveGate = smoothstep(
            saturate(g_DissolveAmount) - dissolveFeather,
            saturate(g_DissolveAmount) + dissolveFeather,
            dissolveValue);
        const float radialCarrier = lerp(
            g_BlacklineParameters[13].z,
            g_BlacklineParameters[13].w,
            pow(saturate(1.f - radius *
                max(g_BlacklineParameters[5].w, 0.f)),
                max(abs(g_BlacklineParameters[5].z), 0.01f)));

        shape = saturate((maskA + maskB) * radialCarrier) * dissolveGate;
        output.SceneColor.rgb = pow(max(
            diffuse * g_BlacklineDiffuseColor.rgb +
            (maskA + maskB) * g_BlacklineMaskColor.rgb,
            float3(0.f, 0.f, 0.f)),
            max(abs(g_BlacklineParameters[13].y), 0.01f)) *
            max(g_BlacklineParameters[13].x, 0.f);
    }
    else if (9 == g_SourceMaterialProfile)
    {
        clip(g_SourceTextureMask == 0x7u ? 1.f : -1.f);
        const float2 dissolveUV = uv * g_LocalCrackParameters[0].xy +
            g_LocalCrackParameters[0].zw * g_EffectLocalTime;
        const float crackDissolve = Sample_SourceTexture2(dissolveUV).r;
        const float dynamicDissolve = Source_DynamicParameterValue(
            dynamicParameter, 3u, 1.f - saturate(g_DissolveAmount));
        const float crackThreshold = max(
            saturate(g_DissolveAmount), saturate(1.f - dynamicDissolve));
        const float crackSoftness = rcp(
            max(abs(g_LocalCrackParameters[1].x), 1.f));
        shape = smoothstep(
            crackThreshold - crackSoftness,
            crackThreshold + crackSoftness,
            crackDissolve + g_LocalCrackParameters[1].y);
        output.SceneColor.rgb = lerp(
            g_LocalCrackOutColor.rgb, g_LocalCrackInColor.rgb,
            crackDissolve);
    }
    else if (10 == g_SourceMaterialProfile)
    {
        const float centeredGlow = pow(
            saturate(1.f - radius),
            max(abs(g_SourceScalars0.y), 1.f)) *
            max(abs(g_SourceScalars0.x), 0.01f);
        const float centerMask = pow(
            saturate(1.f - radius),
            max(abs(g_SourceScalars0.w), 1.f)) *
            min(max(abs(g_SourceScalars0.z), 0.01f), 16.f) * 0.1f;
        const float outerGlow = pow(
            saturate(1.f - radius),
            max(abs(g_SourceScalars1.y), 1.f)) *
            max(abs(g_SourceScalars1.x), 0.01f);
        shape = saturate(
            (centeredGlow + outerGlow) * saturate(centerMask));
    }
    else if (11 == g_SourceMaterialProfile)
    {
        // Bounded reconstruction of fx_j_pa_linearflow_02_tr.  Texture names,
        // groups, MI values, and timing are source-authored; the cooked parent
        // no longer contains enough expression edges to claim graph exactness.
        clip(g_SourceTextureMask == 0x7fu ? 1.f : -1.f);

        const float2 diffuseUV = Transform_LinearFlowUV(
            uv, g_LinearFlowParameters[0].xy,
            g_LinearFlowParameters[1].xy,
            g_LinearFlowParameters[0].zw,
            g_LinearFlowParameters[1].z, 1.f);
        const float2 diffuseNoiseUV =
            uv * g_LinearFlowParameters[2].xy;
        const float4 diffuseNoise = Sample_SourceTexture1(diffuseNoiseUV);
        const float2 warpedDiffuseUV = diffuseUV +
            (diffuseNoise.rg * 2.f - 1.f) *
            g_LinearFlowParameters[1].w * 0.01f;
        float3 diffuse = Sample_SourceTexture0(warpedDiffuseUV).rgb;
        diffuse = Desaturate_LinearFlow(
            diffuse, g_LinearFlowParameters[15].z);
        diffuse = pow(saturate(diffuse),
            max(abs(g_LinearFlowParameters[11].y), 0.01f)) *
            max(g_LinearFlowParameters[11].x, 0.f);

        float2 maskAUV = Transform_LinearFlowUV(
            uv, g_LinearFlowParameters[3].xy,
            g_LinearFlowParameters[4].xy,
            g_LinearFlowParameters[3].zw,
            g_LinearFlowParameters[4].z,
            g_LinearFlowParameters[4].w);
        maskAUV.x += Source_DynamicParameterValue(
            dynamicParameter, 7u, 0.f);
        const float2 noiseAUV = Transform_LinearFlowUV(
            uv, g_LinearFlowParameters[5].xy,
            g_LinearFlowParameters[6].xy,
            g_LinearFlowParameters[5].zw, 0.f, 1.f);
        const float2 maskAWarp =
            (Sample_SourceTexture3(noiseAUV).rg * 2.f - 1.f) *
            g_LinearFlowParameters[6].z * 0.01f *
            Source_DynamicParameterValue(dynamicParameter, 9u, 1.f);
        float maskA = Sample_SourceTexture2(maskAUV + maskAWarp).r;
        maskA = pow(saturate(maskA),
            max(abs(g_LinearFlowParameters[11].w), 0.01f)) *
            max(g_LinearFlowParameters[11].z, 0.f);

        float2 maskBUV = Transform_LinearFlowUV(
            uv, g_LinearFlowParameters[7].xy,
            g_LinearFlowParameters[8].xy,
            g_LinearFlowParameters[7].zw,
            g_LinearFlowParameters[8].z,
            g_LinearFlowParameters[8].w);
        maskBUV.x += Source_DynamicParameterValue(
            dynamicParameter, 8u, 0.f);
        const float2 noiseBUV = Transform_LinearFlowUV(
            uv, g_LinearFlowParameters[9].xy,
            g_LinearFlowParameters[10].xy,
            g_LinearFlowParameters[9].zw, 0.f, 1.f);
        const float2 maskBWarp =
            (Sample_SourceTexture5(noiseBUV).rg * 2.f - 1.f) *
            g_LinearFlowParameters[10].z * 0.01f *
            Source_DynamicParameterValue(dynamicParameter, 10u, 1.f);
        const float maskBRaw = saturate(
            Sample_SourceTexture4(maskBUV + maskBWarp).r);
        float maskB = maskBRaw;
        maskB = pow(saturate(maskB),
            max(abs(g_LinearFlowParameters[12].y), 0.01f)) *
            max(g_LinearFlowParameters[12].x, 0.f);

        const float2 dissolveUV = uv * g_LinearFlowParameters[14].xy +
            g_LinearFlowParameters[14].zw * g_EffectLocalTime;
        const float dissolveValue = Sample_SourceTexture6(
            Rotate_LinearFlowUV(dissolveUV, g_SourceScalars0.x)).r;
        const float dissolveFeather = rcp(
            max(abs(g_SourceScalars0.y), 1.f));
        const float dissolveGate = smoothstep(
            saturate(g_DissolveAmount) - dissolveFeather,
            saturate(g_DissolveAmount) + dissolveFeather,
            dissolveValue);

        const float positiveAWeight = dot(
            max(g_LinearFlowMaskAColor.rgb, float3(0.f, 0.f, 0.f)),
            float3(0.299f, 0.587f, 0.114f));
        const float negativeBWeight = maskBRaw * dot(
            max(-g_LinearFlowMaskBColor.rgb, float3(0.f, 0.f, 0.f)),
            float3(0.299f, 0.587f, 0.114f));
        const float sourceMaskCarrier = saturate(
            maskA * positiveAWeight - negativeBWeight);
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        const float edgeFeather = saturate(edgeDistance * 64.f);

        // The negative B mask is a subtractive cutout, while A is the visible
        // carrier.  Keep that signed order and let ParticleColor own final
        // chroma; the former radial heuristic made their valid supports
        // disjoint and collapsed this exact parent family to zero output.
        shape = pow(saturate(sourceMaskCarrier *
            max(g_LinearFlowParameters[12].z, 0.f)),
            max(abs(g_LinearFlowParameters[12].w), 0.01f)) *
            dissolveGate * edgeFeather;
        output.SceneColor.rgb = diffuse * sourceMaskCarrier;
        output.Distortion.xy =
            (diffuseNoise.rg * 2.f - 1.f) *
            max(g_SourceScalars0.w, 0.f) * shape * distortionScale;
    }
    else if (12 == g_SourceMaterialProfile)
    {
        // Bounded reconstruction of fx_j_pa_slice_01_tr.  The source carrier
        // is an opaque Voronoi texture, so its DDS alpha cannot define the
        // translucent blade; the authored opacity radius owns that envelope.
        const float2 sliceUV = Rotate_LinearFlowUV(
            uv, g_SourceScalars0.x);
        float2 flowUV =
            (sliceUV - float2(0.5f, 0.5f)) * g_SourceScalars1.xy +
            float2(0.5f, 0.5f) + g_SourceScalars1.zw;
        flowUV = Rotate_LinearFlowUV(flowUV, g_SourceVector0.x);
        const float4 flow = g_BaseTexture.Sample(LinearSampler, flowUV);
        const float flowStrength = Source_DynamicParameterValue(
            dynamicParameter, 12u, 1.f);
        const float2 warped = sliceUV - float2(0.5f, 0.5f) +
            (flow.rg * 2.f - 1.f) * g_SourceScalars0.z *
            flowStrength * 0.02f;
        const float bladeWidth = pow(saturate(
            1.f - abs(warped.y) *
                max(abs(g_SourceScalars0.y), 0.01f)), 2.f);
        const float bladeLength = pow(
            saturate(1.f - abs(warped.x) * 2.f), 0.5f);
        const float flowMask = dot(
            flow.rgb, float3(0.299f, 0.587f, 0.114f));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = bladeWidth * bladeLength * saturate(flowMask * 1.5f) *
            saturate(edgeDistance * 64.f);
        // Coverage is applied once through SceneColor.a below. Premultiplying
        // RGB here makes SrcAlpha blending apply the blade envelope twice and
        // can erase the thin slash after Source Trim is reduced.
        output.SceneColor.rgb = flow.rgb;
        output.Distortion.xy = (flow.rg * 2.f - 1.f) *
            min(abs(g_SourceScalars0.w) * 0.001f, 0.5f) *
            shape * distortionScale;
    }
    else if (13 == g_SourceMaterialProfile)
    {
        // Bounded reconstruction of the legacy four-lane missile trail.  Its
        // decoded DDS inputs have opaque alpha, so named R/G carriers and the
        // dissolve curve define the ribbon instead of DDS alpha.
        clip(0 != g_HasMask && 0 != g_HasNoise && 0 != g_HasDissolve ?
            1.f : -1.f);
        const float alphaPan = Source_DynamicParameterValue(
            dynamicParameter, 15u, 0.f);
        const float noiseStrength = min(abs(Source_DynamicParameterValue(
            dynamicParameter, 16u, 1.f)), 4.f);
        const float noisePan = Source_DynamicParameterValue(
            dynamicParameter, 17u, 0.f);
        const float dissolveAmount = saturate(Source_DynamicParameterValue(
            dynamicParameter, 18u, 0.f));
        const float2 noiseUV = uv * max(
            abs(g_TypedTrailParameters[2].xy), float2(0.01f, 0.01f)) +
            (g_TypedTrailParameters[2].zw + float2(noisePan, 0.f)) *
                g_EffectLocalTime;
        const float4 noise = g_NoiseTexture.Sample(LinearSampler, noiseUV);
        const float2 warpedUV = uv + (noise.rg * 2.f - 1.f) *
            clamp(g_TypedTrailParameters[3].x * noiseStrength * 0.01f,
                -0.1f, 0.1f);
        // The source parent exposes independent R/G U-coordinate lanes.  Its
        // `*_r_texcoord` and `*_g_texcoord` values are not a packed float2 UV:
        // each channel scales and offsets U while preserving the mesh V.
        const float2 alphaRUV = float2(
            warpedUV.x * max(abs(g_TypedTrailParameters[1].x), 0.01f) +
                g_TypedTrailParameters[1].z +
                (g_TypedTrailParameters[0].z + alphaPan) *
                    g_EffectLocalTime,
            warpedUV.y);
        const float2 alphaGUV = float2(
            warpedUV.x * max(abs(g_TypedTrailParameters[1].y), 0.01f) +
                g_TypedTrailParameters[1].w +
                (g_TypedTrailParameters[0].w + alphaPan) *
                    g_EffectLocalTime,
            warpedUV.y);
        const float alphaR = g_MaskTexture.Sample(
            LinearSampler, alphaRUV).r;
        const float alphaG = g_MaskTexture.Sample(
            LinearSampler, alphaGUV).g;
        const float alphaTexture = max(alphaR, alphaG);
        const float alphaCarrier = pow(saturate(
            alphaTexture * max(g_TypedTrailParameters[0].x, 0.f)),
            max(abs(g_TypedTrailParameters[0].y), 0.01f));
        const float2 dissolveUV = warpedUV * max(
            abs(g_TypedTrailParameters[7].zw), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[3].zw * g_EffectLocalTime;
        const float dissolveValue = g_DissolveTexture.Sample(
            LinearSampler, dissolveUV).r;
        const float dissolveSoftness = rcp(
            max(abs(g_TypedTrailParameters[3].y), 1.f));
        const float dissolveGate = smoothstep(
            dissolveAmount - dissolveSoftness,
            dissolveAmount + dissolveSoftness,
            dissolveValue);
        const float edgeDistance = min(
            min(uv.x, 1.f - uv.x), min(uv.y, 1.f - uv.y));
        const float edgeFeather = saturate(edgeDistance * 64.f);
        shape = alphaCarrier * dissolveGate * edgeFeather;
        const float4 base = g_BaseTexture.Sample(LinearSampler, warpedUV);
        const float emissivePower = max(
            abs(g_TypedTrailParameters[7].y), 0.01f);
        const float emissiveStrength = min(
            max(g_TypedTrailParameters[7].x, 0.f) * 0.05f, 8.f);
        output.SceneColor.rgb = pow(saturate(
            base.rgb + alphaTexture.xxx), emissivePower) *
            emissiveStrength * shape;
    }
    else if (15 == g_SourceMaterialProfile)
    {
        // Exact alpha/two-emissive/dissolve/noise ordering for the missile
        // trail family.  The large cooked emissive strength is exposure
        // bounded with one shared denominator, retaining chroma instead of
        // independently saturating every channel to white.
        clip(0 != g_HasMask && 0 != g_HasNoise && 0 != g_HasEmissive &&
            0 != g_HasDissolve ? 1.f : -1.f);
        const float alphaPan = Source_DynamicParameterValue(
            dynamicParameter, 15u, 0.f);
        const float noiseStrength = Source_DynamicParameterValue(
            dynamicParameter, 16u, 1.f);
        const float noisePan = Source_DynamicParameterValue(
            dynamicParameter, 17u, 0.f);
        const float dissolveAmount = saturate(Source_DynamicParameterValue(
            dynamicParameter, 18u, 0.f));
        const float2 noiseUV = uv * max(abs(g_TypedTrailParameters[2].xy),
            float2(0.01f, 0.01f)) +
            (g_TypedTrailParameters[2].zw + float2(noisePan, 0.f)) *
                g_EffectLocalTime;
        const float4 noise = g_NoiseTexture.Sample(LinearSampler, noiseUV);
        const float2 warpedUV = uv + (noise.rg * 2.f - 1.f) *
            clamp(g_TypedTrailParameters[3].x * noiseStrength * 0.01f,
                -0.1f, 0.1f);
        const float2 alphaUV = warpedUV * max(
            abs(g_TypedTrailParameters[1].xy), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[1].zw +
            (g_TypedTrailParameters[0].zw + float2(alphaPan, 0.f)) *
                g_EffectLocalTime;
        const float alphaR = g_MaskTexture.Sample(
            LinearSampler, alphaUV).r;
        const float alphaG = g_MaskTexture.Sample(
            LinearSampler, alphaUV).g;
        const float alphaTexture = max(alphaR, alphaG);
        const float alphaCarrier = pow(saturate(
            alphaTexture * max(g_TypedTrailParameters[0].x, 0.f)),
            max(abs(g_TypedTrailParameters[0].y), 0.01f));
        const float2 dissolveUV = warpedUV * max(
            abs(g_TypedTrailParameters[7].zw), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[3].zw * g_EffectLocalTime;
        const float dissolveValue = g_DissolveTexture.Sample(
            LinearSampler, dissolveUV).r;
        const float dissolveSoftness = rcp(
            max(abs(g_TypedTrailParameters[3].y), 1.f));
        const float dissolveGate = smoothstep(
            dissolveAmount - dissolveSoftness,
            dissolveAmount + dissolveSoftness,
            dissolveValue);
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        const float edgeFeather = saturate(edgeDistance * 64.f);
        shape = alphaCarrier * dissolveGate * edgeFeather;
        const float2 emissive01UV = warpedUV * max(
            abs(g_TypedTrailParameters[4].xy), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[6].xy +
            g_TypedTrailParameters[4].zw * g_EffectLocalTime;
        const float2 emissive02UV = warpedUV * max(
            abs(g_TypedTrailParameters[5].xy), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[5].zw * g_EffectLocalTime;
        const float3 emissive01 = saturate(
            g_BaseTexture.Sample(LinearSampler, emissive01UV).rgb);
        const float3 emissive02 = saturate(
            g_EmissiveTexture.Sample(LinearSampler, emissive02UV).rgb);
        const float emissivePower = max(
            abs(g_TypedTrailParameters[7].y), 0.01f);
        const float emissiveStrength = max(
            g_TypedTrailParameters[7].x, 0.f);
        const float3 radiance = pow(
            max(emissive01 * emissive02, float3(0.f, 0.f, 0.f)),
            emissivePower) * emissiveStrength;
        const float peakRadiance = max(
            radiance.r, max(radiance.g, radiance.b));
        output.SceneColor.rgb = radiance / (1.f + peakRadiance) * shape;
    }
    else if (14 == g_SourceMaterialProfile)
    {
        // fx_h_me_watertrail_01 keeps reflection_color in a disabled
        // reflection lane.  Vertex/ParticleColor remains the final tint; the
        // dynamic dissolve channel is direct visibility, never generic
        // inverse opacity, so the source value 1 stays visible.
        clip((g_SourceTextureMask & 0x3u) == 0x3u &&
            g_TypedTrailParameters[6].y < 0.5f ?
            1.f : -1.f);
        const float alphaPan = Source_DynamicParameterValue(
            dynamicParameter, 19u, 0.f);
        const float noisePan = Source_DynamicParameterValue(
            dynamicParameter, 20u, 0.f);
        const float dissolveVisibility = saturate(
            Source_DynamicParameterValue(dynamicParameter, 21u, 1.f));
        const float dynamicNoiseStrength = Source_DynamicParameterValue(
            dynamicParameter, 22u, 0.f);
        const float2 noiseUV = uv * max(
            abs(g_TypedTrailParameters[3].xy), float2(0.01f, 0.01f)) +
            (g_TypedTrailParameters[3].zw + float2(noisePan, 0.f)) *
                g_EffectLocalTime;
        const float4 noise = Sample_SourceTexture1(noiseUV);
        const float noiseStrength = clamp(
            g_TypedTrailParameters[4].x + dynamicNoiseStrength,
            -4.f, 4.f);
        float2 mainUV = uv * max(abs(g_TypedTrailParameters[0].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[1].xy +
            (g_TypedTrailParameters[0].zw +
                g_TypedTrailParameters[1].zw + float2(0.f, alphaPan)) *
                g_EffectLocalTime;
        mainUV += (noise.rg * 2.f - 1.f) * noiseStrength * 0.02f;
        const float4 mainSample = Sample_SourceTexture0(mainUV);
        const float3 mainColor = Desaturate_SourceColor(
            max(mainSample.rgb, float3(0.f, 0.f, 0.f)),
            g_TypedTrailParameters[2].z);
        const float mainPower = max(abs(
            g_TypedTrailParameters[2].x *
            g_TypedTrailParameters[2].y), 0.01f);
        output.SceneColor.rgb = pow(saturate(mainColor), mainPower);
        const float mainCarrier = max(mainSample.a, dot(
            mainSample.rgb, float3(0.299f, 0.587f, 0.114f)));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        const float edgeFeather = saturate(
            edgeDistance * max(g_TypedTrailParameters[4].z, 1.f));
        shape = saturate(mainCarrier *
            max(g_TypedTrailParameters[4].y, 0.f)) *
            dissolveVisibility * edgeFeather;
    }
    else if (16 == g_SourceMaterialProfile || 36 == g_SourceMaterialProfile)
    {
        // fx_k_me_makeflow_02/03: opacity, two diffuse lanes, color, and flow
        // are independent named inputs. The distinct profile numbers preserve
        // parent identity while sharing this bounded translated equation.
        // diff_backcolor is a diffuse fallback, never the final tint;
        // ParticleColor remains the chroma carrier.
        clip(g_SourceTextureMask == 0x1fu ? 1.f : -1.f);
        const float flowStrength = Source_DynamicParameterValue(
            dynamicParameter, 27u, 1.f);
        const float opacityPanV = Source_DynamicParameterValue(
            dynamicParameter, 28u, 0.f);
        const float opacityPanU = Source_DynamicParameterValue(
            dynamicParameter, 29u, 0.f);
        const float2 flowUV = uv * max(abs(g_TypedTrailParameters[3].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[3].zw *
            g_EffectLocalTime;
        const float4 flow = Sample_SourceTexture4(flowUV);
        const float2 flowVector = (flow.rg * 2.f - 1.f) * clamp(
            g_TypedTrailParameters[0].w * flowStrength * 0.02f,
            -0.15f, 0.15f);
        const float2 opacityBaseUV = uv * max(
            abs(g_TypedTrailParameters[0].xy), float2(0.01f, 0.01f)) +
            float2(opacityPanU, opacityPanV) * g_EffectLocalTime;
        const float2 opacityUV = Rotate_LinearFlowUV(
            opacityBaseUV + flowVector, g_TypedTrailParameters[0].z);
        const float4 opacitySample = Sample_SourceTexture0(opacityUV);
        const float opacityTexture = dot(
            opacitySample.rgb, float3(0.299f, 0.587f, 0.114f));
        const float opacityPowered = pow(saturate(opacityTexture),
            max(abs(g_TypedTrailParameters[4].y), 0.01f));
        shape = 1.f - exp(-opacityPowered *
            max(g_TypedTrailParameters[4].x, 0.f) * 0.08f);

        const float2 diff1UV = uv * max(abs(g_TypedTrailParameters[1].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[1].zw *
            g_EffectLocalTime + flowVector;
        const float2 diff2UV = uv * max(abs(g_TypedTrailParameters[2].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[2].zw *
            g_EffectLocalTime - flowVector;
        const float3 diffuse1 = Sample_SourceTexture1(diff1UV).rgb;
        const float3 diffuse2 = Sample_SourceTexture2(diff2UV).rgb;
        const float blend = saturate(flow.r + g_TypedTrailParameters[4].z);
        float3 diffuse = lerp(diffuse1, diffuse2, blend) +
            max(g_TypedTrailParameters[7].rgb, float3(0.f, 0.f, 0.f));
        diffuse = Desaturate_SourceColor(
            diffuse, g_TypedTrailParameters[5].z);
        diffuse = pow(saturate(diffuse),
            max(abs(g_TypedTrailParameters[5].y), 0.01f)) *
            max(g_TypedTrailParameters[5].x, 0.f);
        const float3 colorTexture = Desaturate_SourceColor(
            Sample_SourceTexture3(uv).rgb, g_TypedTrailParameters[6].z);
        const float3 radiance = pow(saturate(diffuse * colorTexture),
            max(abs(g_TypedTrailParameters[6].y), 0.01f)) *
            max(g_TypedTrailParameters[6].x, 0.f);
        const float peakRadiance = max(
            radiance.r, max(radiance.g, radiance.b));
        output.SceneColor.rgb = radiance / (1.f + peakRadiance);
        output.Distortion.xy = (flow.rg * 2.f - 1.f) *
            max(g_TypedTrailParameters[4].w, 0.f) * 0.02f * shape *
            distortionScale;
    }
    else if (17 == g_SourceMaterialProfile)
    {
        // fx_n_pa_ring_05: the DDS alpha is fully opaque. Its RGB emission
        // carrier plus the local radial hole define coverage, preventing the
        // former opaque black quad produced by generic alpha sampling.
        clip(g_SourceTextureMask == 0x3u ? 1.f : -1.f);
        const float2 noiseUV = uv * max(
            abs(g_TypedTrailParameters[1].xx), float2(0.01f, 0.01f)) +
            float2(0.f, g_TypedTrailParameters[1].y * g_EffectLocalTime);
        const float4 noise = Sample_SourceTexture1(noiseUV);
        const float2 ringUV = uv * max(abs(g_TypedTrailParameters[0].xy),
            float2(0.01f, 0.01f)) + (noise.rg * 2.f - 1.f) *
            clamp(g_TypedTrailParameters[1].z * 0.02f, -0.1f, 0.1f);
        const float4 ringSample = Sample_SourceTexture0(ringUV);
        const float ringTexture = dot(
            ringSample.rgb, float3(0.299f, 0.587f, 0.114f));
        const float hole = pow(saturate(radius),
            max(abs(g_TypedTrailParameters[3].x), 0.01f));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        const float edgeFeather = saturate(edgeDistance * 64.f);
        shape = pow(saturate(ringTexture *
            max(g_TypedTrailParameters[2].x, 0.f)),
            max(abs(g_TypedTrailParameters[2].y), 0.01f)) *
            hole * edgeFeather;
        const float3 radiance = ringSample.rgb *
            max(g_TypedTrailParameters[2].z, 0.f);
        const float peakRadiance = max(
            radiance.r, max(radiance.g, radiance.b));
        output.SceneColor.rgb = radiance / (1.f + peakRadiance);
    }
    else if (18 == g_SourceMaterialProfile)
    {
        // fx_m_pa_trail_01: both alpha maps and UV noise are source-evidenced
        // linear DDS lanes. Their RGB channels, not opaque DDS alpha, own the
        // sprite carrier. ParticleColor owns final chroma.
        clip(g_SourceTextureMask == 0x7u ? 1.f : -1.f);
        const float alphaPan = Source_DynamicParameterValue(
            dynamicParameter, 23u, 0.f);
        const float alphaLerp = saturate(Source_DynamicParameterValue(
            dynamicParameter, 24u, 0.5f));
        const float noisePan = Source_DynamicParameterValue(
            dynamicParameter, 25u, 0.f);
        const float noiseStrength = Source_DynamicParameterValue(
            dynamicParameter, 26u, 0.f);
        const float2 noiseUV = uv * max(
            abs(g_TypedTrailParameters[3].xy), float2(0.01f, 0.01f)) +
            float2(noisePan, 0.f) * g_EffectLocalTime;
        const float4 noise = Sample_SourceTexture2(noiseUV);
        const float2 warp = (noise.rg * 2.f - 1.f) * clamp(
            (g_TypedTrailParameters[1].w + noiseStrength) * 0.02f,
            -0.12f, 0.12f);
        float2 alpha01UV = uv * max(abs(g_TypedTrailParameters[0].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[1].xy +
            float2(alphaPan, 0.f) * g_EffectLocalTime + warp;
        alpha01UV = Rotate_LinearFlowUV(
            alpha01UV, g_TypedTrailParameters[1].z);
        const float2 alpha02UV = uv * max(
            abs(g_TypedTrailParameters[0].zw), float2(0.01f, 0.01f)) - warp;
        const float4 alpha01 = Sample_SourceTexture0(alpha01UV);
        const float4 alpha02 = Sample_SourceTexture1(alpha02UV);
        const float carrier01 = max(alpha01.r, alpha01.g);
        const float carrier02 = max(alpha02.r, alpha02.g);
        const float combined = lerp(carrier01, carrier01 * carrier02,
            alphaLerp) + max(g_TypedTrailParameters[2].z, 0.f) * 0.01f;
        const float powered = pow(saturate(combined),
            max(abs(g_TypedTrailParameters[2].y), 0.01f));
        const float exposed = 1.f - exp(-powered *
            max(g_TypedTrailParameters[2].x, 0.f) * 0.001f);
        const float holeStart = saturate(
            g_TypedTrailParameters[3].z * 0.5f);
        const float hole = smoothstep(holeStart,
            max(holeStart + 0.05f, g_TypedTrailParameters[3].z), radius);
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = exposed * hole * saturate(edgeDistance * 64.f);
        output.SceneColor.rgb = combined.xxx;
        output.Distortion.xy = (noise.rg * 2.f - 1.f) *
            max(g_TypedTrailParameters[2].w, 0.f) * 0.01f * shape *
            distortionScale;
    }
    else if (19 == g_SourceMaterialProfile)
    {
        // fx_d_pa_master_01.  C owns coverage, D is a UV warp, and E/F are
        // emission.  A/B are alpha-group lanes, but their exact non-expanding
        // operator is not recovered; this bounded approximation ignores them.
        // The Artist-F oracle likewise evaluates C as the mask, so dense A/B
        // noise must never expand C into a filled billboard.
        clip(0u != (g_SourceTextureMask & 0x1u) &&
            0u != (g_SourceTextureMask & 0x30u) ? 1.f : -1.f);
        const float alphaVisibility = saturate(Source_DynamicParameterValue(
            dynamicParameter, 31u, 1.f));
        const float dynamicPan = Source_DynamicParameterValue(
            dynamicParameter, 32u, 0.f);
        const float dynamicEdge = Source_DynamicParameterValue(
            dynamicParameter, 33u, 1.f);
        const float dynamicDistortion = Source_DynamicParameterValue(
            dynamicParameter, 34u, 0.f);

        float2 warpedUV = uv;
        float2 noiseVector = float2(0.f, 0.f);
        if (0u != (g_SourceTextureMask & 0x8u))
        {
            const float2 noiseUV = uv * max(
                abs(g_TypedTrailParameters[3].xy),
                float2(0.01f, 0.01f)) +
                (g_TypedTrailParameters[3].zw +
                    float2(dynamicPan, 0.f)) * g_EffectLocalTime;
            noiseVector = Sample_SourceTexture3(noiseUV).rg * 2.f - 1.f;
            warpedUV += noiseVector * clamp(
                (g_TypedTrailParameters[4].x + dynamicDistortion) * 0.01f,
                -0.12f, 0.12f);
        }

        const float2 carrierUV = warpedUV * max(
            abs(g_TypedTrailParameters[0].xy),
            float2(0.01f, 0.01f));
        const float3 sampleC = Sample_SourceTexture0(carrierUV).rgb;
        const float alphaCarrier = dot(sampleC,
            float3(0.299f, 0.587f, 0.114f));
        const float poweredCarrier = pow(saturate(alphaCarrier),
            max(abs(g_TypedTrailParameters[0].w), 0.01f));
        const float exposedCarrier = 1.f - exp(-poweredCarrier *
            max(g_TypedTrailParameters[0].z, 0.f) * 0.05f);
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = exposedCarrier * alphaVisibility * saturate(
            edgeDistance * lerp(32.f, 128.f, saturate(abs(dynamicEdge))));

        float3 emission = float3(0.f, 0.f, 0.f);
        if (0u != (g_SourceTextureMask & 0x10u))
        {
            const float2 emissionUV = warpedUV * max(
                abs(g_TypedTrailParameters[5].xy),
                float2(0.01f, 0.01f)) +
                g_TypedTrailParameters[5].zw * g_EffectLocalTime;
            emission += Sample_SourceTexture4(emissionUV).rgb;
        }
        if (0u != (g_SourceTextureMask & 0x20u))
        {
            const float2 emissionUV = warpedUV * max(
                abs(g_TypedTrailParameters[6].xy),
                float2(0.01f, 0.01f)) +
                g_TypedTrailParameters[6].zw * g_EffectLocalTime;
            emission += Sample_SourceTexture5(emissionUV).rgb;
        }
        emission = Desaturate_SourceColor(max(emission,
            float3(0.f, 0.f, 0.f)), g_TypedTrailParameters[4].z);
        const float3 radiance = pow(saturate(emission),
            max(abs(g_TypedTrailParameters[4].y), 0.01f)) *
            max(g_TypedTrailParameters[7].rgb, float3(0.f, 0.f, 0.f)) *
            max(g_SourceVector0.rgb, float3(0.f, 0.f, 0.f));
        output.SceneColor.rgb = radiance;
        output.Distortion.xy = noiseVector * clamp(
            g_TypedTrailParameters[4].x + dynamicDistortion,
            -4.f, 4.f) * 0.01f * shape * distortionScale;
    }
    else if (20 == g_SourceMaterialProfile)
    {
        // fx_m_pa_spritewave_01.  MainTex owns coverage, UV noise only warps
        // it, dissolve textures only gate it, and EmissiveTex02 owns light.
        clip((g_SourceTextureMask & 0x13u) == 0x13u ? 1.f : -1.f);
        const float dynamicMainPan = Source_DynamicParameterValue(
            dynamicParameter, 35u, 0.f);
        const float dynamicDissolve = Source_DynamicParameterValue(
            dynamicParameter, 36u, 0.f);
        const float dynamicNoiseStrength = Source_DynamicParameterValue(
            dynamicParameter, 37u, 0.f);
        const float dynamicNoisePan = Source_DynamicParameterValue(
            dynamicParameter, 38u, 0.f);
        const float2 noiseUV = uv * max(
            abs(g_TypedTrailParameters[2].xy), float2(0.01f, 0.01f)) +
            (g_TypedTrailParameters[2].zw +
                float2(dynamicNoisePan, 0.f)) * g_EffectLocalTime;
        const float4 noise = Sample_SourceTexture1(noiseUV);
        const float noiseStrength = clamp(
            g_TypedTrailParameters[3].x + dynamicNoiseStrength,
            -4.f, 4.f);
        float2 mainUV = uv * max(abs(g_TypedTrailParameters[0].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[1].xy +
            (g_TypedTrailParameters[0].zw +
                float2(dynamicMainPan, 0.f)) * g_EffectLocalTime;
        mainUV = Rotate_LinearFlowUV(mainUV,
            g_TypedTrailParameters[1].z);
        mainUV += (noise.rg * 2.f - 1.f) * noiseStrength * 0.02f;
        const float4 mainSample = Sample_SourceTexture0(mainUV);
        float dissolveCarrier = 1.f;
        if (0u != (g_SourceTextureMask & 0x4u))
        {
            const float2 dissolveUV = mainUV * max(
                abs(g_TypedTrailParameters[4].xy),
                float2(0.01f, 0.01f)) +
                g_TypedTrailParameters[4].zw * g_EffectLocalTime;
            float dissolveValue = Sample_SourceTexture2(dissolveUV).r;
            if (0u != (g_SourceTextureMask & 0x8u))
            {
                dissolveValue += Sample_SourceTexture3(dissolveUV).r *
                    clamp(g_TypedTrailParameters[5].y, 0.f, 2.f) * 0.25f;
            }
            dissolveCarrier = saturate(
                dissolveValue + 1.1f - dynamicDissolve);
        }
        // Recovered Artist-F SpriteWave source equation.  MainTex owns the
        // textured carrier, dissolve owns its life gate, and the source
        // sphere mask prevents even a compressed nonzero DDS background
        // from exposing the rectangular sprite boundary.
        const float radialBase = max(
            1.f - 2.f * length(localUV - float2(0.5f, 0.5f)), 0.f) *
            g_TypedTrailParameters[5].z;
        const float radial = min(max(radialBase,
            g_TypedTrailParameters[5].w), g_TypedTrailParameters[7].w);
        shape = radial * saturate(dissolveCarrier *
            g_TypedTrailParameters[5].x * mainSample.r *
            g_TypedTrailParameters[1].w);
        const float2 emissionUV = mainUV * max(
            abs(g_TypedTrailParameters[6].xy), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[6].zw * g_EffectLocalTime;
        const float3 emission = max(
            Sample_SourceTexture4(emissionUV).rgb,
            float3(0.f, 0.f, 0.f));
        const float3 radiance = pow(saturate(emission),
            max(abs(g_TypedTrailParameters[7].y), 0.01f)) *
            max(g_TypedTrailParameters[7].x, 0.f) +
            mainSample.rgb * max(g_TypedTrailParameters[7].z, 0.f);
        output.SceneColor.rgb = radiance *
            max(g_SourceVector0.rgb, float3(0.f, 0.f, 0.f));
        output.Distortion.xy = (noise.rg * 2.f - 1.f) * noiseStrength *
            0.01f * shape * distortionScale;
    }
    else if (21 == g_SourceMaterialProfile)
    {
        // fx_o_pa_trail_01_01 exposes only alpha02 plus UV noise.  Sampling
        // that alpha lane as the carrier is a bounded approximate evaluator;
        // it does not invent the absent alpha01 texture or change admission.
        clip((g_SourceTextureMask & 0x3u) == 0x3u ? 1.f : -1.f);
        const float alphaPan = Source_DynamicParameterValue(
            dynamicParameter, 23u, 0.f);
        const float noisePan = Source_DynamicParameterValue(
            dynamicParameter, 25u, 0.f);
        const float noiseStrength = Source_DynamicParameterValue(
            dynamicParameter, 26u, 0.f);
        const float2 noiseUV = uv * max(
            abs(g_TypedTrailParameters[3].xy), float2(0.01f, 0.01f)) +
            float2(noisePan, 0.f) * g_EffectLocalTime;
        const float4 noise = Sample_SourceTexture1(noiseUV);
        const float2 warp = (noise.rg * 2.f - 1.f) * clamp(
            (g_TypedTrailParameters[1].w + noiseStrength) * 0.02f,
            -0.12f, 0.12f);
        float2 alphaUV = uv * max(abs(g_TypedTrailParameters[0].zw),
            float2(0.01f, 0.01f)) +
            float2(alphaPan, 0.f) * g_EffectLocalTime + warp;
        const float3 alphaSample = Sample_SourceTexture0(alphaUV).rgb;
        const float carrier = max(alphaSample.r, alphaSample.g);
        const float powered = pow(saturate(carrier),
            max(abs(g_TypedTrailParameters[2].y), 0.01f));
        const float exposed = 1.f - exp(-powered *
            max(g_TypedTrailParameters[2].x, 0.f) * 0.001f);
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = exposed * saturate(edgeDistance * 64.f);
        output.SceneColor.rgb = carrier.xxx;
        output.Distortion.xy = (noise.rg * 2.f - 1.f) *
            max(g_TypedTrailParameters[2].w, 0.f) * 0.01f * shape *
            distortionScale;
    }
    else if (22 == g_SourceMaterialProfile)
    {
        // bfx_d_pa_spla_01: MapSource is coverage, MapD is distortion, and
        // the two spec maps are light.  Spec/noise RGB cannot own opacity.
        clip((g_SourceTextureMask & 0xfu) == 0xfu ? 1.f : -1.f);
        const float2 noiseUV = uv * max(
            abs(g_TypedTrailParameters[1].xy), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[1].zw * g_EffectLocalTime;
        const float4 noise = Sample_SourceTexture1(noiseUV);
        const float2 warpedUV = uv + (noise.rg * 2.f - 1.f) * clamp(
            g_TypedTrailParameters[0].z * 0.02f, -0.12f, 0.12f);
        const float3 source = Sample_SourceTexture0(warpedUV).rgb;
        const float carrier = dot(max(source, float3(0.f, 0.f, 0.f)),
            float3(0.299f, 0.587f, 0.114f));
        const float powered = pow(saturate(carrier),
            max(abs(g_TypedTrailParameters[0].y), 0.01f));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = (1.f - exp(-powered *
            max(g_TypedTrailParameters[0].x, 0.f))) *
            saturate(edgeDistance * 64.f);
        const float specTime = g_TypedTrailParameters[2].y *
            g_EffectLocalTime;
        const float2 specUV = warpedUV * max(
            abs(g_TypedTrailParameters[2].z.xx), float2(0.01f, 0.01f)) +
            float2(specTime, 0.f);
        const float3 specA = Sample_SourceTexture3(specUV).rgb;
        const float3 specB = Sample_SourceTexture2(specUV).rgb;
        const float3 specular = pow(saturate(max(specA, specB)),
            max(abs(g_TypedTrailParameters[2].x), 0.01f)) *
            min(max(g_TypedTrailParameters[0].w, 0.f), 1000.f) *
            max(g_TypedTrailParameters[4].rgb, float3(0.f, 0.f, 0.f));
        output.SceneColor.rgb = (source + specular) *
            max(g_TypedTrailParameters[3].rgb, float3(0.f, 0.f, 0.f));
        output.Distortion.xy = (noise.rg * 2.f - 1.f) *
            g_TypedTrailParameters[0].z * 0.01f * shape * distortionScale;
    }
    else if (23 == g_SourceMaterialProfile)
    {
        // bfx_d_pa_spla_05: MapAlpha is the only coverage texture.  MapA and
        // MapD distort it, while SpecMapA contributes bounded radiance.
        clip((g_SourceTextureMask & 0xfu) == 0xfu ? 1.f : -1.f);
        const float dynamicDissolve = Source_DynamicParameterValue(
            dynamicParameter, 39u, 1.f);
        const float dynamicUvDistort = Source_DynamicParameterValue(
            dynamicParameter, 40u, 0.f);
        const float dynamicDistortion = Source_DynamicParameterValue(
            dynamicParameter, 41u, 0.f);
        const float2 mapDUV = uv * max(
            abs(g_TypedTrailParameters[2].xy), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[2].zw * g_EffectLocalTime;
        const float mapD = Sample_SourceTexture1(mapDUV).r;
        const float warpAmount = mapD * dynamicUvDistort *
            g_TypedTrailParameters[0].z;
        const float2 warpedUV = uv + warpAmount.xx;
        const float2 mapAUV = warpedUV * max(
            abs(g_TypedTrailParameters[1].xy), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[1].zw * g_EffectLocalTime;
        const float mapA = Sample_SourceTexture0(mapAUV).g;
        const float2 alphaUV = warpedUV * max(
            abs(g_TypedTrailParameters[3].xy), float2(0.01f, 0.01f));
        const float alphaTexture = Sample_SourceTexture2(alphaUV).r;

        // Ported from the recovered Artist #25/#29 base-pass equation.
        // min_alpha is an upper-side gate, never a lower opacity floor.
        // The old max(alphaTexture, 0.5) made an empty texel 86.47% opaque
        // for W's exact str=8/power=2 source values.
        const float lifeCut = 1.f - saturate(dynamicDissolve);
        const float sourceShape = saturate(
            (alphaTexture * mapA - lifeCut) *
            g_TypedTrailParameters[0].x);
        const float poweredShape = sourceShape < 0.000001f ? 0.f :
            pow(sourceShape,
                max(abs(g_TypedTrailParameters[0].y), 0.01f));
        const float alphaGate = saturate(
            g_TypedTrailParameters[3].z - alphaTexture + 1.f);
        shape = alphaGate * poweredShape;

        const float specularTime = g_EffectLocalTime *
            g_TypedTrailParameters[4].w;
        const float2 specularUV1 = warpedUV * max(
            abs(g_TypedTrailParameters[4].z), 0.01f) +
            float2(0.f, frac(specularTime * 0.1f));
        const float2 specularUV2 = warpedUV +
            float2(0.f, frac(specularTime * 0.05f));
        const float specularBase =
            Sample_SourceTexture3(specularUV2).r *
            Sample_SourceTexture3(specularUV1).g *
            g_TypedTrailParameters[4].x;
        const float specularCarrier = abs(specularBase) < 0.000001f ? 0.f :
            pow(abs(specularBase),
                max(abs(g_TypedTrailParameters[4].y), 0.01f));
        output.SceneColor.rgb = specularCarrier *
            max(g_TypedTrailParameters[6].rgb, float3(0.f, 0.f, 0.f)) +
            max(g_TypedTrailParameters[5].rgb, float3(0.f, 0.f, 0.f));
        const float2 signedMapD = Sample_SourceTexture1(mapDUV).rg * 2.f - 1.f;
        output.Distortion.xy = signedMapD * clamp(
            g_TypedTrailParameters[0].w + dynamicDistortion,
            -20.f, 20.f) * 0.0025f * shape * distortionScale;
    }
    else if (24 == g_SourceMaterialProfile)
    {
        // fx_e_pa_twinkle_01: mask owns coverage; twinkle lanes animate
        // radiance.  The optional empty additive lane is never required.
        clip((g_SourceTextureMask & 0x7u) == 0x7u ? 1.f : -1.f);
        const float density = saturate(Source_DynamicParameterValue(
            dynamicParameter, 42u, 1.f));
        const float alphaPower = max(abs(Source_DynamicParameterValue(
            dynamicParameter, 43u, 1.f)), 0.01f);
        const float emissiveTiling = max(abs(Source_DynamicParameterValue(
            dynamicParameter, 44u, 1.f)), 0.01f);
        const float lampTime = Source_DynamicParameterValue(
            dynamicParameter, 45u, 1.f);
        const float2 twinkleUV = uv * max(
            g_TypedTrailParameters[1].y * emissiveTiling, 0.01f) +
            float2(g_TypedTrailParameters[1].x * lampTime, 0.f) *
                g_EffectLocalTime;
        const float twinkleA = Sample_SourceTexture0(twinkleUV).r;
        const float twinkleB = Sample_SourceTexture1(-twinkleUV).g;
        const float mask = dot(Sample_SourceTexture2(uv).rgb,
            float3(0.299f, 0.587f, 0.114f));
        const float sparkle = pow(saturate(max(twinkleA, twinkleB) *
            density), alphaPower);
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = saturate(mask) * sparkle * saturate(edgeDistance * 64.f);
        float3 radiance = sparkle.xxx *
            max(g_TypedTrailParameters[0].x, 0.f);
        if (0u != (g_SourceTextureMask & 0x8u))
        {
            radiance += Sample_SourceTexture3(uv *
                max(g_TypedTrailParameters[0].w, 0.01f) +
                float2(g_TypedTrailParameters[0].z, 0.f) *
                    g_EffectLocalTime).rgb *
                max(g_TypedTrailParameters[0].y, 0.f);
        }
        output.SceneColor.rgb = radiance *
            max(g_TypedTrailParameters[2].rgb, float3(0.f, 0.f, 0.f));
    }
    else if (25 == g_SourceMaterialProfile)
    {
        // fx_e_pa_fluid_01: alpha/subUV-alpha own shape, normal only distorts,
        // and emissive owns color.  Opaque normal DDS alpha is ignored.
        clip((g_SourceTextureMask & 0xfu) == 0xfu ? 1.f : -1.f);
        const float2 normalUV = uv * max(
            g_TypedTrailParameters[1].x, 0.01f);
        const float4 normalSample = Sample_SourceTexture0(normalUV);
        const float2 warpedUV = uv + (normalSample.rg * 2.f - 1.f) *
            clamp(g_TypedTrailParameters[0].w * 0.002f, -0.1f, 0.1f);
        const float alphaA = dot(Sample_SourceTexture1(warpedUV).rgb,
            float3(0.299f, 0.587f, 0.114f));
        const float alphaB = dot(Sample_SourceTexture3(warpedUV).rgb,
            float3(0.299f, 0.587f, 0.114f));
        const float carrier = max(alphaA, alphaB);
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = pow(saturate(carrier),
            max(abs(g_TypedTrailParameters[0].y), 0.01f)) *
            max(g_TypedTrailParameters[0].z, 0.f) *
            saturate(edgeDistance * 64.f);
        float3 emission = Desaturate_SourceColor(
            Sample_SourceTexture2(warpedUV).rgb,
            g_TypedTrailParameters[2].x) *
            max(g_TypedTrailParameters[3].rgb, float3(0.f, 0.f, 0.f));
        if (0u != (g_SourceTextureMask & 0x10u))
        {
            emission += pow(saturate(Sample_SourceTexture4(
                uv * max(g_TypedTrailParameters[1].w, 0.01f)).rgb),
                max(abs(g_TypedTrailParameters[1].z), 0.01f)) *
                max(g_TypedTrailParameters[4].rgb,
                    float3(0.f, 0.f, 0.f));
        }
        output.SceneColor.rgb = emission;
        output.Distortion.xy = (normalSample.rg * 2.f - 1.f) *
            g_TypedTrailParameters[0].x * 0.01f * shape * distortionScale;
    }
    else if (26 == g_SourceMaterialProfile)
    {
        // fx_m_pa_worldoffset_01: two normal/noise lanes warp the wave; the
        // wave/dependency define coverage and the world-offset lane emits.
        clip((g_SourceTextureMask & 0x1fu) == 0x1fu ? 1.f : -1.f);
        const float dissolveVisibility = saturate(
            Source_DynamicParameterValue(dynamicParameter, 46u, 1.f));
        const float2 noiseAUV = uv * max(
            abs(g_TypedTrailParameters[3].xx), float2(0.01f, 0.01f));
        const float2 noiseBUV = uv * max(
            abs(g_TypedTrailParameters[3].yy), float2(0.01f, 0.01f));
        const float2 noiseA = Sample_SourceTexture0(noiseAUV).rg * 2.f - 1.f;
        const float2 noiseB = Sample_SourceTexture1(noiseBUV).rg * 2.f - 1.f;
        const float2 waveUV = uv * max(abs(g_TypedTrailParameters[2].xy),
            float2(0.01f, 0.01f)) +
            (noiseA * g_TypedTrailParameters[1].x +
                noiseB * g_TypedTrailParameters[1].y) * 0.02f;
        const float3 wave = Sample_SourceTexture2(waveUV).rgb;
        const float dependency = dot(Sample_SourceTexture4(waveUV).rgb,
            float3(0.299f, 0.587f, 0.114f));
        const float radialLower = min(
            saturate(g_TypedTrailParameters[0].x), 1.f - 1.e-4f);
        const float radialUpper = max(radialLower + 1.e-4f,
            min(radialLower + max(g_TypedTrailParameters[0].y, 0.01f),
                1.f));
        const float radial = smoothstep(
            radialLower, radialUpper, 1.f - radius);
        const float carrier = max(dot(wave,
            float3(0.299f, 0.587f, 0.114f)), dependency);
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = pow(saturate(carrier),
            max(abs(g_TypedTrailParameters[0].w), 0.01f)) * radial *
            dissolveVisibility * saturate(edgeDistance * 64.f);
        const float2 emissionUV = waveUV * max(
            g_TypedTrailParameters[2].w, 0.01f) +
            float2(g_TypedTrailParameters[3].w, 0.f) * g_EffectLocalTime;
        const float3 emission = Sample_SourceTexture3(emissionUV).rgb *
            max(g_TypedTrailParameters[2].z, 0.f) +
            max(g_TypedTrailParameters[5].rgb, float3(0.f, 0.f, 0.f));
        output.SceneColor.rgb = pow(saturate(emission),
            max(abs(g_TypedTrailParameters[4].x), 0.01f));
        output.Distortion.xy = (noiseA + noiseB) *
            g_TypedTrailParameters[1].z * 0.01f * shape * distortionScale;
    }
    else if (27 == g_SourceMaterialProfile)
    {
        // fx_k_pa_makeflow_01: opacity and mask own coverage, the two diffuse
        // lanes plus color own RGB, and the dependency lane is flow only.
        clip((g_SourceTextureMask & 0x3fu) == 0x3fu ? 1.f : -1.f);
        const float2 flowUV = uv * max(abs(g_TypedTrailParameters[3].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[3].zw *
            g_EffectLocalTime;
        const float4 flow = Sample_SourceTexture5(flowUV);
        const float2 flowVector = (flow.rg * 2.f - 1.f) * clamp(
            g_TypedTrailParameters[0].w * 0.02f, -0.12f, 0.12f);
        float2 opacityUV = uv * max(abs(g_TypedTrailParameters[0].xy),
            float2(0.01f, 0.01f)) + flowVector;
        opacityUV = Rotate_LinearFlowUV(opacityUV,
            g_TypedTrailParameters[0].z);
        const float opacityCarrier = dot(
            Sample_SourceTexture3(opacityUV).rgb,
            float3(0.299f, 0.587f, 0.114f));
        const float maskCarrier = dot(Sample_SourceTexture4(uv).rgb,
            float3(0.299f, 0.587f, 0.114f));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = (1.f - exp(-pow(saturate(opacityCarrier),
            max(abs(g_TypedTrailParameters[4].y), 0.01f)) *
            max(g_TypedTrailParameters[4].x, 0.f))) *
            saturate(maskCarrier) * saturate(edgeDistance * 64.f);
        const float2 diff1UV = uv * max(abs(g_TypedTrailParameters[1].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[1].zw *
            g_EffectLocalTime + flowVector;
        const float2 diff2UV = uv * max(abs(g_TypedTrailParameters[2].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[2].zw *
            g_EffectLocalTime - flowVector;
        float3 diffuse = lerp(Sample_SourceTexture1(diff1UV).rgb,
            Sample_SourceTexture0(diff2UV).rgb, saturate(flow.r));
        diffuse = Desaturate_SourceColor(diffuse,
            g_TypedTrailParameters[5].z);
        diffuse = pow(saturate(diffuse),
            max(abs(g_TypedTrailParameters[5].y), 0.01f)) *
            max(g_TypedTrailParameters[5].x, 0.f);
        const float3 colorTexture = Sample_SourceTexture2(uv).rgb;
        output.SceneColor.rgb = pow(saturate(diffuse * colorTexture),
            max(abs(g_TypedTrailParameters[6].y), 0.01f)) *
            max(g_TypedTrailParameters[6].x, 0.f);
        output.Distortion.xy = (flow.rg * 2.f - 1.f) *
            max(g_TypedTrailParameters[4].w, 0.f) * 0.01f * shape *
            distortionScale;
    }
    else if (28 == g_SourceMaterialProfile)
    {
        // fx_c_pa_lensflare_01: the one exact lens texture owns both shape
        // and radiance.  Its opaque DDS alpha is never used as a full quad.
        clip((g_SourceTextureMask & 0x1u) == 0x1u ? 1.f : -1.f);
        const float3 lens = Desaturate_SourceColor(
            Sample_SourceTexture0(uv).rgb, g_TypedTrailParameters[0].x);
        const float carrier = dot(max(lens, float3(0.f, 0.f, 0.f)),
            float3(0.299f, 0.587f, 0.114f));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = saturate(carrier) * saturate(edgeDistance * 64.f);
        output.SceneColor.rgb = lens;
    }
    else if (29 == g_SourceMaterialProfile)
    {
        // Bounded reconstruction of fx_j_pa_glasshole_02_tr.  The aura DDS
        // owns source alpha; crack-normal and inner-hole are material inputs,
        // never alternate full-card alpha carriers.
        clip((g_SourceTextureMask & 0x7u) == 0x7u ? 1.f : -1.f);

        const float2 glassCentered =
            (localUV - float2(0.5f, 0.5f)) /
            max(abs(radialSize), 0.0001f);
        const float glassRadius = length(glassCentered * 2.f);
        const float glassEdge = pow(saturate(
            1.f - glassRadius /
                max(abs(g_TypedTrailParameters[7].y), 0.01f)),
            max(abs(g_TypedTrailParameters[7].x), 1.f));

        const float2 normalUV = localUV * max(
            abs(g_TypedTrailParameters[6].xy), float2(0.01f, 0.01f)) +
            float2(g_TypedTrailParameters[7].z,
                g_TypedTrailParameters[2].w) * g_EffectLocalTime;
        const float2 crackNormal =
            Sample_SourceTexture1(normalUV).rg * 2.f - 1.f;
        const float normalStrength = clamp(
            g_TypedTrailParameters[6].z * 0.0025f, -0.1f, 0.1f);
        const float twist = g_TypedTrailParameters[1].w *
            saturate(glassRadius);
        float2 auraUV = Rotate_LinearFlowUV(localUV, twist);
        auraUV = (auraUV - float2(0.5f, 0.5f)) * max(
            abs(g_TypedTrailParameters[0].xy), float2(0.01f, 0.01f)) +
            float2(0.5f, 0.5f) +
            (g_TypedTrailParameters[0].zw - float2(0.5f, 0.5f)) +
            g_TypedTrailParameters[2].zw * g_EffectLocalTime +
            crackNormal * normalStrength;
        const float4 aura = Sample_SourceTexture0(auraUV);

        const float2 innerUV = localUV *
            max(abs(g_TypedTrailParameters[5].w), 0.01f) +
            g_TypedTrailParameters[4].xy * g_EffectLocalTime +
            crackNormal * g_TypedTrailParameters[3].w * 0.01f;
        const float3 innerRgb = Desaturate_SourceColor(
            Sample_SourceTexture2(innerUV).rgb,
            g_TypedTrailParameters[5].x);
        const float inner = pow(saturate(dot(
            innerRgb, float3(0.299f, 0.587f, 0.114f))),
            max(abs(g_TypedTrailParameters[4].z), 0.01f)) *
            max(g_TypedTrailParameters[4].w, 0.f);
        const float auraAlpha = pow(saturate(aura.a),
            max(abs(g_TypedTrailParameters[1].y), 0.01f)) *
            max(g_TypedTrailParameters[1].x, 0.f);
        const float glassCardDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = saturate(auraAlpha) * glassEdge *
            saturate(glassCardDistance * 64.f);
        output.SceneColor.rgb =
            aura.rgb * g_SourceVector0.rgb *
                max(g_TypedTrailParameters[1].x, 0.f) +
            innerRgb * g_SourceVector1.rgb * inner;
        const float glassDistortion = clamp(
            g_TypedTrailParameters[5].z * 0.001f, -0.25f, 0.25f);
        output.Distortion.xy = crackNormal * glassDistortion *
            saturate(pow(max(inner, 0.0001f),
                rcp(max(abs(g_TypedTrailParameters[5].y), 1.f)))) *
            shape * distortionScale;
    }
    else if (30 == g_SourceMaterialProfile)
    {
        // Bounded reconstruction of fx_k_pa_fluidninja_01_tr.  The two flow
        // lanes warp the caustic; only mask_tex and opacity_tex own coverage.
        clip((g_SourceTextureMask & 0x1fu) == 0x1fu ? 1.f : -1.f);

        const float2 flow1UV = Transform_LinearFlowUV(
            uv, g_TypedTrailParameters[2].xy,
            g_TypedTrailParameters[3].xy,
            g_TypedTrailParameters[2].zw, 0.f,
            g_TypedTrailParameters[3].z);
        const float2 flow2UV = Transform_LinearFlowUV(
            uv, g_TypedTrailParameters[4].xy,
            g_TypedTrailParameters[5].xy,
            g_TypedTrailParameters[4].zw, 0.f,
            g_TypedTrailParameters[5].z);
        const float2 flow1 = Sample_SourceTexture1(flow1UV).rg * 2.f - 1.f;
        const float2 flow2 = Sample_SourceTexture2(flow2UV).rg * 2.f - 1.f;
        const float2 flowWarp =
            flow1 * g_TypedTrailParameters[1].y * 0.05f +
            flow2 * g_TypedTrailParameters[1].z * 0.05f;

        const float2 diffUV = uv * max(
            abs(g_TypedTrailParameters[0].xy), float2(0.01f, 0.01f)) +
            flowWarp;
        float3 diffuse = Desaturate_SourceColor(
            Sample_SourceTexture0(diffUV).rgb,
            g_TypedTrailParameters[1].x);
        diffuse = pow(saturate(diffuse),
            max(abs(g_TypedTrailParameters[0].z), 0.01f)) *
            max(g_TypedTrailParameters[0].w, 0.f);

        const float2 maskUV = uv * max(
            abs(g_TypedTrailParameters[6].xy), float2(0.01f, 0.01f)) +
            flowWarp * g_TypedTrailParameters[3].w;
        const float2 opacityUV = uv * max(
            abs(g_TypedTrailParameters[7].xy), float2(0.01f, 0.01f)) +
            flowWarp;
        const float maskCarrier = pow(saturate(
            Sample_SourceTexture3(maskUV).r *
                max(g_TypedTrailParameters[6].z, 0.f)),
            max(abs(g_TypedTrailParameters[5].w), 0.01f));
        const float opacityCarrier = pow(saturate(
            Sample_SourceTexture4(opacityUV).r *
                max(g_TypedTrailParameters[6].w, 0.f)),
            max(abs(g_TypedTrailParameters[7].z), 0.01f));
        // The cooked graph wiring is absent, but gra_pow is an explicit
        // opacity-gradient control rather than a diffuse-color exponent.
        const float opacityGradient = pow(saturate(opacityCarrier),
            max(abs(g_TypedTrailParameters[7].w), 0.01f));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = maskCarrier * opacityGradient *
            saturate(edgeDistance * 64.f);
        const float blend = saturate(dot(diffuse,
            float3(0.299f, 0.587f, 0.114f)));
        output.SceneColor.rgb = diffuse * lerp(
            g_SourceVector0.rgb, g_SourceVector1.rgb, blend);
        output.Distortion.xy = flowWarp *
            max(abs(g_TypedTrailParameters[1].w), 0.f) *
            shape * distortionScale;
    }
    else if (31 == g_SourceMaterialProfile)
    {
        // Bounded reconstruction of fx_j_pa_customparticle_01_ad.  Its
        // a_noise_01 texture is unresolved in the source capture, so lane 1
        // is optional and the analytic cast envelope remains deterministic.
        clip((g_SourceTextureMask & 0x1u) == 0x1u ? 1.f : -1.f);

        float2 noiseVector = float2(0.f, 0.f);
        if (0u != (g_SourceTextureMask & 0x2u))
        {
            const float2 noiseUV = uv * max(float2(
                abs(g_TypedTrailParameters[2].w),
                abs(g_TypedTrailParameters[3].x)),
                float2(0.01f, 0.01f));
            noiseVector = Sample_SourceTexture1(noiseUV).rg * 2.f - 1.f;
        }
        float2 diffUV = (uv - float2(0.5f, 0.5f)) * max(
            abs(g_TypedTrailParameters[0].xy), float2(0.01f, 0.01f)) +
            float2(0.5f, 0.5f);
        diffUV = Rotate_LinearFlowUV(diffUV,
            g_TypedTrailParameters[1].z);
        diffUV += g_TypedTrailParameters[1].xy +
            g_TypedTrailParameters[0].zw * g_EffectLocalTime +
            noiseVector * g_TypedTrailParameters[2].z * 0.05f;
        const float4 diffSample = Sample_SourceTexture0(diffUV);
        float3 diffuse = Desaturate_SourceColor(
            diffSample.rgb, g_TypedTrailParameters[1].w);
        diffuse = pow(saturate(diffuse),
            max(abs(g_TypedTrailParameters[2].y), 0.01f)) *
            max(g_TypedTrailParameters[2].x, 0.f) * g_SourceVector0.rgb;

        const float direction = g_TypedTrailParameters[4].y < 0.f ? -1.f : 1.f;
        const float sweep = frac(g_TypedTrailParameters[4].x +
            direction * g_TypedTrailParameters[3].w * g_EffectLocalTime);
        const float castWidth = max(
            abs(g_TypedTrailParameters[3].z), 0.01f);
        const float castBand = saturate(
            1.f - abs(localUV.y - sweep) / castWidth);
        const float lateral = abs(localUV.x - 0.5f) * 2.f;
        const float cone = saturate(1.f - lateral /
            max(castWidth + abs(localUV.y - sweep), 0.01f));
        const float castEnvelope = smoothstep(
            saturate(g_TypedTrailParameters[5].x),
            max(g_TypedTrailParameters[5].y, 0.0001f),
            castBand * cone);
        // mask_bias=-1 is a different graph operation in the stripped source;
        // adding it directly to a [0,1] DDS alpha makes this occurrence exactly
        // zero everywhere. Family-lite keeps the recovered mask strength/power
        // and lets the analytic cast envelope own the visible cutout.
        const float carrier = pow(saturate(
            diffSample.a * max(g_TypedTrailParameters[7].x, 0.f)),
            max(abs(g_TypedTrailParameters[7].y), 0.01f));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = carrier * castEnvelope * saturate(edgeDistance * 64.f);
        // Keep RGB unpremultiplied; SceneColor.a applies shape once below.
        output.SceneColor.rgb = diffuse;
    }
    else if (32 == g_SourceMaterialProfile)
    {
        // Bounded reconstruction of fx_k_crackholev2_01.  map_e/map_f own
        // radiance; the left/right masks and mask-noise own the silhouette.
        clip((g_SourceTextureMask & 0x3fu) == 0x3fu ? 1.f : -1.f);

        const float2 normalUV = uv * max(
            abs(g_TypedTrailParameters[4].yz), float2(0.01f, 0.01f)) +
            float2(g_TypedTrailParameters[4].w,
                g_TypedTrailParameters[5].x) * g_EffectLocalTime;
        const float2 normal = Sample_SourceTexture2(normalUV).rg * 2.f - 1.f;
        const float2 maskNoiseUV = uv * max(
            abs(g_TypedTrailParameters[5].zw), float2(0.01f, 0.01f)) +
            float2(0.f, g_TypedTrailParameters[6].z * g_EffectLocalTime);
        const float2 maskNoise =
            Sample_SourceTexture3(maskNoiseUV).rg * 2.f - 1.f;
        const float2 maskWarp = maskNoise *
            g_TypedTrailParameters[5].y * 0.05f;
        const float2 maskUV = uv + g_TypedTrailParameters[6].xy +
            g_TypedTrailParameters[7].xy + maskWarp;
        const float maskL = Sample_SourceTexture4(maskUV).r;
        const float maskR = Sample_SourceTexture5(
            float2(1.f - maskUV.x, maskUV.y)).r;
        const float maskCarrier = max(maskL, maskR);

        // `01_thickness` belongs to the separate round-emission group. The
        // crack silhouette uses the recovered `01.thickness` lane; treating
        // 0.05 as an override for the source value 5 makes the slash 100x too
        // weak and effectively invisible.
        const float crackThickness = max(
            abs(g_TypedTrailParameters[3].x), 0.01f);
        const float alphaCarrier = pow(saturate(maskCarrier *
            min(crackThickness * 0.2f, 4.f) *
            max(g_TypedTrailParameters[2].x, 0.f)),
            max(abs(g_TypedTrailParameters[2].y), 0.01f));
        const float2 crackCentered =
            (localUV - float2(0.5f, 0.5f)) /
            max(abs(radialSize), 0.0001f);
        const float radialEnvelope = pow(saturate(
            1.f - length(crackCentered * 2.f) /
                max(abs(g_TypedTrailParameters[2].z), 0.01f)),
            max(abs(g_TypedTrailParameters[2].w), 1.f));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = alphaCarrier * radialEnvelope *
            saturate(edgeDistance * 64.f);

        const float2 mapEUV = uv * max(
            abs(g_TypedTrailParameters[0].xy), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[0].zw * g_EffectLocalTime + normal * 0.02f;
        const float2 mapFUV = uv * max(
            abs(g_TypedTrailParameters[1].xy), float2(0.01f, 0.01f)) +
            g_TypedTrailParameters[1].zw * g_EffectLocalTime + normal * 0.01f;
        const float3 mapE = Desaturate_SourceColor(
            Sample_SourceTexture0(mapEUV).rgb, 1.f);
        const float3 mapF = Sample_SourceTexture1(mapFUV).rgb;
        output.SceneColor.rgb =
            mapE * g_SourceVector0.rgb + mapF * g_SourceVector1.rgb;
        output.Distortion.xy = normal * clamp(
            g_TypedTrailParameters[4].x * 0.01f, -0.25f, 0.25f) *
            shape * distortionScale;
    }
    else if (33 == g_SourceMaterialProfile)
    {
        // fx_mm_simple_01_ad. One emissive sample, an optional UV-noise offset,
        // a desaturation, additive out. The two UV domains are deliberately
        // separate: uv_panning moves the emissive sample, uv_noise_panning moves
        // only the noise lookup, and the grouped fallback collapsed both into a
        // single pan that scrolled artwork the source keeps still.
        clip(g_SourceTextureMask == 0x1u || g_SourceTextureMask == 0x3u ?
            1.f : -1.f);
        const float2 emissivePan =
            g_TypedTrailParameters[0].xy * g_EffectLocalTime;
        const float2 noisePan =
            g_TypedTrailParameters[0].zw * g_EffectLocalTime;
        const float noiseTile = max(abs(g_TypedTrailParameters[1].x), 0.01f);
        const float noiseIntensity = g_TypedTrailParameters[1].y;
        const float desaturation = saturate(g_TypedTrailParameters[1].z);

        float2 emissiveUV = localUV + emissivePan;
        if (0u != (g_SourceTextureMask & 0x2u) &&
            abs(noiseIntensity) > 1.0e-5f)
        {
            const float2 noiseUV = localUV * noiseTile + noisePan;
            emissiveUV += (Sample_SourceTexture1(noiseUV).rg * 2.f - 1.f) *
                noiseIntensity;
        }

        const float4 emissive = Sample_SourceTexture0(emissiveUV);
        const float3 emissiveColor = Desaturate_SourceColor(
            emissive.rgb, desaturation);

        // The DDS in this family are BC1 with no alpha, so base.a is 1 for every
        // texel and coverage has to come from the artwork itself. The grouped
        // path multiplied that luminance by the constant 1 alpha and then cut a
        // 1/128 border, which left a hard rectangle; a smooth radial envelope
        // removes the rim without touching the interior.
        const float2 centered = localUV - float2(0.5f, 0.5f);
        const float envelope = saturate(
            1.f - smoothstep(0.42f, 0.5f, length(centered)));
        shape = saturate(dot(emissiveColor,
            float3(0.299f, 0.587f, 0.114f))) * envelope;

        output.SceneColor.rgb = emissiveColor;
    }
    else if (34 == g_SourceMaterialProfile)
    {
        // Bounded sprite reconstruction for the exact DimensionMaster-F child
        // of fx_mm_fluid_01. The source parent noise closure and native pass ABI
        // are not recovered, so this profile deliberately admits only its two
        // evidenced sprite lanes. Transition luminance and a radial envelope
        // own coverage; opaque BC DDS alpha must never expose the full quad.
        clip((g_SourceTextureMask & 0x3u) == 0x3u ? 1.f : -1.f);

        const float transitionSpeed = Source_DynamicParameterValue(
            dynamicParameter, 48u, 0.f);
        const float alphaPower = max(abs(Source_DynamicParameterValue(
            dynamicParameter, 49u, 1.f)), 0.01f);
        const float fresnelAlpha = max(abs(Source_DynamicParameterValue(
            dynamicParameter, 50u, 1.f)), 0.01f);

        const float2 transitionUV = uv * max(
            abs(g_TypedTrailParameters[0].x), 0.01f) +
            g_TypedTrailParameters[0].yz * g_EffectLocalTime;
        const float3 transitionColor = Sample_SourceTexture0(transitionUV).rgb;
        const float transitionCarrier = saturate(dot(transitionColor,
            float3(0.299f, 0.587f, 0.114f)));
        // The source lane is authored as 0..1.5, not normalized opacity. Keep
        // the bounded reconstruction inside a useful 0..0.9 carrier range so
        // its current 1.0..1.3 curves cannot collapse the whole card to zero.
        const float threshold = saturate(
            abs(transitionSpeed) / 1.5f * 0.8f +
            g_TypedTrailParameters[1].x * 0.1f);
        const float transitionSoftness = max(
            abs(g_TypedTrailParameters[0].w), 1.f / 255.f);
        const float fillGate = smoothstep(
            threshold - transitionSoftness,
            threshold + transitionSoftness, transitionCarrier);
        const float lineWidth = max(
            abs(g_TypedTrailParameters[1].y) * transitionSoftness,
            1.f / 255.f);
        const float lineGate = saturate(
            1.f - abs(transitionCarrier - threshold) / lineWidth);

        const float2 centeredUV = localUV - float2(0.5f, 0.5f);
        const float radialEnvelope = pow(saturate(
            1.f - smoothstep(0.38f, 0.5f, length(centeredUV))),
            fresnelAlpha);
        shape = pow(saturate(max(fillGate, lineGate)), alphaPower) *
            radialEnvelope;

        const float2 emissiveUV = uv * max(
            abs(g_TypedTrailParameters[2].xy), float2(0.01f, 0.01f));
        const float3 emissiveSample = Desaturate_SourceColor(
            Sample_SourceTexture1(emissiveUV).rgb,
            saturate(g_TypedTrailParameters[2].z));
        const float lineIntensity = max(g_TypedTrailParameters[1].z, 0.f);
        const float emissiveIntensity = max(g_TypedTrailParameters[1].w, 0.f);
        const float totalScale = max(g_TypedTrailParameters[2].w, 0.f);
        output.SceneColor.rgb =
            (emissiveSample * emissiveIntensity +
             transitionColor * lineGate * lineIntensity) * totalScale;
    }
    else if (35 == g_SourceMaterialProfile)
    {
        // Class-neutral, bounded reconstruction of fx_k_flowrib_01_tr.  The
        // exact source particle curves are supplied per trail point; this
        // evaluator owns only the evidenced texture roles and scalar graph
        // boundary.  It does not claim the unrecovered native UE3/DXBC ABI.
        const uint variant = (uint)round(g_TypedTrailParameters[2].w);
        const uint requiredTextureMask = 3u == variant ? 0x7u : 0x3u;
        clip(g_SourceTextureMask == requiredTextureMask ? 1.f : -1.f);

        const float xTiling = max(abs(dynamicParameter.x), 0.0001f);
        const float yTiling = max(abs(dynamicParameter.y), 0.0001f);
        const float2 flowUV = float2(
            sourceUV.x * g_TypedTrailParameters[1].x * xTiling,
            localUV.y * g_TypedTrailParameters[1].y * yTiling);
        const float2 flowSample = 3u == variant ?
            Sample_SourceTexture2(flowUV).rg :
            Sample_SourceTexture1(flowUV).rg;
        const float2 flowOffset = (flowSample * 2.f - 1.f) *
            dynamicParameter.w;
        const float2 mainUV = float2(
            sourceUV.x * g_TypedTrailParameters[0].x * xTiling,
            localUV.y * g_TypedTrailParameters[0].y * yTiling +
                g_TypedTrailParameters[0].z) + flowOffset;
        const float3 mainSample = Sample_SourceTexture0(mainUV).rgb;
        const float mainCarrier = max(0.f, dot(mainSample,
            float3(0.299f, 0.587f, 0.114f))) *
            max(g_TypedTrailParameters[0].w, 0.f);

        // Source dissolve is authored in the same 0.7..5/3 intensity domain
        // as the amplified carrier, not as normalized generic opacity.
        const float dissolveSoftness = max(0.05f,
            0.05f * max(dynamicParameter.z, 1.f));
        shape = smoothstep(dynamicParameter.z - dissolveSoftness,
            dynamicParameter.z + dissolveSoftness, mainCarrier);

        if (3u == variant)
        {
            const float2 colorMapUV = mainUV *
                g_TypedTrailParameters[1].zw;
            float3 colorMap = Sample_SourceTexture1(colorMapUV).rgb;
            colorMap = Desaturate_SourceColor(colorMap,
                g_TypedTrailParameters[2].x);
            colorMap = pow(max(colorMap, 0.f),
                max(g_TypedTrailParameters[2].y, 0.01f));
            output.SceneColor.rgb = colorMap *
                g_TypedTrailParameters[3].rgb *
                max(g_TypedTrailParameters[2].z, 0.f) * mainCarrier;
        }
        else
        {
            output.SceneColor.rgb = mainSample * mainCarrier;
        }
    }
    else if (37 == g_SourceMaterialProfile)
    {
        // Bounded sprite carrier for fx_k_pa_makeflow_03. The parent has no
        // opacity texture: its named mask owns coverage and its one exact
        // dependency/flow lane only perturbs UVs. Missing or ambiguous roles
        // are rejected before this shader is selected.
        clip(g_SourceTextureMask == 0x1fu ? 1.f : -1.f);
        const float2 flowUV = uv * max(abs(g_TypedTrailParameters[3].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[3].zw *
            g_EffectLocalTime;
        const float4 flow = Sample_SourceTexture4(flowUV);
        const float2 flowVector = (flow.rg * 2.f - 1.f) * clamp(
            g_TypedTrailParameters[0].w * 0.02f, -0.12f, 0.12f);
        const float2 maskUV = uv * max(abs(g_TypedTrailParameters[7].xy),
            float2(0.01f, 0.01f)) + flowVector;
        const float4 maskSample = Sample_SourceTexture3(maskUV);
        // Cooked mask DDS alpha is frequently a storage-opaque channel.  The
        // parent names this lane mask_tex, so RGB luminance is the bounded
        // coverage carrier; accepting alpha would turn the whole card opaque.
        const float maskCarrier = dot(maskSample.rgb,
            float3(0.299f, 0.587f, 0.114f));
        const float edgeDistance = min(
            min(localUV.x, 1.f - localUV.x),
            min(localUV.y, 1.f - localUV.y));
        shape = pow(saturate(maskCarrier),
            max(abs(g_TypedTrailParameters[4].y), 0.01f)) *
            saturate(edgeDistance * 64.f);

        const float2 diff1UV = uv * max(abs(g_TypedTrailParameters[1].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[1].zw *
            g_EffectLocalTime + flowVector;
        const float2 diff2UV = uv * max(abs(g_TypedTrailParameters[2].xy),
            float2(0.01f, 0.01f)) + g_TypedTrailParameters[2].zw *
            g_EffectLocalTime - flowVector;
        float3 diffuse = lerp(Sample_SourceTexture0(diff1UV).rgb,
            Sample_SourceTexture1(diff2UV).rgb,
            saturate(flow.r + g_TypedTrailParameters[4].z));
        diffuse = Desaturate_SourceColor(diffuse,
            g_TypedTrailParameters[5].z);
        diffuse = pow(saturate(diffuse),
            max(abs(g_TypedTrailParameters[5].y), 0.01f)) *
            max(g_TypedTrailParameters[5].x, 0.f);
        const float3 colorTexture = Desaturate_SourceColor(
            Sample_SourceTexture2(uv).rgb, g_TypedTrailParameters[6].z);
        output.SceneColor.rgb = pow(saturate(diffuse * colorTexture),
            max(abs(g_TypedTrailParameters[6].y), 0.01f)) *
            max(g_TypedTrailParameters[6].x, 0.f);
        output.Distortion.xy = (flow.rg * 2.f - 1.f) *
            max(g_TypedTrailParameters[4].w, 0.f) * 0.01f * shape *
            distortionScale;
    }
    else if (38 == g_SourceMaterialProfile)
    {
        // Bounded fx_mm_simple_02_tr ground-sprite evaluator. The parent
        // evidence closes two emissive roles plus UV noise; recipe size and
        // rotation curves remain particle state rather than shader constants.
        clip(g_SourceTextureMask == 0x7u ? 1.f : -1.f);
        const float2 emissivePan =
            g_TypedTrailParameters[0].xy * g_EffectLocalTime;
        const float2 noiseUV = localUV * max(
            abs(g_TypedTrailParameters[1].x), 0.01f) +
            g_TypedTrailParameters[0].zw * g_EffectLocalTime;
        const float2 noiseOffset =
            (Sample_SourceTexture1(noiseUV).rg * 2.f - 1.f) *
            clamp(g_TypedTrailParameters[1].y, -0.25f, 0.25f);
        const float2 emissiveUV = localUV + emissivePan + noiseOffset;
        const float3 primary = Desaturate_SourceColor(
            Sample_SourceTexture0(emissiveUV).rgb,
            saturate(g_TypedTrailParameters[1].z));
        const float3 secondary = Sample_SourceTexture2(emissiveUV).rgb *
            max(g_TypedTrailParameters[1].w, 0.f);
        const float carrier = saturate(dot(max(primary, 0.f),
            float3(0.299f, 0.587f, 0.114f)));
        const float2 centered = localUV - float2(0.5f, 0.5f);
        const float envelope = saturate(
            1.f - smoothstep(0.42f, 0.5f, length(centered)));
        shape = carrier * envelope;
        output.SceneColor.rgb = (primary + secondary) *
            max(g_SourceVector0.rgb, float3(0.f, 0.f, 0.f));
    }
    else if (39 == g_SourceMaterialProfile)
    {
        // fx_mm_basic_01_ad / _tr.  The parent exposes one emissive radiance
        // lane, one dedicated coverage lane and two independent uv_noise
        // domains with their own tiling and panning.  The grouped path had a
        // single pan and inferred coverage from base luminance, which both
        // scrolled artwork the source keeps still and treated the dedicated
        // alpha_tex child as a second colour source.
        clip((g_SourceTextureMask & 0x1u) == 0x1u ? 1.f : -1.f);

        const float2 emissivePan =
            g_TypedTrailParameters[0].xy * g_EffectLocalTime;
        // Every current occurrence authors uv_scale 1.0, so this lane is
        // carried for child overrides rather than exercised by the corpus.
        const float uvScale = max(abs(g_TypedTrailParameters[0].z), 0.0001f);
        const float emissivePower = max(g_TypedTrailParameters[0].w, 0.f);
        const float desaturation = saturate(g_TypedTrailParameters[1].x);

        float2 noiseOffset = float2(0.f, 0.f);
        if (0u != (g_SourceTextureMask & 0x4u))
        {
            const float2 noise01UV =
                localUV * max(abs(g_TypedTrailParameters[2].zw), 0.01f) +
                g_TypedTrailParameters[2].xy * g_EffectLocalTime;
            noiseOffset += (Sample_SourceTexture2(noise01UV).rg * 2.f - 1.f) *
                clamp(g_TypedTrailParameters[4].x, -0.25f, 0.25f);
        }
        if (0u != (g_SourceTextureMask & 0x8u))
        {
            const float2 noise02UV =
                localUV * max(abs(g_TypedTrailParameters[3].zw), 0.01f) +
                g_TypedTrailParameters[3].xy * g_EffectLocalTime;
            noiseOffset += (Sample_SourceTexture3(noise02UV).rg * 2.f - 1.f) *
                clamp(g_TypedTrailParameters[4].y, -0.25f, 0.25f);
        }

        const float2 basicCentered = localUV - float2(0.5f, 0.5f);
        const float2 emissiveUV = basicCentered * uvScale +
            float2(0.5f, 0.5f) + emissivePan + noiseOffset;
        const float3 emissiveColor = Desaturate_SourceColor(
            Sample_SourceTexture0(emissiveUV).rgb, desaturation);

        // The dedicated alpha lane owns coverage whenever the child binds one.
        // Without it this family shares the simple_01 situation of BC1 artwork
        // whose alpha is 1 for every texel, so luminance and a radial envelope
        // replace the constant alpha that produced a visible card edge.
        float basicCoverage;
        if (0u != (g_SourceTextureMask & 0x2u))
        {
            basicCoverage = saturate(Sample_SourceTexture1(emissiveUV).r);
        }
        else
        {
            const float envelope = saturate(
                1.f - smoothstep(0.42f, 0.5f, length(basicCentered)));
            basicCoverage = saturate(dot(max(emissiveColor, 0.f),
                float3(0.299f, 0.587f, 0.114f))) * envelope;
        }

        shape = basicCoverage;
        output.SceneColor.rgb = emissiveColor * emissivePower;
        output.Distortion.xy = noiseOffset *
            clamp(g_TypedTrailParameters[1].y * 0.01f, -0.25f, 0.25f) *
            basicCoverage;
    }
    else if (40 == g_SourceMaterialProfile)
    {
        // fx_k_me_flowtrail_01_ts_tr.  diff owns radiance, opacity owns
        // coverage and noise offsets both, each in its own UV domain with its
        // own tile, centre and rotation.  The grouped path has a single shared
        // UV scale, so a trail authored with diff v-tile 0.2 against opacity
        // v-tile 1.0 collapses onto one domain and loses the flow.
        //
        // The source wave group and cameravec_pow are not evaluated here: no
        // child in the corpus overrides wave_tile or wave_pan_speed and the
        // parent expression graph is not in evidence, so the wave geometry
        // would be invented rather than restored.
        clip((g_SourceTextureMask & 0x3u) == 0x3u ? 1.f : -1.f);

        float2 flowNoiseOffset = float2(0.f, 0.f);
        if (0u != (g_SourceTextureMask & 0x4u))
        {
            const float2 flowNoiseUV =
                localUV * max(abs(g_TypedTrailParameters[3].zw), 0.01f) +
                g_TypedTrailParameters[4].xy * g_EffectLocalTime;
            flowNoiseOffset =
                (Sample_SourceTexture2(flowNoiseUV).rg * 2.f - 1.f) *
                clamp(g_TypedTrailParameters[3].y, -1.f, 1.f);
        }

        const float2 flowCentered = localUV - float2(0.5f, 0.5f);

        const float diffRotation = g_TypedTrailParameters[0].w;
        const float2 diffRotated = float2(
            flowCentered.x * cos(diffRotation) - flowCentered.y * sin(diffRotation),
            flowCentered.x * sin(diffRotation) + flowCentered.y * cos(diffRotation));
        const float2 diffUV =
            (diffRotated + float2(0.5f, 0.5f)) *
                max(abs(g_TypedTrailParameters[0].xy), 0.01f) +
            float2(g_TypedTrailParameters[0].z, 0.f) + flowNoiseOffset;
        const float3 diffColor = Desaturate_SourceColor(
            Sample_SourceTexture0(diffUV).rgb,
            saturate(g_TypedTrailParameters[1].z));

        const float opacityRotation = g_TypedTrailParameters[2].w;
        const float2 opacityRotated = float2(
            flowCentered.x * cos(opacityRotation) -
                flowCentered.y * sin(opacityRotation),
            flowCentered.x * sin(opacityRotation) +
                flowCentered.y * cos(opacityRotation));
        const float2 opacityUV =
            (opacityRotated + float2(0.5f, 0.5f)) *
                max(abs(g_TypedTrailParameters[2].xy), 0.01f) +
            float2(g_TypedTrailParameters[2].z, 0.f) + flowNoiseOffset;

        shape = saturate(Sample_SourceTexture1(opacityUV).r *
            max(g_TypedTrailParameters[3].x, 0.f));
        output.SceneColor.rgb =
            pow(saturate(diffColor), max(g_TypedTrailParameters[1].x, 0.01f)) *
            max(g_TypedTrailParameters[1].y, 0.f);
        output.Distortion.xy = flowNoiseOffset *
            clamp(g_TypedTrailParameters[1].w * 0.01f, -0.25f, 0.25f) * shape;
    }
    else if (41 == g_SourceMaterialProfile)
    {
        // fx_d_me_chain_01_ma, the Warlord hook-chain link.  The parent
        // declares no texture parameter and only two world-position-offset
        // scalars, so the material owns the masked cutout while the chain
        // artwork arrives through the element base binding.
        //
        // BLEND_Masked is a binary cutout, not a soft alpha ramp.  The grouped
        // path had no cutout at all (authored colour clip is 0), so the links
        // drew as translucent ghosts with no silhouette.
        //
        // The source DDS has no alpha channel, so artwork luminance drives the
        // cutout.  worldpositionoffset_bias and worldpositionoffset_uvscale are
        // not evaluated: world position offset is a vertex stage output and the
        // 132-slot parent graph that shapes it is not in evidence.  Restoring
        // it belongs to the NATIVE_PARITY backlog, not to this RT0 base.
        clip((g_SourceTextureMask & 0x1u) == 0x1u ? 1.f : -1.f);
        const float3 chain = Sample_SourceTexture0(localUV).rgb;
        const float chainCoverage = saturate(dot(max(chain, 0.f),
            float3(0.299f, 0.587f, 0.114f)));
        // UE3 masked materials cut at OpacityMaskClipValue.  One third is the
        // engine default and this parent serializes no override for it.
        clip(chainCoverage - 0.3333f);
        shape = 1.f;
        output.SceneColor.rgb = chain;
    }

    float4 color = (g_ColorMultiply + g_ColorOffset) * vertexColor;
    if (3 != g_SourceMaterialProfile && 4 != g_SourceMaterialProfile &&
        5 != g_SourceMaterialProfile &&
        7 != g_SourceMaterialProfile && 8 != g_SourceMaterialProfile &&
        9 != g_SourceMaterialProfile && 11 != g_SourceMaterialProfile &&
        12 != g_SourceMaterialProfile && 13 != g_SourceMaterialProfile &&
        14 != g_SourceMaterialProfile && 15 != g_SourceMaterialProfile &&
        16 != g_SourceMaterialProfile && 17 != g_SourceMaterialProfile &&
        18 != g_SourceMaterialProfile && 19 != g_SourceMaterialProfile &&
        20 != g_SourceMaterialProfile && 21 != g_SourceMaterialProfile &&
        22 != g_SourceMaterialProfile && 23 != g_SourceMaterialProfile &&
        24 != g_SourceMaterialProfile && 25 != g_SourceMaterialProfile &&
        26 != g_SourceMaterialProfile && 27 != g_SourceMaterialProfile &&
        28 != g_SourceMaterialProfile && 29 != g_SourceMaterialProfile &&
        30 != g_SourceMaterialProfile && 31 != g_SourceMaterialProfile &&
        32 != g_SourceMaterialProfile && 33 != g_SourceMaterialProfile &&
        34 != g_SourceMaterialProfile && 35 != g_SourceMaterialProfile &&
        36 != g_SourceMaterialProfile && 37 != g_SourceMaterialProfile &&
        38 != g_SourceMaterialProfile &&
        39 != g_SourceMaterialProfile &&
        40 != g_SourceMaterialProfile &&
        41 != g_SourceMaterialProfile)
        output.SceneColor.rgb = color.rgb;
    else
    {
        float3 colorModulator = color.rgb;
        if (35 == g_SourceMaterialProfile)
        {
            // Trail COLOR.g/b transport dissolve/distort. Only COLOR.r is the
            // evidenced grayscale HDR source curve; preserve common tint.
            colorModulator = vertexColor.r *
                (g_ColorMultiply + g_ColorOffset).rgb;
        }
        if (19 == g_SourceMaterialProfile || 20 == g_SourceMaterialProfile)
        {
            // Cooked ParticleColor curves may be signed or cross exact zero.
            // ParticleMaster and SpriteWave already reconstruct RGB from
            // source-owned lanes/vectors. Neutral Detail tint keeps the
            // existing per-channel magnitude path exactly. ParticleMaster
            // chromatic authoring instead keeps that path's peak and applies
            // the normalized Detail hue, so a source color curve cannot erase
            // an explicit purple tint.
            const float3 colorMagnitude = abs(color.rgb);
            const float peakColorMagnitude = max(colorMagnitude.r,
                max(colorMagnitude.g, colorMagnitude.b));
            const float3 authoredTintMagnitude = abs(
                (g_AuthoredColorMultiply + g_ColorOffset).rgb);
            const float peakAuthoredTint = max(authoredTintMagnitude.r,
                max(authoredTintMagnitude.g, authoredTintMagnitude.b));
            const float minimumAuthoredTint = min(authoredTintMagnitude.r,
                min(authoredTintMagnitude.g, authoredTintMagnitude.b));
            const float3 normalizedAuthoredTint = peakAuthoredTint > 0.0001f ?
                authoredTintMagnitude / peakAuthoredTint :
                float3(1.f, 1.f, 1.f);
            if (19 == g_SourceMaterialProfile &&
                peakAuthoredTint - minimumAuthoredTint > 0.0001f)
            {
                colorModulator = peakColorMagnitude > 0.0001f ?
                    peakColorMagnitude * normalizedAuthoredTint :
                    normalizedAuthoredTint;
            }
            else
            {
                colorModulator = peakColorMagnitude > 0.0001f ?
                    colorMagnitude : float3(1.f, 1.f, 1.f);
            }
        }
        output.SceneColor.rgb *= colorModulator;
        if (11 == g_SourceMaterialProfile || 16 == g_SourceMaterialProfile ||
            36 == g_SourceMaterialProfile ||
            17 == g_SourceMaterialProfile || 18 == g_SourceMaterialProfile)
        {
            const float peakRadiance = max(output.SceneColor.r,
                max(output.SceneColor.g, output.SceneColor.b));
            output.SceneColor.rgb /= 1.f + peakRadiance;
        }
    }
    // ParticleMaster keeps its existing bounded source look at intensity 1,
    // then treats the authored value as SceneHDR gain.  Applying that value
    // before the shared-peak denominator makes every value converge below 1
    // and therefore makes Bloom mathematically unreachable.
    const float preNormalizationEmissiveIntensity =
        19 == g_SourceMaterialProfile ? 1.f : g_EmissiveIntensity;
    output.SceneColor.rgb *=
        lighting * preNormalizationEmissiveIntensity * emissive;
    if ((g_SourceMaterialProfile >= 19 && g_SourceMaterialProfile <= 32) ||
        34 == g_SourceMaterialProfile ||
        (g_SourceMaterialProfile >= 37 && g_SourceMaterialProfile <= 40))
    {
        const float peakRadiance = max(output.SceneColor.r,
            max(output.SceneColor.g, output.SceneColor.b));
        output.SceneColor.rgb /= 1.f + peakRadiance;
    }
    if (19 == g_SourceMaterialProfile)
        output.SceneColor.rgb *= g_EmissiveIntensity;
    const float profileOpacity =
        (9 == g_SourceMaterialProfile || 35 == g_SourceMaterialProfile) ?
            1.f : opacity;
    output.SceneColor.a = saturate(color.a * shape * profileOpacity);
    clip(output.SceneColor.a - g_ColorClip);
    return output;
}

EFFECT_PS_OUT Shade_LocalCrackMesh(
    float2 uv,
    float3 worldPosition,
    float3 worldNormal,
    float3 worldTangent,
    float3 worldBinormal,
    float3 cameraPosition,
    float4 vertexColor,
    float4 dynamicParameter)
{
    clip(g_SourceTextureMask == 0x7u ? 1.f : -1.f);

    const float2 normalUV = uv * g_LocalCrackParameters[1].zw;
    const float2 normalXY =
        Sample_SourceTexture0(normalUV).rg * 2.f - 1.f;
    const float normalZ = sqrt(saturate(1.f - dot(normalXY, normalXY)));
    const float3 surfaceNormal = normalize(
        normalize(worldTangent) * normalXY.x +
        normalize(worldBinormal) * normalXY.y +
        normalize(worldNormal) * normalZ);
    const float3 viewDirection = normalize(cameraPosition - worldPosition);
    const float facing = saturate(dot(surfaceNormal, viewDirection));
    const float fresnel = pow(
        saturate(1.f - facing),
        max(abs(g_LocalCrackParameters[2].y), 0.01f));

    const float3 reflected = reflect(-viewDirection, surfaceNormal);
    float2 reflectionUV =
        reflected.xy / max(abs(g_LocalCrackParameters[3].y), 0.01f);
    reflectionUV = reflectionUV * g_LocalCrackParameters[4].yz +
        float2(g_LocalCrackParameters[3].w, g_LocalCrackParameters[4].x) +
        float2(g_LocalCrackParameters[3].z * g_EffectLocalTime, 0.f);
    float3 reflection = Sample_SourceTexture1(reflectionUV).rgb;
    reflection = Desaturate_SourceColor(
        reflection, g_LocalCrackParameters[3].x);
    reflection = pow(saturate(reflection),
        max(abs(g_LocalCrackParameters[2].w), 0.01f));
    reflection *= g_LocalCrackReflectionColor.rgb * fresnel;

    float2 dynamicUV = uv;
    float opacity = 1.f;
    float emissive = 1.f;
    float distortion = 1.f;
    float radialSize = 1.f;
    Apply_DynamicParameter(
        dynamicUV, opacity, emissive, distortion, radialSize,
        dynamicParameter);
    const float2 dissolveUV =
        dynamicUV * g_LocalCrackParameters[0].xy +
        g_LocalCrackParameters[0].zw * g_EffectLocalTime;
    const float dissolve = Sample_SourceTexture2(dissolveUV).r;
    const float dynamicDissolve = Source_DynamicParameterValue(
        dynamicParameter, 3u, 1.f - saturate(g_DissolveAmount));
    const float threshold = max(
        saturate(g_DissolveAmount), saturate(1.f - dynamicDissolve));
    const float softness = rcp(
        max(abs(g_LocalCrackParameters[1].x), 1.f));
    const float dissolveGate = smoothstep(
        threshold - softness, threshold + softness,
        dissolve + g_LocalCrackParameters[1].y);

    const float3 body = lerp(
        g_LocalCrackOutColor.rgb,
        g_LocalCrackInColor.rgb,
        facing);
    const float4 color = (g_ColorMultiply + g_ColorOffset) * vertexColor;
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor.rgb =
        (body + reflection) * color.rgb * g_EmissiveIntensity * emissive;
    output.SceneColor.a = saturate(color.a * dissolveGate);
    clip(output.SceneColor.a - max(g_ColorClip, 1.f / 255.f));
    return output;
}

EFFECT_PS_OUT Shade_EffectParticle(
    float2 sourceUV,
    float3 lighting,
    float4 vertexColor,
    float4 dynamicParameter)
{
    return Shade_EffectParticleUV(
        sourceUV, sourceUV, lighting, vertexColor, dynamicParameter);
}

BlendState BS_EffectOpaque
{
    BlendEnable[0] = false;
    BlendEnable[1] = true;
    SrcBlend[1] = One;
    DestBlend[1] = One;
    BlendOp[1] = Add;
    SrcBlendAlpha[1] = One;
    DestBlendAlpha[1] = One;
    BlendOpAlpha[1] = Add;
    RenderTargetWriteMask[1] = 0x03;
};

BlendState BS_EffectAlpha
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;
    SrcBlend[0] = Src_Alpha;
    DestBlend[0] = Inv_Src_Alpha;
    BlendOp[0] = Add;
    SrcBlendAlpha[0] = One;
    DestBlendAlpha[0] = Inv_Src_Alpha;
    BlendOpAlpha[0] = Add;
    SrcBlend[1] = One;
    DestBlend[1] = One;
    BlendOp[1] = Add;
    SrcBlendAlpha[1] = One;
    DestBlendAlpha[1] = One;
    BlendOpAlpha[1] = Add;
    RenderTargetWriteMask[1] = 0x03;
};

BlendState BS_EffectAdditive
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;
    SrcBlend[0] = Src_Alpha;
    DestBlend[0] = One;
    BlendOp[0] = Add;
    SrcBlendAlpha[0] = One;
    DestBlendAlpha[0] = One;
    BlendOpAlpha[0] = Add;
    SrcBlend[1] = One;
    DestBlend[1] = One;
    BlendOp[1] = Add;
    SrcBlendAlpha[1] = One;
    DestBlendAlpha[1] = One;
    BlendOpAlpha[1] = Add;
    RenderTargetWriteMask[1] = 0x03;
};
