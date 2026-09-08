
#include "Engine_Shader_Defines.hlsli"
#include "Shader_SourceStoneSurface.hlsli"

float4x4    g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4    g_CameraViewMatrix, g_CameraProjMatrix;
float4x4    g_ViewMatrixInverse, g_ProjMatrixInverse;
float4x4    g_LightViewMatrix, g_LightProjMatrix;
texture2D   g_Texture;
texture2D   g_DiffuseTexture, g_ShadeTexture;
texture2D   g_DepthTexture;
texture2D   g_SpecularTexture;
texture2D   g_MaterialSpecularTexture;
texture2D   g_GeometricNormalTexture;
Texture2D g_CharacterSurfaceTexture;
Texture2D g_CharacterGeometryTexture;
float4x4 g_SourceCharacterViewMatrix, g_SourceCharacterProjMatrix;
#define SOURCE_CHARACTER_LIGHT_PASS
#include "Shader_SourceCharacterMaterial.hlsli"

uint g_MaterialDebugView = 0;
texture2D   g_EmissiveTexture;
texture2D   g_LightDepthTexture;
texture2D   g_SceneHDRTexture;
texture2D   g_PostProcessTexture;
texture2D   g_PresentationOverlayTexture;
texture2D   g_BloomTexture;
texture2D   g_DistortionTexture;
texture2D   g_SSAOTexture;

float2      g_vSSAOTexelSize;
uint        g_iSSAOEnabled = 0u;
float       g_fSSAORadius;
float       g_fSSAOBias;
float       g_fSSAOIntensity;
float       g_fSSAOPower;
float       g_fSSAODistanceFade;
uint        g_iApplyDirectionalShadow = 0u;
float2      g_vShadowTexelSize;
float       g_fShadowDepthBias;
float       g_fShadowNormalBias;
float       g_fShadowStrength;
float2      g_vBloomTexelSize;
float2      g_vInverseSceneSize;
uint        g_iBloomEnabled;
float       g_fBloomThreshold;
float       g_fBloomSoftKnee;
float       g_fBloomIntensity;
float       g_fBloomScatter;
float       g_fToneMapExposure;
float       g_fToneMapWhitePoint;
float       g_fToneMapGamma;
uint        g_iFXAAEnabled;
float       g_fFXAASubpixel;
float       g_fFXAAEdgeThreshold;
float       g_fFXAAEdgeThresholdMin;
float       g_fPresentationTime;
float       g_fPresentationIntensity;
float       g_fPresentationSecondaryIntensity;
float       g_fPresentationFrequency;
uint        g_iPresentationSeed;
float4      g_vPresentationTint;
float2      g_vPresentationOverlayPosition;
float2      g_vPresentationOverlayScale;
float       g_fPresentationOverlayRotationDegrees;
float       g_fPresentationOverlayAngularVelocityDegreesPerSecond;
float2      g_vPresentationOverlayUvDriftPerSecond;
float4      g_vPresentationOverlayTint;
float       g_fPresentationOverlayAlpha;
uint        g_iPresentationOverlayCoverageChannel;
uint        g_iPresentationOverlayFilter;
uint        g_iPresentationOverlayAddress;

/* Post effects must not wrap energy from one screen edge to the opposite edge. */
sampler PostProcessSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};

sampler PresentationOverlayPointClampSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};

sampler PresentationOverlayPointWrapSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

sampler PresentationOverlayLinearClampSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};

sampler PresentationOverlayLinearWrapSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

sampler ShadowSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = BORDER;
    AddressV = BORDER;
    AddressW = BORDER;
    BorderColor = float4(1.f, 1.f, 1.f, 1.f);
};

vector      g_vCamPosition;

/* Height fog. The combine step already owns the depth target, so one screen
   space term covers terrain, buildings and characters at once. */
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



vector      g_vLightDir;
vector      g_vLightPos;
float       g_fLightRange;
float       g_fLightFalloffExponent;
float       g_fSpotInnerCos = 1.f;
float       g_fSpotOuterCos = 1.f;
vector      g_vLightDiffuse;
vector      g_vLightAmbient;
vector      g_vLightSpecular;

vector      g_vMtrlAmbient = 1.f;

texture2D   g_NormalTexture;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;    
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    
    matrix      matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix); 
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
  
    return Out;
}



struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_BACKBUFFER
{
    float4 vBackBuffer : SV_TARGET0;
};

struct PS_OUT_LIGHT
{
    float4 vShade : SV_TARGET0;
    float4 vSpecular : SV_TARGET1;
};

float Resolve_AmbientOcclusion(float2 vTexcoord)
{
    float fAmbientOcclusion = 1.f;
    if (0u != g_iSSAOEnabled)
    {
        fAmbientOcclusion = saturate(g_SSAOTexture.Sample(
            PostProcessSampler, vTexcoord).r);
    }
    return fAmbientOcclusion;
}

float Resolve_DirectionalShadow(float4 vWorldPos, float3 vNormal)
{
    float fShadow = 1.f;
    if (0u != g_iApplyDirectionalShadow)
    {
        float4 vBiasedWorldPos = vWorldPos;
        vBiasedWorldPos.xyz += normalize(vNormal) * g_fShadowNormalBias;
        float4 vShadowClip = mul(vBiasedWorldPos, g_LightViewMatrix);
        vShadowClip = mul(vShadowClip, g_LightProjMatrix);
        if (vShadowClip.w > 0.f)
        {
            float3 vShadowNDC = vShadowClip.xyz / vShadowClip.w;
            float2 vShadowUV = float2(
                vShadowNDC.x * 0.5f + 0.5f,
                vShadowNDC.y * -0.5f + 0.5f);
            const bool bInsideShadowMap =
                vShadowUV.x >= 0.f && vShadowUV.x <= 1.f &&
                vShadowUV.y >= 0.f && vShadowUV.y <= 1.f &&
                vShadowNDC.z > 0.f && vShadowNDC.z < 1.f;
            if (bInsideShadowMap)
            {
                const float fReceiverDepth =
                    vShadowNDC.z - max(g_fShadowDepthBias, 0.f);
                float fLitSamples = 0.f;
                [unroll]
                for (int iY = -1; iY <= 1; ++iY)
                {
                    [unroll]
                    for (int iX = -1; iX <= 1; ++iX)
                    {
                        const float2 vSampleUV = vShadowUV +
                            float2((float)iX, (float)iY) *
                            g_vShadowTexelSize;
                        const float fStoredDepth =
                            g_LightDepthTexture.SampleLevel(
                                ShadowSampler, vSampleUV, 0.f).r;
                        fLitSamples +=
                            fReceiverDepth <= fStoredDepth ? 1.f : 0.f;
                    }
                }

                const float fPCF = fLitSamples / 9.f;
                fShadow = lerp(
                    1.f, fPCF, saturate(g_fShadowStrength));
            }
        }
    }
    return fShadow;
}

/* 픽셀 셰이더 */
/* 전달받은 픽셀의 정보를 바탕으로하여 픽셀의 색을 결정한다 */

PS_OUT_BACKBUFFER PS_MAIN_DEBUG(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    Out.vBackBuffer = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    return Out;    
}

float3 Load_MaterialSpecular(PS_IN input, float legacyMask)
{
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    // A nearer legacy object writes depth.w=0 even if RT5 still contains a floor.
    return g_DepthTexture.Load(pixel).w == 1.f ?
        g_MaterialSpecularTexture.Load(pixel).rgb : legacyMask.xxx;
}

float3 Decode_MapPBRGeometricNormal(int3 pixel)
{
    // Load preserves the exact RGBA32_FLOAT mantissa. Bilinear sampling must
    // never interpolate this packed PickPos.W payload.
    const uint packed = asuint(g_GeometricNormalTexture.Load(pixel).w);
    const float2 oct = float2(packed & 2047u, (packed >> 11u) & 2047u) /
        2047.f * 2.f - 1.f;
    float3 normal = float3(oct, 1.f - abs(oct.x) - abs(oct.y));
    const float fold = max(-normal.z, 0.f);
    normal.xy += float2(normal.x >= 0.f ? -fold : fold,
        normal.y >= 0.f ? -fold : fold);
    return normalize(normal);
}

void Evaluate_MapSourcePBRDirect(float3 normal, float3 geometricNormal,
    float3 viewDirection, float3 lightDirection, float roughness, float metallic,
    float3 f0, out float3 diffuseWeight, out float3 specularContribution)
{
    // Recovered from all four actual BG_PCSELECT15 DirectionalLight PS
    // permutations. Preserve the source geometric-normal Fresnel correction,
    // roughness Fresnel term, visibility denominator and color-based peak cap.
    const float3 n = normalize(normal);
    const float3 v = normalize(viewDirection);
    const float3 l = normalize(lightDirection);
    const float3 sum = v + l;
    const float3 h = sum * rsqrt(max(dot(sum, sum), 0.000000000001f));
    const float noH = saturate(dot(n, h));
    const float noV = min(abs(dot(n, v)) + 0.00001f, 1.f);
    const float noL = saturate(dot(n, l));
    const float voH = saturate(dot(v, h));
    const float geometricLight = min(dot(normalize(geometricNormal), l) + 1.f, 1.f);
    const float correctedVoH = min(voH + 1.f - geometricLight, 1.f);
    const float fresnelBase = 1.f - correctedVoH;
    const float fresnelSquare = fresnelBase * fresnelBase;
    const float fresnelPower = fresnelSquare * fresnelSquare * fresnelBase;
    const float3 fresnel = f0 * (1.f - fresnelPower) +
        fresnelPower * saturate(50.f * f0.g) * (max(f0, 1.f - roughness) - f0);

    const float roughness2 = roughness * roughness;
    const float roughness4 = roughness2 * roughness2;
    const float distributionBase = noH * noH * (roughness4 - 1.f) + 1.f;
    const float distribution = roughness4 /
        max(3.14159265359f * distributionBase * distributionBase, 1e-30f);
    const float visibilityDenominator =
        noL * (noV * (1.f - roughness2) + roughness2) +
        noV * (noL * (1.f - roughness2) + roughness2);
    const float sourcePeak = 0.5f * distribution / max(visibilityDenominator, 1e-30f);
    const float cappedPeak = min(sourcePeak,
        3.f / (dot(fresnel, float3(0.3f, 0.59f, 0.11f)) + 0.0001f));
    // Source diffuse includes 1/pi followed by the final pi light scale.
    // Keep material albedo in RT0 so the existing combined pass multiplies it.
    diffuseWeight = (1.f - metallic) * (1.f - fresnel) * noL;
    specularContribution = fresnel * cappedPeak * noL * 3.14159265359f;
}

PS_OUT_LIGHT Resolve_MapPBRLight(PS_IN input, float3 worldPosition,
    float3 lightDirection, float attenuation, float directShadow)
{
    PS_OUT_LIGHT output;
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    const float4 normal = g_NormalTexture.Load(pixel);
    const float4 material = g_MaterialSpecularTexture.Load(pixel);
    const float materialAO = saturate(g_DepthTexture.Load(pixel).z);
    float3 diffuseWeight, specularContribution;
    Evaluate_MapSourcePBRDirect(normal.xyz * 2.f - 1.f,
        Decode_MapPBRGeometricNormal(pixel), g_vCamPosition.xyz - worldPosition,
        lightDirection, normal.a, saturate(material.a), material.rgb,
        diffuseWeight, specularContribution);
    // A bound component lightmap already supplies its indirect irradiance.
    // Keep authored scene values but exclude the duplicate project ambient
    // on those PBR pixels; unbaked PBR retains that explicit approximation.
    const bool hasBakedLighting = (asuint(g_GeometricNormalTexture.Load(pixel).w) &
        0x00400000u) != 0u;
    const float ao = Resolve_AmbientOcclusion(input.vTexcoord) * materialAO;
    const float3 ambient = g_vLightAmbient.rgb * g_vMtrlAmbient.rgb * ao *
        (1.f - saturate(material.a)) * (hasBakedLighting ? 0.f : 1.f);
    output.vShade = float4(g_vLightDiffuse.rgb * attenuation *
        (diffuseWeight * directShadow + ambient), 0.f);
    // Native PBR uses one incoming light color for both BRDF lobes.
    // The legacy Phong specular control does not disable recovered PBR energy.
    output.vSpecular = float4(g_vLightDiffuse.rgb * specularContribution *
        attenuation * directShadow, 0.f);
    return output;
}

PS_OUT_LIGHT Resolve_MapSourceSpecularLight(PS_IN input, float3 worldPosition,
    float3 lightDirection, float attenuation, float directShadow)
{
    PS_OUT_LIGHT output;
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    const float3 n = normalize(g_NormalTexture.Load(pixel).xyz * 2.f - 1.f);
    const float3 l = normalize(lightDirection);
    const float3 v = normalize(g_vCamPosition.xyz - worldPosition);
    const float3 sum = l + v;
    const float3 h = sum * rsqrt(max(dot(sum, sum), 1e-12f));
    const float ndoth = abs(dot(n, h));
    const float specularPower = g_DepthTexture.Load(pixel).z;
    const float lobe = ndoth < 0.000001f ? 0.f : min(pow(ndoth, specularPower), 1.f);
    const float4 material = g_MaterialSpecularTexture.Load(pixel);
    const bool hasBakedLighting = (asuint(g_GeometricNormalTexture.Load(pixel).w) &
        0x00400000u) != 0u;
    const float3 ambient = hasBakedLighting ? 0.f :
        g_vLightAmbient.rgb * g_vMtrlAmbient.rgb * Resolve_AmbientOcclusion(input.vTexcoord);
    // The free non-PBR RT5 alpha carries the albedo HDR normalization scale.
    // Source diffuse can exceed 1 while the product albedo target is UNORM8.
    output.vShade = float4(g_vLightDiffuse.rgb * attenuation * material.a *
        (saturate(dot(n, l)) * directShadow + ambient), 0.f);
    // Native Blinn lobe has an RGB cap of 2 after shadow multiplication and
    // uses the same incoming color as diffuse. It has no extra NoL factor.
    output.vSpecular = float4(g_vLightDiffuse.rgb * attenuation *
        clamp(material.rgb * lobe * directShadow, 0.f, 2.f), 0.f);
    return output;
}

PS_OUT_LIGHT Resolve_MapSourceStoneLight(PS_IN input, float3 worldPosition,
    float3 lightDirection, float attenuation, float directShadow)
{
    PS_OUT_LIGHT output = (PS_OUT_LIGHT)0;
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    const float3 baseNormal = SourceStoneUnit(g_NormalTexture.Load(pixel).xyz * 2.f - 1.f);
    const float3 mixedNormal = g_CharacterGeometryTexture.Load(pixel).xyz;
    const float3 diffuse = g_CharacterSurfaceTexture.Load(pixel).rgb;
    const float3 specular = g_MaterialSpecularTexture.Load(pixel).rgb;
    const float3 radiance = EvaluateSourceStoneDirect(diffuse, specular,
        baseNormal, mixedNormal, g_vCamPosition.xyz - worldPosition,
        lightDirection, g_DepthTexture.Load(pixel).z, g_vLightDiffuse.rgb,
        directShadow.xxx, float4(0.f, 0.f, 0.f, 1.f));
    // RT4 already holds source baked diffuse AND baked specular. Source scene
    // hemisphere/ambient remains explicitly unbound, not duplicated per light.
    // Direct carries albedo, so the combined pass must add it without RT0.
    output.vSpecular = float4(radiance * attenuation, 0.f);
    return output;
}

PS_OUT_LIGHT Resolve_SourceCharacterLight(PS_IN input, float3 lightDirection,
    float attenuation, float directShadow)
{
    PS_OUT_LIGHT output = (PS_OUT_LIGHT)0;
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    const float4 depth = g_DepthTexture.Load(pixel);
    if (depth.w != 5.f || depth.z != float(g_SourceCharacterRow) ||
        g_SourceCharacterRow == 0u || attenuation <= 0.f) discard;
    const float4 surface = g_CharacterSurfaceTexture.Load(pixel);
    const float4 geometry = g_CharacterGeometryTexture.Load(pixel);
    const float3 position = g_GeometricNormalTexture.Load(pixel).xyz;
    const float3 normal = SourceCharacterSafeUnit(geometry.xyz);
    const float3 tangent = SourceCharacterOctDecode(surface.zw);
    const float3 binormal = SourceCharacterSafeUnit(cross(normal, tangent)) * (geometry.w < 0.f ? -1.f : 1.f);
    const float4 clipPosition = mul(mul(float4(position,1.f),
        g_SourceCharacterViewMatrix),g_SourceCharacterProjMatrix);
    SOURCE_CHARACTER_NATIVE_INPUT nativeInput = MakeSourceCharacterInput(surface.xy,
        g_MaterialSpecularTexture.Load(pixel), position, tangent, binormal, normal,
        g_vCamPosition.xyz, clipPosition,
        mul(g_SourceCharacterViewMatrix,g_SourceCharacterProjMatrix),
        lightDirection, g_vLightDiffuse.rgb, directShadow, abs(geometry.w) < 1.5f);
    SOURCE_CHARACTER_NATIVE_OUTPUT native = EvaluateSourceCharacterLight(nativeInput);
    if (native.discarded) return output;
    // Native direct output already contains albedo and both lighting lobes.
    // The existing combined pass adds Specular without multiplying albedo again.
    // Hair's native pass uses its own projected PCF protocol. Until that
    // engine-owned projection is available, use this renderer's shadow once.
    const float shadowAdapter = g_SourceCharacterProgram == 7u ? directShadow : 1.f;
    output.vSpecular = float4(native.targets[0].rgb * (attenuation * shadowAdapter), 0.f);
    // Engine-owned source SH/cube values are not authored in these MICs. Keep
    // the scene's explicit ambient approximation separate from recovered direct.
    output.vShade = g_vLightDiffuse * g_vLightAmbient * g_vMtrlAmbient *
        (attenuation * Resolve_AmbientOcclusion(input.vTexcoord));
    output.vShade.a = 0.f;
    return output;
}

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    vector          vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    
    /* 0 ~ 1 => -1 ~ 1 */
    vector vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);    
    

    
    vector vReflect = reflect(g_vLightDir, vNormal);
    
    
    vector      vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vWorldPos = vWorldPos * fViewZ;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 로컬위치 * 월드행렬  */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);

    const float fDirectDiffuse = saturate(dot(
        normalize(g_vLightDir) * -1.f, normalize(vNormal)));
    const float fDirectionalShadow = Resolve_DirectionalShadow(
        vWorldPos, vNormal.xyz);
    const float4 sourceDepth = g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0));
    if (g_SourceCharacterRow != 0u)
        return Resolve_SourceCharacterLight(In, -g_vLightDir.xyz, 1.f, fDirectionalShadow);
    if (sourceDepth.w == 5.f) return (PS_OUT_LIGHT)0;
    if (sourceDepth.w == 7.f)
        return Resolve_MapSourceStoneLight(In, vWorldPos.xyz, -g_vLightDir.xyz, 1.f, fDirectionalShadow);
    if (g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0)).w == 4.f)
        return Resolve_MapSourceSpecularLight(In, vWorldPos.xyz, -g_vLightDir.xyz,
            1.f, fDirectionalShadow);
    if (g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0)).w == 3.f)
        return Resolve_MapPBRLight(In, vWorldPos.xyz, -g_vLightDir.xyz,
            1.f, fDirectionalShadow);
    const float fAmbientOcclusion =
        Resolve_AmbientOcclusion(In.vTexcoord);
    Out.vShade = g_vLightDiffuse *
        (fDirectDiffuse * fDirectionalShadow +
        (g_vLightAmbient * g_vMtrlAmbient) * fAmbientOcclusion);
    
    vector vLook = vWorldPos - g_vCamPosition;
    
    const float specularPower = vDepthDesc.z > 0.f ? vDepthDesc.z : 50.f;
    Out.vSpecular = g_vLightSpecular *
        pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))),
            specularPower) * float4(Load_MaterialSpecular(In, vNormalDesc.a), 1.f) * fDirectionalShadow;
    
    return Out;
}

float Resolve_PointLightAttenuation(float fDistance)
{
    float fAttenuation = 0.f;
    if (isfinite(g_fLightRange) && g_fLightRange > 0.f)
    {
        fAttenuation = saturate(
            (g_fLightRange - fDistance) / g_fLightRange);
        if (1.f != g_fLightFalloffExponent)
            fAttenuation = pow(
                fAttenuation, g_fLightFalloffExponent);
    }
    return fAttenuation;
}

PS_OUT_LIGHT Resolve_LocalLight(PS_IN In, bool bSpot)
{
    PS_OUT_LIGHT Out;
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    
    /* 0 ~ 1 => -1 ~ 1 */
    vector vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
    
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vWorldPos = vWorldPos * fViewZ;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 로컬위치 * 월드행렬  */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
    
    
    vector vLightDir = vWorldPos - g_vLightPos;
    
    float fAtt = Resolve_PointLightAttenuation(length(vLightDir));
    if (bSpot)
    {
        // The direction points from the light into the illuminated cone.
        if (dot(vLightDir.xyz, vLightDir.xyz) <= 0.000000000001f)
        {
            Out.vShade = 0.f;
            Out.vSpecular = 0.f;
            return Out;
        }
        float fCosAngle = dot(normalize(vLightDir.xyz), normalize(g_vLightDir.xyz));
        float fCone = saturate((fCosAngle - g_fSpotOuterCos) /
            max(g_fSpotInnerCos - g_fSpotOuterCos, 0.0001f));
        fAtt *= fCone * fCone;
    }
    
    const float4 sourceDepth = g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0));
    if (g_SourceCharacterRow != 0u)
        return Resolve_SourceCharacterLight(In, -vLightDir.xyz, fAtt, 1.f);
    if (sourceDepth.w == 5.f) return (PS_OUT_LIGHT)0;
    if (sourceDepth.w == 7.f)
        return Resolve_MapSourceStoneLight(In, vWorldPos.xyz, -vLightDir.xyz, fAtt, 1.f);
    if (g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0)).w == 4.f)
        return Resolve_MapSourceSpecularLight(In, vWorldPos.xyz, -vLightDir.xyz, fAtt, 1.f);
    if (g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0)).w == 3.f)
        return Resolve_MapPBRLight(In, vWorldPos.xyz, -vLightDir.xyz, fAtt, 1.f);
    float fAmbientOcclusion = Resolve_AmbientOcclusion(In.vTexcoord);
    Out.vShade = (g_vLightDiffuse * (saturate(dot(normalize(vLightDir) * -1.f, normalize(vNormal)))
    + (g_vLightAmbient * g_vMtrlAmbient) * fAmbientOcclusion)) * fAtt;
    
    
    vector vReflect = reflect(vLightDir, vNormal);
    
    
    vector vLook = vWorldPos - g_vCamPosition;
    
    const float specularPower = vDepthDesc.z > 0.f ? vDepthDesc.z : 50.f;
    Out.vSpecular = g_vLightSpecular *
        pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))),
            specularPower) * float4(Load_MaterialSpecular(In, vNormalDesc.a), 1.f) * fAtt;
    
    return Out;
}



PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
    return Resolve_LocalLight(In, false);
}

PS_OUT_LIGHT PS_MAIN_SPOT(PS_IN In)
{
    return Resolve_LocalLight(In, true);
}

bool Is_SSAOBackground(float4 vDepthDesc)
{
    return vDepthDesc.x >= 0.99999f || vDepthDesc.y >= 0.99999f;
}

float3 Decode_SSAONormal(float2 vTexcoord)
{
    // Marker 7 stores the unnormalized final mixed normal in the shared RT7;
    // RT1 deliberately retains the base normal used by its direct specular.
    if (g_DepthTexture.SampleLevel(PointSampler, vTexcoord, 0.f).w == 7.f)
        return SourceStoneUnit(g_CharacterGeometryTexture.SampleLevel(
            PointSampler, vTexcoord, 0.f).xyz);
    float3 vNormal = g_NormalTexture.Sample(
        PostProcessSampler, vTexcoord).xyz * 2.f - 1.f;
    float fLengthSquared = dot(vNormal, vNormal);
    return fLengthSquared > 0.000001f ?
        vNormal * rsqrt(fLengthSquared) : float3(0.f, 0.f, 1.f);
}

float3 Decode_SSAOViewNormal(float2 vTexcoord)
{
    float3 vWorldNormal = Decode_SSAONormal(vTexcoord);
    return normalize(mul(
        float4(vWorldNormal, 0.f), g_CameraViewMatrix).xyz);
}

float3 Reconstruct_SSAOViewPosition(
    float2 vTexcoord, float4 vDepthDesc)
{
    float fViewDepth = max(vDepthDesc.y * 1000.f, 0.001f);
    float4 vClipPosition = float4(
        vTexcoord.x * 2.f - 1.f,
        vTexcoord.y * -2.f + 1.f,
        vDepthDesc.x,
        1.f) * fViewDepth;
    float4 vViewPosition = mul(vClipPosition, g_ProjMatrixInverse);
    return vViewPosition.xyz / max(abs(vViewPosition.w), 0.000001f);
}

float SSAO_Hash(float2 vPosition)
{
    return frac(sin(dot(vPosition,
        float2(12.9898f, 78.233f))) * 43758.5453f);
}

PS_OUT_BACKBUFFER PS_MAIN_SSAO_RAW(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float4 vCenterDepthDesc = g_DepthTexture.Sample(
        PostProcessSampler, In.vTexcoord);
    if (Is_SSAOBackground(vCenterDepthDesc))
    {
        Out.vBackBuffer = 1.f;
        return Out;
    }

    float fCenterViewDepth = max(vCenterDepthDesc.y * 1000.f, 0.001f);
    float3 vCenterViewPosition = Reconstruct_SSAOViewPosition(
        In.vTexcoord, vCenterDepthDesc);
    float3 vCenterNormal = Decode_SSAOViewNormal(In.vTexcoord);
    if (dot(vCenterNormal, vCenterViewPosition) > 0.f)
        vCenterNormal *= -1.f;
    float fProjectionY = max(abs(g_CameraProjMatrix[1][1]), 0.0001f);
    float fRadiusPixels = clamp(
        g_fSSAORadius * fProjectionY * 0.5f /
        (fCenterViewDepth * max(g_vSSAOTexelSize.y, 0.000001f)),
        1.f, 64.f);
    float2 vPixel = floor(In.vTexcoord /
        max(g_vSSAOTexelSize, 0.000001f));
    float fRotation = SSAO_Hash(vPixel) * 6.28318530718f;
    float fOcclusion = 0.f;
    float fWeight = 0.f;

    [unroll]
    for (int iSample = 0; iSample < 12; ++iSample)
    {
        float fSampleFraction = ((float)iSample + 0.5f) / 12.f;
        float fAngle = fRotation + fSampleFraction * 6.28318530718f;
        float fSampleRadius = lerp(0.25f, 1.f, fSampleFraction);
        float2 vDirection = float2(cos(fAngle), sin(fAngle));
        float2 vSampleUV = In.vTexcoord + vDirection *
            fRadiusPixels * fSampleRadius * g_vSSAOTexelSize;
        if (vSampleUV.x <= 0.f || vSampleUV.x >= 1.f ||
            vSampleUV.y <= 0.f || vSampleUV.y >= 1.f)
        {
            continue;
        }
        float4 vSampleDepthDesc = g_DepthTexture.Sample(
            PostProcessSampler, vSampleUV);
        if (Is_SSAOBackground(vSampleDepthDesc))
            continue;

        float3 vSampleViewPosition = Reconstruct_SSAOViewPosition(
            vSampleUV, vSampleDepthDesc);
        float3 vCenterToSample =
            vSampleViewPosition - vCenterViewPosition;
        float fSampleDistance = length(vCenterToSample);
        if (fSampleDistance <= 0.0001f ||
            fSampleDistance >= g_fSSAORadius)
        {
            continue;
        }

        float fRangeWeight = saturate(
            1.f - fSampleDistance / max(g_fSSAORadius, 0.001f));
        float fHemisphereOcclusion = saturate(
            (dot(vCenterNormal, vCenterToSample) - g_fSSAOBias) /
            max(fSampleDistance, 0.001f));
        float fRadialWeight = lerp(1.f, 0.5f, fSampleFraction);
        float fSampleWeight = fRangeWeight * fRadialWeight;
        fWeight += fSampleWeight;
        fOcclusion += fHemisphereOcclusion * fSampleWeight;
    }

    fOcclusion = fWeight > 0.00001f ? fOcclusion / fWeight : 0.f;
    float fAmbientOcclusion = pow(saturate(
        1.f - fOcclusion * g_fSSAOIntensity), g_fSSAOPower);
    float fFadeStart = max(
        g_fSSAODistanceFade * 0.65f, g_fSSAORadius);
    float fDistanceFade = saturate(
        (fCenterViewDepth - fFadeStart) /
        max(g_fSSAODistanceFade - fFadeStart, 0.001f));
    fAmbientOcclusion = lerp(fAmbientOcclusion, 1.f, fDistanceFade);
    Out.vBackBuffer = fAmbientOcclusion;
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_SSAO_BLUR(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float4 vCenterDepthDesc = g_DepthTexture.Sample(
        PostProcessSampler, In.vTexcoord);
    if (Is_SSAOBackground(vCenterDepthDesc))
    {
        Out.vBackBuffer = 1.f;
        return Out;
    }

    float fCenterViewDepth = vCenterDepthDesc.y * 1000.f;
    float3 vCenterNormal = Decode_SSAONormal(In.vTexcoord);
    float fDepthSigma = max(
        g_fSSAORadius * 0.25f, g_fSSAOBias * 4.f + 0.001f);
    float fWeightedAO = 0.f;
    float fWeight = 0.f;

    [unroll]
    for (int iY = -2; iY <= 2; ++iY)
    {
        [unroll]
        for (int iX = -2; iX <= 2; ++iX)
        {
            float2 vOffset = float2((float)iX, (float)iY);
            float2 vSampleUV = clamp(
                In.vTexcoord + vOffset * g_vSSAOTexelSize, 0.f, 1.f);
            float4 vSampleDepthDesc = g_DepthTexture.Sample(
                PostProcessSampler, vSampleUV);
            if (Is_SSAOBackground(vSampleDepthDesc))
                continue;

            float fSampleViewDepth = vSampleDepthDesc.y * 1000.f;
            float3 vSampleNormal = Decode_SSAONormal(vSampleUV);
            float fSpatialWeight = exp(-dot(vOffset, vOffset) * 0.35f);
            float fDepthWeight = exp(-abs(
                fCenterViewDepth - fSampleViewDepth) / fDepthSigma);
            float fNormalWeight = pow(
                saturate(dot(vCenterNormal, vSampleNormal)), 16.f);
            float fSampleWeight =
                fSpatialWeight * fDepthWeight * fNormalWeight;
            fWeightedAO += g_SSAOTexture.Sample(
                PostProcessSampler, vSampleUV).r * fSampleWeight;
            fWeight += fSampleWeight;
        }
    }

    float fCenterAO = g_SSAOTexture.Sample(
        PostProcessSampler, In.vTexcoord).r;
    float fAmbientOcclusion = fWeight > 0.00001f ?
        fWeightedAO / fWeight : fCenterAO;
    Out.vBackBuffer = saturate(fAmbientOcclusion);
    return Out;
}

/* The ceiling is the world height the fog fades out at, and the exponential
   term thickens it the deeper a surface sits below that line. Distance is
   folded in so nearby ground stays readable while the valley reads solid.
   Emissive is added after this call so glowing sources survive the fog. */
/* Value noise keeps the cloud banks procedural, so no extra texture has to
   be authored, streamed or kept in sync with an Area. */
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

float3 Resolve_HeightFog(float3 vLitColor, float2 vTexcoord)
{
    if (0u == g_iHeightFogEnabled)
        return vLitColor;

    const vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, vTexcoord);
    /* An untouched depth texel is empty background, not a fogged surface. */
    if (vDepthDesc.y <= 0.f)
        return vLitColor;

    const float fViewZ = vDepthDesc.y * 1000.f;
    vector vWorldPos;
    vWorldPos.x = vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    vWorldPos = vWorldPos * fViewZ;
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);

    const float fDrift = sin(g_fPresentationClock * g_fHeightFogDriftSpeed);
    const float fCeiling =
        g_fHeightFogTopHeight + fDrift * g_fHeightFogDriftHeight;
    const float fDensity = max(0.f,
        g_fHeightFogDensity + fDrift * g_fHeightFogDriftDensity);

    const float fBelowCeiling = max(0.f, fCeiling - vWorldPos.y);
    const float fHeightTerm =
        1.f - exp(-fBelowCeiling * g_fHeightFogFalloff);

    const float fDistance = max(0.f,
        length(vWorldPos.xyz - g_vCamPosition.xyz) - g_fHeightFogStartDistance);
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
            (vWorldPos.xz + vTravel) * g_fFogPatchScale);
        const float fThreshold = 1.f - g_fFogCoverage;
        fCoverage = smoothstep(fThreshold - g_fFogPatchSoftness,
            fThreshold + g_fFogPatchSoftness, fNoise);
    }

    const float fFog = saturate(fHeightTerm * fDistanceTerm * fCoverage) *
        g_fHeightFogMaximumOpacity;
    return lerp(vLitColor, g_vHeightFogColor.rgb, fFog);
}

PS_OUT_BACKBUFFER PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.f == vDiffuse.a)
        discard;
    
    vector vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
    
    vector vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);
    vector vEmissive = g_EmissiveTexture.Sample(LinearSampler, In.vTexcoord);
    
    
    vector vLitColor = vDiffuse * vShade + vSpecular;
    vLitColor.rgb = Resolve_HeightFog(vLitColor.rgb, In.vTexcoord);
    /* Emissive and Effect HDR energy are light sources, not receivers.  They
       must remain available to bloom even when the carrier is in shadow. */
    Out.vBackBuffer = vLitColor + vEmissive;
    return Out;
}


float3 Sanitize_HDR(float3 vColor)
{
    // FP16 SceneHDR can contain INF after heavy additive overlap.  Keep the
    // post chain finite before division in bright-pass and Hable tone mapping.
    return min(max(vColor, 0.f), 60000.f);
}

float3 Extract_Bloom(float3 vColor)
{
    float fThreshold = max(g_fBloomThreshold, 0.f);
    float fSoftKnee = max(g_fBloomSoftKnee, 0.f);

    float fBrightness = max(vColor.r, max(vColor.g, vColor.b));
    float fSoft = clamp(
        fBrightness - fThreshold + fSoftKnee,
        0.f, 2.f * fSoftKnee);
    fSoft = fSoft * fSoft / (4.f * fSoftKnee + 0.00001f);

    float fContribution = max(fBrightness - fThreshold, fSoft) /
        max(fBrightness, 0.00001f);
    return vColor * fContribution;
}

PS_OUT_BACKBUFFER PS_MAIN_BLOOM_EXTRACT(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float3 vScene = Sanitize_HDR(g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb);
    Out.vBackBuffer = float4(Extract_Bloom(vScene), 1.f);
    return Out;
}

float3 Blur_Bloom(float2 vTexcoord, float2 vDirection)
{
    float2 vStep = g_vBloomTexelSize * vDirection *
        max(g_fBloomScatter, 0.25f);
    float3 vResult = g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord).rgb * 0.2270270270f;

    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord + vStep).rgb * 0.1945945946f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord - vStep).rgb * 0.1945945946f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord + vStep * 2.f).rgb * 0.1216216216f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord - vStep * 2.f).rgb * 0.1216216216f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord + vStep * 3.f).rgb * 0.0540540541f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord - vStep * 3.f).rgb * 0.0540540541f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord + vStep * 4.f).rgb * 0.0162162162f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord - vStep * 4.f).rgb * 0.0162162162f;
    return vResult;
}

PS_OUT_BACKBUFFER PS_MAIN_BLOOM_BLUR_H(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    Out.vBackBuffer = float4(
        Blur_Bloom(In.vTexcoord, float2(1.f, 0.f)), 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_BLOOM_BLUR_V(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    Out.vBackBuffer = float4(
        Blur_Bloom(In.vTexcoord, float2(0.f, 1.f)), 1.f);
    return Out;
}

float Presentation_Hash(float2 vPosition, uint iSeed)
{
    float fSeed = (float)(iSeed & 0x00ffffffu);
    return frac(sin(dot(vPosition + fSeed,
        float2(12.9898f, 78.233f))) * 43758.5453f);
}

PS_OUT_BACKBUFFER PS_MAIN_SCENE_RESOLVE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float2 vDistortion = clamp(g_DistortionTexture.Sample(
        PostProcessSampler, In.vTexcoord).rg, -0.05f, 0.05f);
    float3 vScene = Sanitize_HDR(g_SceneHDRTexture.Sample(
        PostProcessSampler, In.vTexcoord + vDistortion).rgb);
    Out.vBackBuffer = float4(vScene, 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_RGB_NOISE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float fTick = floor(g_fPresentationTime *
        max(g_fPresentationFrequency, 0.0001f) * 60.f);
    float fNoise = Presentation_Hash(
        floor(In.vTexcoord * float2(320.f, 180.f)) + fTick,
        g_iPresentationSeed) - 0.5f;
    float fOffset = clamp(g_fPresentationIntensity, 0.f, 8.f) *
        fNoise * 0.015f;
    float3 vColor;
    vColor.r = g_PostProcessTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord + float2(fOffset, 0.f), 0.f, 1.f)).r;
    vColor.g = g_PostProcessTexture.Sample(PostProcessSampler,
        In.vTexcoord).g;
    vColor.b = g_PostProcessTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord - float2(fOffset, 0.f), 0.f, 1.f)).b;
    vColor += fNoise * clamp(g_fPresentationSecondaryIntensity,
        0.f, 8.f) * g_vPresentationTint.rgb;
    Out.vBackBuffer = float4(Sanitize_HDR(vColor), 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_ZOOM_BLUR(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float2 vFromCenter = In.vTexcoord - float2(0.5f, 0.5f);
    float fStrength = clamp(g_fPresentationIntensity, 0.f, 8.f) * 0.025f;
    float3 vColor = 0.f;
    [unroll]
    for (int iSample = 0; iSample < 8; ++iSample)
    {
        float fSample = (float)iSample / 7.f;
        float2 vUV = clamp(In.vTexcoord - vFromCenter *
            fStrength * fSample, 0.f, 1.f);
        vColor += g_PostProcessTexture.Sample(
            PostProcessSampler, vUV).rgb;
    }
    vColor *= 0.125f;
    float3 vSource = g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb;
    float fMix = saturate(g_fPresentationSecondaryIntensity > 0.f ?
        g_fPresentationSecondaryIntensity : g_fPresentationIntensity);
    Out.vBackBuffer = float4(Sanitize_HDR(
        lerp(vSource, vColor * g_vPresentationTint.rgb, fMix)), 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_CHROMATIC_ABERRATION(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float2 vFromCenter = In.vTexcoord - float2(0.5f, 0.5f);
    float fExponent = g_fPresentationSecondaryIntensity > 0.f ?
        clamp(g_fPresentationSecondaryIntensity, 0.5f, 8.f) : 2.f;
    float fFalloff = pow(saturate(length(vFromCenter) * 1.41421f), fExponent);
    float2 vShift = vFromCenter *
        clamp(g_fPresentationIntensity, 0.f, 8.f) * 0.012f * fFalloff;
    float3 vColor;
    vColor.r = g_PostProcessTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord + vShift, 0.f, 1.f)).r;
    vColor.g = g_PostProcessTexture.Sample(PostProcessSampler,
        In.vTexcoord).g;
    vColor.b = g_PostProcessTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord - vShift, 0.f, 1.f)).b;
    Out.vBackBuffer = float4(Sanitize_HDR(
        vColor * g_vPresentationTint.rgb), 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_FILM_NOISE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float fTick = floor(g_fPresentationTime *
        max(g_fPresentationFrequency, 0.0001f) * 60.f);
    float fNoise = Presentation_Hash(
        floor(In.vTexcoord * float2(1024.f, 1024.f)) + fTick,
        g_iPresentationSeed) * 2.f - 1.f;
    float fScan = sin((In.vTexcoord.y + g_fPresentationTime) *
        max(g_fPresentationFrequency, 1.f) * 256.f);
    float3 vSource = g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb;
    float3 vNoise = (fNoise * clamp(g_fPresentationIntensity, 0.f, 8.f) +
        fScan * clamp(g_fPresentationSecondaryIntensity, 0.f, 8.f)) *
        g_vPresentationTint.rgb;
    Out.vBackBuffer = float4(Sanitize_HDR(vSource + vNoise), 1.f);
    return Out;
}

float4 Sample_PresentationOverlay(float2 vTexcoord)
{
	float4 vSample = float4(0.f, 0.f, 0.f, 0.f);
    if (0u == g_iPresentationOverlayFilter)
    {
        if (0u == g_iPresentationOverlayAddress)
            vSample = g_PresentationOverlayTexture.Sample(
                PresentationOverlayPointClampSampler, vTexcoord);
		else
			vSample = g_PresentationOverlayTexture.Sample(
				PresentationOverlayPointWrapSampler, vTexcoord);
    }
	else if (0u == g_iPresentationOverlayAddress)
		vSample = g_PresentationOverlayTexture.Sample(
			PresentationOverlayLinearClampSampler, vTexcoord);
	else
		vSample = g_PresentationOverlayTexture.Sample(
			PresentationOverlayLinearWrapSampler, vTexcoord);
	return vSample;
}

float Select_PresentationOverlayCoverage(float4 vSample)
{
	float fCoverage = vSample.a;
    if (0u == g_iPresentationOverlayCoverageChannel)
		fCoverage = vSample.r;
	else if (1u == g_iPresentationOverlayCoverageChannel)
		fCoverage = vSample.g;
	else if (2u == g_iPresentationOverlayCoverageChannel)
		fCoverage = vSample.b;
	return fCoverage;
}

PS_OUT_BACKBUFFER PS_MAIN_TEXTURED_OVERLAY(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float3 vSource = Sanitize_HDR(g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb);
    float fRotation = radians(-(
        g_fPresentationOverlayRotationDegrees +
        g_fPresentationOverlayAngularVelocityDegreesPerSecond *
            g_fPresentationTime));
    float fCos = cos(fRotation);
    float fSin = sin(fRotation);
    float2 vCentered = In.vTexcoord - g_vPresentationOverlayPosition;
    float2 vRotated = float2(
        vCentered.x * fCos - vCentered.y * fSin,
        vCentered.x * fSin + vCentered.y * fCos);
    float2 vCard = vRotated /
        max(g_vPresentationOverlayScale, float2(0.00001f, 0.00001f)) + 0.5f;
    bool bInside = all(vCard >= float2(0.f, 0.f)) &&
        all(vCard <= float2(1.f, 1.f));
    if (!bInside)
    {
        Out.vBackBuffer = float4(vSource, 1.f);
        return Out;
    }
    float2 vOverlayUv = vCard +
        g_vPresentationOverlayUvDriftPerSecond * g_fPresentationTime;
    float4 vOverlay = Sample_PresentationOverlay(vOverlayUv);
    float fCoverage = saturate(
        Select_PresentationOverlayCoverage(vOverlay));
    float fAlpha = saturate(fCoverage *
        g_fPresentationOverlayAlpha * g_vPresentationOverlayTint.a);
    float3 vRadiance = Sanitize_HDR(
        vOverlay.rgb * g_vPresentationOverlayTint.rgb);
    Out.vBackBuffer = float4(Sanitize_HDR(
        lerp(vSource, vRadiance, fAlpha)), 1.f);
    return Out;
}

/* Hable(Uncharted 2) 필름 커브. 씬 컬러는 선형이고 1을 넘을 수 있다. */
float3 Tonemap_Hable(float3 vColor)
{
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;

    return ((vColor * (A * vColor + C * B) + D * E) /
            (vColor * (A * vColor + B) + D * F)) - E / F;
}

float3 Resolve_FinalLDR(float2 vTexcoord)
{
    float3 vScene = Sanitize_HDR(g_SceneHDRTexture.Sample(
        PostProcessSampler, clamp(vTexcoord, 0.f, 1.f)).rgb);
    float3 vBloom = float3(0.f, 0.f, 0.f);
    if (0u != g_iBloomEnabled)
    {
        vBloom = Sanitize_HDR(g_BloomTexture.Sample(
            PostProcessSampler, clamp(vTexcoord, 0.f, 1.f)).rgb);
    }

    float3 vSceneWithBloom = vScene + vBloom * g_fBloomIntensity;
    float3 vWhiteScale = max(Tonemap_Hable(
        max(g_fToneMapWhitePoint, 1.f).xxx), 0.00001f);
    float3 vMapped = Tonemap_Hable(vSceneWithBloom *
        max(g_fToneMapExposure, 0.01f)) / vWhiteScale;
    return pow(saturate(vMapped),
        1.f / max(g_fToneMapGamma, 1.f));
}

float Final_Luminance(float3 vColor)
{
    return dot(vColor, float3(0.299f, 0.587f, 0.114f));
}

float3 Resolve_FinalFXAA(float2 vTexcoord)
{
    float3 vCenter = float3(0.f, 0.f, 0.f);
    vCenter = Resolve_FinalLDR(vTexcoord);
    if (0u == g_iFXAAEnabled || g_fFXAASubpixel <= 0.f)
        return vCenter;

    float2 vTexel = max(g_vInverseSceneSize, 0.000001f);
    float3 vNW = Resolve_FinalLDR(vTexcoord + vTexel * float2(-1.f, -1.f));
    float3 vNE = Resolve_FinalLDR(vTexcoord + vTexel * float2( 1.f, -1.f));
    float3 vSW = Resolve_FinalLDR(vTexcoord + vTexel * float2(-1.f,  1.f));
    float3 vSE = Resolve_FinalLDR(vTexcoord + vTexel * float2( 1.f,  1.f));

    float fLumaM = Final_Luminance(vCenter);
    float fLumaNW = Final_Luminance(vNW);
    float fLumaNE = Final_Luminance(vNE);
    float fLumaSW = Final_Luminance(vSW);
    float fLumaSE = Final_Luminance(vSE);
    float fLumaMin = min(fLumaM,
        min(min(fLumaNW, fLumaNE), min(fLumaSW, fLumaSE)));
    float fLumaMax = max(fLumaM,
        max(max(fLumaNW, fLumaNE), max(fLumaSW, fLumaSE)));
    float fLumaRange = fLumaMax - fLumaMin;
    float fRequiredRange = max(g_fFXAAEdgeThresholdMin,
        fLumaMax * g_fFXAAEdgeThreshold);
    if (fLumaRange < fRequiredRange)
        return vCenter;

    float2 vDirection;
    vDirection.x = -((fLumaNW + fLumaNE) - (fLumaSW + fLumaSE));
    vDirection.y =  ((fLumaNW + fLumaSW) - (fLumaNE + fLumaSE));
    float fDirectionReduce = max(
        (fLumaNW + fLumaNE + fLumaSW + fLumaSE) * 0.03125f,
        0.0078125f);
    float fReciprocalDirection = 1.f /
        (min(abs(vDirection.x), abs(vDirection.y)) + fDirectionReduce);
    vDirection = clamp(vDirection * fReciprocalDirection,
        -8.f, 8.f) * vTexel;

    float3 vBlendA = 0.5f * (
        Resolve_FinalLDR(vTexcoord + vDirection * (-1.f / 6.f)) +
        Resolve_FinalLDR(vTexcoord + vDirection * ( 1.f / 6.f)));
    float3 vBlendB = vBlendA * 0.5f + 0.25f * (
        Resolve_FinalLDR(vTexcoord + vDirection * -0.5f) +
        Resolve_FinalLDR(vTexcoord + vDirection *  0.5f));
    float fLumaB = Final_Luminance(vBlendB);
    float3 vEdgeColor =
        (fLumaB < fLumaMin || fLumaB > fLumaMax) ? vBlendA : vBlendB;
    return lerp(vCenter, vEdgeColor, saturate(g_fFXAASubpixel));
}

/* SceneHDR을 백버퍼로 옮기는 유일한 지점. 톤매핑, 감마, FXAA는 UI 전에 여기서 처리한다. */
PS_OUT_BACKBUFFER PS_MAIN_FINAL(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    if (g_MaterialDebugView != 0u)
    {
        const int3 pixel = int3(int2(In.vPosition.xy), 0);
        const float marker = g_DepthTexture.Load(pixel).w;
        float3 color = 0.f;
        if (marker == 7.f)
        {
            if (g_MaterialDebugView == 1u)
                color = pow(saturate(g_CharacterSurfaceTexture.Load(pixel).rgb), 1.f / 2.2f);
            else if (g_MaterialDebugView == 2u)
                color = SourceStoneUnit(g_CharacterGeometryTexture.Load(pixel).xyz) * 0.5f + 0.5f;
            else if (g_MaterialDebugView == 3u)
            {
                // This family's additive target holds full source direct light,
                // including diffuse. It is not a separated specular-only pass.
                const float3 direct = max(g_SpecularTexture.Load(pixel).rgb, 0.f);
                color = pow(direct / (1.f + direct), 1.f / 2.2f);
            }
            // No source reflection texture/ORM exists in this permutation.
            Out.vBackBuffer = float4(color, 1.f);
            return Out;
        }
        if (marker == 1.f || marker == 2.f || marker == 3.f || marker == 4.f)
        {
            if (g_MaterialDebugView == 2u)
                color = g_NormalTexture.Load(pixel).rgb;
            else if (g_MaterialDebugView >= 5u)
            {
                // Legacy marker 2 has specular power/mask in these lanes.
                // Do not display those values as a recovered PBR diagnostic.
                if (marker == 3.f)
                {
                    if (g_MaterialDebugView == 5u)
                        color = g_NormalTexture.Load(pixel).aaa;
                    else if (g_MaterialDebugView == 6u)
                        color = g_MaterialSpecularTexture.Load(pixel).aaa;
                    else if (g_MaterialDebugView == 7u)
                        color = g_DepthTexture.Load(pixel).zzz;
                }
            }
            else
            {
                color = g_MaterialDebugView == 3u ?
                    g_SpecularTexture.Load(pixel).rgb : g_DiffuseTexture.Load(pixel).rgb;
                if (marker == 4.f && g_MaterialDebugView != 3u)
                    color *= g_MaterialSpecularTexture.Load(pixel).a;
                // Source contributions bypass scene exposure, Bloom and FXAA.
                // Specular can be HDR; compress only this diagnostic display.
                if (g_MaterialDebugView == 3u)
                    color = max(color, 0.f) / (1.f + max(color, 0.f));
                color = pow(saturate(color), 1.f / 2.2f);
            }
        }
        Out.vBackBuffer = float4(color, 1.f);
        return Out;
    }
    Out.vBackBuffer = float4(Resolve_FinalFXAA(In.vTexcoord), 1.f);

    return Out;
}


technique11 DefaultTechnique
{
    pass Debug
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();        
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }   

    pass Directional
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
    }

    pass Point
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_POINT();
    }

    pass Combined
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }

    /* DEFERRED::FINAL == 4. 반드시 Combined 다음 순서를 유지할 것. */
    pass Final
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FINAL();
    }

    /* FINAL stays at index 4; bloom passes are invoked before it by CRenderer. */
    pass BloomExtract
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_EXTRACT();
    }

    pass BloomBlurHorizontal
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_BLUR_H();
    }

    pass BloomBlurVertical
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_BLUR_V();
    }

    pass SceneResolve
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SCENE_RESOLVE();
    }

    pass PresentationRGBNoise
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_RGB_NOISE();
    }

    pass PresentationZoomBlur
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ZOOM_BLUR();
    }

    pass PresentationFilmNoise
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FILM_NOISE();
    }

    pass SSAORaw
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SSAO_RAW();
    }

    pass SSAOBilateralBlur
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SSAO_BLUR();
    }

    pass PresentationTexturedOverlay
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_TEXTURED_OVERLAY();
    }

    pass PresentationChromaticAberration
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_CHROMATIC_ABERRATION();
    }

    pass Spot
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SPOT();
    }

}



