#define EFFECT_NATIVE_PARTICLE_CARRIER 1
#include "Shader_EffectCommon.hlsli"
#include "Shader_EffectCubeSampleScene.hlsli"
#include "Shader_EffectSliceSceneDepth.hlsli"
#if EFFECT_SHADER_FAMILY == 1
#include "Shader_EffectDimensionMasterQNative.hlsli"
#endif
#if EFFECT_SHADER_FAMILY == 2
#include "Shader_EffectDimensionMasterVNative.hlsli"
#endif
#if EFFECT_SHADER_FAMILY == 3
#include "Shader_EffectDimensionMasterALTVNative.hlsli"
#endif
#if EFFECT_SHADER_FAMILY == 4
#include "Shader_EffectDimensionMasterWRNative.hlsli"
#endif
#if EFFECT_SHADER_FAMILY == 5
#include "Shader_EffectDimensionMasterSDNative.hlsli"
#endif
#if EFFECT_SHADER_FAMILY == 6
#include "Shader_EffectWarlordNative.hlsli"
#endif
#if EFFECT_SHADER_FAMILY == 7
#include "Shader_EffectArtistNative.hlsli"
#endif
#if EFFECT_SHADER_FAMILY == 8
#include "Shader_EffectLanceMasterVANative.hlsli"
#endif
#if EFFECT_SHADER_FAMILY == 0
#include "Shader_Artist31470RuntimeMaterial.hlsli"
#include "Shader_EffectStandardColorV1.hlsli"
#include "Shader_EffectUe3MaterialFamilies.hlsli"
#include "Shader_Artist31470Diagnostic.hlsli"
#endif

float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float4 g_CameraPosition;

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

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float4 dynamicParameter : TEXCOORD1;
    float2 uvNext : TEXCOORD2;
    float2 particleData : TEXCOORD3;
    float2 localUV : TEXCOORD4;
    float2 runtimeLocalUV : TEXCOORD5;
    float4 runtimeSubUVTransform : TEXCOORD6;
    float4 runtimeSubUVTransformNext : TEXCOORD7;
    float sourceProjectionW : TEXCOORD8;
    float3 sourceTangentView : TEXCOORD9;
    float3 worldPosition : TEXCOORD10;
    float3 sourceBasisX : TEXCOORD11;
    float3 sourceBasisZ : TEXCOORD12;
    float sourceHandedness : TEXCOORD13;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    const float4x4 world = float4x4(
        input.world0, input.world1, input.world2, input.world3);
    output.position = mul(
        float4(input.position, 1.f),
        mul(mul(world, g_ViewMatrix), g_ProjMatrix));
    output.uv =
        (input.uv * input.uvTransform.xy + input.uvTransform.zw) *
        g_UVScale + g_UVOffset;
    output.uvNext =
        (input.uv * input.uvTransformNext.xy + input.uvTransformNext.zw) *
        g_UVScale + g_UVOffset;
    output.localUV = input.uv * g_UVScale + g_UVOffset;
    output.runtimeLocalUV = input.uv;
    output.runtimeSubUVTransform = input.uvTransform;
    output.runtimeSubUVTransformNext = input.uvTransformNext;
    output.color = input.color;
    output.dynamicParameter = input.dynamicParameter;
    output.particleData = input.particleData;
    output.sourceProjectionW = output.position.w;
    output.sourceTangentView = float3(0.f, 0.f, 1.f);
    output.worldPosition = mul(float4(input.position, 1.f), world).xyz;
    const float3 sourceT = normalize(input.world0.xyz);
    const float3 sourceB = normalize(input.world1.xyz);
    const float3 sourceN = normalize(input.world2.xyz);
    output.sourceBasisX = float3(sourceT.x, sourceB.x, sourceN.x);
    output.sourceBasisZ = float3(sourceT.y, sourceB.y, sourceN.y);
    output.sourceHandedness = dot(cross(sourceN, sourceT), sourceB) < 0.f ? -1.f : 1.f;
    if (((g_SourceMaterialProfile >= 400u && g_SourceMaterialProfile <= 459u) || (g_SourceMaterialProfile >= 660u && g_SourceMaterialProfile <= 719u) || (g_SourceMaterialProfile >= 1000u && g_SourceMaterialProfile <= 1199u) || (g_SourceMaterialProfile >= 2000u && g_SourceMaterialProfile <= 2008u)) || ((g_SourceMaterialProfile >= 462u && g_SourceMaterialProfile <= 559u) || (g_SourceMaterialProfile >= 820u && g_SourceMaterialProfile <= 939u) || (g_SourceMaterialProfile >= 1600u && g_SourceMaterialProfile <= 1694u) || (g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= 2495u)) || ((g_SourceMaterialProfile >= 560u && g_SourceMaterialProfile <= 659u) || (g_SourceMaterialProfile >= 720u && g_SourceMaterialProfile <= 819u) || (g_SourceMaterialProfile >= 1200u && g_SourceMaterialProfile <= 1355u)) || 51u == g_SourceMaterialProfile ||
        83u == g_SourceMaterialProfile ||
        101u == g_SourceMaterialProfile ||
        102u == g_SourceMaterialProfile ||
        202u == g_SourceMaterialProfile ||
        215u == g_SourceMaterialProfile ||
        216u == g_SourceMaterialProfile ||
        218u == g_SourceMaterialProfile ||
        231u == g_SourceMaterialProfile ||
        235u == g_SourceMaterialProfile ||
        242u == g_SourceMaterialProfile ||
        244u == g_SourceMaterialProfile ||
        247u == g_SourceMaterialProfile ||
        342u == g_SourceMaterialProfile || 343u == g_SourceMaterialProfile ||
        347u == g_SourceMaterialProfile || 348u == g_SourceMaterialProfile ||
        368u == g_SourceMaterialProfile || 375u == g_SourceMaterialProfile)
    {
        const float3 worldPosition = mul(float4(input.position, 1.f), world).xyz;
        const float3 toCamera = g_CameraPosition.xyz - worldPosition;
        output.sourceTangentView = float3(
            dot(normalize(input.world0.xyz), toCamera),
            dot(normalize(input.world1.xyz), toCamera),
            dot(normalize(input.world2.xyz), toCamera));
        // The source Q51 VS uses texture V and cross(U,V); the shared rect
        // stores local Y=.5-v, so both are opposite to world1/world2.
        if (51u == g_SourceMaterialProfile)
            output.sourceTangentView.yz *= -1.f;
    }
    return output;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
#if EFFECT_SHADER_FAMILY == 0
    if (0u != g_StandardColorV1Enabled)
    {
        return Shade_EffectStandardColorV1Particle(
            input.uv, input.uvNext, input.particleData.y, input.color);
    }
    if (0u != g_RuntimeMaterialV2Enabled)
    {
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_PROJECT_TUNED_WATER_DROPLET_BURST)
        {
            return Shade_EffectProjectTunedWaterDropletBurst(
                input.runtimeLocalUV, input.color);
        }
        if (g_RuntimeMaterialV2Opcode ==
                RUNTIME_MATERIAL_V2_PROJECT_TUNED_BASE_COVERAGE_SRGB ||
            g_RuntimeMaterialV2Opcode ==
                RUNTIME_MATERIAL_V2_PROJECT_TUNED_BASE_COVERAGE_LINEAR)
        {
            return Shade_EffectProjectTunedBaseCoverage(
                input.uv, input.uvNext, input.particleData.y, input.color);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_FLUID01_SPRITE_W_FD_01_3)
        {
            return Shade_EffectUe3Fluid01SpriteWFd013Particle(
                input.runtimeLocalUV, input.color,
                input.dynamicParameter);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_SPRITEWAVE_TR)
        {
            return Shade_EffectUe3SpriteWaveTrParticle(
                input.runtimeLocalUV, input.color,
                input.dynamicParameter);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_SPRITEWAVE_AD_NO_EMISSIVE)
        {
            return Shade_EffectUe3SpriteWaveAdditiveNoEmissiveParticle(
                input.runtimeLocalUV, input.color,
                input.dynamicParameter);
        }
        return Shade_RuntimeMaterialV2Particle(
            input.runtimeLocalUV, input.runtimeSubUVTransform,
            input.runtimeSubUVTransformNext, input.particleData.y,
            input.color, input.dynamicParameter);
    }
    if (0u != g_ArtistVisualV4Opcode ||
        0u != g_ReconstructedMaterialEvaluatorEnabled)
    {
        return Shade_ReconstructedMaterial(
            input.uv, float3(1.f, 1.f, 1.f), input.color,
            input.dynamicParameter);
    }
    if (43u == g_SourceMaterialProfile)
    {
        return Shade_EffectSliceSceneDepth(input.uv, input.position.xy,
            input.sourceProjectionW,
            input.color * g_ColorMultiply + g_ColorOffset,
            input.dynamicParameter);
    }
#endif
#if EFFECT_SHADER_FAMILY == 1
    if (g_SourceMaterialProfile >= 44u && g_SourceMaterialProfile <= 51u)
    {
        return Shade_EffectDimensionMasterQNative(g_SourceMaterialProfile,
            input.uv, float2(0.f, 0.f), input.position.xy,
            input.sourceProjectionW, input.sourceTangentView,
            input.color * g_ColorMultiply + g_ColorOffset, input.dynamicParameter);
    }
#endif
#if EFFECT_SHADER_FAMILY == 2
    if (g_SourceMaterialProfile >= 52u && g_SourceMaterialProfile <= 76u)
    {
        return Shade_EffectDimensionMasterVNative(g_SourceMaterialProfile,
            input.uv, float2(0.f, 0.f), input.position.xy,
            input.sourceProjectionW, input.sourceTangentView,
            input.color * g_ColorMultiply + g_ColorOffset, input.dynamicParameter);
    }
#endif
#if EFFECT_SHADER_FAMILY == 3
    if (g_SourceMaterialProfile >= 80u && g_SourceMaterialProfile <= 205u)
    {
        ALTV_NATIVE_INPUT nativeInput = (ALTV_NATIVE_INPUT)0;
        nativeInput.uv = input.uv;
        nativeInput.uv1 = input.uv;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = input.sourceBasisX;
        nativeInput.sourceBasisZ = input.sourceBasisZ;
        nativeInput.handedness = input.sourceHandedness;
        nativeInput.vertexColor = input.color;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.color = input.color * g_ColorMultiply + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectDimensionMasterALTVNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 6
    if (((g_SourceMaterialProfile >= 400u && g_SourceMaterialProfile <= 459u) || (g_SourceMaterialProfile >= 660u && g_SourceMaterialProfile <= 719u) || (g_SourceMaterialProfile >= 1000u && g_SourceMaterialProfile <= 1199u) || (g_SourceMaterialProfile >= 2000u && g_SourceMaterialProfile <= 2008u)))
    {
        WARLORD_NATIVE_INPUT nativeInput = (WARLORD_NATIVE_INPUT)0;
        nativeInput.uv = input.uv;
        nativeInput.uv1 = input.uv;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = input.sourceBasisX;
        nativeInput.sourceBasisZ = input.sourceBasisZ;
        nativeInput.handedness = input.sourceHandedness;
        nativeInput.vertexColor = input.color;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.tangentUp = input.sourceBasisZ;
        nativeInput.color = input.color * g_ColorMultiply + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectWarlordNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 8
    if (((g_SourceMaterialProfile >= 560u && g_SourceMaterialProfile <= 659u) || (g_SourceMaterialProfile >= 720u && g_SourceMaterialProfile <= 819u) || (g_SourceMaterialProfile >= 1200u && g_SourceMaterialProfile <= 1355u)))
    {
        LANCE_VA_NATIVE_INPUT nativeInput = (LANCE_VA_NATIVE_INPUT)0;
        nativeInput.uv = input.uv;
        nativeInput.uv1 = input.uv;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = input.sourceBasisX;
        nativeInput.sourceBasisZ = input.sourceBasisZ;
        nativeInput.handedness = input.sourceHandedness;
        nativeInput.vertexColor = input.color;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.tangentUp = input.sourceBasisZ;
        nativeInput.color = input.color * g_ColorMultiply + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectLanceMasterVANative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 7
    if (((g_SourceMaterialProfile >= 462u && g_SourceMaterialProfile <= 559u) || (g_SourceMaterialProfile >= 820u && g_SourceMaterialProfile <= 939u) || (g_SourceMaterialProfile >= 1600u && g_SourceMaterialProfile <= 1694u) || (g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= 2495u)))
    {
        ARTIST_NATIVE_INPUT nativeInput = (ARTIST_NATIVE_INPUT)0;
        nativeInput.uv = input.uv;
        nativeInput.uv1 = input.uv;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = input.sourceBasisX;
        nativeInput.sourceBasisZ = input.sourceBasisZ;
        nativeInput.handedness = input.sourceHandedness;
        nativeInput.vertexColor = input.color;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.tangentUp = input.sourceBasisZ;
        nativeInput.color = input.color * g_ColorMultiply + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectArtistNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 4
    if (g_SourceMaterialProfile >= 208u && g_SourceMaterialProfile <= 263u || (g_SourceMaterialProfile >= 277u && g_SourceMaterialProfile <= 280u))
    {
        WR_NATIVE_INPUT nativeInput = (WR_NATIVE_INPUT)0;
        nativeInput.uv = input.uv;
        nativeInput.uv1 = input.uv;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = input.sourceBasisX;
        nativeInput.sourceBasisZ = input.sourceBasisZ;
        nativeInput.handedness = input.sourceHandedness;
        nativeInput.vertexColor = input.color;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.color = input.color * g_ColorMultiply + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectDimensionMasterWRNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 5
    if ((g_SourceMaterialProfile >= 320u && g_SourceMaterialProfile <= 323u) ||
        (g_SourceMaterialProfile >= 340u && g_SourceMaterialProfile <= 375u) ||
        (g_SourceMaterialProfile >= 389u && g_SourceMaterialProfile <= 399u))
    {
        SD_NATIVE_INPUT nativeInput = (SD_NATIVE_INPUT)0;
        nativeInput.uv = input.uv;
        nativeInput.uv1 = input.uv;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = input.sourceBasisX;
        nativeInput.sourceBasisZ = input.sourceBasisZ;
        nativeInput.handedness = input.sourceHandedness;
        nativeInput.vertexColor = input.color;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.color = input.color * g_ColorMultiply + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectDimensionMasterSDNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 0
    EFFECT_PS_OUT current = Shade_EffectParticleUV(
        input.uv, input.localUV, float3(1.f, 1.f, 1.f), input.color,
        input.dynamicParameter);
    const float blend = saturate(input.particleData.y);
    if (blend > 0.f)
    {
        const EFFECT_PS_OUT next = Shade_EffectParticleUV(
            input.uvNext, input.localUV, float3(1.f, 1.f, 1.f), input.color,
            input.dynamicParameter);
        current.SceneColor = lerp(current.SceneColor, next.SceneColor, blend);
        current.Distortion = lerp(current.Distortion, next.Distortion, blend);
    }
    return Apply_GenericLinearReveal(current, input.runtimeLocalUV);
#else
    // Native admission has already selected this family before drawing.
    clip(-1.f);
    return (EFFECT_PS_OUT)0;
#endif
}

// The render states differ by pass; the shader programs do not.
// Compile each program once and share it across these passes.
VertexShader EffectPreviewVS = compile vs_5_0 VS_MAIN();
PixelShader EffectPreviewPS = compile ps_5_0 PS_MAIN();

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_EffectOpaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectPreviewVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectPreviewVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectPreviewVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass AlphaOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectPreviewVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass AdditiveOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectPreviewVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
}
