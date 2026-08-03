#pragma once

#include "Client_Defines.h"
#include "ClientLaunchOptions.h"
#include "Engine_Defines.h"

#include <chrono>

//0 9 8 而ㅻ㎤?쒕줈 ?댁긽???고???議곗젅 媛?ν븯寃?援ы쁽

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

class CMapTool;
class CEffect_Tool;
class CAnimation_Tool;
class CHUDLayoutTool;
class CHUDRuntimeView;

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
	ComPtr<ID3D11Device>			m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };
	std::chrono::steady_clock::time_point m_SmokeStartTime;
	CLIENT_SCENARIO m_eSmokeScenario = CLIENT_SCENARIO::FRONT_LOBBY;
	bool_t m_isSmokeDispatched = false;
	bool_t m_hasSmokeObservedNetworkStage = false;
	bool_t m_hasSmokeObservedPendingEnterConnection = false;
	bool_t m_isSmokeComplete = false;
	std::unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	/* Not _DEBUG-gated: the runtime HUD art must render in Release too. */
	std::unique_ptr<CHUDRuntimeView> m_pHUDRuntimeView = { nullptr };

#ifdef _DEBUG
	std::unique_ptr<CMapTool> m_pMapTool = { nullptr };
	std::unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
	std::unique_ptr<CAnimation_Tool> m_pAnimationTool = { nullptr };
	std::unique_ptr<CHUDLayoutTool> m_pHUDLayoutTool = { nullptr };

	bool m_bF1Down = false;
	bool m_bDeveloperToolsVisible = false;
	bool m_bProfilerVisible = false;
	DEBUG_TOOL m_eActiveDebugTool = DEBUG_TOOL::NONE;
	string m_strProfilerCaptureStatus;
	string m_strSceneTransitionStatus = "F1 opens Developer Tools.";
#endif

private:
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_For_Static();
	HRESULT Ready_Prototype_For_LoadingChrome();
	HRESULT Start_Level(LEVEL eStartLevelID);
	void Apply_PendingSceneTransition();
	void UpdateSmokeHarness();
	void CompleteSmokeHarness(bool_t succeeded, const char_t* pReason);
	HRESULT ReadyImGuiRuntime();
	void RenderCombatHUD();
	void RenderOfflinePreviewOverlay();

#ifdef _DEBUG
	HRESULT ReadyDebugTools();
	HRESULT EnsureDebugTool(CLIENT_SCENARIO eScenario);
	void UpdateDebugToolShortcut();
	void RenderDeveloperTools();
	void RenderProfilerOverlay();
	void RenderProfilerSettings();
#endif

public:
	static unique_ptr<CMainApp> Create();
	void Free();
};

NS_END

