#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"
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
	void RenderSkillIcons();
	void RenderSkillCooldowns();
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
