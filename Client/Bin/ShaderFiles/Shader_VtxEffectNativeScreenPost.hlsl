#define EFFECT_NATIVE_SCREEN_POST_CARRIER 1
#include "Shader_EffectCommon.hlsli"
#include "Shader_EffectDimensionMasterVNative.hlsli"
#include "Shader_EffectDimensionMasterALTVNative.hlsli"
#include "Shader_EffectWarlordNative.hlsli"
#include "Shader_EffectArtistNative.hlsli"
#include "Shader_EffectLanceMasterVANative.hlsli"

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
    float4 nativeColor = 0.f;
    if (g_SourceMaterialProfile == 68u || g_SourceMaterialProfile == 76u)
    {
        nativeColor = Shade_EffectDimensionMasterVPostNative(g_SourceMaterialProfile,
            input.uv, screenUV, g_PostSourceProjectionW,
            g_PostSourceColor, g_PostSourceDynamicParameter);
    }
    else if (g_SourceMaterialProfile == 155u || g_SourceMaterialProfile == 156u)
    {
        ALTV_NATIVE_INPUT n = (ALTV_NATIVE_INPUT)0;
        n.uv = input.uv; n.screenUV = screenUV;
        n.projectionW = g_PostSourceProjectionW * 100.f;
        n.color = g_PostSourceColor; n.dynamicParameter = g_PostSourceDynamicParameter;
        n.frontFace = true;
        nativeColor = g_SourceMaterialProfile == 155u ? ALTVNative155(n) : ALTVNative156(n);
    }
    else if (g_SourceMaterialProfile == 415u || g_SourceMaterialProfile == 672u || g_SourceMaterialProfile == 1084u)
    {
        WARLORD_NATIVE_INPUT n = (WARLORD_NATIVE_INPUT)0;
        n.uv = input.uv; n.screenUV = screenUV;
        n.projectionW = g_PostSourceProjectionW * 100.f;
        n.color = g_PostSourceColor; n.dynamicParameter = g_PostSourceDynamicParameter;
        n.frontFace = true;
        nativeColor = g_SourceMaterialProfile == 415u ? WarlordNative415(n) :
            g_SourceMaterialProfile == 672u ? WarlordNative672(n) : WarlordNative1084(n);
    }
    else if ((g_SourceMaterialProfile == 876u || g_SourceMaterialProfile == 894u ||
        g_SourceMaterialProfile == 1619u || g_SourceMaterialProfile == 1623u || g_SourceMaterialProfile == 1648u))
    {
        ARTIST_NATIVE_INPUT n = (ARTIST_NATIVE_INPUT)0;
        n.uv = input.uv; n.screenUV = screenUV;
        n.projectionW = g_PostSourceProjectionW * 100.f;
        n.color = g_PostSourceColor; n.dynamicParameter = g_PostSourceDynamicParameter;
        n.frontFace = true;
        nativeColor = g_SourceMaterialProfile == 876u ? ArtistNative876(n) :
            g_SourceMaterialProfile == 894u ? ArtistNative894(n) :
            g_SourceMaterialProfile == 1619u ? ArtistNative1619(n) :
            g_SourceMaterialProfile == 1623u ? ArtistNative1623(n) : ArtistNative1648(n);
    }
    else if (g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= 3711u)
    {
        ARTIST_NATIVE_INPUT n = (ARTIST_NATIVE_INPUT)0;
        n.uv = input.uv; n.screenUV = screenUV;
        n.projectionW = g_PostSourceProjectionW * 100.f;
        n.color = g_PostSourceColor; n.dynamicParameter = g_PostSourceDynamicParameter;
        n.frontFace = true;
        nativeColor = Shade_EffectArtistNative(g_SourceMaterialProfile, n).SceneColor;
    }
    else if (g_SourceMaterialProfile == 659u || g_SourceMaterialProfile == 1228u || g_SourceMaterialProfile == 1255u)
    {
        LANCE_VA_NATIVE_INPUT n = (LANCE_VA_NATIVE_INPUT)0;
        n.uv = input.uv; n.screenUV = screenUV;
        n.projectionW = g_PostSourceProjectionW * 100.f;
        n.color = g_PostSourceColor; n.dynamicParameter = g_PostSourceDynamicParameter;
        n.frontFace = true;
        nativeColor = g_SourceMaterialProfile == 659u ? LanceVANative659(n) :
            g_SourceMaterialProfile == 1228u ? LanceVANative1228(n) : LanceVANative1255(n);
    }
    else
    {
        clip(-1.f);
    }
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
