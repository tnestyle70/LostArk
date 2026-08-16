#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"
#include "Network/PacketMessages.h"
#include "RenderingProfileService.h"

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

class CMapTool;
class CEffect_Tool;
class CAnimation_Tool;
class CHUDLayoutTool;
class CHUDRuntimeView;
class CBalanceTool;
class CCharacterPreviewPanel;
class CSkillWindowView;
class CChatWindowView;
class CPartyWindowView;


class CMainApp final
{
#ifdef _DEBUG
private:
	enum class DEBUG_TOOL
	{
		NONE,
		MAP,
		ANIMATION,
		EFFECT,
		RENDERING,
		UI,
		BALANCE
	};
#endif

private:
	CMainApp();

public:
	~CMainApp();

public:
	HRESULT Initialize();
	void Update(f32_t fTimeDelta);
	HRESULT Render();

#ifdef _DEBUG
	static void Update_DebugWindowTitleWithFps(const wchar_t* pBaseTitle);
#endif

private:
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_For_Static();
	HRESULT Ready_Prototype_For_LoadingChrome();
	HRESULT Start_Level(
		LEVEL eTargetLevel,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	void Apply_LevelRequest();
	HRESULT ReadyImGuiRuntime();
	void RenderCombatHUD();
	void RenderBossHealthBar();
	/* Room-shared raid Esther gauge bar. Draws nothing when the snapshot says
	the world has no Esther roster (maximum 0). */
	void RenderEstherGauge();
	/* LanceMaster's 3-segment identity meter -- drawn procedurally (matching the real
	LanceMasterProgress.as formula: target.rotation = maxDegree * value/100, cascading through
	3 segments) rather than from extracted art, since the real asset's moving "target" piece is
	a vector shape this pipeline can't crop as an image (see .../HudGfx_Extracted notes). */
	void RenderLanceMasterIdentityGauge();
	void RenderSkillIcons();
	void RenderSkillCooldowns();
	/* Experimental replacement for RenderSkillIcons/RenderSkillCooldowns, built from the real
	extracted QuickSlot.gfx Scaleform asset (icon frame, cooldown sweep, on-use flash) instead
	of hand-placed layers -- see .../HudGfx_Extracted. */
	void RenderQuickSlot();
	void RenderCombatHUDText();

#ifdef _DEBUG
	HRESULT ReadyDebugTools();
	HRESULT EnsureDebugTool(DEBUG_TOOL eTool);
	void UpdateDebugToolShortcut();
	void RenderDeveloperTools();
	void RenderRenderingWorkbench();
	void RenderProfilerOverlay();
	void RenderProfilerSettings();
#endif

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	CRenderingProfileService m_RenderingProfiles;
	unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	/* Not _DEBUG-gated: the runtime HUD art must render in Release too. */
	unique_ptr<CHUDRuntimeView> m_pHUDRuntimeView = { nullptr };
	/* Edge-detects the local player's stance so RenderCombatHUD only calls
	CHUDRuntimeView::Play_KeyframeAnimation on an actual change (or the first frame a stance is
	known at all), instead of re-triggering the icon's animation every frame. NONE never matches a
	real stance, so the very first Render sees an edge and plays the arrival pose. */
	LostArk::Shared::PLAYER_STANCE_ID m_ePreviousHudStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	/* RenderQuickSlot edge-detects "skill just used" per Q..F slot as a ready-to-not-ready
	transition. iCooldownEndTick itself can't be compared directly across frames: for a ready
	skill CombatHUDViewModel defaults it to the current (ever-increasing) serverTick rather than
	a fixed sentinel, so a raw "did it grow" check fires every single frame. Index order matches
	RenderQuickSlot's own INPUT_SLOTS. */
	bool_t m_bPreviousQuickSlotReady[8] = { true, true, true, true, true, true, true, true };
	/* RenderLanceMasterIdentityGauge edge-detects each of the 3 identity segments reaching 100 to
	trigger the real extracted gauge0/1/2 highLightMc "burn" flourish (Lance_Id_GaugeBurn0/1/2)
	exactly once per fill, not every frame it stays full. */
	bool_t m_bLanceGaugeSegmentWasFull[3] = { false, false, false };
	/* Real time (ImGui::GetTime()) each segment's burn ignite flourish started, so RenderLance-
	MasterIdentityGauge can swap Lance_Id_GaugeBurn -> Lance_Id_GaugeBurnLoop once the one-shot
	ignite has actually finished playing, instead of restarting the dim->bright ignite clip on
	every loop iteration. Negative = not currently igniting/looping. */
	f64_t m_dLanceGaugeIgniteStartSeconds[3] = { -1.0, -1.0, -1.0 };
	bool_t m_bLanceGaugeLoopStarted[3] = { false, false, false };
	/* The Lobby's animated title-screen backdrop (Data/UI/Lobby/Lobby_Layout.json), drawn
	behind everything else instead of a flat clear color. Release-safe, like the HUD view. */
	unique_ptr<CHUDRuntimeView> m_pLobbyBackgroundView = { nullptr };
	/* Not _DEBUG-gated: K opens the skill window during real gameplay, in Release too. */
	unique_ptr<CSkillWindowView> m_pSkillWindowView = { nullptr };
	bool_t m_bKDown = false;
	/* Not _DEBUG-gated: Enter opens the chat input during real gameplay, in Release too. */
	unique_ptr<CChatWindowView> m_pChatWindowView = { nullptr };
	bool_t m_bEnterDown = false;
	bool_t m_bEscapeDown = false;
	/* Not _DEBUG-gated: the party roster overlay draws in Release too, same as the rest of the
	combat HUD. UI-only placeholder roster until a party Shared protocol exists. */
	unique_ptr<CPartyWindowView> m_pPartyWindowView = { nullptr };

#ifdef _DEBUG
	unique_ptr<CMapTool> m_pMapTool = { nullptr };
	unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
	unique_ptr<CAnimation_Tool> m_pAnimationTool = { nullptr };
	shared_ptr<CCharacterPreviewPanel> m_pCharacterPreviewPanel = { nullptr };
	unique_ptr<CHUDLayoutTool> m_pHUDLayoutTool = { nullptr };
	unique_ptr<CBalanceTool> m_pBalanceTool = { nullptr };
	bool_t m_bF1Down = false;
	bool_t m_bDeveloperToolsVisible = false;
	bool_t m_bProfilerVisible = false;
	bool_t m_bRenderQualityDraftInitialized = false;
	DEBUG_TOOL m_eActiveDebugTool = DEBUG_TOOL::NONE;
	RENDER_QUALITY_SETTINGS m_RenderQualityDraft = {};
	SCENE_RENDERING_PROFILE m_SceneRenderingDraft = {};
	string m_strRenderingDraftProfileId;
	string m_strToolStatus =
		"Select a tool. Map authoring targets the current level Area.";
	string m_strRenderingStatus =
		"Rendering profiles are loaded from the published runtime catalog.";
	string m_strProfilerCaptureStatus;
#endif

public:
	static unique_ptr<CMainApp> Create();
	void Free();
};

NS_END
