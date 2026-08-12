#include "Presentation_Manager.h"

#include <cmath>

namespace
{
	constexpr size_t MAX_PRESENTATION_PROVIDERS = 256u;
	constexpr size_t MAX_TRANSIENT_LIGHTS = 64u;
	constexpr size_t MAX_SCREEN_POSTS = 64u;

	bool IsFinite4(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

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

	bool Is_CompleteChannelSubmission(
		const PRESENTATION_CHANNEL_SUBMISSION_STATS& Stats)
	{
		return Stats.iExpected <= Stats.iConfigured &&
			Stats.iAttempted == Stats.iExpected &&
			Stats.iAccepted + Stats.iSuppressed == Stats.iAttempted &&
			0u == Stats.iFailed;
	}
}

CPresentation_Manager::CPresentation_Manager()
{
	m_FrameProviders.reserve(MAX_PRESENTATION_PROVIDERS);
	m_TransientLights.reserve(MAX_TRANSIENT_LIGHTS);
	m_ScreenPosts.reserve(MAX_SCREEN_POSTS);
}

CPresentation_Manager& CPresentation_Manager::Get()
{
	static CPresentation_Manager Instance;
	return Instance;
}

HRESULT CPresentation_Manager::Add_FrameProvider(
	shared_ptr<IPresentationProvider> pProvider)
{
	if (nullptr == pProvider ||
		m_FrameProviders.size() >= MAX_PRESENTATION_PROVIDERS)
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

HRESULT CPresentation_Manager::Submit_FrameProviders()
{
	const HRESULT hPendingProviderFailure = m_hPendingProviderFailure;
	m_hPendingProviderFailure = S_OK;
	m_LastSubmissionStats = {};
	m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	m_iLastTransientLightCount = 0u;
	m_iLastScreenPostCount = 0u;
	m_bSubmissionTransactionActive = true;
	m_TransientLights.clear();
	m_ScreenPosts.clear();
	vector<shared_ptr<IPresentationProvider>> Providers =
		std::move(m_FrameProviders);
	m_FrameProviders.clear();
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
	for (const shared_ptr<IPresentationProvider>& Provider : Providers)
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
		const HRESULT hProviderResult = Provider->Submit_Presentation();
		if (FAILED(hProviderResult))
		{
			const PRESENTATION_FAILURE_SCOPE eFailureScope =
				Provider->Get_PresentationFailureScope();
			// The provider owns failure provenance.  Preserve that exact scope
			// through frame finalization; the HRESULT value must never be used to
			// guess whether the failure is object-local or shared runtime state.
			m_eLastFailureScope = eFailureScope;
			const bool_t bIsolatedProviderFailure =
				Provider->Is_PresentationFailureIsolated() &&
				eFailureScope ==
					PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
			++m_LastSubmissionStats.iProviderFailures;
			m_LastSubmissionStats.bCompleted = true;
			m_LastSubmissionStats.bCommitted = false;
			m_bSubmissionTransactionActive = false;
			FinalizeProviders(false);
			Clear_Frame();
			return bIsolatedProviderFailure ? S_FALSE : hProviderResult;
		}
	}
	if (!Is_CompleteChannelSubmission(m_LastSubmissionStats.Lights) ||
		!Is_CompleteChannelSubmission(m_LastSubmissionStats.ScreenPosts))
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
	m_LastSubmissionStats.bCompleted = true;
	m_LastSubmissionStats.bCommitted = true;
	m_bSubmissionTransactionActive = false;
	FinalizeProviders(true);
	return 0u < m_LastSubmissionStats.Lights.iSuppressed ||
		0u < m_LastSubmissionStats.ScreenPosts.iSuppressed ? S_FALSE : S_OK;
}

HRESULT CPresentation_Manager::Add_TransientLight(
	const LIGHT_DESC& LightDesc)
{
	HRESULT hResult = S_OK;
	m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	if (!m_bTransientLightsEnabled)
	{
		hResult = S_FALSE;
	}
	else if (LIGHT::POINT != LightDesc.eType ||
		!std::isfinite(LightDesc.fRange) || LightDesc.fRange <= 0.f ||
		!std::isfinite(LightDesc.fFalloffExponent) ||
		LightDesc.fFalloffExponent <= 0.f ||
		!IsFinite4(LightDesc.vPosition) ||
		!IsFinite4(LightDesc.vDiffuse) ||
		!IsFinite4(LightDesc.vAmbient))
	{
		hResult = E_FAIL;
		m_eLastFailureScope =
			PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	}
	else if (m_TransientLights.size() >= MAX_TRANSIENT_LIGHTS)
	{
		hResult = E_FAIL;
		m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::GLOBAL_RUNTIME;
	}
	else
	{
		m_TransientLights.push_back(LightDesc);
	}
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(m_LastSubmissionStats.Lights, hResult);
	return hResult;
}

HRESULT CPresentation_Manager::Add_ScreenPost(
	const PRESENTATION_SCREEN_POST_DESC& ScreenPostDesc)
{
	HRESULT hResult = S_OK;
	m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	if (!m_bScreenPostsEnabled)
	{
		hResult = S_FALSE;
	}
	else if (ScreenPostDesc.eProfile >= PRESENTATION_SCREEN_POST_PROFILE::END ||
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
	else if (m_ScreenPosts.size() >= MAX_SCREEN_POSTS)
	{
		hResult = E_FAIL;
		m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::GLOBAL_RUNTIME;
	}
	else
	{
		m_ScreenPosts.push_back(ScreenPostDesc);
	}
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(m_LastSubmissionStats.ScreenPosts, hResult);
	return hResult;
}

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

void CPresentation_Manager::Record_TransientLightValidationFailure()
{
	m_eLastFailureScope =
		PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(m_LastSubmissionStats.Lights, E_INVALIDARG);
}

void CPresentation_Manager::Record_ScreenPostValidationFailure()
{
	m_eLastFailureScope =
		PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	if (m_bSubmissionTransactionActive)
		Record_SubmissionResult(m_LastSubmissionStats.ScreenPosts, E_INVALIDARG);
}

void CPresentation_Manager::Clear_TransientLights()
{
	m_TransientLights.clear();
}

void CPresentation_Manager::Clear_ScreenPosts()
{
	m_ScreenPosts.clear();
}

void CPresentation_Manager::Clear_Frame()
{
	m_FrameProviders.clear();
	m_TransientLights.clear();
	m_ScreenPosts.clear();
}
