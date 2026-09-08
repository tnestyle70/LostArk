#ifndef SOURCE_CHARACTER_MATERIAL_INCLUDED
#define SOURCE_CHARACTER_MATERIAL_INCLUDED

#include "Shader_SourceCharacterPrograms.hlsli"

float3 SourceCharacterSafeUnit(float3 value)
{
    return value * rsqrt(max(dot(value, value), 1e-12f));
}

float2 SourceCharacterOctEncode(float3 value)
{
    value /= max(dot(abs(value), 1.f.xxx), 1e-12f);
    return value.z >= 0.f ? value.xy :
        (1.f - abs(value.yx)) * float2(value.x >= 0.f ? 1.f : -1.f,
            value.y >= 0.f ? 1.f : -1.f);
}

float3 SourceCharacterOctDecode(float2 oct)
{
    float3 value = float3(oct, 1.f - abs(oct.x) - abs(oct.y));
    float fold = max(-value.z, 0.f);
    value.xy += float2(value.x >= 0.f ? -fold : fold,
        value.y >= 0.f ? -fold : fold);
    return SourceCharacterSafeUnit(value);
}

SOURCE_CHARACTER_NATIVE_INPUT MakeSourceCharacterInput(float2 uv, float4 extraUV,
    float3 worldPosition, float3 tangent, float3 binormal, float3 normal,
    float3 cameraPosition, float4 clipPosition, float4x4 viewProjection,
    float3 lightDirection, float3 lightColor, float shadow, bool frontFace)
{
    SOURCE_CHARACTER_NATIVE_INPUT input = (SOURCE_CHARACTER_NATIVE_INPUT)0;
    float3 t = SourceCharacterSafeUnit(tangent);
    float3 n = SourceCharacterSafeUnit(normal);
    t = SourceCharacterSafeUnit(t - n * dot(t, n));
    // The existing character import's normal-map basis uses negative binormal.
    float3 b = SourceCharacterSafeUnit(-binormal);
    float3 view = SourceCharacterSafeUnit(cameraPosition - worldPosition);
    float3 light = SourceCharacterSafeUnit(lightDirection);
    float3 tangentView = float3(dot(t, view), dot(b, view), dot(n, view));
    float3 tangentLight = float3(dot(t, light), dot(b, light), dot(n, light));
    float3 up = float3(t.y, b.y, n.y);
    float handedness = dot(cross(t.xzy, b.xzy), n.xzy) < 0.f ? -1.f : 1.f;
    input.values[0] = float4(t.x, b.x, n.x, 0.f);
    input.values[1] = float4(t.y, b.y, n.y, handedness);
    input.values[2] = 1.f;
    input.values[4] = float4(uv, extraUV.yx);
    float4 sourcePosition = float4(worldPosition.xzy * 100.f, 1.f);
    input.projection[0] = viewProjection[0] * 0.01f;
    input.projection[1] = viewProjection[2] * 0.01f;
    input.projection[2] = viewProjection[1] * 0.01f;
    input.projection[3] = viewProjection[3];
#ifdef SOURCE_CHARACTER_LIGHT_PASS
    input.values[5] = float4(tangentLight, 1.f);
    input.values[7] = float4(tangentView, 1.f);
    input.values[8] = (g_SourceCharacterProgram == 3u || g_SourceCharacterProgram == 8u ||
        g_SourceCharacterProgram == 9u) ? clipPosition : sourcePosition;
    if (g_SourceCharacterProgram == 5u) // Eye's own UV1/UV2 varyings.
    {
        input.values[2] = float4(uv, extraUV.yx);
        input.values[3] = float4(extraUV.zw, 0.f, 0.f);
        input.values[4] = float4(tangentLight, 1.f);
        input.values[6] = float4(tangentView, 1.f);
        input.values[7] = clipPosition;
    }
    if (g_SourceCharacterProgram == 6u)
    {
        input.values[2] = float4(uv, 0.f, 0.f);
        input.values[3] = float4(tangentLight, 1.f);
        input.values[5] = float4(tangentView, 1.f);
        input.values[7] = frontFace ? 1.f : 0.f;
    }
#else
    input.values[5] = float4(tangentView, 1.f);
    input.values[6] = float4(up, 0.f);
    input.values[7] = sourcePosition;
    if (g_SourceCharacterProgram == 5u)
    {
        input.values[5] = float4(extraUV.zw, 0.f, 0.f);
        input.values[6] = float4(tangentView, 1.f);
        input.values[7] = float4(up, 0.f);
    }
    if (g_SourceCharacterProgram == 6u || g_SourceCharacterProgram == 7u)
    {
        input.values[5] = float4(0.f, 0.f, 0.f, 1.f); // Source fog identity.
        input.values[6] = g_SourceCharacterProgram == 7u ? float4(tangentView, 1.f) : 0.f;
        input.values[7] = float4(up, 0.f);
        input.values[8] = sourcePosition;
        input.values[9] = frontFace ? 1.f : 0.f;
    }
#endif
    input.lightColor = lightColor;
    input.shadow = shadow;
    return input;
}

#ifndef SOURCE_CHARACTER_LIGHT_PASS
struct SOURCE_CHARACTER_GBUFFER
{
    float4 diffuse;
    float4 normal;
    float4 depth;
    float4 pickPosition;
    float4 indirect;
    float4 extraUV;
    float4 surfaceUVTangent;
    float4 geometricNormal;
};

SOURCE_CHARACTER_GBUFFER EvaluateSourceCharacterGeometry(float2 uv, float4 extraUV,
    float3 worldPosition, float3 tangent, float3 binormal, float3 normal,
    float4 clipPosition, float4 screenPosition, float4x4 view, float4x4 projection, bool frontFace)
{
    float3 cameraPosition = -mul((float3x3)view, view[3].xyz);
    SOURCE_CHARACTER_NATIVE_INPUT nativeInput = MakeSourceCharacterInput(uv, extraUV,
        worldPosition, tangent, binormal, normal, cameraPosition, clipPosition,
        mul(view, projection), float3(0.f, 1.f, 0.f), 0.f, 1.f, frontFace);
    SOURCE_CHARACTER_NATIVE_OUTPUT native = EvaluateSourceCharacterBase(nativeInput);
    if (native.discarded) discard;
    // Opaque native PS alpha is explicitly zero and is not coverage. Hair and
    // eyelash carry actual opacity. The existing opaque character draw uses
    // ordered coverage until its source sorted-translucency passes are present.
    if (g_SourceCharacterProgram == 6u || g_SourceCharacterProgram == 7u)
    {
        static const float threshold[16] = {
            .5f, 8.5f, 2.5f, 10.5f, 12.5f, 4.5f, 14.5f, 6.5f,
            3.5f, 11.5f, 1.5f, 9.5f, 15.5f, 7.5f, 13.5f, 5.5f };
        uint2 pixel = uint2(screenPosition.xy) & 3u;
        clip(saturate(native.targets[0].a) - threshold[pixel.y * 4u + pixel.x] / 16.f);
    }
    SOURCE_CHARACTER_GBUFFER output;
    // The recovered direct PS returns radiance, accumulated into Specular.
    // Native MRT3 is the resolved diffuse colour. Direct radiance goes to
    // Specular; only the existing scene ambient multiplies this colour.
    output.diffuse = float4(native.targets[3].rgb, 1.f);
    float3 sourceMappedNormal = SourceCharacterOctDecode(native.targets[2].xy * 2.f - 1.f);
    output.normal = float4(sourceMappedNormal.xzy * .5f + .5f, 0.f);
    output.depth = float4(clipPosition.z / clipPosition.w, clipPosition.w / 1000.f,
        float(g_SourceCharacterRow), 5.f);
    output.pickPosition = float4(worldPosition, 1.f);
    output.indirect = float4(native.targets[0].rgb, 0.f);
    output.extraUV = extraUV;
    output.surfaceUVTangent = float4(uv, SourceCharacterOctEncode(SourceCharacterSafeUnit(tangent)));
    output.geometricNormal = float4(SourceCharacterSafeUnit(normal),
        (dot(cross(normal, tangent), binormal) < 0.f ? -1.f : 1.f) * (frontFace ? 1.f : 2.f));
    return output;
}
#endif
#endif
