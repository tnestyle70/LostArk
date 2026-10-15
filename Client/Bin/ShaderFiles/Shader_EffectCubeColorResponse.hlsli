#ifndef EFFECT_CUBE_COLOR_RESPONSE_HLSLI
#define EFFECT_CUBE_COLOR_RESPONSE_HLSLI

// Renderer 소유의 현재 변환을 읽기만 한다. Client가 exact Q/R/A draw마다 선택을 갱신한다.
int g_CubeColorResponseEnabled = 0;
int g_iSourcePostProcessEnabled = 0;
float4 g_vSourceToneCurve = float4(.22f, 1.22f, .298f, 1.f);
float g_fSourceToneToe = 1.f;
float g_fToneMapExposure = 1.f;
Texture2D g_SourceGradingLut;

bool CubeColor_IsFinite(float3 value)
{
    // FXC의 fast-math가 isfinite를 제거해도 HDR 경계의 NaN/INF 방어는 남긴다.
    return all((asuint(value) & 0x7f800000u) != 0x7f800000u);
}

float3 CubeColor_Tone(float3 hdr)
{
    // Final의 Sanitize_HDR와 같은 노출 전후 범위다.
    const float3 color = clamp(clamp(hdr, 0.f, 60000.f) * g_fToneMapExposure, 0.f, 60000.f);
    const float3 rational = color / abs(color + g_vSourceToneCurve.x) * g_vSourceToneCurve.y;
    const float3 linearGamma = pow(max(color * g_vSourceToneCurve.w, 0.f), 1.f / 2.2f);
    const float3 shoulder = saturate((color - g_vSourceToneCurve.z) * 10000.f);
    return saturate(lerp(lerp(linearGamma, rational, shoulder), rational, saturate(g_fSourceToneToe)));
}

float3 CubeColor_Grade(float3 color)
{
    color = saturate(color);
    const float slice = floor(color.b * 14.9999f);
    const float2 uv = float2(slice / 16.f + color.r * (15.f / 256.f) + .5f / 256.f,
        color.g * (15.f / 16.f) + .5f / 16.f);
    const float3 low = g_SourceGradingLut.SampleLevel(LinearClampUVSampler, uv, 0.f).rgb;
    const float3 high = g_SourceGradingLut.SampleLevel(LinearClampUVSampler, uv + float2(1.f / 16.f, 0.f), 0.f).rgb;
    return lerp(low, high, color.b * 15.f - slice);
}

float3 CubeColor_Hable(float3 color)
{
    return ((color * (.15f * color + .05f) + .004f) /
        (color * (.15f * color + .50f) + .06f)) - .02f / .30f;
}

// 09-08 Q2 이후 첫 보존본의 Hable 설정이다. 현재 장면 옵션을 과거 값으로 되돌리지 않는다.
float3 CubeColor_Historical(float3 hdr)
{
    return pow(saturate(CubeColor_Hable(clamp(hdr, 0.f, 60000.f) * .6351f) /
        CubeColor_Hable(1.f.xxx)), 1.f / 1.905f);
}

float CubeColor_Metric(float3 a, float3 b)
{
    const float3 luma = float3(.299f, .587f, .114f);
    return dot(a, b) + 16.f * dot(a, luma) * dot(b, luma);
}

float CubeColor_Error(float3 a, float3 b)
{
    return CubeColor_Metric(a - b, a - b);
}

// shoulder 위는 유리함수의 역식으로 즉시 복원한다. toe 혼합 구간만 작은 HDR 구간에서 찾는다.
float3 CubeColor_InverseTone(float3 tone)
{
    const float exposure = max(g_fToneMapExposure, 1.e-6f);
    const float boundary = max(g_vSourceToneCurve.z + .0001f, 0.f) / exposure;
    const float3 threshold = CubeColor_Tone(boundary.xxx);
    const float3 rational = tone * g_vSourceToneCurve.x /
        max(g_vSourceToneCurve.y - tone, 1.e-6f) / exposure;
    if (all(tone >= threshold))
        return min(rational, 60000.f);
    float3 low = 0.f;
    float3 high = min(boundary, 60000.f).xxx;
    [loop] for (uint iteration = 0u; iteration < 20u; ++iteration)
    {
        const float3 middle = .5f * (low + high);
        const float3 mapped = CubeColor_Tone(middle);
        low = float3(mapped.x < tone.x ? middle.x : low.x,
            mapped.y < tone.y ? middle.y : low.y, mapped.z < tone.z ? middle.z : low.z);
        high = float3(mapped.x < tone.x ? high.x : middle.x,
            mapped.y < tone.y ? high.y : middle.y, mapped.z < tone.z ? high.z : middle.z);
    }
    const float3 bisected = .5f * (low + high);
    return float3(tone.x >= threshold.x ? rational.x : bisected.x,
        tone.y >= threshold.y ? rational.y : bisected.y,
        tone.z >= threshold.z ? rational.z : bisected.z);
}

// 현재 저작한 면 발광/투과/alpha의 표시색 기여분을 과거 tone 기준으로 맞춘다.
// clarity 이전 색을 목표로 쓰면 저작한 자체 발광이 다시 제거되므로 실제 합성 입력을 사용한다.
// 굴절 배경, alpha, RT2 소유권은 호출자에 남긴다.
// LUT가 비가역이므로 제한된 최근접 해이며, 같은 occurrence의 고정 snapshot 겹침까지 정확한 역합성은 아니다.
float3 CubeColor_Resolve(float3 currentMaterial, float3 background, float alpha)
{
    [branch] if (g_CubeColorResponseEnabled == 0 || g_iSourcePostProcessEnabled == 0 ||
        alpha <= 1.e-5f || g_fToneMapExposure <= 0.f ||
        !CubeColor_IsFinite(currentMaterial) || !CubeColor_IsFinite(background))
        return currentMaterial;

    background = clamp(background, 0.f, 60000.f);
    const float3 target = saturate(CubeColor_Grade(CubeColor_Tone(background)) +
        CubeColor_Historical(lerp(background, currentMaterial, saturate(alpha))) -
        CubeColor_Historical(background));
    const float3 fixedBackground = background * (1.f - saturate(alpha));
    const float3 lower = CubeColor_Tone(fixedBackground);
    const float3 upper = CubeColor_Tone(fixedBackground + alpha * 60000.f);
    float3 tone = clamp(CubeColor_Tone(lerp(background, currentMaterial, alpha)), lower, upper);
    float3 actual = CubeColor_Grade(tone);
    const float baselineError = CubeColor_Error(actual, target);
    float bestError = baselineError;
    const float3 secondSeed = clamp(target, lower, upper);
    const float3 secondActual = CubeColor_Grade(secondSeed);
    const float secondError = CubeColor_Error(secondActual, target);
    if (secondError < bestError)
    {
        tone = secondSeed;
        actual = secondActual;
        bestError = secondError;
    }

    // 최대8회 damped Gauss-Newton. LUT 미분/역행렬이 불안정한 셀에서도 범위와 기존 오차를 지킨다.
    [loop] for (uint iteration = 0u; iteration < 8u && bestError > 1.e-7f; ++iteration)
    {
        const float3 delta = float3(tone.x > .99f ? -.01f : .01f,
            tone.y > .99f ? -.01f : .01f, tone.z > .99f ? -.01f : .01f);
        const float3 j0 = (CubeColor_Grade(tone + float3(delta.x, 0.f, 0.f)) - actual) / delta.x;
        const float3 j1 = (CubeColor_Grade(tone + float3(0.f, delta.y, 0.f)) - actual) / delta.y;
        const float3 j2 = (CubeColor_Grade(tone + float3(0.f, 0.f, delta.z)) - actual) / delta.z;
        const float3 h0 = float3(CubeColor_Metric(j0, j0) + .0001f, CubeColor_Metric(j0, j1), CubeColor_Metric(j0, j2));
        const float3 h1 = float3(h0.y, CubeColor_Metric(j1, j1) + .0001f, CubeColor_Metric(j1, j2));
        const float3 h2 = float3(h0.z, h1.z, CubeColor_Metric(j2, j2) + .0001f);
        const float3 residual = target - actual;
        const float3 rhs = float3(CubeColor_Metric(j0, residual), CubeColor_Metric(j1, residual), CubeColor_Metric(j2, residual));
        const float determinant = dot(h0, cross(h1, h2));
        const float3 step = clamp(float3(dot(rhs, cross(h1, h2)), dot(h0, cross(rhs, h2)),
            dot(h0, cross(h1, rhs))) / max(determinant, 1.e-12f), -.25f, .25f);
        float3 nextTone = tone;
        float3 nextActual = actual;
        float nextError = bestError;
        [unroll] for (uint trial = 0u; trial < 3u; ++trial)
        {
            const float gain = trial == 0u ? 1.f : (trial == 1u ? .5f : .25f);
            const float3 candidate = clamp(tone + gain * step, lower, upper);
            const float3 candidateActual = CubeColor_Grade(candidate);
            const float candidateError = CubeColor_Error(candidateActual, target);
            if (candidateError < nextError)
            {
                nextTone = candidate;
                nextActual = candidateActual;
                nextError = candidateError;
            }
        }
        if (nextError >= bestError)
            break;
        tone = nextTone;
        actual = nextActual;
        bestError = nextError;
    }

    const float3 material = clamp((CubeColor_InverseTone(tone) - fixedBackground) / alpha, 0.f, 60000.f);
    const float3 resolved = CubeColor_Grade(CubeColor_Tone(lerp(background, material, alpha)));
    return CubeColor_IsFinite(material) && CubeColor_Error(resolved, target) <= baselineError ? material : currentMaterial;
}

#endif
