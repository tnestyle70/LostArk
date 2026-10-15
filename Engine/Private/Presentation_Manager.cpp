// 프레임 제출 후보의 소유·검증·확정/폐기를 구현한다.
#include "Presentation_Manager.h"
// 광원 종류/수신 대상과 설명 필드를 검증한다.
#include "Engine_RenderTypes.h"

// 빛과 화면 합성 수치의 유한성을 검사한다.
#include <cmath>

namespace
{
	// 흔한 크기를 위한 초기 capacity 예약이다. 제출 허용 개수의 상한은 아니다.
	constexpr size_t INITIAL_PRESENTATION_PROVIDERS = 256u;
	constexpr size_t INITIAL_TRANSIENT_LIGHTS = 384u;
	constexpr size_t INITIAL_SCREEN_POSTS = 64u;
	constexpr size_t INITIAL_SCREEN_OVERLAYS = 64u;

	// 색상·방향 등 네 성분이 유한한지 확인한다.
	bool IsFinite4(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	// 화면 위치·크기·UV drift 두 성분이 유한한지 확인한다.
	bool IsFinite2(const float2_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y);
	}

	// 일시적 빛의 종류·색상·기하·감쇠를 검증한다. 설명 값을 정규화하거나 현재 조명 옵션을 바꾸지 않는다.
	bool IsValidTransientLight(const LIGHT_DESC& Light)
	{
		if (Light.staticShadowChannel > 15u || (Light.eReceiver != LIGHT_RECEIVER::ALL &&
			Light.eReceiver != LIGHT_RECEIVER::SOURCE_CHARACTER &&
			Light.eReceiver != LIGHT_RECEIVER::UNBAKED) ||
			(LIGHT::POINT != Light.eType && LIGHT::SPOT != Light.eType &&
			LIGHT::DIRECTIONAL != Light.eType) ||
			!std::isfinite(Light.fFalloffExponent) || Light.fFalloffExponent <= 0.f ||
			!IsFinite4(Light.vDiffuse) || !IsFinite4(Light.vAmbient) ||
			!IsFinite4(Light.vSpecular) ||
			Light.vDiffuse.x < 0.f || Light.vDiffuse.y < 0.f || Light.vDiffuse.z < 0.f ||
			Light.vAmbient.x < 0.f || Light.vAmbient.y < 0.f || Light.vAmbient.z < 0.f ||
			Light.vSpecular.x < 0.f || Light.vSpecular.y < 0.f || Light.vSpecular.z < 0.f)
			return false;
		if (LIGHT::DIRECTIONAL != Light.eType &&
			(!IsFinite4(Light.vPosition) || !std::isfinite(Light.fRange) ||
			 Light.fRange <= 0.f))
			return false;
		if (LIGHT::POINT != Light.eType)
		{
			const float fDirectionLengthSquared =
				Light.vDirection.x * Light.vDirection.x +
				Light.vDirection.y * Light.vDirection.y +
				Light.vDirection.z * Light.vDirection.z;
			if (!IsFinite4(Light.vDirection) ||
				!std::isfinite(fDirectionLengthSquared) ||
				fDirectionLengthSquared <= 0.000001f)
				return false;
		}
		return LIGHT::SPOT != Light.eType ||
			(std::isfinite(Light.fSpotInnerCos) && std::isfinite(Light.fSpotOuterCos) &&
			 Light.fSpotOuterCos > 0.f && Light.fSpotOuterCos <= Light.fSpotInnerCos &&
			 Light.fSpotOuterCos < 1.f && Light.fSpotInnerCos <= 1.f);
	}

	// overlay의 선언 색 공간과 SRV format을 맞추는 sRGB 목록이다.
	bool IsSrgbFormat(const DXGI_FORMAT eFormat)
	{
		switch (eFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	// 선택한 coverage 성분이 형식에 존재하는지 확인한다. R은 UNKNOWN 이외 형식을 허용하고 다른 성분은 명시 목록으로 판정한다.
	bool HasOverlayCoverageChannel(
		const DXGI_FORMAT eFormat,
		const PRESENTATION_SCREEN_OVERLAY_CHANNEL eChannel)
	{
		if (eChannel >= PRESENTATION_SCREEN_OVERLAY_CHANNEL::END)
			return false;
		if (PRESENTATION_SCREEN_OVERLAY_CHANNEL::R == eChannel)
			return DXGI_FORMAT_UNKNOWN != eFormat;

		const bool_t bHasRgb =
			eFormat == DXGI_FORMAT_R8G8B8A8_UNORM ||
			eFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_B8G8R8X8_UNORM ||
			eFormat == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_R16G16B16A16_FLOAT ||
			eFormat == DXGI_FORMAT_R32G32B32A32_FLOAT ||
			eFormat == DXGI_FORMAT_BC1_UNORM ||
			eFormat == DXGI_FORMAT_BC1_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC2_UNORM ||
			eFormat == DXGI_FORMAT_BC2_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC3_UNORM ||
			eFormat == DXGI_FORMAT_BC3_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC7_UNORM ||
			eFormat == DXGI_FORMAT_BC7_UNORM_SRGB;
		if (PRESENTATION_SCREEN_OVERLAY_CHANNEL::G == eChannel)
		{
			return bHasRgb || eFormat == DXGI_FORMAT_R8G8_UNORM ||
				eFormat == DXGI_FORMAT_R16G16_FLOAT ||
				eFormat == DXGI_FORMAT_R32G32_FLOAT;
		}
		if (PRESENTATION_SCREEN_OVERLAY_CHANNEL::B == eChannel)
			return bHasRgb;
		return eFormat == DXGI_FORMAT_R8G8B8A8_UNORM ||
			eFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_R16G16B16A16_FLOAT ||
			eFormat == DXGI_FORMAT_R32G32B32A32_FLOAT ||
			// BC1도 alpha를 제공한다. 불투명 block은 1, 투명 mode는 0/1을 반환한다.
			eFormat == DXGI_FORMAT_BC1_UNORM ||
			eFormat == DXGI_FORMAT_BC1_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC2_UNORM ||
			eFormat == DXGI_FORMAT_BC2_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC3_UNORM ||
			eFormat == DXGI_FORMAT_BC3_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC7_UNORM ||
			eFormat == DXGI_FORMAT_BC7_UNORM_SRGB;
	}

	// descriptor 수치와 Texture2D SRV, coverage 채널, 색 공간의 일치를 검사한다. GPU view를 생성하거나 texture 내용을 읽지는 않는다.
	bool IsValidScreenOverlay(
		const PRESENTATION_SCREEN_OVERLAY_DESC& Desc)
	{
		if (nullptr == Desc.pTexture ||
			Desc.eColorSpace >=
				PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE::END ||
			Desc.eCoverageChannel >=
				PRESENTATION_SCREEN_OVERLAY_CHANNEL::END ||
			Desc.eFilter >= PRESENTATION_SCREEN_OVERLAY_FILTER::END ||
			Desc.eAddress >= PRESENTATION_SCREEN_OVERLAY_ADDRESS::END ||
			!std::isfinite(Desc.fSampleTimeSeconds) ||
			Desc.fSampleTimeSeconds < 0.f ||
			!IsFinite2(Desc.vPosition) || !IsFinite2(Desc.vScale) ||
			Desc.vScale.x <= 0.f || Desc.vScale.y <= 0.f ||
			!std::isfinite(Desc.fRotationDegrees) ||
			!std::isfinite(Desc.fAngularVelocityDegreesPerSecond) ||
			!IsFinite2(Desc.vUvDriftPerSecond) ||
			!IsFinite4(Desc.vTint) || !std::isfinite(Desc.fAlpha) ||
			Desc.fAlpha < 0.f || Desc.fAlpha > 1.f ||
			!std::isfinite(Desc.fBloomIntensity) || (Desc.fBloomIntensity != -1.f && Desc.fBloomIntensity < 0.f) || Desc.fBloomIntensity > 16.f)
		{
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc{};
		Desc.pTexture->GetDesc(&SrvDesc);
		if (D3D11_SRV_DIMENSION_TEXTURE2D != SrvDesc.ViewDimension ||
			!HasOverlayCoverageChannel(
				SrvDesc.Format, Desc.eCoverageChannel))
		{
			return false;
		}
		const bool_t bSrgb = IsSrgbFormat(SrvDesc.Format);
		return bSrgb ==
			(Desc.eColorSpace ==
				PRESENTATION_SCREEN_OVERLAY_COLOR_SPACE::SRGB);
	}

	// 시도 수를 늘리고 정확한 S_OK/S_FALSE만 accepted/suppressed로 분류한다. 그 외 HRESULT는 failed다.
	void Record_SubmissionResult(
		PRESENTATION_CHANNEL_SUBMISSION_STATS& Stats,
		const HRESULT hResult)
	{
		++Stats.iAttempted;
		if (S_OK == hResult)
			++Stats.iAccepted;
		else if (S_FALSE == hResult)
			++Stats.iSuppressed;
		else
			++Stats.iFailed;
	}

	// expected≤configured와 실제 시도 수의 일치, 모든 시도가 accepted/suppressed인지 확인한다.
	bool Is_CompleteChannelSubmission(
		const PRESENTATION_CHANNEL_SUBMISSION_STATS& Stats)
	{
		return Stats.iExpected <= Stats.iConfigured &&
			Stats.iAttempted == Stats.iExpected &&
			Stats.iAccepted + Stats.iSuppressed == Stats.iAttempted &&
			0u == Stats.iFailed;
	}
}

// 초기 capacity를 예약한다. 이후 frame의 제출 개수에 이 값을 상한으로 적용하지 않는다.
CPresentation_Manager::CPresentation_Manager()
{
	m_FrameProviders.reserve(INITIAL_PRESENTATION_PROVIDERS);
	m_TransientLights.reserve(INITIAL_TRANSIENT_LIGHTS);
	m_ScreenPosts.reserve(INITIAL_SCREEN_POSTS);
	m_ScreenOverlays.reserve(INITIAL_SCREEN_OVERLAYS);
}

// process 수명의 singleton을 반환한다. 개별 frame 종료는 Clear_Frame으로 소비 참조를 정리한다.
CPresentation_Manager& CPresentation_Manager::Get()
{
	static CPresentation_Manager Instance;
	return Instance;
}

// 객체 주소로 중복을 구분하고 소유 참조를 등록한다. null은 다음 Submit까지 전역 실패로 기억한다.
HRESULT CPresentation_Manager::Add_FrameProvider(
	shared_ptr<IPresentationProvider> pProvider)
{
	if (nullptr == pProvider)
	{
		m_hPendingProviderFailure = E_FAIL;
		m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::GLOBAL_RUNTIME;
		return E_FAIL;
	}
	const auto Iterator = std::find_if(
		m_FrameProviders.begin(), m_FrameProviders.end(),
		[&pProvider](const shared_ptr<IPresentationProvider>& Existing)
		{
			return Existing.get() == pProvider.get();
		});
	if (Iterator != m_FrameProviders.end())
		return S_FALSE;
	m_FrameProviders.push_back(std::move(pProvider));
	return S_OK;
}

// Renderer가 frame 렌더 전에 호출한다. 앞선 frame 후보를 비우고 이번 provider 기여를 transaction으로 모은다.
HRESULT CPresentation_Manager::Submit_FrameProviders()
{
	// 보류 실패는 한 번 소비한다. 마지막 집계와 candidate 배열을 이번 제출 기준으로 초기화한다.
	const HRESULT hPendingProviderFailure = m_hPendingProviderFailure;
	m_hPendingProviderFailure = S_OK;
	m_LastSubmissionStats = {};
	m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	m_iLastTransientLightCount = 0u;
	m_iLastScreenPostCount = 0u;
	m_iLastScreenOverlayCount = 0u;
	m_bSubmissionTransactionActive = true;
	m_TransientLights.clear();
	m_ScreenPosts.clear();
	m_ScreenOverlays.clear();
	// 등록된 provider 목록을 지역 소유로 인계한다. Finalize가 끝날 때까지 객체가 살아 있게 한다.
	vector<shared_ptr<IPresentationProvider>> Providers =
		std::move(m_FrameProviders);
	m_FrameProviders.clear();
	bool_t bIsolatedProviderFailureSeen = false;
	// 아직 참여 중인 provider에 한 번씩 결과를 통지한다. 격리 취소한 항목은 reset되어 아래 반복에서 제외된다.
	const auto FinalizeProviders =
		[&Providers](const bool_t bCommitted)
		{
			for (const shared_ptr<IPresentationProvider>& Provider :
				Providers)
			{
				if (nullptr != Provider)
					Provider->Finalize_PresentationSubmission(bCommitted);
			}
		};
	for (const shared_ptr<IPresentationProvider>& Provider : Providers)
	{
		if (nullptr != Provider)
			Provider->Begin_PresentationSubmission();
	}
	// 등록 오류가 있으면 Begin으로 준비한 모든 provider를 false로 종료하고 이번 후보를 폐기한다.
	if (FAILED(hPendingProviderFailure))
	{
		++m_LastSubmissionStats.iProviderFailures;
		m_LastSubmissionStats.bCompleted = true;
		m_LastSubmissionStats.bCommitted = false;
		m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::GLOBAL_RUNTIME;
		m_bSubmissionTransactionActive = false;
		FinalizeProviders(false);
		Clear_Frame();
		return hPendingProviderFailure;
	}
	for (shared_ptr<IPresentationProvider>& Provider : Providers)
	{
		if (nullptr == Provider)
		{
			++m_LastSubmissionStats.iProviderFailures;
			m_LastSubmissionStats.bCompleted = true;
			m_LastSubmissionStats.bCommitted = false;
			m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::GLOBAL_RUNTIME;
			m_bSubmissionTransactionActive = false;
			FinalizeProviders(false);
			Clear_Frame();
			return E_FAIL;
		}
		// 각 provider의 기여 직전 크기와 집계를 기록한다. 격리 가능한 local 실패는
		// 해당 기여만 취소해 다른 소유자의 정상 capture·빛·overlay를 보존한다.
		const size_t iLightCheckpoint = m_TransientLights.size();
		const size_t iPostCheckpoint = m_ScreenPosts.size();
		const size_t iOverlayCheckpoint = m_ScreenOverlays.size();
		const PRESENTATION_SUBMISSION_STATS StatsCheckpoint = m_LastSubmissionStats;
		const HRESULT hProviderResult = Provider->Submit_Presentation();
		if (FAILED(hProviderResult))
		{
			const PRESENTATION_FAILURE_SCOPE eFailureScope =
				Provider->Get_PresentationFailureScope();
			// 실패 원인을 아는 provider의 범위를 frame 확정까지 그대로 보존한다.
			// HRESULT 값만으로 객체의 local 실패인지 공유 runtime 실패인지
			// 추측하지 않는다.
			m_eLastFailureScope = eFailureScope;
			const bool_t bIsolatedProviderFailure =
				Provider->Is_PresentationFailureIsolated() &&
				eFailureScope ==
					PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
			if (bIsolatedProviderFailure)
			{
				m_TransientLights.resize(iLightCheckpoint);
				m_ScreenPosts.resize(iPostCheckpoint);
				m_ScreenOverlays.resize(iOverlayCheckpoint);
				m_LastSubmissionStats = StatsCheckpoint;
				++m_LastSubmissionStats.iProviderFailures;
				bIsolatedProviderFailureSeen = true;
				Provider->Finalize_PresentationSubmission(false);
				// 이미 false로 종료했으므로 이후 전역 취소가 다시 Finalize하지 않도록 참조를 비운다.
				Provider.reset();
				continue;
			}
			++m_LastSubmissionStats.iProviderFailures;
			m_LastSubmissionStats.bCompleted = true;
			m_LastSubmissionStats.bCommitted = false;
			m_bSubmissionTransactionActive = false;
			FinalizeProviders(false);
			Clear_Frame();
			return hProviderResult;
		}
	}
	// 채널별 제출 약속이 맞지 않으면 전역 실패로 취소한다. 일부 수락된 후보도 함께 버린다.
	if (!Is_CompleteChannelSubmission(m_LastSubmissionStats.Lights) ||
		!Is_CompleteChannelSubmission(m_LastSubmissionStats.ScreenPosts) ||
		!Is_CompleteChannelSubmission(m_LastSubmissionStats.ScreenOverlays))
	{
		++m_LastSubmissionStats.iProviderFailures;
		m_LastSubmissionStats.bCompleted = true;
		m_LastSubmissionStats.bCommitted = false;
		m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::GLOBAL_RUNTIME;
		m_bSubmissionTransactionActive = false;
		FinalizeProviders(false);
		Clear_Frame();
		return E_FAIL;
	}
	m_iLastTransientLightCount = static_cast<uint32_t>(m_TransientLights.size());
	m_iLastScreenPostCount = static_cast<uint32_t>(m_ScreenPosts.size());
	// overlay의 source order를 맞추고 같은 순서는 제출 순서를 보존한다. GPU 그리기는 이후 Renderer가 수행한다.
	std::stable_sort(m_ScreenOverlays.begin(), m_ScreenOverlays.end(),
		[](const PRESENTATION_SCREEN_OVERLAY_DESC& Left,
			const PRESENTATION_SCREEN_OVERLAY_DESC& Right)
		{
			return Left.iSourceOrder < Right.iSourceOrder;
		});
	m_iLastScreenOverlayCount =
		static_cast<uint32_t>(m_ScreenOverlays.size());
	m_LastSubmissionStats.bCompleted = true;
	// 후보 배열을 소비 가능 상태로 확정하고 provider에 통지한다. 억제/격리 실패가 있으면 S_FALSE로 결과의 차이를 알린다.
	m_LastSubmissionStats.bCommitted = true;
	m_bSubmissionTransactionActive = false;
	FinalizeProviders(true);
	if (bIsolatedProviderFailureSeen)
		m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	return bIsolatedProviderFailureSeen || 0u < m_LastSubmissionStats.Lights.iSuppressed ||
		0u < m_LastSubmissionStats.ScreenPosts.iSuppressed ||
		0u < m_LastSubmissionStats.ScreenOverlays.iSuppressed ? S_FALSE : S_OK;
}

// provider가 제출한 빛을 검증 후 복사한다. 채널이 꺼져 있으면 입력 검증 전에 S_FALSE로 억제한다.
HRESULT CPresentation_Manager::Add_TransientLight(
	const LIGHT_DESC& LightDesc)
{
	HRESULT hResult = S_OK;
	m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	if (!m_bTransientLightsEnabled)
	{
		hResult = S_FALSE;
	}
	else if (!IsValidTransientLight(LightDesc))
	{
		hResult = E_FAIL;
		m_eLastFailureScope =
			PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	}
	else
	{
		m_TransientLights.push_back(LightDesc);
	}
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(m_LastSubmissionStats.Lights, hResult);
	return hResult;
}

// profile과 준비 재질의 짝, 시간/강도/seed를 검사한 뒤 소유 descriptor를 보관한다.
HRESULT CPresentation_Manager::Add_ScreenPost(
	const PRESENTATION_SCREEN_POST_DESC& ScreenPostDesc)
{
	HRESULT hResult = S_OK;
	m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	if (!m_bScreenPostsEnabled)
	{
		hResult = S_FALSE;
	}
	else if ((ScreenPostDesc.eProfile == PRESENTATION_SCREEN_POST_PROFILE::PREPARED_MATERIAL) !=
		static_cast<bool>(ScreenPostDesc.pMaterial) ||
		ScreenPostDesc.eProfile >= PRESENTATION_SCREEN_POST_PROFILE::END ||
		0u == ScreenPostDesc.iRandomSeed ||
		!std::isfinite(ScreenPostDesc.fSampleTimeSeconds) ||
		ScreenPostDesc.fSampleTimeSeconds < 0.f ||
		!std::isfinite(ScreenPostDesc.fIntensity) ||
		ScreenPostDesc.fIntensity < 0.f ||
		!std::isfinite(ScreenPostDesc.fSecondaryIntensity) ||
		ScreenPostDesc.fSecondaryIntensity < 0.f ||
		!std::isfinite(ScreenPostDesc.fFrequency) ||
		ScreenPostDesc.fFrequency < 0.f ||
		!IsFinite4(ScreenPostDesc.vTint))
	{
		hResult = E_FAIL;
		m_eLastFailureScope =
			PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	}
	else
	{
		m_ScreenPosts.push_back(ScreenPostDesc);
	}
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(m_LastSubmissionStats.ScreenPosts, hResult);
	return hResult;
}

// 화면 합성 descriptor를 검증하고 texture COM 참조를 복사해 이번 frame 동안 유지한다.
HRESULT CPresentation_Manager::Add_ScreenOverlay(
	const PRESENTATION_SCREEN_OVERLAY_DESC& ScreenOverlayDesc)
{
	HRESULT hResult = S_OK;
	m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	if (!m_bScreenOverlaysEnabled)
	{
		hResult = S_FALSE;
	}
	else if (!IsValidScreenOverlay(ScreenOverlayDesc))
	{
		hResult = E_FAIL;
		m_eLastFailureScope =
			PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	}
	else
	{
		m_ScreenOverlays.push_back(ScreenOverlayDesc);
	}
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(
			m_LastSubmissionStats.ScreenOverlays, hResult);
	return hResult;
}

// 활성 transaction에서만 빛/후처리의 configured/expected를 누적한다. 이 함수 자체는 수락 여부를 판정하지 않는다.
void CPresentation_Manager::Register_ProviderSubmissionExpectation(
	const uint64_t iConfiguredLightCount,
	const uint64_t iExpectedLightCount,
	const uint64_t iConfiguredScreenPostCount,
	const uint64_t iExpectedScreenPostCount)
{
	if (!m_bSubmissionTransactionActive)
		return;
	m_LastSubmissionStats.Lights.iConfigured += iConfiguredLightCount;
	m_LastSubmissionStats.Lights.iExpected += iExpectedLightCount;
	m_LastSubmissionStats.ScreenPosts.iConfigured +=
		iConfiguredScreenPostCount;
	m_LastSubmissionStats.ScreenPosts.iExpected += iExpectedScreenPostCount;
}

// overlay 제출의 예상 수를 별도로 누적해 frame 종료의 약속 검사에 사용한다.
void CPresentation_Manager::Register_ProviderScreenOverlayExpectation(
	const uint64_t iConfiguredScreenOverlayCount,
	const uint64_t iExpectedScreenOverlayCount)
{
	if (!m_bSubmissionTransactionActive)
		return;
	m_LastSubmissionStats.ScreenOverlays.iConfigured +=
		iConfiguredScreenOverlayCount;
	m_LastSubmissionStats.ScreenOverlays.iExpected +=
		iExpectedScreenOverlayCount;
}

// Add 전에 거부된 빛 입력도 제출 실패 수에 포함한다. 실패 범위는 항상 local contract로 기록한다.
void CPresentation_Manager::Record_TransientLightValidationFailure()
{
	m_eLastFailureScope =
		PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(m_LastSubmissionStats.Lights, E_INVALIDARG);
}

// provider의 선행 후처리 검증 실패를 현재 transaction 집계에 전달한다.
void CPresentation_Manager::Record_ScreenPostValidationFailure()
{
	m_eLastFailureScope =
		PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(m_LastSubmissionStats.ScreenPosts, E_INVALIDARG);
}

// provider의 선행 overlay 검증 실패를 현재 transaction 집계에 전달한다.
void CPresentation_Manager::Record_ScreenOverlayValidationFailure()
{
	m_eLastFailureScope =
		PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(
			m_LastSubmissionStats.ScreenOverlays, E_INVALIDARG);
}

// 보관한 빛만 비운다. 최근 제출 집계와 표시용 개수는 유지한다.
void CPresentation_Manager::Clear_TransientLights()
{
	m_TransientLights.clear();
}

// 소비가 끝난 후처리 descriptor와 그 재질 소유 참조를 놓는다.
void CPresentation_Manager::Clear_ScreenPosts()
{
	m_ScreenPosts.clear();
}

// 소비가 끝난 overlay descriptor와 texture COM 참조를 놓는다.
void CPresentation_Manager::Clear_ScreenOverlays()
{
	m_ScreenOverlays.clear();
}

// Renderer의 frame 종료/실패 경로가 후보와 대기 provider를 비운다. 보류 등록 실패나 마지막 집계는 이 함수에서 지우지 않는다.
void CPresentation_Manager::Clear_Frame()
{
	m_FrameProviders.clear();
	m_TransientLights.clear();
	m_ScreenPosts.clear();
	m_ScreenOverlays.clear();
}
