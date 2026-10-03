// Experimental current-frame screen-space transport for MapPBR receivers only.
// No history, hidden geometry, hardware ray tracing or physical energy conservation.
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4 g_CameraViewMatrix, g_CameraProjMatrix, g_CameraProjMatrixInverse;
Texture2D g_DepthTexture, g_NormalTexture, g_MaterialSpecularTexture, g_DiffuseTexture;
Texture2D g_RadianceTexture, g_SceneHDRTexture, g_SceneBloomTexture;
Texture2D g_SSGIHalfTexture;
float2 g_vInverseSceneSize;
uint g_iSSGIRayCount = 8u;
float g_fSSGIRadius = 4.f, g_fSSGIStrength = .25f;
uint g_iSSRStepCount = 32u;
float g_fSSRMaxDistance = 20.f, g_fSSRThickness = .2f, g_fSSRStrength = .5f;
uint g_iBloomEnabled = 0u;
float g_fBloomThreshold = 1.f, g_fBloomSoftKnee = .5f, g_fBloomIntensity = 1.f;

struct VS_IN { float3 position : POSITION; float2 uv : TEXCOORD0; };
struct PS_IN { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
struct PS_OUT { float4 scene : SV_TARGET0; float4 bloom : SV_TARGET2; };
PS_IN VS_MAIN(VS_IN input)
{
    PS_IN output;
    output.position = mul(float4(input.position, 1.f), mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix));
    output.uv = input.uv;
    return output;
}

float4 FiniteBase(float4 value)
{
    // Preserve finite inputs bit-for-bit, including their alpha. Invalid source
    // components cannot be allowed to poison the remaining FP16 post chain.
    return float4(isfinite(value.x) ? value.x : 0.f, isfinite(value.y) ? value.y : 0.f,
        isfinite(value.z) ? value.z : 0.f, isfinite(value.w) ? value.w : 1.f);
}
PS_OUT LoadBase(PS_IN input)
{
    PS_OUT output;
    int3 pixel = int3(int2(input.position.xy), 0);
    output.scene = FiniteBase(g_SceneHDRTexture.Load(pixel));
    output.bloom = FiniteBase(g_SceneBloomTexture.Load(pixel));
    return output;
}
bool ValidUv(float2 uv)
{
    return all(isfinite(uv)) && all(isfinite(g_vInverseSceneSize)) &&
        all(g_vInverseSceneSize > 0.f) && all(g_vInverseSceneSize <= 1.f) &&
        all(uv >= .5f * g_vInverseSceneSize) && all(uv <= 1.f - .5f * g_vInverseSceneSize);
}
int3 PixelAt(float2 uv) { return int3(int2(uv / g_vInverseSceneSize), 0); }
bool ViewPosition(float2 uv, float4 depth, out float3 position)
{
    position = 0.f;
    // Depth ABI: x=NDC depth, y=view Z / 1000, w=receiver family.
    if (!all(isfinite(depth)) || depth.x < 0.f || depth.x >= .99999f || depth.y <= .000001f || depth.y >= .99999f) return false;
    float4 view = mul(float4(uv * float2(2.f, -2.f) + float2(-1.f, 1.f), depth.x, 1.f), g_CameraProjMatrixInverse);
    if (!all(isfinite(view)) || abs(view.w) < .000001f) return false;
    position = view.xyz / view.w;
    if (!all(isfinite(position)) || position.z <= .001f) return false;
    // Preserve the view-depth channel as the tracing metric, including matrices
    // whose inverse introduces a small reconstruction rounding difference.
    position *= depth.y * 1000.f / position.z;
    return all(isfinite(position));
}
bool ViewNormal(float4 encoded, out float3 normal)
{
    normal = 0.f;
    if (!all(isfinite(encoded))) return false;
    float3 world = encoded.xyz * 2.f - 1.f;
    normal = mul(float4(world, 0.f), g_CameraViewMatrix).xyz;
    float lengthSquared = dot(normal, normal);
    if (!isfinite(lengthSquared) || lengthSquared < .000001f) return false;
    normal *= rsqrt(lengthSquared);
    return all(isfinite(normal));
}
bool Receiver(PS_IN input, out float3 position, out float3 normal, out float4 material, out float4 encoded)
{
    position = normal = 0.f; material = encoded = 0.f;
    if (!ValidUv(input.uv)) return false;
    int3 pixel = int3(int2(input.position.xy), 0);
    float4 depth = g_DepthTexture.Load(pixel);
    if (depth.w != 3.f || !ViewPosition(input.uv, depth, position)) return false;
    encoded = g_NormalTexture.Load(pixel); material = g_MaterialSpecularTexture.Load(pixel);
    return all(isfinite(material)) && ViewNormal(encoded, normal);
}
bool ProjectView(float3 position, out float2 uv)
{
    uv = 0.f;
    if (!all(isfinite(position)) || position.z <= .001f) return false;
    float4 clip = mul(float4(position, 1.f), g_CameraProjMatrix);
    if (!all(isfinite(clip)) || clip.w <= .000001f) return false;
    float3 ndc = clip.xyz / clip.w;
    if (!all(isfinite(ndc)) || ndc.z < 0.f || ndc.z > 1.f) return false;
    uv = ndc.xy * float2(.5f, -.5f) + .5f;
    return ValidUv(uv);
}
bool TraceScreen(float3 origin, float3 direction, float2 receiverUv, float maxDistance,
    float thickness, uint steps, out float2 hitUv, out float hitDistance)
{
    hitUv = 0.f; hitDistance = 0.f;
    [loop] for (uint step = 1u; step <= steps; ++step)
    {
        float distance = maxDistance * ((float)step / (float)steps);
        float3 ray = origin + direction * distance;
        float2 uv;
        if (!ProjectView(ray, uv)) break;
        // Do not reflect/gather the originating depth pixel or subpixel noise.
        float2 pixelOffset = (uv - receiverUv) / g_vInverseSceneSize;
        if (dot(pixelOffset, pixelOffset) < 2.25f) continue;
        float4 depth = g_DepthTexture.Load(PixelAt(uv));
        float3 surface;
        if (!ViewPosition(uv, depth, surface)) continue;
        float separation = ray.z - surface.z;
        if (separation >= 0.f && separation <= thickness)
        {
            hitUv = uv; hitDistance = distance;
            return true;
        }
    }
    return false;
}
float3 BrightPass(float3 color)
{
    color = clamp(color, 0.f, 60000.f);
    float brightness = max(color.x, max(color.y, color.z));
    float soft = clamp(brightness - g_fBloomThreshold + g_fBloomSoftKnee, 0.f, 2.f * g_fBloomSoftKnee);
    soft = soft * soft / (4.f * g_fBloomSoftKnee + .00001f);
    return color * (max(brightness - g_fBloomThreshold, soft) / max(brightness, .00001f));
}
PS_OUT AddContribution(PS_OUT output, float3 contribution)
{
    if (!all(isfinite(contribution)) || all(contribution <= 0.f)) return output;
    contribution = clamp(contribution, 0.f, 60000.f);
    float3 before = output.scene.rgb;
    output.scene.rgb = clamp(before + contribution, 0.f, 60000.f);
    if (g_iBloomEnabled != 0u && isfinite(g_fBloomThreshold) && isfinite(g_fBloomSoftKnee) &&
        isfinite(g_fBloomIntensity) && g_fBloomThreshold >= 0.f && g_fBloomThreshold <= 64.f &&
        g_fBloomSoftKnee >= 0.f && g_fBloomSoftKnee <= 64.f && g_fBloomIntensity > 0.f)
    {
        // Keep authored base bloom. Add only the bright-pass increase caused by
        // this pass, so A/B and sequential GI+SSR do not re-extract base energy.
        float3 added = max(BrightPass(output.scene.rgb) - BrightPass(before), 0.f) * min(g_fBloomIntensity, 16.f);
        output.bloom.rgb = clamp(output.bloom.rgb + added, 0.f, 60000.f);
    }
    return output;
}
PS_OUT PS_SSGI(PS_IN input)
{
    PS_OUT output = LoadBase(input);
    if (!isfinite(g_fSSGIStrength) || g_fSSGIStrength <= 0.f || g_fSSGIStrength > 2.f ||
        !isfinite(g_fSSGIRadius) || g_fSSGIRadius < .1f || g_fSSGIRadius > 20.f ||
        (g_iSSGIRayCount != 4u && g_iSSGIRayCount != 8u && g_iSSGIRayCount != 16u)) return output;
    float3 position, normal; float4 material, encoded;
    if (!Receiver(input, position, normal, material, encoded)) return output;
    float3 albedo = g_DiffuseTexture.Load(int3(int2(input.position.xy), 0)).rgb;
    if (!all(isfinite(albedo))) return output;
    float3 helper = abs(normal.z) < .999f ? float3(0.f, 0.f, 1.f) : float3(0.f, 1.f, 0.f);
    float3 tangent = normalize(cross(helper, normal));
    float3 bitangent = cross(normal, tangent);
    float rotation = frac(sin(dot(floor(input.position.xy), float2(12.9898f, 78.233f))) * 43758.5453f) * 6.28318530718f;
    float bias = clamp(g_fSSGIRadius * .01f, .01f, .1f);
    float3 origin = position + normal * bias;
    float3 gathered = 0.f;
    [loop] for (uint ray = 0u; ray < g_iSSGIRayCount; ++ray)
    {
        // Cosine-weighted hemisphere: mean incident radiance approximates E/pi.
        float fraction = ((float)ray + .5f) / (float)g_iSSGIRayCount;
        float angle = rotation + (float)ray * 2.39996322973f;
        float radius = sqrt(fraction);
        float3 direction = tangent * (cos(angle) * radius) + bitangent * (sin(angle) * radius) + normal * sqrt(1.f - fraction);
        float2 hitUv; float distance;
        if (TraceScreen(origin, direction, input.uv, g_fSSGIRadius, max(.02f, g_fSSGIRadius / 16.f), 8u, hitUv, distance))
        {
            float3 radiance = g_RadianceTexture.Load(PixelAt(hitUv)).rgb;
            if (all(isfinite(radiance))) gathered += clamp(radiance, 0.f, 60000.f) * saturate(1.f - distance / g_fSSGIRadius);
        }
    }
    float3 bounce = gathered / (float)g_iSSGIRayCount * saturate(albedo) * (1.f - saturate(material.a)) * g_fSSGIStrength;
    return AddContribution(output, bounce);
}
// Each half texel represents the lower/right pixel of its 2x2 block; clamp
// the final odd-sized block. The resolve uses the same full-resolution guide.
int2 SSGIRepresentativePixel(int2 halfPixel)
{
    uint width, height; g_DepthTexture.GetDimensions(width, height);
    return min(halfPixel * 2 + 1, int2(width, height) - 1);
}
float4 PS_SSGI_GatherHalf(PS_IN halfInput) : SV_TARGET0
{
    if (!isfinite(g_fSSGIStrength) || g_fSSGIStrength <= 0.f || g_fSSGIStrength > 2.f ||
        !isfinite(g_fSSGIRadius) || g_fSSGIRadius < .1f || g_fSSGIRadius > 20.f ||
        (g_iSSGIRayCount != 4u && g_iSSGIRayCount != 8u && g_iSSGIRayCount != 16u)) return 0.f;
    PS_IN input;
    int2 pixel = SSGIRepresentativePixel(int2(halfInput.position.xy));
    input.position = float4(float2(pixel) + .5f, 0.f, 1.f);
    input.uv = clamp(input.position.xy * g_vInverseSceneSize,
        .5f * g_vInverseSceneSize, 1.f - .5f * g_vInverseSceneSize);
    float3 position, normal; float4 material, encoded;
    if (!Receiver(input, position, normal, material, encoded)) return 0.f;
    float3 helper = abs(normal.z) < .999f ? float3(0.f, 0.f, 1.f) : float3(0.f, 1.f, 0.f);
    float3 tangent = normalize(cross(helper, normal));
    float3 bitangent = cross(normal, tangent);
    float rotation = frac(sin(dot(floor(input.position.xy), float2(12.9898f, 78.233f))) * 43758.5453f) * 6.28318530718f;
    float bias = clamp(g_fSSGIRadius * .01f, .01f, .1f);
    float3 origin = position + normal * bias;
    float3 gathered = 0.f;
    [loop] for (uint ray = 0u; ray < g_iSSGIRayCount; ++ray)
    {
        // Cosine-weighted hemisphere: mean incident radiance approximates E/pi.
        float fraction = ((float)ray + .5f) / (float)g_iSSGIRayCount;
        float angle = rotation + (float)ray * 2.39996322973f;
        float radius = sqrt(fraction);
        float3 direction = tangent * (cos(angle) * radius) + bitangent * (sin(angle) * radius) + normal * sqrt(1.f - fraction);
        float2 hitUv; float distance;
        if (TraceScreen(origin, direction, input.uv, g_fSSGIRadius, max(.02f, g_fSSGIRadius / 16.f), 8u, hitUv, distance))
        {
            float3 radiance = g_RadianceTexture.Load(PixelAt(hitUv)).rgb;
            if (all(isfinite(radiance))) gathered += clamp(radiance, 0.f, 60000.f) * saturate(1.f - distance / g_fSSGIRadius);
        }
    }
    // Store incident radiance, not albedo-modulated bounce. Full-resolution
    // material evaluation below keeps dark/metallic edges from bleeding colour.
    float3 incident = gathered / (float)g_iSSGIRayCount;
    if (!all(isfinite(incident))) return 0.f;
    return float4(clamp(incident, 0.f, 60000.f), position.z);
}
PS_OUT PS_SSGI_ResolveHalf(PS_IN input)
{
    // Raster interpolation can exceed the final pixel center by one ULP at
    // odd viewport sizes. Rebuild and bound only this new path's receiver UV.
    input.uv = clamp((floor(input.position.xy) + .5f) * g_vInverseSceneSize,
        .5f * g_vInverseSceneSize, 1.f - .5f * g_vInverseSceneSize);
    PS_OUT output = LoadBase(input);
    if (!isfinite(g_fSSGIStrength) || g_fSSGIStrength <= 0.f || g_fSSGIStrength > 2.f) return output;
    float3 position, normal; float4 material, encoded;
    if (!Receiver(input, position, normal, material, encoded)) return output;
    float3 albedo = g_DiffuseTexture.Load(int3(int2(input.position.xy), 0)).rgb;
    if (!all(isfinite(albedo))) return output;
    uint halfWidth, halfHeight; g_SSGIHalfTexture.GetDimensions(halfWidth, halfHeight);
    float2 halfPosition = (floor(input.position.xy) - 1.f) * .5f;
    int2 basePixel = int2(floor(halfPosition));
    float2 fraction = frac(halfPosition);
    float3 incident = 0.f;
    float weightSum = 0.f;
    [unroll] for (int y = 0; y < 2; ++y)
    [unroll] for (int x = 0; x < 2; ++x)
    {
        int2 samplePixel = basePixel + int2(x, y);
        if (any(samplePixel < 0) || any(samplePixel >= int2(halfWidth, halfHeight))) continue;
        float4 sampleValue = g_SSGIHalfTexture.Load(int3(samplePixel, 0));
        if (!all(isfinite(sampleValue)) || sampleValue.a <= .001f) continue;
        int2 guidePixel = SSGIRepresentativePixel(samplePixel);
        float4 guideDepth = g_DepthTexture.Load(int3(guidePixel, 0));
        float3 guideNormal;
        if (guideDepth.w != 3.f || !ViewNormal(g_NormalTexture.Load(int3(guidePixel, 0)), guideNormal)) continue;
        float depthDelta = abs(sampleValue.a - position.z);
        float depthScale = max(.02f, position.z * .01f);
        float normalDot = saturate(dot(normal, guideNormal));
        if (depthDelta > 2.f * depthScale || normalDot < .85f) continue;
        float2 spatial = float2(x == 0 ? 1.f - fraction.x : fraction.x,
            y == 0 ? 1.f - fraction.y : fraction.y);
        float weight = spatial.x * spatial.y * exp2(-depthDelta / depthScale) * pow(normalDot, 16.f);
        incident += max(sampleValue.rgb, 0.f) * weight;
        weightSum += weight;
    }
    if (weightSum <= .000001f) return output;
    float3 bounce = incident / weightSum * saturate(albedo) * (1.f - saturate(material.a)) * g_fSSGIStrength;
    return AddContribution(output, bounce);
}

PS_OUT PS_SSR(PS_IN input)
{
    PS_OUT output = LoadBase(input);
    if (!isfinite(g_fSSRStrength) || g_fSSRStrength <= 0.f || g_fSSRStrength > 2.f ||
        !isfinite(g_fSSRMaxDistance) || g_fSSRMaxDistance < .1f || g_fSSRMaxDistance > 100.f ||
        !isfinite(g_fSSRThickness) || g_fSSRThickness < .01f || g_fSSRThickness > 2.f ||
        (g_iSSRStepCount != 16u && g_iSSRStepCount != 32u && g_iSSRStepCount != 64u)) return output;
    float3 position, normal; float4 material, encoded;
    if (!Receiver(input, position, normal, material, encoded)) return output;
    float lengthSquared = dot(position, position);
    if (!isfinite(lengthSquared) || lengthSquared < .000001f) return output;
    float3 incident = position * rsqrt(lengthSquared);
    float3 direction = reflect(incident, normal);
    float3 origin = position + normal * clamp(g_fSSRThickness * .05f, .005f, .05f);
    float2 hitUv; float distance;
    if (!TraceScreen(origin, direction, input.uv, g_fSSRMaxDistance, g_fSSRThickness, g_iSSRStepCount, hitUv, distance)) return output;
    float3 radiance = g_RadianceTexture.Load(PixelAt(hitUv)).rgb;
    if (!all(isfinite(radiance))) return output;
    float roughness = saturate(encoded.a);
    float3 f0 = saturate(material.rgb);
    float grazing = 1.f - saturate(dot(normal, -incident));
    float3 fresnel = f0 + (1.f - f0) * grazing * grazing * grazing * grazing * grazing;
    float edge = saturate(min(min(hitUv.x, 1.f - hitUv.x), min(hitUv.y, 1.f - hitUv.y)) * 20.f);
    float3 reflection = clamp(radiance, 0.f, 60000.f) * fresnel * ((1.f - roughness) * (1.f - roughness)) * edge * g_fSSRStrength;
    return AddContribution(output, reflection);
}

VertexShader ScreenSpaceVS = compile vs_5_0 VS_MAIN();
technique11 ScreenSpaceLighting
{
    pass SSGI
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = ScreenSpaceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSGI();
    }
    pass SSR
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = ScreenSpaceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSR();
    }
    pass SSGIGatherHalf
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = ScreenSpaceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSGI_GatherHalf();
    }
    pass SSGIResolveHalf
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = ScreenSpaceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SSGI_ResolveHalf();
    }
}
