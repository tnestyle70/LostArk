#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "LobbyCommandService.h"

#include <string>
#include <vector>

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
	bool_t Advance_TargetEffectPreparation();
	void Render_LoadingRecoveryProduct();
#ifdef _DEBUG
	void Render_LoadingProgressDiagnostics();
#endif

private:
	LEVEL m_eNextLevelID = LEVEL::END;
	LOBBY_COMMAND_TOKEN m_iLobbyCommandToken =
		INVALID_LOBBY_COMMAND_TOKEN;
	unique_ptr<class CLoader> m_pLoader = { nullptr };
	/* LoadingRecovery.json as real CUI_Sprite GameObjects on this Level's own Layer_UI
	(CUILayoutRuntime), hidden until a Lobby load failure is reported -- the loading screen is
	product UI, so it draws nothing through ImGui. */
	unique_ptr<class CUILayoutRuntime> m_pRecoveryView = { nullptr };
	bool_t m_isActivationRequested = { false };
	bool_t m_isFailureReported = { false };
	bool_t m_isRetryRequested = { false };
	bool_t m_isEffectPreparationRegistered = { false };
	bool_t m_isEffectLoadJobStarted = { false };
	uint64_t m_iEffectLoadJobEpoch = 0u;
	std::vector<std::string> m_EffectPreparationTargets;
	std::string m_strEffectPreparationStatus;
	std::string m_strEffectPreparationRegistrationFailure;
	uint32_t m_iEffectPreparationTargetCount = 0u;
	uint32_t m_iEffectPreparationPendingCount = 0u;
	uint32_t m_iEffectPreparationPreparedCount = 0u;
	uint32_t m_iEffectPreparationFailedCount = 0u;

	/* The progress fill/glow are repositioned every frame, so they are kept separately from the
	rest of the (static, place-once) chrome pieces. */
	shared_ptr<class CUI_Sprite>	m_pProgressFill = { nullptr };
	shared_ptr<class CUI_Sprite>	m_pProgressGlow = { nullptr };
	/* The reveal window, from the ProgressMask marker: retail's crashLoadingTarget clip, whose
	left edge is fixed and whose width is the progress ratio times the track length. */
	f32_t							m_fProgressMaskLeft = 0.f, m_fProgressMaskWidth = 0.f;
	/* The fill sprite's own placement. It is wider and taller than both the mask and the track
	plate -- the art carries a glow falloff past the bar on every side -- so it keeps its own
	rect and is revealed by a UV window rather than by resizing. */
	f32_t							m_fProgressFillLeft = 0.f, m_fProgressFillWidth = 0.f;
	f32_t							m_fProgressFillCenterY = 0.f, m_fProgressFillHeight = 0.f;
	/* The mark art's own placement. Its x is driven every frame; the rest is authored. */
	f32_t							m_fProgressGlowCenterY = 0.f;
	f32_t							m_fProgressGlowWidth = 40.f, m_fProgressGlowHeight = 23.f;

	/* The bar is determinate only when its current denominator is known.  While
	   the Level worker spans opaque phases, a moving segment is shown instead
	   of fabricating an overall percentage. */
	f32_t							m_fDisplayProgress = 0.f;
	f32_t							m_fIndeterminateProgress = 0.f;
	/* Successive lanes (Effect document staging, Product target settling, the Level worker)
	   each report against their own denominator, so the raw fraction above jumps backwards
	   whenever the reporting lane changes. These keep what the player sees monotonic: the
	   target only ever rises, and the drawn value eases toward it instead of teleporting.
	   Neither fabricates a percentage -- they only filter the values the lanes publish. */
	f32_t							m_fTargetProgress = 0.f;
	f32_t							m_fShownProgress = 0.f;
	/* Once any lane has published a real fraction the bar stays a filling bar. Dropping back
	   to the sliding segment between lanes is what made it look like it was thrashing. */
	bool_t							m_hasDeterminateProgress = false;

	wstring_t						m_strTitleText;
	/* The gold label above the tip. Retail's own field swaps between a "scenario" and a "tip"
	string by frame, so it belongs with the per-level text rather than being fixed. */
	wstring_t						m_strScenarioLabel;
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
