#include "imgui.h"

#include "MainApp.h"

#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "Effect_Object.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "ImGuiLayer.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "Level_Loading.h"
#include "LobbyCommandService.h"
#include "NetworkManager.h"
#include "Profiler.h"
#include "Presentation_Manager.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "SkillWindowView.h"
#include "UI_Sprite.h"

#ifdef _DEBUG
#include "Animation_Tool.h"
#include "BalanceTool.h"
#include "CharacterPreviewPanel.h"
#include "Effect_Tool.h"
#include "HUDLayoutTool.h"
#include "MapEditorWorkspaceService.h"
#include "MapTool.h"
#include "NetworkPlayerCommandSink.h"
#include "ProfilerCaptureIO.h"
#endif

#include <algorithm>
#include <fstream>

namespace
{
	wstring Utf8ToWide(const string& value)
	{
		if (value.empty())
			return {};
		const int length = MultiByteToWideChar(
			CP_UTF8, MB_ERR_INVALID_CHARS, value.c_str(),
			static_cast<int>(value.size()), nullptr, 0);
		if (length <= 0)
			return {};
		wstring result(static_cast<size_t>(length), L'\0');
		if (length != MultiByteToWideChar(
			CP_UTF8, MB_ERR_INVALID_CHARS, value.c_str(),
			static_cast<int>(value.size()), result.data(), length))
		{
			return {};
		}
		return result;
	}

	/* Not _DEBUG-gated: the K (skill window) toggle below needs this in Release too, not just
	the _DEBUG-only map tool focus check further down. */
	bool_t IsWindowOwnedByCurrentProcess(HWND hWnd)
	{
		if (nullptr == hWnd)
			return false;

		DWORD processId = {};
		return 0 != GetWindowThreadProcessId(hWnd, &processId) &&
			GetCurrentProcessId() == processId;
	}

#ifdef _DEBUG

	const char_t* GetHUDLayoutClassId(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Yinyangshi";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "DimensionMaster";
		case CHARACTER_CLASS_ID::WARLORD:
			return "Warlord";
		default:
			return "Default";
		}
	}
#endif

	/* HUD_Layout.json's "ownerClass" strings (no spaces) must match the schema/tool names. */
	const string GetHUDOwnerClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Yinyangshi";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "DimensionMaster";
		case CHARACTER_CLASS_ID::WARLORD:
			return "Warlord";
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
	string renderingProfileStatus;
	if (!m_RenderingProfiles.Load_Runtime(renderingProfileStatus))
	{
		OutputDebugStringA((
			"[MainApp] Rendering profile initialization failed: " +
			renderingProfileStatus + "\n").c_str());
#ifdef _DEBUG
		MessageBoxA(g_hWnd, renderingProfileStatus.c_str(),
			"Rendering Profile Load Failed", MB_OK | MB_ICONERROR);
#endif
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

	std::string effectCatalogStatus;
	if (!CEffectCatalog::Load(effectCatalogStatus))
	{
		const std::string diagnostic =
			"[MainApp] Effect Catalog initialization failed: " +
			effectCatalogStatus + "\n";
		OutputDebugStringA(diagnostic.c_str());
#ifdef _DEBUG
		MessageBoxA(g_hWnd, effectCatalogStatus.c_str(),
			"Effect Catalog Load Failed", MB_OK | MB_ICONERROR);
#endif
		return E_FAIL;
	}

	m_pHUDRuntimeView = std::make_unique<CHUDRuntimeView>(m_pDevice, m_pContext);
	m_pLobbyBackgroundView = std::make_unique<CHUDRuntimeView>(
		m_pDevice, m_pContext, L"UI/Lobby/Lobby_Layout.json",
		CHUDRuntimeView::DRAW_TARGET::BACKGROUND);
	m_pSkillWindowView = std::make_unique<CSkillWindowView>(m_pDevice, m_pContext);

	if (FAILED(Start_Level(LEVEL::LOBBY)))
		return E_FAIL;

	return S_OK;
}

void CMainApp::Update(const f32_t fTimeDelta)
{
#ifdef _DEBUG
	UpdateDebugToolShortcut();
#endif

	/* Not _DEBUG-gated: K is a normal gameplay keybind (the skill window), not one of the
	F1/F6 tool-switch keys AGENTS.md reserves. Skip it while ImGui already owns text input,
	so typing in the rune search box (once that becomes real) cannot also toggle the window. */
	if (nullptr != m_pSkillWindowView && !ImGui::GetIO().WantTextInput)
	{
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t kDown = windowFocused &&
			0 != (GetAsyncKeyState(0x4B /* VK_K */) & 0x8000);
		if (kDown && !m_bKDown)
			m_pSkillWindowView->Toggle();
		m_bKDown = kDown;
	}

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
	CEffectPresentationService::Synchronize_FollowAnchors();
	CEffectPresentationService::Update(fTimeDelta);

#ifdef _DEBUG
	if (nullptr != m_pMapTool)
		m_pMapTool->Update(fTimeDelta);
	if (nullptr != m_pEffectTool)
		m_pEffectTool->Update(fTimeDelta);
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
		if (nullptr != m_pLobbyBackgroundView &&
			ETOUI(LEVEL::LOBBY) == CGameInstance::Get().Get_CurrentLevelID())
		{
			m_pLobbyBackgroundView->Render("", 0);
		}
	#ifdef _DEBUG
		const HUD_PLAYER_STATE& hudPlayer =
			CCombatHUDViewModel::Get().Get_Player();
		const uint32_t hudLevel =
			CGameInstance::Get().Get_CurrentLevelID();
		const bool_t supportsAuthoredHUD =
			ETOUI(LEVEL::CHARACTER_SELECT) == hudLevel ||
			ETOUI(LEVEL::DEVELOPMENT) == hudLevel ||
			ETOUI(LEVEL::BERN) == hudLevel ||
			ETOUI(LEVEL::VALTAN_ARENA) == hudLevel;
		/* Same reason RenderCombatHUD skips m_pHUDRuntimeView while the Skill Window is open --
		this is a second, independent path that draws the same class emblem/bars and was not
		gated on that the first time, so it kept bleeding through underneath. */
		const bool_t skillWindowOpenForPreview =
			nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
		if (nullptr != m_pHUDLayoutTool && hudPlayer.isValid &&
			supportsAuthoredHUD && !skillWindowOpenForPreview)
		{
			m_pHUDLayoutTool->Render_RuntimePreview(
				GetHUDLayoutClassId(hudPlayer.eCharacterClass));
		}
	#endif
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
			case DEBUG_TOOL::RENDERING:
				RenderRenderingWorkbench();
				break;
			case DEBUG_TOOL::UI:
				/* Skill Window's slots (tripod plate, node glows, ...) are placed and dragged
				right in this same canvas, exactly like Combat HUD/Screen UI/Loading Screen --
				CHUDLayoutTool::Render_Canvas already draws and hit-tests m_Slots generically
				regardless of which document tab is active, so no separate preview window is
				needed (an earlier version of this code opened one; it only ended up floating
				over this window and blocking it instead of helping). */
				if (nullptr != m_pHUDLayoutTool)
					m_pHUDLayoutTool->Render();
				break;
			case DEBUG_TOOL::BALANCE:
				if (nullptr != m_pBalanceTool)
					m_pBalanceTool->Render();
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
	RenderCombatHUDText();

	return CGameInstance::Get().Render_End();
}

void CMainApp::RenderCombatHUD()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
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

	/* The Combat HUD draws to the always-on-top foreground layer, so it would otherwise show
	through around/behind the Skill Window (which does not necessarily cover every pixel of the
	viewport) instead of being hidden by it like a real full-screen menu hides the HUD. Character
	Select's own ClassSelect_Layout.json panel (Level_CharacterSelect::Render_ClassList) now owns
	that screen's class/identity preview art, so this rough HUD-region preview would otherwise
	just collide with it at the same screen position instead of adding anything. */
	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	const bool_t characterSelectOwnsPreview =
		ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel;

	if (!skillWindowOpen && !characterSelectOwnsPreview && nullptr != m_pHUDRuntimeView)
	{
		/* Base state only for now -- no gauge/resource-driven stage switching yet. */
		const string strOwnerClass = GetHUDOwnerClassName(player.eCharacterClass);
		m_pHUDRuntimeView->Render(strOwnerClass, 0);
	}

	if (nullptr != m_pSkillWindowView)
		m_pSkillWindowView->Render(player.eCharacterClass);
}

void CMainApp::RenderCombatHUDText()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}
	if (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open())
		return;
	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	const float scaleX = viewportSize.x / 1280.f;
	const float scaleY = viewportSize.y / 720.f;
	const float textScale = (std::min)(scaleX, scaleY);
	const auto position = [scaleX, scaleY](const float x, const float y)
	{
		return float2_t(x * scaleX, y * scaleY);
	};
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (player.isValid && player.iMaximumHp > 0u && player.iMaximumResource > 0u)
	{
		const wstring hp = L"HP  " + std::to_wstring(player.iCurrentHp) +
			L" / " + std::to_wstring(player.iMaximumHp);
		const wstring mana = L"MANA  " + std::to_wstring(player.iCurrentResource) +
			L" / " + std::to_wstring(player.iMaximumResource);
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), hp.c_str(),
			position(448.f, 614.f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.42f * textScale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), mana.c_str(),
			position(889.f, 614.f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.42f * textScale);
	}
	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!boss.isValid || 0u == boss.iMaximumHp ||
		0u == boss.iMaximumHealthBars)
	{
		return;
	}
	const std::uint32_t currentHealthBar = 0u == boss.iCurrentHp ? 0u :
		static_cast<std::uint32_t>((
			static_cast<std::uint64_t>(boss.iCurrentHp) * boss.iMaximumHealthBars +
			boss.iMaximumHp - 1u) / boss.iMaximumHp);
	const wstring name = Utf8ToWide(boss.strDisplayName);
	const wstring bars = std::to_wstring(currentHealthBar) + L" / " +
		std::to_wstring(boss.iMaximumHealthBars) + L" \xC904";
	const wstring hp = L"HP  " + std::to_wstring(boss.iCurrentHp) + L" / " +
		std::to_wstring(boss.iMaximumHp);
	CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), name.c_str(),
		position(640.f, 32.f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.55f * textScale);
	CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), bars.c_str(),
		position(640.f, 57.f), Colors::Yellow, 0.f, float2_t(0.5f, 0.5f), 0.46f * textScale);
	CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), hp.c_str(),
		position(640.f, 77.f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.42f * textScale);
}

HRESULT CMainApp::Ready_Fonts()
{
	const filesystem::path fontPath =
		CRuntimeAssetRoot::Resolve_Font(L"161ex.spritefont");
	if (fontPath.empty() || FAILED(CGameInstance::Get().Add_Font(
		TEXT("Font_Default"),
		fontPath.c_str())))
	{
		return E_FAIL;
	}

	/* LostArk's own source fonts (see SourceData/LPK/font/Binaries/Fonts/FontMap.xml),
	converted to DirectXTK .spritefont via MakeSpriteFont. Tag names mirror the
	original $-prefixed FontMap keys. */
	struct SOURCE_FONT { const tchar_t* strTag; const wchar_t* strFile; };
	constexpr SOURCE_FONT sourceFonts[] =
	{
		{ TEXT("Font_YG760"), L"YG760.spritefont" },
		{ TEXT("Font_YG330"), L"YG330.spritefont" },
		{ TEXT("Font_YoonGasiIIM"), L"YoonGasiIIM.spritefont" },
		{ TEXT("Font_EventDamage"), L"BMKkubulim.spritefont" },
	};

	for (const SOURCE_FONT& sourceFont : sourceFonts)
	{
		const filesystem::path sourceFontPath =
			CRuntimeAssetRoot::Resolve_Font(sourceFont.strFile);
		if (sourceFontPath.empty() || FAILED(CGameInstance::Get().Add_Font(
			sourceFont.strTag,
			sourceFontPath.c_str())))
		{
			return E_FAIL;
		}
	}

	return S_OK;
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

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC),
		TEXT("Prototype_GameObject_EffectObject"),
		CEffectObject::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	/* For.Prototype_GameObject_UI_Sprite */
	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC),
		TEXT("Prototype_GameObject_UI_Sprite"),
		CUI_Sprite::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	/* Every texture the loading-screen JSON references gets its own Texture prototype up
	front, keyed by its Resources-relative path -- CLevel_Loading::Ready_Layer_Chrome() Clones
	Prototype_GameObject_UI_Sprite once per slot and looks the texture prototype up by that
	same path string. */
	return Ready_Prototype_For_LoadingChrome();
}

HRESULT CMainApp::Ready_Prototype_For_LoadingChrome()
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

	vector<wstring_t> registeredPaths;
	for (const DATA_JSON_VALUE& slot : pSlots->Get_Array())
	{
		const DATA_JSON_VALUE* pLayers = slot.Find("layers");
		if (nullptr == pLayers || !pLayers->Is_Array())
			continue;

		for (const DATA_JSON_VALUE& layer : pLayers->Get_Array())
		{
			const DATA_JSON_VALUE* pPath = layer.Find("path");
			if (nullptr == pPath || !pPath->Is_String() || pPath->Get_String().empty())
				continue;

			/* Loading chrome paths are plain ASCII filenames, so a naive widen is safe here. */
			const string& narrowPath = pPath->Get_String();
			const wstring_t widePath(narrowPath.begin(), narrowPath.end());

			if (registeredPaths.end() != find(registeredPaths.begin(), registeredPaths.end(), widePath))
				continue;
			registeredPaths.push_back(widePath);

			const filesystem::path resolvedPath = CRuntimeAssetRoot::Resolve(widePath);
			if (resolvedPath.empty())
				continue;

			if (FAILED(CGameInstance::Get().Add_Prototype(
				ETOUI(LEVEL::STATIC), widePath,
				CTexture::Create(m_pDevice, m_pContext, resolvedPath.c_str(), 1))))
			{
				return E_FAIL;
			}
		}
	}

	return S_OK;
}

HRESULT CMainApp::Start_Level(
	const LEVEL eTargetLevel,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	const CLIENT_LEVEL_DESCRIPTOR* pTarget =
		CLevelRegistry::Find(eTargetLevel);
	if (nullptr == pTarget || nullptr == pTarget->pRenderingProfileId ||
		!m_RenderingProfiles.Has_Profile(pTarget->pRenderingProfileId) ||
		!m_RenderingProfiles.Has_Profile(
			CRenderingProfileService::LOADING_PROFILE_ID))
	{
		return E_INVALIDARG;
	}

	unique_ptr<CLevel_Loading> loading =
		CLevel_Loading::Create(
			m_pDevice,
			m_pContext,
			eTargetLevel,
			lobbyCommandToken);
	if (nullptr == loading)
		return E_FAIL;

	const string previousProfileId =
		m_RenderingProfiles.Get_ActiveProfileId();
	string status;
	if (!m_RenderingProfiles.Activate_Profile(
		CRenderingProfileService::LOADING_PROFILE_ID, status))
	{
		return E_FAIL;
	}
	const HRESULT hChange = CGameInstance::Get().Change_Level(
		ETOUI(LEVEL::LOADING),
		move(loading));
	if (FAILED(hChange))
	{
		if (!previousProfileId.empty())
		{
			string rollbackStatus;
			if (!m_RenderingProfiles.Activate_Profile(
				previousProfileId, rollbackStatus))
			{
				OutputDebugStringA((
					"[MainApp] Loading profile rollback failed: " +
					rollbackStatus + "\n").c_str());
			}
		}
		return hChange;
	}
	return S_OK;
}

void CMainApp::Apply_LevelRequest()
{
	LEVEL_TRANSITION_REQUEST request{};
	if (!CLevelTransitionService::Try_Consume(request))
		return;
	const uint32_t iPreviousLevel =
		CGameInstance::Get().Get_CurrentLevelID();

	if (LEVEL_TRANSITION_PHASE::LOAD == request.ePhase)
	{
		const HRESULT result = Start_Level(
			request.eTargetLevel,
			request.iLobbyCommandToken);
		if (FAILED(result))
		{
			if (INVALID_LOBBY_COMMAND_TOKEN != request.iLobbyCommandToken)
			{
				CLobbyCommandService::Cancel(
					request.iLobbyCommandToken,
					"target level loading could not start");
			}
			CLevelTransitionService::Report_LoadFailure(result);
		}
		else
		{
			CEffectPresentationService::Clear_Level(iPreviousLevel);
		}
		return;
	}

	const CLIENT_LEVEL_DESCRIPTOR* pTarget =
		CLevelRegistry::Find(request.eTargetLevel);
	const bool_t hasTargetProfile = nullptr != pTarget &&
		nullptr != pTarget->pRenderingProfileId &&
		m_RenderingProfiles.Has_Profile(pTarget->pRenderingProfileId);
	unique_ptr<CLevel> nextLevel = hasTargetProfile ?
		CLevelRegistry::Create_Level(
			request.eTargetLevel,
			m_pDevice,
			m_pContext) : nullptr;
	const string previousProfileId =
		m_RenderingProfiles.Get_ActiveProfileId();
	string profileStatus;
	const bool_t profileActivated = nullptr != nextLevel &&
		m_RenderingProfiles.Activate_Profile(
			pTarget->pRenderingProfileId, profileStatus);
	if (nullptr != nextLevel && !profileActivated)
	{
		OutputDebugStringA((
			"[MainApp] Target rendering profile activation failed: " +
			profileStatus + "\n").c_str());
	}
	if (profileActivated && SUCCEEDED(CGameInstance::Get().Change_Level(
		ETOUI(request.eTargetLevel), move(nextLevel))))
	{
		CEffectPresentationService::Clear_Level(iPreviousLevel);
		return;
	}
	if (profileActivated && !previousProfileId.empty())
	{
		string rollbackStatus;
		if (!m_RenderingProfiles.Activate_Profile(
			previousProfileId, rollbackStatus))
		{
			OutputDebugStringA((
				"[MainApp] Rendering profile rollback failed after level activation failure: " +
				rollbackStatus + "\n").c_str());
		}
	}

	if (INVALID_LOBBY_COMMAND_TOKEN != request.iLobbyCommandToken)
	{
		CLobbyCommandService::Cancel(
			request.iLobbyCommandToken,
			"target level activation failed");
	}
	CGameInstance::Get().Clear_Resources(ETOUI(request.eTargetLevel));
	CNetworkManager::Get().Close_ServerConnection();
	CLevelTransitionService::Report_LoadFailure(E_FAIL);
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY,
		"main-app.activation-failure"))
	{
		OutputDebugStringA(
			"[MainApp] Failed to stage Lobby recovery after activation failure.\n");
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
	m_pHUDLayoutTool =
		make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
	return S_OK;
}

HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)
{
	if (nullptr != m_pMapTool && DEBUG_TOOL::MAP != eTool)
		m_pMapTool->SetOpen(false);

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
		if (nullptr == m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel =
				make_shared<CCharacterPreviewPanel>();
		if (nullptr == m_pAnimationTool)
			m_pAnimationTool = make_unique<CAnimation_Tool>(
				m_pCharacterPreviewPanel);
		break;
	case DEBUG_TOOL::EFFECT:
		if (nullptr == m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel =
				make_shared<CCharacterPreviewPanel>();
		if (nullptr == m_pEffectTool)
			m_pEffectTool =
				make_unique<CEffect_Tool>(
					m_pDevice, m_pContext, m_pCharacterPreviewPanel);
		break;
	case DEBUG_TOOL::RENDERING:
		if (!m_bRenderQualityDraftInitialized)
		{
			const SCENE_RENDERING_PROFILE* pProfile =
				m_RenderingProfiles.Get_ActiveProfile();
			if (nullptr == pProfile)
				return E_FAIL;
			m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
			m_SceneRenderingDraft = *pProfile;
			m_strRenderingDraftProfileId = pProfile->strProfileId;
			m_bRenderQualityDraftInitialized = true;
		}
		break;
	case DEBUG_TOOL::UI:
		if (nullptr == m_pHUDLayoutTool)
			m_pHUDLayoutTool =
				make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
		break;
	case DEBUG_TOOL::BALANCE:
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>(
				make_shared<CNetworkPlayerCommandSink>());
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
	const bool_t isMapEditorWorkspace =
		ETOUI(LEVEL::DEVELOPMENT) == currentLevelId &&
		CMapEditorWorkspaceService::Is_Active();
	ImGui::Text("Current level id: %u", currentLevelId);
	ImGui::TextDisabled(isMapEditorWorkspace ?
		"Map Editor is active. Open Map Tool to author the selected Area." :
		"F1 only toggles tools. Enter Map Editor through Lobby Test.");
	ImGui::SeparatorText("Tools");

	const auto toolButton = [this](
		const char_t* pLabel,
		const DEBUG_TOOL eTool,
		const bool_t isEnabled)
	{
		ImGui::BeginDisabled(!isEnabled);
		if (ImGui::Button(pLabel))
		{
			m_strToolStatus = SUCCEEDED(EnsureDebugTool(eTool)) ?
				"Tool opened." : "Tool initialization failed.";
		}
		ImGui::EndDisabled();
	};

	toolButton("Map Tool", DEBUG_TOOL::MAP, isMapEditorWorkspace);
	ImGui::SameLine();
	toolButton(
		"Animation Tool",
		DEBUG_TOOL::ANIMATION,
		true);
	toolButton("Effect Tool", DEBUG_TOOL::EFFECT, true);
	ImGui::SameLine();
	toolButton("Rendering Workbench", DEBUG_TOOL::RENDERING, true);
	ImGui::SameLine();
	toolButton("HUD Layout Tool", DEBUG_TOOL::UI, true);
	ImGui::SameLine();
	toolButton("Balance Tool", DEBUG_TOOL::BALANCE, true);
	ImGui::TextWrapped("%s", m_strToolStatus.c_str());

	ImGui::SeparatorText("Diagnostics");
	const ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("FPS: %.1f  |  Frame: %.2f ms",
		io.Framerate,
		io.DeltaTime > 0.f ? io.DeltaTime * 1000.f : 0.f);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"FPS is ImGui's rolling average; Frame is the latest frame time.");
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

void CMainApp::RenderRenderingWorkbench()
{
	const SCENE_RENDERING_PROFILE* pActiveProfile =
		m_RenderingProfiles.Get_ActiveProfile();
	if (nullptr == pActiveProfile)
		return;
	if (!m_bRenderQualityDraftInitialized ||
		m_strRenderingDraftProfileId != pActiveProfile->strProfileId)
	{
		m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
		m_SceneRenderingDraft = *pActiveProfile;
		m_strRenderingDraftProfileId = pActiveProfile->strProfileId;
		m_bRenderQualityDraftInitialized = true;
	}

	if (!ImGui::Begin(
		"Rendering Workbench",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	ImGui::Text("Pipeline: legacy_deferred_v1");
	ImGui::Text("Scene profile: %s", m_strRenderingDraftProfileId.c_str());
	ImGui::Text("Viewport: %.0f x %.0f", viewportSize.x, viewportSize.y);
	ImGui::TextDisabled(
		"FP16 Light -> SceneHDR -> Screen Post -> half-res Bloom -> Hable/FXAA -> UI");
	ImGui::TextDisabled(
		"Global technical settings and scene artistic multipliers are stored separately.");

	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	ImGui::SeparatorText("Effect Presentation");
	bool_t bEffectLights = Presentation.Are_TransientLightsEnabled();
	if (ImGui::Checkbox("Typed Effect Lights", &bEffectLights))
		Presentation.Set_TransientLightsEnabled(bEffectLights);
	ImGui::SameLine();
	bool_t bEffectPosts = Presentation.Are_ScreenPostsEnabled();
	if (ImGui::Checkbox("Typed Effect Screen Posts", &bEffectPosts))
		Presentation.Set_ScreenPostsEnabled(bEffectPosts);
	ImGui::Text("Last submitted: Light %u | Screen Post %u",
		Presentation.Get_LastTransientLightCount(),
		Presentation.Get_LastScreenPostCount());
	ImGui::TextDisabled(
		"Effect Base/Mask/Dissolve/Distortion/Emissive enter SceneHDR before these posts and Bloom.");

	const auto applyGlobal = [this]()
	{
		m_RenderingProfiles.Apply_GlobalQuality(
			m_RenderQualityDraft, m_strRenderingStatus);
		m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
	};
	const auto applyScene = [this]()
	{
		m_RenderingProfiles.Apply_ActiveProfile(
			m_SceneRenderingDraft, m_strRenderingStatus);
		if (const SCENE_RENDERING_PROFILE* pProfile =
			m_RenderingProfiles.Get_ActiveProfile())
		{
			m_SceneRenderingDraft = *pProfile;
		}
	};

	bool_t globalChanged = false;
	ImGui::SeparatorText("Global Technical Quality");
	globalChanged |= ImGui::Checkbox(
		"Enabled##SSAO", &m_RenderQualityDraft.bSSAOEnabled);
	ImGui::BeginDisabled(!m_RenderQualityDraft.bSSAOEnabled);
	globalChanged |= ImGui::DragFloat(
		"SSAO Radius", &m_RenderQualityDraft.fSSAORadius,
		0.01f, 0.01f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Bias", &m_RenderQualityDraft.fSSAOBias,
		0.001f, 0.f, 1.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Intensity", &m_RenderQualityDraft.fSSAOIntensity,
		0.01f, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Power", &m_RenderQualityDraft.fSSAOPower,
		0.01f, 0.1f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Distance Fade", &m_RenderQualityDraft.fSSAODistanceFade,
		0.25f, 1.f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"SSAO darkens ambient lighting only; direct light, emissive, and Bloom remain independent.");

	ImGui::SeparatorText("Bloom / Tone / Anti-Aliasing");
	globalChanged |= ImGui::Checkbox(
		"Enabled##Bloom", &m_RenderQualityDraft.bBloomEnabled);
	ImGui::BeginDisabled(!m_RenderQualityDraft.bBloomEnabled);
	globalChanged |= ImGui::DragFloat(
		"Threshold", &m_RenderQualityDraft.fBloomThreshold,
		0.01f, 0.f, 64.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Soft Knee", &m_RenderQualityDraft.fBloomSoftKnee,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Base Bloom Intensity", &m_RenderQualityDraft.fBloomIntensity,
		0.01f, 0.f, 16.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Scatter", &m_RenderQualityDraft.fBloomScatter,
		0.01f, 0.25f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Bloom spreads pixels already above Threshold; it does not replace lighting or GI.");

	globalChanged |= ImGui::DragFloat(
		"Base Exposure", &m_RenderQualityDraft.fExposure,
		0.01f, 0.01f, 32.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Hable White Point", &m_RenderQualityDraft.fWhitePoint,
		0.05f, 1.f, 64.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Display Gamma", &m_RenderQualityDraft.fGamma,
		0.005f, 1.f, 3.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

	globalChanged |= ImGui::Checkbox(
		"FXAA Enabled", &m_RenderQualityDraft.bFXAAEnabled);
	ImGui::BeginDisabled(!m_RenderQualityDraft.bFXAAEnabled);
	globalChanged |= ImGui::DragFloat(
		"FXAA Blend", &m_RenderQualityDraft.fFXAASubpixel,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"FXAA Edge Threshold", &m_RenderQualityDraft.fFXAAEdgeThreshold,
		0.001f, 0.0312f, 0.333f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"FXAA Edge Threshold Min", &m_RenderQualityDraft.fFXAAEdgeThresholdMin,
		0.001f, 0.0156f, 0.0833f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled("FXAA is evaluated before display-space UI, so HUD text stays sharp.");

	if (globalChanged)
	{
		m_RenderQualityDraft.fSSAOBias = (std::min)(
			m_RenderQualityDraft.fSSAOBias,
			(std::max)(0.f, m_RenderQualityDraft.fSSAORadius - 0.0001f));
		m_RenderQualityDraft.fSSAODistanceFade = (std::max)(
			m_RenderQualityDraft.fSSAODistanceFade,
			m_RenderQualityDraft.fSSAORadius);
		applyGlobal();
	}

	bool_t sceneChanged = false;
	ImGui::SeparatorText("Active Scene Artistic Profile");
	sceneChanged |= ImGui::DragFloat3(
		"Light Direction", &m_SceneRenderingDraft.Light.vDirection.x,
		0.01f, -8.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat3(
		"Diffuse RGB", &m_SceneRenderingDraft.Light.vDiffuse.x,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat3(
		"Ambient RGB", &m_SceneRenderingDraft.Light.vAmbient.x,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat3(
		"Specular RGB", &m_SceneRenderingDraft.Light.vSpecular.x,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Exposure Multiplier", &m_SceneRenderingDraft.fExposureMultiplier,
		0.005f, 0.1f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Bloom Intensity Multiplier",
		&m_SceneRenderingDraft.fBloomIntensityMultiplier,
		0.005f, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::Checkbox(
		"Directional Shadow Enabled",
		&m_SceneRenderingDraft.ShadowSettings.bEnabled);
	ImGui::BeginDisabled(!m_SceneRenderingDraft.ShadowSettings.bEnabled);
	sceneChanged |= ImGui::DragFloat3(
		"Shadow Focus", &m_SceneRenderingDraft.vShadowFocus.x,
		0.1f, -100000.f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Light Distance", &m_SceneRenderingDraft.fShadowDistance,
		0.1f, 0.1f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Coverage Width",
		&m_SceneRenderingDraft.ShadowSettings.fOrthographicWidth,
		0.1f, 0.1f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Coverage Height",
		&m_SceneRenderingDraft.ShadowSettings.fOrthographicHeight,
		0.1f, 0.1f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Near", &m_SceneRenderingDraft.ShadowSettings.fNear,
		0.01f, 0.0001f, 100000.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Far", &m_SceneRenderingDraft.ShadowSettings.fFar,
		0.1f, 0.0001f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Depth Bias", &m_SceneRenderingDraft.ShadowSettings.fDepthBias,
		0.00005f, 0.f, 0.05f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Normal Bias", &m_SceneRenderingDraft.ShadowSettings.fNormalBias,
		0.001f, 0.f, 10.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Shadow Strength", &m_SceneRenderingDraft.ShadowSettings.fStrength,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Shadow uses a fixed 2048 depth map with 3x3 PCF; light eye is derived from focus and scene direction.");
	if (sceneChanged)
	{
		m_SceneRenderingDraft.Light.vDirection.w = 0.f;
		m_SceneRenderingDraft.Light.vDiffuse.w = 1.f;
		m_SceneRenderingDraft.Light.vAmbient.w = 1.f;
		m_SceneRenderingDraft.Light.vSpecular.w = 1.f;
		m_SceneRenderingDraft.ShadowSettings.fFar = (std::max)(
			m_SceneRenderingDraft.ShadowSettings.fFar,
			m_SceneRenderingDraft.ShadowSettings.fNear + 0.0001f);
		applyScene();
	}
	ImGui::TextDisabled(
		"Effective Exposure/Bloom = global base x active scene multiplier (never cumulative).");

	ImGui::SeparatorText("Global A/B Actions");
	if (ImGui::Button("Reset Global Legacy Defaults"))
	{
		m_RenderQualityDraft = {};
		m_RenderQualityDraft.bSSAOEnabled = false;
		applyGlobal();
	}
	ImGui::SameLine();
	if (ImGui::Button("Global Reference A/B Start"))
	{
		m_RenderQualityDraft = {};
		m_RenderQualityDraft.fBloomThreshold = 1.4f;
		m_RenderQualityDraft.fBloomSoftKnee = 0.45f;
		m_RenderQualityDraft.fBloomIntensity = 0.2f;
		m_RenderQualityDraft.fBloomScatter = 1.f;
		m_RenderQualityDraft.fExposure = 1.2f;
		m_RenderQualityDraft.bFXAAEnabled = true;
		applyGlobal();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload Active"))
	{
		m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
		if (const SCENE_RENDERING_PROFILE* pProfile =
			m_RenderingProfiles.Get_ActiveProfile())
		{
			m_SceneRenderingDraft = *pProfile;
			m_strRenderingDraftProfileId = pProfile->strProfileId;
		}
		m_strRenderingStatus = "Drafts reloaded from the active profile service.";
	}

	ImGui::SeparatorText("Authoring Pipeline");
	if (ImGui::Button("Save Authored"))
		m_RenderingProfiles.Save_Authored(m_strRenderingStatus);
	ImGui::SameLine();
	if (ImGui::Button("Publish Runtime"))
		m_RenderingProfiles.Publish_Runtime(m_strRenderingStatus);
	ImGui::SameLine();
	if (ImGui::Button("Reload Runtime"))
	{
		if (m_RenderingProfiles.Reload_Runtime(m_strRenderingStatus))
		{
			m_RenderQualityDraft = m_RenderingProfiles.Get_GlobalQuality();
			if (const SCENE_RENDERING_PROFILE* pProfile =
				m_RenderingProfiles.Get_ActiveProfile())
			{
				m_SceneRenderingDraft = *pProfile;
				m_strRenderingDraftProfileId = pProfile->strProfileId;
			}
		}
	}
	ImGui::TextWrapped("%s", m_strRenderingStatus.c_str());
	ImGui::TextDisabled(
		"Save changes Authored only; Publish validates/promotes Runtime; Reload commits atomically.");
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
	CEffectPresentationService::Release_PreparedResources();
	CEffectCatalog::Clear();
	CNetworkManager::Get().Shutdown();
	CGameInstance::Get().SetInputBlocked(false, false);

#ifdef _DEBUG
	if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
		pProfiler->Set_Enabled(false);
	m_pAnimationTool.reset();
	m_pEffectTool.reset();
	if (nullptr != m_pCharacterPreviewPanel)
		m_pCharacterPreviewPanel->Release(true);
	m_pCharacterPreviewPanel.reset();
	m_pHUDLayoutTool.reset();
	m_pBalanceTool.reset();
	m_pMapTool.reset();
#endif

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->Shutdown();
	m_pImGuiLayer.reset();
	CGameInstance::Get().Release_Engine();
}
