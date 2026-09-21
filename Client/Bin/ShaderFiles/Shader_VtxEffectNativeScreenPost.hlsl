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
float g_PostSourceProjectionZ;

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
float4 Evaluate_NativeScreenPost(VS_OUT input, float2 screenUV)
{
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
    else if (g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= 4543u)
    {
        ARTIST_NATIVE_INPUT n = (ARTIST_NATIVE_INPUT)0;
        n.uv = input.uv; n.screenUV = screenUV;
        n.projectionW = g_PostSourceProjectionW * 100.f;
        n.color = g_PostSourceColor; n.dynamicParameter = g_PostSourceDynamicParameter;
        n.frontFace = true;
        n.projectionZ = g_PostSourceProjectionZ * 100.f;
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
    return nativeColor;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input)
{
    uint width, height;
    g_EffectSceneColorTexture.GetDimensions(width, height);
    const float2 screenUV = input.position.xy / float2(width, height);
    const float4 source = g_EffectSceneColorTexture.SampleLevel(
        LinearClampUVSampler, screenUV, 0.f);
    g_EffectSceneReadMode = 0u;
    const float4 nativeColor = Evaluate_NativeScreenPost(input, screenUV);
    g_EffectSceneReadMode = 1u;
    const float4 transported = Evaluate_NativeScreenPost(input, screenUV);
    g_EffectSceneReadMode = 2u;
    const float4 emission = Evaluate_NativeScreenPost(input, screenUV);
    g_EffectSceneReadMode = 0u;
    // The existing V1 screen-post carrier covers the viewport. Its native
    // translucent material alpha still composes with scene color exactly once.
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor = float4(lerp(source.rgb, nativeColor.rgb, saturate(nativeColor.a)), source.a);
    const float3 sourceBloom = g_EffectSceneBloomTexture.SampleLevel(
        LinearClampUVSampler, screenUV, 0.f).rgb;
    // F(B)-F(0) transports existing ownership through the same scene sampler
    // and native UV/color operations. Only F(0), the post's own RGB emission,
    // receives this document's intensity. Preserve the actual HDR alpha.
    // Native motion blur's saturate remains part of its bloom transport operator.
    const float3 nativeBloom = transported.rgb - emission.rgb + Write_SceneBloom(emission).rgb;
    output.BloomContribution = float4(min(max(lerp(sourceBloom,
        nativeBloom, saturate(nativeColor.a)), 0.f), 60000.f), source.a);
    return output;
}
// Project-authored transition over the image captured before this occurrence.
// Keep bloom ownership and leave product UI outside the scene composition.
Texture2D g_CapturedSceneColor;
Texture2D g_CapturedSceneBloom;
float g_CaptureProgress;
float2 g_CaptureDestinationUV;
float2 g_CaptureDestinationSizeUV;
int g_CaptureOverLiveScene;
float4 g_CaptureEdgeSpeed;
float g_CaptureRotationDegrees;
float g_CaptureBackgroundDim;
int g_CaptureSquare;
EFFECT_PS_OUT PS_SCENE_COLLAPSE(VS_OUT input)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor.a = 1.f;
    output.BloomContribution.a = 1.f;
    if (g_CaptureOverLiveScene != 0)
    {
        output.SceneColor = g_EffectSceneColorTexture.SampleLevel(LinearClampUVSampler, input.uv, 0.f);
        output.BloomContribution = g_EffectSceneBloomTexture.SampleLevel(LinearClampUVSampler, input.uv, 0.f);
    }
    const float t = saturate(g_CaptureProgress);
    const float progress = t * t * (3.f - 2.f * t);
    // Color and its owned bloom always share the same warp and outside attenuation.
    output.SceneColor.rgb *= 1.f - saturate(g_CaptureBackgroundDim) * progress;
    output.BloomContribution.rgb *= 1.f - saturate(g_CaptureBackgroundDim) * progress;
    uint width, height;
    g_CapturedSceneColor.GetDimensions(width, height);
    const float2 dimensions = float2(width, height);
    float2 targetSize = g_CaptureDestinationSizeUV;
    if (g_CaptureSquare != 0)
    {
        const float side = min(targetSize.x * width, targetSize.y * height);
        targetSize = side / dimensions;
    }
    // Each edge has an independent ease speed, with exact shared start/end points.
    // >1 reaches its destination sooner, <1 later; no edge can cross the endpoint.
    const float4 edge = pow(progress.xxxx, 1.f / g_CaptureEdgeSpeed);
    const float2 lower = lerp(float2(0.f, 0.f), g_CaptureDestinationUV - targetSize * .5f, edge.xz);
    const float2 upper = lerp(float2(1.f, 1.f), g_CaptureDestinationUV + targetSize * .5f, edge.yw);
    const float2 scale = upper - lower;
    if (any(scale <= 0.00001f)) return output;
    const float2 center = (lower + upper) * .5f;
    const float angle = radians(g_CaptureRotationDegrees) * progress;
    float sine, cosine;
    sincos(angle, sine, cosine);
    const float2 pixel = (input.uv - center) * dimensions;
    const float2 unrotated = float2(cosine * pixel.x + sine * pixel.y,
        -sine * pixel.x + cosine * pixel.y) / dimensions;
    const float2 uv = unrotated / scale + .5f;
    if (any(uv < 0.f) || any(uv > 1.f)) return output;
    // Crop to the square gradually, rather than squeezing a widescreen scene.
    const float aspect = float(width) / float(height);
    const float2 crop = g_CaptureSquare != 0 ? lerp(float2(1.f, 1.f),
        float2(min(1.f, 1.f / aspect), min(1.f, aspect)), progress) : float2(1.f, 1.f);
    const float2 captureUV = (uv - .5f) * crop + .5f;
    output.SceneColor.rgb = g_CapturedSceneColor.SampleLevel(LinearClampUVSampler, captureUV, 0.f).rgb;
    output.BloomContribution.rgb = g_CapturedSceneBloom.SampleLevel(LinearClampUVSampler, captureUV, 0.f).rgb;
    return output;
}
// Identical entry/profile/arguments compile once; pass states and indices stay unchanged.
VertexShader NativeScreenPostVS = compile vs_5_0 VS_MAIN();

technique11 DefaultTechnique
{
    pass NativeScreenPost
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_EffectOpaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = NativeScreenPostVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass SceneImageCollapse
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_EffectOpaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = NativeScreenPostVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SCENE_COLLAPSE();
    }
}
