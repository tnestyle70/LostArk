#include "Shader_EffectCommon.hlsli"
#include "Shader_Artist31470RuntimeMaterial.hlsli"
#include "Shader_EffectStandardColorV1.hlsli"
#include "Shader_Artist31470Active022DecalMaterial.hlsli"
#include "Shader_EffectLocalDecalAdapter.hlsli"

float4x4 g_ViewMatrixInverse;
float4x4 g_ProjMatrixInverse;
float4x4 g_DecalWorldInverse;
Texture2D g_DepthTexture;
Texture2D g_NormalTexture;
float2 g_DecalSize = float2(1.f, 1.f);
float g_DecalDepth = 1.f;
float g_DecalEdgeFade = 0.f;
float3 g_DecalUp = float3(0.f, 1.f, 0.f);
float g_DecalNormalCutoff = -1.f;

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
    output.position = float4(input.position.x * 2.f, input.position.y * 2.f, 0.f, 1.f);
    output.uv = input.uv;
    return output;
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input)
{
    const float4 depth = g_DepthTexture.Sample(PointSampler, input.uv);
    clip(0.99999f - depth.x);
    clip(0.99999f - depth.y);
    const float viewZ = depth.y * 1000.f;
    float4 worldPosition;
    worldPosition.x = input.uv.x * 2.f - 1.f;
    worldPosition.y = input.uv.y * -2.f + 1.f;
    worldPosition.z = depth.x;
    worldPosition.w = 1.f;
    worldPosition *= viewZ;
    worldPosition = mul(worldPosition, g_ProjMatrixInverse);
    worldPosition = mul(worldPosition, g_ViewMatrixInverse);

    const float3 local = mul(worldPosition, g_DecalWorldInverse).xyz;
    const float2 halfSize = max(g_DecalSize, float2(0.0001f, 0.0001f)) * 0.5f;
    const float halfDepth = max(g_DecalDepth, 0.0001f) * 0.5f;
    clip(halfSize.x - abs(local.x));
    clip(halfSize.y - abs(local.z));
    clip(halfDepth - abs(local.y));
    if (g_DecalNormalCutoff > -1.f)
    {
        const float3 sceneNormal =
            g_NormalTexture.Sample(PointSampler, input.uv).xyz * 2.f - 1.f;
        clip(dot(normalize(sceneNormal), normalize(g_DecalUp)) -
            g_DecalNormalCutoff);
    }

    const float2 decalUV = float2(
        local.x / (halfSize.x * 2.f) + 0.5f,
        0.5f - local.z / (halfSize.y * 2.f));
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    if (0u != g_StandardColorV1Enabled)
    {
        output = Shade_EffectStandardColorV1(
            decalUV * g_UVScale + g_UVOffset,
            float4(1.f, 1.f, 1.f, 1.f));
    }
    else if (0u != g_RuntimeMaterialV2Enabled)
    {
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_ACTIVE022_DECAL)
        {
            output = Shade_Artist31470RuntimeMaterialV2Active022Decal(
                decalUV * g_UVScale + g_UVOffset,
                g_ColorMultiply + g_ColorOffset);
        }
        else if (g_RuntimeMaterialV2Opcode ==
            EFFECT_RUNTIME_ADAPTER_LOCAL_DECAL_RT0_SIX_SRV_V1)
        {
            output = Shade_EffectLocalDecalRt0SixSrvV1(decalUV);
        }
        else
        {
            clip(-1.f);
        }
    }
    else
    {
        output = Shade_Effect(
            decalUV * g_UVScale + g_UVOffset,
            float3(1.f, 1.f, 1.f),
            float4(1.f, 1.f, 1.f, 1.f));
    }
    if (g_DecalEdgeFade > 0.f)
    {
        const float3 normalized = float3(
            abs(local.x) / halfSize.x,
            abs(local.y) / halfDepth,
            abs(local.z) / halfSize.y);
        const float edge = 1.f - max(
            normalized.x, max(normalized.y, normalized.z));
        const float fade = saturate(edge / g_DecalEdgeFade);
        output.SceneColor.a *= fade;
        output.Distortion *= fade;
    }
    return output;
}

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_EffectOpaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ZNone, 0);
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
}
