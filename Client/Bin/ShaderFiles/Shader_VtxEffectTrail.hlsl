#include "Shader_EffectCommon.hlsli"
#include "Shader_Artist31470RuntimeMaterial.hlsli"
#include "Shader_Artist31470Active003RibbonMaterial.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

struct VS_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float2 runtimeUV : TEXCOORD1;
    float4 color : COLOR0;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    output.position = mul(
        float4(input.position, 1.f),
        mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix));
    output.uv = input.uv * g_UVScale + g_UVOffset;
    output.runtimeUV = input.uv;
    output.color = input.color;
    return output;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input)
{
    if (0u != g_RuntimeMaterialV2Enabled)
    {
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_ACTIVE003_RIBBON)
        {
            return Shade_Artist31470Active003RibbonMaterial(
                input.runtimeUV, input.color);
        }

        EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
        clip(-1.f);
        return output;
    }
    return Shade_Effect(input.uv, float3(1.f, 1.f, 1.f), input.color);
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
