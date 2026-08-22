#include "Shader_Ue3ValtanDissolve01.hlsli"

/*
 * Tool-only local-mesh-particle adapter for Masked Dissolve01 RT0.
 *
 * The translated equation preserves the source opacity-mask discard.  The
 * cooked BasePass also forwards SV_Coverage and writes MRT2/3/4/5; this
 * wrapper intentionally returns RT0 only and must not be admitted as Product
 * or coverage parity.  Missing runtime ABI inputs fail closed through the
 * explicit Tool admission/time gates.
 */
float4x4 g_WorldMatrix;
float4x4 g_NormalMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float g_Ue3ValtanDissolve01LocalTimeSeconds = -1.0f;
uint g_Ue3ValtanDissolve01ToolAdmission = 0u;

struct VALTAN_DISSOLVE01_VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD0;
};

struct VALTAN_DISSOLVE01_VS_OUT
{
    float4 tangentToWorld0 : TEXCOORD10;
    float4 tangentToWorld1 : TEXCOORD11;
    float4 meshUv : TEXCOORD0;
    float4 cameraToMesh : TEXCOORD6;
    float4 position : SV_POSITION;
};

float3 NormalizeValtanDissolve01Axis(float3 axis, float3 fallbackAxis)
{
    const float lengthSquared = dot(axis, axis);
    return lengthSquared > 1.0e-12f ?
        axis * rsqrt(lengthSquared) : fallbackAxis;
}

VALTAN_DISSOLVE01_VS_OUT VS_MAIN(VALTAN_DISSOLVE01_VS_IN input)
{
    VALTAN_DISSOLVE01_VS_OUT output;
    const float4 worldPosition =
        mul(float4(input.position, 1.0f), g_WorldMatrix);
    const float4 viewPosition = mul(worldPosition, g_ViewMatrix);
    const float4 clipPosition = mul(viewPosition, g_ProjMatrix);

    const float3 worldTangent = NormalizeValtanDissolve01Axis(
        mul(float4(input.tangent, 0.0f), g_NormalMatrix).xyz,
        float3(1.0f, 0.0f, 0.0f));
    const float3 worldBinormal = NormalizeValtanDissolve01Axis(
        mul(float4(input.binormal, 0.0f), g_NormalMatrix).xyz,
        float3(0.0f, 1.0f, 0.0f));
    const float3 worldNormal = NormalizeValtanDissolve01Axis(
        mul(float4(input.normal, 0.0f), g_NormalMatrix).xyz,
        float3(0.0f, 0.0f, 1.0f));
    const float3 viewTangent = NormalizeValtanDissolve01Axis(
        mul(float4(worldTangent, 0.0f), g_ViewMatrix).xyz,
        float3(1.0f, 0.0f, 0.0f));
    const float3 viewBinormal = NormalizeValtanDissolve01Axis(
        mul(float4(worldBinormal, 0.0f), g_ViewMatrix).xyz,
        float3(0.0f, 1.0f, 0.0f));
    const float3 viewNormal = NormalizeValtanDissolve01Axis(
        mul(float4(worldNormal, 0.0f), g_ViewMatrix).xyz,
        float3(0.0f, 0.0f, 1.0f));
    const float handedness =
        dot(cross(viewTangent, viewBinormal), viewNormal) >= 0.0f ?
        1.0f : -1.0f;

    output.tangentToWorld0 = float4(
        viewTangent.x, viewBinormal.x, viewNormal.x, 0.0f);
    output.tangentToWorld1 = float4(
        viewTangent.z, viewBinormal.z, viewNormal.z, handedness);
    output.meshUv = float4(input.uv, 0.0f, 1.0f);
    output.cameraToMesh = float4(-viewPosition.xyz, 1.0f);
    output.position = clipPosition;
    return output;
}

float4 PS_MAIN(VALTAN_DISSOLVE01_VS_OUT input) : SV_Target0
{
    clip((float)g_Ue3ValtanDissolve01ToolAdmission - 0.5f);
    clip(g_Ue3ValtanDissolve01LocalTimeSeconds);

    Shade_Ue3_fx_d_pa_dissolve_01_ma_INPUT stage;
    stage.v0 = input.tangentToWorld0;
    stage.v1 = input.tangentToWorld1;
    stage.v4 = input.meshUv;
    stage.v5 = input.cameraToMesh;

    /* The function still executes its source discard instruction.  Only o0
       is returned; result.oMask and MRT2/3/4/5 are intentionally omitted. */
    return Shade_Ue3_fx_d_pa_dissolve_01_ma(stage).o0;
}

RasterizerState RS_ValtanDissolve01OneSided
{
    FillMode = Solid;
    CullMode = Back;
    FrontCounterClockwise = false;
};

DepthStencilState DSS_ValtanDissolve01Write
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = less_equal;
};

BlendState BS_ValtanDissolve01Masked
{
    BlendEnable[0] = false;
    RenderTargetWriteMask[0] = 0x0f;
};

technique11 DefaultTechnique
{
    pass Dissolve01ToolRt0
    {
        SetRasterizerState(RS_ValtanDissolve01OneSided);
        SetDepthStencilState(DSS_ValtanDissolve01Write, 0);
        SetBlendState(
            BS_ValtanDissolve01Masked,
            float4(0.0f, 0.0f, 0.0f, 0.0f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
