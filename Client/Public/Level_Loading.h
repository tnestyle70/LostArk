#pragma once

#include "Client_Defines.h"
#include "Level.h"

/* 특정 레벨에 진입하기전에 반드시 거쳐야하는 레벨. */

/* 레벨 본역의 역활 + 다음 레벨에 대한 자원을 준비해준다.*/

NS_BEGIN(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CLevel_Loading();

public:
	virtual HRESULT Initialize(LEVEL eNextLevelID);
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	LEVEL							m_eNextLevelID = { LEVEL::END };
	unique_ptr<class CLoader>		m_pLoader = { nullptr };
	bool_t							m_isFailureReported = { false };
	bool_t							m_isRetryRequested = { false };

	/* The progress fill/glow are repositioned every frame, so they are kept separately from the
	rest of the (static, place-once) chrome pieces. */
	shared_ptr<class CUI_Sprite>	m_pProgressFill = { nullptr };
	shared_ptr<class CUI_Sprite>	m_pProgressGlow = { nullptr };
	f32_t							m_fProgressTrackX = 0.f, m_fProgressTrackY = 0.f, m_fProgressTrackWidth = 0.f, m_fProgressTrackHeight = 0.f;

	/* No per-step byte/asset count exists in CLoader, so this is a simple time-based fill that
	eases toward 90% and only snaps to 100% once the loader actually reports finished. */
	f32_t							m_fDisplayProgress = 0.f;

	wstring_t						m_strTitleText;
	wstring_t						m_strTipText;

	/* Text draw positions only -- color/scale/alignment stay code-owned. Defaults match the
	authored layout and are overridden by LoadingLayout.json's texture-less marker slots. */
	float2_t						m_vTitlePos = { 640.f, 22.f };
	float2_t						m_vScenarioPos = { 640.f, 600.7f };
	float2_t						m_vTipPos = { 660.f, 657.1f };

private:
	void Recover_FromFailure(HRESULT result);
	void Retry_LobbyLoad();
	HRESULT Ready_Layer_Chrome();

public:
	static unique_ptr<CLevel_Loading> Create(ComPtr<ID3D11Device> pDevice, 
		ComPtr<ID3D11DeviceContext> pContext, LEVEL eNextLevelID);
};

NS_END
