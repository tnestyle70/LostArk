// Source BLEND_Translucent surfaces drawn after scene lighting. The same native base
// and light programs as the deferred marker-5 path run here per forward light.
#ifndef SOURCE_CHARACTER_FORWARD_CONSTANTS_PRESENT
float4 g_SourceCharacterLightConstants[64];
#endif
#include "Shader_SourceCharacterLightPrograms.hlsli"
#include "Shader_SceneHeightFog.hlsli"

SOURCE_CHARACTER_NATIVE_INPUT MakeSourceCharacterForwardLightInput(VS_OUT input,
    float3 cameraPosition, float3 lightDirection, float3 lightColor, bool frontFace)
{
    SOURCE_CHARACTER_NATIVE_INPUT light = (SOURCE_CHARACTER_NATIVE_INPUT)0;
    float3 t = SourceCharacterSafeUnit(input.vTangent.xyz);
    const float3 n = SourceCharacterSafeUnit(input.vNormal.xyz);
    t = SourceCharacterSafeUnit(t - n * dot(t, n));
    const float3 b = SourceCharacterSafeUnit(-input.vBinormal.xyz);
    const float3 view = SourceCharacterSafeUnit(cameraPosition - input.vWorldPos.xyz);
    const float3 direction = SourceCharacterSafeUnit(lightDirection);
    const float4x4 viewProjection = mul(g_ViewMatrix, g_ProjMatrix);
    light.values[0] = float4(t.x, b.x, n.x, 0.f);
    light.values[1] = float4(t.y, b.y, n.y,
        dot(cross(t.xzy, b.xzy), n.xzy) < 0.f ? -1.f : 1.f);
    light.values[2] = 1.f;
    light.values[4] = float4(input.vTexcoord, input.vSourceExtraUV.yx);
    light.values[5] = float4(dot(t, direction), dot(b, direction), dot(n, direction), 1.f);
    light.values[7] = float4(dot(t, view), dot(b, view), dot(n, view), 1.f);
    light.values[8] = float4(input.vWorldPos.xzy * 100.f, 1.f);
    if (84u == g_SourceCharacterProgram)
    {
        // Ghost's native direct VS uses UV/light/view/position in 2/3/5/6.
        light.values[2] = float4(input.vTexcoord, 0.f, 0.f);
        light.values[3] = light.values[5];
        light.values[5] = light.values[7];
        light.values[6] = light.values[8];
    }
    if (6u == g_SourceCharacterProgram || 212u == g_SourceCharacterProgram)
    {
        light.values[2] = float4(input.vTexcoord, 0.f, 0.f);
        light.values[3] = light.values[5];
        light.values[5] = light.values[7];
        light.values[7] = frontFace ? 1.f : 0.f;
    }
    if (g_SourceCharacterProgram == 180u || g_SourceCharacterProgram == 181u || g_SourceCharacterProgram == 183u || g_SourceCharacterProgram == 184u || g_SourceCharacterProgram == 185u || g_SourceCharacterProgram == 186u || g_SourceCharacterProgram == 188u || g_SourceCharacterProgram == 189u || g_SourceCharacterProgram == 190u || g_SourceCharacterProgram == 195u || g_SourceCharacterProgram == 198u || g_SourceCharacterProgram == 199u) light.values[8] = input.vProjPos;
    if (187u == g_SourceCharacterProgram)
    {
        light.values[5] = float4(input.vSourceExtraUV.zw, 0.f, 0.f);
        light.values[6] = float4(dot(t, direction), dot(b, direction), dot(n, direction), 1.f);
        light.values[8] = float4(dot(t, view), dot(b, view), dot(n, view), 1.f);
        light.values[9] = float4(input.vWorldPos.xzy * 100.f, 1.f);
    }
    light.projection[0] = viewProjection[0] * 0.01f;
    light.projection[1] = viewProjection[2] * 0.01f;
    light.projection[2] = viewProjection[1] * 0.01f;
    light.projection[3] = viewProjection[3];
    light.lightColor = lightColor;
    light.shadow = 1.f;
    return light;
}

SCENE_COLOR_BLOOM_OUT PS_MAIN_SOURCE_CHARACTER_TRANSLUCENT(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
    if (6u != g_SourceCharacterProgram && 7u != g_SourceCharacterProgram &&
        18u != g_SourceCharacterProgram && 84u != g_SourceCharacterProgram &&
        88u != g_SourceCharacterProgram && 99u != g_SourceCharacterProgram &&
        !(g_SourceCharacterProgram == 160u || g_SourceCharacterProgram == 166u || g_SourceCharacterProgram == 168u || g_SourceCharacterProgram == 169u || g_SourceCharacterProgram == 170u || g_SourceCharacterProgram == 171u || g_SourceCharacterProgram == 172u || g_SourceCharacterProgram == 174u || g_SourceCharacterProgram == 182u || g_SourceCharacterProgram == 187u || g_SourceCharacterProgram == 190u || g_SourceCharacterProgram == 192u || g_SourceCharacterProgram == 195u || g_SourceCharacterProgram == 212u || g_SourceCharacterProgram == 213u)) discard;
    const float3 camera = -mul((float3x3)g_ViewMatrix, g_ViewMatrix[3].xyz);
    float3 ambient = 0.f;
    [loop] for (uint ambientIndex = 0u; ambientIndex < g_SourceMapForwardLightCount; ++ambientIndex)
    {
        float3 unusedDirection;
        const float attenuation = SourceCharacterForwardLightAttenuation(ambientIndex,
            input.vWorldPos.xyz, input.vNormal.xyz, unusedDirection);
        if (attenuation > 0.f)
            ambient += g_SourceMapForwardLightColorExponent[ambientIndex].rgb *
                g_SourceMapForwardLightAmbient[ambientIndex].rgb * attenuation;
    }
    // Base programs never read lightColor; program 88 takes the scene ambient
    // through it as its unbound engine sky-light rows.
    const SOURCE_CHARACTER_NATIVE_INPUT baseInput = MakeSourceCharacterInput(input.vTexcoord,
        input.vSourceExtraUV, input.vWorldPos.xyz, input.vTangent.xyz, input.vBinormal.xyz,
        input.vNormal.xyz, camera, input.vProjPos, mul(g_ViewMatrix, g_ProjMatrix),
        float3(0.f, 1.f, 0.f), ambient, 1.f, frontFace);
    const SOURCE_CHARACTER_NATIVE_OUTPUT base = EvaluateSourceCharacterBase(baseInput);
    const float opacity = saturate(base.targets[0].a);
    if (base.discarded || opacity <= 1e-4f) discard;

    float3 direct = 0.f;
    [loop] for (uint index = 0u; index < g_SourceMapForwardLightCount; ++index)
    {
        const float4 colorExponent = g_SourceMapForwardLightColorExponent[index];
        float3 direction;
        const float attenuation = SourceCharacterForwardLightAttenuation(index,
            input.vWorldPos.xyz, input.vNormal.xyz, direction);
        if (attenuation <= 0.f) continue;
        const SOURCE_CHARACTER_NATIVE_INPUT lightInput =
            MakeSourceCharacterForwardLightInput(input, camera, direction, colorExponent.rgb, frontFace);
        const SOURCE_CHARACTER_NATIVE_OUTPUT lit = EvaluateSourceCharacterLight(lightInput);
        if (!lit.discarded) direct += lit.targets[0].rgb * attenuation;
    }

    float3 color = base.targets[3].rgb * ambient + direct;
    const float4 fog = EvaluateSceneFog(input.vWorldPos.xyz, camera);
    color = color * fog.w + fog.rgb + base.targets[0].rgb;
    return Write_SceneColorAndBloom(float4(color, opacity));
}
