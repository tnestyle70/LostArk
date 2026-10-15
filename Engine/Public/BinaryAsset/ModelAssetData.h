#pragma once

// Engine 공통 타입과 소유 컨테이너를 CPU 모델 입력·결과 계약에 사용한다.
#include "Engine_Defines.h"
// 정적/스킨 정점의 실제 메모리 형식을 mesh별 배열 원소로 사용한다.
#include "Engine_VertexTypes.h"

// shader 슬롯과 digest처럼 길이가 고정된 소유 배열을 선언한다.
#include <array>
// 지형/바람 입력 검증에서 유한성·절댓값·정수 여부를 확인한다.
#include <cmath>
// 로드 요청과 재질 texture의 OS 경로를 값으로 소유한다.
#include <filesystem>

// decoder와 CModel/CMaterial 사이의 소유 데이터 계약이다. vector/string/path가 값을 소유하며 원본 파일 버퍼를 참조하지 않는다.
// 아래 표면 입력은 runtime override도 공유하므로 모든 필드가 WINT 파일에 직렬화된다는 뜻은 아니다.
NS_BEGIN(Engine)

// WMA3의 color mask 채널이 염색 영역을 선택하고 tint가 색을 정한다.
// isEnabled=false와 모두 1인 tint가 기본값이며 V3 이전 재질도 이 값으로 시작한다.
struct MODEL_COLOR_TINT
{
	bool_t isEnabled = {false};
	// CMaterial은 colorMaskPath가 diffusePath와 같을 때 hair mask로 분류한다.
	// 의상의 별도 mask와 달리 hair는 diffuse 자체의 채널을 염색/가닥 입력으로 쓰는 경로다.
	bool_t isHairMask = {false};
	float4_t vDiffuse = {1.f, 1.f, 1.f, 1.f};
	float4_t vRegionA = {1.f, 1.f, 1.f, 1.f};
	float4_t vRegionB = {1.f, 1.f, 1.f, 1.f};
	float4_t vRegionC = {1.f, 1.f, 1.f, 1.f};
};

// Client가 원본 재질의 identity를 해석하고 Engine은 선택된 surface 프로그램의 입력을 소유한다.
enum class MODEL_SURFACE_FAMILY : uint32_t
{
	LEGACY = 0,
	SPECULAR_TEXTURE_REFLECTION = 1,
	DIFFUSE_SPECULAR_REFLECTION = 2,
	PBR_SEAMLESS_OPAQUE = 3,
	PBR_OPAQUE = 4,
	SOURCE_SPECULAR_OPAQUE = 5,
	SOURCE_CHARACTER = 6,
	SOURCE_OVERLAY_OPAQUE = 7,
	SOURCE_BG_OPAQUE_MASKED = 8,
	SOURCE_FOLIAGE_MASKED = 9,
	SOURCE_GRASS_MASKED = 10,
	SOURCE_SNOWICE_OPAQUE = 11,
	SOURCE_VERTEXBLEND_OPAQUE = 12,
	SOURCE_WET_OPAQUE = 13,
	SOURCE_LANDSCAPE_OPAQUE = 14,
};

// placement별 atlas 좌표와 decode 배율이다. texture 쌍은 material variant에 속하지만 이 값은 placement마다 별도로 전달한다.
struct MODEL_BAKED_LIGHTING_INSTANCE
{
	float4_t scaleBias = {0.f, 0.f, 0.f, 0.f};
	float4_t averageScale = {0.f, 0.f, 0.f, 0.f}; // w는 활성 여부다.
	float4_t directionalScale = {0.f, 0.f, 0.f, 0.f};
	float4_t shadowScaleBias = {0.f, 0.f, 0.f, 0.f};
};

// 원본 캐릭터 shader도 CModel/CMaterial 경로를 공유한다.
// Client가 이름 있는 source parameter를 program별 uniform 배열로 변환하며 배열 index는 avatar identity가 아니다.
inline constexpr uint32_t SOURCE_CHARACTER_CONSTANT_COUNT = 64u;
inline constexpr uint32_t SOURCE_CHARACTER_TEXTURE_COUNT = 16u;

struct MODEL_SOURCE_CHARACTER_PARAMETERS
{
	uint32_t program = 0u;
	uint32_t baseTextureMask = 0u;
	uint32_t lightTextureMask = 0u;
	uint32_t requiredExtraUVMask = 0u; // bit 0은 TEXCOORD1, bit 1은 TEXCOORD2다.
	std::array<float4_t, SOURCE_CHARACTER_CONSTANT_COUNT> baseConstants{};
	std::array<float4_t, SOURCE_CHARACTER_CONSTANT_COUNT> lightConstants{};
};

// shader slot에 연결할 texture 경로와 색 공간 선택이다. 경로만 보관하며 GPU texture는 CMaterial이 생성한다.
struct MODEL_SOURCE_CHARACTER_TEXTURE
{
	filesystem::path path;
	bool_t srgb = false;
};

// INHERIT는 기존 표면 정책을 유지하는 요청이다. 구체 pass/cull 선택은 모델·재질 소비자가 검증하고 적용한다.
enum class MODEL_SURFACE_RENDER_MODE : uint32_t
{
	INHERIT,
	DEFERRED,
	TRANSLUCENT,
	BACKGROUND,
	ADDITIVE,
	WATER
};
enum class MODEL_SURFACE_CULL_MODE : uint32_t
{
	INHERIT,
	CULL_BACK,
	CULL_FRONT,
	TWO_SIDED
};

// snow/ice, wet, vertex-blend 프로그램이 사용하는 묶음이다. family/flags에 따라 유효한 필드와 texture가 달라진다.
struct MODEL_SOURCE_SPECIAL_PARAMETERS
{
	uint32_t flags = 0u; // ice는 detail/spec texture, vertex blend는 G/B layer flag다.
	f32_t normalTiling = 1.f;
	float4_t iceCoreColor = {};
	float4_t iceOuterColor = {};
	float4_t iceBlend = {1.f, .5f, .5f, 0.f}; // mask UV, blend, 경계 선명도, 발광 순서다.
	f32_t iceBumpOffset = .3f;
	float4_t wetParameters = {1.f, .5f, 1.f, 1.f}; // normal, 경계 선명도, opacity, specular 순서다.
	f32_t wetSpecularPower = 50.f;
	std::array<float4_t, 4> blendDiffuse{};	 // RGB tint와 밝기다.
	std::array<float4_t, 4> blendSpecular{}; // RGB tint와 강도다.
	std::array<float4_t, 4> blendLayers{};	 // UV XY, normal 강도, specular 지수다.
	f32_t blendSharpness = .5f;
	bool_t maskSRGB = false;
	bool_t blendGSRGB = true;
	bool_t blendBSRGB = true;
};

// 기존 material 경로에 전달하는 painted heightfield 입력이다. 현재 grid 검증은 두 subsection의 62/31 quad 구성을 허용한다.
inline constexpr uint32_t SOURCE_LANDSCAPE_LAYER_COUNT = 6u;
inline constexpr uint32_t SOURCE_LANDSCAPE_WEIGHTMAP_COUNT = 2u;

struct MODEL_SOURCE_LANDSCAPE_PARAMETERS
{
	uint32_t layerMask = 0u;
	uint32_t normalMask = 0u;
	uint32_t weightmapCount = 0u;
	float4_t grid = {}; // section XY, component quad 수, subsection quad 수다.
	float4_t weightmapScaleBias = {};
	float4_t heightmapScaleBias = {};
	std::array<float4_t, SOURCE_LANDSCAPE_LAYER_COUNT> uv{};	   // tiling, source 회전, 예약값 0, 0이다.
	std::array<float4_t, SOURCE_LANDSCAPE_LAYER_COUNT> diffuse{};  // tint RGB와 밝기다.
	std::array<float4_t, SOURCE_LANDSCAPE_LAYER_COUNT> specular{}; // tint RGB와 강도다.
	std::array<float4_t, SOURCE_LANDSCAPE_LAYER_COUNT>
		factors{}; // 탈색, normal 강도, specular 지수, 예약값 0이다.
	std::array<float4_t, SOURCE_LANDSCAPE_LAYER_COUNT>
		weight{}; // map index, 채널, diffuse/normal height mode다.
	std::array<bool_t, SOURCE_LANDSCAPE_LAYER_COUNT> diffuseSRGB{};

	// MapAssetCatalog와 CModel/CMaterial이 공유하는 landscape 수치 검증이다. 객체를 바꾸지 않으며 texture 경로/실물 검증은 호출자가 이어서 한다.
	bool Has_ValidInputs() const
	{
		// 벡터 성분을 유한한 ±1e6 범위로 제한한다. texture 존재나 픽셀 값은 이 수치 검사 대상이 아니다.
		const auto finite = [](const float4_t& v)
		{
			return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) && std::isfinite(v.w) &&
				std::abs(v.x) <= 1e6f && std::abs(v.y) <= 1e6f && std::abs(v.z) <= 1e6f &&
				std::abs(v.w) <= 1e6f;
		};
		// 활성 layer/normal mask의 포함 관계와 지원 grid·양수 UV 배율을 먼저 확인한다.
		if (layerMask == 0u || (layerMask & ~63u) || (normalMask & ~layerMask) || weightmapCount == 0u ||
			weightmapCount > SOURCE_LANDSCAPE_WEIGHTMAP_COUNT || !finite(grid) ||
			std::floor(grid.x) != grid.x || std::floor(grid.y) != grid.y || grid.z != 62.f || grid.w != 31.f ||
			!finite(weightmapScaleBias) || weightmapScaleBias.x <= 0.f || weightmapScaleBias.y <= 0.f ||
			!finite(heightmapScaleBias) || heightmapScaleBias.x <= 0.f || heightmapScaleBias.y <= 0.f)
			return false;
		// 최대 두 weightmap의 RGBA를 8bit 사용표로 묶어 활성 layer끼리 같은 채널을 공유하지 못하게 한다.
		uint32_t usedChannels = 0u, usedMaps = 0u;
		for (uint32_t i = 0u; i < SOURCE_LANDSCAPE_LAYER_COUNT; ++i)
		{
			if ((layerMask & (1u << i)) == 0u)
				continue;
			// 활성 layer만 검사한다. 미사용 슬롯의 값은 이 반복문이 검증하지 않는다.
			const auto& u = uv[i];
			const auto& d = diffuse[i];
			const auto& s = specular[i];
			const auto& f = factors[i];
			const auto& w = weight[i];
			if (!finite(u) || !finite(d) || !finite(s) || !finite(f) || !finite(w) || u.x <= 0.f ||
				u.z != 0.f || u.w != 0.f || d.x < 0.f || d.y < 0.f || d.z < 0.f || d.w < 0.f || s.x < 0.f ||
				s.y < 0.f || s.z < 0.f || s.w < 0.f || f.x < 0.f || f.x > 1.f || f.y < 0.f || f.y > 16.f ||
				f.z < 1.f || f.z > 4096.f || f.w != 0.f || ((normalMask & (1u << i)) == 0u && f.y != 0.f) ||
				w.x < 0.f || w.x >= float(weightmapCount) || std::floor(w.x) != w.x || w.y < 0.f || w.y > 3.f ||
				std::floor(w.y) != w.y || (w.z != 0.f && w.z != 1.f) || (w.w != 0.f && w.w != 1.f))
				return false;
			// map index와 RGBA 채널을 하나의 bit로 바꿔 중복 사용을 거절하고 실제 사용 map을 기록한다.
			const uint32_t channel = 1u << (uint32_t(w.x) * 4u + uint32_t(w.y));
			if (usedChannels & channel)
				return false;
			usedChannels |= channel;
			usedMaps |= 1u << uint32_t(w.x);
		}
		// 선언한 모든 weightmap이 적어도 한 layer에 사용돼야 한다. 한 map의 RGBA 네 채널을 모두 써야 한다는 조건은 아니다.
		return usedMaps == (1u << weightmapCount) - 1u;
	}
};

// 활성 layer와 mask가 필요한 texture slot을 결정한다. 배열 슬롯이 존재한다는 사실만으로 파일을 읽은 것은 아니다.
struct MODEL_SOURCE_LANDSCAPE_TEXTURES
{
	std::array<filesystem::path, SOURCE_LANDSCAPE_LAYER_COUNT> diffuse;
	std::array<filesystem::path, SOURCE_LANDSCAPE_LAYER_COUNT> normal;
	std::array<filesystem::path, SOURCE_LANDSCAPE_WEIGHTMAP_COUNT> weightmaps;
	filesystem::path heightmap;
};

// 여러 actor가 한 asset을 공유하므로 원본 primitive의 위치/크기는 placement별 입력으로 둔다.
struct MODEL_SOURCE_FOLIAGE_WIND_INSTANCE
{
	float4_t actorPositionSourceCm = {};			 // XYZ는 source cm, W=1은 적용 승인된 입력이다.
	float4_t objectDimensionsAndRadiusSourceCm = {}; // 원본 ObjectDimension XYZ와 구 반경 W다.
};

// 재질 variant의 shader 입력 값이다. placement별 RNM atlas/wind instance는 별도 구조체가 전달한다.
struct MODEL_SURFACE_PARAMETERS
{
	// 렌더링 방식과 프로그램 선택이다. 실제 허용 조합은 CModel/CMaterial이 로드할 때 검증한다.
	MODEL_SURFACE_RENDER_MODE renderMode = MODEL_SURFACE_RENDER_MODE::INHERIT;
	MODEL_SURFACE_CULL_MODE cullMode = MODEL_SURFACE_CULL_MODE::INHERIT;
	MODEL_SURFACE_FAMILY family = MODEL_SURFACE_FAMILY::LEGACY;
	bool_t pbrAlphaMasked = false; // 원본 PBR masked의 diffuse alpha 임계값은 0.3333이다.

	// 원본 캐릭터 프로그램은 고정 슬롯 uniform 묶음을 별도로 소비한다.
	MODEL_SOURCE_CHARACTER_PARAMETERS sourceCharacter;

	// 원본 BG static-switch bit다. normal=1, bump=2, specular=4, specular texture=8, reflection=16,
	// world reflection=32, masked=64, vertex-alpha normal=128, simple=256, seamless=512.
	// sampler mirror는 diffuse V=1024, normal U=2048, normal V=4096이며,
	// linear reflection=8192, fixed base normal UV=16384, detail normal=32768이다.
	uint32_t sourceBgFlags = 0u;
	// BG 소품은 명시된 설정에 따라 scene light와 독립적으로 표면 색을 낼 수 있다.
	bool_t sourceBgUnlit = false;
	float4_t sourceBgBump = {0.f, 0.f, 1.f, 0.f};	  // offset, 강도, 밝기, 예약값 순서다.
	float4_t sourceBgUV = {0.f, 1.f, 0.f, 0.f};		  // sin, cos, 고정 이동 X/Y다.
	uint32_t sourceBgFlicker = 0u;					  // 0은 고정, 1은 중첩, 2는 선형 flicker다.
	float2_t sourceBgSubspecular = {0.f, 60.f};		  // 독립 시선 lobe의 강도와 지수다.
	float4_t sourceBgRimlight = {0.f, 0.f, 0.f, 3.f}; // 강도를 곱한 RGB와 지수다.
	f32_t sourceBgSpecularSaturation = 1.f;
	float2_t sourceBgPanning = {0.f, 0.f};


	// 원본 foliage bit는 normal=1, saturation=2, specular=4, specular texture=8,
	// transmission=16, emissive=32, emissive flicker=64다.
	MODEL_SOURCE_SPECIAL_PARAMETERS sourceSpecial;
	MODEL_SOURCE_LANDSCAPE_PARAMETERS sourceLandscape;
	uint32_t sourceFoliageFlags = 0u;
	float4_t sourceFoliageTransmission = {0.f, 0.f, 0.f, 1.f};
	bool_t sourceFoliageMaskSRGB = false;
	// 선택한 원본 foliage VS의 per-material 입력이며 별도 renderer를 만들지 않는다.
	bool_t sourceFoliageWind = false;
	uint32_t sourceFoliageWindProgram = 1u; // 원본 vertex program E4FE/A1C6/1C39/098C 선택값이다.
	float4_t sourceFoliageWindLocalCenter = {0.f, 0.f, 0.f, 1.f};
	float4_t sourceFoliageWindLocalBounds = {1.f, 1.f, 1.f, 1.f};
	float4_t sourceFoliageWindActorPosition = {0.f, 0.f, 0.f, 0.f};
	float4_t sourceFoliageWindDirectionSpeed = {0.f, 0.f, 1.f, 0.f};
	float4_t sourceFoliageWindPlayerPosition = {0.f, 0.f, 0.f, 0.f};
	float4_t sourceFoliageWindScalars[4]{};

	// 지원 wind program의 scalar lane 배치와 예약 시간/미사용 lane을 확인한다. 전체 float finite/범위 검사는 MapAssetCatalog/CModel 호출 경계에 있다.
	bool Has_ValidSourceFoliageWindProgramInputs() const
	{
		if (sourceFoliageWindProgram < 1u || sourceFoliageWindProgram > 4u)
			return false;
		// float4 네 개를 16개 scalar lane으로 읽는다. program별 유효 개수와 time lane 위치가 다르며 실제 시간은 runtime이 주입한다.
		const auto lane = [&](uint32_t i)
		{
			const auto& v = sourceFoliageWindScalars[i / 4u];
			switch (i % 4u)
			{
			case 0u:
				return v.x;
			case 1u:
				return v.y;
			case 2u:
				return v.z;
			default:
				return v.w;
			}
		};
		// 프로그램별 유효 scalar 수와 runtime 시간 슬롯을 정한다. 저작 시간과 미사용 슬롯은 0이어야 한다.
		const uint32_t count =
			sourceFoliageWindProgram == 1u ? 16u : sourceFoliageWindProgram == 3u ? 10u : 7u;
		const uint32_t time =
			sourceFoliageWindProgram == 1u ? 10u : sourceFoliageWindProgram == 3u ? 4u : 1u;
		if (lane(time) != 0.f)
			return false;
		for (uint32_t i = count; i < 16u; ++i)
			if (lane(i) != 0.f)
				return false;
		// 프로그램별 양수 입력과 원점/플레이어 위치의 고정 계약을 확인한다. 나머지 finite 검사는 호출자에 남는다.
		if (sourceFoliageWindProgram == 1u)
			return lane(2u) > 0.f;
		const auto& p = sourceFoliageWindPlayerPosition;
		if (sourceFoliageWindProgram == 2u || sourceFoliageWindProgram == 4u)
			return sourceFoliageWindLocalCenter.x == 0.f && sourceFoliageWindLocalCenter.y == 0.f &&
				sourceFoliageWindLocalCenter.z == 0.f && p.x == 0.f && p.y == 0.f && p.z == 0.f && p.w == 0.f;
		return lane(0u) > 0.f && p.x == 99999.f && p.y == 99999.f && p.z == 0.f && p.w == 0.f;
	}

	// 원본 opaque overlay는 vertex R을 coverage로, vertex A를 normal 강도로 사용한다.
	float4_t overlayColor = {1.f, 1.f, 1.f, 1.f};
	f32_t overlayTiling = 1.f;
	f32_t overlayNormalIntensity = 1.f;
	f32_t overlaySharpness = 1.f;
	f32_t overlayBrightness = 1.f;
	f32_t overlaySaturation = 1.f;
	f32_t overlaySpecularIntensity = 1.f;
	bool_t overlaySRGB = true;
	bool_t overlaySeparateSpecular = false;
	// 기본 법선, overlay 법선, 정점 색칠, 명시 방향, 반전 높이,
	// detail normal, mask, fixed base-normal UV, specular 순서의 flag다.
	uint32_t sourceOverlayFlags = 263u;
	float4_t sourceOverlayDirection = {0.f, 1.f, 0.f, 0.f}; // runtime world 방향 XYZ와 적용량이다.

	// 공통 diffuse/normal/specular/reflection 세기와 색 공간 설정이다.
	f32_t diffuseBrightness = 1.f;
	f32_t normalIntensity = 1.f;
	f32_t specularIntensity = 1.f;
	f32_t specularPower = 50.f;
	f32_t reflectionIntensity = 0.f;
	f32_t reflectionContrast = 0.5f;
	f32_t reflectionTiling = 1.f;
	f32_t diffuseSaturation = 1.f;
	float4_t diffuseColor = {1.f, 1.f, 1.f, 1.f};
	float4_t specularColor = {1.f, 1.f, 1.f, 1.f};
	float4_t reflectionColor = {1.f, 1.f, 1.f, 1.f};
	bool_t diffuseSRGB = true;
	bool_t specularSRGB = false;
	bool_t reflectionSRGB = false;
	bool_t ormSRGB = false;
	bool_t castsShadow = true;

	// 발광의 색·세기와 UV·시간 변화 입력을 묶는다.
	bool_t hasEmissive = false;
	float4_t emissiveColor = {1.f, 1.f, 1.f, 1.f};
	f32_t emissiveIntensity = 0.f;
	float2_t emissiveUVTiling = {1.f, 1.f};
	f32_t emissiveFlickerMinimum = 0.f;
	f32_t emissiveFlickerSpeed = 0.f;
	f32_t emissivePhaseOffset = 0.f;
	bool_t emissiveSRGB = true;

	// 공통 UV와 detail normal, 반사 좌표계 선택이다.
	float2_t uvTiling = {1.f, 1.f};
	bool_t uvFixedNormal = false;
	bool_t useWorldReflection = false;
	f32_t detailNormalIntensity = 0.f;
	f32_t detailNormalTiling = 1.f;

	// PBR 재질의 금속성·거칠기·AO와 밝기 입력이다.
	f32_t metallicIntensity = 1.f;
	f32_t metallicPower = 1.f;
	f32_t roughnessIntensity = 1.f;
	f32_t roughnessPower = 1.f;
	f32_t aoIntensity = 1.f;
	f32_t aoPower = 1.f;
	f32_t specularPBRIntensity = 0.5f;
	f32_t nonmetallicBrightness = 1.f;
	f32_t metallicBrightness = 1.f;
	f32_t minimumRoughness = 0.04f;
	float2_t reflectionOriginOffset = {0.f, 0.f};
	f32_t vertexAlpha = 1.f;

	// 저장된 조명과 정적 그림자의 사용 여부 및 decode 입력이다. 실제 atlas 배치는 instance 값이 전달한다.
	bool_t hasBakedLighting = false;
	bool_t bakedLightingSRGB = false;
	bool_t hasStaticShadow = false;
	uint32_t staticShadowChannel = 0u;
	float4_t staticShadowTransfer = {0.f, 1.f, 1.f, 0.f}; // bias, scale, exponent, 예약값 순서다.

	// 환경 cube 경로와 색·회전 선택이다. 원본 간접광은 아래 별도 입력 묶음을 사용한다.
	bool_t hasEnvironmentCube = false;
	float4_t environmentColor = {1.f, 1.f, 1.f, 0.f};
	float2_t environmentRotation = {0.f, 1.f};
	bool_t environmentLegacyEnabled =
		true; // false는 sourceIndirect를 요구하며 기존 environment 경로에서 cube를 사용하지 않는다.

	// 원본 UE3 PBR 간접광 입력이다. packed SH의 마지막 [6].w는 예약값 1을 사용한다.
	bool_t hasSourceIndirect = false;
	std::array<float4_t, 7> sourceIndirectSH{};
	float4_t sourceIndirectColor = {1.f, 1.f, 1.f, 0.f};
	float2_t sourceIndirectRotation = {0.f, 1.f};
	float3_t sourceUpperSkyColor = {0.f, 0.f, 0.f};
	float3_t sourceLowerSkyColor = {0.f, 0.f, 0.f};
	float4_t sourceAmbientAndSkyFactor = {0.f, 0.f, 0.f, 1.f};
};

// Client가 materialName으로 기존 슬롯을 선택해 전달하는 교체 요청이다. CModel이 경로/입력 조합을 검증해 원본 재질 복사본에 적용한다.
struct MODEL_MATERIAL_OVERRIDE
{
	string materialName;
	bool_t hasDiffuseAddressU = false;
	bool_t diffuseMirrorU = false;
	MODEL_SURFACE_PARAMETERS surface;
	filesystem::path surfaceSpecularPath;
	filesystem::path reflectionPath;
	filesystem::path surfaceDiffusePath;
	filesystem::path surfaceNormalPath;
	filesystem::path overlayDiffusePath;
	filesystem::path overlayNormalPath;
	filesystem::path detailNormalPath;
	filesystem::path surfaceORMPath;

	// foliage/special/vertex-blend 프로그램의 추가 texture 경로다.
	filesystem::path sourceFoliageMaskPath;
	filesystem::path sourceSpecialMaskPath;
	filesystem::path sourceBlendDiffuseGPath;
	filesystem::path sourceBlendDiffuseBPath;
	filesystem::path sourceBlendNormalGPath;
	filesystem::path sourceBlendNormalBPath;
	filesystem::path surfaceEmissivePath;

	// 고정 슬롯을 사용하는 캐릭터/지형 프로그램의 texture 묶음이다.
	std::array<MODEL_SOURCE_CHARACTER_TEXTURE, SOURCE_CHARACTER_TEXTURE_COUNT> sourceCharacterTextures;
	MODEL_SOURCE_LANDSCAPE_TEXTURES sourceLandscapeTextures;

	// 저장 조명·그림자 및 환경/원본 간접광 texture 경로다. 파일 로드는 CMaterial이 수행한다.
	filesystem::path bakedAveragePath;
	filesystem::path bakedDirectionalPath;
	filesystem::path staticShadowPath;
	filesystem::path environmentCubePath;
	filesystem::path environmentBRDFPath;
	filesystem::path sourceIndirectCubePath;
	filesystem::path sourceIndirectBRDFPath;
};

// decoder 기본 재질과 검증된 override를 합친 CPU 입력이다. CMaterial이 texture를 로드하고 실제 tint/표면 상태를 준비한다.
struct MODEL_MATERIAL_DATA
{
	filesystem::path surfaceSpecularPath;
	string name;
	uint64_t nameHash = {};
	bool_t diffuseMirrorU = false;
	filesystem::path diffusePath;
	filesystem::path normalPath;
	filesystem::path specularPath;
	filesystem::path emissivePath;
	filesystem::path opacityPath;
	filesystem::path ormPath;
	filesystem::path metallicPath;
	filesystem::path roughnessPath;
	filesystem::path ambientOcclusionPath;
	filesystem::path colorMaskPath;
	MODEL_COLOR_TINT colorTint;
	MODEL_SURFACE_PARAMETERS surface;
	filesystem::path reflectionPath;
	filesystem::path surfaceDiffusePath;
	filesystem::path surfaceNormalPath;
	filesystem::path overlayDiffusePath;
	filesystem::path overlayNormalPath;
	filesystem::path detailNormalPath;
	filesystem::path surfaceORMPath;

	// foliage/special/vertex-blend 프로그램의 추가 texture 경로다.
	filesystem::path sourceFoliageMaskPath;
	filesystem::path sourceSpecialMaskPath;
	filesystem::path sourceBlendDiffuseGPath;
	filesystem::path sourceBlendDiffuseBPath;
	filesystem::path sourceBlendNormalGPath;
	filesystem::path sourceBlendNormalBPath;
	filesystem::path surfaceEmissivePath;

	// 고정 슬롯을 사용하는 캐릭터/지형 프로그램의 texture 묶음이다.
	std::array<MODEL_SOURCE_CHARACTER_TEXTURE, SOURCE_CHARACTER_TEXTURE_COUNT> sourceCharacterTextures;
	MODEL_SOURCE_LANDSCAPE_TEXTURES sourceLandscapeTextures;

	// 저장 조명·그림자 및 환경/원본 간접광 texture 경로다. 파일 로드는 CMaterial이 수행한다.
	filesystem::path bakedAveragePath;
	filesystem::path bakedDirectionalPath;
	filesystem::path staticShadowPath;
	filesystem::path environmentCubePath;
	filesystem::path environmentBRDFPath;
	filesystem::path sourceIndirectCubePath;
	filesystem::path sourceIndirectBRDFPath;
};

// CModel이 정적/스킨 정점 배열 중 사용할 표현을 선택하는 CPU 계약이다.
enum class MODEL_VERTEX_KIND : uint8_t
{
	STATIC,
	SKINNED,
};

// 정점 채널 보존에 관한 metadata bit다. self-consistent digest를 외부 원본 일치/제품 provenance 인증으로 승격하지 않는다.
enum MODEL_GEOMETRY_EVIDENCE_FLAG : uint32_t
{
	MODEL_GEOMETRY_TANGENT_HANDEDNESS_PRESERVED_FROM_GLTF = 1u << 0,
	MODEL_GEOMETRY_COLOR0_PRESERVED_FROM_GLTF = 1u << 1,
	MODEL_GEOMETRY_BOUNDS_WMODEL_SPACE = 1u << 2,
	MODEL_GEOMETRY_SOURCE_GLTF_SHA256 = 1u << 3,
	MODEL_GEOMETRY_SOURCE_BUFFER_SET_SHA256 = 1u << 4,
	MODEL_GEOMETRY_SOURCE_PACKAGE_OBSERVED_UNBOUND_SHA256 = 1u << 5,
	MODEL_GEOMETRY_SOURCE_OBJECT_PATH_HASH_UNAUTHENTICATED = 1u << 6,
	MODEL_GEOMETRY_LEGACY_CONVERTER_OBSERVED_UNBOUND_SHA256 = 1u << 7,
	MODEL_GEOMETRY_TOOL_SHA256 = 1u << 8,
	MODEL_GEOMETRY_SOURCE_EXPORT_RECEIPT_SHA256 = 1u << 9,
	MODEL_GEOMETRY_LEGACY_COOK_RECEIPT_SHA256 = 1u << 10,
	MODEL_GEOMETRY_CLEAN_SOURCE_EXPORT = 1u << 11,
	MODEL_GEOMETRY_UPK_TO_GLTF_EXACT = 1u << 12,
	MODEL_GEOMETRY_PIVOT_EXACT = 1u << 13,
	MODEL_GEOMETRY_TEXCOORD1_PRESERVED_FROM_GLTF = 1u << 14,
	MODEL_GEOMETRY_TANGENT_HANDEDNESS_PROJECT_RECONSTRUCTED = 1u << 15,
	MODEL_GEOMETRY_NATIVE_PARALLEL_BASIS_PRESERVED = 1u << 16,
	MODEL_GEOMETRY_TEXCOORD2_PRESERVED_FROM_GLTF = 1u << 17,
};

// WModel 공간의 저장 bounds다. reader가 정적 정점과 대조하지만 CModel의 runtime culling bounds는 실제 변환 정점에서 따로 만든다.
struct MODEL_MESH_BOUNDS_DATA
{
	bool_t present = {false};
	float3_t minimum = {};
	float3_t maximum = {};
	float3_t center = {};
	f32_t radius = {};
};

// decoder가 검증한 geometry metadata와 32바이트 digest를 값으로 보관한다. 모델은 배율/진단에 사용하며 검증 중 정점에 자동 배율을 적용하지 않는다.
struct MODEL_GEOMETRY_METADATA_DATA
{
	bool_t present = {false};
	uint16_t versionMajor = {};
	uint16_t versionMinor = {};
	uint32_t channelMask = {};
	uint32_t evidenceFlags = {};
	uint32_t payloadSize = {};
	f32_t sourceToWModelScale = {1.f};
	f32_t geometryPreScale = {1.f};
	array<uint8_t, 32> payloadSha256 = {};
	array<uint8_t, 32> sourceGltfSha256 = {};
	array<uint8_t, 32> sourceBufferSetSha256 = {};
	array<uint8_t, 32> sourcePackageObservedUnboundSha256 = {};
	array<uint8_t, 32> sourceObjectPathHash = {};
	array<uint8_t, 32> legacyConverterObservedUnboundSha256 = {};
	array<uint8_t, 32> geometryToolSha256 = {};
	array<uint8_t, 32> sourceExportReceiptSha256 = {};
	array<uint8_t, 32> legacyCookReceiptSha256 = {};
	array<uint8_t, 32> metadataSha256 = {};
	array<uint8_t, 32> metadataIdentitySha256 = {};
};

// submesh별 소유 CPU geometry다. vertexKind에 맞는 정점 vector와 local uint32 index를 CModel이 GPU mesh 생성에 사용한다.
// 추가 UV는 정점 내부에, 정적 handedness/COLOR0는 별도 배열에 보관한다. has*는 실제 읽힌 채널을 나타낸다.
struct MODEL_MESH_DATA
{
	string name;
	uint32_t materialIndex = {};
	MODEL_VERTEX_KIND vertexKind = {MODEL_VERTEX_KIND::STATIC};
	vector<VTXMESH> vertices;
	vector<VTXANIMMESH> skinnedVertices;
	vector<uint32_t> indices;
	vector<f32_t> tangentHandedness;
	vector<uint32_t> color0Rgba8;
	bool_t hasColor0 = {false};
	bool_t hasTexcoord1 = {false};
	bool_t hasTexcoord2 = {false};
	MODEL_MESH_BOUNDS_DATA embeddedBounds;
};

// 본 index 순서는 skin index/animation resolvedBoneIndex와 공통이다. parent=-1은 root, restLocal은 부모 기준, inverseBind는 WMSH에서 결합한다.
struct MODEL_BONE_DATA
{
	uint64_t nameHash = {};
	string name;
	int32_t parentIndex = {-1};
	float4x4_t restLocal = {};
	float4x4_t inverseBind = {};
};

// socket localOffset은 parentBoneIndex에 대한 상대 행렬이다. 파일 데이터 자체가 runtime 객체/본 포인터를 소유하지 않는다.
struct MODEL_SOCKET_DATA
{
	uint64_t nameHash = {};
	string name;
	int32_t parentBoneIndex = {-1};
	float4x4_t localOffset = {};
};

// bones/socket을 소유하며 skeletonHash는 현재 WSKL reader가 본 nameHash 순서로 계산한다. hierarchy/rest 내용 전체의 hash는 아니다.
struct MODEL_SKELETON_DATA
{
	uint64_t skeletonHash = {};
	float4x4_t globalInverseRoot = {};
	vector<MODEL_BONE_DATA> bones;
	vector<MODEL_SOCKET_DATA> sockets;
};

// position/scale과 rotation은 독립 tick 배열을 가진다. reader는 비감소 시각을 허용하고 rotation을 정규화한다.
struct MODEL_VECTOR_KEY_DATA
{
	f32_t timeTicks = {};
	float3_t value = {};
};

// 회전 키 한 개의 native tick과 quaternion 값이다. position/scale 키와 시간 배열을 공유하지 않는다.
struct MODEL_QUAT_KEY_DATA
{
	f32_t timeTicks = {};
	float4_t value = {0.f, 0.f, 0.f, 1.f};
};

// boneNameHash를 skeleton에서 찾은 resolvedBoneIndex로 연결한다. CChannel은 이 index의 본을 갱신하며 키 vector를 자기 상태로 복사한다.
struct MODEL_ANIMATION_CHANNEL_DATA
{
	uint64_t boneNameHash = {};
	int32_t resolvedBoneIndex = {-1};
	vector<MODEL_VECTOR_KEY_DATA> positionKeys;
	vector<MODEL_QUAT_KEY_DATA> rotationKeys;
	vector<MODEL_VECTOR_KEY_DATA> scaleKeys;
};

// WANM에 담긴 event를 보존하는 CPU 데이터다. 현재 CAnimation::Initialize는 이 배열을 재생 event로 등록하지 않는다.
struct MODEL_ANIMATION_EVENT_DATA
{
	f32_t timeTicks = {};
	uint16_t type = {};
	uint32_t skillId = {};
	uint32_t paramU32 = {};
	f32_t paramF32 = {};
	uint64_t stringHash = {};
};

// Animation Tool 목록용 metadata다. 이 항목을 얻었다고 전체 모델/animation key가 재생 가능하다는 뜻은 아니다.
struct MODEL_ANIMATION_CATALOG_ENTRY
{
	string name;
	f32_t durationTicks = {};
	f32_t ticksPerSecond = {};
};

// native duration/ticksPerSecond와 기본 loop 요청을 보관한다. CModel은 defaultLoop를 Start_Animation에 넘긴다.
// 현재 CAnimation 초기화는 파일 ticksPerSecond 대신 COOKED_TICK_RATE를 재생 시계로 사용하므로 둘을 같은 값으로 단정하지 않는다.
struct MODEL_ANIMATION_DATA
{
	string name;
	uint64_t skeletonHash = {};
	f32_t durationTicks = {};
	f32_t ticksPerSecond = {};
	bool_t defaultLoop = {true};
	vector<MODEL_ANIMATION_CHANNEL_DATA> channels;
	vector<MODEL_ANIMATION_EVENT_DATA> events;
};

// Decode의 최종 CPU 모델 묶음이다. 이후 CModel은 bones→meshes→materials→animations를 준비하며 GPU 실패 가능성은 이 성공과 별개다.
struct MODEL_ASSET_DATA
{
	vector<MODEL_MESH_DATA> meshes;
	vector<MODEL_MATERIAL_DATA> materials;
	MODEL_GEOMETRY_METADATA_DATA geometryMetadata;
	bool_t hasSkeleton = {false};
	MODEL_SKELETON_DATA skeleton;
	vector<MODEL_ANIMATION_DATA> animations;
};

// 호출자가 지정하는 로드/override 입력이다. package는 내부 section을, 분리 WMSH는 companion 경로를 사용한다.
// defaultAnimationName과 materialOverrides의 최종 적용은 decoder가 아니라 CModel이 수행한다.
struct MODEL_ASSET_LOAD_DESC
{
	filesystem::path assetRoot;
	filesystem::path meshPath;
	filesystem::path materialPath;
	filesystem::path skeletonPath;
	vector<filesystem::path> animationPaths;
	filesystem::path fallbackDiffusePath;
	string defaultAnimationName;
	vector<MODEL_MATERIAL_OVERRIDE> materialOverrides;
};

// registry는 호출별 report를 만들어 thread_local 마지막 결과로 보관한다. 하위 reader는 error/통계를 누적하므로 실패 보고서에도 부분 개수가 남을 수 있다.
struct MODEL_DECODE_REPORT
{
	bool_t succeeded = {false};
	bool_t usedSkinnedBindPose = {false};
	filesystem::path meshPath;
	string decoderName;
	string error;
	uint32_t meshCount = {};
	uint32_t materialCount = {};
	uint32_t boneCount = {};
	uint32_t animationCount = {};
	uint64_t vertexCount = {};
	uint64_t indexCount = {};
	uint64_t animationKeyCount = {};
	bool_t hasGeometryMetadata = {false};
	uint32_t geometryEvidenceFlags = {};
};

NS_END
