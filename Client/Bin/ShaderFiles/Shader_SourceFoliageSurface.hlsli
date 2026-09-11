#ifndef SHADER_SOURCE_FOLIAGE_SURFACE
#define SHADER_SOURCE_FOLIAGE_SURFACE

// Native bg_foliage_msk / bg_grass_msk surface equations. Texture decoding,
// instance lightmap scales and scene light authority belong to their callers.
float3 SourceFoliageUnit(float3 value)
{
    return value * rsqrt(max(dot(value, value), 1e-12f));
}

float3 SourceFoliageDesaturate(float3 diffuse, float saturation, float mask)
{
    return diffuse + (dot(diffuse, float3(0.3f, 0.59f, 0.11f)) - diffuse) *
        ((1.f - saturation) * mask);
}

float3 SourceFoliageNormal(float2 encoded, float intensity)
{
    const float2 xy = encoded * 2.f - 1.f;
    return SourceFoliageUnit(float3(xy * intensity,
        sqrt(max(1.f - dot(xy, xy), 0.f)) + 0.00001f));
}

float3 SourceFoliageDirectWeight(float ndotl, float3 transmission, bool grass)
{
    return grass ? 1.f : saturate(ndotl) * (1.f - transmission) +
        transmission * transmission;
}

float3 SourceFoliageHemisphere(float3 normal, float3 direction, float3 upper,
    float3 lower, float intensity, float3 transmission)
{
    const float nd = dot(SourceFoliageUnit(normal), SourceFoliageUnit(direction));
    const float2 weights = float2(0.5f + 0.5f * nd, 0.5f - 0.5f * nd);
    const float3 front = weights.x * weights.x * (1.f - transmission) + transmission * transmission;
    const float3 back = weights.y * weights.y * (1.f - transmission) + transmission * transmission;
    return (front * upper + back * lower) * intensity;
}

float3 SourceFoliageRNMBasis(float3 direction)
{
    return saturate(float3(
        dot(direction.yz, float2(0.81649658f, 0.57735027f)),
        dot(direction, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
        dot(direction, float3(0.70710678f, -0.40824829f, 0.57735027f))));
}

float3 SourceFoliageBaked(float3 diffuse, float3 normal, float3 specular,
    float3 transmission, float3 view, float3 average, float3 coefficients,
    float power, bool grass)
{
    const float3 basis = SourceFoliageRNMBasis(normal);
    const float diffuseWeight = grass ? dot(coefficients, (1.f / 3.f).xxx) :
        dot(coefficients, basis * basis * (1.f - transmission) + transmission);
    const float3 reflected = reflect(-SourceFoliageUnit(view), normal);
    const float specularWeight = dot(coefficients,
        pow(SourceFoliageRNMBasis(reflected), power + 1.f));
    return average * (diffuse * diffuseWeight + specular * specularWeight);
}

float3 SourceFoliageDirectSpecular(float3 normal, float3 view, float3 light,
    float3 specular, float power, float3 shadow, bool grass)
{
    const float3 halfVector = SourceFoliageUnit(SourceFoliageUnit(view) + SourceFoliageUnit(light));
    const float ndoth = abs(dot(normal, halfVector));
    const float lobe = ndoth < 0.000001f ? 0.f : min(pow(ndoth, power), 1.f);
    // Grass caps its specular before shadow; foliage caps after shadow.
    return grass ? clamp(specular * lobe, 0.f, 2.f) * shadow :
        clamp(specular * lobe * shadow, 0.f, 2.f);
}

float SourceFoliageFlicker(float phase, float minimum)
{
    return (1.f + sin((phase + cos(phase * 3.524534f)) * 1.328987f)) * 0.5f + minimum;
}

#endif
