#ifndef EFFECT_CUBESAMPLE_SCENE_HLSLI
#define EFFECT_CUBESAMPLE_SCENE_HLSLI

Texture2D g_EffectSceneColorTexture;

Texture2D g_EffectSceneBloomTexture;
// Invocation-local mode: original HDR, transported bloom, zero-RGB emission baseline.
float4 Read_EffectSceneColor(SamplerState sampleState, float2 uv)
{
    g_EffectSceneSampleUsed = true;
    const float4 scene = g_EffectSceneColorTexture.Sample(sampleState, uv);
    if (g_EffectSceneReadMode == 1u)
        return float4(g_EffectSceneBloomTexture.Sample(sampleState, uv).rgb, scene.a);
    return g_EffectSceneReadMode == 2u ? float4(0.f, 0.f, 0.f, scene.a) : scene;
}
float4 Read_EffectSceneColorBias(SamplerState sampleState, float2 uv, float bias)
{
    g_EffectSceneSampleUsed = true;
    const float4 scene = g_EffectSceneColorTexture.SampleBias(sampleState, uv, bias);
    if (g_EffectSceneReadMode == 1u)
        return float4(g_EffectSceneBloomTexture.SampleBias(sampleState, uv, bias).rgb, scene.a);
    return g_EffectSceneReadMode == 2u ? float4(0.f, 0.f, 0.f, scene.a) : scene;
}
float4 Read_EffectSceneColorLevel(SamplerState sampleState, float2 uv, float level)
{
    g_EffectSceneSampleUsed = true;
    const float4 scene = g_EffectSceneColorTexture.SampleLevel(sampleState, uv, level);
    if (g_EffectSceneReadMode == 1u)
        return float4(g_EffectSceneBloomTexture.SampleLevel(sampleState, uv, level).rgb, scene.a);
    return g_EffectSceneReadMode == 2u ? float4(0.f, 0.f, 0.f, scene.a) : scene;
}



float CubeSample_SourcePow(float value, float exponent)
{
    // Native log/exp branch returns zero even for the near-zero ^0 case.
    return abs(value) < 1.0e-6f ? 0.f : pow(abs(value), exponent);
}

EFFECT_PS_OUT Shade_EffectCubeSampleScene(float2 meshUV,
    float2 pixelPosition, float3 tangentView, float4 particleColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    uint width = 0u;
    uint height = 0u;
    g_EffectSceneColorTexture.GetDimensions(width, height);
    if (g_SourceTextureMask != 1u || width == 0u || height == 0u ||
        !all(isfinite(tangentView)) || dot(tangentView, tangentView) < 1.0e-12f)
    {
        clip(-1.f);
        return output;
    }

    // PS f548f39885bbfe42ac405ccd46dc2919: selected unclamped CubeSample.
    // SceneHDR snapshot and particleColor replace engine-owned scene/prefix
    // inputs. Native fog and auxiliary MRT2-5 are outside this RT0 adapter.
    const float3 V = normalize(tangentView);
    const float facing = abs(V.z);
    const float grazing = 1.f - facing;
    const float2 sceneUV = pixelPosition / float2(width, height) +
        0.1f * g_SourceVector1.x * grazing * V.yx;
    const float3 scene = Read_EffectSceneColor(
        LinearClampUVSampler, sceneUV).rgb;
    const float3 powerBase = max(abs(scene), 1.0e-6f);
    const float3 power2 = powerBase * powerBase;
    const float3 power5 = power2 * power2 * powerBase;
    const float3 refracted = lerp(5.f * power5, scene, facing);
    const float3 caustic = Sample_SourceTexture0(V.xy * 3.f).rgb *
        CubeSample_SourcePow(grazing, 7.f) * g_SourceScalars0.x;
    const float2 edgeUV = 2.f * (meshUV - 0.5f);
    const float edge = min(1.f,
        CubeSample_SourcePow(edgeUV.x, g_SourceScalars0.y) +
        CubeSample_SourcePow(edgeUV.y, g_SourceScalars0.y)) * g_SourceScalars0.z;

    // Mesh particle RGB owns the edge term; multiplying all body RGB by it
    // would apply the source color-over-life gain to the wrong expression.
    output.SceneColor.rgb = ((refracted + caustic) *
        g_SourceVector0.rgb * g_SourceVector0.a + edge * particleColor.rgb) *
        g_EmissiveIntensity;
    output.SceneColor.a = saturate(particleColor.a);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    if (g_ColorClip > 0.f)
        clip(output.SceneColor.a - g_ColorClip);
    return output;
}

#endif
