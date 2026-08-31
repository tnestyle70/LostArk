#include "Shader_EffectCommon.hlsli"
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
    return output;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input)
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

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_EffectOpaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AdditiveOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaOneSidedMirroredDepthRead
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AdditiveOneSidedMirroredDepthRead
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
