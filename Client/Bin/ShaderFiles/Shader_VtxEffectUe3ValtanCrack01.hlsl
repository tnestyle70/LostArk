#include "Shader_Ue3ValtanCrack01.hlsli"

/*
 * Tool-only local-mesh-particle adapter for Crack01 RT0.
 *
 * The exact equation requires live cb0[17], cb1[4], cb2[4], and the source
 * scene-depth pair t5/s0.  The runtime binder must populate all of them before
 * setting g_Ue3ValtanCrack01ToolAdmission=1.  No texture, scene constant or
 * local-mesh semantic falls back to the family-lite shader from this file.
 */
float4x4 g_WorldMatrix;
float4x4 g_NormalMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float g_Ue3ValtanCrack01LocalTimeSeconds = -1.0f;
uint g_Ue3ValtanCrack01ToolAdmission = 0u;

struct VALTAN_CRACK01_VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD0;
};

struct VALTAN_CRACK01_VS_OUT
{
    float4 tangentToWorld0 : TEXCOORD10;
    float4 tangentToWorld1 : TEXCOORD11;
    float4 meshUv : TEXCOORD0;
    float4 fogAndSelection : TEXCOORD4;
    float4 cameraToMesh : TEXCOORD7;
    float4 screenPosition : TEXCOORD5;
    float4 position : SV_POSITION;
};

float3 NormalizeValtanCrack01Axis(float3 axis, float3 fallbackAxis)
{
    const float lengthSquared = dot(axis, axis);
    return lengthSquared > 1.0e-12f ?
        axis * rsqrt(lengthSquared) : fallbackAxis;
}

VALTAN_CRACK01_VS_OUT VS_MAIN(VALTAN_CRACK01_VS_IN input)
{
    VALTAN_CRACK01_VS_OUT output;
    const float4 worldPosition =
        mul(float4(input.position, 1.0f), g_WorldMatrix);
    const float4 viewPosition = mul(worldPosition, g_ViewMatrix);
    const float4 clipPosition = mul(viewPosition, g_ProjMatrix);

    const float3 worldTangent = NormalizeValtanCrack01Axis(
        mul(float4(input.tangent, 0.0f), g_NormalMatrix).xyz,
        float3(1.0f, 0.0f, 0.0f));
    const float3 worldBinormal = NormalizeValtanCrack01Axis(
        mul(float4(input.binormal, 0.0f), g_NormalMatrix).xyz,
        float3(0.0f, 1.0f, 0.0f));
    const float3 worldNormal = NormalizeValtanCrack01Axis(
        mul(float4(input.normal, 0.0f), g_NormalMatrix).xyz,
        float3(0.0f, 0.0f, 1.0f));
    const float3 viewTangent = NormalizeValtanCrack01Axis(
        mul(float4(worldTangent, 0.0f), g_ViewMatrix).xyz,
        float3(1.0f, 0.0f, 0.0f));
    const float3 viewBinormal = NormalizeValtanCrack01Axis(
        mul(float4(worldBinormal, 0.0f), g_ViewMatrix).xyz,
        float3(0.0f, 1.0f, 0.0f));
    const float3 viewNormal = NormalizeValtanCrack01Axis(
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
    output.fogAndSelection = float4(0.0f, 0.0f, 0.0f, 1.0f);
    output.cameraToMesh = float4(-viewPosition.xyz, 1.0f);
    output.screenPosition = clipPosition;
    output.position = clipPosition;
    return output;
}

float4 PS_MAIN(VALTAN_CRACK01_VS_OUT input) : SV_Target0
{
    clip((float)g_Ue3ValtanCrack01ToolAdmission - 0.5f);
    clip(g_Ue3ValtanCrack01LocalTimeSeconds);

    Shade_Ue3_fx_d_me_crack_01_tr_INPUT stage;
    stage.v0 = input.tangentToWorld0;
    stage.v1 = input.tangentToWorld1;
    stage.v4 = input.meshUv;
    stage.v5 = input.fogAndSelection;
    stage.v7 = input.cameraToMesh;
    stage.v8 = input.screenPosition;

    /* Selecting o0 omits the source BasePass MRT2/3/4/5 tail, but does not
       relax the scene-depth, cb1 or cb2 bindings consumed by RT0. */
    return Shade_Ue3_fx_d_me_crack_01_tr(stage).o0;
}

RasterizerState RS_ValtanCrack01OneSided
{
    FillMode = Solid;
    CullMode = Back;
    FrontCounterClockwise = false;
};

DepthStencilState DSS_ValtanCrack01ReadOnly
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less_equal;
};

BlendState BS_ValtanCrack01Translucent
{
    BlendEnable[0] = true;
    SrcBlend = Src_Alpha;
    DestBlend = Inv_Src_Alpha;
    BlendOp = Add;
    SrcBlendAlpha = One;
    DestBlendAlpha = Inv_Src_Alpha;
    BlendOpAlpha = Add;
    RenderTargetWriteMask[0] = 0x0f;
};

technique11 DefaultTechnique
{
    pass Crack01ToolRt0
    {
        SetRasterizerState(RS_ValtanCrack01OneSided);
        SetDepthStencilState(DSS_ValtanCrack01ReadOnly, 0);
        SetBlendState(
            BS_ValtanCrack01Translucent,
            float4(0.0f, 0.0f, 0.0f, 0.0f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
