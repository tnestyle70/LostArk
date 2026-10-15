#pragma once

// 수치·문자열·DirectX 저장 벡터의 엔진 별칭을 사용한다.
#include "Engine_Typedef.h"
// 광원 수신 범위 LIGHT_RECEIVER와 공개 렌더링 타입의 선행 선언을 사용한다.
#include "Engine_RenderFwd.h"
// 환경 큐브의 ID3D11ShaderResourceView 타입을 사용한다.
#include <d3d11.h>
// LIGHT 값으로 방향광·점광원·스폿광을 구분한다.
#include "Engine_Enum.h"
// 준비/활성 환경 상태가 큐브 SRV의 COM 참조를 함께 소유한다.
#include <wrl/client.h>
// LIGHT_DESC 필드의 byte 위치를 offsetof로 고정한다.
#include <cstddef>
// 고정 크기 SH 계수와 LUT 픽셀을 값으로 소유한다.
#include <array>
// 원본 LUT의 안정적인 asset ID를 보관한다.
#include <string>
// 수정 불가 LUT를 전환 중 여러 레이어가 공동 소유한다.
#include <memory>
// 런타임 색 보정 전환의 LUT 레이어 목록을 소유한다.
#include <vector>

namespace Engine
{
    // 벤치마크가 세션 동안 비교할 렌더 작업 경로다. 저작 품질·자산 준비·gameplay 설정의 저장값은 아니다.
    // Renderer가 설정 변경을 채택할 때 Revision을 갱신한다. 호출자가 준 Revision을 저장 정본으로 쓰지 않는다.
    struct RENDER_OPTIMIZATION_SETTINGS final
    {
        bool_t FrustumEnabled = true, MeshLodEnabled = true, MapInstancingEnabled = true;
        bool_t IdenticalBatchEnabled = true, LightingBankEnabled = true, StaticShadowCacheEnabled = true;
        bool_t NpcPoseReuseEnabled = true, ParticleRootCacheEnabled = true, ParticleWorkersEnabled = true;
        uint64_t Revision = 1u;
    };
    // 세션의 맵 가시성 처리 경로다. 저작 렌더 품질과 별도로 적용한다.
    struct MAP_VISIBILITY_SETTINGS final
    {
        bool_t OcclusionEnabled = true, DistanceEnabled = true, ParallelPreparationEnabled = false;
        // 세션 비교 스위치는 팀장의 저작 품질 설정을 저장하거나 변경하지 않는다.
        bool_t GpuVisibilityEnabled = true, GpuHiZEnabled = true;
        // 베른의 파생 가시성 정책이다. 작은 소품은 기존 문턱을, 큰 구조물은 2px 이하의 보수적 문턱을 사용한다. 저작 품질 값은 바꾸지 않는다.
        f32_t DistanceScale = 1.f, DistanceMaxPixels = 48.f;
        uint64_t Revision = 1u;

        // 거리 배율을 줄일 때 화면 크기 문턱도 같은 방향으로 옮겨 두 조건 중 하나만 고정되지 않게 한다.
        // Profiler 표시와 실제 거리 컬링이 같은 문턱을 읽으며, 잘못된 입력은 0으로 돌려 숨김을 막는다.
        f32_t Get_DistancePixelLimit() const
        {
            if (!(DistanceScale >= .25f && DistanceScale <= 4.f) ||
                !(DistanceMaxPixels >= 4.f && DistanceMaxPixels <= 128.f))
                return 0.f;
            const f32_t scaled = DistanceMaxPixels / DistanceScale;
            return scaled < 4.f ? 4.f : scaled > 128.f ? 128.f : scaled;
        }
    };

	// Renderer의 활성 환경과 적용 전 후보가 큐브 SRV를 각각 참조한다. 준비만으로 현재 조명을 바꾸지 않는다.
	struct RENDER_ENVIRONMENT_STATE final
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pCube;
		wstring_t strCubePath;
		float4_t vColor = { 1.f, 1.f, 1.f, 0.f };
		float4_t vRotationIntensity = { 0.f, 1.f, 1.f, 0.f };
        // 큐브 좌표계의 확산 조명 SH 계수다. Renderer의 큐브 확산 평가에 전달하며 원본 캐릭터의 입사 SH 입력과 구분한다.
        std::array<float4_t, 7> vDiffuseSH{};
        f32_t fDiffuseIntensity = 0.f;
        bool_t bUseSourcePBRIndirect = false;
	};

	// Light_Manager/Light가 CPU 광원 입력으로 소비한다. GPU bank에는 별도 구조체로 옮기므로 아래 byte 배치를 shader 배열과 동일시하지 않는다.
	typedef struct tagLightDesc
	{
		LIGHT		eType;
		DirectX::XMFLOAT4	vDirection;
		DirectX::XMFLOAT4	vPosition;
		float		fRange;
		float		fFalloffExponent = 1.f;
		DirectX::XMFLOAT4	vDiffuse, vAmbient, vSpecular;
		float		fSpotInnerCos = 1.f;
		float		fSpotOuterCos = 1.f;
		LIGHT_RECEIVER eReceiver = LIGHT_RECEIVER::ALL;
        uint32_t staticShadowChannel = 0u;
        // 원본 캐릭터용 선택 간접광 입력이다. 직접광 RGB와 독립적으로 전달한다.
        float4_t vSourceCharacterAmbient{ 0.f, 0.f, 0.f, 0.f };
	}LIGHT_DESC;

	// 기존 CPU 필드 배치 계약이다. 의미 단위로 설명을 추가해도 필드 순서·형식은 바꾸지 않는다.
	static_assert(sizeof(LIGHT_DESC) == 124u);
	static_assert(offsetof(LIGHT_DESC, vSourceCharacterAmbient) == 108u);
	static_assert(offsetof(LIGHT_DESC, fRange) == 36u);
	static_assert(offsetof(LIGHT_DESC, fFalloffExponent) == 40u);
	static_assert(offsetof(LIGHT_DESC, vDiffuse) == 44u);
	static_assert(offsetof(LIGHT_DESC, fSpotInnerCos) == 92u);
	static_assert(offsetof(LIGHT_DESC, fSpotOuterCos) == 96u);

	// Shadow가 적용 전에 범위를 검증한다. 직교 크기·near/far는 월드 길이이며 shader bias/강도와 구분한다.
	typedef struct tagShadowSettings
	{
		bool_t	bEnabled = false;
		f32_t	fOrthographicWidth = 40.f;
		f32_t	fOrthographicHeight = 40.f;
		f32_t	fNear = 0.1f;
		f32_t	fFar = 150.f;
		f32_t	fDepthBias = 0.0015f;
		f32_t	fNormalBias = 0.02f;
		f32_t	fStrength = 0.7f;
        // 동적 그림자 가림으로 PBR의 구운 간접광을 감쇠할 선택 강도다.
        f32_t fDynamicBakedStrength = 0.f;
        // 세션 품질 비교의 PCF 반경이다. 0/1/2는 정사각형 1/9/25개 표본을 사용한다.
        uint32_t iPCFFilterRadius = 1u;
	}SHADOW_SETTINGS;

	// 광원의 월드 Eye/At과 그림자 설정을 함께 전달한다. 활성 설정의 Apply 성공 후 VIEW/PROJ가 이 값으로 갱신된다.
	typedef struct tagShadowLightDesc
	{
		float4_t			vEye = float4_t(0.f, 20.f, -20.f, 1.f);
		float4_t			vAt = float4_t(0.f, 0.f, 0.f, 1.f);
		SHADOW_SETTINGS		Settings = {};
	}SHADOW_LIGHT_DESC;


	// 16x16x16 색상 격자를 256x16 atlas에 펼친 UNORM 입력이다. 레이어는 const 공유 참조로 픽셀을 보존한다.
	struct SOURCE_COLOR_GRADING_LUT final
	{
		std::string strAssetId;
		std::array<float4_t, 256u * 16u> Pixels{};
	};
	// 하나의 LUT를 공동 소유하고 혼합 비중을 보관한다. 전환 중 동일 LUT의 픽셀을 수정하지 않는다.
	struct SOURCE_COLOR_GRADING_LAYER final
	{
		std::shared_ptr<const SOURCE_COLOR_GRADING_LUT> pLut;
		f32_t fWeight = 1.f; // LUT가 없으면 색을 바꾸지 않는 중립 격자의 가중치다.
	};
	// 원본 tone/색 보정 입력이다. Renderer는 검증과 LUT 자원 준비 후 활성 품질에 채택한다.
	struct SOURCE_POST_PROCESS_SETTINGS final
	{
		bool_t bEnabled = false;
		f32_t fToneScale = 1.f, fToneRange = 8.f, fToneToe = 1.f;
		float3_t vHighlights{ 1.f, 1.f, 1.f }, vMidtones{ 1.f, 1.f, 1.f };
		float3_t vShadows{}, vColorize{ 1.f, 1.f, 1.f };
		f32_t fDesaturation = 0.f;
		// 저작 프로필은 중립 0개 또는 LUT 1개다. 런타임 영역 전환은 원본 LUT 가중치를 섞은 뒤 색 보정식을 적용한다.
		std::vector<SOURCE_COLOR_GRADING_LAYER> LutLayers;
	};

	// 저작 프로필과 세션 비교가 Renderer에 전달하는 품질 값이다. 이 구조체의 기본값만으로 현재 장면의 저장 설정을 판단하지 않는다.
	typedef struct tagRenderQualitySettings
	{
		bool_t	bSSAOEnabled = true;
		f32_t	fSSAORadius = 0.75f;
		f32_t	fSSAOBias = 0.025f;
		f32_t	fSSAOIntensity = 1.f;
		f32_t	fSSAOPower = 1.25f;
		f32_t	fSSAODistanceFade = 60.f;
		bool_t	bBloomEnabled = true;
		f32_t	fBloomThreshold = 1.f;
		f32_t	fBloomSoftKnee = 0.5f;
		f32_t	fBloomIntensity = 0.8f;
		f32_t	fBloomScatter = 1.f;
		f32_t	fExposure = 2.f;
		f32_t	fWhitePoint = 11.2f;
		f32_t	fGamma = 2.2f;
		bool_t	bFXAAEnabled = false;
		f32_t	fFXAASubpixel = 0.75f;
		f32_t	fFXAAEdgeThreshold = 0.166f;
		f32_t	fFXAAEdgeThresholdMin = 0.0833f;
		// 장면 색의 선택 보정이다. 흰 bloom tint와 채도 감소 0은 기존 색을 유지한다.
		float4_t vBloomTint = { 1.f, 1.f, 1.f, 1.f };
		f32_t fSceneDesaturation = 0.f;
		// 표시 색의 색각 보정이다. 0은 끄기, 1/2/3은 적색/녹색/청색 계열 보정이며 강도는 0~1이다.
		int32_t iColorFilterType = 0;
		f32_t fColorFilterStrength = 0.f;
		SOURCE_POST_PROCESS_SETTINGS SourcePostProcess{};
        // 세션 품질 비교에 쓰는 SSAO 표본 수다. 현재 4/8/12만 허용한다.
        uint32_t iSSAOSampleCount = 12u;
        // 세션에서 명시적으로 켜는 화면 공간 조명이다. 맵 PBR G-buffer를 사용하며 이전 프레임·화면 밖 장면은 입력하지 않는다.
        bool_t bSSGIEnabled = false;
        f32_t fSSGIStrength = 0.25f;
        f32_t fSSGIRadius = 4.f;
        uint32_t iSSGISampleCount = 8u;
        bool_t bSSREnabled = false;
        f32_t fSSRStrength = 0.5f;
        f32_t fSSRMaxDistance = 20.f;
        f32_t fSSRThickness = 0.2f;
        uint32_t iSSRStepCount = 32u;
        // SSGI가 켜진 경우 절반 해상도로 모으는 선택 경로다. false면 기존 해상도 경로를 사용한다.
        bool_t bSSGIHalfResolution = false;
		// 표면 texture에서 허용할 가장 세밀한 mip 수준이다. 0~3을 사용하며 0은 원본 sampler를 유지한다.
		uint32_t iTextureMinMip = 0u;
        // 세션에서 명시적으로 켜는 AO/SSR 계산 방법이다. 기본 false는 기존 렌더 경로를 유지한다.
        bool_t bHorizonAOEnabled = false;
        bool_t bSSRRefinementEnabled = false;
        bool_t bSSRRoughnessFilterEnabled = false;
	}RENDER_QUALITY_SETTINGS;

	// 재질 진단 shader에 전달하는 표시 번호다. 항목 순서를 바꾸면 소비하는 숫자 계약도 바뀐다.
	enum class MATERIAL_DEBUG_VIEW : uint32_t
	{
		FINAL,
		BASE_COLOR,
		NORMAL,
		DIRECT_SPECULAR,
		REFLECTION_DELTA,
		ROUGHNESS,
		METALLIC,
		AMBIENT_OCCLUSION,
		BAKED_DIFFUSE,
		ENVIRONMENT_SPECULAR,
		DIRECT_DIFFUSE,
		SCENE_HDR,
		SCENE_TONE,
		SCENE_GRADED,
        CUBE_DIFFUSE,
		END,
	};

	// 지정 level에서만 적용할 PBR 기여도 비교다. 기본 배율은 현재 기여도를 유지한다.
	struct MAP_PBR_COMPARISON_SETTINGS
	{
		bool_t bEnabled = false;
		uint32_t iLevel = 0u;
		// xyzw는 직접 확산광·직접 반사광·RNM 확산광·환경 반사광의 비교 배율이다.
		float4_t vContributionScale = { 1.f, 1.f, 1.f, 1.f };
		// xyzw는 법선 배율·거칠기 가산값·기존 RNM 식 선택(0/1)·예약값이다.
		float4_t vSurfaceParameters = { 1.f, 0.f, 0.f, 0.f };
        f32_t fCubeDiffuseScale = 1.f;
		// Renderer와 맵 재질 binder가 현재 level의 비교 적용 여부를 함께 조회한다. 상태는 바꾸지 않는다.
		bool_t Is_Active(uint32_t level) const noexcept
		{
			return bEnabled && iLevel == level;
		}
	};

	/* 장면 품질 저장과 분리된 세션의 재질 비교 상태다. */
	struct MATERIAL_RENDER_SETTINGS
	{
		bool_t bUseSourceMaterials = true;
		MATERIAL_DEBUG_VIEW eDebugView = MATERIAL_DEBUG_VIEW::FINAL;
		MAP_PBR_COMPARISON_SETTINGS MapPBR;
	};

	/* 장면 안개의 공통 입력이다. Deferred 합성은 복원한 월드 위치를 사용하고,
	   원본 캐릭터/맵 forward 경로도 같은 EvaluateSceneFog를 호출한다.
	   기본 높이 모델은 fTopHeight 아래의 높이·거리 감쇠를 계산한다.
	   drift는 shader의 presentation 시계로 높이/밀도를 흔들며 별도 CPU 안개 시뮬레이션은 없다.
	   bSourceExponential 경로는 별도 지수 적분식을 사용하며 drift/patch를 사용하지 않는다. */
	typedef struct tagHeightFogSettings
	{
		bool_t		bEnabled = false;
		float4_t	vColor = float4_t(0.55f, 0.62f, 0.72f, 1.f);
		f32_t		fDensity = 0.35f;
		f32_t		fHeightFalloff = 0.08f;
		f32_t		fTopHeight = 24.f;
		f32_t		fStartDistance = 0.f;
		f32_t		fMaximumOpacity = 0.9f;
		f32_t		fDriftSpeed = 0.f;
		f32_t		fDriftHeightAmplitude = 0.f;
		f32_t		fDriftDensityAmplitude = 0.f;
		/* 기본 높이 모델의 공간 noise 문턱이다. 1은 patch 감쇠를 생략하고 낮은 값은 영역을 줄인다.
		   정확한 맵 면적 비율을 보장하는 값은 아니다. 바람 방향과 시계로 world XZ 표본 위치를 이동한다. */
		f32_t		fCoveragePercent = 1.f;
		f32_t		fWindDirectionX = 1.f;
		f32_t		fWindDirectionZ = 0.f;
		f32_t		fWindSpeed = 0.f;
		f32_t		fPatchScale = 0.01f;
		f32_t		fPatchSoftness = 0.15f;
        // 원본 지수 안개 모델 선택이다. 전달하는 거리/높이는 런타임 미터 단위다.
        bool_t bSourceExponential = false;
        float4_t vInscatteringColor = float4_t(0.f, 0.f, 0.f, 0.f);
        // xyz는 방향광을 향하는 방향이며 w는 색 전환의 기준 각도 cosine이다.
        float4_t vFogLightDirection = float4_t(0.f, 1.f, 0.f, 0.f);
	}HEIGHT_FOG_SETTINGS;
}
