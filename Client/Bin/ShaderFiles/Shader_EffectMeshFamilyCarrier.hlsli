#define EFFECT_NATIVE_MESH_CARRIER 1
#include "Shader_EffectCommon.hlsli"
#include "Shader_EffectSliceSceneDepth.hlsli"
#include "Shader_EffectCubeSampleScene.hlsli"
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
#include "Shader_Artist31470Diagnostic.hlsli"
#include "Shader_Artist31470Active011OuterMaterial.hlsli"
#include "Shader_EffectUe3MaterialFamilies.hlsli"
#endif

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
    float4 sourceColor : COLOR0;
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
    float4 sourceColor : COLOR0;
    float3 sourceTangentUp : TEXCOORD9;
    nointerpolation float4 particleColor : COLOR1;
    nointerpolation float4 dynamicParameter : TEXCOORD10;
    nointerpolation float2 particleData : TEXCOORD11;
#if EFFECT_SHADER_FAMILY == 6
    float3 sourceLightBasisX : TEXCOORD12;
    float3 sourceLightBasisY : TEXCOORD13;
    float3 sourceLightBasisZ : TEXCOORD14;
#endif
};

VS_OUT Build_EffectMeshVertex(VS_IN input, float4x4 world, float4x4 normalMatrix,
    float4 particleColor, float4 dynamicParameter, float4 subUVCurrent,
    float4 subUVNext, float2 particleData)
{
    VS_OUT output;
    float4 worldPosition = mul(
        float4(input.position, 1.f), world);
#if EFFECT_SHADER_FAMILY == 6
    worldPosition.xyz += Apply_WarlordNativeWorldPositionOffset(g_SourceMaterialProfile,
        input.position, input.sourceColor, dynamicParameter, world, normalMatrix);
#endif
    output.position = mul(
        worldPosition, mul(g_ViewMatrix, g_ProjMatrix));
    output.worldPosition = worldPosition.xyz;
    output.normal = normalize(
        mul(float4(input.normal, 0.f), normalMatrix).xyz);
    const float3 transformedTangent =
        mul(float4(input.tangent, 0.f), world).xyz;
    output.tangent = normalize(transformedTangent -
        output.normal * dot(transformedTangent, output.normal));
    const float handedness = dot(
        cross(input.normal, input.tangent), input.binormal) < 0.f ? -1.f : 1.f;
    output.binormal = normalize(cross(output.normal, output.tangent)) * handedness;
    const float2 currentUV = 0u != g_StandardColorV1MeshSubUVEnabled ?
        input.uv * subUVCurrent.xy +
            subUVCurrent.zw :
        input.uv;
    const float2 nextUV = 0u != g_StandardColorV1MeshSubUVEnabled ?
        input.uv * subUVNext.xy +
            subUVNext.zw :
        input.uv;
    output.uv = currentUV * g_UVScale + g_UVOffset;
    output.uvNext = nextUV * g_UVScale + g_UVOffset;
    output.carrierUV = input.uv;
    // UE-compatible vertex-factory adapter repeats the final source UV set.
    // The selected Q swing mesh has one UV set in the original package too.
    output.sourceUV1 = g_SourceMeshHasUV1 != 0u ? input.sourceUV1 : input.uv;
    output.sourceProjectionW = output.position.w;
    output.sourceColor = input.sourceColor;
    const float3 localUp = mul(float4(0.f, 1.f, 0.f, 0.f), transpose(normalMatrix)).xyz;
    output.sourceTangentUp = float3(dot(input.tangent, localUp), dot(input.binormal, localUp), dot(input.normal, localUp));
    output.sourceTangentView = float3(0.f, 0.f, 1.f);
#if EFFECT_SHADER_FAMILY == 6
    // Original directional VS transforms the light into object space before
    // tangent dot products. Keep the same order for nonuniform instances.
    const float3 localLightX=mul(float4(1.f,0.f,0.f,0.f),transpose(normalMatrix)).xyz;
    const float3 localLightY=mul(float4(0.f,1.f,0.f,0.f),transpose(normalMatrix)).xyz;
    const float3 localLightZ=mul(float4(0.f,0.f,1.f,0.f),transpose(normalMatrix)).xyz;
    output.sourceLightBasisX=float3(dot(input.tangent,localLightX),dot(input.binormal,localLightX),dot(input.normal,localLightX));
    output.sourceLightBasisY=float3(dot(input.tangent,localLightY),dot(input.binormal,localLightY),dot(input.normal,localLightY));
    output.sourceLightBasisZ=float3(dot(input.tangent,localLightZ),dot(input.binormal,localLightZ),dot(input.normal,localLightZ));
#endif
    if (((g_SourceMaterialProfile >= 400u && g_SourceMaterialProfile <= 459u) || (g_SourceMaterialProfile >= 660u && g_SourceMaterialProfile <= 719u) || (g_SourceMaterialProfile >= 1000u && g_SourceMaterialProfile <= 1199u) || (g_SourceMaterialProfile >= 2000u && g_SourceMaterialProfile <= 2008u)) || ((g_SourceMaterialProfile >= 462u && g_SourceMaterialProfile <= 559u) || (g_SourceMaterialProfile >= 820u && g_SourceMaterialProfile <= 939u) || (g_SourceMaterialProfile >= 1600u && g_SourceMaterialProfile <= 1694u) || (g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= 2495u)) || ((g_SourceMaterialProfile >= 560u && g_SourceMaterialProfile <= 659u) || (g_SourceMaterialProfile >= 720u && g_SourceMaterialProfile <= 819u) || (g_SourceMaterialProfile >= 1200u && g_SourceMaterialProfile <= 1355u)) || 42u == g_SourceMaterialProfile || 50u == g_SourceMaterialProfile ||
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
        280u == g_SourceMaterialProfile ||
        324u == g_SourceMaterialProfile ||
        (g_SourceMaterialProfile >= 328u && g_SourceMaterialProfile <= 332u) ||
        362u == g_SourceMaterialProfile || 365u == g_SourceMaterialProfile ||
        366u == g_SourceMaterialProfile || 391u == g_SourceMaterialProfile ||
        392u == g_SourceMaterialProfile || 394u == g_SourceMaterialProfile)
    {
        // Native VS transforms camera-to-vertex into object space before the
        // tangent dot products. Preserve that order under nonuniform scale.
        const float3 localView = mul(float4(
            g_CameraPosition.xyz - worldPosition.xyz, 0.f),
            transpose(normalMatrix)).xyz;
        output.sourceTangentView = float3(dot(input.tangent, localView),
            dot(input.binormal, localView), dot(input.normal, localView));
    }
    output.particleColor = particleColor;
    output.dynamicParameter = dynamicParameter;
    output.particleData = particleData;
    return output;
}

VS_OUT VS_MAIN(VS_IN input)
{
    return Build_EffectMeshVertex(input, g_WorldMatrix, g_NormalMatrix,
        g_ColorMultiply, g_EffectDynamicParameter,
        g_StandardColorV1MeshSubUVCurrent, g_StandardColorV1MeshSubUVNext,
        float2(0.f, g_StandardColorV1MeshSubUVBlend));
}

#if EFFECT_SHADER_FAMILY != 0
struct MESH_INSTANCE_IN
{
    float4 world0 : WORLD0;
    float4 world1 : WORLD1;
    float4 world2 : WORLD2;
    float4 world3 : WORLD3;
    float4 normal0 : WORLDINVTRANSPOSE0;
    float4 normal1 : WORLDINVTRANSPOSE1;
    float4 normal2 : WORLDINVTRANSPOSE2;
    float4 normal3 : WORLDINVTRANSPOSE3;
    float4 color : INSTANCE_COLOR0;
    float4 dynamicParameter : DYNAMIC0;
    float4 subUVCurrent : UVTRANSFORM0;
    float4 subUVNext : UVTRANSFORM1;
    float2 particleData : PARTICLEDATA0;
};

VS_OUT VS_INSTANCE_MAIN(VS_IN input, MESH_INSTANCE_IN instance)
{
    return Build_EffectMeshVertex(input,
        float4x4(instance.world0, instance.world1, instance.world2, instance.world3),
        float4x4(instance.normal0, instance.normal1, instance.normal2, instance.normal3),
        instance.color, instance.dynamicParameter,
        instance.subUVCurrent, instance.subUVNext, instance.particleData);
}
#endif

EFFECT_PS_OUT PS_MAIN(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
#if EFFECT_SHADER_FAMILY == 0
    if (0u != g_StandardColorV1Enabled)
    {
        if (0u != g_StandardColorV1MeshSubUVEnabled)
        {
            return Shade_EffectStandardColorV1Particle(
                input.uv, input.uvNext,
                input.particleData.y,
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
                input.particleColor + g_ColorOffset);
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
                input.particleColor + g_ColorOffset,
                input.dynamicParameter);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_DRAGON_PH_MASKED_MESH)
        {
            return Shade_EffectUe3DragonPhMaskedMesh(
                input.uv, input.worldPosition, input.normal,
                input.tangent, input.binormal, g_CameraPosition.xyz,
                input.particleColor + g_ColorOffset,
                input.dynamicParameter);
        }
        if (g_RuntimeMaterialV2Opcode ==
            RUNTIME_MATERIAL_V2_UE3_WPO_SINWAVE_ELECTRIC_RT0_MESH)
        {
            return Shade_EffectUe3WpoSinWaveElectricRt0Mesh(
                input.uv,
                input.particleColor + g_ColorOffset,
                input.dynamicParameter);
        }
        return Shade_RuntimeMaterialV2Mesh(
            input.uv, input.worldPosition, input.normal,
            input.tangent, input.binormal, g_CameraPosition.xyz,
            input.particleColor + g_ColorOffset,
            input.dynamicParameter);
    }
    if (0u != g_ArtistVisualV4Opcode ||
        0u != g_ReconstructedMaterialEvaluatorEnabled)
    {
        return Shade_ReconstructedMaterial(
            input.uv, float3(1.f, 1.f, 1.f),
            float4(1.f, 1.f, 1.f, 1.f), input.dynamicParameter);
    }
    if (42u == g_SourceMaterialProfile)
    {
        return Shade_EffectCubeSampleScene(input.carrierUV, input.position.xy,
            input.sourceTangentView, input.particleColor + g_ColorOffset);
    }
#endif
#if EFFECT_SHADER_FAMILY == 1
    if (g_SourceMaterialProfile >= 44u && g_SourceMaterialProfile <= 51u)
    {
        return Shade_EffectDimensionMasterQNative(g_SourceMaterialProfile,
            input.carrierUV, input.sourceUV1, input.position.xy,
            input.sourceProjectionW, input.sourceTangentView,
            input.particleColor + g_ColorOffset, input.dynamicParameter);
    }
#endif
#if EFFECT_SHADER_FAMILY == 2
    if (g_SourceMaterialProfile >= 52u && g_SourceMaterialProfile <= 76u)
    {
        return Shade_EffectDimensionMasterVNative(g_SourceMaterialProfile,
            input.carrierUV, input.sourceUV1, input.position.xy,
            input.sourceProjectionW, input.sourceTangentView,
            input.particleColor + g_ColorOffset, input.dynamicParameter);
    }
#endif
#if EFFECT_SHADER_FAMILY == 3
    if (g_SourceMaterialProfile >= 80u && g_SourceMaterialProfile <= 205u)
    {
        ALTV_NATIVE_INPUT nativeInput = (ALTV_NATIVE_INPUT)0;
        nativeInput.uv = input.uv;
        nativeInput.uv1 = input.sourceUV1;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
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
        nativeInput.color = input.particleColor + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectDimensionMasterALTVNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 6
    if (((g_SourceMaterialProfile >= 400u && g_SourceMaterialProfile <= 459u) || (g_SourceMaterialProfile >= 660u && g_SourceMaterialProfile <= 719u) || (g_SourceMaterialProfile >= 1000u && g_SourceMaterialProfile <= 1199u) || (g_SourceMaterialProfile >= 2000u && g_SourceMaterialProfile <= 2008u)))
    {
        WARLORD_NATIVE_INPUT nativeInput = (WARLORD_NATIVE_INPUT)0;
        nativeInput.uv = input.carrierUV;
        nativeInput.uv1 = input.sourceUV1;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = float3(input.tangent.x, input.binormal.x, input.normal.x);
        nativeInput.sourceBasisZ = float3(input.tangent.y, input.binormal.y, input.normal.y);
        nativeInput.handedness = dot(cross(input.normal, input.tangent), input.binormal) < 0.f ? -1.f : 1.f;
        nativeInput.vertexColor = input.sourceColor;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.tangentUp = input.sourceTangentUp;
        nativeInput.color = input.particleColor + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        nativeInput.lightBasisX=input.sourceLightBasisX;
        nativeInput.lightBasisY=input.sourceLightBasisY;
        nativeInput.lightBasisZ=input.sourceLightBasisZ;
        return Shade_EffectWarlordNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 8
    if (((g_SourceMaterialProfile >= 560u && g_SourceMaterialProfile <= 659u) || (g_SourceMaterialProfile >= 720u && g_SourceMaterialProfile <= 819u) || (g_SourceMaterialProfile >= 1200u && g_SourceMaterialProfile <= 1355u)))
    {
        LANCE_VA_NATIVE_INPUT nativeInput = (LANCE_VA_NATIVE_INPUT)0;
        nativeInput.uv = input.carrierUV;
        nativeInput.uv1 = input.sourceUV1;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = float3(input.tangent.x, input.binormal.x, input.normal.x);
        nativeInput.sourceBasisZ = float3(input.tangent.y, input.binormal.y, input.normal.y);
        nativeInput.handedness = dot(cross(input.normal, input.tangent), input.binormal) < 0.f ? -1.f : 1.f;
        nativeInput.vertexColor = input.sourceColor;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.tangentUp = input.sourceTangentUp;
        nativeInput.color = input.particleColor + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectLanceMasterVANative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 7
    if (((g_SourceMaterialProfile >= 462u && g_SourceMaterialProfile <= 559u) || (g_SourceMaterialProfile >= 820u && g_SourceMaterialProfile <= 939u) || (g_SourceMaterialProfile >= 1600u && g_SourceMaterialProfile <= 1694u) || (g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= 2495u)))
    {
        ARTIST_NATIVE_INPUT nativeInput = (ARTIST_NATIVE_INPUT)0;
        nativeInput.uv = input.carrierUV;
        nativeInput.uv1 = input.sourceUV1;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = float3(input.tangent.x, input.binormal.x, input.normal.x);
        nativeInput.sourceBasisZ = float3(input.tangent.y, input.binormal.y, input.normal.y);
        nativeInput.handedness = dot(cross(input.normal, input.tangent), input.binormal) < 0.f ? -1.f : 1.f;
        nativeInput.vertexColor = input.sourceColor;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.tangentUp = input.sourceTangentUp;
        nativeInput.color = input.particleColor + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectArtistNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 4
    if (g_SourceMaterialProfile >= 208u && g_SourceMaterialProfile <= 263u || (g_SourceMaterialProfile >= 277u && g_SourceMaterialProfile <= 280u))
    {
        WR_NATIVE_INPUT nativeInput = (WR_NATIVE_INPUT)0;
        nativeInput.uv = input.carrierUV;
        nativeInput.uv1 = input.sourceUV1;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
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
        nativeInput.color = input.particleColor + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        nativeInput.frontFace = frontFace;
        return Shade_EffectDimensionMasterWRNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 5
    if ((g_SourceMaterialProfile >= 324u && g_SourceMaterialProfile <= 332u) ||
        (g_SourceMaterialProfile >= 362u && g_SourceMaterialProfile <= 367u) ||
        391u == g_SourceMaterialProfile || 392u == g_SourceMaterialProfile ||
        394u == g_SourceMaterialProfile)
    {
        SD_NATIVE_INPUT nativeInput = (SD_NATIVE_INPUT)0;
        nativeInput.uv = input.carrierUV;
        nativeInput.uv1 = input.sourceUV1;
        nativeInput.uvNext = input.uvNext;
        nativeInput.subUVBlend = input.particleData.y;
        nativeInput.sourceWorldPosition = float3(input.worldPosition.x, -input.worldPosition.z, input.worldPosition.y) * 100.f;
        nativeInput.sourceBasisX = float3(input.tangent.x, input.binormal.x, input.normal.x);
        nativeInput.sourceBasisZ = float3(input.tangent.y, input.binormal.y, input.normal.y);
        nativeInput.handedness = dot(cross(input.normal, input.tangent), input.binormal) < 0.f ? -1.f : 1.f;
        nativeInput.vertexColor = input.sourceColor;
        nativeInput.screenUV = ALTVNativeScreenUV(input.position.xy);
        nativeInput.projectionW = input.sourceProjectionW * 100.f;
        nativeInput.projectionZ = input.position.z * nativeInput.projectionW;
        nativeInput.frontFace = frontFace;
        nativeInput.tangentView = input.sourceTangentView;
        nativeInput.color = input.particleColor + g_ColorOffset;
        nativeInput.dynamicParameter = input.dynamicParameter;
        return Shade_EffectDimensionMasterSDNative(g_SourceMaterialProfile, nativeInput);
    }
#endif
#if EFFECT_SHADER_FAMILY == 0
    if (9 == g_SourceMaterialProfile)
    {
        return Shade_LocalCrackMesh(
            input.uv, input.worldPosition, input.normal,
            input.tangent, input.binormal, g_CameraPosition.xyz,
            float4(1.f, 1.f, 1.f, 1.f),
            input.dynamicParameter);
    }
    EFFECT_PS_OUT output = Shade_EffectParticle(
        input.uv,
        float3(1.f, 1.f, 1.f),
        float4(1.f, 1.f, 1.f, 1.f),
        input.dynamicParameter);
    if (0u == g_SourceMaterialProfile)
        output = Apply_GenericMeshRingFill(output, input.carrierUV);
    return output;
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
#if EFFECT_SHADER_FAMILY != 0
VertexShader EffectInstanceVS = compile vs_5_0 VS_INSTANCE_MAIN();
#endif

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

#if EFFECT_SHADER_FAMILY != 0

    pass InstancedOpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_EffectOpaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectInstanceVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass InstancedAlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectInstanceVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass InstancedAdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectInstanceVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass InstancedAlphaOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectInstanceVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass InstancedAdditiveOneSidedDepthRead
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectInstanceVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass InstancedAlphaOneSidedMirroredDepthRead
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectInstanceVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
    pass InstancedAdditiveOneSidedMirroredDepthRead
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_EffectAdditive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectInstanceVS;
        GeometryShader = NULL;
        PixelShader = EffectPreviewPS;
    }
#endif
}
