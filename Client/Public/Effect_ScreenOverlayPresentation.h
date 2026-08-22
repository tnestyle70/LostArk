#pragma once

#include "Client_Defines.h"
#include "PresentationProvider.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class CEffectScreenOverlayPresentation final :
	public Engine::IPresentationProvider,
	public std::enable_shared_from_this<CEffectScreenOverlayPresentation>
{
public:
	explicit CEffectScreenOverlayPresentation(
		ComPtr<ID3D11Device> pDevice);
	~CEffectScreenOverlayPresentation() override = default;

	static shared_ptr<CEffectScreenOverlayPresentation> Create(
		ComPtr<ID3D11Device> pDevice);
	shared_ptr<CEffectScreenOverlayPresentation>
		Clone_PlaybackInstance() const;

	bool_t Stage_AndCommit(
		std::string_view strUtf8Json,
		std::string& strOutError);
	HRESULT Start();
	HRESULT Seek(f32_t fElapsedSeconds);
	HRESULT Update(f32_t fDeltaSeconds);
	void Stop();
	void Cancel();
	HRESULT Queue_Frame();

	uint64_t Get_CommittedGeneration() const
	{
		return m_iCommittedGeneration;
	}
	const string& Get_PresentationId() const
	{
		return m_strPresentationId;
	}
	size_t Get_PreparedOverlayCount() const
	{
		return m_PreparedOverlays.size();
	}
	size_t Get_ActiveOverlayCount() const;
	f32_t Get_ElapsedSeconds() const
	{
		return m_fElapsedSeconds;
	}
	f32_t Get_MaximumEndSeconds() const
	{
		return m_fMaximumEndSeconds;
	}
	bool_t Is_Playing() const
	{
		return m_bPlaying;
	}

	void Begin_PresentationSubmission() override;
	HRESULT Submit_Presentation() override;
	bool_t Is_PresentationFailureIsolated() const override
	{
		return true;
	}
	Engine::PRESENTATION_FAILURE_SCOPE
	Get_PresentationFailureScope() const override
	{
		return m_eLastFailureScope;
	}

private:
	struct PREPARED_OVERLAY final
	{
		string strOverlayId;
		f32_t fStartSeconds = 0.f;
		f32_t fLifetimeSeconds = 0.f;
		f32_t fAlphaStart = 1.f;
		f32_t fAlphaEnd = 0.f;
		Engine::PRESENTATION_SCREEN_OVERLAY_DESC Desc;
	};

	ComPtr<ID3D11Device> m_pDevice;
	vector<PREPARED_OVERLAY> m_PreparedOverlays;
	string m_strPresentationId;
	uint64_t m_iCommittedGeneration = 0u;
	f32_t m_fElapsedSeconds = 0.f;
	f32_t m_fMaximumEndSeconds = 0.f;
	bool_t m_bPlaying = false;
	Engine::PRESENTATION_FAILURE_SCOPE m_eLastFailureScope =
		Engine::PRESENTATION_FAILURE_SCOPE::NONE;
};

NS_END
