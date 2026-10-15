#ifndef SOURCE_CHARACTER_PROGRAM_GROUP
#define SOURCE_CHARACTER_PROGRAM_GROUP 0
#endif

#include "Engine_Shader_Defines.hlsli"
#include "Shader_SourceStoneSurface.hlsli"
#include "Shader_SourceFoliageSurface.hlsli"

// Engine 소유 deferred/light/postprocess FX 정본이다. Client의 동명 HLSL은 SDK 배포 복사본이다.
// 표면 PS가 기록한 G-buffer를 읽고 조명 RT를 합산한 뒤 HDR→표시색으로 변환한다.
// 아래 technique의 pass 번호는 CRenderer와 CLight의 계약이므로 함수 정리와 별개로 순서를 보존한다.
float4x4    g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4    g_CameraViewMatrix, g_CameraProjMatrix;
float4x4    g_ViewMatrixInverse, g_ProjMatrixInverse;
float4x4    g_LightViewMatrix, g_LightProjMatrix;
texture2D   g_Texture;
texture2D   g_DiffuseTexture, g_ShadeTexture;
texture2D   g_DepthTexture;
uint g_iPortraitCapture = 0u;
texture2D   g_SpecularTexture;
texture2D   g_MaterialSpecularTexture;
texture2D   g_GeometricNormalTexture;
Texture2D g_CharacterSurfaceTexture;
Texture2D g_CharacterGeometryTexture;
float4x4 g_SourceCharacterViewMatrix, g_SourceCharacterProjMatrix;
uint g_ApplyStaticShadow = 0u;
uint g_LightReceiver = 0u; // 수신 범위: ALL=0, SOURCE_CHARACTER=1, UNBAKED=2.
#define SOURCE_CHARACTER_LIGHT_PASS
#include "Shader_SourceCharacterMaterial.hlsli"

uint g_MaterialDebugView = 0;
float4 g_MapPBRContributionScale = float4(1.f, 1.f, 1.f, 1.f);
// 선택적 project cube-diffuse 입력이다. Client MapMaterialSurface의 native g_SourcePBRPackedSH와 별도다.
// CRenderer::Render_Combined는 sourcePBRIndirect가 켜진 경우 이 intensity를 0으로 두어 두 경로를 겹치지 않는다.
float4 g_CubeDiffuseSH[7];
float g_CubeDiffuseIntensity = 0.f;
float4 g_CubeDiffuseColor = float4(1.f, 1.f, 1.f, 0.f);
float4 g_CubeDiffuseRotation = float4(0.f, 1.f, 1.f, 0.f);
texture2D   g_EmissiveTexture;
texture2D   g_LightDepthTexture;
texture2D   g_StaticLightDepthTexture;
texture2D   g_SceneHDRTexture;
texture2D   g_PostProcessTexture;
texture2D   g_PresentationOverlayTexture;
texture2D   g_BloomTexture;
texture2D   g_SceneBloomTexture;
texture2D   g_PostBloomTexture;
texture2D   g_DistortionTexture;
texture2D   g_SSAOTexture;

float2      g_vSSAOTexelSize;
uint        g_iSSAOEnabled = 0u;
uint        g_iSSAOSampleCount = 12u;
uint        g_iHorizonAOEnabled = 0u;
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
uint        g_iShadowPCFFilterRadius = 1u;
float       g_fDynamicBakedShadowStrength = 0.f;
uint        g_iDynamicBakedShadowEnabled = 0u;
float2      g_vBloomTexelSize;
float2      g_vInverseSceneSize;
uint        g_iBloomEnabled;
float       g_fBloomThreshold;
float       g_fBloomSoftKnee;
float       g_fBloomScatter;
float       g_fToneMapExposure;
float       g_fToneMapWhitePoint;
float       g_fToneMapGamma;
float4      g_vSceneBloomTint = float4(1.f, 1.f, 1.f, 1.f);
float       g_fSceneDesaturation = 0.f;
// 색각 접근성 필터: 0=끔, 1=protan, 2=deutan, 3=tritan. 보정 강도는 0..1이다.
int         g_iColorFilterType = 0;
float       g_fColorFilterStrength = 0.f;
int         g_iSourcePostProcessEnabled = 0;
float4      g_vSourceToneCurve = float4(.22f, 1.0275f, .255447f, 1.f);
float       g_fSourceToneToe = 1.f;
Texture2D   g_SourceGradingLut;
Texture2D   g_SourceLutInputs[4];
float4      g_vSourceLutWeights;
float       g_fSourceNeutralLutWeight = 1.f;
float4      g_vSourceGradingParameters[7];
SamplerState SourceGradingSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};
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

/* 후처리 빛이 화면 한쪽 경계를 넘어 반대쪽으로 wrap되지 않게 한다. */
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

/* 높이 안개. Combine이 이미 depth target을 읽으므로 화면 공간의 한 항으로 지형·건물·캐릭터를 함께 처리한다. */
#include "Shader_SceneHeightFog.hlsli"


vector      g_vLightDir;
vector      g_vLightPos;
float       g_fLightRange;
float       g_fLightFalloffExponent;
float       g_fSpotInnerCos = 1.f;
float       g_fSpotOuterCos = 1.f;
vector      g_vLightDiffuse;
vector      g_vLightAmbient;
float4      g_vSourceCharacterAmbient = 0.f;
vector      g_vLightSpecular;

// 한 광원의 방향/위치·RGB·감쇠·수신 정책을 담은 GPU 입력이다.
// 기존 단일 draw는 LegacyLightInput을, instanced draw는 offset+instanceId의 행을 같은 함수에 넘긴다.
struct DEFERRED_LIGHT_INPUT
{
    float4 direction;
    float4 position;
    float4 diffuse;
    float4 ambient;
    float4 specular;
    float4 attenuation; // 범위, 감쇠 지수, spot 내부/외부 각도의 cosine.
    uint4 flags; // 수신 범위, static-shadow 채널, 방향광 shadow 사용, source local 경계 사용.
    float4 sourceCharacterAmbient;
};

cbuffer DeferredLightInstances
{
    DEFERRED_LIGHT_INPUT g_LightInstances[400];
};
uint g_LightInstanceOffset = 0u;

// 기존 개별 uniform을 공통 광원 행으로 모은다. source local bounds flag는 이 경로에서 0이다.
DEFERRED_LIGHT_INPUT LegacyLightInput()
{
    DEFERRED_LIGHT_INPUT light;
    light.direction = g_vLightDir;
    light.position = g_vLightPos;
    light.diffuse = g_vLightDiffuse;
    light.ambient = g_vLightAmbient;
    light.sourceCharacterAmbient = g_vSourceCharacterAmbient;
    light.specular = g_vLightSpecular;
    light.attenuation = float4(g_fLightRange, g_fLightFalloffExponent,
        g_fSpotInnerCos, g_fSpotOuterCos);
    light.flags = uint4(g_LightReceiver, g_ApplyStaticShadow, g_iApplyDirectionalShadow, 0u);
    return light;
}

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

// 전면 quad 정점을 행벡터로 World→View→Projection 순서로 옮긴다.
// 이 draw용 행렬과 장면의 g_CameraView/Proj, source-character View/Proj를 혼동하지 않는다.
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
    float4 vBloomContribution : SV_TARGET2;
};

struct PS_OUT_LIGHT
{
    float4 vShade : SV_TARGET0;
    float4 vSpecular : SV_TARGET1;
};

// SSAO가 꺼져 있으면 1을 반환해 기존 빛을 유지한다. texture AO와 재질 ORM의 AO는 서로 다른 입력이다.
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

// world 위치에 normal bias를 더한 뒤 광원 clip/UV로 투영하여 저장 깊이와 비교한다.
// PCF는 반경 0/1/2의 1/9/25개 비교를 평균하며 그림자 맵 밖·광원 뒤는 1로 남긴다.
// BRDF의 미세 표면 masking/visibility와 장면 shadow map의 가림을 구분한다.
float Resolve_DirectionalShadow(float4 vWorldPos, float3 vNormal, DEFERRED_LIGHT_INPUT light)
{
    float fShadow = 1.f;
    if (0u != light.flags.z)
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
                const int iFilterRadius = (int)min(g_iShadowPCFFilterRadius, 2u);
                [loop]
                for (int iY = -iFilterRadius; iY <= iFilterRadius; ++iY)
                {
                    [loop]
                    for (int iX = -iFilterRadius; iX <= iFilterRadius; ++iX)
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

                const int iFilterWidth = 2 * iFilterRadius + 1;
                // 기본 반경에서는 기존 3×3 가중치 정규화를 유지한다.
                const float fPCF = iFilterRadius == 1 ? fLitSamples / 9.f :
                    fLitSamples / (float)(iFilterWidth * iFilterWidth);
                fShadow = lerp(
                    1.f, fPCF, saturate(g_fShadowStrength));
            }
        }
    }
    return fShadow;
}

/* 픽셀 셰이더 */
/* 전달받은 픽셀의 정보를 바탕으로하여 픽셀의 색을 결정한다 */

// 선택한 RT를 그대로 보여주는 진단 pass다. 아래 실제 재질 조명이나 tone mapping을 거치지 않는다.
PS_OUT_BACKBUFFER PS_MAIN_DEBUG(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    
    Out.vBackBuffer = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    return Out;    
}

// marker1만 RGB 재질 반사값을 사용하고 나머지는 기존 scalar mask를 세 채널에 복제한다.
// 앞 물체가 다른 marker를 썼을 때 뒤 바닥의 RT5 잔여값을 읽지 않는 경계다.
float3 Load_MaterialSpecular(PS_IN input, float legacyMask)
{
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    // RT5에 바닥 값이 남아 있어도 더 가까운 legacy 물체는 depth.w=0을 기록한다.
    return g_DepthTexture.Load(pixel).w == 1.f ?
        g_MaterialSpecularTexture.Load(pixel).rgb : legacyMask.xxx;
}

// 복원된 바닥 재질은 RGB 반사값을 소유한다. 맵 광원은 입사 radiance를
// diffuse로 전달하고 별도 legacy Phong 조절값은 0으로 둔다.
// marker 0/2는 저작값 0을 포함해 그 legacy 조절값을 그대로 사용한다.
float4 Resolve_MaterialSpecularLight(PS_IN input, DEFERRED_LIGHT_INPUT light)
{
    return g_DepthTexture.Load(int3(int2(input.vPosition.xy), 0)).w == 1.f ?
        float4(light.diffuse.rgb, 0.f) : light.specular;
}

// PickPos.W 가수의 11bit×2를 octahedral 법선으로 복원한다. marker가 맞는 호출자만 이 의미로 읽어야 한다.
float3 Decode_MapPBRGeometricNormal(int3 pixel)
{
    // Load는 RGBA32_FLOAT의 정확한 가수를 유지한다. 이 PickPos.W의
    // packed payload는 bilinear sampling으로 보간하면 안 된다.
    const uint packed = asuint(g_GeometricNormalTexture.Load(pixel).w);
    const float2 oct = float2(packed & 2047u, (packed >> 11u) & 2047u) /
        2047.f * 2.f - 1.f;
    float3 normal = float3(oct, 1.f - abs(oct.x) - abs(oct.y));
    const float fold = max(-normal.z, 0.f);
    normal.xy += float2(normal.x >= 0.f ? -fold : fold,
        normal.y >= 0.f ? -fold : fold);
    return normalize(normal);
}

// MapPBR 직접광의 계산식이다. N=normal map을 적용한 world 법선, Ng=기하 법선, V=표면→시점, L=표면→광원이다.
// BRDF는 입사 방향의 빛이 시점 방향으로 얼마나 반사되는지 연결한다. 여기서는 광색·감쇠·shadow를 곱하기 전의 두 기여를 반환한다.
// D(미세 법선 분포)·F(각도에 따른 반사)·visibility(미세 표면 가림)의 역할로 읽되 원본 보정식을 일반 교과서 식으로 바꾸지 않는다.
void Evaluate_MapSourcePBRDirect(float3 normal, float3 geometricNormal,
    float3 viewDirection, float3 lightDirection, float roughness, float metallic,
    float3 f0, out float3 diffuseWeight, out float3 specularContribution)
{
    // 실제 BG_PCSELECT15 DirectionalLight PS의 네 permutation에서 복원했다.
    // 원본의 기하 법선 Fresnel 보정, roughness Fresnel 항,
    // visibility 분모와 색 기반 peak 상한을 유지한다.
    const float3 n = normalize(normal);
    const float3 v = normalize(viewDirection);
    const float3 l = normalize(lightDirection);

    // H는 V와 L의 중간 방향이다. 서로 반대여서 합이 0인 경우를 위해 길이 제곱의 하한을 둔다.
    // NoV는 abs+작은 양수 보정을, NoL/NoH/VoH는 saturate를 사용하므로 네 내적의 경계 처리가 같지 않다.
    const float3 sum = v + l;
    const float3 h = sum * rsqrt(max(dot(sum, sum), 0.000000000001f));
    const float noH = saturate(dot(n, h));
    const float noV = min(abs(dot(n, v)) + 0.00001f, 1.f);
    const float noL = saturate(dot(n, l));
    const float voH = saturate(dot(v, h));

    // 기하 법선으로 VoH를 보정하고 (1-correctedVoH)^5를 만든다.
    // Schlick의 각도별 5제곱 형태를 쓰지만 아래 F는 F0+(1-F0)(1-VoH)^5와 동일하지 않다.
    // 현재 식의 roughness 및 F0.g 보정을 그대로 보아야 하며 F0는 MapMaterialSurface에서 만든 RGB 정면 반사색이다.
    const float geometricLight = min(dot(normalize(geometricNormal), l) + 1.f, 1.f);
    const float correctedVoH = min(voH + 1.f - geometricLight, 1.f);
    const float fresnelBase = 1.f - correctedVoH;
    const float fresnelSquare = fresnelBase * fresnelBase;
    const float fresnelPower = fresnelSquare * fresnelSquare * fresnelBase;
    const float3 fresnel = f0 * (1.f - fresnelPower) +
        fresnelPower * saturate(50.f * f0.g) * (max(f0, 1.f - roughness) - f0);

    // 입력 r에서 α=r², α²=r⁴를 만든다. 등방성 GGX/Trowbridge-Reitz 분포 D의 분자와 분모에 같은 r⁴를 쓴다.
    // D는 H 방향을 향하는 미세 법선의 분포를 나타내며 단순 반사율 0~1이 아니다.
    const float roughness2 = roughness * roughness;
    const float roughness4 = roughness2 * roughness2;
    const float distributionBase = noH * noH * (roughness4 - 1.f) + 1.f;
    const float distribution = roughness4 /
        max(3.14159265359f * distributionBase * distributionBase, 1e-30f);

    // NoL/NoV와 r²를 결합하는 현재 visibility 근사다. D에 0.5/denominator를 곱한 뒤 색 휘도 기반 peak 상한을 둔다.
    // 장면 물체 간 가림은 이후 shadow 입력이 담당하며 이 분모 자체가 SSAO나 shadow map은 아니다.
    const float visibilityDenominator =
        noL * (noV * (1.f - roughness2) + roughness2) +
        noV * (noL * (1.f - roughness2) + roughness2);
    const float sourcePeak = 0.5f * distribution / max(visibilityDenominator, 1e-30f);
    const float cappedPeak = min(sourcePeak,
        3.f / (dot(fresnel, float3(0.3f, 0.59f, 0.11f)) + 0.0001f));
    // 원본 diffuse에는 1/π 뒤 최종 광량의 π 배율이 적용된다.
    // 재질 albedo를 RT0에 남겨 기존 Combine이 한 번 곱하게 한다.
    // diffuse는 (1-metallic)×(1-F)×NoL이고 albedo는 아직 곱하지 않는다. Combine이 RT0 albedo를 한 번 곱한다.
    // specular는 F×제한된 peak×NoL×π로 완결해 별도 RT에 더한다. 이 π 배율을 다른 Lambert 정규화와 중복 적용하지 않는다.
    diffuseWeight = (1.f - metallic) * (1.f - fresnel) * noL;
    specularContribution = fresnel * cappedPeak * noL * 3.14159265359f;
}

// marker3의 normal.a=r, material.rgb=F0, material.a=metallic, depth.z=재질 AO를 읽는다.
// 직접광의 diffuse/specular를 각각 RT0/RT1에 누적하며, component lightmap이 있으면 중복 project ambient를 제외한다.
PS_OUT_LIGHT Resolve_MapPBRLight(PS_IN input, float3 worldPosition,
    float3 lightDirection, float attenuation, float directShadow, DEFERRED_LIGHT_INPUT light)
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
    // 바인딩된 component lightmap은 이미 해당 표면의 간접 irradiance를 제공한다.
    // 저작 scene 값은 유지하되 이 PBR pixel에는 중복 project ambient를
    // 제외한다. lightmap 없는 PBR은 그 명시적인 ambient 근사를 유지한다.
    const bool hasBakedLighting = (asuint(g_GeometricNormalTexture.Load(pixel).w) &
        0x00400000u) != 0u;
    const float ao = Resolve_AmbientOcclusion(input.vTexcoord) * materialAO;
    const float3 ambient = light.ambient.rgb * g_vMtrlAmbient.rgb * ao *
        (1.f - saturate(material.a)) * (hasBakedLighting ? 0.f : 1.f);
    output.vShade = float4(light.diffuse.rgb * attenuation *
        (diffuseWeight * directShadow * g_MapPBRContributionScale.x + ambient), 0.f);
    // native PBR은 두 BRDF lobe에 같은 입사 광색을 사용한다.
    // legacy Phong specular 조절값은 복원한 PBR의 빛을 끄는 용도가 아니다.
    output.vSpecular = float4(light.diffuse.rgb * specularContribution *
        attenuation * directShadow * g_MapPBRContributionScale.y, 0.f);
    return output;
}

// 복원된 바닥 MIC의 지수 30/100은 Phong이 아닌 native Blinn 지수다.
// marker1 payload를 HDR scale용 RT5 ABI와 구분해 유지한다.
// marker1 복원 바닥은 L+V의 half vector와 normal 내적에 Blinn 지수를 적용한다. legacy 반사벡터 Phong과 입력이 다르다.
float Evaluate_RecoveredFloorSpecularLobe(float3 normal, float3 viewDirection,
    float3 lightDirection, float specularPower)
{
    const float3 sum = normalize(lightDirection) + normalize(viewDirection);
    const float3 halfVector = sum * rsqrt(max(dot(sum, sum), 1e-12f));
    const float ndoth = abs(dot(normalize(normal), halfVector));
    return ndoth < 0.000001f ? 0.f : min(pow(ndoth, specularPower), 1.f);
}

// non-PBR family별 Blinn/RGB cap/rim 정책을 선택한다. material.a는 이 경로에서 metallic이 아니라 HDR albedo 복원 scale이다.
PS_OUT_LIGHT Resolve_MapSourceSpecularLight(PS_IN input, float3 worldPosition,
    float3 lightDirection, float attenuation, float directShadow, DEFERRED_LIGHT_INPUT light)
{
    PS_OUT_LIGHT output;
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    const float3 n = normalize(g_NormalTexture.Load(pixel).xyz * 2.f - 1.f);
    const float3 l = normalize(lightDirection);
    const float3 v = normalize(g_vCamPosition.xyz - worldPosition);
    const float3 sum = l + v;
    const float3 h = sum * rsqrt(max(dot(sum, sum), 1e-12f));
    const bool sourceLandscape = g_DepthTexture.Load(pixel).w == 14.f;
    const float ndoth = sourceLandscape ? saturate(dot(n, h)) : abs(dot(n, h));
    const float specularPower = g_DepthTexture.Load(pixel).z;
    const float lobe = ndoth < 0.000001f ? 0.f : min(pow(ndoth, specularPower), 1.f);
    const float4 material = g_MaterialSpecularTexture.Load(pixel);
    const bool hasBakedLighting = (asuint(g_GeometricNormalTexture.Load(pixel).w) &
        0x00400000u) != 0u;
    const float3 ambient = hasBakedLighting ? 0.f :
        light.ambient.rgb * g_vMtrlAmbient.rgb * Resolve_AmbientOcclusion(input.vTexcoord);
    // non-PBR에서 비어 있는 RT5 alpha는 albedo HDR 정규화 배율을 전달한다.
    // 원본 diffuse는 1을 넘을 수 있지만 제품 albedo target은 UNORM8이다.
    output.vShade = float4(light.diffuse.rgb * attenuation * material.a *
        (saturate(dot(n, l)) * directShadow + ambient), 0.f);
    // native Blinn lobe는 shadow 곱 뒤 RGB를 2로 제한하고
    // diffuse와 같은 입사 광색을 쓴다. 별도의 NoL 항은 없다.
    float3 specular = material.rgb * lobe * directShadow;
    if (g_DepthTexture.Load(pixel).w == 8.f)
    {
        const uint flags = (uint)round(g_CharacterSurfaceTexture.Load(pixel).w);
        if ((flags & 256u) != 0u)
            specular = saturate(specular);
        else if ((flags & 512u) != 0u)
            specular = clamp(specular, 0.f, 2.f);
        // native BG rim은 시점에 따라 달라지며 기하 표면의 뒤에서 오는 빛만 받는다.
        // normal map으로 바꾼 법선의 반사 lobe와는 독립된 항이다.
        specular += g_CharacterSurfaceTexture.Load(pixel).rgb * directShadow *
            saturate(-dot(Decode_MapPBRGeometricNormal(pixel), l));
    }
    else if (g_DepthTexture.Load(pixel).w == 11.f || sourceLandscape)
    { /* native ice와 Landscape는 RGB specular를 이 상한으로 제한하지 않는다. */ }
    else if (g_DepthTexture.Load(pixel).w == 13.f)
        specular = clamp(material.rgb * lobe, 0.f, 2.f) * directShadow;
    else
    {
        specular = clamp(specular, 0.f, 2.f);
        if (g_DepthTexture.Load(pixel).w == 12.f)
            specular += g_CharacterSurfaceTexture.Load(pixel).rgb *
                saturate(-dot(Decode_MapPBRGeometricNormal(pixel), l));
    }
    output.vSpecular = float4(light.diffuse.rgb * attenuation * specular, 0.f);
    return output;
}

// 잎/풀 전용 transmission·직접광 helper를 사용한다. 같은 G-buffer 채널도 marker9/10의 의미로 읽는다.
PS_OUT_LIGHT Resolve_MapSourceFoliageLight(PS_IN input, float3 worldPosition,
    float3 lightDirection, float attenuation, float directShadow, DEFERRED_LIGHT_INPUT light)
{
    PS_OUT_LIGHT output = (PS_OUT_LIGHT)0;
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    const float4 depth = g_DepthTexture.Load(pixel);
    const bool grass = depth.w == 10.f;
    const float3 normal = SourceFoliageUnit(g_NormalTexture.Load(pixel).xyz * 2.f - 1.f);
    const float3 unitLight = SourceFoliageUnit(lightDirection);
    const float3 transmission = g_CharacterSurfaceTexture.Load(pixel).rgb;
    const float4 material = g_MaterialSpecularTexture.Load(pixel);
    const float3 weight = SourceFoliageDirectWeight(dot(normal, unitLight), transmission, grass);
    output.vShade = float4(light.diffuse.rgb * attenuation * material.a * weight * directShadow, 0.f);
    output.vSpecular = float4(light.diffuse.rgb * attenuation * SourceFoliageDirectSpecular(
        normal, g_vCamPosition.xyz - worldPosition, unitLight, material.rgb, depth.z, directShadow.xxx, grass), 0.f);
    return output;
}

// stone의 base normal과 길이를 보존한 mixed normal을 전용 helper에 넘긴다.
// 이미 albedo가 들어간 radiance는 Specular RT로 전달해 Combine의 albedo 곱을 피한다.
PS_OUT_LIGHT Resolve_MapSourceStoneLight(PS_IN input, float3 worldPosition,
    float3 lightDirection, float attenuation, float directShadow, DEFERRED_LIGHT_INPUT light,
    bool applySceneAmbient)
{
    PS_OUT_LIGHT output = (PS_OUT_LIGHT)0;
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    const float3 baseNormal = SourceStoneUnit(g_NormalTexture.Load(pixel).xyz * 2.f - 1.f);
    const float3 mixedNormal = g_CharacterGeometryTexture.Load(pixel).xyz;
    const float3 diffuse = g_CharacterSurfaceTexture.Load(pixel).rgb;
    const float3 specular = g_MaterialSpecularTexture.Load(pixel).rgb;
    const float3 radiance = EvaluateSourceStoneDirect(diffuse, specular,
        baseNormal, mixedNormal, g_vCamPosition.xyz - worldPosition,
        lightDirection, g_DepthTexture.Load(pixel).z, light.diffuse.rgb,
        directShadow.xxx, float4(0.f, 0.f, 0.f, 1.f));
    // 정적 stone은 이미 RT4로 간접광을 받는다. lightmap 없는 Deploy stone은
    // 기존 scene ambient adapter를 한 번 소비하며, 이 값은 scene 방향광에서 온다.
    // 그 ambient는 직접광의 가시성과 독립적이다.
    // 이 adapter를 UE3의 동적 LightEnvironment 복원으로 해석하지 않는다.
    const bool hasBakedLighting = (asuint(g_GeometricNormalTexture.Load(pixel).w) &
        0x00400000u) != 0u;
    const float3 ambient = applySceneAmbient && !hasBakedLighting ?
        diffuse * light.diffuse.rgb * light.ambient.rgb * g_vMtrlAmbient.rgb *
        Resolve_AmbientOcclusion(input.vTexcoord) : 0.f;
    // 두 항 모두 HDR albedo를 포함하므로 UNORM RT0를 다시 곱하지 않는다.
    output.vSpecular = float4(radiance * attenuation + ambient, 0.f);
    return output;
}

// marker5와 정확한 material row가 일치하는 pixel만 native 입력으로 복원한다.
// T/B/N·UV·vertex 채널·시점·광원을 조합하고 native 출력은 색이 포함된 결과이므로 Specular RT에 넣는다.
// shader가 discarded로 돌려주면 0 기여이며 현재 미연결 native 장면 입력을 보완하는 ambient는 별도 adapter 항이다.
PS_OUT_LIGHT Resolve_SourceCharacterLight(PS_IN input, float3 lightDirection,
    float attenuation, float directShadow, DEFERRED_LIGHT_INPUT light)
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
    float4 vertexChannels = g_MaterialSpecularTexture.Load(pixel);
    if (IsSourceMovieStatic(g_SourceCharacterProgram) && vertexChannels.z < 0.f)
        vertexChannels.z = -1.f-vertexChannels.z;
    SOURCE_CHARACTER_NATIVE_INPUT nativeInput = MakeSourceCharacterInput(surface.xy,
        vertexChannels, position, tangent, binormal, normal,
        g_vCamPosition.xyz, clipPosition,
        mul(g_SourceCharacterViewMatrix,g_SourceCharacterProjMatrix),
        lightDirection, light.diffuse.rgb, directShadow, abs(geometry.w) < 1.5f);
    SOURCE_CHARACTER_NATIVE_OUTPUT native = EvaluateSourceCharacterLight(nativeInput);
    if (native.discarded)
        return output;
    // native 직접광 출력은 이미 albedo와 두 lighting lobe를 포함한다.
    // 기존 Combine은 Specular를 더할 때 albedo를 다시 곱하지 않는다.
    // hair의 native pass는 자체 투영 PCF 계약을 쓴다. 그 엔진 소유 투영 입력이
    // 연결되기 전에는 현재 renderer의 shadow를 한 번 적용한다.
    const float shadowAdapter = (g_SourceCharacterProgram == 7u ||
        g_SourceCharacterProgram == 18u || g_SourceCharacterProgram == 20u ||
        g_SourceCharacterProgram == 99u) ? directShadow : 1.f;
    output.vSpecular = float4(native.targets[0].rgb * (attenuation * shadowAdapter), 0.f);
    // 엔진 소유 원본 SH/cube 값은 이 MIC의 저작값이 아니다.
    // scene이 명시한 ambient 근사를 복원된 직접광과 별개로 유지한다.
    const bool nativeMapMaterial = (g_SourceCharacterProgram >= 80u && g_SourceCharacterProgram <= 83u) || IsSourceStaticMapSL10();
    const bool nativeMapBaked = nativeMapMaterial && g_SourceMapMonsterBakedEnabled != 0u &&
        (IsSourceMovieStatic(g_SourceCharacterProgram) ? g_MaterialSpecularTexture.Load(pixel).z < 0.f :
         (IsSourceStaticMapSL10() ? g_MaterialSpecularTexture.Load(pixel).z : g_MaterialSpecularTexture.Load(pixel).w) > .5f);
    // 누락된 원본 SH/probe 입력은 scene에서 명시적인 값으로 보완할 수 있다.
    // 이 값은 제외된 방향광 RGB와 독립적이며 기본값은 0이다.
    // native IBL은 RT4에 남기고 복원한 MRT3 albedo는 Combine에서 한 번 곱한다.
    output.vShade = nativeMapBaked ? 0.f :
        (light.diffuse * light.ambient + (nativeMapMaterial ? 0.f : light.sourceCharacterAmbient)) * g_vMtrlAmbient *
        (attenuation * Resolve_AmbientOcclusion(input.vTexcoord));
    output.vShade.a = 0.f;
    return output;
}

// 원본 정적 광원은 RNM이 없는 움직이는 배경에도 빛을 준다.
// packed baked flag는 복원된 map family의 값이며 legacy pixel이나
// marker5 native row의 flag가 아니다. native row는 해당 재질 조명 pass에서 검사한다.
// 수신 범위와 baked flag로 이 광원을 이미 lightmap에 포함한 표면을 제외한다.
// marker5의 native row와 일반 map의 packed flag를 같은 bit 의미로 무조건 읽지 않는다.
bool Reject_LightReceiver(PS_IN input, DEFERRED_LIGHT_INPUT light)
{
    if (light.flags.x == 1u && g_SourceCharacterRow == 0u)
        return true;
    // native static map row는 character carrier를 공유한다. RNM이 있는 경우
    // 일반 map family와 마찬가지로 이미 bake된 원본 광원을 제외한다.
    if (light.flags.x != 1u && light.flags.x != 2u)
        return false;
    const int3 pixel = int3(int2(input.vPosition.xy), 0);
    const float marker = g_DepthTexture.Load(pixel).w;
    if (g_SourceCharacterRow != 0u)
        return marker == 5.f && ((g_SourceCharacterProgram >= 80u &&
            g_SourceCharacterProgram <= 83u) || IsSourceStaticMapSL10()) && g_SourceMapMonsterBakedEnabled != 0u &&
            (IsSourceMovieStatic(g_SourceCharacterProgram) ? g_MaterialSpecularTexture.Load(pixel).z < 0.f :
         (IsSourceStaticMapSL10() ? g_MaterialSpecularTexture.Load(pixel).z : g_MaterialSpecularTexture.Load(pixel).w) > .5f);
    const bool sourceMap = marker == 3.f || marker == 4.f ||
        (marker >= 7.f && marker <= 14.f);
    return sourceMap && (asuint(g_GeometricNormalTexture.Load(pixel).w) & 0x00400000u) != 0u;
}

// 방향광의 공통 진입이다. 먼저 수신/row를 거른 뒤 world 위치·shadow를 준비하고 marker별 조명식으로 분기한다.
// 입력 light.direction은 빛이 진행하는 방향이므로 표면→광원 L에는 부호를 반대로 넘긴다.
PS_OUT_LIGHT Resolve_DirectionalLight(PS_IN In, DEFERRED_LIGHT_INPUT light)
{
    if (Reject_LightReceiver(In, light))
        return (PS_OUT_LIGHT)0;
    // 각 source row는 자기 marker5 pixel만 소유한다. 다른 row는 world 위치 복원이나
    // 방향광 shadow map sampling 전에 제외한다.
    const float4 sourceDepth = g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0));
    if (g_SourceCharacterRow != 0u)
    {
        if (sourceDepth.w != 5.f || sourceDepth.z != float(g_SourceCharacterRow))
            discard;
    }
    else if (sourceDepth.w == 5.f)
        return (PS_OUT_LIGHT)0;
    PS_OUT_LIGHT Out;
    
    vector          vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    
    /* 0 ~ 1 => -1 ~ 1 */
    vector vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);    
    

    
    vector vReflect = reflect(light.direction, vNormal);
    
    
    vector      vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */

    // Depth.x는 NDC z, Depth.y×1000은 저장한 clip w다. UV의 Y를 뒤집어 NDC를 만들고 w를 복원한 뒤 역투영→역뷰 순서로 world를 얻는다.
    // 이 복원은 현재 writer의 perspective/depth packing 계약을 전제로 하며 임의 depth texture에 쓸 수 있는 공식은 아니다.
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
        normalize(light.direction) * -1.f, normalize(vNormal)));
    const uint shadowChannel = ((asuint(g_GeometricNormalTexture.Load(int3(int2(In.vPosition.xy), 0)).w) >> 23u) & 255u) - 127u;
    const float staticShadow = light.flags.y != 0u && shadowChannel == light.flags.y ?
        1.f - saturate(g_EmissiveTexture.Load(int3(int2(In.vPosition.xy), 0)).a) : 1.f;
    const float fDirectionalShadow = Resolve_DirectionalShadow(
        vWorldPos, vNormal.xyz, light) * staticShadow;
    if (g_SourceCharacterRow != 0u)
        return Resolve_SourceCharacterLight(In, -light.direction.xyz, 1.f, fDirectionalShadow, light);
    if (sourceDepth.w == 7.f)
        return Resolve_MapSourceStoneLight(In, vWorldPos.xyz, -light.direction.xyz, 1.f, fDirectionalShadow, light, true);
    if (sourceDepth.w == 9.f || sourceDepth.w == 10.f)
        return Resolve_MapSourceFoliageLight(In, vWorldPos.xyz, -light.direction.xyz, 1.f, fDirectionalShadow, light);
    if (sourceDepth.w == 4.f || sourceDepth.w == 8.f || (sourceDepth.w >= 11.f && sourceDepth.w <= 14.f))
        return Resolve_MapSourceSpecularLight(In, vWorldPos.xyz, -light.direction.xyz,
            1.f, fDirectionalShadow, light);
    if (g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0)).w == 3.f)
        return Resolve_MapPBRLight(In, vWorldPos.xyz, -light.direction.xyz,
            1.f, fDirectionalShadow, light);
    const float fAmbientOcclusion =
        Resolve_AmbientOcclusion(In.vTexcoord);
    Out.vShade = light.diffuse *
        (fDirectDiffuse * fDirectionalShadow +
        (light.ambient * g_vMtrlAmbient) * fAmbientOcclusion);
    
    vector vLook = vWorldPos - g_vCamPosition;
    
    const float specularPower = vDepthDesc.z > 0.f ? vDepthDesc.z : 50.f;
    float specularLobe = pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))),
        specularPower);
    if (sourceDepth.w == 1.f)
        specularLobe = Evaluate_RecoveredFloorSpecularLobe(vNormal.xyz,
            -vLook.xyz, -light.direction.xyz, specularPower);
    Out.vSpecular = Resolve_MaterialSpecularLight(In, light) * specularLobe *
        float4(Load_MaterialSpecular(In, vNormalDesc.a), 1.f) * fDirectionalShadow;
    
    return Out;
}

// 범위까지 선형으로 줄어드는 비율을 지정 지수로 거듭제곱한다. 물리적인 1/d² 감쇠식은 아니다.
// range의 유한성과 양수만 여기서 확인하며 falloff exponent의 계약은 CPU 입력 검증에 의존한다.
float Resolve_PointLightAttenuation(float fDistance, DEFERRED_LIGHT_INPUT light)
{
    float fAttenuation = 0.f;
    if (isfinite(light.attenuation.x) && light.attenuation.x > 0.f)
    {
        fAttenuation = saturate(
            (light.attenuation.x - fDistance) / light.attenuation.x);
        if (1.f != light.attenuation.y)
            fAttenuation = pow(
                fAttenuation, light.attenuation.y);
    }
    return fAttenuation;
}

// point/spot 공통 경로다. 위치→광원 방향과 거리 감쇠를 만든 뒤 spot이면 원뿔 cosine 가중치를 제곱해 곱한다.
// 기여가 정확히 0이면 표면별 계산 전에 반환한다. local shadow는 선택된 static channel을 사용한다.
PS_OUT_LIGHT Resolve_LocalLight(PS_IN In, bool bSpot, DEFERRED_LIGHT_INPUT light)
{
    if (Reject_LightReceiver(In, light))
        return (PS_OUT_LIGHT)0;
    // 각 source row는 자기 marker5 pixel만 소유한다. 다른 row는 world 위치 복원이나
    // 방향광 shadow map sampling 전에 제외한다.
    const float4 sourceDepth = g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0));
    if (g_SourceCharacterRow != 0u)
    {
        if (sourceDepth.w != 5.f || sourceDepth.z != float(g_SourceCharacterRow))
            discard;
    }
    else if (sourceDepth.w == 5.f)
        return (PS_OUT_LIGHT)0;
    PS_OUT_LIGHT Out;
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    
    /* 0 ~ 1 => -1 ~ 1 */
    vector vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
    
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */

    // Depth.x는 NDC z, Depth.y×1000은 저장한 clip w다. UV의 Y를 뒤집어 NDC를 만들고 w를 복원한 뒤 역투영→역뷰 순서로 world를 얻는다.
    // 이 복원은 현재 writer의 perspective/depth packing 계약을 전제로 하며 임의 depth texture에 쓸 수 있는 공식은 아니다.
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
    
    
    vector vLightDir = vWorldPos - light.position;
    
    float fAtt = Resolve_PointLightAttenuation(length(vLightDir), light);
    if (bSpot)
    {
        // direction은 광원에서 조명하는 원뿔 안쪽으로 향한다.
        if (dot(vLightDir.xyz, vLightDir.xyz) <= 0.000000000001f)
        {
            Out.vShade = 0.f;
            Out.vSpecular = 0.f;
            return Out;
        }
        float fCosAngle = dot(normalize(vLightDir.xyz), normalize(light.direction.xyz));
        float fCone = saturate((fCosAngle - light.attenuation.w) /
            max(light.attenuation.z - light.attenuation.w, 0.0001f));
        fAtt *= fCone * fCone;
    }
    
    // 모든 local-light 출력에는 attenuation이 곱해진다. quad와 blend 순서는 유지하고
    // 기여가 정확히 0일 때만 material/texture 계산을 생략한다.
    if (fAtt == 0.f)
        return (PS_OUT_LIGHT)0;

    const uint shadowChannel = ((asuint(g_GeometricNormalTexture.Load(int3(int2(In.vPosition.xy), 0)).w) >> 23u) & 255u) - 127u;
    const float staticShadow = light.flags.y != 0u && shadowChannel == light.flags.y ?
        1.f - saturate(g_EmissiveTexture.Load(int3(int2(In.vPosition.xy), 0)).a) : 1.f;

    if (g_SourceCharacterRow != 0u)
        return Resolve_SourceCharacterLight(In, -vLightDir.xyz, fAtt, staticShadow, light);
    if (sourceDepth.w == 7.f)
        return Resolve_MapSourceStoneLight(In, vWorldPos.xyz, -vLightDir.xyz, fAtt, staticShadow, light, false);
    if (sourceDepth.w == 9.f || sourceDepth.w == 10.f)
        return Resolve_MapSourceFoliageLight(In, vWorldPos.xyz, -vLightDir.xyz, fAtt, staticShadow, light);
    if (sourceDepth.w == 4.f || sourceDepth.w == 8.f || (sourceDepth.w >= 11.f && sourceDepth.w <= 14.f))
        return Resolve_MapSourceSpecularLight(In, vWorldPos.xyz, -vLightDir.xyz, fAtt, staticShadow, light);
    if (g_DepthTexture.Load(int3(int2(In.vPosition.xy), 0)).w == 3.f)
        return Resolve_MapPBRLight(In, vWorldPos.xyz, -vLightDir.xyz, fAtt, staticShadow, light);
    float fAmbientOcclusion = Resolve_AmbientOcclusion(In.vTexcoord);
    Out.vShade = (light.diffuse * (saturate(dot(normalize(vLightDir) * -1.f, normalize(vNormal)))
    + (light.ambient * g_vMtrlAmbient) * fAmbientOcclusion)) * fAtt;
    
    
    vector vReflect = reflect(vLightDir, vNormal);
    
    
    vector vLook = vWorldPos - g_vCamPosition;
    
    const float specularPower = vDepthDesc.z > 0.f ? vDepthDesc.z : 50.f;
    float specularLobe = pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))),
        specularPower);
    if (sourceDepth.w == 1.f)
        specularLobe = Evaluate_RecoveredFloorSpecularLobe(vNormal.xyz,
            -vLook.xyz, -vLightDir.xyz, specularPower);
    Out.vSpecular = Resolve_MaterialSpecularLight(In, light) * specularLobe *
        float4(Load_MaterialSpecular(In, vNormalDesc.a), 1.f) * fAtt;
    
    return Out;
}



PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
    return Resolve_DirectionalLight(In, LegacyLightInput());
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
    return Resolve_LocalLight(In, false, LegacyLightInput());
}

PS_OUT_LIGHT PS_MAIN_SPOT(PS_IN In)
{
    return Resolve_LocalLight(In, true, LegacyLightInput());
}

// depth의 NDC 또는 정규화 view-depth가 끝값에 가까우면 AO의 배경으로 분류한다. 모든 게임 오브젝트의 존재 판정은 아니다.
bool Is_SSAOBackground(float4 vDepthDesc)
{
    return vDepthDesc.x >= 0.99999f || vDepthDesc.y >= 0.99999f;
}

// AO에 쓸 world normal을 얻는다. stone marker7은 별도 mixed normal을 쓰고 일반 normal의 퇴화 길이는 +Z로 대체한다.
float3 Decode_SSAONormal(float2 vTexcoord)
{
    // marker7은 최종 혼합 법선을 정규화하지 않은 채 공유 RT7에 기록한다.
    // RT1에는 직접 specular에 사용하는 base normal을 의도적으로 남긴다.
    if (g_DepthTexture.SampleLevel(PointSampler, vTexcoord, 0.f).w == 7.f)
        return SourceStoneUnit(g_CharacterGeometryTexture.SampleLevel(
            PointSampler, vTexcoord, 0.f).xyz);
    float3 vNormal = g_NormalTexture.Sample(
        PostProcessSampler, vTexcoord).xyz * 2.f - 1.f;
    float fLengthSquared = dot(vNormal, vNormal);
    return fLengthSquared > 0.000001f ?
        vNormal * rsqrt(fLengthSquared) : float3(0.f, 0.f, 1.f);
}

// world normal을 위치 이동이 없는 w=0 방향으로 camera view 공간에 옮긴다.
float3 Decode_SSAOViewNormal(float2 vTexcoord)
{
    float3 vWorldNormal = Decode_SSAONormal(vTexcoord);
    return normalize(mul(
        float4(vWorldNormal, 0.f), g_CameraViewMatrix).xyz);
}

// 화면 UV와 저장 깊이로 view 위치를 복원한다. AO 반경/거리 비교가 같은 view 공간에서 이루어져야 한다.
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

// pixel 위치를 고정된 의사난수 각도로 바꿔 AO 표본 방향을 흩는다. 프레임별 RNG 상태나 temporal history는 없다.
float SSAO_Hash(float2 vPosition)
{
    return frac(sin(dot(vPosition,
        float2(12.9898f, 78.233f))) * 43758.5453f);
}

// 화면 깊이에서 4/8/12개의 주변 표본을 복원하고 중심 법선의 앞쪽을 가리는 정도를 가중 평균한다.
// 화면 밖·배경·반경 밖 표본은 제외하고, 멀어질수록 AO=1로 되돌린다. 화면 밖 geometry는 알 수 없다.
PS_OUT_BACKBUFFER Evaluate_SSAORaw(PS_IN In, int iSampleCount)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
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
    for (int iSample = 0; iSample < iSampleCount; ++iSample)
    {
        float fSampleFraction = ((float)iSample + 0.5f) / (float)iSampleCount;
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

// 기존 AO 경로 안에서 horizon을 탐색한다. 방위각마다 새로 가려진 고도만
// 거리 감쇠와 함께 적분하며 temporal 누적이나 보이지 않는 geometry는 사용하지 않는다.
// horizon/tangent 원리에 따른 구현이며 NVIDIA HBAO+ 구현과 같다는 뜻은 아니다.
// UV를 실제 point depth texel 중심에 맞추고 유효 view 위치를 구한다.
// false여도 inout UV가 snap됐거나 out position이 계산된 상태일 수 있어 출력 보존 함수로 취급하지 않는다.
bool HorizonAOPosition(inout float2 uv, out float3 position)
{
    position = 0.f;
    if (!all(isfinite(uv)) || any(uv <= 0.f) || any(uv >= 1.f))
        return false;
    uint width, height; g_DepthTexture.GetDimensions(width, height);
    int2 pixel = int2(uv * float2(width, height));
    // 절반 해상도 AO를 포함해 point depth를 실제로 읽은 texel의 위치를 복원한다.
    uv = (float2(pixel) + .5f) / float2(width, height);
    float4 depth = g_DepthTexture.Load(int3(pixel, 0));
    if (!all(isfinite(depth)) || Is_SSAOBackground(depth) || depth.x < 0.f ||
        depth.y <= .000001f) return false;
    position = Reconstruct_SSAOViewPosition(uv, depth);
    return all(isfinite(position)) && position.z > .001f;
}

// 방향당 4개 거리 표본에서 새로 높아진 horizon만 누적한다. 입력/깊이 실패는 AO=1로 열어 둔다.
// 현재 표본의 실제 방향에서 tangent를 계산해 기울어진 평면의 자체 차폐를 줄이며 temporal 누적은 없다.
PS_OUT_BACKBUFFER Evaluate_HorizonAO(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    Out.vBackBuffer = 1.f;
    if (!isfinite(g_fSSAORadius) || g_fSSAORadius <= 0.f ||
        !isfinite(g_fSSAOBias) || g_fSSAOBias < 0.f || g_fSSAOBias >= g_fSSAORadius ||
        !isfinite(g_fSSAOIntensity) || g_fSSAOIntensity <= 0.f ||
        !isfinite(g_fSSAOPower) || g_fSSAOPower <= 0.f ||
        !isfinite(g_fSSAODistanceFade) || g_fSSAODistanceFade < g_fSSAORadius ||
        (g_iSSAOSampleCount != 4u && g_iSSAOSampleCount != 8u && g_iSSAOSampleCount != 12u) ||
        !all(isfinite(g_vSSAOTexelSize)) || any(g_vSSAOTexelSize <= 0.f)) return Out;
    float3 center;
    if (!HorizonAOPosition(In.vTexcoord, center))
        return Out;
    float3 normal = Decode_SSAOViewNormal(In.vTexcoord);
    float lengthSquared = dot(center, center);
    if (!all(isfinite(normal)) || !isfinite(lengthSquared) || lengthSquared < .000001f)
        return Out;
    float3 view = -center * rsqrt(lengthSquared);
    if (dot(normal, view) < 0.f)
        normal = -normal;
    float normalView = dot(normal, view);
    if (normalView < .0001f)
        return Out;
    float projectionY = abs(g_CameraProjMatrix[1][1]);
    if (!isfinite(projectionY) || projectionY < .0001f)
        return Out;
    float radiusPixels = clamp(g_fSSAORadius * projectionY * .5f /
        (center.z * g_vSSAOTexelSize.y), 1.f, 64.f);
    float rotation = SSAO_Hash(floor(In.vTexcoord / g_vSSAOTexelSize)) * 6.28318530718f;
    float occlusion = 0.f;
    [loop] for (uint directionIndex = 0u; directionIndex < g_iSSAOSampleCount; ++directionIndex)
    {
        float angle = rotation + (float)directionIndex * (6.28318530718f / (float)g_iSSAOSampleCount);
        float2 direction = float2(cos(angle), sin(angle));
        float horizon = 0.f;
        float directionOcclusion = 0.f;
        [unroll] for (uint stepIndex = 1u; stepIndex <= 4u; ++stepIndex)
        {
            float fraction = (float)stepIndex * .25f;
            float2 sampleUv = In.vTexcoord + direction * radiusPixels * fraction * g_vSSAOTexelSize;
            float3 samplePosition;
            if (!HorizonAOPosition(sampleUv, samplePosition))
                continue;
            float3 delta = samplePosition - center;
            float distanceSquared = dot(delta, delta);
            if (!isfinite(distanceSquared) || distanceSquared <= .000001f ||
                distanceSquared >= g_fSSAORadius * g_fSSAORadius) continue;
            // pixel snapping이 같은 방위각의 각 표본 방향을 바꿀 수 있다.
            // 실제 표본 광선에서 tangent를 계산해 기울어진 평면의 불필요한 차폐를 피한다.
            float3 slice = delta - view * dot(delta, view);
            float sliceLength = dot(slice, slice);
            if (!isfinite(sliceLength) || sliceLength < .000001f)
                continue;
            slice *= rsqrt(sliceLength);
            float normalSlice = dot(normal, slice);
            float tangentSin = -normalSlice * rsqrt(normalView * normalView + normalSlice * normalSlice);
            float sampleHorizon = max(0.f, dot(delta, view) * rsqrt(distanceSquared) -
                tangentSin - g_fSSAOBias / g_fSSAORadius);
            if (sampleHorizon > horizon)
            {
                float attenuation = saturate(1.f - distanceSquared / (g_fSSAORadius * g_fSSAORadius));
                directionOcclusion += (sampleHorizon - horizon) * attenuation;
                horizon = sampleHorizon;
            }
        }
        occlusion += directionOcclusion;
    }
    float ao = pow(saturate(1.f - occlusion / (float)g_iSSAOSampleCount * g_fSSAOIntensity), g_fSSAOPower);
    float fadeStart = max(g_fSSAODistanceFade * .65f, g_fSSAORadius);
    float fade = saturate((center.z - fadeStart) / max(g_fSSAODistanceFade - fadeStart, .001f));
    Out.vBackBuffer = isfinite(ao) ? lerp(ao, 1.f, fade) : 1.f;
    return Out;
}

// uniform 선택으로 펼친 커널의 표본 수를 정확히 4/8/12개로 고른다.
// 12개 경로는 기존 표본 수식과 누적 순서를 유지한다.
// 선택한 Horizon 경로를 우선하고, 일반 SSAO는 고정 표본 수의 커널을 호출한다.
PS_OUT_BACKBUFFER PS_MAIN_SSAO_RAW(PS_IN In)
{
    [branch] if (g_iHorizonAOEnabled != 0u)
        return Evaluate_HorizonAO(In);
    [branch] if (g_iSSAOSampleCount == 4u)
        return Evaluate_SSAORaw(In, 4);
    [branch] if (g_iSSAOSampleCount == 8u)
        return Evaluate_SSAORaw(In, 8);
    return Evaluate_SSAORaw(In, 12);
}

// 5×5 주변 AO를 공간 거리·view 깊이 차이·법선 유사도 세 가중치로 평균한다.
// 경계 넘어 번짐을 줄이는 bilateral blur이며 유효 가중치가 없으면 중심값을 유지한다.
PS_OUT_BACKBUFFER PS_MAIN_SSAO_BLUR(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
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

/* ceiling은 안개가 사라지는 world 높이이고 그 아래로 내려갈수록 지수 항으로 짙어진다.
   거리를 함께 반영해 가까운 지면은 읽히고 계곡 안쪽은 짙어지게 한다.
   emissive는 이 호출 뒤에 더하므로 발광이 이 안개 곱으로 사라지지 않는다. */
/* value noise로 구름 덩어리를 절차적으로 계산하므로 별도 texture를 저작·stream하거나 Area와 동기화하지 않는다. */
// 저장 깊이로 world 위치를 복원해 공용 fog의 (가산 RGB, 투과율 W)을 적용한다.
// 이 호출 뒤 Combine이 emissive를 더하므로 이 경로의 발광은 같은 fog 곱을 받지 않는다.
float3 Resolve_HeightFog(float3 vLitColor, float2 vTexcoord)
{
    if (0u == g_iHeightFogEnabled)
        return vLitColor;

    const vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, vTexcoord);
    /* 아직 기록되지 않은 depth texel은 빈 배경이므로 안개를 받는 표면으로 취급하지 않는다. */
    if (vDepthDesc.y <= 0.f)
        return vLitColor;
    // native black blocker는 안개를 끈다. row -1에는 character light 소유자가 없다.
    if (vDepthDesc.w == 5.f && vDepthDesc.z == -1.f)
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

    const float4 transfer = EvaluateSceneFog(vWorldPos.xyz, g_vCamPosition.xyz);
    return vLitColor * transfer.w + transfer.rgb;
}

// 전체 shadow depth가 static-only depth보다 앞선 표본만 움직이는 가림으로 센다.
// RNM에 이미 있는 정적 그림자를 다시 어둡게 하지 않기 위한 project 보정이며 맵 밖은 1을 유지한다.
float Resolve_DynamicBakedShadow(float3 position, float3 normal)
{
    if (g_iDynamicBakedShadowEnabled == 0u || g_fDynamicBakedShadowStrength <= 0.f)
        return 1.f;
    const float4 clip = mul(mul(float4(position + normalize(normal) *
        g_fShadowNormalBias, 1.f), g_LightViewMatrix), g_LightProjMatrix);
    if (clip.w <= 0.f)
        return 1.f;
    const float3 ndc = clip.xyz / clip.w;
    const float2 uv = ndc.xy * float2(.5f, -.5f) + .5f;
    if (any(uv < 0.f) || any(uv > 1.f) || ndc.z <= 0.f || ndc.z >= 1.f)
        return 1.f;
    const float receiver = ndc.z - max(g_fShadowDepthBias, 0.f);
    float occluded = 0.f;
    const int radius = (int)min(g_iShadowPCFFilterRadius, 2u);
    [loop] for (int y = -radius; y <= radius; ++y)
    {
        [loop] for (int x = -radius; x <= radius; ++x)
        {
            const float2 sampleUV = uv + float2(x, y) * g_vShadowTexelSize;
            const float complete = g_LightDepthTexture.SampleLevel(ShadowSampler, sampleUV, 0.f).r;
            const float fixedDepth = g_StaticLightDepthTexture.SampleLevel(ShadowSampler, sampleUV, 0.f).r;
            // 동일한 정적 깊이로 RNM을 다시 어둡게 하지 않는다. RNM에 포함되지 않은
            // 더 가까운 움직이는 caster는 추가 가림으로 반영할 수 있다.
            occluded += complete + 0.000001f < fixedDepth && receiver > complete ? 1.f : 0.f;
        }
    }
    const int width = 2 * radius + 1;
    const float filtered = radius == 1 ? occluded / 9.f : occluded / (float)(width * width);
    return 1.f - filtered * saturate(g_fDynamicBakedShadowStrength);
}

// project diffuse sky adapter다. 계수에는 Lambert E/π가 이미 포함되어 있다.
// native SH9 packing이나 동적 native hemisphere를 여기서 추론하지 않는다.
// 광원 개수·방향광 shadow와 독립적으로 Combine에서 한 번 적용한다.
// 여기의 SH 계수는 project Lambert diffuse E/π가 포함된 입력 계약이다.
// 상수/일차·xy/yz/z²/xz·x²-y²를 평가하고 색/강도를 곱한다. native MapSourcePBRSHColor처럼 끝에 π를 다시 곱하지 않는다.
float3 Evaluate_CubeDiffuseIrradiance(float3 worldNormal)
{
    const float3 n = worldNormal * rsqrt(max(dot(worldNormal, worldNormal), 1e-12f));
    const float a = g_CubeDiffuseRotation.x, b = g_CubeDiffuseRotation.y;
    const float4 q = float4(b * n.x + a * n.z, n.y, a * n.x - b * n.z, 1.f);
    const float4 products = q.yzzx * q.xyzz;
    const float3 firstOrder = float3(dot(g_CubeDiffuseSH[0], q),
        dot(g_CubeDiffuseSH[1], q), dot(g_CubeDiffuseSH[2], q));
    const float3 quadratic = float3(dot(g_CubeDiffuseSH[3], products),
        dot(g_CubeDiffuseSH[4], products), dot(g_CubeDiffuseSH[5], products));
    return max(firstOrder + quadratic + g_CubeDiffuseSH[6].rgb *
        (q.x * q.x - q.y * q.y), 0.f) * g_CubeDiffuseColor.rgb * g_CubeDiffuseIntensity;
}

// marker3만 받는 선택적 diffuse 근사다. albedo·비금속 비율·(1-F0)·재질 AO를 곱한다.
// 반사 방향 cube/BRDF 계산은 Client 표면 단계에 있으며 이 함수는 법선 방향 SH만 평가한다.
float3 Resolve_MapCubeDiffuse(int3 pixel)
{
    if (g_CubeDiffuseIntensity <= 0.f || g_DepthTexture.Load(pixel).w != 3.f)
        return 0.f;
    const float3 albedo = g_DiffuseTexture.Load(pixel).rgb;
    const float4 material = g_MaterialSpecularTexture.Load(pixel);
    const float ao = saturate(g_DepthTexture.Load(pixel).z);
    // 제한된 F0 diffuse 에너지 분리는 이 명시적인 근사의 정책이다.
    // 원본 RNM/reflection-BRDF와 emissive 계산은 변경하지 않는다.
    return albedo * (1.f - saturate(material.a)) * (1.f - saturate(material.rgb)) * ao *
        Evaluate_CubeDiffuseIrradiance(g_NormalTexture.Load(pixel).xyz * 2.f - 1.f);
}

// albedo×누적 diffuse + 누적 specular를 기본으로 합성한다. marker3의 별도 간접광에는 SSAO와 움직이는 그림자 보정을 적용한다.
// 그 뒤 fog, 마지막에 emissive를 더하고 문서별 Bloom 배율을 전달한다. 실제 UI/최종 tone은 뒤 pass다.
PS_OUT_BACKBUFFER PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.f == vDiffuse.a)
        discard;
    
    vector vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
    
    vector vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);
    vector vEmissive = g_EmissiveTexture.Sample(LinearSampler, In.vTexcoord);
    
    
    vector vLitColor = vDiffuse * vShade + vSpecular;
    const int3 pixel = int3(int2(In.vPosition.xy), 0);
    if (g_DepthTexture.Load(pixel).w == 3.f)
    {
        const float3 indirect = g_CharacterGeometryTexture.Load(pixel).rgb;
        const float3 position = g_GeometricNormalTexture.Load(pixel).xyz;
        const float3 normal = Decode_MapPBRGeometricNormal(pixel);
        const float screenAO = Resolve_AmbientOcclusion(In.vTexcoord);
        vLitColor.rgb += (indirect * Resolve_DynamicBakedShadow(position, normal) +
            Resolve_MapCubeDiffuse(pixel)) * screenAO;
    }
    vLitColor.rgb = Resolve_HeightFog(vLitColor.rgb, In.vTexcoord);
    /* Emissive와 Effect의 HDR 기여는 조명을 받는 표면이 아니라 발광원이다.
       carrier가 그림자 안에 있어도 Bloom 입력에 남겨야 한다. */
    Out.vBackBuffer = vLitColor + vEmissive;
    if (g_iPortraitCapture != 0u)
        Out.vBackBuffer.a = 1.f;
    // source-character G-buffer의 normal.a는 typed 조명에서 다른 용도로 쓰지 않는다.
    // deferred model-cue 경로에서도 문서별 배율을 보존한다.
    const float bloomMarker = g_NormalTexture.Load(pixel).a;
    const bool hasBloomOverride = g_DepthTexture.Load(pixel).w == 5.f && bloomMarker > 0.f;
    // UNORM16 alpha=0은 scene 기본값 선택이며 명시적인 intensity=0은 정확히 code3855다.
    const float authoredBloom = bloomMarker <= 1.f / 17.f + 0.5f / 65535.f ?
        0.f : clamp(bloomMarker * 17.f - 1.f, 0.f, 16.f);
    const float bloomIntensity = hasBloomOverride ? authoredBloom : g_fSceneBloomIntensity;
    Out.vBloomContribution = float4(min(Extract_SceneBloom(Out.vBackBuffer.rgb) *
        bloomIntensity, 60000.f), Out.vBackBuffer.a);
    return Out;
}


// 음수와 큰 HDR 값을 FP16 범위 안의 60000으로 제한한다. 별도 isfinite/NaN 검사를 하는 함수는 아니다.
float3 Sanitize_HDR(float3 vColor)
{
    // additive가 많이 겹치면 FP16 SceneHDR에 INF가 생길 수 있다. bright-pass와
    // Hable의 나눗셈 전에 후처리 값을 유한 범위로 제한한다. NaN 별도 검사는 아니다.
    return min(max(vColor, 0.f), 60000.f);
}

// 밝기 문턱과 soft knee로 색 비율을 유지한 bright-pass를 계산하는 helper다.
// 현재 PS_MAIN_BLOOM_EXTRACT는 이미 가중된 기여 texture를 받아 이 helper를 다시 호출하지 않는다.
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

// 입력에는 각 draw의 bright-pass와 intensity가 이미 들어 있다. 여기서는 정리한 RGB를 blur 입력으로 전달한다.
PS_OUT_BACKBUFFER PS_MAIN_BLOOM_EXTRACT(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    float3 vScene = Sanitize_HDR(g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb);
    // 입력에는 이미 각 발광 기여의 bright-pass와 intensity가 들어 있다.
    Out.vBackBuffer = float4(vScene, 1.f);
    return Out;
}

// 한 축의 중앙+양쪽 4개, 총 9개 표본으로 blur한다. H/V pass가 같은 함수를 다른 방향으로 두 번 사용한다.
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
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    Out.vBackBuffer = float4(
        Blur_Bloom(In.vTexcoord, float2(1.f, 0.f)), 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_BLOOM_BLUR_V(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    Out.vBackBuffer = float4(
        Blur_Bloom(In.vTexcoord, float2(0.f, 1.f)), 1.f);
    return Out;
}

// seed의 하위 24bit와 화면 위치를 float 계산에 넣는 의사난수다. 게임플레이 난수나 보안 난수와 무관하다.
float Presentation_Hash(float2 vPosition, uint iSeed)
{
    float fSeed = (float)(iSeed & 0x00ffffffu);
    return frac(sin(dot(vPosition + fSeed,
        float2(12.9898f, 78.233f))) * 43758.5453f);
}

// marker0/5의 skinned bit와 선택 native actor program을 읽어 보호할 수신면을 판별한다.
// 다른 map marker가 같은 W에 넣은 payload를 actor ID로 해석하지 않는다.
bool Is_ProtectedNoiseReceiver(int2 pixel, float4 depth)
{
    const uint bits = asuint(g_GeometricNormalTexture.Load(int3(pixel, 0)).w);
    // 기본/source model writer만 bit8을 소유하며 다른 marker는 같은 곳에 map 데이터를 넣는다.
    if ((depth.w == 0.f || depth.w == 5.f) && (bits & 256u) != 0u)
        return true;
    if (depth.w != 5.f)
        return false;
    const uint program = (bits & 255u) | (((bits >> 9u) & 255u) << 8u);
    return (program >= 1u && program <= 24u) ||
        (program >= 26u && program <= 29u);
}

// 양옆 깊이 변화 중 작은 쪽을 써 같은 표면 쪽의 NDC 기울기를 추정한다. 가림 경계를 건너는 큰 차이를 피하는 근사다.
float ProtectedNoise_DepthSlope(float before, float center, float after)
{
    // 가림 경계에서는 같은 표면에 있는 이웃을 고른다. 평면의 NDC 깊이는
    // 기울어진 아레나 바닥도 포함해 screen 좌표의 아핀 함수다.
    const float a = center - before, b = after - center;
    return abs(a) < abs(b) ? a : b;
}

// bilinear 보간에 실제 기여하는 2×2 texel을 검사한다. actor/배경/허용 기울기를 넘는 깊이 차이가 하나라도 있으면 거절한다.
bool ProtectedNoise_SafeFootprint(float2 uv, int2 dimensions, int2 centerPixel,
    float4 centerDepth, float2 depthSlope)
{
    if (any(uv < 0.f) || any(uv > 1.f))
        return false;
    const float2 texel = uv * float2(dimensions) - .5f;
    const int2 first = int2(floor(texel));
    const float2 fraction = frac(texel);
    // 깊이 허용치는 5cm이며 작은 float 정밀도 하한도 유지한다.
    const float tolerance = max(0.0000001f,
        abs(1.f - centerDepth.x) * .05f / max(centerDepth.y * 1000.f, .05f));
    [unroll] for (int y = 0; y < 2; ++y)
    [unroll] for (int x = 0; x < 2; ++x)
    {
        const float weight = (x ? fraction.x : 1.f - fraction.x) *
            (y ? fraction.y : 1.f - fraction.y);
        if (weight <= 0.000001f)
            continue;
        const int2 pixel = clamp(first + int2(x, y), int2(0, 0), dimensions - 1);
        const float4 depth = g_DepthTexture.Load(int3(pixel, 0));
        const float expected = centerDepth.x + dot(float2(pixel - centerPixel), depthSlope);
        if (Is_ProtectedNoiseReceiver(pixel, depth) || depth.x >= .99999f ||
            depth.y >= .99999f || abs(depth.x - expected) > tolerance)
            return false;
    }
    return true;
}

// 일반 RG 왜곡은 유지하고 보호용 BA 왜곡만 더할 수 있는지 검사한다.
// 이전/최종 두 sampling footprint가 모두 안전해야 하며 실패하면 일반 offset으로 돌아간다.
float2 Resolve_ProtectedNoiseOffset(float2 uv, float2 ordinary, float2 protectedOffset)
{
    if (!any(protectedOffset != 0.f))
        return ordinary;
    uint width, height;
    g_DepthTexture.GetDimensions(width, height);
    if (!width || !height)
        return ordinary;
    const int2 dimensions = int2(width, height);
    const int2 pixel = clamp(int2(uv * float2(dimensions)), int2(0, 0), dimensions - 1);
    const float4 depth = g_DepthTexture.Load(int3(pixel, 0));
    if (Is_ProtectedNoiseReceiver(pixel, depth) || depth.x >= .99999f || depth.y >= .99999f)
        return ordinary;
    const float left = g_DepthTexture.Load(int3(max(pixel - int2(1, 0), 0), 0)).x;
    const float right = g_DepthTexture.Load(int3(min(pixel + int2(1, 0), dimensions - 1), 0)).x;
    const float top = g_DepthTexture.Load(int3(max(pixel - int2(0, 1), 0), 0)).x;
    const float bottom = g_DepthTexture.Load(int3(min(pixel + int2(0, 1), dimensions - 1), 0)).x;
    const float2 slope = float2(ProtectedNoise_DepthSlope(left, depth.x, right),
        ProtectedNoise_DepthSlope(top, depth.x, bottom));
    const float2 combined = clamp(ordinary + protectedOffset, -0.05f, 0.05f);
    // 두 sampling footprint를 모두 검사한다. 그렇지 않으면 이동한 배경 pixel이 선형 보간으로
    // 이웃 캐릭터/폭탄을 끌어와 두 번째 상처럼 보이게 할 수 있다.
    if (!ProtectedNoise_SafeFootprint(uv + ordinary, dimensions, pixel, depth, slope) ||
        !ProtectedNoise_SafeFootprint(uv + combined, dimensions, pixel, depth, slope))
        return ordinary;
    return combined;
}

// 같은 왜곡 UV로 SceneHDR와 이미 가중된 Bloom을 함께 옮긴다. 두 입력이 다른 위치를 읽으면 색과 Bloom이 어긋난다.
PS_OUT_BACKBUFFER PS_MAIN_SCENE_RESOLVE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    const float4 distortion = g_DistortionTexture.Sample(PostProcessSampler, In.vTexcoord);
    const float2 ordinary = clamp(distortion.rg, -0.05f, 0.05f);
    const float2 vDistortion = Resolve_ProtectedNoiseOffset(In.vTexcoord, ordinary, distortion.ba);
    float3 vScene = Sanitize_HDR(g_SceneHDRTexture.Sample(
        PostProcessSampler, In.vTexcoord + vDistortion).rgb);
    Out.vBackBuffer = float4(vScene, 1.f);
    Out.vBloomContribution = float4(Sanitize_HDR(g_SceneBloomTexture.Sample(
        PostProcessSampler, In.vTexcoord + vDistortion).rgb), 1.f);
    return Out;
}

// 고정 화면 격자의 시간별 noise로 R/B를 반대 방향에 읽고 RGB tint noise를 더한다.
// Bloom도 같은 채널 이동을 사용하지만 새 tint noise를 발광으로 추가하지 않는다.
PS_OUT_BACKBUFFER PS_MAIN_RGB_NOISE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
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
    float3 vBloom;
    vBloom.r = g_PostBloomTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord + float2(fOffset, 0.f), 0.f, 1.f)).r;
    vBloom.g = g_PostBloomTexture.Sample(PostProcessSampler,
        In.vTexcoord).g;
    vBloom.b = g_PostBloomTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord - float2(fOffset, 0.f), 0.f, 1.f)).b;
    Out.vBloomContribution = float4(Sanitize_HDR(vBloom), 1.f);
    return Out;
}

// 중심 방향으로 8개 표본을 평균하고 원본과 강도로 섞는다. 같은 UV/혼합을 Bloom에도 적용한다.
PS_OUT_BACKBUFFER PS_MAIN_ZOOM_BLUR(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    float2 vFromCenter = In.vTexcoord - float2(0.5f, 0.5f);
    float fStrength = clamp(g_fPresentationIntensity, 0.f, 8.f) * 0.025f;
    float3 vColor = 0.f;
    float3 vBloom = 0.f;
    [unroll]
    for (int iSample = 0; iSample < 8; ++iSample)
    {
        float fSample = (float)iSample / 7.f;
        float2 vUV = clamp(In.vTexcoord - vFromCenter *
            fStrength * fSample, 0.f, 1.f);
        vColor += g_PostProcessTexture.Sample(
            PostProcessSampler, vUV).rgb;
        vBloom += g_PostBloomTexture.Sample(PostProcessSampler, vUV).rgb;
    }
    vColor *= 0.125f;
    float3 vSource = g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb;
    float fMix = saturate(g_fPresentationSecondaryIntensity > 0.f ?
        g_fPresentationSecondaryIntensity : g_fPresentationIntensity);
    Out.vBackBuffer = float4(Sanitize_HDR(
        lerp(vSource, vColor * g_vPresentationTint.rgb, fMix)), 1.f);
    Out.vBloomContribution = float4(Sanitize_HDR(lerp(
        g_PostBloomTexture.Sample(PostProcessSampler, In.vTexcoord).rgb,
        vBloom * 0.125f * g_vPresentationTint.rgb, fMix)), 1.f);
    return Out;
}

// 중심에서 멀수록 강해지는 offset으로 R/B를 반대로 옮기고 G는 중심에 둔다.
// 색수차 표현용 화면 연산이며 광학 렌즈 전체를 물리적으로 추적하는 식은 아니다.
PS_OUT_BACKBUFFER PS_MAIN_CHROMATIC_ABERRATION(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
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
    float3 vBloom;
    vBloom.r = g_PostBloomTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord + vShift, 0.f, 1.f)).r;
    vBloom.g = g_PostBloomTexture.Sample(PostProcessSampler,
        In.vTexcoord).g;
    vBloom.b = g_PostBloomTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord - vShift, 0.f, 1.f)).b;
    Out.vBloomContribution = float4(Sanitize_HDR(vBloom * g_vPresentationTint.rgb), 1.f);
    return Out;
}

// noise와 scan 색을 화면에 더한다. 기존 Bloom은 그대로 전달해 이 화면 색 변화가 새 스킬 발광이 되지 않게 한다.
PS_OUT_BACKBUFFER PS_MAIN_FILM_NOISE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
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
    // film/noise는 화면 색 연산이며 새로운 스킬 발광 기여가 아니다.
    Out.vBloomContribution = float4(g_PostBloomTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb, 1.f);
    return Out;
}

// 저작 filter/address 선택에 따라 point/linear, clamp/wrap sampler를 고른다.
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

// 지정된 R/G/B 또는 기본 alpha 채널을 coverage로 읽는다. grayscale texture를 항상 alpha로 읽지 않는다.
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

// 화면 UV에서 위치를 빼고 역회전·역scale로 카드 UV를 구한다. 카드 밖은 투명이다.
// 시간 drift는 texture UV에만 더하고 coverage×저작 alpha×tint alpha를 최종 alpha로 만든다.
float4 Resolve_PresentationOverlay(float2 texcoord)
{
    float fRotation = radians(-(
        g_fPresentationOverlayRotationDegrees +
        g_fPresentationOverlayAngularVelocityDegreesPerSecond *
            g_fPresentationTime));
    float fCos = cos(fRotation);
    float fSin = sin(fRotation);
    float2 vCentered = texcoord - g_vPresentationOverlayPosition;
    float2 vRotated = float2(
        vCentered.x * fCos - vCentered.y * fSin,
        vCentered.x * fSin + vCentered.y * fCos);
    float2 vCard = vRotated /
        max(g_vPresentationOverlayScale, float2(0.00001f, 0.00001f)) + 0.5f;
    bool bInside = all(vCard >= float2(0.f, 0.f)) &&
        all(vCard <= float2(1.f, 1.f));
    if (!bInside)
    {
        return float4(0.f, 0.f, 0.f, 0.f);
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
    return float4(vRadiance, fAlpha);
}

// HDR 장면 위에 overlay를 alpha로 섞고 Bloom도 같은 coverage로 합성한다.
PS_OUT_BACKBUFFER PS_MAIN_TEXTURED_OVERLAY(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    float3 source = Sanitize_HDR(g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb);
    float4 overlay = Resolve_PresentationOverlay(In.vTexcoord);
    Out.vBackBuffer = float4(Sanitize_HDR(lerp(source, overlay.rgb, overlay.a)), 1.f);
    const float3 sourceBloom = g_PostBloomTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb;
    Out.vBloomContribution = float4(lerp(sourceBloom,
        Write_SceneBloom(overlay).rgb, overlay.a), 1.f);
    return Out;
}

// 표시용 overlay는 선형 RGB를 sRGB 구간식으로 한 번 encode한다. 장면 exposure/tone/Bloom/gamma 조절은 거치지 않는다.
PS_OUT_BACKBUFFER PS_MAIN_DISPLAY_OVERLAY(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    float4 overlay = Resolve_PresentationOverlay(In.vTexcoord);
    // sRGB texture view는 이미 선형 값으로 decode한다. 표시 이미지는 한 번만 encode하고
    // scene의 exposure·film curve·Bloom·gamma 설정은 적용하지 않는다.
    float3 linearColor = saturate(overlay.rgb);
    float3 low = linearColor * 12.92f;
    float3 high = 1.055f * pow(linearColor, 1.f / 2.4f) - 0.055f;
    float3 displayColor = float3(
        linearColor.r <= 0.0031308f ? low.r : high.r,
        linearColor.g <= 0.0031308f ? low.g : high.g,
        linearColor.b <= 0.0031308f ? low.b : high.b);
    Out.vBackBuffer = float4(displayColor, overlay.a);
    return Out;
}

/* Hable(Uncharted 2) 필름 커브. 씬 컬러는 선형이고 1을 넘을 수 있다. */
// 색각 접근성 보정: LMS 공간에서 선택한 색각을 모사하고
// 구분하기 어려워지는 색 정보의 차이를 다른 채널로 분배한다.
// strength로 보정 전 색과 보정한 색을 섞는다.
// 위 기존 Hable 메모와 별개로, 이 함수는 RGB→LMS→선택 색각 모사 뒤 잃는 색 차이를 다른 채널에 분배한다.
// 실제 Hable 곡선은 아래 Tonemap_Hable이며 현재 함수는 strength로 원본 표시색과 보정색을 섞는다.
float3 Apply_ColorVisionFilter(float3 vColor)
{
    const float3x3 RGB_TO_LMS = float3x3(
        17.8824f, 43.5161f, 4.11935f,
        3.45565f, 27.1554f, 3.86714f,
        0.0299566f, 0.184309f, 1.46709f);
    const float3x3 LMS_TO_RGB = float3x3(
        0.0809444479f, -0.130504409f, 0.116721066f,
        -0.0102485335f, 0.0540193266f, -0.113614708f,
        -0.000365296938f, -0.00412161469f, 0.693511405f);
    float3x3 simulate;
    if (g_iColorFilterType == 1)
        simulate = float3x3(0.f, 2.02344f, -2.52581f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
    else if (g_iColorFilterType == 2)
        simulate = float3x3(1.f, 0.f, 0.f, 0.494207f, 0.f, 1.24827f, 0.f, 0.f, 1.f);
    else
        simulate = float3x3(1.f, 0.f, 0.f, 0.f, 1.f, 0.f, -0.395913f, 0.801109f, 0.f);
    float3 lms = mul(RGB_TO_LMS, vColor);
    float3 simulated = mul(LMS_TO_RGB, mul(simulate, lms));
    float3 lost = vColor - simulated;
    const float3x3 SHIFT = float3x3(
        0.f, 0.f, 0.f,
        0.7f, 1.f, 0.f,
        0.7f, 0.f, 1.f);
    float3 corrected = saturate(vColor + mul(SHIFT, lost));
    return lerp(vColor, corrected, saturate(g_fColorFilterStrength));
}

// HDR 색에 유리함수 필름 곡선을 적용한다. 이 함수 자체에는 exposure나 white-point 정규화가 없고 호출자가 적용한다.
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

// 복원 근거는 EFEngine CPU packing과 FUberPostProcessBlendPixelShader02000이다.
// 색 보정 LUT atlas를 그리는 전용 vertex 경로다. 일반 world/view 변환 대신 입력 XY를 clip 범위로 확장한다.
VS_OUT VS_SOURCE_LUT_BAKE(VS_IN input)
{
    VS_OUT output;
    output.vPosition = float4(input.vPosition.xy * 2.f, 0.f, 1.f);
    output.vTexcoord = input.vTexcoord;
    return output;
}

// CPU가 포장한 grading parameter로 offset·scale·power·gray 혼합을 적용한다.
// BRDF lookup과 다른 색 보정용 LUT 준비 단계이며 아래 neutral/원본 LUT 혼합 뒤 공통으로 호출된다.
float4 Grade_SourceLut(float4 color)
{
    float3 graded = saturate(color.rgb - g_vSourceGradingParameters[0].rgb) * g_vSourceGradingParameters[1].rgb;
    graded = pow(max(graded, 1.e-8f), g_vSourceGradingParameters[2].rgb);
    float gray = dot(graded, g_vSourceGradingParameters[3].rgb);
    graded = graded * g_vSourceGradingParameters[0].w + gray;
    graded = graded * g_vSourceGradingParameters[5].rgb + g_vSourceGradingParameters[6].rgb;
    color.rgb = graded * g_vSourceGradingParameters[4].rgb;
    return pow(max(color, 1.e-8f), 2.2f * g_vSourceGradingParameters[5].w);
}

// 256×16의 16³ RGB 격자를 복원해 neutral weight와 grading만 적용한다.
float4 PS_SOURCE_LUT_BAKE_NEUTRAL(PS_IN input) : SV_TARGET0
{
    float2 coordinate = input.vTexcoord - float2(.5f / 256.f, .5f / 16.f);
    float red = frac(coordinate.x * 16.f);
    float blue = coordinate.x - red * (1.f / 16.f);
    float4 color = float4(float3(red, coordinate.y, blue) * g_fSourceNeutralLutWeight * (16.f / 15.f), 0.f);
    return Grade_SourceLut(color);
}

// neutral 격자와 원본 LUT 최대4개를 각 weight로 합한 뒤 grading해 하나의 atlas를 만든다.
float4 PS_SOURCE_LUT_BAKE(PS_IN input) : SV_TARGET0
{
    // FLUTBlender: neutral 격자와 원본 LUT를 합하고 조회 전에 grading한다.
    float2 coordinate = input.vTexcoord - float2(.5f / 256.f, .5f / 16.f);
    float red = frac(coordinate.x * 16.f);
    float blue = coordinate.x - red * (1.f / 16.f);
    float4 color = float4(float3(red, coordinate.y, blue) * g_fSourceNeutralLutWeight * (16.f / 15.f), 0.f);
    color += g_SourceLutInputs[0].SampleLevel(SourceGradingSampler, input.vTexcoord, 0.f) * g_vSourceLutWeights.x;
    color += g_SourceLutInputs[1].SampleLevel(SourceGradingSampler, input.vTexcoord, 0.f) * g_vSourceLutWeights.y;
    color += g_SourceLutInputs[2].SampleLevel(SourceGradingSampler, input.vTexcoord, 0.f) * g_vSourceLutWeights.z;
    color += g_SourceLutInputs[3].SampleLevel(SourceGradingSampler, input.vTexcoord, 0.f) * g_vSourceLutWeights.w;
    return Grade_SourceLut(color);
}

// 음수 HDR을 제거하고 저휘도 gamma형과 유리함수 shoulder를 현재 source parameter로 섞어 [0,1]에 놓는다.
float3 Tonemap_SourceCustomizable(float3 color)
{
    color = max(color, 0.f);
    float3 rational = color / abs(color + g_vSourceToneCurve.x) * g_vSourceToneCurve.y;
    float3 linearGamma = pow(color * g_vSourceToneCurve.w, 1.f / 2.2f);
    float3 shoulder = saturate((color - g_vSourceToneCurve.z) * 10000.f);
    return saturate(lerp(lerp(linearGamma, rational, shoulder), rational, saturate(g_fSourceToneToe)));
}

// R/G는 한 slice 안에서 bilinear, B는 인접 두 slice 사이에서 보간해 3D LUT를 2D atlas로 읽는다.
// 입력은 앞 tone 단계의 [0,1] 표시색이고 환경 BRDF LUT의 각도/roughness 좌표와는 무관하다.
float3 Sample_SourceGradingLut(float3 color)
{
    // B는 가로로 나열한 인접 slice를 고르고 R/G는 slice 안에서 bilinear 보간한다.
    // 14.9999 제한으로 B=1도 slice14에서 시작하며 보간 가중치는 정확히 1이다.
    float slice = floor(color.b * 14.9999f);
    float2 uv = float2(slice / 16.f + color.r * (15.f / 256.f) + .5f / 256.f,
        color.g * (15.f / 16.f) + .5f / 16.f);
    float3 low = g_SourceGradingLut.SampleLevel(SourceGradingSampler, uv, 0.f).rgb;
    float3 high = g_SourceGradingLut.SampleLevel(SourceGradingSampler, uv + float2(1.f/16.f,0.f), 0.f).rgb;
    return lerp(low, high, color.b * 15.f - slice);
}

// SceneHDR와 선택 Bloom을 합하고 exposure를 적용한다. source tone+grading 또는 Hable+gamma 경로 중 하나를 고른다.
// portrait는 먼저 alpha-over 결과를 straight RGB로 복구한다. color filter/desaturation은 현재 Hable 분기에서만 적용된다.
float3 Resolve_FinalLDR(float2 vTexcoord)
{
    const float4 sceneSample = g_SceneHDRTexture.Sample(
        PostProcessSampler, clamp(vTexcoord, 0.f, 1.f));
    // UI는 straight alpha를 소비한다. field tone 곡선 적용 전에 캡처의
    // 검은 배경 alpha-over를 풀어 hair 경계가 두 번 어두워지지 않게 한다.
    float3 vScene = Sanitize_HDR(g_iPortraitCapture != 0u ?
        sceneSample.rgb / max(saturate(sceneSample.a), 1e-4f) : sceneSample.rgb);
    float3 vBloom = float3(0.f, 0.f, 0.f);
    if (0u != g_iBloomEnabled)
    {
        vBloom = Sanitize_HDR(g_BloomTexture.Sample(
            PostProcessSampler, clamp(vTexcoord, 0.f, 1.f)).rgb);
    }

    float3 vSceneWithBloom = vScene + vBloom * g_vSceneBloomTint.rgb;
    if (g_iSourcePostProcessEnabled != 0)
        return Sample_SourceGradingLut(Tonemap_SourceCustomizable(
            Sanitize_HDR(vSceneWithBloom * g_fToneMapExposure)));
    float3 vWhiteScale = max(Tonemap_Hable(
        max(g_fToneMapWhitePoint, 1.f).xxx), 0.00001f);
    float3 vMapped = Tonemap_Hable(vSceneWithBloom *
        max(g_fToneMapExposure, 0.01f)) / vWhiteScale;
    float3 displayColor = pow(saturate(vMapped),
        1.f / max(g_fToneMapGamma, 1.f));
    // 표시 공간의 adapter이며 원작의 packed LUT 변환 자체는 아니다.
    // 선택적인 색 조절이 없으면 기존 반환 경로를 그대로 유지한다.
    if (g_fSceneDesaturation > 0.f)
    {
        float luminance = dot(displayColor, float3(0.299f, 0.587f, 0.114f));
        displayColor = lerp(displayColor, luminance.xxx, saturate(g_fSceneDesaturation));
    }
    if (g_iColorFilterType > 0 && g_fColorFilterStrength > 0.f)
        displayColor = Apply_ColorVisionFilter(displayColor);
    return displayColor;
}

float Final_Luminance(float3 vColor)
{
    return dot(vColor, float3(0.299f, 0.587f, 0.114f));
}

// 현재 pixel과 주변 표시색의 휘도 대비로 edge 방향을 추정하고 제한된 구간을 다시 샘플한다.
// LDR 변환을 끝낸 색을 필터링하며 OFF면 중심값 그대로다. 이는 MSAA geometry coverage 계산과 다르다.
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

// 최대 RGB 하나로 나눠 채널 비율을 유지한 뒤 진단용 gamma를 적용한다. 제품 장면 tone 곡선은 아니다.
float3 Compress_MaterialDiagnosticHDR(float3 color)
{
    color = max(color, 0.f);
    // 하나의 scalar로 압축해 표시 encode 전에 RGB 비율을 유지한다.
    const float peak = max(color.r, max(color.g, color.b));
    return pow(color / (1.f + peak), 1.f / 2.2f);
}

// 진단용으로 Bloom/exposure/tone까지만 보여준다. source LUT grading과 FXAA 전의 성분을 분리한다.
float3 Resolve_SceneToneBeforeGrade(float2 uv)
{
    const float3 scene = Sanitize_HDR(g_SceneHDRTexture.Sample(
        PostProcessSampler, clamp(uv, 0.f, 1.f)).rgb);
    const float3 bloom = g_iBloomEnabled != 0u ? Sanitize_HDR(g_BloomTexture.Sample(
        PostProcessSampler, clamp(uv, 0.f, 1.f)).rgb) : 0.f;
    const float3 input = scene + bloom * g_vSceneBloomTint.rgb;
    if (g_iSourcePostProcessEnabled != 0)
        return Tonemap_SourceCustomizable(Sanitize_HDR(input * g_fToneMapExposure));
    const float3 white = max(Tonemap_Hable(max(g_fToneMapWhitePoint, 1.f).xxx), 0.00001f);
    const float3 mapped = Tonemap_Hable(input * max(g_fToneMapExposure, 0.01f)) / white;
    return pow(saturate(mapped), 1.f / max(g_fToneMapGamma, 1.f));
}

/* SceneHDR을 백버퍼로 옮기는 유일한 지점. 톤매핑, 감마, FXAA는 UI 전에 여기서 처리한다. */
// 일반 장면 출력 또는 선택한 재질/장면 진단을 백버퍼로 보낸다.
// 진단의 marker별 채널 의미를 확인해야 하며 marker7의 Specular RT에는 diffuse를 포함한 직접광이 들어 있다.
PS_OUT_BACKBUFFER PS_MAIN_FINAL(PS_IN In)
{
    PS_OUT_BACKBUFFER Out = (PS_OUT_BACKBUFFER)0;
    // 장면 전체 진단은 marker와 무관하게 캐릭터와 배경을 포함한다.
    // HDR은 Bloom/exposure/tone 전이고 Tone은 Bloom/exposure를 포함하되 grading 전이다.
    // Graded는 FXAA와 제품 UI 전 실제 최종 색 변환까지 반영한다.
    if (g_MaterialDebugView >= 11u && g_MaterialDebugView <= 13u)
    {
        float3 color;
        if (g_MaterialDebugView == 11u)
            color = Compress_MaterialDiagnosticHDR(Sanitize_HDR(g_SceneHDRTexture.Sample(
                PostProcessSampler, In.vTexcoord).rgb));
        else if (g_MaterialDebugView == 12u)
            color = Resolve_SceneToneBeforeGrade(In.vTexcoord);
        else
            color = Resolve_FinalLDR(In.vTexcoord);
        Out.vBackBuffer = float4(color, 1.f);
        return Out;
    }
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
                // 이 family의 가산 target에는 diffuse를 포함한 원본 직접광 전체가 들어 있다.
                // specular만 분리해 기록한 pass로 해석하면 안 된다.
                const float3 direct = max(g_SpecularTexture.Load(pixel).rgb, 0.f);
                color = pow(direct / (1.f + direct), 1.f / 2.2f);
            }
            // 이 permutation에는 원본 reflection texture/ORM 입력이 없다.
            Out.vBackBuffer = float4(color, 1.f);
            return Out;
        }
        if (marker == 1.f || marker == 2.f || marker == 3.f ||
            (marker == 4.f || marker == 8.f || marker == 9.f || marker == 10.f ||
                (marker >= 11.f && marker <= 14.f)))
        {
            if (g_MaterialDebugView == 2u)
                color = g_NormalTexture.Load(pixel).rgb;
            else if (g_MaterialDebugView >= 5u)
            {
                // legacy marker2는 이 채널에 specular power/mask를 기록한다.
                // 그 값을 복원된 PBR 진단값처럼 표시하지 않는다.
                if (marker == 3.f)
                {
                    if (g_MaterialDebugView == 14u)
                        color = Compress_MaterialDiagnosticHDR(Resolve_MapCubeDiffuse(pixel));
                    else if (g_MaterialDebugView == 5u)
                        color = g_NormalTexture.Load(pixel).aaa;
                    else if (g_MaterialDebugView == 6u)
                        color = g_MaterialSpecularTexture.Load(pixel).aaa;
                    else if (g_MaterialDebugView == 7u)
                        color = g_DepthTexture.Load(pixel).zzz;
                    else if (g_MaterialDebugView >= 8u && g_MaterialDebugView <= 10u)
                    {
                        const float3 environment = g_CharacterSurfaceTexture.Load(pixel).rgb;
                        const float3 combined = g_CharacterGeometryTexture.Load(pixel).rgb;
                        if (g_MaterialDebugView == 8u)
                            color = combined - environment;
                        else if (g_MaterialDebugView == 9u)
                            color = environment;
                        else
                            color = g_DiffuseTexture.Load(pixel).rgb * g_ShadeTexture.Load(pixel).rgb;
                        // 직접 specular와 같은 HDR 진단 곡선을 쓴다.
                        color = Compress_MaterialDiagnosticHDR(color);
                    }
                }
            }
            else
            {
                color = g_MaterialDebugView == 3u ?
                    g_SpecularTexture.Load(pixel).rgb : g_DiffuseTexture.Load(pixel).rgb;
                if ((marker == 4.f || marker == 8.f || marker == 9.f || marker == 10.f ||
                    (marker >= 11.f && marker <= 14.f)) && g_MaterialDebugView != 3u)
                    color *= g_MaterialSpecularTexture.Load(pixel).a;
                // 원본 기여 진단은 scene exposure·Bloom·FXAA를 거치지 않는다.
                // specular는 HDR일 수 있으므로 이 진단 표시에서만 압축한다.
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

/* deferred 표면과 forward 전용 hair 모두 SceneHDR coverage에 기여한다.
Depth marker만으로는 body 실루엣 밖 반투명 pixel을 나타낼 수 없다. */
// SceneHDR의 coverage가 거의 0이면 투명으로 내보내고, 나머지는 최종 색과 coverage를 함께 반환한다.
// 불투명 body와 forward hair가 만든 alpha를 사용하므로 depth marker만으로 실루엣을 대체하지 않는다.
float4 PS_MAIN_PORTRAIT_RESOLVE(PS_IN In) : SV_TARGET0
{
    const float coverage = saturate(g_SceneHDRTexture.Sample(
        PostProcessSampler, saturate(In.vTexcoord)).a);
    if (coverage <= 1e-4f)
        return float4(0.f, 0.f, 0.f, 0.f);
    return float4(Resolve_FinalFXAA(In.vTexcoord), coverage);
}


// source light row는 이미 다른 marker를 거부한다. stencil도 PS 호출 전에 같은 대상을
// 제외하되 scene outline용 stencil은 건드리지 않는다.
// source marker5 mask를 쓴 뒤 읽기 pass가 stencil=1인 영역만 조명한다. material row의 세부 판정은 PS에서도 유지한다.
DepthStencilState DSS_SourceLightMaskWrite
{
    DepthEnable = false;
    DepthWriteMask = zero;
    StencilEnable = true;
    StencilReadMask = 0xff;
    StencilWriteMask = 0xff;
    FrontFaceStencilFunc = always;
    FrontFaceStencilPass = replace;
    FrontFaceStencilFail = keep;
    FrontFaceStencilDepthFail = keep;
    BackFaceStencilFunc = always;
    BackFaceStencilPass = replace;
    BackFaceStencilFail = keep;
    BackFaceStencilDepthFail = keep;
};

DepthStencilState DSS_SourceLightMaskRead
{
    DepthEnable = false;
    DepthWriteMask = zero;
    StencilEnable = true;
    StencilReadMask = 0xff;
    StencilWriteMask = 0x00;
    FrontFaceStencilFunc = equal;
    FrontFaceStencilPass = keep;
    FrontFaceStencilFail = keep;
    FrontFaceStencilDepthFail = keep;
    BackFaceStencilFunc = equal;
    BackFaceStencilPass = keep;
    BackFaceStencilFail = keep;
    BackFaceStencilDepthFail = keep;
};

void PS_MAIN_SOURCE_LIGHT_MASK(PS_IN input)
{
    if (g_DepthTexture.Load(int3(int2(input.vPosition.xy), 0)).w != 5.f)
        discard;
}

[earlydepthstencil]
PS_OUT_LIGHT PS_MAIN_SOURCE_DIRECTIONAL(PS_IN input)
{
    return PS_MAIN_DIRECTIONAL(input);
}
[earlydepthstencil]
PS_OUT_LIGHT PS_MAIN_SOURCE_POINT(PS_IN input)
{
    return PS_MAIN_POINT(input);
}
[earlydepthstencil]
PS_OUT_LIGHT PS_MAIN_SOURCE_SPOT(PS_IN input)
{
    return PS_MAIN_SPOT(input);
}

// source local-light의 양수 기여는 해당 range sphere 안에 있어야 한다.
// 이를 감싸는 world AABB를 투영하되 기존 fullscreen triangle 위치와 UV 평면은 유지한다.
// hardware clip distance는 그 바깥 부분만 제거한다.
// 광원의 range sphere를 감싼 world AABB 8꼭짓점을 투영해 quad의 clip 경계를 만든다.
// near plane을 가로지르거나 입력/투영이 불확실하면 false로 전체 quad를 유지한다. 완전한 가림 culling은 아니다.
bool SourceLocalLightClipBounds(DEFERRED_LIGHT_INPUT light, out float4 bounds)
{
    bounds = float4(-1.f, -1.f, 1.f, 1.f);
    if (light.flags.w == 0u || !all(isfinite(light.position)) ||
        !isfinite(light.attenuation.x) || light.attenuation.x <= 0.f)
        return false;

    // 기존 light quad는 viewport 너비/높이의 직교 투영을 쓴다.
    // 대각 성분이 2/size이므로 아래 식은 VS texture 바인딩이나 뒤 Final pass uniform에
    // 의존하지 않고 경계를 두 pixel만큼 넓힌다.
    const float2 margin = 2.f * float2(g_ProjMatrix[0][0], g_ProjMatrix[1][1]);
    if (!all(isfinite(margin)) || any(margin <= 0.f) ||
        g_ProjMatrix[0][3] != 0.f || g_ProjMatrix[1][3] != 0.f ||
        g_ProjMatrix[2][3] != 0.f || g_ProjMatrix[3][3] != 1.f)
        return false;

    float2 minimum = float2(3.402823466e+38f, 3.402823466e+38f);
    float2 maximum = -minimum;
    [unroll]
    for (uint corner = 0u; corner < 8u; ++corner)
    {
        const float3 side = float3((corner & 1u) != 0u ? 1.f : -1.f,
            (corner & 2u) != 0u ? 1.f : -1.f,
            (corner & 4u) != 0u ? 1.f : -1.f);
        const float3 position = light.position.xyz + side * light.attenuation.x;
        const float4 clip = mul(mul(float4(position, 1.f),
            g_SourceCharacterViewMatrix), g_SourceCharacterProjMatrix);
        // AABB 전체의 동차 w와 near-plane 거리가 양수면 투영 극값은 꼭짓점에 있다.
        // 이 조건을 확인할 수 없으면 전체 quad를 유지한다.
        if (!all(isfinite(clip)) || clip.w <= 0.00001f || clip.z <= 0.00001f)
            return false;
        const float2 projected = clip.xy / clip.w;
        if (!all(isfinite(projected)))
            return false;
        minimum = min(minimum, projected);
        maximum = max(maximum, projected);
    }
    bounds = float4(max(minimum - margin, -1.f), min(maximum + margin, 1.f));
    return all(isfinite(bounds));
}

struct LIGHT_VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
    uint instanceId : SV_InstanceID;
};
struct LIGHT_PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    nointerpolation uint lightIndex : TEXCOORD1;
    float4 clipDistance : SV_ClipDistance0;
};
// instanceId에 CPU offset을 더해 한 광원 행을 고른다. 배열 범위와 draw 개수는 CPU 제출 계약이다.
// 유효한 화면 경계가 있을 때만 SV_ClipDistance 네 평면으로 quad 밖을 제거한다.
LIGHT_PS_IN VS_LIGHT_INSTANCE(LIGHT_VS_IN input)
{
    VS_IN vertex;
    vertex.vPosition = input.vPosition;
    vertex.vTexcoord = input.vTexcoord;
    const VS_OUT transformed = VS_MAIN(vertex);
    LIGHT_PS_IN output;
    output.vPosition = transformed.vPosition;
    output.vTexcoord = transformed.vTexcoord;
    output.lightIndex = g_LightInstanceOffset + input.instanceId;
    output.clipDistance = 1.f;
    float4 bounds;
    if (SourceLocalLightClipBounds(g_LightInstances[output.lightIndex], bounds))
    {
        output.clipDistance = float4(output.vPosition.x - bounds.x * output.vPosition.w,
            bounds.z * output.vPosition.w - output.vPosition.x,
            output.vPosition.y - bounds.y * output.vPosition.w,
            bounds.w * output.vPosition.w - output.vPosition.y);
    }
    return output;
}
PS_IN LightPixelInput(LIGHT_PS_IN input)
{
    PS_IN result;
    result.vPosition = input.vPosition;
    result.vTexcoord = input.vTexcoord;
    return result;
}
PS_OUT_LIGHT PS_LIGHT_INSTANCE_DIRECTIONAL(LIGHT_PS_IN input)
{
    return Resolve_DirectionalLight(LightPixelInput(input), g_LightInstances[input.lightIndex]);
}
PS_OUT_LIGHT PS_LIGHT_INSTANCE_POINT(LIGHT_PS_IN input)
{
    return Resolve_LocalLight(LightPixelInput(input), false, g_LightInstances[input.lightIndex]);
}
PS_OUT_LIGHT PS_LIGHT_INSTANCE_SPOT(LIGHT_PS_IN input)
{
    return Resolve_LocalLight(LightPixelInput(input), true, g_LightInstances[input.lightIndex]);
}
[earlydepthstencil]
PS_OUT_LIGHT PS_SOURCE_LIGHT_INSTANCE_DIRECTIONAL(LIGHT_PS_IN input)
{
    return PS_LIGHT_INSTANCE_DIRECTIONAL(input);
}
[earlydepthstencil]
PS_OUT_LIGHT PS_SOURCE_LIGHT_INSTANCE_POINT(LIGHT_PS_IN input)
{
    return PS_LIGHT_INSTANCE_POINT(input);
}
[earlydepthstencil]
PS_OUT_LIGHT PS_SOURCE_LIGHT_INSTANCE_SPOT(LIGHT_PS_IN input)
{
    return PS_LIGHT_INSTANCE_SPOT(input);
}

// entry/profile/인자가 같은 shader는 한 번 컴파일하며 pass 상태와 index는 유지한다.
VertexShader DeferredVS = compile vs_5_0 VS_MAIN();
VertexShader DeferredLightInstanceVS = compile vs_5_0 VS_LIGHT_INSTANCE();
VertexShader DeferredSourceLutBakeVS = compile vs_5_0 VS_SOURCE_LUT_BAKE();

// pass 선언은 호출 API의 안정된 index 표다. pass 안의 shader는 공유해도 blend/depth/stencil 상태와 번호는 유지한다.
technique11 DefaultTechnique
{
    pass Debug
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }   

    pass Directional
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
    }

    pass Point
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_POINT();
    }

    pass Combined
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }

    /* DEFERRED::FINAL == 4. 반드시 Combined 다음 순서를 유지할 것. */
    pass Final
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FINAL();
    }

    /* FINAL은 index4를 유지한다. CRenderer가 그 전에 Bloom pass를 호출한다. */
    pass BloomExtract
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_EXTRACT();
    }

    pass BloomBlurHorizontal
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_BLUR_H();
    }

    pass BloomBlurVertical
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_BLUR_V();
    }

    pass SceneResolve
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SCENE_RESOLVE();
    }

    pass PresentationRGBNoise
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_RGB_NOISE();
    }

    pass PresentationZoomBlur
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ZOOM_BLUR();
    }

    pass PresentationFilmNoise
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FILM_NOISE();
    }

    pass SSAORaw
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SSAO_RAW();
    }

    pass SSAOBilateralBlur
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SSAO_BLUR();
    }

    pass PresentationTexturedOverlay
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_TEXTURED_OVERLAY();
    }

    pass PresentationChromaticAberration
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_CHROMATIC_ABERRATION();
    }

    pass Spot
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SPOT();
    }

    pass PresentationDisplayOverlay
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DISPLAY_OVERLAY();
    }

    // 기존 pass index 0~17을 유지한다.
    pass SourceLightMask
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_SourceLightMaskWrite, 1);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SOURCE_LIGHT_MASK();
    }

    pass SourceDirectional
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_SourceLightMaskRead, 1);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SOURCE_DIRECTIONAL();
    }

    pass SourcePoint
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_SourceLightMaskRead, 1);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SOURCE_POINT();
    }

    pass SourceSpot
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_SourceLightMaskRead, 1);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SOURCE_SPOT();
    }

    // 추가한 22~27은 순서를 유지하는 instanced light 제출이다.
    pass InstancedDirectional
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredLightInstanceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT_INSTANCE_DIRECTIONAL();
    }

    pass InstancedPoint
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredLightInstanceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT_INSTANCE_POINT();
    }

    pass InstancedSpot
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredLightInstanceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_LIGHT_INSTANCE_SPOT();
    }

    pass SourceInstancedDirectional
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_SourceLightMaskRead, 1);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredLightInstanceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SOURCE_LIGHT_INSTANCE_DIRECTIONAL();
    }

    pass SourceInstancedPoint
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_SourceLightMaskRead, 1);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredLightInstanceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SOURCE_LIGHT_INSTANCE_POINT();
    }

    pass SourceInstancedSpot
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_SourceLightMaskRead, 1);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredLightInstanceVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SOURCE_LIGHT_INSTANCE_SPOT();
    }

    // 추가한 28은 원본 grading atlas이며 기존 draw pass index를 유지한다.
    pass SourceLutBake
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredSourceLutBakeVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SOURCE_LUT_BAKE();
    }

    pass SourceLutBakeNeutral
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredSourceLutBakeVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SOURCE_LUT_BAKE_NEUTRAL();
    }

    /* DEFERRED::PORTRAIT_RESOLVE는 30이다. 마지막에 추가해 기존 index를 유지한다. */
    pass PortraitResolve
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = DeferredVS;
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_PORTRAIT_RESOLVE();
    }

}



