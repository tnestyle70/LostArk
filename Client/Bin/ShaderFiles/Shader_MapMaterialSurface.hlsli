#ifndef LOSTARK_MAP_MATERIAL_SURFACE
#define LOSTARK_MAP_MATERIAL_SURFACE

// A source-evidenced parallel N/T has a zero bitangent. Keep it finite and
// preserve zero; never manufacture a normal axis for that native input.
float3 MapGeometryNormalizeOrZero(float3 value)
{
    return dot(value, value) > 0.f ? normalize(value) : float3(0.f, 0.f, 0.f);
}


#include "Shader_SourceStoneSurface.hlsli"
#include "Shader_SourceFoliageSurface.hlsli"

// Program 7: actual Valtan bg_base_opa overlay permutation. These are material
// inputs; normal textures remain linear and color SRVs retain source SRGB.
Texture2D g_SurfaceOverlayDiffuseTexture;
Texture2D g_SurfaceOverlayNormalTexture;
float4 g_SurfaceOverlayColor = 1.f;
float g_SurfaceOverlayTiling = 1.f;
float g_SurfaceOverlayNormalIntensity = 1.f;
float g_SurfaceOverlaySharpness = 0.f;
float g_SurfaceOverlayBrightness = 1.f;
float g_SurfaceOverlaySaturation = 1.f;
float g_SurfaceOverlaySpecularIntensity = 0.f;
uint g_SurfaceOverlaySeparateSpecular = 0u;
uint g_SourceOverlayFlags = 263u;
float4 g_SourceOverlayDirection = float4(0.f, 1.f, 0.f, 0.f);
float4 g_SourceOverlayUV = float4(0.f, 1.f, 0.f, 0.f);

// Opt-in surface programs. Each SRV preserves its source color-space contract;
// the actual Character Select overrides include a linear diffuse texture.
uint g_SurfaceProgram = 0;
uint g_SourceBgFlags = 0u;
float4 g_SourceBgBump = float4(0.f, 0.f, 1.f, 0.f);
float4 g_SourceBgUV = float4(0.f, 1.f, 0.f, 0.f);
uint g_SourceBgFlicker = 0u;
float2 g_SourceBgSubspecular = float2(0.f, 60.f);
float4 g_SourceBgRimlight = float4(0.f, 0.f, 0.f, 3.f);
float g_SourceBgSpecularSaturation = 1.f;
float2 g_SourceBgPanning = 0.f;
uint g_SourceFoliageFlags = 0u;
float4 g_SourceFoliageTransmission = float4(0.f, 0.f, 0.f, 1.f);
Texture2D g_SourceFoliageMaskTexture;

uint g_HasSurfaceDefinition = 0;
uint g_SurfaceDebugView = 0;
float g_SurfaceDiffuseBrightness = 1.f;
float g_SurfaceNormalIntensity = 1.f;
float g_SurfaceSpecularIntensity = 1.f;
float g_SurfaceSpecularPower = 50.f;
float g_SurfaceReflectionIntensity = 0.f;
float g_SurfaceReflectionContrast = 0.5f;
float g_SurfaceReflectionTiling = 1.f;
float g_SurfaceDiffuseSaturation = 1.f;
float4 g_SurfaceDiffuseColor = 1.f;
float4 g_SurfaceSpecularColor = 1.f;
float4 g_SurfaceReflectionColor = 1.f;

// Actual Character Select PBR overrides (3=seamless opaque, 4=opaque).
// These inputs are immutable named-material data, not scene-light controls.
Texture2D g_SurfaceORMTexture;
float2 g_SurfaceUVTiling = 1.f;
uint g_SurfaceUVFixedNormal = 0;
uint g_SurfaceUseWorldReflection = 0;
float g_SurfaceDetailNormalIntensity = 0.f;
float g_SurfaceDetailNormalTiling = 1.f;
float g_SurfaceMetallicIntensity = 1.f;
float g_SurfaceMetallicPower = 1.f;
float g_SurfaceRoughnessIntensity = 1.f;
float g_SurfaceRoughnessPower = 1.f;
float g_SurfaceAOIntensity = 1.f;
float g_SurfaceAOPower = 1.f;
float g_SurfaceSpecularPBRIntensity = 0.5f;
float g_SurfaceNonmetallicBrightness = 1.f;
float g_SurfaceMetallicBrightness = 1.f;
// The original engine-owned minimum and world-reflection origin have not been
// identified. Explicit material inputs retain these approximation boundaries.
float g_SurfaceMinimumRoughness = 0.04f;
float2 g_SurfaceReflectionOriginOffset = 0.f;
float g_SurfaceVertexAlpha = 1.f;
uint g_HasSurfaceEmissive = 0;
Texture2D g_SurfaceEmissiveTexture;
float4 g_SurfaceEmissiveColor = 1.f;
float g_SurfaceEmissiveIntensity = 0.f;
float2 g_SurfaceEmissiveUVTiling = 1.f;
float g_SurfaceEmissiveFlickerMinimum = 0.f;
float g_SurfaceEmissiveFlickerSpeed = 0.f;
float g_SurfaceEmissivePhaseOffset = 0.f;
float g_SurfaceEmissiveTime = 0.f;
// Source component LOD lightmap bindings. Ordinary draws bind these values;
// instanced draws transport the same scale/bias and coefficient vectors.
#include "Shader_StaticShadowMap.hlsli"

uint g_HasBakedLighting = 0;
Texture2D g_BakedAverageTexture;
Texture2D g_BakedDirectionalTexture;
float4 g_LightmapScaleBias = float4(1.f, 1.f, 0.f, 0.f);
float4 g_LightmapAverageScale = 0.f; // RGB coefficient scale, W enabled
float4 g_LightmapDirectionalScale = 0.f;
uint g_HasEnvironmentCube = 0;
uint g_HasEnvironmentBRDFLookup = 0;
TextureCube g_EnvironmentCubeTexture;
Texture2D g_EnvironmentBRDFLookupTexture;
float4 g_EnvironmentColor = float4(1.f, 1.f, 1.f, 0.f);
float2 g_EnvironmentRotation = float2(0.f, 1.f); // source CB13 (a,b)

sampler SurfaceLightmapSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

sampler SurfaceAnisotropicSampler = sampler_state
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = WRAP;
    AddressV = WRAP;
};

// Original Texture2D AddressX is carried by the named diffuse material.
uint g_DiffuseMirrorU = 0;
sampler SurfaceMirrorUSampler = sampler_state
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = MIRROR;
    AddressV = WRAP;
};

sampler SurfaceMirrorVSampler = sampler_state
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = WRAP;
    AddressV = MIRROR;
};
sampler SurfaceMirrorUVSampler = sampler_state
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = MIRROR;
    AddressV = MIRROR;
};

float4 SampleMapDiffuseTexture(float2 uv, SamplerState wrapSampler)
{
    float4 sampleValue = 0.f;
    if (g_SurfaceProgram == 8u && (g_SourceBgFlags & 1024u) != 0u)
    {
        if (g_DiffuseMirrorU != 0u) sampleValue = g_DiffuseTexture.Sample(SurfaceMirrorUVSampler, uv);
        else sampleValue = g_DiffuseTexture.Sample(SurfaceMirrorVSampler, uv);
    }
    else if (g_DiffuseMirrorU != 0u)
        sampleValue = g_DiffuseTexture.Sample(SurfaceMirrorUSampler, uv);
    else sampleValue = g_DiffuseTexture.Sample(wrapSampler, uv);
    return sampleValue;
}

float3 EvaluateMapSurfaceEmissive(float2 meshUV, uint surfaceProgram)
{
    if (g_HasSurfaceEmissive == 0u)
        return 0.f;
    if (surfaceProgram == 9u || surfaceProgram == 10u)
    {
        const float4 sampleValue = g_SurfaceEmissiveTexture.Sample(SurfaceAnisotropicSampler, meshUV);
        const float phase = g_SurfaceEmissivePhaseOffset + sampleValue.a *
            g_SurfaceEmissiveTime * g_SurfaceEmissiveFlickerSpeed;
        const float flicker = (g_SourceFoliageFlags & 64u) != 0u ?
            SourceFoliageFlicker(phase, g_SurfaceEmissiveFlickerMinimum) : 1.f;
        return sampleValue.rgb * g_SurfaceEmissiveColor.rgb * g_SurfaceEmissiveIntensity * flicker;
    }
    // Source bg_base_pbr_opa flicker, including its nested cosine modulation.
    // phaseOffset owns the unavailable engine-origin constant explicitly.
    const float phase = g_SurfaceEmissivePhaseOffset +
        g_SurfaceEmissiveTime * g_SurfaceEmissiveFlickerSpeed;
    float flicker = 0.5f * (1.f +
        sin((phase + cos(phase * 3.524534f)) * 1.328987f)) +
        g_SurfaceEmissiveFlickerMinimum;
    if (surfaceProgram == 8u)
    {
        if (g_SourceBgFlicker == 0u) flicker = 1.f;
        else if (g_SourceBgFlicker == 2u)
        {
            const float ramp = 2.f * (frac(phase) - 0.5f);
            flicker = ramp * ramp + g_SurfaceEmissiveFlickerMinimum;
        }
    }
    return g_SurfaceEmissiveTexture.Sample(SurfaceAnisotropicSampler,
        meshUV * g_SurfaceEmissiveUVTiling).rgb * g_SurfaceEmissiveColor.rgb *
        g_SurfaceEmissiveIntensity * flicker;
}

float3 EvaluateMapSurfaceEmissive(float2 meshUV)
{
    return EvaluateMapSurfaceEmissive(meshUV, g_SurfaceProgram);
}

bool IsMapSurfacePBR()
{
    return g_SurfaceProgram == 3u || g_SurfaceProgram == 4u;
}

bool IsMapSurfaceSourceSpecular()
{
    return g_SurfaceProgram == 5u;
}

float EncodeMapSurfaceGeometricNormal(float3 normal, bool hasBakedLighting)
{
    normal = MapGeometryNormalizeOrZero(normal);
    normal /= abs(normal.x) + abs(normal.y) + abs(normal.z);
    float2 oct = normal.xy;
    if (normal.z < 0.f)
        oct = (1.f - abs(oct.yx)) * float2(oct.x >= 0.f ? 1.f : -1.f,
            oct.y >= 0.f ? 1.f : -1.f);
    const uint2 packed = (uint2)round(saturate(oct * 0.5f + 0.5f) * 2047.f);
    // PickPos is RGBA32_FLOAT. Its W is always finite/nonzero; CPicking uses
    // XYZ and restores the returned homogeneous W=1. Only depth marker 3
    // authorizes decoding these 22 mantissa bits in the light pass.
    // Mantissa bit 22 marks a real component lightmap, so its baked irradiance
    // replaces the old project ambient addition without editing scene values.
    return asfloat(0x3f800000u | packed.x | (packed.y << 11u) |
        (hasBakedLighting ? 0x00400000u : 0u));
}

float MapSurfaceSafePow(float value, float power)
{
    // Matches the source log/exp branch, including metallic intensity=power=0.
    return abs(value) < 0.000001f ? 0.f : pow(abs(value), power);
}

struct MAP_SURFACE_SAMPLE
{
    float4 diffuse;
    float3 worldNormal;
    float3 tangentNormal;
    float3 specular;
    float3 reflectionDelta;
    float3 transmission;
    float3 subspecularRadiance;
    float3 rimlightRadiance;
    float specularPower;
    float roughness;
    float metallic;
    float ambientOcclusion;
};

#include "Shader_SourceSpecialSurface.hlsli"

bool IsMapSurfaceSourceSpecial() { return g_SurfaceProgram >= 11u && g_SurfaceProgram <= 13u; }

bool IsMapSurfaceSourceFoliage() { return g_SurfaceProgram == 9u || g_SurfaceProgram == 10u; }

MAP_SURFACE_SAMPLE EvaluateMapSourceFoliageSurface(float2 uv, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    const bool grass = g_SurfaceProgram == 10u;
    const float4 raw = g_DiffuseTexture.Sample(SurfaceAnisotropicSampler, uv);
    clip(raw.a - 0.3333f);
    const float4 mask = grass ? 1.f : g_SourceFoliageMaskTexture.Sample(SurfaceAnisotropicSampler, uv);
    float3 diffuse = (g_SourceFoliageFlags & 2u) != 0u ?
        SourceFoliageDesaturate(raw.rgb, g_SurfaceDiffuseSaturation, grass ? 1.f : mask.a) : raw.rgb;
    const float3 specular = grass ? diffuse : ((g_SourceFoliageFlags & 8u) != 0u ?
        g_SpecularTexture.Sample(SurfaceAnisotropicSampler, uv).rgb : mask.rrr);
    diffuse *= grass ? g_SurfaceDiffuseBrightness * g_SurfaceDiffuseColor.rgb :
        1.f + mask.a * (g_SurfaceDiffuseBrightness * g_SurfaceDiffuseColor.rgb - 1.f);
    result.diffuse = float4(diffuse, raw.a);
    result.tangentNormal = (g_SourceFoliageFlags & 1u) != 0u ?
        SourceFoliageNormal(g_NormalTexture.Sample(SurfaceAnisotropicSampler, uv).rg,
            g_SurfaceNormalIntensity) : float3(0.f, 0.f, 1.f);
    const float3x3 tangentToWorld = float3x3(SourceFoliageUnit(tangent),
        SourceFoliageUnit(binormal), SourceFoliageUnit(normal));
    result.worldNormal = SourceFoliageUnit(mul(result.tangentNormal, tangentToWorld));
    result.specular = (g_SourceFoliageFlags & 4u) != 0u ?
        specular * g_SurfaceSpecularColor.rgb * g_SurfaceSpecularIntensity : 0.f;
    result.transmission = grass ? g_SourceFoliageTransmission.rgb :
        ((g_SourceFoliageFlags & 16u) != 0u ?
            mask.b * g_SourceFoliageTransmission.rgb * g_SourceFoliageTransmission.w : 0.f);
    result.ambientOcclusion = 1.f;
    return result;
}

bool IsMapSurfaceSourceBG() { return g_SurfaceProgram == 8u; }

float2 MapSourceBGUV(float2 uv)
{
    const float2 centered = uv - 0.5f;
    return (float2(centered.x * g_SourceBgUV.y - centered.y * g_SourceBgUV.x,
        centered.y * g_SourceBgUV.y + centered.x * g_SourceBgUV.x) + 0.5f) *
        g_SurfaceUVTiling + g_SourceBgUV.zw + g_SourceBgPanning * g_SurfaceEmissiveTime;
}

float4 MapSourceBGDiffuse(float2 meshUV, float3 tangentView, out float2 surfaceUV, out float bumpShade)
{
    surfaceUV = MapSourceBGUV(meshUV);
    const float4 original = SampleMapDiffuseTexture(surfaceUV, SurfaceAnisotropicSampler);
    float4 shifted = original;
    bumpShade = 1.f;
    if ((g_SourceBgFlags & 2u) != 0u)
    {
        surfaceUV += (original.a - g_SourceBgBump.x) * g_SourceBgBump.y * tangentView.xy;
        shifted = SampleMapDiffuseTexture(surfaceUV, SurfaceAnisotropicSampler);
        bumpShade = saturate(original.a + g_SourceBgBump.z);
    }
    if ((g_SourceBgFlags & 64u) != 0u)
        clip(((g_SourceBgFlags & 2u) != 0u ? saturate(original.a + shifted.a) : shifted.a) - 0.3333f);
    return shifted;
}

MAP_SURFACE_SAMPLE EvaluateMapSourceBGSurface(float2 meshUV, float4 vertexColor, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    result.ambientOcclusion = 1.f;
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent), MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    const float3 tangentView = normalize(mul(tangentToWorld, normalize(g_vCamPosition.xyz - worldPosition)));
    float2 surfaceUV; float bumpShade;
    const float4 diffuse = MapSourceBGDiffuse(meshUV, tangentView, surfaceUV, bumpShade);
    float4 encodedNormal = float4(0.5f, 0.5f, 1.f, 1.f);
    float3 tangentNormal = float3(0.f, 0.f, 1.f);
    if ((g_SourceBgFlags & 1u) != 0u)
    {
        const float2 normalUV = (g_SourceBgFlags & 16384u) != 0u ? meshUV : surfaceUV;
        if ((g_SourceBgFlags & 6144u) == 6144u)
            encodedNormal = g_NormalTexture.Sample(SurfaceMirrorUVSampler, normalUV);
        else if ((g_SourceBgFlags & 2048u) != 0u)
            encodedNormal = g_NormalTexture.Sample(SurfaceMirrorUSampler, normalUV);
        else if ((g_SourceBgFlags & 4096u) != 0u)
            encodedNormal = g_NormalTexture.Sample(SurfaceMirrorVSampler, normalUV);
        else encodedNormal = g_NormalTexture.Sample(SurfaceAnisotropicSampler, normalUV);
        const float2 raw = encodedNormal.rg * 2.f - 1.f;
        const float vertexStrength = (g_SourceBgFlags & 128u) != 0u ? vertexColor.a : 1.f;
        const float2 detail = (g_SourceBgFlags & 32768u) != 0u ?
            (g_DetailNormalTexture.Sample(SurfaceAnisotropicSampler, surfaceUV * g_SurfaceDetailNormalTiling).rg * 2.f - 1.f) * g_SurfaceDetailNormalIntensity : 0.f;
        tangentNormal = normalize(float3((raw * g_SurfaceNormalIntensity + detail) * vertexStrength,
            sqrt(max(1.f - dot(raw, raw), 0.f)) + 0.00001f));
    }
    result.tangentNormal = tangentNormal;
    result.worldNormal = normalize(mul(tangentNormal, tangentToWorld));
    const float luminance = dot(diffuse.rgb, float3(0.3f, 0.59f, 0.11f));
    const float3 baseDiffuse = lerp(luminance.xxx, diffuse.rgb, g_SurfaceDiffuseSaturation) *
        g_SurfaceDiffuseColor.rgb * g_SurfaceDiffuseBrightness * bumpShade;
    result.diffuse = float4(baseDiffuse, diffuse.a);
    float3 subspecularColor = 0.f;
    if ((g_SourceBgFlags & 4u) != 0u || g_SourceBgSubspecular.x != 0.f)
    {
        const float3 sampledSpecular = (g_SourceBgFlags & 8u) != 0u ?
            g_SpecularTexture.Sample(SurfaceAnisotropicSampler, surfaceUV).rgb : diffuse.rgb;
        const float3 specular = lerp(dot(sampledSpecular, float3(0.3f, 0.59f, 0.11f)).xxx,
            sampledSpecular, g_SourceBgSpecularSaturation);
        subspecularColor = ((g_SourceBgFlags & 256u) != 0u && (g_SourceBgFlags & 8u) == 0u ?
            dot(specular, float3(0.3f, 0.59f, 0.11f)).xxx : specular) *
            g_SurfaceSpecularColor.rgb * g_SurfaceSpecularIntensity;
        if ((g_SourceBgFlags & 4u) != 0u) result.specular = subspecularColor;
    }
    if ((g_SourceBgFlags & 16u) != 0u)
    {
        const float3 reflected = reflect(-tangentView, tangentNormal);
        float2 reflectionUV;
        if ((g_SourceBgFlags & 32u) != 0u)
        {
            const float3 worldReflection = mul(reflected, tangentToWorld);
            reflectionUV = float2(worldReflection.x, -worldReflection.z) * g_SurfaceReflectionTiling + g_SurfaceReflectionOriginOffset;
        }
        else reflectionUV = lerp(MapSourceBGUV(meshUV), (reflected.xy + 0.5f) * 0.5f, 0.75f);
        const float3 sampled = g_ReflectionTexture.Sample(SurfaceAnisotropicSampler, reflectionUV).rgb;
        const float3 signedReflection = sampled * g_SurfaceReflectionColor.rgb * (1.f + g_SurfaceReflectionContrast) - g_SurfaceReflectionContrast;
        const float weight = ((g_SourceBgFlags & 1u) != 0u ? encodedNormal.a : 1.f) * g_SurfaceReflectionIntensity;
        result.diffuse.rgb = (g_SourceBgFlags & 8192u) != 0u ?
            lerp(baseDiffuse, sampled * g_SurfaceReflectionColor.rgb, weight) :
            clamp((baseDiffuse + weight * saturate(signedReflection)) *
            (1.f - weight * saturate(-signedReflection)), 0.f, 999.f);
        result.reflectionDelta = abs(result.diffuse.rgb - baseDiffuse);
    }
    const float facing = saturate(dot(tangentNormal, tangentView));
    result.subspecularRadiance = subspecularColor * g_SourceBgSubspecular.x *
        MapSurfaceSafePow(facing * facing, g_SourceBgSubspecular.y);
    const float rim = (1.f - abs(dot(tangentNormal, tangentView))) * (1.f - abs(tangentView.z));
    result.rimlightRadiance = g_SourceBgRimlight.rgb * MapSurfaceSafePow(rim, g_SourceBgRimlight.w);
    return result;
}

MAP_SURFACE_SAMPLE EvaluateMapSourceSpecularSurface(float2 meshUV, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    // Actual MAGICFLOOR component MIC selects this tiled, opaque source
    // permutation. It is distinct from the older program-1 compatibility path.
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    const float2 uv = meshUV * g_SurfaceUVTiling;
    const float4 diffuse = g_DiffuseTexture.Sample(SurfaceAnisotropicSampler, uv);
    const float4 encodedNormal = g_NormalTexture.Sample(SurfaceAnisotropicSampler, uv);
    const float2 xy = encodedNormal.rg * 2.f - 1.f;
    const float z = sqrt(max(1.f - dot(xy, xy), 0.f)) + 0.00001f;
    result.tangentNormal = normalize(float3(xy * g_SurfaceNormalIntensity, z));
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent), MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    result.worldNormal = normalize(mul(result.tangentNormal, tangentToWorld));
    const float3 view = normalize(mul(tangentToWorld, normalize(g_vCamPosition.xyz - worldPosition)));
    const float3 reflected = mul(reflect(-view, result.tangentNormal), tangentToWorld);
    const float2 reflectionUV = float2(reflected.x, -reflected.z) *
        g_SurfaceReflectionTiling + g_SurfaceReflectionOriginOffset;
    const float3 reflection = g_ReflectionTexture.Sample(SurfaceAnisotropicSampler, reflectionUV).rgb;
    const float3 q = reflection * g_SurfaceReflectionColor.rgb *
        (1.f + g_SurfaceReflectionContrast) - g_SurfaceReflectionContrast;
    const float w = encodedNormal.a * g_SurfaceReflectionIntensity;
    const float3 base = diffuse.rgb * g_SurfaceDiffuseColor.rgb * g_SurfaceDiffuseBrightness;
    result.diffuse = float4(clamp((base + w * saturate(q)) * (1.f - w * saturate(-q)), 0.f, 999.f), 1.f);
    result.reflectionDelta = abs(result.diffuse.rgb - base);
    result.specular = g_SpecularTexture.Sample(SurfaceAnisotropicSampler, uv).rgb *
        g_SurfaceSpecularColor.rgb * g_SurfaceSpecularIntensity;
    result.roughness = 0.f;
    result.metallic = 0.f;
    result.ambientOcclusion = 1.f;
    return result;
}

MAP_SURFACE_SAMPLE EvaluateMapPBRSurface(float2 meshUV, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    const float2 tiledUV = meshUV * g_SurfaceUVTiling;
    const float2 normalUV = g_SurfaceUVFixedNormal != 0u ? meshUV : tiledUV;
    const float4 diffuse = g_DiffuseTexture.Sample(SurfaceAnisotropicSampler, tiledUV);
    const float4 encodedNormal = g_NormalTexture.Sample(SurfaceAnisotropicSampler, normalUV);
    const float2 detailXY = g_DetailNormalTexture.Sample(SurfaceAnisotropicSampler,
        tiledUV * g_SurfaceDetailNormalTiling).rg * 2.f - 1.f;
    const float2 rawXY = encodedNormal.rg * 2.f - 1.f;
    const float rawZ = sqrt(max(1.f - dot(rawXY, rawXY), 0.f)) + 0.00001f;
    float3 tangentNormal = normalize(float3((rawXY * g_SurfaceNormalIntensity +
        detailXY * g_SurfaceDetailNormalIntensity) * g_SurfaceVertexAlpha, rawZ));
    // The seamless permutation explicitly removes near-flat normal noise.
    if (g_SurfaceProgram == 3u && tangentNormal.z >= 0.99985f)
        tangentNormal = float3(0.f, 0.f, 1.f);
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    result.tangentNormal = tangentNormal;
    result.worldNormal = normalize(mul(tangentNormal, tangentToWorld));
    const float3 worldView = normalize(g_vCamPosition.xyz - worldPosition);
    const float3 tangentView = normalize(mul(tangentToWorld, worldView));
    const float3 tangentReflection = reflect(-tangentView, tangentNormal);
    float2 reflectionUV;
    if (g_SurfaceUseWorldReflection != 0u)
    {
        const float3 worldReflection = mul(tangentReflection, tangentToWorld);
        reflectionUV = float2(worldReflection.x, -worldReflection.z) *
            g_SurfaceReflectionTiling + g_SurfaceReflectionOriginOffset;
    }
    else
        reflectionUV = lerp(meshUV, (tangentReflection.xy + 0.5f) * 0.5f, 0.75f);

    const float luminance = dot(diffuse.rgb, float3(0.3f, 0.59f, 0.11f));
    const float3 baseDiffuse = lerp(luminance.xxx, diffuse.rgb,
        g_SurfaceDiffuseSaturation) * g_SurfaceDiffuseColor.rgb * g_SurfaceDiffuseBrightness;
    const float3 reflection = g_ReflectionTexture.Sample(SurfaceAnisotropicSampler, reflectionUV).rgb;
    const float3 signedReflection = reflection * g_SurfaceReflectionColor.rgb *
        (1.f + g_SurfaceReflectionContrast) - g_SurfaceReflectionContrast;
    const float weight = encodedNormal.a * g_SurfaceReflectionIntensity;
    const float3 reflectedDiffuse = clamp((baseDiffuse + weight * saturate(signedReflection)) *
        (1.f - weight * saturate(-signedReflection)), 0.f, 999.f);
    result.reflectionDelta = abs(reflectedDiffuse - baseDiffuse);

    const float3 orm = g_SurfaceORMTexture.Sample(SurfaceAnisotropicSampler, tiledUV).rgb;
    result.metallic = saturate(MapSurfaceSafePow(orm.b * g_SurfaceMetallicIntensity,
        g_SurfaceMetallicPower));
    result.roughness = saturate(max(g_SurfaceMinimumRoughness,
        MapSurfaceSafePow(orm.g * clamp(g_SurfaceRoughnessIntensity, 0.f, 100.f),
            g_SurfaceRoughnessPower)));
    result.ambientOcclusion = saturate(MapSurfaceSafePow(orm.r * g_SurfaceAOIntensity,
        g_SurfaceAOPower));
    // Native hit/selection color and engine global material adjustments use
    // their inactive identities here. Component lightmaps are evaluated below;
    // the native environment lookup remains a separate unresolved input.
    const float3 albedo = saturate(reflectedDiffuse * lerp(g_SurfaceNonmetallicBrightness,
        g_SurfaceMetallicBrightness, result.metallic));
    result.diffuse = float4(albedo, 1.f); // Native opaque permutations never alpha-clip.
    result.specular = lerp((0.08f * saturate(g_SurfaceSpecularPBRIntensity)).xxx,
        albedo, result.metallic);
    return result;
}

float3 MapSourceAOMultiBounce(float ao, float3 color)
{
    // Source BasePass polynomial, not a generic AO multiplication.
    const float3 a = 2.0404f * color - 0.3324f;
    const float3 b = -4.7951f * color + 0.6417f;
    const float3 c = 2.7552f * color + 0.6903f;
    return max(ao.xxx, ((ao * a + b) * ao + c) * ao);
}

float MapSourceDirectionalLightmapWeight(float3 tangentDirection, float3 coefficients)
{
    float3 weights = saturate(float3(
        dot(tangentDirection.yz, float2(0.81649658f, 0.57735027f)),
        dot(tangentDirection, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
        dot(tangentDirection, float3(0.70710678f, -0.40824829f, 0.57735027f))));
    return dot(coefficients, weights * weights);
}

float3 EvaluateMapSourceBGIndirectLighting(MAP_SURFACE_SAMPLE surface, float2 lightmapUV,
    float4 averageScale, float4 directionalScale, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal, bool sourceSpecial)
{
    if (g_HasBakedLighting == 0u || averageScale.w == 0.f) return surface.subspecularRadiance;
    const float3 average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * averageScale.rgb;
    const float3 coefficients = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * directionalScale.rgb;
    float3 radiance = surface.diffuse.rgb * average *
        MapSourceDirectionalLightmapWeight(surface.tangentNormal, coefficients);
    if (sourceSpecial || (g_SourceBgFlags & 4u) != 0u)
    {
        const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent), MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
        const float3 tangentView = normalize(mul(tangentToWorld, g_vCamPosition.xyz - worldPosition));
        const float3 reflected = reflect(-tangentView, surface.tangentNormal);
        const float3 lobes = saturate(float3(
            dot(reflected.yz, float2(0.81649658f, 0.57735027f)),
            dot(reflected, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
            dot(reflected, float3(0.70710678f, -0.40824829f, 0.57735027f))));
        radiance += surface.specular * average * dot(coefficients, pow(lobes, (sourceSpecial ? surface.specularPower : g_SurfaceSpecularPower) + 1.f));
    }
    return radiance + surface.subspecularRadiance;
}

float3 EvaluateMapSourceIndirectLighting(MAP_SURFACE_SAMPLE surface, float2 lightmapUV,
    float4 averageScale, float4 directionalScale, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    if (IsMapSurfaceSourceFoliage())
    {
        const bool grass = g_SurfaceProgram == 10u;
        float3 radiance = grass ? surface.diffuse.rgb * surface.transmission : 0.f;
        if (g_HasBakedLighting == 0u || averageScale.w == 0.f) return radiance;
        const float3 average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * averageScale.rgb;
        const float3 coefficients = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * directionalScale.rgb;
        const float3x3 tangentToWorld = float3x3(SourceFoliageUnit(tangent), SourceFoliageUnit(binormal), SourceFoliageUnit(normal));
        const float3 tangentView = mul(tangentToWorld, g_vCamPosition.xyz - worldPosition);
        return radiance + SourceFoliageBaked(surface.diffuse.rgb, surface.tangentNormal,
            surface.specular, surface.transmission, tangentView, average, coefficients,
            g_SurfaceSpecularPower, grass);
    }
    if (IsMapSurfaceSourceBG() || IsMapSurfaceSourceSpecial())
        return EvaluateMapSourceBGIndirectLighting(surface, lightmapUV,
            averageScale, directionalScale, worldPosition, tangent, binormal, normal,
            IsMapSurfaceSourceSpecial());
    if (IsMapSurfaceSourceSpecular())
    {
        // The original non-PBR BasePass adds three powered reflection lobes
        // in UE world coordinates. This is source material behavior, not IBL.
        const float3 reflectedWorld = reflect(-normalize(g_vCamPosition.xyz - worldPosition),
            surface.worldNormal);
        const float3 reflectedUE = float3(reflectedWorld.x, -reflectedWorld.z, reflectedWorld.y);
        const float3 dots = min(abs(float3(
            dot(reflectedUE.yz, float2(0.81649658f, 0.57735027f)),
            dot(reflectedUE, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
            dot(reflectedUE, float3(0.70710678f, -0.40824829f, 0.57735027f)))), 1.f);
        const float3 powered = pow(dots, g_SurfaceSpecularPower);
        float3 indirect = surface.specular * (powered.x + powered.y + powered.z);
        if (g_HasBakedLighting != 0u && averageScale.w != 0.f)
        {
            const float3 average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler,
                lightmapUV).rgb * averageScale.rgb;
            const float3 direction = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler,
                lightmapUV).rgb * directionalScale.rgb;
            indirect += surface.diffuse.rgb * average *
                MapSourceDirectionalLightmapWeight(surface.tangentNormal, direction);
        }
        // Source hemisphere, hit/selection and global CB colors remain at
        // inactive identities; PBR AO/cube/lookup do not belong to this family.
        return indirect;
    }
    if (!IsMapSurfacePBR() || g_HasBakedLighting == 0u || averageScale.w == 0.f)
        return 0.f;
    const float3 average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler,
        lightmapUV).rgb * averageScale.rgb;
    const float3 direction = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler,
        lightmapUV).rgb * directionalScale.rgb;
    const float3 n = surface.tangentNormal;
    const float normalIrradiance = MapSourceDirectionalLightmapWeight(n, direction);
    // Native directional-lightmap BasePass adds this term before applying
    // material AO's color-dependent bounce. The source term has no (1-M).
    float3 indirect = surface.diffuse.rgb * average * normalIrradiance *
        MapSourceAOMultiBounce(surface.ambientOcclusion, surface.diffuse.rgb);
    if (g_HasEnvironmentCube == 0u || g_HasEnvironmentBRDFLookup == 0u)
        return indirect;

    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    const float3 worldView = normalize(g_vCamPosition.xyz - worldPosition);
    const float3 tangentView = normalize(mul(tangentToWorld, worldView));
    const float normalView = dot(n, tangentView);
    const float3 reflection = reflect(-tangentView, n);
    const float lookupView = saturate(normalView + 1.f - min(reflection.z + 1.f, 1.f));
    const float2 normalViewGradient = float2(ddx_coarse(normalView), ddy_coarse(normalView));
    const float roughnessAA = saturate(surface.roughness + 0.3f * length(normalViewGradient));
    // The SRV currently contains a numerically integrated PROJECT approximation
    // of the unresolved native t5 lookup, in the same R=Fresnel/G=base contract.
    const float2 lookup = g_EnvironmentBRDFLookupTexture.Sample(SurfaceLightmapSampler,
        float2(lookupView, roughnessAA)).rg;
    const float3 fresnelDelta = saturate(50.f * surface.specular.g) *
        (max((1.f - roughnessAA).xxx, surface.specular) - surface.specular);
    const float3 reflectionBRDF = (lookup.g * surface.specular + lookup.r * fresnelDelta) *
        (1.f + surface.specular * (rcp(max(lookup.g, 0.000001f)) - 1.f));

    const float3 worldReflection = mul(reflection, tangentToWorld);
    // UE->runtime is (x,z,-y). Source CB13 rotates UE XY before the cube's XZY
    // sampling permutation. (0,1) remains an explicit project interpretation.
    const float a = g_EnvironmentRotation.x, b = g_EnvironmentRotation.y;
    const float3 cubeDirection = float3(b * worldReflection.x + a * worldReflection.z,
        worldReflection.y, a * worldReflection.x - b * worldReflection.z);
    const float4 encodedEnvironment = g_EnvironmentCubeTexture.SampleLevel(
        SurfaceLightmapSampler, cubeDirection, roughnessAA * 5.f);
    const float3 environment = encodedEnvironment.rgb * encodedEnvironment.a *
        g_EnvironmentColor.rgb * 6.f + g_EnvironmentColor.w;
    const float reflectedIrradiance = MapSourceDirectionalLightmapWeight(reflection, direction);
    const float3 specularIrradiance = average * lerp(reflectedIrradiance,
        normalIrradiance, surface.roughness);
    const float specularAO = saturate(MapSurfaceSafePow(surface.ambientOcclusion + lookupView,
        roughnessAA * roughnessAA) + surface.ambientOcclusion - 1.f);
    const float f0Luminance = dot(surface.specular, float3(0.3f, 0.59f, 0.11f));
    indirect += environment * specularIrradiance * reflectionBRDF *
        MapSourceAOMultiBounce(specularAO, f0Luminance.xxx);
    // Native hemisphere CB22/23/24 and SH9->CB packing are unresolved. They are
    // not replaced by a synthetic ambient tint: only the bound baked irradiance
    // drives this explicitly partial environment reconstruction.
    return indirect;
}

struct MAP_STONE_GBUFFER
{
    float4 diffuse;
    float4 normal;
    float4 depth;
    float4 pickPosition;
    float4 indirect;
    float4 materialSpecular;
    float4 surface;
    float4 geometry;
};

float2 MapSourceOverlayUV(float2 meshUV)
{
    const float2 centeredUV = meshUV - 0.5f;
    return (float2(dot(centeredUV, float2(g_SourceOverlayUV.y, -g_SourceOverlayUV.x)),
        dot(centeredUV, float2(g_SourceOverlayUV.x, g_SourceOverlayUV.y))) + 0.5f) * g_SurfaceUVTiling + g_SourceOverlayUV.zw;
}

MAP_STONE_GBUFFER EvaluateMapSourceStoneGeometry(float2 meshUV, float4 vertexColor,
    float3 worldPosition, float3 tangent, float3 binormal, float3 normal,
    float4 projectedPosition, float2 lightmapUV, float4 averageScale,
    float4 directionalScale)
{
    SOURCE_STONE_PARAMETERS material;
    material.diffuseColor = g_SurfaceDiffuseColor.rgb;
    material.overlayColor = g_SurfaceOverlayColor.rgb;
    material.specularColor = g_SurfaceSpecularColor.rgb;
    material.normalIntensity = g_SurfaceNormalIntensity;
    material.overlayNormalIntensity = g_SurfaceOverlayNormalIntensity;
    material.overlaySharpness = g_SurfaceOverlaySharpness;
    material.diffuseSaturation = g_SurfaceDiffuseSaturation;
    material.diffuseBrightness = g_SurfaceDiffuseBrightness;
    material.overlayBrightness = g_SurfaceOverlayBrightness;
    material.overlaySaturation = g_SurfaceOverlaySaturation;
    material.specularIntensity = g_SurfaceSpecularIntensity;
    material.overlaySpecularIntensity = g_SurfaceOverlaySpecularIntensity;
    material.specularPower = g_SurfaceSpecularPower;
    const float2 overlayUV = meshUV * g_SurfaceOverlayTiling;
    const float2 surfaceUV = MapSourceOverlayUV(meshUV);
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    const float4 sampledDiffuse = SampleMapDiffuseTexture(surfaceUV, SurfaceAnisotropicSampler);
    if ((g_SourceOverlayFlags & 64u) != 0u) clip(sampledDiffuse.a - 0.3333f);
    // Original material instructions use SampleBias(0). The existing aniso
    // sampler is the project filtering adapter; it does not alter UV domains.
    SOURCE_STONE_SURFACE stone = EvaluateSourceStoneVariants(
        sampledDiffuse,
        (g_SourceOverlayFlags & 1u) != 0u ? g_NormalTexture.Sample(SurfaceAnisotropicSampler,
            (g_SourceOverlayFlags & 128u) != 0u ? meshUV : surfaceUV) : float4(0.5f, 0.5f, 1.f, 1.f),
        g_SurfaceOverlayDiffuseTexture.Sample(SurfaceAnisotropicSampler, overlayUV),
        (g_SourceOverlayFlags & 2u) != 0u ? g_SurfaceOverlayNormalTexture.Sample(SurfaceAnisotropicSampler, overlayUV) : float4(0.5f, 0.5f, 1.f, 1.f),
        (g_SourceOverlayFlags & 32u) != 0u ? g_DetailNormalTexture.Sample(SurfaceAnisotropicSampler,
            surfaceUV * g_SurfaceDetailNormalTiling) : float4(0.5f, 0.5f, 1.f, 1.f),
        vertexColor, material, g_SourceOverlayFlags, mul(tangentToWorld, g_SourceOverlayDirection.xyz),
        g_SourceOverlayDirection.w, g_SurfaceDetailNormalIntensity);
    if (g_SurfaceOverlaySeparateSpecular != 0u)
    {
        const float3 baseSpecular = g_SpecularTexture.Sample(SurfaceAnisotropicSampler,
            surfaceUV).rgb * material.specularColor * material.specularIntensity;
        const float3 overlaySpecular = SourceStoneSaturation(g_SurfaceOverlayDiffuseTexture.Sample(
            SurfaceAnisotropicSampler, overlayUV).rgb, material.overlaySaturation) * material.overlaySpecularIntensity;
        stone.specular = lerp(baseSpecular, overlaySpecular, stone.overlayWeight);
    }
    const bool hasLightmap = g_HasBakedLighting != 0u && averageScale.w != 0.f;
    float3 average = 0.f, coefficients = 0.f;
    if (hasLightmap)
    {
        average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler,
            lightmapUV).rgb * averageScale.rgb;
        coefficients = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler,
            lightmapUV).rgb * directionalScale.rgb;
    }
    const SOURCE_STONE_BASE_LIGHT base = EvaluateSourceStoneBase(stone,
        mul(tangentToWorld, g_vCamPosition.xyz - worldPosition),
        mul(tangentToWorld, float3(0.f, 1.f, 0.f)), average, coefficients,
        hasLightmap, material.specularPower, SourceStoneInactiveEngineInputs());
    MAP_STONE_GBUFFER output;
    // RT0 is the ordinary albedo debug view. HDR shading uses RT4/RT6, never
    // multiplies this UNORM value back into an already colored source result.
    output.diffuse = float4(base.diffuse, 1.f);
    output.normal = float4(SourceStoneUnit(mul(stone.baseNormal, tangentToWorld)) *
        0.5f + 0.5f, 0.f);
    output.depth = float4(projectedPosition.z / projectedPosition.w,
        projectedPosition.w / 1000.f, material.specularPower, 7.f);
    output.pickPosition = float4(worldPosition,
        EncodeMapSurfaceGeometricNormal(normal, hasLightmap));
    output.indirect = float4(base.radiance, 0.f);
    output.materialSpecular = float4(stone.specular, stone.overlayWeight);
    output.surface = float4(stone.diffuse, 0.f);
    // Preserve the length of the mixed normal: source Direct never normalizes
    // it. The selected Base/Baked normal is normalized inside the pure helper.
    output.geometry = float4(mul(stone.directMixedNormal, tangentToWorld), 0.f);
    return output;
}

MAP_SURFACE_SAMPLE EvaluateMapSurface(float2 meshUV, float4 vertexColor, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    if (g_SurfaceProgram == 11u)
        return EvaluateMapSourceIceSurface(meshUV, vertexColor, worldPosition, tangent, binormal, normal);
    if (g_SurfaceProgram == 12u)
        return EvaluateMapSourceVertexBlendSurface(meshUV, vertexColor, worldPosition, tangent, binormal, normal);
    if (g_SurfaceProgram == 13u)
        return EvaluateMapSourceWetSurface(meshUV, vertexColor, worldPosition, tangent, binormal, normal);
    if (IsMapSurfaceSourceFoliage())
        return EvaluateMapSourceFoliageSurface(meshUV, worldPosition, tangent, binormal, normal);
    if (IsMapSurfaceSourceBG())
        return EvaluateMapSourceBGSurface(meshUV, vertexColor, worldPosition, tangent, binormal, normal);
    if (IsMapSurfaceSourceSpecular())
        return EvaluateMapSourceSpecularSurface(meshUV, worldPosition, tangent, binormal, normal);
    if (IsMapSurfacePBR())
        return EvaluateMapPBRSurface(meshUV, worldPosition, tangent, binormal, normal);
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    result.roughness = 0.f;
    result.metallic = 0.f;
    result.ambientOcclusion = 1.f;
    const float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, meshUV);
    clip(diffuse.a - 0.3333f);
    const float4 encodedNormal = g_NormalTexture.Sample(LinearSampler, meshUV);
    const float2 rawXY = encodedNormal.rg * 2.f - 1.f;
    const float rawZ = sqrt(max(1.f - dot(rawXY, rawXY), 0.f)) + 0.00001f;
    // The opted-in top mesh has no exported COLOR_0. Its vertex-alpha factor
    // uses the default 1; native placement override-color semantics remain open.
    const float3 tangentNormal = normalize(float3(rawXY * g_SurfaceNormalIntensity, rawZ));
    // These source meshes preserve tangent handedness in their recovered WModel.
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    result.tangentNormal = tangentNormal;
    result.worldNormal = normalize(mul(tangentNormal, tangentToWorld));
    const float3 worldView = normalize(g_vCamPosition.xyz - worldPosition);
    const float3 tangentView = normalize(mul(tangentToWorld, worldView));
    const float3 tangentReflection = reflect(-tangentView, tangentNormal);
    float2 reflectionUV;
    float3 baseDiffuse = diffuse.rgb;
    if (g_SurfaceProgram == 1u)
    {
        const float3 worldReflection = mul(tangentReflection, tangentToWorld);
        // Source (x,y,z) -> runtime (x,z,-y). The source CB0 engine-origin
        // identity is unresolved: its extra xy*0.0003 offset is explicitly 0.
        reflectionUV = float2(worldReflection.x, -worldReflection.z) * g_SurfaceReflectionTiling;
        result.specular = g_SpecularTexture.Sample(LinearSampler, meshUV).rgb;
    }
    else
    {
        reflectionUV = lerp(meshUV, (tangentReflection.xy + 0.5f) * 0.5f, 0.75f);
        const float luminance = dot(diffuse.rgb, float3(0.3f, 0.59f, 0.11f));
        baseDiffuse = lerp(luminance.xxx, diffuse.rgb, g_SurfaceDiffuseSaturation);
        // This family's disabled S texture branch consumes original D.rgb.
        result.specular = diffuse.rgb;
    }
    baseDiffuse *= g_SurfaceDiffuseColor.rgb * g_SurfaceDiffuseBrightness;
    result.specular *= g_SurfaceSpecularColor.rgb * g_SurfaceSpecularIntensity;
    const float3 reflection = g_ReflectionTexture.Sample(LinearSampler, reflectionUV).rgb;
    const float3 signedReflection = reflection * g_SurfaceReflectionColor.rgb *
        (1.f + g_SurfaceReflectionContrast) - g_SurfaceReflectionContrast;
    const float weight = encodedNormal.a * g_SurfaceReflectionIntensity;
    result.diffuse = float4(clamp((baseDiffuse + weight * saturate(signedReflection)) *
        (1.f - weight * saturate(-signedReflection)), 0.f, 999.f), diffuse.a);
    result.reflectionDelta = abs(result.diffuse.rgb - baseDiffuse);
    return result;
}
#endif
