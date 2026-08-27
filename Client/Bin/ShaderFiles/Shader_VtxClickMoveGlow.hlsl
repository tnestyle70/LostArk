#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
texture2D g_CoverageTexture;
float4 g_TintLinear;

sampler CoverageSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

struct VS_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    output.position = mul(float4(input.position, 1.f),
        mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix));
    output.uv = input.uv;
    return output;
}

float4 PS_MAIN(VS_OUT input) : SV_TARGET0
{
    // Same grayscale-mask convention as Shader_VtxSkillGroundTargetPreview:
    // R is explicit coverage. BS_Additive is SrcAlpha/One, so the GPU blend
    // unit itself scales RGB by the alpha returned here -- do not
    // premultiply manually or the glow would darken twice.
    const float coverage = saturate(
        g_CoverageTexture.Sample(CoverageSampler, input.uv).r);
    clip(coverage - (1.f / 255.f));
    return float4(g_TintLinear.rgb, g_TintLinear.a * coverage);
}

technique11 DefaultTechnique
{
    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
