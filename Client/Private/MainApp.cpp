#include "imgui.h"

#include "MainApp.h"

#include "CharacterSelectionState.h"
#include "ChatWindowView.h"
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
#include "PartyWindowView.h"
#include "PlayerSkillCatalog.h"
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
#include <cmath>
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
	CGameInstance::Get().Update_Engine(fTimeDelta);
	CEffectPresentationService::Commit_PendingSpawns();
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
	}

	/* Disabled for now -- every attempt at the 3-segment gauge visual (procedural arcs, then the
	real track art) has landed wrong (bad radius/position, missing tint) and needs a proper
	in-game reference to get right rather than another guess. RenderLanceMasterIdentityGauge stays
	defined, unchanged, for whenever that reference is available. */
	// if (LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER == player.eCharacterClass)
	//	RenderLanceMasterIdentityGauge();

	if (nullptr != m_pSkillWindowView)
		m_pSkillWindowView->Render(player.eCharacterClass);
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

	/* No procedural track/fill arc here anymore -- a prior pass drew one using each track piece's
	own long-axis pixel length (44.25/48) as an ImGui PathArcTo *radius*, which is wrong: those
	pieces are thin curved strips (their real footprint is 12.75x44.25 / 48x12 reference px, drawn
	as-is by the real Lance_Id_GaugeTrack0/1/2 slots below), not a description of some much larger
	circle. Using that length as a radius instead drew a circle nearly as wide as the whole icon --
	the "two huge curves" this replaced. The AS3-driven fill ("target", the piece that actually
	moves as value climbs 0->100) is confirmed vector-only, no extractable bitmap and no baked
	track/fill *color* data either, so it is not redrawn here at all rather than guessed again;
	real reference footage is needed to fill this back in correctly. */
	for (int32_t i = 0; i < 3; ++i)
	{
		/* Real extracted gauge0/1/2 highLightMc flourish (Data/.../Gauge0/1/2Burn.json, baked from
		lancemaster_identity.xml sprite386/438/368) -- the orange/yellow "this segment just filled"
		burn, not an invented effect. Triggered once on the empty->full edge, same pattern as
		RenderQuickSlot's on-use flash. */
		const bool_t bIsFull = fSegmentValue[i] >= SEGMENT_MAX;
		if (bIsFull && !m_bLanceGaugeSegmentWasFull[i])
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(
				string("Lance_Id_GaugeBurn") + std::to_string(i), "burn");
		}
		m_bLanceGaugeSegmentWasFull[i] = bIsFull;
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
	const float healthRatio = (std::clamp)(
		static_cast<float>(boss.iCurrentHp) /
		static_cast<float>(boss.iMaximumHp), 0.f, 1.f);
	const ImVec2 barMin{
		pViewport->WorkPos.x + 360.f * scaleX,
		pViewport->WorkPos.y + 30.f * scaleY };
	const ImVec2 barMax{
		pViewport->WorkPos.x + 920.f * scaleX,
		pViewport->WorkPos.y + 50.f * scaleY };
	const float fillRight = barMin.x +
		(barMax.x - barMin.x) * healthRatio;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);
	pDrawList->AddRectFilled(
		barMin, barMax, IM_COL32(24, 24, 28, 230), 3.f * uiScale);
	if (fillRight > barMin.x)
	{
		pDrawList->AddRectFilled(
			barMin,
			ImVec2(fillRight, barMax.y),
			IM_COL32(176, 34, 40, 255),
			3.f * uiScale);
	}
	pDrawList->AddRect(
		barMin, barMax, IM_COL32(224, 208, 176, 255),
		3.f * uiScale, 0, (std::max)(1.f, uiScale));
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
		/* LanceMaster -- Short Spear (no D/F skill in that stance) */
		{ 34540, "UI/Skill/LanceMaster/34540_SpiralingSpear.png" },
		{ 34550, "UI/Skill/LanceMaster/34550_4HeadedDragon.png" },
		{ 34560, "UI/Skill/LanceMaster/34560_ThrustOfDestruction.png" },
		{ 34570, "UI/Skill/LanceMaster/34570_StarfallPounce.png" },
		{ 34580, "UI/Skill/LanceMaster/34580_DragonscaleDefense.png" },
		{ 34590, "UI/Skill/LanceMaster/34590_RedDragonsHorn.png" },
		/* Warlord */
		{ 17030, "UI/Skill/Warlord/17030_SharpSpear.png" },
		{ 17060, "UI/Skill/Warlord/17060_FireBullet.png" },
		{ 17080, "UI/Skill/Warlord/17080_DashUpperFire.png" },
		{ 17110, "UI/Skill/Warlord/17110_LeapAttack.png" },
		{ 17090, "UI/Skill/Warlord/17090_HookChain.png" },
		{ 17040, "UI/Skill/Warlord/17040_Bash.png" },
		{ 17100, "UI/Skill/Warlord/17100_ShieldShock.png" },
		{ 17140, "UI/Skill/Warlord/17140_GuardiansLightning.png" },
		/* Artist */
		{ 31200, "UI/Skill/Artist/31200_InkShower.png" },
		{ 31430, "UI/Skill/Artist/31430_Scatter.png" },
		{ 31480, "UI/Skill/Artist/31480_CraneWings.png" },
		{ 31210, "UI/Skill/Artist/31210_Kongkongi.png" },
		{ 31460, "UI/Skill/Artist/31460_ButterflyDream.png" },
		{ 31420, "UI/Skill/Artist/31420_OrchidStrike.png" },
		{ 31490, "UI/Skill/Artist/31490_TigerSlash.png" },
		{ 31470, "UI/Skill/Artist/31470_OneStroke.png" },
		/* DimensionMaster */
		{ 2050100, "UI/Skill/DimensionMaster/2050100_OneNeedle.png" },
		{ 2050120, "UI/Skill/DimensionMaster/2050120_Fragment.png" },
		{ 2050160, "UI/Skill/DimensionMaster/2050160_CrossThrust.png" },
		{ 2050180, "UI/Skill/DimensionMaster/2050180_BeyondSlash.png" },
		{ 2050210, "UI/Skill/DimensionMaster/2050210_LightSplit.png" },
		{ 2050220, "UI/Skill/DimensionMaster/2050220_PointPierce.png" },
		{ 2050240, "UI/Skill/DimensionMaster/2050240_BoundaryBreak.png" },
		{ 2050230, "UI/Skill/DimensionMaster/2050230_TimeShatter.png" },
	};

	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F" };

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

	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F" };

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
		const wstring hp = L"HP  " + std::to_wstring(player.iCurrentHp) +
			L" / " + std::to_wstring(player.iMaximumHp);
		const wstring mana = L"MANA  " + std::to_wstring(player.iCurrentResource) +
			L" / " + std::to_wstring(player.iMaximumResource);
		/* Positions/size follow the same 0.75 anchor-scale (around 673.675, 747.092) and -12
		vertical shift applied to the whole bottom HUD in HUD_Layout.json -- these two labels
		are drawn here in C++, not from that JSON, so they need the same transform by hand or
		they drift off the now-smaller HP/mana bars. */
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), hp.c_str(),
			position(504.419f, 635.273f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.315f * textScale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), mana.c_str(),
			position(835.169f, 635.273f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.315f * textScale);
	}
	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!boss.isValid || 0u == boss.iMaximumHp)
	{
		return;
	}

	const wstring hp = std::to_wstring(boss.iCurrentHp) + L" / " +
		std::to_wstring(boss.iMaximumHp);
	CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), hp.c_str(),
		position(640.f, 58.f), Colors::White, 0.f,
		float2_t(0.5f, 0.5f), 0.42f * textScale);
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
