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
	m_TransientLights.clear();
	m_ScreenPosts.clear();
	vector<shared_ptr<IPresentationProvider>> Providers =
		std::move(m_FrameProviders);
	m_FrameProviders.clear();
	for (const shared_ptr<IPresentationProvider>& Provider : Providers)
	{
		if (nullptr == Provider || FAILED(Provider->Submit_Presentation()))
		{
			Clear_Frame();
			return E_FAIL;
		}
	}
	m_iLastTransientLightCount = static_cast<uint32_t>(m_TransientLights.size());
	m_iLastScreenPostCount = static_cast<uint32_t>(m_ScreenPosts.size());
	return S_OK;
}

HRESULT CPresentation_Manager::Add_TransientLight(
	const LIGHT_DESC& LightDesc)
{
	if (!m_bTransientLightsEnabled)
		return S_FALSE;
	if (LIGHT::POINT != LightDesc.eType ||
		!std::isfinite(LightDesc.fRange) || LightDesc.fRange <= 0.f ||
		!std::isfinite(LightDesc.fFalloffExponent) ||
		LightDesc.fFalloffExponent <= 0.f ||
		!IsFinite4(LightDesc.vPosition) ||
		!IsFinite4(LightDesc.vDiffuse) ||
		!IsFinite4(LightDesc.vAmbient) ||
		m_TransientLights.size() >= MAX_TRANSIENT_LIGHTS)
	{
		return E_FAIL;
	}
	m_TransientLights.push_back(LightDesc);
	return S_OK;
}

HRESULT CPresentation_Manager::Add_ScreenPost(
	const PRESENTATION_SCREEN_POST_DESC& ScreenPostDesc)
{
	if (!m_bScreenPostsEnabled)
		return S_FALSE;
	if (ScreenPostDesc.eProfile >= PRESENTATION_SCREEN_POST_PROFILE::END ||
		0u == ScreenPostDesc.iRandomSeed ||
		!std::isfinite(ScreenPostDesc.fSampleTimeSeconds) ||
		ScreenPostDesc.fSampleTimeSeconds < 0.f ||
		!std::isfinite(ScreenPostDesc.fIntensity) ||
		ScreenPostDesc.fIntensity < 0.f ||
		!std::isfinite(ScreenPostDesc.fSecondaryIntensity) ||
		ScreenPostDesc.fSecondaryIntensity < 0.f ||
		!std::isfinite(ScreenPostDesc.fFrequency) ||
		ScreenPostDesc.fFrequency < 0.f ||
		!IsFinite4(ScreenPostDesc.vTint) ||
		m_ScreenPosts.size() >= MAX_SCREEN_POSTS)
	{
		return E_FAIL;
	}
	m_ScreenPosts.push_back(ScreenPostDesc);
	return S_OK;
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
