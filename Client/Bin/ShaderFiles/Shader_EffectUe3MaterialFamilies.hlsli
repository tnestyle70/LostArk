#ifndef LOSTARK_SHADER_EFFECT_UE3_MATERIAL_FAMILIES_HLSLI
#define LOSTARK_SHADER_EFFECT_UE3_MATERIAL_FAMILIES_HLSLI

/*
 * Shared UE3 Material-family dispatch.
 *
 * Runtime documents select only a fixed opcode and a validated
 * RuntimeMaterialV2 packet.  No runtime-authored HLSL or graph bytecode is
 * accepted here.  The first cross-class family reuses the recovered
 * fx_m_pa_spritewave_01_tr equation that was originally admitted for Artist
 * 31470 active-011.
 */
#include "Shader_Artist31470Active011OuterMaterial.hlsli"

static const uint RUNTIME_MATERIAL_V2_UE3_SPRITEWAVE_TR = 15u;

EFFECT_PS_OUT Shade_EffectUe3SpriteWaveTrParticle(
    float2 localUV,
    float4 particleColor,
    float4 dynamicParameter)
{
    return Shade_Artist31470Active011OuterMaterial(
        localUV,
        float3(0.f, 0.f, 0.f),
        float3(0.f, 1.f, 0.f),
        float3(0.f, 0.f, 0.f),
        particleColor,
        dynamicParameter);
}

#endif
