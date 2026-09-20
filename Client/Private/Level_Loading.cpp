#include "imgui.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "Level_Loading.h"
#include "WorldSequencePlayer.h"

#include "AnimationEffectCueDocument.h"
#include "ActorCatalog.h"
#include "CharacterCatalog.h"
#include "CharacterSelectionState.h"
#include "CharacterSpec.h"
#include "ClickMoveEffect.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "EffectFailureDiagnostic.h"
#include "Effect_LoadPreparationJob.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"
#include "LevelTransitionService.h"
#include "LevelRegistry.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "Loader.h"
#include "MapAssetCatalog.h"
#include "MapEffectDocument.h"
#include "NetworkManager.h"
#include "ProjectDataRoot.h"
#include "UI_Sprite.h"
#include "ValtanPatternEffectCueDocument.h"
#include "ValtanPatternTree.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <fstream>

namespace
{

	/* A worker phase without a denominator creeps toward this share of its slice on elapsed
	time (tau in ms) so the bar keeps moving without ever running ahead of the next phase. */
	constexpr f32_t OPAQUE_PHASE_CAP = 0.85f;
	constexpr f32_t OPAQUE_PHASE_TAU_MS = 2500.f;
	/* Share of the bar the Effect lanes (document staging, then Product target settling) own
	when the target Level uses them; the Level worker owns the rest. */
	constexpr f32_t EFFECT_LANE_SHARE = 0.25f;
	/* How far across its own art the mark's anchor sits. Retail places the mark's registration
	point on the reveal edge and draws the comet at (-293,-25) from it, so 293 of the art's 356
	px lie behind that edge and only its short bright end pokes past. */
	constexpr f32_t GLOW_ANCHOR_RATIO = 293.f / 356.f;

	/* The movie is authored on a 1920x1080 stage; this layout's reference is 1280x720. */
	constexpr f32_t GFX_TO_REF = 2.f / 3.f;
	/* Em sizes off each line's own text field: nameTF 36, TitleTip 26, tipTF 18. */
	constexpr f32_t TITLE_TEXT_SIZE = 36.f * GFX_TO_REF;
	constexpr f32_t SCENARIO_TEXT_SIZE = 26.f * GFX_TO_REF;
	/* The tip line reads too small at its authored size on this screen, so the user asked for
	1.3x. The other two lines stay at the size their fields carry. */
	constexpr f32_t TIP_TEXT_SIZE = 18.f * GFX_TO_REF * 1.3f;
	/* A text field's height is its em size plus the font's ascent/descent overshoot, which the
	sprite fonts bake into their line spacing. The exact ratio is not recoverable from either
	side, so this is the one approximated number here. */
	constexpr f32_t EM_TO_LINE_SPACING = 1.25f;
	/* TitleTip's markup colour, #ffdd8a. */
	const XMVECTORF32 SCENARIO_TEXT_COLOR = { { { 1.f, 0.867f, 0.541f, 1.f } } };
}

CLevel_Loading::CLevel_Loading(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
}

CLevel_Loading::~CLevel_Loading()
{
	if (m_isEffectLoadJobStarted && nullptr != m_pLoader)
	{
		CEffectPresentationService::Cancel_LoadingProductCuePreparation(
			m_pLoader->Get_EffectLoadJob(), m_iEffectLoadJobEpoch);
	}
}

HRESULT CLevel_Loading::Initialize(
	const LEVEL eNextLevelID,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (INVALID_LOBBY_COMMAND_TOKEN != lobbyCommandToken &&
		LEVEL::LOBBY != eNextLevelID)
	{
		return E_INVALIDARG;
	}

	m_eNextLevelID = eNextLevelID;
	m_iLobbyCommandToken = lobbyCommandToken;
	if (LEVEL::KAKULSAYDON_ARENA == eNextLevelID)
	{
		m_iKoukuLoadStartedMs = GetTickCount64();
#ifdef _DEBUG
		Write_EffectFailureDiagnostic("Kouku.Loading.Begin", "configuration=Debug raid_effects=lazy load_started_ms=" +
			std::to_string(m_iKoukuLoadStartedMs));
#else
		Write_EffectFailureDiagnostic("Kouku.Loading.Begin", "configuration=Release raid_effects=preload load_started_ms=" +
			std::to_string(m_iKoukuLoadStartedMs));
#endif
	}

	/* No per-scenario text source exists yet, so every target level shows a fixed
	placeholder title/tip. Written as \x escapes -- this source file has no BOM and the
	project builds without /utf-8, so literal Korean text here would be misread as CP949.
	Valtan Arena: "Revived Heart of the Beast" / "From the Revived Heart of the Beast, the
	howls of beasts can be heard." Character Select (creation): retail's prologue loading
	screen "Destiny Begins" / "Prepare for the start of a new journey." Everything else keeps
	Bern's placeholder: "Bern Castle" / "Bern Castle is the capital of Bern, where many races
	live mixed together." The label above the tip is "Scenario" except on Character Select,
	which shows "Info" over a line about the 161st class. */
	m_strScenarioLabel = L"\xC2DC\xB098\xB9AC\xC624";
	if (LEVEL::CHARACTER_SELECT == m_eNextLevelID)
	{
		m_strTitleText = L"\xC2DC\xC791\xB418\xB294 \xC6B4\xBA85";
		m_strScenarioLabel = L"\xC815\xBCF4";
		m_strTipText = L"161\xAE30\xB294 \xC6D0\xB798 11\xBA85 \xC774\xC5C8\xC2B5\xB2C8\xB2E4";
	}
	else if (LEVEL::VALTAN_ARENA == m_eNextLevelID)
	{
		m_strTitleText = L"\xBD80\xD65C\xD55C \xB9C8\xC218\xC758 \xC2EC\xC7A5";
		m_strTipText = L"\xBD80\xD65C\xD55C \xB9C8\xC218\xC758 \xC2EC\xC7A5\xC5D0\xC11C \xC9D0\xC2B9\xB4E4\xC758 \xC6B8\xBD80\xC9D6\xC74C\xC774 \xB4E4\xB824\xC635\xB2C8\xB2E4.";
	}
	else if (LEVEL::KAKULSAYDON_ARENA == m_eNextLevelID)
	{
		m_strTitleText = L"\xD55C\xBC24\xC911\xC758 \xC11C\xCEE4\xC2A4";
		m_strTipText = L"\xD55C\xBC24\xC911\xC758 \xC11C\xCEE4\xC2A4\xB294 \xB2F9\xC2E0\xC744 \xC9C4\xC2EC\xC73C\xB85C \xD658\xC601\xD569\xB2C8\xB2E4.";
	}
	else
	{
		m_strTitleText = L"\xBCA0\xB978 \xC131";
		m_strScenarioLabel = L"\xC815\xBCF4";
		m_strTipText = L"\xC81C 1\xB300 \xC774\xD399\xD2B8 \xB2F4\xB2F9\xC790\xB294 \xADF9\xC2EC\xD55C \xC6B0\xC6B8\xC99D\xC744 \xD638\xC18C\xD558\xBA70 \xC774\xD399\xD2B8 \xB2F4\xB2F9\xC9C1\xC744 \xC0AC\xD1F4\xD588\xC2B5\xB2C8\xB2E4";
	}

	if (FAILED(Ready_Layer_Chrome()))
		return E_FAIL;
	m_pRecoveryView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::LOADING), TEXT("Layer_UI"),
		L"UI/Loading/LoadingRecovery.json");
	/* Authored tints are opaque -- the failure panel must stay invisible until a Lobby load
	failure is actually reported (Render_LoadingRecoveryProduct owns it from there). */
	m_pRecoveryView->Set_SlotVisible("LoadingRecovery_Panel", false);
	m_pRecoveryView->Set_SlotVisible("LoadingRecovery_RetryButton", false);

	const bool_t bUsesEffectLoadJob = CClickMoveEffect::Uses_LevelMarkers(m_eNextLevelID);
	uint64_t iEffectCatalogRevision = 0u;
	if (bUsesEffectLoadJob)
	{
		m_iEffectLoadJobEpoch =
			CEffectPresentationService::Allocate_ProductPreparationEpoch();
		if (0u == m_iEffectLoadJobEpoch)
			return E_FAIL;
		iEffectCatalogRevision = CEffectCatalog::Get_RuntimeRevision();
		if (0u == iEffectCatalogRevision)
			return E_FAIL;
	}
	m_pLoader = CLoader::Create(
		m_pDevice, m_pContext, m_eNextLevelID,
		m_iEffectLoadJobEpoch, iEffectCatalogRevision);
	if (nullptr == m_pLoader)
		return E_FAIL;

	/* The target class/encounter is fixed and both Loader producers are live.
	   Capture immutable Product requests on this owner and post them now, so
	   Effect staging overlaps level resources while this frame owns commits. */
	if (bUsesEffectLoadJob)
	{
		Advance_TargetEffectPreparation();
	}
	return S_OK;
}

void CLevel_Loading::Update(const f32_t fTimeDelta)
{
	if (m_isRetryRequested)
	{
		m_isRetryRequested = false;
		Retry_LobbyLoad();
		return;
	}

	if (nullptr == m_pLoader)
		return;
	if (m_pLoader->Failed())
	{
		Recover_FromFailure(m_pLoader->Get_Result());
		return;
	}

	bool_t bTargetPresentationReady = true;
	if (CClickMoveEffect::Uses_LevelMarkers(m_eNextLevelID) &&
		!m_isActivationRequested)
	{
		bTargetPresentationReady =
			Advance_TargetEffectPreparation();
	}

	const bool_t bLoaderFinished = m_pLoader->Finished();
	// All producers have joined before recovery releases their prototype stage.
	// Required raid Effects cannot turn a failed preparation into a live fight.
	if (LEVEL::KAKULSAYDON_ARENA == m_eNextLevelID && bLoaderFinished &&
		m_iEffectPreparationFailedCount != 0u)
	{
		Recover_FromFailure(E_FAIL);
		return;
	}
	const CLoader::PROGRESS_SNAPSHOT LoaderProgress =
		m_pLoader->Get_ProgressSnapshot();
	EFFECT_LOAD_PROGRESS_SNAPSHOT EffectProgress;
	bool_t bHasEffectProgress = false;
	if (const std::shared_ptr<CEffectLoadPreparationJob> Job =
			m_pLoader->Get_EffectLoadJob(); nullptr != Job)
	{
		EffectProgress = Job->Get_Progress();
		bHasEffectProgress = true;
	}

	/* One bar for the whole load. Level worker lane: (phases done + fraction inside the current
	phase) / declared phases, where a phase with a denominator (model prototypes, character
	bundles) reports it and an opaque phase creeps on elapsed time. Effect lane: document staging
	fills the first half, Product target settling the second. The two lanes run in parallel, so
	they are summed by share rather than chained. */
	f32_t fLoaderLane = 0.f;
	if (bLoaderFinished)
	{
		fLoaderLane = 1.f;
	}
	else if (0u != LoaderProgress.iPhaseCount)
	{
		const size_t iPhase = (std::min)(
			LoaderProgress.iPhaseIndex > 0u ? LoaderProgress.iPhaseIndex - 1u : 0u,
			LoaderProgress.iPhaseCount - 1u);
		f32_t fInPhase = 0.f;
		if (LoaderProgress.bDeterminate && 0u != LoaderProgress.iTotal)
		{
			fInPhase = static_cast<f32_t>((std::min)(LoaderProgress.iCompleted, LoaderProgress.iTotal)) /
				static_cast<f32_t>(LoaderProgress.iTotal);
		}
		else
		{
			fInPhase = OPAQUE_PHASE_CAP * (1.f - std::exp(
				-static_cast<f32_t>(LoaderProgress.iElapsedMs) / OPAQUE_PHASE_TAU_MS));
		}
		fLoaderLane = (static_cast<f32_t>(iPhase) + std::clamp(fInPhase, 0.f, 1.f)) /
			static_cast<f32_t>(LoaderProgress.iPhaseCount);
	}

	f32_t fEffectLane = 0.f;
	if (bHasEffectProgress)
	{
		switch (EffectProgress.ePhase)
		{
		case EFFECT_LOAD_PROGRESS_PHASE::TARGET_STAGE:
			if (EffectProgress.bDeterminate && 0u != EffectProgress.iTotal)
			{
				fEffectLane = 0.5f * static_cast<f32_t>((std::min)(EffectProgress.iCompleted, EffectProgress.iTotal)) /
					static_cast<f32_t>(EffectProgress.iTotal);
			}
			break;
		case EFFECT_LOAD_PROGRESS_PHASE::EPOCH_STAGE_COMPLETE:
		case EFFECT_LOAD_PROGRESS_PHASE::CLOSED:
		{
			const uint32_t iSettledCount = (min)(
				m_iEffectPreparationPreparedCount + m_iEffectPreparationFailedCount,
				m_iEffectPreparationTargetCount);
			fEffectLane = 0.5f + 0.5f * (0u != m_iEffectPreparationTargetCount ?
				static_cast<f32_t>(iSettledCount) / static_cast<f32_t>(m_iEffectPreparationTargetCount) : 1.f);
			break;
		}
		default:
			break;
		}
		if (bTargetPresentationReady)
			fEffectLane = 1.f;
	}

	if (m_isEffectLoadJobStarted && !m_isActivationRequested)
	{
		CEffectPresentationService::Advance_LoadingProductCuePreparation(
			m_pDevice, m_pContext, m_pLoader->Get_EffectLoadJob(),
			m_iEffectLoadJobEpoch);
	}

	const bool_t bLoadComplete = bLoaderFinished && bTargetPresentationReady;
	const f32_t fEffectShare = bHasEffectProgress ? EFFECT_LANE_SHARE : 0.f;
	m_fDisplayProgress = bLoadComplete ? 1.f :
		(std::min)(fLoaderLane * (1.f - fEffectShare) + fEffectLane * fEffectShare, 0.99f);
	/* Only ever rises: a lane restarting from a small numerator cannot drag the bar back. */
	m_fTargetProgress = (max)(m_fTargetProgress, std::clamp(m_fDisplayProgress, 0.f, 1.f));
	if (bLoadComplete)
	{
		/* Completion is the one case that snaps: the load really is done, so the bar must not
		   still be easing while the Level activates. */
		m_fShownProgress = 1.f;
	}
	else
	{
		const f32_t fStep = std::clamp((max)(0.f, fTimeDelta), 0.f, 0.1f);
		m_fShownProgress += (m_fTargetProgress - m_fShownProgress) *
			(1.f - std::exp(-6.f * fStep));
	}

	/* The reveal window in layout space, the way retail drives the bar: a clip whose left edge
	is fixed and whose width is the ratio times the track length. */
	const f32_t fRevealLeft = m_fProgressMaskLeft;
	const f32_t fRevealRight = fRevealLeft + m_fProgressMaskWidth *
		std::clamp(m_fShownProgress, 0.f, 1.f);

	if (nullptr != m_pProgressFill && m_fProgressFillWidth > 0.f)
	{
		/* The revealed run, as a window over the fill art's own extent. Sizing the quad to that
		window and sampling the matching UV slice draws the art at its authored scale; resizing
		the quad alone squeezed the whole 1804 px gradient into the filled run, so the run read
		as a dark smear with only its bright right end showing under the mark. */
		const f32_t fLeftU = std::clamp(
			(fRevealLeft - m_fProgressFillLeft) / m_fProgressFillWidth, 0.f, 1.f);
		const f32_t fRightU = std::clamp(
			(fRevealRight - m_fProgressFillLeft) / m_fProgressFillWidth, 0.f, 1.f);
		const f32_t fWindowWidth = (fRightU - fLeftU) * m_fProgressFillWidth;

		/* A zero-width quad would scale the transform's axes to nothing, so the sprite is
		hidden for that frame instead. */
		m_pProgressFill->Set_Visible(fWindowWidth > 0.f);
		if (fWindowWidth > 0.f)
		{
			m_pProgressFill->Set_UVWindow(
				float2_t(fLeftU, 0.f), float2_t(fRightU - fLeftU, 1.f));
			m_pProgressFill->Set_Rect(
				m_fProgressFillLeft + fLeftU * m_fProgressFillWidth + fWindowWidth * 0.5f,
				m_fProgressFillCenterY, fWindowWidth, m_fProgressFillHeight);
		}
	}

	if (nullptr != m_pProgressGlow)
	{
		/* Retail puts the mark's registration point on the reveal edge; GLOW_ANCHOR_RATIO is
		where that point sits across the art, so the trail lies back along the filled run. */
		m_pProgressGlow->Set_Rect(
			fRevealRight - (GLOW_ANCHOR_RATIO - 0.5f) * m_fProgressGlowWidth,
			m_fProgressGlowCenterY, m_fProgressGlowWidth, m_fProgressGlowHeight);
	}

	if (m_pLoader->Finished() && bTargetPresentationReady &&
		!m_isActivationRequested)
	{
		if (CLevelTransitionService::Request_Activation(
			m_eNextLevelID,
			"loading.complete",
			m_iLobbyCommandToken))
		{
			m_isActivationRequested = true;
			m_iLobbyCommandToken = INVALID_LOBBY_COMMAND_TOKEN;
			if (LEVEL::KAKULSAYDON_ARENA == m_eNextLevelID)
				Write_EffectFailureDiagnostic("Kouku.Loading.GateReady",
					"elapsed_ms=" + std::to_string(GetTickCount64() - m_iKoukuLoadStartedMs) +
					" v1_targets=" + std::to_string(m_iEffectPreparationTargetCount) +
					" v2_targets=" + std::to_string(m_KoukuV2EffectTargets.size()));
		}
		else
		{
			OutputDebugStringA(
				"[Level_Loading] Activation request was rejected; retrying.\n");
		}
	}

	__super::Update(fTimeDelta);
}

HRESULT CLevel_Loading::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	/* Each line is drawn at the point size its own text field carries, scaled from the movie's
	1920x1080 stage into this layout's 1280x720 reference. The fields are top-anchored and
	centre-aligned, so the marker position is the field's top-centre. */
	const auto Fn_DrawLine = [](const wstring_t& strFamily, const wchar_t* pText,
		const float2_t& vTopCenter, f32_t fEmSize, const fvector_t& vColor)
	{
		if (nullptr == pText || L'\0' == pText[0])
			return;
		/* UILabelFont sizes by line spacing, the gfx field carries an em size, so the one goes
		through the other. Measuring the string itself instead would size every line by its own
		tallest glyph, which is why the Korean lines came out smaller than the label size asked
		for. */
		f32_t fScale = 1.f;
		const wstring_t strFont =
			UILabelFont::Resolve(strFamily, fEmSize * EM_TO_LINE_SPACING, fScale);
		CGameInstance::Get().Draw_Text(strFont, pText, vTopCenter, vColor, 0.f,
			float2_t(0.5f, 0.f), fScale);
	};

	Fn_DrawLine(TEXT("Font_YoonGasiIIM"), m_strTitleText.c_str(),
		m_vTitlePos, TITLE_TEXT_SIZE, Colors::White);

	if (!m_strTipText.empty())
	{
		/* Colour is the one the field's own markup carries. */
		Fn_DrawLine(TEXT("Font_YoonGasiIIM"), m_strScenarioLabel.c_str(),
			m_vScenarioPos, SCENARIO_TEXT_SIZE, SCENARIO_TEXT_COLOR);

		Fn_DrawLine(TEXT("Font_YG760"), m_strTipText.c_str(),
			m_vTipPos, TIP_TEXT_SIZE, Colors::White);
	}

	Render_LoadingRecoveryProduct();

#ifdef _DEBUG
	Render_LoadingProgressDiagnostics();
	if (nullptr != m_pLoader)
		m_pLoader->Print_Text();
#endif
	return S_OK;
}

void CLevel_Loading::Render_LoadingRecoveryProduct()
{
	if (nullptr == m_pRecoveryView)
		return;
	if (!m_isFailureReported || LEVEL::LOBBY != m_eNextLevelID)
	{
		m_pRecoveryView->Set_SlotVisible("LoadingRecovery_Panel", false);
		m_pRecoveryView->Set_SlotVisible("LoadingRecovery_RetryButton", false);
		return;
	}

	/* The panel/retry art are real CUI_Sprite slots now (LoadingRecovery.json); this keeps
	their visibility/hover state current and draws the two text lines via the same Draw_Text
	pass the title/tip above already use. If an authored slot is somehow missing, the matching
	default rect still places the text and the retry hit-test so recovery stays usable -- the
	old ImGui fallback rectangles had no engine-path equivalent and are gone. */
	m_pRecoveryView->Set_SlotVisible("LoadingRecovery_Panel", true);
	m_pRecoveryView->Set_SlotVisible("LoadingRecovery_RetryButton", true);

	struct PRODUCT_RECT
	{
		f32_t fX;
		f32_t fY;
		f32_t fWidth;
		f32_t fHeight;
	};
	constexpr PRODUCT_RECT DEFAULT_MESSAGE_RECT{ 310.f, 292.f, 660.f, 72.f };
	constexpr PRODUCT_RECT DEFAULT_RETRY_RECT{ 569.f, 388.f, 142.f, 48.f };

	const auto ResolveProductRect = [this](
		const char_t* pSlotId,
		const PRODUCT_RECT& DefaultRect,
		PRODUCT_RECT& outRect)
	{
		outRect = DefaultRect;
		PRODUCT_RECT AuthoredRect{};
		if (!m_pRecoveryView->Get_SlotRect(
			pSlotId, AuthoredRect.fX, AuthoredRect.fY,
			AuthoredRect.fWidth, AuthoredRect.fHeight) ||
			!std::isfinite(AuthoredRect.fX) || !std::isfinite(AuthoredRect.fY) ||
			!std::isfinite(AuthoredRect.fWidth) || !std::isfinite(AuthoredRect.fHeight) ||
			AuthoredRect.fWidth <= 0.f || AuthoredRect.fHeight <= 0.f)
		{
			return false;
		}
		outRect = AuthoredRect;
		return true;
	};

	const f32_t fRefWidth = m_pRecoveryView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pRecoveryView->Get_ResolutionHeight();
	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = fRefWidth > 0.f ? vViewportSize.x / fRefWidth : 1.f;
	const f32_t fScaleY = fRefHeight > 0.f ? vViewportSize.y / fRefHeight : 1.f;
	const f32_t fTextScale = (std::min)(fScaleX, fScaleY);
	const auto Fn_DrawCenteredAscii = [&](
		const PRODUCT_RECT& Rect, const char_t* pText, f32_t fTargetHeight)
	{
		const std::wstring wide(pText, pText + std::char_traits<char_t>::length(pText));
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YG330"), wide.c_str());
		if (vMeasured.y <= 0.f)
			return;
		const f32_t fScale = (fTargetHeight / vMeasured.y) * fTextScale;
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), wide.c_str(),
			float2_t(
				(Rect.fX + Rect.fWidth * 0.5f) * fScaleX,
				(Rect.fY + Rect.fHeight * 0.5f) * fScaleY),
			Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale);
	};

	PRODUCT_RECT MessageRect{};
	ResolveProductRect(
		"LoadingRecovery_Message", DEFAULT_MESSAGE_RECT, MessageRect);
	Fn_DrawCenteredAscii(MessageRect,
		"Lobby resources could not be loaded. Partial resources were rolled back.", 16.f);

	PRODUCT_RECT RetryRect{};
	ResolveProductRect(
		"LoadingRecovery_RetryButton", DEFAULT_RETRY_RECT, RetryRect);
	CUIInputRouter& Router = CUIInputRouter::Get();
	const bool_t bHovered = Router.Is_Hovered(
		RetryRect.fX, RetryRect.fY, RetryRect.fWidth, RetryRect.fHeight,
		fRefWidth, fRefHeight);
	m_pRecoveryView->Set_SlotTexture("LoadingRecovery_RetryButton",
		bHovered ? "UI/Lobby/create_character_button_hover.png" : "");
	if (bHovered)
	{
		Router.Claim_Mouse_This_Frame();
		if (Router.Is_Clicked(
			RetryRect.fX, RetryRect.fY, RetryRect.fWidth, RetryRect.fHeight,
			fRefWidth, fRefHeight))
		{
			m_isRetryRequested = true;
		}
	}
	Fn_DrawCenteredAscii(RetryRect, "Retry Lobby", 18.f);
}

#ifdef _DEBUG
void CLevel_Loading::Render_LoadingProgressDiagnostics()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr == viewport || nullptr == m_pLoader)
		return;

	const CLoader::PROGRESS_SNAPSHOT Progress =
		m_pLoader->Get_ProgressSnapshot();
	std::string loadingStatus = "Level resources: " + Progress.strStatus;
	if (Progress.bDeterminate)
	{
		loadingStatus += " " + std::to_string(Progress.iCompleted) + "/" +
			std::to_string(Progress.iTotal);
	}
	loadingStatus += " (" + std::to_string(Progress.iElapsedMs) + " ms)";
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowPos(
		ImVec2(
			viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
			viewport->WorkPos.y + 16.f),
		ImGuiCond_Always,
		ImVec2(0.5f, 0.f));
	ImGui::SetNextWindowBgAlpha(0.82f);
	if (ImGui::Begin(
		"Loading progress",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImGui::TextUnformatted(loadingStatus.c_str());
		if ((LEVEL::CHARACTER_SELECT == m_eNextLevelID ||
			 LEVEL::BERN == m_eNextLevelID ||
			 LEVEL::VALTAN_ARENA == m_eNextLevelID ||
			 LEVEL::KAKULSAYDON_ARENA == m_eNextLevelID) &&
			!m_strEffectPreparationStatus.empty())
		{
			const std::string EffectStatus =
				"Effects: " + m_strEffectPreparationStatus;
			ImGui::TextUnformatted(EffectStatus.c_str());
			const std::shared_ptr<CEffectLoadPreparationJob> Job =
				m_pLoader->Get_EffectLoadJob();
			if (nullptr != Job)
			{
				const EFFECT_LOAD_PROGRESS_SNAPSHOT EffectProgress =
					Job->Get_Progress();
				if (!EffectProgress.strStatus.empty())
				{
					std::string WorkerStatus = "Effect worker: " +
						EffectProgress.strStatus;
					if (EffectProgress.bDeterminate)
					{
						WorkerStatus += " " +
							std::to_string(EffectProgress.iCompleted) + "/" +
							std::to_string(EffectProgress.iTotal);
					}
					if (!EffectProgress.strCurrentId.empty())
						WorkerStatus += " - " + EffectProgress.strCurrentId;
					ImGui::TextUnformatted(WorkerStatus.c_str());
				}
			}
		}
	}
	ImGui::End();
}
#endif

bool_t CLevel_Loading::Advance_TargetEffectPreparation()
{
	const bool_t bCharacterSelect =
		LEVEL::CHARACTER_SELECT == m_eNextLevelID;
	const bool_t bBern = LEVEL::BERN == m_eNextLevelID;
	const bool_t bValtanArena = LEVEL::VALTAN_ARENA == m_eNextLevelID;
	const bool_t bKoukuArena = LEVEL::KAKULSAYDON_ARENA == m_eNextLevelID;
	if (!CClickMoveEffect::Uses_LevelMarkers(m_eNextLevelID))
	{
		return true;
	}
	if (!CEffectPresentationService::Drain_RuntimePreparationForLoading())
	{
		m_strEffectPreparationStatus = "Waiting for previous Effect worker cleanup.";
		return false;
	}
	const std::string TargetLabel =
		bCharacterSelect ? "CHARACTER SELECT" : (bBern ? "BERN" :
		(bValtanArena ? "VALTAN ARENA" : (bKoukuArena ? "KOUKUSAYDON ARENA" : "PLAYABLE LEVEL")));

	const auto IsolateFailure = [this, &TargetLabel](const std::string& Status)
	{
		m_isEffectPreparationRegistered = true;
		m_EffectPreparationTargets.clear();
		m_iEffectPreparationTargetCount = 0u;
		m_iEffectPreparationPendingCount = 0u;
		m_iEffectPreparationPreparedCount = 0u;
		m_iEffectPreparationFailedCount = 1u;
		m_strEffectPreparationRegistrationFailure = Status;
		m_strEffectPreparationStatus =
			TargetLabel + ": Effect preparation isolated: " + Status;
		if (!m_isEffectLoadJobStarted && nullptr != m_pLoader)
		{
			std::string JobStatus;
			if (CEffectPresentationService::Begin_LoadingProductCuePreparation(
					m_pLoader->Get_EffectLoadJob(), m_iEffectLoadJobEpoch,
					{}, JobStatus))
			{
				m_isEffectLoadJobStarted = true;
			}
			else
			{
				CEffectPresentationService::Cancel_LoadingProductCuePreparation(
					m_pLoader->Get_EffectLoadJob(), m_iEffectLoadJobEpoch);
			}
		}
		OutputDebugStringA(("[Level_Loading] " +
			m_strEffectPreparationStatus + "\n").c_str());
		return false;
	};

    std::vector<std::string> sourceCinematicEffects;
    if (bValtanArena && !m_isEffectPreparationRegistered &&
        !CWorldSequencePlayer::Try_CollectPreparedAreaV1EffectTargets(
            ETOUI(LEVEL::VALTAN_ARENA), "LV_LUT_HEARTRB_ED", sourceCinematicEffects))
    {
        m_strEffectPreparationStatus = "Waiting for Valtan source cinematic metadata.";
        return false;
    }
	if (!m_isEffectPreparationRegistered)
	{
		std::string Status;
		// Optional World marker registration is owner-thread metadata only. Their
		// CPU resources join this same Loader worker; per-target failures already
		// settle as isolated decorations and never require a first-click load.
		m_EffectPreparationTargets = CClickMoveEffect::Queue_LevelResources(m_eNextLevelID);
		if (bCharacterSelect || bBern || bValtanArena || bKoukuArena)
		{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		const CHARACTER_CLASS_ID SelectedClass = CNetworkManager::Get().Get_LocalCharacterClass();
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(SelectedClass))
		{
			return IsolateFailure("Server admission has no supported Effect preparation class.");
		}
		const CHARACTER_SPEC* pSpec =
			CCharacterCatalog::Find_Spec(SelectedClass);
		if (nullptr == pSpec || nullptr == pSpec->pAssetName)
		{
			return IsolateFailure(
				"selected class has no animation asset spec.");
		}
		ANIMATION_EFFECT_CUE_DOCUMENT PlayerCueDocument;
		std::vector<std::string> PlayerEffectAssetIds;
		if (!CAnimationEffectCueDocument::Load_ForProductPrewarm(
				pSpec->pAssetName, PlayerCueDocument, Status) ||
			!CEffectPresentationService::Queue_ProductCues_Priority(
				PlayerCueDocument.Cues, PlayerEffectAssetIds, Status))
		{
			return IsolateFailure(Status);
		}
		m_EffectPreparationTargets.insert(m_EffectPreparationTargets.end(),
			PlayerEffectAssetIds.begin(), PlayerEffectAssetIds.end());

		/* Character Select can audition the same catalog-backed Valtan lazily.
		   Its ambient attachments use this existing worker preparation gate. */
		if (bCharacterSelect || bValtanArena)
		{
			std::vector<std::string> defaultEffects;
			for (const auto& boss : CActorCatalog::Get_Bosses())
				if (boss.clientPresentationId == "boss.valtan.client.v1")
					for (const auto& particle : boss.defaultParticles)
						defaultEffects.push_back(particle.effectAssetId);
			if (!defaultEffects.empty())
			{
				std::vector<std::string> preparedTargets;
				if (!CEffectPresentationService::Queue_ProductTargets_Priority(
					defaultEffects, preparedTargets, Status))
					return IsolateFailure(Status);
				m_EffectPreparationTargets.insert(m_EffectPreparationTargets.end(),
					preparedTargets.begin(), preparedTargets.end());
			}
		}

		if (bValtanArena)
		{
			CValtanCanonicalProductReadAdmission ProductAdmission;
			VALTAN_CANONICAL_READ_DIAGNOSTIC ProductDiagnostic;
			if (!ProductAdmission.Acquire(ProductDiagnostic))
				return IsolateFailure(ProductDiagnostic.strStatus);
			VALTAN_PATTERN_EFFECT_CUE_DOCUMENT CueDocument;
			if (!CValtanPatternEffectCueDocument::Load_ForProductPrewarm(
					CueDocument, Status) || CueDocument.Cues.empty())
			{
				return IsolateFailure(Status);
			}
			const BOSS_ACTOR_ENTRY* pBossActor = CActorCatalog::Find_Boss(
				CueDocument.strOwnerArchetypeId);
			if (nullptr == pBossActor ||
				pBossActor->combatObjectVisuals.empty())
			{
				return IsolateFailure(nullptr == pBossActor ?
					CActorCatalog::Get_Status() :
					"Valtan BossCatalog has no combat-object visuals to prepare.");
			}
			std::vector<std::string> EffectAssetIds;
			EffectAssetIds.insert(EffectAssetIds.end(), sourceCinematicEffects.begin(), sourceCinematicEffects.end());
			EffectAssetIds.reserve(CueDocument.Cues.size() +
				pBossActor->combatObjectVisuals.size());
#ifdef _DEBUG
			std::vector<std::string> OptionalV1EffectAssetIds;
			OptionalV1EffectAssetIds.reserve(CueDocument.Cues.size());
#endif
			for (const VALTAN_PATTERN_EFFECT_CUE& Cue : CueDocument.Cues)
			{
				EffectAssetIds.push_back(Cue.strEffectAssetId);
			#ifdef _DEBUG
				if (!Cue.strV1EffectAssetId.empty())
					OptionalV1EffectAssetIds.push_back(Cue.strV1EffectAssetId);
			#endif
			}
			for (const BOSS_COMBAT_OBJECT_VISUAL_ENTRY& Visual :
				pBossActor->combatObjectVisuals)
			{
				if (BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1 ==
					Visual.activeEffectKind)
				{
					EffectAssetIds.push_back(Visual.effectAssetId);
				}
				if (!Visual.hitEffectAssetId.empty())
					EffectAssetIds.push_back(Visual.hitEffectAssetId);
			}
			if (!ProductAdmission.Validate_StillCurrent(Status))
				return IsolateFailure(Status);
#ifdef _DEBUG
			/* V1 is an optional audition lane. Queue it before the required V0
			   targets so the following priority enqueue restores V0 to the FIFO
			   front. Its registration or later preparation failure never enters
			   the Level activation probe. */
			if (!OptionalV1EffectAssetIds.empty())
			{
				std::vector<std::string> IgnoredV1Targets;
				std::string V1Status;
				if (!CEffectPresentationService::Queue_ProductTargets_Priority(
						OptionalV1EffectAssetIds, IgnoredV1Targets, V1Status))
				{
					OutputDebugStringA((
						"[Level_Loading] Optional Valtan Material V1 prewarm registration isolated: " +
						V1Status + "\n").c_str());
				}
			}
#endif
			std::vector<std::string> BossEffectAssetIds;
			if (!CEffectPresentationService::Queue_ProductTargets_Priority(
					EffectAssetIds, BossEffectAssetIds, Status) ||
				BossEffectAssetIds.empty())
			{
				return IsolateFailure(Status);
			}
			m_EffectPreparationTargets.insert(
				m_EffectPreparationTargets.end(),
				BossEffectAssetIds.begin(), BossEffectAssetIds.end());
			std::sort(m_EffectPreparationTargets.begin(),
				m_EffectPreparationTargets.end());
			m_EffectPreparationTargets.erase(std::unique(
				m_EffectPreparationTargets.begin(),
				m_EffectPreparationTargets.end()),
				m_EffectPreparationTargets.end());
		}

		if (bKoukuArena)
		{
			if (!CActorCatalog::Initialize())
				return IsolateFailure(CActorCatalog::Get_Status());
			std::vector<std::string> EffectAssetIds;
#ifndef _DEBUG
			// Release must finish the entire published raid dependency closure.
			// Debug keeps the existing on-demand worker/consumer paths so editing
			// one pattern does not prepare every other pattern and cinematic.
			if (!CKoukuSaydonPresentationPlayer::Collect_ProductEffectTargets(
				EffectAssetIds, m_KoukuV2EffectTargets, Status))
				return IsolateFailure(Status);
#endif
			// Monster clip-end cues share the same prepared V1 target cache.
			for (const auto& monster : CActorCatalog::Get_Monsters())
				for (const auto& attack : monster.attackPresentations)
					if (!attack.endEffectAssetId.empty())
						EffectAssetIds.push_back(attack.endEffectAssetId);
			/* Server combat-object pulses have no animation Effect cue. Prepare
			   their catalog visuals for every supported arena body before entry,
			   through the same Loader worker and target activation probe. */
			for (const BOSS_ACTOR_ENTRY& Boss : CActorCatalog::Get_Bosses())
			{
				if (!Boss.archetypeId.starts_with("BOSS_KAKULSAYDON_") ||
					!Boss.clientPresentationId.starts_with("boss.kakulsaydon.") ||
					!Boss.clientPresentationId.ends_with(".client.v1"))
				{
					continue;
				}
				for (const BOSS_COMBAT_OBJECT_VISUAL_ENTRY& Visual :
					Boss.combatObjectVisuals)
				{
					if (BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1 ==
						Visual.activeEffectKind)
					{
						EffectAssetIds.push_back(Visual.effectAssetId);
					}
					if (!Visual.hitEffectAssetId.empty())
						EffectAssetIds.push_back(Visual.hitEffectAssetId);
				}
			}
			std::sort(EffectAssetIds.begin(), EffectAssetIds.end());
			EffectAssetIds.erase(std::unique(EffectAssetIds.begin(),
				EffectAssetIds.end()), EffectAssetIds.end());
			if (!EffectAssetIds.empty())
			{
				std::vector<std::string> BossEffectAssetIds;
				if (!CEffectPresentationService::Queue_ProductTargets_Priority(
						EffectAssetIds, BossEffectAssetIds, Status))
				{
					return IsolateFailure(Status);
				}
				m_EffectPreparationTargets.insert(
					m_EffectPreparationTargets.end(),
					BossEffectAssetIds.begin(), BossEffectAssetIds.end());
			}
			std::sort(m_EffectPreparationTargets.begin(),
				m_EffectPreparationTargets.end());
			m_EffectPreparationTargets.erase(std::unique(
				m_EffectPreparationTargets.begin(),
				m_EffectPreparationTargets.end()),
				m_EffectPreparationTargets.end());
		}
		}
		/* Every published Area world Effect joins the existing loader worker.
		   The runtime document is optional outside the Valtan contract; malformed
		   published data is an isolated preparation failure, never source fallback. */
		const CLIENT_LEVEL_DESCRIPTOR* mapLevel = CLevelRegistry::Find(m_eNextLevelID);
		if (nullptr != mapLevel && nullptr != mapLevel->pMapAreaId)
		{
			const std::filesystem::path mapEffectsPath = CMapAssetCatalog::Get_MapDataRoot() /
				(std::string(mapLevel->pMapAreaId) + ".mapeffects.json");
			std::error_code mapEffectsError;
			const bool mapEffectsExist = std::filesystem::exists(mapEffectsPath, mapEffectsError);
			if (mapEffectsError)
				return IsolateFailure("Area Map Effect path could not be inspected: " + mapEffectsError.message());
			if (bValtanArena || mapEffectsExist)
			{
				CMapEffectDocument mapEffects;
				if (!mapEffects.Load(mapEffectsPath, mapLevel->pMapAreaId, Status))
					return IsolateFailure(Status);
				std::vector<std::string> mapEffectIds;
				for (const auto& world : mapEffects.Get_WorldEffects())
					mapEffectIds.push_back(world.effectAssetId);
				if (!mapEffectIds.empty())
				{
					std::vector<std::string> preparedMapEffects;
					if (!CEffectPresentationService::Queue_ProductTargets_Priority(
							mapEffectIds, preparedMapEffects, Status))
						return IsolateFailure(Status);
					m_EffectPreparationTargets.insert(m_EffectPreparationTargets.end(),
						preparedMapEffects.begin(), preparedMapEffects.end());
				}
			}
		}
		std::sort(m_EffectPreparationTargets.begin(), m_EffectPreparationTargets.end());
		m_EffectPreparationTargets.erase(std::unique(m_EffectPreparationTargets.begin(),
			m_EffectPreparationTargets.end()), m_EffectPreparationTargets.end());
		m_isEffectPreparationRegistered = true;
		m_iEffectPreparationTargetCount = static_cast<uint32_t>(
			m_EffectPreparationTargets.size());
		m_strEffectPreparationStatus =
			TargetLabel + ": queued " +
			std::to_string(m_iEffectPreparationTargetCount) +
			" Product Effects for the target presentation.";
	}
	if (!m_isEffectLoadJobStarted)
	{
		std::string JobStatus;
		if (!CEffectPresentationService::Begin_LoadingProductCuePreparation(
				m_pLoader->Get_EffectLoadJob(), m_iEffectLoadJobEpoch,
				m_EffectPreparationTargets, JobStatus))
		{
			return IsolateFailure(
				"Loader-worker Effect staging could not start: " + JobStatus);
		}
		m_isEffectLoadJobStarted = true;
		if (bKoukuArena) m_iKoukuV1StartedMs = GetTickCount64();
		m_strEffectPreparationStatus = JobStatus;
	}

	const EFFECT_PRODUCT_PREWARM_TARGET_PROBE Probe =
		CEffectPresentationService::Get_ProductCuePreparationProbe(
			m_EffectPreparationTargets);
	m_iEffectPreparationTargetCount = Probe.iTargetCount;
	m_iEffectPreparationPendingCount = Probe.iPendingCount;
	m_iEffectPreparationPreparedCount = Probe.iPreparedCount;
	m_iEffectPreparationFailedCount =
		Probe.iFailedCount + Probe.iUnavailableCount +
		(m_strEffectPreparationRegistrationFailure.empty() ? 0u : 1u);
	const bool_t bActivationReady =
		Is_ProductPrewarmTargetActivationReady(
			Probe, !m_strEffectPreparationRegistrationFailure.empty());
	if (!bActivationReady)
	{
		if (!m_strEffectPreparationRegistrationFailure.empty())
		{
			m_strEffectPreparationStatus =
				TargetLabel + ": Effect preparation isolated; waiting for "
				"the current Product Effect catalog revision.";
		}
		else
		{
			m_strEffectPreparationStatus =
				TargetLabel + ": preparing Product Effects " +
				std::to_string(m_iEffectPreparationPreparedCount +
					m_iEffectPreparationFailedCount) + "/" +
				std::to_string(m_iEffectPreparationTargetCount) +
				" (selected pending " +
				std::to_string(Probe.iPendingCount) +
				", background pending " +
				std::to_string(Probe.iQueuePendingCount) + ").";
		}
		return false;
	}
	if (bKoukuArena)
	{
		if (!m_isKoukuV1TimingRecorded)
		{
			m_isKoukuV1TimingRecorded = true;
			Write_EffectFailureDiagnostic("Kouku.Loading.V1Settled",
				"elapsed_ms=" + std::to_string(GetTickCount64() - m_iKoukuV1StartedMs) +
				" targets=" + std::to_string(m_iEffectPreparationTargetCount) +
				" prepared=" + std::to_string(m_iEffectPreparationPreparedCount) +
				" failed=" + std::to_string(m_iEffectPreparationFailedCount));
		}
		if (m_iEffectPreparationFailedCount != 0u)
		{
			m_strEffectPreparationStatus = "Required raid Effect preparation failed: " +
				m_strEffectPreparationRegistrationFailure;
			for (const auto& id : m_EffectPreparationTargets)
			{
				const auto failure = CEffectPresentationService::Get_ProductCuePreparationFailure(id);
				if (!failure.empty()) { m_strEffectPreparationStatus += id + ": " + failure; break; }
			}
			return false;
		}
		if (!m_pLoader->Finished()) return false;
		if (!m_isKoukuV2Prepared)
		{
#ifndef _DEBUG
			if (!CKoukuSaydonPresentationPlayer::Prewarm_ProductEffectResources(
				m_pDevice, m_pContext, m_KoukuV2EffectTargets, m_strEffectPreparationStatus))
			{
				m_strEffectPreparationRegistrationFailure = m_strEffectPreparationStatus;
				m_iEffectPreparationFailedCount = 1u;
				return false;
			}
#endif
			// Debug leaves V2 preparation to the existing first-use consumer.
			m_isKoukuV2Prepared = true;
		}
	}

	if (!m_strEffectPreparationRegistrationFailure.empty())
	{
		m_strEffectPreparationStatus =
			TargetLabel + ": Effect preparation isolated: " +
			m_strEffectPreparationRegistrationFailure +
			" Continuing without blocking on unrelated Effect work.";
	}
	else if (0u != m_iEffectPreparationFailedCount)
	{
		m_strEffectPreparationStatus =
			TargetLabel + ": Product Effect preparation settled with " +
			std::to_string(m_iEffectPreparationFailedCount) +
			" isolated failure(s); continuing.";
	}
	else
	{
		m_strEffectPreparationStatus =
			TargetLabel + ": prepared " +
			std::to_string(m_iEffectPreparationPreparedCount) +
			" Product Effects for the target presentation.";
	}
	return true;
}

void CLevel_Loading::Recover_FromFailure(const HRESULT result)
{
	if (m_isFailureReported)
		return;

	m_isFailureReported = true;
	if (LEVEL::KAKULSAYDON_ARENA == m_eNextLevelID)
		Write_EffectFailureDiagnostic("Kouku.Loading.Failed",
			"elapsed_ms=" + std::to_string(GetTickCount64() - m_iKoukuLoadStartedMs) +
			" status=" + m_strEffectPreparationStatus);
	if (m_isEffectLoadJobStarted && nullptr != m_pLoader)
	{
		CEffectPresentationService::Cancel_LoadingProductCuePreparation(
			m_pLoader->Get_EffectLoadJob(), m_iEffectLoadJobEpoch);
	}
	CCharacterSelectionState::Cancel_PendingCreation();
	Cancel_LobbyCommand("target level loading failed");
	/* The loader's live progress line names the stage that refused, and it is
	the only record of it once the loading Level is torn down. */
	CLevelTransitionService::Report_Recovery(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_LOAD_FAILED,
		"loading.target-resource-load",
		"[Loader] " + CLoader::Get_ActiveStatus() +
			(m_strEffectPreparationStatus.empty() ? "" : " / " + m_strEffectPreparationStatus),
		result);
	CNetworkManager::Get().Close_ServerConnection();

	if (FAILED(CGameInstance::Get().Clear_Resources(
		ETOUI(m_eNextLevelID))))
	{
		OutputDebugStringA(
			"[Level_Loading] Failed to clear partial target resources.\n");
	}

	OutputDebugStringA(
		"[Level_Loading] Load failed; session closed and partial resources rolled back.\n");

	if (LEVEL::LOBBY != m_eNextLevelID)
		Retry_LobbyLoad();
}

void CLevel_Loading::Cancel_LobbyCommand(const char_t* pReason)
{
	if (INVALID_LOBBY_COMMAND_TOKEN == m_iLobbyCommandToken)
		return;

	CLobbyCommandService::Cancel(m_iLobbyCommandToken, pReason);
	m_iLobbyCommandToken = INVALID_LOBBY_COMMAND_TOKEN;
}

void CLevel_Loading::Retry_LobbyLoad()
{
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"loading.recovery"))
	{
		OutputDebugStringA(
			"[Level_Loading] Failed to stage Lobby recovery.\n");
	}
}

HRESULT CLevel_Loading::Ready_Layer_Chrome()
{
	const filesystem::path layoutPath =
		CProjectDataRoot::Resolve(L"UI/Loading/LoadingLayout.json");

	ifstream stream(layoutPath);
	if (!stream.is_open())
		return S_OK;

	const string text(
		(istreambuf_iterator<char>(stream)),
		istreambuf_iterator<char>());

	DATA_JSON_VALUE root;
	string error;
	if (!CDataJson::Parse(text, root, error))
		return S_OK;

	const DATA_JSON_VALUE* pSlots = root.Find("slots");
	if (nullptr == pSlots || !pSlots->Is_Array())
		return S_OK;

	for (const DATA_JSON_VALUE& slot : pSlots->Get_Array())
	{
		const DATA_JSON_VALUE* pId = slot.Find("id");
		const DATA_JSON_VALUE* pRect = slot.Find("rect");
		const DATA_JSON_VALUE* pLayers = slot.Find("layers");
		if (nullptr == pId || !pId->Is_String() ||
			nullptr == pRect || !pRect->Is_Object())
			continue;

		const DATA_JSON_VALUE* pX = pRect->Find("x");
		const DATA_JSON_VALUE* pY = pRect->Find("y");
		const DATA_JSON_VALUE* pW = pRect->Find("width");
		const DATA_JSON_VALUE* pH = pRect->Find("height");
		if (nullptr == pX || nullptr == pY || nullptr == pW || nullptr == pH)
			continue;

		const f32_t fX = static_cast<f32_t>(pX->Get_Number());
		const f32_t fY = static_cast<f32_t>(pY->Get_Number());
		const f32_t fWidth = static_cast<f32_t>(pW->Get_Number());
		const f32_t fHeight = static_cast<f32_t>(pH->Get_Number());

		const string& strId = pId->Get_String();

		string strTexturePath;
		bool_t bFlipX = false;
		if (nullptr != pLayers && pLayers->Is_Array() && !pLayers->Get_Array().empty())
		{
			const DATA_JSON_VALUE& layer = pLayers->Get_Array().front();
			const DATA_JSON_VALUE* pPath = layer.Find("path");
			if (nullptr != pPath && pPath->Is_String())
				strTexturePath = pPath->Get_String();
			/* The side gradients are one texture placed twice, the second copy mirrored. Without
			this the mirrored copy draws its opaque edge inwards, as a hard vertical seam across
			the background. */
			const DATA_JSON_VALUE* pFlipX = layer.Find("flipX");
			if (nullptr != pFlipX && pFlipX->Is_Boolean())
				bFlipX = pFlipX->Get_Boolean();
		}

		/* Valtan Arena reuses every other chrome slot from the shared JSON as-is; only its
		Background art is swapped here instead of forking a second layout document for one
		texture path. */
		if ("Background" == strId && LEVEL::VALTAN_ARENA == m_eNextLevelID)
			strTexturePath = "UI/Loading/Loading_Background_Valtan.png";
		/* Character Select (creation) shows retail's prologue nebula ("시작되는 운명"). */
		if ("Background" == strId && LEVEL::CHARACTER_SELECT == m_eNextLevelID)
			strTexturePath = "UI/Loading/Loading_Background_Prologue.png";
		/* KoukuSaydon Arena. The raid names itself \xD55C\xBC24\xC911\xC758 \xC11C\xCEE4\xC2A4 in
		   EFTable_GameMsg sys.commander.dungeon_name_koukusaton, which is also what
		   the map id LV_LUT_MIDNIGHTC_ED spells out, and the loading hint is that
		   zone's own sys.hint.zone_commanderraid_011. */
		if ("Background" == strId && LEVEL::KAKULSAYDON_ARENA == m_eNextLevelID)
			strTexturePath = "UI/Loading/Loading_Background_Kouku.png";

		/* Texture-less slots (empty "layers") are position-only markers the HUD Layout Tool
		can still drag -- pull text draw positions from them instead of creating a sprite. */
		if (strTexturePath.empty())
		{
			/* Retail's text fields are top-anchored boxes tall enough for more lines than this
			screen draws, so a line goes at the top edge, not at the box centre. */
			const float2_t vTopCenter(fX + fWidth * 0.5f, fY);

			if ("TitleText" == strId)
				m_vTitlePos = vTopCenter;
			else if ("ScenarioLabel" == strId)
				m_vScenarioPos = vTopCenter;
			else if ("TipText" == strId)
				m_vTipPos = vTopCenter;
			else if ("ProgressMask" == strId)
			{
				/* Retail's crashLoadingTarget: a clip whose left edge is fixed and whose width
				at full progress is the track length the ratio multiplies. It draws nothing
				itself, so it stays a marker. */
				m_fProgressMaskLeft = fX;
				m_fProgressMaskWidth = fWidth;
			}

			continue;
		}

		/* The HUD Layout Tool's canvas (and this JSON) store top-left + size, but CUIObject
		positions itself by center -- convert once here instead of teaching the tool a second
		convention. */
		const wstring_t widePath(strTexturePath.begin(), strTexturePath.end());

		CUI_Sprite::UI_SPRITE_DESC Desc{};
		Desc.fX = fX + fWidth * 0.5f;
		Desc.fY = fY + fHeight * 0.5f;
		Desc.fSizeX = fWidth;
		Desc.fSizeY = fHeight;
		Desc.strTextureTag = widePath;

		shared_ptr<CGameObject> pObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::STATIC),
			TEXT("Prototype_GameObject_UI_Sprite"),
			ETOUI(LEVEL::LOADING), TEXT("Layer_Chrome"), &Desc, &pObject)))
		{
			/* One missing/renamed chrome texture should not take the whole loading screen down. */
			continue;
		}

		if (bFlipX)
			static_pointer_cast<CUI_Sprite>(pObject)->Set_FlipX(true);

		if ("ProgressFill" == strId)
		{
			m_pProgressFill = static_pointer_cast<CUI_Sprite>(pObject);
			m_fProgressFillLeft = fX;
			m_fProgressFillWidth = fWidth;
			m_fProgressFillCenterY = Desc.fY;
			m_fProgressFillHeight = fHeight;
		}
		else if ("ProgressGlow" == strId)
		{
			m_pProgressGlow = static_pointer_cast<CUI_Sprite>(pObject);
			m_fProgressGlowCenterY = Desc.fY;
			m_fProgressGlowWidth = fWidth;
			m_fProgressGlowHeight = fHeight;
		}
	}

	return S_OK;
}

unique_ptr<CLevel_Loading> CLevel_Loading::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const LEVEL eNextLevelID,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	auto instance = unique_ptr<CLevel_Loading>(
		new CLevel_Loading(pDevice, pContext));
	if (FAILED(instance->Initialize(
		eNextLevelID,
		lobbyCommandToken)))
		return nullptr;
	return instance;
}
