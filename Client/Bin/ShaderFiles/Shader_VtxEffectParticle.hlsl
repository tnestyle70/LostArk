#include "Shader_EffectCommon.hlsli"
#include "Shader_Artist31470RuntimeMaterial.hlsli"
#include "Shader_EffectStandardColorV1.hlsli"
#include "Shader_EffectUe3MaterialFamilies.hlsli"
#include "Shader_Artist31470Diagnostic.hlsli"

float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

struct VS_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 world0 : WORLD0;
    float4 world1 : WORLD1;
    float4 world2 : WORLD2;
    float4 world3 : WORLD3;
    float4 color : COLOR0;
    float4 dynamicParameter : DYNAMIC0;
    float4 uvTransform : UVTRANSFORM0;
    float4 uvTransformNext : UVTRANSFORM1;
    float2 particleData : PARTICLEDATA0;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float4 dynamicParameter : TEXCOORD1;
    float2 uvNext : TEXCOORD2;
    float2 particleData : TEXCOORD3;
    float2 localUV : TEXCOORD4;
    float2 runtimeLocalUV : TEXCOORD5;
    float4 runtimeSubUVTransform : TEXCOORD6;
    float4 runtimeSubUVTransformNext : TEXCOORD7;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    const float4x4 world = float4x4(
        input.world0, input.world1, input.world2, input.world3);
    output.position = mul(
        float4(input.position, 1.f),
        mul(mul(world, g_ViewMatrix), g_ProjMatrix));
    output.uv =
        (input.uv * input.uvTransform.xy + input.uvTransform.zw) *
        g_UVScale + g_UVOffset;
    output.uvNext =
        (input.uv * input.uvTransformNext.xy + input.uvTransformNext.zw) *
        g_UVScale + g_UVOffset;
    output.localUV = input.uv * g_UVScale + g_UVOffset;
    output.runtimeLocalUV = input.uv;
    output.runtimeSubUVTransform = input.uvTransform;
    output.runtimeSubUVTransformNext = input.uvTransformNext;
    output.color = input.color;
    output.dynamicParameter = input.dynamicParameter;
    output.particleData = input.particleData;
    return output;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input)
{
    if (0u != g_StandardColorV1Enabled)
    {
        return Shade_EffectStandardColorV1Particle(
            input.uv, input.uvNext, input.particleData.y, input.color);
    }
    if (0u != g_RuntimeMaterialV2Enabled)
    {
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_GLASSHOLE02_SPRITE_K01)
        {
            return Shade_EffectUe3Glasshole02K01Particle(
                input.runtimeLocalUV, input.color,
                input.dynamicParameter);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_SPRITEWAVE_TR)
        {
            return Shade_EffectUe3SpriteWaveTrParticle(
                input.runtimeLocalUV, input.color,
                input.dynamicParameter);
        }
        return Shade_RuntimeMaterialV2Particle(
            input.runtimeLocalUV, input.runtimeSubUVTransform,
            input.runtimeSubUVTransformNext, input.particleData.y,
            input.color, input.dynamicParameter);
    }
    if (0u != g_ArtistVisualV4Opcode ||
        0u != g_ReconstructedMaterialEvaluatorEnabled)
    {
        return Shade_ReconstructedMaterial(
            input.uv, float3(1.f, 1.f, 1.f), input.color,
            input.dynamicParameter);
    }
    EFFECT_PS_OUT current = Shade_EffectParticleUV(
        input.uv, input.localUV, float3(1.f, 1.f, 1.f), input.color,
        input.dynamicParameter);
    const float blend = saturate(input.particleData.y);
    if (blend <= 0.f)
        return current;
    const EFFECT_PS_OUT next = Shade_EffectParticleUV(
        input.uvNext, input.localUV, float3(1.f, 1.f, 1.f), input.color,
        input.dynamicParameter);
    current.SceneColor = lerp(current.SceneColor, next.SceneColor, blend);
    current.Distortion = lerp(current.Distortion, next.Distortion, blend);
    return current;
}

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_EffectOpaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AdditiveOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
