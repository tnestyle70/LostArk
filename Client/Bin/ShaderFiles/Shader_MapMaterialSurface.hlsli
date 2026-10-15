#ifndef LOSTARK_MAP_MATERIAL_SURFACE
#define LOSTARK_MAP_MATERIAL_SURFACE

// 맵 material의 표면값과 baked/environment 간접광을 계산하는 Client 소유 include다.
// MapAssetRenderUtils가 program·상수를, CMaterial::Bind_SurfaceLighting이 SRV를 바인딩한다.
// MeshBinary/MapInstance 등의 PS가 결과를 G-buffer에 쓰고 Engine Shader_Deferred가 직접광·합성을 이어받는다.
uint g_SourceBgUnlit = 0u;
uint g_SurfacePBRMasked = 0u;

// Rendering Benchmark는 복원된 맵 PBR 기여분만 조절한다.
// x/y는 Deferred 직접 diffuse/specular, z/w는 아래 간접 diffuse/environment specular 비교 배율이다.
// 저장된 재질 자체의 metallic·roughness와 세션 비교 배율을 구분한다.
float4 g_MapPBRContributionScale = float4(1.f, 1.f, 1.f, 1.f);
// 법선 강도, roughness 보정량, 에너지 보정 전 RNM 비교를 위한 값이다.
float4 g_MapPBRDiagnosticParameters = float4(1.f, 0.f, 0.f, 0.f);

// 원본에서 확인된 평행 N/T 입력은 bitangent가 0이다. 이 0축을 유지하며
// 해당 원본 입력에 임의의 법선 축을 만들어 넣지 않는다.
// 길이가 양수인 축만 정규화하고 0축은 유지한다. 입력의 유한성 전체를 검증하는 함수는 아니다.
// 뒤에서 T/B/N을 행으로 묶으며, mul(tangentNormal, tangentToWorld)는 tangent→world 방향 변환이다.
float3 MapGeometryNormalizeOrZero(float3 value)
{
    return dot(value, value) > 0.f ? normalize(value) : float3(0.f, 0.f, 0.f);
}


#include "Shader_SourceStoneSurface.hlsli"
#include "Shader_SourceFoliageSurface.hlsli"

// Program 7은 실제 발탄 bg_base_opa overlay 변형에 해당하며 아래 값은 재질 입력이다.
// normal texture는 linear로, 색상 SRV는 원본 SRGB 설정을 유지한다.
Texture2D g_SurfaceOverlayDiffuseTexture;
Texture2D g_SurfaceOverlayNormalTexture;
float4 g_SurfaceOverlayColor = 1.f;
float g_SurfaceOverlayTiling = 1.f;
float g_SurfaceOverlayNormalIntensity = 1.f;
float g_SurfaceOverlaySharpness = 0.f;
float g_SurfaceOverlayBrightness = 1.f;
float g_SurfaceOverlaySaturation = 1.f;
float g_SurfaceOverlaySpecularIntensity = 0.f;
uint g_SurfaceOverlaySeparateSpecular = 0u;
uint g_SourceOverlayFlags = 263u;
float4 g_SourceOverlayDirection = float4(0.f, 1.f, 0.f, 0.f);
float4 g_SourceOverlayUV = float4(0.f, 1.f, 0.f, 0.f);

// 명시적으로 선택한 surface program이다. 각 SRV는 원본 색 공간 계약을 유지하며
// 실제 Character Select override에는 linear diffuse texture도 포함된다.
uint g_SurfaceProgram = 0;
uint g_SourceBgFlags = 0u;
float4 g_SourceBgBump = float4(0.f, 0.f, 1.f, 0.f);
float4 g_SourceBgUV = float4(0.f, 1.f, 0.f, 0.f);
uint g_SourceBgFlicker = 0u;
float2 g_SourceBgSubspecular = float2(0.f, 60.f);
float4 g_SourceBgRimlight = float4(0.f, 0.f, 0.f, 3.f);
float g_SourceBgSpecularSaturation = 1.f;
float2 g_SourceBgPanning = 0.f;
uint g_SourceFoliageFlags = 0u;
float4 g_SourceFoliageTransmission = float4(0.f, 0.f, 0.f, 1.f);
Texture2D g_SourceFoliageMaskTexture;

uint g_HasSurfaceDefinition = 0;
uint g_SurfaceDebugView = 0;
float g_SurfaceDiffuseBrightness = 1.f;
float g_SurfaceNormalIntensity = 1.f;
float g_SurfaceSpecularIntensity = 1.f;
float g_SurfaceSpecularPower = 50.f;
float g_SurfaceReflectionIntensity = 0.f;
float g_SurfaceReflectionContrast = 0.5f;
float g_SurfaceReflectionTiling = 1.f;
float g_SurfaceDiffuseSaturation = 1.f;
float4 g_SurfaceDiffuseColor = 1.f;
float4 g_SurfaceSpecularColor = 1.f;
float4 g_SurfaceReflectionColor = 1.f;

// 실제 Character Select PBR override다. 3=seamless opaque, 4=opaque이다.
// 이 입력은 이름으로 식별한 재질의 고정 데이터이며 scene 조명 조절값과 구분한다.
// program 3/4만 아래 ORM 계약을 쓴다. R=재질 AO, G=roughness, B=metallic이며 색 텍스처와 다르다.
// 모든 source family의 S/mask 채널을 이 계약으로 해석하지 않는다.
Texture2D g_SurfaceORMTexture;
float2 g_SurfaceUVTiling = 1.f;
uint g_SurfaceUVFixedNormal = 0;
uint g_SurfaceUseWorldReflection = 0;
float g_SurfaceDetailNormalIntensity = 0.f;
float g_SurfaceDetailNormalTiling = 1.f;
float g_SurfaceMetallicIntensity = 1.f;
float g_SurfaceMetallicPower = 1.f;
float g_SurfaceRoughnessIntensity = 1.f;
float g_SurfaceRoughnessPower = 1.f;
float g_SurfaceAOIntensity = 1.f;
float g_SurfaceAOPower = 1.f;
float g_SurfaceSpecularPBRIntensity = 0.5f;
float g_SurfaceNonmetallicBrightness = 1.f;
float g_SurfaceMetallicBrightness = 1.f;
// 기존 최소 roughness와 아직 해석하지 못한 world-reflection 원점 입력을 남겨 둔다.
// 원본 PBR은 EFEngine RVA 0x1578afc의 engine binder 상수 0.045를 사용한다.
float g_SurfaceMinimumRoughness = 0.04f;
float2 g_SurfaceReflectionOriginOffset = 0.f;
float g_SurfaceVertexAlpha = 1.f;
uint g_HasSurfaceEmissive = 0;
Texture2D g_SurfaceEmissiveTexture;
float4 g_SurfaceEmissiveColor = 1.f;
float g_SurfaceEmissiveIntensity = 0.f;
float2 g_SurfaceEmissiveUVTiling = 1.f;
float g_SurfaceEmissiveFlickerMinimum = 0.f;
float g_SurfaceEmissiveFlickerSpeed = 0.f;
float g_SurfaceEmissivePhaseOffset = 0.f;
float g_SurfaceEmissiveTime = 0.f;
// 원본 component LOD lightmap 바인딩이다. 일반 draw는 이 값을 직접 바인딩하며
// instanced draw는 같은 scale/bias와 계수 vector를 전달한다.
#include "Shader_StaticShadowMap.hlsli"

// 평균 광량과 RNM 방향 계수는 component별 lightmap UV/scale을 사용한다.
// 환경 cube·BRDF lookup은 반사 방향/거칠기로 조회하므로 mesh UV를 그대로 쓰는 입력이 아니다.
uint g_HasBakedLighting = 0;
Texture2D g_BakedAverageTexture;
Texture2D g_BakedDirectionalTexture;
float4 g_LightmapScaleBias = float4(1.f, 1.f, 0.f, 0.f);
float4 g_LightmapAverageScale = 0.f; // RGB는 계수 배율이며 W는 사용 여부다.
float4 g_LightmapDirectionalScale = 0.f;
uint g_HasEnvironmentCube = 0;
uint g_HasEnvironmentBRDFLookup = 0;
TextureCube g_EnvironmentCubeTexture;
Texture2D g_EnvironmentBRDFLookupTexture;
float4 g_EnvironmentColor = float4(1.f, 1.f, 1.f, 0.f);
float2 g_EnvironmentRotation = float2(0.f, 1.f); // 원본 CB13의 (a,b).
uint g_UseSourcePBRIndirect = 0u;
// native source 간접광의 RGB 방향 함수를 7개의 float4에 포장한 값이다.
// MapSourcePBRSHColor의 기저·축 변환·π 배율까지 같은 계약으로 읽으며 단순 ambient RGB로 대체하지 않는다.
float4 g_SourcePBRPackedSH[7];
float3 g_SourcePBRUpperSky = 0.f;
float3 g_SourcePBRLowerSky = 0.f;
float4 g_SourcePBRAmbientAndSkyFactor = float4(0.f, 0.f, 0.f, 1.f);

sampler SurfaceLightmapSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

sampler SurfaceAnisotropicSampler = sampler_state
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = WRAP;
    AddressV = WRAP;
};

// 이름으로 식별한 diffuse 재질이 원본 Texture2D의 AddressX를 전달한다.
uint g_DiffuseMirrorU = 0;
sampler SurfaceMirrorUSampler = sampler_state
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = MIRROR;
    AddressV = WRAP;
};

sampler SurfaceMirrorVSampler = sampler_state
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = WRAP;
    AddressV = MIRROR;
};
sampler SurfaceMirrorUVSampler = sampler_state
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = MIRROR;
    AddressV = MIRROR;
};

// BG flag와 material의 mirror 설정에 맞는 sampler로 같은 diffuse SRV를 읽는다.
// SRV의 sRGB/linear 선택은 CPU resource 준비 책임이고 이 함수는 별도 gamma 변환을 하지 않는다.
float4 SampleMapDiffuseTexture(float2 uv, SamplerState wrapSampler)
{
    float4 sampleValue = 0.f;
    if (g_SurfaceProgram == 8u && (g_SourceBgFlags & 1024u) != 0u)
    {
        if (g_DiffuseMirrorU != 0u)
            sampleValue = g_DiffuseTexture.Sample(SurfaceMirrorUVSampler, uv);
        else
            sampleValue = g_DiffuseTexture.Sample(SurfaceMirrorVSampler, uv);
    }
    else if (g_DiffuseMirrorU != 0u)
        sampleValue = g_DiffuseTexture.Sample(SurfaceMirrorUSampler, uv);
    else
        sampleValue = g_DiffuseTexture.Sample(wrapSampler, uv);
    return sampleValue;
}

// family마다 다른 UV·alpha 시간 변조·flicker 식을 선택해 발광 RGB를 반환한다.
// 이 값은 간접 반사광과 구분하여 G-buffer의 emissive로 전달되므로 조명 밝기 조절값으로 대신하지 않는다.
float3 EvaluateMapSurfaceEmissive(float2 meshUV, uint surfaceProgram)
{
    if (g_HasSurfaceEmissive == 0u)
        return 0.f;
    if (surfaceProgram == 9u || surfaceProgram == 10u)
    {
        const float4 sampleValue = g_SurfaceEmissiveTexture.Sample(SurfaceAnisotropicSampler, meshUV);
        const float phase = g_SurfaceEmissivePhaseOffset + sampleValue.a *
            g_SurfaceEmissiveTime * g_SurfaceEmissiveFlickerSpeed;
        const float flicker = (g_SourceFoliageFlags & 64u) != 0u ?
            SourceFoliageFlicker(phase, g_SurfaceEmissiveFlickerMinimum) : 1.f;
        return sampleValue.rgb * g_SurfaceEmissiveColor.rgb * g_SurfaceEmissiveIntensity * flicker;
    }
    // 원본 overlay 발광은 overlay coverage와 별개이며
    // 여기서 허용한 변형에는 flicker가 없다.
    if (surfaceProgram == 7u ||
        ((surfaceProgram == 3u || surfaceProgram == 4u) && g_SourceBgFlicker == 3u))
        return g_SurfaceEmissiveTexture.Sample(SurfaceAnisotropicSampler,
            meshUV * g_SurfaceEmissiveUVTiling).rgb * g_SurfaceEmissiveColor.rgb * g_SurfaceEmissiveIntensity;
    // 원본 bg_base_pbr_opa의 중첩 cosine 변조까지 포함한 flicker 계산이다.
    // 확인하지 못한 engine-origin 상수는 phaseOffset 입력으로 명시해서 남겨 둔다.
    const float phase = g_SurfaceEmissivePhaseOffset +
        g_SurfaceEmissiveTime * g_SurfaceEmissiveFlickerSpeed;
    float flicker = 0.5f * (1.f +
        sin((phase + cos(phase * 3.524534f)) * 1.328987f)) +
        g_SurfaceEmissiveFlickerMinimum;
    if (surfaceProgram == 8u)
    {
        if (g_SourceBgFlicker == 0u)
            flicker = 1.f;
        else if (g_SourceBgFlicker == 2u)
        {
            const float ramp = 2.f * (frac(phase) - 0.5f);
            flicker = ramp * ramp + g_SurfaceEmissiveFlickerMinimum;
        }
    }
    return g_SurfaceEmissiveTexture.Sample(SurfaceAnisotropicSampler,
        meshUV * g_SurfaceEmissiveUVTiling).rgb * g_SurfaceEmissiveColor.rgb *
        g_SurfaceEmissiveIntensity * flicker;
}

// 호출자가 program을 생략하면 현재 draw에 바인딩된 g_SurfaceProgram으로 같은 계산을 사용한다.
float3 EvaluateMapSurfaceEmissive(float2 meshUV)
{
    return EvaluateMapSurfaceEmissive(meshUV, g_SurfaceProgram);
}

// 이 include의 family 번호 판정이다. 전체 프로젝트의 모든 PBR 재질이 이 두 번호를 쓰는 것은 아니다.
bool IsMapSurfacePBR()
{
    return g_SurfaceProgram == 3u || g_SurfaceProgram == 4u;
}

bool IsMapSurfaceSourceSpecular()
{
    return g_SurfaceProgram == 5u;
}

// 기하 법선의 octahedral 두 성분을 각각 11bit로 양자화해 PickPos.W의 가수에 넣는다.
// 법선은 0이 아닌 유한 입력을 전제로 한다. 위 zero helper 뒤에도 L1 길이로 나누므로 0법선 안전성을 보장하지 않는다.
// Deferred는 depth marker와 함께 이 payload를 Load로 읽는다. 선형 filtering하면 bit의 의미가 사라진다.
float EncodeMapSurfaceGeometricNormal(float3 normal, bool hasBakedLighting)
{
    normal = MapGeometryNormalizeOrZero(normal);
    normal /= abs(normal.x) + abs(normal.y) + abs(normal.z);
    float2 oct = normal.xy;
    if (normal.z < 0.f)
        oct = (1.f - abs(oct.yx)) * float2(oct.x >= 0.f ? 1.f : -1.f,
            oct.y >= 0.f ? 1.f : -1.f);
    const uint2 packed = (uint2)round(saturate(oct * 0.5f + 0.5f) * 2047.f);
    // PickPos는 RGBA32_FLOAT이며 아래 포장값의 W는 유한한 0 이외의 값이다. CPicking은
    // XYZ를 사용하고 반환하는 동차좌표의 W를 1로 복원한다. light pass는 depth marker 3일 때만
    // 이 22개 가수 비트의 포장값을 해석한다.
    // 가수 bit 22는 실제 component lightmap을 표시한다. 이때 baked irradiance가
    // scene 값을 편집하지 않고 기존 프로젝트의 ambient 가산을 대체한다.
    return asfloat(0x3f800000u | packed.x | (packed.y << 11u) |
        (hasBakedLighting ? 0x00400000u : 0u));
}

// 거의 0인 밑을 먼저 0으로 처리한다. 따라서 metallic intensity=0, power=0도 pow(0,0) 대신 0이다.
// 음수의 부호는 abs로 버리며 일반적인 유효성 검사나 지수 범위 제한까지 하지는 않는다.
float MapSurfaceSafePow(float value, float power)
{
    // metallic intensity=power=0인 경우를 포함해 원본 log/exp 분기에 맞춘다.
    return abs(value) < 0.000001f ? 0.f : pow(abs(value), power);
}

// 한 pixel의 계산 결과다. worldNormal과 tangentNormal은 서로 다른 공간이며 specular의 의미도 family별로 다르다.
// PBR에서는 specular=정면 반사색 F0, roughness/metallic/AO는 Deferred와 간접광에 전달할 재질값이다.
struct MAP_SURFACE_SAMPLE
{
    float4 diffuse;
    float3 worldNormal;
    float3 geometricNormal; // Program 14의 포장된 Landscape height-normal 입력이다.
    float3 landscapeTangent;
    float3 landscapeBinormal;
    float3 tangentNormal;
    float3 specular;
    float3 reflectionDelta;
    float3 transmission;
    float3 subspecularRadiance;
    float3 rimlightRadiance;
    float specularPower;
    float roughness;
    float metallic;
    float ambientOcclusion;
};

#include "Shader_SourceSpecialSurface.hlsli"
#include "Shader_SourceLandscapeSurface.hlsli"

bool IsMapSurfaceSourceSpecial()
{
    return g_SurfaceProgram >= 11u && g_SurfaceProgram <= 14u;
}

bool IsMapSurfaceSourceLandscape()
{
    return g_SurfaceProgram == 14u;
}

bool IsMapSurfaceSourceFoliage()
{
    return g_SurfaceProgram == 9u || g_SurfaceProgram == 10u;
}

// 잎/풀의 alpha clip, normal, 반사색과 transmission을 만든다. ORM/PBR 공식을 적용하는 경로가 아니다.
// 풀은 diffuse를 specular와 transmission에 재사용하며 flag가 꺼진 잎의 입력은 0으로 남긴다.
MAP_SURFACE_SAMPLE EvaluateMapSourceFoliageSurface(float2 uv, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    const bool grass = g_SurfaceProgram == 10u;
    const float4 raw = g_DiffuseTexture.Sample(SurfaceAnisotropicSampler, uv);
    clip(raw.a - 0.3333f);
    const float4 mask = grass ? 1.f : g_SourceFoliageMaskTexture.Sample(SurfaceAnisotropicSampler, uv);
    float3 diffuse = (g_SourceFoliageFlags & 2u) != 0u ?
        SourceFoliageDesaturate(raw.rgb, g_SurfaceDiffuseSaturation, grass ? 1.f : mask.a) : raw.rgb;
    const float3 specular = grass ? diffuse : ((g_SourceFoliageFlags & 8u) != 0u ?
        g_SpecularTexture.Sample(SurfaceAnisotropicSampler, uv).rgb : mask.rrr);
    diffuse *= grass ? g_SurfaceDiffuseBrightness * g_SurfaceDiffuseColor.rgb :
        1.f + mask.a * (g_SurfaceDiffuseBrightness * g_SurfaceDiffuseColor.rgb - 1.f);
    result.diffuse = float4(diffuse, raw.a);
    result.tangentNormal = (g_SourceFoliageFlags & 1u) != 0u ?
        SourceFoliageNormal(g_NormalTexture.Sample(SurfaceAnisotropicSampler, uv).rg,
            g_SurfaceNormalIntensity) : float3(0.f, 0.f, 1.f);
    const float3x3 tangentToWorld = float3x3(SourceFoliageUnit(tangent),
        SourceFoliageUnit(binormal), SourceFoliageUnit(normal));
    result.worldNormal = SourceFoliageUnit(mul(result.tangentNormal, tangentToWorld));
    result.specular = (g_SourceFoliageFlags & 4u) != 0u ?
        specular * g_SurfaceSpecularColor.rgb * g_SurfaceSpecularIntensity : 0.f;
    result.transmission = grass ? g_SourceFoliageTransmission.rgb :
        ((g_SourceFoliageFlags & 16u) != 0u ?
            mask.b * g_SourceFoliageTransmission.rgb * g_SourceFoliageTransmission.w : 0.f);
    result.ambientOcclusion = 1.f;
    return result;
}

bool IsMapSurfaceSourceBG()
{
    return g_SurfaceProgram == 8u;
}

// UV 중심 0.5를 기준으로 회전한 뒤 tiling·offset·시간 panning을 순서대로 적용한다.
// g_SourceBgUV.xy는 회전 성분이고 g_SurfaceEmissiveTime을 UV 이동에도 공유한다.
float2 MapSourceBGUV(float2 uv)
{
    const float2 centered = uv - 0.5f;
    return (float2(centered.x * g_SourceBgUV.y - centered.y * g_SourceBgUV.x,
        centered.y * g_SourceBgUV.y + centered.x * g_SourceBgUV.x) + 0.5f) *
        g_SurfaceUVTiling + g_SourceBgUV.zw + g_SourceBgPanning * g_SurfaceEmissiveTime;
}

// 먼저 읽은 alpha를 높이처럼 사용해 tangentView.xy 방향으로 UV를 이동하는 선택적 bump 경로다.
// 이동 전/후 alpha의 조합이 clip과 bumpShade에 쓰이므로 alpha를 항상 최종 투명도로 해석하면 안 된다.
float4 MapSourceBGDiffuse(float2 meshUV, float3 tangentView, out float2 surfaceUV, out float bumpShade)
{
    surfaceUV = MapSourceBGUV(meshUV);
    const float4 original = SampleMapDiffuseTexture(surfaceUV, SurfaceAnisotropicSampler);
    float4 shifted = original;
    bumpShade = 1.f;
    if ((g_SourceBgFlags & 2u) != 0u)
    {
        surfaceUV += (original.a - g_SourceBgBump.x) * g_SourceBgBump.y * tangentView.xy;
        shifted = SampleMapDiffuseTexture(surfaceUV, SurfaceAnisotropicSampler);
        bumpShade = saturate(original.a + g_SourceBgBump.z);
    }
    if ((g_SourceBgFlags & 64u) != 0u)
        clip(((g_SourceBgFlags & 2u) != 0u ? saturate(original.a + shifted.a) : shifted.a) - 0.3333f);
    return shifted;
}

// BG flag별 normal/detail·RGB specular·2D reflection·subspecular·rim 값을 만든다.
// reflection texture로 base diffuse를 보정하는 이 경로와 아래 cube/BRDF 기반 환경 반사는 별개다.
MAP_SURFACE_SAMPLE EvaluateMapSourceBGSurface(float2 meshUV, float4 vertexColor, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    result.ambientOcclusion = 1.f;
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    const float3 tangentView = normalize(mul(tangentToWorld, normalize(g_vCamPosition.xyz - worldPosition)));
    float2 surfaceUV; float bumpShade;
    const float4 diffuse = MapSourceBGDiffuse(meshUV, tangentView, surfaceUV, bumpShade);
    float4 encodedNormal = float4(0.5f, 0.5f, 1.f, 1.f);
    float3 tangentNormal = float3(0.f, 0.f, 1.f);
    if ((g_SourceBgFlags & 1u) != 0u)
    {
        const float2 normalUV = (g_SourceBgFlags & 16384u) != 0u ? meshUV : surfaceUV;
        if ((g_SourceBgFlags & 6144u) == 6144u)
            encodedNormal = g_NormalTexture.Sample(SurfaceMirrorUVSampler, normalUV);
        else if ((g_SourceBgFlags & 2048u) != 0u)
            encodedNormal = g_NormalTexture.Sample(SurfaceMirrorUSampler, normalUV);
        else if ((g_SourceBgFlags & 4096u) != 0u)
            encodedNormal = g_NormalTexture.Sample(SurfaceMirrorVSampler, normalUV);
        else
            encodedNormal = g_NormalTexture.Sample(SurfaceAnisotropicSampler, normalUV);
        const float2 raw = encodedNormal.rg * 2.f - 1.f;
        const float vertexStrength = (g_SourceBgFlags & 128u) != 0u ? vertexColor.a : 1.f;
        float2 detail = 0.f;
        [branch] if ((g_SourceBgFlags & 32768u) != 0u)
            detail = (g_DetailNormalTexture.Sample(SurfaceAnisotropicSampler,
                surfaceUV * g_SurfaceDetailNormalTiling).rg * 2.f - 1.f) *
                g_SurfaceDetailNormalIntensity;
        tangentNormal = normalize(float3((raw * g_SurfaceNormalIntensity + detail) * vertexStrength,
            sqrt(max(1.f - dot(raw, raw), 0.f)) + 0.00001f));
    }
    result.tangentNormal = tangentNormal;
    result.worldNormal = normalize(mul(tangentNormal, tangentToWorld));
    const float luminance = dot(diffuse.rgb, float3(0.3f, 0.59f, 0.11f));
    const float3 baseDiffuse = lerp(luminance.xxx, diffuse.rgb, g_SurfaceDiffuseSaturation) *
        g_SurfaceDiffuseColor.rgb * g_SurfaceDiffuseBrightness * bumpShade;
    result.diffuse = float4(baseDiffuse, diffuse.a);
    float3 subspecularColor = 0.f;
    if ((g_SourceBgFlags & 4u) != 0u || g_SourceBgSubspecular.x != 0.f)
    {
        float3 sampledSpecular = diffuse.rgb;
        [branch] if ((g_SourceBgFlags & 8u) != 0u)
            sampledSpecular = g_SpecularTexture.Sample(SurfaceAnisotropicSampler, surfaceUV).rgb;
        const float3 specular = lerp(dot(sampledSpecular, float3(0.3f, 0.59f, 0.11f)).xxx,
            sampledSpecular, g_SourceBgSpecularSaturation);
        subspecularColor = ((g_SourceBgFlags & 256u) != 0u && (g_SourceBgFlags & 8u) == 0u ?
            dot(specular, float3(0.3f, 0.59f, 0.11f)).xxx : specular) *
            g_SurfaceSpecularColor.rgb * g_SurfaceSpecularIntensity;
        if ((g_SourceBgFlags & 4u) != 0u)
            result.specular = subspecularColor;
    }
    if ((g_SourceBgFlags & 16u) != 0u)
    {
        const float3 reflected = reflect(-tangentView, tangentNormal);
        float2 reflectionUV;
        if ((g_SourceBgFlags & 32u) != 0u)
        {
            const float3 worldReflection = mul(reflected, tangentToWorld);
            reflectionUV = float2(worldReflection.x, -worldReflection.z) * g_SurfaceReflectionTiling + g_SurfaceReflectionOriginOffset;
        }
        else
            reflectionUV = lerp(MapSourceBGUV(meshUV), (reflected.xy + 0.5f) * 0.5f, 0.75f);
        const float3 sampled = g_ReflectionTexture.Sample(SurfaceAnisotropicSampler, reflectionUV).rgb;
        const float3 signedReflection = sampled * g_SurfaceReflectionColor.rgb *
            (1.f + g_SurfaceReflectionContrast) - g_SurfaceReflectionContrast;
        const float weight = ((g_SourceBgFlags & 1u) != 0u ? encodedNormal.a : 1.f) * g_SurfaceReflectionIntensity;
        result.diffuse.rgb = (g_SourceBgFlags & 8192u) != 0u ?
            lerp(baseDiffuse, sampled * g_SurfaceReflectionColor.rgb, weight) :
            clamp((baseDiffuse + weight * saturate(signedReflection)) *
            (1.f - weight * saturate(-signedReflection)), 0.f, 999.f);
        result.reflectionDelta = abs(result.diffuse.rgb - baseDiffuse);
    }
    const float facing = saturate(dot(tangentNormal, tangentView));
    result.subspecularRadiance = subspecularColor * g_SourceBgSubspecular.x *
        MapSurfaceSafePow(facing * facing, g_SourceBgSubspecular.y);
    const float rim = (1.f - abs(dot(tangentNormal, tangentView))) * (1.f - abs(tangentView.z));
    result.rimlightRadiance = g_SourceBgRimlight.rgb * MapSurfaceSafePow(rim, g_SourceBgRimlight.w);
    return result;
}

// program 5의 RGB specular와 world-XZ 기반 2D reflection 경로다.
// roughness/metallic은 사용하지 않고 직접광에서 native Blinn 지수를 읽는다.
MAP_SURFACE_SAMPLE EvaluateMapSourceSpecularSurface(float2 meshUV, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    // 실제 MAGICFLOOR component MIC는 타일링하는 이 불투명 원본 변형을 선택한다.
    // 이 변형은 기존 program-1 호환 경로와 별개다.
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    const float2 uv = meshUV * g_SurfaceUVTiling;
    const float4 diffuse = g_DiffuseTexture.Sample(SurfaceAnisotropicSampler, uv);
    const float4 encodedNormal = g_NormalTexture.Sample(SurfaceAnisotropicSampler, uv);
    const float2 xy = encodedNormal.rg * 2.f - 1.f;
    const float z = sqrt(max(1.f - dot(xy, xy), 0.f)) + 0.00001f;
    result.tangentNormal = normalize(float3(xy * g_SurfaceNormalIntensity, z));
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    result.worldNormal = normalize(mul(result.tangentNormal, tangentToWorld));
    const float3 view = normalize(mul(tangentToWorld, normalize(g_vCamPosition.xyz - worldPosition)));
    const float3 reflected = mul(reflect(-view, result.tangentNormal), tangentToWorld);
    const float2 reflectionUV = float2(reflected.x, -reflected.z) *
        g_SurfaceReflectionTiling + g_SurfaceReflectionOriginOffset;
    const float3 reflection = g_ReflectionTexture.Sample(SurfaceAnisotropicSampler, reflectionUV).rgb;
    const float3 q = reflection * g_SurfaceReflectionColor.rgb *
        (1.f + g_SurfaceReflectionContrast) - g_SurfaceReflectionContrast;
    const float w = encodedNormal.a * g_SurfaceReflectionIntensity;
    const float3 base = diffuse.rgb * g_SurfaceDiffuseColor.rgb * g_SurfaceDiffuseBrightness;
    result.diffuse = float4(clamp((base + w * saturate(q)) * (1.f - w * saturate(-q)), 0.f, 999.f), 1.f);
    result.reflectionDelta = abs(result.diffuse.rgb - base);
    result.specular = g_SpecularTexture.Sample(SurfaceAnisotropicSampler, uv).rgb *
        g_SurfaceSpecularColor.rgb * g_SurfaceSpecularIntensity;
    result.roughness = 0.f;
    result.metallic = 0.f;
    result.ambientOcclusion = 1.f;
    return result;
}

// program 3/4의 표면 단계다. texture → tangent normal → world normal → albedo/F0/ORM 결과 순서다.
// 입력 worldPosition·T/B/N·카메라 위치는 같은 runtime world 좌표를 사용한다. 광원 합산은 아직 하지 않는다.
MAP_SURFACE_SAMPLE EvaluateMapPBRSurface(float2 meshUV, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    const float2 tiledUV = meshUV * g_SurfaceUVTiling;
    const float2 normalUV = g_SurfaceUVFixedNormal != 0u ? meshUV : tiledUV;
    const float4 diffuse = g_DiffuseTexture.Sample(SurfaceAnisotropicSampler, tiledUV);
    // masked PBR만 diffuse alpha 0.3333으로 clip한다.
    // 통과한 표면의 result.diffuse.a는 아래에서 1로 기록한다.
    if (g_SurfacePBRMasked != 0u)
        clip(diffuse.a - 0.3333f);
    const float4 encodedNormal = g_NormalTexture.Sample(SurfaceAnisotropicSampler, normalUV);

    // normal RG를 [0,1]에서 [-1,1]로 풀고 Z를 sqrt(1-x²-y²)로 복원한다.
    // detail/강도/vertex alpha는 XY에만 적용한 뒤 정규화한다. roughness가 normal map의 높이를 뜻하지는 않는다.
    const float2 detailXY = g_DetailNormalTexture.Sample(SurfaceAnisotropicSampler,
        tiledUV * g_SurfaceDetailNormalTiling).rg * 2.f - 1.f;
    const float2 rawXY = encodedNormal.rg * 2.f - 1.f;
    const float rawZ = sqrt(max(1.f - dot(rawXY, rawXY), 0.f)) + 0.00001f;
    float3 tangentNormal = normalize(float3((rawXY * g_SurfaceNormalIntensity +
        detailXY * g_SurfaceDetailNormalIntensity) * g_SurfaceVertexAlpha *
        max(g_MapPBRDiagnosticParameters.x, 0.f), rawZ));
    // seamless 변형은 거의 평평한 법선의 잡음을 명시적으로 제거한다.
    if (g_SurfaceProgram == 3u && tangentNormal.z >= 0.99985f)
        tangentNormal = float3(0.f, 0.f, 1.f);
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    result.tangentNormal = tangentNormal;
    result.worldNormal = normalize(mul(tangentNormal, tangentToWorld));
    const float3 worldView = normalize(g_vCamPosition.xyz - worldPosition);
    const float3 tangentView = normalize(mul(tangentToWorld, worldView));
    const float3 tangentReflection = reflect(-tangentView, tangentNormal);
    float2 reflectionUV;
    if (g_SurfaceUseWorldReflection != 0u)
    {
        const float3 worldReflection = mul(tangentReflection, tangentToWorld);
        reflectionUV = float2(worldReflection.x, -worldReflection.z) *
            g_SurfaceReflectionTiling + g_SurfaceReflectionOriginOffset;
    }
    else
        reflectionUV = lerp(meshUV, (tangentReflection.xy + 0.5f) * 0.5f, 0.75f);

    const float luminance = dot(diffuse.rgb, float3(0.3f, 0.59f, 0.11f));
    const float3 baseDiffuse = lerp(luminance.xxx, diffuse.rgb,
        g_SurfaceDiffuseSaturation) * g_SurfaceDiffuseColor.rgb * g_SurfaceDiffuseBrightness;
    const float3 reflection = g_ReflectionTexture.Sample(SurfaceAnisotropicSampler, reflectionUV).rgb;
    const float3 signedReflection = reflection * g_SurfaceReflectionColor.rgb *
        (1.f + g_SurfaceReflectionContrast) - g_SurfaceReflectionContrast;
    const float weight = encodedNormal.a * g_SurfaceReflectionIntensity;
    const float3 reflectedDiffuse = clamp((baseDiffuse + weight * saturate(signedReflection)) *
        (1.f - weight * saturate(-signedReflection)), 0.f, 999.f);
    result.reflectionDelta = abs(reflectedDiffuse - baseDiffuse);

    // metallic은 비금속↔금속 반사 구성의 혼합값이다. roughness r은 뒤 직접광에서 α=r², D의 α²=r⁴로 쓰인다.
    // 현재 r의 하한을 먼저 적용한 뒤 diagnostic offset을 더하므로 음수 offset은 그 하한보다 낮출 수 있다.
    // AO는 재질에 저장된 차폐 입력이며 화면 깊이로 계산하는 SSAO와 다른 값이다.
    const float3 orm = g_SurfaceORMTexture.Sample(SurfaceAnisotropicSampler, tiledUV).rgb;
    result.metallic = saturate(MapSurfaceSafePow(orm.b * g_SurfaceMetallicIntensity,
        g_SurfaceMetallicPower));
    const float minimumRoughness = g_UseSourcePBRIndirect != 0u ?
        0.045f : g_SurfaceMinimumRoughness;
    result.roughness = saturate(max(minimumRoughness,
        MapSurfaceSafePow(orm.g * clamp(g_SurfaceRoughnessIntensity, 0.f, 100.f),
            g_SurfaceRoughnessPower)) + g_MapPBRDiagnosticParameters.y);
    result.ambientOcclusion = saturate(MapSurfaceSafePow(orm.r * g_SurfaceAOIntensity,
        g_SurfaceAOPower));
    // 이 표면 단계는 hit/selection과 전역 material 보정을 비활성 항등값으로 둔다.
    // component lightmap과 연결된 native 환경 lookup은 아래 간접광 단계에서 평가한다.
    const float3 albedo = saturate(reflectedDiffuse * lerp(g_SurfaceNonmetallicBrightness,
        g_SurfaceMetallicBrightness, result.metallic));
    result.diffuse = float4(albedo, 1.f); // 원본 불투명 변형에서는 alpha-clip을 수행하지 않는다.

    // 정면 반사색 F0다. 비금속은 0.08×specularPBRIntensity, 금속은 albedo로 보간한다.
    // 예를 들어 intensity=0.5, metallic=0이면 F0=0.04이며 이것이 최종 밝기 4% 고정이라는 뜻은 아니다.
    // 시선/광원 각도에 따른 Fresnel은 Deferred 직접광과 아래 BRDF lookup에서 따로 계산한다.
    result.specular = lerp((0.08f * saturate(g_SurfaceSpecularPBRIntensity)).xxx,
        albedo, result.metallic);
    return result;
}

// AO와 색으로 계산하는 다항 근사다. 어두워진 영역의 색 의존 보정을 위해 단순 AO 곱과 다르게 쓴다.
// 실제 geometry 사이의 광선을 추적하거나 여러 반사를 새로 적분하는 함수는 아니다.
float3 MapSourceAOMultiBounce(float ao, float3 color)
{
    // 원본 BasePass의 다항식이다. 일반적인 AO 단순 곱과 구분한다.
    const float3 a = 2.0404f * color - 0.3324f;
    const float3 b = -4.7951f * color + 0.6417f;
    const float3 c = 2.7552f * color + 0.6903f;
    return max(ao.xxx, ((ao * a + b) * ao + c) * ao);
}

// tangent 공간의 세 RNM 기저 방향과 내적하고 [0,1]로 제한해 제곱한 뒤 저장 계수로 합한다.
// 같은 lightmap도 normal map의 방향에 따라 밝기가 바뀌게 하며, SH의 9개 기저와는 다른 표현이다.
float MapSourceDirectionalLightmapWeight(float3 tangentDirection, float3 coefficients)
{
    float3 weights = saturate(float3(
        dot(tangentDirection.yz, float2(0.81649658f, 0.57735027f)),
        dot(tangentDirection, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
        dot(tangentDirection, float3(0.70710678f, -0.40824829f, 0.57735027f))));
    return dot(coefficients, weights * weights);
}

// 일반 draw와 instanced texture-bank draw가 같은 RNM 수식을 소비한다.
// 이미 읽고 scale을 곱한 RNM 값을 ordinary/texture-bank draw가 함께 소비한다.
// diffuse와 선택적 반사 lobe를 색까지 포함해 반환하므로 합성 단계에서 albedo를 다시 곱하지 않는다.
float3 EvaluateMapSourceBGIndirectSamples(MAP_SURFACE_SAMPLE surface,
    float3 average, float3 coefficients, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal, bool sourceSpecial)
{
    float3 radiance = surface.diffuse.rgb * average *
        MapSourceDirectionalLightmapWeight(surface.tangentNormal, coefficients);
    if (sourceSpecial || (g_SourceBgFlags & 4u) != 0u)
    {
        // Landscape의 VS varying은 변환 축을 전달하며 height texture가
        // 원본 RNM specular 정책에 사용할 pixel별 지형 좌표축을 제공한다.
        const float3x3 tangentToWorld = g_SurfaceProgram == 14u ?
            float3x3(surface.landscapeTangent, surface.landscapeBinormal, surface.geometricNormal) :
            float3x3(MapGeometryNormalizeOrZero(tangent),
                MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
        const float3 tangentView = normalize(mul(tangentToWorld, g_vCamPosition.xyz - worldPosition));
        const float3 reflected = reflect(-tangentView, surface.tangentNormal);
        const float3 lobes = saturate(float3(
            dot(reflected.yz, float2(0.81649658f, 0.57735027f)),
            dot(reflected, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
            dot(reflected, float3(0.70710678f, -0.40824829f, 0.57735027f))));
        radiance += surface.specular * average *
            dot(coefficients, pow(lobes,
                (sourceSpecial ? surface.specularPower : g_SurfaceSpecularPower) + 1.f));
    }
    return radiance + surface.subspecularRadiance;
}

// lightmap을 사용할 수 없으면 subspecular만 반환한다. 사용 가능하면 두 texture를 읽어 공통 RNM 식에 넘긴다.
float3 EvaluateMapSourceBGIndirectLighting(MAP_SURFACE_SAMPLE surface, float2 lightmapUV,
    float4 averageScale, float4 directionalScale, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal, bool sourceSpecial)
{
    if (g_HasBakedLighting == 0u || averageScale.w == 0.f)
        return surface.subspecularRadiance;
    const float3 average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * averageScale.rgb;
    const float3 coefficients = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * directionalScale.rgb;
    return EvaluateMapSourceBGIndirectSamples(surface, average, coefficients,
        worldPosition, tangent, binormal, normal, sourceSpecial);
}

// EFEngine 원본 packer 0x5f0c30과 정확한 FLOOR12_01 BasePass에 대응한다.
// world normal을 cube와 같은 축/회전 계약의 q로 바꾸고 저차 SH 다항식을 평가한다.
// products=(xy,yz,z²,xz), 별도 (x²-y²)와 상수/일차항을 7개 float4에 나눠 저장한다.
// 계수는 이미 해당 기저에 맞게 포장되어 있다. 여기서 다시 SH 정규화 계수나 E/π를 임의로 곱하면 다른 식이 된다.
// 이 native 경로의 끝은 max(합,0)×환경 RGB×π+환경 W이며 Deferred의 project diffuse SH와 배율이 다르다.
float3 MapSourcePBRSHColor(float3 worldNormal)
{
    const float a = g_EnvironmentRotation.x, b = g_EnvironmentRotation.y;
    const float4 q = float4(b * worldNormal.x + a * worldNormal.z,
        worldNormal.y, a * worldNormal.x - b * worldNormal.z, 1.f);
    const float4 products = q.yzzx * q.xyzz;
    const float3 first = float3(dot(g_SourcePBRPackedSH[0], q),
        dot(g_SourcePBRPackedSH[1], q), dot(g_SourcePBRPackedSH[2], q));
    const float3 second = float3(dot(g_SourcePBRPackedSH[3], products),
        dot(g_SourcePBRPackedSH[4], products), dot(g_SourcePBRPackedSH[5], products));
    return max(first + second + g_SourcePBRPackedSH[6].rgb * (q.x * q.x - q.y * q.y), 0.f) *
        g_EnvironmentColor.rgb * 3.14159265358979323846f + g_EnvironmentColor.w;
}

// world 위쪽을 tangent 공간으로 옮긴 tangentUp과 방향의 내적으로 상/하 sky 가중치를 만든다.
// (0.5±0.5d)를 각각 제곱하므로 단순 선형 sky lerp와 같지 않다. 입력 단위 방향은 호출 경로가 준비한다.
float3 MapSourcePBRHemisphere(float3 tangentDirection, float3 tangentUp)
{
    const float d = dot(MapGeometryNormalizeOrZero(tangentUp), tangentDirection);
    const float2 weight = float2(0.5f + 0.5f * d, 0.5f - 0.5f * d);
    return (weight.x * weight.x * g_SourcePBRUpperSky +
        weight.y * weight.y * g_SourcePBRLowerSky) *
        g_SourcePBRAmbientAndSkyFactor.w;
}

// family에 맞는 간접광을 선택한다. PBR 반환값은 diffuse/ambient와 environmentSpecular를 이미 합친 RGB다.
// out environmentSpecular는 진단용 별도 성분이고, 호출자가 이를 반환값에 다시 더하면 중복된다.
// PBR에서는 RT7에 합계, RT6에 환경 specular를 기록하고 Deferred Combine이 RT7을 한 번 더한다.
float3 EvaluateMapSourceIndirectLighting(MAP_SURFACE_SAMPLE surface, float2 lightmapUV,
    float4 averageScale, float4 directionalScale, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal, out float3 environmentSpecular)
{
    environmentSpecular = 0.f;
    if (IsMapSurfaceSourceFoliage())
    {
        const bool grass = g_SurfaceProgram == 10u;
        float3 radiance = grass ? surface.diffuse.rgb * surface.transmission : 0.f;
        if (g_HasBakedLighting == 0u || averageScale.w == 0.f)
            return radiance;
        const float3 average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * averageScale.rgb;
        const float3 coefficients = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * directionalScale.rgb;
        const float3x3 tangentToWorld = float3x3(SourceFoliageUnit(tangent), SourceFoliageUnit(binormal), SourceFoliageUnit(normal));
        const float3 tangentView = mul(tangentToWorld, g_vCamPosition.xyz - worldPosition);
        return radiance + SourceFoliageBaked(surface.diffuse.rgb, surface.tangentNormal,
            surface.specular, surface.transmission, tangentView, average, coefficients,
            g_SurfaceSpecularPower, grass);
    }
    if (IsMapSurfaceSourceBG() || IsMapSurfaceSourceSpecial())
        return EvaluateMapSourceBGIndirectLighting(surface, lightmapUV,
            averageScale, directionalScale, worldPosition, tangent, binormal, normal,
            IsMapSurfaceSourceSpecial());
    if (IsMapSurfaceSourceSpecular())
    {
        // 원본 non-PBR BasePass는 UE 월드 좌표에서 세 반사 lobe에 거듭제곱을 적용해 더한다.
        // 이것은 원본 재질의 동작이며 IBL과 구분한다.
        const float3 reflectedWorld = reflect(-normalize(g_vCamPosition.xyz - worldPosition),
            surface.worldNormal);
        const float3 reflectedUE = float3(reflectedWorld.x, -reflectedWorld.z, reflectedWorld.y);
        const float3 dots = min(abs(float3(
            dot(reflectedUE.yz, float2(0.81649658f, 0.57735027f)),
            dot(reflectedUE, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
            dot(reflectedUE, float3(0.70710678f, -0.40824829f, 0.57735027f)))), 1.f);
        const float3 powered = pow(dots, g_SurfaceSpecularPower);
        float3 indirect = surface.specular * (powered.x + powered.y + powered.z);
        if (g_HasBakedLighting != 0u && averageScale.w != 0.f)
        {
            const float3 average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler,
                lightmapUV).rgb * averageScale.rgb;
            const float3 direction = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler,
                lightmapUV).rgb * directionalScale.rgb;
            indirect += surface.diffuse.rgb * average *
                MapSourceDirectionalLightmapWeight(surface.tangentNormal, direction);
        }
        // 원본 hemisphere, hit/selection 및 전역 CB 색은 효과를 더하지 않는 기본값으로 남겨 둔다.
        // PBR의 AO/cube/lookup 입력은 이 재질 계열에 속하지 않는다.
        return indirect;
    }
    if (!IsMapSurfacePBR())
        return 0.f;

    // PBR만 이 아래로 온다. native 간접광과 component lightmap이 모두 없으면 0이다.
    // CPU는 native material 입력과 scene 선택을 함께 확인해 g_UseSourcePBRIndirect를 정한다.
    const bool nativeIndirect = g_UseSourcePBRIndirect != 0u;
    const bool hasLightmap = g_HasBakedLighting != 0u && averageScale.w != 0.f;
    if (!hasLightmap && !nativeIndirect)
        return 0.f;
    float3 average = 0.f, direction = 0.f;
    if (hasLightmap)
    {
        average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * averageScale.rgb;
        direction = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler, lightmapUV).rgb * directionalScale.rgb;
    }
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    const float3 tangentUp = mul(tangentToWorld, float3(0.f, 1.f, 0.f));
    const float3 normalHemisphere = nativeIndirect ?
        MapSourcePBRHemisphere(surface.tangentNormal, tangentUp) : 0.f;
    const float3 ambient = nativeIndirect ?
        surface.diffuse.rgb * g_SourcePBRAmbientAndSkyFactor.rgb : 0.f;
    const float3 n = surface.tangentNormal;
    const float normalIrradiance = MapSourceDirectionalLightmapWeight(n, direction);
    // 원본 directional-lightmap PBR diffuse에는 metallic 마스크와
    // reflection-BRDF 에너지 가중치가 함께 들어간다. 이전 부분 복원식은
    // 두 항을 모두 생략했으므로 그 이전 식을 명시적으로 비교하는 경로를 남긴다.
    const bool legacyUnbalancedRNM = g_MapPBRDiagnosticParameters.z > 0.5f;

    // RNM 법선 방향 광량과 hemisphere를 먼저 합하고 albedo·색 의존 AO를 곱한다.
    // native일 때만 SH를 추가로 곱한다. legacyUnbalancedRNM이 꺼져 있으면 indirect에 (1-metallic)을 곱한다.
    // 별도 ambient 항은 이 metallic 마스크 밖이므로 metallic=1의 모든 간접 diffuse가 제거된다고 일반화하지 않는다.
    float3 indirect = surface.diffuse.rgb * (average * normalIrradiance + normalHemisphere) *
        MapSourceAOMultiBounce(surface.ambientOcclusion, surface.diffuse.rgb);
    if (nativeIndirect)
        indirect *= MapSourcePBRSHColor(surface.worldNormal);
    if (!legacyUnbalancedRNM)
        indirect *= 1.f - surface.metallic;
    // 복원되어 바인딩된 lookup이 없는 행은 그 입력을 미해결 상태로 유지한다.
    // Fresnel 값이나 environment texture를 임의로 만들어 대체하지 않는다.
    if (g_HasEnvironmentCube == 0u || g_HasEnvironmentBRDFLookup == 0u)
        return (indirect + ambient) * g_MapPBRContributionScale.z;
    const float3 worldView = normalize(g_vCamPosition.xyz - worldPosition);
    const float3 tangentView = normalize(mul(tangentToWorld, worldView));

    // V는 표면→카메라, reflect(-V,N)은 표면을 떠나는 환경 조회 방향이다.
    // lookupView는 단순 saturate(N·V)가 아니라 reflection.z로 보정한 원본 조회 좌표다.
    const float normalView = dot(n, tangentView);
    const float3 reflection = reflect(-tangentView, n);
    const float lookupView = saturate(normalView + 1.f - min(reflection.z + 1.f, 1.f));

    // ddx/ddy는 시간 변화가 아니라 인접 pixel quad의 공간 변화다. N·V가 급변할수록 roughness를 올려 반사 aliasing을 완화한다.
    // 이 보정 r은 BRDF 조회와 cube mip에 쓰이고, 아래 RNM 방향 혼합은 원래 surface.roughness를 쓴다.
    const float2 normalViewGradient = float2(ddx_coarse(normalView), ddy_coarse(normalView));
    const float roughnessAA = saturate(surface.roughness + 0.3f * length(normalViewGradient));
    // 원본 lookup은 R=Fresnel, G=base로 저장하며 그 내용은 재질이 소유한다.
    // 따라서 기존 비교용 asset은 원래 프로젝트 lookup을 유지할 수 있다.
    const float2 lookup = g_EnvironmentBRDFLookupTexture.Sample(SurfaceLightmapSampler,
        float2(lookupView, roughnessAA)).rg;
    const float3 fresnelDelta = saturate(50.f * surface.specular.g) *
        (max((1.f - roughnessAA).xxx, surface.specular) - surface.specular);

    // 조회표 G×F0 + R×fresnelDelta에 F0/lookup.G 기반 보상을 곱하는 현재 식이다.
    // 일반적인 split-sum LUT의 채널 의미를 그대로 가져오지 않는다. lookup은 mesh UV가 아닌 (lookupView,roughnessAA)로 읽는다.
    // 다음 (1-reflectionBRDF)는 diffuse 쪽 반사 에너지를 분리하는 근사이며 임의 입력의 엄밀한 에너지 보존 검증은 아니다.
    const float3 reflectionBRDF = (lookup.g * surface.specular + lookup.r * fresnelDelta) *
        (1.f + surface.specular * (rcp(max(lookup.g, 0.000001f)) - 1.f));
    if (!legacyUnbalancedRNM)
        indirect *= 1.f - reflectionBRDF;

    const float3 worldReflection = mul(reflection, tangentToWorld);
    // UE->runtime 변환은 (x,z,-y)다. 원본 CB13은 cube의 XZY sampling 순열 전에 UE XY를 회전한다.
    // CPU binder는 (sin(angle),cos(angle))을 저장한다.
    const float a = g_EnvironmentRotation.x, b = g_EnvironmentRotation.y;
    const float3 cubeDirection = float3(b * worldReflection.x + a * worldReflection.z,
        worldReflection.y, a * worldReflection.x - b * worldReflection.z);

    // roughnessAA×5가 명시적 mip LOD다. roughness를 최종 색에 직접 곱해 어둡게 만드는 식이 아니다.
    // RGBM6 저장값을 RGB×A×6으로 풀고 환경 RGB/W 보정을 적용한다. mip의 사전 필터링 품질은 입력 cube 준비 책임이다.
    const float4 encodedEnvironment = g_EnvironmentCubeTexture.SampleLevel(
        SurfaceLightmapSampler, cubeDirection, roughnessAA * 5.f);
    const float3 environment = encodedEnvironment.rgb * encodedEnvironment.a *
        g_EnvironmentColor.rgb * 6.f + g_EnvironmentColor.w;
    const float reflectedIrradiance = MapSourceDirectionalLightmapWeight(reflection, direction);
    const float3 specularIrradiance = hasLightmap ? normalHemisphere +
        average * lerp(reflectedIrradiance, normalIrradiance, surface.roughness) :
        MapSourcePBRHemisphere(reflection, tangentUp);

    // AO와 보정된 시선 좌표로 환경 specular의 차폐를 근사한다. 직접광 shadow map이나 SSR 교차 판정은 아니다.
    const float specularAO = saturate(MapSurfaceSafePow(surface.ambientOcclusion + lookupView,
        roughnessAA * roughnessAA) + surface.ambientOcclusion - 1.f);
    const float f0Luminance = dot(surface.specular, float3(0.3f, 0.59f, 0.11f));

    // 환경 RGB × RNM/hemisphere 광량 × reflectionBRDF × specular AO × 비교 배율이다.
    // 환경맵이 연결됐다는 사실만으로 이 모든 곱의 결과가 0이 아님을 보장하지는 않는다.
    environmentSpecular = environment * specularIrradiance * reflectionBRDF *
        MapSourceAOMultiBounce(specularAO, f0Luminance.xxx) * g_MapPBRContributionScale.w;
    return (indirect + ambient) * g_MapPBRContributionScale.z + environmentSpecular;
}

struct MAP_STONE_GBUFFER
{
    float4 diffuse;
    float4 normal;
    float4 depth;
    float4 pickPosition;
    float4 indirect;
    float4 materialSpecular;
    float4 surface;
    float4 geometry;
};

// overlay 기본 UV도 중심 회전→tiling→offset 순서다. 별도 overlay texture는 아래에서 meshUV×overlayTiling을 사용한다.
float2 MapSourceOverlayUV(float2 meshUV)
{
    const float2 centeredUV = meshUV - 0.5f;
    return (float2(dot(centeredUV, float2(g_SourceOverlayUV.y, -g_SourceOverlayUV.x)),
        dot(centeredUV, float2(g_SourceOverlayUV.x, g_SourceOverlayUV.y))) + 0.5f) * g_SurfaceUVTiling + g_SourceOverlayUV.zw;
}

// stone/overlay helper의 색과 두 normal 의미를 marker7 G-buffer 계약으로 포장한다.
// 직접광의 mixed normal 길이와 Base용 normal을 분리하며, RT0 UNORM 색을 이미 색이 들어간 광량에 다시 곱하지 않는다.
MAP_STONE_GBUFFER EvaluateMapSourceStoneGeometry(float2 meshUV, float4 vertexColor,
    float3 worldPosition, float3 tangent, float3 binormal, float3 normal,
    float4 projectedPosition, float2 lightmapUV, float4 averageScale,
    float4 directionalScale)
{
    SOURCE_STONE_PARAMETERS material;
    material.diffuseColor = g_SurfaceDiffuseColor.rgb;
    material.overlayColor = g_SurfaceOverlayColor.rgb;
    material.specularColor = g_SurfaceSpecularColor.rgb;
    material.normalIntensity = g_SurfaceNormalIntensity;
    material.overlayNormalIntensity = g_SurfaceOverlayNormalIntensity;
    material.overlaySharpness = g_SurfaceOverlaySharpness;
    material.diffuseSaturation = g_SurfaceDiffuseSaturation;
    material.diffuseBrightness = g_SurfaceDiffuseBrightness;
    material.overlayBrightness = g_SurfaceOverlayBrightness;
    material.overlaySaturation = g_SurfaceOverlaySaturation;
    material.specularIntensity = g_SurfaceSpecularIntensity;
    material.overlaySpecularIntensity = g_SurfaceOverlaySpecularIntensity;
    material.specularPower = g_SurfaceSpecularPower;
    const float2 overlayUV = meshUV * g_SurfaceOverlayTiling;
    float2 surfaceUV = MapSourceOverlayUV(meshUV);
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    const float3 tangentView = SourceStoneUnit(mul(tangentToWorld, g_vCamPosition.xyz - worldPosition));
    float4 sampledDiffuse = SampleMapDiffuseTexture(surfaceUV, SurfaceAnisotropicSampler);
    float opacity = sampledDiffuse.a;
    if ((g_SourceOverlayFlags & 512u) != 0u)
    {
        const float originalAlpha = sampledDiffuse.a;
        surfaceUV += (originalAlpha - g_SourceBgBump.x) * g_SourceBgBump.y * tangentView.xy;
        sampledDiffuse = SampleMapDiffuseTexture(surfaceUV, SurfaceAnisotropicSampler);
        opacity = saturate(originalAlpha + sampledDiffuse.a);
        material.diffuseBrightness *= saturate(originalAlpha + g_SourceBgBump.z);
    }
    if ((g_SourceOverlayFlags & 64u) != 0u)
        clip(opacity - 0.3333f);
    const float4 sampledOverlay = g_SurfaceOverlayDiffuseTexture.Sample(SurfaceAnisotropicSampler, overlayUV);
    // 원본 재질 명령은 SampleBias(0)을 사용한다. 기존 aniso sampler는
    // 프로젝트의 filtering 연결부이며 UV가 속한 좌표 영역을 바꾸지 않는다.
    SOURCE_STONE_SURFACE stone = EvaluateSourceStoneVariants(
        sampledDiffuse,
        (g_SourceOverlayFlags & 1u) != 0u ? g_NormalTexture.Sample(SurfaceAnisotropicSampler,
            (g_SourceOverlayFlags & 128u) != 0u ? meshUV : surfaceUV) : float4(0.5f, 0.5f, 1.f, 1.f),
        sampledOverlay,
        (g_SourceOverlayFlags & 2u) != 0u ?
            g_SurfaceOverlayNormalTexture.Sample(SurfaceAnisotropicSampler, overlayUV) :
            float4(0.5f, 0.5f, 1.f, 1.f),
        (g_SourceOverlayFlags & 32u) != 0u ? g_DetailNormalTexture.Sample(SurfaceAnisotropicSampler,
            surfaceUV * g_SurfaceDetailNormalTiling) : float4(0.5f, 0.5f, 1.f, 1.f),
        vertexColor, material, g_SourceOverlayFlags, mul(tangentToWorld, g_SourceOverlayDirection.xyz),
        g_SourceOverlayDirection.w, g_SurfaceDetailNormalIntensity);
    // Subspecular는 direct specular를 꺼도 같은 raw specular 색을 사용한다.
    // 이 lobe에서는 원본의 혼합 법선을 정규화하지 않는다.
    const float3 specularSample = g_SurfaceOverlaySeparateSpecular != 0u ?
        g_SpecularTexture.Sample(SurfaceAnisotropicSampler, surfaceUV).rgb : sampledDiffuse.rgb;
    const float3 baseSpecular = SourceStoneSaturation(specularSample, g_SourceBgSpecularSaturation) *
        material.specularColor * material.specularIntensity;
    const float3 overlaySpecular = SourceStoneSaturation(sampledOverlay.rgb,
        material.overlaySaturation) * material.overlaySpecularIntensity;
    const float3 sourceSpecular = lerp(baseSpecular, overlaySpecular, stone.overlayWeight);
    stone.specular = (g_SourceOverlayFlags & 256u) != 0u ? sourceSpecular : 0.f;
    const float facing = saturate(dot(stone.directMixedNormal, tangentView));
    const float3 subspecular = sourceSpecular * g_SourceBgSubspecular.x *
        MapSurfaceSafePow(facing * facing, g_SourceBgSubspecular.y);
    const bool hasLightmap = g_HasBakedLighting != 0u && averageScale.w != 0.f;
    float3 average = 0.f, coefficients = 0.f;
    if (hasLightmap)
    {
        average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler,
            lightmapUV).rgb * averageScale.rgb;
        coefficients = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler,
            lightmapUV).rgb * directionalScale.rgb;
    }
    const SOURCE_STONE_BASE_LIGHT base = EvaluateSourceStoneBase(stone,
        mul(tangentToWorld, g_vCamPosition.xyz - worldPosition),
        mul(tangentToWorld, float3(0.f, 1.f, 0.f)), average, coefficients,
        hasLightmap, material.specularPower, SourceStoneInactiveEngineInputs());
    MAP_STONE_GBUFFER output;
    // RT0는 일반 albedo debug 표시이며 HDR shading은 RT4/RT6를 사용한다.
    // 이미 색을 포함한 원본 결과에 이 UNORM 값을 다시 곱하지 않는다.
    output.diffuse = float4(base.diffuse, 1.f);
    output.normal = float4(SourceStoneUnit(mul(stone.baseNormal, tangentToWorld)) *
        0.5f + 0.5f, 0.f);
    output.depth = float4(projectedPosition.z / projectedPosition.w,
        projectedPosition.w / 1000.f, material.specularPower, 7.f);
    output.pickPosition = float4(worldPosition,
        EncodeMapSurfaceGeometricNormal(normal, hasLightmap));
    output.indirect = float4(base.radiance + subspecular + EvaluateMapSurfaceEmissive(meshUV, 7u), 0.f);
    output.materialSpecular = float4(stone.specular, stone.overlayWeight);
    output.surface = float4(stone.diffuse, 0.f);
    // 원본 Direct는 혼합 법선을 정규화하지 않으므로 그 길이를 유지한다.
    // 선택된 Base/Baked 법선은 순수 계산 helper 안에서 정규화한다.
    output.geometry = float4(mul(stone.directMixedNormal, tangentToWorld),
        (g_SourceOverlayFlags & 1024u) != 0u ? 1.f : 0.f);
    return output;
}

// 현재 material의 program 번호로 표면 helper를 한 번 선택한다. 11~14·foliage·BG·specular·PBR 순이다.
// 마지막은 기존 compatibility 경로이며, 지원 texture나 native ABI 검증을 이 함수가 대신 수행하지는 않는다.
MAP_SURFACE_SAMPLE EvaluateMapSurface(float2 meshUV, float4 vertexColor, float3 worldPosition,
    float3 tangent, float3 binormal, float3 normal)
{
    if (g_SurfaceProgram == 14u)
        return EvaluateMapSourceLandscapeSurface(meshUV, tangent, binormal, normal);
    if (g_SurfaceProgram == 11u)
        return EvaluateMapSourceIceSurface(meshUV, vertexColor, worldPosition, tangent, binormal, normal);
    if (g_SurfaceProgram == 12u)
        return EvaluateMapSourceVertexBlendSurface(meshUV, vertexColor, worldPosition, tangent, binormal, normal);
    if (g_SurfaceProgram == 13u)
        return EvaluateMapSourceWetSurface(meshUV, vertexColor, worldPosition, tangent, binormal, normal);
    if (IsMapSurfaceSourceFoliage())
        return EvaluateMapSourceFoliageSurface(meshUV, worldPosition, tangent, binormal, normal);
    if (IsMapSurfaceSourceBG())
        return EvaluateMapSourceBGSurface(meshUV, vertexColor, worldPosition, tangent, binormal, normal);
    if (IsMapSurfaceSourceSpecular())
        return EvaluateMapSourceSpecularSurface(meshUV, worldPosition, tangent, binormal, normal);
    if (IsMapSurfacePBR())
        return EvaluateMapPBRSurface(meshUV, worldPosition, tangent, binormal, normal);
    MAP_SURFACE_SAMPLE result = (MAP_SURFACE_SAMPLE)0;
    result.roughness = 0.f;
    result.metallic = 0.f;
    result.ambientOcclusion = 1.f;
    const float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, meshUV);
    clip(diffuse.a - 0.3333f);
    const float4 encodedNormal = g_NormalTexture.Sample(LinearSampler, meshUV);
    const float2 rawXY = encodedNormal.rg * 2.f - 1.f;
    const float rawZ = sqrt(max(1.f - dot(rawXY, rawXY), 0.f)) + 0.00001f;
    // 선택된 top mesh에는 내보낸 COLOR_0가 없어 vertex-alpha 계수로
    // 기본값 1을 사용한다. 원본 placement override-color의 의미는 아직 확인되지 않았다.
    const float3 tangentNormal = normalize(float3(rawXY * g_SurfaceNormalIntensity, rawZ));
    // 이 원본 mesh들은 복원된 WModel에 tangent 좌표계의 handedness를 보존한다.
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(tangent),
        MapGeometryNormalizeOrZero(binormal), MapGeometryNormalizeOrZero(normal));
    result.tangentNormal = tangentNormal;
    result.worldNormal = normalize(mul(tangentNormal, tangentToWorld));
    const float3 worldView = normalize(g_vCamPosition.xyz - worldPosition);
    const float3 tangentView = normalize(mul(tangentToWorld, worldView));
    const float3 tangentReflection = reflect(-tangentView, tangentNormal);
    float2 reflectionUV;
    float3 baseDiffuse = diffuse.rgb;
    if (g_SurfaceProgram == 1u)
    {
        const float3 worldReflection = mul(tangentReflection, tangentToWorld);
        // 원본 (x,y,z) -> runtime (x,z,-y) 변환이다. 원본 CB0의 engine-origin은
        // 정체가 확인되지 않아 추가 xy*0.0003 보정량을 명시적으로 0으로 둔다.
        reflectionUV = float2(worldReflection.x, -worldReflection.z) * g_SurfaceReflectionTiling;
        result.specular = g_SpecularTexture.Sample(LinearSampler, meshUV).rgb;
    }
    else
    {
        reflectionUV = lerp(meshUV, (tangentReflection.xy + 0.5f) * 0.5f, 0.75f);
        const float luminance = dot(diffuse.rgb, float3(0.3f, 0.59f, 0.11f));
        baseDiffuse = lerp(luminance.xxx, diffuse.rgb, g_SurfaceDiffuseSaturation);
        // 이 재질 계열에서 S texture를 끈 분기는 원본 D.rgb를 소비한다.
        result.specular = diffuse.rgb;
    }
    baseDiffuse *= g_SurfaceDiffuseColor.rgb * g_SurfaceDiffuseBrightness;
    result.specular *= g_SurfaceSpecularColor.rgb * g_SurfaceSpecularIntensity;
    const float3 reflection = g_ReflectionTexture.Sample(LinearSampler, reflectionUV).rgb;
    const float3 signedReflection = reflection * g_SurfaceReflectionColor.rgb *
        (1.f + g_SurfaceReflectionContrast) - g_SurfaceReflectionContrast;
    const float weight = encodedNormal.a * g_SurfaceReflectionIntensity;
    result.diffuse = float4(clamp((baseDiffuse + weight * saturate(signedReflection)) *
        (1.f - weight * saturate(-signedReflection)), 0.f, 999.f), diffuse.a);
    result.reflectionDelta = abs(result.diffuse.rgb - baseDiffuse);
    return result;
}
#endif
