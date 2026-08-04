#include "imgui.h"

#include "MainApp.h"

#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "ImGuiLayer.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "Level_Loading.h"
#include "NetworkManager.h"
#include "Profiler.h"
#include "RuntimeAssetRoot.h"

#ifdef _DEBUG
#include "Animation_Tool.h"
#include "Effect_Tool.h"
#include "HUDLayoutTool.h"
#include "MapTool.h"
#include "ProfilerCaptureIO.h"
#endif

namespace
{
#ifdef _DEBUG
	bool_t IsWindowOwnedByCurrentProcess(HWND hWnd)
	{
		if (nullptr == hWnd)
			return false;

		DWORD processId = {};
		return 0 != GetWindowThreadProcessId(hWnd, &processId) &&
			GetCurrentProcessId() == processId;
	}
#endif

	const char_t* GetCharacterClassDisplayName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "Lance Master";
		// TODO: display-only relabel until real character assets exist; underlying class/binding is unchanged.
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "War Lord";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Alchemist";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Dimension Master";
		default:
			return "Unknown";
		}
	}

	/* HUD_Layout.json's "ownerClass" strings (no spaces) -- separate from the display name
	above since the JSON schema/tool already used these exact names. */
	const string GetHUDOwnerClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "WarLord";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Alchemist";
		case CHARACTER_CLASS_ID::ARTIST:
			return "DimensionMaster";
		default:
			return "";
		}
	}
}

CMainApp::CMainApp()
{
}

CMainApp::~CMainApp()
{
	Free();
}

HRESULT CMainApp::Initialize()
{
	/* CreateWICTextureFromFile (used by the HUD runtime view for non-DDS art) needs COM on the
	calling thread. The main thread never initializes it otherwise. */
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	ENGINE_DESC engineDesc{};
	engineDesc.hInstance = g_hInst;
	engineDesc.hWnd = g_hWnd;
	engineDesc.eWinMode = WINMODE::WIN;
	engineDesc.iNumLevels = ETOUI(LEVEL::END);
	engineDesc.iWinSizeX = g_iWinSizeX;
	engineDesc.iWinSizeY = g_iWinSizeY;

	if (FAILED(CGameInstance::Get().Initialize_Engine(
		engineDesc,
		m_pDevice,
		m_pContext)))
	{
		return E_FAIL;
	}

	if (!CNetworkManager::Get().Initialize())
		return E_FAIL;
	if (FAILED(ReadyImGuiRuntime()))
		return E_FAIL;

#ifdef _DEBUG
	if (FAILED(ReadyDebugTools()))
		return E_FAIL;
#endif

	if (FAILED(Ready_Fonts()) ||
		FAILED(Ready_Prototype_For_Static()))
	{
		return E_FAIL;
	}

	m_pHUDRuntimeView = std::make_unique<CHUDRuntimeView>(m_pDevice, m_pContext);

	if (FAILED(Start_Level(LEVEL::LOBBY)))
		return E_FAIL;

	return S_OK;
}

void CMainApp::Update(const f32_t fTimeDelta)
{
#ifdef _DEBUG
	UpdateDebugToolShortcut();
#endif

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->BeginFrame();

#ifdef _DEBUG
	const bool_t mapToolOpen = m_bDeveloperToolsVisible &&
		nullptr != m_pMapTool && m_pMapTool->IsOpen();
	const HWND foregroundWindow = GetForegroundWindow();
	const bool_t externalToolFocused = mapToolOpen &&
		nullptr != foregroundWindow &&
		foregroundWindow != g_hWnd &&
		IsWindowOwnedByCurrentProcess(foregroundWindow);
	const bool_t worldLeftMouseConsumed =
		nullptr != m_pMapTool && m_pMapTool->ConsumesWorldLeftMouse();
#else
	constexpr bool_t externalToolFocused = false;
	constexpr bool_t worldLeftMouseConsumed = false;
#endif

	const bool_t keyboardCaptured = nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->WantsCaptureKeyboard() || externalToolFocused);
	const bool_t mouseCaptured = nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->WantsCaptureMouse() || externalToolFocused);
	CGameInstance::Get().SetInputBlocked(keyboardCaptured, mouseCaptured);
	CGameInstance::Get().SetMouseButtonBlocked(
		DIM::LB,
		worldLeftMouseConsumed);

	CNetworkManager::Get().Update();
	CGameInstance::Get().Update_Engine(fTimeDelta);

#ifdef _DEBUG
	if (nullptr != m_pMapTool)
		m_pMapTool->Update(fTimeDelta);
#endif

	// 현재 Level의 Update가 끝난 뒤에만 기존 Level을 파괴한다.
	Apply_LevelRequest();
}

HRESULT CMainApp::Render()
{
	float4_t clearColor = { 0.008f, 0.012f, 0.025f, 1.f };
	if (FAILED(CGameInstance::Get().Render_Begin(&clearColor)))
	{
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return E_FAIL;
	}

	if (FAILED(CGameInstance::Get().Render()))
	{
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return E_FAIL;
	}

	if (nullptr != m_pImGuiLayer)
	{
		RenderCombatHUD();
#ifdef _DEBUG
		if (m_bDeveloperToolsVisible)
		{
			RenderDeveloperTools();
			switch (m_eActiveDebugTool)
			{
			case DEBUG_TOOL::MAP:
				if (nullptr != m_pMapTool)
					m_pMapTool->Render();
				break;
			case DEBUG_TOOL::ANIMATION:
				if (nullptr != m_pAnimationTool)
					m_pAnimationTool->Render();
				break;
			case DEBUG_TOOL::EFFECT:
				if (nullptr != m_pEffectTool)
					m_pEffectTool->Render();
				break;
			case DEBUG_TOOL::UI:
				if (nullptr != m_pHUDLayoutTool)
					m_pHUDLayoutTool->Render();
				break;
			default:
				break;
			}

			if (m_bProfilerVisible)
			{
				RenderProfilerOverlay();
				RenderProfilerSettings();
			}
		}
#endif
		m_pImGuiLayer->EndFrame();
	}

	return CGameInstance::Get().Render_End();
}

void CMainApp::RenderCombatHUD()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT))
	{
		return;
	}

	const HUD_PLAYER_STATE& player =
		CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumHp ||
		0u == player.iMaximumResource)
	{
		return;
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowPos(
		ImVec2(
			viewport->WorkPos.x + 20.f,
			viewport->WorkPos.y + viewport->WorkSize.y - 20.f),
		ImGuiCond_Always,
		ImVec2(0.f, 1.f));
	ImGui::SetNextWindowBgAlpha(0.78f);
	constexpr ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoInputs;

	if (ImGui::Begin("##RuntimeCombatHUD", nullptr, flags))
	{
		ImGui::Text("%s", GetCharacterClassDisplayName(
			player.eCharacterClass));
		ImGui::Text("HP  %u / %u", player.iCurrentHp, player.iMaximumHp);
		ImGui::ProgressBar(
			static_cast<float>(player.iCurrentHp) /
			static_cast<float>(player.iMaximumHp),
			ImVec2(280.f, 0.f));
		ImGui::Text(
			"Resource  %u / %u",
			player.iCurrentResource,
			player.iMaximumResource);
		ImGui::ProgressBar(
			static_cast<float>(player.iCurrentResource) /
			static_cast<float>(player.iMaximumResource),
			ImVec2(280.f, 0.f));
		for (const HUD_SKILL_STATE& skill : player.Skills)
		{
			const std::uint32_t remainingTicks =
				skill.iCooldownEndTick > player.iServerTick ?
				skill.iCooldownEndTick - player.iServerTick : 0u;
			ImGui::Text(
				"[%s] %s  %.1fs  DMG %u",
				skill.strInputSlot.c_str(),
				skill.strDisplayName.c_str(),
				static_cast<float>(remainingTicks) / 30.f,
				skill.iDamage);
		}
	}
	ImGui::End();

	if (nullptr != m_pHUDRuntimeView)
	{
		/* Base state only for now -- no gauge/resource-driven stage switching yet. */
		const string strOwnerClass = GetHUDOwnerClassName(player.eCharacterClass);
		m_pHUDRuntimeView->Render(strOwnerClass, 0);
	}
}

HRESULT CMainApp::Ready_Fonts()
{
	const filesystem::path fontPath =
		CRuntimeAssetRoot::Resolve_Font(L"161ex.spritefont");
	return fontPath.empty() || FAILED(CGameInstance::Get().Add_Font(
		TEXT("Font_Default"),
		fontPath.c_str())) ? E_FAIL : S_OK;
}

HRESULT CMainApp::Ready_Prototype_For_Static()
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC),
		TEXT("Prototype_Component_Shader_VtxTex"),
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxTex.hlsl"),
			VTXTEX::Elements,
			VTXTEX::iNumElements))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::STATIC),
			TEXT("Prototype_Component_VIBuffer_Rect"),
			CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CMainApp::Start_Level(const LEVEL eTargetLevel)
{
	if (nullptr == CLevelRegistry::Find(eTargetLevel))
		return E_INVALIDARG;

	unique_ptr<CLevel_Loading> loading =
		CLevel_Loading::Create(m_pDevice, m_pContext, eTargetLevel);
	if (nullptr == loading)
		return E_FAIL;

	return CGameInstance::Get().Change_Level(
		ETOUI(LEVEL::LOADING),
		move(loading));
}

void CMainApp::Apply_LevelRequest()
{
	LEVEL_TRANSITION_REQUEST request{};
	if (!CLevelTransitionService::Try_Consume(request))
		return;

	if (LEVEL_TRANSITION_PHASE::LOAD == request.ePhase)
	{
		const HRESULT result = Start_Level(request.eTargetLevel);
		if (FAILED(result))
			CLevelTransitionService::Report_LoadFailure(result);
		return;
	}

	unique_ptr<CLevel> nextLevel = CLevelRegistry::Create_Level(
		request.eTargetLevel,
		m_pDevice,
		m_pContext);
	if (nullptr != nextLevel && SUCCEEDED(CGameInstance::Get().Change_Level(
		ETOUI(request.eTargetLevel),
		move(nextLevel))))
	{
		return;
	}

	CGameInstance::Get().Clear_Resources(ETOUI(request.eTargetLevel));
	CNetworkManager::Get().Close_ServerConnection();
	CLevelTransitionService::Report_LoadFailure(E_FAIL);
	if (LEVEL::LOBBY != request.eTargetLevel)
	{
		CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"main-app.activation-failure");
	}
}

HRESULT CMainApp::ReadyImGuiRuntime()
{
	m_pImGuiLayer = make_unique<Engine::CImGuiLayer>();
	if (!m_pImGuiLayer->Initialize(
		g_hWnd,
		m_pDevice.Get(),
		m_pContext.Get()))
	{
		return E_FAIL;
	}
	return S_OK;
}

#ifdef _DEBUG
HRESULT CMainApp::ReadyDebugTools()
{
	if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Reset_History();
		pProfiler->Set_Enabled(false);
	}
	m_bProfilerVisible = false;
	return S_OK;
}

HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)
{
	switch (eTool)
	{
	case DEBUG_TOOL::MAP:
		if (nullptr == m_pMapTool)
		{
			auto mapTool = make_unique<CMapTool>();
			if (FAILED(mapTool->Initialize(m_pDevice, m_pContext)))
				return E_FAIL;
			m_pMapTool = move(mapTool);
		}
		m_pMapTool->SetOpen(true);
		break;
	case DEBUG_TOOL::ANIMATION:
		if (nullptr == m_pAnimationTool)
			m_pAnimationTool = make_unique<CAnimation_Tool>();
		break;
	case DEBUG_TOOL::EFFECT:
		if (nullptr == m_pEffectTool)
			m_pEffectTool = make_unique<CEffect_Tool>(m_pDevice);
		break;
	case DEBUG_TOOL::UI:
		if (nullptr == m_pHUDLayoutTool)
			m_pHUDLayoutTool =
				make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
		break;
	default:
		return E_INVALIDARG;
	}

	m_eActiveDebugTool = eTool;
	return S_OK;
}

void CMainApp::RenderDeveloperTools()
{
	if (!ImGui::Begin(
		"LostArk Developer Tools",
		&m_bDeveloperToolsVisible,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	const uint32_t currentLevelId =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isTestLevel =
		ETOUI(LEVEL::DEVELOPMENT) == currentLevelId;
	ImGui::Text("Current level id: %u", currentLevelId);
	ImGui::TextDisabled(
		"Authoring tools are enabled only in the server-authorized Test level.");
	ImGui::SeparatorText("Tools");

	const auto toolButton = [this, isTestLevel](
		const char_t* pLabel,
		const DEBUG_TOOL eTool)
	{
		ImGui::BeginDisabled(!isTestLevel);
		if (ImGui::Button(pLabel))
		{
			m_strToolStatus = SUCCEEDED(EnsureDebugTool(eTool)) ?
				"Tool opened." : "Tool initialization failed.";
		}
		ImGui::EndDisabled();
	};

	toolButton("Map Tool", DEBUG_TOOL::MAP);
	ImGui::SameLine();
	toolButton("Animation Tool", DEBUG_TOOL::ANIMATION);
	toolButton("Effect Tool", DEBUG_TOOL::EFFECT);
	ImGui::SameLine();
	toolButton("HUD Layout Tool", DEBUG_TOOL::UI);
	ImGui::TextWrapped("%s", m_strToolStatus.c_str());

	ImGui::SeparatorText("Diagnostics");
	bool_t profilerVisible = m_bProfilerVisible;
	if (ImGui::Checkbox("Profiler", &profilerVisible))
	{
		m_bProfilerVisible = profilerVisible;
		if (Engine::CProfiler* pProfiler =
			CGameInstance::Get().Get_Profiler())
		{
			if (m_bProfilerVisible)
				pProfiler->Reset_History();
			pProfiler->Set_Enabled(m_bProfilerVisible);
		}
	}
	ImGui::TextDisabled("F1: Developer Tools  |  F6: Follow/Free Camera");
	ImGui::End();
}

void CMainApp::RenderProfilerOverlay()
{
	if (!m_bProfilerVisible)
		return;

	Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
	if (nullptr == pProfiler)
		return;

	Engine::FProfilerLiveStats stats{};
	const bool_t hasStats = pProfiler->Get_LiveStats(stats);
	const ImGuiIO& io = ImGui::GetIO();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowPos(
		ImVec2(viewport->WorkPos.x + 10.f, viewport->WorkPos.y + 30.f),
		ImGuiCond_Always);
	constexpr ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoInputs;
	if (ImGui::Begin("##LostArkPerfOverlay", nullptr, flags))
	{
		ImGui::Text("FPS: %.1f  (%.3f ms)",
			io.Framerate,
			io.Framerate > 0.f ? 1000.f / io.Framerate : 0.f);
		if (hasStats)
		{
			ImGui::Text("CPU: %.3f ms", stats.CpuFrameMs);
			ImGui::Text("GPU: %s",
				stats.GpuValid ? "available" : "warming up");
			if (stats.GpuValid)
				ImGui::Text("GPU time: %.3f ms", stats.GpuFrameMs);
		}
	}
	ImGui::End();
}

void CMainApp::RenderProfilerSettings()
{
	if (!m_bProfilerVisible)
		return;

	if (!ImGui::Begin(
		"LostArk Profiler Details",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler();
	if (ImGui::Button("Reset profiler history") && nullptr != pProfiler)
		pProfiler->Reset_History();
	ImGui::SameLine();
	if (ImGui::Button("Save profiler JSON"))
	{
		if (nullptr == pProfiler)
		{
			m_strProfilerCaptureStatus = "Profiler is not available.";
		}
		else
		{
			const Engine::FProfilerCaptureSnapshot snapshot =
				pProfiler->Snapshot();
			const uint64_t frameNumber = snapshot.Frames.empty() ?
				0u : snapshot.Frames.back().FrameNumber;
			const filesystem::path outputPath =
				CProfilerCaptureIO::Make_DefaultPath(frameNumber);
			string error;
			m_strProfilerCaptureStatus = CProfilerCaptureIO::Save_Json(
				snapshot,
				outputPath,
				&error) ? "Saved: " + outputPath.string() : error;
		}
	}
	if (!m_strProfilerCaptureStatus.empty())
		ImGui::TextWrapped("%s", m_strProfilerCaptureStatus.c_str());
	ImGui::End();
}

void CMainApp::UpdateDebugToolShortcut()
{
	const bool_t windowFocused =
		IsWindowOwnedByCurrentProcess(GetForegroundWindow());
	const bool_t f1Down = windowFocused &&
		0 != (GetAsyncKeyState(VK_F1) & 0x8000);
	if (f1Down && !m_bF1Down)
		m_bDeveloperToolsVisible = !m_bDeveloperToolsVisible;
	m_bF1Down = f1Down;
}
#endif

unique_ptr<CMainApp> CMainApp::Create()
{
	auto instance = unique_ptr<CMainApp>(new CMainApp());
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}

void CMainApp::Free()
{
	CNetworkManager::Get().Shutdown();
	CGameInstance::Get().SetInputBlocked(false, false);

#ifdef _DEBUG
	if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
		pProfiler->Set_Enabled(false);
	m_pAnimationTool.reset();
	m_pEffectTool.reset();
	m_pHUDLayoutTool.reset();
	m_pMapTool.reset();
#endif

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->Shutdown();
	m_pImGuiLayer.reset();
	CGameInstance::Get().Release_Engine();
}
