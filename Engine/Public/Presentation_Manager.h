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
	PRESENTATION_CHANNEL_SUBMISSION_STATS ScreenOverlays;
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
	HRESULT Add_ScreenOverlay(
		const PRESENTATION_SCREEN_OVERLAY_DESC& ScreenOverlayDesc);
	PRESENTATION_FAILURE_SCOPE Get_LastFailureScope() const
	{
		return m_eLastFailureScope;
	}
	void Register_ProviderSubmissionExpectation(
		uint64_t iConfiguredLightCount,
		uint64_t iExpectedLightCount,
		uint64_t iConfiguredScreenPostCount,
		uint64_t iExpectedScreenPostCount);
	void Register_ProviderScreenOverlayExpectation(
		uint64_t iConfiguredScreenOverlayCount,
		uint64_t iExpectedScreenOverlayCount);
	void Record_TransientLightValidationFailure();
	void Record_ScreenPostValidationFailure();
	void Record_ScreenOverlayValidationFailure();
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
	const vector<PRESENTATION_SCREEN_OVERLAY_DESC>& Get_ScreenOverlays() const
	{
		return m_ScreenOverlays;
	}
	void Clear_TransientLights();
	void Clear_ScreenPosts();
	void Clear_ScreenOverlays();
	void Clear_Frame();
	void Set_TransientLightsEnabled(bool_t bEnabled) {
		m_bTransientLightsEnabled = bEnabled;
	}
	void Set_ScreenPostsEnabled(bool_t bEnabled) {
		m_bScreenPostsEnabled = bEnabled;
	}
	void Set_ScreenOverlaysEnabled(bool_t bEnabled) {
		m_bScreenOverlaysEnabled = bEnabled;
	}
	bool_t Are_TransientLightsEnabled() const {
		return m_bTransientLightsEnabled;
	}
	bool_t Are_ScreenPostsEnabled() const {
		return m_bScreenPostsEnabled;
	}
	bool_t Are_ScreenOverlaysEnabled() const {
		return m_bScreenOverlaysEnabled;
	}
	uint32_t Get_LastTransientLightCount() const {
		return m_iLastTransientLightCount;
	}
	uint32_t Get_LastScreenPostCount() const {
		return m_iLastScreenPostCount;
	}
	uint32_t Get_LastScreenOverlayCount() const {
		return m_iLastScreenOverlayCount;
	}

private:
	vector<shared_ptr<IPresentationProvider>> m_FrameProviders;
	vector<LIGHT_DESC> m_TransientLights;
	vector<PRESENTATION_SCREEN_POST_DESC> m_ScreenPosts;
	vector<PRESENTATION_SCREEN_OVERLAY_DESC> m_ScreenOverlays;
	bool_t m_bTransientLightsEnabled = true;
	bool_t m_bScreenPostsEnabled = true;
	bool_t m_bScreenOverlaysEnabled = true;
	bool_t m_bSubmissionTransactionActive = false;
	uint32_t m_iLastTransientLightCount = {};
	uint32_t m_iLastScreenPostCount = {};
	uint32_t m_iLastScreenOverlayCount = {};
	PRESENTATION_SUBMISSION_STATS m_LastSubmissionStats;
	PRESENTATION_FAILURE_SCOPE m_eLastFailureScope =
		PRESENTATION_FAILURE_SCOPE::NONE;
	HRESULT m_hPendingProviderFailure = S_OK;
};

NS_END
