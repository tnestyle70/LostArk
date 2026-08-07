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
float4 g_GroupedUVScalePan = float4(1.f, 1.f, 0.f, 0.f);
float4 g_GroupedAlphaEmissive = float4(1.f, 1.f, 1.f, 1.f);
float4 g_GroupedNoiseDissolve = float4(0.f, 0.f, 0.f, 1.f);
float4 g_GroupedTint = float4(1.f, 1.f, 1.f, 1.f);
uint g_GroupedMaterialFlags = 0;
float g_EffectLocalTime = 0.f;
uint4 g_DynamicParameterSemantics = uint4(0, 0, 0, 0);

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

EFFECT_PS_OUT Shade_EffectParticle(
    float2 sourceUV,
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

    const float2 centered = (uv - float2(0.5f, 0.5f)) /
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
        const float4 cloud = g_NoiseTexture.Sample(LinearSampler, uv);
        const float2 auraUV = uv + (cloud.rg * 2.f - 1.f) * 0.03f;
        const float4 aura = g_BaseTexture.Sample(LinearSampler, auraUV);
        shape = aura.a * saturate(0.5f + cloud.a) *
            saturate(1.f - radius * 0.5f);
        output.SceneColor.rgb = aura.rgb * (0.75f + cloud.rgb * 0.5f);
    }
    else if (5 == g_SourceMaterialProfile)
    {
        const float4 noise = g_NoiseTexture.Sample(LinearSampler, uv);
        shape = saturate(noise.a + noise.r);
        output.Distortion.xy = (noise.rg * 2.f - 1.f) *
            max(g_DistortionIntensity, 0.f) * distortionScale;
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
        const float4 auraBase = g_BaseTexture.Sample(LinearSampler, uv);
        const float auraMask = 0 != g_HasMask ?
            g_MaskTexture.Sample(LinearSampler, uv).r : 0.f;
        float auraDissolve = 1.f;
        if (0 != g_HasDissolve)
        {
            auraDissolve = g_DissolveTexture.Sample(LinearSampler, uv).r;
            auraDissolve = smoothstep(
                saturate(g_DissolveAmount) - 0.1f,
                saturate(g_DissolveAmount) + 0.1f,
                auraDissolve);
        }
        shape = pow(
            saturate(auraMask),
            max(abs(g_SourceScalars0.w), 0.01f)) * auraDissolve;
        const float auraCarrier = 0.25f + 0.75f * saturate(dot(
            auraBase.rgb, float3(0.299f, 0.587f, 0.114f)));
        output.SceneColor.rgb = lerp(
            g_SourceVector0.rgb, g_SourceVector1.rgb, auraMask) *
            auraCarrier;
    }
    else if (9 == g_SourceMaterialProfile)
    {
        const float crackDissolve = 0 != g_HasDissolve ?
            g_DissolveTexture.Sample(LinearSampler, uv).r : 0.f;
        const float crackThreshold =
            1.f - saturate(dynamicParameter.z);
        shape = smoothstep(
            crackThreshold - 0.1f,
            crackThreshold + 0.1f,
            crackDissolve);
        output.SceneColor.rgb = lerp(
            g_SourceVector0.rgb, g_SourceVector1.rgb, crackDissolve);
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
        const float4 diffuseNoise = g_SourceTexture1.Sample(
            LinearSampler, diffuseNoiseUV);
        const float2 warpedDiffuseUV = diffuseUV +
            (diffuseNoise.rg * 2.f - 1.f) *
            g_LinearFlowParameters[1].w * 0.01f;
        float3 diffuse = g_SourceTexture0.Sample(
            LinearSampler, warpedDiffuseUV).rgb;
        diffuse = Desaturate_LinearFlow(
            diffuse, g_LinearFlowParameters[15].z);
        diffuse = pow(saturate(diffuse),
            max(abs(g_LinearFlowParameters[11].y), 0.01f)) *
            max(g_LinearFlowParameters[11].x, 0.f);

        const float2 maskAUV = Transform_LinearFlowUV(
            uv, g_LinearFlowParameters[3].xy,
            g_LinearFlowParameters[4].xy,
            g_LinearFlowParameters[3].zw,
            g_LinearFlowParameters[4].z,
            g_LinearFlowParameters[4].w);
        const float2 noiseAUV = Transform_LinearFlowUV(
            uv, g_LinearFlowParameters[5].xy,
            g_LinearFlowParameters[6].xy,
            g_LinearFlowParameters[5].zw, 0.f, 1.f);
        const float2 maskAWarp =
            (g_SourceTexture3.Sample(LinearSampler, noiseAUV).rg * 2.f - 1.f) *
            g_LinearFlowParameters[6].z * 0.01f;
        float maskA = g_SourceTexture2.Sample(
            LinearSampler, maskAUV + maskAWarp).r;
        maskA = pow(saturate(maskA),
            max(abs(g_LinearFlowParameters[11].w), 0.01f)) *
            max(g_LinearFlowParameters[11].z, 0.f);

        const float2 maskBUV = Transform_LinearFlowUV(
            uv, g_LinearFlowParameters[7].xy,
            g_LinearFlowParameters[8].xy,
            g_LinearFlowParameters[7].zw,
            g_LinearFlowParameters[8].z,
            g_LinearFlowParameters[8].w);
        const float2 noiseBUV = Transform_LinearFlowUV(
            uv, g_LinearFlowParameters[9].xy,
            g_LinearFlowParameters[10].xy,
            g_LinearFlowParameters[9].zw, 0.f, 1.f);
        const float2 maskBWarp =
            (g_SourceTexture5.Sample(LinearSampler, noiseBUV).rg * 2.f - 1.f) *
            g_LinearFlowParameters[10].z * 0.01f;
        float maskB = g_SourceTexture4.Sample(
            LinearSampler, maskBUV + maskBWarp).r;
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
        const float dissolveValue = g_SourceTexture6.Sample(
            LinearSampler,
            Rotate_LinearFlowUV(dissolveUV, g_SourceScalars0.x)).r;
        const float dissolveFeather = rcp(
            max(abs(g_SourceScalars0.y), 1.f));
        const float dissolveGate = smoothstep(
            saturate(g_DissolveAmount) - dissolveFeather,
            saturate(g_DissolveAmount) + dissolveFeather,
            dissolveValue);

        shape = pow(combinedMask,
            max(abs(g_LinearFlowParameters[12].w), 0.01f)) *
            max(g_LinearFlowParameters[12].z, 0.f) * dissolveGate;
        output.SceneColor.rgb = max(
            diffuse * (
                g_LinearFlowMaskAColor.rgb * maskA +
                g_LinearFlowMaskBColor.rgb * maskB),
            float3(0.f, 0.f, 0.f));
        output.Distortion.xy =
            (diffuseNoise.rg * 2.f - 1.f) *
            max(g_SourceScalars0.w, 0.f) * shape * distortionScale;
    }

    float4 color = (g_ColorMultiply + g_ColorOffset) * vertexColor;
    if (3 != g_SourceMaterialProfile && 4 != g_SourceMaterialProfile &&
        7 != g_SourceMaterialProfile && 8 != g_SourceMaterialProfile &&
        9 != g_SourceMaterialProfile && 11 != g_SourceMaterialProfile)
        output.SceneColor.rgb = color.rgb;
    else
        output.SceneColor.rgb *= color.rgb;
    output.SceneColor.rgb *= lighting * g_EmissiveIntensity * emissive;
    const float profileOpacity = 9 == g_SourceMaterialProfile ? 1.f : opacity;
    output.SceneColor.a = saturate(color.a * shape * profileOpacity);
    clip(output.SceneColor.a - g_ColorClip);
    return output;
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
