#ifndef LOSTARK_SOURCE_SPECIAL_SURFACE
#define LOSTARK_SOURCE_SPECIAL_SURFACE

// Original snow/ice, vertex blend and wet material expressions. The caller
// supplies source texture color spaces and the mesh's actual vertex RGBA.
uint g_SourceSpecialFlags = 0u;
float g_SourceNormalTiling = 1.f;
float4 g_SourceIceCoreColor = 0.f;
float4 g_SourceIceOuterColor = 0.f;
float4 g_SourceIceBlend = float4(1.f, .5f, .5f, 0.f); // mask UV, amount, sharpness, emission
float g_SourceIceBumpOffset = .3f;
float4 g_SourceWetParameters = float4(1.f, .5f, 1.f, 1.f); // normal, sharpness, opacity, specular
float g_SourceWetSpecularPower = 50.f;
float4 g_SourceBlendDiffuse[4]; // RGB tint, brightness
float4 g_SourceBlendSpecular[4]; // RGB tint, intensity
float4 g_SourceBlendLayers[4]; // UV XY, normal strength, specular power
float g_SourceBlendSharpness = .5f;
Texture2D g_SourceSpecialMaskTexture;
Texture2D g_SourceBlendDiffuseGTexture;
Texture2D g_SourceBlendDiffuseBTexture;
Texture2D g_SourceBlendNormalGTexture;
Texture2D g_SourceBlendNormalBTexture;

float3 SourceSpecialNormal(float4 encoded, float strength)
{
    const float2 xy = encoded.rg * 2.f - 1.f;
    return float3(xy * strength, sqrt(max(1.f - dot(xy, xy), 0.f)) + .00001f);
}

float SourceSpecialSharp(float value, float sharpness)
{
    const float threshold = clamp(sharpness, 0.f, .99f);
    return saturate((value - threshold) / (1.f - threshold));
}

MAP_SURFACE_SAMPLE EvaluateMapSourceIceSurface(float2 uv, float4 vertexColor,
    float3 worldPosition, float3 tangent, float3 binormal, float3 normal)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    const float3 view = normalize(mul(tangentToWorld, g_vCamPosition.xyz - worldPosition));
    float3 n = SourceSpecialNormal(g_NormalTexture.Sample(SurfaceAnisotropicSampler,
        uv * g_SourceNormalTiling), g_SurfaceNormalIntensity);
    if ((g_SourceSpecialFlags & 1u) != 0u)
        n.xy += (g_DetailNormalTexture.Sample(SurfaceAnisotropicSampler,
            uv * g_SurfaceDetailNormalTiling).rg * 2.f - 1.f) * g_SurfaceDetailNormalIntensity;
    n.xy *= vertexColor.a;
    n = normalize(n);
    const float fresnel = 1.f - saturate(dot(n, view));
    float3 ice = lerp(g_SourceIceCoreColor.rgb, g_SourceIceOuterColor.rgb, fresnel * fresnel);
    const float3 environment = g_ReflectionTexture.Sample(SurfaceAnisotropicSampler,
        reflect(-view, n).xy).rgb * g_SurfaceReflectionColor.rgb;
    ice = (ice + environment) * (1.f + environment) * .5f;
    const float mask = g_SourceSpecialMaskTexture.Sample(SurfaceAnisotropicSampler,
        uv * g_SourceIceBlend.x).r + (g_SourceIceBlend.y - .5f) * 2.f;
    const float blend = SourceSpecialSharp(mask, g_SourceIceBlend.z);
    const float offset = (1.f - mask) * g_SourceIceBumpOffset * .05f - .025f;
    const float2 diffuseUV = uv * g_SurfaceUVTiling + offset * view.xy;
    const float3 rawDiffuse = g_DiffuseTexture.Sample(SurfaceAnisotropicSampler, diffuseUV).rgb;
    result.diffuse = float4(lerp(rawDiffuse * g_SurfaceDiffuseColor.rgb, ice, blend), 1.f);
    result.subspecularRadiance = ice * (g_SourceIceBlend.w * blend);
    result.specular = ((g_SourceSpecialFlags & 2u) != 0u ?
        g_SpecularTexture.Sample(SurfaceAnisotropicSampler, diffuseUV).rgb : rawDiffuse) * g_SurfaceSpecularIntensity;
    result.specularPower = g_SurfaceSpecularPower;
    result.tangentNormal = n;
    result.worldNormal = normalize(mul(n, tangentToWorld));
    return result;
}

MAP_SURFACE_SAMPLE EvaluateMapSourceWetSurface(float2 uv, float4 vertexColor,
    float3 worldPosition, float3 tangent, float3 binormal, float3 normal)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    const float3 view = normalize(mul(tangentToWorld, g_vCamPosition.xyz - worldPosition));
    const float3 dryNormal = SourceSpecialNormal(g_NormalTexture.Sample(SurfaceAnisotropicSampler,
        uv * g_SourceNormalTiling), g_SurfaceNormalIntensity);
    const float3 wetNormal = (dryNormal - float3(0.f, 0.f, 1.f)) *
        g_SourceWetParameters.x + float3(0.f, 0.f, 1.f);
    const float4 rawDiffuse = g_DiffuseTexture.Sample(SurfaceAnisotropicSampler, uv * g_SurfaceUVTiling);
    const float sharpened = SourceSpecialSharp(vertexColor.r, g_SourceWetParameters.y);
    const float blend = lerp(vertexColor.r, sharpened * sharpened, rawDiffuse.a);
    const float3 n = normalize(lerp(dryNormal, wetNormal, blend));
    const float3 reflection = g_ReflectionTexture.Sample(SurfaceAnisotropicSampler,
        (reflect(-view, n).xy + .5f) * .5f).rgb * g_SurfaceReflectionColor.rgb;
    const float3 diffuse = rawDiffuse.rgb * g_SurfaceDiffuseColor.rgb * g_SurfaceDiffuseBrightness;
    result.diffuse = float4(lerp(diffuse, (diffuse + 1.f) * reflection,
        saturate(blend * g_SourceWetParameters.z)), 1.f);
    result.specular = lerp(g_SpecularTexture.Sample(SurfaceAnisotropicSampler,
        uv * g_SurfaceUVTiling).rgb * g_SurfaceSpecularColor.rgb * g_SurfaceSpecularIntensity,
        g_SourceWetParameters.w.xxx, blend);
    result.specularPower = lerp(g_SurfaceSpecularPower, g_SourceWetSpecularPower, blend);
    result.tangentNormal = n;
    result.worldNormal = normalize(mul(n, tangentToWorld));
    return result;
}

MAP_SURFACE_SAMPLE EvaluateMapSourceVertexBlendSurface(float2 uv, float4 vertexColor,
    float3 worldPosition, float3 tangent, float3 binormal, float3 normal)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    float4 diffuse[4];
    float3 normals[4];
    diffuse[0] = g_DiffuseTexture.Sample(SurfaceAnisotropicSampler, uv * g_SourceBlendLayers[0].xy);
    diffuse[1] = g_SurfaceOverlayDiffuseTexture.Sample(SurfaceAnisotropicSampler, uv * g_SourceBlendLayers[1].xy);
    normals[0] = SourceSpecialNormal(g_NormalTexture.Sample(SurfaceAnisotropicSampler,
        uv * g_SourceBlendLayers[0].xy), g_SourceBlendLayers[0].z);
    normals[1] = SourceSpecialNormal(g_SurfaceOverlayNormalTexture.Sample(SurfaceAnisotropicSampler,
        uv * g_SourceBlendLayers[1].xy), g_SourceBlendLayers[1].z);
    diffuse[2] = diffuse[3] = 0.f;
    normals[2] = normals[3] = float3(0.f, 0.f, 1.f);
    if ((g_SourceSpecialFlags & 1u) != 0u)
    {
        diffuse[2] = g_SourceBlendDiffuseGTexture.Sample(SurfaceAnisotropicSampler, uv * g_SourceBlendLayers[2].xy);
        normals[2] = SourceSpecialNormal(g_SourceBlendNormalGTexture.Sample(SurfaceAnisotropicSampler,
            uv * g_SourceBlendLayers[2].xy), g_SourceBlendLayers[2].z);
    }
    if ((g_SourceSpecialFlags & 2u) != 0u)
    {
        diffuse[3] = g_SourceBlendDiffuseBTexture.Sample(SurfaceAnisotropicSampler, uv * g_SourceBlendLayers[3].xy);
        normals[3] = SourceSpecialNormal(g_SourceBlendNormalBTexture.Sample(SurfaceAnisotropicSampler,
            uv * g_SourceBlendLayers[3].xy), g_SourceBlendLayers[3].z);
    }
    const float4 baseNormal = g_DetailNormalTexture.Sample(SurfaceAnisotropicSampler, uv);
    float3 color = diffuse[0].rgb;
    float3 n = normals[0];
    float3 tint = g_SourceBlendDiffuse[0].rgb * g_SourceBlendDiffuse[0].w;
    float4 specular = g_SourceBlendSpecular[0];
    float power = g_SourceBlendLayers[0].w;
    [unroll] for (uint layer = 1u; layer < 4u; ++layer)
    {
        if (layer > 1u && (g_SourceSpecialFlags & (1u << (layer - 2u))) == 0u) continue;
        const float value = saturate(vertexColor[layer - 1u] * (diffuse[layer].a + 1.f));
        const float sharp = SourceSpecialSharp(value, g_SourceBlendSharpness);
        const float blend = lerp(sharp, value, diffuse[layer].a * baseNormal.a);
        color = lerp(color, diffuse[layer].rgb, blend);
        n = lerp(n, normals[layer], blend);
        tint = lerp(tint, g_SourceBlendDiffuse[layer].rgb * g_SourceBlendDiffuse[layer].w, blend);
        specular = lerp(specular, g_SourceBlendSpecular[layer], blend);
        power = lerp(power, g_SourceBlendLayers[layer].w, blend);
    }
    n.xy += (baseNormal.rg * 2.f - 1.f) * g_SurfaceDetailNormalIntensity;
    n = normalize(n);
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    const float3 view = normalize(mul(tangentToWorld, g_vCamPosition.xyz - worldPosition));
    result.diffuse = float4(color * tint, 1.f);
    result.specular = color * specular.rgb * specular.w;
    result.specularPower = power;
    result.tangentNormal = n;
    result.worldNormal = normalize(mul(n, tangentToWorld));
    const float rim = (1.f - abs(dot(n, view))) * (1.f - abs(view.z));
    result.rimlightRadiance = g_SourceBgRimlight.rgb * MapSurfaceSafePow(rim, g_SourceBgRimlight.w);
    return result;
}

#endif
