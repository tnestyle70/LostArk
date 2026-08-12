#pragma once

#include "PresentationProvider.h"

NS_BEGIN(Engine)

struct PRESENTATION_CHANNEL_SUBMISSION_STATS final
{
	uint64_t iConfigured = 0u;
	uint64_t iExpected = 0u;
	uint64_t iAttempted = 0u;
	uint64_t iAccepted = 0u;
	uint64_t iSuppressed = 0u;
	uint64_t iFailed = 0u;
};

struct PRESENTATION_SUBMISSION_STATS final
{
	PRESENTATION_CHANNEL_SUBMISSION_STATS Lights;
	PRESENTATION_CHANNEL_SUBMISSION_STATS ScreenPosts;
	uint64_t iProviderFailures = 0u;
	bool_t bCompleted = false;
	bool_t bCommitted = false;
};

class ENGINE_DLL CPresentation_Manager final
{
private:
	CPresentation_Manager();

public:
	static CPresentation_Manager& Get();

	HRESULT Add_FrameProvider(shared_ptr<IPresentationProvider> pProvider);
	HRESULT Submit_FrameProviders();
	HRESULT Add_TransientLight(const LIGHT_DESC& LightDesc);
	HRESULT Add_ScreenPost(
		const PRESENTATION_SCREEN_POST_DESC& ScreenPostDesc);
	PRESENTATION_FAILURE_SCOPE Get_LastFailureScope() const
	{
		return m_eLastFailureScope;
	}
	void Register_ProviderSubmissionExpectation(
		uint64_t iConfiguredLightCount,
		uint64_t iExpectedLightCount,
		uint64_t iConfiguredScreenPostCount,
		uint64_t iExpectedScreenPostCount);
	void Record_TransientLightValidationFailure();
	void Record_ScreenPostValidationFailure();
	const PRESENTATION_SUBMISSION_STATS& Get_LastSubmissionStats() const
	{
		return m_LastSubmissionStats;
	}
	const vector<LIGHT_DESC>& Get_TransientLights() const
	{
		return m_TransientLights;
	}
	const vector<PRESENTATION_SCREEN_POST_DESC>& Get_ScreenPosts() const
	{
		return m_ScreenPosts;
	}
	void Clear_TransientLights();
	void Clear_ScreenPosts();
	void Clear_Frame();
	void Set_TransientLightsEnabled(bool_t bEnabled) {
		m_bTransientLightsEnabled = bEnabled;
	}
	void Set_ScreenPostsEnabled(bool_t bEnabled) {
		m_bScreenPostsEnabled = bEnabled;
	}
	bool_t Are_TransientLightsEnabled() const {
		return m_bTransientLightsEnabled;
	}
	bool_t Are_ScreenPostsEnabled() const {
		return m_bScreenPostsEnabled;
	}
	uint32_t Get_LastTransientLightCount() const {
		return m_iLastTransientLightCount;
	}
	uint32_t Get_LastScreenPostCount() const {
		return m_iLastScreenPostCount;
	}

private:
	vector<shared_ptr<IPresentationProvider>> m_FrameProviders;
	vector<LIGHT_DESC> m_TransientLights;
	vector<PRESENTATION_SCREEN_POST_DESC> m_ScreenPosts;
	bool_t m_bTransientLightsEnabled = true;
	bool_t m_bScreenPostsEnabled = true;
	bool_t m_bSubmissionTransactionActive = false;
	uint32_t m_iLastTransientLightCount = {};
	uint32_t m_iLastScreenPostCount = {};
	PRESENTATION_SUBMISSION_STATS m_LastSubmissionStats;
	PRESENTATION_FAILURE_SCOPE m_eLastFailureScope =
		PRESENTATION_FAILURE_SCOPE::NONE;
	HRESULT m_hPendingProviderFailure = S_OK;
};

NS_END
