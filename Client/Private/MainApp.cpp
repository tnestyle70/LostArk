#include "imgui.h"

#include "MainApp.h"

#include "CharacterSelectionState.h"
#include "CharacterSelectWindowView.h"
#include "MinimapView.h"
#include "ChatWindowView.h"
#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "EstherCutinPresentationService.h"
#include "RaidBossShowcaseService.h"
#include "EstherActionSoundCueDocument.h"
#include "Effect_Object.h"
#include "Effect_PresentationService.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "ImGuiLayer.h"
#include "ItemCatalog.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "Level_Bern.h"
#include "Level_Lobby.h"
#include "ActionPresentationTimeline.h"
#include "Level_CharacterSelect.h"
#include "Level_KakulSaydonArena.h"
#include "Level_Loading.h"
#include "Level_ValtanArena.h"
#include "LobbyCommandService.h"
#include "NetworkManager.h"
#include "PartyWindowView.h"
#include "PlayerSkillCatalog.h"
#include "Profiler.h"
#include "Presentation_Manager.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "AvatarBookWindowView.h"
#include "UILabelFont.h"
#include "CharacterInfoWindowView.h"
#include "InventoryView.h"
#include "SkillWindowView.h"
#include "SkillGroundTargetPreview.h"
#include "ClickMoveEffect.h"
#include "SoundCueCatalog.h"
#include "UI_Sprite.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#ifdef _DEBUG
#include "Animation_Tool.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonBossTool.h"
#include "KoukuSaydonPatternAuditionService.h"
#include "ValtanActionWorkbench.h"
#include "BalanceTool.h"
#include "ValtanBossTool.h"
#include "CameraTool.h"
#include "Camera_Free.h"
#include "CharacterPreviewPanel.h"
#include "ClientReplication.h"
#include "Effect_Tool.h"
#include "Effect_Tool_V2.h"
#include "EquipmentAuthoringTool.h"
#include "HUDLayoutTool.h"
#include "MapEditorWorkspaceService.h"
#include "MapTool.h"
#include "NetworkPlayerCommandSink.h"
#include "ProfilerCaptureIO.h"
#include "ProfilerTool.h"
#include "RenderingBenchmark.h"
#include "SequencerTool.h"
#include "ValtanPatternAuditionService.h"
#include "ValtanPatternFlowService.h"
#include "ValtanTuningCommandService.h"
#endif

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <random>
#include <tuple>
#include <cwchar>
#include <fstream>

namespace
{
	/* Product-path wall clock (seconds since first call) -- replaces ImGui::GetTime() in every
	non-Debug timer here, so no product state machine depends on the ImGui frame loop. */
	f64_t Product_Now_Seconds()
	{
		static const std::chrono::steady_clock::time_point s_Epoch =
			std::chrono::steady_clock::now();
		return std::chrono::duration<f64_t>(
			std::chrono::steady_clock::now() - s_Epoch).count();
	}

	/* Retail login/server-select screen (EFUI_LOBBY login.gfx, Server_Login group) rebuilt on
	the Lobby: server rows + 접속(서버 선택) button + 종료/뒤로/환경설정 icon buttons, slots
	authored in Lobby_Layout.json at 1280x720 (retail 1920x1080 coordinates x 2/3). The row list
	itself comes from Data/UI/Lobby/LobbyServers.json (name/state/tag/count are the user's
	choice, not Server data -- this project has no account/server directory). */
	constexpr size_t LOBBY_SERVER_ROW_COUNT = 8;
	/* 37-frame retail logo reveal (total_sequence_00001..00073, every other frame of a 40fps
	timeline) at 20 fps, then the static Logo_Final image takes over. */
	constexpr f32_t LOBBY_LOGO_INTRO_SECONDS = 37.f / 20.f;

	struct LOBBY_PRODUCT_RECT final
	{
		f32_t fX = 0.f;
		f32_t fY = 0.f;
		f32_t fWidth = 0.f;
		f32_t fHeight = 0.f;
	};

	bool_t Is_ValidProductRect(const LOBBY_PRODUCT_RECT& Rect)
	{
		return std::isfinite(Rect.fX) && std::isfinite(Rect.fY) &&
			std::isfinite(Rect.fWidth) && std::isfinite(Rect.fHeight) &&
			Rect.fWidth > 0.f && Rect.fHeight > 0.f;
	}

	bool_t Get_LobbySlotRect(Client::CUILayoutRuntime* pView, const char* pSlotId,
		LOBBY_PRODUCT_RECT& outRect)
	{
		return nullptr != pView && pView->Get_SlotRect(pSlotId,
			outRect.fX, outRect.fY, outRect.fWidth, outRect.fHeight) &&
			Is_ValidProductRect(outRect);
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

	/* Shared by RenderItemUpgradeListText (left-list rows), RenderItemUpgradeLevelText (the big
	"selected item" name label), Update_ItemUpgradeSelection (click-to-select + icon swap), and
	the success/fail detail text so the id/name/icon triple has exactly one source. Built fresh
	from the real replicated inventory each time it's needed (cheap in-memory filter, same cost
	class as CInventoryView::Update's own per-frame rebuild) rather than cached, so a fresh
	S2C_INVENTORY_SNAPSHOT (e.g. right after the Valtan clear rewards land) is reflected the very
	next frame with no separate invalidation path. */
	struct ITEM_UPGRADE_SLOT_INFO
	{
		string strItemId;
		wstring strName;
		string strIconPath;
	};

	bool_t ConvertUtf8ToWide(const string& strUtf8, wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return false;
		const int iRequiredLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), nullptr, 0);
		if (iRequiredLength <= 0)
			return false;
		outWide.resize(static_cast<size_t>(iRequiredLength));
		return iRequiredLength == MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), outWide.data(), iRequiredLength);
	}

	/* Only the "combat" (equipment) category slice of the real inventory -- consumables/materials
	/currency ("use") never belong in a 재련 list. Order follows S2C_INVENTORY_SNAPSHOT's own
	item order (Server-assigned on Grant_Item), not an authored literal order. */
	vector<ITEM_UPGRADE_SLOT_INFO> BuildItemUpgradeSlots()
	{
		vector<ITEM_UPGRADE_SLOT_INFO> slots;
		for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item :
			Client::CCombatHUDViewModel::Get().Get_Inventory().Items)
		{
			const ITEM_DEFINITION* pDefinition = CItemCatalog::Find_ById(item.strItemId);
			if (nullptr == pDefinition || "combat" != pDefinition->strCategory)
				continue;
			wstring strName;
			if (!ConvertUtf8ToWide(pDefinition->strDisplayName, strName))
				continue;
			ITEM_UPGRADE_SLOT_INFO info{};
			info.strItemId = pDefinition->strItemId;
			info.strName = std::move(strName);
			info.strIconPath = pDefinition->strIconPath;
			slots.push_back(std::move(info));
		}
		return slots;
	}

	/* Every slot ItemUpgradeUI.json authors, for Hide_ItemUpgrade (all false) and
	Open_ItemUpgradeWindow's own initial "show everything" pass (all true, before the existing
	explicit hides for the completion-effect/modal/result slots that must start hidden run on top
	of it) -- CUILayoutRuntime has no generic Render(class, revision) pass to fall back on the way
	CHUDRuntimeView did, so every one of these 97 authored slots needs an explicit visibility
	owner now instead of an implicit "wasn't drawn this frame". */
	constexpr const char_t* ITEM_UPGRADE_ALL_SLOTS[] =
	{
		"ItemUpgrade_SuccessModalBg", "ItemUpgrade_PanelBg", "ItemUpgrade_RecipeIconBgExample",
		"ItemUpgrade_RecipeMaterial0", "ItemUpgrade_RecipeAmount0", "ItemUpgrade_RecipeIconBg1",
		"ItemUpgrade_RecipeMaterial1", "ItemUpgrade_RecipeAmount1", "ItemUpgrade_RecipeIconBg2",
		"ItemUpgrade_RecipeMaterial2", "ItemUpgrade_RecipeAmount2", "ItemUpgrade_DecoIcon",
		"ItemUpgrade_LevelUpMotion2Big", "ItemUpgrade_GaugeFill", "ItemUpgrade_WingedRingGold",
		"ItemUpgrade_SelectedItemIconBounds", "ItemUpgrade_LevelUpBtn", "ItemUpgrade_EquipExpPageLine",
		"ItemUpgrade_SmeltGlow", "ItemUpgrade_CoreFlash", "ItemUpgrade_ShockwaveRing",
		"ItemUpgrade_CompleteEffect", "ItemUpgrade_WingDecoFade", "ItemUpgrade_LeftListBg",
		"ItemUpgrade_ListGradeBg0", "ItemUpgrade_SelectedItemGradeBg", "ItemUpgrade_SelectedItemIcon",
		"ItemUpgrade_ItemNameLabel", "ItemUpgrade_CurLevelLabel", "ItemUpgrade_NextLevelLabel",
		"ItemUpgrade_ListGradeBg1", "ItemUpgrade_ListGradeBg2", "ItemUpgrade_ListGradeBg3",
		"ItemUpgrade_RightGradeListBg", "ItemUpgrade_GradeRowEmblem0", "ItemUpgrade_GradeRowEmblem1",
		"ItemUpgrade_GradeRowEmblem2", "ItemUpgrade_GradeRowEmblem3", "ItemUpgrade_GradeRowEmblem4",
		"ItemUpgrade_GradeRowEmblem5", "ItemUpgrade_GradeRowEmblem6", "ItemUpgrade_GradeStripB0",
		"ItemUpgrade_GradeStripB1", "ItemUpgrade_ListSelectedExample", "ItemUpgrade_GradeStripB2",
		"ItemUpgrade_GradeStripB6", "ItemUpgrade_GradeStripB5", "ItemUpgrade_GradeStripB4",
		"ItemUpgrade_GradeStripB3", "ItemUpgrade_GradeSelectedExample", "ItemUpgrade_ListItemIcon0",
		"ItemUpgrade_ReforgeButton", "ItemUpgrade_ListLevel0", "ItemUpgrade_ListItemName0",
		"ItemUpgrade_ListLevel1", "ItemUpgrade_ListItemName1", "ItemUpgrade_ListLevel2",
		"ItemUpgrade_ListItemName2", "ItemUpgrade_ListLevel3", "ItemUpgrade_ListItemName3",
		"ItemUpgrade_ListLevel4", "ItemUpgrade_ListItemName4", "ItemUpgrade_ListLevel5",
		"ItemUpgrade_ListItemName5", "ItemUpgrade_GradeRowText6", "ItemUpgrade_GradeRowText0",
		"ItemUpgrade_GradeRowText5", "ItemUpgrade_GradeRowText1", "ItemUpgrade_GradeRowText4",
		"ItemUpgrade_GradeRowText2", "ItemUpgrade_GradeRowText3", "ItemUpgrade_ListItemIcon1",
		"ItemUpgrade_ListItemIcon2", "ItemUpgrade_ListItemIcon3", "ItemUpgrade_ListItemIcon4",
		"ItemUpgrade_ListGradeBg4", "ItemUpgrade_ListGradeBg5", "ItemUpgrade_LevelArrowBase",
		"ItemUpgrade_LevelArrow", "ItemUpgrade_ListItemIcon5", "ItemUpgrade_FailModalBg",
		"ItemUpgrade_ResultWaitBg", "ItemUpgrade_ResultWaitEmblem", "ItemUpgrade_SuccessEffect",
		"ItemUpgrade_FailEffect", "ItemUpgrade_SuccessDiamondWinged", "ItemUpgrade_SuccessDiamondFrame",
		"ItemUpgrade_SuccessItemIconMarker", "ItemUpgrade_SuccessItemNameMarker",
		"ItemUpgrade_SuccessGradeMarker", "ItemUpgrade_SuccessStatusMarker",
		"ItemUpgrade_FailDiamondFrame", "ItemUpgrade_FailItemIconMarker",
		"ItemUpgrade_FailItemNameMarker", "ItemUpgrade_FailStatusMarker",
		"ItemUpgrade_SuccessOkBtn", "ItemUpgrade_FailOkBtn",
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

	/* Server tick deadlines wrap at uint32_t. Keep the HUD's status presentation on the same
	half-range ordering as CGameRoom instead of treating a wrapped future deadline as expired. */
	bool_t Is_ServerDeadlinePending(const uint32_t iServerTick, const uint32_t iDeadlineTick)
	{
		return 0u != iDeadlineTick &&
			static_cast<int32_t>(iServerTick - iDeadlineTick) < 0;
	}

}

CMainApp* CMainApp::s_pActiveInstance = nullptr;

CMainApp::CMainApp()
{
	s_pActiveInstance = this;
}

CMainApp::~CMainApp()
{
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
	Free();
}

void CMainApp::Play_UIButtonClickSound()
{
	const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
		L"Sound/UI/Select/ui_default_button_click2__59426200.wav");
	CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
}

void CMainApp::Open_ItemUpgradeWindow()
{
	if (nullptr == m_pItemUpgradeView || m_bItemUpgradePreviewVisible)
		return;

	m_bItemUpgradePreviewVisible = true;
	/* Show every authored slot first (Hide_ItemUpgrade's own inverse) -- unlike the old
	CHUDRuntimeView generic Render(class, revision) pass, a CUI_Sprite has no implicit
	"wasn't drawn this frame" default, so every slot needs an explicit owner. The explicit hides
	right below (100%-only art, modal/result slots) then apply on top of this, same as before. */
	for (const char_t* pSlotId : ITEM_UPGRADE_ALL_SLOTS)
		m_pItemUpgradeView->Set_SlotVisible(pSlotId, true);

	/* Reopening always starts the gauge idle at 0 -- reset the state machine and hide
	the 100%-only art so a completed run from a previous open doesn't carry over. */
	m_iItemUpgradePreviousPercent = 0;
	m_bItemUpgradeGrowing = false;
	m_dItemUpgradeGrowStartSeconds = -1.0;
	m_bItemUpgradeCoreFlashPending = false;
	m_dItemUpgradeShockwaveScheduledAt = -1.0;
	m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	m_dItemUpgradeResultSettleAt = -1.0;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", false);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", false);
	m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::NONE;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessModalBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessOkBtn", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailModalBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailOkBtn", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessEffect", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailEffect", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondWinged", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondFrame", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessItemIconMarker", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailDiamondFrame", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailItemIconMarker", false);
	Set_ItemUpgradeCenterPanelVisible(true);
}

void CMainApp::Hide_ItemUpgrade()
{
	if (nullptr == m_pItemUpgradeView)
		return;
	for (const char_t* pSlotId : ITEM_UPGRADE_ALL_SLOTS)
		m_pItemUpgradeView->Set_SlotVisible(pSlotId, false);
}

void CMainApp::Update_ItemUpgrade(const f32_t fTimeDelta)
{
	if (nullptr == m_pItemUpgradeView)
		return;

	/* The P key toggle has no level awareness of its own (see m_pItemUpgradeView's declaration
	comment), so m_bItemUpgradePreviewVisible can go true while sitting in a level
	Update_CombatHUD never supported (e.g. Lobby) -- same level set as Update_CombatHUD's own gate.
	Unlike the old ImGui pass (which simply wasn't reached and drew nothing there), these slots
	live under LEVEL::STATIC and would otherwise show through regardless of level. */
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel =
		currentLevel == ETOUI(LEVEL::BERN) ||
		currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
		currentLevel == ETOUI(LEVEL::DEVELOPMENT) ||
		currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) ||
		currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA);
	if (!m_bItemUpgradePreviewVisible || !isSupportedLevel)
	{
		Hide_ItemUpgrade();
		return;
	}

	/* Drives every "animation.frames" flipbook this view owns that ISN'T manually pinned below
	(SmeltGlow's idle loop, CoreFlash/ShockwaveRing/CompleteEffect/WingDecoFade/ResultWaitEmblem/
	SuccessEffect/FailEffect) off real elapsed time -- GaugeFill is the one slot this function
	pins to an exact frame every tick instead (see below), same as CHUDRuntimeView's own
	Set_Animation_Frame override did before this migration. */
	m_pItemUpgradeView->Update(fTimeDelta);

	Update_ItemUpgradeSelection();
	Update_ItemUpgradeGrowButton();
	/* Wait-click checked before Reforge triggers a new WAITING -- both react to the same
	real left-click-down-edge this frame, so if Reforge ran first the very click that opened the
	wait overlay would also satisfy the wait-click's "clicked anywhere" check and reveal on the
	same frame it appeared. */
	Update_ItemUpgradeResultWaitClick();
	Update_ItemUpgradeReforgeButton();
	Update_ItemUpgradeResultOkButton();

	/* No real Server 재련 percent exists yet (see m_pItemUpgradeView's declaration comment), so
	the gauge is a manual state machine driven by ItemUpgrade_LevelUpBtn's click
	(Update_ItemUpgradeGrowButton) instead of a free-running clock. Idle at 0 until clicked;
	0->100 fill plays once per click; holds at 100 until the next click. Once a real gauge value
	exists this should read it the same way Update_LanceMasterIdentityGauge() reads
	player.iCurrentIdentity, not this state. */
	if (m_bItemUpgradeGrowing)
	{
		constexpr f32_t GAUGE_FILL_FPS = 45.f;
		constexpr f32_t GAUGE_FILL_FRAME_COUNT = 100.f;
		const f32_t fCycleSeconds = GAUGE_FILL_FRAME_COUNT / GAUGE_FILL_FPS;
		const f64_t fElapsed = Product_Now_Seconds() - m_dItemUpgradeGrowStartSeconds;
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
			m_dItemUpgradeCompleteRevealStartSeconds = Product_Now_Seconds();
			m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", true);
			m_pItemUpgradeView->Restart_Animation("ItemUpgrade_CompleteEffect");
		}
		else
		{
			m_iItemUpgradePreviousPercent = iPercent;
			/* GaugeFill's own AnimationFrames clock starts independently once this slot's first
			frame plays, so it would otherwise drift out of phase with iPercent (which is
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
				(Product_Now_Seconds() - m_dItemUpgradeCompleteRevealStartSeconds) / REVEAL_FADE_SECONDS,
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
		m_dItemUpgradeShockwaveScheduledAt = Product_Now_Seconds() + CORE_FLASH_DURATION_SECONDS;
		m_bItemUpgradeCoreFlashPending = false;
	}
	if (m_dItemUpgradeShockwaveScheduledAt >= 0.0 &&
		Product_Now_Seconds() >= m_dItemUpgradeShockwaveScheduledAt)
	{
		m_pItemUpgradeView->Restart_Animation("ItemUpgrade_ShockwaveRing");
		m_dItemUpgradeShockwaveScheduledAt = -1.0;
	}
	if (m_dItemUpgradeResultSettleAt >= 0.0 && Product_Now_Seconds() >= m_dItemUpgradeResultSettleAt)
	{
		// Burst's real one-shot duration is over -- swap the circle+burst out for the settled
		// icon/name/result content (RenderItemUpgradeSuccessDetailText/FailDetailText gate on
		// this same "settled" condition -- m_dItemUpgradeResultSettleAt < 0.0 while
		// SUCCESS/FAIL is showing -- so the text appears in lockstep with this reveal).
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", false);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessEffect", false);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailEffect", false);

		const bool_t bSuccess = m_bItemUpgradePendingAttemptSuccess;
		const vector<ITEM_UPGRADE_SLOT_INFO> upgradeSlots = BuildItemUpgradeSlots();
		const bool_t bHasSelection = !upgradeSlots.empty();
		const int32_t iSelectedSlot = bHasSelection ? std::clamp(
			m_iItemUpgradeSelectedSlot, 0, static_cast<int32_t>(upgradeSlots.size()) - 1) : 0;
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessOkBtn", bSuccess);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailOkBtn", !bSuccess);
		// Real success_mc/fail_mc detail: a decorative frame + item icon sit behind the settled
		// result text (real local placements traced from ItemBuildUpLevelWndContent's own
		// success_mc/fail_mc timelines). Real in-game capture shows just icon/name/result -- no
		// wide winged ribbon banner -- so SuccessDiamondWinged is never shown. Same frame for
		// both outcomes now (SuccessDiamondFrame reused for fail too) -- FailDiamondFrame is
		// never shown.
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondFrame", true);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessItemIconMarker", bSuccess);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailItemIconMarker", !bSuccess);
		if (bHasSelection)
		{
			if (bSuccess)
			{
				// Same real icon already shown in the base window's ItemUpgrade_SelectedItemIcon --
				// the item being reforged doesn't change just because the result modal is up.
				m_pItemUpgradeView->Set_SlotTexture(
					"ItemUpgrade_SuccessItemIconMarker", upgradeSlots[iSelectedSlot].strIconPath);
				// The actual level-up: a real 재련 success raises this item's own tracked level by
				// 1, so the left list / right ladder / center 현재-다음 all read the new level once
				// this result is dismissed. A fail leaves the level untouched.
				++ItemUpgradeLevelRef(upgradeSlots[iSelectedSlot].strItemId);
			}
			else
			{
				m_pItemUpgradeView->Set_SlotTexture(
					"ItemUpgrade_FailItemIconMarker", upgradeSlots[iSelectedSlot].strIconPath);
			}
		}
		m_dItemUpgradeResultSettleAt = -1.0;
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
	std::string estherActionSoundStatus;
	if (!Client::CEstherActionSoundCueDocument::Load(estherActionSoundStatus))
	{
		const std::string diagnostic =
			"[MainApp] Esther action Sound cues isolated: " +
			estherActionSoundStatus + "\n";
		OutputDebugStringA(diagnostic.c_str());
	}

	/* Created FIRST among the STATIC-level UI documents -- CUI_Sprite draw order follows
	creation order, and the always-on combat HUD must sit underneath the boss bar, Esther
	window, Item Upgrade window, and inventory, matching the old ImGui submission order. */
	m_pHUDRuntimeView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/HUD/HUD_Layout.json");
	Hide_CombatHUD();
	Load_KoukuHudModes();
	m_pBossUIView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/BossUI/BossUI.json");
	/* Authored layer tints are opaque -- every real slot would otherwise sit fully visible from
	this Level::STATIC construction until the first Update_BossHealthBar() call finds a valid
	boss. */
	Hide_BossHealthBar();
	m_pEstherUIView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Esther/EstherUI.json");
	/* Same reasoning as Hide_BossHealthBar just above -- hidden until Update_EstherGauge finds a
	real Esther roster in a Valtan room. */
	Hide_EstherUI();
	m_pEstherCutinService = std::make_unique<CEstherCutinPresentationService>(
		m_pDevice, m_pContext);
	m_pItemUpgradeView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/ItemUpgrade/ItemUpgradeUI.json");
	/* Authored layer tints are opaque -- every slot would otherwise sit fully visible from this
	Level::STATIC construction until the first real Update_ItemUpgrade() call (P not pressed
	yet). */
	Hide_ItemUpgrade();
	m_pLobbyBackgroundView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Lobby/Lobby_Layout.json");
	/* Hidden until Update_LobbyButtons finds the Lobby active -- and behind every ImGui window
	by construction (engine sprites render before ImGui), which is all the old BACKGROUND draw
	target actually guaranteed here. */
	m_pLobbyBackgroundView->Set_AllSlotsVisible(false);
	Load_LobbyServers();
	/* After the Lobby view on purpose: created later means its sprites join Layer_UI later and
	draw on top of the Lobby backdrop/buttons while the window is open. Its own constructor
	hides every slot. */
	m_pCharacterSelectWindowView = std::make_unique<CCharacterSelectWindowView>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC));
	/* m_pSkillWindowView is intentionally never constructed anymore: the K keybind that opened
	it was removed by product decision (see the migrated keybind block below), so the window can
	never open, and constructing it would stand up the last ImGui product-path renderer for
	nothing. Every "skillWindowOpen" gate already null-checks it. The class/files stay for a
	future real re-introduction. */
	m_pInventoryView = std::make_unique<CInventoryView>(m_pDevice, m_pContext);
	m_pChatWindowView = std::make_unique<CChatWindowView>(m_pDevice, m_pContext);
	m_pPartyWindowView = std::make_unique<CPartyWindowView>(m_pDevice, m_pContext);
	m_pMinimapView = std::make_unique<CMinimapView>(m_pDevice, m_pContext, ETOUI(LEVEL::STATIC));
	/* Last of the runtime windows on purpose: its sprites join Layer_UI last and draw over the
	inventory/chat/party panels while it is open (it also registers itself as the router's top
	window so their text passes stay underneath). */
	m_pCharacterInfoView = std::make_unique<CCharacterInfoWindowView>(m_pDevice, m_pContext);
	/* Opened from the character info window's avatar page, drawn over it: constructed last. */
	m_pAvatarBookView = std::make_unique<CAvatarBookWindowView>(m_pDevice, m_pContext);

	if (FAILED(Start_Level(LEVEL::LOBBY)))
		return E_FAIL;

	return S_OK;
}

void CMainApp::Update(const f32_t fTimeDelta)
{
	/* Once per frame, before any screen's Update()/Render() checks its own widgets via
	CUIInputRouter -- resets its click-edge tracking. End_Frame() (this function's very end)
	applies the gameplay-mouse block for anything that claimed the mouse this frame. */
	CUIInputRouter::Get().Begin_Frame();

#ifdef _DEBUG
	UpdateDebugToolShortcut();
#endif

	/* I is a normal gameplay keybind (the inventory), not an F1/F6 tool-switch key.
	Is_TextInputActive is the runtime UI's own WantTextInput (the ImGui-free nickname field) --
	both must gate every keybind below the same way. */
	if (nullptr != m_pInventoryView && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
	{
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t iDown = windowFocused &&
			0 != (GetAsyncKeyState(0x49 /* VK_I */) & 0x8000);
		if (iDown && !m_bIDown)
		{
			m_pInventoryView->Toggle();
			const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
				m_pInventoryView->Is_Open() ?
				L"Sound/UI/Select/ui_inventory_show1__669750910.wav" :
				L"Sound/UI/Select/ui_inventory_hide1__7273537.wav");
			CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
		}
		m_bIDown = iDown;
	}

	/* P is the retail character info window keybind (same gating as I above). */
	if (nullptr != m_pCharacterInfoView && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
	{
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t keyDown = windowFocused &&
			0 != (GetAsyncKeyState(0x50 /* VK_P */) & 0x8000);
		if (keyDown && !m_bCharacterInfoKeyDown)
		{
			m_pCharacterInfoView->Toggle();
			const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
				m_pCharacterInfoView->Is_Open() ?
				L"Sound/UI/Select/ui_inventory_show1__669750910.wav" :
				L"Sound/UI/Select/ui_inventory_hide1__7273537.wav");
			CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
		}
		m_bCharacterInfoKeyDown = keyDown;
	}

	Update_LobbyButtons(fTimeDelta);
	Update_CharacterSelectWindow(fTimeDelta);
	Update_Minimap(fTimeDelta);
	Update_CombatHUD(fTimeDelta);
	Update_ItemUpgrade(fTimeDelta);
	Update_BossHealthBar();
	Update_EstherGauge();
	if (nullptr != m_pEstherCutinService)
		m_pEstherCutinService->Update(fTimeDelta);

	/* 1/2/3/4 use whatever item is registered on Item_1..4 (drag-drop from the inventory --
	see Update_ItemQuickSlots). Same gating as K/I; the Server is the one that actually
	validates ownership and applies the heal, this only ever sends the request. */
	if (!ImGui::GetIO().WantTextInput && !CUIInputRouter::Get().Is_TextInputActive())
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
	if (nullptr != m_pChatWindowView && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
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

	/* The runtime nickname field blocks DirectInput keyboard polling exactly the way an
	ImGui InputText's WantsCaptureKeyboard does -- WASD/skill keys must not fire mid-typing. */
	const bool_t keyboardCaptured = (nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->WantsCaptureKeyboard() || externalToolFocused)) ||
		CUIInputRouter::Get().Is_TextInputActive();
	/* A runtime UI window (inventory, character info, party, chat) that has the cursor claims
	the mouse through CUIInputRouter; without folding that in here this call would re-open the
	gameplay mouse for CPlayerController's tick right after End_Frame() closed it, so a left
	click on the panel became a basic attack underneath. Last frame's claim covers the windows
	that only hover-claim during Render (after this point). */
	const bool_t mouseCaptured = (nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->WantsCaptureMouse() || externalToolFocused)) ||
		CUIInputRouter::Get().Is_MouseClaimedThisFrame() ||
		CUIInputRouter::Get().Was_MouseClaimedLastFrame();
	CGameInstance::Get().SetInputBlocked(keyboardCaptured, mouseCaptured);
	CGameInstance::Get().SetMouseButtonBlocked(
		DIM::LB,
		worldLeftMouseConsumed);
	CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, false);

	CNetworkManager::Get().Update();
#ifdef _DEBUG
	/* PLAY_PATTERN_ID has one process-wide verdict/lifecycle queue shared by
	   Balance, Effect, and Valtan Boss Tools. Drain it once per frame here, independent
	   of which panel is visible or which tree row is expanded. */
	CValtanPatternAuditionService::Get().Update();
	CValtanPatternFlowService::Get().Update();
	CValtanTuningCommandService::Get().Update();
	CKoukuSaydonPatternAuditionService::Get().Update();
#endif
	CGameInstance::Get().Update_Engine(fTimeDelta);
	if (ETOUI(LEVEL::LOADING) !=
		CGameInstance::Get().Get_CurrentLevelID())
	{
		CEffectPresentationService::Advance_ProductCuePreparation(
			m_pDevice, m_pContext);
	}
	CEffectPresentationService::Commit_PendingSpawns();
	CEffectPresentationService::Synchronize_FollowAnchors();
	CEffectPresentationService::Update(fTimeDelta);
	/* Product combat-object groups and Effect Tool previews share one free-group
	   clock. MainApp advances it exactly once after the Engine object tick. */
	CEffectV2Runtime::Advance_ProductGroups(fTimeDelta, m_pDevice, m_pContext);

	#ifdef _DEBUG
	if (nullptr != m_pMapTool)
	{
		m_pMapTool->Update(
			fTimeDelta,
			m_bDeveloperToolsVisible &&
			IsDebugToolVisible(DEBUG_TOOL::MAP) &&
			DEBUG_TOOL::MAP == m_eDebugInputOwner);
	}
	/* Composition emits a one-shot claim; MainApp remains the sole input-owner
	   authority. Consume it before Animation_Tool::Update so reclaiming after a
	   domain deep-link does not stop the active preview for one extra frame. */
	if (nullptr != m_pValtanActionWorkbench &&
		m_pValtanActionWorkbench->Consume_PreviewOwnerClaimRequest())
	{
		if (m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) &&
			nullptr != m_pSequencerTool &&
			COMPOSITION_WORKBENCH_BOSS::VALTAN == m_pSequencerTool->Get_SelectedBoss())
		{
			m_eDebugInputOwner = DEBUG_TOOL::SEQUENCER;
			m_strToolStatus =
				"Valtan Action Workbench reclaimed viewport/preview input.";
		}
		else
		{
			m_strToolStatus =
				"Preview-owner claim rejected because Valtan Action Workbench is not visible.";
		}
	}
	if (nullptr != m_pAnimationTool)
	{
		const bool_t bAnimationPreviewOwned =
			(IsDebugToolVisible(DEBUG_TOOL::ANIMATION) &&
			 DEBUG_TOOL::ANIMATION == m_eDebugInputOwner) ||
			(IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) &&
			 DEBUG_TOOL::SEQUENCER == m_eDebugInputOwner);
		m_pAnimationTool->Update(
			fTimeDelta,
			m_bDeveloperToolsVisible && bAnimationPreviewOwned);
	}
	/* Save and publish jobs are process owners, not window owners. Poll the
	   immutable Save receipt first, let a hidden Composition caller accept and
	   reopen its exact local owners, then poll the optional Full DataOnly job. */
	if (nullptr != m_pBalanceTool)
		m_pBalanceTool->Update_ValtanSaveJob();
	if (nullptr != m_pValtanActionWorkbench)
		m_pValtanActionWorkbench->Update_SaveState();
	if (nullptr != m_pBalanceTool)
		m_pBalanceTool->Update_ServerRuntimeSetPublishJob();
	/* Composition Save receipts are delivered independently of both Developer
	   Tools visibility and the Composition window's Render call. If Effect Tool
	   does not exist yet, leave the request pending in the Workbench. */
	if (nullptr != m_pValtanActionWorkbench && nullptr != m_pEffectTool)
	{
		std::string ExpectedValtanSourceRevision;
		if (m_pValtanActionWorkbench->Consume_EffectGraphRefreshRequest(
				ExpectedValtanSourceRevision))
		{
			m_pEffectTool->Request_ValtanGraphRefresh(
				ExpectedValtanSourceRevision);
		}
	}
	if (nullptr != m_pEffectTool)
	{
		m_pEffectTool->Update(fTimeDelta);
		EFFECT_RESOURCE_KEY ResourceKey;
		if (m_pEffectTool->Consume_TypedEffectResourceOpenRequest(ResourceKey))
		{
			const bool_t bOpened = nullptr != m_pEffectToolV2 &&
				m_pEffectToolV2->Open_Resource(ResourceKey);
			m_strToolStatus = bOpened ?
				"Opened the selected resource in its typed Effect owner." :
				"The typed Effect owner preserved its current draft; inspect the owner status.";
		}
	}
	if (nullptr != m_pValtanBossTool)
	{
		m_pValtanBossTool->Update(
			m_bDeveloperToolsVisible &&
				IsDebugToolVisible(DEBUG_TOOL::VALTAN_BOSS),
			m_bDeveloperToolsVisible &&
				IsDebugToolVisible(DEBUG_TOOL::VALTAN_LOGIC_PATTERN));
		if (m_pValtanBossTool->Consume_LogicPatternOpenRequest())
		{
			(void)EnsureDebugTool(DEBUG_TOOL::VALTAN_LOGIC_PATTERN);
		}
		CAMERA_TOOL_OPEN_REQUEST cameraRequest;
		if (m_pValtanBossTool->Consume_CameraToolOpenRequest(cameraRequest))
		{
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::CAMERA)) &&
				nullptr != m_pCameraTool)
			{
				(void)m_pCameraTool->Open_Cue(cameraRequest);
			}
		}
		EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST effectRequest;
		if (m_pValtanBossTool->Consume_EffectToolOpenRequest(effectRequest))
		{
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)) &&
				nullptr != m_pEffectTool)
			{
				const bool_t bOpened =
					m_pEffectTool->Open_ValtanProductEffect(effectRequest);
				m_strToolStatus = bOpened ?
					"Opened the exact Valtan Product Effect in Effect Tool." :
					"Effect Tool opened, but the exact Product occurrence needs attention.";
			}
		}
	}
	/* Workbench links use the same stable Product requests as the domain
	   owners. Each one-shot is drained here; the Workbench never creates a
	   second Effect or Camera runtime. */
	if (nullptr != m_pAnimationTool)
	{
		EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST effectRequest;
		if (m_pAnimationTool->Consume_EffectToolOpenRequest(effectRequest))
		{
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)) &&
				nullptr != m_pEffectTool)
			{
				const bool_t bOpened =
					m_pEffectTool->Open_ValtanProductEffect(effectRequest);
				m_strToolStatus = bOpened ?
					"Opened the Workbench Product Effect in Effect Tool." :
					"Effect Tool opened, but the Workbench occurrence needs attention.";
			}
		}
		CAMERA_TOOL_OPEN_REQUEST cameraRequest;
		if (m_pAnimationTool->Consume_CameraToolOpenRequest(cameraRequest))
		{
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::CAMERA)) &&
				nullptr != m_pCameraTool)
			{
				const bool_t bOpened = m_pCameraTool->Open_Cue(cameraRequest);
				m_strToolStatus = bOpened ?
					"Opened the Workbench camera sequence in Camera Tool." :
					"Camera Tool opened, but the Workbench cue needs attention.";
			}
		}
	}
	if (nullptr != m_pValtanActionWorkbench)
	{
		EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST effectRequest;
		if (m_pValtanActionWorkbench->Consume_EffectToolOpenRequest(
				effectRequest))
		{
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)) &&
				nullptr != m_pEffectTool)
			{
				const bool_t bOpened =
					m_pEffectTool->Open_ValtanProductEffect(effectRequest);
				m_strToolStatus = bOpened ?
					"Opened the exact Composition Sequencer Effect in its owner Tool." :
					"Effect Tool opened, but the selected Composition occurrence needs attention.";
			}
		}
		CAMERA_TOOL_OPEN_REQUEST cameraRequest;
		if (m_pValtanActionWorkbench->Consume_CameraToolOpenRequest(
				cameraRequest))
		{
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::CAMERA)) &&
				nullptr != m_pCameraTool)
			{
				const bool_t bOpened = m_pCameraTool->Open_Cue(cameraRequest);
				m_strToolStatus = bOpened ?
					"Opened the exact Composition Sequencer camera cue." :
					"Camera Tool opened, but the selected Composition cue needs attention.";
			}
		}
		if (m_pValtanActionWorkbench->Consume_AnimationToolOpenRequest())
		{
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::ANIMATION)) &&
				nullptr != m_pAnimationTool)
			{
				(void)m_pAnimationTool->Open_ValtanWorkspace();
				m_strToolStatus =
					"Opened Animation Clip/Sequence Intake beside Valtan Action Workbench.";
			}
		}
		m_pValtanActionWorkbench->Set_PreviewOwnerActive(
			m_bDeveloperToolsVisible &&
			IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) &&
			DEBUG_TOOL::SEQUENCER == m_eDebugInputOwner && nullptr != m_pSequencerTool &&
			COMPOSITION_WORKBENCH_BOSS::VALTAN == m_pSequencerTool->Get_SelectedBoss());
	}
	// Both boss sessions share the physical inventory. Consume each refresh flag
	// independently so a simultaneous request does not cause a second read next frame.
	const bool shellResourceRefresh = nullptr != m_pSequencerTool &&
		m_pSequencerTool->Consume_ResourceRefreshRequest();
	const bool koukuResourceRefresh = nullptr != m_pKoukuSaydonActionWorkbench &&
		m_pKoukuSaydonActionWorkbench->Consume_ResourceRefreshRequest();
	if (shellResourceRefresh || koukuResourceRefresh)
	{
		if (SUCCEEDED(EnsureAnimationPreviewBackend()))
		{
			std::vector<COMPOSITION_ANIMATION_RESOURCE> resources;
			std::string status;
			(void)m_pAnimationTool->Read_CompositionAnimationResources(resources, status);
			std::vector<CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW> sequences;
			std::vector<COMPOSITION_ANIMATION_SEQUENCE_RESOURCE> sequenceResources;
			std::string sequenceStatus;
			const bool_t sequencesLoaded = m_pAnimationTool->Get_ValtanCompositionSequences(
				sequences, sequenceStatus);
			if (sequencesLoaded)
			{
				for (const auto& sequence : sequences)
				{
					COMPOSITION_ANIMATION_SEQUENCE_RESOURCE resourceSequence;
					resourceSequence.strStableId = sequence.strStableId;
					resourceSequence.strDisplayName = sequence.strDisplayName;
					resourceSequence.strProfileId = "Valtan";
					resourceSequence.strTargetAssetName = "Valtan";
					for (const auto& clip : sequence.Clips)
					{
						const auto native = std::find_if(resources.begin(), resources.end(),
							[&clip](const auto& candidate) {
								return candidate.strTargetAssetName == "Valtan" &&
									candidate.strRuntimeClip == clip.strClipName;
							});
						COMPOSITION_ANIMATION_RESOURCE resource;
						if (native != resources.end()) resource = *native;
						const std::uint32_t nativeDurationMs = resource.iDurationMs;
						resource.strTargetAssetName = resource.strProfileId = "Valtan";
						resource.strRuntimeClip = clip.strClipName;
						if (!clip.bUsesNativeDuration && clip.iDurationMs > 0u)
							resource.iDurationMs = clip.iDurationMs;
						if (nativeDurationMs > 0u && resource.iDurationMs > nativeDurationMs)
							resource.strEndPolicy = clip.strClipName.find("_loop") != std::string::npos ?
								"LOOP_TO_WINDOW" : "HOLD_LAST_POSE";
						resourceSequence.Clips.push_back(std::move(resource));
					}
					sequenceResources.push_back(std::move(resourceSequence));
				}
			}
			if (nullptr != m_pSequencerTool)
				m_pSequencerTool->Set_AnimationResources(resources, status);
			if (nullptr != m_pKoukuSaydonActionWorkbench)
			{
				m_pKoukuSaydonActionWorkbench->Set_ModelResources(std::move(resources), std::move(status));
				m_pKoukuSaydonActionWorkbench->Set_SequenceResources(
					std::move(sequenceResources), std::move(sequenceStatus), sequencesLoaded);
			}
		}
		else
		{
			m_strToolStatus = "Animation preview backend could not initialize for Resources.";
			if (nullptr != m_pSequencerTool)
				m_pSequencerTool->Set_AnimationResources({}, m_strToolStatus);
		}
	}
	if (nullptr != m_pSequencerTool)
	{
		COMPOSITION_ANIMATION_RESOURCE resource;
		if (m_pSequencerTool->Consume_AnimationPreviewRequest(resource))
		{
			if (SUCCEEDED(EnsureAnimationPreviewBackend()) && nullptr != m_pAnimationTool)
			{
				if (m_pAnimationTool->Preview_CompositionAnimationResource(resource, m_strToolStatus))
				{
					m_eDebugInputOwner = DEBUG_TOOL::SEQUENCER;
				}
			}
			else
				m_strToolStatus = "Animation preview backend could not initialize for the selected resource.";
			m_pSequencerTool->Set_AnimationPreviewStatus(m_strToolStatus);
		}
	}
	/* The K Workbench owns only composition data. A click transfers an immutable
	   occurrence/pattern value to the existing real-CModel preview owner; no
	   draft pointer or Valtan document crosses this boundary. */
	std::string previewRouteStatus;
	if (nullptr != m_pKoukuSaydonActionWorkbench)
	{
		m_pKoukuSaydonActionWorkbench->Tick_Background();
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence;
		if (m_pKoukuSaydonActionWorkbench->Consume_AnimationPreviewRequest(
				occurrence))
		{
			if (SUCCEEDED(EnsureAnimationPreviewBackend()) &&
				nullptr != m_pAnimationTool)
			{
				const bool_t previewed =
					m_pAnimationTool->Preview_KoukuSaydonCompositionAnimation(
						occurrence, m_strToolStatus);
				if (previewed)
				{
					m_eDebugInputOwner = DEBUG_TOOL::SEQUENCER;
				}
				else
					previewRouteStatus = m_strToolStatus;
			}
			else
			{
				m_strToolStatus =
					"Animation Tool could not open for KoukuSaydon resource preview.";
				previewRouteStatus = m_strToolStatus;
			}
		}

		KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
		std::uint32_t startClockMs = 0u;
		bool_t startPaused = false;
		std::string targetAssetName;
		if (m_pKoukuSaydonActionWorkbench->Consume_PatternPreviewRequest(
				pattern, startClockMs, startPaused, targetAssetName))
		{
			if (SUCCEEDED(EnsureAnimationPreviewBackend()) &&
				nullptr != m_pAnimationTool)
			{
				const bool_t previewed = targetAssetName.empty() ?
					m_pAnimationTool->Preview_KoukuSaydonCompositionPattern(
						pattern, m_strToolStatus, startClockMs, startPaused) :
					m_pAnimationTool->Preview_CompositionResourcePattern(
						pattern, targetAssetName, m_strToolStatus, startClockMs, startPaused);
				if (previewed)
				{
					m_eDebugInputOwner = DEBUG_TOOL::SEQUENCER;
				}
				else
					previewRouteStatus = m_strToolStatus;
			}
			else
			{
				m_strToolStatus =
					"Animation Tool could not open for KoukuSaydon pattern preview.";
				previewRouteStatus = m_strToolStatus;
			}
		}

		/* Apply transport after a same-frame preview start, then return the
		   resulting identity and clock together to the Workbench. */
		KOUKU_PREVIEW_TRANSPORT transport = KOUKU_PREVIEW_TRANSPORT::NONE;
		std::uint32_t seekMs = 0u;
		if (m_pKoukuSaydonActionWorkbench->Consume_PreviewTransportRequest(
				transport, seekMs) &&
			nullptr != m_pAnimationTool)
		{
			std::string transportStatus;
			switch (transport)
			{
			case KOUKU_PREVIEW_TRANSPORT::PAUSE:
				(void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(
					true, transportStatus);
				break;
			case KOUKU_PREVIEW_TRANSPORT::RESUME:
				(void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(
					false, transportStatus);
				break;
			case KOUKU_PREVIEW_TRANSPORT::STOP:
				(void)m_pAnimationTool->Stop_KoukuCompositionPreview(transportStatus);
				break;
			case KOUKU_PREVIEW_TRANSPORT::SEEK:
				(void)m_pAnimationTool->Seek_KoukuCompositionPreview(
					seekMs, transportStatus);
				break;
			default:
				break;
			}
			if (!transportStatus.empty())
				m_strToolStatus = transportStatus;
		}

		std::string serverPatternId;
		std::uint32_t sourceRevision = 0u;
		if (m_pKoukuSaydonActionWorkbench->Consume_ServerPlayRequest(
				serverPatternId, sourceRevision))
		{
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::KOUKU_SAYDON_BOSS)) &&
				nullptr != m_pKoukuSaydonBossTool)
			{
				SetDebugToolVisible(DEBUG_TOOL::KOUKU_SAYDON_BOSS, true);
				(void)m_pKoukuSaydonBossTool->Play_PatternById(
					serverPatternId, sourceRevision, m_strToolStatus);
				m_eDebugInputOwner = DEBUG_TOOL::KOUKU_SAYDON_BOSS;
				m_eDebugWindowFocusPending = DEBUG_TOOL::KOUKU_SAYDON_BOSS;
			}
			else
			{
				m_strToolStatus =
					"KoukuSaydon Boss Tool could not open for Server Play.";
			}
		}
	}
	// Apply shared transport after every queued start. A Stop in Resources
	// must also cancel a Pattern/row start submitted during the same UI frame.
	if (nullptr != m_pSequencerTool)
	{
		CSequencerTool::ANIMATION_PREVIEW_TRANSPORT transport =
			CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::NONE;
		if (m_pSequencerTool->Consume_AnimationPreviewTransportRequest(transport))
		{
			if (nullptr != m_pAnimationTool)
			{
				switch (transport)
				{
				case CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::PAUSE:
					(void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(true, m_strToolStatus);
					break;
				case CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::RESUME:
					(void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(false, m_strToolStatus);
					break;
				case CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP:
					(void)m_pAnimationTool->Stop_KoukuCompositionPreview(m_strToolStatus);
					break;
				case CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::NONE:
					break;
				}
			}
			else
				m_strToolStatus = "No animation preview backend is active.";
			m_pSequencerTool->Set_AnimationPreviewStatus(m_strToolStatus);
		}
	}
	// Both panes receive the final sampler identity and clock from this frame.
	if (nullptr != m_pKoukuSaydonActionWorkbench)
	{
		if (nullptr != m_pAnimationTool)
		{
			const CAnimation_Tool::KOUKU_COMPOSITION_PREVIEW_STATE previewState =
				m_pAnimationTool->Get_KoukuCompositionPreviewState();
			KOUKU_PREVIEW_STATE state;
			state.strPatternId = previewState.strPatternId;
			state.strStatus = previewRouteStatus.empty() ? previewState.strStatus : previewRouteStatus;
			state.bPlaying = previewState.bPlaying;
			state.bPaused = previewState.bPaused;
			state.iClockMs = previewState.iClockMs;
			state.iDurationMs = previewState.iDurationMs;
			m_pKoukuSaydonActionWorkbench->Set_PreviewState(state);
		}
		else if (!previewRouteStatus.empty())
		{
			KOUKU_PREVIEW_STATE state;
			state.strStatus = previewRouteStatus;
			m_pKoukuSaydonActionWorkbench->Set_PreviewState(state);
		}
	}
	if (nullptr != m_pSequencerTool && nullptr != m_pAnimationTool)
	{
		const auto previewState = m_pAnimationTool->Get_KoukuCompositionPreviewState();
		CSequencerTool::ANIMATION_PREVIEW_STATE state;
		state.strPatternId = previewState.strPatternId;
		state.strStatus = previewState.strStatus;
		state.bPlaying = previewState.bPlaying;
		state.bPaused = previewState.bPaused;
		state.iClockMs = previewState.iClockMs;
		state.iDurationMs = previewState.iDurationMs;
		m_pSequencerTool->Set_AnimationPreviewState(std::move(state));
	}
	if (nullptr != m_pCameraTool)
	{
		m_pCameraTool->Update(
			fTimeDelta,
			m_bDeveloperToolsVisible &&
			IsDebugToolVisible(DEBUG_TOOL::CAMERA) &&
			DEBUG_TOOL::CAMERA == m_eDebugInputOwner);
	}
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

	/* The character info window's live portrait draws into its own target here, before the
	world/UI pass whose CI_Preview sprite samples it. */
	if (nullptr != m_pCharacterInfoView)
		(void)m_pCharacterInfoView->Render_Portrait();
	if (nullptr != m_pAvatarBookView)
		(void)m_pAvatarBookView->Render_Portrait();

	const HRESULT hWorldResult = CGameInstance::Get().Render();
	if (FAILED(hWorldResult))
	{
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return hWorldResult;
	}

	if (nullptr != m_pImGuiLayer)
	{
		/* No Lobby draw call here anymore -- Update_LobbyButtons() (called from Update())
		drives the Lobby's real CUI_Sprite slots. */
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
		const bool_t skillWindowOpenForPreview =
			nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
		/* The O-key raid-entry preview's left info column/panel frame sit in the
		   same screen region as the class HUD (portrait, skill icons, identity
		   gauge) -- Update_CombatHUD applies the same gate to the real sprites. */
		const bool_t isCharSelectDebugPreviewOpen =
			ETOUI(LEVEL::CHARACTER_SELECT) == hudLevel &&
			nullptr != CLevel_CharacterSelect::Get_Active() &&
			CLevel_CharacterSelect::Get_Active()->Is_DebugRaidEntryPreviewOpen();
		if (nullptr != m_pHUDLayoutTool && hudPlayer.isValid &&
			supportsAuthoredHUD && !skillWindowOpenForPreview &&
			!isCharSelectDebugPreviewOpen)
		{
			m_pHUDLayoutTool->Render_RuntimePreview(
				GetHUDLayoutClassId(hudPlayer.eCharacterClass));
		}
	#endif
		/* No combat-HUD draw calls here anymore -- Update_CombatHUD() (called from Update())
		drives the HUD's real CUI_Sprite slots, which render through the normal engine
		pipeline. */
		/* Must run after the combat HUD renders above: Render_ValtanEntryModal's
		   art draws to ImGui::GetForegroundDrawList(), the same shared list
		   combat HUD draws once used, and that
		   list composites in real submission order -- calling it earlier let the
		   always-on combat HUD paint over the full-screen raid-entry panel.
		   (Update_BossHealthBar's own CUI_Sprite slots render through the normal
		   engine pipeline instead, so they're not part of this particular
		   ordering concern anymore.) */
		if (ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID())
		{
			if (CLevel_Bern* pBern = CLevel_Bern::Get_Active())
				pBern->Render_ValtanEntryModal();
		}
#ifdef _DEBUG
		/* O-key visual-only preview of the same raid-entry panel from Character
		   Select -- same foreground-drawlist ordering requirement as Bern's real
		   one above. See CLevel_CharacterSelect::Update_RaidEntryDebugPreviewKey. */
		if (ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID())
		{
			if (CLevel_CharacterSelect* pCharacterSelectDebug = CLevel_CharacterSelect::Get_Active())
				pCharacterSelectDebug->Render_RaidEntryDebugPreview();
		}
#endif
		if (nullptr != m_pChatWindowView)
		{
			/* Only in actual in-game play (Bern/Valtan), not Character Select -- more levels join
			this list as real in-game stages are added. Real send needs the active level's own
			command sink, same reasoning as the party roster fetch just below. */
			const uint32_t chatLevel = CGameInstance::Get().Get_CurrentLevelID();
			if (ETOUI(LEVEL::BERN) == chatLevel)
			{
				CLevel_Bern* pBern = CLevel_Bern::Get_Active();
				m_pChatWindowView->Render(
					nullptr != pBern ? pBern->Get_PlayerCommandSink() : nullptr);
			}
			else if (ETOUI(LEVEL::VALTAN_ARENA) == chatLevel)
			{
				CLevel_ValtanArena* pValtanArena = CLevel_ValtanArena::Get_Active();
				m_pChatWindowView->Render(
					nullptr != pValtanArena ?
						pValtanArena->Get_PlayerCommandSink() : nullptr);
			}
		}
		if (nullptr != m_pPartyWindowView)
		{
			/* Same level set as the chat window. Each level owns its own CClientReplication
			(and therefore its own Server-synced roster), so the active level is asked for its
			current roster the same way Render_ValtanEntryModalText() reaches CLevel_Bern below. */
			const uint32_t partyLevel = CGameInstance::Get().Get_CurrentLevelID();
			if (ETOUI(LEVEL::BERN) == partyLevel)
			{
				if (CLevel_Bern* pBern = CLevel_Bern::Get_Active())
					m_pPartyWindowView->Sync_From_Roster(
						pBern->Get_PartyRoster(), pBern->Get_PlayerHealth());
				m_pPartyWindowView->Render();
			}
			else if (ETOUI(LEVEL::VALTAN_ARENA) == partyLevel)
			{
				if (CLevel_ValtanArena* pValtanArena = CLevel_ValtanArena::Get_Active())
					m_pPartyWindowView->Sync_From_Roster(
						pValtanArena->Get_PartyRoster(), pValtanArena->Get_PlayerHealth());
				m_pPartyWindowView->Render();
			}
		}
#ifdef _DEBUG
		if (nullptr != m_pRenderingBenchmark)
			m_pRenderingBenchmark->Update(CGameInstance::Get().Get_Profiler());
		if (m_bDeveloperToolsVisible)
		{
			Engine::CProfilerScope developerToolsScope(
				CGameInstance::Get().Get_Profiler(), "ImGui.DeveloperTools");
			RenderDeveloperTools();
			/* The Workbench shell renders one selected boss session. Other domain
			   tools remain independent windows; every owner retains its own draft. */
			const auto focusNextWindow = [this](const DEBUG_TOOL eTool)
			{
				if (m_eDebugWindowFocusPending != eTool)
					return;
				ImGui::SetNextWindowFocus();
				m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
			};
			if (IsDebugToolVisible(DEBUG_TOOL::MAP) && nullptr != m_pMapTool)
			{
				focusNextWindow(DEBUG_TOOL::MAP);
				m_pMapTool->Render();
			}
			if (IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) && nullptr != m_pSequencerTool)
			{
				focusNextWindow(DEBUG_TOOL::SEQUENCER);
				m_pSequencerTool->Render();
				if (!m_pSequencerTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::SEQUENCER, false);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::ANIMATION) &&
				nullptr != m_pAnimationTool)
			{
				focusNextWindow(DEBUG_TOOL::ANIMATION);
				m_pAnimationTool->Render();
			}
			if (IsDebugToolVisible(DEBUG_TOOL::EFFECT))
			{
				focusNextWindow(DEBUG_TOOL::EFFECT);
				if (nullptr != m_pEffectTool)
					m_pEffectTool->Render();
				if (nullptr != m_pEffectToolV2)
					m_pEffectToolV2->Render();
			}
			if (IsDebugToolVisible(DEBUG_TOOL::RENDERING))
			{
				focusNextWindow(DEBUG_TOOL::RENDERING);
				RenderRenderingWorkbench();
			}
			if (IsDebugToolVisible(DEBUG_TOOL::PROFILER) &&
				nullptr != m_pProfilerTool)
			{
				focusNextWindow(DEBUG_TOOL::PROFILER);
				m_pProfilerTool->Render(CGameInstance::Get().Get_Profiler());
				if (!m_pProfilerTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::PROFILER, false);
			}
			/* Skill Window's slots are still authored by their existing UI owner;
			   rendering it alongside other tools does not create a second UI runtime. */
			if (IsDebugToolVisible(DEBUG_TOOL::UI) && nullptr != m_pHUDLayoutTool)
			{
				focusNextWindow(DEBUG_TOOL::UI);
				m_pHUDLayoutTool->Render();
			}
			if (IsDebugToolVisible(DEBUG_TOOL::BALANCE) && nullptr != m_pBalanceTool)
			{
				focusNextWindow(DEBUG_TOOL::BALANCE);
				m_pBalanceTool->Render();
			}
			if (IsDebugToolVisible(DEBUG_TOOL::VALTAN_BOSS) && nullptr != m_pValtanBossTool)
			{
				focusNextWindow(DEBUG_TOOL::VALTAN_BOSS);
				m_pValtanBossTool->Render();
				if (!m_pValtanBossTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::VALTAN_BOSS, false);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::KOUKU_SAYDON_BOSS) &&
				nullptr != m_pKoukuSaydonBossTool)
			{
				focusNextWindow(DEBUG_TOOL::KOUKU_SAYDON_BOSS);
				m_pKoukuSaydonBossTool->Render();
				if (!m_pKoukuSaydonBossTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::KOUKU_SAYDON_BOSS, false);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::VALTAN_LOGIC_PATTERN) && nullptr != m_pValtanBossTool)
			{
				focusNextWindow(DEBUG_TOOL::VALTAN_LOGIC_PATTERN);
				m_pValtanBossTool->Render_LogicPatternWindow();
				if (!m_pValtanBossTool->Is_LogicPatternOpen())
					SetDebugToolVisible(DEBUG_TOOL::VALTAN_LOGIC_PATTERN, false);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::CAMERA) && nullptr != m_pCameraTool)
			{
				focusNextWindow(DEBUG_TOOL::CAMERA);
				m_pCameraTool->Render();
			}
			if (IsDebugToolVisible(DEBUG_TOOL::EQUIPMENT) &&
				nullptr != m_pEquipmentAuthoringTool)
			{
				focusNextWindow(DEBUG_TOOL::EQUIPMENT);
				m_pEquipmentAuthoringTool->Render(
					DEBUG_TOOL::EQUIPMENT == m_eDebugInputOwner);
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
	/* Raid entry popup's live boss: over the popup's own sprites, under the Draw_Text pass
	   that letters it. */
	CRaidBossShowcaseService::Render(m_pDevice, m_pContext);
	/* Same reasoning as the old combat-HUD/boss-bar/charge-gauge
	   image gate above (isCharSelectDebugPreviewOpen there) -- these are that
	   HUD's own text counterparts (HP/MP numbers, boss HP text, gauge percent),
	   drawn from this separate post-EndFrame() text pass, so they need the same
	   gate here instead of bleeding the resource numbers through the O-key raid-
	   entry preview. That earlier local is out of scope by this point (declared
	   inside the now-closed m_pImGuiLayer block), so it is recomputed
	   Release-safely. */
#ifdef _DEBUG
	const bool_t isCharSelectDebugPreviewOpenForText =
		ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID() &&
		nullptr != CLevel_CharacterSelect::Get_Active() &&
		CLevel_CharacterSelect::Get_Active()->Is_DebugRaidEntryPreviewOpen();
#else
	const bool_t isCharSelectDebugPreviewOpenForText = false;
#endif
	if (!isCharSelectDebugPreviewOpenForText)
	{
		RenderCombatHUDText();
		RenderBossHealthBarText();
		RenderChargeGaugeText();
		RenderSkillCooldownText();
		/* VALTAN_ARENA-only inside; no CharSelect-preview overlap possible, but grouped with the
		other combat-HUD text anyway since it is that HUD's own caption. */
		RenderEstherGaugeText();
		/* The quick slot key captions belong to that same combat HUD. Left outside this gate
		they kept drawing over the raid entry preview -- this whole text pass runs after
		CImGuiLayer::EndFrame(), so it lands on top of every sprite including that popup. */
		RenderQuickSlotKeyLabels();
	}
	RenderDeadSceneText();
	RenderRaidClearText();
	RenderItemAnnounceText();
	RenderDamageNumbers();
	if (nullptr != m_pInventoryView)
		m_pInventoryView->Render_Text();
	RenderLobbyButtonText();
	RenderCharacterSelectWindowText();
	RenderMinimapText();
	RenderItemUpgradeButtonText();
	RenderItemUpgradeLevelText();
	RenderItemUpgradeMaterialCounts();
	RenderItemUpgradeGaugePercentText();
	RenderItemUpgradeResultWaitText();
	RenderItemUpgradeSuccessDetailText();
	RenderItemUpgradeFailDetailText();
	RenderItemUpgradeListText();
	if (ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID())
	{
		if (CLevel_CharacterSelect* pCharacterSelect = CLevel_CharacterSelect::Get_Active())
		{
			// Same gate as Update_ArenaSpawnButtons's own image draw -- these are
			// its text labels, drawn from this separate text pass.
			if (!isCharSelectDebugPreviewOpenForText)
				pCharacterSelect->Render_ArenaSpawnLabels();
#ifdef _DEBUG
			pCharacterSelect->Render_RaidEntryDebugPreviewText();
#endif
		}
	}
	if (ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID())
	{
		if (CLevel_Bern* pBern = CLevel_Bern::Get_Active())
		{
			pBern->Render_ValtanEntryModalText();
			pBern->Render_PartyInviteText();
		}
	}
	else if (ETOUI(LEVEL::VALTAN_ARENA) == CGameInstance::Get().Get_CurrentLevelID())
	{
		if (CLevel_ValtanArena* pValtanArena = CLevel_ValtanArena::Get_Active())
			pValtanArena->Render_PartyInviteText();
	}
	/* Not level-gated -- both views self-gate internally (open/roster state). */
	if (nullptr != m_pChatWindowView)
		m_pChatWindowView->RenderText();
	if (nullptr != m_pPartyWindowView)
		m_pPartyWindowView->RenderText();

	/* The topmost runtime windows draw their text last: everything above was clipped out of the
	top window's rect (CUIInputRouter::Set_TopWindowRect -> CGameInstance::Set_TextClipOutRect),
	these two draw with the clip cleared -- the info window's own labels skip the avatar book's
	rect through Set_Covered while the book is open. */
	CGameInstance::Get().Clear_TextClipOutRect();
	if (nullptr != m_pCharacterInfoView)
		m_pCharacterInfoView->Render_Text();
	if (nullptr != m_pAvatarBookView)
		m_pAvatarBookView->Render_Text();

	/* Every CUIInputRouter-based screen's click-edge check has run by this point (both this
	function's own render pass and the Update() pass earlier this same frame) -- rolls the
	left-button edge state forward for next frame and applies SetInputBlocked for anything
	that called Claim_Mouse_This_Frame(). */
	CUIInputRouter::Get().End_Frame();

	return CGameInstance::Get().Render_End();
}

void CMainApp::Hide_CombatHUD()
{
	if (nullptr != m_pHUDRuntimeView)
		m_pHUDRuntimeView->Set_AllSlotsVisible(false);
}

void CMainApp::Update_CombatHUD(const f32_t fTimeDelta)
{
	if (nullptr == m_pHUDRuntimeView)
		return;

	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel =
		currentLevel == ETOUI(LEVEL::BERN) ||
		currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
		currentLevel == ETOUI(LEVEL::DEVELOPMENT) ||
		currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) ||
		currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA);
	/* The Skill Window (when one exists) and the Debug O-key raid-entry preview both replace
	this whole screen region -- same gates the old ImGui pass applied at its call sites. */
	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
#ifdef _DEBUG
	const bool_t isCharSelectDebugPreviewOpen =
		ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel &&
		nullptr != CLevel_CharacterSelect::Get_Active() &&
		CLevel_CharacterSelect::Get_Active()->Is_DebugRaidEntryPreviewOpen();
#else
	const bool_t isCharSelectDebugPreviewOpen = false;
#endif

	const HUD_PLAYER_STATE& player =
		CCombatHUDViewModel::Get().Get_Player();
	if (!isSupportedLevel || skillWindowOpen || isCharSelectDebugPreviewOpen ||
		!player.isValid || 0u == player.iMaximumHp || 0u == player.iMaximumResource)
	{
		Hide_CombatHUD();
		/* m_pInventoryView's CUI_Sprite slots live under LEVEL::STATIC too (so the panel
		survives a Bern<->Valtan transition instead of resetting) -- they keep showing their
		last state across a level change unless told otherwise, same as this HUD's own. */
		if (nullptr != m_pInventoryView)
			m_pInventoryView->Hide();
		if (nullptr != m_pCharacterInfoView)
			m_pCharacterInfoView->Hide();
		if (nullptr != m_pAvatarBookView)
			m_pAvatarBookView->Hide();
		return;
	}

	{
		/* Base pass: every neutral slot on, then the ownerClass filter (the old
		Render(strOwnerClass, 0) pass) picks the active class's own slots. The Update_* helpers
		below overwrite the dynamic slots' visibility (fills, icons, cooldown overlays) the same
		frame, before anything renders. Base state only for now -- no gauge/resource-driven
		stage switching yet. */
		const string strOwnerClass = GetHUDOwnerClassName(player.eCharacterClass);
		m_pHUDRuntimeView->Set_AllSlotsVisible(true);
		m_pHUDRuntimeView->Set_ActiveOwnerClass(strOwnerClass);

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
		Update_PlayerHealthManaBar();
	}

	/* Real gauge0/1/2 fill (target-rotation-masked track) and burn flourish are baked and wired;
	the 3 segments' screen position (Lance_Id_GaugeBg/Fill/Burn0/1/2 rect in HUD_Layout.json) is
	still a placeholder shared with Lance_Id_Stance's own rect -- needs live in-game tuning to
	place left/bottom/right segments at their real offsets. */
	if (LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER == player.eCharacterClass)
		Update_LanceMasterIdentityGauge();

	Update_ChargeGauge();
	Update_SkillIcons();
	Update_SkillCooldowns();
	Update_QuickSlotFlash();
	Update_ItemQuickSlots();
	Update_KoukuHudMode();
	if (nullptr != m_pInventoryView)
		m_pInventoryView->Update(CCombatHUDViewModel::Get().Get_Inventory().Items);
	if (nullptr != m_pCharacterInfoView)
	{
		/* Each level owns its own CClientReplication, the same way the party window above
		asks the active level for its roster. */
		shared_ptr<CCharacter> pLocalCharacter;
		if (ETOUI(LEVEL::BERN) == currentLevel)
		{
			if (CLevel_Bern* pBern = CLevel_Bern::Get_Active())
				pLocalCharacter = pBern->Get_LocalCharacter();
		}
		else if (ETOUI(LEVEL::VALTAN_ARENA) == currentLevel)
		{
			if (CLevel_ValtanArena* pValtanArena = CLevel_ValtanArena::Get_Active())
				pLocalCharacter = pValtanArena->Get_LocalCharacter();
		}
		else if (ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel)
		{
			if (CLevel_CharacterSelect* pCharacterSelect = CLevel_CharacterSelect::Get_Active())
				pLocalCharacter = pCharacterSelect->Get_LocalCharacter();
		}
		m_pCharacterInfoView->Update(fTimeDelta, pLocalCharacter, player);
		if (nullptr != m_pAvatarBookView)
		{
			/* The avatar-page avatar book button toggles the book; the book lives only while the
			info window is open (its slot map / avatar item ids come from that window). */
			if (m_pCharacterInfoView->Take_AvatarBookRequest())
			{
				if (m_pAvatarBookView->Is_Open())
					m_pAvatarBookView->Close();
				else
					m_pAvatarBookView->Open();
			}
			if (!m_pCharacterInfoView->Is_Open())
				m_pAvatarBookView->Close();
			m_pCharacterInfoView->Set_Covered(m_pAvatarBookView->Is_Open());
			m_pAvatarBookView->Update(fTimeDelta, pLocalCharacter, player, *m_pCharacterInfoView);
		}
	}

	/* Advances every keyframe-animation slot the per-class blocks above played (and any
	flipbooks, though this document has none) -- must run after them so a Play call issued this
	frame evaluates into its sprites before this frame renders. */
	m_pHUDRuntimeView->Update(fTimeDelta);
}

void CMainApp::RenderQuickSlotKeyLabels()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) &&
		currentLevel != ETOUI(LEVEL::KAKULSAYDON_ARENA))
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

	/* T/V are hidden by the KoukuSaydon interaction mode (Update_KoukuHudMode); a
	label over a hidden slot would float on the emblem. */
	const HUD_KOUKU_GIMMICK_STATE& koukuLabelState = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	const bool_t bKoukuModeLabels = koukuLabelState.isValid &&
		HUD_KOUKU_HUD_MODE::NONE != koukuLabelState.eHudMode &&
		ETOUI(LEVEL::KAKULSAYDON_ARENA) == currentLevel;
	for (const KEY_LABEL& Label : LABELS)
	{
		if (bKoukuModeLabels &&
			(0 == std::strcmp(Label.pSlotId, "Skill_T") || 0 == std::strcmp(Label.pSlotId, "Skill_V")))
		{
			continue;
		}
		DrawKeyLabel(Label.pSlotId, Label.pLabel);
	}

	/* Artist's Z ("저무는 달") and Warlord's X/Z ("전장의 방패"/"방어 태세 전환") are not drawn
	here. The only "Skill_Z" slot in HUD_Layout.json is Warlord-owned, KEYFRAME_ANIMATION type
	with a placeholder 1x1 rect (its real on-screen size/position lives in the keyframe document
	SkillZState.json, not this JSON's rect) -- DrawKeyLabel's slot-width-based centering math
	doesn't apply to it, so both the Artist and Warlord cases produced a mispositioned label.
	Skip until there's a real anchor to read (either from the keyframe document's own bounds, or
	a dedicated non-keyframe slot). */
}

namespace
{
	constexpr const char* KOUKU_SLOT_KEYS[HUD_KOUKU_SLOT_COUNT] =
		{ "Q", "W", "E", "R", "A", "S", "D", "F" };
	constexpr const char* KOUKU_EMBLEM_SLOTS[] = { "Kouku_Emblem_Npc", "Kouku_Emblem_Pickup" };
	/* Slots the retail interaction layout drops entirely (no T/V column). */
	constexpr const char* KOUKU_HIDDEN_SLOTS[] =
	{
		"Skill_T", "Skill_T_Icon", "Skill_T_Frame", "Skill_T_Cooldown", "Skill_T_Flash",
		"Skill_V", "Skill_V_Icon", "Skill_V_Frame", "Skill_V_Cooldown", "Skill_V_Flash",
		"Skill_V_Edge1", "Skill_V_Edge2",
	};

	const char* KoukuHudModeId(const HUD_KOUKU_HUD_MODE eMode)
	{
		switch (eMode)
		{
		case HUD_KOUKU_HUD_MODE::POLYMORPH: return "POLYMORPH";
		case HUD_KOUKU_HUD_MODE::MARIO: return "MARIO";
		case HUD_KOUKU_HUD_MODE::DANCE: return "DANCE";
		case HUD_KOUKU_HUD_MODE::MAZE: return "MAZE";
		default: return nullptr;
		}
	}
}

void CMainApp::Load_KoukuHudModes()
{
	m_KoukuHudModes.clear();
	const filesystem::path DataPath =
		CProjectDataRoot::Resolve(L"UI/KoukuSaydon/KoukuHudModes.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
	{
		OutputDebugStringA("[KoukuHudMode] KoukuHudModes.json missing -- no interaction mode can show.\n");
		return;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		OutputDebugStringA(("[KoukuHudMode] KoukuHudModes.json parse failed: " + Error + "\n").c_str());
		return;
	}
	const DATA_JSON_VALUE* pModes = Root.Find("modes");
	if (nullptr == pModes || !pModes->Is_Array())
		return;

	vector<KOUKU_HUD_MODE_DEF> Staged;
	for (const DATA_JSON_VALUE& Value : pModes->Get_Array())
	{
		if (!Value.Is_Object())
			return;
		KOUKU_HUD_MODE_DEF Mode{};
		const DATA_JSON_VALUE* pId = Value.Find("id");
		const DATA_JSON_VALUE* pEmblem = Value.Find("emblemSlot");
		const DATA_JSON_VALUE* pSkills = Value.Find("skills");
		if (nullptr == pId || !pId->Is_String() || pId->Get_String().empty() ||
			nullptr == pEmblem || !pEmblem->Is_String() ||
			nullptr == pSkills || !pSkills->Is_Array() ||
			pSkills->Get_Array().size() > HUD_KOUKU_SLOT_COUNT)
		{
			OutputDebugStringA("[KoukuHudMode] KoukuHudModes.json has an invalid mode entry -- file rejected.\n");
			return;
		}
		Mode.strId = pId->Get_String();
		Mode.strEmblemSlot = pEmblem->Get_String();
		if (const DATA_JSON_VALUE* pRandom = Value.Find("randomOrder");
			nullptr != pRandom && pRandom->Is_Boolean())
		{
			Mode.bRandomOrder = pRandom->Get_Boolean();
		}
		for (const DATA_JSON_VALUE& Skill : pSkills->Get_Array())
		{
			const DATA_JSON_VALUE* pIcon = Skill.Is_Object() ? Skill.Find("iconPath") : nullptr;
			if (nullptr == pIcon || !pIcon->Is_String() || pIcon->Get_String().empty())
			{
				OutputDebugStringA("[KoukuHudMode] KoukuHudModes.json skill without iconPath -- file rejected.\n");
				return;
			}
			KOUKU_HUD_MODE_SKILL Entry{};
			Entry.strIconPath = pIcon->Get_String();
			if (const DATA_JSON_VALUE* pName = Skill.Find("displayName");
				nullptr != pName && pName->Is_String())
			{
				Entry.strDisplayName = pName->Get_String();
			}
			Mode.Skills.push_back(std::move(Entry));
		}
		Staged.push_back(std::move(Mode));
	}
	m_KoukuHudModes = std::move(Staged);
}

void CMainApp::Update_KoukuHudMode()
{
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	const float4_t vCooldownTint = float4_t(0.f, 0.f, 0.f, 150.f / 255.f);

	const HUD_KOUKU_GIMMICK_STATE& kouku = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const bool_t bActive = kouku.isValid &&
		HUD_KOUKU_HUD_MODE::NONE != kouku.eHudMode &&
		ETOUI(LEVEL::KAKULSAYDON_ARENA) == CGameInstance::Get().Get_CurrentLevelID();
	const KOUKU_HUD_MODE_DEF* pMode = nullptr;
	if (bActive)
	{
		const char* pModeId = KoukuHudModeId(kouku.eHudMode);
		for (const KOUKU_HUD_MODE_DEF& Mode : m_KoukuHudModes)
		{
			if (nullptr != pModeId && Mode.strId == pModeId)
			{
				pMode = &Mode;
				break;
			}
		}
	}

	/* Update_CombatHUD turns every slot on first, so the appended emblem slots
	must be re-hidden every frame the mode is off. */
	for (const char* pEmblemSlot : KOUKU_EMBLEM_SLOTS)
		m_pHUDRuntimeView->Set_SlotVisible(pEmblemSlot, false);
	if (nullptr == pMode)
		return;

	/* No class owns this name, so every ownerClass slot (identity blocks) hides. */
	m_pHUDRuntimeView->Set_ActiveOwnerClass("KoukuSaydonInteraction");
	for (const char* pHiddenSlot : KOUKU_HIDDEN_SLOTS)
		m_pHUDRuntimeView->Set_SlotVisible(pHiddenSlot, false);
	m_pHUDRuntimeView->Set_SlotVisible(pMode->strEmblemSlot, true);

	for (size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
	{
		const string strKey = KOUKU_SLOT_KEYS[i];
		const string strIconSlot = "Skill_" + strKey + "_Icon";
		const string strCooldownSlot = "Skill_" + strKey + "_Cooldown";

		const int32_t iSkillIndex = kouku.ModeSkillIndexBySlot[i];
		const bool_t bHasSkill = iSkillIndex >= 0 &&
			static_cast<size_t>(iSkillIndex) < pMode->Skills.size();
		if (bHasSkill)
		{
			m_pHUDRuntimeView->Set_SlotTexture(strIconSlot,
				pMode->Skills[static_cast<size_t>(iSkillIndex)].strIconPath);
		}
		/* No skill = the plain dark slot, exactly what the retail interaction layout
		shows for its unused keys. */
		m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, bHasSkill);

		const uint32_t iEndTick = kouku.CooldownEndTicks[i];
		const uint32_t iRemaining = (bHasSkill && iEndTick > player.iServerTick) ?
			iEndTick - player.iServerTick : 0u;
		if (0u == iRemaining)
		{
			m_pHUDRuntimeView->Set_SlotArcRatio(strCooldownSlot, 0.f);
			m_pHUDRuntimeView->Set_SlotVisible(strCooldownSlot, false);
			continue;
		}
		const f32_t fRemainingSeconds = static_cast<f32_t>(iRemaining) / SERVER_TICK_HZ;
		const f32_t fTotalSeconds = kouku.CooldownDurationTicks[i] > 0u ?
			static_cast<f32_t>(kouku.CooldownDurationTicks[i]) / SERVER_TICK_HZ : fRemainingSeconds;
		const f32_t fFraction = fTotalSeconds > 0.f ?
			(std::min)(1.f, (std::max)(0.f, fRemainingSeconds / fTotalSeconds)) : 0.f;
		m_pHUDRuntimeView->Set_SlotTint(strCooldownSlot, vCooldownTint);
		m_pHUDRuntimeView->Set_SlotArcRatio(strCooldownSlot, fFraction);
		m_pHUDRuntimeView->Set_SlotVisible(strCooldownSlot, true);
	}
}

void CMainApp::Update_LobbyButtons(const f32_t fTimeDelta)
{
	if (nullptr == m_pLobbyBackgroundView)
		return;
	if (ETOUI(LEVEL::LOBBY) != CGameInstance::Get().Get_CurrentLevelID())
	{
		m_pLobbyBackgroundView->Set_AllSlotsVisible(false);
		m_bLobbyWasActive = false;
		return;
	}

	/* The character-select window is modal over the Lobby: while it is open the Lobby
	backdrop/buttons hide and skip their own hover/click pass entirely --
	Update_CharacterSelectWindow owns the frame's pointer. */
	if (nullptr != m_pCharacterSelectWindowView &&
		m_pCharacterSelectWindowView->Is_Open())
	{
		m_pLobbyBackgroundView->Set_AllSlotsVisible(false);
		return;
	}

	/* Entering the Lobby (first run, or back from a stage) replays the retail logo reveal once;
	closing the character-select window does not (retail keeps the settled logo). */
	if (!m_bLobbyWasActive)
	{
		m_bLobbyWasActive = true;
		m_fLobbyLogoIntroElapsed = 0.f;
		m_pLobbyBackgroundView->Restart_Animation("Lobby_LogoIntro");
	}
	m_fLobbyLogoIntroElapsed += fTimeDelta;
	const bool_t bLogoIntroDone = m_fLobbyLogoIntroElapsed >= LOBBY_LOGO_INTRO_SECONDS;

	m_pLobbyBackgroundView->Set_AllSlotsVisible(true);
	m_pLobbyBackgroundView->Set_SlotVisible("Lobby_LogoIntro", !bLogoIntroDone);
	m_pLobbyBackgroundView->Set_SlotVisible("Lobby_Logo", bLogoIntroDone);

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pLobbyBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pLobbyBackgroundView->Get_ResolutionHeight();
	const auto HoverAndClick = [&](const LOBBY_PRODUCT_RECT& Rect, bool_t& outClicked)
	{
		outClicked = false;
		const bool_t bHovered = Router.Is_Hovered(
			Rect.fX, Rect.fY, Rect.fWidth, Rect.fHeight, fRefWidth, fRefHeight);
		if (bHovered)
		{
			Router.Claim_Mouse_This_Frame();
			outClicked = Router.Is_Clicked(Rect.fX, Rect.fY, Rect.fWidth, Rect.fHeight,
				fRefWidth, fRefHeight);
		}
		return bHovered;
	};

	/* Server rows (retail ServerRenderer: 'over' skin under the pointer, 'selected_up' skin on
	the chosen row; a click selects). Rows past the authored list stay hidden. */
	const size_t iRowCount = (std::min)(m_LobbyServers.size(), LOBBY_SERVER_ROW_COUNT);
	if (m_iLobbySelectedServer >= static_cast<int32_t>(iRowCount))
		m_iLobbySelectedServer = iRowCount > 0 ? 0 : -1;
	for (size_t i = 0; i < LOBBY_SERVER_ROW_COUNT; ++i)
	{
		const string strRowId = "Lobby_ServerRow_" + std::to_string(i);
		const string strCountId = "Lobby_ServerRowCount_" + std::to_string(i);
		if (i >= iRowCount)
		{
			m_pLobbyBackgroundView->Set_SlotVisible(strRowId, false);
			m_pLobbyBackgroundView->Set_SlotVisible(strCountId, false);
			continue;
		}
		LOBBY_PRODUCT_RECT Rect{};
		bool_t bClicked = false;
		const bool_t bHovered = Get_LobbySlotRect(m_pLobbyBackgroundView.get(),
			strRowId.c_str(), Rect) && HoverAndClick(Rect, bClicked);
		if (bClicked)
		{
			m_iLobbySelectedServer = static_cast<int32_t>(i);
			Play_UIButtonClickSound();
		}
		const bool_t bSelected = static_cast<int32_t>(i) == m_iLobbySelectedServer;
		m_pLobbyBackgroundView->Set_SlotTexture(strRowId, bSelected ?
			"UI/Lobby/ServerSelect/row_selected.png" :
			bHovered ? "UI/Lobby/ServerSelect/row_over.png" : "");
		/* Retail shows the head-count icon only for a server that still accepts characters;
		otherwise the row's characterState text ("생성 불가") takes that column. */
		m_pLobbyBackgroundView->Set_SlotVisible(strCountId, m_LobbyServers[i].bCreatable);
	}

	/* 접속(서버 선택) -- the old 게임 시작 button: opens the character-select window for the
	selected server. The window's own card still submits CHARACTER_SELECT. */
	{
		LOBBY_PRODUCT_RECT Rect{};
		bool_t bClicked = false;
		const bool_t bHovered = Get_LobbySlotRect(m_pLobbyBackgroundView.get(),
			"Lobby_JoinButton", Rect) && HoverAndClick(Rect, bClicked);
		const bool_t bEnabled = m_iLobbySelectedServer >= 0 &&
			nullptr != m_pCharacterSelectWindowView &&
			CLevel_Lobby::Can_SubmitProductCommand();
		m_pLobbyBackgroundView->Set_SlotTexture("Lobby_JoinButton",
			bHovered && bEnabled ? "UI/Lobby/ServerSelect/btn_join_over.png" : "");
		if (bClicked && bEnabled)
		{
			m_pCharacterSelectWindowView->Open();
			Play_UIButtonClickSound();
		}
	}

	/* Bottom icon buttons. 종료 closes the game (WM_CLOSE -> WM_DESTROY -> quit, the same
	path as the title-bar X); 뒤로 (retail: back to the login page) and 환경설정 have no
	product target here yet, so they only show their hover art. */
	struct LOBBY_ICON_BUTTON
	{
		const char* pSlotId;
		const char* pOverTexture;
		bool_t bExit;
	};
	constexpr LOBBY_ICON_BUTTON IconButtons[3] =
	{
		{ "Lobby_ExitIcon", "UI/Lobby/ServerSelect/btn_exit_over.png", true },
		{ "Lobby_PrevIcon", "UI/Lobby/ServerSelect/btn_prev_over.png", false },
		{ "Lobby_OptionIcon", "UI/Lobby/ServerSelect/btn_option_over.png", false },
	};
	for (const LOBBY_ICON_BUTTON& Icon : IconButtons)
	{
		LOBBY_PRODUCT_RECT Rect{};
		if (!Get_LobbySlotRect(m_pLobbyBackgroundView.get(), Icon.pSlotId, Rect))
			continue;
		/* Retail's hit area is the 118x82 button group (icon + caption), not just the icon. */
		const LOBBY_PRODUCT_RECT HitRect{ Rect.fX - 26.f, Rect.fY - 6.f, Rect.fWidth + 52.f, 54.f };
		bool_t bClicked = false;
		const bool_t bHovered = HoverAndClick(HitRect, bClicked);
		m_pLobbyBackgroundView->Set_SlotTexture(Icon.pSlotId, bHovered ? Icon.pOverTexture : "");
		if (bClicked && Icon.bExit)
		{
			Play_UIButtonClickSound();
			PostMessage(g_hWnd, WM_CLOSE, 0, 0);
		}
	}

	/* TitleBackground's looping movie flipbook and the one-shot logo reveal. */
	m_pLobbyBackgroundView->Update(fTimeDelta);
}

void CMainApp::Update_CharacterSelectWindow(const f32_t fTimeDelta)
{
	if (nullptr == m_pCharacterSelectWindowView)
		return;
	if (ETOUI(LEVEL::LOBBY) != CGameInstance::Get().Get_CurrentLevelID())
	{
		/* A level change while open (the submitted CHARACTER_SELECT approval landing) must
		not leave the window's sprites showing under the next level -- they live on
		LEVEL::STATIC. */
		if (m_pCharacterSelectWindowView->Is_Open())
			m_pCharacterSelectWindowView->Close();
		return;
	}

	m_pCharacterSelectWindowView->Update(fTimeDelta);

	switch (m_pCharacterSelectWindowView->Consume_Intent())
	{
	case CCharacterSelectWindowView::INTENT::NEW_CHARACTER:
		/* The same product command the Lobby button used to submit directly. The window
		stays open while the Server approval runs -- success changes the level (the branch
		above closes it), a rejection leaves the window up and the user can ESC back to the
		Lobby's status line. */
		(void)CLevel_Lobby::Submit_ProductCommand(LOBBY_STAGE::CHARACTER_SELECT);
		break;
	case CCharacterSelectWindowView::INTENT::CLOSE:
		m_pCharacterSelectWindowView->Close();
		break;
	default:
		break;
	}
}

void CMainApp::RenderCharacterSelectWindowText()
{
	if (nullptr == m_pCharacterSelectWindowView)
		return;
	if (ETOUI(LEVEL::LOBBY) != CGameInstance::Get().Get_CurrentLevelID())
		return;
	m_pCharacterSelectWindowView->RenderText();
}

void CMainApp::Update_Minimap(const f32_t fTimeDelta)
{
	if (nullptr == m_pMinimapView)
		return;
	/* Each level owns its own CClientReplication, so the active one is asked for the marker
	snapshot the same way the party window reaches Get_PartyRoster(). */
	const uint32_t iLevel = CGameInstance::Get().Get_CurrentLevelID();
	CClientReplication::MINIMAP_MARKER_SNAPSHOT Snapshot{};
	LEVEL eLevel = LEVEL::END;
	bool_t bHasSnapshot = false;
	if (ETOUI(LEVEL::BERN) == iLevel)
	{
		if (CLevel_Bern* pBern = CLevel_Bern::Get_Active())
		{
			pBern->Collect_MinimapMarkers(Snapshot);
			eLevel = LEVEL::BERN;
			bHasSnapshot = true;
		}
	}
	else if (ETOUI(LEVEL::VALTAN_ARENA) == iLevel)
	{
		if (CLevel_ValtanArena* pValtanArena = CLevel_ValtanArena::Get_Active())
		{
			pValtanArena->Collect_MinimapMarkers(Snapshot);
			eLevel = LEVEL::VALTAN_ARENA;
			bHasSnapshot = true;
		}
	}
	else if (ETOUI(LEVEL::KAKULSAYDON_ARENA) == iLevel)
	{
		if (CLevel_KakulSaydonArena* pKakulSaydonArena = CLevel_KakulSaydonArena::Get_Active())
		{
			pKakulSaydonArena->Collect_MinimapMarkers(Snapshot);
			eLevel = LEVEL::KAKULSAYDON_ARENA;
			bHasSnapshot = true;
		}
	}
	m_pMinimapView->Update(fTimeDelta, eLevel, bHasSnapshot ? &Snapshot : nullptr);
}

void CMainApp::RenderMinimapText()
{
	if (nullptr != m_pMinimapView)
		m_pMinimapView->RenderText();
}

void CMainApp::RenderLobbyButtonText()
{
	if (ETOUI(LEVEL::LOBBY) != CGameInstance::Get().Get_CurrentLevelID())
		return;
	if (nullptr == m_pLobbyBackgroundView)
		return;
	/* The Lobby's sprites are hidden while the character-select window is open -- their
	labels (and the Release status line) must not float over it either. */
	if (nullptr != m_pCharacterSelectWindowView &&
		m_pCharacterSelectWindowView->Is_Open())
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	/* Retail font sizes are 1920x1080 pixels; fFontPx is that x 2/3 (1280 reference units).
	fAlign: 0 = left edge at fX, 0.5 = centered on fX, 1 = right edge at fX. */
	const auto DrawAt = [&](const wchar_t* pText, const wstring& strFont, f32_t fFontPx,
		f32_t fX, f32_t fCenterY, f32_t fAlign, f32_t fMaxWidth, fvector_t vColor)
	{
		if (nullptr == pText || 0 == pText[0])
			return;
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, pText);
		f32_t fScale = (vMeasured.y > 0.f) ? (fFontPx / vMeasured.y) : 1.f;
		if (fMaxWidth > 0.f && vMeasured.x * fScale > fMaxWidth)
			fScale = fMaxWidth / vMeasured.x;
		CGameInstance::Get().Draw_Text(strFont, pText,
			float2_t(fX * textScaleX, fCenterY * textScaleY),
			vColor, 0.f, float2_t(fAlign, 0.5f), fScale * textUiScale);
	};
	const auto DrawInSlot = [&](const char* pSlotId, const wchar_t* pText,
		const wstring& strFont, f32_t fFontPx, fvector_t vColor)
	{
		LOBBY_PRODUCT_RECT Rect{};
		if (!Get_LobbySlotRect(m_pLobbyBackgroundView.get(), pSlotId, Rect))
			return;
		DrawAt(pText, strFont, fFontPx, Rect.fX + Rect.fWidth * 0.5f,
			Rect.fY + Rect.fHeight * 0.5f, 0.5f, Rect.fWidth, vColor);
	};
	const auto Rgb = [](uint32_t iHex)
	{
		return XMVectorSet(((iHex >> 16) & 0xFF) / 255.f, ((iHex >> 8) & 0xFF) / 255.f,
			(iHex & 0xFF) / 255.f, 1.f);
	};
	const wstring strYoon = TEXT("Font_YoonGasiIIM");
	const wstring strYG760 = TEXT("Font_YG760");

	/* Panel title ($YoonGasiIIM 22 #fff7e2) and the list header row ($YoonGasiIIM 18 #d5ac66):
	서버 / 상태 / 캐릭터. */
	DrawInSlot("Lobby_ServerTitleText", L"\xC11C\xBC84 \xC120\xD0DD", strYoon, 20.f, Rgb(0xfff7e2));
	DrawInSlot("Lobby_ServerHeaderName", L"\xC11C\xBC84", strYoon, 18.f, Rgb(0xd5ac66));
	DrawInSlot("Lobby_ServerHeaderState", L"\xC0C1\xD0DC", strYoon, 18.f, Rgb(0xd5ac66));
	DrawInSlot("Lobby_ServerHeaderChar", L"\xCE90\xB9AD\xD130", strYoon, 18.f, Rgb(0xd5ac66));

	/* Rows (retail ServerRenderer, 626x37 at 1920 -> x 2/3): typeMc tag at x+10 ($YoonGasiIIM
	16, 신규 #ffd43e / 추천 #00aeff / else #22facf), serverName centered in 238 at x+62,
	serverState centered in 137 at x+314, then either the head-count icon + number ($YG760 14,
	left at x+548) or the characterState text centered in 124 at x+476. The state colours are
	this project's reading of the retail palette (원활 green / 보통 amber / 혼잡 red). */
	const size_t iRowCount = (std::min)(m_LobbyServers.size(), LOBBY_SERVER_ROW_COUNT);
	for (size_t i = 0; i < iRowCount; ++i)
	{
		const LOBBY_SERVER_ENTRY& Entry = m_LobbyServers[i];
		LOBBY_PRODUCT_RECT Rect{};
		if (!Get_LobbySlotRect(m_pLobbyBackgroundView.get(),
			("Lobby_ServerRow_" + std::to_string(i)).c_str(), Rect))
			continue;
		const f32_t fCenterY = Rect.fY + 12.333f;
		if (!Entry.strTag.empty())
		{
			const fvector_t vTagColor = Entry.strTag == L"\xC2E0\xADDC" ? Rgb(0xffd43e) :
				Entry.strTag == L"\xCD94\xCC9C" ? Rgb(0x00aeff) : Rgb(0x22facf);
			DrawAt(Entry.strTag.c_str(), strYoon, 16.f, Rect.fX + 6.667f, fCenterY,
				0.f, 52.f, vTagColor);
		}
		DrawAt(Entry.strName.c_str(), strYG760, 16.f, Rect.fX + 41.333f + 79.333f,
			fCenterY, 0.5f, 158.667f, Colors::White);
		const fvector_t vStateColor = Entry.strState == L"\xC6D0\xD65C" ? Rgb(0x7ed957) :
			Entry.strState == L"\xBCF4\xD1B5" ? Rgb(0xffd43e) :
			Entry.strState == L"\xD63C\xC7A1" ? Rgb(0xff6a5a) : Colors::White;
		DrawAt(Entry.strState.c_str(), strYG760, 16.f, Rect.fX + 209.333f + 45.667f,
			fCenterY, 0.5f, 91.333f, vStateColor);
		if (Entry.bCreatable)
		{
			const wstring strCount = std::to_wstring(Entry.iCharacterCount);
			DrawAt(strCount.c_str(), strYG760, 16.f, Rect.fX + 365.333f, fCenterY,
				0.f, 33.333f, Colors::White);
		}
		else
		{
			DrawAt(L"\xC0DD\xC131 \xBD88\xAC00", strYG760, 16.f, Rect.fX + 317.333f + 41.333f,
				fCenterY, 0.5f, 82.667f, Rgb(0xb0b0b0));
		}
	}

	/* 접속(서버 선택) button caption ($YG760 16 x 1.278 button scale), the three icon-button
	captions ($YG760 14, 45px under the button top at 1920), and the copyright line. */
	{
		LOBBY_PRODUCT_RECT Rect{};
		if (Get_LobbySlotRect(m_pLobbyBackgroundView.get(), "Lobby_JoinButton", Rect))
		{
			const bool_t bEnabled = m_iLobbySelectedServer >= 0;
			DrawAt(L"\xC11C\xBC84 \xC120\xD0DD", strYG760, 13.6f, Rect.fX + Rect.fWidth * 0.5f,
				Rect.fY + 16.2f, 0.5f, Rect.fWidth - 8.f,
				bEnabled ? Colors::White : Rgb(0x787878));
		}
	}
	struct LOBBY_ICON_CAPTION { const char* pSlotId; const wchar_t* pLabel; f32_t fCenterX; };
	const LOBBY_ICON_CAPTION IconCaptions[3] =
	{
		{ "Lobby_ExitIcon", L"\xC885\xB8CC", 54.667f },
		{ "Lobby_PrevIcon", L"\xB4A4\xB85C", 133.333f },
		{ "Lobby_OptionIcon", L"\xD658\xACBD\xC124\xC815", 1226.667f },
	};
	for (const LOBBY_ICON_CAPTION& Caption : IconCaptions)
	{
		LOBBY_PRODUCT_RECT Rect{};
		if (!Get_LobbySlotRect(m_pLobbyBackgroundView.get(), Caption.pSlotId, Rect))
			continue;
		DrawAt(Caption.pLabel, strYG760, 9.333f, Caption.fCenterX, 672.f, 0.5f, 100.f,
			Colors::White);
	}
	DrawAt(L"(C) Smilegate RPG, Inc. All rights reserved.", strYG760, 9.333f, 640.f,
		711.f, 0.5f, 800.f, Rgb(0xeeeeee));

#ifndef _DEBUG
	/* Release product status line (Debug shows the same status inside the Lobby debug panel
	instead). Was an ImGui wrapped-text draw; Draw_Text has no wrapping, so the whole line is
	scaled to fit the status rect's width instead -- status strings are one sentence. */
	LOBBY_PRODUCT_RECT StatusRect{ 240.f, 566.f, 800.f, 54.f };
	LOBBY_PRODUCT_RECT AuthoredStatusRect{};
	if (m_pLobbyBackgroundView->Get_SlotRect(
		"Lobby_StatusText", AuthoredStatusRect.fX, AuthoredStatusRect.fY,
		AuthoredStatusRect.fWidth, AuthoredStatusRect.fHeight) &&
		Is_ValidProductRect(AuthoredStatusRect))
	{
		StatusRect = AuthoredStatusRect;
	}
	const string strStatus = CLevel_Lobby::Get_ProductStatus();
	if (!strStatus.empty())
	{
		wstring strWideStatus;
		const int iRequiredLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strStatus.data(), static_cast<int>(strStatus.size()), nullptr, 0);
		if (iRequiredLength > 0)
		{
			strWideStatus.resize(static_cast<size_t>(iRequiredLength));
			if (iRequiredLength == MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
				strStatus.data(), static_cast<int>(strStatus.size()),
				strWideStatus.data(), iRequiredLength))
			{
				const float2_t vStatusMeasured = CGameInstance::Get().Measure_Text(
					TEXT("Font_YoonGasiIIM"), strWideStatus.c_str());
				const f32_t fScaleByHeight = (vStatusMeasured.y > 0.f) ?
					(16.f / vStatusMeasured.y) : 1.f;
				const f32_t fScaleByWidth = (vStatusMeasured.x > 0.f) ?
					((StatusRect.fWidth - 16.f) / vStatusMeasured.x) : 1.f;
				const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
				const f32_t fCenterX = StatusRect.fX + StatusRect.fWidth * 0.5f;
				const f32_t fCenterY = StatusRect.fY + StatusRect.fHeight * 0.5f;
				CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strWideStatus.c_str(),
					float2_t(fCenterX * textScaleX + 1.f, fCenterY * textScaleY + 1.f),
					XMVectorSet(0.f, 0.f, 0.f, 220.f / 255.f), 0.f, float2_t(0.5f, 0.5f),
					fScale * textUiScale);
				CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strWideStatus.c_str(),
					float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
					XMVectorSet(1.f, 225.f / 255.f, 150.f / 255.f, 1.f), 0.f,
					float2_t(0.5f, 0.5f), fScale * textUiScale);
			}
		}
	}
#endif
}

void CMainApp::Load_LobbyServers()
{
	m_LobbyServers.clear();
	m_iLobbySelectedServer = -1;
	const filesystem::path DataPath = CProjectDataRoot::Resolve(L"UI/Lobby/LobbyServers.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
	{
		OutputDebugStringA("[Lobby] LobbyServers.json missing -- server list stays empty.\n");
		return;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		OutputDebugStringA(("[Lobby] LobbyServers.json parse failed: " + Error + "\n").c_str());
		return;
	}
	const DATA_JSON_VALUE* pServers = Root.Find("servers");
	if (nullptr == pServers || !pServers->Is_Array())
		return;
	for (const DATA_JSON_VALUE& Value : pServers->Get_Array())
	{
		if (!Value.Is_Object())
			continue;
		LOBBY_SERVER_ENTRY Entry{};
		const auto ReadText = [&](const char* pKey, wstring& outText)
		{
			const DATA_JSON_VALUE* pText = Value.Find(pKey);
			if (nullptr != pText && pText->Is_String())
				(void)ConvertUtf8ToWide(pText->Get_String(), outText);
		};
		ReadText("name", Entry.strName);
		ReadText("state", Entry.strState);
		ReadText("tag", Entry.strTag);
		if (const DATA_JSON_VALUE* pCount = Value.Find("characterCount"))
			if (pCount->Is_Number() && pCount->Get_Number() >= 0.0)
				Entry.iCharacterCount = static_cast<uint32_t>(pCount->Get_Number());
		if (const DATA_JSON_VALUE* pCreatable = Value.Find("creatable"))
			if (pCreatable->Is_Boolean())
				Entry.bCreatable = pCreatable->Get_Boolean();
		if (Entry.strName.empty())
			continue;
		m_LobbyServers.push_back(std::move(Entry));
		if (m_LobbyServers.size() >= LOBBY_SERVER_ROW_COUNT)
			break;
	}
	if (!m_LobbyServers.empty())
		m_iLobbySelectedServer = 0;
}

void CMainApp::RenderItemUpgradeButtonText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

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
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

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
	the user to nudge in the Tool rather than an exact traced position. Rows only draw for items
	actually present in the real "combat"-category inventory (BuildItemUpgradeSlots) -- fewer than
	6 owned items just leaves the remaining ItemUpgrade_ListLevel/ItemUpgrade_ListItemName slots
	blank, same as any other real inventory-backed list. */
	const vector<ITEM_UPGRADE_SLOT_INFO> upgradeSlots = BuildItemUpgradeSlots();
	// real sampled reference pixels (list row level text):
	// level = (255,189,74) same gold as curLevel_lb; name = (227,199,161) warm cream, not white.
	const fvector_t vLevelColor = XMVectorSet(1.0f, 0.7412f, 0.2902f, 1.f); // #FFBD4A
	const fvector_t vNameColor = XMVectorSet(0.8902f, 0.7804f, 0.6314f, 1.f); // #E3C7A1
	for (int32_t i = 0; i < 6 && i < static_cast<int32_t>(upgradeSlots.size()); ++i)
	{
		const string strLevelSlot = "ItemUpgrade_ListLevel" + to_string(i);
		const string strNameSlot = "ItemUpgrade_ListItemName" + to_string(i);
		const wstring strLevel =
			to_wstring(ItemUpgradeLevelRef(upgradeSlots[i].strItemId)) + L"\xB2E8\xACC4"; // "N단계"
		DrawFit(strLevelSlot.c_str(), strLevel.c_str(), 0.765f, vLevelColor); // "18단계" (0.85 * 0.9)
		DrawFit(strNameSlot.c_str(), upgradeSlots[i].strName.c_str(), 0.72f, vNameColor); // 0.8 * 0.9
	}

	/* Right 재련 단계 list: 7 rows now (JSON grew GradeRowEmblem/GradeStripB/GradeRowText from 4 to
	7, evenly filling the panel from its top edge down) -- the ask was more row slots, not more
	stat lines per row, so this stays at 1 stat line ("공격력 +N") like before. Real reference
	scrolls higher levels at the TOP and the current level at the BOTTOM (numbers increase
	bottom -> top), so row 0 (topmost) is 6 above the current level and row 6 (bottom, nearest the
	gauge) is the selected item's own CURRENT level -- 10 -> 11 reforge highlights 10, the level
	you're actually standing at, not 11 (that's the separate curLevel/nextLevel ">>>" display
	elsewhere). Computed from the selected item's own tracked level instead of a fixed literal so
	this ladder shifts with the real level instead of staying frozen at the old 19/25 placeholder
	range. GradeSelectedExample sits on row 6 to match. Non-selected rows sample as a muted gray
	(real 24/23/22단계 rows, (103,103,103)); the selected (현재) row uses gold for the level and
	white for its stat, matching every other "selected" element in this window reading brighter
	than its neighbors. Stat is a placeholder "공격력 +N" (N = that row's own level) until real
	per-level balance data exists. */
	const fvector_t vRowGray = XMVectorSet(0.4039f, 0.4039f, 0.4039f, 1.f); // #676767
	const fvector_t vSelectedGold = XMVectorSet(1.0f, 0.7412f, 0.2902f, 1.f); // #FFBD4A
	const int32_t ROW_COUNT = 7;
	const int32_t iSelectedForLadder = upgradeSlots.empty() ? -1 : std::clamp(
		m_iItemUpgradeSelectedSlot, 0, static_cast<int32_t>(upgradeSlots.size()) - 1);
	const int32_t iCurrentLevel = (iSelectedForLadder >= 0) ?
		ItemUpgradeLevelRef(upgradeSlots[iSelectedForLadder].strItemId) : 10;
	int32_t ROW_LEVELS[ROW_COUNT];
	for (int32_t i = 0; i < ROW_COUNT; ++i)
		ROW_LEVELS[i] = iCurrentLevel + (ROW_COUNT - 1 - i);
	for (int32_t i = 0; i < ROW_COUNT; ++i)
	{
		const string strSlot = "ItemUpgrade_GradeRowText" + to_string(i);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(strSlot.c_str(), fX, fY, fWidth, fHeight))
			continue;

		const bool_t bSelected = (iCurrentLevel == ROW_LEVELS[i]);
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
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	f32_t fListX = 0.f, fListY = 0.f, fListWidth = 0.f, fListHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect(
		"ItemUpgrade_LeftListBg", fListX, fListY, fListWidth, fListHeight))
	{
		return;
	}

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pItemUpgradeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pItemUpgradeView->Get_ResolutionHeight();
	const vector<ITEM_UPGRADE_SLOT_INFO> upgradeSlots = BuildItemUpgradeSlots();

	for (int32_t i = 0; i < 6 && i < static_cast<int32_t>(upgradeSlots.size()); ++i)
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
		const bool_t bHovered = Router.Is_Hovered(
			fListX, fRowY, fListWidth, fRowHeight, fRefWidth, fRefHeight);
		if (!bHovered)
			continue;
		Router.Claim_Mouse_This_Frame();
		if (!Router.Is_Clicked(fListX, fRowY, fListWidth, fRowHeight, fRefWidth, fRefHeight))
			continue;

		Play_UIButtonClickSound();
		m_iItemUpgradeSelectedSlot = i;

		f32_t fTargetX = 0.f, fTargetY = 0.f, fTargetWidth = 0.f, fTargetHeight = 0.f;
		if (m_pItemUpgradeView->Get_SlotRect(
			strGradeBgSlot.c_str(), fTargetX, fTargetY, fTargetWidth, fTargetHeight))
		{
			m_pItemUpgradeView->Set_SlotPosition(
				"ItemUpgrade_ListSelectedExample", fTargetX, fTargetY);
		}
		m_pItemUpgradeView->Set_SlotTexture(
			"ItemUpgrade_SelectedItemIcon", upgradeSlots[i].strIconPath);
		break;
	}
}

void CMainApp::Update_ItemUpgradeGrowButton()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_LevelUpBtn", fX, fY, fWidth, fHeight))
		return;

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pItemUpgradeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pItemUpgradeView->Get_ResolutionHeight();
	if (!Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;
	Router.Claim_Mouse_This_Frame();
	if (!Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;

	Play_UIButtonClickSound();

	/* (Re)starts the fill from 0 even if a previous run already completed -- a debug preview
	button is expected to be repeatable. Hides the 100%-only art immediately so a re-click during
	the held-100 state doesn't leave it showing through the new fill. */
	m_iItemUpgradePreviousPercent = 0;
	m_bItemUpgradeGrowing = true;
	m_dItemUpgradeGrowStartSeconds = Product_Now_Seconds();
	m_bItemUpgradeCoreFlashPending = true;
	m_dItemUpgradeShockwaveScheduledAt = -1.0;
	m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", false);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", false);

	const filesystem::path growSoundPath = CRuntimeAssetRoot::Resolve(
		L"Sound/UI/Enhancement/sys_enhance_levelup_growup_full1__465402134.wav");
	CGameInstance::Get().Play_Sound(growSoundPath.wstring(), 1.f);
}

void CMainApp::Update_ItemUpgradeReforgeButton()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		m_bItemUpgradeGrowing || 100 != m_iItemUpgradePreviousPercent ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_ReforgeButton", fX, fY, fWidth, fHeight))
		return;

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pItemUpgradeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pItemUpgradeView->Get_ResolutionHeight();
	if (!Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;
	Router.Claim_Mouse_This_Frame();
	if (!Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;

	Play_UIButtonClickSound();

	/* Placeholder pass/fail rate -- Data/Balance has no real 재련 success-rate field yet, so this
	is a flat 50% purely so both result screens can be exercised while testing. The Client never
	owns real success/fail authority; replace with a real Server-resolved outcome once one exists,
	the same way every other "no real Server data yet" placeholder in this preview is flagged.
	Rolled now but held in m_bItemUpgradePendingAttemptSuccess -- not shown until
	Update_ItemUpgradeResultWaitClick() reveals it, matching the real "화면을 클릭하여 결과 즉시
	확인" suspense screen instead of an instant reveal.
	static std::mt19937, not std::rand(): std::rand() is never seeded (no srand() call anywhere
	in this codebase) so its first call after process start is always the same fixed value --
	every fresh session's first 재련 attempt was landing on the same outcome every time. */
	static std::mt19937 s_itemUpgradeRng{ std::random_device{}() };
	m_bItemUpgradePendingAttemptSuccess = (s_itemUpgradeRng() % 100) < 50;
	m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::WAITING;
	// The wait screen replaces the whole window's content, not just the reforge button --
	// snap the gauge back to idle 0% now instead of leaving the old 100% fill visible behind it,
	// and hide the base window's own icon/gauge/button content so it doesn't bleed through
	// (real Lost Ark's wait/result screens read as their own separate screen).
	Reset_ItemUpgradeIdleGauge();
	Set_ItemUpgradeCenterPanelVisible(false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitBg", true);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", true);
	m_pItemUpgradeView->Restart_Animation("ItemUpgrade_ResultWaitEmblem");

	/* UI owns this loop independently; level/encounter BGM keeps playing. */
	const filesystem::path waitSoundPath = CRuntimeAssetRoot::Resolve(
		L"Sound/UI/Enhancement/sys_enhance_3_waiting1__95424590.wav");
	CGameInstance::Get().Play_LoopingSound(waitSoundPath.wstring(), 1.f);
}

void CMainApp::Update_ItemUpgradeResultWaitClick()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::WAITING != m_eItemUpgradeAttemptResult ||
		!CUIInputRouter::Get().Is_LeftClickEdge())
	{
		return;
	}
	// This screen already owns the whole frame's input while WAITING is showing (no other
	// ItemUpgrade widget's Is_Hovered/Is_Clicked can be true at the same time -- the base
	// window's own buttons are hidden by Set_ItemUpgradeCenterPanelVisible(false)), so claiming
	// the mouse here is a formality for consistency with every other real click this router sees.
	CUIInputRouter::Get().Claim_Mouse_This_Frame();

	// ItemUpgrade_ResultWaitBg (the same solid-black backdrop already showing behind the wait
	// circle) stays visible all the way through burst-playing and the settled result -- it's the
	// one background for this whole screen, not something to swap out mid-flow. Only the dismiss
	// (OK button) turns it off, back to the normal reforge window.
	// While the burst plays, ONLY the SmeltSuccess/Fail burst shows (the SmeltLoding wait loop is
	// switched off below as the burst starts) -- no icon/name/result text/OK button yet (those
	// would sit on top of the burst otherwise). Once the burst's own real one-shot duration
	// finishes, the per-frame settle check in Update() hides the burst and reveals the
	// icon/name/result content in its place.
	/* Real smelt_SWeffect / smelt_FailEffectComp movies (ItemUpgradeUI.json SmeltSuccess/
	SmeltFail flipbooks) are 120 frames at 30 fps -- must match the JSON frame count so the
	result content reveals exactly when the burst's last frame lands. */
	constexpr f64_t RESULT_BURST_DURATION_SECONDS = 120.0 / 30.0;
	m_dItemUpgradeResultSettleAt = Product_Now_Seconds() + RESULT_BURST_DURATION_SECONDS;

	const bool_t bSuccess = m_bItemUpgradePendingAttemptSuccess;
	m_eItemUpgradeAttemptResult = bSuccess ?
		ITEM_UPGRADE_ATTEMPT_RESULT::SUCCESS : ITEM_UPGRADE_ATTEMPT_RESULT::FAIL;
	// Real reforge result screens dim the reforge window itself (drawn above, in Update()) instead
	// of an opaque bounded panel image -- SuccessModalBg/FailModalBg are never shown.
	// The wait loop (SmeltLoding) stops the moment the result burst starts -- the real
	// smelt_SWeffect / smelt_FailEffectComp clips are distinct art from the loading loop, so
	// leaving it additively underneath would double-expose the burst.
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessEffect", bSuccess);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailEffect", !bSuccess);
	if (bSuccess)
		m_pItemUpgradeView->Restart_Animation("ItemUpgrade_SuccessEffect");
	else
		m_pItemUpgradeView->Restart_Animation("ItemUpgrade_FailEffect");

	CGameInstance::Get().Stop_LoopingSound();
	// Equally-weighted variants (the same real success/fail vox recorded several times),
	// same pattern CSoundCueCatalog already documents for Character/Valtan cues.
	static std::mt19937 s_itemUpgradeResultSoundRng{ std::random_device{}() };
	if (bSuccess)
	{
		static const wchar_t* const SUCCESS_SOUNDS[] = {
			L"Sound/UI/Enhancement/sys_enhance_1_success1__168050678.wav",
			L"Sound/UI/Enhancement/sys_enhance_2_success1__225949772.wav",
			L"Sound/UI/Enhancement/sys_enhance_4_success1__596670890.wav",
		};
		const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
			SUCCESS_SOUNDS[s_itemUpgradeResultSoundRng() % 3u]);
		CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
	}
	else
	{
		static const wchar_t* const FAIL_SOUNDS[] = {
			L"Sound/UI/Enhancement/sys_enhance_3_casting_fail1__668013724.wav",
			L"Sound/UI/Enhancement/sys_enhance_4_casting_fail1__490424786.wav",
		};
		const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
			FAIL_SOUNDS[s_itemUpgradeResultSoundRng() % 2u]);
		CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
	}
}

void CMainApp::Update_ItemUpgradeResultOkButton()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		(ITEM_UPGRADE_ATTEMPT_RESULT::SUCCESS != m_eItemUpgradeAttemptResult &&
		 ITEM_UPGRADE_ATTEMPT_RESULT::FAIL != m_eItemUpgradeAttemptResult) ||
		// Not settled yet (burst still playing) -- the OK button isn't shown yet either, so a click
		// landing on its still-unrevealed rect must not dismiss early.
		m_dItemUpgradeResultSettleAt >= 0.0)
	{
		return;
	}

	const char_t* pOkButtonSlotId =
		ITEM_UPGRADE_ATTEMPT_RESULT::SUCCESS == m_eItemUpgradeAttemptResult ?
		"ItemUpgrade_SuccessOkBtn" : "ItemUpgrade_FailOkBtn";

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect(pOkButtonSlotId, fX, fY, fWidth, fHeight))
		return;

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pItemUpgradeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pItemUpgradeView->Get_ResolutionHeight();
	if (!Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;
	Router.Claim_Mouse_This_Frame();
	if (!Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;

	Play_UIButtonClickSound();

	m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::NONE;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessModalBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessOkBtn", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailModalBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailOkBtn", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessEffect", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailEffect", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondWinged", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondFrame", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessItemIconMarker", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailDiamondFrame", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailItemIconMarker", false);

	// Dismissing a result always returns the gauge to a clean idle (0%) state so "성장" can be
	// tried again, and brings the base window's own content back now that no modal covers it.
	Reset_ItemUpgradeIdleGauge();
	Set_ItemUpgradeCenterPanelVisible(true);
}

void CMainApp::Reset_ItemUpgradeIdleGauge()
{
	m_iItemUpgradePreviousPercent = 0;
	m_bItemUpgradeGrowing = false;
	m_dItemUpgradeGrowStartSeconds = -1.0;
	m_bItemUpgradeCoreFlashPending = false;
	m_dItemUpgradeShockwaveScheduledAt = -1.0;
	m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	m_dItemUpgradeResultSettleAt = -1.0;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", false);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", false);
}

int32_t& CMainApp::ItemUpgradeLevelRef(const string& strItemId)
{
	return m_ItemUpgradeLevels.try_emplace(strItemId, 10).first->second;
}

void CMainApp::Set_ItemUpgradeCenterPanelVisible(bool_t bVisible)
{
	if (nullptr == m_pItemUpgradeView)
		return;

	constexpr const char_t* CENTER_PANEL_SLOTS[] =
	{
		"ItemUpgrade_PanelBg",
		"ItemUpgrade_RecipeIconBgExample", "ItemUpgrade_RecipeMaterial0", "ItemUpgrade_RecipeAmount0",
		"ItemUpgrade_RecipeIconBg1", "ItemUpgrade_RecipeMaterial1", "ItemUpgrade_RecipeAmount1",
		"ItemUpgrade_RecipeIconBg2", "ItemUpgrade_RecipeMaterial2", "ItemUpgrade_RecipeAmount2",
		"ItemUpgrade_DecoIcon", "ItemUpgrade_GaugeFill", "ItemUpgrade_SelectedItemIconBounds",
		"ItemUpgrade_SelectedItemIcon", "ItemUpgrade_LevelUpBtn", "ItemUpgrade_EquipExpPageLine",
		"ItemUpgrade_ReforgeButton", "ItemUpgrade_LevelArrowBase", "ItemUpgrade_LevelArrow",
	};
	for (const char_t* pSlotId : CENTER_PANEL_SLOTS)
		m_pItemUpgradeView->Set_SlotVisible(pSlotId, bVisible);

	// Completion-effect slots only ever get hidden here (entering a result), never force-restored --
	// Reset_ItemUpgradeIdleGauge/Update_ItemUpgradeGrowButton own when these come back on.
	if (!bVisible)
	{
		constexpr const char_t* CENTER_PANEL_EFFECT_SLOTS[] =
		{
			"ItemUpgrade_WingedRingGold", "ItemUpgrade_LevelUpMotion2Big", "ItemUpgrade_SmeltGlow",
			"ItemUpgrade_CoreFlash", "ItemUpgrade_ShockwaveRing", "ItemUpgrade_CompleteEffect",
			"ItemUpgrade_WingDecoFade",
		};
		for (const char_t* pSlotId : CENTER_PANEL_EFFECT_SLOTS)
			m_pItemUpgradeView->Set_SlotVisible(pSlotId, false);
	}
}

void CMainApp::RenderItemUpgradeLevelText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	// colors are the real ItemBuildUpLevelWndContent/ItemBuildUpLevelGroupMc .as
	// itemName_lb.color=13769983(0xD21CFF), curLevel_lb.color=16760138(0xFFBD4A),
	// nextLevel_lb.color=12057344(0xB7FB00). The ">>>" arrow is a real animated
	// flourish icon (ItemUpgrade_LevelArrow AnimationFrames), not text.
	// The item name/level tracks m_iItemUpgradeSelectedSlot (Update_ItemUpgradeSelection) into the
	// real "combat"-category inventory (BuildItemUpgradeSlots) -- nothing to show while empty.
	const vector<ITEM_UPGRADE_SLOT_INFO> upgradeSlots = BuildItemUpgradeSlots();
	if (upgradeSlots.empty())
		return;
	const int32_t iSelectedSlot = std::clamp(
		m_iItemUpgradeSelectedSlot, 0, static_cast<int32_t>(upgradeSlots.size()) - 1);
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_ItemNameLabel", fX, fY, fWidth, fHeight))
		{
			const wchar_t* pLabel = upgradeSlots[iSelectedSlot].strName.c_str();
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

	// "N단계" / "(N+1)단계" -- reads the same real per-item level state everything else in this
	// preview now shares (ItemUpgradeLevelRef), instead of a fixed "18단계"/"19단계" literal.
	const int32_t iLevel = ItemUpgradeLevelRef(upgradeSlots[iSelectedSlot].strItemId);
	const wstring strCurLevel = to_wstring(iLevel) + L"\xB2E8\xACC4";
	const wstring strNextLevel = to_wstring(iLevel + 1) + L"\xB2E8\xACC4";
	struct LEVEL_TEXT_ENTRY
	{
		const char* pSlotId;
		const wstring& strLabel;
		f32_t fHeightRatio;
		fvector_t vColor;
	};
	const LEVEL_TEXT_ENTRY SLOTS[] =
	{
		{ "ItemUpgrade_CurLevelLabel", strCurLevel, 0.95f,
			XMVectorSet(1.0f, 0.7412f, 0.2902f, 1.f) },
		{ "ItemUpgrade_NextLevelLabel", strNextLevel, 0.95f,
			XMVectorSet(0.7176f, 0.9843f, 0.0f, 1.f) },
	};

	for (const LEVEL_TEXT_ENTRY& Slot : SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(Slot.pSlotId, fX, fY, fWidth, fHeight))
			continue;

		const f32_t fCenterX = fX + fWidth * 0.5f;
		const f32_t fCenterY = fY + fHeight * 0.5f;

		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), Slot.strLabel.c_str());
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * Slot.fHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), Slot.strLabel.c_str(),
			float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
			Slot.vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	}
}

void CMainApp::RenderItemUpgradeMaterialCounts()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

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
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

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

void CMainApp::RenderItemUpgradeResultWaitText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::WAITING != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_ResultWaitBg", fX, fY, fWidth, fHeight))
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	/* "화면을 클릭하여 결과 즉시 확인" -- real suspense-screen prompt, sits near the bottom of
	the wait panel under the SmeltLoding circle (matches the real reference screenshot layout).
	The real flow has this as a second stage (a first white "버튼을 클릭해 재련 결과를 확인하세요"
	+ its own confirm button, only after which this yellow prompt appears) -- simplified here to
	just this one yellow line per instruction, skipping the first stage/button entirely. */
	const wstring strPrompt =
		L"\xD654\xBA74\xC744 \xD074\xB9AD\xD558\xC5EC \xACB0\xACFC \xC989\xC2DC \xD655\xC778";
	const float2_t vMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strPrompt.c_str());
	const f32_t fScale = (vMeasured.y > 0.f) ? (fHeight * 0.045f / vMeasured.y) : 1.f;

	// bright glowing gold/yellow, same tone as RenderItemUpgradeGaugePercentText's percent number.
	const fvector_t vColor = XMVectorSet(1.0f, 0.851f, 0.353f, 1.f); // #FFD959
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strPrompt.c_str(),
		float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.88f) * textScaleY),
		vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
}

void CMainApp::RenderItemUpgradeSuccessDetailText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::SUCCESS != m_eItemUpgradeAttemptResult ||
		// Not settled yet (burst still playing) -- real in-game capture shows only the circle+burst
		// during this phase, not the icon/name/result text on top of it.
		m_dItemUpgradeResultSettleAt >= 0.0)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto DrawCentered = [&](const char_t* pSlotId, const wchar_t* pLabel,
		f32_t fHeightRatio, const fvector_t& vColor)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
			return;
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * fHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
			float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	// Item name -- same selected-item name/gold-orange tone AND same real size (rect height 19.2,
	// 0.95f ratio) as RenderItemUpgradeLevelText already draws over the base window's own
	// ItemUpgrade_ItemNameLabel, directly under the icon.
	const vector<ITEM_UPGRADE_SLOT_INFO> upgradeSlots = BuildItemUpgradeSlots();
	if (upgradeSlots.empty())
		return;
	const int32_t iSelectedSlot = std::clamp(
		m_iItemUpgradeSelectedSlot, 0, static_cast<int32_t>(upgradeSlots.size()) - 1);
	const fvector_t vGoldOrange = XMVectorSet(1.0f, 0.5686f, 0.0f, 1.f);
	DrawCentered("ItemUpgrade_SuccessItemNameMarker",
		upgradeSlots[iSelectedSlot].strName.c_str(), 0.95f, vGoldOrange);

	// Grade/level reached -- the selected item's level is already incremented by the time this
	// shows (Update_ItemUpgradeResultWaitClick bumps it the instant success is revealed), so this
	// reads the real new level directly instead of a "+1" guess.
	const wstring strReachedLevel =
		to_wstring(ItemUpgradeLevelRef(upgradeSlots[iSelectedSlot].strItemId)) + L"\xB2E8\xACC4";
	DrawCentered("ItemUpgrade_SuccessGradeMarker", strReachedLevel.c_str(), 0.9f, vGoldOrange);

	// "재련 성공" -- light green (연두), by explicit user request.
	const fvector_t vLightGreen = XMVectorSet(0.5647f, 0.9333f, 0.5647f, 1.f); // #90EE90
	DrawCentered("ItemUpgrade_SuccessStatusMarker", L"\xC7AC\xB828 \xC131\xACF5", 0.9f, vLightGreen);

	// "확인" -- same label style as RenderItemUpgradeButtonText's other button labels (0.32 height
	// ratio, white). SuccessOkBtn only shows once settled, same as this whole function.
	DrawCentered("ItemUpgrade_SuccessOkBtn", L"\xD655\xC778", 0.32f, Colors::White);
}

void CMainApp::RenderItemUpgradeFailDetailText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::FAIL != m_eItemUpgradeAttemptResult ||
		// Not settled yet (burst still playing) -- real in-game capture shows only the circle+burst
		// during this phase, not the icon/name/result text on top of it.
		m_dItemUpgradeResultSettleAt >= 0.0)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto DrawCentered = [&](const char_t* pSlotId, const wchar_t* pLabel,
		f32_t fHeightRatio, const fvector_t& vColor)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
			return;
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * fHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
			float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	// Item name -- same gold/orange tone AND same real size (rect height 19.2, 0.95f ratio) as
	// RenderItemUpgradeLevelText already draws over the base window's own ItemUpgrade_ItemNameLabel,
	// directly under the icon (real failItemName_lb placement traced from fail_mc's timeline; no
	// distinct color was recoverable for it, so this reuses success's confirmed tone).
	const vector<ITEM_UPGRADE_SLOT_INFO> upgradeSlots = BuildItemUpgradeSlots();
	if (upgradeSlots.empty())
		return;
	const int32_t iSelectedSlot = std::clamp(
		m_iItemUpgradeSelectedSlot, 0, static_cast<int32_t>(upgradeSlots.size()) - 1);
	const fvector_t vGoldOrange = XMVectorSet(1.0f, 0.5686f, 0.0f, 1.f);
	DrawCentered("ItemUpgrade_FailItemNameMarker",
		upgradeSlots[iSelectedSlot].strName.c_str(), 0.95f, vGoldOrange);

	// "재련 실패" -- red, by explicit user request. A failed reforge does not change level, so unlike
	// the success screen there is no grade/level marker here -- this sits directly under the name.
	const fvector_t vRed = XMVectorSet(0.9098f, 0.1608f, 0.1608f, 1.f); // #E82929
	DrawCentered("ItemUpgrade_FailStatusMarker", L"\xC7AC\xB828 \xC2E4\xD328", 0.9f, vRed);

	// "확인" -- same label style as RenderItemUpgradeButtonText's other button labels.
	DrawCentered("ItemUpgrade_FailOkBtn", L"\xD655\xC778", 0.32f, Colors::White);
}

void CMainApp::Update_ItemQuickSlots()
{
	if (nullptr == m_pInventoryView)
		return;

	constexpr const char* ITEM_SLOT_IDS[4] = { "Item_1", "Item_2", "Item_3", "Item_4" };
	constexpr const char* ITEM_ICON_SLOT_IDS[4] =
		{ "Item_1_Icon", "Item_2_Icon", "Item_3_Icon", "Item_4_Icon" };

	string strDroppedItemId;
	float fDropX = 0.f, fDropY = 0.f;
	if (m_pInventoryView->Try_Consume_ItemDrop(strDroppedItemId, fDropX, fDropY))
	{
		/* The drop position is a real client-area pixel (CUIInputRouter's
		Get_ClientCursorPosition); slot rects are reference-resolution -- same viewport scale
		Get_SlotRect callers always apply, just without ImGui in the middle. */
		const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
		const float scaleX = vViewportSize.x / 1280.f;
		const float scaleY = vViewportSize.y / 720.f;
		for (int32_t i = 0; i < 4; ++i)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!m_pHUDRuntimeView->Get_SlotRect(ITEM_SLOT_IDS[i], fX, fY, fWidth, fHeight))
				continue;
			const float left = fX * scaleX;
			const float top = fY * scaleY;
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
		const ITEM_DEFINITION* pDefinition = m_strItemQuickSlot[i].empty() ?
			nullptr : CItemCatalog::Find_ById(m_strItemQuickSlot[i]);
		if (nullptr == pDefinition || pDefinition->strIconPath.empty())
		{
			m_pHUDRuntimeView->Set_SlotVisible(ITEM_ICON_SLOT_IDS[i], false);
			continue;
		}
		m_pHUDRuntimeView->Set_SlotTexture(ITEM_ICON_SLOT_IDS[i], pDefinition->strIconPath);
		m_pHUDRuntimeView->Set_SlotVisible(ITEM_ICON_SLOT_IDS[i], true);
	}
}

void CMainApp::Update_PlayerHealthManaBar()
{
	/* Only reached from Update_CombatHUD's own show path, which already validated the player
	snapshot and this view. */
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();

	const float healthRatio = (std::clamp)(
		static_cast<float>(player.iCurrentHp) / static_cast<float>(player.iMaximumHp), 0.f, 1.f);
	const float manaRatio = (std::clamp)(
		static_cast<float>(player.iCurrentResource) / static_cast<float>(player.iMaximumResource), 0.f, 1.f);

	m_pHUDRuntimeView->Set_SlotFillRatio("HealthBar_Fill", healthRatio);
	m_pHUDRuntimeView->Set_SlotVisible("HealthBar_Fill", healthRatio > 0.f);
	m_pHUDRuntimeView->Set_SlotFillRatio("ManaBar_Fill", manaRatio);
	m_pHUDRuntimeView->Set_SlotVisible("ManaBar_Fill", manaRatio > 0.f);
}

void CMainApp::Update_LanceMasterIdentityGauge()
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
			m_dLanceGaugeIgniteStartSeconds[i] = Product_Now_Seconds();
			m_bLanceGaugeLoopStarted[i] = false;
		}
		m_bLanceGaugeSegmentWasFull[i] = bIsFull;

		const bool_t bIgniteDone = bIsFull && m_dLanceGaugeIgniteStartSeconds[i] >= 0.0 &&
			(Product_Now_Seconds() - m_dLanceGaugeIgniteStartSeconds[i]) >= BURN_IGNITE_SECONDS[i];
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

void CMainApp::Update_SkillCooldowns()
{
	/* Only reached from Update_CombatHUD's own show path. Matches the fixed server tick rate
	other Client files already redeclare locally (CombatHUDViewModel.cpp, Character.cpp) rather
	than exposing a Shared constant for it. */
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V" };

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const float4_t vCooldownTint =
		float4_t(0.f, 0.f, 0.f, 150.f / 255.f);
	const float4_t vSilenceTint =
		float4_t(0.85f, 0.f, 0.f, 150.f / 255.f);
	const bool_t bSilenced =
		Is_ServerDeadlinePending(player.iServerTick, player.iSilenceEndTick);

	/* Default every overlay to either the exact Server-owned silence mask or off;
	then the cooldown loop below turns on just the ordinary cooldowns when silence
	is not active.  Silence reuses the same White1x1 arc mask with only its R value
	raised.  It never changes an icon or any replicated cooldown value.  Restrict
	the full red mask to slots actually present in the current class/stance.
	Sweeps clockwise from 12 o'clock as the *remaining* cooldown, shrinking back to nothing as
	it expires -- the icon starts fully covered right after use and is revealed clockwise,
	matching the reference cooldown swipe. The pie itself is the appended Skill_<X>_Cooldown
	overlay slot (black 150/255 White1x1) with the g_ArcRatio shader clip; angle-testing the
	square quad reproduces the old "radius past the corners, clipped to the rect" construction
	exactly. The "Ns" numbers moved to RenderSkillCooldownText (post-EndFrame text pass). */
	for (const char* pInputSlot : INPUT_SLOTS)
	{
		const string strOverlaySlot = string("Skill_") + pInputSlot + "_Cooldown";
		const bool_t bActiveSlot = std::any_of(
			player.Skills.begin(), player.Skills.end(),
			[pInputSlot](const HUD_SKILL_STATE& Skill)
			{
				return Skill.strInputSlot == pInputSlot;
			});
		m_pHUDRuntimeView->Set_SlotTint(
			strOverlaySlot, bSilenced ? vSilenceTint : vCooldownTint);
		m_pHUDRuntimeView->Set_SlotArcRatio(
			strOverlaySlot, bSilenced && bActiveSlot ? 1.f : 0.f);
		m_pHUDRuntimeView->Set_SlotVisible(
			strOverlaySlot, bSilenced && bActiveSlot);
	}
	if (bSilenced)
		return;

	for (const HUD_SKILL_STATE& Skill : player.Skills)
	{
		if (Skill.strInputSlot.empty() || Skill.Is_Ready(player.iServerTick))
			continue;

		/* Warlord's Z/X badges have their own real extracted cooldown visual (WarLordSkinFrame's
		SkillSlot "disabled" state swaps the whole icon to a real dark variant, see the Skill_Z/X
		keyframe wiring in Update_CombatHUD), and Artist's Z/X use their own real keyframed wipe
		-- the generic pie was built for Q-F/T/V only. */
		if ("Z" == Skill.strInputSlot || "X" == Skill.strInputSlot)
			continue;

		const uint32_t remainingTicks = Skill.iCooldownEndTick > player.iServerTick ?
			Skill.iCooldownEndTick - player.iServerTick : 0u;
		if (0u == remainingTicks)
			continue;

		const f32_t fRemainingSeconds = static_cast<f32_t>(remainingTicks) / SERVER_TICK_HZ;
		const f32_t fTotalSeconds = Skill.iCooldownDurationTicks > 0u ?
			static_cast<f32_t>(Skill.iCooldownDurationTicks) / SERVER_TICK_HZ : fRemainingSeconds;
		const f32_t fFraction = fTotalSeconds > 0.f ?
			(std::min)(1.f, (std::max)(0.f, fRemainingSeconds / fTotalSeconds)) : 0.f;

		const string strOverlaySlot = "Skill_" + Skill.strInputSlot + "_Cooldown";
		m_pHUDRuntimeView->Set_SlotArcRatio(strOverlaySlot, fFraction);
		m_pHUDRuntimeView->Set_SlotVisible(strOverlaySlot, true);
	}

}

void CMainApp::RenderSkillCooldownText()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) &&
		currentLevel != ETOUI(LEVEL::KAKULSAYDON_ARENA))
	{
		return;
	}
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumHp || 0u == player.iMaximumResource)
		return;
	if ((nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open()) ||
		nullptr == m_pHUDRuntimeView)
	{
		return;
	}

	constexpr f32_t SERVER_TICK_HZ = 30.f;
	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto DrawCooldownLabel = [&](f32_t fCenterX, f32_t fCenterY, f32_t fTargetHeight,
		const wstring& strLabel)
	{
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str());
		const f32_t fScale = (vMeasured.y > 0.f) ? (fTargetHeight / vMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str(),
			float2_t(fCenterX * textScaleX + 1.f, fCenterY * textScaleY + 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 220.f / 255.f), 0.f, float2_t(0.5f, 0.5f),
			fScale * textUiScale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str(),
			float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
			Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	for (const HUD_SKILL_STATE& Skill : player.Skills)
	{
		if (Skill.strInputSlot.empty() || Skill.Is_Ready(player.iServerTick))
			continue;
		const uint32_t remainingTicks = Skill.iCooldownEndTick > player.iServerTick ?
			Skill.iCooldownEndTick - player.iServerTick : 0u;
		if (0u == remainingTicks)
			continue;
		const f32_t fRemainingSeconds = static_cast<f32_t>(remainingTicks) / SERVER_TICK_HZ;
		const int32_t iDisplaySeconds = static_cast<int32_t>(ceilf(fRemainingSeconds));
		const wstring strLabel = std::to_wstring(iDisplaySeconds) + L"s";

		if ("Z" == Skill.strInputSlot || "X" == Skill.strInputSlot)
		{
			/* Artist's Z/X slots are real ARKNewSlot instances with a real cooldownText
			TextField sub-component (yinYangShiSlot symbol 343, depth15, "cooldownText") -- just
			the countdown number, no generic pie (Artist's own keyframed wipe already covers
			that). Warlord's Z/X badge swap has no number in the source, same as before. */
			if (LostArk::Shared::CHARACTER_CLASS_ID::ARTIST != player.eCharacterClass)
				continue;
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!m_pHUDRuntimeView->Get_SlotRect(
				"Yin_Skill_" + Skill.strInputSlot, fX, fY, fWidth, fHeight))
			{
				continue;
			}
			/* Same 22.5-reference-px anchor box the old draw used (the slot's own rect is a
			placeholder; the real art size lives in the keyframe document). */
			DrawCooldownLabel(fX + 22.5f * 0.5f, fY + 22.5f * 0.5f, 22.5f * 0.34f, strLabel);
			continue;
		}

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect("Skill_" + Skill.strInputSlot, fX, fY, fWidth, fHeight))
			continue;
		DrawCooldownLabel(fX + fWidth * 0.5f, fY + fHeight * 0.5f, fHeight * 0.34f, strLabel);
	}
}

void CMainApp::Hide_BossHealthBar()
{
	if (nullptr == m_pBossUIView)
		return;
	constexpr const char_t* BOSS_UI_ALL_SLOTS[] = {
		"Boss_Frame", "Boss_FillBehind", "Boss_Fill", "Boss_StaggerBg", "Boss_StaggerFill",
		"Boss_StaggerTrack", "Boss_Separator", "Boss_TickFlash", "Boss_HitGlow",
		/* Position-only markers for the labels RenderBossHealthBarText draws. They carry real
		boss_text_placeholder.png art that the old manual draw path never submitted, so unlike a
		marker with no layer they have a CUI_Sprite of their own and would otherwise sit on
		screen permanently -- LEVEL::STATIC keeps them across every Level, including Character
		Select and Lobby. Nothing ever shows these. */
		"Boss_TitleText", "Boss_HPText", "Boss_BarCountText",
	};
	for (const char_t* pSlotId : BOSS_UI_ALL_SLOTS)
		m_pBossUIView->Set_SlotVisible(pSlotId, false);
}

void CMainApp::Update_BossHealthBar()
{
	if (nullptr == m_pBossUIView)
	{
		return;
	}

	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel =
		currentLevel == ETOUI(LEVEL::BERN) ||
		currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
		currentLevel == ETOUI(LEVEL::DEVELOPMENT) ||
		currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) ||
		currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA);
	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	/* Same reasoning as Update_ClassList's own gate -- the O-key raid-entry preview's left info
	column/panel frame occupy this same screen region in Debug builds. */
#ifdef _DEBUG
	const bool_t isCharSelectDebugPreviewOpen =
		ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel &&
		nullptr != CLevel_CharacterSelect::Get_Active() &&
		CLevel_CharacterSelect::Get_Active()->Is_DebugRaidEntryPreviewOpen();
#else
	const bool_t isCharSelectDebugPreviewOpen = false;
#endif

	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!isSupportedLevel || skillWindowOpen || isCharSelectDebugPreviewOpen ||
		!boss.isValid || 0u == boss.iMaximumHp)
	{
		Hide_BossHealthBar();
		return;
	}

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
			m_dBossBarTickFlashStartSeconds = Product_Now_Seconds();
		if (boss.iCurrentHp < m_iPreviousBossCurrentHp)
		{
			m_dBossHitGlowStartSeconds = Product_Now_Seconds();
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
	f32_t fFillRectX = 0.f, fFillRectY = 0.f, fFillRectWidth = 0.f, fFillRectHeight = 0.f;
	if (!m_pBossUIView->Get_SlotRect(
		"Boss_Fill", fFillRectX, fFillRectY, fFillRectWidth, fFillRectHeight))
	{
		Hide_BossHealthBar();
		return;
	}
	/* Boss_Fill's own real visibility (healthRatio > 0.f) is set further below, once healthRatio
	is known to be nonzero -- these three have no such data-driven condition, always on
	whenever the bar itself is showing at all. */
	m_pBossUIView->Set_SlotVisible("Boss_Frame", true);
	m_pBossUIView->Set_SlotVisible("Boss_StaggerBg", true);
	m_pBossUIView->Set_SlotVisible("Boss_StaggerTrack", true);

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

	m_pBossUIView->Set_SlotTexture("Boss_Fill", FILL_COLOR_CYCLE[iColorIndex]);
	m_pBossUIView->Set_SlotFillRatio("Boss_Fill", healthRatio > 0.f ? healthRatio : 0.f);
	m_pBossUIView->Set_SlotVisible("Boss_Fill", healthRatio > 0.f);

	/* Reference-resolution equivalent of the old fillInset/fillMin/fillMax screen-pixel math --
	CUILayoutRuntime's Set_SlotPosition/Set_SlotRect already apply m_fScaleX/Y themselves, so
	these stay in the document's own units instead of re-deriving a viewport scale here. */
	const f32_t fFillInset = 2.f;
	const f32_t fFillMinX = fFillRectX + fFillInset;
	const f32_t fFillMinY = fFillRectY + fFillInset;
	const f32_t fFillMaxX = fFillRectX + fFillRectWidth - fFillInset;
	const f32_t fFillMaxY = fFillRectY + fFillRectHeight - fFillInset;
	const f32_t fFillBoundaryX = fFillMinX + (fFillMaxX - fFillMinX) * healthRatio;

	/* The area the current segment's fill hasn't reached yet isn't empty -- it's the next bar
	segment's own color already sitting behind it, so draining the current segment reveals the
	next one's color instead of a gap. Only the current segment (iColorIndex) is fill-ratio
	clipped; this background is the (iBarsRemaining - 1) segment drawn full-width underneath. No
	"next" color once the last bar is draining (iBarsRemaining == 1). */
	if (iBarsRemaining > 1u)
	{
		const uint32_t iNextColorCycleValue = iBarsRemaining - 1u;
		const uint32_t iNextColorIndex = ((iNextColorCycleValue % FILL_COLOR_COUNT == 0) ?
			FILL_COLOR_COUNT : (iNextColorCycleValue % FILL_COLOR_COUNT)) - 1u;
		m_pBossUIView->Set_SlotTexture("Boss_FillBehind", FILL_COLOR_CYCLE[iNextColorIndex]);
		m_pBossUIView->Set_SlotVisible("Boss_FillBehind", true);
	}
	else
	{
		m_pBossUIView->Set_SlotVisible("Boss_FillBehind", false);
	}

	/* Stagger/paralyzation gauge (real paralyzationGauge -- background + fill + hollow
	purple-bordered track, char 473 in TargetGrade_Boss). The Server snapshot owns
	current/maximum accumulated stagger damage. Presentation crops the authored CUI fill
	to the remaining amount so admitted stagger damage visibly depletes toward the break. */
	if (0u != boss.iMaximumStagger)
	{
		const f32_t fStaggerRatio = (std::clamp)(
			static_cast<f32_t>(
				boss.iMaximumStagger - (std::min)(
					boss.iCurrentStagger, boss.iMaximumStagger)) /
				static_cast<f32_t>(boss.iMaximumStagger),
			0.f, 1.f);
		m_pBossUIView->Set_SlotFillRatio("Boss_StaggerFill", fStaggerRatio);
		m_pBossUIView->Set_SlotVisible("Boss_StaggerFill", true);
	}
	else
	{
		m_pBossUIView->Set_SlotVisible("Boss_StaggerFill", false);
	}

	/* User-supplied boundary marker (HP seperate Bar.png -- a tiny 3x15 soft cream vertical glow
	line, not an EFUI_STATUS extraction) drawn at the current fill/empty edge, matching the real
	Progress::mark concept (an edge indicator repositioned every update) this session couldn't
	trace real art for earlier. Hidden exactly at 0%/100%, same as the real useAutoHideMark
	behaviour, since there's no boundary to mark once the bar is fully empty or full. */
	const bool_t bShowSeparator = healthRatio > 0.f && healthRatio < 1.f;
	m_pBossUIView->Set_SlotVisible("Boss_Separator", bShowSeparator);
	if (bShowSeparator)
	{
		constexpr f32_t SEPARATOR_HALF_WIDTH = 3.f;
		m_pBossUIView->Set_SlotRect("Boss_Separator",
			fFillBoundaryX - SEPARATOR_HALF_WIDTH, fFillMinY,
			SEPARATOR_HALF_WIDTH * 2.f, fFillMaxY - fFillMinY);
	}

	/* Real ProgressMultiTrack::updateTarget cross-fades a second "cloneTarget" fill instance --
	same shape as the fill, colorTransform forced to solid white -- over the real fill whenever a
	bar segment ticks over, instead of hard-cutting back to full. Approximated with a flat white
	rect (no separate white-silhouette asset was extracted, same as before this migration) over
	the same now-full fill area, fading out over the tween window real gaugeComplete() uses
	(0.1-0.23s -- rounded up here since a flash reads as more of a blip at the real duration). */
	bool_t bShowTickFlash = false;
	if (m_dBossBarTickFlashStartSeconds >= 0.0)
	{
		constexpr f64_t BAR_TICK_FLASH_SECONDS = 0.3;
		const f64_t fFlashAge = Product_Now_Seconds() - m_dBossBarTickFlashStartSeconds;
		if (fFlashAge < BAR_TICK_FLASH_SECONDS)
		{
			bShowTickFlash = true;
			const f32_t fFlashAlpha = 1.f - static_cast<f32_t>(fFlashAge / BAR_TICK_FLASH_SECONDS);
			m_pBossUIView->Set_SlotRect("Boss_TickFlash",
				fFillMinX, fFillMinY, fFillMaxX - fFillMinX, fFillMaxY - fFillMinY);
			/* This alpha IS the visibility while flashing -- Set_SlotVisible(true) would
			clobber it back to fully opaque (it resets the tint to solid white). */
			m_pBossUIView->Set_SlotAlpha("Boss_TickFlash", (220.f / 255.f) * fFlashAlpha);
		}
	}
	if (!bShowTickFlash)
		m_pBossUIView->Set_SlotVisible("Boss_TickFlash", false);

	/* Real Progress::updateMark positions a "mark" clip at the fill's own edge every update; its
	symbol (character 732) is a 39-frame animated additive glow (grows ~1.0->1.5/3.0 scale, fades
	~256->92 alpha) rather than a static line. No source art was traced for it, so this reuses
	click_move_glow.dds (the ground click-move indicator's own soft radial glow) resized/faded to
	approximate the same motion -- a real dedicated hit-glow asset would still be a closer match. */
	bool_t bShowHitGlow = false;
	if (m_dBossHitGlowStartSeconds >= 0.0)
	{
		constexpr f64_t HIT_GLOW_SECONDS = 0.35;
		const f64_t fGlowAge = Product_Now_Seconds() - m_dBossHitGlowStartSeconds;
		if (fGlowAge < HIT_GLOW_SECONDS)
		{
			bShowHitGlow = true;
			const f32_t fGlowT = static_cast<f32_t>(fGlowAge / HIT_GLOW_SECONDS);
			const f32_t fGlowAlpha = 1.f - fGlowT;
			const f32_t fGlowRadius = 6.f + 10.f * fGlowT;
			const f32_t fGlowCenterX = fFillMinX + (fFillMaxX - fFillMinX) * m_fBossHitGlowFillRatio;
			const f32_t fGlowCenterY = (fFillMinY + fFillMaxY) * 0.5f;
			m_pBossUIView->Set_SlotRect("Boss_HitGlow",
				fGlowCenterX - fGlowRadius, fGlowCenterY - fGlowRadius,
				fGlowRadius * 2.f, fGlowRadius * 2.f);
			/* Same warm near-white (255,250,230) the original IM_COL32 glow used, not
			Set_SlotAlpha's plain white -- that would wash out the warm tint. This tint IS
			the visibility while glowing, same as Boss_TickFlash's alpha above. */
			m_pBossUIView->Set_SlotTint("Boss_HitGlow",
				float4_t(1.f, 250.f / 255.f, 230.f / 255.f, (200.f / 255.f) * fGlowAlpha));
		}
	}
	if (!bShowHitGlow)
		m_pBossUIView->Set_SlotVisible("Boss_HitGlow", false);
}

/* Split from Update_BossHealthBar() -- see the declaration comment in MainApp.h for why this
uses ImGui's own font/foreground draw list (CGameInstance::Draw_Text, called after
CImGuiLayer::EndFrame()) instead of anything driven from that CUI_Sprite state. Re-derives
healthRatio/iBarsRemaining from the same boss snapshot Update_BossHealthBar already used this
frame; cheap pure arithmetic, safer than threading the values out as member state. */
void CMainApp::RenderBossHealthBarText()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) &&
		currentLevel != ETOUI(LEVEL::KAKULSAYDON_ARENA))
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

void CMainApp::RenderDeadSceneText()
{
	if (ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return;

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid ||
		LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction)
	{
		return;
	}

	const HUD_DEADSCENE_TEXT_RECTS& rects = CCombatHUDViewModel::Get().Get_DeadSceneTextRects();
	if (!rects.isValid)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	/* Positions/sizes come from CLevel_ValtanArena::Update_DeadScene(), which reads the live
	DeadScene_TitleTextMarker/_ReviveButton/_SpectateButton/_ReviveMessageMarker slot rects out of
	its own m_pDeadSceneView every frame -- moving any of those in the HUD Layout Tool moves this
	text with them, instead of a hand-copied constant here drifting out of sync the way it just did. */
	const wstring strTitle = L"\xC0AC\xB9DD\xD558\xC600\xC2B5\xB2C8\xB2E4"; // 사망하였습니다
	const float2_t vTitleMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strTitle.c_str());
	const f32_t fTitleScale = (vTitleMeasured.y > 0.f) ?
		(rects.fTitleHeight * 0.6f / vTitleMeasured.y) : 1.f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strTitle.c_str(),
		float2_t((rects.fTitleX + rects.fTitleWidth * 0.5f) * textScaleX,
			(rects.fTitleY + rects.fTitleHeight * 0.5f) * textScaleY),
		Colors::White, 0.f, float2_t(0.5f, 0.5f), fTitleScale * textUiScale);

	const wstring strReviveLabel = L"\xBD80\xD65C"; // 부활
	const float2_t vReviveMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strReviveLabel.c_str());
	const f32_t fReviveScale = (vReviveMeasured.y > 0.f) ?
		(rects.fReviveTextHeight * 0.55f / vReviveMeasured.y) : 1.f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strReviveLabel.c_str(),
		float2_t((rects.fReviveTextX + rects.fReviveTextWidth * 0.5f) * textScaleX,
			(rects.fReviveTextY + rects.fReviveTextHeight * 0.5f) * textScaleY),
		Colors::White, 0.f, float2_t(0.5f, 0.5f), fReviveScale * textUiScale);

	const wstring strSpectateLabel = L"\xAD00\xC804\xD558\xAE30"; // 관전하기
	const float2_t vSpectateMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strSpectateLabel.c_str());
	const f32_t fSpectateScale = (vSpectateMeasured.y > 0.f) ?
		(rects.fSpectateHeight * 0.5f / vSpectateMeasured.y) : 1.f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strSpectateLabel.c_str(),
		float2_t((rects.fSpectateX + rects.fSpectateWidth * 0.5f) * textScaleX,
			(rects.fSpectateY + rects.fSpectateHeight * 0.5f) * textScaleY),
		Colors::White, 0.f, float2_t(0.5f, 0.5f), fSpectateScale * textUiScale);

	const wstring strReviveMessage = L"\xBD80\xD65C\xD558\xC2DC\xACA0\xC2B5\xB2C8\xAE4C?"; // 부활하시겠습니까?
	const float2_t vMessageMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strReviveMessage.c_str());
	const f32_t fMessageScale = (vMessageMeasured.y > 0.f) ?
		(rects.fMessageHeight * 0.6f / vMessageMeasured.y) : 1.f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strReviveMessage.c_str(),
		float2_t((rects.fMessageX + rects.fMessageWidth * 0.5f) * textScaleX,
			(rects.fMessageY + rects.fMessageHeight * 0.5f) * textScaleY),
		Colors::White, 0.f, float2_t(0.5f, 0.5f), fMessageScale * textUiScale);
}

void CMainApp::RenderRaidClearText()
{
	if (ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return;

	const HUD_RAIDCLEAR_TEXT_RECTS& rects = CCombatHUDViewModel::Get().Get_RaidClearTextRects();
	if (!rects.isValid && !rects.isButtonValid)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	if (rects.isValid)
	{
		/* Real loc key traced from epicgatecommonclear.gfx's clearTF field (fontClass=$YoonGasiIIM,
		white, initialText="[$]commander.dungeon_clear") -- this project has no loc-key table, so
		the real Korean string it resolves to in the reference screenshot is used directly. */
		const wstring strTitle = L"\xB358\xC804 \xD074\xB9AC\xC5B4"; // 던전 클리어
		const float2_t vTitleMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strTitle.c_str());
		const f32_t fTitleScale = (vTitleMeasured.y > 0.f) ?
			(rects.fTitleHeight * 0.6f / vTitleMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strTitle.c_str(),
			float2_t((rects.fTitleX + rects.fTitleWidth * 0.5f) * textScaleX,
				(rects.fTitleY + rects.fTitleHeight * 0.5f) * textScaleY),
			Colors::White, 0.f, float2_t(0.5f, 0.5f), fTitleScale * textUiScale);
	}

	// "돌아가기" -- RaidClear_ReturnButton's own label, same style/height ratio as
	// RenderItemUpgradeButtonText's other NormalButton labels (0.6 of the button
	// rect's own height, white, centered). Only appears once the celebration
	// overlay itself has finished (isButtonValid), never together with isValid.
	if (rects.isButtonValid)
	{
		const wstring strReturnLabel = L"\xB3CC\xC544\xAC00\xAE30"; // 돌아가기
		const float2_t vReturnMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strReturnLabel.c_str());
		const f32_t fScaleByHeight = (vReturnMeasured.y > 0.f) ?
			(rects.fButtonHeight * 0.6f / vReturnMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vReturnMeasured.x > 0.f) ?
			(rects.fButtonWidth * 0.85f / vReturnMeasured.x) : 1.f;
		const f32_t fReturnScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strReturnLabel.c_str(),
			float2_t((rects.fButtonX + rects.fButtonWidth * 0.5f) * textScaleX,
				(rects.fButtonY + rects.fButtonHeight * 0.5f) * textScaleY),
			Colors::White, 0.f, float2_t(0.5f, 0.5f), fReturnScale * textUiScale);
	}
}

void CMainApp::RenderItemAnnounceText()
{
	if (ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return;

	const HUD_ITEMANNOUNCE_TEXT_RECTS& rects =
		CCombatHUDViewModel::Get().Get_ItemAnnounceTextRects();
	if (!rects.isValid || rects.strItemName.empty())
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	// Scale is fit against the combined string's own measured extent, same as
	// before the name/suffix split, so this doesn't change size/wrapping behavior.
	const wstring strCombined = rects.strItemName + rects.strSuffix;
	const float2_t vMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strCombined.c_str());
	const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (rects.fTextHeight * 0.7f / vMeasured.y) : 1.f;
	const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (rects.fTextWidth * 0.95f / vMeasured.x) : 1.f;
	const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth) * 0.8f * textUiScale;

	const float2_t vNameMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), rects.strItemName.c_str());
	const f32_t fCenterY = (rects.fTextY + rects.fTextHeight * 0.5f) * textScaleY;
	const f32_t fNameX = rects.fTextX * textScaleX;
	// Item grade gold/orange -- same #FF9100-ish tone RenderItemUpgradeLevelText
	// already uses for an equipment item's own name label.
	const fvector_t vGoldOrange = XMVectorSet(1.0f, 0.5686f, 0.0f, 1.f);
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), rects.strItemName.c_str(),
		float2_t(fNameX, fCenterY), vGoldOrange, 0.f, float2_t(0.f, 0.5f), fScale);
	if (!rects.strSuffix.empty())
	{
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), rects.strSuffix.c_str(),
			float2_t(fNameX + vNameMeasured.x * fScale, fCenterY),
			Colors::White, 0.f, float2_t(0.f, 0.5f), fScale);
	}
}

void CMainApp::Update_ChargeGauge()
{
	if (nullptr == m_pHUDRuntimeView)
		return;

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();

	/* Real HOLD skill. Default (34590 적룡포 and every other HOLD skill): the gauge fills ONCE,
	continuously, from action start and reaches 100% exactly at the real hit -- the first
	comboStages[] entry with a nonzero hitTimeMs (confirmed real data: 34590's stage index 2 has
	hitTimeMs=322). "Elapsed since action start" is NOT the sum of authored iActionDurationMs for
	completed stages -- a HOLD skill's loop stage can be cut short (PlayerSkillSystem.cpp's
	holdLeavesLoop/holdSkipsLoop) when the player releases early, and hasReleasedHold never reaches
	the wire, so this tracks the REAL elapsed of each completed stage from observed
	iComboStage/iActionStartTick edges instead (m_fChargeGaugeElapsedBeforeCurrentStageMs). The
	instant an observed stage turns out shorter than its authored duration (or a whole stage was
	skipped), that's proof of an early release -- the gauge cancels (hides) for the rest of this
	skill-use instance instead of jumping ahead toward a now-moot target.

	Special case (17240 풀배럴 캐넌, Warlord): confirmed against Warlord.skillbindings.json's real
	clip chain -- stage1 (eternalcyclone_01) is the raise+shout windup (no gauge), stage2
	(eternalcyclone_02/03/04, 300ms each, summing to stage2's own 900ms) is the 3 real arm-lowering
	motions in sync with the 3 "thud" sounds (one gauge pump per 300ms sub-clip), and stage3
	(eternalcyclone_07, the real hit) is the firing animation alone -- gauge hidden, matching the
	real screen (3rd pump completes, gauge disappears, then it fires). No general data field
	distinguishes this "single fill" vs "per-stage-of-substage pump" HOLD shape yet -- this is a
	skillId-keyed exception until a second real multi-pump skill shows what that field should
	look like. */
	constexpr LostArk::Shared::SKILL_ID FULL_BARREL_CANNON_SKILL_ID = 17240;
	constexpr std::uint8_t FULL_BARREL_CANNON_PUMP_STAGE = 2u;
	constexpr f32_t FULL_BARREL_CANNON_PUMP_MS = 300.f;
	constexpr std::uint32_t FULL_BARREL_CANNON_PUMP_COUNT = 3u;
	constexpr f32_t SERVER_TICK_HZ = 30.f;

	bool_t bCharging = false;
	f32_t fChargeProgress = 0.f;
	const bool_t bIsHoldAction = player.isValid &&
		LostArk::Shared::PLAYER_ACTION_STATE::SKILL == player.eAction &&
		0u != player.iComboStage;
	const PLAYER_SKILL_DEFINITION* pSkill = bIsHoldAction ?
		CPlayerSkillCatalog::Find_ById(player.iCurrentSkillId) : nullptr;
	const bool_t bValidHoldStage = nullptr != pSkill &&
		LostArk::Shared::PLAYER_SKILL_KIND::HOLD == pSkill->eSkillKind &&
		player.iComboStage <= pSkill->ComboStages.size();

	if (!bValidHoldStage)
	{
		// Not (or no longer) charging a HOLD skill -- next real charge starts fully fresh.
		m_iChargeGaugeTrackedSkillId = LostArk::Shared::INVALID_SKILL_ID;
		m_bChargeGaugeCancelled = false;
	}
	else if (FULL_BARREL_CANNON_SKILL_ID == pSkill->iSkillId)
	{
		if (FULL_BARREL_CANNON_PUMP_STAGE == player.iComboStage)
		{
			f32_t fStageAgeSeconds = 0.f;
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
				player.iServerTick, player.iActionStartTick, SERVER_TICK_HZ, fStageAgeSeconds);
			const f32_t fStageAgeMs = fStageAgeSeconds * 1000.f;
			const f32_t fPumpTotalMs = FULL_BARREL_CANNON_PUMP_MS * FULL_BARREL_CANNON_PUMP_COUNT;
			const f32_t fClampedAgeMs = std::clamp(fStageAgeMs, 0.f, fPumpTotalMs);
			const f32_t fWithinPumpMs = std::fmod(fClampedAgeMs, FULL_BARREL_CANNON_PUMP_MS);
			fChargeProgress = std::clamp(fWithinPumpMs / FULL_BARREL_CANNON_PUMP_MS, 0.f, 1.f);
			bCharging = true;
		}
		// Stage 1 (windup) and stage 3 (firing) show no gauge at all for this skill.
	}
	else
	{
		const bool_t bFreshCharge =
			LostArk::Shared::INVALID_SKILL_ID == m_iChargeGaugeTrackedSkillId ||
			m_iChargeGaugeTrackedSkillId != pSkill->iSkillId ||
			player.iComboStage < m_iChargeGaugeTrackedComboStage;
		if (bFreshCharge)
		{
			m_iChargeGaugeTrackedSkillId = pSkill->iSkillId;
			m_iChargeGaugeTrackedComboStage = player.iComboStage;
			m_iChargeGaugeStageStartTick = player.iActionStartTick;
			m_fChargeGaugeElapsedBeforeCurrentStageMs = 0.f;
			m_bChargeGaugeCancelled = false;
		}
		else if (player.iComboStage != m_iChargeGaugeTrackedComboStage)
		{
			// Real elapsed of the stage that just ended = the new stage's own start tick minus
			// the old stage's start tick -- exact regardless of whether it ran its full authored
			// duration or was cut short.
			f32_t fEndedStageSeconds = 0.f;
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
				player.iActionStartTick, m_iChargeGaugeStageStartTick, SERVER_TICK_HZ,
				fEndedStageSeconds);
			const f32_t fEndedStageRealMs = fEndedStageSeconds * 1000.f;
			const std::size_t iEndedStageIndex =
				static_cast<std::size_t>(m_iChargeGaugeTrackedComboStage) - 1u;
			const f32_t fEndedStageAuthoredMs = static_cast<f32_t>(
				pSkill->ComboStages[iEndedStageIndex].iActionDurationMs);
			constexpr f32_t TRUNCATION_TOLERANCE_MS = 50.f;
			const bool_t bStageWasSkipped =
				player.iComboStage > m_iChargeGaugeTrackedComboStage + 1u;
			const bool_t bStageWasCutShort =
				fEndedStageRealMs < fEndedStageAuthoredMs - TRUNCATION_TOLERANCE_MS;
			if (bStageWasSkipped || bStageWasCutShort)
				m_bChargeGaugeCancelled = true;

			m_fChargeGaugeElapsedBeforeCurrentStageMs += fEndedStageRealMs;
			m_iChargeGaugeTrackedComboStage = player.iComboStage;
			m_iChargeGaugeStageStartTick = player.iActionStartTick;
		}

		if (!m_bChargeGaugeCancelled)
		{
			std::size_t iHitStageIndex = pSkill->ComboStages.size() - 1;
			for (std::size_t i = 0; i < pSkill->ComboStages.size(); ++i)
			{
				if (0u != pSkill->ComboStages[i].iHitTimeMs)
				{
					iHitStageIndex = i;
					break;
				}
			}
			f32_t fTargetMs = 0.f;
			for (std::size_t i = 0; i < iHitStageIndex; ++i)
				fTargetMs += static_cast<f32_t>(pSkill->ComboStages[i].iActionDurationMs);
			fTargetMs += (0u != pSkill->ComboStages[iHitStageIndex].iHitTimeMs) ?
				static_cast<f32_t>(pSkill->ComboStages[iHitStageIndex].iHitTimeMs) :
				static_cast<f32_t>(pSkill->ComboStages[iHitStageIndex].iActionDurationMs);

			f32_t fStageAgeSeconds = 0.f;
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
				player.iServerTick, player.iActionStartTick, SERVER_TICK_HZ, fStageAgeSeconds);
			const f32_t fStageAgeMs = fStageAgeSeconds * 1000.f;

			if (fTargetMs > 0.f)
			{
				fChargeProgress = std::clamp(
					(m_fChargeGaugeElapsedBeforeCurrentStageMs + fStageAgeMs) / fTargetMs, 0.f, 1.f);
				bCharging = true;
			}
		}
	}

	/* Bg/Track are static full images (their own authored slots); Fill is the same authored
	slot, clipped by the real charge progress via the g_FillRatio shader clip -- same technique
	the boss/player bars use. */
	m_pHUDRuntimeView->Set_SlotVisible("ChargeGauge_Bg", bCharging);
	m_pHUDRuntimeView->Set_SlotVisible("ChargeGauge_Track", bCharging);
	const bool_t bShowFill = bCharging && fChargeProgress > 0.f;
	m_pHUDRuntimeView->Set_SlotVisible("ChargeGauge_Fill", bShowFill);
	if (bShowFill)
		m_pHUDRuntimeView->Set_SlotFillRatio("ChargeGauge_Fill", fChargeProgress);
}

void CMainApp::RenderChargeGaugeText()
{
	if (nullptr == m_pHUDRuntimeView)
		return;

	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid ||
		LostArk::Shared::PLAYER_ACTION_STATE::SKILL != player.eAction ||
		0u == player.iComboStage)
	{
		return;
	}

	const PLAYER_SKILL_DEFINITION* pSkill =
		CPlayerSkillCatalog::Find_ById(player.iCurrentSkillId);
	if (nullptr == pSkill ||
		LostArk::Shared::PLAYER_SKILL_KIND::HOLD != pSkill->eSkillKind ||
		player.iComboStage > pSkill->ComboStages.size())
	{
		return;
	}
	// Must mirror Update_ChargeGauge's own bCharging gate exactly -- 17240 풀배럴 캐넌 only shows a
	// gauge (so only shows this label) during its stage-2 pump; every other HOLD skill hides both
	// once an early release cancels the charge (m_bChargeGaugeCancelled, set earlier this same
	// frame by Update_ChargeGauge).
	constexpr LostArk::Shared::SKILL_ID FULL_BARREL_CANNON_SKILL_ID = 17240;
	if (FULL_BARREL_CANNON_SKILL_ID == pSkill->iSkillId)
	{
		if (2u != player.iComboStage)
			return;
	}
	else if (m_bChargeGaugeCancelled)
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pHUDRuntimeView->Get_SlotRect("ChargeGauge_Track", fX, fY, fWidth, fHeight))
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	// strDisplayName is UTF-8 (Data/Balance/PlayerSkills.json); a byte-wise widen would garble
	// every Korean skill name, so this needs a real MultiByteToWideChar conversion, same as
	// CWorldPlayerNameplateView::Try_ConvertUtf8 already does for the equivalent nickname case.
	wstring strLabel;
	const int iRequiredLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		pSkill->strDisplayName.data(), static_cast<int>(pSkill->strDisplayName.size()),
		nullptr, 0);
	if (iRequiredLength <= 0)
		return;
	strLabel.resize(static_cast<size_t>(iRequiredLength));
	if (iRequiredLength != MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		pSkill->strDisplayName.data(), static_cast<int>(pSkill->strDisplayName.size()),
		strLabel.data(), iRequiredLength))
	{
		return;
	}

	const float2_t vMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str());
	const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * 0.7f / vMeasured.y) : 1.f;
	const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.9f / vMeasured.x) : 1.f;
	const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str(),
		float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
		Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
}

void CMainApp::Hide_EstherUI()
{
	if (nullptr == m_pEstherUIView)
		return;
	constexpr const char_t* ESTHER_ALL_SLOTS[] = {
		"Esther_HeaderFrame",
		"Esther_Slot1_Frame", "Esther_Slot1_Icon", "Esther_Slot1_Ready",
		"Esther_Slot2_Frame", "Esther_Slot2_Icon", "Esther_Slot2_Ready",
		"Esther_Slot3_Frame", "Esther_Slot3_Icon", "Esther_Slot3_Ready",
		"Esther_GaugeTrack", "Esther_GaugeFill",
	};
	for (const char_t* pSlotId : ESTHER_ALL_SLOTS)
		m_pEstherUIView->Set_SlotVisible(pSlotId, false);
}

void CMainApp::Update_EstherGauge()
{
	if (nullptr == m_pEstherUIView)
		return;

	/* Esther's skill-select window is a Valtan raid mechanic -- Character Select's live Server
	room can still populate a nonzero gauge maximum for the selected class, which drew this
	window there too even though there is no raid encounter to use it against. */
	const uint32_t maximum =
		CCombatHUDViewModel::Get().Get_EstherGaugeMaximum();
	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isRaidArena =
		ETOUI(LEVEL::VALTAN_ARENA) == currentLevel ||
		ETOUI(LEVEL::KAKULSAYDON_ARENA) == currentLevel;
	if (!isRaidArena || 0u == maximum || skillWindowOpen ||
		!CCombatHUDViewModel::Get().Get_Player().isValid)
	{
		Hide_EstherUI();
		return;
	}

	/* Static pieces (frame/portraits/track) show whenever the window itself shows; real pieces
	(frame/lock/gauge) traced from the actual estherweaponskill.gfx + EFUI_ICONATLAS_E packages,
	not placeholder rects. Esther_GaugeFill is a separate Tool-placeable slot (own rect,
	independently adjustable) fill-ratio-clipped by the real gauge value -- same technique as the
	boss/player HP bars. The 3 Ready glows only show at full gauge. */
	constexpr const char_t* ESTHER_STATIC_SLOTS[] = {
		"Esther_HeaderFrame",
		"Esther_Slot1_Frame", "Esther_Slot1_Icon",
		"Esther_Slot2_Frame", "Esther_Slot2_Icon",
		"Esther_Slot3_Frame", "Esther_Slot3_Icon",
		"Esther_GaugeTrack",
	};
	for (const char_t* pSlotId : ESTHER_STATIC_SLOTS)
		m_pEstherUIView->Set_SlotVisible(pSlotId, true);

	/* Each raid grants its own three esther skills, in its own order -- the same three the raid
	entry screen lists for that raid, so the window a player sees inside the raid matches what
	they were shown on the way in. The level is the raid: this only runs in an arena level, and
	the document's authored icons are Valtan's, so only Kakul needs to be swapped in. */
	if (ETOUI(LEVEL::KAKULSAYDON_ARENA) == currentLevel)
	{
		m_pEstherUIView->Set_SlotTexture("Esther_Slot1_Icon", "UI/Esther/esther_icon_3.png");
		m_pEstherUIView->Set_SlotTexture("Esther_Slot2_Icon", "UI/Esther/esther_portrait_wei.png");
		m_pEstherUIView->Set_SlotTexture("Esther_Slot3_Icon", "UI/Esther/esther_icon_4.png");
	}

	const uint32_t gauge = CCombatHUDViewModel::Get().Get_EstherGauge();
	const float fillRatio = (std::clamp)(
		static_cast<float>(gauge) / static_cast<float>(maximum), 0.f, 1.f);
	m_pEstherUIView->Set_SlotFillRatio("Esther_GaugeFill", fillRatio);
	m_pEstherUIView->Set_SlotVisible("Esther_GaugeFill", fillRatio > 0.f);

	const bool_t bReady = gauge >= maximum;
	m_pEstherUIView->Set_SlotVisible("Esther_Slot1_Ready", bReady);
	m_pEstherUIView->Set_SlotVisible("Esther_Slot2_Ready", bReady);
	m_pEstherUIView->Set_SlotVisible("Esther_Slot3_Ready", bReady);
}

void CMainApp::RenderEstherGaugeText()
{
	/* Same gates as Update_EstherGauge -- this label is that window's own caption, so it must
	disappear and reappear together with the art instead of floating without it. */
	if (nullptr == m_pEstherUIView ||
		ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
	{
		return;
	}
	const uint32_t maximum =
		CCombatHUDViewModel::Get().Get_EstherGaugeMaximum();
	if (0u == maximum ||
		(nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open()) ||
		!CCombatHUDViewModel::Get().Get_Player().isValid)
	{
		return;
	}

	f32_t fTrackX = 0.f, fTrackY = 0.f, fTrackWidth = 0.f, fTrackHeight = 0.f;
	if (!m_pEstherUIView->Get_SlotRect(
		"Esther_GaugeTrack", fTrackX, fTrackY, fTrackWidth, fTrackHeight))
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const uint32_t gauge = CCombatHUDViewModel::Get().Get_EstherGauge();
	const wchar_t* pLabel = gauge >= maximum ?
		L"ESTHER READY  Ctrl+Z/X/C" : L"ESTHER";
	const float2_t vMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
	/* ~12 reference px tall, centered above the track's own top edge -- matching the old
	ImGui-font label's placement (its default font was ~13 screen px). */
	constexpr f32_t LABEL_HEIGHT = 12.f;
	const f32_t fScale = (vMeasured.y > 0.f) ? (LABEL_HEIGHT / vMeasured.y) : 1.f;
	const f32_t fCenterX = fTrackX + fTrackWidth * 0.5f;
	const f32_t fCenterY = fTrackY - 2.f - LABEL_HEIGHT * 0.5f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
		float2_t(fCenterX * textScaleX + 1.f, fCenterY * textScaleY + 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 220.f / 255.f), 0.f, float2_t(0.5f, 0.5f),
		fScale * textUiScale);
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
		float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
		XMVectorSet(214.f / 255.f, 238.f / 255.f, 1.f, 1.f), 0.f, float2_t(0.5f, 0.5f),
		fScale * textUiScale);
}

void CMainApp::Update_SkillIcons()
{
	/* Which skill icon belongs in Skill_Q.."Skill_F" is content, not layout: it depends on the
	live (class, stance) pair via CPlayerSkillCatalog::Find_BySlot, the same source of truth the
	input controller already resolves quick slots from. HUD_Layout.json only owns the shared
	frame's position/size (ownerClass null "Skill_Q".."Skill_F"); it must not carry a second,
	class-hardcoded copy of "which icon" that can drift out of sync with PlayerSkills.json.
	Find_BySlot already resolves stance-gated skills correctly and ignores the stance argument
	for classes whose skills have no requiredStance. Only reached from Update_CombatHUD's own
	show path. */
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();

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

	for (const char* pInputSlot : INPUT_SLOTS)
	{
		/* Icon sits between the slot background and the frame in real sprite order: Skill_<X>
		keeps only "Slot Bg.png" as its authored layer now, the dynamic Skill_<X>_Icon slot
		carries whatever this resolves, and the appended Skill_<X>_Frame slot (the frame art
		stripped from the base slot) draws over both -- the same border-above-icon stacking the
		old redraw achieved. */
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

		const string strIconSlot = string("Skill_") + pInputSlot + "_Icon";
		if (nullptr != pIconPath)
		{
			m_pHUDRuntimeView->Set_SlotTexture(strIconSlot, pIconPath);
			m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, true);
		}
		else
		{
			m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, false);
		}
	}
}

void CMainApp::Update_QuickSlotFlash()
{
	/* Icon art, slot frame, keybind label, and cooldown sweep aren't extracted from QuickSlot.gfx
	yet, so this only plays the real on-use flash -- Update_SkillIcons/Update_SkillCooldowns
	(called alongside this, not instead of it) still own everything else. Only reached from
	Update_CombatHUD's own show path. */
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();

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
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) &&
		currentLevel != ETOUI(LEVEL::KAKULSAYDON_ARENA))
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
	/* Boss HP number/name/grade text moved into RenderBossHealthBarText() -- the decompiled
	targetstatus_loc_int.gfx places them relative to the bar's own real position (see that
	function), not this hardcoded (640, 58). */
}

void CMainApp::RenderDamageNumbers()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) &&
		currentLevel != ETOUI(LEVEL::KAKULSAYDON_ARENA))
	{
		return;
	}
	if (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open())
		return;

	/* Retail damagetext.gfx (EFUI_DAMAGE, class DamageTextCBT2): $YoonGasiIIM 32px centered text
	on the 1080p stage, driven by two linear tweens. The native side passes the tween values and
	no shipped data table carries them, so the factory defaults in the decompiled AS3 are the
	evidence used here: phase 0 (450 ms) holds 32px and drifts 15px up; phase 1 (70 ms) shrinks
	to 19px, drifts on to 100px up and fades to 0. Stage pixels scale with the viewport height. */
	constexpr f64_t DAMAGE_PHASE0_SECONDS = 0.45;
	constexpr f64_t DAMAGE_PHASE1_SECONDS = 0.07;
	constexpr f64_t DAMAGE_NUMBER_LIFETIME_SECONDS = DAMAGE_PHASE0_SECONDS + DAMAGE_PHASE1_SECONDS;
	constexpr f32_t DAMAGE_FONT_PX_START = 32.f;
	constexpr f32_t DAMAGE_FONT_PX_END = 19.f;
	constexpr f32_t DAMAGE_RISE_PX_PHASE0 = 15.f;
	constexpr f32_t DAMAGE_RISE_PX_END = 100.f;
	constexpr size_t MAX_FLOATING_DAMAGE_NUMBERS = 48u;

	/* Get_DamageEvents() keeps every retained hit, not just this frame's -- only spawn a floating
	number for events strictly newer than the last batch we already spawned from. See the member
	comment on m_iLastRenderedDamageServerTick for why a serverTick cursor is safe here even though
	the buffer trims from the front. */
	const std::vector<HUD_DAMAGE_EVENT>& damageEvents =
		CCombatHUDViewModel::Get().Get_DamageEvents();
	/* Compare against the cursor as it was before this pass: one snapshot batches every hit of
	that tick under the same serverTick, so advancing the cursor inside the loop would drop all
	but the first number of a multi-target hit. */
	const uint32_t iSpawnedUpToTick = m_iLastRenderedDamageServerTick;
	for (const HUD_DAMAGE_EVENT& damageEvent : damageEvents)
	{
		if (damageEvent.iServerTick <= iSpawnedUpToTick)
			continue;
		FLOATING_DAMAGE_NUMBER number{};
		number.dSpawnSeconds = Product_Now_Seconds();
		number.vWorldPosition = float3_t(
			damageEvent.Event.fPositionX,
			damageEvent.Event.fPositionY,
			damageEvent.Event.fPositionZ);
		number.iAmount = damageEvent.Event.iAmount;
		number.isOutgoing = damageEvent.Event.isOutgoing;
		m_FloatingDamageNumbers.push_back(number);
		m_iLastRenderedDamageServerTick =
			(std::max)(m_iLastRenderedDamageServerTick, damageEvent.iServerTick);
	}
	if (m_FloatingDamageNumbers.size() > MAX_FLOATING_DAMAGE_NUMBERS)
	{
		m_FloatingDamageNumbers.erase(
			m_FloatingDamageNumbers.begin(),
			m_FloatingDamageNumbers.begin() +
				(m_FloatingDamageNumbers.size() - MAX_FLOATING_DAMAGE_NUMBERS));
	}

	const f64_t dNow = Product_Now_Seconds();
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
	const f32_t stageScale = viewportSize.y / 1080.f;

	for (const FLOATING_DAMAGE_NUMBER& number : m_FloatingDamageNumbers)
	{
		const f64_t dAge = dNow - number.dSpawnSeconds;
		f32_t fFontPx = DAMAGE_FONT_PX_START;
		f32_t fRisePx = 0.f;
		f32_t fAlpha = 1.f;
		if (dAge < DAMAGE_PHASE0_SECONDS)
		{
			fRisePx = DAMAGE_RISE_PX_PHASE0 * static_cast<f32_t>(dAge / DAMAGE_PHASE0_SECONDS);
		}
		else
		{
			const f32_t t = (std::clamp)(
				static_cast<f32_t>((dAge - DAMAGE_PHASE0_SECONDS) / DAMAGE_PHASE1_SECONDS), 0.f, 1.f);
			fFontPx = DAMAGE_FONT_PX_START + (DAMAGE_FONT_PX_END - DAMAGE_FONT_PX_START) * t;
			fRisePx = DAMAGE_RISE_PX_PHASE0 + (DAMAGE_RISE_PX_END - DAMAGE_RISE_PX_PHASE0) * t;
			fAlpha = 1.f - t;
		}
		/* Anchored a little above the hit point (the event carries the target's ground position);
		the tween moves it in screen space from there, like retail's canvas does. */
		const vector_t vProjected = XMVector3Project(
			XMVectorSet(
				number.vWorldPosition.x,
				number.vWorldPosition.y + 1.f,
				number.vWorldPosition.z,
				1.f),
			0.f, 0.f, viewportSize.x, viewportSize.y, 0.f, 1.f,
			projection, view, XMMatrixIdentity());
		if (XMVectorGetZ(vProjected) < 0.f || XMVectorGetZ(vProjected) > 1.f)
			continue;
		const wstring strAmount = Format_ThousandsSeparated(number.iAmount);
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_EventDamage"), strAmount.c_str());
		const f32_t fScale = vMeasured.y > 0.f ? (fFontPx * stageScale) / vMeasured.y : 1.f;
		/* Retail DamageTextWnd colours: outgoing hits use the critical yellow (0xFFCC00) for every
		hit by project decision, incoming hits the enemy red (0xFF0000). */
		const fvector_t vColor = number.isOutgoing ?
			XMVectorSet(1.f, 0.8f, 0.f, fAlpha) :
			XMVectorSet(1.f, 0.f, 0.f, fAlpha);
		CGameInstance::Get().Draw_Text(TEXT("Font_EventDamage"), strAmount.c_str(),
			float2_t(XMVectorGetX(vProjected), XMVectorGetY(vProjected) - fRisePx * stageScale),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale);
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

	/* Small-label variants of two of the fonts above, Lanczos pre-downsampled offline to each
	line-spacing step in UILabelFont::BAKED_SIZES (the originals are baked at 32-42 px in 4-bit
	BC2 and blur when SpriteBatch resamples them for 10-18 px labels; the windows draw these
	1:1). They ship with the UI folder, so a missing file is not fatal: UILabelFont::Resolve
	falls back to the full-size font when a tag is absent. */
	for (const wchar_t* pFamily : { L"YG760", L"YoonGasiIIM" })
	{
		for (const int32_t iSize : UILabelFont::BAKED_SIZES)
		{
			const wstring strTag = wstring(L"Font_") + pFamily + L"_" + std::to_wstring(iSize);
			const wstring strFile = wstring(L"UI/Fonts/") + pFamily + L"_" + std::to_wstring(iSize) + L".spritefont";
			const filesystem::path smallFontPath = CRuntimeAssetRoot::Resolve(strFile);
			if (smallFontPath.empty() || FAILED(CGameInstance::Get().Add_Font(strTag, smallFontPath.c_str())))
				OutputDebugStringW((L"[Fonts] optional small label font missing: " + strFile + L"\n").c_str());
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
			CClickMoveEffect::GLOW_SHADER_TAG,
			CShader::Create(
				m_pDevice,
				m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxClickMoveGlow.hlsl"),
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
	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC),
		CClickMoveEffect::PROTOTYPE_TAG,
		CClickMoveEffect::Create(m_pDevice, m_pContext))))
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

	/* Per-target loading backgrounds: Level_Loading::Ready_Layer_Chrome swaps the Background
	slot's texture path to one of these at runtime (Valtan Arena, Character Select), so they
	never appear in LoadingLayout.json's own layers and the scan above never finds them --
	register them explicitly or the CUI_Sprite clone fails to find a texture prototype. */
	for (const wchar_t* pLoadingBackground : {
		L"UI/Loading/Loading_Background_Valtan.png",
		L"UI/Loading/Loading_Background_Prologue.png" })
	{
		const filesystem::path resolvedPath =
			CRuntimeAssetRoot::Resolve(pLoadingBackground);
		if (!resolvedPath.empty() &&
			FAILED(CGameInstance::Get().Add_Prototype(
				ETOUI(LEVEL::STATIC), pLoadingBackground,
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
	has no level awareness of its own -- it just shows whenever the flag is on. Without this,
	leaving Character Select with the preview open (e.g. entering Valtan) left its CUI_Sprite
	slots (LEVEL::STATIC, so they survive a level change) showing over the Loading screen and the
	destination level too, and its text drawing along with them. Any real level transition ends
	it. */
	m_bItemUpgradePreviewVisible = false;
	Hide_ItemUpgrade();
	CGameInstance::Get().Stop_LoopingSound();

	const CLIENT_LEVEL_DESCRIPTOR* pTarget =
		CLevelRegistry::Find(eTargetLevel);
	if (nullptr == pTarget || nullptr == pTarget->pRenderingProfileId ||
		!m_RenderingProfiles.Has_Profile(pTarget->pRenderingProfileId) ||
		!m_RenderingProfiles.Has_Profile(
			CRenderingProfileService::LOADING_PROFILE_ID))
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_ACTIVATION_PROFILE_MISSING,
			"main-app.start-level-profile",
			"Target or Loading rendering profile is not registered.",
			E_INVALIDARG);
		return E_INVALIDARG;
	}

	unique_ptr<CLevel_Loading> loading =
		CLevel_Loading::Create(
			m_pDevice,
			m_pContext,
			eTargetLevel,
			lobbyCommandToken);
	if (nullptr == loading)
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_LOADING_START_FAILED,
			"main-app.loading-level-create",
			"CLevel_Loading::Create returned null.",
			E_FAIL);
		return E_FAIL;
	}

	const string previousProfileId =
		m_RenderingProfiles.Get_ActiveProfileId();
	string status;
	if (!m_RenderingProfiles.Activate_Profile(
		CRenderingProfileService::LOADING_PROFILE_ID, status))
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_ACTIVATION_PROFILE_FAILED,
			"main-app.loading-profile-activation",
			status,
			E_FAIL);
		return E_FAIL;
	}
	const HRESULT hChange = CGameInstance::Get().Change_Level(
		ETOUI(LEVEL::LOADING),
		move(loading));
	if (FAILED(hChange))
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_LOADING_START_FAILED,
			"main-app.loading-change-level",
			"Change_Level(LOADING) failed.",
			hChange);
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
			CLevelTransitionService::Report_Recovery(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
					CLIENT_LOADING_START_FAILED,
				"main-app.start-level-request",
				request.strSource,
				result);
			// Admission already consumed a Server room slot. Reclaim it here so
			// the next Lobby click starts a fresh diagnostic generation instead
			// of reusing a bound session whose loading transition failed.
			CNetworkManager::Get().Close_ServerConnection();
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
	const bool_t levelCreated = nullptr != nextLevel;
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
#ifdef _DEBUG
	if (profileActivated && nullptr != m_pCameraTool)
		m_pCameraTool->On_LevelChanged();
	if (profileActivated && nullptr != m_pEffectToolV2)
		m_pEffectToolV2->On_LevelChanged();
#endif
	const bool_t levelChanged = profileActivated &&
		SUCCEEDED(CGameInstance::Get().Change_Level(
			ETOUI(request.eTargetLevel), move(nextLevel)));
	if (levelChanged)
	{
	#ifdef _DEBUG
		if (nullptr != m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel->On_LevelChanged();
		if (nullptr != m_pAnimationTool)
			m_pAnimationTool->On_LevelChanged();
		if (nullptr != m_pValtanActionWorkbench)
			m_pValtanActionWorkbench->On_LevelChanged();
		CKoukuSaydonPatternAuditionService::Get().Reset(
			"Level changed; no KoukuSaydon Server pattern is Live in this Client session.");
		if (nullptr != m_pEquipmentAuthoringTool)
			m_pEquipmentAuthoringTool->On_LevelChanged();
	#endif
		CEffectPresentationService::Clear_Level(iPreviousLevel);
		if (LEVEL::BERN == request.eTargetLevel &&
			CCharacterSelectionState::Has_PendingCreation() &&
			!CCharacterSelectionState::Commit_PendingCreation())
		{
			OutputDebugStringA(
				"[MainApp] Bern identity commit invariant failed.\n");
			CNetworkManager::Get().Close_ServerConnection();
			CLevelTransitionService::Report_Recovery(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
					CLIENT_IDENTITY_COMMIT_FAILED,
				"main-app.identity-commit",
				"Bern pending identity commit invariant failed.",
				E_FAIL);
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
	if (!hasTargetProfile)
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_ACTIVATION_PROFILE_MISSING,
			"main-app.target-profile-missing",
			request.strSource,
			E_FAIL);
	}
	else if (!levelCreated)
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_ACTIVATION_LEVEL_CREATE_FAILED,
			"main-app.target-level-create",
			request.strSource,
			E_FAIL);
	}
	else if (!profileActivated)
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_ACTIVATION_PROFILE_FAILED,
			"main-app.target-profile-activation",
			profileStatus,
			E_FAIL);
	}
	else
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_ACTIVATION_CHANGE_LEVEL_FAILED,
			"main-app.target-change-level",
			request.strSource,
			E_FAIL);
	}
	CNetworkManager::Get().Close_ServerConnection();
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

bool_t CMainApp::IsDebugToolVisible(const DEBUG_TOOL eTool) const
{
	const DEBUG_TOOL eCanonicalTool = DEBUG_TOOL::EFFECT_V2 == eTool ?
		DEBUG_TOOL::EFFECT :
		(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH == eTool ||
		 DEBUG_TOOL::KOUKU_SAYDON_ACTION_WORKBENCH == eTool) ? DEBUG_TOOL::SEQUENCER : eTool;
	const size_t iTool = static_cast<size_t>(eCanonicalTool);
	return DEBUG_TOOL::NONE != eCanonicalTool &&
		DEBUG_TOOL::COUNT != eCanonicalTool &&
		iTool < m_DebugToolVisible.size() && m_DebugToolVisible[iTool];
}

void CMainApp::SetDebugToolVisible(
	const DEBUG_TOOL eTool,
	const bool_t bVisible)
{
	const DEBUG_TOOL eCanonicalTool = DEBUG_TOOL::EFFECT_V2 == eTool ?
		DEBUG_TOOL::EFFECT :
		(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH == eTool ||
		 DEBUG_TOOL::KOUKU_SAYDON_ACTION_WORKBENCH == eTool) ? DEBUG_TOOL::SEQUENCER : eTool;
	const size_t iTool = static_cast<size_t>(eCanonicalTool);
	if (DEBUG_TOOL::NONE == eCanonicalTool ||
		DEBUG_TOOL::COUNT == eCanonicalTool ||
		iTool >= m_DebugToolVisible.size())
	{
		return;
	}

	m_DebugToolVisible[iTool] = bVisible;
	if (DEBUG_TOOL::SEQUENCER == eCanonicalTool)
	{
		m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH)] = false;
		m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::KOUKU_SAYDON_ACTION_WORKBENCH)] = false;
	}
	if (DEBUG_TOOL::EFFECT == eCanonicalTool)
	{
		/* The retired compatibility slot never owns independent visibility. */
		m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::EFFECT_V2)] = false;
	}
	if (!bVisible)
	{
		if (m_eDebugInputOwner == eCanonicalTool ||
			(DEBUG_TOOL::EFFECT == eCanonicalTool &&
			 DEBUG_TOOL::EFFECT_V2 == m_eDebugInputOwner))
			m_eDebugInputOwner = DEBUG_TOOL::NONE;
		if (m_eDebugWindowFocusPending == eCanonicalTool ||
			(DEBUG_TOOL::EFFECT == eCanonicalTool &&
			 DEBUG_TOOL::EFFECT_V2 == m_eDebugWindowFocusPending))
			m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
		if (DEBUG_TOOL::MAP == eCanonicalTool && nullptr != m_pMapTool)
			m_pMapTool->SetOpen(false);
		else if (DEBUG_TOOL::CAMERA == eCanonicalTool && nullptr != m_pCameraTool)
			m_pCameraTool->Deactivate();
		else if (DEBUG_TOOL::EFFECT == eCanonicalTool &&
			nullptr != m_pEffectToolV2)
		{
			/* Both codecs stop rendering on this shared visibility edge. The
			   typed backend additionally owns standalone preview actors, so it
			   explicitly releases them while preserving its authored draft. */
			m_pEffectToolV2->Deactivate();
		}
	}
}

void CMainApp::CloseAllDebugTools()
{
	for (size_t iTool = static_cast<size_t>(DEBUG_TOOL::NONE) + 1u;
		iTool < static_cast<size_t>(DEBUG_TOOL::COUNT); ++iTool)
	{
		const DEBUG_TOOL eTool = static_cast<DEBUG_TOOL>(iTool);
		if (DEBUG_TOOL::EFFECT_V2 != eTool)
			SetDebugToolVisible(eTool, false);
	}
	m_strToolStatus = "All authoring windows hidden; domain drafts remain owned by their tools.";
}

HRESULT CMainApp::EnsureAnimationPreviewBackend()
{
	// Resource clicks need the shared CModel owner, without showing or focusing
	// the Animation Tool window. Explicit Open Animation still uses EnsureDebugTool.
	if (nullptr == m_pCharacterPreviewPanel)
		m_pCharacterPreviewPanel =
			make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
	if (nullptr == m_pBalanceTool)
		m_pBalanceTool = make_unique<CBalanceTool>(false);
	if (nullptr == m_pValtanBossTool)
		m_pValtanBossTool = make_unique<CValtanBossTool>(
			make_shared<CNetworkPlayerCommandSink>(), m_pBalanceTool.get());
	if (nullptr == m_pAnimationTool)
		m_pAnimationTool = make_unique<CAnimation_Tool>(
			m_pCharacterPreviewPanel, m_pBalanceTool.get(), m_pValtanBossTool.get());
	return S_OK;
}

HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)
{
	/* Keep the old enum value as an internal compatibility route only. */
	if (DEBUG_TOOL::EFFECT_V2 == eTool)
		return EnsureDebugTool(DEBUG_TOOL::EFFECT);
	if (DEBUG_TOOL::VALTAN_ACTION_WORKBENCH == eTool ||
		DEBUG_TOOL::KOUKU_SAYDON_ACTION_WORKBENCH == eTool)
	{
		if (FAILED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER)))
			return E_FAIL;
		m_pSequencerTool->Open(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH == eTool ?
			COMPOSITION_WORKBENCH_BOSS::VALTAN : COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON);
		return S_OK;
	}

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
		if (FAILED(EnsureAnimationPreviewBackend())) return E_FAIL;
		break;
	case DEBUG_TOOL::EFFECT:
	{
		if (nullptr == m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel =
				make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>();
		const bool_t bFirstEffectToolOpen = nullptr == m_pEffectTool;
		if (nullptr == m_pEffectTool)
			m_pEffectTool =
				make_unique<CEffect_Tool>(
					m_pDevice, m_pContext, m_pCharacterPreviewPanel,
					m_pBalanceTool.get());
		/* A Save may have completed while the Effect owner did not exist. Hand
		   over its exact receipt before first-open code can perform an unpinned
		   canonical refresh. */
		if (nullptr != m_pValtanActionWorkbench)
		{
			std::string ExpectedValtanSourceRevision;
			if (m_pValtanActionWorkbench->
					Consume_EffectGraphRefreshRequest(
						ExpectedValtanSourceRevision))
			{
				m_pEffectTool->Request_ValtanGraphRefresh(
					ExpectedValtanSourceRevision);
			}
		}
		if (nullptr == m_pEffectToolV2)
			m_pEffectToolV2 =
				make_unique<CEffect_Tool_V2>(m_pDevice, m_pContext);
		/* The Valtan Arena's normal F1 entry should open the existing authored /
		   Server Pattern / independent Effect workspace, not an unrelated Player
		   skill list.  Preserve an explicit Character selection on later hide/show. */
		if (bFirstEffectToolOpen &&
			ETOUI(LEVEL::VALTAN_ARENA) ==
				CGameInstance::Get().Get_CurrentLevelID())
		{
			(void)m_pEffectTool->Open_ValtanAllEffectsWorkspace();
		}
		break;
	}
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
		if (nullptr == m_pRenderingBenchmark)
			m_pRenderingBenchmark = make_unique<CRenderingBenchmark>();
		break;
	case DEBUG_TOOL::PROFILER:
		if (nullptr == m_pProfilerTool)
			m_pProfilerTool = make_unique<CProfilerTool>();
		m_pProfilerTool->Open();
		if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
			pProfiler->Set_Enabled(true);
		break;
	case DEBUG_TOOL::SEQUENCER:
		if (FAILED(EnsureAnimationPreviewBackend())) return E_FAIL;
		// Construct both independent sessions without loading the other boss's
		// documents. The selected session lazily prepares its own frame.
		if (nullptr == m_pValtanActionWorkbench)
			m_pValtanActionWorkbench = make_unique<CValtanActionWorkbench>(
				m_pAnimationTool.get(), m_pBalanceTool.get(), m_pValtanBossTool.get());
		if (nullptr == m_pKoukuSaydonActionWorkbench)
			m_pKoukuSaydonActionWorkbench = make_unique<CKoukuSaydonActionWorkbench>();
		if (nullptr == m_pSequencerTool)
		{
			m_pSequencerTool = make_unique<CSequencerTool>(
				m_pValtanActionWorkbench.get(), m_pKoukuSaydonActionWorkbench.get());
			m_pSequencerTool->Open(ETOUI(LEVEL::KAKULSAYDON_ARENA) ==
				CGameInstance::Get().Get_CurrentLevelID() ?
				COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON : COMPOSITION_WORKBENCH_BOSS::VALTAN);
		}
		else
			m_pSequencerTool->Open();
		break;
	case DEBUG_TOOL::UI:
		if (nullptr == m_pHUDLayoutTool)
			m_pHUDLayoutTool =
				make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
		break;
	case DEBUG_TOOL::BALANCE:
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>();
		m_pBalanceTool->Open();
		break;
	case DEBUG_TOOL::VALTAN_BOSS:
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>();
		(void)m_pBalanceTool->Ensure_Initialized();
		if (nullptr == m_pValtanBossTool)
			m_pValtanBossTool = make_unique<CValtanBossTool>(
				make_shared<CNetworkPlayerCommandSink>(),
				m_pBalanceTool.get());
		m_pValtanBossTool->Open();
		break;
	case DEBUG_TOOL::KOUKU_SAYDON_BOSS:
		if (nullptr == m_pKoukuSaydonBossTool)
			m_pKoukuSaydonBossTool = make_unique<CKoukuSaydonBossTool>();
		m_pKoukuSaydonBossTool->Open();
		break;
	case DEBUG_TOOL::VALTAN_LOGIC_PATTERN:
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>();
		(void)m_pBalanceTool->Ensure_Initialized();
		if (nullptr == m_pValtanBossTool)
			m_pValtanBossTool = make_unique<CValtanBossTool>(
				make_shared<CNetworkPlayerCommandSink>(),
				m_pBalanceTool.get());
		m_pValtanBossTool->Open_LogicPattern();
		break;
	case DEBUG_TOOL::CAMERA:
		if (nullptr == m_pCameraTool)
			m_pCameraTool = make_unique<CCameraTool>();
		m_pCameraTool->Open();
		break;
	case DEBUG_TOOL::EQUIPMENT:
		if (nullptr == m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel =
				make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
		if (nullptr == m_pEquipmentAuthoringTool)
			m_pEquipmentAuthoringTool =
				make_unique<CEquipmentAuthoringTool>(
					m_pDevice, m_pContext, m_pCharacterPreviewPanel);
		break;
	default:
		return E_INVALIDARG;
	}

	SetDebugToolVisible(eTool, true);
	/* Valtan Logic Pattern is a read-only Server-state blueprint. Opening or focusing
	it must not steal the explicit world/preview input owner. */
	if (DEBUG_TOOL::VALTAN_LOGIC_PATTERN != eTool)
		m_eDebugInputOwner = eTool;
	m_eDebugWindowFocusPending = eTool;
	return S_OK;
}

bool_t CMainApp::RequestDebugLevelNavigation(const LEVEL eTargetLevel)
{
	const LEVEL currentLevel = static_cast<LEVEL>(
		CGameInstance::Get().Get_CurrentLevelID());
	if (LEVEL::END == eTargetLevel || LEVEL::LOADING == currentLevel)
	{
		m_strDebugLevelNavigationStatus =
			"Level navigation is unavailable while the current Level is loading or the target is invalid.";
		return false;
	}
	if (eTargetLevel == currentLevel)
	{
		m_strDebugLevelNavigationStatus =
			"The selected Level is already current.";
		return false;
	}
	if (LEVEL::END != m_eDebugLevelNavigationTarget ||
		CLevelTransitionService::Is_Pending())
	{
		m_strDebugLevelNavigationStatus =
			"Another Server entry or Level transition is already pending.";
		return false;
	}

	auto routeThroughLobby = [this, currentLevel](
		const LOBBY_STAGE stage,
		const LEVEL finalTarget,
		const char_t* targetName) -> bool_t
	{
		bool_t accepted = false;
		if (LEVEL::LOBBY == currentLevel)
		{
			accepted = CLevel_Lobby::Submit_ProductCommand(stage);
			if (!accepted)
			{
				m_strDebugLevelNavigationStatus =
					CLevel_Lobby::Get_ProductStatus();
				return false;
			}
		}
		else if (LEVEL::CHARACTER_SELECT == currentLevel)
		{
			CLevel_CharacterSelect* pCharacterSelect =
				CLevel_CharacterSelect::Get_Active();
			accepted = nullptr != pCharacterSelect &&
				pCharacterSelect->Debug_Request_ProductStage(stage);
			if (!accepted)
			{
				m_strDebugLevelNavigationStatus = nullptr != pCharacterSelect ?
					pCharacterSelect->Debug_GetNavigationStatus() :
					"The active Character Select owner is unavailable.";
				return false;
			}
		}
		else
		{
			LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
			if (!CLobbyCommandService::Request(stage, token))
			{
				m_strDebugLevelNavigationStatus =
					CLobbyCommandService::Get_Status();
				return false;
			}
			if (!CLevelTransitionService::Request_Load(
				LEVEL::LOBBY,
				"f1.level-navigation.route-via-lobby",
				token))
			{
				CLobbyCommandService::Cancel(
					token, "F1 Level Navigation load was rejected");
				m_strDebugLevelNavigationStatus =
					CLevelTransitionService::Get_Status();
				return false;
			}
			accepted = true;
		}

		m_eDebugLevelNavigationTarget = finalTarget;
		m_DebugLevelNavigationDeadline =
			std::chrono::steady_clock::now() + std::chrono::seconds(15);
		m_bDebugLevelNavigationDeadlineActive = true;
		m_strDebugLevelNavigationStatus =
			string("Server-approved route staged for ") + targetName +
			". Lobby owns connection and admission; F1 does not send a packet directly.";
		return accepted;
	};

	if (LEVEL::LOBBY == eTargetLevel)
	{
		if (!CLevelTransitionService::Request_Load(
			LEVEL::LOBBY, "f1.level-navigation.return-lobby"))
		{
			m_strDebugLevelNavigationStatus =
				CLevelTransitionService::Get_Status();
			return false;
		}
		m_eDebugLevelNavigationTarget = LEVEL::LOBBY;
		m_DebugLevelNavigationDeadline =
			std::chrono::steady_clock::now() + std::chrono::seconds(15);
		m_bDebugLevelNavigationDeadlineActive = true;
		m_strDebugLevelNavigationStatus =
			"Typed Lobby return staged. Gameplay Areas remain Server-authoritative and no destination admission was fabricated.";
		return true;
	}
	if (LEVEL::KAKULSAYDON_ARENA == eTargetLevel)
	{
		if (LEVEL::CHARACTER_SELECT == currentLevel)
		{
			CLevel_CharacterSelect* pCharacterSelect =
				CLevel_CharacterSelect::Get_Active();
			if (nullptr == pCharacterSelect ||
				!pCharacterSelect->Debug_Request_KakulSaydonArena())
			{
				m_strDebugLevelNavigationStatus = nullptr != pCharacterSelect ?
					pCharacterSelect->Debug_GetNavigationStatus() :
					"The active Character Select owner is unavailable.";
				return false;
			}
			m_eDebugLevelNavigationTarget = LEVEL::KAKULSAYDON_ARENA;
			m_DebugLevelNavigationDeadline =
				std::chrono::steady_clock::now() + std::chrono::seconds(15);
			m_bDebugLevelNavigationDeadlineActive = true;
			m_strDebugLevelNavigationStatus =
				pCharacterSelect->Debug_GetNavigationStatus();
			return true;
		}

		if (!routeThroughLobby(
			LOBBY_STAGE::CHARACTER_SELECT,
			LEVEL::CHARACTER_SELECT,
			"Character Select (required before KoukuSaydon)"))
		{
			return false;
		}
		m_strDebugLevelNavigationStatus =
			"KoukuSaydon requires its existing Character Select command sink. Routing to Server-approved Character Select first; press KoukuSaydon again after admission.";
		return true;
	}
	if (LEVEL::CHARACTER_SELECT == eTargetLevel)
		return routeThroughLobby(
			LOBBY_STAGE::CHARACTER_SELECT, eTargetLevel, "Character Select");
	if (LEVEL::BERN == eTargetLevel)
		return routeThroughLobby(LOBBY_STAGE::BERN, eTargetLevel, "Bern");
	if (LEVEL::VALTAN_ARENA == eTargetLevel)
		return routeThroughLobby(LOBBY_STAGE::VALTAN, eTargetLevel, "Valtan");

	m_strDebugLevelNavigationStatus =
		"F1 Level Navigation exposes only Lobby, Character Select, Bern, Valtan and KoukuSaydon.";
	return false;
}

void CMainApp::RenderArenaCameraAndPlayerControls()
{
	const LEVEL level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
	CLevel_ValtanArena* valtan = LEVEL::VALTAN_ARENA == level ?
		CLevel_ValtanArena::Get_Active() : nullptr;
	CLevel_KakulSaydonArena* kouku = LEVEL::KAKULSAYDON_ARENA == level ?
		CLevel_KakulSaydonArena::Get_Active() : nullptr;
	shared_ptr<CCamera_Free> camera;
	CPlayerController* controller = nullptr;
	if (nullptr != valtan)
	{
		camera = valtan->Get_DebugCamera();
		controller = &valtan->Get_DebugPlayerController();
	}
	else if (nullptr != kouku)
	{
		camera = kouku->Get_DebugCamera();
		controller = &kouku->Get_DebugPlayerController();
	}
	if (nullptr == camera || nullptr == controller)
		return;
	const auto setSpeed = [valtan, kouku](const f32_t speed)
	{
		if (nullptr != valtan) valtan->Set_DebugCameraSpeed(speed);
		else if (nullptr != kouku) kouku->Set_DebugCameraSpeed(speed);
	};
	ImGui::SeparatorText("Arena Camera / Player");
	ImGui::Text("Current arena: %s", nullptr != valtan ? "Valtan" : "KoukuSaydon");
	f32_t speed = camera->Get_FreeMoveSpeed();
	if (ImGui::DragFloat("Free camera speed (m/s)", &speed, 0.5f,
		CCamera_Free::MIN_FREE_MOVE_SPEED, CCamera_Free::MAX_FREE_MOVE_SPEED,
		"%.1f", ImGuiSliderFlags_AlwaysClamp))
		setSpeed(speed);
	if (ImGui::Button("Reset speed to 20 m/s"))
		setSpeed(CCamera_Free::DEFAULT_ARENA_MOVE_SPEED);
	ImGui::TextDisabled("Shift: x%.0f (%.1f m/s). Saved per arena for this session.",
		CCamera_Free::FREE_MOVE_SPRINT_MULTIPLIER,
		camera->Get_FreeMoveSpeed() * CCamera_Free::FREE_MOVE_SPRINT_MULTIPLIER);

	const bool_t freeCamera = !camera->Is_FollowRequested() &&
		!camera->Is_PresentationOverrideActive();
	ImGui::BeginDisabled(!freeCamera || controller->Is_DebugPlayerPlacementPending() ||
		controller->Is_DebugPlayerPlacementArmed());
	if (ImGui::Button("Move Player"))
	{
		const auto world = nullptr != valtan ? LostArk::Shared::WORLD_ID::VALTAN_ARENA :
			LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA;
		if (controller->Begin_DebugPlayerPlacement(world))
			camera->Set_MouseLookEnabled(false);
	}
	ImGui::EndDisabled();
	if (controller->Is_DebugPlayerPlacementArmed())
	{
		ImGui::SameLine();
		if (ImGui::Button("Cancel Pick"))
			controller->Cancel_DebugPlayerPlacement();
	}
	ImGui::TextDisabled("F6 free camera -> Move Player -> click ground. Esc / right-click cancels.");
	ImGui::TextDisabled("Tab toggles mouse-look. Only your player moves after Server approval.");
	if (!controller->Get_DebugPlayerPlacementStatus().empty())
		ImGui::TextWrapped("%s", controller->Get_DebugPlayerPlacementStatus().c_str());
	if (nullptr != kouku)
		RenderKoukuUiPreviewControls();
}

void CMainApp::RenderKoukuUiPreviewControls()
{
	ImGui::SeparatorText("Kouku UI Preview (Debug)");
	CCombatHUDViewModel& viewModel = CCombatHUDViewModel::Get();
	const KOUKU_HUD_MODE_DEF* pMode = nullptr;
	for (const KOUKU_HUD_MODE_DEF& Mode : m_KoukuHudModes)
	{
		const char* pModeId = KoukuHudModeId(m_KoukuUiPreview.eHudMode);
		if (nullptr != pModeId && Mode.strId == pModeId)
		{
			pMode = &Mode;
			break;
		}
	}
	/* Skill k -> slot k in list order; the dance mode shuffles the four so QWER
	differs per activation, the way the real gimmick randomises it. */
	const auto assignSlots = [this, &pMode]()
	{
		for (std::int8_t& iIndex : m_KoukuUiPreview.ModeSkillIndexBySlot)
			iIndex = -1;
		for (uint32_t& iTick : m_KoukuUiPreview.CooldownEndTicks)
			iTick = 0u;
		if (nullptr == pMode)
			return;
		const size_t iCount = (std::min)(pMode->Skills.size(), HUD_KOUKU_SLOT_COUNT);
		vector<std::int8_t> Order;
		for (size_t k = 0; k < iCount; ++k)
			Order.push_back(static_cast<std::int8_t>(k));
		if (pMode->bRandomOrder)
		{
			std::mt19937 Rng(static_cast<uint32_t>(
				std::chrono::steady_clock::now().time_since_epoch().count()));
			std::shuffle(Order.begin(), Order.end(), Rng);
		}
		for (size_t k = 0; k < iCount; ++k)
			m_KoukuUiPreview.ModeSkillIndexBySlot[k] = Order[k];
	};

	bool_t bChanged = false;
	if (ImGui::Checkbox("Enable preview##Kouku", &m_bKoukuUiPreview))
		bChanged = true;
	int32_t iGauge = static_cast<int32_t>(m_KoukuUiPreview.iMadnessGauge);
	if (ImGui::SliderInt("Madness gauge##Kouku", &iGauge, 0, 100))
	{
		m_KoukuUiPreview.iMadnessGauge = static_cast<uint32_t>(iGauge);
		bChanged = true;
	}
	constexpr const char* MODE_LABELS[] =
		{ "None", "Polymorph (clown)", "Mario", "Dance", "Card maze" };
	int32_t iMode = static_cast<int32_t>(m_KoukuUiPreview.eHudMode);
	if (ImGui::Combo("HUD mode##Kouku", &iMode, MODE_LABELS,
		static_cast<int32_t>(sizeof(MODE_LABELS) / sizeof(MODE_LABELS[0]))))
	{
		m_KoukuUiPreview.eHudMode = static_cast<HUD_KOUKU_HUD_MODE>(iMode);
		pMode = nullptr;
		for (const KOUKU_HUD_MODE_DEF& Mode : m_KoukuHudModes)
		{
			const char* pModeId = KoukuHudModeId(m_KoukuUiPreview.eHudMode);
			if (nullptr != pModeId && Mode.strId == pModeId)
			{
				pMode = &Mode;
				break;
			}
		}
		assignSlots();
		bChanged = true;
	}
	if (ImGui::Button("Reroll slot order##Kouku"))
	{
		assignSlots();
		bChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Start 5s cooldowns##Kouku"))
	{
		const uint32_t iTick = viewModel.Get_Player().iServerTick;
		for (size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
		{
			if (m_KoukuUiPreview.ModeSkillIndexBySlot[i] < 0)
				continue;
			m_KoukuUiPreview.CooldownDurationTicks[i] = 150u;
			m_KoukuUiPreview.CooldownEndTicks[i] = iTick + 150u;
		}
		bChanged = true;
	}
	if (bChanged)
	{
		m_KoukuUiPreview.isValid = m_bKoukuUiPreview;
		m_KoukuUiPreview.iMadnessMaximum = 100u;
		viewModel.Apply_KoukuGimmick(m_KoukuUiPreview);
	}
	ImGui::TextDisabled("Preview only (no Server truth). Modes: %zu loaded.", m_KoukuHudModes.size());
}

void CMainApp::RenderDebugLevelNavigation()
{
	auto levelName = [](const LEVEL level) -> const char_t*
	{
		switch (level)
		{
		case LEVEL::LOBBY: return "Lobby";
		case LEVEL::CHARACTER_SELECT: return "Character Select";
		case LEVEL::BERN: return "Bern";
		case LEVEL::VALTAN_ARENA: return "Valtan";
		case LEVEL::KAKULSAYDON_ARENA: return "KoukuSaydon";
		case LEVEL::LOADING: return "Loading";
		case LEVEL::DEVELOPMENT: return "Development";
		default: return "Other";
		}
	};
	const LEVEL currentLevel = static_cast<LEVEL>(
		CGameInstance::Get().Get_CurrentLevelID());
	const bool_t transitionPending =
		CLevelTransitionService::Is_Pending();
	if (LEVEL::END != m_eDebugLevelNavigationTarget &&
		!transitionPending && currentLevel == m_eDebugLevelNavigationTarget)
	{
		m_strDebugLevelNavigationStatus =
			string("Arrived at ") + levelName(currentLevel) +
			" through the typed Level route.";
		m_eDebugLevelNavigationTarget = LEVEL::END;
		m_bDebugLevelNavigationDeadlineActive = false;
	}
	else if (LEVEL::END != m_eDebugLevelNavigationTarget &&
		m_bDebugLevelNavigationDeadlineActive &&
		std::chrono::steady_clock::now() >=
			m_DebugLevelNavigationDeadline)
	{
		const string ownerStatus = LEVEL::LOBBY == currentLevel ?
			CLevel_Lobby::Get_ProductStatus() :
			CLevelTransitionService::Get_Status();
		m_strDebugLevelNavigationStatus =
			"F1 route tracking timed out after 15 seconds and was unlocked. The typed owner remains authoritative; retry only after checking: " +
			ownerStatus;
		m_eDebugLevelNavigationTarget = LEVEL::END;
		m_bDebugLevelNavigationDeadlineActive = false;
	}

	ImGui::SeparatorText("Level Navigation");
	ImGui::Text("Current: %s", levelName(currentLevel));
	ImGui::SameLine();
	ImGui::TextDisabled(
		"| Pending: %s",
		LEVEL::END != m_eDebugLevelNavigationTarget ?
			levelName(m_eDebugLevelNavigationTarget) :
			(transitionPending ? "typed transition" : "None"));
	ImGui::TextWrapped("%s", m_strDebugLevelNavigationStatus.c_str());
	ImGui::TextDisabled(
		"Bern/Valtan/Character Select use Lobby admission. KoukuSaydon uses Character Select's world-transfer command sink.");

	constexpr std::array<std::pair<LEVEL, const char_t*>, 5> destinations = {{
		{ LEVEL::LOBBY, "Lobby" },
		{ LEVEL::CHARACTER_SELECT, "Character Select" },
		{ LEVEL::BERN, "Bern" },
		{ LEVEL::VALTAN_ARENA, "Valtan" },
		{ LEVEL::KAKULSAYDON_ARENA, "KoukuSaydon" },
	}};
	for (size_t iDestination = 0u;
		iDestination < destinations.size(); ++iDestination)
	{
		if (0u != iDestination)
			ImGui::SameLine();
		const LEVEL target = destinations[iDestination].first;
		const bool_t disable = currentLevel == target ||
			LEVEL::END != m_eDebugLevelNavigationTarget ||
			transitionPending || LEVEL::LOADING == currentLevel;
		ImGui::BeginDisabled(disable);
		if (ImGui::SmallButton(destinations[iDestination].second))
			(void)RequestDebugLevelNavigation(target);
		ImGui::EndDisabled();
	}
	if (transitionPending)
	{
		ImGui::TextDisabled(
			"Transition owner: %s",
			CLevelTransitionService::Get_Status().c_str());
	}
}

void CMainApp::RefreshDebugAuthoringSources()
{
	const filesystem::path dataRoot = CProjectDataRoot::Get();
	auto addSource = [this](
		const char_t* domain,
		const char_t* displayName,
		const char_t* canonicalPath,
		const char_t* description,
		const char_t* actionLabel,
		const DEBUG_TOOL tool,
		const size_t documentCount,
		const bool_t sourceReady,
		const bool_t openValtanWorkspace,
		const bool_t openValtanEffectWorkspace)
	{
		DEBUG_AUTHORING_SOURCE source;
		source.strDomain = domain;
		source.strDisplayName = displayName;
		source.strCanonicalPath = canonicalPath;
		source.strDescription = description;
		source.strActionLabel = actionLabel;
		source.eTool = tool;
		source.iDocumentCount = documentCount;
		source.bCanonicalSourceReady = sourceReady;
		source.bOpenValtanWorkspace = openValtanWorkspace;
		source.bOpenValtanEffectWorkspace = openValtanEffectWorkspace;
		m_DebugAuthoringSources.push_back(std::move(source));
	};
	auto sourceExists = [&dataRoot](const filesystem::path& relative)
	{
		std::error_code error;
		return filesystem::exists(dataRoot / relative, error) && !error;
	};

	m_bDebugAuthoringSourceRefreshAttempted = true;
	m_DebugAuthoringSources.clear();

	addSource(
		"Boss / Pattern", "Valtan Action Composition",
		"Data/Valtan/Valtan.gameplay.json",
		"Canonical gameplay source joined with Valtan.presentation.json into the Server and presentation Products.",
		"Open Action Workbench (Valtan)", DEBUG_TOOL::VALTAN_ACTION_WORKBENCH,
		2u,
		sourceExists(L"Valtan/Valtan.gameplay.json") &&
			sourceExists(L"Valtan/Valtan.presentation.json"), true, false);
	addSource(
		"Character / Animation", "Valtan Animation & Sequence Source",
		"Data/Valtan/Valtan.presentation.json",
		"Writable Pattern animation occurrences, Effect invocations and camera edges. Generated bindings remain read-only Product.",
		"Open Composition Detail", DEBUG_TOOL::VALTAN_ACTION_WORKBENCH,
		1u,
		sourceExists(L"Valtan/Valtan.presentation.json"), true, false);
	addSource(
		"Effect Resource", "Valtan Effect Resources",
		"Data/Effects",
		"One Effect Resource entry joins direct-authored effect.valtan.* documents with typed boss.valtan.* leaves, groups and BOSS_VALTAN bindings.",
		"Open Effect Tool", DEBUG_TOOL::EFFECT,
		2u,
		sourceExists(L"Effects/EffectCatalog.json") &&
			sourceExists(L"Effects/V2/Authored") &&
			sourceExists(L"Effects/V2/Groups") &&
			sourceExists(L"Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"),
		false, true);
	addSource(
		"Sound", "Valtan Pattern Sound Cues",
		"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json",
		"Sound cues joined to stable Pattern and Sequence slots; edited from the integrated Valtan Detail.",
		"Open Sound Cue Detail", DEBUG_TOOL::VALTAN_ACTION_WORKBENCH,
		1u,
		sourceExists(L"Animation/Authored/Valtan/Valtan.patternsoundcues.json"), true, false);
	addSource(
		"Gameplay", "Valtan Gameplay & Balance",
		"Data/Balance/BossProfiles.json",
		"Server-owned boss HP, stagger and damage tuning. Pattern presentation never overrides these values.",
		"Open Balance Tool", DEBUG_TOOL::BALANCE,
		1u,
		sourceExists(L"Balance/BossProfiles.json"), false, false);
	addSource(
		"Encounter", "Valtan Encounter Runtime",
		"Data/Encounters/Valtan/ValtanEncounter.json",
		"Server arena, walls, world events, flow and Complete Play admission documents.",
		"Open Boss Verification", DEBUG_TOOL::VALTAN_BOSS,
		1u,
		sourceExists(L"Encounters/Valtan/ValtanEncounter.json"), false, false);
	addSource(
		"Character / Animation", "All Authored Character Bindings",
		"Data/Animation/Authored",
		"Character skill/action bindings only. Extracted clips and reference timing stay read-only intake data.",
		"Open Animation Intake", DEBUG_TOOL::ANIMATION,
		1u,
		sourceExists(L"Animation/Authored"), false, false);
}

void CMainApp::OpenDebugAuthoringSource(const size_t iSource)
{
	if (iSource >= m_DebugAuthoringSources.size())
		return;
	const DEBUG_AUTHORING_SOURCE& source = m_DebugAuthoringSources[iSource];
	if (!source.bCanonicalSourceReady)
	{
		m_strToolStatus = "Canonical source is unavailable; no owner was opened: " +
			source.strCanonicalPath;
		return;
	}
	if (FAILED(EnsureDebugTool(source.eTool)))
	{
		m_strToolStatus = "Canonical source is ready, but its owner Tool failed to initialize: " +
			source.strCanonicalPath;
		return;
	}
	if (source.bOpenValtanWorkspace)
	{
		(void)EnsureDebugTool(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH);
		if (nullptr == m_pValtanActionWorkbench)
		{
			m_strToolStatus =
				"Valtan sources are present, but Valtan Action Workbench is unavailable.";
			return;
		}
		const bool_t bFullyAdmitted =
			m_pValtanActionWorkbench->Open_Valtan();
		if (!bFullyAdmitted)
		{
			if (!m_pValtanActionWorkbench->Has_DisplaySnapshot())
			{
				m_strToolStatus =
					"Valtan Action Workbench opened, but canonical Product admission was rejected; inspect its diagnostic banner.";
				return;
			}
			m_strToolStatus =
				"Valtan Action Workbench opened with a canonical Product snapshot in read-only mode; typed source join needs attention before Save or Server playback.";
			return;
		}
	}
	if (source.bOpenValtanEffectWorkspace &&
		(nullptr == m_pEffectTool ||
		!m_pEffectTool->Open_ValtanAllEffectsWorkspace()))
	{
		m_strToolStatus = "Valtan Effect Resource is ready, but its direct-authored workspace could not be opened.";
		return;
	}
	m_strToolStatus = "Opened " + source.strDisplayName + " from " +
		source.strCanonicalPath + ". Select its semantic row to edit in Detail; this F1 summary never edits or republishes files directly.";
}

void CMainApp::RefreshDebugResourceFiles()
{
	struct RESOURCE_ROOT
	{
		const char_t* pDomain;
		const char_t* pSource;
		filesystem::path Root;
		const char_t* pStablePrefix;
		DEBUG_TOOL eTool;
	};

	const filesystem::path resourceRoot =
		CRuntimeAssetRoot::Get_ResourceRoot();
	const filesystem::path dataRoot = CProjectDataRoot::Get();
	const filesystem::path publishedDataRoot =
		resourceRoot.parent_path() / L"DataFiles";
	const std::array<RESOURCE_ROOT, 27> roots = {{
		{ "Character / Animation", "Resources", resourceRoot / L"Character",
			"Resources/Character", DEBUG_TOOL::ANIMATION },
		{ "Character / Animation", "Data", dataRoot / L"Animation",
			"Data/Animation", DEBUG_TOOL::ANIMATION },
		{ "Boss / Pattern", "Data", dataRoot / L"Valtan",
			"Data/Valtan", DEBUG_TOOL::VALTAN_ACTION_WORKBENCH },
		{ "Boss / Pattern", "Data", dataRoot / L"Encounters",
			"Data/Encounters", DEBUG_TOOL::VALTAN_BOSS },
		{ "Boss / Pattern", "Data", dataRoot / L"Actors",
			"Data/Actors", DEBUG_TOOL::VALTAN_BOSS },
		{ "Effect Resource", "Resources", resourceRoot / L"Effect",
			"Resources/Effect", DEBUG_TOOL::EFFECT },
		{ "Effect Resource", "Data", dataRoot / L"Effects" / L"Authored",
			"Data/Effects/Authored", DEBUG_TOOL::EFFECT },
		{ "Effect Resource", "Data", dataRoot / L"Effects" / L"Assemblies",
			"Data/Effects/Assemblies", DEBUG_TOOL::EFFECT },
		{ "Effect Resource", "Data", dataRoot / L"Effects" / L"V2",
			"Data/Effects/V2", DEBUG_TOOL::EFFECT },
		{ "Sound", "Resources", resourceRoot / L"Sound",
			"Resources/Sound", DEBUG_TOOL::ANIMATION },
		{ "Sound", "Data", dataRoot / L"Sound",
			"Data/Sound", DEBUG_TOOL::ANIMATION },
		{ "Map / World / Navigation", "Resources", resourceRoot / L"Map",
			"Resources/Map", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Resources", resourceRoot / L"Deploy",
			"Resources/Deploy", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Data", dataRoot / L"Maps",
			"Data/Maps", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Data", dataRoot / L"Worlds",
			"Data/Worlds", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Data", dataRoot / L"Navigation",
			"Data/Navigation", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Published", publishedDataRoot / L"Map",
			"DataFiles/Map", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Published", publishedDataRoot / L"World",
			"DataFiles/World", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Published", publishedDataRoot / L"Navigation",
			"DataFiles/Navigation", DEBUG_TOOL::MAP },
		{ "UI / Fonts", "Resources", resourceRoot / L"UI",
			"Resources/UI", DEBUG_TOOL::UI },
		{ "UI / Fonts", "Resources", resourceRoot / L"Fonts",
			"Resources/Fonts", DEBUG_TOOL::UI },
		{ "UI / Fonts", "Data", dataRoot / L"UI",
			"Data/UI", DEBUG_TOOL::UI },
		{ "Gameplay / Rendering", "Data", dataRoot / L"Balance",
			"Data/Balance", DEBUG_TOOL::BALANCE },
		{ "Gameplay / Rendering", "Data", dataRoot / L"Items",
			"Data/Items", DEBUG_TOOL::BALANCE },
		{ "Gameplay / Rendering", "Data", dataRoot / L"Rendering",
			"Data/Rendering", DEBUG_TOOL::RENDERING },
		{ "Gameplay / Rendering", "Published", publishedDataRoot / L"Rendering",
			"DataFiles/Rendering", DEBUG_TOOL::RENDERING },
		{ "Gameplay / Rendering", "Data", dataRoot / L"ResourceIntake",
			"Data/ResourceIntake", DEBUG_TOOL::MAP },
	}};

	auto pathToUtf8 = [](const filesystem::path& path)
	{
		const u8string utf8 = path.generic_u8string();
		return string(utf8.begin(), utf8.end());
	};
	auto lowerAscii = [](string value)
	{
		std::transform(value.begin(), value.end(), value.begin(),
			[](const unsigned char character)
			{
				return static_cast<char_t>(std::tolower(character));
			});
		return value;
	};

	m_bDebugResourceScanAttempted = true;
	m_DebugResourceFiles.clear();
	m_iSelectedDebugResourceFile = static_cast<size_t>(-1);
	constexpr size_t MAX_RESOURCE_FILES = 50000u;
	std::error_code error;
	for (const RESOURCE_ROOT& root : roots)
	{
		error.clear();
		if (!filesystem::is_directory(root.Root, error) || error)
			continue;

		filesystem::recursive_directory_iterator iterator(
			root.Root,
			filesystem::directory_options::skip_permission_denied,
			error);
		const filesystem::recursive_directory_iterator end;
		for (; !error && iterator != end; iterator.increment(error))
		{
			if (m_DebugResourceFiles.size() >= MAX_RESOURCE_FILES)
				break;
			if (!iterator->is_regular_file(error) || error)
			{
				error.clear();
				continue;
			}
			const filesystem::path relative =
				iterator->path().lexically_relative(root.Root);
			if (relative.empty())
				continue;

			DEBUG_RESOURCE_FILE file;
			file.strDomain = root.pDomain;
			file.strSource = root.pSource;
			file.strRelativePath = std::string(root.pStablePrefix) + "/" +
				pathToUtf8(relative);
			file.strSearchText = lowerAscii(
				file.strDomain + " " + file.strSource + " " +
				file.strRelativePath);
			file.eTool = root.eTool;
			m_DebugResourceFiles.push_back(std::move(file));
		}
		if (m_DebugResourceFiles.size() >= MAX_RESOURCE_FILES)
			break;
	}

	/* Data itself is also scanned above to expose catalogs that have no dedicated
	   domain root. Remove exact duplicate stable paths while retaining the more
	   specific domain entry inserted first. */
	std::stable_sort(
		m_DebugResourceFiles.begin(), m_DebugResourceFiles.end(),
		[](const DEBUG_RESOURCE_FILE& left, const DEBUG_RESOURCE_FILE& right)
		{
			return std::tie(left.strRelativePath, left.strDomain) <
				std::tie(right.strRelativePath, right.strDomain);
		});
	m_DebugResourceFiles.erase(
		std::unique(
			m_DebugResourceFiles.begin(), m_DebugResourceFiles.end(),
			[](const DEBUG_RESOURCE_FILE& left, const DEBUG_RESOURCE_FILE& right)
			{
				return left.strRelativePath == right.strRelativePath;
			}),
		m_DebugResourceFiles.end());

	/* KoukuSaydon is a user-facing collection, not a replacement for package or
	   Area identity. Present the extracted closure under one virtual branch while
	   keeping paths such as LV_LUT_MIDNIGHTC_ED and MN_RPCT_05 unchanged. */
	const auto startsWith = [](const string& value, const char_t* prefix)
	{
		return 0u == value.rfind(prefix, 0u);
	};
	std::vector<DEBUG_RESOURCE_FILE> koukuSaydonCollection;
	for (const DEBUG_RESOURCE_FILE& file : m_DebugResourceFiles)
	{
		const string& path = file.strRelativePath;
		const bool_t isKoukuSaydonAsset =
			startsWith(path, "Resources/Map/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "DataFiles/Map/LV_LUT_MIDNIGHTC_ED") ||
			startsWith(path, "Resources/Effect/KoukuSaton/") ||
			startsWith(path, "Resources/Effect/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "Resources/UI/KoukuSaton/") ||
			startsWith(path, "Resources/UI/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "Resources/Sound/KoukuSaton/") ||
			startsWith(path, "Data/Animation/Reference/KoukuSaydon/") ||
			startsWith(path, "Data/Animation/Authored/KoukuSaydon/") ||
			startsWith(path, "Data/ResourceIntake/LV_LUT_MIDNIGHTC_ED") ||
			startsWith(path, "Resources/Character/KoukuSaton/") ||
			startsWith(path, "Resources/Character/MN_RPCT_00/") ||
			startsWith(path, "Resources/Character/MN_RPCT_05/") ||
			startsWith(path, "Resources/Character/MN_RPCT_06/") ||
			startsWith(path, "Resources/Character/MN_RPCZ_00/") ||
			startsWith(path, "Resources/Character/WP_MN_RPCT_05/") ||
			startsWith(path, "Resources/Character/WP_MN_RPCT_06/");
		if (!isKoukuSaydonAsset)
			continue;
		DEBUG_RESOURCE_FILE collectionFile = file;
		collectionFile.strDomain = "KoukuSaydon";
		collectionFile.strSearchText = lowerAscii(
			collectionFile.strDomain + " " + collectionFile.strSource + " " +
			collectionFile.strRelativePath);
		koukuSaydonCollection.push_back(std::move(collectionFile));
	}
	m_DebugResourceFiles.insert(
		m_DebugResourceFiles.end(),
		std::make_move_iterator(koukuSaydonCollection.begin()),
		std::make_move_iterator(koukuSaydonCollection.end()));
	std::stable_sort(
		m_DebugResourceFiles.begin(), m_DebugResourceFiles.end(),
		[](const DEBUG_RESOURCE_FILE& left, const DEBUG_RESOURCE_FILE& right)
		{
			return std::tie(left.strDomain, left.strSource, left.strRelativePath) <
				std::tie(right.strDomain, right.strSource, right.strRelativePath);
		});

	m_strDebugResourceStatus = "Indexed " +
		std::to_string(m_DebugResourceFiles.size()) +
		" files from the active Resources and Data roots.";
	if (m_DebugResourceFiles.size() >= MAX_RESOURCE_FILES)
		m_strDebugResourceStatus += " Scan stopped at the 50,000-file safety limit.";
}

void CMainApp::OpenDebugResourceFile(const size_t iFile)
{
	if (iFile >= m_DebugResourceFiles.size())
		return;
	const DEBUG_RESOURCE_FILE& file = m_DebugResourceFiles[iFile];
	if (FAILED(EnsureDebugTool(file.eTool)))
	{
		m_strToolStatus = "Resource selected, but its domain tool failed to initialize: " +
			file.strRelativePath;
		return;
	}

	/* Boss and pattern rows are a joined domain: Workbench owns the lanes and
	   Valtan Boss Tool owns the Server command. Opening both is intentional and does
	   not duplicate either runtime. */
	bool_t bValtanWorkspaceOpened = false;
	if ("Boss / Pattern" == file.strDomain)
	{
		(void)EnsureDebugTool(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH);
		if (nullptr != m_pValtanActionWorkbench &&
			std::string::npos != file.strRelativePath.find("Valtan"))
		{
			bValtanWorkspaceOpened =
				m_pValtanActionWorkbench->Open_Valtan();
		}
	}

	/* A KoukuSaydon action/reference file names a concrete authoring profile, not just
	   the broad Animation domain. Hand that stable profile to Workbench so the
	   matching physical WModel and action document open together. Other KoukuSaydon
	   resources continue to open only their existing owner Tool. */
	const bool_t isKoukuSaydonAnimationPath =
		0u == file.strRelativePath.rfind(
			"Data/Animation/Reference/KoukuSaydon/", 0u) ||
		0u == file.strRelativePath.rfind(
			"Data/Animation/Authored/KoukuSaydon/", 0u) ||
		0u == file.strRelativePath.rfind(
			"Resources/Character/KoukuSaton/", 0u);
	const char_t* pKoukuSaydonProfile = nullptr;
	if (isKoukuSaydonAnimationPath)
	{
		constexpr const char_t* profiles[] =
		{
			"MN_RPCT_00", "MN_RPCT_05", "MN_RPCT_06",
			"MN_RPCT_07", "MN_RPCZ_00"
		};
		for (const char_t* pProfile : profiles)
		{
			if (std::string::npos != file.strRelativePath.find(pProfile))
			{
				pKoukuSaydonProfile = pProfile;
				break;
			}
		}
	}
	bool_t bKoukuSaydonProfileOpened = false;
	if (nullptr != pKoukuSaydonProfile &&
		SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::ANIMATION)) &&
		nullptr != m_pAnimationTool)
	{
		bKoukuSaydonProfileOpened =
			m_pAnimationTool->Open_KoukuSaydonProfile(pKoukuSaydonProfile);
	}
	m_iSelectedDebugResourceFile = iFile;
	if (nullptr != pKoukuSaydonProfile)
	{
		m_strToolStatus = bKoukuSaydonProfileOpened ?
			("Selected " + file.strRelativePath +
				 ". Workbench opened the exact KoukuSaydon profile as Local Extracted Action Preview.") :
			("Selected " + file.strRelativePath +
				 ", but the KoukuSaydon profile preview could not open. Enter Development and preserve any unsaved draft.");
		return;
	}
	m_strToolStatus = bValtanWorkspaceOpened ?
		("Read-only raw path selected: " + file.strRelativePath +
			". Opened the canonical Valtan Pattern Workbench; select a semantic row there to edit Detail.") :
		("Read-only raw path selected: " + file.strRelativePath +
			". Opened its domain owner only; this arbitrary file was not loaded or presented as an editable canonical document.");
}

void CMainApp::RenderDebugResourceFiles()
{
	ImGui::SeparatorText("Authoring Sources");
	ImGui::TextDisabled(
		"Canonical owner entry points only. Detailed Pattern, Sequence, Effect, Sound and Resource rows load inside the selected Tool, never in F1.");
	if (!m_bDebugAuthoringSourceRefreshAttempted)
		RefreshDebugAuthoringSources();
	if (ImGui::SmallButton("Refresh Canonical Sources"))
		RefreshDebugAuthoringSources();
	ImGui::SameLine();
	ImGui::TextDisabled("%zu meaningful entry points", m_DebugAuthoringSources.size());

	if (ImGui::BeginChild(
		"CanonicalAuthoringSources", ImVec2(0.f, 300.f), true))
	{
		if (ImGui::BeginTable(
			"CanonicalAuthoringSourceTable", 3,
			ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
			ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("Workbench", ImGuiTableColumnFlags_WidthFixed, 185.f);
			ImGui::TableSetupColumn("Canonical source / role", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("State / action", ImGuiTableColumnFlags_WidthFixed, 155.f);
			ImGui::TableHeadersRow();
			for (size_t iSource = 0u;
				iSource < m_DebugAuthoringSources.size(); ++iSource)
			{
				const DEBUG_AUTHORING_SOURCE& source =
					m_DebugAuthoringSources[iSource];
				ImGui::PushID(static_cast<int32_t>(iSource));
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(source.strDisplayName.c_str());
				ImGui::TextDisabled("%s", source.strDomain.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped("%s", source.strCanonicalPath.c_str());
				ImGui::TextDisabled(
					"%zu canonical owner document%s",
					source.iDocumentCount,
					1u == source.iDocumentCount ? "" : "s");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", source.strDescription.c_str());
				ImGui::TableSetColumnIndex(2);
				if (source.bCanonicalSourceReady)
					ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.45f, 1.f), "READY");
				else
					ImGui::TextColored(ImVec4(0.95f, 0.35f, 0.3f, 1.f), "MISSING");
				ImGui::BeginDisabled(!source.bCanonicalSourceReady);
				if (ImGui::SmallButton(source.strActionLabel.c_str()))
					OpenDebugAuthoringSource(iSource);
				ImGui::EndDisabled();
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
	ImGui::TextDisabled(
		"F1 does not build a raw physical-file index. Open the owning Tool to enumerate and edit its admitted resources.");
}

void CMainApp::RefreshCompletePlayPatternOptions()
{
	if (nullptr == m_pBalanceTool)
		m_pBalanceTool = make_unique<CBalanceTool>();
	if (nullptr == m_pValtanBossTool)
	{
		m_pValtanBossTool = make_unique<CValtanBossTool>(
			make_shared<CNetworkPlayerCommandSink>(),
			m_pBalanceTool.get());
	}
	m_bCompletePlayPatternLoadAttempted = true;
	std::vector<CValtanBossTool::SERVER_PATTERN_OPTION> options;
	if (!m_pValtanBossTool->Get_ServerPatternOptions(
			options, m_strCompletePlayStatus))
	{
		return;
	}
	const std::string previous = m_strCompletePlayPatternId;
	m_CompletePlayPatternIds.clear();
	m_CompletePlayPatternLabels.clear();
	m_CompletePlayPatternIds.reserve(options.size());
	m_CompletePlayPatternLabels.reserve(options.size());
	for (const CValtanBossTool::SERVER_PATTERN_OPTION& option : options)
	{
		m_CompletePlayPatternIds.push_back(option.strPatternId);
		m_CompletePlayPatternLabels.push_back(
			option.strPatternId + " | " + option.strDisplayName);
	}
	m_strCompletePlayPatternId.clear();
	if (!m_CompletePlayPatternIds.empty())
	{
		const auto found = std::find(
			m_CompletePlayPatternIds.begin(),
			m_CompletePlayPatternIds.end(), previous);
		if (m_CompletePlayPatternIds.end() != found)
			m_strCompletePlayPatternId = *found;
		else
			m_strCompletePlayPatternId = m_CompletePlayPatternIds.front();
	}
}

bool_t CMainApp::Debug_SelectCompletePlayPattern(
	const std::string& strPatternId)
{
	if (strPatternId.empty())
	{
		m_strCompletePlayStatus =
			"Complete Play selection requires a stable pattern ID.";
		return false;
	}
	if (!m_bCompletePlayPatternLoadAttempted)
		RefreshCompletePlayPatternOptions();
	auto found = std::find(
		m_CompletePlayPatternIds.begin(),
		m_CompletePlayPatternIds.end(), strPatternId);
	if (m_CompletePlayPatternIds.end() == found)
	{
		/* A domain owner may have refreshed its joined graph before the workspace
		   inventory.  One bounded reload on the selection edge closes that skew. */
		RefreshCompletePlayPatternOptions();
		found = std::find(
			m_CompletePlayPatternIds.begin(),
			m_CompletePlayPatternIds.end(), strPatternId);
	}
	if (m_CompletePlayPatternIds.end() == found)
	{
		m_strCompletePlayStatus =
			"Pattern is not in the current Server-admitted Complete Play inventory: " +
			strPatternId + ".";
		return false;
	}
	m_strCompletePlayPatternId = *found;
	m_strCompletePlayStatus =
		"Complete Play selection: " + m_strCompletePlayPatternId + ".";
	return true;
}

const std::string& CMainApp::Debug_GetSelectedCompletePlayPatternId() const
{
	return m_strCompletePlayPatternId;
}

bool_t CMainApp::Debug_CompletePlaySelected(std::string& strOutStatus)
{
	if (!m_bCompletePlayPatternLoadAttempted)
		RefreshCompletePlayPatternOptions();
	if (nullptr == m_pValtanBossTool || m_strCompletePlayPatternId.empty() ||
		m_CompletePlayPatternIds.end() == std::find(
			m_CompletePlayPatternIds.begin(),
			m_CompletePlayPatternIds.end(), m_strCompletePlayPatternId))
	{
		strOutStatus =
			"Complete Play requires one Server-admitted saved pattern selection.";
		m_strCompletePlayStatus = strOutStatus;
		return false;
	}
	const bool_t submitted = m_pValtanBossTool->Play_ServerPattern(
		m_strCompletePlayPatternId,
		strOutStatus);
	m_strCompletePlayStatus = strOutStatus;
	if (submitted)
	{
		m_bCompletePlayStatusTracking = true;
		m_strCompletePlayTrackedPatternId = m_strCompletePlayPatternId;
	}
	return submitted;
}

bool_t CMainApp::Debug_OpenValtanPatternFlow(std::string& strOutStatus)
{
	if (FAILED(EnsureDebugTool(DEBUG_TOOL::VALTAN_BOSS)) || nullptr == m_pValtanBossTool)
	{
		strOutStatus = "Pattern Flow owner could not be opened.";
		return false;
	}
	m_pValtanBossTool->Open_PatternFlow();
	strOutStatus =
		"Opened the canonical Boss Pattern Flow owner. Save/Apply and playback remain in that typed owner.";
	return true;
}

void CMainApp::RenderCompletePlayControls()
{
	if (!ImGui::CollapsingHeader(
		"Valtan Complete Play (Server Boss Replay)"))
	{
		return;
	}
	ImGui::TextDisabled(
		"Shared by every open tool: semantic pattern ID -> Server stages/hits -> replicated animation, Effect, Sound, camera and world events.");
	ImGui::TextDisabled(
		"A raw clip or unsaved/unbound asset remains Local Asset Preview and cannot become Complete Play.");
	ImGui::TextDisabled(
		"Complete Play resets boss-owned replay state only; the current replicated arena walls, floors, debris, collision, and Nav state are preserved.");
	if (m_bCompletePlayStatusTracking && nullptr != m_pValtanBossTool)
	{
		std::string serverStatus;
		bool_t inFlight = false;
		if (m_pValtanBossTool->Get_ServerPatternStatus(
				m_strCompletePlayTrackedPatternId, serverStatus, inFlight))
		{
			m_strCompletePlayStatus = std::move(serverStatus);
			m_bCompletePlayStatusTracking = inFlight;
		}
	}
	if (ImGui::SmallButton(
		m_bCompletePlayPatternLoadAttempted ?
			"Reload Complete Play Inventory" :
			"Load Complete Play Inventory"))
	{
		RefreshCompletePlayPatternOptions();
	}
	if (!m_bCompletePlayPatternLoadAttempted)
	{
		ImGui::TextDisabled(
			"Inventory is loaded only on request so opening F1 never parses the canonical Pattern graph.");
		return;
	}

	if (!m_CompletePlayPatternIds.empty())
	{
		ImGui::Text("Saved Patterns (%zu)", m_CompletePlayPatternIds.size());
		if (ImGui::BeginChild(
			"CompletePlayPatternInventory", ImVec2(0.f, 220.f), true))
		{
			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int32_t>(
				m_CompletePlayPatternIds.size()));
			while (clipper.Step())
			{
				for (int32_t iPattern = clipper.DisplayStart;
					iPattern < clipper.DisplayEnd; ++iPattern)
				{
					if (ImGui::Selectable(
						m_CompletePlayPatternLabels[
							static_cast<size_t>(iPattern)].c_str(),
						m_CompletePlayPatternIds[
							static_cast<size_t>(iPattern)] ==
							m_strCompletePlayPatternId))
					{
						(void)Debug_SelectCompletePlayPattern(
							m_CompletePlayPatternIds[
								static_cast<size_t>(iPattern)]);
					}
				}
			}
		}
		ImGui::EndChild();
	}
	const bool_t isValtanArena = ETOUI(LEVEL::VALTAN_ARENA) ==
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t canCompletePlay = nullptr != m_pValtanBossTool &&
		!m_strCompletePlayPatternId.empty() && isValtanArena;
	ImGui::BeginDisabled(!canCompletePlay);
	if (ImGui::Button("Complete Play##GlobalServerPattern"))
	{
		(void)Debug_CompletePlaySelected(m_strCompletePlayStatus);
	}
	ImGui::EndDisabled();
	if (!canCompletePlay)
	{
		ImGui::TextDisabled(
			"Complete Play requires a loaded Pattern selection and Lobby -> Valtan Server admission.");
	}
	else
	{
		ImGui::TextDisabled(
			"The exact Product revision and Sound source generation are validated once when Complete Play is pressed, not once per rendered frame.");
	}
	ImGui::TextWrapped("%s", m_strCompletePlayStatus.c_str());
}

void CMainApp::RenderServerArenaActiveControls()
{
	if (!ImGui::CollapsingHeader("Server Arena Active"))
	{
		return;
	}
	ImGui::TextDisabled(
		"Shared control for Effect Resource, Boss, Map, UI and Workbench. Values are replicated Server actual state.");
	if (!ImGui::BeginTabBar("##ServerArenaActiveTabs"))
		return;

	if (ImGui::BeginTabItem("Valtan"))
	{
		if (ETOUI(LEVEL::VALTAN_ARENA) !=
			CGameInstance::Get().Get_CurrentLevelID())
		{
			ImGui::TextDisabled(
				"Enter Valtan from Lobby. No local wall, debris, collision or NavCell fallback exists.");
		}
		else
		{
			if (nullptr == m_pBalanceTool)
				m_pBalanceTool = make_unique<CBalanceTool>();
			if (nullptr == m_pValtanBossTool)
			{
				m_pValtanBossTool = make_unique<CValtanBossTool>(
					make_shared<CNetworkPlayerCommandSink>(),
					m_pBalanceTool.get());
			}

			CValtanBossTool::VALTAN_ARENA_ACTIVE_STATE state{};
			std::string readStatus;
			const bool_t ready = m_pValtanBossTool->Get_ServerArenaActiveState(
				state, readStatus);
			if (m_bServerArenaPresetStatusTracking)
			{
				m_strServerArenaActiveStatus =
					m_pValtanBossTool->Get_ServerArenaPresetStatus();
				m_bServerArenaPresetStatusTracking =
					m_pValtanBossTool->Is_ServerArenaPresetPending();
			}
			const auto actualCheckbox = [](
				const char_t* label, const bool_t actual)
			{
				bool_t value = actual;
				ImGui::BeginDisabled(true);
				ImGui::Checkbox(label, &value);
				ImGui::EndDisabled();
			};
			actualCheckbox(
				"Ordinary walls / debris sources Active##GlobalArena",
				state.bOrdinaryWallsActive);
			actualCheckbox(
				"109 outer ring Active##GlobalArena",
				state.bOuterRingActive);
			actualCheckbox(
				"3 o'clock floor / collision / Nav Active##GlobalArena",
				state.bThreeOClockFloorActive);
			actualCheckbox(
				"9 o'clock floor / collision / Nav Active##GlobalArena",
				state.bNineOClockFloorActive);
			ImGui::TextDisabled(
				"Active boxes are replicated facts. Arena mutations use exact Server presets because the encounter does not admit arbitrary wall/floor combinations.");
			const auto presetButton = [this, ready](
				const char_t* label,
				const LostArk::Shared::VALTAN_ARENA_PRESET preset)
			{
				ImGui::BeginDisabled(!ready);
				if (ImGui::SmallButton(label))
				{
					std::string submitStatus;
					const bool_t submitted =
						m_pValtanBossTool->Set_ServerArenaPreset(
							preset, submitStatus);
					m_strServerArenaActiveStatus = std::move(submitStatus);
					if (submitted)
						m_bServerArenaPresetStatusTracking = true;
				}
				ImGui::EndDisabled();
			};
			presetButton("Fresh / Restore Entire Arena##GlobalArenaPreset",
				LostArk::Shared::VALTAN_ARENA_PRESET::FRESH);
			ImGui::SameLine();
			presetButton("Circle / Remove All Walls##GlobalArenaPreset",
				LostArk::Shared::VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE);
			presetButton("Break 3 O'Clock Floor##GlobalArenaPreset",
				LostArk::Shared::VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN);
			ImGui::SameLine();
			presetButton("Break 9 O'Clock Floor##GlobalArenaPreset",
				LostArk::Shared::VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN);
			ImGui::SameLine();
			presetButton("Final / Break 3 + 9 O'Clock Floors##GlobalArenaPreset",
				LostArk::Shared::VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN);
			ImGui::Text(
				"Debris actors %u | active collision %u | active nav regions %u | nav revision %llu",
				state.iDebrisActorCount,
				state.iActiveCollisionCount,
				state.iActiveNavigationRegionCount,
				static_cast<unsigned long long>(state.iNavigationRevision));
			if (!readStatus.empty())
				ImGui::TextDisabled("%s", readStatus.c_str());
			if (!m_strServerArenaActiveStatus.empty())
				ImGui::TextWrapped("%s", m_strServerArenaActiveStatus.c_str());
		}
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("KoukuSaydon"))
	{
		if (ETOUI(LEVEL::KAKULSAYDON_ARENA) !=
			CGameInstance::Get().Get_CurrentLevelID())
		{
			ImGui::TextDisabled(
				"Enter KoukuSaydon Arena through Server admission. Stage controls never move a local Character directly.");
		}
		else if (CLevel_KakulSaydonArena* pKakulArena =
			CLevel_KakulSaydonArena::Get_Active())
		{
			const auto& stageMarkers = pKakulArena->Get_StageMarkers();
			ImGui::Text(
				"Runtime-projected stages (%zu)", stageMarkers.size());
			ImGui::TextDisabled(
				"SL identities come from the published StageMarkers document; no Mario/gate meaning or local coordinate is inferred.");
			if (stageMarkers.empty())
			{
				ImGui::TextDisabled(
					"No admitted KoukuSaydon StageMarkers are available in this Level instance.");
			}
			for (const auto& marker : stageMarkers)
			{
				std::string stageLabel = marker.strSourceLevelId;
				const size_t separator = stageLabel.find_last_of('_');
				if (std::string::npos != separator &&
					separator + 1u < stageLabel.size())
				{
					stageLabel = stageLabel.substr(separator + 1u);
				}
				const std::string buttonLabel = stageLabel +
					"##KakulStageTeleport_" + marker.strPlacementId;
				const bool_t sequenceExhausted =
					0u == m_iNextKakulStageTeleportRequestSequence;
				ImGui::BeginDisabled(sequenceExhausted);
				if (ImGui::SmallButton(buttonLabel.c_str()))
				{
					const std::uint32_t requestSequence =
						m_iNextKakulStageTeleportRequestSequence;
					if ((std::numeric_limits<std::uint32_t>::max)() ==
						m_iNextKakulStageTeleportRequestSequence)
					{
						m_iNextKakulStageTeleportRequestSequence = 0u;
					}
					else
					{
						++m_iNextKakulStageTeleportRequestSequence;
					}
					(void)pKakulArena->Request_StageTeleport(
						requestSequence, marker.strPlacementId,
						m_strKakulStageTeleportStatus);
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::TextWrapped(
					"%s | %s",
					marker.strDisplayNameKo.c_str(),
					marker.strPlacementId.c_str());
			}
			if (0u == m_iNextKakulStageTeleportRequestSequence)
			{
				m_strKakulStageTeleportStatus =
					"KoukuSaydon stage teleport request sequence is exhausted; restart the Client before another request.";
			}
		}
		else
		{
			ImGui::TextDisabled(
				"KoukuSaydon Arena is selected, but its active Level instance is unavailable.");
		}
		ImGui::TextWrapped("%s", m_strKakulStageTeleportStatus.c_str());
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}

void CMainApp::RenderDeveloperTools()
{
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	ImVec2 vDefaultSize(720.f, 760.f);
	if (nullptr != pViewport)
	{
		const ImVec2 vFirstUseAvailableSize(
			(std::max)(320.f, pViewport->WorkSize.x - 48.f),
			(std::max)(240.f, pViewport->WorkSize.y - 48.f));
		vDefaultSize = ImVec2(
			(std::min)(vDefaultSize.x, vFirstUseAvailableSize.x),
			(std::min)(vDefaultSize.y, vFirstUseAvailableSize.y));
		ImGui::SetNextWindowPos(
			ImVec2(pViewport->WorkPos.x + 24.f, pViewport->WorkPos.y + 24.f),
			ImGuiCond_FirstUseEver);
	}
	ImGui::SetNextWindowSize(vDefaultSize, ImGuiCond_FirstUseEver);

	/* Only the first-use size is suggested. No minimum or maximum constraint is
	   installed: ImGui's normal edge/corner resizing and saved layout own every
	   subsequent size, including windows larger than the current viewport. */
	if (!ImGui::Begin(
		"LostArk Developer Tools###LostArkDeveloperToolsResizableV1",
		&m_bDeveloperToolsVisible))
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
		const bool_t bVisible = IsDebugToolVisible(eTool);
		const std::string label =
			std::string(bVisible ? "Hide " : "Open ") + pLabel;
		if (ImGui::Button(
				label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0.f)))
		{
			if (bVisible)
			{
				SetDebugToolVisible(eTool, false);
				m_strToolStatus = std::string(pLabel) +
					" hidden; its domain draft was not discarded.";
			}
			else
			{
				m_strToolStatus = SUCCEEDED(EnsureDebugTool(eTool)) ?
					std::string(pLabel) + " opened alongside existing tools." :
					std::string(pLabel) + " initialization failed.";
			}
		}
		ImGui::EndDisabled();
	};

	const float fToolButtonWidth = ImGui::GetContentRegionAvail().x;
	const int32_t iToolButtonColumns = fToolButtonWidth >= 1080.f ? 3 :
		(fToolButtonWidth >= 660.f ? 2 : 1);
	if (ImGui::BeginTable(
			"##DeveloperToolLaunchGrid", iToolButtonColumns,
			ImGuiTableFlags_SizingStretchSame))
	{
		const auto toolCell = [&toolButton](
			const char_t* const pLabel, const DEBUG_TOOL eTool)
		{
			ImGui::TableNextColumn();
			toolButton(pLabel, eTool, true);
		};
		toolCell("Valtan Boss Tool", DEBUG_TOOL::VALTAN_BOSS);
		toolCell("KoukuSaydon Boss Tool", DEBUG_TOOL::KOUKU_SAYDON_BOSS);
		toolCell("Valtan Logic Pattern", DEBUG_TOOL::VALTAN_LOGIC_PATTERN);
		toolCell("Camera Tool", DEBUG_TOOL::CAMERA);
		toolCell("Action Workbench", DEBUG_TOOL::SEQUENCER);
		toolCell("Animation Clip Tool", DEBUG_TOOL::ANIMATION);
		toolCell("Effect Tool", DEBUG_TOOL::EFFECT);
		toolCell("Map Tool", DEBUG_TOOL::MAP);
		toolCell("Rendering Workbench", DEBUG_TOOL::RENDERING);
		toolCell("Profiler", DEBUG_TOOL::PROFILER);
		toolCell("HUD Layout Tool", DEBUG_TOOL::UI);
		toolCell("Balance Tool", DEBUG_TOOL::BALANCE);
		toolCell("Equipment Authoring Tool", DEBUG_TOOL::EQUIPMENT);
		ImGui::EndTable();
	}
	if (ImGui::Button("Close All Tools"))
		CloseAllDebugTools();
	constexpr std::array<std::pair<DEBUG_TOOL, const char_t*>, 12>
		TOOL_FOCUS_OPTIONS = {{
			{ DEBUG_TOOL::MAP, "Map Tool" },
			{ DEBUG_TOOL::SEQUENCER, "Action Workbench" },
			{ DEBUG_TOOL::ANIMATION, "Animation Clip Tool" },
			{ DEBUG_TOOL::EFFECT, "Effect Tool" },
			{ DEBUG_TOOL::RENDERING, "Rendering Workbench" },
			{ DEBUG_TOOL::PROFILER, "Profiler" },
			{ DEBUG_TOOL::UI, "HUD Layout Tool" },
			{ DEBUG_TOOL::BALANCE, "Balance Tool" },
			{ DEBUG_TOOL::VALTAN_BOSS, "Valtan Boss Tool" },
			{ DEBUG_TOOL::KOUKU_SAYDON_BOSS, "KoukuSaydon Boss Tool" },
			{ DEBUG_TOOL::CAMERA, "Camera Tool" },
			{ DEBUG_TOOL::EQUIPMENT, "Equipment Authoring Tool" },
		}};
	const char_t* pInputOwnerLabel = "None";
	for (const auto& [eTool, pLabel] : TOOL_FOCUS_OPTIONS)
	{
		if (eTool == m_eDebugInputOwner)
		{
			pInputOwnerLabel = pLabel;
			break;
		}
	}
	ImGui::SetNextItemWidth(310.f);
	if (ImGui::BeginCombo("Explicit viewport/preview owner", pInputOwnerLabel))
	{
		for (const auto& [eTool, pLabel] : TOOL_FOCUS_OPTIONS)
		{
			if (!IsDebugToolVisible(eTool))
				continue;
			const bool_t isSelected = eTool == m_eDebugInputOwner;
			if (ImGui::Selectable(pLabel, isSelected))
			{
				m_eDebugInputOwner = eTool;
				m_eDebugWindowFocusPending = eTool;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::TextWrapped("%s", m_strToolStatus.c_str());
	if (!isMapEditorWorkspace && IsDebugToolVisible(DEBUG_TOOL::MAP))
	{
		ImGui::TextDisabled(
			"Map Tool is open in inspect-only mode. Enter Lobby > Test > Map Editor to save map placement/navigation.");
	}

	RenderDebugLevelNavigation();
	RenderArenaCameraAndPlayerControls();
	RenderDebugResourceFiles();
	RenderCompletePlayControls();
	RenderServerArenaActiveControls();

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
	ImGui::SeparatorText("Live Combat Geometry");
	Client::COMBAT_DEBUG_VISIBILITY_SNAPSHOT CombatDebug =
		Client::CClientReplication::Get_GlobalCombatDebugVisibility();
	bool_t bCombatDebugChanged = false;
	if (ImGui::Checkbox(
			"Boss Body Collider", &CombatDebug.bBossBodyCollider))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Boss Pattern Hit Pulse", &CombatDebug.bBossPatternHitPulse))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Boss Stage Geometry (whole Stage)",
			&CombatDebug.bBossStageGeometry))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Combat Object Hit", &CombatDebug.bCombatObjectHit))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Counter Proxy", &CombatDebug.bCounterProxy))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Player Skill Hit Geometry",
			&CombatDebug.bPlayerSkillHitGeometry))
	{
		bCombatDebugChanged = true;
	}
	if (bCombatDebugChanged)
	{
		Client::CClientReplication::Set_GlobalCombatDebugVisibility(
			CombatDebug);
		CombatDebug =
			Client::CClientReplication::Get_GlobalCombatDebugVisibility();
	}
	ImGui::TextDisabled(
		"Global revision %llu. Pink=pulse, amber=Stage, green=combat object, cyan=counter. Authoring preview remains independent.",
		static_cast<unsigned long long>(CombatDebug.iRevision));
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
		ImGui::TextDisabled(
			"Replays the full-screen cutin movie (NpcCatalog cutinMovie) once.");
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
	}

	if (ImGui::CollapsingHeader("Raid Boss Showcase (Debug)"))
	{
		RAID_BOSS_SHOWCASE_TUNING& showcaseTuning =
			CRaidBossShowcaseService::Debug_Tuning();
		ImGui::DragFloat("Yaw (deg)##Showcase",
			&showcaseTuning.fModelYawDegrees, 1.f, -360.f, 360.f);
		ImGui::DragFloat("Eye X / Height##Showcase",
			&showcaseTuning.fEyeXPerHeight, 0.01f, -2.f, 2.f);
		ImGui::DragFloat("Eye Y / Height##Showcase",
			&showcaseTuning.fEyeYPerHeight, 0.01f, -1.f, 3.f);
		ImGui::DragFloat("Distance / Height##Showcase",
			&showcaseTuning.fDistancePerHeight, 0.02f, 0.3f, 6.f);
		ImGui::DragFloat("Target Y / Height##Showcase",
			&showcaseTuning.fAtYPerHeight, 0.01f, 0.f, 2.f);
		ImGui::DragFloat("FOV (deg)##Showcase",
			&showcaseTuning.fFovDegrees, 0.5f, 10.f, 90.f);
		ImGui::DragFloat4("Rect X/Y/W/H (720p)##Showcase",
			&showcaseTuning.fRectX, 2.f, -400.f, 1600.f);
		if (ImGui::Button("Reset Tuning##Showcase"))
			CRaidBossShowcaseService::Debug_ResetTuning();
		ImGui::TextDisabled(
			"Open the raid entry popup on its Valtan tab to see the live"
			" boss; values apply immediately.");
		ImGui::Text(
			"yaw %.1f  eye(%.2f, %.2f)  dist %.2f  target %.2f  fov %.1f\n"
			"rect (%.0f, %.0f, %.0f, %.0f)",
			showcaseTuning.fModelYawDegrees,
			showcaseTuning.fEyeXPerHeight,
			showcaseTuning.fEyeYPerHeight,
			showcaseTuning.fDistancePerHeight,
			showcaseTuning.fAtYPerHeight,
			showcaseTuning.fFovDegrees,
			showcaseTuning.fRectX,
			showcaseTuning.fRectY,
			showcaseTuning.fRectWidth,
			showcaseTuning.fRectHeight);
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
	if (nullptr != m_pRenderingBenchmark)
	{
		const RENDER_QUALITY_SETTINGS& Quality =
			m_RenderingProfiles.Get_GlobalQuality();
		const string strQualitySummary = "profile=" + m_strRenderingDraftProfileId +
			" ssao=" + (Quality.bSSAOEnabled ? "on" : "off") +
			" bloom=" + (Quality.bBloomEnabled ? "on" : "off") +
			" fxaa=" + (Quality.bFXAAEnabled ? "on" : "off") +
			" shadow=" + (pActiveProfile->ShadowSettings.bEnabled ? "on" : "off") +
			" fog=" + (pActiveProfile->Fog.bEnabled ? "on" : "off") +
			" exposure=" + std::to_string(Quality.fExposure);
		m_pRenderingBenchmark->Render_Section(
			CGameInstance::Get().Get_Profiler(), strQualitySummary);
	}

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

	ImGui::SeparatorText("Height Fog");
	sceneChanged |= ImGui::Checkbox(
		"Height Fog Enabled", &m_SceneRenderingDraft.Fog.bEnabled);
	ImGui::BeginDisabled(!m_SceneRenderingDraft.Fog.bEnabled);
	sceneChanged |= ImGui::ColorEdit3(
		"Fog Color", &m_SceneRenderingDraft.Fog.vColor.x);
	sceneChanged |= ImGui::DragFloat(
		"Fog Top Height", &m_SceneRenderingDraft.Fog.fTopHeight,
		0.25f, -10000.f, 10000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Height Falloff", &m_SceneRenderingDraft.Fog.fHeightFalloff,
		0.002f, 0.0001f, 4.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Density", &m_SceneRenderingDraft.Fog.fDensity,
		0.01f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Start Distance", &m_SceneRenderingDraft.Fog.fStartDistance,
		0.25f, 0.f, 100000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Maximum Opacity", &m_SceneRenderingDraft.Fog.fMaximumOpacity,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Drift Speed", &m_SceneRenderingDraft.Fog.fDriftSpeed,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Drift Height", &m_SceneRenderingDraft.Fog.fDriftHeightAmplitude,
		0.05f, 0.f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Fog Drift Density", &m_SceneRenderingDraft.Fog.fDriftDensityAmplitude,
		0.005f, 0.f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

	ImGui::SeparatorText("Cloud Banks");
	/* The authored value is a fraction; the slider speaks percent because that
	   is how the map coverage is judged by eye. */
	f32_t fFogCoveragePercent =
		m_SceneRenderingDraft.Fog.fCoveragePercent * 100.f;
	if (ImGui::DragFloat("Map Coverage", &fFogCoveragePercent,
		0.5f, 0.f, 100.f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp))
	{
		m_SceneRenderingDraft.Fog.fCoveragePercent =
			fFogCoveragePercent * 0.01f;
		sceneChanged = true;
	}
	sceneChanged |= ImGui::DragFloat(
		"Wind Direction X", &m_SceneRenderingDraft.Fog.fWindDirectionX,
		0.01f, -1.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Wind Direction Z", &m_SceneRenderingDraft.Fog.fWindDirectionZ,
		0.01f, -1.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Wind Speed", &m_SceneRenderingDraft.Fog.fWindSpeed,
		0.05f, 0.f, 200.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Patch Scale", &m_SceneRenderingDraft.Fog.fPatchScale,
		0.0005f, 0.0001f, 1.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	sceneChanged |= ImGui::DragFloat(
		"Patch Softness", &m_SceneRenderingDraft.Fog.fPatchSoftness,
		0.005f, 0.001f, 0.5f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::TextDisabled(
		"Coverage 100%% is one blanket. Lower it and the fog breaks into banks that the wind walks across world XZ; Patch Scale sets their size.");
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Fog fills below Top Height and is applied in the deferred combine, so effects and the blend group stay clear of it.");
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
	/* Active instances must leave ObjectManager while the Engine is alive, but
	   prepared renderer/catalog globals cannot be cleared until a Loading level
	   has cancelled and joined its worker.  Release_Engine tears the current
	   level down first; retained COM references keep the device valid until the
	   post-join cache release below. */
	CEffectPresentationService::Clear_All();
	CNetworkManager::Get().Shutdown();
	CGameInstance::Get().Stop_LoopingSound();
	CGameInstance::Get().SetInputBlocked(false, false);

#ifdef _DEBUG
	if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
		pProfiler->Set_Enabled(false);
	m_pSequencerTool.reset();
	m_pProfilerTool.reset();
	m_pRenderingBenchmark.reset();
	m_pKoukuSaydonActionWorkbench.reset();
	m_pValtanActionWorkbench.reset();
	m_pAnimationTool.reset();
	m_pEffectTool.reset();
	m_pEffectToolV2.reset();
	m_pEquipmentAuthoringTool.reset();
	if (nullptr != m_pCharacterPreviewPanel)
		m_pCharacterPreviewPanel->Release(true);
	m_pCharacterPreviewPanel.reset();
	m_pHUDLayoutTool.reset();
	m_pBalanceTool.reset();
	m_pKoukuSaydonBossTool.reset();
	m_pValtanBossTool.reset();
	m_pCameraTool.reset();
	m_pMapTool.reset();
#endif

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->Shutdown();
	m_pImGuiLayer.reset();
	CGameInstance::Get().Release_Engine();
	CEffectPresentationService::Release_PreparedResources();
	CEffectCatalog::Clear();
}
