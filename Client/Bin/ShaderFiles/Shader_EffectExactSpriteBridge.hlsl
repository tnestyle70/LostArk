#include "Engine_Shader_Defines.hlsli"

/*
 * Authoring-only UE3 cooked pixel-shader bridge.
 *
 * The source-controlled VS owns the engine VTXEFFECT_PARTICLE ABI while the
 * renderer replaces this file's inert PS with a sealed cooked PS after the
 * pass has established the input layout, VS, depth and rasterizer state.
 */
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

/* Field order and semantics mirror the admitted UE3 BasePass PS signature.
   SV_IsFrontFace is supplied by the rasterizer and therefore has no VS row. */
struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 texcoord10 : TEXCOORD10;
    float4 texcoord11 : TEXCOORD11;
    float4 texcoord0 : TEXCOORD0;
    float4 texcoord1 : TEXCOORD1;
    float4 texcoord2 : TEXCOORD2;
    float4 texcoord4 : TEXCOORD4;
    float4 texcoord6 : TEXCOORD6;
    float4 texcoord5 : TEXCOORD5;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    const float4x4 world = float4x4(
        input.world0, input.world1, input.world2, input.world3);
    const float4 worldPosition = mul(float4(input.position, 1.f), world);
    const float4 clipPosition = mul(
        worldPosition, mul(g_ViewMatrix, g_ProjMatrix));
    const float2 currentUV =
        input.uv * input.uvTransform.xy + input.uvTransform.zw;
    output.position = clipPosition;
    output.texcoord10 = clipPosition;
    output.texcoord11 = worldPosition;
    output.texcoord0 = float4(currentUV, input.particleData);
    output.texcoord1 = input.color;
    output.texcoord2 = input.dynamicParameter;
    /* No-fog authoring canary carrier. UE3's exact CustomParticle PS reads
       TEXCOORD4.w as attenuation, so zero here would erase the RGB output. */
    output.texcoord4 = float4(0.f, 0.f, 0.f, 1.f);
    output.texcoord6 = float4(0.f, 0.f, 1.f, 0.f);
    output.texcoord5 = clipPosition;
    return output;
}

float4 PS_BRIDGE_SENTINEL(VS_OUT input, bool frontFace : SV_IsFrontFace)
    : SV_TARGET0
{
    const float4 carrier = input.texcoord10 + input.texcoord11 +
        input.texcoord0 + input.texcoord1 + input.texcoord2 +
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
