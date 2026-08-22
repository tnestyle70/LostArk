#include "Shader_Ue3ValtanGround04.hlsli"

/*
 * Tool-only bounded LOCAL_DECAL adapter for Ground04 RT0.
 *
 * The source program was selected with FLocalDecalVertexFactory.  Winters
 * renders Local Decal documents as a full-screen rectangle followed by scene
 * depth reconstruction, so this wrapper is deliberately not source VF/pass
 * parity.  The binder must keep actualVfPass=false and Product disabled.
 * g_Ue3ValtanGround04SourceTexcoord0ZW carries the two source TEXCOORD0 lanes
 * that are not derivable from projector UV.  Leaving any admission input at
 * its default value discards the draw instead of inventing a fallback.
 */
float4x4 g_ViewMatrix;
float4x4 g_ViewMatrixInverse;
float4x4 g_ProjMatrixInverse;
float4x4 g_DecalWorldInverse;
float2 g_DecalSize = float2(0.0f, 0.0f);
float g_DecalDepth = 0.0f;
float2 g_Ue3ValtanGround04SourceTexcoord0ZW = float2(0.0f, 0.0f);
float g_Ue3ValtanGround04LocalTimeSeconds = -1.0f;
uint g_Ue3ValtanGround04ToolAdmission = 0u;

Texture2D<float4> g_Ue3ValtanGround04ProjectionDepth;

SamplerState g_Ue3ValtanGround04ProjectionDepthSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
};

struct VALTAN_GROUND04_VS_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VALTAN_GROUND04_VS_OUT
{
    float4 position : SV_POSITION;
    float2 screenUv : TEXCOORD0;
};

VALTAN_GROUND04_VS_OUT VS_MAIN(VALTAN_GROUND04_VS_IN input)
{
    VALTAN_GROUND04_VS_OUT output;
    output.position = float4(
        input.position.x * 2.0f,
        input.position.y * 2.0f,
        0.0f,
        1.0f);
    output.screenUv = input.uv;
    return output;
}

float4 PS_MAIN(VALTAN_GROUND04_VS_OUT input) : SV_Target0
{
    clip((float)g_Ue3ValtanGround04ToolAdmission - 0.5f);
    clip(g_Ue3ValtanGround04LocalTimeSeconds);
    clip(g_DecalSize.x - 1.0e-6f);
    clip(g_DecalSize.y - 1.0e-6f);
    clip(g_DecalDepth - 1.0e-6f);

    const float4 depth = g_Ue3ValtanGround04ProjectionDepth.SampleLevel(
        g_Ue3ValtanGround04ProjectionDepthSampler,
        input.screenUv,
        0.0f);
    const float viewZ = depth.y * 1000.0f;
    clip(viewZ - 1.0e-6f);

    float4 worldPosition;
    worldPosition.x = input.screenUv.x * 2.0f - 1.0f;
    worldPosition.y = input.screenUv.y * -2.0f + 1.0f;
    worldPosition.z = depth.x;
    worldPosition.w = 1.0f;
    worldPosition *= viewZ;
    worldPosition = mul(worldPosition, g_ProjMatrixInverse);
    worldPosition = mul(worldPosition, g_ViewMatrixInverse);

    const float3 localPosition =
        mul(worldPosition, g_DecalWorldInverse).xyz;
    const float2 halfSize = g_DecalSize * 0.5f;
    clip(halfSize.x - abs(localPosition.x));
    clip(halfSize.y - abs(localPosition.z));
    clip(g_DecalDepth * 0.5f - abs(localPosition.y));

    const float2 decalUv = float2(
        localPosition.x / g_DecalSize.x + 0.5f,
        0.5f - localPosition.z / g_DecalSize.y);
    const float4 viewPosition = mul(worldPosition, g_ViewMatrix);
    const float3 cameraToSurfaceView = -viewPosition.xyz;

    Shade_Ue3_fx_d_de_ground_04_tr_INPUT stage;
    /* v0 only contributes to the source BasePass normal MRT tail.  Supplying
       a finite identity keeps the full evaluator defined; RT0 does not claim
       source normal-VF parity. */
    stage.v0 = float4(0.0f, 1.0f, 0.0f, 1.0f);
    stage.v4 = float4(
        decalUv,
        g_Ue3ValtanGround04SourceTexcoord0ZW.x,
        g_Ue3ValtanGround04SourceTexcoord0ZW.y);
    stage.v5 = float4(0.0f, 0.0f, 0.0f, 1.0f);
    stage.v6 = float4(cameraToSurfaceView, 1.0f);
    stage.v7 = float4(cameraToSurfaceView, 1.0f);

    /* Selecting o0 lets the compiler remove MRT2/3/4/5. */
    return Shade_Ue3_fx_d_de_ground_04_tr(stage).o0;
}

RasterizerState RS_ValtanGround04Projector
{
    FillMode = Solid;
    CullMode = None;
    FrontCounterClockwise = false;
};

DepthStencilState DSS_ValtanGround04Projector
{
    /* The full-screen adapter reconstructs and clips against Target_Depth in
       PS_MAIN; hardware depth testing here would test the carrier rectangle,
       not the source local-decal volume. */
    DepthEnable = false;
    DepthWriteMask = zero;
    DepthFunc = less_equal;
};

BlendState BS_ValtanGround04Translucent
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
    pass Ground04ToolRt0
    {
        SetRasterizerState(RS_ValtanGround04Projector);
        SetDepthStencilState(DSS_ValtanGround04Projector, 0);
        SetBlendState(
            BS_ValtanGround04Translucent,
            float4(0.0f, 0.0f, 0.0f, 0.0f),
            0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
