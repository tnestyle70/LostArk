#include "Shader_EffectCommon.hlsli"
#include "Shader_Artist31470RuntimeMaterial.hlsli"
#include "Shader_Artist31470Diagnostic.hlsli"
#include "Shader_Artist31470Active011OuterMaterial.hlsli"
#include "Shader_EffectUe3MaterialFamilies.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_NormalMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float4 g_CameraPosition;
uint g_UseBaseOverride = 0;
float4 g_EffectDynamicParameter = float4(0.f, 0.f, 0.f, 0.f);

struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    const float4 worldPosition = mul(
        float4(input.position, 1.f), g_WorldMatrix);
    output.position = mul(
        worldPosition, mul(g_ViewMatrix, g_ProjMatrix));
    output.worldPosition = worldPosition.xyz;
    output.normal = normalize(
        mul(float4(input.normal, 0.f), g_NormalMatrix).xyz);
    const float3 transformedTangent =
        mul(float4(input.tangent, 0.f), g_WorldMatrix).xyz;
    output.tangent = normalize(transformedTangent -
        output.normal * dot(transformedTangent, output.normal));
    const float handedness = dot(
        cross(input.normal, input.tangent), input.binormal) < 0.f ? -1.f : 1.f;
    output.binormal = normalize(cross(output.normal, output.tangent)) * handedness;
    output.uv = input.uv * g_UVScale + g_UVOffset;
    return output;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input)
{
    if (0u != g_RuntimeMaterialV2Enabled)
    {
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_ACTIVE011_OUTER_MESH)
        {
            return Shade_Artist31470Active011OuterMaterial(
                input.uv, input.worldPosition, input.normal,
                g_CameraPosition.xyz,
                g_ColorMultiply + g_ColorOffset,
                g_EffectDynamicParameter);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_DRAGON_PH_MASKED_MESH)
        {
            return Shade_EffectUe3DragonPhMaskedMesh(
                input.uv, input.worldPosition, input.normal,
                input.tangent, input.binormal, g_CameraPosition.xyz,
                g_ColorMultiply + g_ColorOffset,
                g_EffectDynamicParameter);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_WPO_SINWAVE_ELECTRIC_RT0_MESH)
        {
            return Shade_EffectUe3WpoSinWaveElectricRt0Mesh(
                input.uv,
                g_ColorMultiply + g_ColorOffset,
                g_EffectDynamicParameter);
        }
        return Shade_RuntimeMaterialV2Mesh(
            input.uv, input.worldPosition, input.normal,
            input.tangent, input.binormal, g_CameraPosition.xyz,
            g_ColorMultiply + g_ColorOffset,
            g_EffectDynamicParameter);
    }
    if (0u != g_ArtistVisualV4Opcode ||
        0u != g_ReconstructedMaterialEvaluatorEnabled)
    {
        return Shade_ReconstructedMaterial(
            input.uv, float3(1.f, 1.f, 1.f),
            float4(1.f, 1.f, 1.f, 1.f), g_EffectDynamicParameter);
    }
    if (9 == g_SourceMaterialProfile)
    {
        return Shade_LocalCrackMesh(
            input.uv, input.worldPosition, input.normal,
            input.tangent, input.binormal, g_CameraPosition.xyz,
            float4(1.f, 1.f, 1.f, 1.f),
            g_EffectDynamicParameter);
    }
    return Shade_EffectParticle(
        input.uv,
        float3(1.f, 1.f, 1.f),
        float4(1.f, 1.f, 1.f, 1.f),
        g_EffectDynamicParameter);
}

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Default);
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
    pass AlphaOneSidedMirroredDepthRead
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AdditiveOneSidedMirroredDepthRead
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
