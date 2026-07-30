#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_WorldInvTransposeMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_EmissiveTexture;
Texture2D g_SpecularTexture;
Texture2D g_OpacityTexture;
uint g_HasNormalTexture = 0;
uint g_HasEmissiveTexture = 0;
uint g_HasSpecularTexture = 0;
uint g_HasOpacityTexture = 0;
float g_EmissiveIntensity = 1.f;
float g_SpecularIntensity = 1.f;
float g_SpecularPower = 50.f;
float2 g_UVScale = float2(1.f, 1.f);
float2 g_UVOffset = float2(0.f, 0.f);
float g_Opacity = 1.f;
float g_OpacityPower = 1.f;
float4 g_ColorTint = 1.f;

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
    float3 normal = normalize(
        mul(float4(input.vNormal, 0.f), g_WorldInvTransposeMatrix).xyz);
    float3 tangentLinear =
        mul(float4(input.vTangent, 0.f), g_WorldMatrix).xyz;
    float3 tangent = normalize(
        tangentLinear - normal * dot(tangentLinear, normal));
    float3 sourceBinormalLinear =
        mul(float4(input.vBinormal, 0.f), g_WorldMatrix).xyz;
    float handedness =
        dot(cross(normal, tangent), sourceBinormalLinear) < 0.f ? -1.f : 1.f;
    float3 binormal = normalize(cross(normal, tangent)) * handedness;
    output.vNormal = float4(normal, 0.f);
    output.vTangent = float4(tangent, 0.f);
    output.vBinormal = float4(binormal, 0.f);
    output.vTexcoord = input.vTexcoord * g_UVScale + g_UVOffset;
    output.vWorldPos = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
    output.vProjPos = output.vPosition;
    return output;
}

VS_OUT VS_MAIN_SKY(VS_IN input)
{
    VS_OUT output = VS_MAIN(input);
    output.vPosition.z = output.vPosition.w * 0.99999f;
    output.vProjPos = output.vPosition;
    return output;
}

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vPickPos : SV_TARGET3;
    float4 vEmissive : SV_TARGET4;
};

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;
    float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    diffuse *= g_ColorTint;
    if (diffuse.a < 0.3f)
        discard;
    float3 normal = normalize(input.vNormal.xyz);
    if (0 != g_HasNormalTexture)
    {
        float4 encodedNormal =
            g_NormalTexture.Sample(LinearSampler, input.vTexcoord);
        float3 tangentNormal;
        if (encodedNormal.b <= 0.0001f)
        {
            float2 tangentXY = encodedNormal.rg * 2.f - 1.f;
            float tangentZ = sqrt(saturate(1.f - dot(tangentXY, tangentXY)));
            tangentNormal = float3(tangentXY, tangentZ);
        }
        else
        {
            tangentNormal = normalize(encodedNormal.xyz * 2.f - 1.f);
        }
        float3x3 tangentToWorld = float3x3(
            normalize(input.vTangent.xyz),
            normalize(input.vBinormal.xyz) * -1.f,
            normal);
        normal = normalize(mul(tangentNormal, tangentToWorld));
    }
    output.vDiffuse = diffuse;
    float specularMask = g_SpecularIntensity;
    if (0 != g_HasSpecularTexture)
    {
        float3 specular = g_SpecularTexture.Sample(
            LinearSampler, input.vTexcoord).rgb;
        specularMask *= dot(specular, float3(0.299f, 0.587f, 0.114f));
    }
    output.vNormal = float4(normal * 0.5f + 0.5f, specularMask);
    output.vDepth = float4(
        input.vProjPos.z / input.vProjPos.w,
        input.vProjPos.w / 1000.f, g_SpecularPower, 0.f);
    output.vPickPos = input.vWorldPos;
    output.vEmissive = 0.f;
    if (0 != g_HasEmissiveTexture)
    {
        float3 emissive = g_EmissiveTexture.Sample(
            LinearSampler, input.vTexcoord).rgb;
        output.vEmissive = float4(emissive * g_EmissiveIntensity, 0.f);
    }
    return output;
}

struct PS_OUT_FORWARD
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_FORWARD PS_MAIN_ALPHA(VS_OUT input)
{
    PS_OUT_FORWARD output;
    float4 color = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    color.rgb *= g_ColorTint.rgb;
    float opacityMask = 1.f;
    if (0 != g_HasOpacityTexture)
    {
        float3 opacitySample =
            g_OpacityTexture.Sample(LinearSampler, input.vTexcoord).rgb;
        opacityMask = pow(saturate(dot(
            opacitySample, float3(0.299f, 0.587f, 0.114f))),
            g_OpacityPower);
    }
    color.a = saturate(
        color.a * opacityMask * g_Opacity * g_ColorTint.a);
    if (color.a < 0.001f)
        discard;
    output.vColor = color;
    return output;
}

PS_OUT_FORWARD PS_MAIN_SKY(VS_OUT input)
{
    PS_OUT_FORWARD output;
    float4 color = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    output.vColor = float4(color.rgb * g_ColorTint.rgb, 1.f);
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
    pass MirroredPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass TwoSidedOpaquePass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass AlphaFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass AlphaTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass SkyBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN_SKY();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SKY();
    }
    pass SkyFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN_SKY();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SKY();
    }
    pass SkyTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN_SKY();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SKY();
    }
    pass AdditiveBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass AdditiveFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass AdditiveTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
}
