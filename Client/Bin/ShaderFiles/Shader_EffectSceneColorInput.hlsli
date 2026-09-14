#ifndef EFFECT_SCENE_COLOR_INPUT_HLSLI
#define EFFECT_SCENE_COLOR_INPUT_HLSLI

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



#endif
