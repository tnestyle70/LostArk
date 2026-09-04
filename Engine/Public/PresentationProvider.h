#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

enum class PRESENTATION_SCREEN_POST_PROFILE : uint8_t
{
	RGB_NOISE_RECONSTRUCTED,
	ZOOM_BLUR_RECONSTRUCTED,
	FILM_NOISE_RECONSTRUCTED,
	CHROMATIC_ABERRATION_RECONSTRUCTED,
	END
};

enum class PRESENTATION_FAILURE_SCOPE : uint8_t
{
	NONE,
	LOCAL_PROVIDER_CONTRACT,
	GLOBAL_RUNTIME,
};

enum class PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE : uint8_t
{
	LINEAR,
	SRGB,
	END
};

enum class PRESENTATION_SCREEN_OVERLAY_CHANNEL : uint8_t
{
	R,
	G,
	B,
	A,
	END
};

enum class PRESENTATION_SCREEN_OVERLAY_FILTER : uint8_t
{
	POINT,
	LINEAR,
	END
};

enum class PRESENTATION_SCREEN_OVERLAY_ADDRESS : uint8_t
{
	CLAMP,
	WRAP,
	END
};

inline constexpr uint32_t PRESENTATION_TEXTURED_OVERLAY_PASS_INDEX = 14u;

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
};

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
	PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE eColorSpace =
		PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE::LINEAR;
	PRESENTATION_SCREEN_OVERLAY_CHANNEL eCoverageChannel =
		PRESENTATION_SCREEN_OVERLAY_CHANNEL::A;
	PRESENTATION_SCREEN_OVERLAY_FILTER eFilter =
		PRESENTATION_SCREEN_OVERLAY_FILTER::LINEAR;
	PRESENTATION_SCREEN_OVERLAY_ADDRESS eAddress =
		PRESENTATION_SCREEN_OVERLAY_ADDRESS::CLAMP;
	ComPtr<ID3D11ShaderResourceView> pTexture;
};

struct PRESENTATION_SCREEN_POST_PLAN_STEP final
{
	uint32_t iSourceTarget = 0u;
	uint32_t iDestinationTarget = 1u;
};

inline PRESENTATION_SCREEN_POST_PLAN_STEP
Build_PresentationScreenPostPlanStep(const size_t iPassIndex)
{
	PRESENTATION_SCREEN_POST_PLAN_STEP Step;
	Step.iSourceTarget = static_cast<uint32_t>(iPassIndex & 1u);
	Step.iDestinationTarget = 1u - Step.iSourceTarget;
	return Step;
}

inline uint32_t PresentationScreenPostFinalTarget(
	const size_t iPostCount)
{
	return static_cast<uint32_t>(iPostCount & 1u);
}

inline PRESENTATION_SCREEN_POST_PLAN_STEP
Build_PresentationScreenOverlayPlanStep(
	const size_t iScreenPostCount,
	const size_t iOverlayIndex)
{
	return Build_PresentationScreenPostPlanStep(
		iScreenPostCount + iOverlayIndex);
}

inline uint32_t PresentationScreenCompositionFinalTarget(
	const size_t iScreenPostCount,
	const size_t iOverlayCount)
{
	return PresentationScreenPostFinalTarget(
		iScreenPostCount + iOverlayCount);
}

class ENGINE_DLL IPresentationProvider
{
public:
	virtual ~IPresentationProvider() = default;
	virtual void Begin_PresentationSubmission() {}
	virtual HRESULT Submit_Presentation() = 0;
	virtual bool_t Is_PresentationFailureIsolated() const
	{
		return false;
	}
	virtual PRESENTATION_FAILURE_SCOPE Get_PresentationFailureScope() const
	{
		return PRESENTATION_FAILURE_SCOPE::NONE;
	}
	virtual void Finalize_PresentationSubmission(bool_t bCommitted)
	{
		(void)bCommitted;
	}
};

NS_END
