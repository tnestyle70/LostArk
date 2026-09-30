#ifndef LOSTARK_SOURCE_LANDSCAPE_SURFACE
#define LOSTARK_SOURCE_LANDSCAPE_SURFACE

// Bern Landscape native Material 2cf1c13888745046a6a2742eff3f6b86.
// Six painted layers, two source weightmaps and the packed height-normal map.
// UV0 is the component grid / componentSizeQuads on every triangle.
float4 g_SourceLandscapeGrid = float4(0.f, 0.f, 62.f, 31.f);
float4 g_SourceLandscapeWeightmapScaleBias = float4(1.f / 64.f, 1.f / 64.f, .5f / 64.f, .5f / 64.f);
float4 g_SourceLandscapeHeightmapScaleBias = float4(1.f / 64.f, 1.f / 64.f, 0.f, 0.f);
uint g_SourceLandscapeLayerMask = 0u;
uint g_SourceLandscapeNormalMask = 0u;
uint g_SourceLandscapeWeightmapCount = 0u;
float4 g_SourceLandscapeUV[6]; // tiling, source rotation scalar, reserved
float4 g_SourceLandscapeDiffuse[6]; // tint RGB, brightness
float4 g_SourceLandscapeSpecular[6]; // tint RGB, intensity
float4 g_SourceLandscapeFactors[6]; // desaturation, normal intensity, power, reserved
float4 g_SourceLandscapeWeight[6]; // texture index, channel, diffuse height mode, normal height mode

Texture2D g_SourceLandscapeLayerDiffuse0;
Texture2D g_SourceLandscapeLayerDiffuse1;
Texture2D g_SourceLandscapeLayerDiffuse2;
Texture2D g_SourceLandscapeLayerDiffuse3;
Texture2D g_SourceLandscapeLayerDiffuse4;
Texture2D g_SourceLandscapeLayerDiffuse5;
Texture2D g_SourceLandscapeLayerNormal0;
Texture2D g_SourceLandscapeLayerNormal1;
Texture2D g_SourceLandscapeLayerNormal2;
Texture2D g_SourceLandscapeLayerNormal3;
Texture2D g_SourceLandscapeLayerNormal4;
Texture2D g_SourceLandscapeLayerNormal5;
Texture2D g_SourceLandscapeWeightmap0;
Texture2D g_SourceLandscapeWeightmap1;
Texture2D g_SourceLandscapeHeightmap;

float4 SampleSourceLandscapeDiffuse(uint layer, float2 uv)
{
    float4 result = 0.f;
    if (layer == 0u) result = g_SourceLandscapeLayerDiffuse0.Sample(SurfaceAnisotropicSampler, uv);
    else if (layer == 1u) result = g_SourceLandscapeLayerDiffuse1.Sample(SurfaceAnisotropicSampler, uv);
    else if (layer == 2u) result = g_SourceLandscapeLayerDiffuse2.Sample(SurfaceAnisotropicSampler, uv);
    else if (layer == 3u) result = g_SourceLandscapeLayerDiffuse3.Sample(SurfaceAnisotropicSampler, uv);
    else if (layer == 4u) result = g_SourceLandscapeLayerDiffuse4.Sample(SurfaceAnisotropicSampler, uv);
    else if (layer == 5u) result = g_SourceLandscapeLayerDiffuse5.Sample(SurfaceAnisotropicSampler, uv);
    return result;
}

float2 SampleSourceLandscapeNormal(uint layer, float2 uv)
{
    float2 result = .5f;
    if (layer == 0u) result = g_SourceLandscapeLayerNormal0.Sample(SurfaceAnisotropicSampler, uv).rg;
    else if (layer == 1u) result = g_SourceLandscapeLayerNormal1.Sample(SurfaceAnisotropicSampler, uv).rg;
    else if (layer == 2u) result = g_SourceLandscapeLayerNormal2.Sample(SurfaceAnisotropicSampler, uv).rg;
    else if (layer == 3u) result = g_SourceLandscapeLayerNormal3.Sample(SurfaceAnisotropicSampler, uv).rg;
    else if (layer == 4u) result = g_SourceLandscapeLayerNormal4.Sample(SurfaceAnisotropicSampler, uv).rg;
    else if (layer == 5u) result = g_SourceLandscapeLayerNormal5.Sample(SurfaceAnisotropicSampler, uv).rg;
    return result;
}

float2 SourceLandscapeLayerUV(float2 grid, float2 tilingRotation)
{
    const float2 centered = grid * .1f - .5f;
    // This is the original serialized uniform constant, not pi or degrees.
    float sine, cosine;
    sincos(tilingRotation.y * 3.140000104904175f, sine, cosine);
    return (float2(dot(centered, float2(cosine, -sine)),
        dot(centered, float2(sine, cosine))) + .5f) * tilingRotation.x;
}

float3 SourceLandscapeDesaturate(float3 linearColor, float amount)
{
    return lerp(linearColor, dot(linearColor, float3(.3f, .59f, .11f)).xxx, amount);
}

float3 SourceLandscapeLayerNormal(float2 sampledRG, float intensity)
{
    const float2 xy = sampledRG * 2.f - 1.f;
    return float3(xy * intensity, sqrt(max(0.f, 1.f - dot(xy, xy))) + .00001f);
}

MAP_SURFACE_SAMPLE EvaluateMapSourceLandscapeSurface(float2 rawUV,
    float3 worldAxisX, float3 worldAxisY, float3 worldAxisZ)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    const float2 localGrid = rawUV * g_SourceLandscapeGrid.z;
    const float2 grid = localGrid + g_SourceLandscapeGrid.xy;
    // Two 31-quad subsections duplicate their shared row/column in the 64 map.
    const float2 textureGrid = localGrid + float2(
        localGrid.x > g_SourceLandscapeGrid.w ? 1.f : 0.f,
        localGrid.y > g_SourceLandscapeGrid.w ? 1.f : 0.f);
    const float2 weightUV = textureGrid * g_SourceLandscapeWeightmapScaleBias.xy +
        g_SourceLandscapeWeightmapScaleBias.zw;
    // The original VF interpolates within each subsection. Its duplicated edge
    // texel is a coordinate offset, not part of the texture LOD footprint.
    const float2 gridDx = ddx(localGrid);
    const float2 gridDy = ddy(localGrid);
    const float2 weightDx = gridDx * g_SourceLandscapeWeightmapScaleBias.xy;
    const float2 weightDy = gridDy * g_SourceLandscapeWeightmapScaleBias.xy;
    const float4 weight0 = g_SourceLandscapeWeightmap0.SampleGrad(
        SurfaceLightmapSampler, weightUV, weightDx, weightDy);
    const float4 weight1 = g_SourceLandscapeWeightmapCount > 1u ?
        g_SourceLandscapeWeightmap1.SampleGrad(SurfaceLightmapSampler, weightUV, weightDx, weightDy) : 0.f;
    float diffuseWeightSum = 0.f;
    float normalWeightSum = 0.f;
    float3 diffuseSum = 0.f;
    float3 specularSum = 0.f;
    float3 normalSum = 0.f;
    float powerSum = 0.f;
    float powerWeightSum = 0.f;
    [unroll] for (uint layer = 0u; layer < 6u; ++layer)
    {
        if ((g_SourceLandscapeLayerMask & (1u << layer)) == 0u) continue;
        const float4 binding = g_SourceLandscapeWeight[layer];
        const float4 weights = binding.x > .5f ? weight1 : weight0;
        const float paint = weights[(uint)binding.y];
        const float2 uv = SourceLandscapeLayerUV(grid, g_SourceLandscapeUV[layer].xy);
        const float4 sampled = SampleSourceLandscapeDiffuse(layer, uv);
        const float heightWeight = saturate(2.f * paint - 1.f + sampled.a);
        const float diffuseWeight = binding.z > .5f ? heightWeight : paint;
        const float normalWeight = binding.w > .5f ? heightWeight : paint;
        const float3 color = SourceLandscapeDesaturate(sampled.rgb, g_SourceLandscapeFactors[layer].x);
        diffuseSum += diffuseWeight * color * g_SourceLandscapeDiffuse[layer].rgb *
            g_SourceLandscapeDiffuse[layer].a;
        // The original specular uses desaturated diffuse before diffuse tint/brightness.
        specularSum += diffuseWeight * color * g_SourceLandscapeSpecular[layer].rgb *
            g_SourceLandscapeSpecular[layer].a;
        const float3 layerNormal = (g_SourceLandscapeNormalMask & (1u << layer)) != 0u ?
            SourceLandscapeLayerNormal(SampleSourceLandscapeNormal(layer, uv),
                g_SourceLandscapeFactors[layer].y) : float3(0.f, 0.f, 1.f);
        // UV rotates; the original shader does not rotate the decoded normal XY.
        normalSum += normalWeight * layerNormal;
        diffuseWeightSum += diffuseWeight;
        normalWeightSum += normalWeight;
        const float power = g_SourceLandscapeFactors[layer].z;
        const float powerWeight = binding.z > .5f ? saturate(2.f * paint - 1.f + power) : paint;
        powerSum += powerWeight * power;
        powerWeightSum += powerWeight;
    }
    result.diffuse = float4(diffuseSum / max(diffuseWeightSum, .0001f), 1.f);
    result.specular = specularSum / max(diffuseWeightSum, .0001f);
    result.specularPower = powerSum / max(powerWeightSum, .0001f);
    result.tangentNormal = normalize(normalSum / max(normalWeightSum, .0001f) + float3(0.f, 0.f, .001f));

    const float2 heightUV = (textureGrid + .5f) * g_SourceLandscapeHeightmapScaleBias.xy +
        g_SourceLandscapeHeightmapScaleBias.zw;
    const float2 sourceXY = g_SourceLandscapeHeightmap.SampleGrad(SurfaceLightmapSampler,
        heightUV, gridDx * g_SourceLandscapeHeightmapScaleBias.xy,
        gridDy * g_SourceLandscapeHeightmapScaleBias.xy).ba * 2.f - 1.f;
    const float sourceZ = sqrt(max(0.f, 1.f - dot(sourceXY, sourceXY)));
    const float3 localNormal = float3(sourceXY.x, sourceZ, -sourceXY.y);
    const float3 localTangent = MapGeometryNormalizeOrZero(float3(sourceZ, -sourceXY.x, 0.f));
    const float3 localBinormal = cross(localNormal, localTangent);
    const float3 localPerturbed = result.tangentNormal.x * localTangent +
        result.tangentNormal.y * localBinormal + result.tangentNormal.z * localNormal;
    // Both ordinary and instanced VS supply their inverse-transpose XYZ axes.
    const float3x3 normalToWorld = float3x3(worldAxisX, worldAxisY, worldAxisZ);
    result.geometricNormal = MapGeometryNormalizeOrZero(mul(localNormal, normalToWorld));
    result.worldNormal = MapGeometryNormalizeOrZero(mul(localPerturbed, normalToWorld));
    result.ambientOcclusion = 1.f;
    return result;
}

#endif
