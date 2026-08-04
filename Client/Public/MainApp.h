#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

class CMapTool;
class CEffect_Tool;
class CAnimation_Tool;
class CHUDLayoutTool;

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
		UI
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
	HRESULT Start_Level(
		LEVEL eTargetLevel,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	void Apply_LevelRequest();
	HRESULT ReadyImGuiRuntime();
	void RenderCombatHUD();

#ifdef _DEBUG
	HRESULT ReadyDebugTools();
	HRESULT EnsureDebugTool(DEBUG_TOOL eTool);
	void UpdateDebugToolShortcut();
	void RenderDeveloperTools();
	void RenderProfilerOverlay();
	void RenderProfilerSettings();
#endif

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };

#ifdef _DEBUG
	unique_ptr<CMapTool> m_pMapTool = { nullptr };
	unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
	unique_ptr<CAnimation_Tool> m_pAnimationTool = { nullptr };
	unique_ptr<CHUDLayoutTool> m_pHUDLayoutTool = { nullptr };
	bool_t m_bF1Down = false;
	bool_t m_bDeveloperToolsVisible = false;
	bool_t m_bProfilerVisible = false;
	DEBUG_TOOL m_eActiveDebugTool = DEBUG_TOOL::NONE;
	string m_strToolStatus =
		"Select a tool. Map authoring targets the current level Area.";
	string m_strProfilerCaptureStatus;
#endif

public:
	static unique_ptr<CMainApp> Create();
	void Free();
};

NS_END
