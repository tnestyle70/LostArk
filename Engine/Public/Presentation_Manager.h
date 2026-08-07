#pragma once

#include "PresentationProvider.h"

NS_BEGIN(Engine)

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
	uint32_t m_iLastTransientLightCount = {};
	uint32_t m_iLastScreenPostCount = {};
};

NS_END
