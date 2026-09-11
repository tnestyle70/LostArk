#ifndef LOSTARK_SCENE_HEIGHT_FOG
#define LOSTARK_SCENE_HEIGHT_FOG
uint        g_iHeightFogEnabled = 0u;
float4      g_vHeightFogColor = float4(0.55f, 0.62f, 0.72f, 1.f);
float       g_fHeightFogDensity = 0.35f;
float       g_fHeightFogFalloff = 0.08f;
float       g_fHeightFogTopHeight = 24.f;
float       g_fHeightFogStartDistance = 0.f;
float       g_fHeightFogMaximumOpacity = 0.9f;
float       g_fHeightFogDriftSpeed = 0.f;
float       g_fHeightFogDriftHeight = 0.f;
float       g_fHeightFogDriftDensity = 0.f;
float       g_fPresentationClock = 0.f;
float       g_fFogCoverage = 1.f;
float2      g_vFogWindDirection = float2(1.f, 0.f);
float       g_fFogWindSpeed = 0.f;
float       g_fFogPatchScale = 0.01f;
float       g_fFogPatchSoftness = 0.15f;

uint g_iSourceExponentialFog = 0u;
float4 g_vFogInscatteringColor = 0.f;
float4 g_vFogLightDirection = float4(0.f,1.f,0.f,0.f);

float Fog_Hash(float2 vPoint)
{
    vPoint = frac(vPoint * float2(127.1f, 311.7f));
    vPoint += dot(vPoint, vPoint + 34.23f);
    return frac(vPoint.x * vPoint.y);
}

float Fog_ValueNoise(float2 vPoint)
{
    const float2 vCell = floor(vPoint);
    const float2 vLocal = frac(vPoint);
    const float2 vWeight = vLocal * vLocal * (3.f - 2.f * vLocal);
    const float fA = Fog_Hash(vCell);
    const float fB = Fog_Hash(vCell + float2(1.f, 0.f));
    const float fC = Fog_Hash(vCell + float2(0.f, 1.f));
    const float fD = Fog_Hash(vCell + float2(1.f, 1.f));
    return lerp(lerp(fA, fB, vWeight.x), lerp(fC, fD, vWeight.x), vWeight.y);
}

/* Three octaves read as cloud rather than as a grid and stay cheap enough for
   a full screen pass. */
float Fog_PatchNoise(float2 vPoint)
{
    float fValue = 0.f;
    float fAmplitude = 0.5f;
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        fValue += Fog_ValueNoise(vPoint) * fAmplitude;
        vPoint *= 2.03f;
        fAmplitude *= 0.5f;
    }
    return saturate(fValue / 0.875f);
}

// Original TExponentialHeightFogPixelShader PS equation. The native CPU
// parameter setup is adapted: source centimetres, density/falloff * .001,
// and the source terminator angle defines the 50% colour transition.
float4 EvaluateSourceExponentialFog(float3 worldPosition, float3 cameraPosition)
{
    float3 ray = (worldPosition - cameraPosition) * 100.f;
    float distance = length(ray);
    if (distance < .000001f) return float4(0.f, 0.f, 0.f, 1.f);
    float3 direction = ray / distance;
    float vertical = abs(ray.y) > .01f ? ray.y : .01f;
    float heightDelta = g_fHeightFogFalloff * .001f * vertical;
    float integral = (1.f - exp2(-heightDelta)) / heightDelta;
    float cameraDensity = g_fHeightFogDensity * .001f *
        exp2(clamp(-g_fHeightFogFalloff * .001f *
            (cameraPosition.y - g_fHeightFogTopHeight) * 100.f, -80.f, 80.f));
    float transmittance = max(min(exp2(-integral *
        max(distance - g_fHeightFogStartDistance * 100.f, 0.f) * cameraDensity), 1.f),
        1.f - g_fHeightFogMaximumOpacity);
    float angleMidpoint = clamp(.5f + .499f * g_vFogLightDirection.w, .001f, .999f);
    float power = log2(.5f) / log2(angleMidpoint);
    float hemisphere = pow(.5f + .499f *
        dot(normalize(g_vFogLightDirection.xyz), direction), power);
    float3 color = lerp(g_vHeightFogColor.rgb, g_vFogInscatteringColor.rgb, hemisphere);
    return float4(color * (1.f - transmittance), transmittance);
}

float4 EvaluateSceneFog(float3 worldPosition, float3 cameraPosition)
{
    if (0u == g_iHeightFogEnabled) return float4(0.f, 0.f, 0.f, 1.f);
    if (g_iSourceExponentialFog != 0u)
        return EvaluateSourceExponentialFog(worldPosition, cameraPosition);
    const float fDrift = sin(g_fPresentationClock * g_fHeightFogDriftSpeed);
    const float fCeiling =
        g_fHeightFogTopHeight + fDrift * g_fHeightFogDriftHeight;
    const float fDensity = max(0.f,
        g_fHeightFogDensity + fDrift * g_fHeightFogDriftDensity);

    const float fBelowCeiling = max(0.f, fCeiling - worldPosition.y);
    const float fHeightTerm =
        1.f - exp(-fBelowCeiling * g_fHeightFogFalloff);

    const float fDistance = max(0.f,
        length(worldPosition - cameraPosition) - g_fHeightFogStartDistance);
    const float fDistanceTerm = 1.f - exp(-fDistance * fDensity * 0.02f);

    /* Coverage thins the blanket into banks whose share of the map matches
       the authored percentage, and the wind vector walks the pattern through
       world XZ so the banks travel on the same clock the drift uses. */
    float fCoverage = 1.f;
    if (g_fFogCoverage < 0.999f)
    {
        const float fWindLength = max(length(g_vFogWindDirection), 0.0001f);
        const float2 vTravel = (g_vFogWindDirection / fWindLength) *
            (g_fPresentationClock * g_fFogWindSpeed);
        const float fNoise = Fog_PatchNoise(
            (worldPosition.xz + vTravel) * g_fFogPatchScale);
        const float fThreshold = 1.f - g_fFogCoverage;
        fCoverage = smoothstep(fThreshold - g_fFogPatchSoftness,
            fThreshold + g_fFogPatchSoftness, fNoise);
    }

    const float fFog = saturate(fHeightTerm * fDistanceTerm * fCoverage) *
        g_fHeightFogMaximumOpacity;
    return float4(g_vHeightFogColor.rgb * fFog, 1.f - fFog);
}
#endif
