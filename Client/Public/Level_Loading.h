#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "LobbyCommandService.h"

NS_BEGIN(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Loading();

public:
	virtual HRESULT Initialize(
		LEVEL eNextLevelID,
		LOBBY_COMMAND_TOKEN lobbyCommandToken);
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Recover_FromFailure(HRESULT result);
	void Cancel_LobbyCommand(const char_t* pReason);
	void Retry_LobbyLoad();
	HRESULT Ready_Layer_Chrome();

private:
	LEVEL m_eNextLevelID = LEVEL::END;
	LOBBY_COMMAND_TOKEN m_iLobbyCommandToken =
		INVALID_LOBBY_COMMAND_TOKEN;
	unique_ptr<class CLoader> m_pLoader = { nullptr };
	bool_t m_isActivationRequested = { false };
	bool_t m_isFailureReported = { false };
	bool_t m_isRetryRequested = { false };

	/* The progress fill/glow are repositioned every frame, so they are kept separately from the
	rest of the (static, place-once) chrome pieces. */
	shared_ptr<class CUI_Sprite>	m_pProgressFill = { nullptr };
	shared_ptr<class CUI_Sprite>	m_pProgressGlow = { nullptr };
	f32_t							m_fProgressTrackX = 0.f, m_fProgressTrackY = 0.f, m_fProgressTrackWidth = 0.f, m_fProgressTrackHeight = 0.f;
	/* Authored size of the ProgressGlow slot itself (distinct from the track it slides along). */
	f32_t							m_fProgressGlowWidth = 40.f, m_fProgressGlowHeight = 23.f;

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

public:
	static unique_ptr<CLevel_Loading> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eNextLevelID,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
};

NS_END
