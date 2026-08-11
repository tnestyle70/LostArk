uint g_ReconstructedMaterialEvaluatorEnabled = 0;
uint g_ReconstructedMaterialFeatureMask = 0;
float2 g_ReconstructedUVScale = float2(1.f, 1.f);
float4 g_ReconstructedPanRotationAux = float4(0.f, 0.f, 0.f, 0.f);
float4 g_ReconstructedColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_ReconstructedParams0 = float4(0.f, 0.f, 0.f, 0.f);
float4 g_ReconstructedParams1 = float4(0.f, 0.f, 0.f, 0.f);

static const uint RECONSTRUCTED_FEATURE_SECOND_TEXTURE = 1u << 0u;
static const uint RECONSTRUCTED_FEATURE_UV_TRANSFORM = 1u << 1u;
static const uint RECONSTRUCTED_FEATURE_PANNER = 1u << 2u;
static const uint RECONSTRUCTED_FEATURE_COLOR = 1u << 3u;
static const uint RECONSTRUCTED_FEATURE_DESATURATION = 1u << 4u;
static const uint RECONSTRUCTED_FEATURE_POWER = 1u << 5u;
static const uint RECONSTRUCTED_FEATURE_DISSOLVE = 1u << 6u;
static const uint RECONSTRUCTED_FEATURE_FRESNEL = 1u << 7u;
static const uint RECONSTRUCTED_FEATURE_DISTORTION = 1u << 8u;
static const uint RECONSTRUCTED_FEATURE_ALPHA = 1u << 9u;

float Reconstructed_SignedPower(float value, float exponent)
{
    return pow(abs(value), exponent) * (value >= 0.f ? 1.f : -1.f);
}

float4 Evaluate_ReconstructedMaterial(
    float4 texture0,
    float4 texture1)
{
    float4 result = texture0;
    float2 uv = float2(0.375f, 0.625f) * g_ReconstructedUVScale.xy
        + g_ReconstructedPanRotationAux.xy * g_EffectLocalTime;

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_SECOND_TEXTURE) != 0u)
    {
        result *= 0.5f + 0.5f * texture1;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_UV_TRANSFORM) != 0u)
    {
        result.rgb += (frac(uv.x) + frac(uv.y)) * 0.03125f;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_PANNER) != 0u)
    {
        result.rgb += frac(g_EffectLocalTime * 0.125f +
            g_ReconstructedPanRotationAux.z) * 0.015625f;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_COLOR) != 0u)
    {
        result.rgb *= g_ReconstructedColor.rgb *
            max(g_ReconstructedColor.a, 0.f);
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_DESATURATION) != 0u)
    {
        float luminance = dot(result.rgb, float3(0.299f, 0.587f, 0.114f));
        result.rgb = lerp(result.rgb, luminance.xxx,
            saturate(g_ReconstructedParams0.z));
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_POWER) != 0u)
    {
        float exponent = max(abs(g_ReconstructedParams0.y), 0.001f);
        result.rgb = float3(
            Reconstructed_SignedPower(result.r, exponent),
            Reconstructed_SignedPower(result.g, exponent),
            Reconstructed_SignedPower(result.b, exponent));
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_FRESNEL) != 0u)
    {
        float gain = 1.f + pow(
            saturate(g_ReconstructedParams1.y),
            max(abs(g_ReconstructedParams0.y), 0.001f));
        result.rgb *= gain;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_DISTORTION) != 0u)
    {
        result.xy += (texture1.xy * 2.f - 1.f) *
            g_ReconstructedParams1.z;
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_DISSOLVE) != 0u)
    {
        result.a = saturate((result.a - g_ReconstructedParams1.x) *
            max(abs(g_ReconstructedParams0.x), 0.001f));
    }

    if ((g_ReconstructedMaterialFeatureMask &
        RECONSTRUCTED_FEATURE_ALPHA) != 0u)
    {
        result.a = saturate(result.a * g_ReconstructedParams0.w);
    }

    return result;
}

EFFECT_PS_OUT Shade_ReconstructedMaterial(
    float2 sourceUV,
    float3 lighting,
    float4 vertexColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    output.SceneColor = Evaluate_ReconstructedMaterial(
        Sample_SourceTexture0(sourceUV),
        Sample_SourceTexture1(sourceUV));
    output.SceneColor.rgb *= lighting * vertexColor.rgb;
    output.SceneColor.a = saturate(output.SceneColor.a * vertexColor.a);
    return output;
}
