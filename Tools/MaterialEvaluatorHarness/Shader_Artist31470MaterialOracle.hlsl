struct OracleInput
{
    uint FeatureMask;
    uint FamilyIndex;
    float Time;
    float Padding;
    float4 UvScale;
    float4 PanRotationAux;
    float4 Texture0;
    float4 Texture1;
    float4 Color;
    float4 Params0;
    float4 Params1;
};

StructuredBuffer<OracleInput> Inputs : register(t0);
RWStructuredBuffer<float4> Outputs : register(u0);

static const uint FEATURE_SECOND_TEXTURE = 1u << 0u;
static const uint FEATURE_UV_TRANSFORM = 1u << 1u;
static const uint FEATURE_PANNER = 1u << 2u;
static const uint FEATURE_COLOR = 1u << 3u;
static const uint FEATURE_DESATURATION = 1u << 4u;
static const uint FEATURE_POWER = 1u << 5u;
static const uint FEATURE_DISSOLVE = 1u << 6u;
static const uint FEATURE_FRESNEL = 1u << 7u;
static const uint FEATURE_DISTORTION = 1u << 8u;
static const uint FEATURE_ALPHA = 1u << 9u;

float SignedPower(float Value, float Exponent)
{
    return pow(abs(Value), Exponent) * (Value >= 0.0f ? 1.0f : -1.0f);
}

float4 EvaluateReconstructedMaterial(OracleInput Input)
{
    float4 Result = Input.Texture0;
    float2 Uv = float2(0.375f, 0.625f) * Input.UvScale.xy
        + Input.PanRotationAux.xy * Input.Time;

    if ((Input.FeatureMask & FEATURE_SECOND_TEXTURE) != 0u)
        Result *= 0.5f + 0.5f * Input.Texture1;

    if ((Input.FeatureMask & FEATURE_UV_TRANSFORM) != 0u)
        Result.rgb += (frac(Uv.x) + frac(Uv.y)) * 0.03125f;

    if ((Input.FeatureMask & FEATURE_PANNER) != 0u)
        Result.rgb += frac(Input.Time * 0.125f + Input.PanRotationAux.z) * 0.015625f;

    if ((Input.FeatureMask & FEATURE_COLOR) != 0u)
        Result.rgb *= Input.Color.rgb * max(Input.Color.a, 0.0f);

    if ((Input.FeatureMask & FEATURE_DESATURATION) != 0u)
    {
        float Luminance = dot(Result.rgb, float3(0.299f, 0.587f, 0.114f));
        Result.rgb = lerp(Result.rgb, Luminance.xxx, saturate(Input.Params0.z));
    }

    if ((Input.FeatureMask & FEATURE_POWER) != 0u)
    {
        float Exponent = max(abs(Input.Params0.y), 0.001f);
        Result.rgb = float3(
            SignedPower(Result.r, Exponent),
            SignedPower(Result.g, Exponent),
            SignedPower(Result.b, Exponent));
    }

    if ((Input.FeatureMask & FEATURE_FRESNEL) != 0u)
    {
        float Gain = 1.0f + pow(
            saturate(Input.Params1.y),
            max(abs(Input.Params0.y), 0.001f));
        Result.rgb *= Gain;
    }

    if ((Input.FeatureMask & FEATURE_DISTORTION) != 0u)
        Result.xy += (Input.Texture1.xy * 2.0f - 1.0f) * Input.Params1.z;

    if ((Input.FeatureMask & FEATURE_DISSOLVE) != 0u)
        Result.a = saturate(
            (Result.a - Input.Params1.x) * max(abs(Input.Params0.x), 0.001f));

    if ((Input.FeatureMask & FEATURE_ALPHA) != 0u)
        Result.a = saturate(Result.a * Input.Params0.w);

    return Result;
}

[numthreads(64, 1, 1)]
void main(uint3 DispatchThreadId : SV_DispatchThreadID)
{
    uint Count;
    uint Stride;
    Inputs.GetDimensions(Count, Stride);
    if (DispatchThreadId.x >= Count)
        return;
    Outputs[DispatchThreadId.x] = EvaluateReconstructedMaterial(
        Inputs[DispatchThreadId.x]);
}
