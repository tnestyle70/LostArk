#ifndef EFFECT_SLICE_SCENE_DEPTH_HLSLI
#define EFFECT_SLICE_SCENE_DEPTH_HLSLI

Texture2D g_EffectSceneDepthTexture;
SamplerState EffectSliceDepthSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

float Slice_SourcePow(float value, float exponent)
{
    return abs(value) < 1.0e-6f ? 0.f : pow(abs(value), exponent);
}

EFFECT_PS_OUT Shade_EffectSliceSceneDepth(float2 uv, float2 pixelPosition,
    float projectionW, float4 particleColor, float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    uint width = 0u;
    uint height = 0u;
    g_EffectSceneDepthTexture.GetDimensions(width, height);
    if (g_SourceTextureMask != 1u || width == 0u || height == 0u ||
        !isfinite(projectionW) || projectionW <= 0.f ||
        !all(isfinite(dynamicParameter)) || !all(isfinite(particleColor)))
    {
        clip(-1.f);
        return output;
    }

    // Selected source PS 12959b8a47f91c4dab8a19b9de0871ed, RT0.
    // The source flow sample's destination/register swizzles select RED.
    const float2 centered = uv - 0.5f;
    const float2 flowUV = (float2(dot(g_SourceVector0.xy, centered),
        dot(g_SourceVector0.zw, centered)) + 0.5f) *
        g_SourceScalars1.xy + g_SourceScalars1.zw;
    const float flow = Sample_SourceTexture0(flowUV).r;
    const float2 warped = uv + flow * dynamicParameter.z * g_SourceScalars0.z;
    const float2 blade = float2(1.f - 2.f * warped.x, 2.f * warped.y - 1.f);
    const float signedSlice = 0.01f / (dot(g_SourceVector1.zw, blade) -
        dot(g_SourceVector1.xy, blade));
    const float radial = Slice_SourcePow(1.f - dot(centered, centered),
        10.f * g_SourceScalars0.y + 10.f);
    const float coverage = saturate(5.f * radial * dynamicParameter.y *
        particleColor.a * signedSlice);

    // Project adapter: Target_Depth.y stores projectionW / 1000, not UE's
    // nonlinear device depth. Both current writers and this VS use the same W.
    // Convert their difference from project meters to source centimeters.
    const float sceneProjectionW = g_EffectSceneDepthTexture.SampleLevel(
        EffectSliceDepthSampler, pixelPosition / float2(width, height), 0.f).y * 1000.f;
    const float fadeDistanceCm = max(100.f * (1.f - g_SourceScalars0.w), 0.001f);
    const float depthFade = saturate((sceneProjectionW - projectionW) *
        100.f / fadeDistanceCm);

    const float2 radiusUV = float2(1.f - uv.x, uv.y) * 2.f - 1.f;
    const float radialDistance = length(radiusUV) - dynamicParameter.x + signedSlice;
    const float light = min(0.1f / abs(radialDistance), 1.f);
    output.SceneColor.rgb = light * particleColor.rgb * g_EmissiveIntensity;
    output.SceneColor.a = coverage * depthFade;
    // Native engine opacity=1, selection=0 and identity fog are explicit
    // project inputs. This RT0 adapter does not fabricate auxiliary MRTs or
    // the material's separate distortion pass from its stored distortion=50.
    if (g_ColorClip > 0.f)
        clip(output.SceneColor.a - g_ColorClip);
    return output;
}

#endif
