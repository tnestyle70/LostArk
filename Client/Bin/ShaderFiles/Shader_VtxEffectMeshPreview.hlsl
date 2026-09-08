#include "Shader_EffectCommon.hlsli"
#include "Shader_EffectCubeSampleScene.hlsli"
#include "Shader_EffectDimensionMasterQNative.hlsli"
#include "Shader_EffectDimensionMasterVNative.hlsli"
#include "Shader_EffectDimensionMasterALTVNative.hlsli"
#include "Shader_EffectDimensionMasterWRNative.hlsli"
#include "Shader_Artist31470RuntimeMaterial.hlsli"
#include "Shader_EffectStandardColorV1.hlsli"
#include "Shader_Artist31470Diagnostic.hlsli"
#include "Shader_Artist31470Active011OuterMaterial.hlsli"
#include "Shader_EffectUe3MaterialFamilies.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_NormalMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float4 g_CameraPosition;
uint g_UseBaseOverride = 0;
uint g_SourceMeshHasUV1 = 0u;
float4 g_EffectDynamicParameter = float4(0.f, 0.f, 0.f, 0.f);
uint g_StandardColorV1MeshSubUVEnabled = 0;
float4 g_StandardColorV1MeshSubUVCurrent = float4(1.f, 1.f, 0.f, 0.f);
float4 g_StandardColorV1MeshSubUVNext = float4(1.f, 1.f, 0.f, 0.f);
float g_StandardColorV1MeshSubUVBlend = 0.f;

struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD0;
    float2 sourceUV1 : TEXCOORD1;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
    float2 uvNext : TEXCOORD4;
    float2 carrierUV : TEXCOORD5;
    float3 sourceTangentView : TEXCOORD6;
    float2 sourceUV1 : TEXCOORD7;
    float sourceProjectionW : TEXCOORD8;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    const float4 worldPosition = mul(
        float4(input.position, 1.f), g_WorldMatrix);
    output.position = mul(
        worldPosition, mul(g_ViewMatrix, g_ProjMatrix));
    output.worldPosition = worldPosition.xyz;
    output.normal = normalize(
        mul(float4(input.normal, 0.f), g_NormalMatrix).xyz);
    const float3 transformedTangent =
        mul(float4(input.tangent, 0.f), g_WorldMatrix).xyz;
    output.tangent = normalize(transformedTangent -
        output.normal * dot(transformedTangent, output.normal));
    const float handedness = dot(
        cross(input.normal, input.tangent), input.binormal) < 0.f ? -1.f : 1.f;
    output.binormal = normalize(cross(output.normal, output.tangent)) * handedness;
    const float2 currentUV = 0u != g_StandardColorV1MeshSubUVEnabled ?
        input.uv * g_StandardColorV1MeshSubUVCurrent.xy +
            g_StandardColorV1MeshSubUVCurrent.zw :
        input.uv;
    const float2 nextUV = 0u != g_StandardColorV1MeshSubUVEnabled ?
        input.uv * g_StandardColorV1MeshSubUVNext.xy +
            g_StandardColorV1MeshSubUVNext.zw :
        input.uv;
    output.uv = currentUV * g_UVScale + g_UVOffset;
    output.uvNext = nextUV * g_UVScale + g_UVOffset;
    output.carrierUV = input.uv;
    // UE-compatible vertex-factory adapter repeats the final source UV set.
    // The selected Q swing mesh has one UV set in the original package too.
    output.sourceUV1 = g_SourceMeshHasUV1 != 0u ? input.sourceUV1 : input.uv;
    output.sourceProjectionW = output.position.w;
    output.sourceTangentView = float3(0.f, 0.f, 1.f);
    if (42u == g_SourceMaterialProfile || 50u == g_SourceMaterialProfile ||
        60u == g_SourceMaterialProfile || 66u == g_SourceMaterialProfile || 70u == g_SourceMaterialProfile ||
        84u == g_SourceMaterialProfile ||
        87u == g_SourceMaterialProfile ||
        91u == g_SourceMaterialProfile ||
        114u == g_SourceMaterialProfile ||
        119u == g_SourceMaterialProfile ||
        125u == g_SourceMaterialProfile ||
        140u == g_SourceMaterialProfile ||
        141u == g_SourceMaterialProfile ||
        142u == g_SourceMaterialProfile ||
        143u == g_SourceMaterialProfile ||
        144u == g_SourceMaterialProfile ||
        145u == g_SourceMaterialProfile ||
        146u == g_SourceMaterialProfile ||
        151u == g_SourceMaterialProfile ||
        157u == g_SourceMaterialProfile ||
        171u == g_SourceMaterialProfile ||
        178u == g_SourceMaterialProfile ||
        214u == g_SourceMaterialProfile ||
        234u == g_SourceMaterialProfile ||
        238u == g_SourceMaterialProfile ||
        245u == g_SourceMaterialProfile ||
        258u == g_SourceMaterialProfile ||
        263u == g_SourceMaterialProfile ||
        277u == g_SourceMaterialProfile ||
        279u == g_SourceMaterialProfile ||
        280u == g_SourceMaterialProfile)
    {
        // Native VS transforms camera-to-vertex into object space before the
        // tangent dot products. Preserve that order under nonuniform scale.
        const float3 localView = mul(float4(
            g_CameraPosition.xyz - worldPosition.xyz, 0.f),
            transpose(g_NormalMatrix)).xyz;
        output.sourceTangentView = float3(dot(input.tangent, localView),
            dot(input.binormal, localView), dot(input.normal, localView));
    }
    return output;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
    if (0u != g_StandardColorV1Enabled)
    {
        if (0u != g_StandardColorV1MeshSubUVEnabled)
        {
            return Shade_EffectStandardColorV1Particle(
                input.uv, input.uvNext,
                g_StandardColorV1MeshSubUVBlend,
                float4(1.f, 1.f, 1.f, 1.f));
        }
        return Shade_EffectStandardColorV1(
            input.uv, float4(1.f, 1.f, 1.f, 1.f));
    }
    if (0u != g_RuntimeMaterialV2Enabled)
    {
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_PROJECT_TUNED_GLASS_MESH_V1)
        {
            const float3 pixelWorldNormal = normalize(input.normal);
            const float3 pixelViewNormal = normalize(
                mul(float4(pixelWorldNormal, 0.f), g_ViewMatrix).xyz);
            return Shade_EffectProjectTunedGlassMeshV1(
                input.uv, input.worldPosition, pixelWorldNormal,
                pixelViewNormal.xy,
                g_CameraPosition.xyz,
                g_ColorMultiply + g_ColorOffset);
        }
        if (g_RuntimeMaterialV2Opcode ==
                RUNTIME_MATERIAL_V2_PROJECT_TUNED_BASE_COVERAGE_SRGB ||
            g_RuntimeMaterialV2Opcode ==
                RUNTIME_MATERIAL_V2_PROJECT_TUNED_BASE_COVERAGE_LINEAR)
        {
            return Shade_EffectProjectTunedBaseCoverage(
                input.uv, input.uv, 0.f,
                float4(1.f, 1.f, 1.f, 1.f));
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_ACTIVE011_OUTER_MESH)
        {
            return Shade_Artist31470Active011OuterMaterial(
                input.uv, input.worldPosition, input.normal,
                g_CameraPosition.xyz,
                g_ColorMultiply + g_ColorOffset,
                g_EffectDynamicParameter);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_DRAGON_PH_MASKED_MESH)
        {
            return Shade_EffectUe3DragonPhMaskedMesh(
                input.uv, input.worldPosition, input.normal,
                input.tangent, input.binormal, g_CameraPosition.xyz,
                g_ColorMultiply + g_ColorOffset,
                g_EffectDynamicParameter);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_WPO_SINWAVE_ELECTRIC_RT0_MESH)
        {
            return Shade_EffectUe3WpoSinWaveElectricRt0Mesh(
                input.uv,
                g_ColorMultiply + g_ColorOffset,
                g_EffectDynamicParameter);
        }
        return Shade_RuntimeMaterialV2Mesh(
            input.uv, input.worldPosition, input.normal,
            input.tangent, input.binormal, g_CameraPosition.xyz,
            g_ColorMultiply + g_ColorOffset,
            g_EffectDynamicParameter);
    }
    if (0u != g_ArtistVisualV4Opcode ||
        0u != g_ReconstructedMaterialEvaluatorEnabled)
    {
        return Shade_ReconstructedMaterial(
            input.uv, float3(1.f, 1.f, 1.f),
            float4(1.f, 1.f, 1.f, 1.f), g_EffectDynamicParameter);
    }
    if (42u == g_SourceMaterialProfile)
    {
        return Shade_EffectCubeSampleScene(input.carrierUV, input.position.xy,
            input.sourceTangentView, g_ColorMultiply + g_ColorOffset);
    }
    if (g_SourceMaterialProfile >= 44u && g_SourceMaterialProfile <= 51u)
    {
        return Shade_EffectDimensionMasterQNative(g_SourceMaterialProfile,
            input.carrierUV, input.sourceUV1, input.position.xy,
            input.sourceProjectionW, input.sourceTangentView,
            g_ColorMultiply + g_ColorOffset, g_EffectDynamicParameter);
    }
    if (g_SourceMaterialProfile >= 52u && g_SourceMaterialProfile <= 76u)
    {
        return Shade_EffectDimensionMasterVNative(g_SourceMaterialProfile,
            input.carrierUV, input.sourceUV1, input.position.xy,
            input.sourceProjectionW, input.sourceTangentView,
            g_ColorMultiply + g_ColorOffset, g_EffectDynamicParameter);
    }
    if (g_SourceMaterialProfile >= 80u && g_SourceMaterialProfile <= 205u)
    {
        ALTV_NATIVE_INPUT nativeInput = (ALTV_NATIVE_INPUT)0;
        nativeInput.uv = input.uv;
        nativeInput.uv1 = input.sourceUV1;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = g_StandardColorV1MeshSubUVBlend;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        // Native TEXCOORD10/11 are rows of tangent-to-world, not T/N vectors.
        nativeInput.sourceBasisX = float3(input.tangent.x, input.binormal.x, input.normal.x);
        nativeInput.sourceBasisZ = float3(input.tangent.y, input.binormal.y, input.normal.y);
        nativeInput.handedness = dot(cross(input.normal, input.tangent), input.binormal) < 0.f ? -1.f : 1.f;
        // Existing effect mesh layout has no native vertex color stream.
        nativeInput.vertexColor = float4(1.f, 1.f, 1.f, 1.f);
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.color = g_ColorMultiply + g_ColorOffset;
        nativeInput.dynamicParameter = g_EffectDynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectDimensionMasterALTVNative(g_SourceMaterialProfile, nativeInput);
    }
    if (g_SourceMaterialProfile >= 208u && g_SourceMaterialProfile <= 263u || (g_SourceMaterialProfile >= 277u && g_SourceMaterialProfile <= 280u))
    {
        WR_NATIVE_INPUT nativeInput = (WR_NATIVE_INPUT)0;
        nativeInput.uv = input.carrierUV;
        nativeInput.uv1 = input.sourceUV1;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = g_StandardColorV1MeshSubUVBlend;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        // Native TEXCOORD10/11 are rows of tangent-to-world, not T/N vectors.
        nativeInput.sourceBasisX = float3(input.tangent.x, input.binormal.x, input.normal.x);
        nativeInput.sourceBasisZ = float3(input.tangent.y, input.binormal.y, input.normal.y);
        nativeInput.handedness = dot(cross(input.normal, input.tangent), input.binormal) < 0.f ? -1.f : 1.f;
        // All selected W/R mesh PS signatures leave COLOR0/COLOR1 unread.
        nativeInput.vertexColor = float4(1.f, 1.f, 1.f, 1.f);
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.color = g_ColorMultiply + g_ColorOffset;
        nativeInput.dynamicParameter = g_EffectDynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectDimensionMasterWRNative(g_SourceMaterialProfile, nativeInput);
    }
    if (9 == g_SourceMaterialProfile)
    {
        return Shade_LocalCrackMesh(
            input.uv, input.worldPosition, input.normal,
            input.tangent, input.binormal, g_CameraPosition.xyz,
            float4(1.f, 1.f, 1.f, 1.f),
            g_EffectDynamicParameter);
    }
    EFFECT_PS_OUT output = Shade_EffectParticle(
        input.uv,
        float3(1.f, 1.f, 1.f),
        float4(1.f, 1.f, 1.f, 1.f),
        g_EffectDynamicParameter);
    if (0u == g_SourceMaterialProfile)
        output = Apply_GenericMeshRingFill(output, input.carrierUV);
    return output;
}

// The render states differ by pass; the shader programs do not.
// Compile each program once and share it across these passes.
VertexShader EffectPreviewVS = compile vs_5_0 VS_MAIN();
PixelShader EffectPreviewPS = compile ps_5_0 PS_MAIN();

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Default);
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
    pass AlphaOneSidedMirroredDepthRead
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectPreviewVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass AdditiveOneSidedMirroredDepthRead
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectPreviewVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
}
