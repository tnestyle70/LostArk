#ifndef LOSTARK_EFFECT_RUNTIME_MATERIAL_PACKET_HLSLI
#define LOSTARK_EFFECT_RUNTIME_MATERIAL_PACKET_HLSLI

/*
 * Class-neutral runtime material packet shared by typed effect-family
 * adapters.  Admission and packet population are owned by the visual-program
 * data; shader adapters consume only this validated ABI.
 */
uint g_RuntimeMaterialV2Enabled = 0u;
uint g_RuntimeMaterialV2Opcode = 0u;
uint g_RuntimeMaterialV2TextureLaneCount = 0u;
uint g_RuntimeMaterialV2TextureMask = 0u;
uint g_RuntimeMaterialV2DynamicConsumedMask = 0u;
uint g_RuntimeMaterialV2DynamicSuppressedMask = 0u;
uint g_RuntimeMaterialV2ParticleColorPolicy = 0u;
uint g_RuntimeMaterialV2ParticleColorConsumedMask = 0u;
uint g_RuntimeMaterialV2ParticleColorSuppressedMask = 0u;
uint g_RuntimeMaterialV2ScalarCount = 0u;
uint g_RuntimeMaterialV2VectorCount = 0u;
uint g_RuntimeMaterialV2InputCount = 0u;
uint2 g_RuntimeMaterialV2InputConsumedMask = uint2(0u, 0u);
uint2 g_RuntimeMaterialV2InputSuppressedMask = uint2(0u, 0u);
uint g_RuntimeMaterialV2StaticInputCount = 0u;
uint g_RuntimeMaterialV2StaticSelectedMask = 0u;
uint g_RuntimeMaterialV2StaticConsumedMask = 0u;
uint g_RuntimeMaterialV2StaticSuppressedMask = 0u;
uint g_RuntimeMaterialV2RenderInputCount = 0u;
uint g_RuntimeMaterialV2RenderConsumedMask = 0u;
uint g_RuntimeMaterialV2RenderSuppressedMask = 0u;
float4 g_RuntimeMaterialV2ScalarBlocks[13];
float4 g_RuntimeMaterialV2Vectors[3];
uint3 g_RuntimeMaterialV2VectorComponentConsumedMask = uint3(0u, 0u, 0u);
uint3 g_RuntimeMaterialV2VectorComponentSuppressedMask = uint3(0u, 0u, 0u);
float g_RuntimeMaterialV2NormalizedLife = 0.f;

SamplerState g_RuntimeMaterialV2Sampler0 : register(s5);
SamplerState g_RuntimeMaterialV2Sampler1 : register(s6);
SamplerState g_RuntimeMaterialV2Sampler2 : register(s7);
SamplerState g_RuntimeMaterialV2Sampler3 : register(s8);
SamplerState g_RuntimeMaterialV2Sampler4 : register(s9);
SamplerState g_RuntimeMaterialV2Sampler5 : register(s10);

#endif
