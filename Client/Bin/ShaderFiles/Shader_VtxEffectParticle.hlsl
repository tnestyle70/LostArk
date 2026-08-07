#include "Shader_EffectCommon.hlsli"

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
    output.color = input.color;
    output.dynamicParameter = input.dynamicParameter;
    output.particleData = input.particleData;
    return output;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input)
{
    EFFECT_PS_OUT current = Shade_EffectParticle(
        input.uv, float3(1.f, 1.f, 1.f), input.color,
        input.dynamicParameter);
    const float blend = saturate(input.particleData.y);
    if (blend <= 0.f)
        return current;
    const EFFECT_PS_OUT next = Shade_EffectParticle(
        input.uvNext, float3(1.f, 1.f, 1.f), input.color,
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
