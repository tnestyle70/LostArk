#include "imgui.h"

#include "MainApp.h"

#include "CharacterSelectionState.h"
#include "ChatWindowView.h"
#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "EstherCutinPresentationService.h"
#include "Effect_Object.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "ImGuiLayer.h"
#include "ItemCatalog.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "Level_Bern.h"
#include "Level_CharacterSelect.h"
#include "Level_Loading.h"
#include "LobbyCommandService.h"
#include "NetworkManager.h"
#include "PartyWindowView.h"
#include "PlayerSkillCatalog.h"
#include "Profiler.h"
#include "Presentation_Manager.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "InventoryView.h"
#include "SkillWindowView.h"
#include "SkillGroundTargetPreview.h"
#include "SoundCueCatalog.h"
#include "UI_Sprite.h"

#ifdef _DEBUG
#include "Animation_Tool.h"
#include "BalanceTool.h"
#include "CharacterPreviewPanel.h"
#include "Effect_Tool.h"
#include "Effect_Tool_V2.h"
#include "HUDLayoutTool.h"
#include "MapEditorWorkspaceService.h"
#include "MapTool.h"
#include "NetworkPlayerCommandSink.h"
#include "ProfilerCaptureIO.h"
#include "ValtanPatternAuditionService.h"
#endif

#include <algorithm>
#include <cmath>
#include <cwchar>
#include <fstream>

namespace
{
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

	/* "12345" -> "12,345", for floating damage numbers -- not _DEBUG-gated, floating damage draws
	in Release too. */
	wstring Format_ThousandsSeparated(uint32_t iValue)
	{
		const wstring strDigits = std::to_wstring(iValue);
		wstring strResult;
		int32_t iDigitsSinceComma = 0;
		for (auto it = strDigits.rbegin(); it != strDigits.rend(); ++it)
		{
			if (0 != iDigitsSinceComma && 0 == iDigitsSinceComma % 3)
				strResult.push_back(L',');
			strResult.push_back(*it);
			++iDigitsSinceComma;
		}
		std::reverse(strResult.begin(), strResult.end());
		return strResult;
	}

	/* Shared by RenderItemUpgradeListText (all 6 left-list rows), RenderItemUpgradeLevelText (the
	big "selected item" name label), and Update_ItemUpgradeSelection (click-to-select + icon swap)
	so the row order/name/icon triple has exactly one source instead of drifting across three
	independent literals. Same placeholder "운명의 업화" set/order established earlier -- real
	inventory/equipment data isn't wired in yet. */
	struct ITEM_UPGRADE_SLOT_INFO
	{
		const wchar_t* pName;
		const char* pIconPath;
	};
	const ITEM_UPGRADE_SLOT_INFO ITEM_UPGRADE_SLOTS[6] =
	{
		{ L"\xC6B4\xBA85\xC758 \xC5C5\xD654 \xBA38\xB9AC\xC7A5\xC2DD", "UI/ItemUpgrade/lm_head_icon.png" },     // 운명의 업화 머리장식
		{ L"\xC6B4\xBA85\xC758 \xC5C5\xD654 \xC5B4\xAE68\xC7A5\xC2DD", "UI/ItemUpgrade/lm_shoulder_icon.png" }, // 운명의 업화 어깨장식
		{ L"\xC6B4\xBA85\xC758 \xC5C5\xD654 \xC0C1\xC758", "UI/ItemUpgrade/lm_top_icon.png" },                 // 운명의 업화 상의
		{ L"\xC6B4\xBA85\xC758 \xC5C5\xD654 \xD558\xC758", "UI/ItemUpgrade/lm_bottom_icon.png" },              // 운명의 업화 하의
		{ L"\xC6B4\xBA85\xC758 \xC5C5\xD654 \xC7A5\xAC11", "UI/ItemUpgrade/lm_glove_icon.png" },               // 운명의 업화 장갑
		{ L"\xC6B4\xBA85\xC758 \xC5C5\xD654 \xC2DC\xACC4", "UI/ItemUpgrade/lm_weapon_icon.png" },              // 운명의 업화 시계
	};

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
			return "Artist";
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
			return "Artist";
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

#ifdef _DEBUG
void CMainApp::Update_DebugWindowTitleWithFps(const wchar_t* pBaseTitle)
{
	if (nullptr == g_hWnd || nullptr == pBaseTitle || L'\0' == pBaseTitle[0] ||
		nullptr == ImGui::GetCurrentContext())
		return;

	constexpr size_t WINDOW_TITLE_CAPACITY = 128u;
	constexpr ULONGLONG REFRESH_INTERVAL_MS = 500ull;
	static WCHAR previousBaseTitle[WINDOW_TITLE_CAPACITY]{};
	static ULONGLONG previousRefreshTick = 0ull;

	const ULONGLONG currentTick = ::GetTickCount64();
	const bool baseTitleChanged =
		0 != std::wcscmp(previousBaseTitle, pBaseTitle);
	if (!baseTitleChanged &&
		currentTick - previousRefreshTick < REFRESH_INTERVAL_MS)
		return;

	::wcsncpy_s(
		previousBaseTitle,
		_countof(previousBaseTitle),
		pBaseTitle,
		_TRUNCATE);
	previousRefreshTick = currentTick;

	WCHAR windowTitle[WINDOW_TITLE_CAPACITY]{};
	::_snwprintf_s(
		windowTitle,
		_countof(windowTitle),
		_TRUNCATE,
		L"%ls | FPS %.1f",
		pBaseTitle,
		ImGui::GetIO().Framerate);
	::SetWindowTextW(g_hWnd, windowTitle);
}
#endif

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

	/* Was only ever loaded lazily from the F1 "Inventory (Debug)" panel's own render code, so
	CItemCatalog::Find_ById() returned nullptr for every real item (no icon, just the quantity
	text) until a player opened Developer Tools at least once. Not fatal on failure -- the debug
	panel already tolerated an empty catalog (just disables "Give"), so real gameplay should too
	rather than blocking Client startup over it. */
	std::string itemCatalogStatus;
	if (!Client::CItemCatalog::Load(itemCatalogStatus))
	{
		const std::string diagnostic =
			"[MainApp] Item Catalog initialization failed: " + itemCatalogStatus + "\n";
		OutputDebugStringA(diagnostic.c_str());
	}

	/* Not fatal, same reasoning as CItemCatalog above -- a missing/broken sound catalog just
	means CCharacter::Update_SoundCues() finds no variants for every cue and silently plays
	nothing (Client-only presentation, no gameplay authority depends on it). */
	std::string soundCatalogStatus;
	if (!Client::CSoundCueCatalog::Load(soundCatalogStatus))
	{
		const std::string diagnostic =
			"[MainApp] Sound Cue Catalog initialization failed: " + soundCatalogStatus + "\n";
		OutputDebugStringA(diagnostic.c_str());
	}

	m_pHUDRuntimeView = std::make_unique<CHUDRuntimeView>(m_pDevice, m_pContext);
	m_pBossUIView = std::make_unique<CHUDRuntimeView>(
		m_pDevice, m_pContext, L"UI/BossUI/BossUI.json");
	m_pEstherUIView = std::make_unique<CHUDRuntimeView>(
		m_pDevice, m_pContext, L"UI/Esther/EstherUI.json");
	m_pItemUpgradeView = std::make_unique<CHUDRuntimeView>(
		m_pDevice, m_pContext, L"UI/ItemUpgrade/ItemUpgradeUI.json");
	m_pLobbyBackgroundView = std::make_unique<CHUDRuntimeView>(
		m_pDevice, m_pContext, L"UI/Lobby/Lobby_Layout.json",
		CHUDRuntimeView::DRAW_TARGET::BACKGROUND);
	m_pSkillWindowView = std::make_unique<CSkillWindowView>(m_pDevice, m_pContext);
	m_pInventoryView = std::make_unique<CInventoryView>(m_pDevice, m_pContext);
	m_pChatWindowView = std::make_unique<CChatWindowView>(m_pDevice);
	m_pPartyWindowView = std::make_unique<CPartyWindowView>(m_pDevice);

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

	/* Same reasoning/gating as K above: I is a normal gameplay keybind (the inventory), not
	an F1/F6 tool-switch key. */
	if (nullptr != m_pInventoryView && !ImGui::GetIO().WantTextInput)
	{
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t iDown = windowFocused &&
			0 != (GetAsyncKeyState(0x49 /* VK_I */) & 0x8000);
		if (iDown && !m_bIDown)
			m_pInventoryView->Toggle();
		m_bIDown = iDown;
	}

	/* Same reasoning/gating as K/I above: P is a free normal gameplay keybind, not an F1/F6
	tool-switch key. Toggles the debug-only Item Upgrade static art preview (see the
	m_pItemUpgradeView declaration comment in MainApp.h). */
	if (nullptr != m_pItemUpgradeView && !ImGui::GetIO().WantTextInput)
	{
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t pDown = windowFocused &&
			0 != (GetAsyncKeyState(0x50 /* VK_P */) & 0x8000);
		if (pDown && !m_bPDown)
		{
			m_bItemUpgradePreviewVisible = !m_bItemUpgradePreviewVisible;
			if (m_bItemUpgradePreviewVisible)
			{
				/* Reopening always starts the gauge idle at 0 -- reset the state machine and hide
				the 100%-only art so a completed run from a previous open doesn't carry over. */
				m_iItemUpgradePreviousPercent = 0;
				m_bItemUpgradeGrowing = false;
				m_dItemUpgradeGrowStartSeconds = -1.0;
				m_bItemUpgradeCoreFlashPending = false;
				m_dItemUpgradeShockwaveScheduledAt = -1.0;
				m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
				m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", false);
				m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", false);
				m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
				m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
				m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", false);
			}
		}
		m_bPDown = pDown;
	}

	/* 1/2/3/4 use whatever item is registered on Item_1..4 (drag-drop from the inventory --
	see Render_ItemQuickSlots). Same gating as K/I; the Server is the one that actually
	validates ownership and applies the heal, this only ever sends the request. */
	if (!ImGui::GetIO().WantTextInput)
	{
		constexpr int VIRTUAL_KEYS[4] = { 0x31, 0x32, 0x33, 0x34 }; // VK_1..VK_4
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		for (int32_t i = 0; i < 4; ++i)
		{
			const bool_t keyDown = windowFocused &&
				0 != (GetAsyncKeyState(VIRTUAL_KEYS[i]) & 0x8000);
			if (keyDown && !m_bItemKeyDown[i] && !m_strItemQuickSlot[i].empty())
			{
				CNetworkManager::Get().Send_UseItem(
					m_iNextUseItemSequence++, m_strItemQuickSlot[i]);
			}
			m_bItemKeyDown[i] = keyDown;
		}
	}

	/* Enter opens the chat input the same way K toggles the skill window: only while nothing
	else already owns text input, so it cannot hijack an unrelated focused field. Once open,
	ImGui::GetIO().WantTextInput is true for as long as the InputText keeps focus, which both
	naturally blocks this same re-open check and (via the keyboardCaptured/SetInputBlocked
	logic below) blocks gameplay key polling while typing -- no separate plumbing needed for
	that part. Escape closes it and is checked outside the WantTextInput guard, since that is
	exactly the state Escape needs to fire in. */
	if (nullptr != m_pChatWindowView && !ImGui::GetIO().WantTextInput)
	{
		/* Same level restriction as the chat window's own Render() gate -- Enter should not open
		an input box that would render invisible outside Bern/Valtan. */
		const uint32_t chatLevel = CGameInstance::Get().Get_CurrentLevelID();
		const bool_t chatLevelAllowed =
			ETOUI(LEVEL::BERN) == chatLevel || ETOUI(LEVEL::VALTAN_ARENA) == chatLevel;
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t enterDown = chatLevelAllowed && windowFocused &&
			0 != (GetAsyncKeyState(VK_RETURN) & 0x8000);
		if (enterDown && !m_bEnterDown && !m_pChatWindowView->Is_Open())
			m_pChatWindowView->Open_Input();
		m_bEnterDown = enterDown;
	}
	if (nullptr != m_pChatWindowView && m_pChatWindowView->Is_Open())
	{
		const bool_t escapeDown =
			0 != (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
		if (escapeDown && !m_bEscapeDown)
			m_pChatWindowView->Close_Input();
		m_bEscapeDown = escapeDown;
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
#ifdef _DEBUG
	/* PLAY_PATTERN_ID has one process-wide verdict/lifecycle queue shared by
	   Balance Tool and Effect Tool. Drain it once per frame here, independent
	   of which panel is visible or which tree row is expanded. */
	CValtanPatternAuditionService::Get().Update();
#endif
	CGameInstance::Get().Update_Engine(fTimeDelta);
	CEffectPresentationService::Advance_ProductCuePreparation(
		m_pDevice, m_pContext);
	CEffectPresentationService::Commit_PendingSpawns();
	CEffectPresentationService::Synchronize_FollowAnchors();
	CEffectPresentationService::Update(fTimeDelta);

#ifdef _DEBUG
	if (nullptr != m_pMapTool)
		m_pMapTool->Update(fTimeDelta);
	if (nullptr != m_pAnimationTool)
	{
		m_pAnimationTool->Update(
			fTimeDelta,
			m_bDeveloperToolsVisible &&
			DEBUG_TOOL::ANIMATION == m_eActiveDebugTool);
	}
	if (nullptr != m_pEffectTool)
		m_pEffectTool->Update(fTimeDelta);
#endif

	// 현재 Level의 Update가 끝난 뒤에만 기존 Level을 파괴한다.
	Apply_LevelRequest();
}

HRESULT CMainApp::Render()
{
	float4_t clearColor = { 0.008f, 0.012f, 0.025f, 1.f };
	const HRESULT hBeginResult =
		CGameInstance::Get().Render_Begin(&clearColor);
	if (FAILED(hBeginResult))
	{
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return hBeginResult;
	}

	const HRESULT hWorldResult = CGameInstance::Get().Render();
	if (FAILED(hWorldResult))
	{
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return hWorldResult;
	}

	if (nullptr != m_pImGuiLayer)
	{
		if (nullptr != m_pLobbyBackgroundView &&
			ETOUI(LEVEL::LOBBY) == CGameInstance::Get().Get_CurrentLevelID())
		{
			m_pLobbyBackgroundView->Render("", 0);
			Render_LobbyButtons();
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
		RenderBossHealthBar();
		RenderEstherGauge();
		/* RenderQuickSlot only draws the extracted QuickSlot.gfx on-use flash overlay -- it does
		not draw icon art, cooldown sweep, or keybind text for any class, so it is additive on top
		of the existing icon/cooldown rendering below, not a replacement for it. Disabling these
		two calls previously took every class's skill icons off screen, not just LanceMaster's. */
		RenderSkillIcons();
		RenderSkillCooldowns();
		RenderQuickSlot();
		if (nullptr != m_pChatWindowView)
		{
			/* Only in actual in-game play (Bern/Valtan), not Character Select -- more levels join
			this list as real in-game stages are added. */
			const uint32_t chatLevel = CGameInstance::Get().Get_CurrentLevelID();
			if (ETOUI(LEVEL::BERN) == chatLevel || ETOUI(LEVEL::VALTAN_ARENA) == chatLevel)
				m_pChatWindowView->Render();
		}
		if (nullptr != m_pPartyWindowView)
		{
			/* Same level set as the chat window. UI-only placeholder roster for now (no party
			Shared protocol to gate on actual invite-accepted state yet). */
			const uint32_t partyLevel = CGameInstance::Get().Get_CurrentLevelID();
			if (ETOUI(LEVEL::BERN) == partyLevel || ETOUI(LEVEL::VALTAN_ARENA) == partyLevel)
			{
				m_pPartyWindowView->Render();
			}
		}
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
			case DEBUG_TOOL::EFFECT_V2:
				if (nullptr != m_pEffectToolV2)
					m_pEffectToolV2->Render();
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
	CEstherCutinPresentationService::Render(m_pDevice, m_pContext);
	RenderCombatHUDText();
	RenderBossHealthBarText();
	RenderDamageNumbers();
	if (nullptr != m_pInventoryView)
		m_pInventoryView->Render_Text();
	RenderQuickSlotKeyLabels();
	RenderLobbyButtonText();
	RenderItemUpgradeButtonText();
	RenderItemUpgradeLevelText();
	RenderItemUpgradeMaterialCounts();
	RenderItemUpgradeGaugePercentText();
	RenderItemUpgradeListText();
	if (ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID())
	{
		if (CLevel_CharacterSelect* pCharacterSelect = CLevel_CharacterSelect::Get_Active())
			pCharacterSelect->Render_ArenaSpawnLabels();
	}
	if (ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID())
	{
		if (CLevel_Bern* pBern = CLevel_Bern::Get_Active())
			pBern->Render_ValtanEntryModalText();
	}

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
	viewport) instead of being hidden by it like a real full-screen menu hides the HUD. */
	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();

	if (!skillWindowOpen && nullptr != m_pHUDRuntimeView)
	{
		/* Base state only for now -- no gauge/resource-driven stage switching yet. */
		const string strOwnerClass = GetHUDOwnerClassName(player.eCharacterClass);

		/* LanceMaster's identity icon is a keyframe-animated Scaleform extraction, not a static
		layer stack -- it has to be told to play, and only on an actual stance edge (the source
		asset's own stanceMc.gotoAndPlay("focus"/"wild") trigger, see LanceMasterSkinFrame.as).
		Every other class's identity art still comes from Slot.Layers and needs nothing here. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER == player.eCharacterClass &&
			player.eStance != m_ePreviousHudStance)
		{
			/* spear01 (used by the "wild" label frame) is visually the straight short spear;
			spear02 (used by "focus") is the curved glaive blade -- opposite of what the asset's
			own "spear01/spear02" filenames suggest, confirmed by actually opening both crops. */
			const char_t* pLabel =
				LostArk::Shared::PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR == player.eStance ?
					"wild" : "focus";
			m_pHUDRuntimeView->Play_KeyframeAnimation("Lance_Id_Stance", pLabel);
		}
		/* Warlord's defense-mode toggle (Z: skill 17800 WARLORD_NORMAL->WARLORD_DEFENSE, 17810
		reverses it -- confirmed in Data/Balance/PlayerSkills.json, not X) is a real extracted
		Scaleform clip too (WarLordSkinFrame::defenseMode -> defenseBody/defenseEffect
		.gotoAndPlay("on"/"off")). protectMode (X: skill 17820, requiredStance/setsStance both
		NONE) has no server-side stance field in HUD_PLAYER_STATE -- WarL_Id_ProtectBody/
		ProtectEffect stay defined but untriggered until that data exists, rather than guessing a
		fake source. */
		else if (LostArk::Shared::CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass &&
			player.eStance != m_ePreviousHudStance)
		{
			const char_t* pLabel =
				LostArk::Shared::PLAYER_STANCE_ID::WARLORD_DEFENSE == player.eStance ?
					"on" : "off";
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_DefenseBody", pLabel);
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_DefenseEffect", pLabel);
		}
		m_ePreviousHudStance = player.eStance;

		/* Warlord's gaugeL/gaugeR identity-gauge fill is a real per-percentage reveal, not a
		time-based clip: the source Scaleform mask (gaugeMask_8, symbol 730) is 100 distinct hand-
		authored vector frames traced along the badge's own hex border and jumped to directly via
		gotoAndStop(percentage) (WarLordSkinFrame::refreshGauge). GaugeL.json/GaugeR.json bake that
		real 100-frame reveal into one real composited texture per percentage and expose each frame
		under its own integer-string label ("0".."99"), so this reuses Play_KeyframeAnimation
		exactly as-is -- no separate percentage-driven playback path needed in the engine. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass &&
			player.iMaximumIdentity > 0u)
		{
			const int32_t iGaugeFrame = std::clamp(
				static_cast<int32_t>(player.iCurrentIdentity * 99u / player.iMaximumIdentity),
				0, 99);
			const string strGaugeLabel = std::to_string(iGaugeFrame);
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_GaugeR", strGaugeLabel);
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_GaugeL", strGaugeLabel);
		}

		/* Warlord's real Z/X identity-slot art (WarLordEnableEffectMc, symbol 74) includes a hex
		glow ring the source only shows while that slot's ability is off cooldown -- the AS3 is
		`enabledEffect_0.visible = !this._defenseValue && this._slotEnable0` (and the _1/protectMode
		mirror for X). eStance already gives us the exact "_defenseValue" equivalent for Z; there is
		no client-side "_protectValue" (protectMode active) field to mirror for X (same gap noted
		on WarL_Id_ProtectBody/ProtectEffect above), so the X ring only reflects real cooldown
		readiness, not a "currently protecting" exclusion. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass)
		{
			bool_t bZReady = false, bXReady = false;
			const HUD_SKILL_STATE* pZSkill = nullptr;
			const HUD_SKILL_STATE* pXSkill = nullptr;
			for (const HUD_SKILL_STATE& Skill : player.Skills)
			{
				if ("Z" == Skill.strInputSlot)
				{
					bZReady = Skill.Is_Ready(player.iServerTick);
					pZSkill = &Skill;
				}
				else if ("X" == Skill.strInputSlot)
				{
					bXReady = Skill.Is_Ready(player.iServerTick);
					pXSkill = &Skill;
				}
			}
			const bool_t bShowZRing =
				bZReady && LostArk::Shared::PLAYER_STANCE_ID::WARLORD_DEFENSE != player.eStance;
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_EnableRingZ", bShowZRing ? "on" : "off");
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_EnableRingX", bXReady ? "on" : "off");

			/* The badge itself is the real cooldown indicator (WarLoardSkillSlot's SlotState.
			SLOT_STATE_DISABLED instantly swaps depth11's child from the bright shape(589/89) to
			the dark shape(594/584) -- confirmed frame-by-frame in the source timeline, no gradual
			reveal baked into that swap). SkillZState.json/SkillXState.json hold both real frames
			under "ready"/"cooldown" labels. */
			m_pHUDRuntimeView->Play_KeyframeAnimation("Skill_Z", bZReady ? "ready" : "cooldown");
			m_pHUDRuntimeView->Play_KeyframeAnimation("Skill_X", bXReady ? "ready" : "cooldown");

			/* The real clockwise reveal itself lives in a separate overlay (WarLoardSkillSlot's
			shared "coolDown" component, symbol 328 -- 240 real hand-authored frames, confirmed by
			opening the source timeline directly) layered on top of the dark badge, not baked into
			the badge swap above. Frame index tracks elapsed cooldown fraction: the source shape
			starts at full coverage right after use and shrinks clockwise to nothing as the real
			240-frame sequence progresses, so elapsed (not remaining) maps directly to frame. */
			const auto PlayCooldownWipe = [&](const char* pSlotId, const HUD_SKILL_STATE* pSkill, bool_t bReady)
			{
				if (bReady || nullptr == pSkill || 0u == pSkill->iCooldownDurationTicks)
				{
					m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotId, "ready");
					return;
				}
				const uint32_t remainingTicks = pSkill->iCooldownEndTick > player.iServerTick ?
					pSkill->iCooldownEndTick - player.iServerTick : 0u;
				const f32_t fElapsedFraction = std::clamp(1.f -
					static_cast<f32_t>(remainingTicks) / static_cast<f32_t>(pSkill->iCooldownDurationTicks),
					0.f, 1.f);
				const int32_t iWipeFrame = std::clamp(
					static_cast<int32_t>(fElapsedFraction * 239.f), 0, 239);
				m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotId, std::to_string(iWipeFrame));
			};
			PlayCooldownWipe("WarL_Id_CooldownWipeZ", pZSkill, bZReady);
			PlayCooldownWipe("WarL_Id_CooldownWipeX", pXSkill, bXReady);
		}

		/* DimensionMaster's identity gauge is configured cyclic (Data/Balance/PlayerProfiles.json
		identityCyclic=1): it fills 0..100 and wraps back to 0 forever, rather than holding at full
		like every other class's gauge. The source's own minuteHand is driven the same way, one
		full clockwise turn per cycle -- confirmed against the real DimensionMasterSkinFrame.as
		(setMinuteHand writes straight to MovieClip.rotation, no frame-based clip involved), so
		this maps the gauge fraction straight to degrees with no guessing. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER == player.eCharacterClass &&
			player.iMaximumIdentity > 0u)
		{
			const f32_t fIdentityFraction =
				static_cast<f32_t>(player.iCurrentIdentity) / static_cast<f32_t>(player.iMaximumIdentity);
			m_pHUDRuntimeView->Set_SlotRotation("Dimen_MinuteHand", fIdentityFraction * 360.f);
		}

		/* DimensionMaster's backplate also holds 6 real independently spinning gear ornaments
		(source symbols 515/519/523/529/535/545, each wrapping its own multi-frame rotating leaf)
		-- confirmed by sampling each one's own SWF sub-timeline directly (source frameRate=40, so
		1 frame = 25ms). None of the 6 map to any server field or DimensionMasterSkinFrame setter;
		they are pure clockwork idle animation baked into the movieclip timeline itself, so they
		run on wall-clock phase and loop unconditionally regardless of identity/cooldown state.
		515/523/529 are true gears; 545 spins a full continuous 360 deg every 80 frames (measured
		~4.515 deg/frame, 4.515*80=361.2 -- confirmed against the real curve, not assumed). 519/535
		are small decorative filigree pieces, not circular gears (confirmed visually from the
		extracted art), with only a brief few-degree real sway near the end of their own cycle;
		included for completeness since real per-frame timeline data exists for both. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER == player.eCharacterClass)
		{
			const uint64_t ullNowMs = GetTickCount64();

			// gear545: continuous full spin, real curve 0 -> 360 deg over 80 frames (2000ms)
			{
				constexpr uint64_t GEAR545_CYCLE_MS = 2000ull;
				const f32_t fFrac = static_cast<f32_t>(ullNowMs % GEAR545_CYCLE_MS) / static_cast<f32_t>(GEAR545_CYCLE_MS);
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_545", fFrac * 360.f);
			}

			// gear523: real full-cycle linear sweep 0 -> -90 deg over 120 frames (3000ms), snaps
			// back to 0 at wrap (measured directly, no snap visible mid-cycle)
			{
				constexpr uint64_t GEAR523_CYCLE_MS = 3000ull;
				const f32_t fFrac = static_cast<f32_t>(ullNowMs % GEAR523_CYCLE_MS) / static_cast<f32_t>(GEAR523_CYCLE_MS);
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_523", fFrac * -90.f);
			}

			// gear529: real full-cycle linear sweep 0 -> -60 deg over 80 frames (2000ms), snaps
			// back to 0 at wrap
			{
				constexpr uint64_t GEAR529_CYCLE_MS = 2000ull;
				const f32_t fFrac = static_cast<f32_t>(ullNowMs % GEAR529_CYCLE_MS) / static_cast<f32_t>(GEAR529_CYCLE_MS);
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_529", fFrac * -60.f);
			}

			// gear515: real curve holds at 0 deg through most of the cycle, winds 0->60 deg late
			// (frac 0.625~0.8, matching source frames50-64 of 80), then holds at 60 deg until
			// wrap -- measured directly from the source timeline, not a plain sawtooth.
			{
				constexpr uint64_t GEAR515_CYCLE_MS = 2000ull;
				const f32_t fFrac = static_cast<f32_t>(ullNowMs % GEAR515_CYCLE_MS) / static_cast<f32_t>(GEAR515_CYCLE_MS);
				f32_t fDegrees = 0.f;
				if (fFrac >= 0.8f)
					fDegrees = 60.f;
				else if (fFrac >= 0.625f)
					fDegrees = 60.f * (fFrac - 0.625f) / 0.175f;
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_515", fDegrees);
			}

			// gear519: real brief symmetric sway 0->7.5->0 deg near frac 0.80~0.8625 of its
			// 80-frame (2000ms) cycle, still the rest of the time
			{
				constexpr uint64_t GEAR519_CYCLE_MS = 2000ull;
				const f32_t fFrac = static_cast<f32_t>(ullNowMs % GEAR519_CYCLE_MS) / static_cast<f32_t>(GEAR519_CYCLE_MS);
				f32_t fDegrees = 0.f;
				if (fFrac >= 0.80f && fFrac < 0.825f)
					fDegrees = 7.5f * (fFrac - 0.80f) / 0.025f;
				else if (fFrac >= 0.825f && fFrac < 0.8625f)
					fDegrees = 7.5f * (1.f - (fFrac - 0.825f) / 0.0375f);
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_519", fDegrees);
			}

			// gear535: real down-then-up swing (0 -> -8.984 -> 6.015 -> 0 deg) near the end of
			// its 95-frame (2375ms) cycle, still the rest of the time
			{
				constexpr uint64_t GEAR535_CYCLE_MS = 2375ull;
				const f32_t fFrac = static_cast<f32_t>(ullNowMs % GEAR535_CYCLE_MS) / static_cast<f32_t>(GEAR535_CYCLE_MS);
				f32_t fDegrees = 0.f;
				if (fFrac >= 0.7368f && fFrac < 0.9053f)
					fDegrees = -8.984f * (fFrac - 0.7368f) / (0.9053f - 0.7368f);
				else if (fFrac >= 0.9053f && fFrac < 0.9474f)
					fDegrees = -8.984f + (6.015f - -8.984f) * (fFrac - 0.9053f) / (0.9474f - 0.9053f);
				else if (fFrac >= 0.9474f && fFrac < 0.9895f)
					fDegrees = 6.015f * (1.f - (fFrac - 0.9474f) / (0.9895f - 0.9474f));
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_535", fDegrees);
			}
		}

		/* stanceMc's own real timeline (frame labels bubble_0/1/2/3) plays a small idle pulse
		(depth4/8 pieces breathing/flashing) continuously regardless of gauge state -- baked here
		as a real 27-frame loop (BrushIdle.json) so Yi_id_brush is never a single static frame.
		Play_KeyframeAnimation only needs to run once; the engine's own loop wraparound
		(HUDRuntimeView.cpp) keeps it playing after that. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass)
		{
			static bool_t bBrushLoopStarted = false;
			if (!bBrushLoopStarted)
			{
				m_pHUDRuntimeView->Play_KeyframeAnimation("Yi_id_brush", "idle");
				bBrushLoopStarted = true;
			}
		}

		/* Artist's (yinyangshi) real 0..100 identity gauge (identityCyclic=0, holds at max --
		Server has no separate bubble/stance counter, confirmed absent from PLAYER_STANCE_ID and
		PLAYER_SNAPSHOT) is split into 3 equal thirds on the client to drive the 3 real
		positionGuide slots. The current third plays the real per-percentage gauge ring
		(coolDown_ArtistGauge, "0".."99"); thirds not yet reached stay empty. A third that just
		completed plays the real one-shot bubble-fill sprite (artistBubbleEffect, real playback
		order i56/i50/i51/i52/i53/i54/i55) exactly once on the edge into "filled" -- Play_
		KeyframeAnimation() restarts its window on every call, so re-triggering it every frame
		would never let the sprite finish -- then the engine's own "hold on last frame" behavior
		(HUDRuntimeView.h) settles on that window's final key, which is the real persistent glow
		(shape469) baked in as the sprite's last frame, matching the real play-once-then-stay-lit
		behavior. This reads only the existing real iCurrentIdentity/iMaximumIdentity fields -- no
		new server field. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass &&
			player.iMaximumIdentity > 0u)
		{
			const f32_t fFraction = static_cast<f32_t>(player.iCurrentIdentity) /
				static_cast<f32_t>(player.iMaximumIdentity);
			const f32_t fScaled = std::clamp(fFraction, 0.f, 1.f) * 3.f;
			/* iCompletedCount is how many of the 3 slots are fully done -- 0..3, NOT clamped to 2,
			so fFraction==1.0 (fScaled==3.0) correctly marks all 3 slots complete instead of
			leaving the 3rd stuck showing a 99%-full ring forever (the previous std::min(2, ...)
			clamp made "i < iCurrentSegment" impossible to satisfy for i==2). */
			const int32_t iCompletedCount = std::min(3, static_cast<int32_t>(fScaled));
			const f32_t fSegmentFraction = fScaled - static_cast<f32_t>(iCompletedCount);

			const char* pSlotIds[3] = { "Yi_Id_GaugeRing", "Yi_Id_GaugeSlot_2", "Yi_Id_GaugeSlot_3" };
			static bool_t bPopped[3] = { false, false, false };
			for (int32_t i = 0; i < 3; ++i)
			{
				if (i < iCompletedCount)
				{
					if (!bPopped[i])
					{
						m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotIds[i], "pop");
						bPopped[i] = true;
					}
				}
				else if (i > iCompletedCount)
				{
					bPopped[i] = false;
					m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotIds[i], "empty");
				}
				else
				{
					bPopped[i] = false;
					const int32_t iGaugeFrame = std::clamp(
						static_cast<int32_t>(fSegmentFraction * 99.f), 0, 99);
					m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotIds[i], std::to_string(iGaugeFrame));
				}
			}
		}

		/* Artist's Z/X identity slots are real ARKNewSlot instances (yinYangShiSlot, symbol 343,
		shared by both) -- confirmed real skills (Z=31050, X=31110 in PlayerSkills.json) reacting
		through the exact same generic mechanism as every other skill slot in the game: keyBind
		text/icon dims to 30% brightness when not ready (ARKNewSlot.activate's real
		ColorTransform(0.3,0.3,0.3,1)), and a real 238-frame coolDown wipe (symbol 334, same
		per-frame vector mask technique as Warlord's 240-frame wipe) sweeps over the icon. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass)
		{
			bool_t bZReady = false, bXReady = false;
			const HUD_SKILL_STATE* pZSkill = nullptr;
			const HUD_SKILL_STATE* pXSkill = nullptr;
			for (const HUD_SKILL_STATE& Skill : player.Skills)
			{
				if ("Z" == Skill.strInputSlot)
				{
					bZReady = Skill.Is_Ready(player.iServerTick);
					pZSkill = &Skill;
				}
				else if ("X" == Skill.strInputSlot)
				{
					bXReady = Skill.Is_Ready(player.iServerTick);
					pXSkill = &Skill;
				}
			}
			m_pHUDRuntimeView->Play_KeyframeAnimation("Yin_Skill_Z", bZReady ? "ready" : "cooldown");
			m_pHUDRuntimeView->Play_KeyframeAnimation("Yin_Skill_X", bXReady ? "ready" : "cooldown");

			const auto PlayZXWipe = [&](const char* pSlotId, const HUD_SKILL_STATE* pSkill, bool_t bReady)
			{
				if (bReady || nullptr == pSkill || 0u == pSkill->iCooldownDurationTicks)
				{
					m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotId, "ready");
					return;
				}
				const uint32_t remainingTicks = pSkill->iCooldownEndTick > player.iServerTick ?
					pSkill->iCooldownEndTick - player.iServerTick : 0u;
				const f32_t fElapsedFraction = std::clamp(1.f -
					static_cast<f32_t>(remainingTicks) / static_cast<f32_t>(pSkill->iCooldownDurationTicks),
					0.f, 1.f);
				const int32_t iWipeFrame = std::clamp(
					static_cast<int32_t>(fElapsedFraction * 237.f), 0, 237);
				m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotId, std::to_string(iWipeFrame));
			};
			PlayZXWipe("Yin_Skill_Z_Wipe", pZSkill, bZReady);
			PlayZXWipe("Yin_Skill_X_Wipe", pXSkill, bXReady);
		}
		m_pHUDRuntimeView->Render(strOwnerClass, 0);
		RenderPlayerHealthManaBar();
		/* Static Esther slots (portraits/frame/lock/track) draw generically here. GaugeFill and the
		3 Ready glows are also authored as ordinary Tool-placeable slots (so they show up on the
		canvas for placement), but their real gameplay visibility is gauge-state-driven, not
		always-on -- force them hidden here and let RenderEstherGauge() (called later) draw the real
		clipped fill / conditional glow instead, so there's no double-draw. Esther is a Valtan raid
		mechanic -- RenderEstherGauge() already skips outside VALTAN_ARENA, but this static frame is
		a separate draw call reached by RenderCombatHUD's own broader level gate (which includes
		CHARACTER_SELECT for HP/mana/skill icons), so it needs the same VALTAN_ARENA-only check or
		the empty portrait frame keeps showing there. */
		if (nullptr != m_pEstherUIView &&
			ETOUI(LEVEL::VALTAN_ARENA) == currentLevel)
		{
			m_pEstherUIView->Set_SlotVisible("Esther_GaugeFill", false);
			m_pEstherUIView->Set_SlotVisible("Esther_Slot1_Ready", false);
			m_pEstherUIView->Set_SlotVisible("Esther_Slot2_Ready", false);
			m_pEstherUIView->Set_SlotVisible("Esther_Slot3_Ready", false);
			m_pEstherUIView->Render("Default", 0);
		}
	}

	/* Real gauge0/1/2 fill (target-rotation-masked track) and burn flourish are baked and wired;
	the 3 segments' screen position (Lance_Id_GaugeBg/Fill/Burn0/1/2 rect in HUD_Layout.json) is
	still a placeholder shared with Lance_Id_Stance's own rect -- needs live in-game tuning to
	place left/bottom/right segments at their real offsets. */
	if (LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER == player.eCharacterClass)
		RenderLanceMasterIdentityGauge();

	if (nullptr != m_pSkillWindowView)
		m_pSkillWindowView->Render(player.eCharacterClass);
	if (nullptr != m_pInventoryView)
		m_pInventoryView->Render(CCombatHUDViewModel::Get().Get_Inventory().Items);
	/* P-toggled static preview of the real traced ItemUpgradeUI.json art/positions -- see the
	m_pItemUpgradeView declaration comment in MainApp.h. No per-slot gameplay logic (no real
	Server 재련 data exists yet), just the same generic Render("Default", 0) pass Esther/Boss UI
	use for their own static slots. */
	if (nullptr != m_pItemUpgradeView && m_bItemUpgradePreviewVisible)
	{
		m_pItemUpgradeView->Render("Default", 0);
		Update_ItemUpgradeSelection();
		Update_ItemUpgradeGrowButton();

		/* No real Server 재련 percent exists yet (see comment above), so the gauge is a manual
		state machine driven by ItemUpgrade_LevelUpBtn's click (Update_ItemUpgradeGrowButton) instead
		of a free-running clock. Idle at 0 until clicked; 0->100 fill plays once per click; holds at
		100 until the next click. Once a real gauge value exists this should read it the same way
		RenderLanceMasterIdentityGauge() reads player.iCurrentIdentity, not this state. */
		if (m_bItemUpgradeGrowing)
		{
			constexpr f32_t GAUGE_FILL_FPS = 45.f;
			constexpr f32_t GAUGE_FILL_FRAME_COUNT = 100.f;
			const f32_t fCycleSeconds = GAUGE_FILL_FRAME_COUNT / GAUGE_FILL_FPS;
			const f64_t fElapsed = ImGui::GetTime() - m_dItemUpgradeGrowStartSeconds;
			const int32_t iPercent = std::clamp(
				static_cast<int32_t>(fElapsed / fCycleSeconds * GAUGE_FILL_FRAME_COUNT),
				0, 100);

			if (100 <= iPercent)
			{
				m_iItemUpgradePreviousPercent = 100;
				m_bItemUpgradeGrowing = false;
				/* Alpha starts at 0 here -- Set_SlotVisible only lifts bForceHidden (both slots
				become drawable this same frame), the actual reveal is the fade-in progress block
				below, driven by m_dItemUpgradeCompleteRevealStartSeconds. */
				m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", true);
				m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", true);
				m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
				m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
				m_dItemUpgradeCompleteRevealStartSeconds = ImGui::GetTime();
				m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", true);
				m_pItemUpgradeView->Restart_Animation("ItemUpgrade_CompleteEffect");
			}
			else
			{
				m_iItemUpgradePreviousPercent = iPercent;
				/* GaugeFill's own AnimationFrames clock starts independently on this slot's first
				Render() call, so it would otherwise drift out of phase with iPercent (which is
				anchored to m_dItemUpgradeGrowStartSeconds) -- pin it to the exact frame every tick
				instead of letting the two clocks disagree about what "63%" looks like. */
				m_pItemUpgradeView->Set_Animation_Frame("ItemUpgrade_GaugeFill", iPercent);
			}
		}
		else if (100 == m_iItemUpgradePreviousPercent)
		{
			/* Held-100 state: nothing re-pins this once growing flips false above, so without this
			GaugeFill's own looping AnimationFrames clock (JSON loop=true, 100 frames/45fps) would
			free-run straight past frame 99 and repeat the whole 0->100 sweep visually even though
			m_iItemUpgradePreviousPercent correctly stays at 100. */
			m_pItemUpgradeView->Set_Animation_Frame("ItemUpgrade_GaugeFill", 99);

			if (m_dItemUpgradeCompleteRevealStartSeconds >= 0.0)
			{
				constexpr f64_t REVEAL_FADE_SECONDS = 0.45;
				const f32_t fFadeAlpha = static_cast<f32_t>(std::clamp(
					(ImGui::GetTime() - m_dItemUpgradeCompleteRevealStartSeconds) / REVEAL_FADE_SECONDS,
					0.0, 1.0));
				m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", fFadeAlpha);
				m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", fFadeAlpha);
			}
		}

		/* CoreFlash fires exactly once, the same frame Update_ItemUpgradeGrowButton starts a fill;
		ShockwaveRing is scheduled for CoreFlash's own real duration (28 frames/20fps) later so the
		two real Scaleform layers play in their authored order instead of together (real ordering:
		coreLevelEffect1 before compF_shockwave_red inside levelUpMotion_mc). */
		if (m_bItemUpgradeCoreFlashPending)
		{
			m_pItemUpgradeView->Restart_Animation("ItemUpgrade_CoreFlash");
			constexpr f64_t CORE_FLASH_DURATION_SECONDS = 28.0 / 20.0;
			m_dItemUpgradeShockwaveScheduledAt = ImGui::GetTime() + CORE_FLASH_DURATION_SECONDS;
			m_bItemUpgradeCoreFlashPending = false;
		}
		if (m_dItemUpgradeShockwaveScheduledAt >= 0.0 &&
			ImGui::GetTime() >= m_dItemUpgradeShockwaveScheduledAt)
		{
			m_pItemUpgradeView->Restart_Animation("ItemUpgrade_ShockwaveRing");
			m_dItemUpgradeShockwaveScheduledAt = -1.0;
		}

		/* Idle (not growing, held at 0) is the only state SmeltGlow's own JSON loop should be
		visible in -- hidden for the rest of the fill and at 100% so it doesn't glow underneath the
		completion art. WingedRingGold/LevelUpMotion2Big/CompleteEffect are the inverse: hidden
		everywhere except the held-100 state set above. A fresh click (Update_ItemUpgradeGrowButton)
		re-hides all three the same frame it restarts the fill from 0. */
		const bool_t bIdle = !m_bItemUpgradeGrowing && 0 == m_iItemUpgradePreviousPercent;
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SmeltGlow", bIdle);
		/* Idle has no per-frame branch above to pin this itself (unlike the growing/held-100
		cases), so GaugeFill's own clock would otherwise free-run through its looping AnimationFrames
		while sitting idle at 0%. */
		if (bIdle)
			m_pItemUpgradeView->Set_Animation_Frame("ItemUpgrade_GaugeFill", 0);

		/* Drawing the percent number itself is deferred to RenderItemUpgradeGaugePercentText()
		(called after CImGuiLayer::EndFrame(), same reason as RenderBossHealthBar's text split --
		this Render() call's own AnimationFrames images composite later inside EndFrame() and would
		otherwise paint over a Draw_Text() submitted here). m_iItemUpgradePreviousPercent is already
		updated above and doubles as that function's read of "current percent". */
	}
	Render_ItemQuickSlots();
}

void CMainApp::RenderQuickSlotKeyLabels()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}
	const HUD_PLAYER_STATE& keyLabelPlayer = CCombatHUDViewModel::Get().Get_Player();
	if (!keyLabelPlayer.isValid)
		return;
	if (nullptr == m_pHUDRuntimeView)
		return;

	struct KEY_LABEL { const char* pSlotId; const wchar_t* pLabel; };
	constexpr KEY_LABEL LABELS[] = {
		{ "Skill_Q", L"Q" }, { "Skill_W", L"W" }, { "Skill_E", L"E" }, { "Skill_R", L"R" },
		{ "Skill_A", L"A" }, { "Skill_S", L"S" }, { "Skill_D", L"D" }, { "Skill_F", L"F" },
		{ "Skill_T", L"T" }, { "Skill_V", L"V" },
		{ "SpecialSkill_1", L"6" }, { "SpecialSkill_2", L"7" }, { "SpecialSkill_3", L"8" },
		{ "SpecialSkill_4", L"9" }, { "SpecialSkill_5", L"0" },
		{ "Item_1", L"1" }, { "Item_2", L"2" }, { "Item_3", L"3" }, { "Item_4", L"4" },
	};

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto DrawKeyLabel = [&](const char* pSlotId, const wchar_t* pLabel)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
			return;

		/* "Empty Slot.png"/"Empty Slot 2.png" are both 52x52 art with a solid pointed tab
		filling roughly the bottom 30% (measured directly from the source pixels: transparent
		above y~35, solid by y~38 of 52) -- this places the label centred in that tab instead
		of the slot's own centre. */
		const f32_t fLabelCenterX = fX + fWidth * 0.5f;
		const f32_t fLabelCenterY = fY + fHeight * 0.87f;

		/* YoonGasiIIM measured objectively bolder than YG760 (glyph opacity ratio 0.427 vs
		0.407 for '8', 0.336 vs 0.306 for 'Q') -- same font already used for combat damage
		numbers because it needs to read clearly at a glance too. */
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScale = (vMeasured.y > 0.f) ?
			(fHeight * 0.22f / vMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
			float2_t(fLabelCenterX * textScaleX, fLabelCenterY * textScaleY),
			Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	for (const KEY_LABEL& Label : LABELS)
		DrawKeyLabel(Label.pSlotId, Label.pLabel);

	/* Artist's Z ("저무는 달") and Warlord's X/Z ("전장의 방패"/"방어 태세 전환") are not drawn
	here. The only "Skill_Z" slot in HUD_Layout.json is Warlord-owned, KEYFRAME_ANIMATION type
	with a placeholder 1x1 rect (its real on-screen size/position lives in the keyframe document
	SkillZState.json, not this JSON's rect) -- DrawKeyLabel's slot-width-based centering math
	doesn't apply to it, so both the Artist and Warlord cases produced a mispositioned label.
	Skip until there's a real anchor to read (either from the keyframe document's own bounds, or
	a dedicated non-keyframe slot). */
}

void CMainApp::Render_LobbyButtons()
{
	if (nullptr == m_pLobbyBackgroundView)
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr == pViewport)
		return;
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pLobbyBackgroundView->Get_SlotRect(
		"Lobby_CreateCharacterButton", fX, fY, fWidth, fHeight))
	{
		return;
	}

	const ImVec2 vMin(
		pViewport->WorkPos.x + fX * scaleX, pViewport->WorkPos.y + fY * scaleY);
	const ImVec2 vMax(vMin.x + fWidth * scaleX, vMin.y + fHeight * scaleY);
	const ImVec2 vMouse = ImGui::GetMousePos();
	const bool_t bHovered = vMouse.x >= vMin.x && vMouse.x < vMax.x &&
		vMouse.y >= vMin.y && vMouse.y < vMax.y;

	if (bHovered)
	{
		/* Base idle art is already drawn by the generic m_pLobbyBackgroundView->Render("", 0)
		pass above; this draws the real hover art directly on top of it (same rect, fully opaque),
		matching how CInventoryView::Render_CategoryTabs swaps hover art manually. */
		if (ID3D11ShaderResourceView* pHoverSRV = m_pLobbyBackgroundView->Load_Texture(
			"UI/Lobby/create_character_button_hover.png"))
		{
			ImGui::GetForegroundDrawList(pViewport)->AddImage(pHoverSRV, vMin, vMax);
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			CLobbyCommandService::Request(LOBBY_STAGE::CHARACTER_SELECT);
	}
}

void CMainApp::RenderLobbyButtonText()
{
	if (ETOUI(LEVEL::LOBBY) != CGameInstance::Get().Get_CurrentLevelID())
		return;
	if (nullptr == m_pLobbyBackgroundView)
		return;

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pLobbyBackgroundView->Get_SlotRect(
		"Lobby_CreateCharacterButton", fX, fY, fWidth, fHeight))
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const f32_t fCenterX = fX + fWidth * 0.5f;
	const f32_t fCenterY = fY + fHeight * 0.5f;

	/* Font_YoonGasiIIM -- same real LostArk source font already used for the inventory title and
	HUD key labels (see RenderQuickSlotKeyLabels), picked for real readability over YG760.
	Written as \x-escaped UTF-16 code units (not raw UTF-8 source bytes) -- this file has no BOM,
	so MSVC's source-encoding fallback mangles literal non-ASCII bytes in string literals into
	garbage code points at compile time (matches InventoryView.cpp's existing "\xC18C\xC9C0\xD488"
	pattern for "소지품"). */
	const wchar_t* pLabel = L"\xCE90\xB9AD\xD130 \xC0DD\xC131"; // "캐릭터 생성"
	const float2_t vMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
	/* 6-glyph label on a narrow 142px button -- height-only scaling (like the single-glyph HUD
	key labels) ran the text past the button's own edges, so this also caps by width. */
	const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * 0.32f / vMeasured.y) : 1.f;
	const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.8f / vMeasured.x) : 1.f;
	const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
		float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
		Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
}

void CMainApp::RenderItemUpgradeButtonText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible)
		return;

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect(
		"ItemUpgrade_ReforgeButton", fX, fY, fWidth, fHeight))
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const f32_t fCenterX = fX + fWidth * 0.5f;
	const f32_t fCenterY = fY + fHeight * 0.5f;

	const wchar_t* pLabel = L"\xC7A5\xBE44 \xC7AC\xB828"; // "장비 재련"
	const float2_t vMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
	const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * 0.32f / vMeasured.y) : 1.f;
	const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.8f / vMeasured.x) : 1.f;
	const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
		float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
		Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);

	// "성장" label for ItemUpgrade_LevelUpBtn, same pattern as the button above.
	f32_t fGrowX = 0.f, fGrowY = 0.f, fGrowWidth = 0.f, fGrowHeight = 0.f;
	if (m_pItemUpgradeView->Get_SlotRect(
		"ItemUpgrade_LevelUpBtn", fGrowX, fGrowY, fGrowWidth, fGrowHeight))
	{
		const wchar_t* pGrowLabel = L"\xC131\xC7A5"; // "성장"
		const float2_t vGrowMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pGrowLabel);
		const f32_t fGrowScaleByHeight = (vGrowMeasured.y > 0.f) ? (fGrowHeight * 0.35f / vGrowMeasured.y) : 1.f; // 0.5 * 0.7
		const f32_t fGrowScaleByWidth = (vGrowMeasured.x > 0.f) ? (fGrowWidth * 0.8f / vGrowMeasured.x) : 1.f;
		const f32_t fGrowScale = (std::min)(fGrowScaleByHeight, fGrowScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pGrowLabel,
			float2_t((fGrowX + fGrowWidth * 0.5f) * textScaleX, (fGrowY + fGrowHeight * 0.5f) * textScaleY),
			Colors::White, 0.f, float2_t(0.5f, 0.5f), fGrowScale * textUiScale);
	}
}

void CMainApp::RenderItemUpgradeListText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	auto DrawFit = [&](const char* pSlotId, const wchar_t* pLabel, f32_t fHeightRatio, fvector_t vColor)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
			return;

		const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * fHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
			float2_t(fX * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
			vColor, 0.f, float2_t(0.f, 0.5f), fScale * textUiScale);
	};

	/* Left equipment list: level + item name per row, real slot/text field names
	(buildUpGrade_txt / itemName_txt) confirmed in ItemBuildUpListRendererMc's own trace, though
	this renderer is instantiated per-row purely by AS3 (no static per-row placement to trace an
	exact position from), so the anchor rects here are a reasonable icon-relative placement for
	the user to nudge in the Tool rather than an exact traced position. Placeholder item data
	(same "운명의 업화" set, level 18) until real inventory/equipment data is wired in. */
	// real sampled reference pixels (list row "18 단계" / item name text):
	// level = (255,189,74) same gold as curLevel_lb; name = (227,199,161) warm cream, not white.
	const fvector_t vLevelColor = XMVectorSet(1.0f, 0.7412f, 0.2902f, 1.f); // #FFBD4A
	const fvector_t vNameColor = XMVectorSet(0.8902f, 0.7804f, 0.6314f, 1.f); // #E3C7A1
	for (int32_t i = 0; i < 6; ++i)
	{
		const string strLevelSlot = "ItemUpgrade_ListLevel" + to_string(i);
		const string strNameSlot = "ItemUpgrade_ListItemName" + to_string(i);
		DrawFit(strLevelSlot.c_str(), L"18\xB2E8\xACC4", 0.765f, vLevelColor); // "18단계" (0.85 * 0.9)
		DrawFit(strNameSlot.c_str(), ITEM_UPGRADE_SLOTS[i].pName, 0.72f, vNameColor); // 0.8 * 0.9
	}

	/* Right 재련 단계 list: 7 rows now (JSON grew GradeRowEmblem/GradeStripB/GradeRowText from 4 to
	7, evenly filling the panel from its top edge down) -- the ask was more row slots, not more
	stat lines per row, so this stays at 1 stat line ("공격력 +N") like before. Real reference
	scrolls higher levels at the TOP and the current level at the BOTTOM (24단계...18단계 reading
	top to bottom, i.e. numbers increase bottom -> top), so row 0 (topmost) is the highest of our
	seven and row 6 (bottom, nearest the gauge) is 19 -- the level actually being reforged toward.
	GradeSelectedExample sits on row 6 to match. Non-selected rows sample as a muted gray (real
	24/23/22단계 rows, (103,103,103)); the selected (현재/19단계) row uses gold for the level and
	white for its stat, matching every other "selected" element in this window reading brighter
	than its neighbors. Stat is a placeholder "공격력 +N" (N = that row's own level) until real
	per-level balance data exists. */
	const fvector_t vRowGray = XMVectorSet(0.4039f, 0.4039f, 0.4039f, 1.f); // #676767
	const fvector_t vSelectedGold = XMVectorSet(1.0f, 0.7412f, 0.2902f, 1.f); // #FFBD4A
	const int32_t ROW_COUNT = 7;
	const int32_t ROW_LEVELS[ROW_COUNT] = { 25, 24, 23, 22, 21, 20, 19 };
	for (int32_t i = 0; i < ROW_COUNT; ++i)
	{
		const string strSlot = "ItemUpgrade_GradeRowText" + to_string(i);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(strSlot.c_str(), fX, fY, fWidth, fHeight))
			continue;

		const bool_t bSelected = (19 == ROW_LEVELS[i]);
		const fvector_t vNumColor = bSelected ? vSelectedGold : vRowGray;
		const fvector_t vStatTextColor = bSelected ? Colors::White : vRowGray;

		const wstring strNum = to_wstring(ROW_LEVELS[i]);
		const wchar_t* pDanggye = L"\xB2E8\xACC4"; // "단계"
		const wstring strStat = L"\xACF5\xACA9\xB825 +" + to_wstring(ROW_LEVELS[i]); // "공격력 +N"

		const float2_t vNumMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strNum.c_str());
		const f32_t fNumScale = (std::min)(
			(vNumMeasured.y > 0.f) ? fHeight * 0.24f / vNumMeasured.y : 1.f,
			(vNumMeasured.x > 0.f) ? fWidth * 0.3f / vNumMeasured.x : 1.f);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strNum.c_str(),
			float2_t((fX + fWidth * 0.15f) * textScaleX, (fY + fHeight * 0.35f) * textScaleY),
			vNumColor, 0.f, float2_t(0.5f, 0.5f), fNumScale * textUiScale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pDanggye,
			float2_t((fX + fWidth * 0.15f) * textScaleX, (fY + fHeight * 0.65f) * textScaleY),
			vNumColor, 0.f, float2_t(0.5f, 0.5f), fNumScale * textUiScale);

		const float2_t vStatMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strStat.c_str());
		const f32_t fStatScale = (vStatMeasured.y > 0.f) ? (fHeight * 0.18f / vStatMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strStat.c_str(),
			float2_t((fX + fWidth * 0.35f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
			vStatTextColor, 0.f, float2_t(0.f, 0.5f), fStatScale * textUiScale);
	}
}

void CMainApp::Update_ItemUpgradeSelection()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible)
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr == pViewport)
		return;
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;

	f32_t fListX = 0.f, fListY = 0.f, fListWidth = 0.f, fListHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect(
		"ItemUpgrade_LeftListBg", fListX, fListY, fListWidth, fListHeight))
	{
		return;
	}

	const ImVec2 vMouse = ImGui::GetMousePos();
	const bool_t bClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

	for (int32_t i = 0; i < 6; ++i)
	{
		const string strGradeBgSlot = "ItemUpgrade_ListGradeBg" + to_string(i);
		f32_t fRowX = 0.f, fRowY = 0.f, fRowWidth = 0.f, fRowHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(
			strGradeBgSlot.c_str(), fRowX, fRowY, fRowWidth, fRowHeight))
		{
			continue;
		}

		/* Full row width (LeftListBg's own x/width), not just the icon/grade-glow's own narrower
		rect, so clicking anywhere across the name text also selects this row. */
		const ImVec2 vMin(
			pViewport->WorkPos.x + fListX * scaleX,
			pViewport->WorkPos.y + fRowY * scaleY);
		const ImVec2 vMax(
			vMin.x + fListWidth * scaleX,
			vMin.y + fRowHeight * scaleY);
		const bool_t bHovered = vMouse.x >= vMin.x && vMouse.x < vMax.x &&
			vMouse.y >= vMin.y && vMouse.y < vMax.y;
		if (!bHovered || !bClicked)
			continue;

		m_iItemUpgradeSelectedSlot = i;

		f32_t fTargetX = 0.f, fTargetY = 0.f, fTargetWidth = 0.f, fTargetHeight = 0.f;
		if (m_pItemUpgradeView->Get_SlotRect(
			strGradeBgSlot.c_str(), fTargetX, fTargetY, fTargetWidth, fTargetHeight))
		{
			m_pItemUpgradeView->Set_SlotPosition(
				"ItemUpgrade_ListSelectedExample", fTargetX, fTargetY);
		}
		m_pItemUpgradeView->Set_SlotTexture(
			"ItemUpgrade_SelectedItemIcon", ITEM_UPGRADE_SLOTS[i].pIconPath);
		break;
	}
}

void CMainApp::Update_ItemUpgradeGrowButton()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible)
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr == pViewport)
		return;
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_LevelUpBtn", fX, fY, fWidth, fHeight))
		return;

	const ImVec2 vMin(pViewport->WorkPos.x + fX * scaleX, pViewport->WorkPos.y + fY * scaleY);
	const ImVec2 vMax(vMin.x + fWidth * scaleX, vMin.y + fHeight * scaleY);
	const ImVec2 vMouse = ImGui::GetMousePos();
	const bool_t bHovered = vMouse.x >= vMin.x && vMouse.x < vMax.x &&
		vMouse.y >= vMin.y && vMouse.y < vMax.y;
	if (!bHovered || !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		return;

	/* (Re)starts the fill from 0 even if a previous run already completed -- a debug preview
	button is expected to be repeatable. Hides the 100%-only art immediately so a re-click during
	the held-100 state doesn't leave it showing through the new fill. */
	m_iItemUpgradePreviousPercent = 0;
	m_bItemUpgradeGrowing = true;
	m_dItemUpgradeGrowStartSeconds = ImGui::GetTime();
	m_bItemUpgradeCoreFlashPending = true;
	m_dItemUpgradeShockwaveScheduledAt = -1.0;
	m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", false);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", false);
}

void CMainApp::RenderItemUpgradeLevelText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	struct LEVEL_TEXT_SLOT
	{
		const char* pSlotId;
		const wchar_t* pLabel;
		f32_t fHeightRatio;
		fvector_t vColor;
	};
	// colors are the real ItemBuildUpLevelWndContent/ItemBuildUpLevelGroupMc .as
	// itemName_lb.color=13769983(0xD21CFF), curLevel_lb.color=16760138(0xFFBD4A),
	// nextLevel_lb.color=12057344(0xB7FB00). The ">>>" arrow is a real animated
	// flourish icon (ItemUpgrade_LevelArrow AnimationFrames), not text.
	// "운명의 업화 상의" -- same legend-grade gold as buildup_gradebg_legend.png. The item name
	// itself tracks m_iItemUpgradeSelectedSlot (Update_ItemUpgradeSelection), so it can't live in
	// the fixed-literal SLOTS array below like CurLevel/NextLevel.
	const int32_t iSelectedSlot = std::clamp(m_iItemUpgradeSelectedSlot, 0, 5);
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_ItemNameLabel", fX, fY, fWidth, fHeight))
		{
			const wchar_t* pLabel = ITEM_UPGRADE_SLOTS[iSelectedSlot].pName;
			const float2_t vMeasured =
				CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
			const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * 0.95f / vMeasured.y) : 1.f;
			const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
			const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
			CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
				float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
				XMVectorSet(1.0f, 0.5686f, 0.0f, 1.f), 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
		}
	}

	const LEVEL_TEXT_SLOT SLOTS[] =
	{
		{ "ItemUpgrade_CurLevelLabel", L"18\xB2E8\xACC4", 0.95f,
			XMVectorSet(1.0f, 0.7412f, 0.2902f, 1.f) }, // "18단계"
		{ "ItemUpgrade_NextLevelLabel", L"19\xB2E8\xACC4", 0.95f,
			XMVectorSet(0.7176f, 0.9843f, 0.0f, 1.f) }, // "19단계"
	};

	for (const LEVEL_TEXT_SLOT& Slot : SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(Slot.pSlotId, fX, fY, fWidth, fHeight))
			continue;

		const f32_t fCenterX = fX + fWidth * 0.5f;
		const f32_t fCenterY = fY + fHeight * 0.5f;

		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), Slot.pLabel);
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * Slot.fHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), Slot.pLabel,
			float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
			Slot.vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	}
}

void CMainApp::RenderItemUpgradeMaterialCounts()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	/* Placeholder preview economy, matching the fixed 18 -> 19 level text drawn above
	(no live Server balance data is wired into this preview): required amount for the
	18 -> 19 transition, i.e. targetLevel = 19.
	  - blue crystal : 100 per level (100 * targetLevel)
	  - pink gem     : 5 per every 5-level band (5 * ceil(targetLevel / 5))
	  - orange gem   : 3 per every 5-level band (3 * ceil(targetLevel / 5))
	Owned is a placeholder 9999 until real inventory data is wired in. Colors are the real
	sampled reference (lime "owned/[B7FB00]" when owned >= required, red "0xE73517" style
	when short) -- reference screenshot shows all three counts at one shared font size
	regardless of digit count, so this measures the longest string once (rather than
	independently fitting each slot's own box) and reuses that scale for all three. */
	const int32_t iTargetLevel = 19;
	const int32_t iBand = (iTargetLevel + 4) / 5;
	const int32_t iOwned = 9999;
	const int32_t iRequired[3] = { 100 * iTargetLevel, 5 * iBand, 3 * iBand };
	const char* SLOT_IDS[3] = { "ItemUpgrade_RecipeAmount0", "ItemUpgrade_RecipeAmount1", "ItemUpgrade_RecipeAmount2" };
	const fvector_t vSufficientColor = XMVectorSet(0.7176f, 0.9843f, 0.0f, 1.f); // #B7FB00
	const fvector_t vInsufficientColor = XMVectorSet(0.9059f, 0.2078f, 0.0902f, 1.f); // #E73517

	/* One shared scale for all three (so digit-count differences don't change apparent size),
	but taken as the minimum fit across all three slots' own box -- not just the first slot's
	height -- so the longest string ("9999 / 1900") can't overflow into its neighbors. */
	f32_t fSharedScale = 1.f;
	{
		bool_t bAny = false;
		for (int32_t i = 0; i < 3; ++i)
		{
			f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
			if (!m_pItemUpgradeView->Get_SlotRect(SLOT_IDS[i], fX, fY, fW, fH))
				continue;

			const wstring strAmount = to_wstring(iOwned) + L" / " + to_wstring(iRequired[i]);
			const float2_t vMeasured =
				CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strAmount.c_str());
			if (vMeasured.y <= 0.f || vMeasured.x <= 0.f)
				continue;

			const f32_t fFit = (std::min)(fH * 0.9f / vMeasured.y, fW * 0.95f / vMeasured.x);
			fSharedScale = bAny ? (std::min)(fSharedScale, fFit) : fFit;
			bAny = true;
		}
	}

	for (int32_t i = 0; i < 3; ++i)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(SLOT_IDS[i], fX, fY, fWidth, fHeight))
			continue;

		const f32_t fCenterX = fX + fWidth * 0.5f;
		const f32_t fCenterY = fY + fHeight * 0.5f;

		const wstring strAmount = to_wstring(iOwned) + L" / " + to_wstring(iRequired[i]);
		const fvector_t vColor = (iOwned >= iRequired[i]) ? vSufficientColor : vInsufficientColor;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strAmount.c_str(),
			float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fSharedScale * textUiScale);
	}
}

void CMainApp::RenderItemUpgradeGaugePercentText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible)
		return;

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_GaugeFill", fX, fY, fWidth, fHeight))
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const wstring strPercent = to_wstring(m_iItemUpgradePreviousPercent) + L"%";
	const float2_t vMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strPercent.c_str());
	const f32_t fScale = (vMeasured.y > 0.f) ? (fHeight * 0.2f / vMeasured.y) : 1.f;

	// bright glowing gold/yellow, matching the ring's own fire-flourish palette.
	const fvector_t vColor = XMVectorSet(1.0f, 0.851f, 0.353f, 1.f); // #FFD959
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strPercent.c_str(),
		float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
		vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
}

void CMainApp::Render_ItemQuickSlots()
{
	if (nullptr == m_pHUDRuntimeView || nullptr == m_pInventoryView)
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr == pViewport)
		return;
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	constexpr const char* ITEM_SLOT_IDS[4] = { "Item_1", "Item_2", "Item_3", "Item_4" };

	string strDroppedItemId;
	float fDropX = 0.f, fDropY = 0.f;
	if (m_pInventoryView->Try_Consume_ItemDrop(strDroppedItemId, fDropX, fDropY))
	{
		for (int32_t i = 0; i < 4; ++i)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!m_pHUDRuntimeView->Get_SlotRect(ITEM_SLOT_IDS[i], fX, fY, fWidth, fHeight))
				continue;
			const float left = pViewport->WorkPos.x + fX * scaleX;
			const float top = pViewport->WorkPos.y + fY * scaleY;
			const float right = left + fWidth * scaleX;
			const float bottom = top + fHeight * scaleY;
			if (fDropX >= left && fDropX < right && fDropY >= top && fDropY < bottom)
			{
				m_strItemQuickSlot[i] = strDroppedItemId;
				break;
			}
		}
	}

	for (int32_t i = 0; i < 4; ++i)
	{
		if (m_strItemQuickSlot[i].empty())
			continue;
		const ITEM_DEFINITION* pDefinition = CItemCatalog::Find_ById(m_strItemQuickSlot[i]);
		if (nullptr == pDefinition || pDefinition->strIconPath.empty())
			continue;
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect(ITEM_SLOT_IDS[i], fX, fY, fWidth, fHeight))
			continue;
		const ImVec2 vMin(
			pViewport->WorkPos.x + fX * scaleX, pViewport->WorkPos.y + fY * scaleY);
		const ImVec2 vMax(
			vMin.x + fWidth * scaleX, vMin.y + fHeight * scaleY);
		if (ID3D11ShaderResourceView* pIconSRV =
			m_pHUDRuntimeView->Load_Texture(pDefinition->strIconPath))
		{
			pDrawList->AddImage(pIconSRV, vMin, vMax);
		}
	}
}

void CMainApp::RenderPlayerHealthManaBar()
{
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumHp || 0u == player.iMaximumResource)
		return;
	if (nullptr == m_pHUDRuntimeView)
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr == pViewport)
		return;
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	const float healthRatio = (std::clamp)(
		static_cast<float>(player.iCurrentHp) / static_cast<float>(player.iMaximumHp), 0.f, 1.f);
	const float manaRatio = (std::clamp)(
		static_cast<float>(player.iCurrentResource) / static_cast<float>(player.iMaximumResource), 0.f, 1.f);

	f32_t fHpX = 0.f, fHpY = 0.f, fHpWidth = 0.f, fHpHeight = 0.f;
	if (m_pHUDRuntimeView->Get_SlotRect("HealthBar", fHpX, fHpY, fHpWidth, fHpHeight))
	{
		if (ID3D11ShaderResourceView* pHpSRV = m_pHUDRuntimeView->Load_Texture("UI/HUD/Common/HP Bar.png"))
		{
			const ImVec2 vMin{
				pViewport->WorkPos.x + fHpX * scaleX,
				pViewport->WorkPos.y + fHpY * scaleY };
			const ImVec2 vMax{
				vMin.x + fHpWidth * scaleX,
				vMin.y + fHpHeight * scaleY };
			const float fBoundaryX = vMin.x + (vMax.x - vMin.x) * healthRatio;
			if (healthRatio > 0.f)
			{
				pDrawList->AddImage(pHpSRV, vMin, ImVec2(fBoundaryX, vMax.y),
					ImVec2(0.f, 0.f), ImVec2(healthRatio, 1.f));
			}
		}
	}

	f32_t fMpX = 0.f, fMpY = 0.f, fMpWidth = 0.f, fMpHeight = 0.f;
	if (m_pHUDRuntimeView->Get_SlotRect("ManaBar", fMpX, fMpY, fMpWidth, fMpHeight))
	{
		if (ID3D11ShaderResourceView* pMpSRV = m_pHUDRuntimeView->Load_Texture("UI/HUD/Common/MP Bar.png"))
		{
			const ImVec2 vMin{
				pViewport->WorkPos.x + fMpX * scaleX,
				pViewport->WorkPos.y + fMpY * scaleY };
			const ImVec2 vMax{
				vMin.x + fMpWidth * scaleX,
				vMin.y + fMpHeight * scaleY };
			const float fBoundaryX = vMin.x + (vMax.x - vMin.x) * manaRatio;
			if (manaRatio > 0.f)
			{
				pDrawList->AddImage(pMpSRV, vMin, ImVec2(fBoundaryX, vMax.y),
					ImVec2(0.f, 0.f), ImVec2(manaRatio, 1.f));
			}
		}
	}
}

void CMainApp::RenderLanceMasterIdentityGauge()
{
	/* Same real formula as ark.ui.identityLanceMaster.LanceMasterProgress::updateProgress():
	each of the 3 segments independently tracks 0..100, and only fills once every segment
	before it is already full (LanceMasterStance.as's bubbleEffect cascade). Degrees come from
	the real gauge0/1/2.maxDegree constants (-80/-82/-76); sign only decided sweep direction in
	Flash's own rotation convention, so the sweep magnitude is what is real about it. */
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumIdentity || nullptr == m_pHUDRuntimeView)
		return;

	constexpr f32_t SEGMENT_MAX = 100.f;
	const f32_t fSegmentScale = player.iMaximumIdentity / (SEGMENT_MAX * 3.f);
	f32_t fRemaining = static_cast<f32_t>(player.iCurrentIdentity) / fSegmentScale;
	f32_t fSegmentValue[3];
	for (int32_t i = 0; i < 3; ++i)
	{
		fSegmentValue[i] = (std::min)(SEGMENT_MAX, (std::max)(0.f, fRemaining));
		fRemaining -= SEGMENT_MAX;
	}

	/* target.rotation = maxDegree * (value/100) confirmed straight from the decompiled
	ark.ui.identityLanceMaster.LanceMasterProgress::updateProgress() -- target (depth3, clipDepth=7)
	is a rotating mask that reveals track (depth5, the real white fill bitmap) as it turns. Rather
	than reproduce that mask at runtime, Gauge0/1/2Fill.json bakes the real target-rotated-mask x
	track composite for every integer percentage (same pattern as Warlord's GaugeL/R 100-frame
	reveal), so this only has to pick the frame for the current percentage. */
	for (int32_t i = 0; i < 3; ++i)
	{
		const int32_t iFillFrame = std::clamp(static_cast<int32_t>(fSegmentValue[i]), 0, 99);
		m_pHUDRuntimeView->Play_KeyframeAnimation(
			string("Lance_Id_GaugeFill") + std::to_string(i), std::to_string(iFillFrame));

		/* Real extracted gauge0/1/2 highLightMc flourish -- a one-shot dim->bright ignite
		(Lance_Id_GaugeBurn0/1/2, Gauge0/1/2Burn.json) followed by a continuous sustain loop
		(Lance_Id_GaugeBurnLoop0/1/2, Gauge0/1/2BurnLoop.json) once ignite finishes, so the flame
		keeps burning instead of restarting its dim intro every loop. Each ignite doc's own
		[startFrame,frameCount) span at 40fps gives its real playback duration. */
		constexpr f32_t BURN_IGNITE_SECONDS[3] = { 36.f / 40.f, 32.f / 40.f, 36.f / 40.f };
		const bool_t bIsFull = fSegmentValue[i] >= SEGMENT_MAX;
		if (bIsFull && !m_bLanceGaugeSegmentWasFull[i])
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(
				string("Lance_Id_GaugeBurn") + std::to_string(i), "burn");
			m_dLanceGaugeIgniteStartSeconds[i] = ImGui::GetTime();
			m_bLanceGaugeLoopStarted[i] = false;
		}
		m_bLanceGaugeSegmentWasFull[i] = bIsFull;

		const bool_t bIgniteDone = bIsFull && m_dLanceGaugeIgniteStartSeconds[i] >= 0.0 &&
			(ImGui::GetTime() - m_dLanceGaugeIgniteStartSeconds[i]) >= BURN_IGNITE_SECONDS[i];
		if (bIgniteDone && !m_bLanceGaugeLoopStarted[i])
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(
				string("Lance_Id_GaugeBurnLoop") + std::to_string(i), "burn");
			m_bLanceGaugeLoopStarted[i] = true;
		}

		/* The burn flourish only makes sense while a segment is actually full -- Fill's own
		frame 99 already swaps to the orange "charged" art once full, so no separate glow
		slot is needed on top of it. */
		m_pHUDRuntimeView->Set_SlotVisible(string("Lance_Id_GaugeBurn") + std::to_string(i), bIsFull && !bIgniteDone);
		m_pHUDRuntimeView->Set_SlotVisible(string("Lance_Id_GaugeBurnLoop") + std::to_string(i), bIsFull && bIgniteDone);
	}
}

void CMainApp::RenderSkillCooldowns()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumHp || 0u == player.iMaximumResource)
		return;

	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	if (skillWindowOpen || nullptr == m_pHUDRuntimeView)
		return;

	/* Matches the fixed server tick rate other Client files already redeclare locally
	(CombatHUDViewModel.cpp, Character.cpp) rather than exposing a Shared constant for it. */
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	constexpr f32_t PI = 3.14159265f;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const f32_t fScaleX = pViewport->WorkSize.x / REF_WIDTH;
	const f32_t fScaleY = pViewport->WorkSize.y / REF_HEIGHT;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	for (const HUD_SKILL_STATE& Skill : player.Skills)
	{
		if (Skill.strInputSlot.empty() || Skill.Is_Ready(player.iServerTick))
			continue;

		/* Warlord's Z/X badges have their own real extracted cooldown visual (WarLordSkinFrame's
		SkillSlot "disabled" state swaps the whole icon to a real dark variant, see the Skill_Z/X
		keyframe wiring below) instead of this generic pie sweep, which was built for Q-F only. */
		if ("Z" == Skill.strInputSlot || "X" == Skill.strInputSlot)
		{
			/* Artist's Z/X slots are real ARKNewSlot instances too, which have their own real
			coolDown wipe (already keyframed, see Yin_Skill_Z/X_Wipe below) AND a real cooldownText
			TextField sub-component (confirmed: yinYangShiSlot symbol 343, depth15, name
			"cooldownText") -- draw just the countdown number here, same "Ns" style as Q-F, without
			the generic pie (Artist's real wipe shape already covers that). */
			if (LostArk::Shared::CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass)
			{
				const uint32_t remainingTicks = Skill.iCooldownEndTick > player.iServerTick ?
					Skill.iCooldownEndTick - player.iServerTick : 0u;
				if (0u != remainingTicks)
				{
					f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
					if (m_pHUDRuntimeView->Get_SlotRect("Yin_Skill_" + Skill.strInputSlot, fX, fY, fWidth, fHeight))
					{
						const f32_t fRemainingSeconds = static_cast<f32_t>(remainingTicks) / SERVER_TICK_HZ;
						const int32_t iDisplaySeconds = static_cast<int32_t>(ceilf(fRemainingSeconds));
						const string strCooldownLabel = std::to_string(iDisplaySeconds) + "s";

						const ImVec2 vTopLeft(
							pViewport->WorkPos.x + fX * fScaleX,
							pViewport->WorkPos.y + fY * fScaleY);
						const ImVec2 vCenter(
							vTopLeft.x + 22.5f * 0.5f * fScaleX,
							vTopLeft.y + 22.5f * 0.5f * fScaleY);

						ImFont* pFont = ImGui::GetFont();
						const f32_t fFontSize = 22.5f * fScaleY * 0.34f;
						const ImVec2 vTextSize =
							pFont->CalcTextSizeA(fFontSize, FLT_MAX, 0.f, strCooldownLabel.c_str());
						const ImVec2 vTextPos(
							vCenter.x - vTextSize.x * 0.5f,
							vCenter.y - vTextSize.y * 0.5f);

						pDrawList->AddText(pFont, fFontSize, ImVec2(vTextPos.x + 1.f, vTextPos.y + 1.f),
							IM_COL32(0, 0, 0, 220), strCooldownLabel.c_str());
						pDrawList->AddText(pFont, fFontSize, vTextPos,
							IM_COL32(255, 255, 255, 255), strCooldownLabel.c_str());
					}
				}
			}
			continue;
		}

		const uint32_t remainingTicks = Skill.iCooldownEndTick > player.iServerTick ?
			Skill.iCooldownEndTick - player.iServerTick : 0u;
		if (0u == remainingTicks)
			continue;

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect("Skill_" + Skill.strInputSlot, fX, fY, fWidth, fHeight))
			continue;

		const f32_t fRemainingSeconds = static_cast<f32_t>(remainingTicks) / SERVER_TICK_HZ;
		const f32_t fTotalSeconds = Skill.iCooldownDurationTicks > 0u ?
			static_cast<f32_t>(Skill.iCooldownDurationTicks) / SERVER_TICK_HZ : fRemainingSeconds;
		const f32_t fFraction = fTotalSeconds > 0.f ?
			(std::min)(1.f, (std::max)(0.f, fRemainingSeconds / fTotalSeconds)) : 0.f;

		const ImVec2 vTopLeft(
			pViewport->WorkPos.x + fX * fScaleX,
			pViewport->WorkPos.y + fY * fScaleY);
		const ImVec2 vBotRight(
			vTopLeft.x + fWidth * fScaleX,
			vTopLeft.y + fHeight * fScaleY);
		const ImVec2 vCenter(
			(vTopLeft.x + vBotRight.x) * 0.5f,
			(vTopLeft.y + vBotRight.y) * 0.5f);
		const f32_t fHalfW = (vBotRight.x - vTopLeft.x) * 0.5f;
		const f32_t fHalfH = (vBotRight.y - vTopLeft.y) * 0.5f;
		/* Sized past the slot's corners and clipped to its rect below, so the visible edge of
		the pie traces the square's own border instead of an inscribed circle -- a plain
		circular-sector fill would leave the corners uncovered while mostly full. */
		const f32_t fRadius = sqrtf(fHalfW * fHalfW + fHalfH * fHalfH) + 2.f;

		/* Sweeps clockwise from 12 o'clock as the *remaining* cooldown, shrinking back to
		nothing as it expires -- the icon starts fully covered right after use and is revealed
		clockwise, matching the reference cooldown swipe. */
		const f32_t fStartAngle = -PI * 0.5f;
		const f32_t fEndAngle = fStartAngle + fFraction * 2.f * PI;

		pDrawList->PushClipRect(vTopLeft, vBotRight, true);
		pDrawList->PathClear();
		pDrawList->PathLineTo(vCenter);
		pDrawList->PathArcTo(vCenter, fRadius, fStartAngle, fEndAngle, 32);
		pDrawList->PathFillConvex(IM_COL32(0, 0, 0, 150));
		pDrawList->PopClipRect();

		const int32_t iDisplaySeconds = static_cast<int32_t>(ceilf(fRemainingSeconds));
		const string strCooldownLabel = std::to_string(iDisplaySeconds) + "s";

		ImFont* pFont = ImGui::GetFont();
		const f32_t fFontSize = fHeight * fScaleY * 0.34f;
		const ImVec2 vTextSize =
			pFont->CalcTextSizeA(fFontSize, FLT_MAX, 0.f, strCooldownLabel.c_str());
		const ImVec2 vTextPos(
			vCenter.x - vTextSize.x * 0.5f,
			vCenter.y - vTextSize.y * 0.5f);

		pDrawList->AddText(pFont, fFontSize, ImVec2(vTextPos.x + 1.f, vTextPos.y + 1.f),
			IM_COL32(0, 0, 0, 220), strCooldownLabel.c_str());
		pDrawList->AddText(pFont, fFontSize, vTextPos,
			IM_COL32(255, 255, 255, 255), strCooldownLabel.c_str());
	}
}

void CMainApp::RenderBossHealthBar()
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

	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!boss.isValid || 0u == boss.iMaximumHp)
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr == pViewport)
		return;
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;
	const float uiScale = (std::min)(scaleX, scaleY);

	/* Real Lost Ark raid bosses (Valtan: Data/Balance/BossProfiles.json maximumHealthBars=160)
	don't show current/max HP as one continuous 0..100% bar -- total HP is split into
	iMaximumHealthBars equal segments ("줄"), the single visible bar only ever shows the CURRENT
	segment's own fill, and it resets to full and the count ticks down by one every time a segment
	fully drains. iMaximumHp/iMaximumHealthBars segments of iCurrentHp is exact server data, not a
	client guess -- this is just choosing how to lay the same numbers out visually. */
	const uint32_t iMaximumBars = (std::max)(1u, boss.iMaximumHealthBars);
	const double segmentHp =
		static_cast<double>(boss.iMaximumHp) / static_cast<double>(iMaximumBars);
	float healthRatio = 0.f;
	uint32_t iBarsRemaining = 0u;
	if (boss.iCurrentHp > 0u && segmentHp > 0.0)
	{
		const double scaledHp = static_cast<double>(boss.iCurrentHp) / segmentHp;
		const double fraction = scaledHp - std::floor(scaledHp);
		/* Exactly on a segment boundary (fraction == 0) means that segment is untouched and full,
		not freshly emptied -- e.g. iCurrentHp == iMaximumHp must render as a full bar. */
		healthRatio = static_cast<float>(0.0 == fraction ? 1.0 : fraction);
		iBarsRemaining = (std::clamp)(
			static_cast<uint32_t>(std::ceil(scaledHp)), 1u, iMaximumBars);
	}

	/* Edge-detect against the previous frame's bars-remaining/HP to trigger the two real effects
	below -- reset instead of comparing on a target swap, or the new boss's lower HP/bar-count would
	read as damage taken against the old one. */
	if (boss.strArchetypeId != m_strPreviousBossArchetypeId)
	{
		m_strPreviousBossArchetypeId = boss.strArchetypeId;
		m_iPreviousBossBarsRemaining = iBarsRemaining;
		m_iPreviousBossCurrentHp = boss.iCurrentHp;
		m_dBossBarTickFlashStartSeconds = -1.0;
		m_dBossHitGlowStartSeconds = -1.0;
	}
	else
	{
		if (iBarsRemaining < m_iPreviousBossBarsRemaining)
			m_dBossBarTickFlashStartSeconds = ImGui::GetTime();
		if (boss.iCurrentHp < m_iPreviousBossCurrentHp)
		{
			m_dBossHitGlowStartSeconds = ImGui::GetTime();
			m_fBossHitGlowFillRatio = healthRatio;
		}
		m_iPreviousBossBarsRemaining = iBarsRemaining;
		m_iPreviousBossCurrentHp = boss.iCurrentHp;
	}

	/* Position/size come from the HUD Layout Tool (F1) now, not derived SWF-trace math or any
	combined/shared rect this code computes on its own -- every raw piece (frame, fill, stagger
	background, stagger track) is its own independent slot in the "Boss UI" document
	(m_pBossUIView), each defaulting to that resource's native pixel size, so the user places and
	resizes each one by hand without this function deciding how they relate to each other. Nothing
	to draw if the fill slot -- the one piece that actually needs to exist for the bar to mean
	anything -- hasn't loaded. */
	if (nullptr == m_pBossUIView)
		return;
	f32_t fFillRectX = 0.f, fFillRectY = 0.f, fFillRectWidth = 0.f, fFillRectHeight = 0.f;
	if (!m_pBossUIView->Get_SlotRect(
		"Boss_Fill", fFillRectX, fFillRectY, fFillRectWidth, fFillRectHeight))
	{
		return;
	}
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	/* Real EFUI_STATUS pieces -- see Resources/UI/BossUI. The boss_bar_fill_* set are solid-color
	bar rows cropped directly from targetstatus_loc_int_i2.dds's top cluster (square left edge,
	tapered right point -- the same silhouette the real bar's shapeBounds gives, 485x28); drawn
	inset and UV-clipped by healthRatio instead of stretched, so a partial bar shows the real art's
	left portion, not a squished copy. Fill color cycles per bar segment, matching the real
	ProgressMultiTrack.trackValue formula (trackValue % trackColorLength, 1-based, wrapping to the
	length instead of 0) but with the 6 colors picked by hand from i2.dds's cluster --
	teal/blue/purple/orange/red/green -- instead of the real SWF's own 7-color cycle
	(white/translucent-red/black/gray rows explicitly excluded per direct instruction). */
	constexpr const char* FILL_COLOR_CYCLE[] = {
		"UI/BossUI/boss_bar_fill_teal.png",
		"UI/BossUI/boss_bar_fill_blue.png",
		"UI/BossUI/boss_bar_fill_purple.png",
		"UI/BossUI/boss_bar_fill_orange.png",
		"UI/BossUI/boss_bar_fill_red.png",
		"UI/BossUI/boss_bar_fill_green.png",
	};
	constexpr uint32_t FILL_COLOR_COUNT =
		static_cast<uint32_t>(sizeof(FILL_COLOR_CYCLE) / sizeof(FILL_COLOR_CYCLE[0]));
	const uint32_t iColorCycleValue = (std::max)(1u, iBarsRemaining);
	const uint32_t iColorIndex = ((iColorCycleValue % FILL_COLOR_COUNT == 0) ?
		FILL_COLOR_COUNT : (iColorCycleValue % FILL_COLOR_COUNT)) - 1u;

	ID3D11ShaderResourceView* pFillSRV =
		m_pBossUIView->Load_Texture(FILL_COLOR_CYCLE[iColorIndex]);

	const ImVec2 fillTrackMin{
		pViewport->WorkPos.x + fFillRectX * scaleX,
		pViewport->WorkPos.y + fFillRectY * scaleY };
	const ImVec2 fillTrackMax{
		fillTrackMin.x + fFillRectWidth * scaleX,
		fillTrackMin.y + fFillRectHeight * scaleY };
	const float fillInset = 2.f * uiScale;
	const ImVec2 fillMin{ fillTrackMin.x + fillInset, fillTrackMin.y + fillInset };
	const ImVec2 fillMax{ fillTrackMax.x - fillInset, fillTrackMax.y - fillInset };
	const float fFillBoundaryX = fillMin.x + (fillMax.x - fillMin.x) * healthRatio;

	/* The area the current segment's fill hasn't reached yet isn't empty -- it's the next bar
	segment's own color already sitting behind it, so draining the current segment reveals the
	next one's color instead of a gap. Only the current segment (iColorIndex) is UV-clipped by
	healthRatio; this background is the (iBarsRemaining - 1) segment drawn full-width underneath.
	No "next" color once the last bar is draining (iBarsRemaining == 1). */
	if (iBarsRemaining > 1u)
	{
		const uint32_t iNextColorCycleValue = iBarsRemaining - 1u;
		const uint32_t iNextColorIndex = ((iNextColorCycleValue % FILL_COLOR_COUNT == 0) ?
			FILL_COLOR_COUNT : (iNextColorCycleValue % FILL_COLOR_COUNT)) - 1u;
		if (ID3D11ShaderResourceView* pNextFillSRV =
			m_pBossUIView->Load_Texture(FILL_COLOR_CYCLE[iNextColorIndex]))
		{
			pDrawList->AddImage(pNextFillSRV, fillMin, fillMax);
		}
	}

	if (nullptr != pFillSRV && healthRatio > 0.f)
	{
		pDrawList->AddImage(pFillSRV, fillMin, ImVec2(fFillBoundaryX, fillMax.y),
			ImVec2(0.f, 0.f), ImVec2(healthRatio, 1.f));
	}

	/* boss_bar_ornate_frame.png (the user's reference capture 제목없음.png, used whole, badge and
	border together) is purely decorative and independently placed via its own "Boss_Frame" slot --
	it does not define the fill's 0-100% width, that's Boss_Fill's own rect above. */
	f32_t fFrameRectX = 0.f, fFrameRectY = 0.f, fFrameRectWidth = 0.f, fFrameRectHeight = 0.f;
	if (m_pBossUIView->Get_SlotRect(
		"Boss_Frame", fFrameRectX, fFrameRectY, fFrameRectWidth, fFrameRectHeight))
	{
		if (ID3D11ShaderResourceView* pOrnateFrameSRV =
			m_pBossUIView->Load_Texture("UI/BossUI/boss_bar_ornate_frame.png"))
		{
			const ImVec2 vFrameMin{
				pViewport->WorkPos.x + fFrameRectX * scaleX,
				pViewport->WorkPos.y + fFrameRectY * scaleY };
			const ImVec2 vFrameMax{
				vFrameMin.x + fFrameRectWidth * scaleX,
				vFrameMin.y + fFrameRectHeight * scaleY };
			pDrawList->AddImage(pOrnateFrameSRV, vFrameMin, vFrameMax);
		}
	}

	/* Stagger/paralyzation gauge (real paralyzationGauge -- background + fill + hollow
	purple-bordered track, char 473 in TargetGrade_Boss). The Server snapshot owns
	current/maximum; presentation only crops the existing fill art inside the authored track. */
	f32_t fStaggerBgX = 0.f, fStaggerBgY = 0.f, fStaggerBgWidth = 0.f, fStaggerBgHeight = 0.f;
	if (m_pBossUIView->Get_SlotRect(
		"Boss_StaggerBg", fStaggerBgX, fStaggerBgY, fStaggerBgWidth, fStaggerBgHeight))
	{
		if (ID3D11ShaderResourceView* pStaggerBgSRV =
			m_pBossUIView->Load_Texture("UI/BossUI/boss_stagger_bg.png"))
		{
			const ImVec2 vStaggerBgMin{
				pViewport->WorkPos.x + fStaggerBgX * scaleX,
				pViewport->WorkPos.y + fStaggerBgY * scaleY };
			const ImVec2 vStaggerBgMax{
				vStaggerBgMin.x + fStaggerBgWidth * scaleX,
				vStaggerBgMin.y + fStaggerBgHeight * scaleY };
			pDrawList->AddImage(pStaggerBgSRV, vStaggerBgMin, vStaggerBgMax);
		}
	}
	f32_t fStaggerTrackX = 0.f, fStaggerTrackY = 0.f, fStaggerTrackWidth = 0.f, fStaggerTrackHeight = 0.f;
	if (m_pBossUIView->Get_SlotRect(
		"Boss_StaggerTrack", fStaggerTrackX, fStaggerTrackY, fStaggerTrackWidth, fStaggerTrackHeight))
	{
		const ImVec2 vStaggerTrackMin{
			pViewport->WorkPos.x + fStaggerTrackX * scaleX,
			pViewport->WorkPos.y + fStaggerTrackY * scaleY };
		const ImVec2 vStaggerTrackMax{
			vStaggerTrackMin.x + fStaggerTrackWidth * scaleX,
			vStaggerTrackMin.y + fStaggerTrackHeight * scaleY };
		if (0u != boss.iMaximumStagger && 0u != boss.iCurrentStagger)
		{
			const f32_t fStaggerRatio = (std::clamp)(
				static_cast<f32_t>(boss.iCurrentStagger) /
					static_cast<f32_t>(boss.iMaximumStagger),
				0.f, 1.f);
			if (ID3D11ShaderResourceView* pStaggerFillSRV =
				m_pBossUIView->Load_Texture(
					"UI/BossUI/boss_stagger_fill.png"))
			{
				pDrawList->AddImage(
					pStaggerFillSRV,
					vStaggerTrackMin,
					ImVec2(
						vStaggerTrackMin.x +
							(vStaggerTrackMax.x - vStaggerTrackMin.x) *
							fStaggerRatio,
						vStaggerTrackMax.y),
					ImVec2(0.f, 0.f),
					ImVec2(fStaggerRatio, 1.f));
			}
		}
		if (ID3D11ShaderResourceView* pStaggerTrackSRV =
			m_pBossUIView->Load_Texture("UI/BossUI/boss_stagger_track.png"))
		{
			pDrawList->AddImage(pStaggerTrackSRV, vStaggerTrackMin, vStaggerTrackMax);
		}
	}

	/* User-supplied boundary marker (HP seperate Bar.png -- a tiny 3x15 soft cream vertical glow
	line, not an EFUI_STATUS extraction) drawn at the current fill/empty edge, matching the real
	Progress::mark concept (an edge indicator repositioned every update) this session couldn't
	trace real art for earlier. Hidden exactly at 0%/100%, same as the real useAutoHideMark
	behaviour, since there's no boundary to mark once the bar is fully empty or full. */
	ID3D11ShaderResourceView* pSeparatorSRV =
		m_pBossUIView->Load_Texture("UI/BossUI/boss_bar_separator.png");
	if (nullptr != pSeparatorSRV && healthRatio > 0.f && healthRatio < 1.f)
	{
		const float fSeparatorHalfWidth = 3.f * uiScale;
		pDrawList->AddImage(pSeparatorSRV,
			ImVec2(fFillBoundaryX - fSeparatorHalfWidth, fillMin.y),
			ImVec2(fFillBoundaryX + fSeparatorHalfWidth, fillMax.y));
	}

	/* Real ProgressMultiTrack::updateTarget cross-fades a second "cloneTarget" fill instance --
	same shape as the fill, colorTransform forced to solid white -- over the real fill whenever a
	bar segment ticks over, instead of hard-cutting back to full. Approximated with a flat white
	rect (no separate white-silhouette asset was extracted) over the same now-full fill area,
	fading out over the tween window real gaugeComplete() uses (0.1-0.23s -- rounded up here since
	an ImGui flash reads as more of a blip at the real duration). */
	if (m_dBossBarTickFlashStartSeconds >= 0.0)
	{
		constexpr f64_t BAR_TICK_FLASH_SECONDS = 0.3;
		const f64_t fFlashAge = ImGui::GetTime() - m_dBossBarTickFlashStartSeconds;
		if (fFlashAge < BAR_TICK_FLASH_SECONDS)
		{
			const f32_t fFlashAlpha = 1.f - static_cast<f32_t>(fFlashAge / BAR_TICK_FLASH_SECONDS);
			const float fillInset = 2.f * uiScale;
			pDrawList->AddRectFilled(
				ImVec2(fillTrackMin.x + fillInset, fillTrackMin.y + fillInset),
				ImVec2(fillTrackMax.x - fillInset, fillTrackMax.y - fillInset),
				IM_COL32(255, 255, 255, static_cast<int>(220.f * fFlashAlpha)));
		}
	}

	/* Real Progress::updateMark positions a "mark" clip at the fill's own edge every update; its
	symbol (character 732) is a 39-frame animated additive glow (grows ~1.0->1.5/3.0 scale, fades
	~256->92 alpha) rather than a static line. No source art was traced for it, so this reproduces
	the motion procedurally: a soft glow at the edge that grows then fades on every HP drop. */
	if (m_dBossHitGlowStartSeconds >= 0.0)
	{
		constexpr f64_t HIT_GLOW_SECONDS = 0.35;
		const f64_t fGlowAge = ImGui::GetTime() - m_dBossHitGlowStartSeconds;
		if (fGlowAge < HIT_GLOW_SECONDS)
		{
			const f32_t fGlowT = static_cast<f32_t>(fGlowAge / HIT_GLOW_SECONDS);
			const f32_t fGlowAlpha = 1.f - fGlowT;
			const f32_t fGlowRadius = (6.f + 10.f * fGlowT) * uiScale;
			const ImVec2 vGlowCenter{
				fillTrackMin.x + (fillTrackMax.x - fillTrackMin.x) * m_fBossHitGlowFillRatio,
				(fillTrackMin.y + fillTrackMax.y) * 0.5f };
			pDrawList->AddCircleFilled(vGlowCenter, fGlowRadius,
				IM_COL32(255, 250, 230, static_cast<int>(200.f * fGlowAlpha)));
		}
	}

}

/* Split from RenderBossHealthBar() -- see the declaration comment in MainApp.h for why this has
to run after CImGuiLayer::EndFrame() instead of alongside the bar/frame image draws. Re-derives
healthRatio/iBarsRemaining from the same boss snapshot RenderBossHealthBar already used this frame;
cheap pure arithmetic, safer than threading the values out as member state. */
void CMainApp::RenderBossHealthBarText()
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

	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!boss.isValid || 0u == boss.iMaximumHp)
		return;
	if (nullptr == m_pBossUIView)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const uint32_t iMaximumBars = (std::max)(1u, boss.iMaximumHealthBars);
	const double segmentHp =
		static_cast<double>(boss.iMaximumHp) / static_cast<double>(iMaximumBars);
	uint32_t iBarsRemaining = 0u;
	if (boss.iCurrentHp > 0u && segmentHp > 0.0)
	{
		const double scaledHp = static_cast<double>(boss.iCurrentHp) / segmentHp;
		iBarsRemaining = (std::clamp)(
			static_cast<uint32_t>(std::ceil(scaledHp)), 1u, iMaximumBars);
	}
	const bool_t isMultiBar = boss.iMaximumHealthBars > 1u;

	/* Boss title, positioned via its own hand-placed slot ("Boss_TitleText"). No real per-boss
	title field exists in BossProfiles.json/HUD_BOSS_STATE yet (only strDisplayName="발탄") -- this
	project currently only has Valtan configured, so the full real title seen on-screen
	("마수군단장 발탄") is spelled out directly here instead of a data-driven prefix + name, same
	caveat as the earlier "보스" grade label attempt: this needs a real title field once a second
	boss exists, not a hardcoded string that only happens to be right for one archetype. */
	f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleWidth = 0.f, fTitleHeight = 0.f;
	if (m_pBossUIView->Get_SlotRect(
		"Boss_TitleText", fTitleX, fTitleY, fTitleWidth, fTitleHeight))
	{
		const wstring strBossTitle = L"\xB9C8\xC218\xAD70\xB2E8\xC7A5 \xBC1C\xD0C4";
		/* Text draw scale is derived from the slot's own box height (measured at scale=1 via
		Measure_Text) instead of a hand-picked constant, so resizing the slot in the HUD Layout
		Tool is what actually controls the rendered text size -- no more guessing pixel scales. */
		const float2_t vTitleMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), strBossTitle.c_str());
		const f32_t fTitleScale = (vTitleMeasured.y > 0.f) ? (fTitleHeight / vTitleMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), strBossTitle.c_str(),
			float2_t(
				(fTitleX + fTitleWidth * 0.5f) * textScaleX,
				(fTitleY + fTitleHeight * 0.5f) * textScaleY),
			XMVectorSet(1.f, 0.31f, 0.24f, 1.f), 0.f, float2_t(0.5f, 0.5f), fTitleScale * textUiScale);
	}

	/* HP number and bar-count text now come from their own hand-placed slots
	("Boss_HPText"/"Boss_BarCountText") instead of coordinates derived from the SWF trace -- the
	fury/enrage timer (icon + "광폭화까지" countdown) is dropped per direct instruction; that
	feature isn't being used. Text format itself is still real: "cur / max" and "X " + count (the
	latter from the decompiled ProgressMultiTrack::hpCount setter -- capital X, one space, hidden
	once only 1 bar is left, not just at 0). */
	f32_t fHpTextX = 0.f, fHpTextY = 0.f, fHpTextWidth = 0.f, fHpTextHeight = 0.f;
	if (m_pBossUIView->Get_SlotRect(
		"Boss_HPText", fHpTextX, fHpTextY, fHpTextWidth, fHpTextHeight))
	{
		const wstring strHpNumbers = std::to_wstring(boss.iCurrentHp) +
			L" / " + std::to_wstring(boss.iMaximumHp);
		/* Same box-height-fit approach as the title: the slot's own height (set in the HUD
		Layout Tool) is what determines the rendered digit height, not a hardcoded constant. */
		const float2_t vHpMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), strHpNumbers.c_str());
		const f32_t fHpScale = (vHpMeasured.y > 0.f) ? (fHpTextHeight / vHpMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), strHpNumbers.c_str(),
			float2_t(
				(fHpTextX + fHpTextWidth * 0.5f) * textScaleX,
				(fHpTextY + fHpTextHeight * 0.5f) * textScaleY),
			Colors::White, 0.f, float2_t(0.5f, 0.5f), fHpScale * textUiScale);
	}
	if (isMultiBar && iBarsRemaining > 1u)
	{
		f32_t fCountX = 0.f, fCountY = 0.f, fCountWidth = 0.f, fCountHeight = 0.f;
		if (m_pBossUIView->Get_SlotRect(
			"Boss_BarCountText", fCountX, fCountY, fCountWidth, fCountHeight))
		{
			const wstring strBarCount = L"X " + std::to_wstring(iBarsRemaining);
			const float2_t vCountMeasured =
				CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), strBarCount.c_str());
			const f32_t fCountScale = (vCountMeasured.y > 0.f) ? (fCountHeight / vCountMeasured.y) : 1.f;
			CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), strBarCount.c_str(),
				float2_t(
					(fCountX + fCountWidth * 0.5f) * textScaleX,
					(fCountY + fCountHeight * 0.5f) * textScaleY),
				Colors::White, 0.f, float2_t(0.5f, 0.5f), fCountScale * textUiScale);
		}
	}
}

void CMainApp::RenderEstherGauge()
{
	/* Esther's skill-select window is a Valtan raid mechanic -- Character Select's live Server
	room can still populate a nonzero gauge maximum for the selected class, which drew this
	window there too even though there is no raid encounter to use it against. */
	if (ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return;

	const uint32_t maximum =
		CCombatHUDViewModel::Get().Get_EstherGaugeMaximum();
	if (0u == maximum)
		return;
	if (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open())
		return;
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid)
		return;
	if (nullptr == m_pEstherUIView)
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr == pViewport)
		return;
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;
	const float uiScale = (std::min)(scaleX, scaleY);
	const uint32_t gauge = CCombatHUDViewModel::Get().Get_EstherGauge();
	const float fillRatio = (std::clamp)(
		static_cast<float>(gauge) / static_cast<float>(maximum), 0.f, 1.f);

	/* Esther_GaugeTrack's own rect (EstherUI.json, Tool-editable) positions the static background
	image (drawn generically by m_pEstherUIView->Render()) and the label below. Esther_GaugeFill is
	a separate Tool-placeable slot (own rect, independently adjustable) whose static art is forced
	hidden every frame (see the Set_SlotVisible calls above); this draws the real UV-clipped fill in
	its place -- same technique as the boss/player HP bars. Real pieces (frame/lock/gauge) traced
	from the actual estherweaponskill.gfx + EFUI_ICONATLAS_E packages, not placeholder rects. */
	f32_t fTrackX = 0.f, fTrackY = 0.f, fTrackWidth = 0.f, fTrackHeight = 0.f;
	if (!m_pEstherUIView->Get_SlotRect(
		"Esther_GaugeTrack", fTrackX, fTrackY, fTrackWidth, fTrackHeight))
	{
		return;
	}
	const ImVec2 barMin{
		pViewport->WorkPos.x + fTrackX * scaleX,
		pViewport->WorkPos.y + fTrackY * scaleY };
	const ImVec2 barMax{
		barMin.x + fTrackWidth * scaleX,
		barMin.y + fTrackHeight * scaleY };
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	f32_t fFillX = 0.f, fFillY = 0.f, fFillWidth = 0.f, fFillHeight = 0.f;
	if (m_pEstherUIView->Get_SlotRect("Esther_GaugeFill", fFillX, fFillY, fFillWidth, fFillHeight))
	{
		const ImVec2 fillMin{
			pViewport->WorkPos.x + fFillX * scaleX,
			pViewport->WorkPos.y + fFillY * scaleY };
		const ImVec2 fillMax{
			fillMin.x + fFillWidth * scaleX,
			fillMin.y + fFillHeight * scaleY };
		const float fillRight = fillMin.x + (fillMax.x - fillMin.x) * fillRatio;
		if (fillRight > fillMin.x)
		{
			if (ID3D11ShaderResourceView* pFillSRV =
				m_pEstherUIView->Load_Texture("UI/Esther/esther_gauge_fill_gold.png"))
			{
				pDrawList->AddImage(pFillSRV, fillMin, ImVec2(fillRight, fillMax.y),
					ImVec2(0.f, 0.f), ImVec2(fillRatio, 1.f));
			}
		}
	}
	if (gauge >= maximum)
	{
		ID3D11ShaderResourceView* pReadySRV =
			m_pEstherUIView->Load_Texture("UI/Esther/esther_slot_ready.png");
		if (nullptr != pReadySRV)
		{
			for (const char* pReadySlotId :
				{ "Esther_Slot1_Ready", "Esther_Slot2_Ready", "Esther_Slot3_Ready" })
			{
				f32_t fReadyX = 0.f, fReadyY = 0.f, fReadyWidth = 0.f, fReadyHeight = 0.f;
				if (!m_pEstherUIView->Get_SlotRect(
					pReadySlotId, fReadyX, fReadyY, fReadyWidth, fReadyHeight))
				{
					continue;
				}
				const ImVec2 readyMin{
					pViewport->WorkPos.x + fReadyX * scaleX,
					pViewport->WorkPos.y + fReadyY * scaleY };
				const ImVec2 readyMax{
					readyMin.x + fReadyWidth * scaleX,
					readyMin.y + fReadyHeight * scaleY };
				pDrawList->AddImage(pReadySRV, readyMin, readyMax);
			}
		}
	}

	const char* pLabel = gauge >= maximum ?
		"ESTHER READY  Ctrl+Z/X/C" : "ESTHER";
	const ImVec2 labelSize = ImGui::CalcTextSize(pLabel);
	const ImVec2 labelPos(
		(barMin.x + barMax.x - labelSize.x) * 0.5f,
		barMin.y - labelSize.y - 2.f * uiScale);
	pDrawList->AddText(
		ImVec2(labelPos.x + 1.f, labelPos.y + 1.f),
		IM_COL32(0, 0, 0, 220), pLabel);
	pDrawList->AddText(labelPos, IM_COL32(214, 238, 255, 255), pLabel);
}

void CMainApp::RenderSkillIcons()
{
	/* Which skill icon belongs in Skill_Q.."Skill_F" is content, not layout: it depends on the
	live (class, stance) pair via CPlayerSkillCatalog::Find_BySlot, the same source of truth the
	input controller already resolves quick slots from. HUD_Layout.json only owns the shared
	frame's position/size (ownerClass null "Skill_Q".."Skill_F"); it must not carry a second,
	class-hardcoded copy of "which icon" that can drift out of sync with PlayerSkills.json. This
	replaces the LanceMaster-only stance special-case with one path for every class -- Find_BySlot
	already resolves stance-gated skills correctly and ignores the stance argument for classes
	whose skills have no requiredStance. */
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid)
		return;

	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	if (skillWindowOpen || nullptr == m_pHUDRuntimeView)
		return;

	struct SKILL_ICON_ENTRY { LostArk::Shared::SKILL_ID iSkillId; const char* pIconPath; };
	constexpr SKILL_ICON_ENTRY SKILL_ICON_TABLE[] =
	{
		/* LanceMaster -- Long Spear */
		{ 34040, "UI/Skill/LanceMaster/34040_DoubleStrike.png" },
		{ 34090, "UI/Skill/LanceMaster/34090_ThornJab.png" },
		{ 34100, "UI/Skill/LanceMaster/34100_BlueDragonsClaw.png" },
		{ 34160, "UI/Skill/LanceMaster/34160_SpearDive.png" },
		{ 34140, "UI/Skill/LanceMaster/34140_SoulCutter.png" },
		{ 34120, "UI/Skill/LanceMaster/34120_ChainSlash.png" },
		{ 34110, "UI/Skill/LanceMaster/34110_HalfMoonSlash.png" },
		{ 34150, "UI/Skill/LanceMaster/34150_RagingDragonSlash.png" },
		/* LanceMaster -- T */
		{ 34650, "UI/Skill/LanceMaster/34650_DeadlyRedDragon.png" },
		/* LanceMaster -- Short Spear (no D/F skill in that stance) */
		{ 34540, "UI/Skill/LanceMaster/34540_SpiralingSpear.png" },
		{ 34550, "UI/Skill/LanceMaster/34550_4HeadedDragon.png" },
		{ 34560, "UI/Skill/LanceMaster/34560_ThrustOfDestruction.png" },
		{ 34570, "UI/Skill/LanceMaster/34570_StarfallPounce.png" },
		{ 34580, "UI/Skill/LanceMaster/34580_DragonscaleDefense.png" },
		{ 34590, "UI/Skill/LanceMaster/34590_RedDragonsHorn.png" },
		/* LanceMaster -- V (awakening) */
		{ 34610, "UI/Skill/LanceMaster/34610_StormingRedDragon.png" },
		/* Warlord */
		{ 17030, "UI/Skill/Warlord/17030_SharpSpear.png" },
		{ 17060, "UI/Skill/Warlord/17060_FireBullet.png" },
		{ 17080, "UI/Skill/Warlord/17080_DashUpperFire.png" },
		{ 17110, "UI/Skill/Warlord/17110_LeapAttack.png" },
		{ 17090, "UI/Skill/Warlord/17090_HookChain.png" },
		{ 17040, "UI/Skill/Warlord/17040_Bash.png" },
		{ 17100, "UI/Skill/Warlord/17100_ShieldShock.png" },
		{ 17140, "UI/Skill/Warlord/17140_GuardiansLightning.png" },
		/* Warlord -- T */
		{ 17240, "UI/Skill/Warlord/17240_FullBarrelCannon.png" },
		/* Warlord -- V (awakening) */
		{ 17170, "UI/Skill/Warlord/17170_GuardiansProtection.png" },
		/* Artist */
		{ 31200, "UI/Skill/Artist/31200_InkShower.png" },
		{ 31430, "UI/Skill/Artist/31430_Scatter.png" },
		{ 31480, "UI/Skill/Artist/31480_CraneWings.png" },
		{ 31210, "UI/Skill/Artist/31210_Kongkongi.png" },
		{ 31460, "UI/Skill/Artist/31460_ButterflyDream.png" },
		{ 31420, "UI/Skill/Artist/31420_OrchidStrike.png" },
		{ 31490, "UI/Skill/Artist/31490_TigerSlash.png" },
		{ 31470, "UI/Skill/Artist/31470_OneStroke.png" },
		/* Artist -- T */
		{ 31950, "UI/Skill/Artist/31950_DragonEngraving.png" },
		/* Artist -- V (awakening) */
		{ 31910, "UI/Skill/Artist/31910_DreamPeachGarden.png" },
		/* DimensionMaster */
		{ 2050100, "UI/Skill/DimensionMaster/2050100_OneNeedle.png" },
		{ 2050120, "UI/Skill/DimensionMaster/2050120_Fragment.png" },
		{ 2050160, "UI/Skill/DimensionMaster/2050160_CrossThrust.png" },
		{ 2050180, "UI/Skill/DimensionMaster/2050180_BeyondSlash.png" },
		{ 2050210, "UI/Skill/DimensionMaster/2050210_LightSplit.png" },
		{ 2050220, "UI/Skill/DimensionMaster/2050220_PointPierce.png" },
		{ 2050240, "UI/Skill/DimensionMaster/2050240_BoundaryBreak.png" },
		{ 2050230, "UI/Skill/DimensionMaster/2050230_TimeShatter.png" },
		/* DimensionMaster -- T */
		{ 2050500, "UI/Skill/DimensionMaster/2050500_KarmaBoundary.png" },
		/* DimensionMaster -- V (awakening) */
		{ 2050520, "UI/Skill/DimensionMaster/2050520_TimeShackles.png" },
	};

	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V" };

	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const f32_t fScaleX = pViewport->WorkSize.x / REF_WIDTH;
	const f32_t fScaleY = pViewport->WorkSize.y / REF_HEIGHT;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	ID3D11ShaderResourceView* pEmptySlotSRV =
		m_pHUDRuntimeView->Load_Texture("UI/HUD/Common/Empty Slot.png");

	for (const char* pInputSlot : INPUT_SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect(
			string("Skill_") + pInputSlot, fX, fY, fWidth, fHeight))
		{
			continue;
		}

		const ImVec2 vTopLeft(
			pViewport->WorkPos.x + fX * fScaleX,
			pViewport->WorkPos.y + fY * fScaleY);
		const ImVec2 vBotRight(
			vTopLeft.x + fWidth * fScaleX,
			vTopLeft.y + fHeight * fScaleY);

		const char* pIconPath = nullptr;
		if (const PLAYER_SKILL_DEFINITION* pSkill = CPlayerSkillCatalog::Find_BySlot(
			player.eCharacterClass, pInputSlot, player.eStance))
		{
			for (const SKILL_ICON_ENTRY& Entry : SKILL_ICON_TABLE)
			{
				if (Entry.iSkillId == pSkill->iSkillId)
				{
					pIconPath = Entry.pIconPath;
					break;
				}
			}
		}

		/* Icon first, then the shared frame's border/tab back on top -- the shared "Skill_Q"
		slot already drew that same border underneath before this runs, so redrawing it here
		is what keeps it above the icon instead of the icon covering it. */
		if (nullptr != pIconPath)
		{
			if (ID3D11ShaderResourceView* pIconSRV = m_pHUDRuntimeView->Load_Texture(pIconPath))
				pDrawList->AddImage(pIconSRV, vTopLeft, vBotRight);
		}
		if (nullptr != pEmptySlotSRV)
			pDrawList->AddImage(pEmptySlotSRV, vTopLeft, vBotRight);
	}
}

void CMainApp::RenderQuickSlot()
{
	/* Icon art, slot frame, keybind label, and cooldown sweep aren't extracted from QuickSlot.gfx
	yet, so this only draws the real on-use flash -- RenderSkillIcons/RenderSkillCooldowns (called
	alongside this, not instead of it) still own everything else. */

	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT))
	{
		return;
	}

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || nullptr == m_pHUDRuntimeView)
		return;

	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	if (skillWindowOpen)
		return;

	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V" };

	uint32_t iSlotIndex = 0;
	for (const char* pInputSlot : INPUT_SLOTS)
	{
		bool_t bIsReady = true;
		for (const HUD_SKILL_STATE& Skill : player.Skills)
		{
			if (Skill.strInputSlot == pInputSlot)
			{
				bIsReady = Skill.Is_Ready(player.iServerTick);
				break;
			}
		}

		/* Ready-to-not-ready is the only reliable "used just now" signal -- see the member
		comment on m_bPreviousQuickSlotReady for why comparing raw iCooldownEndTick doesn't work. */
		if (m_bPreviousQuickSlotReady[iSlotIndex] && !bIsReady)
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(
				string("Skill_") + pInputSlot + "_Flash", "flash");
		}
		m_bPreviousQuickSlotReady[iSlotIndex] = bIsReady;
		++iSlotIndex;
	}
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
		const wstring hp = std::to_wstring(player.iCurrentHp) +
			L" / " + std::to_wstring(player.iMaximumHp);
		const wstring mana = std::to_wstring(player.iCurrentResource) +
			L" / " + std::to_wstring(player.iMaximumResource);
		/* Positions/size follow the same 0.75 anchor-scale (around 673.675, 747.092) and -12
		vertical shift applied to the whole bottom HUD in HUD_Layout.json -- these two labels
		are drawn here in C++, not from that JSON, so they need the same transform by hand or
		they drift off the now-smaller HP/mana bars. */
		CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), hp.c_str(),
			position(504.419f, 635.273f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.315f * textScale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), mana.c_str(),
			position(835.169f, 635.273f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.315f * textScale);
	}
	/* Boss HP number/name/grade text moved into RenderBossHealthBar() -- the decompiled
	targetstatus_loc_int.gfx places them relative to the bar's own real position (see that
	function), not this hardcoded (640, 58). */
}

void CMainApp::RenderDamageNumbers()
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

	constexpr f64_t DAMAGE_NUMBER_LIFETIME_SECONDS = 1.1;
	constexpr size_t MAX_FLOATING_DAMAGE_NUMBERS = 48u;

	/* Get_DamageEvents() keeps every retained hit, not just this frame's -- only spawn a floating
	number for events strictly newer than the last batch we already spawned from. See the member
	comment on m_iLastRenderedDamageServerTick for why a serverTick cursor is safe here even though
	the buffer trims from the front. */
	const std::vector<HUD_DAMAGE_EVENT>& damageEvents =
		CCombatHUDViewModel::Get().Get_DamageEvents();
	for (const HUD_DAMAGE_EVENT& damageEvent : damageEvents)
	{
		if (damageEvent.iServerTick <= m_iLastRenderedDamageServerTick)
			continue;
		FLOATING_DAMAGE_NUMBER number{};
		number.dSpawnSeconds = ImGui::GetTime();
		number.vWorldPosition = float3_t(
			damageEvent.Event.fPositionX,
			damageEvent.Event.fPositionY,
			damageEvent.Event.fPositionZ);
		number.iAmount = damageEvent.Event.iAmount;
		number.isOutgoing = damageEvent.Event.isOutgoing;
		m_FloatingDamageNumbers.push_back(number);
		m_iLastRenderedDamageServerTick = damageEvent.iServerTick;
	}
	if (m_FloatingDamageNumbers.size() > MAX_FLOATING_DAMAGE_NUMBERS)
	{
		m_FloatingDamageNumbers.erase(
			m_FloatingDamageNumbers.begin(),
			m_FloatingDamageNumbers.begin() +
				(m_FloatingDamageNumbers.size() - MAX_FLOATING_DAMAGE_NUMBERS));
	}

	const f64_t dNow = ImGui::GetTime();
	m_FloatingDamageNumbers.erase(
		std::remove_if(m_FloatingDamageNumbers.begin(), m_FloatingDamageNumbers.end(),
			[dNow](const FLOATING_DAMAGE_NUMBER& number)
			{
				return dNow - number.dSpawnSeconds >= DAMAGE_NUMBER_LIFETIME_SECONDS;
			}),
		m_FloatingDamageNumbers.end());
	if (m_FloatingDamageNumbers.empty())
		return;

	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	if (viewportSize.x <= 0.f || viewportSize.y <= 0.f)
		return;
	const matrix_t view = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW));
	const matrix_t projection = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ));
	const f32_t textScale = (std::min)(viewportSize.x / 1280.f, viewportSize.y / 720.f);

	for (const FLOATING_DAMAGE_NUMBER& number : m_FloatingDamageNumbers)
	{
		const f32_t fLifeRatio = (std::clamp)(
			static_cast<f32_t>((dNow - number.dSpawnSeconds) / DAMAGE_NUMBER_LIFETIME_SECONDS),
			0.f, 1.f);
		/* Rises above the real hit position and fades out over the back half of its lifetime,
		instead of sitting flat on the hit point the whole time. */
		const vector_t vProjected = XMVector3Project(
			XMVectorSet(
				number.vWorldPosition.x,
				number.vWorldPosition.y + 0.4f + 0.6f * fLifeRatio,
				number.vWorldPosition.z,
				1.f),
			0.f, 0.f, viewportSize.x, viewportSize.y, 0.f, 1.f,
			projection, view, XMMatrixIdentity());
		if (XMVectorGetZ(vProjected) < 0.f || XMVectorGetZ(vProjected) > 1.f)
			continue;
		const f32_t fAlpha = fLifeRatio < 0.6f ? 1.f : 1.f - (fLifeRatio - 0.6f) / 0.4f;
		const wstring strAmount = Format_ThousandsSeparated(number.iAmount);
		const fvector_t vColor = number.isOutgoing ?
			XMVectorSet(1.f, 0.86f, 0.24f, fAlpha) :
			XMVectorSet(1.f, 0.28f, 0.22f, fAlpha);
		CGameInstance::Get().Draw_Text(TEXT("Font_EventDamage"), strAmount.c_str(),
			float2_t(XMVectorGetX(vProjected), XMVectorGetY(vProjected)),
			vColor, 0.f, float2_t(0.5f, 0.5f), 0.6f * textScale);
	}
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
		/* Small 95-glyph ASCII-only subset (no Korean) -- fine for the boss HP number and "X N"
		bar-count text, which are both pure digits/ASCII. User picked this one from the same 8-font
		comparison gallery that settled Font_EventDamage. */
		{ TEXT("Font_159"), L"159.spritefont" },
		/* Was BMKkubulim.spritefont (a casual hand-lettering font, "BM꾸불림체" -- wrong for
		combat numbers), then briefly 159.spritefont. User compared all 8 Resources/Fonts
		candidates rendered side by side and picked YoonGasiIIM. */
		{ TEXT("Font_EventDamage"), L"YoonGasiIIM.spritefont" },
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
		CSkillGroundTargetPreview::SHADER_TAG,
		CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxSkillGroundTargetPreview.hlsl"),
			VTXTEX::Elements,
			VTXTEX::iNumElements))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
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
	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC),
		CSkillGroundTargetPreview::PROTOTYPE_TAG,
		CSkillGroundTargetPreview::Create(m_pDevice, m_pContext))))
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

	/* Valtan Arena's own loading background: Level_Loading::Ready_Layer_Chrome swaps the
	Background slot's texture path to this one at runtime (LEVEL::VALTAN_ARENA only), so it
	never appears in LoadingLayout.json's own layers and the scan above never finds it --
	register it explicitly or its CUI_Sprite clone fails to find a texture prototype. */
	{
		constexpr wchar_t VALTAN_LOADING_BACKGROUND[] = L"UI/Loading/Loading_Background_Valtan.png";
		const filesystem::path resolvedPath =
			CRuntimeAssetRoot::Resolve(VALTAN_LOADING_BACKGROUND);
		if (!resolvedPath.empty() &&
			FAILED(CGameInstance::Get().Add_Prototype(
				ETOUI(LEVEL::STATIC), VALTAN_LOADING_BACKGROUND,
				CTexture::Create(m_pDevice, m_pContext, resolvedPath.c_str(), 1))))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMainApp::Start_Level(
	const LEVEL eTargetLevel,
	const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	/* The P-toggled Item Upgrade debug preview (see m_pItemUpgradeView's declaration comment)
	has no level awareness of its own -- it just draws whenever the flag is on. Without this,
	leaving Character Select with the preview open (e.g. entering Valtan) left its text drawing
	over the Loading screen and the destination level too. Any real level transition ends it. */
	m_bItemUpgradePreviewVisible = false;

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
			CCharacterSelectionState::Cancel_PendingCreation();
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
	if (!hasTargetProfile)
	{
		OutputDebugStringA(
			"[MainApp] Activation target has no registered rendering profile.\n");
	}
	unique_ptr<CLevel> nextLevel = hasTargetProfile ?
		CLevelRegistry::Create_Level(
			request.eTargetLevel,
			m_pDevice,
			m_pContext) : nullptr;
	if (hasTargetProfile && nullptr == nextLevel)
	{
		OutputDebugStringA(
			"[MainApp] Create_Level returned null (target Level::Initialize failed).\n");
	}
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
	const bool_t levelChanged = profileActivated &&
		SUCCEEDED(CGameInstance::Get().Change_Level(
			ETOUI(request.eTargetLevel), move(nextLevel)));
	if (levelChanged)
	{
#ifdef _DEBUG
		if (nullptr != m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel->On_LevelChanged();
#endif
		CEffectPresentationService::Clear_Level(iPreviousLevel);
		if (LEVEL::BERN == request.eTargetLevel &&
			CCharacterSelectionState::Has_PendingCreation() &&
			!CCharacterSelectionState::Commit_PendingCreation())
		{
			OutputDebugStringA(
				"[MainApp] Bern identity commit invariant failed.\n");
			CNetworkManager::Get().Close_ServerConnection();
			CLevelTransitionService::Report_LoadFailure(E_FAIL);
			if (!CLevelTransitionService::Request_Load(
				LEVEL::LOBBY,
				"main-app.identity-commit-failure"))
			{
				OutputDebugStringA(
					"[MainApp] Failed to stage Lobby after identity commit failure.\n");
			}
			return;
		}
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

	CCharacterSelectionState::Cancel_PendingCreation();
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
				make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
		if (nullptr == m_pAnimationTool)
			m_pAnimationTool = make_unique<CAnimation_Tool>(
				m_pCharacterPreviewPanel);
		break;
	case DEBUG_TOOL::EFFECT:
		if (nullptr == m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel =
				make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
		if (nullptr == m_pEffectTool)
			m_pEffectTool =
				make_unique<CEffect_Tool>(
					m_pDevice, m_pContext, m_pCharacterPreviewPanel);
		break;
	case DEBUG_TOOL::EFFECT_V2:
		if (nullptr == m_pEffectToolV2)
			m_pEffectToolV2 =
				make_unique<CEffect_Tool_V2>(m_pDevice, m_pContext);
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
	toolButton("Effect Tool v2", DEBUG_TOOL::EFFECT_V2, true);
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
	ImGui::SeparatorText("Inventory (Debug)");
	const std::vector<Client::ITEM_DEFINITION>& debugItems =
		Client::CItemCatalog::Get_Items();
	CNetworkManager& debugNetworkManager = CNetworkManager::Get();
	const bool_t canGiveItem = !debugItems.empty() &&
		debugNetworkManager.Is_Connected() &&
		LostArk::Shared::INVALID_PLAYER_ID !=
			debugNetworkManager.Get_LocalPlayerId();
	if (debugItems.empty())
	{
		ImGui::TextDisabled("Item catalog failed to load.");
	}
	else
	{
		if (m_iSelectedDebugItemIndex >=
			static_cast<int32_t>(debugItems.size()))
		{
			m_iSelectedDebugItemIndex = 0;
		}
		const Client::ITEM_DEFINITION& selectedItem =
			debugItems[m_iSelectedDebugItemIndex];
		if (ImGui::BeginCombo("Item", selectedItem.strDisplayName.c_str()))
		{
			for (int32_t index = 0;
				index < static_cast<int32_t>(debugItems.size()); ++index)
			{
				const bool_t isSelected = index == m_iSelectedDebugItemIndex;
				if (ImGui::Selectable(
					debugItems[index].strDisplayName.c_str(), isSelected))
				{
					m_iSelectedDebugItemIndex = index;
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::BeginDisabled(!canGiveItem);
		if (ImGui::Button("Give"))
		{
			if (debugNetworkManager.Send_DebugGiveItem(
				m_iNextDebugGiveItemSequence,
				selectedItem.strItemId,
				1u))
			{
				m_strDebugItemStatus = "Requested " + selectedItem.strDisplayName;
				++m_iNextDebugGiveItemSequence;
			}
			else
			{
				m_strDebugItemStatus = "Give item request failed to send.";
			}
		}
		ImGui::EndDisabled();
		if (!canGiveItem)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("Connect and enter a world first.");
		}
		if (!m_strDebugItemStatus.empty())
			ImGui::TextDisabled("%s", m_strDebugItemStatus.c_str());
	}

	ImGui::TextUnformatted("Current inventory (Server truth):");
	const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& debugInventory =
		Client::CCombatHUDViewModel::Get().Get_Inventory();
	if (debugInventory.Items.empty())
	{
		ImGui::TextDisabled("(empty)");
	}
	else
	{
		for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item :
			debugInventory.Items)
		{
			const Client::ITEM_DEFINITION* definition =
				Client::CItemCatalog::Find_ById(item.strItemId);
			ImGui::Text(
				"%s x%u",
				nullptr != definition ?
					definition->strDisplayName.c_str() : item.strItemId.c_str(),
				item.iQuantity);
		}
	}

	if (ImGui::CollapsingHeader("Esther Cutin (Debug)"))
	{
		ESTHER_CUTIN_TUNING& cutinTuning =
			CEstherCutinPresentationService::Debug_Tuning();
		ImGui::DragFloat("Model Yaw (deg)",
			&cutinTuning.fModelYawDegrees, 1.f, -360.f, 360.f);
		ImGui::DragFloat("Eye X / Height",
			&cutinTuning.fEyeXPerHeight, 0.01f, -2.f, 2.f);
		ImGui::DragFloat("Eye Y / Height",
			&cutinTuning.fEyeYPerHeight, 0.01f, -1.f, 3.f);
		ImGui::DragFloat("Distance / Height",
			&cutinTuning.fDistancePerHeight, 0.02f, 0.3f, 6.f);
		ImGui::DragFloat("Target Y / Height",
			&cutinTuning.fAtYPerHeight, 0.01f, 0.f, 2.f);
		ImGui::DragFloat("FOV (deg)",
			&cutinTuning.fFovDegrees, 0.5f, 10.f, 90.f);
		ImGui::DragFloat4("Rect X/Y/W/H (720p)",
			&cutinTuning.fRectX, 2.f, -400.f, 1600.f);
		if (ImGui::Button("Reset Tuning"))
			CEstherCutinPresentationService::Debug_ResetTuning();
		ImGui::TextDisabled(
			"Preview replays the cutin only; prototypes must be loaded"
			" (enter Valtan first).");
		const auto previewButton = [](
			const char_t* pLabel, const char_t* pArchetypeId)
		{
			if (ImGui::Button(pLabel))
				CEstherCutinPresentationService::Debug_Preview(pArchetypeId);
		};
		previewButton("Preview Sillian", "NPC_59030");
		ImGui::SameLine();
		previewButton("Preview Wei", "NPC_58700");
		ImGui::SameLine();
		previewButton("Preview Bahuntur", "NPC_59060");
		ImGui::Text(
			"yaw %.1f  eye(%.2f, %.2f)  dist %.2f  target %.2f  fov %.1f",
			cutinTuning.fModelYawDegrees,
			cutinTuning.fEyeXPerHeight,
			cutinTuning.fEyeYPerHeight,
			cutinTuning.fDistancePerHeight,
			cutinTuning.fAtYPerHeight,
			cutinTuning.fFovDegrees);
		ImGui::Text(
			"rect (%.0f, %.0f, %.0f, %.0f)",
			cutinTuning.fRectX,
			cutinTuning.fRectY,
			cutinTuning.fRectWidth,
			cutinTuning.fRectHeight);
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
	m_pEffectToolV2.reset();
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
