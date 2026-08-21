/* Esther summon NPC surface (Sillian / Wei / Bahuntur).

   A pinned copy of the skinned deferred material path. The shared
   Shader_VtxAnimMeshBinary keeps growing effect-cue passes and its pass
   indices have already shifted under consumers once; the esther NPCs and the
   screen cutin bind THIS file so their look and pass order stay stable.

   Pass indices are a public contract:
     0 Default (deferred)   1 Shadow   2 ScreenCutin
   Do not insert passes in the middle; append only. Uniform names below are
   the Bind_DeferredMaterialInputs contract and must not be renamed. */
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_SpecularTexture;
Texture2D g_EmissiveTexture;
uint g_HasNormalTexture = 0;
uint g_HasSpecularTexture = 0;
uint g_HasEmissiveTexture = 0;
float g_SpecularIntensity = 1.f;
float g_SpecularPower = 50.f;
float4 g_EmissiveColor = 1.f;
float g_EmissiveIntensity = 1.f;
uint g_HasFullSurfaceEmissiveOverride = 0;
float4 g_FullSurfaceEmissiveColor = 1.f;
float g_FullSurfaceEmissiveIntensity = 0.f;
uint g_FullSurfaceEmissiveMaskMode = 0;
matrix g_BoneMatrices[512];

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    uint4 vBlendIndices : BLENDINDEX;
    float4 vBlendWeights : BLENDWEIGHT;
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

    matrix boneMatrix =
        g_BoneMatrices[input.vBlendIndices.x] * input.vBlendWeights.x +
        g_BoneMatrices[input.vBlendIndices.y] * input.vBlendWeights.y +
        g_BoneMatrices[input.vBlendIndices.z] * input.vBlendWeights.z +
        g_BoneMatrices[input.vBlendIndices.w] * input.vBlendWeights.w;

    float4 position = mul(float4(input.vPosition, 1.f), boneMatrix);
    float4 normal = mul(float4(input.vNormal, 0.f), boneMatrix);
    float4 tangent = mul(float4(input.vTangent, 0.f), boneMatrix);
    float4 binormal = mul(float4(input.vBinormal, 0.f), boneMatrix);

    matrix worldView = mul(g_WorldMatrix, g_ViewMatrix);
    matrix worldViewProjection = mul(worldView, g_ProjMatrix);

    output.vPosition = mul(position, worldViewProjection);
    output.vNormal = normalize(mul(normal, g_WorldMatrix));
    output.vTangent = normalize(mul(tangent, g_WorldMatrix));
    output.vBinormal = normalize(mul(binormal, g_WorldMatrix));
    output.vTexcoord = input.vTexcoord;
    output.vWorldPos = mul(position, g_WorldMatrix);
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

float3 Evaluate_WorldNormal(VS_OUT input)
{
    float3 normal = normalize(input.vNormal.xyz);
    if (0 != g_HasNormalTexture)
    {
        /* Monster normal maps ship as BC5 (RG only); a plain xyz decode reads
           z = -1 and flips the normal into the surface. */
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
    return normal;
}

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;
    float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    if (diffuse.a < 0.3f)
        discard;

    float3 normal = Evaluate_WorldNormal(input);

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
        input.vProjPos.w / 1000.f,
        g_SpecularPower,
        0.f);
    output.vPickPos = input.vWorldPos;
    output.vEmissive = 0.f;
    if (0 != g_HasEmissiveTexture)
    {
        float3 emissive = g_EmissiveTexture.Sample(
            LinearSampler, input.vTexcoord).rgb;
        output.vEmissive = float4(
            emissive * g_EmissiveColor.rgb * g_EmissiveIntensity, 0.f);
    }
    if (0 != g_HasFullSurfaceEmissiveOverride)
    {
        if (1 == g_FullSurfaceEmissiveMaskMode)
        {
            const float3 cameraPosition =
                -mul((float3x3)g_ViewMatrix, g_ViewMatrix[3].xyz);
            const float3 toCamera =
                normalize(cameraPosition - input.vWorldPos.xyz);
            const float rim = pow(1.f - saturate(dot(normal, toCamera)), 3.f);
            output.vEmissive.rgb += g_FullSurfaceEmissiveColor.rgb *
                rim * g_FullSurfaceEmissiveIntensity * diffuse.a;
        }
        else
        {
            const float textureDetail = saturate(0.35f +
                dot(diffuse.rgb, float3(0.299f, 0.587f, 0.114f)) * 0.65f);
            output.vEmissive.rgb +=
                g_FullSurfaceEmissiveColor.rgb *
                g_FullSurfaceEmissiveIntensity * textureDetail * diffuse.a;
        }
    }
    return output;
}

void PS_MAIN_SHADOW(VS_OUT input)
{
    float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    if (diffuse.a < 0.3f)
        discard;
}

float3 g_CutinLightDirection = float3(-0.45f, -0.75f, 0.35f);

/* The cutin is a screen illustration, not part of the scene: it renders after
   the deferred combine with its own fixed light and no exposure/bloom, so a
   dark arena never darkens it. High ambient keeps the read close to the
   texture; the directional term and specular only carry the shape. */
float4 PS_MAIN_SCREEN_CUTIN(VS_OUT input) : SV_TARGET0
{
    float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    if (diffuse.a < 0.3f)
        discard;

    float3 normal = Evaluate_WorldNormal(input);

    const float3 cameraPosition =
        -mul((float3x3)g_ViewMatrix, g_ViewMatrix[3].xyz);
    const float3 toCamera = normalize(cameraPosition - input.vWorldPos.xyz);
    const float3 light = normalize(-g_CutinLightDirection);
    const float diffuseLight = saturate(dot(normal, light));

    float3 color = diffuse.rgb * saturate(0.85f + diffuseLight * 0.3f);

    float specularMask = 0.25f;
    if (0 != g_HasSpecularTexture)
    {
        float3 specular = g_SpecularTexture.Sample(
            LinearSampler, input.vTexcoord).rgb;
        specularMask = dot(specular, float3(0.299f, 0.587f, 0.114f));
    }
    const float3 halfVector = normalize(light + toCamera);
    color += pow(saturate(dot(normal, halfVector)), 32.f) * specularMask;

    if (0 != g_HasEmissiveTexture)
    {
        float3 emissive =
            g_EmissiveTexture.Sample(LinearSampler, input.vTexcoord).rgb;
        color += emissive * g_EmissiveColor.rgb * g_EmissiveIntensity;
    }
    /* The scene's final combine gamma-lifts everything it drew; the cutin
       renders after that pass, so it applies the same lift itself or it reads
       darker than every surface around it. */
    color = pow(saturate(color), 1.f / 2.2f);
    return float4(color, 1.f);
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

    pass Shadow
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }

    pass ScreenCutin
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SCREEN_CUTIN();
    }
}
