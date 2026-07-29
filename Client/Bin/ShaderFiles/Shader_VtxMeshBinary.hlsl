#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
uint g_HasNormalTexture = 0;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    matrix worldView = mul(g_WorldMatrix, g_ViewMatrix);
    matrix worldViewProjection = mul(worldView, g_ProjMatrix);
    output.vPosition = mul(float4(input.vPosition, 1.f), worldViewProjection);
    output.vNormal = normalize(mul(float4(input.vNormal, 0.f), g_WorldMatrix));
    output.vTangent = normalize(mul(float4(input.vTangent, 0.f), g_WorldMatrix));
    output.vBinormal = normalize(mul(float4(input.vBinormal, 0.f), g_WorldMatrix));
    output.vTexcoord = input.vTexcoord;
    output.vWorldPos = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
    output.vProjPos = output.vPosition;
    return output;
}

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vPickPos : SV_TARGET3;
};

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;
    float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    if (diffuse.a < 0.3f)
        discard;

    float3 normal = normalize(input.vNormal.xyz);
    if (0 != g_HasNormalTexture)
    {
        float3 tangentNormal =
            g_NormalTexture.Sample(LinearSampler, input.vTexcoord).xyz * 2.f - 1.f;
        float3x3 tangentToWorld = float3x3(
            normalize(input.vTangent.xyz),
            normalize(input.vBinormal.xyz) * -1.f,
            normal);
        normal = normalize(mul(tangentNormal, tangentToWorld));
    }

    output.vDiffuse = diffuse;
    output.vNormal = float4(normal * 0.5f + 0.5f, 0.f);
    output.vDepth = float4(
        input.vProjPos.z / input.vProjPos.w,
        input.vProjPos.w / 1000.f,
        0.f,
        0.f);
    output.vPickPos = input.vWorldPos;
    return output;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
