#include "Engine_Shader_Defines.hlsli"

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;
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
uint g_SourceMaterialProfile = 0;
float4 g_SourceScalars0 = float4(0.f, 0.f, 0.f, 0.f);
float4 g_SourceScalars1 = float4(0.f, 0.f, 0.f, 0.f);
float4 g_SourceVector0 = float4(0.f, 0.f, 0.f, 0.f);
float4 g_SourceVector1 = float4(0.f, 0.f, 0.f, 0.f);
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
    const float4 noiseSample = 0 != g_HasNoise ?
        g_NoiseTexture.Sample(LinearSampler, uv) : float4(0.f, 0.f, 0.f, 0.f);
    const float surfaceWarp = 0 != g_HasNoise ?
        clamp(g_DistortionIntensity * 0.01f, -0.25f, 0.25f) : 0.f;
    const float2 surfaceUV = uv +
        (noiseSample.rg * 2.f - 1.f) * surfaceWarp;
    const float4 base = g_BaseTexture.Sample(LinearSampler, surfaceUV);
    const float mask = 0 != g_HasMask ?
        g_MaskTexture.Sample(LinearSampler, surfaceUV).r : 1.f;

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
        0 != g_DistortionOnBaseMaterial || 0 != g_HasNoise;
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

    // A reconstructed profile must never expose the hard carrier quad edge.
    // This guard only feathers the outer texel border; it does not invent the
    // missing UE3 expression topology.
    const float edgeDistance = min(
        min(sourceUV.x, 1.f - sourceUV.x),
        min(sourceUV.y, 1.f - sourceUV.y));
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
            uv, lighting, vertexColor, opacity, emissive,
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

        const float radialMask = pow(
            saturate(1.f - radius * max(g_LinearFlowParameters[13].y, 0.f)),
            max(abs(g_LinearFlowParameters[13].w), 0.01f));
        const float combinedMask = saturate(
            (maskA + maskB * saturate(g_LinearFlowParameters[13].z)) *
            max(g_LinearFlowParameters[13].x, 0.f) * radialMask);
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

        const float3 positiveMaskColor =
            max(g_LinearFlowMaskAColor.rgb, float3(0.f, 0.f, 0.f)) * maskA +
            max(g_LinearFlowMaskBColor.rgb, float3(0.f, 0.f, 0.f)) * maskB;
        const float negativeBWeight = saturate(maskBRaw * dot(
            max(-g_LinearFlowMaskBColor.rgb, float3(0.f, 0.f, 0.f)),
            float3(0.299f, 0.587f, 0.114f)));
        const float3 finiteMaskColor =
            positiveMaskColor * (1.f - negativeBWeight);
        const float visibleColorCarrier = saturate(max(
            finiteMaskColor.r,
            max(finiteMaskColor.g, finiteMaskColor.b)));

        // Negative source mask colors are subtractive/cutout inputs.  They
        // must not become an opaque black RGB layer after the finite shader's
        // non-negative clamp (the `_14` black teardrop failure).
        shape = pow(combinedMask,
            max(abs(g_LinearFlowParameters[12].w), 0.01f)) *
            max(g_LinearFlowParameters[12].z, 0.f) * dissolveGate *
            visibleColorCarrier;
        output.SceneColor.rgb = diffuse * finiteMaskColor;
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
        shape = bladeWidth * bladeLength * saturate(flowMask * 1.5f);
        output.SceneColor.rgb = flow.rgb * shape;
        output.Distortion.xy = (flow.rg * 2.f - 1.f) *
            min(abs(g_SourceScalars0.w) * 0.001f, 0.5f) *
            shape * distortionScale;
    }
    else if (13 == g_SourceMaterialProfile)
    {
        // Bounded reconstruction of fx_m_pa_missiletrail_01_tr.  Its four
        // decoded DDS inputs have opaque alpha, so the named R/G carriers and
        // dissolve curve must define the ribbon instead of the DDS alpha.
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
            abs(g_SourceVector0.zw), float2(0.01f, 0.01f)) +
            float2(noisePan, alphaPan) * g_EffectLocalTime;
        const float4 noise = g_NoiseTexture.Sample(LinearSampler, noiseUV);
        const float2 warpedUV = uv + (noise.rg * 2.f - 1.f) *
            min(abs(g_SourceScalars1.z) * noiseStrength * 0.0025f, 0.1f);
        const float2 alphaRUV = float2(
            warpedUV.x * max(abs(g_SourceScalars1.x), 0.01f) +
                g_SourceVector0.x + alphaPan * g_EffectLocalTime,
            warpedUV.y);
        const float2 alphaGUV = float2(
            warpedUV.x * max(abs(g_SourceScalars1.y), 0.01f) +
                g_SourceVector0.y + alphaPan * g_EffectLocalTime,
            warpedUV.y);
        const float alphaR = g_MaskTexture.Sample(
            LinearSampler, alphaRUV).r;
        const float alphaG = g_MaskTexture.Sample(
            LinearSampler, alphaGUV).g;
        const float alphaTexture = max(alphaR, alphaG);
        const float alphaCarrier = smoothstep(
            0.04f, 0.2f, alphaTexture *
                max(g_SourceScalars0.x, 0.f) * 0.15f);
        const float dissolveValue = g_DissolveTexture.Sample(
            LinearSampler, warpedUV).r;
        const float dissolveSoftness = rcp(
            max(abs(g_SourceScalars1.w), 1.f));
        const float dissolveGate = smoothstep(
            dissolveAmount - dissolveSoftness,
            dissolveAmount + dissolveSoftness,
            dissolveValue);
        const float edgeDistance = min(
            min(uv.x, 1.f - uv.x), min(uv.y, 1.f - uv.y));
        const float edgeFeather = saturate(edgeDistance * 64.f);
        shape = alphaCarrier * dissolveGate * edgeFeather;
        const float4 base = g_BaseTexture.Sample(LinearSampler, warpedUV);
        const float emissivePower = max(abs(g_SourceScalars0.w), 0.01f);
        const float emissiveStrength = min(
            max(g_SourceScalars0.z, 0.f) * 0.05f, 8.f);
        output.SceneColor.rgb = pow(saturate(
            base.rgb + alphaTexture.xxx), emissivePower) *
            emissiveStrength * shape;
    }

    float4 color = (g_ColorMultiply + g_ColorOffset) * vertexColor;
    if (3 != g_SourceMaterialProfile && 4 != g_SourceMaterialProfile &&
        5 != g_SourceMaterialProfile &&
        7 != g_SourceMaterialProfile && 8 != g_SourceMaterialProfile &&
        9 != g_SourceMaterialProfile && 11 != g_SourceMaterialProfile &&
        12 != g_SourceMaterialProfile && 13 != g_SourceMaterialProfile)
        output.SceneColor.rgb = color.rgb;
    else
        output.SceneColor.rgb *= color.rgb;
    output.SceneColor.rgb *= lighting * g_EmissiveIntensity * emissive;
    const float profileOpacity = 9 == g_SourceMaterialProfile ? 1.f : opacity;
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
