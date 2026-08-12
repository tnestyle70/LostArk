#ifndef SHADER_ARTIST31470_ACTIVE011_OUTER_MATERIAL_HLSLI
#define SHADER_ARTIST31470_ACTIVE011_OUTER_MATERIAL_HLSLI

/*
 * SOURCE_SHADERMAP_DXBC_REPLAYED_CANDIDATE for Artist F active-011 RT0.
 *
 * The same active FStaticParameterSet, LocalVertexFactory base-pass shader,
 * uniform-expression set and native register bindings were recovered from two
 * official exact-source cache cohorts.  This helper translates that recovered
 * RT0 program onto the frozen RuntimeMaterial v2 packet.  c0.x and pass fog
 * remain explicit neutral external inputs; particleColor is the correlated c1
 * renderer carrier and dynamicParameter is exact uniform vector expression c3.
 */
EFFECT_PS_OUT Shade_Artist31470Active011OuterMaterial(
    float2 sourceUV,
    float3 worldPosition,
    float3 worldNormal,
    float3 cameraPosition,
    float4 particleColor,
    float4 dynamicParameter)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (g_RuntimeMaterialV2TextureLaneCount != 4u ||
        g_RuntimeMaterialV2TextureMask != 0x0fu ||
        g_RuntimeMaterialV2DynamicConsumedMask != 0x07u ||
        g_RuntimeMaterialV2DynamicSuppressedMask != 0x08u ||
        g_RuntimeMaterialV2ParticleColorPolicy !=
            RUNTIME_MATERIAL_V2_PARTICLE_COLOR_RGBA_CONSUMED_V1 ||
        g_RuntimeMaterialV2ParticleColorConsumedMask != 0x0fu ||
        g_RuntimeMaterialV2ParticleColorSuppressedMask != 0u ||
        g_RuntimeMaterialV2ScalarCount != 47u ||
        g_RuntimeMaterialV2VectorCount != 1u ||
        g_RuntimeMaterialV2InputCount != 55u ||
        any(g_RuntimeMaterialV2InputConsumedMask !=
            uint2(0xffffffffu, 0x0000007fu)) ||
        any(g_RuntimeMaterialV2InputSuppressedMask !=
            uint2(0u, 0x007fff80u)) ||
        any(g_RuntimeMaterialV2VectorComponentConsumedMask !=
            uint3(0x07u, 0u, 0u)) ||
        any(g_RuntimeMaterialV2VectorComponentSuppressedMask !=
            uint3(0x08u, 0u, 0u)) ||
        g_RuntimeMaterialV2StaticInputCount != 9u ||
        g_RuntimeMaterialV2StaticSelectedMask != 0x1efu ||
        g_RuntimeMaterialV2StaticConsumedMask != 0x1ffu ||
        g_RuntimeMaterialV2StaticSuppressedMask != 0u ||
        g_RuntimeMaterialV2RenderInputCount != 6u ||
        g_RuntimeMaterialV2RenderConsumedMask != 0x2fu ||
        g_RuntimeMaterialV2RenderSuppressedMask != 0x10u)
    {
        clip(-1.f);
        return output;
    }

    const float4 b0 = g_RuntimeMaterialV2ScalarBlocks[0];
    const float4 b1 = g_RuntimeMaterialV2ScalarBlocks[1];
    const float4 b2 = g_RuntimeMaterialV2ScalarBlocks[2];
    const float4 b3 = g_RuntimeMaterialV2ScalarBlocks[3];
    const float4 b4 = g_RuntimeMaterialV2ScalarBlocks[4];
    const float4 b5 = g_RuntimeMaterialV2ScalarBlocks[5];
    const float4 b6 = g_RuntimeMaterialV2ScalarBlocks[6];
    const float4 b7 = g_RuntimeMaterialV2ScalarBlocks[7];
    const float2 b8 = g_RuntimeMaterialV2ScalarBlocks[8].xy;
    const float4 edgeColor = g_RuntimeMaterialV2Vectors[0];

    if (!all(isfinite(sourceUV)) ||
        !all(isfinite(particleColor)) ||
        !all(isfinite(dynamicParameter)) ||
        !all(isfinite(b0)) || !all(isfinite(b1)) ||
        !all(isfinite(b2)) || !all(isfinite(b3)) ||
        !all(isfinite(b4)) || !all(isfinite(b5)) ||
        !all(isfinite(b6)) || !all(isfinite(b7)) ||
        !all(isfinite(b8)) ||
        !all(isfinite(edgeColor)) ||
        !isfinite(g_EffectLocalTime))
    {
        clip(-1.f);
        return output;
    }

    const float mainAngleA =
        3.14000010490417480469f * b5.y * 0.25f;
    const float mainAngleB =
        b5.y * 6.28000020980834960938f * 0.25f;
    const float dissolveAngle = b1.y * 0.25f;
    const float sineA = sin(mainAngleA);
    const float cosineA = cos(mainAngleA);
    const float sineB = sin(mainAngleB);
    const float cosineB = cos(mainAngleB);
    const float sineD = sin(dissolveAngle);
    const float cosineD = cos(dissolveAngle);

    const float2 uvNoiseUV = float2(
        g_EffectLocalTime * b6.y + sourceUV.x * b6.w + b7.y,
        sourceUV.y * b7.x + g_EffectLocalTime * b6.z + b7.z);
    const float4 uvNoiseSample = g_SourceTexture1.Sample(
        g_RuntimeMaterialV2Sampler1, uvNoiseUV);
    const float2 rawMainOffset =
        uvNoiseSample.r * (dynamicParameter.z + b6.x) + b4.zw;

    const float2 centeredSource = sourceUV - float2(0.5f, 0.5f);
    float2 mainBaseUV = float2(
        dot(float2(cosineA, -sineA), centeredSource),
        dot(float2(sineA, cosineA), centeredSource));
    mainBaseUV += dynamicParameter.x * b4.xy + float2(0.5f, 0.5f);
    mainBaseUV *= b5.zw;
    mainBaseUV += g_EffectLocalTime * float2(0.f, b5.x);
    const float2 mainCentered = rawMainOffset + mainBaseUV -
        float2(0.5f, 0.5f);
    const float2 mainUV = float2(
        dot(float2(cosineB, -sineB), mainCentered),
        dot(float2(sineB, cosineB), mainCentered)) +
        float2(0.5f, 0.5f);
    const float4 mainSample = g_SourceTexture0.SampleLevel(
        g_RuntimeMaterialV2Sampler0, mainUV, -1.f);

    const float2 dissolveBaseUV = float2(
        dot(float2(cosineD, -sineD), centeredSource),
        dot(float2(sineD, cosineD), centeredSource)) +
        float2(0.5f, 0.5f);
    const float2 dissolveNoiseUV = float2(
        sourceUV.x * b2.z + g_EffectLocalTime * b2.x,
        sourceUV.y * b2.w + g_EffectLocalTime * b2.y);
    const float4 dissolveNoiseSample = g_SourceTexture3.Sample(
        g_RuntimeMaterialV2Sampler3, dissolveNoiseUV);
    const float2 dissolveShift = dissolveBaseUV +
        dissolveNoiseSample.r * b1.w;
    const float2 dissolveUV = float2(
        dissolveShift.x * b0.z + g_EffectLocalTime * b0.x,
        dissolveShift.y * b0.w + g_EffectLocalTime * b0.y);
    const float4 dissolveSample = g_SourceTexture2.Sample(
        g_RuntimeMaterialV2Sampler2, dissolveUV);
    const float dissolveCarrier = saturate(
        dissolveSample.r + 1.1f - dynamicParameter.y);

    const float radialBase = max(1.f - 2.f * length(centeredSource), 0.f) *
        b7.w;
    const float radial = min(max(radialBase, b8.y), b8.x);
    const float alpha = radial * saturate(dissolveCarrier * b1.x *
        mainSample.r * b3.w * particleColor.a);

    const float corePower = abs(mainSample.r) < 1.0e-6f ? 0.f :
        pow(abs(mainSample.r), b3.y);
    const float core = corePower * b3.z + mainSample.r * b3.x;
    const float edgePower = abs(dissolveCarrier) < 1.0e-6f ? 0.f :
        pow(abs(dissolveCarrier), b1.z);
    const float edge = max(saturate(dissolveCarrier * b1.x) -
        saturate(edgePower * b1.x), 0.f);
    const float3 rgb = (core.xxx + edge * edgeColor.rgb) * particleColor.rgb;

    if (!all(isfinite(uvNoiseSample)) || !all(isfinite(mainSample)) ||
        !all(isfinite(dissolveSample)) ||
        !all(isfinite(dissolveNoiseSample)) ||
        !all(isfinite(float4(rgb, alpha))))
    {
        clip(-1.f);
        return output;
    }

    output.SceneColor = float4(rgb, alpha);
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return output;
}

#endif
