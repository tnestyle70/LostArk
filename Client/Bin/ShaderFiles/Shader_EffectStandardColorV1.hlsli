#ifndef LOSTARK_EFFECT_STANDARD_COLOR_V1_HLSLI
#define LOSTARK_EFFECT_STANDARD_COLOR_V1_HLSLI

#include "Shader_EffectRuntimeMaterialPacket.hlsli"

/*
 * Opt-in, class-neutral color/coverage ABI for standard SpriteParticle,
 * DecalParticle, and Trail carriers.  This packet deliberately does not share
 * RuntimeMaterialV2's opcode namespace.  The CPU admission stage owns exact
 * lane color-space, source-channel, SRV-format, and sampler validation; this
 * shader repeats the structural closure and fails closed before sampling.
 */
uint g_StandardColorV1Enabled = 0u;
uint4 g_StandardColorV1Header = uint4(0u, 0u, 0u, 0u);
uint4 g_StandardColorV1BaseCoverage = uint4(0u, 0u, 0u, 0u);
uint4 g_StandardColorV1Dissolve = uint4(0u, 0u, 0u, 0u);
uint4 g_StandardColorV1Policies = uint4(0u, 0u, 0u, 0u);
float4 g_StandardColorV1Scalars = float4(0.f, 0.f, 0.f, 0.f);

static const uint STANDARD_COLOR_V1_PACKET_VERSION = 1u;
static const uint STANDARD_COLOR_V1_OPCODE = 1u;
static const uint STANDARD_COLOR_V1_CHANNEL_INVALID = 0u;
static const uint STANDARD_COLOR_V1_CHANNEL_R = 1u;
static const uint STANDARD_COLOR_V1_CHANNEL_G = 2u;
static const uint STANDARD_COLOR_V1_CHANNEL_B = 3u;
static const uint STANDARD_COLOR_V1_CHANNEL_A = 4u;
static const uint STANDARD_COLOR_V1_CHANNEL_RGB = 5u;
static const uint STANDARD_COLOR_V1_EMISSIVE_NONE = 0u;
static const uint STANDARD_COLOR_V1_EMISSIVE_BASE_RADIANCE = 1u;
static const uint STANDARD_COLOR_V1_LIFETIME_CARRIER_ALPHA = 1u;
static const uint STANDARD_COLOR_V1_DISSOLVE_NONE = 0u;
static const uint STANDARD_COLOR_V1_DISSOLVE_LANE_THRESHOLD = 1u;
static const uint STANDARD_COLOR_V1_MISSING_LANE_FAIL_CLOSED = 1u;

bool StandardColorV1_IsScalarChannel(uint channel)
{
    return channel >= STANDARD_COLOR_V1_CHANNEL_R &&
        channel <= STANDARD_COLOR_V1_CHANNEL_A;
}

bool StandardColorV1_IsPacketValid()
{
    const uint laneCount = g_StandardColorV1Header.z;
    const uint textureMask = g_StandardColorV1Header.w;
    const uint expectedMask = laneCount > 0u && laneCount <= 6u ?
        ((1u << laneCount) - 1u) : 0u;
    const uint baseLane = g_StandardColorV1BaseCoverage.x;
    const uint baseChannel = g_StandardColorV1BaseCoverage.y;
    const uint coverageLane = g_StandardColorV1BaseCoverage.z;
    const uint coverageChannel = g_StandardColorV1BaseCoverage.w;
    const uint dissolveMode = g_StandardColorV1Dissolve.x;
    const uint dissolveLane = g_StandardColorV1Dissolve.y;
    const uint dissolveChannel = g_StandardColorV1Dissolve.z;
    const float dissolveSoftness = g_StandardColorV1Scalars.x;

    if (g_StandardColorV1Enabled != 1u ||
        g_StandardColorV1Header.x != STANDARD_COLOR_V1_PACKET_VERSION ||
        g_StandardColorV1Header.y != STANDARD_COLOR_V1_OPCODE ||
        textureMask != expectedMask ||
        g_StandardColorV1Policies.z != textureMask ||
        g_StandardColorV1Policies.w != textureMask ||
        baseLane >= laneCount || coverageLane >= laneCount ||
        (textureMask & (1u << baseLane)) == 0u ||
        (textureMask & (1u << coverageLane)) == 0u ||
        !((baseChannel >= STANDARD_COLOR_V1_CHANNEL_R &&
           baseChannel <= STANDARD_COLOR_V1_CHANNEL_B) ||
          baseChannel == STANDARD_COLOR_V1_CHANNEL_RGB) ||
        !StandardColorV1_IsScalarChannel(coverageChannel) ||
        g_StandardColorV1Policies.x >
            STANDARD_COLOR_V1_EMISSIVE_BASE_RADIANCE ||
        g_StandardColorV1Policies.y !=
            STANDARD_COLOR_V1_LIFETIME_CARRIER_ALPHA ||
        g_StandardColorV1Dissolve.w !=
            STANDARD_COLOR_V1_MISSING_LANE_FAIL_CLOSED ||
        !isfinite(dissolveSoftness) || dissolveSoftness < 0.f ||
        dissolveSoftness > 1.f ||
        any(g_StandardColorV1Scalars.yzw != float3(0.f, 0.f, 0.f)))
    {
        return false;
    }

    if (dissolveMode == STANDARD_COLOR_V1_DISSOLVE_NONE)
    {
        return dissolveLane == 0xffffffffu &&
            dissolveChannel == STANDARD_COLOR_V1_CHANNEL_INVALID &&
            dissolveSoftness == 0.f;
    }
    if (dissolveMode != STANDARD_COLOR_V1_DISSOLVE_LANE_THRESHOLD ||
        dissolveLane >= laneCount ||
        (textureMask & (1u << dissolveLane)) == 0u ||
        !StandardColorV1_IsScalarChannel(dissolveChannel))
    {
        return false;
    }
    return true;
}

float4 StandardColorV1_SampleLane(uint lane, float2 uv)
{
    switch (lane)
    {
    case 0u: return g_SourceTexture0.Sample(g_RuntimeMaterialV2Sampler0, uv);
    case 1u: return g_SourceTexture1.Sample(g_RuntimeMaterialV2Sampler1, uv);
    case 2u: return g_SourceTexture2.Sample(g_RuntimeMaterialV2Sampler2, uv);
    case 3u: return g_SourceTexture3.Sample(g_RuntimeMaterialV2Sampler3, uv);
    case 4u: return g_SourceTexture4.Sample(g_RuntimeMaterialV2Sampler4, uv);
    case 5u: return g_SourceTexture5.Sample(g_RuntimeMaterialV2Sampler5, uv);
    default: return float4(0.f, 0.f, 0.f, 0.f);
    }
}

float StandardColorV1_DecodeScalar(float4 sampleValue, uint channel)
{
    switch (channel)
    {
    case STANDARD_COLOR_V1_CHANNEL_R: return sampleValue.r;
    case STANDARD_COLOR_V1_CHANNEL_G: return sampleValue.g;
    case STANDARD_COLOR_V1_CHANNEL_B: return sampleValue.b;
    case STANDARD_COLOR_V1_CHANNEL_A: return sampleValue.a;
    default: return 0.f;
    }
}

float3 StandardColorV1_DecodeBaseRadiance(float4 sampleValue, uint channel)
{
    if (channel == STANDARD_COLOR_V1_CHANNEL_RGB)
        return sampleValue.rgb;
    return StandardColorV1_DecodeScalar(sampleValue, channel).xxx;
}

struct STANDARD_COLOR_V1_RESULT
{
    EFFECT_PS_OUT output;
};

STANDARD_COLOR_V1_RESULT StandardColorV1_Evaluate(float2 uv, float4 carrier)
{
    STANDARD_COLOR_V1_RESULT result = (STANDARD_COLOR_V1_RESULT)0;
    const float4 baseSample = StandardColorV1_SampleLane(
        g_StandardColorV1BaseCoverage.x, uv);
    const float4 coverageSample = StandardColorV1_SampleLane(
        g_StandardColorV1BaseCoverage.z, uv);
    const float3 baseRadiance = StandardColorV1_DecodeBaseRadiance(
        baseSample, g_StandardColorV1BaseCoverage.y);
    const float coverage = saturate(StandardColorV1_DecodeScalar(
        coverageSample, g_StandardColorV1BaseCoverage.w));

    float dissolveGate = 1.f;
    if (g_StandardColorV1Dissolve.x ==
        STANDARD_COLOR_V1_DISSOLVE_LANE_THRESHOLD)
    {
        const float dissolveSample = saturate(StandardColorV1_DecodeScalar(
            StandardColorV1_SampleLane(g_StandardColorV1Dissolve.y, uv),
            g_StandardColorV1Dissolve.z));
        const float softness = g_StandardColorV1Scalars.x;
        dissolveGate = softness > 0.f ?
            smoothstep(g_DissolveAmount - softness,
                g_DissolveAmount + softness, dissolveSample) :
            step(g_DissolveAmount, dissolveSample);
    }

    float3 radiance = max(float3(0.f, 0.f, 0.f),
        baseRadiance * g_ColorMultiply.rgb + g_ColorOffset.rgb) * carrier.rgb;
    if (g_StandardColorV1Policies.x ==
        STANDARD_COLOR_V1_EMISSIVE_BASE_RADIANCE)
    {
        radiance *= max(g_EmissiveIntensity, 0.f);
    }
    const float lifetime = saturate(
        (g_ColorMultiply.a + g_ColorOffset.a) * carrier.a);
    const float alpha = saturate(coverage * lifetime * dissolveGate);
    result.output.SceneColor = float4(radiance, alpha);
    result.output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    return result;
}

void StandardColorV1_ClipCoverage(float alpha)
{
    clip(alpha - max(g_ColorClip, 1.f / 255.f));
}

EFFECT_PS_OUT Shade_EffectStandardColorV1(float2 uv, float4 carrier)
{
    if (!StandardColorV1_IsPacketValid())
    {
        clip(-1.f);
        return (EFFECT_PS_OUT)0;
    }
    const STANDARD_COLOR_V1_RESULT result =
        StandardColorV1_Evaluate(uv, carrier);
    StandardColorV1_ClipCoverage(result.output.SceneColor.a);
    return result.output;
}

EFFECT_PS_OUT Shade_EffectStandardColorV1Particle(
    float2 currentUV,
    float2 nextUV,
    float blend,
    float4 carrier)
{
    if (!StandardColorV1_IsPacketValid())
    {
        clip(-1.f);
        return (EFFECT_PS_OUT)0;
    }
    const STANDARD_COLOR_V1_RESULT current =
        StandardColorV1_Evaluate(currentUV, carrier);
    const STANDARD_COLOR_V1_RESULT next =
        StandardColorV1_Evaluate(nextUV, carrier);
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor = lerp(
        current.output.SceneColor, next.output.SceneColor, saturate(blend));
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    StandardColorV1_ClipCoverage(output.SceneColor.a);
    return output;
}

#endif
