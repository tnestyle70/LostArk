#ifndef EFFECT_CUBESAMPLE_SCENE_HLSLI
#define EFFECT_CUBESAMPLE_SCENE_HLSLI
#include "Shader_EffectSceneColorInput.hlsli"

float CubeSample_SourcePow(float value, float exponent)
{
    // Native log/exp branch returns zero even for the near-zero ^0 case.
    return abs(value) < 1.0e-6f ? 0.f : pow(abs(value), exponent);
}

// Limit only transmitted background radiance. Authored self-emission and
// the original caustic/edge terms retain their HDR range.
float CubeSample_ClarityTransmissionScale(float3 radiance, float ceiling)
{
    const float peak = max(radiance.r, max(radiance.g, radiance.b));
    const float excess = max(peak - 0.35f, 0.f);
    const float shoulder = ceiling - 0.35f;
    const float compressed = 0.35f + shoulder * (excess / (excess + shoulder));
    return peak > 0.35f ? compressed / peak : 1.f;
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
    const float sourceAlpha = saturate(particleColor.a);
    output.SceneColor.a = sourceAlpha;
    if (g_SourceScalars0.w > 0.f)
    {
        // Opt-in only: all scene/bloom/black evaluations use the same actual
        // HDR scene to choose transmission. Suppressed scene
        // bloom is never reconstructed from that HDR sample.
        const float strength = saturate(g_SourceScalars0.w);
        const float3 actualScene = min(max(g_EffectSceneColorTexture.Sample(
            LinearClampUVSampler, sceneUV).rgb, 0.f), 60000.f);
        const float scenePeak = max(actualScene.r, max(actualScene.g, actualScene.b));
        const float sceneWeight = smoothstep(0.15f, 0.8f, scenePeak);
        const float3 actualSquared = actualScene * actualScene;
        const float3 nativeTransmission = (5.f * actualSquared * actualSquared *
            grazing + facing) * g_SourceVector0.rgb *
            g_SourceVector0.a * g_EmissiveIntensity;
        const float bodyScale = CubeSample_ClarityTransmissionScale(
            actualScene * nativeTransmission, 0.65f + 0.45f * facing);
        // Source-tone compression can wash a pale HDR tint to white. Use the
        // authored particle chroma for absorption as well as self-emission.
        const float3 particleRadiance = max(particleColor.rgb, 0.f);
        const float particlePeak = max(particleRadiance.r,
            max(particleRadiance.g, particleRadiance.b));
        const float3 particleChroma = particleRadiance / max(particlePeak, 1.e-4f);
        const float3 transmission = nativeTransmission * particleChroma *
            ((1.f - sceneWeight) + sceneWeight * bodyScale);
        const float3 nativeCaustic = caustic * g_SourceVector0.rgb *
            g_SourceVector0.a * g_EmissiveIntensity;
        // The source HDR particle color supplies the gold hue. This authored
        // face term is independent of SceneColor, so it remains in F(black)
        // and participates in the existing per-effect bloom extraction.
        const float faceEmissionWeight = 0.0875f + 0.0525f * grazing * grazing;
        const float3 faceEmission = particleRadiance * particleChroma * g_SourceVector0.rgb *
            g_SourceVector0.a * g_EmissiveIntensity * faceEmissionWeight;
        const float3 clearColor = scene * transmission + nativeCaustic +
            edge * particleColor.rgb * g_EmissiveIntensity + faceEmission;
        // Weighted sum avoids cancellation against the legacy C^5 output at
        // strength1. Only the opted-in contribution transports bloom linearly.
        output.SceneColor.rgb = (1.f - strength) * output.SceneColor.rgb +
            strength * clearColor;
        // Reduce unfiltered background leakage only on bright scenes. Preserve
        // fade endpoints and the original alpha-test threshold below.
        output.SceneColor.a += strength * smoothstep(0.5f, 4.f, scenePeak) *
            sourceAlpha * (1.f - sourceAlpha);
    }
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    if (g_ColorClip > 0.f)
        clip(sourceAlpha - g_ColorClip);
    return output;
}

#endif
