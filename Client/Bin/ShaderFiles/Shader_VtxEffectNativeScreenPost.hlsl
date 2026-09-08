#include "Shader_EffectCommon.hlsli"
#include "Shader_EffectDimensionMasterVNative.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float4 g_PostSourceColor;
float4 g_PostSourceDynamicParameter;
float g_PostSourceProjectionW;

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
    uint width, height;
    g_EffectSceneColorTexture.GetDimensions(width, height);
    const float2 screenUV = input.position.xy / float2(width, height);
    const float4 source = g_EffectSceneColorTexture.SampleLevel(
        LinearClampUVSampler, screenUV, 0.f);
    const float4 nativeColor = Shade_EffectDimensionMasterVPostNative(
        g_SourceMaterialProfile, input.uv, screenUV,
        g_PostSourceProjectionW, g_PostSourceColor, g_PostSourceDynamicParameter);
    // The existing V1 screen-post carrier covers the viewport. Its native
    // translucent material alpha still composes with scene color exactly once.
    return float4(lerp(source.rgb, nativeColor.rgb, saturate(nativeColor.a)), source.a);
}
technique11 DefaultTechnique
{
    pass NativeScreenPost
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_EffectOpaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
