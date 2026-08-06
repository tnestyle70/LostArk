#include "Engine_Shader_Defines.hlsli"

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;

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
    const float4 base = g_BaseTexture.Sample(LinearSampler, uv);
    const float4 noiseSample = 0 != g_HasNoise ?
        g_NoiseTexture.Sample(LinearSampler, uv) : float4(0.f, 0.f, 0.f, 0.f);
    const float mask = 0 != g_HasMask ?
        g_MaskTexture.Sample(LinearSampler, uv).r : 1.f;

    if (0 != g_HasDissolve)
    {
        const float dissolve =
            g_DissolveTexture.Sample(LinearSampler, uv).r + noiseSample.r * 0.1f;
        clip(dissolve - g_DissolveAmount);
    }

    float4 color = (base * g_ColorMultiply + g_ColorOffset) * vertexColor;
    color.rgb *= lighting;
    color.rgb *= g_EmissiveIntensity;
    color.a *= mask;
    clip(color.a - g_ColorClip);

    if (0 != g_HasEmissive)
    {
        color.rgb += g_EmissiveTexture.Sample(
            LinearSampler, uv).rgb * g_EmissiveIntensity;
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
            max(g_DistortionIntensity, 0.01f) * distortionScale;
    }

    float4 color = (g_ColorMultiply + g_ColorOffset) * vertexColor;
    if (3 != g_SourceMaterialProfile && 4 != g_SourceMaterialProfile)
        output.SceneColor.rgb = color.rgb;
    else
        output.SceneColor.rgb *= color.rgb;
    output.SceneColor.rgb *= lighting * g_EmissiveIntensity * emissive;
    output.SceneColor.a = saturate(color.a * shape * opacity);
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
