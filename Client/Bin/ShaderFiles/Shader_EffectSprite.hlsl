#include "Engine_Shader_Defines.hlsli"

float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float4x4 g_ViewMatrixInverse;

texture2D g_Texture;
float4 g_vParticleCenter;
float4 g_vParticleSize;
float4 g_vUVRect;
float4 g_vTint;
float g_fRotation;

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
    float2 vTexcoord : TEXCOORD0;
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

    float s = sin(g_fRotation);
    float c = cos(g_fRotation);
    float2 corner = In.vPosition.xy * g_vParticleSize.xy;
    corner = float2(
        corner.x * c - corner.y * s,
        corner.x * s + corner.y * c);

    float3 cameraRight = normalize(
        float3(
            g_ViewMatrixInverse._11,
            g_ViewMatrixInverse._12,
            g_ViewMatrixInverse._13));
    float3 cameraUp = normalize(
        float3(
            g_ViewMatrixInverse._21,
            g_ViewMatrixInverse._22,
            g_ViewMatrixInverse._23));

    float3 worldPosition =
        g_vParticleCenter.xyz +
        cameraRight * corner.x +
        cameraUp * corner.y;

    float4 viewPosition = mul(
        float4(worldPosition, 1.f),
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
        g_vUVRect,
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
