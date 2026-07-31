texture2D g_OpacityTexture;
texture2D g_DissolveTexture;
texture2D g_DistortionTexture;
texture2D g_EffectDepthTexture;

float2 g_vUVTiling = float2(1.f, 1.f);
float2 g_vUVOffset = float2(0.f, 0.f);
float2 g_vUVPanner = float2(0.f, 0.f);
float2 g_vEffectViewportSize = float2(1.f, 1.f);
float4 g_vDissolveEdgeColor = float4(0.f, 0.f, 0.f, 0.f);
float4 g_vDynamicParameter = float4(1.f, 1.f, 0.f, 1.f);
float g_fEffectTime = 0.f;
float g_fEmissiveStrength = 1.f;
float g_fOpacityMaskThreshold = 0.f;
float g_fDissolveAmount = 0.f;
float g_fDissolveEdgeWidth = 0.05f;
float g_fSoftParticleDistance = 0.f;
float g_fDistortionStrength = 0.f;

sampler EffectDepthSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

BlendState BS_EffectAlphaMRT
{
    BlendEnable[0] = true;
    SrcBlend[0] = Src_Alpha;
    DestBlend[0] = Inv_Src_Alpha;
    BlendOp[0] = Add;
    SrcBlendAlpha[0] = One;
    DestBlendAlpha[0] = Inv_Src_Alpha;
    BlendOpAlpha[0] = Add;
    RenderTargetWriteMask[0] = 0x0f;

    BlendEnable[1] = true;
    SrcBlend[1] = One;
    DestBlend[1] = One;
    BlendOp[1] = Add;
    SrcBlendAlpha[1] = One;
    DestBlendAlpha[1] = One;
    BlendOpAlpha[1] = Add;
    RenderTargetWriteMask[1] = 0x03;
};

BlendState BS_EffectAdditiveMRT
{
    BlendEnable[0] = true;
    SrcBlend[0] = Src_Alpha;
    DestBlend[0] = One;
    BlendOp[0] = Add;
    SrcBlendAlpha[0] = One;
    DestBlendAlpha[0] = One;
    BlendOpAlpha[0] = Add;
    RenderTargetWriteMask[0] = 0x0f;

    BlendEnable[1] = true;
    SrcBlend[1] = One;
    DestBlend[1] = One;
    BlendOp[1] = Add;
    SrcBlendAlpha[1] = One;
    DestBlendAlpha[1] = One;
    BlendOpAlpha[1] = Add;
    RenderTargetWriteMask[1] = 0x03;
};

struct EFFECT_PS_OUT
{
    float4 vColor : SV_TARGET0;
    float4 vDistortion : SV_TARGET1;
};

float2 CalculateEffectUV(float2 sourceUV, float4 uvRect)
{
    // Repeat inside the selected SubUV frame.  Letting localUV escape the
    // 0..1 range would sample a neighbouring atlas frame.
    const float2 localUV = frac(
        sourceUV * g_vUVTiling + g_vUVOffset +
        g_vUVPanner * g_fEffectTime);
    return lerp(uvRect.xy, uvRect.zw, localUV);
}

float CalculateSoftParticleFade(float4 pixelPosition, float particleViewZ)
{
    float softFade = 1.f;
    if (g_fSoftParticleDistance > 0.f)
    {
        const float2 viewportSize = max(
            g_vEffectViewportSize, float2(1.f, 1.f));
        const float2 screenUV = saturate(
            pixelPosition.xy / viewportSize);
        const float sceneViewZ = g_EffectDepthTexture.Sample(
            EffectDepthSampler, screenUV).y * 1000.f;

        // This framework clears Target_Depth.y to 1 (1000 world units).
        // Treat that sentinel as no opaque surface instead of fading at the
        // far plane. Zero is accepted as a second clear sentinel.
        if (sceneViewZ > 0.f && sceneViewZ < 999.9f)
        {
            softFade = saturate(
                (sceneViewZ - max(0.f, particleViewZ)) /
                max(g_fSoftParticleDistance, 0.0001f));
        }
    }
    return softFade;
}

EFFECT_PS_OUT ShadeEffectMaterial(
    float2 sourceUV,
    float4 uvRect,
    float4 tint,
    float particleViewZ,
    float4 pixelPosition)
{
    EFFECT_PS_OUT Out;
    const float2 uv = CalculateEffectUV(sourceUV, uvRect);
    const float4 baseColor = g_Texture.Sample(LinearSampler, uv);
    const float opacityMask = g_OpacityTexture.Sample(LinearSampler, uv).r;
    const float dissolve = saturate(
        g_fDissolveAmount + g_vDynamicParameter.z);

    float dissolveEdge = 0.f;
    if (dissolve >= 1.f)
    {
        clip(-1.f);
    }
    else if (dissolve > 0.f)
    {
        const float dissolveNoise =
            g_DissolveTexture.Sample(LinearSampler, uv).r;
        clip(dissolveNoise - dissolve);
        dissolveEdge = saturate(
            1.f - (dissolveNoise - dissolve) /
            max(g_fDissolveEdgeWidth, 0.0001f));
    }

    float4 color = baseColor * tint;
    color.rgb *= max(0.f,
        g_fEmissiveStrength * g_vDynamicParameter.x);
    color.rgb += g_vDissolveEdgeColor.rgb *
        (g_vDissolveEdgeColor.a * dissolveEdge);
    // Stay below the finite FP16 ceiling.  The final pass sanitizes again in
    // case additive accumulation over several particles overflows SceneHDR.
    color.rgb = min(max(color.rgb, 0.f), 60000.f);

    const float materialAlpha = color.a * opacityMask *
        max(0.f, g_vDynamicParameter.y);
    clip(materialAlpha - max(0.001f, g_fOpacityMaskThreshold));
    color.a = materialAlpha * CalculateSoftParticleFade(
        pixelPosition, particleViewZ);

    const float2 distortionVector =
        g_DistortionTexture.Sample(LinearSampler, uv).rg * 2.f - 1.f;
    const float distortionScale =
        g_fDistortionStrength * g_vDynamicParameter.w * color.a;
    const float2 distortionOffset = clamp(
        distortionVector * distortionScale, -0.05f, 0.05f);

    Out.vColor = color;
    Out.vDistortion = float4(distortionOffset, 0.f, 0.f);
    return Out;
}
