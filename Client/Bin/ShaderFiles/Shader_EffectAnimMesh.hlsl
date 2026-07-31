#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
matrix g_BoneMatrices[512];

Texture2D g_Texture;
float4 g_vTint;

#include "Shader_EffectMaterial.hlsli"

RasterizerState RS_EffectCullNone
{
    FillMode = Solid;
    CullMode = None;
    FrontCounterClockwise = false;
};

DepthStencilState DSS_EffectReadOnly
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less_equal;
};

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    uint4 vBlendIndices : BLENDINDEX;
    float4 vBlendWeights : BLENDWEIGHT;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float fViewZ : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;

    matrix BoneMatrix =
        g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x +
        g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y +
        g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z +
        g_BoneMatrices[In.vBlendIndices.w] * In.vBlendWeights.w;

    float4 skinnedPosition = mul(
        float4(In.vPosition, 1.f),
        BoneMatrix);
    float4 worldPosition = mul(
        skinnedPosition,
        g_WorldMatrix);
    float4 viewPosition = mul(
        worldPosition,
        g_ViewMatrix);
    Out.vPosition = mul(viewPosition, g_ProjMatrix);
    Out.vTexcoord = In.vTexcoord;
    Out.fViewZ = viewPosition.z;
    return Out;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT In)
{
    return ShadeEffectMaterial(
        In.vTexcoord,
        float4(0.f, 0.f, 1.f, 1.f),
        g_vTint,
        In.fViewZ,
        In.vPosition);
}

technique11 DefaultTechnique
{
    pass Alpha
    {
        SetRasterizerState(RS_EffectCullNone);
        SetDepthStencilState(DSS_EffectReadOnly, 0);
        SetBlendState(
            BS_EffectAlphaMRT,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Additive
    {
        SetRasterizerState(RS_EffectCullNone);
        SetDepthStencilState(DSS_EffectReadOnly, 0);
        SetBlendState(
            BS_EffectAdditiveMRT,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
