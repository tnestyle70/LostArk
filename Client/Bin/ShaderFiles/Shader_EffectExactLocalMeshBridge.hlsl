#include "Engine_Shader_Defines.hlsli"

/* Authoring-only VTXMESH -> admitted UE3 BasePass pixel-input bridge. */
float4x4 g_WorldMatrix;
float4x4 g_NormalMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float4 g_ExactColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_ExactDynamicParameter = float4(0.f, 0.f, 0.f, 0.f);

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
    float4 texcoord10 : TEXCOORD10;
    float4 texcoord11 : TEXCOORD11;
    float4 color0 : COLOR0;
    float4 color1 : COLOR1;
    float4 texcoord0 : TEXCOORD0;
    float4 texcoord4 : TEXCOORD4;
    float4 texcoord6 : TEXCOORD6;
    float4 texcoord5 : TEXCOORD5;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    const float4 worldPosition = mul(float4(input.position, 1.f), g_WorldMatrix);
    const float4 clipPosition = mul(
        worldPosition, mul(g_ViewMatrix, g_ProjMatrix));
    const float3 normal = normalize(
        mul(float4(input.normal, 0.f), g_NormalMatrix).xyz);
    output.position = clipPosition;
    output.texcoord10 = float4(normal, 1.f);
    output.texcoord11 = worldPosition;
    output.color0 = g_ExactColor;
    output.color1 = g_ExactDynamicParameter;
    output.texcoord0 = float4(input.uv, 0.f, 1.f);
    /* Helix RT0 consumes UV, fog attenuation and tangent-space view.  The
       first canary intentionally uses the bounded no-fog/front-view identity;
       the sealed PS still owns every material equation. */
    output.texcoord4 = float4(0.f, 0.f, 0.f, 1.f);
    output.texcoord6 = float4(0.f, 0.f, 1.f, 0.f);
    output.texcoord5 = clipPosition;
    return output;
}

float4 PS_BRIDGE_SENTINEL(VS_OUT input, bool frontFace : SV_IsFrontFace)
    : SV_TARGET0
{
    const float4 carrier = input.texcoord10 + input.texcoord11 +
        input.color0 + input.color1 + input.texcoord0 +
        input.texcoord4 + input.texcoord6 + input.texcoord5;
    return frontFace ? carrier : -carrier;
}

technique11 DefaultTechnique
{
    pass ExactBridge
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BRIDGE_SENTINEL();
    }
}
