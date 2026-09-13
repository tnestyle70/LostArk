// Warlord A/S/F/V native RT0 programs; see native_runtime_contract.json.
// Existing Product and grouped material programs never select these IDs.
#ifndef EFFECT_WARLORD_NATIVE_HLSLI
#define EFFECT_WARLORD_NATIVE_HLSLI
#include "Shader_EffectSliceSceneDepth.hlsli"
#include "Shader_EffectCubeSampleScene.hlsli"
#include "Shader_EffectNativeScreenUV.hlsli"

float4 g_WarlordSourceMaterialParameters[32];
float g_WarlordSourceMaterialTime = 0.f;
float4 g_WarlordSkyUpper;
float4 g_WarlordSkyLower;
float4 g_WarlordAmbient;
// Same bounded packet as 16 committed scene + 64 accepted transient lights.
uint g_WarlordGuardianLightCount = 0u;
float4 g_WarlordGuardianDirections[80];
float4 g_WarlordGuardianPositions[80];
float4 g_WarlordGuardianColors[80];
float4 g_WarlordGuardianSpotCones[80];

float4 WarlordNativeAppend(float4 a, float4 b, uint n)
{
    if (n == 1u) return float4(a.x, b.xyz);
    if (n == 2u) return float4(a.xy, b.xy);
    if (n == 3u) return float4(a.xyz, b.x);
    return a;
}
float4 WarlordNativePeriodic(float4 a) { return sign(a) * frac(abs(a)); }

struct WARLORD_NATIVE_INPUT
{
    float2 uv;
    float2 uv1;
    float2 uvNext;
    float subUVBlend;
    float3 sourceWorldPosition;
    float3 sourceBasisX;
    float3 sourceBasisZ;
    float handedness;
    float4 vertexColor;
    float2 screenUV;
    float projectionW;
    float projectionZ;
    float3 tangentView;
    float3 tangentUp;
    float3 lightBasisX;
    float3 lightBasisY;
    float3 lightBasisZ;
    float4 color;
    float4 dynamicParameter;
    bool frontFace;
};

// Existing project point/spot adapter from Shader_Deferred::Resolve_LocalLight.
// The material BRDF remains the exact recovered directional native PS.
bool WarlordGuardianLight(uint index, WARLORD_NATIVE_INPUT input,
    out float3 tangentLight, out float3 color, out float attenuation)
{
    float3 direction = g_WarlordGuardianDirections[index].xyz;
    attenuation = 1.f;
    const float type = g_WarlordGuardianDirections[index].w;
    if (type != 0.f)
    {
        const float3 worldPosition = float3(input.sourceWorldPosition.x,
            input.sourceWorldPosition.z, -input.sourceWorldPosition.y) * 0.01f;
        const float3 toLight = g_WarlordGuardianPositions[index].xyz - worldPosition;
        const float distanceSquared = dot(toLight, toLight);
        const float distance = sqrt(distanceSquared);
        const float range = g_WarlordGuardianPositions[index].w;
        attenuation = isfinite(range) && range > 0.f ? saturate((range - distance) / range) : 0.f;
        const float exponent = g_WarlordGuardianColors[index].w;
        if (exponent != 1.f) attenuation = pow(attenuation, exponent);
        // A coincident point has no incoming direction; avoid undefined native rsqrt(0).
        if (distanceSquared <= 1.e-12f || attenuation <= 0.f)
        { tangentLight = 0.f; color = 0.f; return false; }
        const float3 incoming = toLight * rsqrt(distanceSquared);
        if (type == 2.f)
        {
            const float2 cone = g_WarlordGuardianSpotCones[index].xy;
            const float factor = saturate((dot(-incoming, direction) - cone.y) / max(cone.x - cone.y, 0.0001f));
            attenuation *= factor * factor;
        }
        direction = incoming;
    }
    tangentLight = direction.x * input.lightBasisX + direction.y * input.lightBasisY + direction.z * input.lightBasisZ;
    color = g_WarlordGuardianColors[index].rgb;
    return attenuation > 0.f;
}

float4 WarlordNativeSample0(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 0u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 0u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture0.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture0.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture0.SampleBias(LinearSampler, uv, lod);
}

float4 WarlordNativeSample1(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 1u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 1u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture1.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture1.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture1.SampleBias(LinearSampler, uv, lod);
}

float4 WarlordNativeSample2(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 2u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 2u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture2.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture2.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture2.SampleBias(LinearSampler, uv, lod);
}

float4 WarlordNativeSample3(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 3u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 3u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture3.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture3.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture3.SampleBias(LinearSampler, uv, lod);
}

float4 WarlordNativeSample4(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 4u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 4u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture4.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture4.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture4.SampleBias(LinearSampler, uv, lod);
}

float4 WarlordNativeSample5(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 5u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 5u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture5.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture5.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture5.SampleBias(LinearSampler, uv, lod);
}

float4 WarlordNativeSample6(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 6u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 6u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture6.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture6.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture6.SampleBias(LinearSampler, uv, lod);
}

float4 WarlordNativeSample7(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 7u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 7u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture7.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture7.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture7.SampleBias(LinearSampler, uv, lod);
}

float4 WarlordNativeSample8(float2 uv, float lod, bool explicitLod)
{
    const uint mode = (((g_SourceTextureClampVMask >> 8u) & 1u) << 1u) | ((g_SourceTextureClampUMask >> 8u) & 1u);
    if (mode == 1u) return explicitLod ? g_SourceTexture8.SampleLevel(LinearClampUSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearClampUSampler, uv, lod);
    if (mode == 2u) return explicitLod ? g_SourceTexture8.SampleLevel(LinearClampVSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearClampVSampler, uv, lod);
    if (mode == 3u) return explicitLod ? g_SourceTexture8.SampleLevel(LinearClampUVSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearClampUVSampler, uv, lod);
    return explicitLod ? g_SourceTexture8.SampleLevel(LinearSampler, uv, lod) : g_SourceTexture8.SampleBias(LinearSampler, uv, lod);
}

// Native Warlord mesh WPO expressions, bindings recovered at ShaderObject byte 268.
// Source local geometry is cm; CMesh has already applied modelPreScale=.01.
// This adapter preserves the native node equations and maps source world XYZ to Client X,Y,Z.
#ifndef EFFECT_WARLORD_VERTEX_NATIVE_HLSLI
#define EFFECT_WARLORD_VERTEX_NATIVE_HLSLI
float3 WarlordToSource(float3 value) { return float3(value.x,-value.z,value.y); }
float3 WarlordToClient(float3 value) { return float3(value.x,value.z,-value.y); }
float3 Apply_WarlordNativeWorldPositionOffset(uint profile, float3 localPositionMetres,
    float4 sourceVertexColor, float4 dynamicParameter, float4x4 worldMatrix, float4x4 normalMatrix)
{
    if (profile != 418u && profile != 446u) return 0.f;
    const float3 localSourceCM=WarlordToSource(localPositionMetres)*100.f;
    float3 sourceOffsetCM=0.f;
    if (profile == 418u)
    {
        // VS f2d41fff62e2df479277b98fe58d41e9 instructions 34-47.
        // Native StaticMesh bounds origin, ObjectWorldPosition->WorldToLocal.
        float3 q=localSourceCM-float3(0.0000019073486328125f,0.f,400.0000305175781f);
        q+=WarlordToSource(mul(float4(0.f,dynamicParameter.w*.01f,0.f,0.f),transpose(normalMatrix)).xyz)*100.f;
        const float uvScale=g_WarlordSourceMaterialParameters[0u].y;
        const float bias=g_WarlordSourceMaterialParameters[0u].x;
        const float2 m=dynamicParameter.yx*float2(uvScale,bias);
        q*=m.x*.00125f;
        const float3 phase=q*6.283185f;
        const float3 displaced=q+float3(sin(phase.x),cos(phase.y),cos(phase.z))*m.y;
        sourceOffsetCM=sourceVertexColor.rgb*displaced;
    }
    else
    {
        // VS343a3e992efbf84683a44fb8904e9a82 instructions13-27.
        // cb0[4].w is object bounds sphere radius, scaled by max object axis length.
        float3 q=localSourceCM-float3(-101.39295959472656f,-22.966995239257812f,16.743955612182617f);
        q+=g_WarlordSourceMaterialParameters[4u].xyz+dynamicParameter.x;
        q=q*g_WarlordSourceMaterialParameters[3u].xyz+g_WarlordSourceMaterialParameters[5u].xyz*g_WarlordSourceMaterialTime;
        q=sin(q*6.283185f);
        q*=dynamicParameter.z*g_WarlordSourceMaterialParameters[6u].xyz;
        const float maxAxis=max(length(worldMatrix[0].xyz),max(length(worldMatrix[1].xyz),length(worldMatrix[2].xyz)));
        const float objectRadiusCM=106.80001831054688f*maxAxis;
        q*=objectRadiusCM*g_WarlordSourceMaterialParameters[1u].w;
        sourceOffsetCM=sourceVertexColor.r*q;
    }
    return WarlordToClient(sourceOffsetCM)*.01f;
}
#endif

#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 0
#include "Shader_EffectWarlordNativeGroup000.hlsli"
#endif

#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 960
#include "Shader_EffectWarlordNativeGroup960.hlsli"
#endif
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 1024
#include "Shader_EffectWarlordNativeGroup1024.hlsli"
#endif
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 1088
#include "Shader_EffectWarlordNativeGroup1088.hlsli"
#endif
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 1152
#include "Shader_EffectWarlordNativeGroup1152.hlsli"
#endif
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 1984
#include "Shader_EffectWarlordNativeGroup1984.hlsli"
#endif

EFFECT_PS_OUT Shade_EffectWarlordNative(uint profile, WARLORD_NATIVE_INPUT input)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    float4 nativeColor=0.f;
    bool additive=false;
    switch(profile)
    {
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 0
#include "Shader_EffectWarlordNativeDispatchBase0.hlsli"
#endif
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 960
#include "Shader_EffectWarlordNativeDispatchBase960.hlsli"
#endif
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 1024
#include "Shader_EffectWarlordNativeDispatchBase1024.hlsli"
#endif
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 1088
#include "Shader_EffectWarlordNativeDispatchBase1088.hlsli"
#endif
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 1152
#include "Shader_EffectWarlordNativeDispatchBase1152.hlsli"
#endif
#if !defined(EFFECT_WARLORD_GROUP) || EFFECT_WARLORD_GROUP == 1984
#include "Shader_EffectWarlordNativeDispatchBase1984.hlsli"
#endif
    default: clip(-1.f); return output;
    }
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, (additive || profile == 418u || profile == 419u || profile == 1124u || profile == 1013u || profile == 1143u) ? 1.f : nativeColor.a);
    output.Distortion=0.f;
    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif
