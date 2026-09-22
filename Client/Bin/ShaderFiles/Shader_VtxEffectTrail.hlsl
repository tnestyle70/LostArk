#include "Shader_EffectCommon.hlsli"
#include "Shader_Artist31470RuntimeMaterial.hlsli"
#include "Shader_EffectStandardColorV1.hlsli"
#include "Shader_Artist31470Active003RibbonMaterial.hlsli"
#include "Shader_EffectUe3MaterialFamilies.hlsli"
#define EFFECT_NATIVE_TRAIL_CARRIER 1
#define EFFECT_NATIVE_PROFILE_GROUP 2304
#include "Shader_EffectArtistNative.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float4 g_CameraPosition;
// Per-draw coverage scale/offset and reviewed positive-tiling adapter enable.
float4 g_TrailSourceUVTransform;

struct VS_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float4 dynamicParameter : TEXCOORD1;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float2 runtimeUV : TEXCOORD1;
    float4 color : COLOR0;
    float4 dynamicParameter : TEXCOORD2;
    float sourceProjectionW : TEXCOORD3;
    float3 worldPosition : TEXCOORD4;
    float2 coverageUV : TEXCOORD5;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    output.position = mul(
        float4(input.position, 1.f),
        mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix));
    output.sourceProjectionW = output.position.w;
    output.worldPosition = mul(float4(input.position, 1.f), g_WorldMatrix).xyz;
    output.uv = input.uv * g_UVScale + g_UVOffset;
    output.runtimeUV = input.uv;
    output.coverageUV = float2(input.uv.x * g_TrailSourceUVTransform.x +
        g_TrailSourceUVTransform.y, input.uv.y);
    output.color = input.color;
    output.dynamicParameter = input.dynamicParameter;
    return output;
}

// Packed channels are selected from the recovered pixel-program consumers.
// They are not a claim about unrecovered UE3 CPU vertex-buffer packing.
float4 Resolve_NativeTrailUV(float2 distanceUV, float2 coverageUV,
    uint sourceProfile, float enabled)
{
    if (enabled > 0.f)
    {
        // AnimationTrail: alpha mask uses xy; detail/noise uses zw.
        // FlowRib 2410 (Valtan axe ribbon): x is a finite 0..1 length
        // gradient (2x-1, 1-x); the tiled/panned samples read zw.
        if (sourceProfile == 2379u || sourceProfile == 2410u ||
            sourceProfile == 4058u || sourceProfile == 4059u ||
            sourceProfile == 4177u || sourceProfile == 4178u ||
            sourceProfile == 4203u || sourceProfile == 4256u)
            return float4(coverageUV, distanceUV);
        // WaterRibbon: the mask and finite 0..1 end taper both use zw.
        if (sourceProfile == 2346u || sourceProfile == 2836u ||
            sourceProfile == 3007u)
            return float4(distanceUV, coverageUV);
    }
    if (sourceProfile == 3968u || sourceProfile == 3969u || sourceProfile == 4027u)
        return float4(coverageUV, coverageUV.yx);
    return float4(distanceUV, distanceUV.yx);
}

EFFECT_PS_OUT PS_MATERIAL(VS_OUT input)
{
    if (0u != g_StandardColorV1Enabled)
    {
        return Shade_EffectStandardColorV1(input.uv, input.color);
    }
    if (0u != g_RuntimeMaterialV2Enabled)
    {
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_ACTIVE003_RIBBON)
        {
            return Shade_Artist31470Active003RibbonMaterial(
                input.runtimeUV, input.color);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_RIBBONLIQUID01_PARENT_DEFAULT)
        {
            return Shade_EffectUe3RibbonLiquid01ParentDefault(
                input.runtimeUV, input.color, input.dynamicParameter);
        }

        EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
        clip(-1.f);
        return output;
    }
    if (g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= 4671u)
    {
        ARTIST_NATIVE_INPUT nativeInput = (ARTIST_NATIVE_INPUT)0;
        const float4 packedUV = Resolve_NativeTrailUV(input.runtimeUV,
            input.coverageUV, g_SourceMaterialProfile, g_TrailSourceUVTransform.z);
        nativeInput.uv = packedUV.xy;
        nativeInput.uv1 = packedUV.zw;
        nativeInput.color = input.color * g_ColorMultiply + g_ColorOffset;
        nativeInput.vertexColor = input.color;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        [unroll] for (uint i = 0u; i < 4u; ++i) nativeInput.sourceProjection[i] = g_KoukuSourceProjection[i];
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.frontFace = true;
        if ((g_SourceMaterialProfile >= 3968u && g_SourceMaterialProfile <= 4303u) ||
            g_SourceMaterialProfile == 4542u)
        {
            // These recovered programs consume a tangent-space view vector.
            // Derive the frame from the actual strip and its UV orientation;
            // zero-initialized input causes the native rsqrt(dot(view,view)) to fail.
            const float3 dx = ddx(input.worldPosition);
            const float3 dy = ddy(input.worldPosition);
            const float2 du = ddx(input.runtimeUV);
            const float2 dv = ddy(input.runtimeUV);
            const float determinant = du.x * dv.y - du.y * dv.x;
            clip(abs(determinant) - 1.e-12f);
            const float3 rawTangent = (dx * dv.y - dy * du.y) / determinant;
            clip(dot(rawTangent, rawTangent) - 1.e-12f);
            const float3 tangent = normalize(rawTangent);
            const float3 rawBinormal = (dy * du.x - dx * dv.x) / determinant;
            const float3 orthogonalBinormal = rawBinormal - tangent * dot(rawBinormal, tangent);
            clip(dot(orthogonalBinormal, orthogonalBinormal) - 1.e-12f);
            const float3 binormal = normalize(orthogonalBinormal);
            const float3 normal = normalize(cross(tangent, binormal));
            const float3 toCamera = g_CameraPosition.xyz - input.worldPosition;
            clip(dot(toCamera, toCamera) - 1.e-12f);
            nativeInput.tangentView = float3(dot(tangent, toCamera), dot(binormal, toCamera), dot(normal, toCamera));
            nativeInput.sourceBasisX = float3(tangent.x, binormal.x, normal.x);
            nativeInput.sourceBasisZ = float3(tangent.y, binormal.y, normal.y);
            nativeInput.handedness = 1.f;
            nativeInput.tangentUp = nativeInput.sourceBasisZ;
        }
        return Shade_EffectArtistNative(g_SourceMaterialProfile, nativeInput);
    }
    if (35u == g_SourceMaterialProfile)
    {
        const float4 sourceColor = float4(
            input.color.r, input.color.r, input.color.r, input.color.a);
        const float4 dynamicParameter = float4(
            g_TypedTrailParameters[4].xy, input.color.g, input.color.b);
        return Shade_EffectParticleUV(input.uv, input.runtimeUV,
            float3(1.f, 1.f, 1.f), sourceColor, dynamicParameter);
    }
    return Shade_Effect(input.uv, float3(1.f, 1.f, 1.f), input.color);
}

EFFECT_PS_OUT PS_MAIN(VS_OUT input)
{
    g_EffectSceneReadMode = 0u;
    g_EffectSceneSampleUsed = false;
    EFFECT_PS_OUT output = PS_MATERIAL(input);
    if (!g_EffectSceneSampleUsed)
        return Write_EffectBloom(output);
    g_EffectSceneReadMode = 1u;
    const EFFECT_PS_OUT transported = PS_MATERIAL(input);
    g_EffectSceneReadMode = 2u;
    const EFFECT_PS_OUT emission = PS_MATERIAL(input);
    g_EffectSceneReadMode = 0u;
    output.BloomContribution = float4(transported.SceneColor.rgb - emission.SceneColor.rgb +
        Write_SceneBloom(emission.SceneColor).rgb, output.SceneColor.a);
    return output;
}

// Identical entry/profile/arguments compile once; pass states and indices stay unchanged.
VertexShader EffectTrailVS = compile vs_5_0 VS_MAIN();
PixelShader EffectTrailPS = compile ps_5_0 PS_MAIN();

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_EffectOpaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectTrailVS;
        GeometryShader = NULL;
        PixelShader = EffectTrailPS;
    }
    pass AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectTrailVS;
        GeometryShader = NULL;
        PixelShader = EffectTrailPS;
    }
    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectTrailVS;
        GeometryShader = NULL;
        PixelShader = EffectTrailPS;
    }
    pass AlphaOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectTrailVS;
        GeometryShader = NULL;
        PixelShader = EffectTrailPS;
    }
    pass AdditiveOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectTrailVS;
        GeometryShader = NULL;
        PixelShader = EffectTrailPS;
    }
}
