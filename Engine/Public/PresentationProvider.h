#pragma once

// 화면 합성 수치/행렬과 GPU view의 COM 소유 참조를 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// 화면 후처리 종류다. PREPARED_MATERIAL은 provider가 준비한 바인딩 객체를 소비한다.
enum class PRESENTATION_SCREEN_POST_PROFILE : uint8_t
{
	RGB_NOISE_RECONSTRUCTED,
	ZOOM_BLUR_RECONSTRUCTED,
	FILM_NOISE_RECONSTRUCTED,
	CHROMATIC_ABERRATION_RECONSTRUCTED,
	PREPARED_MATERIAL,
	END
};

// 실패를 해당 provider의 입력 문제와 공유 런타임 문제로 구분한다. HRESULT 값만으로 범위를 추측하지 않는다.
enum class PRESENTATION_FAILURE_SCOPE : uint8_t
{
	NONE,
	LOCAL_PROVIDER_CONTRACT,
	GLOBAL_RUNTIME,
};

// texture SRV의 sRGB 해석 여부와 일치해야 하는 색 공간 계약이다.
enum class PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE : uint8_t
{
	LINEAR,
	SRGB,
	END
};

// texture에서 coverage로 읽을 채널이다. 선택 채널의 존재 여부는 제출 검증에서 확인한다.
enum class PRESENTATION_SCREEN_OVERLAY_CHANNEL : uint8_t
{
	R,
	G,
	B,
	A,
	END
};

// overlay texture 샘플의 보간 방식을 정한다.
enum class PRESENTATION_SCREEN_OVERLAY_FILTER : uint8_t
{
	POINT,
	LINEAR,
	END
};

// overlay texture의 UV 범위 밖 주소 처리를 정한다.
enum class PRESENTATION_SCREEN_OVERLAY_ADDRESS : uint8_t
{
	CLAMP,
	WRAP,
	END
};

// Shader_Deferred의 텍스처 overlay pass와 맞춰야 하는 index다.
inline constexpr uint32_t PRESENTATION_TEXTURED_OVERLAY_PASS_INDEX = 14u;

// Renderer가 Bind 동안 제공하는 현재 scene view와 quad 변환이다. COM 멤버 복사는 자원 참조 수명을 연장한다.
struct PRESENTATION_SCREEN_POST_MATERIAL_INPUT final
{
	ComPtr<ID3D11ShaderResourceView> pSceneColor;
	ComPtr<ID3D11ShaderResourceView> pSceneBloom;
	ComPtr<ID3D11ShaderResourceView> pSceneDepth;
	float4x4_t World{};
	float4x4_t View{};
	float4x4_t Projection{};
};

// provider가 준비해 공유 소유로 넘기는 불변 바인딩 인터페이스다. target 전환과 draw는 Renderer가 유지한다.
class IPresentationScreenPostMaterial
{
public:
	virtual ~IPresentationScreenPostMaterial() = default;
	// 대기 중 frame이 소비될 때까지 재질 구현이 모든 program 입력을 소유한다.
	// 여기서는 바인딩만 하며 target 선택·ping-pong 순서·quad draw는 Renderer가 담당한다.
	virtual HRESULT Bind(const PRESENTATION_SCREEN_POST_MATERIAL_INPUT& Input) const = 0;
	// 캡처 scene 대체는 살아 있는 forward 이펙트보다 먼저, 일반 후처리는 그 뒤에 합성한다.
	virtual bool Replaces_SceneBeforeBlend() const
	{
		return false;
	}
};

// 한 번의 후처리 제출을 값으로 보관한다. sample 시간은 초이며 profile과 pMaterial의 유무가 일치해야 한다.
struct PRESENTATION_SCREEN_POST_DESC final
{
	PRESENTATION_SCREEN_POST_PROFILE eProfile =
		PRESENTATION_SCREEN_POST_PROFILE::END;
	uint32_t iSourceOrder = 0u;
	uint32_t iRandomSeed = 1u;
	f32_t fSampleTimeSeconds = 0.f;
	f32_t fIntensity = 0.f;
	f32_t fSecondaryIntensity = 0.f;
	f32_t fFrequency = 1.f;
	float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
	// queued frame이 소비될 때까지 재질의 소유 수명을 유지한다.
	std::shared_ptr<const IPresentationScreenPostMaterial> pMaterial;
};

// 한 texture overlay의 transform과 합성 설정이다. 위치/크기는 화면 UV 기준이고 회전은 degree, drift는 UV/초다.
struct PRESENTATION_SCREEN_OVERLAY_DESC final
{
	uint32_t iSourceOrder = 0u;
	f32_t fSampleTimeSeconds = 0.f;
	float2_t vPosition = { 0.5f, 0.5f };
	float2_t vScale = { 1.f, 1.f };
	f32_t fRotationDegrees = 0.f;
	f32_t fAngularVelocityDegreesPerSecond = 0.f;
	float2_t vUvDriftPerSecond = {};
	float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
	f32_t fAlpha = 1.f;
	f32_t fBloomIntensity = -1.f; // -1이면 scene의 benchmark bloom 기준을 따른다.
	bool_t bDisplaySpace = false; // display 이미지 경로는 scene exposure/bloom 적용을 거치지 않는다.
	PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE eColorSpace =
		PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE::LINEAR;
	PRESENTATION_SCREEN_OVERLAY_CHANNEL eCoverageChannel =
		PRESENTATION_SCREEN_OVERLAY_CHANNEL::A;
	PRESENTATION_SCREEN_OVERLAY_FILTER eFilter =
		PRESENTATION_SCREEN_OVERLAY_FILTER::LINEAR;
	PRESENTATION_SCREEN_OVERLAY_ADDRESS eAddress =
		PRESENTATION_SCREEN_OVERLAY_ADDRESS::CLAMP;
	// 제출 descriptor가 GPU view의 강한 참조를 보관한다. 파일 경로나 저작 원본을 소유하는 구조는 아니다.
	ComPtr<ID3D11ShaderResourceView> pTexture;
};

// 두 ping-pong target 사이 한 번의 합성에서 읽고 쓸 index다. 같은 target을 동시에 읽고 쓰지 않는다.
struct PRESENTATION_SCREEN_POST_PLAN_STEP final
{
	uint32_t iSourceTarget = 0u;
	uint32_t iDestinationTarget = 1u;
};

// 0번 target에서 시작해 pass마다 source/destination을 번갈아 고른다. GPU 상태 자체는 변경하지 않는다.
inline PRESENTATION_SCREEN_POST_PLAN_STEP
Build_PresentationScreenPostPlanStep(const size_t iPassIndex)
{
	PRESENTATION_SCREEN_POST_PLAN_STEP Step;
	Step.iSourceTarget = static_cast<uint32_t>(iPassIndex & 1u);
	Step.iDestinationTarget = 1u - Step.iSourceTarget;
	return Step;
}

// 완료한 pass 개수의 홀짝으로 최종 target을 찾는다. 0회면 처음 scene이 든 0번이다.
inline uint32_t PresentationScreenPostFinalTarget(
	const size_t iPostCount)
{
	return static_cast<uint32_t>(iPostCount & 1u);
}

// 앞선 post의 실제 합성 횟수 뒤에 overlay 순서를 이어 같은 ping-pong 규칙을 사용한다.
inline PRESENTATION_SCREEN_POST_PLAN_STEP
Build_PresentationScreenOverlayPlanStep(
	const size_t iScreenPostCount,
	const size_t iOverlayIndex)
{
	return Build_PresentationScreenPostPlanStep(
		iScreenPostCount + iOverlayIndex);
}

// 실제 소비한 post와 overlay 개수로 최종 target을 정한다. display-space overlay는 별도 렌더 경로에서 소비된다.
inline uint32_t PresentationScreenCompositionFinalTarget(
	const size_t iScreenPostCount,
	const size_t iOverlayCount)
{
	return PresentationScreenPostFinalTarget(
		iScreenPostCount + iOverlayCount);
}

// 한 프레임의 빛/후처리/overlay 제출자다. 관리자가 소유 참조를 보관하고 Begin→Submit→Finalize 경계를 호출한다.
class ENGINE_DLL IPresentationProvider
{
public:
	virtual ~IPresentationProvider() = default;
	// provider가 이번 제출의 후보 상태를 준비한다. 기본 구현은 아무 상태도 바꾸지 않는다.
	virtual void Begin_PresentationSubmission()
	{
	}
	// 관리자 Add_*와 예상 수 등록으로 이번 기여를 제출한다. 실패 범위와 격리 가능 여부도 provider가 설명해야 한다.
	virtual HRESULT Submit_Presentation() = 0;
	// 기본은 전체 실패다. true와 LOCAL_PROVIDER_CONTRACT가 모두 맞을 때만 해당 provider의 기여를 되돌린다.
	virtual bool_t Is_PresentationFailureIsolated() const
	{
		return false;
	}
	virtual PRESENTATION_FAILURE_SCOPE Get_PresentationFailureScope() const
	{
		return PRESENTATION_FAILURE_SCOPE::NONE;
	}
	// 이번 후보를 확정/취소하는 통지다. 이미 격리되어 false로 끝낸 provider에는 중복 호출하지 않는다.
	virtual void Finalize_PresentationSubmission(bool_t bCommitted)
	{
		(void)bCommitted;
	}
};

NS_END
