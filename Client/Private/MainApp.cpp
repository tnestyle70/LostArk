#include <WinSock2.h>
#include "imgui.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")
#include "Engine_InitTypes.h"
#include "Engine_RenderTypes.h"
#include "Engine_VertexTypes.h"

#include "MainApp.h"
#include "PlayableCharacterAssetService.h"
#include "DungeonTimerView.h"
#include "BossImmuneGaugeView.h"

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
#include "EffectV2_Catalog.h"
#include "KoukuSaydonPatternAuditionService.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "Valtan.h"
#include "KoukuSaydonCompositionDocument.h"
#include "PlayerCommandSink.h"
#include "GameInstance.h"
#include "ImGuiLayer.h"
#include "ItemCatalog.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "Level_Bern.h"
#include "Level_Development.h"
#include "Level_Lobby.h"
#include "ActionPresentationTimeline.h"
#include "Level_CharacterSelect.h"
#include "Level_KakulSaydonArena.h"
#include "Level_Loading.h"
#include "Level_ValtanArena.h"
#include "LobbyCommandService.h"
#include "NetworkManager.h"
#include "Npc.h"
#include "PartyWindowView.h"
#include "UITextOcclusion.h"
#include "PlayerSkillCatalog.h"
#include "Profiler.h"
#include "Presentation_Manager.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "UserSettingsDocument.h"
#include "WorldGameplayDocument.h"
#include "WorldSequencePlayer.h"
#include <set>
#include "AvatarBookWindowView.h"
#include "UILabelFont.h"
#include "CharacterInfoWindowView.h"
#include "VehicleWindowView.h"
#include "SystemOptionWindowView.h"
#include "ClientWindowDisplay.h"
#include "CombatAnalysisFrameView.h"
#include "HonorTitleCatalog.h"
#include "HonorTitleWindowView.h"
#include "WorldMapWindowView.h"
#include "SongCastGaugeView.h"
#include "Network/PacketMessages.h"
#include "InventoryView.h"
#include "QuickSlotDragView.h"
#include "SkillWindowView.h"
#include "SkillGroundTargetPreview.h"
#include "ClickMoveEffect.h"
#include "SoundCueCatalog.h"
#include "UI_Sprite.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#ifdef _DEBUG
#include "ActorCatalog.h"
#include "Animation_Tool.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonBossTool.h"
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
#include "MapAssetRenderUtils.h"
#include "MapEditorWorkspaceService.h"
#include "LevelNavigationDebug.h"
#include "MapTool.h"
#include "NetworkPlayerCommandSink.h"
#include "ProfilerCaptureIO.h"
#include "ProfilerTool.h"
#include "RenderingBenchmark.h"
#include "SequencerTool.h"
#include "CharacterActionWorkbench.h"
#include "EffectAuthoringSequencer.h"
#include "WorldObjectTool.h"
#include "WorldLevelTool.h"
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
#include <iomanip>

namespace
{
    void WriteStartupDiagnostic(const char* stage, const HRESULT result,
        const std::string& status)
    {
        // Resolve beside the repository launcher, independent of the process cwd.
        wchar_t modulePath[32768]{};
        const DWORD pathLength = GetModuleFileNameW(nullptr, modulePath,
            static_cast<DWORD>(std::size(modulePath)));
        std::filesystem::path logPath = "ClientStartup.user.log";
        if (pathLength > 0 && pathLength < std::size(modulePath))
            logPath = std::filesystem::path(modulePath).parent_path().parent_path().parent_path() /
                L"Default" / L"ClientStartup.user.log";
        std::ofstream output(logPath, std::ios::binary | std::ios::app);
        if (!output)
            return;
        SYSTEMTIME time{};
        GetLocalTime(&time);
        output << std::setfill('0') << time.wYear << '-'
            << std::setw(2) << time.wMonth << '-' << std::setw(2) << time.wDay << ' '
            << std::setw(2) << time.wHour << ':' << std::setw(2) << time.wMinute << ':'
            << std::setw(2) << time.wSecond << " pid=" << GetCurrentProcessId()
            << " stage=" << stage << " hr=0x" << std::hex << std::uppercase
            << static_cast<unsigned long>(result) << std::dec << " status=" << status << '\n';
    }

	/* The MVP award page is a full-screen modal (retail places it on
	   sortingLayer "3_topmostHUD" with orderInLayer 2000), so the combat HUD,
	   the boss bar and their text passes all stop while it is up. */
	bool_t Is_MvpResultPageOpen()
	{
		const CLevel_KakulSaydonArena* pArena = CLevel_KakulSaydonArena::Get_Active();
		if (nullptr != pArena && pArena->Debug_Is_MvpResultVisible())
			return true;
		const CLevel_ValtanArena* pValtan = CLevel_ValtanArena::Get_Active();
		return nullptr != pValtan && pValtan->Is_MvpResultVisible();
	}

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
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT:
			return "GuardianKnight";
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
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT:
			return "GuardianKnight";
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

void CMainApp::Update_KoukuGateSceneProfile()
{
    auto* arena = CLevel_KakulSaydonArena::Get_Active();
    if (!arena || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::KAKULSAYDON_ARENA))
    {
        // The newly activated Level owns its profile; do not restore across Levels.
        m_strKoukuGateProfileRequest.clear();
        m_strKoukuGateProfileApplied.clear();
        m_strSceneProfileBeforeKoukuGate.clear();
        return;
    }
    const string requested = arena->Get_GatePresentationProfileId();
    if (requested == m_strKoukuGateProfileRequest) return;
    // Consume one gate edge, including a failed request. A Sequence may own a
    // different profile on following frames and must not be overwritten here.
    m_strKoukuGateProfileRequest = requested;
    string status;
    bool changed = false;
    if (!requested.empty())
    {
        const string previous = m_RenderingProfiles.Get_ActiveProfileId();
        changed = m_RenderingProfiles.Activate_Profile(requested, status);
        if (changed)
        {
            if (m_strSceneProfileBeforeKoukuGate.empty())
                m_strSceneProfileBeforeKoukuGate = previous;
            m_strKoukuGateProfileApplied = requested;
        }
    }
    else
    {
        if (!m_strKoukuGateProfileApplied.empty() &&
            m_RenderingProfiles.Get_ActiveProfileId() == m_strKoukuGateProfileApplied &&
            !m_strSceneProfileBeforeKoukuGate.empty())
            changed = m_RenderingProfiles.Activate_Profile(m_strSceneProfileBeforeKoukuGate, status);
        m_strKoukuGateProfileApplied.clear();
        m_strSceneProfileBeforeKoukuGate.clear();
    }
    if (!status.empty())
    {
#ifdef _DEBUG
        m_strRenderingStatus = status;
#endif
        if (!changed)
            OutputDebugStringA(("[MainApp][KoukuGateProfile] " + status + "\n").c_str());
    }
}

void CMainApp::Update_CustomizingSceneProfile()
{
	/* The class list wants the stage readable and character creation wants it
	nearly black behind the panels, which is where the retail screen sits. The
	profile service belongs to this class, so the swap lives here rather than in
	the Level, and it only fires when the screen opens or closes. */
	static constexpr const char_t* CUSTOMIZING_PROFILE_ID =
		"scene.character-select.customizing-dark.v1";
	auto* characterSelect = CLevel_CharacterSelect::Get_Active();
	const bool_t wantsDarkStage =
		nullptr != characterSelect && characterSelect->Is_CustomizingOpen();
	const bool_t holdsDarkStage = !m_strSceneProfileBeforeCustomizing.empty();
	if (wantsDarkStage == holdsDarkStage)
		return;

	string status;
	if (wantsDarkStage)
	{
		const string previous = m_RenderingProfiles.Get_ActiveProfileId();
		if (!m_RenderingProfiles.Activate_Profile(CUSTOMIZING_PROFILE_ID, status))
		{
			/* A missing profile leaves the bright stage up rather than failing the
			screen; nothing else on it depends on the swap. */
			OutputDebugStringA(("[MainApp][SceneProfile] " + status + "\n").c_str());
			return;
		}
		m_strSceneProfileBeforeCustomizing = previous;
		return;
	}

	if (!m_RenderingProfiles.Activate_Profile(
		m_strSceneProfileBeforeCustomizing, status))
	{
		OutputDebugStringA(("[MainApp][SceneProfile] " + status + "\n").c_str());
	}
	m_strSceneProfileBeforeCustomizing.clear();
}

void CMainApp::Update_ItemUpgrade(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.ItemUpgrade.Update");
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
    WriteStartupDiagnostic("Initialize", S_OK, "begin");
    const auto InitializeStage = [](const char* stage, const auto& initialize)
    {
        WriteStartupDiagnostic(stage, S_OK, "begin");
        const HRESULT result = initialize();
        WriteStartupDiagnostic(stage, result, FAILED(result) ? "failed" : "ready");
        return result;
    };
	/* CreateWICTextureFromFile (used by the HUD runtime view for non-DDS art) needs COM on the
	calling thread. The main thread never initializes it otherwise. */
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	ENGINE_DESC engineDesc{};
	engineDesc.hInstance = g_hInst;
	engineDesc.hWnd = g_hWnd;
	engineDesc.eWinMode = WINMODE::WIN;
	engineDesc.iNumLevels = ETOUI(LEVEL::END);
	RECT physicalClient{};
	if (!GetClientRect(g_hWnd, &physicalClient) || physicalClient.right <= 0 || physicalClient.bottom <= 0)
		return E_FAIL;
	engineDesc.iWinSizeX = physicalClient.right;
	engineDesc.iWinSizeY = physicalClient.bottom;

    HRESULT startupResult = InitializeStage("Engine.Initialize", [&]()
    {
        return CGameInstance::Get().Initialize_Engine(engineDesc, m_pDevice, m_pContext);
    });
    if (FAILED(startupResult))
        return startupResult;

	string displayStatus;
	if (!CClientWindowDisplay::Attach_Engine(displayStatus))
		return E_FAIL;
	CUserSettings::Get().Set_DisplayApplyCallback(CClientWindowDisplay::Apply);
	/* Preferences were parsed before window creation; apply audio after Engine startup. */
	CUserSettings::Get().Initialize();

    WriteStartupDiagnostic("Rendering.Load_Runtime", S_OK, "begin");
	string renderingProfileStatus;
	if (!m_RenderingProfiles.Load_Runtime(renderingProfileStatus))
	{
        WriteStartupDiagnostic("Rendering.Load_Runtime", E_FAIL, renderingProfileStatus);
		OutputDebugStringA((
			"[MainApp] Rendering profile initialization failed: " +
			renderingProfileStatus + "\n").c_str());
#ifdef _DEBUG
		MessageBoxA(g_hWnd, renderingProfileStatus.c_str(),
			"Rendering Profile Load Failed", MB_OK | MB_ICONERROR);
#endif
		return E_FAIL;
	}

    WriteStartupDiagnostic("Rendering.Load_Runtime", S_OK, renderingProfileStatus);
	std::string lightStatus;
	if (!m_LightResources.Load_Runtime(lightStatus))
		OutputDebugStringA(("[MainApp] Light resources unavailable: "+lightStatus+"\n").c_str());
	if (!m_LightResources.Refresh_MapResources("LV_LUT_MIDNIGHTC_ED", lightStatus))
		OutputDebugStringA(("[MainApp] Map light resources unavailable: "+lightStatus+"\n").c_str());
    WriteStartupDiagnostic("Network.Initialize", S_OK, "begin");
    if (!CNetworkManager::Get().Initialize())
    {
        const int networkError = CNetworkManager::Get().Get_LastErrorCode();
        const HRESULT networkResult = networkError ? HRESULT_FROM_WIN32(networkError) : E_FAIL;
        WriteStartupDiagnostic("Network.Initialize", networkResult,
            "WSAStartup error=" + std::to_string(networkError));
        return networkResult;
    }
    WriteStartupDiagnostic("Network.Initialize", S_OK, "ready");
    startupResult = InitializeStage("ImGui.Initialize", [&]() { return ReadyImGuiRuntime(); });
    if (FAILED(startupResult))
        return startupResult;

#ifdef _DEBUG
    startupResult = InitializeStage("DebugTools.Initialize", [&]() { return ReadyDebugTools(); });
    if (FAILED(startupResult))
        return startupResult;
#endif

    startupResult = InitializeStage("Fonts.Initialize", [&]() { return Ready_Fonts(); });
    if (FAILED(startupResult))
        return startupResult;
    startupResult = InitializeStage("StaticPrototypes.Initialize", [&]() { return Ready_Prototype_For_Static(); });
    if (FAILED(startupResult))
        return startupResult;

    WriteStartupDiagnostic("EffectCatalog.Load", S_OK, "begin");
	std::string effectCatalogStatus;
	if (!CEffectCatalog::Load(effectCatalogStatus))
	{
        WriteStartupDiagnostic("EffectCatalog.Load", E_FAIL, effectCatalogStatus);
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

    WriteStartupDiagnostic("EffectCatalog.Load", S_OK, effectCatalogStatus);

	// The Loader snapshots skill definitions before starting its worker. Release
	// must not depend on opening an authoring tool to initialize these definitions.
	// Preserve the existing Level/replication failure and recovery boundary.
	if (CPlayerSkillCatalog::Get_Skills().empty())
	{
		Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Catalog.PlayerSkills.Initialize");
		std::string skillCatalogStatus;
		if (!CPlayerSkillCatalog::Load(skillCatalogStatus))
			OutputDebugStringA(("[MainApp] Player Skill Catalog initialization failed: " +
				skillCatalogStatus + "\n").c_str());
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
	/* Same policy: without it nameplates show bare names and the title window is empty. */
	std::string honorTitleStatus;
	if (!Client::CHonorTitleCatalog::Load(honorTitleStatus))
	{
		OutputDebugStringA(("[MainApp] Honor title catalog initialization failed: " + honorTitleStatus + "\n").c_str());
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
    WriteStartupDiagnostic("UI.Initialize", S_OK, "begin");
	m_pHUDRuntimeView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/HUD/HUD_Layout.json");
	Hide_CombatHUD();
	m_pCombatAnalysisView = std::make_unique<CCombatAnalysisFrameView>(m_pDevice, m_pContext);
	Load_KoukuHudModes();
	Load_HudQuickSlotData();
	m_pBossUIView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/BossUI/BossUI.json");

	m_pDungeonTimerView = std::make_unique<CDungeonTimerView>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC));
	/* Authored layer tints are opaque -- every real slot would otherwise sit fully visible from
	this Level::STATIC construction until the first Update_BossHealthBar() call finds a valid
	boss. */
	Hide_BossHealthBar();
	m_pBossImmuneGaugeView = std::make_unique<CBossImmuneGaugeView>(
		m_pDevice, m_pContext, ETOUI(LEVEL::STATIC));
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
	/* Vehicle window (N): a separate panel on the right, drawn over the windows above. */
	m_pVehicleWindowView = std::make_unique<CVehicleWindowView>(m_pDevice, m_pContext);
	/* Honor title window: opened from the character info window, drawn over it. */
	m_pHonorTitleWindowView = std::make_unique<CHonorTitleWindowView>(m_pDevice, m_pContext);
	/* World map window (M): a large centred panel, drawn over the windows above. */
	m_pWorldMapWindowView = std::make_unique<CWorldMapWindowView>(m_pDevice, m_pContext);
	m_pSongCastGaugeView = std::make_unique<CSongCastGaugeView>(m_pDevice, m_pContext);
	/* System option window (Escape): a centred panel over the windows above. */
	m_pSystemOptionView = std::make_unique<CSystemOptionWindowView>(m_pDevice, m_pContext);
	/* Last of all: the carried quick-slot icon must ride over every window above. */
	m_pQuickSlotDragView = std::make_unique<CQuickSlotDragView>(m_pDevice, m_pContext);

    WriteStartupDiagnostic("UI.Initialize", S_OK, "ready");
    WriteStartupDiagnostic("Lobby.Start_Level", S_OK, "begin");
    startupResult = Start_Level(LEVEL::LOBBY);
    WriteStartupDiagnostic("Lobby.Start_Level", startupResult,
        FAILED(startupResult) ? CLevelTransitionService::Get_Status() : "ready");
    if (FAILED(startupResult))
        return startupResult;
    WriteStartupDiagnostic("Initialize", S_OK, "ready");
	return S_OK;
}


#ifdef _DEBUG
namespace
{
 void Refresh_KoukuPresentationResources(const std::vector<CKoukuSaydonActionWorkbench*>& workbenches,
  CRenderingProfileService& profiles, CLightResourceCatalog& lights)
 {
  using Kind = KOUKU_SAYDON_PRESENTATION_KIND;
  std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> rows;
  std::string status;
  std::vector<EFFECT_V2_RESOURCE_SUMMARY> effectRows;
  if (CEffectV2Catalog::Get().Read_Inventory(effectRows, status))
   for (const auto& effect : effectRows)
   {
    KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
    row.strResourceId = row.strAssetId = effect.strResourceId;
    row.strDisplayName = effect.strDisplayName.empty() ? effect.strResourceId : effect.strDisplayName;
    row.eKind = Kind::EFFECT;
    row.strResourceKind = effect.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "GROUP" : "LEAF";
    row.iDurationMs = effect.iDurationMs;
    rows.push_back(std::move(row));
   }
  for (const auto& id : CEffectCatalog::Get_EffectAssetIds())
  {
   if (!CEffectCatalog::Is_DirectAuthoredDocument(id)) continue;
   KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
   row.strResourceId = "v1:" + id;
   row.strAssetId = id;
   row.strDisplayName = id;
   row.eKind = Kind::EFFECT;
   row.strResourceKind = "V1_EFFECT";
   row.iDurationMs = 3000u;
   rows.push_back(std::move(row));
  }
  // This explicit refresh inventories physical Sound assets once. Playback
  // still resolves every Resources-relative ID through the existing audio owner.
  std::error_code ec;
  const auto root = CRuntimeAssetRoot::Get_ResourceRoot();
  const auto sounds = CRuntimeAssetRoot::Resolve(L"Sound");
  if (!sounds.empty())
   for (std::filesystem::recursive_directory_iterator it(sounds,
      std::filesystem::directory_options::skip_permission_denied, ec), end;
     !ec && it != end; it.increment(ec))
   {
    if (!it->is_regular_file(ec)) continue;
    auto extension = it->path().extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
     [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (extension != ".wav" && extension != ".ogg" && extension != ".mp3") continue;
    const auto id = it->path().lexically_relative(root).generic_string();
    if (CRuntimeAssetRoot::Resolve(id).empty()) continue;
    KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
    row.strResourceId = row.strAssetId = id; row.strDisplayName = id;
    row.eKind = Kind::SOUND; row.iDurationMs = 3000u;
    rows.push_back(std::move(row));
   }
  if (ec) status += " Sound inventory: " + ec.message();
  if (const auto* arena = CLevel_KakulSaydonArena::Get_Active())
   for (const auto& shot : arena->Get_CameraShots())
   {
    KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
    row.strResourceId = row.strAssetId = shot.strShotId;
    row.strDisplayName = shot.strDisplayName.empty() ? shot.strShotId : shot.strDisplayName;
    row.eKind = Kind::CAMERA;
    row.iDurationMs = shot.hasCameraTrack ? shot.CameraTrack.iDurationMs : shot.iBlendInMs + shot.iDefaultHoldMs;
    rows.push_back(std::move(row));
   }
  const auto profileIds = profiles.Collect_ProfileIds();
	(void)lights.Refresh_MapResources("LV_LUT_MIDNIGHTC_ED", status);
	for(const auto& light : lights.Get_Resources())
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
		row.strResourceId=row.strAssetId=light.strLightResourceId;row.strDisplayName=light.strDisplayName;
		row.eKind=Kind::LIGHT;row.strResourceKind.clear();row.iDurationMs=3000u;
		row.strDefaultAnchorKind=light.strDefaultAnchorKind;rows.push_back(std::move(row));
	}
	std::vector<std::string> protectedProfiles;
	for (const auto* workbench : workbenches)
		for (const auto& scene : workbench->Get_Composition().SceneProfiles) protectedProfiles.push_back(scene.strRenderingProfileId);
	profiles.Protect_ProfileIds(protectedProfiles);
  for (auto* workbench : workbenches)
  {
   workbench->Set_RenderingProfileResources(profileIds,
    "Loaded " + std::to_string(profileIds.size()) + " admitted rendering profiles.");
   workbench->Set_PresentationResources(rows, status);
  }
 }

 bool Resolve_KoukuWorldPreviewActor(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
  const CWorldSequenceDocument& sequences, const std::string& selectedPatternId,
  KOUKU_SAYDON_COMPOSITION_PATTERN& resourcePattern, bool& requiresActor, std::string& status)
 {
  requiresActor = false;
  std::string requiredArchetype;
  for (const auto& box : resourcePattern.WorldOccurrences)
  {
   const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
    [&box](const auto& value) { return value.strWorldId == box.strWorldId; });
   const auto* instance = world == document.Worlds.end() ? nullptr : sequences.Find_Instance(world->strSequenceInstanceId);
   if (!instance) { status = "World Object Preview has no saved motion for: " + box.strWorldId; return false; }
   if (instance->anchorKind != "BOSS") continue;
   for (const auto& binding : instance->bindings)
   {
    if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
    const auto* object = sequences.Find_ObjectResource(binding.targetId);
    if (!object || object->anchorBossArchetypeId.empty())
    { status = "World Object Preview has no exact Boss anchor: " + binding.targetId; return false; }
    if (!requiredArchetype.empty() && requiredArchetype != object->anchorBossArchetypeId)
    { status = "World Object Preview contains different Boss anchors; preview their Patterns separately."; return false; }
    requiredArchetype = object->anchorBossArchetypeId;
    requiresActor = true;
   }
  }
  if (!requiresActor) return true;
  const auto selected = std::find_if(document.Patterns.begin(), document.Patterns.end(),
   [&selectedPatternId](const auto& value) { return value.strPatternId == selectedPatternId; });
  if (selected == document.Patterns.end() || !selected->strLoadError.empty() ||
   selected->strActorProfileId.empty() || selected->strGateId.empty() ||
   CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(selected->strTargetBossPlacementId) != requiredArchetype)
  { status = "World Object Preview requires its Boss Pattern to be selected: " + requiredArchetype; return false; }
  resourcePattern.strActorProfileId = selected->strActorProfileId;
  resourcePattern.strGateId = selected->strGateId;
  resourcePattern.strTargetBossPlacementId = selected->strTargetBossPlacementId;
  return true;
 }

 bool Begin_KoukuWorldPreview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
  const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::string& status,
  const CWorldSequenceDocument* sourceDocument = nullptr, const bool previewAtCharacter = false)
 {
  auto* arena = CLevel_KakulSaydonArena::Get_Active();
  if (!arena) { status = "World preview requires the KoukuSaydon Arena."; return false; }
  std::vector<CLevel_KakulSaydonArena::COMPOSITION_WORLD_PREVIEW_CUE> cues;
  CWorldGameplayDocument placements;
  bool loaded = false;
  for (const auto& box : pattern.WorldOccurrences)
  {
   const auto def = std::find_if(document.Worlds.begin(), document.Worlds.end(),
    [&box](const auto& world) { return world.strWorldId == box.strWorldId; });
   if (def == document.Worlds.end()) { status = "World box refers to an unknown resource."; return false; }
   float3_t offset(static_cast<float>(def->PositionOffset[0]),
    static_cast<float>(def->PositionOffset[1]), static_cast<float>(def->PositionOffset[2]));
   if (previewAtCharacter)
   {
    const auto& sequences = sourceDocument ? *sourceDocument : arena->Get_WorldSequenceDocument();
    const auto* instance = sequences.Find_Instance(def->strSequenceInstanceId);
    if (!instance) { status = "Resource preview has no saved default motion."; return false; }
    if (instance->anchorKind == "WORLD")
    {
     float3_t baseline, position;
     if (!arena->Try_GetWorldSequencePlacementBaseline(*instance, baseline, &sequences))
     { status = "Resource preview cannot resolve its saved placement group."; return false; }
     if (!arena->Try_Get_AuthoringForwardPlacement(position, status)) return false;
     offset = {position.x - baseline.x, position.y - baseline.y, position.z - baseline.z};
    }
   }
   else if (!box.Placement && def->strAnchorKind == "BOSS_SPAWN")
   {
    if (!loaded)
    {
     if (!placements.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") /
        document.strAreaId / "Gameplay.world.json"), document.strAreaId, status)) return false;
     loaded = true;
    }
    std::string placementId = document.strBossPlacementId;
    const auto gate = arena->Get_ActiveDebugGate();
    const auto& gates = arena->Get_DebugGates();
    if (gate < gates.size() && gates[gate].pAuditionPlacementId)
     placementId = gates[gate].pAuditionPlacementId;
    const auto* boss = placements.Find(placementId);
    if (!boss) { status = "World preview cannot resolve the boss spawn placement."; return false; }
    offset.x += boss->position.x - static_cast<float>(def->AnchorPosition[0]);
    offset.y += boss->position.y - static_cast<float>(def->AnchorPosition[1]);
    offset.z += boss->position.z - static_cast<float>(def->AnchorPosition[2]);
   }
   std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> placement;
   if (box.Placement)
   {
    const auto& value = *box.Placement;
    placement = CWorldSequencePlayer::OBJECT_PLACEMENT{
     {float(value.Position[0]), float(value.Position[1]), float(value.Position[2])},
     {float(value.RotationDegrees[0]), float(value.RotationDegrees[1]), float(value.RotationDegrees[2])},
     {float(value.Scale[0]), float(value.Scale[1]), float(value.Scale[2])}};
   }
   cues.push_back({box.strOccurrenceId, def->strSequenceInstanceId, box.iStartMs, box.iDurationMs,
    box.fPlaybackSpeed, offset, placement, previewAtCharacter ? CKoukuSaydonPresentationPlayer::WORLD_EMISSION_ANCHOR{} :
     CKoukuSaydonPresentationPlayer::Make_WorldEmissionAnchor(pattern, *def, box),
    previewAtCharacter ? std::string{} : std::string(CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(pattern.strTargetBossPlacementId)),
    previewAtCharacter ? std::string{} : pattern.strActorProfileId});
  }
  return arena->Debug_BeginCompositionWorldPreview(pattern.strPatternId, std::move(cues), status, sourceDocument);
 }
}
#endif

namespace
{
    void Record_KoukuRaidRequestEvent(const std::string_view eventName,
        const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request,
        const uint64_t worldGeneration, const uint32_t runEpoch, const std::string_view reason) noexcept
    {
        try
        {
            CNetworkManager::Get().Record_SessionEvent(eventName,
                "request=" + std::to_string(request.iRequestSequence) + "; operation=" + std::to_string(unsigned(request.eOperation)) +
                "; worldGeneration=" + std::to_string(worldGeneration) + "; runEpoch=" + std::to_string(runEpoch) +
                "; gate=" + request.strStartGateId + "; reason=" + std::string(reason));
        }
        catch (...) { } // Diagnostics never decide admission or change request ownership.
    }
}

void CMainApp::UpdateKoukuGateCompletePlay()
{
    using namespace LostArk::Shared;
    auto* arena = CLevel_KakulSaydonArena::Get_Active();
    auto& network = CNetworkManager::Get();
    const bool worldChanged = m_iKoukuCompletePlayWorldGeneration != 0u &&
        m_iKoukuCompletePlayWorldGeneration != network.Get_WorldInboundGeneration();
    if (!arena || !network.Is_Connected() || worldChanged)
    {
        if (arena) arena->Expect_KoukuRaidReply(0u);
        if (m_iKoukuCompletePlayWorldGeneration || m_iKoukuRaidPendingRequest)
            m_strKoukuCompletePlayStatus = "The connected world session changed; the previous raid request is no longer tracked.";
        if (m_iKoukuRaidPendingRequest)
        {
            Record_KoukuRaidRequestEvent("kouku.raid.request.reset", m_KoukuRaidRequest,
                m_iKoukuCompletePlayWorldGeneration, 0u, m_strKoukuCompletePlayStatus);
        }
#ifdef _DEBUG
        if (arena) arena->Debug_ResetCompletePlayPreparation();
        m_KoukuRaidResourcePreparation.reset();
        m_iKoukuRaidResourceEpoch = 0u; m_KoukuRaidResourcePatternIds.clear();
#endif
        if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_IsServerClock()) m_pKoukuPresentationPlayer->Stop_Preview();
        m_pKoukuRaidSequenceDocument.reset(); m_iKoukuRaidDocumentEpoch = m_iKoukuRaidAcknowledgedEpoch = 0u;
        m_strKoukuRaidPresentationKey.clear(); m_strKoukuRaidFailedKey.clear(); m_strKoukuCompletePlayFlowGate.clear();
        m_iKoukuRaidPendingRequest = 0u; m_bKoukuRaidStopAfterAdmission = false;
        m_KoukuRaidRequest = {}; m_KoukuRaidReplyDeadline = {}; m_strKoukuRaidReplyStatus.clear();
        m_iKoukuCompletePlayWorldGeneration = 0u;
        return;
    }
#ifdef _DEBUG
    if (m_KoukuRaidResourcePreparation)
    {
        const auto& pending = *m_KoukuRaidResourcePreparation;
        const auto fail = [&](std::string reason) {
            m_KoukuRaidResourcePreparation.reset(); arena->Debug_ResetCompletePlayPreparation();
            m_strKoukuCompletePlayStatus = "Complete raid preparation stopped; no Server start was sent. " + reason;
        };
        if (!CNetworkManager::Get().Is_Connected() || !arena->Get_PlayerCommandSink() ||
            pending.worldGeneration != CNetworkManager::Get().Get_WorldInboundGeneration() ||
            pending.request.ExpectedGameplayRevision != CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision)
        { fail("The connected world or gameplay revision changed."); return; }
        if (std::chrono::steady_clock::now() >= pending.deadline)
        { fail("Preparation exceeded 20 minutes. " + m_strKoukuCompletePlayStatus); return; }
        if ((m_pKoukuSaydonActionWorkbench && (m_pKoukuSaydonActionWorkbench->Is_Dirty() || m_pKoukuSaydonActionWorkbench->Is_PublishRunning())) ||
            (m_pSequenceActionWorkbench && (m_pSequenceActionWorkbench->Is_Dirty() || m_pSequenceActionWorkbench->Is_PublishRunning())) ||
            (m_pKoukuSaydonBossTool && m_pKoukuSaydonBossTool->Is_PlayPreparationPending()) ||
            CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight() ||
            CKoukuSaydonPatternAuditionService::Get().Get_FlowSnapshot().bActive)
        { fail("An editor mutation or another playback replaced this preparation."); return; }
        bool ready = false; std::string reason;
        if (!arena->Debug_PrepareCompletePlayResources(pending.patternIds, {}, pending.request.iActionSourceRevision,
            ready, reason, pending.request.strStartGateId != "BINGO")) { fail(reason); return; }
        m_strKoukuCompletePlayStatus = std::move(reason);
        if (!ready) return;
        CKoukuSaydonCompositionDocument actions;
        CKoukuSaydonCompositionDocument sequences(CKoukuSaydonCompositionDocument::Resolve_SequencePath());
        CKoukuSaydonBossTool product;
        if (!actions.Reload(reason) || !sequences.Reload(reason) || !product.Reload(reason) ||
            actions.Get_LastGood().iRevision != pending.request.iActionSourceRevision ||
            product.Get_SourceRevision() != pending.request.iActionSourceRevision ||
            sequences.Get_LastGood().iRevision != pending.request.iSequenceSourceRevision)
        { fail("Saved or published Action/Sequence changed while preparing. " + reason); return; }
        const auto request = pending.request;
        if (!arena->Get_PlayerCommandSink()->Request_KoukuRaid(request))
        { fail("The prepared raid request could not be submitted."); return; }
        m_KoukuRaidResourcePreparation.reset();
        arena->Expect_KoukuRaidReply(request.iRequestSequence);
        m_KoukuRaidRequest = request; m_iKoukuRaidPendingRequest = request.iRequestSequence;
        m_KoukuRaidReplyDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        m_iKoukuCompletePlayWorldGeneration = CNetworkManager::Get().Get_WorldInboundGeneration();
        m_strKoukuRaidReplyStatus.clear();
        m_strKoukuCompletePlayStatus = "All raid resources prepared; waiting for Server admission.";
        Record_KoukuRaidRequestEvent("kouku.raid.request.sent", request,
            m_iKoukuCompletePlayWorldGeneration, request.iExpectedRunEpoch, {});
    }
#endif
    const auto reply = arena->Get_KoukuRaidReply();
    if (m_iKoukuRaidPendingRequest && reply.iRequestSequence == m_iKoukuRaidPendingRequest &&
        (!reply.iRunEpoch || reply.iOwnerPlayerId == CNetworkManager::Get().Get_LocalPlayerId()))
    {
        Record_KoukuRaidRequestEvent("kouku.raid.request.reply", m_KoukuRaidRequest,
            m_iKoukuCompletePlayWorldGeneration, reply.iRunEpoch, reply.strReason);
        arena->Expect_KoukuRaidReply(0u);
        m_iKoukuRaidPendingRequest = 0u; m_KoukuRaidReplyDeadline = {};
        m_strKoukuRaidReplyStatus.clear();
        if (!reply.iRunEpoch)
        {
            m_bKoukuRaidStopAfterAdmission = false;
            m_strKoukuRaidReplyStatus = m_strKoukuCompletePlayStatus = reply.strReason;
            return;
        }
    }
    if (m_iKoukuRaidPendingRequest && m_KoukuRaidReplyDeadline != std::chrono::steady_clock::time_point{} &&
        std::chrono::steady_clock::now() >= m_KoukuRaidReplyDeadline)
    {
        // A deadline is a notice, not a Server verdict. Retain the exact request
        // until its reply or the owning world session ends; never allow a second START.
        m_KoukuRaidReplyDeadline = {};
        m_strKoukuRaidReplyStatus = m_strKoukuCompletePlayStatus =
            "Server raid reply timed out; still waiting for this request's final Server response.";
        Record_KoukuRaidRequestEvent("kouku.raid.request.timeout", m_KoukuRaidRequest,
            m_iKoukuCompletePlayWorldGeneration, m_KoukuRaidRequest.iExpectedRunEpoch, m_strKoukuRaidReplyStatus);
    }
    const auto& state = arena->Get_KoukuRaidState();
    if (!state.iRunEpoch) return;
    const bool active = state.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING || state.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC || state.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT ||
        state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE || state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME || state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY;
    m_iKoukuCompletePlayWorldGeneration = CNetworkManager::Get().Get_WorldInboundGeneration();
    m_strKoukuCompletePlayFlowGate = active ? state.strGateId : std::string{};
#ifdef _DEBUG
    if (m_bKoukuRaidStopAfterAdmission && !m_iKoukuRaidPendingRequest)
    {
        m_bKoukuRaidStopAfterAdmission = false;
        if (active && state.iRunEpoch == reply.iRunEpoch && state.iOwnerPlayerId == reply.iOwnerPlayerId)
        { CancelKoukuGateCompletePlay("Queued Complete Play stop."); return; }
    }
#endif
    if (state.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING)
    {
        m_strKoukuCompletePlayStatus = "Preparing the shared raid documents; waiting for every participant.";
        const auto localId = CNetworkManager::Get().Get_LocalPlayerId();
        if (std::find(state.ParticipantPlayerIds.begin(), state.ParticipantPlayerIds.end(), localId) == state.ParticipantPlayerIds.end() ||
            m_iKoukuRaidAcknowledgedEpoch == state.iRunEpoch || !arena->Get_PlayerCommandSink()) return;
        std::string preparationError;
        CKoukuSaydonCompositionDocument actions;
        CKoukuSaydonCompositionDocument sequences(CKoukuSaydonCompositionDocument::Resolve_SequencePath());
        std::string preparationStage = "action.reload";
        bool ready = true;
        if (m_iKoukuRaidDocumentEpoch != state.iRunEpoch || !m_pKoukuRaidSequenceDocument)
        {
        ready = actions.Reload(preparationError);
        if (ready)
        {
            preparationStage = "sequence.reload";
            ready = sequences.Reload(preparationError);
        }
        if (ready) preparationStage = "revision.match";
        if (ready && (actions.Get_LastGood().iRevision != state.iActionSourceRevision ||
            sequences.Get_LastGood().iRevision != state.iSequenceSourceRevision ||
            sequences.Get_LastGood().strCompositionId != state.strSequenceCompositionId ||
            CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision != state.PinnedGameplayRevision))
        { ready = false; preparationError = "Saved Action/Sequence or gameplay revision differs from the Server pin"; }
        if (ready)
        {
            // Preload and validate immutable documents only. No playback, spawn, camera or teleport occurs here.
            for (const auto& pattern : sequences.Get_LastGood().Patterns)
            {
                preparationStage = "sequence.expand:" + pattern.strPatternId;
                KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
                if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(sequences.Get_LastGood(), pattern.strPatternId, expanded, preparationError))
                { ready = false; break; }
            }
        }
        if (ready)
        {
            m_pKoukuRaidSequenceDocument = std::make_unique<KOUKU_SAYDON_COMPOSITION_DOCUMENT>(sequences.Get_LastGood());
            m_iKoukuRaidDocumentEpoch = state.iRunEpoch;
        }
        else if (preparationError.empty()) preparationError = "Raid document preparation failed";
        }
#ifdef _DEBUG
        if (ready)
        {
            preparationStage = "resources.prepare";
            CKoukuSaydonBossTool published;
            if (m_iKoukuRaidResourceEpoch != state.iRunEpoch)
            {
                if (!published.Reload(preparationError) || published.Get_SourceRevision() != state.iActionSourceRevision)
                    ready = false;
                else
                {
                    m_KoukuRaidResourcePatternIds = published.Get_PlayAllPatternIds();
                    m_iKoukuRaidResourceEpoch = state.iRunEpoch;
                }
            }
            bool resourcesReady = false;
            if (ready && !arena->Debug_PrepareCompletePlayResources(m_KoukuRaidResourcePatternIds, {},
                state.iActionSourceRevision, resourcesReady, preparationError, true)) ready = false;
            if (ready && !resourcesReady)
            {
                m_strKoukuCompletePlayStatus = preparationError + " Waiting for all raid participants.";
                return; // PREPARING owns no playback clock; never acknowledge queued-only work.
            }
            if (ready)
            {
                preparationStage = "resources.final_revision";
                ready = actions.Reload(preparationError) && sequences.Reload(preparationError) && published.Reload(preparationError) &&
                    actions.Get_LastGood().iRevision == state.iActionSourceRevision &&
                    sequences.Get_LastGood().iRevision == state.iSequenceSourceRevision &&
                    published.Get_SourceRevision() == state.iActionSourceRevision;
                if (!ready && preparationError.empty()) preparationError = "Saved or published data changed during resource preparation";
            }
        }
#endif
        if (ready)
        {
            preparationStage = "gate.prepare";
            ready = arena->Prepare_ServerRaidGatePresentation(state.strGateId, preparationError);
        }
        // Preserve the exact local preflight result before the bounded wire reason is shortened.
        try
        {
            CNetworkManager::Get().Record_SessionEvent("kouku.raid.prepare",
                "runEpoch=" + std::to_string(state.iRunEpoch) + "; ready=" + (ready ? "true" : "false") +
                "; stage=" + preparationStage + "; actionLocal=" + std::to_string(actions.Get_LastGood().iRevision) +
                "; actionPinned=" + std::to_string(state.iActionSourceRevision) +
                "; sequenceLocal=" + std::to_string(sequences.Get_LastGood().iRevision) +
                "; sequencePinned=" + std::to_string(state.iSequenceSourceRevision) +
                "; compositionLocal=" + sequences.Get_LastGood().strCompositionId +
                "; compositionPinned=" + state.strSequenceCompositionId +
                "; gameplayLocal=" + Format_GameplayDataRevision(CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision) +
                "; gameplayPinned=" + Format_GameplayDataRevision(state.PinnedGameplayRevision) +
                "; reason=" + preparationError);
        }
        catch (...) { } // Diagnostics never decide readiness or alter the acknowledgement.
        if (preparationError.size() > MAX_KOUKUSAYDON_PATTERN_AUDITION_REASON_BYTES)
        {
            auto cut = MAX_KOUKUSAYDON_PATTERN_AUDITION_REASON_BYTES;
            while (cut && (static_cast<unsigned char>(preparationError[cut]) & 0xc0u) == 0x80u) --cut;
            preparationError.resize(cut);
        }
        if (!m_iNextKoukuRaidRequest || state.iRequestSequence == UINT32_MAX)
        { m_strKoukuCompletePlayStatus = "Raid request sequence is exhausted"; return; }
        m_iNextKoukuRaidRequest = (std::max)(m_iNextKoukuRaidRequest, state.iRequestSequence + 1u);
        C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST acknowledgement;
        acknowledgement.iRequestSequence = m_iNextKoukuRaidRequest++;
        acknowledgement.eWorldId = state.eWorldId;
        acknowledgement.eOperation = ready ? KOUKUSAYDON_RAID_OPERATION::READY : KOUKUSAYDON_RAID_OPERATION::FAILED;
        acknowledgement.iExpectedRunEpoch = state.iRunEpoch;
        acknowledgement.ExpectedGameplayRevision = state.PinnedGameplayRevision;
        acknowledgement.iActionSourceRevision = state.iActionSourceRevision;
        acknowledgement.iSequenceSourceRevision = state.iSequenceSourceRevision;
        acknowledgement.strStartGateId = state.strGateId;
        if (!ready) acknowledgement.strReason = preparationError;
        if (arena->Get_PlayerCommandSink()->Request_KoukuRaid(acknowledgement)) m_iKoukuRaidAcknowledgedEpoch = state.iRunEpoch;
        m_strKoukuCompletePlayStatus = ready ? "Raid documents prepared; waiting for the shared start tick." : preparationError;
        return;
    }
    const bool cinematic = active && state.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC;
    arena->Debug_SetSequenceCombatPending(cinematic);
    if (!cinematic)
    {
        if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_IsServerClock())
        {
            m_pKoukuPresentationPlayer->Stop_Preview();
            // Natural cinematic completion keeps the authored return blend alive.
            // Explicit Stop/abort still uses Debug_ReturnToPlayerCamera below.
            arena->Stop_CompositionCamera(!active);
        }
        // Commit the prepared combat owner before returning the cinematic lease.
        // Restoring the old book first would Play/Seek it only to replace it again.
        std::string restorationStatus;
        if (active)
        {
            std::string gateStatus;
            if (!arena->Apply_ServerRaidGatePresentation(state.strGateId, state.iRunEpoch, gateStatus))
            {
                (void)arena->End_ServerRaidCinematicPresentation(true, restorationStatus);
                m_strKoukuCompletePlayStatus = "Server gate presentation failed: " + gateStatus;
                if (!restorationStatus.empty()) m_strKoukuCompletePlayStatus += " / " + restorationStatus;
                return;
            }
            // Stop_Preview restores its entry profile. Commit this gate's environment
            // now so the camera return and first visible HUD frame use the same scene.
            Update_KoukuGateSceneProfile();
        }
        if (!arena->End_ServerRaidCinematicPresentation(!active, restorationStatus))
        { m_strKoukuCompletePlayStatus = "Previous gate restore failed: " + restorationStatus; return; }
        m_strKoukuRaidPresentationKey.clear(); m_strKoukuRaidFailedKey.clear();
        m_strKoukuCompletePlayStatus = state.strReason.empty() ? "Server " + state.strGateId + " | " +
            (state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY ? "waiting for Gate 3 entry approval" : state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE ? "boss defeated: waiting for the gate entry vote" : state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME ? "waiting for every maze hunter to return" :
             state.ePhase == KOUKUSAYDON_RAID_PHASE::COMPLETE ? "raid completed" : "saved flow " + state.strFlowEntryId) : state.strReason;
        if (!active) { m_pKoukuRaidSequenceDocument.reset(); m_iKoukuRaidDocumentEpoch = 0u; }
        return;
    }
    const std::string key = std::to_string(state.iRunEpoch) + ":" + state.strSequenceCompositionId + ":" + state.strSequencePatternId + ":" + std::to_string(state.iStartTick);
    if (key == m_strKoukuRaidFailedKey)
    {
        arena->Debug_SetSequenceCombatPending(false);
        std::string restore;
        if (!arena->End_ServerRaidCinematicPresentation(true, restore))
            m_strKoukuCompletePlayStatus = "Previous gate restore failed: " + restore;
        return;
    }
    if (!m_pKoukuPresentationPlayer) return;
    std::string status;
    const auto fail = [&](const std::string& reason) {
        m_strKoukuRaidFailedKey = key;
        if (m_pKoukuPresentationPlayer->Preview_IsServerClock()) m_pKoukuPresentationPlayer->Stop_Preview();
        arena->Debug_SetSequenceCombatPending(false);
        arena->Debug_ReturnToPlayerCamera();
        CNetworkManager::Get().Record_SessionEvent("kouku.cinematic.failed", key + " / " + reason);
        std::string restore;
        m_strKoukuCompletePlayStatus = "Server cinematic presentation failed: " + reason;
        if (!arena->End_ServerRaidCinematicPresentation(true, restore)) m_strKoukuCompletePlayStatus += " / " + restore;
    };
    if (m_iKoukuRaidDocumentEpoch != state.iRunEpoch || !m_pKoukuRaidSequenceDocument)
    {
        CKoukuSaydonCompositionDocument source(CKoukuSaydonCompositionDocument::Resolve_SequencePath());
        if (!source.Reload(status)) { fail(status); return; }
        if (source.Get_LastGood().iRevision != state.iSequenceSourceRevision || source.Get_LastGood().strCompositionId != state.strSequenceCompositionId)
        { fail("Saved Sequence identity/revision differs from the Server pin. Reload the published source before restarting."); return; }
        m_pKoukuRaidSequenceDocument = std::make_unique<KOUKU_SAYDON_COMPOSITION_DOCUMENT>(source.Get_LastGood());
        m_iKoukuRaidDocumentEpoch = state.iRunEpoch;
    }
    const auto snapshotTick = arena->Get_PresentationServerTick();
    const auto serverTick = static_cast<std::int32_t>(snapshotTick - state.iServerTick) >= 0 ? snapshotTick : state.iServerTick;
    const auto elapsedTicks = static_cast<std::int32_t>(serverTick - state.iStartTick);
    const std::uint32_t clockMs = elapsedTicks <= 0 ? 0u : static_cast<std::uint32_t>((std::min)(std::uint64_t(elapsedTicks) * 1000u / 30u, std::uint64_t(600000u)));
    if (m_strKoukuRaidPresentationKey != key)
    {
        KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
        if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(*m_pKoukuRaidSequenceDocument, state.strSequencePatternId, expanded, status))
        { fail(status); return; }
        const auto pattern = std::find_if(expanded.Patterns.begin(), expanded.Patterns.end(), [&](const auto& item) { return item.strPatternId == state.strSequencePatternId; });
        if (pattern == expanded.Patterns.end()) { fail("Pinned Sequence pattern is absent."); return; }
        KOUKU_SAYDON_COMPOSITION_BUNDLE wrapper;
        wrapper.strBundleId = "raid.sequence." + std::to_string(state.iRunEpoch); wrapper.strGateId = pattern->strGateId;
        wrapper.Members.push_back({wrapper.strBundleId + ".member", state.strSequencePatternId, 0u});
        const auto bundleId = wrapper.strBundleId; expanded.Bundles.push_back(std::move(wrapper));
#ifdef _DEBUG
        StopCompositionPreview(m_eCompositionPreviewOwner); ClearKoukuSequenceArrivals();
#endif
        // Later gates and late observers may enter without the initial PREPARING phase.
        if (!arena->Prepare_ServerRaidGatePresentation(state.strGateId, status)) { fail(status); return; }
        if (!arena->Begin_ServerRaidCinematicPresentation(status)) { fail(status); return; }
        if (!m_pKoukuPresentationPlayer->Begin_BundlePreview(expanded, bundleId, clockMs, false, status, &arena->Get_WorldSequenceDocument(), true, false))
        { fail(status); return; }
        m_strKoukuRaidPresentationKey = key;
    }
    m_pKoukuPresentationPlayer->Sample_ServerSequence(clockMs);
    if (!m_pKoukuPresentationPlayer->Preview_Playing()) { fail(m_pKoukuPresentationPlayer->Status()); return; }
    m_strKoukuCompletePlayStatus = "Server " + state.strGateId + " | " + state.strSequencePatternId + " | " + std::to_string(clockMs) + " ms";
}


void CMainApp::Sync_KoukuCinematicUI()
{
	const auto* arena = CLevel_KakulSaydonArena::Get_Active();
	const bool_t suppressed = arena &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA) &&
		arena->Is_CinematicPresentationActive();
	auto& router = CUIInputRouter::Get();
	if (suppressed && !router.Is_CinematicSuppressed())
	{
		// Release may arrive while updates are hidden; no gesture may commit after the cutscene.
		if (m_pInventoryView) m_pInventoryView->Cancel_Interaction();
		if (m_pCharacterInfoView) m_pCharacterInfoView->Cancel_Interaction();
		if (m_pAvatarBookView) m_pAvatarBookView->Cancel_Interaction();
		if (m_pVehicleWindowView) m_pVehicleWindowView->Cancel_Interaction();
		if (m_pHonorTitleWindowView) m_pHonorTitleWindowView->Cancel_Interaction();
		if (m_pWorldMapWindowView) m_pWorldMapWindowView->Cancel_Interaction();
		if (m_pSystemOptionView) m_pSystemOptionView->Cancel_Interaction();
		if (m_pChatWindowView) m_pChatWindowView->Cancel_Interaction();
		if (m_pPartyWindowView) m_pPartyWindowView->Cancel_Interaction();
		if (m_pQuickSlotDragView) m_pQuickSlotDragView->Cancel();
	}
	router.Set_CinematicSuppressed(suppressed);
}

void CMainApp::Update(const f32_t fTimeDelta)
{
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.InputAndUI.Update");
	Update_CustomizingSceneProfile();

	/* Once per frame, before any screen's Update()/Render() checks its own widgets via
	CUIInputRouter -- resets its click-edge tracking. End_Frame() (this function's very end)
	applies the gameplay-mouse block for anything that claimed the mouse this frame. */
	CUIInputRouter::Get().Begin_Frame();
	CUITextOcclusion::Get().Begin_Frame();
	Sync_KoukuCinematicUI();

#ifdef _DEBUG
	// Complete export even when F1 or the profiler window is hidden.
	if (m_pProfilerTool)
		m_pProfilerTool->Update_SaveState();
	UpdateDebugToolShortcut();
#endif

	/* System option rows read every frame: the cursor lock follows focus and window moves,
	the FPS readout is a smoothed frame rate. */
	CUserSettings::Get().Update_CursorLock(IsWindowOwnedByCurrentProcess(GetForegroundWindow()));
	if (fTimeDelta > 0.f)
	{
		const f32_t fInstantFps = 1.f / fTimeDelta;
		m_fSmoothedFps = m_fSmoothedFps <= 0.f ? fInstantFps : m_fSmoothedFps + (fInstantFps - m_fSmoothedFps) * 0.1f;
	}

	/* The loading screen owns the screen: no window opens or toggles under it, and anything left
	open from the previous Level closes here. */
	const bool_t bLoadingLevel =
		ETOUI(LEVEL::LOADING) == CGameInstance::Get().Get_CurrentLevelID();
	if (bLoadingLevel && !m_bWasLoadingLevel)
		Close_RuntimeWindowsForLoading();
	m_bWasLoadingLevel = bLoadingLevel;

	if (!Is_RuntimeUIScreenSuppressed())
	{
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

	/* N is the vehicle window keybind (same gating as I / P above). */
	if (nullptr != m_pVehicleWindowView && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
	{
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t keyDown = windowFocused &&
			0 != (GetAsyncKeyState(0x4E /* VK_N */) & 0x8000);
		if (keyDown && !m_bVehicleWindowKeyDown)
		{
			m_pVehicleWindowView->Toggle();
			const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
				m_pVehicleWindowView->Is_Open() ?
				L"Sound/UI/Select/ui_inventory_show1__669750910.wav" :
				L"Sound/UI/Select/ui_inventory_hide1__7273537.wav");
			CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
		}
		m_bVehicleWindowKeyDown = keyDown;
	}

	/* M is the retail world map keybind (same gating as I / P / N above). The window itself
	stays hidden on levels without a minimap area (see Update_Minimap). */
	if (nullptr != m_pWorldMapWindowView && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
	{
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t keyDown = windowFocused &&
			0 != (GetAsyncKeyState(0x4D /* VK_M */) & 0x8000);
		if (keyDown && !m_bWorldMapKeyDown)
		{
			m_pWorldMapWindowView->Toggle();
			const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
				m_pWorldMapWindowView->Is_Open() ?
				L"Sound/UI/Select/ui_inventory_show1__669750910.wav" :
				L"Sound/UI/Select/ui_inventory_hide1__7273537.wav");
			CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
		}
		m_bWorldMapKeyDown = keyDown;
	}

	Update_SystemOptionWindow(fTimeDelta);
	Update_LobbyButtons(fTimeDelta);
	Update_CharacterSelectWindow(fTimeDelta);
	Update_Minimap(fTimeDelta);
	Update_CombatHUD(fTimeDelta);
	/* Screen-anchored, so it runs in every Level -- including the one the HUD
	   Layout Tool is used from. */
	if (nullptr != m_pDungeonTimerView)
	{
#ifdef _DEBUG
		CCombatHUDViewModel::Get().Debug_Tick_DungeonTimer(fTimeDelta);
#endif
		m_pDungeonTimerView->Update(fTimeDelta,
			CCombatHUDViewModel::Get().Get_DungeonTimer());
	}
	Update_ItemUpgrade(fTimeDelta);
	Update_BossHealthBar();
	Update_BossImmuneGauge(fTimeDelta);
	Update_EstherGauge(fTimeDelta);
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

	/* 5/6/7/8/9/0 ride whatever vehicle is registered on SpecialSkill_1..6 (click-carried from
	the vehicle window, see Update_QuickSlotDrag); the mounted vehicle's own key dismounts. Same
	Server round trip as the window's button and the H key. */
	if (!ImGui::GetIO().WantTextInput && !CUIInputRouter::Get().Is_TextInputActive())
	{
		constexpr int SPECIAL_VIRTUAL_KEYS[6] = { 0x35, 0x36, 0x37, 0x38, 0x39, 0x30 }; // VK_5..VK_9, VK_0
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		for (int32_t i = 0; i < 6; ++i)
		{
			const bool_t keyDown = windowFocused &&
				0 != (GetAsyncKeyState(SPECIAL_VIRTUAL_KEYS[i]) & 0x8000);
			if (keyDown && !m_bSpecialKeyDown[i] && 0u != m_iSpecialQuickSlotVehicle[i])
			{
				const HUD_PLAYER_STATE& quickSlotPlayer = CCombatHUDViewModel::Get().Get_Player();
				const uint32_t iRequest = quickSlotPlayer.iVehicleId == m_iSpecialQuickSlotVehicle[i] ?
					0u : m_iSpecialQuickSlotVehicle[i];
				if (CPlayerController* pController = Find_ActivePlayerController())
					(void)pController->Request_VehicleRiding(iRequest);
			}
			m_bSpecialKeyDown[i] = keyDown;
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
		an input box that would render invisible in a level the window is not drawn in. */
		const uint32_t chatLevel = CGameInstance::Get().Get_CurrentLevelID();
		const bool_t chatLevelAllowed =
			ETOUI(LEVEL::BERN) == chatLevel || ETOUI(LEVEL::VALTAN_ARENA) == chatLevel ||
			ETOUI(LEVEL::KAKULSAYDON_ARENA) == chatLevel;
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
		const bool_t escapeDown = IsWindowOwnedByCurrentProcess(GetForegroundWindow()) &&
			0 != (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
		if (escapeDown && !m_bEscapeDown)
			m_pChatWindowView->Close_Input();
		m_bEscapeDown = escapeDown;
	}

	}
	else
	{
		// Keep the real key edges current without changing any hidden window's state.
		const bool_t focused = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const auto down = [focused](int key) { return focused && 0 != (GetAsyncKeyState(key) & 0x8000); };
		m_bIDown = down(0x49); m_bCharacterInfoKeyDown = down(0x50);
		m_bVehicleWindowKeyDown = down(0x4e); m_bWorldMapKeyDown = down(0x4d);
		m_bSystemOptionKeyDown = m_bEscapeDown = down(VK_ESCAPE); m_bEnterDown = down(VK_RETURN);
		for (int i = 0; i < 4; ++i) m_bItemKeyDown[i] = down(0x31 + i);
		const int keys[] = {0x35, 0x36, 0x37, 0x38, 0x39, 0x30};
		for (int i = 0; i < 6; ++i) m_bSpecialKeyDown[i] = down(keys[i]);
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
	const bool_t authoredEffectPlacementConsumed =
		(nullptr != m_pEffectTool && m_pEffectTool->Update_AuthoringPlacementInput(m_bDeveloperToolsVisible &&
			IsDebugToolVisible(DEBUG_TOOL::EFFECT) && m_eDebugInputOwner == DEBUG_TOOL::EFFECT)) |
		(nullptr != m_pEffectToolV2 && m_pEffectToolV2->Update_AuthoringPlacementInput(m_bDeveloperToolsVisible &&
			IsDebugToolVisible(DEBUG_TOOL::EFFECT_V2) && m_eDebugInputOwner == DEBUG_TOOL::EFFECT_V2));
	/* Evaluated before the OR so every armed picker still runs its own frame. */
	const bool_t worldLevelPickConsumed = UpdateWorldLevelPlacementPickInput();
	const bool_t mapEffectPlacementConsumed = UpdateMapEffectPlacementInput() ||
		authoredEffectPlacementConsumed || worldLevelPickConsumed;
	const bool_t worldLeftMouseConsumed = mapEffectPlacementConsumed ||
		(nullptr != m_pMapTool && m_pMapTool->ConsumesWorldLeftMouse());
#else
	constexpr bool_t mapEffectPlacementConsumed = false;
	constexpr bool_t externalToolFocused = false;
	constexpr bool_t worldLeftMouseConsumed = false;
#endif

	/* The runtime nickname field blocks DirectInput keyboard polling exactly the way an
	ImGui InputText's WantsCaptureKeyboard does -- WASD/skill keys must not fire mid-typing. */
	const bool_t keyboardCaptured = (nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->WantsCaptureKeyboard() || externalToolFocused)) ||
		CUIInputRouter::Get().Is_TextInputActive() || CUIInputRouter::Get().Is_CinematicSuppressed();
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
	CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, mapEffectPlacementConsumed);

	}
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Network.DrainAndDispatch");
		CNetworkManager::Get().Update();
	}
	// Release raids also emit owner lifecycle events; drain the shared queues every frame.
	CKoukuSaydonPatternAuditionService::Get().Update();
#ifdef _DEBUG
	/* PLAY_PATTERN_ID has one process-wide verdict/lifecycle queue shared by
	   Balance, Effect, and Valtan Boss Tools. Drain it once per frame here, independent
	   of which panel is visible or which tree row is expanded. */
	CValtanPatternAuditionService::Get().Update();
	CValtanPatternFlowService::Get().Update();
	CValtanTuningCommandService::Get().Update();
	if (m_pKoukuSaydonBossTool)
	{
		const bool preparing = m_pKoukuSaydonBossTool->Is_PlayPreparationPending();
		if (m_pKoukuSaydonActionWorkbench &&
			(m_pKoukuSaydonActionWorkbench->Consume_ServerPlayCancelRequest() ||
				(preparing && (m_pKoukuSaydonActionWorkbench->Is_Dirty() ||
					m_pKoukuSaydonActionWorkbench->Is_PublishRunning()))))
			(void)m_pKoukuSaydonBossTool->Cancel_PlayPreparation(m_strKoukuCompletePlayStatus);
		m_pKoukuSaydonBossTool->Update();
		if (preparing) m_strKoukuCompletePlayStatus = m_pKoukuSaydonBossTool->Get_Status();
		if (m_pKoukuSaydonActionWorkbench)
			m_pKoukuSaydonActionWorkbench->Set_ServerPlayPreparationPending(
				m_pKoukuSaydonBossTool->Is_PlayPreparationPending(),
				preparing ? std::string_view(m_pKoukuSaydonBossTool->Get_Status()) : std::string_view{});
	}
#endif
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.Engine.Update");
	CGameInstance::Get().Update_Engine(fTimeDelta);
	}
	Update_KoukuGateSceneProfile();
#ifdef _DEBUG
	// Debug reset handling follows the common raid consumer.
#endif
	if (!CLevel_KakulSaydonArena::Get_Active()) UpdateKoukuGateCompletePlay();
#ifdef _DEBUG
	// Consume the Server reset before a finished entry preview can queue its next gate.
	if (auto* startArena = CLevel_KakulSaydonArena::Get_Active(); startArena && startArena->Consume_DebugReturnToStartSucceeded())
	{
		CancelKoukuGateCompletePlay("Returned to arena start after Server reset.");
		StopCompositionPreview(DEBUG_TOOL::SEQUENCER);
	}
#endif
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.Presentation.Prepare");
	std::vector<BOSS_STAGE_ENVIRONMENT_SAMPLE> valtanEnvironmentSamples;
	CValtan::Collect_StageEnvironmentSamples(valtanEnvironmentSamples);
	if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
	{
		if (!m_pKoukuPresentationPlayer)
			m_pKoukuPresentationPlayer = std::make_unique<CKoukuSaydonPresentationPlayer>(
				m_pDevice, m_pContext, m_RenderingProfiles);
		arena->Set_TargetedCombatPresentationPlayer(m_pKoukuPresentationPlayer.get());
		std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
		m_pKoukuPresentationPlayer->Set_LightResources(&m_LightResources);
		std::vector<KOUKU_CARD_PRESENTATION_VIEW> cards;
		arena->Collect_KoukuPresentationViews(bosses, cards);
		UpdateKoukuGateCompletePlay();
		m_pKoukuPresentationPlayer->Update(fTimeDelta, bosses, cards);
		arena->Set_CompositionWorldEmissionResolver([this](std::uint32_t sourceRevision, std::string_view patternId, std::string_view occurrenceId,
			CLevel_KakulSaydonArena::WORLD_EMISSION_ANCHOR& anchor)
		{ return m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Resolve_ProductWorldEmissionAnchor(
			sourceRevision, patternId, occurrenceId, anchor); });
#ifdef _DEBUG
        if (m_pEffectTool) m_pEffectTool->Set_AuthoringPlayer(m_pKoukuPresentationPlayer.get());
        if (m_pEffectToolV2) m_pEffectToolV2->Set_AuthoringPlayer(m_pKoukuPresentationPlayer.get());
#endif
	}
	else if (m_pKoukuPresentationPlayer && !CLevel_ValtanArena::Get_Active() && valtanEnvironmentSamples.empty())
	{
#ifdef _DEBUG
        if (m_pEffectTool) m_pEffectTool->Set_AuthoringPlayer(nullptr);
        if (m_pEffectToolV2) m_pEffectToolV2->Set_AuthoringPlayer(nullptr);
		StopCompositionPreview(m_eCompositionPreviewOwner);
#endif
		m_pKoukuPresentationPlayer->Reset();
		m_pKoukuPresentationPlayer.reset();
	}
	if (!valtanEnvironmentSamples.empty() && !m_pKoukuPresentationPlayer)
		m_pKoukuPresentationPlayer = std::make_unique<CKoukuSaydonPresentationPlayer>(
			m_pDevice, m_pContext, m_RenderingProfiles);
	if (m_pKoukuPresentationPlayer)
	{
		m_pKoukuPresentationPlayer->Set_LightResources(&m_LightResources);
		m_pKoukuPresentationPlayer->Update_BossStageEnvironments(valtanEnvironmentSamples);
	}
#ifdef _DEBUG
	UpdateLightingPreview();
	if (m_pRenderingBenchmark)
		m_pRenderingBenchmark->Update_RestorationPreview(m_RenderingProfiles,
			m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::RENDERING) &&
			m_bRenderingQualityWindowVisible);
#endif
	if (ETOUI(LEVEL::LOADING) !=
		CGameInstance::Get().Get_CurrentLevelID())
	{
		CEffectPresentationService::Advance_ProductCuePreparation(
			m_pDevice, m_pContext);
	}
	CEffectPresentationService::Commit_PendingSpawns();
	CEffectPresentationService::Prepare_FrameCamera(fTimeDelta);
	CEffectPresentationService::Synchronize_FollowAnchors();
	}
	CEffectPresentationService::Update(fTimeDelta);
	/* Product combat-object groups and Effect Tool previews share one free-group
	   clock. MainApp advances it exactly once after the Engine object tick. */
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.ProductGroups.Update");
	CEffectV2Runtime::Advance_ProductGroups(fTimeDelta, m_pDevice, m_pContext);
	}

	#ifdef _DEBUG
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.DebugTools.Update");
	if (nullptr != m_pMapTool)
	{
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Map.Update");
			m_pMapTool->Update(
			fTimeDelta,
			m_bDeveloperToolsVisible &&
			IsDebugToolVisible(DEBUG_TOOL::MAP) &&
			DEBUG_TOOL::MAP == m_eDebugInputOwner);
		}
	}
	UpdateSequenceViewer();
	UpdateWorldLevelTool();
	/* Composition emits a one-shot claim; MainApp remains the sole input-owner
	   authority. Consume it before Animation_Tool::Update so reclaiming after a
	   domain deep-link does not stop the active preview for one extra frame. */
	if (nullptr != m_pValtanActionWorkbench &&
		m_pValtanActionWorkbench->Consume_PreviewOwnerClaimRequest())
	{
		if (m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) &&
			nullptr != m_pSequencerTool &&
			m_pSequencerTool->Uses_ValtanSession())
		{
			StopCompositionPreview(m_eCompositionPreviewOwner);
			m_eDebugInputOwner = m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE ?
				DEBUG_TOOL::SEQUENCER_BENCHMARK : DEBUG_TOOL::SEQUENCER;
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
		const bool_t bValtanClockActive = m_pAnimationTool->Get_ValtanCompositionPreviewState().bPlaying;
		const bool_t bAnimationPreviewOwned =
			(IsDebugToolVisible(DEBUG_TOOL::ANIMATION) &&
			 DEBUG_TOOL::ANIMATION == m_eDebugInputOwner) ||
			(IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) && m_pSequencerTool &&
			 ((m_pSequencerTool->Is_BossSelected() && DEBUG_TOOL::SEQUENCER == m_eDebugInputOwner) ||
			  (m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE &&
			   DEBUG_TOOL::SEQUENCER_BENCHMARK == m_eDebugInputOwner))) ||
			(IsDebugToolVisible(DEBUG_TOOL::SEQUENCER_BENCHMARK) &&
			 DEBUG_TOOL::SEQUENCER_BENCHMARK == m_eDebugInputOwner) ||
			(m_eCompositionPreviewOwner != DEBUG_TOOL::NONE && !bValtanClockActive);
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Animation.Update");
			m_pAnimationTool->Update(
			fTimeDelta,
			m_bDeveloperToolsVisible && bAnimationPreviewOwned);
		}
	}
	/* Save and publish jobs are process owners, not window owners. Poll the
	   immutable Save receipt first, let a hidden Composition caller accept and
	   reopen its exact local owners, then poll the optional Full DataOnly job. */
	if (nullptr != m_pBalanceTool)
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Balance.Update_ValtanSaveJob");
			m_pBalanceTool->Update_ValtanSaveJob();
		}
	if (nullptr != m_pValtanActionWorkbench)
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.ValtanComposition.Update_SaveState");
			m_pValtanActionWorkbench->Update_SaveState();
		}
	if (nullptr != m_pBalanceTool)
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Balance.Update_ServerRuntimeSetPublishJob");
			m_pBalanceTool->Update_ServerRuntimeSetPublishJob();
		}
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
	if (m_pEffectTool || m_pEffectToolV2)
	{
        shared_ptr<CCamera_Free> effectCamera;
        const LEVEL effectLevel = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
        if (effectLevel == LEVEL::CHARACTER_SELECT)
        { if (auto* arena = CLevel_CharacterSelect::Get_Active()) effectCamera = arena->Get_DebugCamera(); }
        else if (effectLevel == LEVEL::VALTAN_ARENA)
        { if (auto* arena = CLevel_ValtanArena::Get_Active()) effectCamera = arena->Get_DebugCamera(); }
        else if (effectLevel == LEVEL::KAKULSAYDON_ARENA)
        { if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) effectCamera = arena->Get_DebugCamera(); }
        if (m_pCharacterActionWorkbench) m_pCharacterActionWorkbench->Set_Camera(effectCamera);
        if (m_pEffectToolV2)
        {
            m_pEffectToolV2->Set_AuthoringCamera(effectCamera);
            {
                Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.EffectV2.Update_AuthoringWorkspace");
                m_pEffectToolV2->Update_AuthoringWorkspace(fTimeDelta, m_bDeveloperToolsVisible &&
                IsDebugToolVisible(DEBUG_TOOL::EFFECT_V2) && m_eDebugInputOwner == DEBUG_TOOL::EFFECT_V2);
            }
        }
        if (m_pEffectTool)
        {
            m_pEffectTool->Set_AuthoringCamera(effectCamera);
            {
                Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.EffectV1.Update_AuthoringWorkspace");
                m_pEffectTool->Update_AuthoringWorkspace(fTimeDelta, m_bDeveloperToolsVisible &&
                IsDebugToolVisible(DEBUG_TOOL::EFFECT) && m_eDebugInputOwner == DEBUG_TOOL::EFFECT);
            }
            {
                Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.EffectV1.Update");
                m_pEffectTool->Update(fTimeDelta);
            }
            EFFECT_RESOURCE_KEY ResourceKey;
            if (m_pEffectTool->Consume_TypedEffectResourceOpenRequest(ResourceKey))
            {
                const bool_t bOpened = ResourceKey.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ?
                    m_pEffectTool->Open_AuthoringResource(ResourceKey) :
                    SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT_V2)) && m_pEffectToolV2->Open_Resource(ResourceKey);
                m_strToolStatus = bOpened ?
                    "Opened the selected resource in its typed Effect owner." :
                    "The typed Effect owner preserved its current draft; inspect the owner status.";
            }
        }
    }
	if (nullptr != m_pValtanBossTool)
	{
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.ValtanBoss.Update");
			m_pValtanBossTool->Update(
			m_bDeveloperToolsVisible &&
				IsDebugToolVisible(DEBUG_TOOL::VALTAN_BOSS),
			m_bDeveloperToolsVisible &&
				IsDebugToolVisible(DEBUG_TOOL::VALTAN_LOGIC_PATTERN));
		}
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
	if (m_pKoukuSaydonBossTool && m_pKoukuSaydonBossTool->Consume_PublishRequest())
	{
		if (FAILED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER)) || !m_pKoukuSaydonActionWorkbench)
			m_strKoukuCompletePlayStatus = "The Composition publisher workspace could not initialize.";
		else if (m_pKoukuSaydonActionWorkbench->Is_Dirty() || m_pKoukuSaydonActionWorkbench->Is_PublishRunning())
			m_strKoukuCompletePlayStatus = "Save Composition edits and wait for the current publish before publishing Pattern Flow.";
		else if (m_pKoukuSaydonActionWorkbench->Reload(m_strKoukuCompletePlayStatus))
			m_bKoukuFlowPublishPending = m_pKoukuSaydonActionWorkbench->Publish_AllPatterns(m_strKoukuCompletePlayStatus);
		m_pKoukuSaydonBossTool->Set_Status(m_strKoukuCompletePlayStatus);
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
			nullptr != m_pSequencerTool && m_pSequencerTool->Uses_ValtanSession() &&
			m_eDebugInputOwner == (m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE ?
				DEBUG_TOOL::SEQUENCER_BENCHMARK : DEBUG_TOOL::SEQUENCER));
	}
	struct COMPOSITION_SESSION_ROUTE
	{
		DEBUG_TOOL owner;
		CKoukuSaydonActionWorkbench* workbench;
		CSequencerTool* shell;
	};
	const std::array<COMPOSITION_SESSION_ROUTE, 2> compositionRoutes{{
		{DEBUG_TOOL::SEQUENCER, m_pKoukuSaydonActionWorkbench.get(),
            m_pSequencerTool && m_pSequencerTool->Is_BossSelected() ? m_pSequencerTool.get() : nullptr},
		{DEBUG_TOOL::SEQUENCER_BENCHMARK, m_pSequenceActionWorkbench.get(),
            m_pSequencerTool && m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE ?
                m_pSequencerTool.get() : nullptr}
	}};
	// Drain each edge, read physical inventory once, then distribute immutable rows.
	bool shellResourceRefresh = false, koukuResourceRefresh = false, presentationResourceRefresh = false;
	std::vector<CKoukuSaydonActionWorkbench*> resourceSessions;
	for (const auto& route : compositionRoutes)
	{
		if (route.shell) shellResourceRefresh |= route.shell->Consume_ResourceRefreshRequest();
		if (!route.workbench) continue;
		resourceSessions.push_back(route.workbench);
		route.workbench->Set_PresentationModelResolver([this](const auto& pattern, auto target, auto& view) {
			return m_pKoukuPresentationPlayer &&
				m_pKoukuPresentationPlayer->Resolve_PatternModelTarget(pattern, target, view);
		});
		koukuResourceRefresh |= route.workbench->Consume_ResourceRefreshRequest();
		presentationResourceRefresh |= route.workbench->Consume_PresentationResourceRefreshRequest();
	}
	if (presentationResourceRefresh)
		Refresh_KoukuPresentationResources(resourceSessions, m_RenderingProfiles, m_LightResources);
	if ((shellResourceRefresh || presentationResourceRefresh) && m_pValtanActionWorkbench)
	{
		std::vector<std::string> lights;
		for (const auto& resource : m_LightResources.Get_Resources()) lights.push_back(resource.strLightResourceId);
		m_pValtanActionWorkbench->Set_EnvironmentResources(m_RenderingProfiles.Collect_ProfileIds(), std::move(lights));
	}
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
			for (const auto& route : compositionRoutes)
			{
				if (route.shell) route.shell->Set_AnimationResources(resources, status);
				if (!route.workbench) continue;
				route.workbench->Set_ModelResources(resources, status);
				route.workbench->Set_SequenceResources(sequenceResources, sequenceStatus, sequencesLoaded);
			}
		}
		else
		{
			m_strToolStatus = "Animation preview backend could not initialize for Resources.";
			for (const auto& route : compositionRoutes)
				if (route.shell) route.shell->Set_AnimationResources({}, m_strToolStatus);
		}
	}
	// A draft generation is local to its session. Give the shared consumer a monotonic
	// serial only after selecting the owner, so equal occurrence IDs cannot cross drafts.
	for (const auto& route : compositionRoutes)
	{
		if (!m_strKoukuCompletePlayFlowGate.empty() || !route.workbench || !m_pKoukuPresentationPlayer) continue;
		const bool ownsPreview = route.owner == m_eCompositionPreviewOwner;
		const bool productDebugAuthoring = m_eCompositionPreviewOwner == DEBUG_TOOL::NONE &&
			!m_pKoukuPresentationPlayer->Preview_Playing() && route.owner == DEBUG_TOOL::SEQUENCER;
		if (!ownsPreview && !productDebugAuthoring) continue;
		const auto generation = route.workbench->Get_DraftGeneration();
		if (m_eColliderAuthoringOwner != route.owner || m_iColliderAuthoringDraftGeneration != generation)
		{
			m_eColliderAuthoringOwner = route.owner;
			m_iColliderAuthoringDraftGeneration = generation;
			m_pKoukuPresentationPlayer->Refresh_ColliderAuthoring(
				route.workbench->Get_Composition(), ++m_iColliderAuthoringSerial);
		}
	}
	std::string activePreviewRouteStatus;
	DEBUG_TOOL previewStatusOwner = DEBUG_TOOL::NONE;
    if (m_strKoukuCompletePlayFlowGate.empty()) m_bKoukuLocalPreviewStopRequested = false;
	for (const auto& route : compositionRoutes)
	{
		auto* workbench = route.workbench;
		auto* shell = route.shell;
		if (!workbench) continue;
		// Observe publisher completion even while the Server raid owns playback.
		workbench->Tick_Background();
		if (route.owner == DEBUG_TOOL::SEQUENCER && m_bKoukuFlowPublishPending && !workbench->Is_PublishRunning())
		{
			m_bKoukuFlowPublishPending = false;
			m_strKoukuCompletePlayStatus = workbench->Get_Status();
			if (m_pKoukuSaydonBossTool) m_pKoukuSaydonBossTool->Set_Status(m_strKoukuCompletePlayStatus);
		}
		if (workbench->Consume_ProductInventoryRefreshRequest() &&
			route.owner == DEBUG_TOOL::SEQUENCER && m_pKoukuSaydonBossTool)
			(void)m_pKoukuSaydonBossTool->Reload(m_strKoukuCompletePlayStatus);
        if (!m_strKoukuCompletePlayFlowGate.empty())
        {
            // Reset/Play must not disappear behind the active raid guard. Stop
            // through the existing Server owner, then consume the immutable
            // local draft request only after the terminal raid state arrives.
            bool resetRequested = false;
            if (workbench->Has_PendingPreviewReset())
            {
                KOUKU_PREVIEW_TRANSPORT transport;
                std::uint32_t seekMs = 0u;
                resetRequested = workbench->Consume_PreviewTransportRequest(transport, seekMs);
            }
            if (shell && !shell->Uses_ValtanSession())
            {
                CSequencerTool::ANIMATION_PREVIEW_TRANSPORT transport;
                if (shell->Consume_AnimationPreviewTransportRequest(transport))
                    resetRequested |= transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP;
            }
            if (resetRequested) m_bKoukuLocalPreviewStopRequested = false;
            if (resetRequested || workbench->Has_PendingLocalPreviewRequest())
            {
                if (!m_bKoukuLocalPreviewStopRequested)
                {
                    m_bKoukuLocalPreviewStopRequested = true;
                    CancelKoukuGateCompletePlay("Stopped Complete Play for local Sequence editing.");
                }
                auto state = workbench->Get_PreviewState();
                state.strStatus = m_strKoukuCompletePlayStatus;
                workbench->Set_PreviewState(state);
                if (shell) shell->Set_AnimationPreviewStatus(state.strStatus);
            }
            continue;
        }
	if (nullptr != shell)
	{
		COMPOSITION_ANIMATION_RESOURCE resource;
		if (shell->Consume_AnimationPreviewRequest(resource))
		{
			workbench->Cancel_CompleteSequencePlay();
			if (SUCCEEDED(EnsureAnimationPreviewBackend()) && nullptr != m_pAnimationTool)
			{
				if (m_pAnimationTool->Preview_CompositionAnimationResource(resource, m_strToolStatus))
				{
					if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_StopCompositionWorldPreview();
					if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Stop_Preview();
					ClaimCompositionPreviewOwner(route.owner);
					m_eDebugInputOwner = route.owner;
				}
			}
			else
				m_strToolStatus = "Animation preview backend could not initialize for the selected resource.";
			shell->Set_AnimationPreviewStatus(m_strToolStatus);
		}
	}
	/* The K Workbench owns only composition data. A click transfers an immutable
	   occurrence/pattern value to the existing real-CModel preview owner; no
	   draft pointer or Valtan document crosses this boundary. */
	std::string previewRouteStatus;
    const auto notifySequencePlaybackStarted = [&]() {
        if (route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
            if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
                arena->Notify_SequencePlaybackStarted();
    };
    const auto pausePreviousLocalPreview = [&]() {
        if (m_eCompositionPreviewOwner != route.owner || !m_pKoukuPresentationPlayer ||
            m_pKoukuPresentationPlayer->Preview_IsServerClock()) return;
        // A rejected replacement keeps the previous scene, but its old sound
        // windows must not continue behind the newly edited timeline.
        m_pKoukuPresentationPlayer->Pause_Preview(true);
        if (m_pAnimationTool)
        {
            std::string pauseStatus;
            (void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(true, pauseStatus);
        }
    };
	if (nullptr != workbench)
	{
		KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence;
		if (workbench->Consume_AnimationPreviewRequest(
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
					if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_StopCompositionWorldPreview();
					if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Stop_Preview();
					ClaimCompositionPreviewOwner(route.owner);
					m_eDebugInputOwner = route.owner;
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

		std::string bundlePreviewId;
		std::uint32_t bundleClockMs = 0; bool_t bundlePaused = false;
		if (workbench->Consume_BundlePreviewRequest(bundlePreviewId, bundleClockMs, bundlePaused))
		{
			if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Begin_BundlePreview(
				workbench->Get_PatternPreviewDocument(), bundlePreviewId, bundleClockMs, bundlePaused, previewRouteStatus,
				CompositionPreviewWorldSource()))
			{
				if (m_pAnimationTool) { std::string stoppedAnimationStatus; (void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus); }
				if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_StopCompositionWorldPreview();
				ClaimCompositionPreviewOwner(route.owner);
					m_eDebugInputOwner = route.owner;
                workbench->Notify_SequencePreviewAdmission(true, previewRouteStatus);
                if (!bundlePaused) notifySequencePlaybackStarted();
			}
			else
            {
                if (!m_pKoukuPresentationPlayer) previewRouteStatus = "Bundle preview requires the KoukuSaydon Arena.";
                workbench->Notify_SequencePreviewAdmission(false, previewRouteStatus);
                pausePreviousLocalPreview();
            }
		}
		std::string serverBundleId; std::uint32_t bundleRevision = 0;
		if (workbench->Consume_BundleServerPlayRequest(serverBundleId, bundleRevision) &&
			route.owner == DEBUG_TOOL::SEQUENCER)
		{
			if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Stop_Preview();
			if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_StopCompositionWorldPreview();
			ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
			if (!m_pKoukuSaydonBossTool) m_pKoukuSaydonBossTool = make_unique<CKoukuSaydonBossTool>();
			(void)m_pKoukuSaydonBossTool->Play_BundleById(serverBundleId, bundleRevision, m_strToolStatus);
			workbench->Set_ServerPlayPreparationPending(m_pKoukuSaydonBossTool->Is_PlayPreparationPending(), m_strToolStatus);
		}
		KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
		std::uint32_t startClockMs = 0u;
		bool_t startPaused = false;
		std::string targetAssetName;
		if (workbench->Consume_PatternPreviewRequest(
    pattern, startClockMs, startPaused, targetAssetName))
  {
   const bool completeSequenceRequested = workbench->Is_CompleteSequencePlaying();
   if (!completeSequenceRequested && route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK &&
    !m_strKoukuCompletePlayFlowGate.empty())
    CancelKoukuGateCompletePlay("Switched to Play Sequence; automatic combat entry was cancelled.");
   bool previewAccepted = false;
   const bool hasAnimation = std::any_of(pattern.Stages.begin(), pattern.Stages.end(),
    [](const auto& stage) { return !stage.AnimationOccurrences.empty(); });
   const auto& previewDocument = workbench->Get_PatternPreviewDocument();
   const bool hasActorLogic = std::any_of(pattern.SummonOccurrences.begin(), pattern.SummonOccurrences.end(),
    [&](const auto& summon) {
     return !summon.PatternSpawns.empty() || std::any_of(previewDocument.Summons.begin(), previewDocument.Summons.end(),
      [&](const auto& definition) { return definition.strSummonId == summon.strSummonId &&
       definition.strSummonKind == "CROSS_DIRECTION_CLONES"; });
    }) ||
    std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(), [&](const auto& box) {
     return box.bEnabled && std::any_of(previewDocument.Logics.begin(), previewDocument.Logics.end(),
      [&](const auto& logic) { return logic.strLogicId == box.strLogicId &&
       (logic.strJudgementKind == "CROSS_DIRECTION_CLONES" || logic.strJudgementKind == "BOSS_TRACK_TARGET" ||
        logic.strJudgementKind == "SHOWTIME_PLAYER_TARGETS" || logic.strTriggerKind == "BOSS_TELEPORT_XZ" ||
        logic.strTriggerKind == "BOSS_TELEPORT_GROUNDED" ||
        logic.strTriggerKind == "ALBION_AIRBORNE" || logic.strTriggerKind == "ALBION_BLUE_CIRCLE"); });
    });
   if ((hasAnimation || hasActorLogic) && (!pattern.strActorProfileId.empty() || pattern.BossMotion || pattern.fAnimationRootVerticalScale != 1.0) && targetAssetName.empty())
   {
    auto document = workbench->Get_PatternPreviewDocument();
    for (auto& previewPattern : document.Patterns)
     if (previewPattern.strPatternId == pattern.strPatternId) { previewPattern = pattern; break; }
    KOUKU_SAYDON_COMPOSITION_BUNDLE single;
    single.strBundleId = pattern.strPatternId;
    single.strGateId = pattern.strGateId;
    single.Members.push_back({pattern.strPatternId + ".preview.member", pattern.strPatternId, 0u});
    document.Bundles.push_back(std::move(single));
    if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Begin_BundlePreview(
      document, pattern.strPatternId, startClockMs, startPaused, previewRouteStatus,
      CompositionPreviewWorldSource(), true, true))
    {
     if (m_pAnimationTool) { std::string stoppedAnimationStatus; (void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus); }

     ClaimCompositionPreviewOwner(route.owner);
     previewAccepted = Begin_KoukuWorldPreview(document, pattern, previewRouteStatus,
      CompositionPreviewWorldSource());
     if (!previewAccepted) StopCompositionPreview(route.owner);
					m_eDebugInputOwner = route.owner;
    }
    else if (!m_pKoukuPresentationPlayer) previewRouteStatus = "Pattern actor preview requires the KoukuSaydon Arena.";

   }
   else
   {
   bool previewed = !hasAnimation;
   if (hasAnimation && SUCCEEDED(EnsureAnimationPreviewBackend()) && m_pAnimationTool)
    previewed = targetAssetName.empty() ?
     m_pAnimationTool->Preview_KoukuSaydonCompositionPattern(pattern, m_strToolStatus, startClockMs, startPaused) :
     m_pAnimationTool->Preview_CompositionResourcePattern(pattern, targetAssetName, m_strToolStatus, startClockMs, startPaused);
   if (previewed && m_pKoukuPresentationPlayer)
   {
    const auto& document = workbench->Get_PatternPreviewDocument();
    previewed = m_pKoukuPresentationPlayer->Begin_Preview(document, pattern,
     !hasAnimation, startClockMs, startPaused, previewRouteStatus);
    if (previewed)
    {
     if (!hasAnimation && m_pAnimationTool) { std::string stoppedAnimationStatus; (void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus); }
     ClaimCompositionPreviewOwner(route.owner);
     m_eDebugInputOwner = route.owner;
     previewAccepted = Begin_KoukuWorldPreview(document, pattern, previewRouteStatus,
      CompositionPreviewWorldSource());
     if (!previewAccepted)
     {
      workbench->Notify_SequencePreviewAdmission(false, previewRouteStatus);
      StopCompositionPreview(route.owner);
     }
    }
   }
   else if (previewed) previewRouteStatus = "Presentation preview requires the KoukuSaydon Arena.";
   else previewRouteStatus = m_strToolStatus;
   }
   if (previewAccepted && route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
   {
    previewAccepted = BeginKoukuSequenceArrivals(workbench->Get_PatternPreviewDocument(), pattern,
     startClockMs, previewRouteStatus);
    if (!previewAccepted) StopCompositionPreview(route.owner);
   }
   workbench->Notify_SequencePreviewAdmission(previewAccepted, previewRouteStatus);
   if (previewAccepted && !startPaused) notifySequencePlaybackStarted();
   if (!previewAccepted && !completeSequenceRequested) pausePreviousLocalPreview();
   if (completeSequenceRequested && !previewAccepted)
   {
    StopCompositionPreview(route.owner);
    if (route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
     CancelKoukuGateCompletePlay(previewRouteStatus);
   }
  }

  KOUKU_SUMMON_PLACEMENT_PREVIEW_REQUEST summonPlacementPreview;
  if (workbench->Consume_SummonPlacementPreviewRequest(summonPlacementPreview) &&
   m_pKoukuPresentationPlayer && m_eCompositionPreviewOwner == route.owner)
   if (!m_pKoukuPresentationPlayer->Preview_SummonPlacement(
    summonPlacementPreview.strPatternId, summonPlacementPreview.Occurrence))
    workbench->Notify_WorldObjectEditResult("Summon placement preview is unavailable. Use World Preview to rebuild this box.");

  KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST presentationGeometryPreview;
  while (workbench->Consume_PresentationGeometryPreviewRequest(presentationGeometryPreview))
   if (m_pKoukuPresentationPlayer && m_eCompositionPreviewOwner == route.owner)
    (void)m_pKoukuPresentationPlayer->Preview_PresentationGeometry(
     presentationGeometryPreview.strPatternId, presentationGeometryPreview.Occurrence);

  KOUKU_PRESENTATION_PREVIEW_REQUEST resourcePreview;
  if (workbench->Consume_PresentationPreviewRequest(resourcePreview))
  {
   if (m_pWorldObjectTool) m_pWorldObjectTool->Deactivate();
   const bool keepCurrentPreview = m_eCompositionPreviewOwner == route.owner && m_pKoukuPresentationPlayer &&
    m_pKoukuPresentationPlayer->Preview_HasActiveWorldBox(resourcePreview.strEditedOccurrenceId);
   if (!keepCurrentPreview)
   {
   if (m_pKoukuPresentationPlayer)
   {
    auto document = workbench->Get_Composition();
    KOUKU_SAYDON_COMPOSITION_PATTERN resourcePattern;
    resourcePattern.strPatternId = "preview.kouku.resource";
    // Resource audition uses the selected boss context; its temporary placement
    // never replaces the anchor authored in a Timeline occurrence.
    if (resourcePreview.Resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
     (resourcePreview.Resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT && resourcePreview.Occurrence.strAnchorKind == "BOSS"))
    {
     const auto selected = std::find_if(document.Patterns.begin(), document.Patterns.end(),
      [&](const auto& item) { return item.strPatternId == workbench->Get_SelectedPatternId(); });
     if (selected != document.Patterns.end() && selected->strLoadError.empty())
     {
      resourcePattern.strActorProfileId = selected->strActorProfileId;
      resourcePattern.strGateId = selected->strGateId;
      resourcePattern.strTargetBossPlacementId = selected->strTargetBossPlacementId;
     }
    }
    KOUKU_SAYDON_COMPOSITION_STAGE stage;
    stage.strStageId = "preview.resource.stage"; stage.iDurationMs = resourcePreview.Occurrence.iDurationMs;
    resourcePattern.Stages.push_back(stage);
    if (resourcePreview.Resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::WORLD)
    {
     if (!resourcePreview.WorldBoxes.empty())
     {
      resourcePattern.WorldOccurrences = resourcePreview.WorldBoxes;
      resourcePattern.Stages.front().iDurationMs = 600000u;
     }
     else
     {
     if (resourcePreview.Resource.strResourceId.empty())
     {
      KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION world;
      world.strWorldId = "preview.resource.world-definition";
      world.strDisplayName = resourcePreview.Resource.strDisplayName;
      world.strSequenceInstanceId = resourcePreview.Resource.strAssetId;
      world.PositionOffset = resourcePreview.Occurrence.PositionOffset;
      document.Worlds.push_back(world);
      resourcePreview.Resource.strResourceId = world.strWorldId;
     }
     KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE box;
     box.strWorldId = resourcePreview.Resource.strResourceId;
     box.strOccurrenceId = "preview.resource.world";
     box.iDurationMs = resourcePreview.Occurrence.iDurationMs;
     resourcePattern.WorldOccurrences.push_back(box);
     const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
      [&box](const auto& value) { return value.strWorldId == box.strWorldId; });
     if (world != document.Worlds.end() && !world->strCompanionEffectResourceId.empty())
     {
      KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE effect;
      effect.strOccurrenceId = "preview.resource.world.effect";
      effect.strResourceId = world->strCompanionEffectResourceId;
      effect.strWorldOccurrenceId = box.strOccurrenceId;
      effect.iDurationMs = box.iDurationMs;
      resourcePattern.PresentationOccurrences.push_back(std::move(effect));
     }
     }
    }
    else
    {
     if (resourcePreview.Resource.strResourceId.empty())
      resourcePreview.Resource.strResourceId = "preview.resource";
     resourcePreview.Occurrence.strResourceId = resourcePreview.Resource.strResourceId;
     if (resourcePreview.Occurrence.strOccurrenceId.empty())
      resourcePreview.Occurrence.strOccurrenceId = "preview.resource.occurrence";
     resourcePreview.Occurrence.iStartMs = 0u;
     const auto existing = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
      [&resourcePreview](const auto& resource) { return resource.strResourceId == resourcePreview.Resource.strResourceId; });
     if (existing == document.PresentationResources.end()) document.PresentationResources.push_back(resourcePreview.Resource);
     else *existing = resourcePreview.Resource;
     resourcePattern.PresentationOccurrences.push_back(resourcePreview.Occurrence);
     if (resourcePreview.Occurrence.strAnchorKind == "WORLD")
     {
      KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE worldBox;
      worldBox.strWorldId = resourcePreview.Occurrence.strWorldId;
      worldBox.strOccurrenceId = "preview.resource.anchor";
      worldBox.iDurationMs = resourcePreview.Occurrence.iDurationMs;
      resourcePattern.WorldOccurrences.push_back(worldBox);
     }
    }
    const auto* worldSource = CompositionPreviewWorldSource();
    bool worldRequiresActor = false;
    bool contextReady = true;
    if (resourcePreview.Resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::WORLD)
    {
     const auto* arena = CLevel_KakulSaydonArena::Get_Active();
     if (!arena) { previewRouteStatus = "World Object Preview requires the KoukuSaydon Arena."; contextReady = false; }
     else contextReady = Resolve_KoukuWorldPreviewActor(document,
      worldSource ? *worldSource : arena->Get_WorldSequenceDocument(),
      resourcePreview.strSourcePatternId.empty() ? workbench->Get_SelectedPatternId() : resourcePreview.strSourcePatternId,
      resourcePattern, worldRequiresActor, previewRouteStatus);
    }
    bool resourceStarted = false;
    if (contextReady && worldRequiresActor)
    {
     // Hand props share the exact actor and pose owner used by Pattern Play.
     // Level still owns WORLD visibility, placement and lighting restoration.
     document.Patterns.push_back(resourcePattern);
     KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
     bundle.strBundleId = resourcePattern.strPatternId;
     bundle.strGateId = resourcePattern.strGateId;
     bundle.Members.push_back({resourcePattern.strPatternId + ".member", resourcePattern.strPatternId, 0u});
     document.Bundles.push_back(std::move(bundle));
     resourceStarted = m_pKoukuPresentationPlayer->Begin_BundlePreview(document, resourcePattern.strPatternId,
      0u, !resourcePreview.WorldBoxes.empty(), previewRouteStatus, worldSource, true, true);
    }
    else if (contextReady)
     resourceStarted = m_pKoukuPresentationPlayer->Begin_Preview(document, resourcePattern, true, 0u,
      !resourcePreview.WorldBoxes.empty(), previewRouteStatus);
    if (resourceStarted)
    {
     if (m_pAnimationTool) { std::string stoppedAnimationStatus; (void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus); }
     ClaimCompositionPreviewOwner(route.owner);
     m_eDebugInputOwner = route.owner;
     if ((worldRequiresActor || !m_pKoukuPresentationPlayer->Preview_IsBundle()) &&
      !Begin_KoukuWorldPreview(document, resourcePattern, previewRouteStatus, worldSource,
      !worldRequiresActor && resourcePreview.Resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::WORLD && resourcePreview.WorldBoxes.empty()))
      StopCompositionPreview(route.owner);
    }
   }
   else previewRouteStatus = "Presentation preview requires the KoukuSaydon Arena.";
   }
  }

		/* Apply transport after a same-frame preview start, then return the
		   resulting identity and clock together to the Workbench. */
		KOUKU_PREVIEW_TRANSPORT transport = KOUKU_PREVIEW_TRANSPORT::NONE;
		std::uint32_t seekMs = 0u;
		if (workbench->Consume_PreviewTransportRequest(transport, seekMs) &&
			m_eCompositionPreviewOwner == route.owner)
		{
			const bool ownClock = m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_OwnsClock();
            bool previewResumed = false;
			if (ownClock)
			{
				if (transport == KOUKU_PREVIEW_TRANSPORT::PAUSE)
				{
					// Restore the displayed clock before freezing; Update already advanced this frame.
					m_pKoukuPresentationPlayer->Seek_Preview(seekMs);
					m_pKoukuPresentationPlayer->Pause_Preview(true);
				}
				if (transport == KOUKU_PREVIEW_TRANSPORT::RESUME)
                {
                    m_pKoukuPresentationPlayer->Pause_Preview(false);
                    previewResumed = m_pKoukuPresentationPlayer->Preview_Playing();
                }
				if (transport == KOUKU_PREVIEW_TRANSPORT::SEEK) m_pKoukuPresentationPlayer->Seek_Preview(seekMs);
			}
			else if (m_pAnimationTool)
			{
				if (transport == KOUKU_PREVIEW_TRANSPORT::PAUSE &&
					m_pAnimationTool->Seek_KoukuCompositionPreview(seekMs, m_strToolStatus))
				{
					(void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(true, m_strToolStatus);
					if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Seek_Preview(seekMs);
				}
				if (transport == KOUKU_PREVIEW_TRANSPORT::RESUME)
                    previewResumed = m_pAnimationTool->Set_KoukuCompositionPreviewPaused(false, m_strToolStatus);
				if (transport == KOUKU_PREVIEW_TRANSPORT::SEEK &&
					m_pAnimationTool->Seek_KoukuCompositionPreview(seekMs, m_strToolStatus))
				{
					if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Seek_Preview(seekMs);
				}
			}
            if (previewResumed) notifySequencePlaybackStarted();
			if (transport == KOUKU_PREVIEW_TRANSPORT::STOP)
			{
				StopCompositionPreview(route.owner);
			}
		}

		std::string serverPatternId;
		std::uint32_t sourceRevision = 0u;
		// Only the Server-admitted run epoch may replace Product presentation.
		if (workbench->Consume_ServerPlayRequest(
				serverPatternId, sourceRevision) && route.owner == DEBUG_TOOL::SEQUENCER)
		{
			if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_StopCompositionWorldPreview();
			if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Stop_Preview();
			ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::KOUKU_SAYDON_BOSS)) &&
				nullptr != m_pKoukuSaydonBossTool)
			{
				SetDebugToolVisible(DEBUG_TOOL::KOUKU_SAYDON_BOSS, true);
				(void)m_pKoukuSaydonBossTool->Play_PatternById(
					serverPatternId, sourceRevision, m_strToolStatus);
				workbench->Set_ServerPlayPreparationPending(m_pKoukuSaydonBossTool->Is_PlayPreparationPending(), m_strToolStatus);
				m_eDebugInputOwner = DEBUG_TOOL::KOUKU_SAYDON_BOSS;
				m_eDebugWindowFocusPending = DEBUG_TOOL::KOUKU_SAYDON_BOSS;
			}
			else
			{
				m_strToolStatus =
					"KoukuSaydon Boss Tool could not open for Server Play.";
				workbench->Set_ServerPlayPreparationPending(false, m_strToolStatus);
			}
		}
	}
	// Apply shared transport after every queued start. A Stop in Resources
	// must also cancel a Pattern/row start submitted during the same UI frame.
	if (nullptr != shell)
	{
		CSequencerTool::ANIMATION_PREVIEW_TRANSPORT transport =
			CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::NONE;
		const bool completeEntryPending = route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK &&
			!m_strKoukuCompletePlayFlowGate.empty();
		bool hasTransport = shell->Consume_AnimationPreviewTransportRequest(transport);
		if (hasTransport && shell->Uses_ValtanSession() && m_pAnimationTool)
		{
			const auto valtan = m_pAnimationTool->Get_ValtanCompositionPreviewState();
			if (valtan.bPlaying)
			{
				if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP)
					m_pAnimationTool->Stop_ValtanCompositionPattern(m_strToolStatus);
				else if (valtan.bSourceSequencePlaying)
				{
					const bool pause = transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::PAUSE;
					const uint32_t position = !pause && valtan.bPaused && valtan.iPositionMs >= valtan.iDurationMs ?
						0u : valtan.iPositionMs;
					(void)m_pAnimationTool->Seek_ValtanCompositionSourceSequence(position, pause, m_strToolStatus);
				}
				else
					(void)m_pAnimationTool->Seek_ValtanCompositionPattern(valtan.strPatternId,
						valtan.iPositionMs, transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::PAUSE,
						m_strToolStatus);
				shell->Set_AnimationPreviewStatus(m_strToolStatus);
				hasTransport = false;
			}
		}
		if (hasTransport &&
			(m_eCompositionPreviewOwner == route.owner ||
				(completeEntryPending && transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP)))
		{
			if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP)
			{
				if (completeEntryPending)
					CancelKoukuGateCompletePlay("Complete Play stopped before Pattern Flow.");
				else StopCompositionPreview(route.owner);
			}
            else if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::RESUME && workbench)
            {
                // Workbench validates the immutable snapshot before resuming.
                // Its transport or refreshed request is consumed next frame.
                (void)workbench->Request_PreviewResume();
            }
			else if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_OwnsClock())
			{
				if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::PAUSE) m_pKoukuPresentationPlayer->Pause_Preview(true);
				if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::RESUME) m_pKoukuPresentationPlayer->Pause_Preview(false);
			}
			else if (nullptr != m_pAnimationTool)
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
				case CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::NONE:
					break;
				}
			}
			else
				m_strToolStatus = "No animation preview backend is active.";
			shell->Set_AnimationPreviewStatus(m_strToolStatus);
		}
	}

		if (!previewRouteStatus.empty())
		{
			auto state = workbench->Get_PreviewState();
			state.strStatus = previewRouteStatus;
			workbench->Set_PreviewState(state);
			if (shell) shell->Set_AnimationPreviewStatus(previewRouteStatus);
			if (m_eCompositionPreviewOwner == route.owner)
			{ activePreviewRouteStatus = previewRouteStatus; previewStatusOwner = route.owner; }
		}
	}
	if (!m_pKoukuPresentationPlayer || !m_pKoukuPresentationPlayer->Preview_IsServerClock())
	{
	KOUKU_PREVIEW_STATE finalPreview;
	std::string worldAnchorWaitingStatus;
	std::string completedPreviewId;
    bool previewFailed = false;
    const auto rejectPreview = [&](const std::string& patternId, const std::string& error)
    {
        previewFailed = true;
        completedPreviewId.clear();
        finalPreview.strPatternId = patternId;
        finalPreview.bPlaying = finalPreview.bPaused = false;
        finalPreview.strStatus = error;
        const auto failedOwner = m_eCompositionPreviewOwner;
        if (failedOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK && !m_strKoukuCompletePlayFlowGate.empty())
            CancelKoukuGateCompletePlay(error);
        else StopCompositionPreview(failedOwner);
        for (const auto& route : compositionRoutes)
            if (route.owner == failedOwner)
            {
                if (route.workbench)
                {
                    route.workbench->Notify_SequencePreviewAdmission(false, error);
                    route.workbench->Set_PreviewState(finalPreview);
                }
                if (route.shell) route.shell->Set_AnimationPreviewStatus(error);
            }
    };
    if (m_pKoukuPresentationPlayer)
    {
        std::string failedPatternId, failureStatus;
        if (m_pKoukuPresentationPlayer->Consume_FailedPreview(failedPatternId, failureStatus))
            rejectPreview(failedPatternId, failureStatus);
        else (void)m_pKoukuPresentationPlayer->Consume_CompletedPreview(completedPreviewId);
    }
	const bool ownedClock = m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_OwnsClock();
    if (previewFailed) { /* The terminal error already owns finalPreview. */ }
	else if (!completedPreviewId.empty())
	{
		finalPreview.strPatternId = completedPreviewId;
		finalPreview.iClockMs = finalPreview.iDurationMs = m_pKoukuPresentationPlayer->Preview_DurationMs();
		finalPreview.strStatus = "Sequence preview completed.";
	}
	else if (ownedClock)
	{
		finalPreview.strPatternId = m_pKoukuPresentationPlayer->Preview_PatternId();
		finalPreview.bPlaying = m_pKoukuPresentationPlayer->Preview_Playing();
		finalPreview.bPaused = m_pKoukuPresentationPlayer->Preview_Paused();
		finalPreview.iClockMs = m_pKoukuPresentationPlayer->Preview_ClockMs();
		finalPreview.iDurationMs = m_pKoukuPresentationPlayer->Preview_DurationMs();
		finalPreview.strStatus = m_pKoukuPresentationPlayer->Status();
	}
	else if (m_pAnimationTool)
	{
		const auto sampled = m_pAnimationTool->Get_KoukuCompositionPreviewState();
		finalPreview.strPatternId = sampled.strPatternId; finalPreview.bPlaying = sampled.bPlaying;
		finalPreview.bPaused = sampled.bPaused; finalPreview.iClockMs = sampled.iClockMs;
		finalPreview.iDurationMs = sampled.iDurationMs; finalPreview.strStatus = sampled.strStatus;
	}
    if (!previewFailed && finalPreview.bPlaying && m_pKoukuPresentationPlayer &&
        !m_pKoukuPresentationPlayer->Preview_IsBundle())
    {
        std::uint32_t effectiveMs = finalPreview.iClockMs;
        if (!m_pKoukuPresentationPlayer->Resolve_PreviewCaptureClock(finalPreview.iClockMs, effectiveMs))
        {
            const std::string captureError = m_pKoukuPresentationPlayer->Status();
            rejectPreview(finalPreview.strPatternId, captureError);
        }
        else
        {
            // WORLD, animation and Camera must render the capture boundary
            // together before the user's requested cursor can be displayed.
            if (effectiveMs != finalPreview.iClockMs && !ownedClock && m_pAnimationTool)
            {
                std::string captureStatus;
                if (!m_pAnimationTool->Seek_KoukuCompositionPreview(effectiveMs, captureStatus))
                    rejectPreview(finalPreview.strPatternId, captureStatus);
            }
            finalPreview.iClockMs = effectiveMs;
        }
    }
	if (auto* arena = CLevel_KakulSaydonArena::Get_Active(); !previewFailed && arena && (!m_pKoukuPresentationPlayer || !m_pKoukuPresentationPlayer->Preview_IsBundle()))
	{
		std::string worldPreviewStatus;
		if (!arena->Debug_SampleCompositionWorldPreview(finalPreview.strPatternId,
			finalPreview.bPlaying, finalPreview.iClockMs, worldPreviewStatus))
		{
            rejectPreview(finalPreview.strPatternId, worldPreviewStatus);
		}
		else worldAnchorWaitingStatus = worldPreviewStatus;
	}
	if (!previewFailed && m_pKoukuPresentationPlayer && !m_pKoukuPresentationPlayer->Preview_IsBundle())
	{
		float4x4_t pivot{};
		const bool colliderResource = m_pKoukuPresentationPlayer->Preview_IsColliderResource();
		auto model = colliderResource ? std::shared_ptr<Engine::CModel>{} : CAnimationTargetService::Resolve_Model();
		bool pivotReady = !colliderResource && CAnimationTargetService::Resolve_RootTransform(&pivot);
		if (!pivotReady)
		{
			const auto* arena = CLevel_KakulSaydonArena::Get_Active();
			const auto character = arena ? arena->Get_LocalCharacter() : nullptr;
			if (character && character->Get_Transform())
			{
				pivot = *character->Get_Transform()->Get_WorldMatrixPtr();
				model = character->Get_BodyModel();
				pivotReady = true;
			}
		}
		if (pivotReady && finalPreview.strPatternId == m_pKoukuPresentationPlayer->Preview_PatternId())
		{
			m_pKoukuPresentationPlayer->Sample_Preview(finalPreview.iClockMs,
				finalPreview.bPlaying, finalPreview.bPaused, pivot, model);
			if (!m_pKoukuPresentationPlayer->Status().empty())
				finalPreview.strStatus = m_pKoukuPresentationPlayer->Status();
		}
		else if (!pivotReady && finalPreview.bPlaying)
			finalPreview.strStatus = "Preview requires an active arena character or animation target.";
	}
	if (!previewFailed && m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
	{
		std::string arrivalStatus;
		if (!SampleKoukuSequenceArrivals(finalPreview.iClockMs,
			(finalPreview.bPlaying && !finalPreview.bPaused) || !completedPreviewId.empty(), arrivalStatus))
			rejectPreview(finalPreview.strPatternId, arrivalStatus);
		else
		{
			if (!completedPreviewId.empty())
				m_strKoukuSequenceArrivalCompletedPreview = completedPreviewId;
			if (!m_strKoukuSequenceArrivalCompletedPreview.empty())
			{
				if (KoukuSequenceArrivalsPending())
				{
					completedPreviewId.clear();
					finalPreview.strStatus = "Sequence finished; waiting for Server player arrival approval.";
				}
				else completedPreviewId = std::exchange(m_strKoukuSequenceArrivalCompletedPreview, {});
			}
		}
	}
	if (previewStatusOwner == m_eCompositionPreviewOwner && !activePreviewRouteStatus.empty())
		finalPreview.strStatus = activePreviewRouteStatus;
	if (!previewFailed && !worldAnchorWaitingStatus.empty())
		finalPreview.strStatus = worldAnchorWaitingStatus;
	for (const auto& route : compositionRoutes)
	{
		if (route.owner != m_eCompositionPreviewOwner) continue;
		if (route.workbench)
		{
			route.workbench->Set_PreviewState(finalPreview);
			std::string completedGate;
			if (!completedPreviewId.empty() && route.workbench->Advance_CompleteSequencePlay(completedPreviewId, &completedGate))
			{
				finalPreview.strStatus = route.workbench->Get_Status();
				if (!route.workbench->Is_CompleteSequencePlaying())
				{
					m_bKoukuCompletePreservesArrivalPosition = std::any_of(m_KoukuSequenceArrivalCues.begin(),
						m_KoukuSequenceArrivalCues.end(), [](const auto& cue) { return cue.moved; });
					StopCompositionPreview(route.owner);
					if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_ReturnToPlayerCamera();
				}
				if (!completedGate.empty() && route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
					FinishKoukuGateCompletePlay(completedGate, finalPreview.strStatus);
				route.workbench->Set_PreviewState(finalPreview);
			}
			else if (!completedPreviewId.empty() && route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
			{
				StopCompositionPreview(route.owner);
				if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_ReturnToPlayerCamera();
				m_eDebugInputOwner = DEBUG_TOOL::NONE;
				finalPreview.strStatus = "Sequence finished. Player camera restored; combat was not started.";
				route.workbench->Set_PreviewState(finalPreview);
			}
		}
		if (!route.shell) continue;
		CSequencerTool::ANIMATION_PREVIEW_STATE state;
		state.strPatternId = finalPreview.strPatternId; state.strStatus = finalPreview.strStatus;
		state.bPlaying = finalPreview.bPlaying; state.bPaused = finalPreview.bPaused;
		state.iClockMs = finalPreview.iClockMs; state.iDurationMs = finalPreview.iDurationMs;
		route.shell->Set_AnimationPreviewState(std::move(state));
	}

	if (m_pSequencerTool && m_pSequencerTool->Uses_ValtanSession() && m_pAnimationTool)
	{
		const auto valtan = m_pAnimationTool->Get_ValtanCompositionPreviewState();
		// Publish the stopped state too; a direct Workbench preview may never
		// have claimed the generic resource-preview route above.
		if (valtan.bPlaying || !finalPreview.bPlaying)
		{
			CSequencerTool::ANIMATION_PREVIEW_STATE state;
			state.strPatternId = valtan.strPatternId;
			state.strStatus = valtan.strStatus;
			state.bPlaying = valtan.bPlaying; state.bPaused = valtan.bPaused;
			state.iClockMs = valtan.iPositionMs; state.iDurationMs = valtan.iDurationMs;
			m_pSequencerTool->Set_AnimationPreviewState(std::move(state));
		}
	}
	}
	// Re-sample after this frame's typed Seek/Stop so model, environment and
	// camera agree before Render. Always retire the camera override on handoff.
	std::vector<BOSS_STAGE_ENVIRONMENT_SAMPLE> currentValtanSamples;
	CValtan::Collect_StageEnvironmentSamples(currentValtanSamples);
	if (m_pKoukuPresentationPlayer)
		m_pKoukuPresentationPlayer->Update_BossStageEnvironments(currentValtanSamples);
	if (m_pCameraTool)
	{
		const bool activeValtanSession = m_pSequencerTool && m_pSequencerTool->Uses_ValtanSession() &&
			m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) &&
			m_pAnimationTool && m_pAnimationTool->Get_ValtanCompositionPreviewState().bPlaying;
		const auto local = std::find_if(currentValtanSamples.begin(), currentValtanSamples.end(),
			[](const auto& sample) { return sample.bPreview; });
		if (activeValtanSession && local != currentValtanSamples.end())
		{
			std::string cameraStatus;
			if (!m_pCameraTool->Sample_CompositionPreview(*local, cameraStatus) && !cameraStatus.empty())
				m_strToolStatus = std::move(cameraStatus);
		}
		else m_pCameraTool->Stop_CompositionPreview();
	}
    if (m_pCharacterActionWorkbench)
        m_pCharacterActionWorkbench->Update(fTimeDelta, m_bDeveloperToolsVisible &&
            IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) && m_pSequencerTool &&
            m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::CHARACTER &&
            m_eDebugInputOwner == DEBUG_TOOL::SEQUENCER);
	if (m_pWorldObjectTool)
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.WorldObjects.Update");
			m_pWorldObjectTool->Update(fTimeDelta, m_bDeveloperToolsVisible &&
			IsDebugToolVisible(DEBUG_TOOL::WORLD_OBJECT) && DEBUG_TOOL::WORLD_OBJECT == m_eDebugInputOwner);
		}
	if (m_pWorldObjectTool)
	{
		std::string patternId, status;
		uint32_t sourceRevision = 0u;
		if (m_pWorldObjectTool->Consume_ServerPlayRequest(patternId, sourceRevision))
		{
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::KOUKU_SAYDON_BOSS)) && m_pKoukuSaydonBossTool)
			{
				if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_StopCompositionWorldPreview();
				if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Stop_Preview();
				ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
				(void)m_pKoukuSaydonBossTool->Play_PatternById(patternId, sourceRevision, status);
			}
			else status = "KoukuSaydon Boss Tool could not prepare Server collision playback.";
			m_pWorldObjectTool->Set_ServerPlayStatus(std::move(status));
		}
		if (m_pWorldObjectTool->Consume_ServerStopRequest())
		{
			if (m_pKoukuSaydonBossTool) (void)m_pKoukuSaydonBossTool->Cancel_PlayPreparation(status);
			(void)CKoukuSaydonPatternAuditionService::Get().Stop(status);
			m_pWorldObjectTool->Set_ServerPlayStatus(std::move(status));
		}
		m_pWorldObjectTool->Set_ServerPlayPreparationPending(
			m_pKoukuSaydonBossTool && m_pKoukuSaydonBossTool->Is_PlayPreparationPending());
	}
	RefreshWorldObjectResources();
	if (nullptr != m_pCameraTool)
	{
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Camera.Update");
			m_pCameraTool->Update(
			fTimeDelta,
			m_bDeveloperToolsVisible &&
			IsDebugToolVisible(DEBUG_TOOL::CAMERA) &&
			DEBUG_TOOL::CAMERA == m_eDebugInputOwner);
		}
	}
	}
#endif

	// 현재 Level의 Update가 끝난 뒤에만 기존 Level을 파괴한다.
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.LevelAndEnvironment.Update");
    string environmentStatus;
    const auto* cinematicArena = CLevel_KakulSaydonArena::Get_Active();
    const bool_t koukuCinematic = cinematicArena &&
        CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA) &&
        cinematicArena->Is_CinematicPresentationActive();
    const auto* valtanArena = CLevel_ValtanArena::Get_Active();
    const bool_t valtanCinematic = valtanArena &&
        CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA) &&
        valtanArena->Is_CinematicCameraActive();
    const bool_t marioStage = cinematicArena &&
        CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA) &&
        cinematicArena->Is_LocalMarioStageActive();
    const auto localCharacter = CAnimationTargetService::Resolve_SceneCharacter();
    float vehicleBrightness = localCharacter ? localCharacter->Get_VehicleDirectionalBrightness() : 1.f;
    float controlsBrightness=1.f;float4_t controlsColor{};
    const auto selectedCharacter=CAnimationTargetService::Resolve_Character();
    if(!(selectedCharacter&&selectedCharacter->Get_PresentationDirectionalControl(controlsBrightness,controlsColor)) && localCharacter)
        localCharacter->Get_PresentationDirectionalControl(controlsBrightness,controlsColor);
    vehicleBrightness=std::clamp(vehicleBrightness*controlsBrightness,0.f,16.f);
    if (!m_RenderingProfiles.Apply_CameraEnvironment(fTimeDelta, environmentStatus,
        koukuCinematic || valtanCinematic || marioStage, nullptr, vehicleBrightness, &controlsColor))
        OutputDebugStringA((environmentStatus + "\n").c_str());
	Apply_LevelRequest();
	}

	/* Every shown runtime surface now declares where it covers the screen and on which layer,
	so each text group can be hidden exactly where a surface above it covers it. */
	Sync_KoukuCinematicUI();
	if (!CUIInputRouter::Get().Is_CinematicSuppressed()) Register_UITextOccluders();
}

void CMainApp::Register_UITextOccluders()
{
	CUITextOcclusion& Occlusion = CUITextOcclusion::Get();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	/* Windows stack in the order CMainApp builds their sprites; each gets its own step. */
	if (nullptr != m_pInventoryView && m_pInventoryView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_INVENTORY, fX, fY, fWidth, fHeight);
	if (nullptr != m_pCharacterInfoView && m_pCharacterInfoView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_CHARACTER_INFO, fX, fY, fWidth, fHeight);
	if (nullptr != m_pAvatarBookView && m_pAvatarBookView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_AVATAR_BOOK, fX, fY, fWidth, fHeight);
	if (nullptr != m_pVehicleWindowView && m_pVehicleWindowView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_VEHICLE, fX, fY, fWidth, fHeight);
	if (nullptr != m_pHonorTitleWindowView && m_pHonorTitleWindowView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_HONOR_TITLE, fX, fY, fWidth, fHeight);
	if (nullptr != m_pWorldMapWindowView && m_pWorldMapWindowView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_WORLD_MAP, fX, fY, fWidth, fHeight);
	if (nullptr != m_pSystemOptionView && m_pSystemOptionView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_SYSTEM_OPTION, fX, fY, fWidth, fHeight);
	if (m_bItemUpgradePreviewVisible && nullptr != m_pItemUpgradeView)
		Occlusion.Add_SlotOccluder(UI_TEXT_LAYER::WINDOW_ITEM_UPGRADE, *m_pItemUpgradeView, "ItemUpgrade_PanelBg");
	/* HUD surfaces in the levels that show them. */
	const uint32_t iLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (nullptr != m_pPartyWindowView && (ETOUI(LEVEL::BERN) == iLevel ||
		ETOUI(LEVEL::VALTAN_ARENA) == iLevel || ETOUI(LEVEL::KAKULSAYDON_ARENA) == iLevel))
		m_pPartyWindowView->Register_TextOccluders();
	/* Full-screen surfaces: the award page and the customizing screen (Bern registers its
	   own raid entry window). */
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (Is_MvpResultPageOpen())
		Occlusion.Add_Occluder(UI_TEXT_LAYER::PAGE, 0.f, 0.f, vViewport.x, vViewport.y);
	if (ETOUI(LEVEL::CHARACTER_SELECT) == iLevel && nullptr != CLevel_CharacterSelect::Get_Active() &&
		CLevel_CharacterSelect::Get_Active()->Is_CustomizingOpen())
		Occlusion.Add_Occluder(UI_TEXT_LAYER::MODAL, 0.f, 0.f, vViewport.x, vViewport.y);
}

HRESULT CMainApp::Render()
{
    const auto recordRenderFailure = [this](const char* stage, HRESULT result) noexcept
    {
        try
        {
            CNetworkManager::Get().Record_SessionEvent("render.failed",
                std::string("stage=") + stage + "; hresult=" +
                std::to_string(static_cast<unsigned long>(result)) +
                "; deviceRemovedReason=" + std::to_string(static_cast<unsigned long>(
                    m_pDevice ? m_pDevice->GetDeviceRemovedReason() : E_POINTER)));
        }
        catch (...) { } // Preserve the original rendering failure and transport state.
    };
	Sync_KoukuCinematicUI();
	float4_t clearColor = { 0.008f, 0.012f, 0.025f, 1.f };
	HRESULT hBeginResult;
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.BeginFrame");
		Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.BeginFrame");
		hBeginResult = CGameInstance::Get().Render_Begin(&clearColor);
	}
	if (FAILED(hBeginResult))
	{
        recordRenderFailure("begin-frame", hBeginResult);
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return hBeginResult;
	}

	/* The character info window's live portrait draws into its own target here, before the
	world/UI pass whose CI_Preview sprite samples it. */
	if (!CUIInputRouter::Get().Is_CinematicSuppressed())
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.Portraits");
		Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.Portraits");
	if (nullptr != m_pCharacterInfoView)
		(void)m_pCharacterInfoView->Render_Portrait();
	if (nullptr != m_pAvatarBookView)
		(void)m_pAvatarBookView->Render_Portrait();
	if (auto* pArena = CLevel_KakulSaydonArena::Get_Active();
		nullptr != pArena &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
		pArena->Render_MvpPortraits();
	if (auto* pValtan = CLevel_ValtanArena::Get_Active();
		nullptr != pValtan &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
		pValtan->Render_MvpPortraits();
	}

	// Composition WORLD/Seek/Stop has committed this frame before choosing the map-light owner.
	if (auto* arena = CLevel_KakulSaydonArena::Get_Active(); arena &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
	{
		arena->Submit_MapLightFrame();
		arena->Trace_CinematicPresentation(m_RenderingProfiles.Get_ActiveProfileId());
		arena->Submit_EntranceTriggerMarkers();
	}
	if (auto* pValtanArena = CLevel_ValtanArena::Get_Active(); pValtanArena &&
		CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
		pValtanArena->Submit_TriggerMarkers();
	HRESULT hWorldResult;
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.World");
		/* Level text (nameplates, chat bubbles) is world text: under every UI surface. */
		CUITextOcclusion::Get().Apply(UI_TEXT_LAYER::WORLD);
		hWorldResult = CGameInstance::Get().Render();
	}
	if (FAILED(hWorldResult))
	{
        recordRenderFailure("world", hWorldResult);
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return hWorldResult;
	}

	if (nullptr != m_pImGuiLayer)
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "ImGui.BuildAndSubmit");
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
		/* The customizing screen is a full-screen product overlay on the same Level: the class
		HUD chrome underneath has to stop drawing for it exactly the way it does for the Debug
		raid-entry preview above. */
		const bool_t isCharSelectOverlayOpen = isCharSelectDebugPreviewOpen ||
			(ETOUI(LEVEL::CHARACTER_SELECT) == hudLevel &&
				nullptr != CLevel_CharacterSelect::Get_Active() &&
				CLevel_CharacterSelect::Get_Active()->Is_CustomizingOpen());
		if (!CUIInputRouter::Get().Is_CinematicSuppressed() && nullptr != m_pHUDLayoutTool && hudPlayer.isValid &&
			supportsAuthoredHUD && !skillWindowOpenForPreview &&
			!isCharSelectOverlayOpen)
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
		if (!CUIInputRouter::Get().Is_CinematicSuppressed())
		{
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
			/* Received lines, sender included, reach the log through the active level's own
			replication -- the Server broadcast is the single source, so a line shows the same
			Server nickname on every client. */
			std::vector<CClientReplication::CHAT_LINE> chatLines;
			if (ETOUI(LEVEL::BERN) == chatLevel)
			{
				CLevel_Bern* pBern = CLevel_Bern::Get_Active();
				if (nullptr != pBern)
					pBern->Drain_ChatLines(chatLines);
				for (const CClientReplication::CHAT_LINE& Line : chatLines)
					m_pChatWindowView->Append_ReceivedLine(Line.strNickname, Line.strText);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Chat.Build");
					m_pChatWindowView->Render(
					nullptr != pBern ? pBern->Get_PlayerCommandSink() : nullptr);
				}
			}
			else if (ETOUI(LEVEL::VALTAN_ARENA) == chatLevel)
			{
				CLevel_ValtanArena* pValtanArena = CLevel_ValtanArena::Get_Active();
				if (nullptr != pValtanArena)
					pValtanArena->Drain_ChatLines(chatLines);
				for (const CClientReplication::CHAT_LINE& Line : chatLines)
					m_pChatWindowView->Append_ReceivedLine(Line.strNickname, Line.strText);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Chat.Build");
					m_pChatWindowView->Render(
					nullptr != pValtanArena ?
						pValtanArena->Get_PlayerCommandSink() : nullptr);
				}
			}
			else if (ETOUI(LEVEL::KAKULSAYDON_ARENA) == chatLevel)
			{
				CLevel_KakulSaydonArena* pKouku = CLevel_KakulSaydonArena::Get_Active();
				if (nullptr != pKouku)
					pKouku->Drain_ChatLines(chatLines);
				for (const CClientReplication::CHAT_LINE& Line : chatLines)
					m_pChatWindowView->Append_ReceivedLine(Line.strNickname, Line.strText);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Chat.Build");
					m_pChatWindowView->Render(
						nullptr != pKouku ? pKouku->Get_PlayerCommandSink() : nullptr);
				}
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
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Party.Build");
					m_pPartyWindowView->Render();
				}
			}
			else if (ETOUI(LEVEL::VALTAN_ARENA) == partyLevel)
			{
				if (CLevel_ValtanArena* pValtanArena = CLevel_ValtanArena::Get_Active())
					m_pPartyWindowView->Sync_From_Roster(
						pValtanArena->Get_PartyRoster(), pValtanArena->Get_PlayerHealth());
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Party.Build");
					m_pPartyWindowView->Render();
				}
			}
			else if (ETOUI(LEVEL::KAKULSAYDON_ARENA) == partyLevel)
			{
				if (CLevel_KakulSaydonArena* pKoukuArena = CLevel_KakulSaydonArena::Get_Active())
					m_pPartyWindowView->Sync_From_Roster(
						pKoukuArena->Get_PartyRoster(), pKoukuArena->Get_PlayerHealth());
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Party.Build");
					m_pPartyWindowView->Render();
				}
			}
		}
		}
#ifdef _DEBUG
		if (!m_pLevelNavigationDebug)
			m_pLevelNavigationDebug = std::make_unique<CLevelNavigationDebug>();
		// MapTool owns its selected Area inside the Development editor workspace.
		m_pLevelNavigationDebug->Sync_Level(CMapEditorWorkspaceService::Is_Active() ?
			LEVEL::END : static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID()));
		m_pLevelNavigationDebug->Render_Overlay();
		if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Render_Debug();
		if (nullptr != m_pRenderingBenchmark)
			m_pRenderingBenchmark->Update(CGameInstance::Get().Get_Profiler());
		if (m_bDeveloperToolsVisible)
		{
			Engine::CProfilerScope developerToolsScope(
				CGameInstance::Get().Get_Profiler(), "ImGui.DeveloperTools");
			RenderDeveloperTools();
			RenderWorldLevelTool();
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
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Map.Build");
					m_pMapTool->Render();
				}
			}
			/* Report the authoring dirty state the integrated Save button shows,
			   and run the ask it raised. Source only: no publisher here. */
			if (nullptr != m_pMapTool && m_pMapTool->Is_IntegratedCutsceneViewOpen())
			{
				/* The integrated Save owns the KoukuSaydon composition only.
				   Valtan keeps its own split-source save in its workbench, so
				   claim nothing here while that Area is hosted. */
				const bool_t koukuHosted = m_pMapTool->Get_HostedCompositionSession() ==
					static_cast<ICompositionWorkbenchSession*>(m_pSequenceActionWorkbench.get());
				const bool_t sequenceDirty = koukuHosted &&
					m_pSequenceActionWorkbench &&
					m_pSequenceActionWorkbench->Is_Dirty();
				/* WorldObjectTool edits the Kouku Area only (its AREA_ID is
				   fixed), so its draft is saved here only while Kouku is hosted.
				   A Valtan Save must never write the Kouku object document. */
				const bool_t objectDirty = koukuHosted && m_pWorldObjectTool &&
					m_pWorldObjectTool->Is_Dirty();
				std::string integratedStatus;
				if (m_pMapTool->Consume_IntegratedSaveRequest())
				{
					bool_t saved = true;
					if (sequenceDirty && m_pSequenceActionWorkbench)
						saved = m_pSequenceActionWorkbench->Save(integratedStatus) && saved;
					if (objectDirty && m_pWorldObjectTool)
					{
						/* false: authoring sources only. The Area publisher and the
						   linked battle pattern publish stay explicit actions. */
						saved = m_pWorldObjectTool->Save_Source(false) && saved;
					}
					if (integratedStatus.empty())
						integratedStatus = saved ?
							"Saved authoring sources. Runtime data was not published." :
							"Save failed; the existing authoring edits are preserved.";
				}
				m_pMapTool->Set_IntegratedSaveState(sequenceDirty, objectDirty,
					std::move(integratedStatus));
			}
			/* Map Tool renders first, so the shell learns here whether the
			   Sequence session frame was already opened this frame. */
			if (nullptr != m_pSequencerTool)
			{
				/* Suppress the session the Map Tool actually opened, which
				   follows the Area it is editing, not a fixed one. */
				m_pSequencerTool->Suppress_SessionFrameThisFrame(
					nullptr != m_pMapTool ?
					m_pMapTool->Get_HostedCompositionSession() : nullptr,
					nullptr != m_pMapTool ? m_pMapTool->Get_HostedObjectSession() : nullptr);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) && nullptr != m_pSequencerTool)
			{
				focusNextWindow(DEBUG_TOOL::SEQUENCER);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Composition.Build");
					m_pSequencerTool->Render();
				}
                if (m_pCharacterActionWorkbench && m_pCharacterActionWorkbench->Consume_InteractionRequest() &&
                    m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::CHARACTER)
                    m_eDebugInputOwner = DEBUG_TOOL::SEQUENCER;
				if (m_pWorldObjectTool && m_pWorldObjectTool->Consume_InteractionRequest() &&
					m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::OBJECT)
					m_eDebugInputOwner = DEBUG_TOOL::WORLD_OBJECT;
				if (!m_pSequencerTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::SEQUENCER, false);
			}
			for (auto* workbench : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
			{
				KOUKU_WORLD_OBJECT_EDIT_REQUEST request;
				if (!workbench || !workbench->Consume_WorldObjectEditRequest(request)) continue;
				std::string status;
				if (FAILED(EnsureDebugTool(DEBUG_TOOL::WORLD_OBJECT)) || !m_pWorldObjectTool)
					status = "Object Tool is unavailable. Existing authoring edits are preserved.";
				else
				{
					std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> placement;
					if (request.PreviewPlacement)
					{
						const auto& value = *request.PreviewPlacement;
						placement = CWorldSequencePlayer::OBJECT_PLACEMENT{
							{float(value.Position[0]), float(value.Position[1]), float(value.Position[2])},
							{float(value.RotationDegrees[0]), float(value.RotationDegrees[1]), float(value.RotationDegrees[2])},
							{float(value.Scale[0]), float(value.Scale[1]), float(value.Scale[2])}};
					}
					(void)m_pWorldObjectTool->Open_ObjectMotion(request.strObjectId, request.strMotionInstanceId, status, placement);
				}
				workbench->Notify_WorldObjectEditResult(std::move(status));
			}
			RenderMapEffectPlacementMarker();
			if (IsDebugToolVisible(DEBUG_TOOL::ANIMATION) &&
				nullptr != m_pAnimationTool)
			{
				focusNextWindow(DEBUG_TOOL::ANIMATION);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Animation.Build");
					m_pAnimationTool->Render();
				}
			}


			if (IsDebugToolVisible(DEBUG_TOOL::EFFECT) && m_pEffectTool)
			{
				focusNextWindow(DEBUG_TOOL::EFFECT);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.EffectV1.Build");
					m_pEffectTool->Render();
				}
				if (m_pEffectTool->Consume_AuthoringInteraction())
					m_eDebugInputOwner = DEBUG_TOOL::EFFECT;
			}
			if (IsDebugToolVisible(DEBUG_TOOL::EFFECT_V2) && m_pEffectToolV2)
			{
				focusNextWindow(DEBUG_TOOL::EFFECT_V2);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.EffectV2.Build");
					m_pEffectToolV2->Render();
				}
				if (m_pEffectToolV2->Consume_AuthoringInteraction())
					m_eDebugInputOwner = DEBUG_TOOL::EFFECT_V2;
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
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.CompositionProfiler.Build");
					m_pProfilerTool->Render(CGameInstance::Get().Get_Profiler());
				}
				if (!m_pProfilerTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::PROFILER, false);
			}
			/* Skill Window's slots are still authored by their existing UI owner;
			   rendering it alongside other tools does not create a second UI runtime. */
			if (IsDebugToolVisible(DEBUG_TOOL::UI) && nullptr != m_pHUDLayoutTool)
			{
				focusNextWindow(DEBUG_TOOL::UI);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.HUDLayout.Build");
					m_pHUDLayoutTool->Render();
				}
			}
			if (IsDebugToolVisible(DEBUG_TOOL::BALANCE) && nullptr != m_pBalanceTool)
			{
				focusNextWindow(DEBUG_TOOL::BALANCE);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Balance.Build");
					m_pBalanceTool->Render();
				}
			}
			if (IsDebugToolVisible(DEBUG_TOOL::VALTAN_BOSS) && nullptr != m_pValtanBossTool)
			{
				focusNextWindow(DEBUG_TOOL::VALTAN_BOSS);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.ValtanBoss.Build");
					m_pValtanBossTool->Render();
				}
				if (!m_pValtanBossTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::VALTAN_BOSS, false);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::KOUKU_SAYDON_BOSS) &&
				nullptr != m_pKoukuSaydonBossTool)
			{
				focusNextWindow(DEBUG_TOOL::KOUKU_SAYDON_BOSS);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.KoukuBoss.Build");
					m_pKoukuSaydonBossTool->Render();
				}
				if (!m_pKoukuSaydonBossTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::KOUKU_SAYDON_BOSS, false);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::VALTAN_LOGIC_PATTERN) && nullptr != m_pValtanBossTool)
			{
				focusNextWindow(DEBUG_TOOL::VALTAN_LOGIC_PATTERN);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.ValtanBoss.Render_LogicPatternWindow");
					m_pValtanBossTool->Render_LogicPatternWindow();
				}
				if (!m_pValtanBossTool->Is_LogicPatternOpen())
					SetDebugToolVisible(DEBUG_TOOL::VALTAN_LOGIC_PATTERN, false);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::CAMERA) && nullptr != m_pCameraTool)
			{
				focusNextWindow(DEBUG_TOOL::CAMERA);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Camera.Build");
					m_pCameraTool->Render();
				}
			}
			if (IsDebugToolVisible(DEBUG_TOOL::EQUIPMENT) &&
				nullptr != m_pEquipmentAuthoringTool)
			{
				focusNextWindow(DEBUG_TOOL::EQUIPMENT);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Equipment.Build");
					m_pEquipmentAuthoringTool->Render(
					DEBUG_TOOL::EQUIPMENT == m_eDebugInputOwner);
				}
			}

			if (m_bProfilerVisible)
			{
				RenderProfilerOverlay();
				RenderProfilerSettings();
			}
		}
#endif
		{
			Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "ImGui.BackendSubmit");
		Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "ImGui.BackendSubmit");
			m_pImGuiLayer->EndFrame();
		}
	}
	/* Raid entry popup's live boss: over the popup's own sprites, under the Draw_Text pass
	   that letters it. */
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.BossShowcase");
		Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.BossShowcase");
	if (!CUIInputRouter::Get().Is_CinematicSuppressed()) CRaidBossShowcaseService::Render(m_pDevice, m_pContext);
	}
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.UIText");
		Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.UIText");
	if (!Is_RuntimeUIScreenSuppressed())
	{
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
	/* The customizing screen is a full-screen product overlay on the same Level: the class
	HUD chrome underneath has to stop drawing for it exactly the way it does for the Debug
	raid-entry preview above. */
	const bool_t isCharSelectOverlayOpen = isCharSelectDebugPreviewOpenForText ||
		(ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID() &&
			nullptr != CLevel_CharacterSelect::Get_Active() &&
			CLevel_CharacterSelect::Get_Active()->Is_CustomizingOpen());
	if (!isCharSelectOverlayOpen && !Is_MvpResultPageOpen())
	{
		CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
		RenderCombatHUDText();
		RenderBossHealthBarText();
		if (nullptr != m_pDungeonTimerView)
			m_pDungeonTimerView->Render();
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
	{
		CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
		RenderDeadSceneText();
		RenderRaidClearText();
		RenderItemAnnounceText();
	}
	RenderDamageNumbers();
	{
		CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
		if (nullptr != m_pCombatAnalysisView)
			m_pCombatAnalysisView->Render_Text();
	}
	{
		CUITextLayerScope WindowText(UI_TEXT_LAYER::WINDOW_INVENTORY);
		if (nullptr != m_pInventoryView)
			m_pInventoryView->Render_Text();
	}
	{
		CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
		RenderLobbyButtonText();
		RenderCharacterSelectWindowText();
		if (!Is_MvpResultPageOpen())
			RenderMinimapText();
	}
	{
		CUITextLayerScope TopText(UI_TEXT_LAYER::PAGE);
		RenderFpsText();
	}
	{
		CUITextLayerScope WindowText(UI_TEXT_LAYER::WINDOW_ITEM_UPGRADE);
		RenderItemUpgradeButtonText();
		RenderItemUpgradeLevelText();
		RenderItemUpgradeMaterialCounts();
		RenderItemUpgradeGaugePercentText();
		RenderItemUpgradeResultWaitText();
		RenderItemUpgradeSuccessDetailText();
		RenderItemUpgradeFailDetailText();
		RenderItemUpgradeListText();
	}
	if (ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID())
	{
		if (CLevel_CharacterSelect* pCharacterSelect = CLevel_CharacterSelect::Get_Active())
		{
			// Same gate as Update_ArenaSpawnButtons's own image draw -- these are
			// its text labels, drawn from this separate text pass.
			if (!isCharSelectOverlayOpen)
			{
				CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
				pCharacterSelect->Render_ArenaSpawnLabels();
			}
			/* Outside that gate: the nickname step opens from the customizing screen, so the
			gate that hides the spawn captions would take every glyph of this modal with it. */
			CUITextLayerScope ModalText(UI_TEXT_LAYER::MODAL);
			pCharacterSelect->Render_CreateCharacterModalText();
			pCharacterSelect->Render_CustomizingText();
#ifdef _DEBUG
			pCharacterSelect->Render_RaidEntryDebugPreviewText();
#endif
		}
	}
	if (ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID())
	{
		if (CLevel_Bern* pBern = CLevel_Bern::Get_Active())
		{
			CUITextLayerScope ModalText(UI_TEXT_LAYER::MODAL);
			pBern->Render_ValtanEntryModalText();
			pBern->Render_PartyInviteText();
		}
	}
	else if (ETOUI(LEVEL::VALTAN_ARENA) == CGameInstance::Get().Get_CurrentLevelID())
	{
		if (CLevel_ValtanArena* pValtanArena = CLevel_ValtanArena::Get_Active())
		{
			CUITextLayerScope ModalText(UI_TEXT_LAYER::MODAL);
			pValtanArena->Render_PartyInviteText();
		}
	}
	CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
	/* The chat sprites are only driven in the levels that own a chat (see the Render call
	above); its labels follow them, or the channel label and the last lines sit over the
	loading screen and the next level. */
	{
		const uint32_t chatTextLevel = CGameInstance::Get().Get_CurrentLevelID();
		const bool_t bChatTextLevel = ETOUI(LEVEL::BERN) == chatTextLevel ||
			ETOUI(LEVEL::VALTAN_ARENA) == chatTextLevel ||
			ETOUI(LEVEL::KAKULSAYDON_ARENA) == chatTextLevel;
		if (nullptr != m_pChatWindowView && bChatTextLevel)
			m_pChatWindowView->RenderText();
	}
	/* The roster sprites only draw in the levels above; the labels follow them, or the
	   member names of the last room sit over the loading screen and the next level. */
	{
		const uint32_t partyTextLevel = CGameInstance::Get().Get_CurrentLevelID();
		const bool_t bPartyTextLevel = ETOUI(LEVEL::BERN) == partyTextLevel ||
			ETOUI(LEVEL::VALTAN_ARENA) == partyTextLevel ||
			ETOUI(LEVEL::KAKULSAYDON_ARENA) == partyTextLevel;
		if (nullptr != m_pPartyWindowView && bPartyTextLevel)
			m_pPartyWindowView->RenderText();
	}

	/* The runtime windows' labels, each on its own window layer (the steps
	Register_UITextOccluders gave them): a window's text is hidden exactly where a window
	stacked above it covers it. None of them draw over the loading screen -- their sprites are
	hidden there, so their labels would be the only thing left on it. */
	if (ETOUI(LEVEL::LOADING) != CGameInstance::Get().Get_CurrentLevelID())
	{
	CUITextOcclusion& Occlusion = CUITextOcclusion::Get();
	Occlusion.Apply(UI_TEXT_LAYER::WINDOW_CHARACTER_INFO);
	if (nullptr != m_pCharacterInfoView)
		m_pCharacterInfoView->Render_Text();
	Occlusion.Apply(UI_TEXT_LAYER::WINDOW_AVATAR_BOOK);
	if (nullptr != m_pAvatarBookView)
		m_pAvatarBookView->Render_Text();
	Occlusion.Apply(UI_TEXT_LAYER::WINDOW_VEHICLE);
	if (nullptr != m_pVehicleWindowView)
		m_pVehicleWindowView->Render_Text();
	Occlusion.Apply(UI_TEXT_LAYER::WINDOW_HONOR_TITLE);
	if (nullptr != m_pHonorTitleWindowView)
		m_pHonorTitleWindowView->Render_Text();
	Occlusion.Apply(UI_TEXT_LAYER::WINDOW_WORLD_MAP);
	if (nullptr != m_pWorldMapWindowView)
		m_pWorldMapWindowView->Render_Text();
	Occlusion.Apply(UI_TEXT_LAYER::WINDOW_SYSTEM_OPTION);
	if (nullptr != m_pSystemOptionView)
		m_pSystemOptionView->Render_Text();
	Occlusion.Apply(UI_TEXT_LAYER::HUD);
	if (nullptr != m_pSongCastGaugeView)
		m_pSongCastGaugeView->Render_Text();
	}

	}
	// Cinematic subtitles remain visible while ordinary combat UI is suppressed.
	RenderCinematicSubtitles();
	// Advance the event cursor even while cinematic UI is hidden; never replay old hits.
	if (CUIInputRouter::Get().Is_CinematicSuppressed()) RenderDamageNumbers();
	/* Every CUIInputRouter-based screen's click-edge check has run by this point (both this
	function's own render pass and the Update() pass earlier this same frame) -- rolls the
	left-button edge state forward for next frame and applies SetInputBlocked for anything
	that called Claim_Mouse_This_Frame(). */
	CUIInputRouter::Get().End_Frame();

	}
	return CGameInstance::Get().Render_End();
}

void CMainApp::Hide_CombatHUD()
{
	if (nullptr != m_pHUDRuntimeView)
		m_pHUDRuntimeView->Set_AllSlotsVisible(false);
}

void CMainApp::Update_CombatHUD(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.CombatHUD.Update");
	if (nullptr == m_pHUDRuntimeView)
		return;

	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel =
		currentLevel == ETOUI(LEVEL::BERN) ||
		currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
		currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA) ||
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
	/* The customizing screen is a full-screen product overlay on the same Level: the class
	HUD chrome underneath has to stop drawing for it exactly the way it does for the Debug
	raid-entry preview above. */
	const bool_t isCharSelectOverlayOpen = isCharSelectDebugPreviewOpen ||
		(ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel &&
			nullptr != CLevel_CharacterSelect::Get_Active() &&
			CLevel_CharacterSelect::Get_Active()->Is_CustomizingOpen());

	const HUD_PLAYER_STATE& player =
		CCombatHUDViewModel::Get().Get_Player();
	if (!isSupportedLevel || skillWindowOpen || isCharSelectOverlayOpen ||
		Is_MvpResultPageOpen() ||
		!player.isValid || 0u == player.iMaximumHp || 0u == player.iMaximumResource)
	{
		Hide_CombatHUD();
		if (nullptr != m_pCombatAnalysisView)
			m_pCombatAnalysisView->Hide();
		/* m_pInventoryView's CUI_Sprite slots live under LEVEL::STATIC too (so the panel
		survives a Bern<->Valtan transition instead of resetting) -- they keep showing their
		last state across a level change unless told otherwise, same as this HUD's own. */
		if (nullptr != m_pInventoryView)
			m_pInventoryView->Hide();
		if (nullptr != m_pCharacterInfoView)
			m_pCharacterInfoView->Hide();
		if (nullptr != m_pAvatarBookView)
			m_pAvatarBookView->Hide();
		if (nullptr != m_pVehicleWindowView)
			m_pVehicleWindowView->Hide();
		if (nullptr != m_pHonorTitleWindowView)
			m_pHonorTitleWindowView->Hide();
		if (nullptr != m_pQuickSlotDragView)
		{
			m_pQuickSlotDragView->Cancel();
			m_pQuickSlotDragView->Hide();
		}
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
	Update_SkillSlotMarks();
	Update_SkillCooldowns();
	Update_QuickSlotFlash();
	Update_ItemQuickSlots();
	Update_SpecialQuickSlots();
	/* The combat analyser belongs to the raid arenas: in town and the class-select arena it
	   has nothing to measure. */
	if (nullptr != m_pCombatAnalysisView)
	{
		if (ETOUI(LEVEL::VALTAN_ARENA) == currentLevel || ETOUI(LEVEL::KAKULSAYDON_ARENA) == currentLevel)
			m_pCombatAnalysisView->Update(fTimeDelta, player);
		else
			m_pCombatAnalysisView->Hide();
	}
	m_HudTimedTexts.clear();
	Update_KoukuHudMode();
	Update_VehicleHud();
	Update_SpecialSlot();
	Update_BuffBar();
	if (nullptr != m_pInventoryView)
	{
		m_pInventoryView->Update(CCombatHUDViewModel::Get().Get_Inventory().Items);
		/* Right-click equip: the first free slot of the item's kind (the two earrings / two
		   rings), or the first one when both are worn. The Server checks kind and class. */
		string strEquipItemId;
		if (m_pInventoryView->Try_Consume_EquipRequest(strEquipItemId))
		{
			using LostArk::Shared::EQUIPMENT_SLOT;
			const ITEM_DEFINITION* pEquip = CItemCatalog::Find_ById(strEquipItemId);
			EQUIPMENT_SLOT eTarget = EQUIPMENT_SLOT::NONE;
			for (uint32_t iSlot = ETOUI(EQUIPMENT_SLOT::HELMET);
				nullptr != pEquip && iSlot < ETOUI(EQUIPMENT_SLOT::END); ++iSlot)
			{
				const EQUIPMENT_SLOT eSlot = static_cast<EQUIPMENT_SLOT>(iSlot);
				const char* pKind = LostArk::Shared::Equipment_SlotKind(eSlot);
				if (nullptr == pKind || pEquip->strEquipSlot != pKind)
					continue;
				bool_t bWorn = false;
				for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& Item :
					CCombatHUDViewModel::Get().Get_Inventory().Items)
					bWorn = bWorn || Item.eEquippedSlot == eSlot;
				if (EQUIPMENT_SLOT::NONE == eTarget)
					eTarget = eSlot;
				if (!bWorn)
				{
					eTarget = eSlot;
					break;
				}
			}
			if (EQUIPMENT_SLOT::NONE != eTarget)
				(void)CNetworkManager::Get().Send_SetEquipment(
					m_iNextUseItemSequence++, eTarget, true, strEquipItemId);
		}
	}
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
				pLocalCharacter = pCharacterSelect->Get_CharacterInfoCharacter();
		}
		else if (ETOUI(LEVEL::KAKULSAYDON_ARENA) == currentLevel)
		{
			if (CLevel_KakulSaydonArena* pKoukuArena = CLevel_KakulSaydonArena::Get_Active())
				pLocalCharacter = pKoukuArena->Get_LocalCharacter();
		}
		m_pCharacterInfoView->Update(fTimeDelta, pLocalCharacter, player);
		LostArk::Shared::EQUIPMENT_SLOT eUnequipSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
		if (m_pCharacterInfoView->Take_UnequipRequest(eUnequipSlot))
			(void)CNetworkManager::Get().Send_SetEquipment(
				m_iNextUseItemSequence++, eUnequipSlot, false, {});
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
		if (nullptr != m_pVehicleWindowView)
		{
			m_pVehicleWindowView->Update(fTimeDelta, pLocalCharacter, player);
			/* The window only names the vehicle; the level's CPlayerController owns the Server
			round trip (pending sequence, result log) exactly as for its own H key. Arena levels
			get the request too -- the Server answers REJECTED_WORLD_NOT_ALLOWED there. */
			uint32_t iVehicleId = 0u;
			if (m_pVehicleWindowView->Take_RidingRequest(iVehicleId))
			{
				CPlayerController* pController = Find_ActivePlayerController();
				if (nullptr == pController || !pController->Request_VehicleRiding(iVehicleId))
					OutputDebugStringA("[Client][VehicleWindow] Riding request not sent (no controller, or one is still pending).\n");
			}
		}
		if (nullptr != m_pHonorTitleWindowView)
		{
			/* The info window's title row button toggles the title window; the window closes
			with the info window. The Server round trip is the controller's, as for vehicles. */
			if (m_pCharacterInfoView->Take_HonorTitleWindowRequest())
				m_pHonorTitleWindowView->Toggle();
			if (!m_pCharacterInfoView->Is_Open())
				m_pHonorTitleWindowView->Close();
			/* Same covering rule as the avatar book: the info window's labels under the open
			title window's rect are skipped so the title window really sits on top. */
			if (m_pHonorTitleWindowView->Is_Open())
				m_pCharacterInfoView->Set_Covered(true);
			m_pHonorTitleWindowView->Update(fTimeDelta, player);
			uint32_t iTitleId = 0u;
			if (m_pHonorTitleWindowView->Take_TitleRequest(iTitleId))
			{
				CPlayerController* pController = Find_ActivePlayerController();
				if (nullptr == pController || !pController->Request_HonorTitle(iTitleId))
					OutputDebugStringA("[Client][HonorTitleWindow] Title request not sent (no controller, or one is still pending).\n");
			}
		}
	}
	Update_QuickSlotDrag();

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
		{ "SpecialSkill_1", L"5" }, { "SpecialSkill_2", L"6" }, { "SpecialSkill_3", L"7" },
		{ "SpecialSkill_4", L"8" }, { "SpecialSkill_5", L"9" }, { "SpecialSkill_6", L"0" },
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
	/* The mounted-vehicle HUD (Update_VehicleHud) drops the T/V column the same way. */
	const bool_t bMountedLabels = 0u != keyLabelPlayer.iVehicleId;
	for (const KEY_LABEL& Label : LABELS)
	{
		if ((bKoukuModeLabels || bMountedLabels) &&
			(0 == std::strcmp(Label.pSlotId, "Skill_T") || 0 == std::strcmp(Label.pSlotId, "Skill_V")))
		{
			continue;
		}
		DrawKeyLabel(Label.pSlotId, Label.pLabel);
	}
	if (m_bHudSpecialSlotShown)
		DrawKeyLabel("Special_Space", L"Space");

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

CPlayerController* CMainApp::Find_ActivePlayerController() const
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (ETOUI(LEVEL::BERN) == currentLevel)
	{
		if (CLevel_Bern* pBern = CLevel_Bern::Get_Active())
			return &pBern->Get_PlayerController();
	}
	else if (ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel)
	{
		if (CLevel_CharacterSelect* pCharacterSelect = CLevel_CharacterSelect::Get_Active())
			return &pCharacterSelect->Get_DebugPlayerController();
	}
	else if (ETOUI(LEVEL::VALTAN_ARENA) == currentLevel)
	{
		if (CLevel_ValtanArena* pValtanArena = CLevel_ValtanArena::Get_Active())
			return &pValtanArena->Get_DebugPlayerController();
	}
	else if (ETOUI(LEVEL::KAKULSAYDON_ARENA) == currentLevel)
	{
		if (CLevel_KakulSaydonArena* pKoukuArena = CLevel_KakulSaydonArena::Get_Active())
			return &pKoukuArena->Get_DebugPlayerController();
	}
	return nullptr;
}

void CMainApp::Update_QuickSlotDrag()
{
	if (nullptr == m_pQuickSlotDragView || nullptr == m_pHUDRuntimeView)
		return;
	string strItemId, strIconPath;
	if (nullptr != m_pInventoryView && m_pInventoryView->Try_Consume_ItemPick(strItemId, strIconPath))
		m_pQuickSlotDragView->Begin_ItemCarry(strItemId, strIconPath);
	uint32_t iVehicleId = 0u;
	if (nullptr != m_pVehicleWindowView && m_pVehicleWindowView->Take_IconPick(iVehicleId, strIconPath))
		m_pQuickSlotDragView->Begin_VehicleCarry(iVehicleId, strIconPath);
	if (!m_pQuickSlotDragView->Update())
		return;

	/* Drop click: a quick slot of the payload's kind takes it, anywhere else lets go. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pHUDRuntimeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pHUDRuntimeView->Get_ResolutionHeight();
	const auto Hovered = [&](const char* pSlotId)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		return m_pHUDRuntimeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight) &&
			Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
	};
	if (CQuickSlotDragView::PAYLOAD::ITEM == m_pQuickSlotDragView->Get_Payload())
	{
		constexpr const char* ITEM_SLOT_IDS[4] = { "Item_1", "Item_2", "Item_3", "Item_4" };
		for (int32_t i = 0; i < 4; ++i)
		{
			if (!Hovered(ITEM_SLOT_IDS[i]))
				continue;
			m_strItemQuickSlot[i] = m_pQuickSlotDragView->Get_ItemId();
			Play_UIButtonClickSound();
			break;
		}
	}
	else if (CQuickSlotDragView::PAYLOAD::VEHICLE == m_pQuickSlotDragView->Get_Payload())
	{
		constexpr const char* SPECIAL_SLOT_IDS[6] = {
			"SpecialSkill_1", "SpecialSkill_2", "SpecialSkill_3",
			"SpecialSkill_4", "SpecialSkill_5", "SpecialSkill_6" };
		for (int32_t i = 0; i < 6; ++i)
		{
			if (!Hovered(SPECIAL_SLOT_IDS[i]))
				continue;
			m_iSpecialQuickSlotVehicle[i] = m_pQuickSlotDragView->Get_VehicleId();
			Play_UIButtonClickSound();
			break;
		}
	}
	m_pQuickSlotDragView->Cancel();
}

void CMainApp::Update_SpecialQuickSlots()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	constexpr const char* SPECIAL_ICON_SLOT_IDS[6] = {
		"SpecialSkill_1_Icon", "SpecialSkill_2_Icon", "SpecialSkill_3_Icon",
		"SpecialSkill_4_Icon", "SpecialSkill_5_Icon", "SpecialSkill_6_Icon" };
	for (int32_t i = 0; i < 6; ++i)
	{
		const string* pIcon = (0u != m_iSpecialQuickSlotVehicle[i] && nullptr != m_pVehicleWindowView) ?
			m_pVehicleWindowView->Find_IconAsset(m_iSpecialQuickSlotVehicle[i]) : nullptr;
		if (nullptr == pIcon || pIcon->empty())
		{
			m_pHUDRuntimeView->Set_SlotVisible(SPECIAL_ICON_SLOT_IDS[i], false);
			continue;
		}
		m_pHUDRuntimeView->Set_SlotTexture(SPECIAL_ICON_SLOT_IDS[i], *pIcon);
		m_pHUDRuntimeView->Set_SlotVisible(SPECIAL_ICON_SLOT_IDS[i], true);
	}
}

namespace
{
	/* Defined with the skill icon table ahead of Update_SkillIcons below. */
	const char* Find_HudSkillIcon(LostArk::Shared::SKILL_ID iSkillId);
}

void CMainApp::Update_VehicleHud()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	/* Not mounted: the base ownerClass pass in Update_CombatHUD already hid the
	VehicleRiding-owned emblem (no class owns that name), nothing to undo. */
	if (0u == player.iVehicleId)
		return;
	const std::vector<VEHICLE_SKILL_UI>* pSkills = nullptr != m_pVehicleWindowView ?
		m_pVehicleWindowView->Find_Skills(player.iVehicleId) : nullptr;

	/* Same shape as the KoukuSaydon interaction mode: identity block off (no class owns this
	name), no T/V column, the vehicle emblem in the centre. */
	m_pHUDRuntimeView->Set_ActiveOwnerClass("VehicleRiding");
	for (const char* pHiddenSlot : KOUKU_HIDDEN_SLOTS)
		m_pHUDRuntimeView->Set_SlotVisible(pHiddenSlot, false);
	m_pHUDRuntimeView->Set_SlotVisible("Vehicle_Hud_Emblem", true);

	/* Q/W/E carry the vehicle's own actions (VehicleProfiles.json skills[], the keys the Server
	binds); SPACE goes to the special slot (Update_SpecialSlot). R dismounts and, like the
	whole A/S/D/F row, shows the retail locked-slot icon. The pies read the vehicle skills'
	replicated cooldowns -- class cooldowns mean nothing here. */
	for (size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
	{
		const string strKey = KOUKU_SLOT_KEYS[i];
		const VEHICLE_SKILL_UI* pSkill = nullptr;
		if (nullptr != pSkills)
		{
			for (const VEHICLE_SKILL_UI& Skill : *pSkills)
			{
				if (Skill.strSlot == strKey)
					pSkill = &Skill;
			}
		}
		const string strIconSlot = "Skill_" + strKey + "_Icon";
		m_pHUDRuntimeView->Set_SlotTexture(strIconSlot,
			nullptr != pSkill ? pSkill->strIconAsset : string("UI/Vehicle/Vehicle_LockIcon.png"));
		m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, true);
		const string strCooldownSlot = "Skill_" + strKey + "_Cooldown";
		const f32_t fRatio = nullptr != pSkill ?
			Resolve_HudCooldownRatio(player, pSkill->iSkillId, pSkill->iCooldownMs, "Skill_" + strKey) : 0.f;
		m_pHUDRuntimeView->Set_SlotTint(strCooldownSlot, float4_t(0.f, 0.f, 0.f, 150.f / 255.f));
		m_pHUDRuntimeView->Set_SlotArcRatio(strCooldownSlot, fRatio);
		m_pHUDRuntimeView->Set_SlotVisible(strCooldownSlot, fRatio > 0.f);
	}
}

f32_t CMainApp::Resolve_HudCooldownRatio(const HUD_PLAYER_STATE& player, const uint32_t iSkillId,
	const uint32_t iCooldownMs, const string& strTextSlotId)
{
	/* Remaining share of a replicated cooldown for a skill outside the class quick slots (a
	vehicle action, the special slot). The duration is the catalog's cooldownMs the Server
	applied; the end tick is the Server's. Also queues the seconds text over strTextSlotId. */
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	for (const LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT& Cooldown : player.Cooldowns)
	{
		if (Cooldown.iSkillId != iSkillId || Cooldown.iCooldownEndTick <= player.iServerTick)
			continue;
		const f32_t fRemaining = static_cast<f32_t>(Cooldown.iCooldownEndTick - player.iServerTick) / SERVER_TICK_HZ;
		const f32_t fTotal = iCooldownMs > 0u ? static_cast<f32_t>(iCooldownMs) / 1000.f : fRemaining;
		if (!strTextSlotId.empty())
			m_HudTimedTexts.push_back({ strTextSlotId, Cooldown.iCooldownEndTick, false, false });
		return fTotal > 0.f ? std::clamp(fRemaining / fTotal, 0.f, 1.f) : 0.f;
	}
	return 0.f;
}

void CMainApp::Update_SpecialSlot()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const HUD_KOUKU_GIMMICK_STATE& kouku = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	const bool_t bKoukuMode = kouku.isValid && HUD_KOUKU_HUD_MODE::NONE != kouku.eHudMode;

	/* What Space does right now: the ridden vehicle's SPACE action, else the class move skill
	for the current stance (Find_BySlot is stance-aware, like the other quick slots). */
	const char* pIcon = nullptr;
	f32_t fRatio = 0.f;
	if (0u != player.iVehicleId)
	{
		const std::vector<VEHICLE_SKILL_UI>* pSkills = nullptr != m_pVehicleWindowView ?
			m_pVehicleWindowView->Find_Skills(player.iVehicleId) : nullptr;
		if (nullptr != pSkills)
		{
			for (const VEHICLE_SKILL_UI& Skill : *pSkills)
			{
				if ("SPACE" != Skill.strSlot)
					continue;
				pIcon = Skill.strIconAsset.c_str();
				fRatio = Resolve_HudCooldownRatio(player, Skill.iSkillId, Skill.iCooldownMs, "Special_Space");
			}
		}
	}
	else if (const PLAYER_SKILL_DEFINITION* pSkill = CPlayerSkillCatalog::Find_BySlot(
		player.eCharacterClass, "SPACE", player.eStance); nullptr != pSkill)
	{
		pIcon = Find_HudSkillIcon(pSkill->iSkillId);
		fRatio = Resolve_HudCooldownRatio(player, pSkill->iSkillId, pSkill->iCooldownMs, "Special_Space");
	}

	/* Retail shows the special slot only while its skill is cooling down
	(QuickSlotSpecialSlotManager.playingCoolDownLength counts the slots that are). */
	m_bHudSpecialSlotShown = !bKoukuMode && nullptr != pIcon && player.isValid && fRatio > 0.f;
	m_pHUDRuntimeView->Set_SlotVisible("Special_Space", m_bHudSpecialSlotShown);
	m_pHUDRuntimeView->Set_SlotVisible("Special_Space_Frame", m_bHudSpecialSlotShown);
	m_pHUDRuntimeView->Set_SlotVisible("Special_Space_Icon", m_bHudSpecialSlotShown);
	if (m_bHudSpecialSlotShown)
		m_pHUDRuntimeView->Set_SlotTexture("Special_Space_Icon", pIcon);
	m_pHUDRuntimeView->Set_SlotArcRatio("Special_Space_Cooldown", fRatio);
	m_pHUDRuntimeView->Set_SlotVisible("Special_Space_Cooldown", m_bHudSpecialSlotShown);
	if (!m_bHudSpecialSlotShown)
	{
		/* Drop the seconds text Resolve_HudCooldownRatio queued for a hidden slot. */
		m_HudTimedTexts.erase(std::remove_if(m_HudTimedTexts.begin(), m_HudTimedTexts.end(),
			[](const HUD_TIMED_TEXT& Text) { return "Special_Space" == Text.strSlotId; }), m_HudTimedTexts.end());
	}
}

void CMainApp::Update_BuffBar()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	constexpr size_t BUFF_SLOTS = 4;
	struct BUFF_DRAW { const string* pIcon; uint32_t iEndTick; f32_t fRatio; };
	std::vector<BUFF_DRAW> Buffs, Debuffs;
	Buffs.reserve(BUFF_SLOTS);
	Debuffs.reserve(BUFF_SLOTS);

	/* Every entry is a replicated fact; the JSON only says which icon stands for it. */
	for (const HUD_BUFF_SOURCE& Source : m_HudBuffSources)
	{
		if (!player.isValid)
			break;
		BUFF_DRAW Draw{ &Source.strIconAsset, 0u, 0.f };
		bool_t bActive = false;
		if ("vehicle" == Source.strSource)
		{
			const string* pVehicleIcon = (0u != player.iVehicleId && nullptr != m_pVehicleWindowView) ?
				m_pVehicleWindowView->Find_IconAsset(player.iVehicleId) : nullptr;
			bActive = nullptr != pVehicleIcon && !pVehicleIcon->empty();
			Draw.pIcon = pVehicleIcon;
		}
		else if ("stance" == Source.strSource)
		{
			bActive = ("WARLORD_DEFENSE" == Source.strStance &&
				LostArk::Shared::PLAYER_STANCE_ID::WARLORD_DEFENSE == player.eStance) ||
				("GUARDIANKNIGHT_DRAGON" == Source.strStance &&
				LostArk::Shared::PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON == player.eStance);
		}
		else if ("silence" == Source.strSource)
		{
			bActive = Is_ServerDeadlinePending(player.iServerTick, player.iSilenceEndTick);
			Draw.iEndTick = player.iSilenceEndTick;
			if (bActive && 0u != player.iSilenceDurationTicks)
			{
				Draw.fRatio = std::clamp(static_cast<f32_t>(player.iSilenceEndTick - player.iServerTick) /
					static_cast<f32_t>(player.iSilenceDurationTicks), 0.f, 1.f);
			}
		}
		else if ("fetter" == Source.strSource)
		{
			bActive = player.isPatternBound;
			Draw.iEndTick = player.iPatternBindEndTick > player.iServerTick ? player.iPatternBindEndTick : 0u;
		}
		else if ("fear" == Source.strSource)
		{
			bActive = Is_ServerDeadlinePending(player.iServerTick, player.iFearEndTick);
			Draw.iEndTick = player.iFearEndTick;
		}
		if (!bActive || nullptr == Draw.pIcon || Draw.pIcon->empty())
			continue;
		std::vector<BUFF_DRAW>& List = Source.bDebuff ? Debuffs : Buffs;
		if (List.size() < BUFF_SLOTS)
			List.push_back(Draw);
	}

	const auto Apply = [&](const char* pPrefix, const std::vector<BUFF_DRAW>& List, const bool_t bDebuff)
	{
		for (size_t i = 0; i < BUFF_SLOTS; ++i)
		{
			const string strBase = string(pPrefix) + "_" + std::to_string(i);
			const bool_t bShown = i < List.size();
			m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Bg", bShown);
			m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Border", bShown);
			m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Icon", bShown);
			if (!bShown)
			{
				m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Cooldown", false);
				continue;
			}
			const BUFF_DRAW& Draw = List[i];
			m_pHUDRuntimeView->Set_SlotTexture(strBase + "_Icon", *Draw.pIcon);
			m_pHUDRuntimeView->Set_SlotArcRatio(strBase + "_Cooldown", Draw.fRatio);
			m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Cooldown", Draw.fRatio > 0.f);
			if (0u != Draw.iEndTick)
				m_HudTimedTexts.push_back({ strBase + "_Icon", Draw.iEndTick, bDebuff, true });
		}
	};
	Apply("Buff", Buffs, false);
	Apply("Debuff", Debuffs, true);
}

void CMainApp::Update_SkillSlotMarks()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const HUD_KOUKU_GIMMICK_STATE& kouku = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	const bool_t bClassSlots = 0u == player.iVehicleId &&
		!(kouku.isValid && HUD_KOUKU_HUD_MODE::NONE != kouku.eHudMode);
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V" };

	for (const char* pInputSlot : INPUT_SLOTS)
	{
		const string strMarkSlot = string("Skill_") + pInputSlot + "_TypeMark";
		const string strChainSlot = string("Skill_") + pInputSlot + "_Chain";
		const PLAYER_SKILL_DEFINITION* pSkill = bClassSlots ? CPlayerSkillCatalog::Find_BySlot(
			player.eCharacterClass, pInputSlot, player.eStance) : nullptr;

		/* Type mark (SkillSlotTypeMark frames 2 / 3 / 13): which skills carry one is data. */
		const string* pMarkAsset = nullptr;
		if (nullptr != pSkill)
		{
			for (const HUD_SKILL_MARK& Mark : m_HudSkillMarks)
			{
				if (Mark.iSkillId != pSkill->iSkillId)
					continue;
				for (const auto& Asset : m_HudSkillMarkAssets)
				{
					if (Asset.first == Mark.strMark)
						pMarkAsset = &Asset.second;
				}
			}
		}
		if (nullptr != pMarkAsset)
			m_pHUDRuntimeView->Set_SlotTexture(strMarkSlot, *pMarkAsset);
		m_pHUDRuntimeView->Set_SlotVisible(strMarkSlot, nullptr != pMarkAsset);

		/* Chain time (chainSkillTimeEffectMc): while this slot's skill is the running action
		and the Server's current combo stage has an input window, the pie is the share of that
		window still open -- all four numbers are the Server's (stage table, action start tick). */
		f32_t fChain = 0.f;
		if (nullptr != pSkill && pSkill->iSkillId == player.iCurrentSkillId &&
			player.iComboStage > 0u && player.iComboStage <= pSkill->ComboStages.size())
		{
			const PLAYER_COMBO_STAGE_TIMING& Stage = pSkill->ComboStages[player.iComboStage - 1u];
			if (Stage.iInputCloseMs > Stage.iInputOpenMs && player.iServerTick >= player.iActionStartTick)
			{
				const f32_t fElapsedMs = static_cast<f32_t>(player.iServerTick - player.iActionStartTick) *
					1000.f / SERVER_TICK_HZ;
				if (fElapsedMs >= static_cast<f32_t>(Stage.iInputOpenMs) &&
					fElapsedMs <= static_cast<f32_t>(Stage.iInputCloseMs))
				{
					fChain = (static_cast<f32_t>(Stage.iInputCloseMs) - fElapsedMs) /
						static_cast<f32_t>(Stage.iInputCloseMs - Stage.iInputOpenMs);
				}
			}
		}
		m_pHUDRuntimeView->Set_SlotArcRatio(strChainSlot, std::clamp(fChain, 0.f, 1.f));
		m_pHUDRuntimeView->Set_SlotVisible(strChainSlot, fChain > 0.f);
	}
}

void CMainApp::Load_HudQuickSlotData()
{
	m_HudSkillMarks.clear();
	m_HudSkillMarkAssets.clear();
	m_HudBuffSources.clear();
	const auto ReadObject = [](const wchar_t* pRelative, DATA_JSON_VALUE& Root) -> bool_t
	{
		ifstream Stream(CProjectDataRoot::Resolve(pRelative), ios::binary);
		if (!Stream.is_open())
			return false;
		const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
		string Error;
		return CDataJson::Parse(Text, Root, Error) && Root.Is_Object();
	};

	DATA_JSON_VALUE Marks;
	if (ReadObject(L"UI/HUD/SkillSlotMarks.json", Marks))
	{
		vector<HUD_SKILL_MARK> StagedMarks;
		vector<pair<string, string>> StagedAssets;
		bool_t bValid = true;
		if (const DATA_JSON_VALUE* pAssets = Marks.Find("assets"); nullptr != pAssets && pAssets->Is_Object())
		{
			for (const auto& Entry : pAssets->Get_Object())
			{
				if (!Entry.second.Is_String())
				{
					bValid = false;
					break;
				}
				StagedAssets.emplace_back(Entry.first, Entry.second.Get_String());
			}
		}
		const DATA_JSON_VALUE* pMarks = Marks.Find("marks");
		if (nullptr == pMarks || !pMarks->Is_Array())
			bValid = false;
		else
		{
			for (const DATA_JSON_VALUE& Value : pMarks->Get_Array())
			{
				const DATA_JSON_VALUE* pId = Value.Is_Object() ? Value.Find("skillId") : nullptr;
				const DATA_JSON_VALUE* pMark = Value.Is_Object() ? Value.Find("mark") : nullptr;
				if (nullptr == pId || !pId->Is_Number() || nullptr == pMark || !pMark->Is_String())
				{
					bValid = false;
					break;
				}
				StagedMarks.push_back({ static_cast<uint32_t>(pId->Get_Number()), pMark->Get_String() });
			}
		}
		if (bValid)
		{
			m_HudSkillMarks = std::move(StagedMarks);
			m_HudSkillMarkAssets = std::move(StagedAssets);
		}
		else
			OutputDebugStringA("[HUD] SkillSlotMarks.json is invalid -- no skill type marks.\n");
	}

	DATA_JSON_VALUE Buffs;
	if (ReadObject(L"UI/HUD/HudBuffSources.json", Buffs))
	{
		vector<HUD_BUFF_SOURCE> Staged;
		bool_t bValid = true;
		const DATA_JSON_VALUE* pSources = Buffs.Find("sources");
		if (nullptr == pSources || !pSources->Is_Array())
			bValid = false;
		else
		{
			for (const DATA_JSON_VALUE& Value : pSources->Get_Array())
			{
				const DATA_JSON_VALUE* pSource = Value.Is_Object() ? Value.Find("source") : nullptr;
				const DATA_JSON_VALUE* pKind = Value.Is_Object() ? Value.Find("kind") : nullptr;
				if (nullptr == pSource || !pSource->Is_String() || nullptr == pKind || !pKind->Is_String())
				{
					bValid = false;
					break;
				}
				HUD_BUFF_SOURCE Source{};
				Source.strSource = pSource->Get_String();
				Source.bDebuff = "debuff" == pKind->Get_String();
				if (const DATA_JSON_VALUE* pStance = Value.Find("stance"); nullptr != pStance && pStance->Is_String())
					Source.strStance = pStance->Get_String();
				if (const DATA_JSON_VALUE* pIcon = Value.Find("iconAsset"); nullptr != pIcon && pIcon->Is_String())
					Source.strIconAsset = pIcon->Get_String();
				Staged.push_back(std::move(Source));
			}
		}
		if (bValid)
			m_HudBuffSources = std::move(Staged);
		else
			OutputDebugStringA("[HUD] HudBuffSources.json is invalid -- no buff bar.\n");
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
		/* The mouse hammer remains an input action, but the maze HUD presents Q only. */
		const bool_t bHasSkill = (HUD_KOUKU_HUD_MODE::MAZE != kouku.eHudMode || 0u == i) &&
			iSkillIndex >= 0 && static_cast<size_t>(iSkillIndex) < pMode->Skills.size();
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
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.LobbyButtons.Update");
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

bool_t CMainApp::Is_RuntimeUIScreenSuppressed() const
{
	return CUIInputRouter::Get().Is_CinematicSuppressed() ||
		ETOUI(LEVEL::LOADING) == CGameInstance::Get().Get_CurrentLevelID();
}

void CMainApp::Close_RuntimeWindowsForLoading()
{
	if (nullptr != m_pInventoryView) m_pInventoryView->Close();
	if (nullptr != m_pCharacterInfoView) m_pCharacterInfoView->Close();
	if (nullptr != m_pAvatarBookView) m_pAvatarBookView->Close();
	if (nullptr != m_pVehicleWindowView) m_pVehicleWindowView->Close();
	if (nullptr != m_pHonorTitleWindowView) m_pHonorTitleWindowView->Close();
	if (nullptr != m_pWorldMapWindowView) m_pWorldMapWindowView->Close();
	if (nullptr != m_pSystemOptionView) m_pSystemOptionView->Close();
	if (nullptr != m_pQuickSlotDragView) m_pQuickSlotDragView->Cancel();
	if (nullptr != m_pChatWindowView)
	{
		m_pChatWindowView->Close_Input();
		m_pChatWindowView->Hide_AllSlots();
	}
	m_iEscapeWindowCount = 0u;
}

bool_t CMainApp::Is_AnyRuntimeWindowOpen() const
{
	/* Every runtime window that answers Escape with "close me". The system option window
	itself is deliberately not in this list: Update_SystemOptionWindow owns its Escape edge. */
	const bool_t bOpen =
		(nullptr != m_pInventoryView && m_pInventoryView->Is_Open()) ||
		(nullptr != m_pCharacterInfoView && m_pCharacterInfoView->Is_Open()) ||
		(nullptr != m_pAvatarBookView && m_pAvatarBookView->Is_Open()) ||
		(nullptr != m_pVehicleWindowView && m_pVehicleWindowView->Is_Open()) ||
		(nullptr != m_pHonorTitleWindowView && m_pHonorTitleWindowView->Is_Open()) ||
		(nullptr != m_pWorldMapWindowView && m_pWorldMapWindowView->Is_Open()) ||
		(nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open()) ||
		(nullptr != m_pChatWindowView && m_pChatWindowView->Is_Open()) ||
		(nullptr != m_pCharacterSelectWindowView && m_pCharacterSelectWindowView->Is_Open()) ||
		m_bItemUpgradePreviewVisible || Is_MvpResultPageOpen();
	return bOpen;
}

bool_t CMainApp::Is_EscapeWindowOpen(const ESCAPE_WINDOW eWindow) const
{
	switch (eWindow)
	{
	case ESCAPE_WINDOW::INVENTORY: return nullptr != m_pInventoryView && m_pInventoryView->Is_Open();
	case ESCAPE_WINDOW::CHARACTER_INFO: return nullptr != m_pCharacterInfoView && m_pCharacterInfoView->Is_Open();
	case ESCAPE_WINDOW::AVATAR_BOOK: return nullptr != m_pAvatarBookView && m_pAvatarBookView->Is_Open();
	case ESCAPE_WINDOW::HONOR_TITLE: return nullptr != m_pHonorTitleWindowView && m_pHonorTitleWindowView->Is_Open();
	case ESCAPE_WINDOW::VEHICLE: return nullptr != m_pVehicleWindowView && m_pVehicleWindowView->Is_Open();
	case ESCAPE_WINDOW::WORLD_MAP: return nullptr != m_pWorldMapWindowView && m_pWorldMapWindowView->Is_Open();
	default: return false;
	}
}

void CMainApp::Sync_EscapeWindowOrder()
{
	uint32_t iKept = 0u;
	for (uint32_t i = 0u; i < m_iEscapeWindowCount; ++i)
	{
		if (Is_EscapeWindowOpen(m_EscapeWindowOrder[i]))
			m_EscapeWindowOrder[iKept++] = m_EscapeWindowOrder[i];
	}
	m_iEscapeWindowCount = iKept;
	/* Enum order breaks a same-frame tie, so the avatar book / title window land above the info
	window they open over. */
	for (uint32_t iWindow = 0u; iWindow < ETOUI(ESCAPE_WINDOW::END); ++iWindow)
	{
		const ESCAPE_WINDOW eWindow = static_cast<ESCAPE_WINDOW>(iWindow);
		if (!Is_EscapeWindowOpen(eWindow))
			continue;
		bool_t bListed = false;
		for (uint32_t i = 0u; i < m_iEscapeWindowCount && !bListed; ++i)
			bListed = m_EscapeWindowOrder[i] == eWindow;
		if (!bListed)
			m_EscapeWindowOrder[m_iEscapeWindowCount++] = eWindow;
	}
}

bool_t CMainApp::Close_TopEscapeWindow()
{
	Sync_EscapeWindowOrder();
	if (0u == m_iEscapeWindowCount)
		return false;
	const ESCAPE_WINDOW eTop = m_EscapeWindowOrder[--m_iEscapeWindowCount];
	switch (eTop)
	{
	case ESCAPE_WINDOW::INVENTORY: m_pInventoryView->Close(); break;
	case ESCAPE_WINDOW::CHARACTER_INFO: m_pCharacterInfoView->Close(); break;
	case ESCAPE_WINDOW::AVATAR_BOOK: m_pAvatarBookView->Close(); break;
	case ESCAPE_WINDOW::HONOR_TITLE: m_pHonorTitleWindowView->Close(); break;
	case ESCAPE_WINDOW::VEHICLE: m_pVehicleWindowView->Close(); break;
	case ESCAPE_WINDOW::WORLD_MAP:
		/* Its hole dialog takes the press first; the window stays on top until it closes. */
		m_pWorldMapWindowView->Handle_EscapeEdge();
		if (m_pWorldMapWindowView->Is_Open())
			++m_iEscapeWindowCount;
		break;
	default: break;
	}
	return true;
}

bool_t CMainApp::Is_EscapeOwnedElsewhere() const
{
	/* These read the same press in their own Update, which runs later this frame. */
	if (nullptr != m_pQuickSlotDragView && m_pQuickSlotDragView->Is_Carrying())
		return true;
	if (CLevel_Bern* pBern = CLevel_Bern::Get_Active();
		nullptr != pBern && ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID() &&
		pBern->Is_ValtanEntryModalOpen())
		return true;
#ifdef _DEBUG
	if (CLevel_CharacterSelect* pCharacterSelect = CLevel_CharacterSelect::Get_Active();
		nullptr != pCharacterSelect &&
		ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID() &&
		pCharacterSelect->Is_DebugRaidEntryPreviewOpen())
		return true;
#endif
	return false;
}

void CMainApp::Update_SystemOptionWindow(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(),
		"UI.Runtime.SystemOptionWindow.Update");
	if (nullptr == m_pSystemOptionView)
		return;

	/* Read the other windows before their own Update runs this frame: an Escape that closes
	one of them must not fall through to opening this. This one Escape edge, read here from
	the same key source every frame, both opens and closes the window -- the view polling
	DirectInput on its own lagged a frame behind and closed what this had just opened. The same
	edge closes the toggle windows (inventory, info, vehicle, map...) one per press. */
	const bool_t bOtherWindowOpen = Is_AnyRuntimeWindowOpen();
	Sync_EscapeWindowOrder();
	if (!ImGui::GetIO().WantTextInput && !CUIInputRouter::Get().Is_TextInputActive())
	{
		const bool_t bWindowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t bKeyDown = bWindowFocused &&
			0 != (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
		if (bKeyDown && !m_bSystemOptionKeyDown)
		{
			if (m_pSystemOptionView->Is_Open())
				m_pSystemOptionView->Handle_EscapeEdge();
			else if (Is_MvpResultPageOpen())
			{
				/* The award page is the topmost modal: Escape closes it, and nothing
				   (least of all this window) opens under it on the same press. */
				if (CLevel_KakulSaydonArena* pArena = CLevel_KakulSaydonArena::Get_Active())
					pArena->Debug_Hide_MvpResult();
				if (CLevel_ValtanArena* pValtan = CLevel_ValtanArena::Get_Active())
					pValtan->Hide_MvpResult();
			}
			else if (m_bItemUpgradePreviewVisible)
			{
				m_bItemUpgradePreviewVisible = false;
				Hide_ItemUpgrade();
			}
			else if (Is_EscapeOwnedElsewhere())
			{
				/* The carry / popup closes itself on this press; nothing else does. */
			}
			else if (Close_TopEscapeWindow())
			{
				/* One press, one window: the newest open one. */
			}
			else if (!bOtherWindowOpen)
			{
				m_pSystemOptionView->Open();
				Play_UIButtonClickSound();
			}
		}
		m_bSystemOptionKeyDown = bKeyDown;
	}

	m_pSystemOptionView->Update(fTimeDelta);

	/* A brightness / post-process edit only reaches the renderer when quality is resolved
	again, which normally happens on a Level or region change. Re-activating the profile that
	is already active runs that same path now. */
	if (m_pSystemOptionView->Take_VideoDirty())
	{
		const string strActiveProfileId = m_RenderingProfiles.Get_ActiveProfileId();
		string strStatus;
		if (!strActiveProfileId.empty() &&
			!m_RenderingProfiles.Activate_Profile(strActiveProfileId, strStatus))
		{
			OutputDebugStringA(("[MainApp] System option video re-apply failed: " +
				strStatus + "\n").c_str());
		}
	}
}

void CMainApp::Update_CharacterSelectWindow(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.CharacterSelectWindow.Update");
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
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.Minimap.Update");
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
	/* Part of the in-game HUD, so it clears for the award page like the rest.
	   A null snapshot is this view's own documented "hide every slot". */
	if (Is_MvpResultPageOpen())
		bHasSnapshot = false;
	m_pMinimapView->Update(fTimeDelta, eLevel, bHasSnapshot ? &Snapshot : nullptr);
	/* The world map window reads the same marker snapshot; it hides itself without one. */
	if (nullptr != m_pWorldMapWindowView)
	{
		m_pWorldMapWindowView->Update(fTimeDelta, eLevel, bHasSnapshot ? &Snapshot : nullptr);
		/* A square-hole click: the level's controller owns the Server round trip, as for
		vehicles and titles; the Server's SQUAREHOLE_SONG action drives the gauge below. */
		uint16_t iHoleId = 0u;
		if (m_pWorldMapWindowView->Take_SquareHoleRequest(iHoleId))
		{
			CPlayerController* pController = Find_ActivePlayerController();
			if (nullptr == pController || !pController->Request_UseSquareHole(iHoleId))
				OutputDebugStringA("[Client][WorldMapWindow] Square hole request not sent (no controller, or the player is busy).\n");
		}
		if (m_pWorldMapWindowView->Take_ShipTravelRequest())
		{
			CPlayerController* pController = Find_ActivePlayerController();
			if (nullptr == pController || !pController->Request_UseSquareHole(
				LostArk::Shared::WORLD_MAP_SHIP_TRAVEL_DESTINATION_ID))
			{
				OutputDebugStringA("[Client][WorldMapWindow] Ship travel request not sent (no controller, or the player is busy).\n");
			}
		}
	}
	if (nullptr != m_pSongCastGaugeView)
		m_pSongCastGaugeView->Update(fTimeDelta, CCombatHUDViewModel::Get().Get_Player(),
			ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID());
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
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.ItemQuickSlots.Update");
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
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.PlayerHealthManaBar.Update");
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
	/* A class with an ember pool runs on ember, not mana: the mana fill stays
	hidden and the readout below the bar shows the orb gauge instead. */
	m_pHUDRuntimeView->Set_SlotVisible("ManaBar_Fill",
		manaRatio > 0.f && 0u == player.iEmberMaximumSockets);
}

void CMainApp::Update_LanceMasterIdentityGauge()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.LanceMasterIdentityGauge.Update");
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
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.SkillCooldowns.Update");
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

	/* Texts the HUD update pass queued this frame: cooldown seconds over the special slot and
	the vehicle Q/W/E, remaining seconds under each buff / debuff (Shared_BuffSlot_Common
	cooldownText: YG760 10 px, #9BD979 for a buff, #E2C87A for a debuff, centred under the
	icon). Vehicle-mode slots are excluded from the class loop above, so nothing doubles. */
	constexpr f32_t BUFF_TEXT_PX = 10.f * (2.f / 3.f);
	const fvector_t vBuffText = XMVectorSet(155.f / 255.f, 217.f / 255.f, 121.f / 255.f, 1.f);
	const fvector_t vDebuffText = XMVectorSet(226.f / 255.f, 200.f / 255.f, 122.f / 255.f, 1.f);
	for (const HUD_TIMED_TEXT& Text : m_HudTimedTexts)
	{
		if (Text.iEndTick <= player.iServerTick)
			continue;
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect(Text.strSlotId, fX, fY, fWidth, fHeight))
			continue;
		const int32_t iSeconds = static_cast<int32_t>(std::ceil(
			static_cast<f32_t>(Text.iEndTick - player.iServerTick) / SERVER_TICK_HZ));
		const wstring strSeconds = std::to_wstring(iSeconds) + (Text.bUnderSlot ? L"" : L"s");
		if (!Text.bUnderSlot)
		{
			DrawCooldownLabel(fX + fWidth * 0.5f, fY + fHeight * 0.5f, fHeight * 0.34f, strSeconds);
			continue;
		}
		f32_t fScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(TEXT("Font_YG760"), BUFF_TEXT_PX * textUiScale, fScale);
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, strSeconds.c_str());
		const float2_t vPosition(
			std::round((fX + fWidth * 0.5f) * textScaleX - vMeasured.x * fScale * 0.5f),
			std::round((fY + fHeight) * textScaleY + 1.f));
		CGameInstance::Get().Draw_Text(strFont, strSeconds.c_str(), float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 0.75f), 0.f, float2_t(0.f, 0.f), fScale);
		CGameInstance::Get().Draw_Text(strFont, strSeconds.c_str(), vPosition,
			Text.bDebuff ? vDebuffText : vBuffText, 0.f, float2_t(0.f, 0.f), fScale);
	}

	const auto kouku = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	if (kouku.isValid && kouku.eHudMode != HUD_KOUKU_HUD_MODE::NONE &&
		currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA))
	{
		for (std::size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
		{
			if (kouku.ModeSkillIndexBySlot[i] < 0 || kouku.CooldownEndTicks[i] <= player.iServerTick) continue;
			float x, y, width, height;
			if (m_pHUDRuntimeView->Get_SlotRect(std::string("Skill_") + KOUKU_SLOT_KEYS[i] + "_Icon", x, y, width, height))
			{
				const auto seconds = static_cast<unsigned>(std::ceil((kouku.CooldownEndTicks[i] - player.iServerTick) / SERVER_TICK_HZ));
				DrawCooldownLabel(x + width * .5f, y + height * .5f, height * .34f, std::to_wstring(seconds) + L"s");
			}
		}
		return;
	}

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

void CMainApp::Update_BossImmuneGauge(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.BossImmuneGauge.Update");
	if (nullptr == m_pBossImmuneGaugeView)
		return;
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel =
		currentLevel == ETOUI(LEVEL::BERN) ||
		currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
		currentLevel == ETOUI(LEVEL::DEVELOPMENT) ||
		currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) ||
		currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA);
	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	m_pBossImmuneGaugeView->Update(fTimeDelta,
		CCombatHUDViewModel::Get().Get_Boss(), isSupportedLevel && !skillWindowOpen);
}

void CMainApp::Update_BossHealthBar()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.BossHealthBar.Update");
	if (nullptr == m_pBossUIView)
	{
		return;
	}

	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel =
		currentLevel == ETOUI(LEVEL::BERN) ||
		currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
		currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA) ||
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
	/* The customizing screen is a full-screen product overlay on the same Level: the class
	HUD chrome underneath has to stop drawing for it exactly the way it does for the Debug
	raid-entry preview above. */
	const bool_t isCharSelectOverlayOpen = isCharSelectDebugPreviewOpen ||
		(ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel &&
			nullptr != CLevel_CharacterSelect::Get_Active() &&
			CLevel_CharacterSelect::Get_Active()->Is_CustomizingOpen());

	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!isSupportedLevel || skillWindowOpen || isCharSelectOverlayOpen ||
		Is_MvpResultPageOpen() ||
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
		currentLevel != ETOUI(LEVEL::KAKULSAYDON_ARENA) &&
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

	/* Boss title, positioned via its own hand-placed slot ("Boss_TitleText"). Valtan keeps the
	full on-screen title ("마수군단장 발탄") because no title field exists in BossProfiles.json;
	every other boss shows its BossProfiles.json displayName, which is what HUD_BOSS_STATE
	already carries (KoukuSaydon gate bosses: "세이튼", "쿠크", "앵콜을 외친 쿠크세이튼"). */
	f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleWidth = 0.f, fTitleHeight = 0.f;
	if (m_pBossUIView->Get_SlotRect(
		"Boss_TitleText", fTitleX, fTitleY, fTitleWidth, fTitleHeight))
	{
		wstring strBossTitle;
		if ("BOSS_VALTAN" == boss.strArchetypeId)
			strBossTitle = L"\xB9C8\xC218\xAD70\xB2E8\xC7A5 \xBC1C\xD0C4";
		else if (!ConvertUtf8ToWide(boss.strDisplayName, strBossTitle) ||
			strBossTitle.empty())
			strBossTitle = L"BOSS";
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
	if (ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID() &&
		ETOUI(LEVEL::KAKULSAYDON_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
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
	/* Both arenas drive the same RaidClear_Layout.json document now; KoukuSaydon
	   publishes the same text rects from its own Update_RaidClear. */
	const uint32_t iRaidClearLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (ETOUI(LEVEL::VALTAN_ARENA) != iRaidClearLevel &&
		ETOUI(LEVEL::KAKULSAYDON_ARENA) != iRaidClearLevel)
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

	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / 1280.f;
	const f32_t fScaleY = vViewport.y / 720.f;
	/* announce.gfx text sizes are 1920x1080 stage px: the text line 16, the quality row 12.
	ITEM_ANNOUNCE_TEXT_SCALE is the project's own enlargement (user request), not retail. */
	constexpr f32_t ITEM_ANNOUNCE_TEXT_SCALE = 1.3f;
	const f32_t fTextPx = 16.f * 2.f / 3.f * ITEM_ANNOUNCE_TEXT_SCALE * fScaleY;
	const f32_t fQualityPx = 12.f * 2.f / 3.f * ITEM_ANNOUNCE_TEXT_SCALE * fScaleY;

	/* textField 95: one centred line, the name in its grade colour, the rest white. */
	f32_t fNameScale = 1.f, fSuffixScale = 1.f;
	const wstring_t strNameFont = UILabelFont::Resolve(TEXT("Font_YG760"), fTextPx, fNameScale);
	const wstring_t strSuffixFont = UILabelFont::Resolve(TEXT("Font_YG760"), fTextPx, fSuffixScale);
	const f32_t fNameWidth =
		CGameInstance::Get().Measure_Text(strNameFont, rects.strItemName.c_str()).x * fNameScale;
	const float2_t vSuffixSize = CGameInstance::Get().Measure_Text(strSuffixFont, rects.strSuffix.c_str());
	const f32_t fLineWidth = fNameWidth + vSuffixSize.x * fSuffixScale;
	const f32_t fLineLeft = std::round((rects.fTextX + rects.fTextWidth * 0.5f) * fScaleX - fLineWidth * 0.5f);
	const f32_t fLineTop = std::round((rects.fTextY + rects.fTextHeight * 0.5f) * fScaleY -
		vSuffixSize.y * fSuffixScale * 0.5f);
	const auto Rgb = [](const std::uint32_t iRgb, const f32_t fAlpha)
	{
		return XMVectorSet(((iRgb >> 16) & 0xffu) / 255.f, ((iRgb >> 8) & 0xffu) / 255.f,
			(iRgb & 0xffu) / 255.f, fAlpha);
	};
	const fvector_t vNameColor = Rgb(rects.iNameRgb, rects.fTextAlpha);
	CGameInstance::Get().Draw_Text(strNameFont, rects.strItemName.c_str(),
		float2_t(fLineLeft, fLineTop), vNameColor, 0.f, float2_t(0.f, 0.f), fNameScale);
	CGameInstance::Get().Draw_Text(strSuffixFont, rects.strSuffix.c_str(),
		float2_t(std::round(fLineLeft + fNameWidth), fLineTop),
		XMVectorSet(1.f, 1.f, 1.f, rects.fTextAlpha), 0.f, float2_t(0.f, 0.f), fSuffixScale);

	/* qualityProgress: "[$]item.option_quality" (품질) centred in its box, the value
	right-aligned in targetText and coloured by the tier the value reaches. */
	UILabelFont::Draw_Centered(TEXT("Font_YG760"), L"\xD488\xC9C8",
		(rects.fQualityLabelX + rects.fQualityLabelWidth * 0.5f) * fScaleX,
		(rects.fQualityLabelY + rects.fQualityLabelHeight * 0.5f) * fScaleY,
		fQualityPx, XMVectorSet(1.f, 1.f, 1.f, rects.fQualityAlpha));
	f32_t fValueScale = 1.f;
	const wstring_t strValueFont = UILabelFont::Resolve(TEXT("Font_YG760"), fQualityPx, fValueScale);
	const float2_t vValueSize = CGameInstance::Get().Measure_Text(strValueFont, rects.strQualityValue.c_str());
	CGameInstance::Get().Draw_Text(strValueFont, rects.strQualityValue.c_str(),
		float2_t(std::round((rects.fQualityValueX + rects.fQualityValueWidth) * fScaleX - vValueSize.x * fValueScale),
			std::round((rects.fQualityValueY + rects.fQualityValueHeight * 0.5f) * fScaleY -
				vValueSize.y * fValueScale * 0.5f)),
		Rgb(rects.iQualityRgb, rects.fQualityAlpha),
		0.f, float2_t(0.f, 0.f), fValueScale);
}

void CMainApp::Update_ChargeGauge()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.ChargeGauge.Update");
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
	/* A branching HOLD (Guardian Knight glide): the start stage hands off to the
	loop at comboAdvanceMs, before its own motion ends, and the loop is the part
	the player is actually holding. The gauge is that loop's remaining time, so it
	shows only during stage 2 and never treats the cut start stage as a cancel. */
	constexpr std::uint8_t BRANCHING_HOLD_LOOP_STAGE = 2u;

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
	else if (3u == pSkill->ComboStages.size() &&
		pSkill->ComboStages[0].iComboAdvanceMs < pSkill->ComboStages[0].iActionDurationMs)
	{
		if (BRANCHING_HOLD_LOOP_STAGE == player.iComboStage)
		{
			f32_t fStageAgeSeconds = 0.f;
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
				player.iServerTick, player.iActionStartTick, SERVER_TICK_HZ, fStageAgeSeconds);
			const f32_t fLoopMs = static_cast<f32_t>(
				pSkill->ComboStages[BRANCHING_HOLD_LOOP_STAGE - 1u].iActionDurationMs);
			if (fLoopMs > 0.f)
			{
				fChargeProgress = std::clamp(fStageAgeSeconds * 1000.f / fLoopMs, 0.f, 1.f);
				bCharging = true;
			}
		}
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
	else if (3u == pSkill->ComboStages.size() &&
		pSkill->ComboStages[0].iComboAdvanceMs < pSkill->ComboStages[0].iActionDurationMs)
	{
		// Branching HOLD: the gauge (and this label) exist only while the loop is held.
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
		"Esther_Slot1_Frame", "Esther_Slot1_KeyBg", "Esther_Slot1_Icon", "Esther_Slot1_Ready",
		"Esther_Slot2_Frame", "Esther_Slot2_KeyBg", "Esther_Slot2_Icon", "Esther_Slot2_Ready",
		"Esther_Slot3_Frame", "Esther_Slot3_KeyBg", "Esther_Slot3_Icon", "Esther_Slot3_Ready",
		"Esther_GaugeTrack", "Esther_GaugeFill",
	};
	for (const char_t* pSlotId : ESTHER_ALL_SLOTS)
		m_pEstherUIView->Set_SlotVisible(pSlotId, false);
}

void CMainApp::Update_EstherGauge(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.EstherGauge.Update");
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
		"Esther_Slot1_Frame", "Esther_Slot1_KeyBg", "Esther_Slot1_Icon",
		"Esther_Slot2_Frame", "Esther_Slot2_KeyBg", "Esther_Slot2_Icon",
		"Esther_Slot3_Frame", "Esther_Slot3_KeyBg", "Esther_Slot3_Icon",
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
	/* The ready glow is the real 30-frame EpicSkillAbleSlotEffect flipbook; it only
	advances while the view is updated. */
	m_pEstherUIView->Update(fTimeDelta);
	CUITextOcclusion::Get().Add_SlotOccluder(UI_TEXT_LAYER::HUD, *m_pEstherUIView, "Esther_HeaderFrame");
}

void CMainApp::RenderEstherGaugeText()
{
	/* Same gates as Update_EstherGauge -- these labels belong to that window, so they must
	disappear and reappear together with the art instead of floating without it. */
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isRaidArena =
		ETOUI(LEVEL::VALTAN_ARENA) == currentLevel ||
		ETOUI(LEVEL::KAKULSAYDON_ARENA) == currentLevel;
	if (nullptr == m_pEstherUIView || !isRaidArena)
		return;
	const uint32_t maximum =
		CCombatHUDViewModel::Get().Get_EstherGaugeMaximum();
	if (0u == maximum ||
		(nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open()) ||
		!CCombatHUDViewModel::Get().Get_Player().isValid)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	/* Retail epicskill.gfx arkSlot keyBind: a 12px label centred on the key plate under each
	portrait, white until the gauge is full and gold (#FFD200) once the skills are usable. The
	Controller's Esther keys are Ctrl+Z/X/C, shown the way retail abbreviates them. */
	const uint32_t gauge = CCombatHUDViewModel::Get().Get_EstherGauge();
	const vector_t vColor = gauge >= maximum ?
		XMVectorSet(1.f, 210.f / 255.f, 0.f, 1.f) : Colors::White;
	struct ESTHER_KEY_LABEL { const char_t* pPlateSlot; const wchar_t* pLabel; };
	constexpr ESTHER_KEY_LABEL KEY_LABELS[] = {
		{ "Esther_Slot1_KeyBg", L"C+Z" },
		{ "Esther_Slot2_KeyBg", L"C+X" },
		{ "Esther_Slot3_KeyBg", L"C+C" },
	};
	constexpr f32_t LABEL_HEIGHT = 12.f * (2.f / 3.f);
	for (const ESTHER_KEY_LABEL& Label : KEY_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pEstherUIView->Get_SlotRect(Label.pPlateSlot, fX, fY, fWidth, fHeight))
			continue;
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), Label.pLabel);
		const f32_t fScale = (vMeasured.y > 0.f) ? (LABEL_HEIGHT / vMeasured.y) : 1.f;
		/* Retail text box sits at plate y+5 with a 22px box: centre 16px below the plate top. */
		const f32_t fCenterX = fX + fWidth * 0.5f;
		const f32_t fCenterY = fY + 16.f * (2.f / 3.f);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), Label.pLabel,
			float2_t(fCenterX * textScaleX + 1.f, fCenterY * textScaleY + 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 220.f / 255.f), 0.f, float2_t(0.5f, 0.5f),
			fScale * textUiScale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), Label.pLabel,
			float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	}
}

namespace
{
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
		/* Move (Space) skills, cut by build_quickslot_hud_ui.py for the special slot. */
		{ 34020, "UI/Skill/LanceMaster/34020_Space.png" },
		{ 34520, "UI/Skill/LanceMaster/34520_Space.png" },
		{ 17020, "UI/Skill/Warlord/17020_Space.png" },
		{ 31020, "UI/Skill/Artist/31020_Space.png" },
		{ 2050020, "UI/Skill/DimensionMaster/2050020_Space.png" },
	};

	const char* Find_HudSkillIcon(const LostArk::Shared::SKILL_ID iSkillId)
	{
		for (const SKILL_ICON_ENTRY& Entry : SKILL_ICON_TABLE)
		{
			if (Entry.iSkillId == iSkillId)
				return Entry.pIconPath;
		}
		return nullptr;
	}
}

void CMainApp::Update_SkillIcons()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.SkillIcons.Update");
	/* Which skill icon belongs in Skill_Q.."Skill_F" is content, not layout: it depends on the
	live (class, stance) pair via CPlayerSkillCatalog::Find_BySlot, the same source of truth the
	input controller already resolves quick slots from. HUD_Layout.json only owns the shared
	frame's position/size (ownerClass null "Skill_Q".."Skill_F"); it must not carry a second,
	class-hardcoded copy of "which icon" that can drift out of sync with PlayerSkills.json.
	Find_BySlot already resolves stance-gated skills correctly and ignores the stance argument
	for classes whose skills have no requiredStance. Only reached from Update_CombatHUD's own
	show path. */
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();

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
			pIconPath = Find_HudSkillIcon(pSkill->iSkillId);
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
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.QuickSlotFlash.Update");
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
		currentLevel != ETOUI(LEVEL::KAKULSAYDON_ARENA) &&
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
		const wstring mana = 0u != player.iEmberMaximumSockets ?
			/* Guardian Knight: orb gauge percent and ember orbs over the open
			sockets, in place of mana until the class HUD art exists. */
			L"\uC624\uBE0C " + std::to_wstring(0u == player.iMaximumIdentity ? 0u :
				player.iCurrentIdentity * 100u / player.iMaximumIdentity) +
			L"%  \uAE30\uC6B4 " + std::to_wstring(player.iEmberOrbs) + L" / " +
			std::to_wstring(player.iEmberMaximumSockets - player.iEmberLockedSockets) :
			std::to_wstring(player.iCurrentResource) +
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
	if (CUIInputRouter::Get().Is_CinematicSuppressed())
	{
		for (const auto& event : CCombatHUDViewModel::Get().Get_DamageEvents())
			m_iLastRenderedDamageServerTick = (std::max)(m_iLastRenderedDamageServerTick, event.iServerTick);
		m_FloatingDamageNumbers.clear();
		return;
	}
	if (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open())
		return;

	/* Retail damagetext.gfx (EFUI_DAMAGE): $YoonGasiIIM 32px centred text on the 1080p stage.
	What animates it is DamageTextElement's own timeline, which DamageTextWnd starts per hit
	kind (gotoAndPlay "playerDamageType0" / "critical" / "heal") -- not DamageTextCBT2's tween
	parameters, which only a test harness drives. The movie runs at 30 fps and the frames below
	are its scaleX keys, so a number punches out to 2.65x within five frames and settles back
	onto 1.0 by frame 18. It does not travel: the timeline has no translation and no alpha. */
	constexpr f32_t DAMAGE_TIMELINE_FPS = 30.f;
	constexpr f32_t DAMAGE_SCALE_NORMAL[] = {
		1.00f, 1.25f, 2.00f, 2.33f, 2.65f, 2.60f, 2.55f, 2.50f, 2.17f,
		1.83f, 1.50f, 1.37f, 1.26f, 1.16f, 1.09f, 1.04f, 1.01f, 1.00f };
	/* "critical": a bigger punch to 4.0 that settles on 3.0 and stays there. */
	constexpr f32_t DAMAGE_SCALE_CRITICAL[] = {
		1.01f, 1.19f, 1.75f, 2.69f, 4.00f, 3.50f, 3.00f };
	/* "heal" holds 1.5 for its whole run. */
	constexpr f32_t DAMAGE_SCALE_HEAL = 1.5f;
	constexpr f32_t DAMAGE_FONT_PX_START = 32.f;
	/* The timeline ends on its last key and the host then drops the element. How long the
	settled number is held and how it leaves are native values the movie does not carry, so
	the number rests where it landed and fades from there. */
	constexpr f64_t DAMAGE_HOLD_SECONDS = 0.25;
	constexpr f64_t DAMAGE_FADE_SECONDS = 0.25;
	constexpr f64_t DAMAGE_NUMBER_LIFETIME_SECONDS =
		std::size(DAMAGE_SCALE_NORMAL) / DAMAGE_TIMELINE_FPS +
		DAMAGE_HOLD_SECONDS + DAMAGE_FADE_SECONDS;
	/* DamageTextTween.DAMAGE_ANI_LIMIT: retail animates at most 20 numbers at once. */
	constexpr size_t MAX_FLOATING_DAMAGE_NUMBERS = 20u;

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
		m_iLastRenderedDamageServerTick =
			(std::max)(m_iLastRenderedDamageServerTick, damageEvent.iServerTick);
		/* A shard belongs to the one hunter who was dealt that suit, so the
		other hunters' shards are not drawn on this screen. */
		if (LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE !=
				damageEvent.Event.eCardMazeSuit &&
			damageEvent.Event.eCardMazeSuit !=
				CCombatHUDViewModel::Get().Get_KoukuGimmick().eCardMazeSuit)
		{
			continue;
		}
		FLOATING_DAMAGE_NUMBER number{};
		number.dSpawnSeconds = Product_Now_Seconds();
		number.vWorldPosition = float3_t(
			damageEvent.Event.fPositionX,
			damageEvent.Event.fPositionY,
			damageEvent.Event.fPositionZ);
		number.iAmount = damageEvent.Event.iAmount;
		number.isOutgoing = damageEvent.Event.isOutgoing;
		number.eCardMazeSuit = damageEvent.Event.eCardMazeSuit;
		number.eHitFlag = damageEvent.Event.eHitFlag;
		/* Retail fills DamageTextElement's randValue/direction per hit so a burst does not
		stack on one point; the spread reads wider than the status words' 26x18. The values
		are native, so the range is the project's. */
		{
			static std::mt19937 scatterRandom{ std::random_device{}() };
			std::uniform_real_distribution<f32_t> spreadX(-42.f, 42.f);
			std::uniform_real_distribution<f32_t> spreadY(-26.f, 12.f);
			number.fScatterX = spreadX(scatterRandom);
			number.fScatterY = spreadY(scatterRandom);
		}
		m_dLastDamageSeconds = number.dSpawnSeconds;
		m_FloatingDamageNumbers.push_back(number);
	}
	/* System option "show damage": the events are still consumed above so switching it back
	on does not replay a backlog. */
	if (!CUserSettings::Get().Is_DamageNumberShown())
	{
		m_FloatingDamageNumbers.clear();
		return;
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
	/* The system option battle font size (75 .. 300%) scales the whole tween. */
	const f32_t stageScale = viewportSize.y / 1080.f * CUserSettings::Get().Get_DamageFontScale();

	/* A card maze shard rises where the damage number would: "<suit> jogak x N".
	   Korean is written with universal character names so this file keeps its
	   existing bytes; Font_EventDamage is the same YoonGasiIIM sprite font that
	   carries every Hangul syllable used here. */
	const auto shardText = [](const LostArk::Shared::MECHANIC_CARD_SYMBOL suit,
		const uint32_t count) -> wstring
	{
		const wchar_t* name = L"\uBB38\uC591";
		switch (suit)
		{
		case LostArk::Shared::MECHANIC_CARD_SYMBOL::HEART:
			name = L"\uD558\uD2B8"; break;
		case LostArk::Shared::MECHANIC_CARD_SYMBOL::SPADE:
			name = L"\uC2A4\uD398\uC774\uB4DC"; break;
		case LostArk::Shared::MECHANIC_CARD_SYMBOL::CLUB:
			name = L"\uD074\uB85C\uBC84"; break;
		case LostArk::Shared::MECHANIC_CARD_SYMBOL::DIAMOND:
			name = L"\uB2E4\uC774\uC544"; break;
		default: break;
		}
		return wstring(name) + L" \uC870\uAC01 x " + std::to_wstring(count);
	};
	for (const FLOATING_DAMAGE_NUMBER& number : m_FloatingDamageNumbers)
	{
		const f64_t dAge = dNow - number.dSpawnSeconds;
		/* Walk the element's own frames: hold the last key once the timeline has run out. */
		const bool_t isCritical =
			LostArk::Shared::DAMAGE_HIT_FLAG::CRITICAL == number.eHitFlag;
		const bool_t isHeal = LostArk::Shared::DAMAGE_HIT_FLAG::HEAL == number.eHitFlag;
		const f32_t* pCurve = isCritical ? DAMAGE_SCALE_CRITICAL : DAMAGE_SCALE_NORMAL;
		const size_t iCurveKeys = isCritical ?
			std::size(DAMAGE_SCALE_CRITICAL) : std::size(DAMAGE_SCALE_NORMAL);
		const f32_t fFrame = static_cast<f32_t>(dAge) * DAMAGE_TIMELINE_FPS;
		f32_t fTimelineScale = DAMAGE_SCALE_HEAL;
		if (!isHeal)
		{
			const size_t iKey = (std::min)(static_cast<size_t>((std::max)(fFrame, 0.f)),
				iCurveKeys - 1u);
			const size_t iNext = (std::min)(iKey + 1u, iCurveKeys - 1u);
			const f32_t fBlend = (std::clamp)(fFrame - static_cast<f32_t>(iKey), 0.f, 1.f);
			fTimelineScale = pCurve[iKey] + (pCurve[iNext] - pCurve[iKey]) * fBlend;
		}
		const f32_t fFontPx = DAMAGE_FONT_PX_START * fTimelineScale;
		/* The number stays where it landed; only the tail fades. */
		const f64_t dTimeline = iCurveKeys / DAMAGE_TIMELINE_FPS;
		const f32_t fAlpha = dAge <= dTimeline + DAMAGE_HOLD_SECONDS ? 1.f :
			1.f - (std::clamp)(static_cast<f32_t>(
				(dAge - dTimeline - DAMAGE_HOLD_SECONDS) / DAMAGE_FADE_SECONDS), 0.f, 1.f);
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
		const bool_t isShard =
			LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE != number.eCardMazeSuit;
		/* INVINCIBLE is drawn by nothing in retail either. */
		if (LostArk::Shared::DAMAGE_HIT_FLAG::INVINCIBLE == number.eHitFlag)
			continue;
		const wstring strAmount = isShard ?
			shardText(number.eCardMazeSuit, number.iAmount) :
			Format_ThousandsSeparated(number.iAmount);
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_EventDamage"), strAmount.c_str());
		const f32_t fScale = vMeasured.y > 0.f ? (fFontPx * stageScale) / vMeasured.y : 1.f;
		/* Retail DamageTextWnd's own colour table, selected by the Server's hit flag:
		COLOR_PC_DAMAGE 0xFFFFFF for what the player deals, COLOR_ENEMY_DAMAGE 0xFF0000 for what
		it takes, COLOR_CRITICAL_DAMAGE 0xFFCC00, COLOR_MISS_DAMAGE 0x999999, COLOR_HEAL
		0x00FF00. A potion heal is the one non-combat flag the Server raises today; CRITICAL and
		MISS light up once the combat numbers that decide them exist. */
		vector_t vColor = number.isOutgoing ?
			XMVectorSet(1.f, 1.f, 1.f, fAlpha) :
			XMVectorSet(1.f, 0.f, 0.f, fAlpha);
		switch (number.eHitFlag)
		{
		case LostArk::Shared::DAMAGE_HIT_FLAG::CRITICAL:
			vColor = XMVectorSet(1.f, 204.f / 255.f, 0.f, fAlpha); break;
		case LostArk::Shared::DAMAGE_HIT_FLAG::MISS:
			vColor = XMVectorSet(0.6f, 0.6f, 0.6f, fAlpha); break;
		case LostArk::Shared::DAMAGE_HIT_FLAG::HEAL:
			vColor = XMVectorSet(0.f, 1.f, 0.f, fAlpha); break;
		default: break;
		}
		/* A shard is a pickup, not a hit, so it keeps the plain white. */
		if (isShard)
			vColor = XMVectorSet(1.f, 1.f, 1.f, fAlpha);
		const float2_t vDrawPosition(
			XMVectorGetX(vProjected) + number.fScatterX * stageScale,
			XMVectorGetY(vProjected) + number.fScatterY * stageScale);
		/* textContainer carries a GLOWFILTER (blur 5, strength 1, opaque black) in retail, which
		is what keeps a number readable over a bright floor. A sprite font cannot blur, so the
		same black is stamped around the glyphs once per direction before the coloured pass. */
		const f32_t fGlowOffset = (std::max)(1.f, fFontPx * 0.06f * stageScale);
		const fvector_t vGlowColor = XMVectorSet(0.f, 0.f, 0.f, fAlpha);
		for (const float2_t& vStep : {
			float2_t(-1.f, 0.f), float2_t(1.f, 0.f), float2_t(0.f, -1.f), float2_t(0.f, 1.f),
			float2_t(-0.7f, -0.7f), float2_t(0.7f, -0.7f), float2_t(-0.7f, 0.7f),
			float2_t(0.7f, 0.7f) })
		{
			CGameInstance::Get().Draw_Text(TEXT("Font_EventDamage"), strAmount.c_str(),
				float2_t(vDrawPosition.x + vStep.x * fGlowOffset,
					vDrawPosition.y + vStep.y * fGlowOffset),
				vGlowColor, 0.f, float2_t(0.5f, 0.5f), fScale);
		}
		CGameInstance::Get().Draw_Text(TEXT("Font_EventDamage"), strAmount.c_str(),
			vDrawPosition, vColor, 0.f, float2_t(0.5f, 0.5f), fScale);
	}
}

void CMainApp::RenderCinematicSubtitles()
{
    const auto viewport = CGameInstance::Get().Get_ViewportSize();
    if (viewport.x <= 0.f || viewport.y <= 0.f) return;
    std::array<std::vector<std::wstring>, 2> lines;
    const auto splitLines = [](const std::string& utf8, std::vector<std::wstring>& target)
    {
        std::wstring text;
        if (!ConvertUtf8ToWide(utf8, text)) return;
        size_t begin = 0u;
        while (begin < text.size())
        {
            const size_t end = text.find(L'\n', begin);
            auto line = text.substr(begin, end == std::wstring::npos ? end : end - begin);
            if (!line.empty() && line.back() == L'\r') line.pop_back();
            if (!line.empty()) target.push_back(std::move(line));
            if (end == std::wstring::npos) break;
            begin = end + 1u;
        }
    };
    if (m_pKoukuPresentationPlayer &&
        CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
        for (const auto& subtitle : m_pKoukuPresentationPlayer->Collect_Subtitles())
            splitLines(subtitle.strText, lines[subtitle.bUpper ? 1u : 0u]);

    std::vector<WORLD_SEQUENCE_SUBTITLE_SAMPLE> worldSubtitles;
    if (auto* arena = CLevel_ValtanArena::Get_Active())
        arena->Collect_SourceCinematicSubtitles(worldSubtitles);
#ifdef _DEBUG
    if (m_pMapTool) m_pMapTool->Collect_WorldSequenceSubtitles(worldSubtitles);
#endif
    std::set<std::pair<std::string, std::string>> seen;
    std::vector<WORLD_SEQUENCE_SUBTITLE_SAMPLE> balloons;
    for (const auto& subtitle : worldSubtitles)
    {
        if (!seen.emplace(subtitle.instanceId, subtitle.subtitleTrackId).second) continue;
        if (subtitle.position == "BALLOON") balloons.push_back(subtitle);
        else splitLines(subtitle.text, lines[subtitle.position == "UPPER" ? 1u : 0u]);
    }
    if (lines[0].empty() && lines[1].empty() && balloons.empty()) return;
    const CUITextLayerScope subtitleLayer(UI_TEXT_LAYER::PAGE);
    const f32_t uiScale = viewport.y / 1080.f;
    const f32_t fontHeight = 26.f * uiScale;
    const f32_t lineStep = 34.f * uiScale;
    const f32_t shadow = std::max(1.f, std::round(2.f * uiScale));
    const auto drawLine = [&](const std::wstring& line, f32_t x, f32_t y, f32_t width)
    {
        UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"), line.c_str(),
            x + shadow, y + shadow, fontHeight,
            XMVectorSet(0.f, 0.f, 0.f, 0.95f), width);
        UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"), line.c_str(),
            x, y, fontHeight, XMVectorSet(1.f, 1.f, 1.f, 1.f), width);
    };
    for (size_t side = 0u; side < lines.size(); ++side)
    {
        f32_t y = side == 1u ? viewport.y * 0.10f :
            viewport.y * 0.90f - lineStep * static_cast<f32_t>(lines[side].size());
        for (const auto& line : lines[side])
        {
            drawLine(line, viewport.x * 0.5f, y, viewport.x * 0.84f);
            y += lineStep;
        }
    }
    const matrix_t view = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW));
    const matrix_t projection = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ));
    for (const auto& subtitle : balloons)
    {
        const auto& anchor = subtitle.worldPosition;
        const vector_t point = XMVectorSet(anchor.x, anchor.y, anchor.z, 1.f);
        if (XMVectorGetW(XMVector4Transform(point, view * projection)) <= 0.f) continue;
        const vector_t projected = XMVector3Project(point, 0.f, 0.f, viewport.x, viewport.y,
            0.f, 1.f, projection, view, XMMatrixIdentity());
        const f32_t x = XMVectorGetX(projected), y = XMVectorGetY(projected), z = XMVectorGetZ(projected);
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
            z < 0.f || z > 1.f || x < 0.f || x > viewport.x || y < 0.f || y > viewport.y) continue;
        std::vector<std::wstring> bubbleLines;
        splitLines(subtitle.text, bubbleLines);
        f32_t textY = y - lineStep * static_cast<f32_t>(bubbleLines.size()) - 12.f * uiScale;
        for (const auto& line : bubbleLines)
        {
            drawLine(line, x, textY, viewport.x * 0.42f);
            textY += lineStep;
        }
    }
}

void CMainApp::RenderFpsText()
{
	/* combobox_fps: 0 always, 1 in combat only (a hit within the last few seconds), 2 never.
	Small YG760 line in the top-left corner; the retail placement was not traced. */
	const int32_t iMode = CUserSettings::Get().Get_FpsDisplayMode();
	if (2 == iMode || m_fSmoothedFps <= 0.f)
		return;
	if (1 == iMode && Product_Now_Seconds() - m_dLastDamageSeconds > 6.0)
		return;
	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	if (viewportSize.x <= 0.f || viewportSize.y <= 0.f)
		return;
	wchar_t text[32]{};
	::_snwprintf_s(text, _countof(text), _TRUNCATE, L"FPS %d",
		static_cast<int32_t>(std::lround(m_fSmoothedFps)));
	const float2_t measured = CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), text);
	if (measured.y <= 0.f)
		return;
	const f32_t refScale = viewportSize.y / 720.f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), text,
		float2_t(std::round(8.f * refScale), std::round(6.f * refScale)),
		XMVectorSet(1.f, 0.95f, 0.6f, 1.f), 0.f, float2_t(0.f, 0.f), 13.f * refScale / measured.y);
}

void CMainApp::Limit_FrameRate()
{
	const bool_t bForeground = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
	const int32_t iLimit = CUserSettings::Get().Get_FrameLimit(bForeground);
	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	if (iLimit <= 0)
	{
		m_LastFrameEnd = now;
		return;
	}
	const std::chrono::steady_clock::time_point target = m_LastFrameEnd +
		std::chrono::duration_cast<std::chrono::steady_clock::duration>(
			std::chrono::duration<f64_t>(1.0 / static_cast<f64_t>(iLimit)));
	if (now >= target)
	{
		/* A slow frame: no catch-up burst, the next period starts now. */
		m_LastFrameEnd = now;
		return;
	}
	/* Sleep the bulk on a high-resolution timer (1 ms class), spin the last stretch. */
	static const HANDLE s_hTimer = CreateWaitableTimerExW(nullptr, nullptr,
		CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	std::chrono::steady_clock::time_point current = now;
	while (current < target)
	{
		const std::chrono::nanoseconds remain = target - current;
		if (nullptr != s_hTimer && remain > std::chrono::milliseconds(2))
		{
			LARGE_INTEGER due{};
			due.QuadPart = -static_cast<LONGLONG>((remain - std::chrono::milliseconds(1)).count() / 100);
			if (SetWaitableTimerEx(s_hTimer, &due, 0, nullptr, nullptr, nullptr, 0))
				WaitForSingleObject(s_hTimer, 20);
		}
		else
		{
			YieldProcessor();
		}
		current = std::chrono::steady_clock::now();
	}
	m_LastFrameEnd = target;
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
		/* The floating damage number is damagetext.gfx's DamageTextCBT2, whose own
		text field (character 231) is fontClass $YoonGasiIIM at 32pt. font.lpk's
		Korean FontMap.xml does bind $eventDamageFont to BMKkubulimTTF.ttf, but the
		three fields that ask for that token belong to DamageTextFoolsDay0, the
		April Fools' variant -- not to the damage number. Reading the token name as
		"the damage font" put BMKkubulim on every hit for a while; the gfx's own 43
		text fields settle it ($YG760 x38, $eventDamageFont x3, $YoonGasiIIM x1 at
		32pt, $YG330 x1). The tag name is kept so its consumers stay untouched. */
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

	/* Display-size variants, re-rasterised from the retail TTF (nothing above 42 px
	exists to downsample from). Latin only -- see UILabelFont::LATIN_DISPLAY_SIZES --
	so they are reached through Resolve_LatinDisplay and never by ordinary labels. */
	for (const int32_t iSize : UILabelFont::LATIN_DISPLAY_SIZES)
	{
		const wstring strTag = wstring(L"Font_YoonGasiIIM_Latin") + std::to_wstring(iSize);
		const wstring strFile = wstring(L"UI/Fonts/YoonGasiIIM_Latin") + std::to_wstring(iSize) + L".spritefont";
		const filesystem::path displayFontPath = CRuntimeAssetRoot::Resolve(strFile);
		if (displayFontPath.empty() || FAILED(CGameInstance::Get().Add_Font(strTag, displayFontPath.c_str())))
			OutputDebugStringW((L"[Fonts] optional display font missing: " + strFile + L"\n").c_str());
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
		L"UI/Loading/Loading_Background_Kouku.png",
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
	CPresentation_Manager::Get().Clear_Frame();
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
	// A single model decode is cooperatively cancellable only at its boundaries.
	// Keep the request queued and the current scene ticking until retirement ends.
	if (CLevelTransitionService::Is_Pending())
	{
		Engine::CProfilerScope drainScope(CGameInstance::Get().Get_Profiler(), "CharacterAssets.LevelTransition.Drain");
		CPlayableCharacterAssetService::Cancel_AllAsyncPreparations();
		if (CPlayableCharacterAssetService::Has_ActivePreparations()) return;
	}
	LEVEL_TRANSITION_REQUEST request{};
	if (!CLevelTransitionService::Try_Consume(request))
		return;
#ifdef _DEBUG
	if (m_pEffectTool) m_pEffectTool->Deactivate_AuthoringWorkspace();
    if (m_pEffectToolV2) m_pEffectToolV2->Deactivate();
	if (m_pWorldObjectTool) m_pWorldObjectTool->Deactivate();
	StopCompositionPreview(m_eCompositionPreviewOwner);
	m_pWorldObjectCatalogSource = nullptr;
	m_iWorldObjectCatalogGeneration = UINT64_MAX;
#endif
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
	const string previousLevelQualityId =
		m_RenderingProfiles.Get_LevelQualityProfileId();
	string profileStatus;
	const bool_t profileActivated = nullptr != nextLevel &&
		m_RenderingProfiles.Activate_LevelProfile(
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
	if (profileActivated)
		CPresentation_Manager::Get().Clear_Frame();
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
        if (m_pCharacterActionWorkbench) m_pCharacterActionWorkbench->On_LevelChanged();
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
		const string& rollbackQualityId = previousLevelQualityId.empty() ? previousProfileId : previousLevelQualityId;
		if (!m_RenderingProfiles.Activate_LevelProfile(rollbackQualityId, rollbackStatus) ||
			(previousProfileId != rollbackQualityId && !m_RenderingProfiles.Activate_Profile(previousProfileId, rollbackStatus)))
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
	if (eTool == DEBUG_TOOL::WORLD_OBJECT || eTool == DEBUG_TOOL::SEQUENCER_BENCHMARK)
		return m_pSequencerTool && m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::SEQUENCER)] &&
			m_pSequencerTool->Get_SelectedTarget() == (eTool == DEBUG_TOOL::WORLD_OBJECT ?
				COMPOSITION_WORKBENCH_TARGET::OBJECT : COMPOSITION_WORKBENCH_TARGET::SEQUENCE);
	const DEBUG_TOOL eCanonicalTool = DEBUG_TOOL::EFFECT_COMPOSITION == eTool ?
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
	const DEBUG_TOOL eCanonicalTool = DEBUG_TOOL::EFFECT_COMPOSITION == eTool ?
		DEBUG_TOOL::EFFECT :
		(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH == eTool ||
		 DEBUG_TOOL::KOUKU_SAYDON_ACTION_WORKBENCH == eTool) ? DEBUG_TOOL::SEQUENCER : eTool;
	if (eTool == DEBUG_TOOL::WORLD_OBJECT || eTool == DEBUG_TOOL::SEQUENCER_BENCHMARK)
	{
		if (bVisible && m_pSequencerTool)
			m_pSequencerTool->Open(eTool == DEBUG_TOOL::WORLD_OBJECT ?
				COMPOSITION_WORKBENCH_TARGET::OBJECT : COMPOSITION_WORKBENCH_TARGET::SEQUENCE);
		if (bVisible || IsDebugToolVisible(eTool)) SetDebugToolVisible(DEBUG_TOOL::SEQUENCER, bVisible);
		return;
	}
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
        m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::EFFECT_COMPOSITION)] = false;
	}
	if (!bVisible)
	{
		if (eCanonicalTool == DEBUG_TOOL::SEQUENCER)
		{
			if (m_pSequencerTool) m_pSequencerTool->Deactivate();
			StopCompositionPreview(m_eCompositionPreviewOwner);
			if (m_eDebugInputOwner == DEBUG_TOOL::WORLD_OBJECT || m_eDebugInputOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
				m_eDebugInputOwner = DEBUG_TOOL::NONE;
		}
		if (eCanonicalTool == DEBUG_TOOL::SEQUENCER || eCanonicalTool == DEBUG_TOOL::SEQUENCER_BENCHMARK)
			StopCompositionPreview(eCanonicalTool);
		if (m_eDebugInputOwner == eCanonicalTool)
			m_eDebugInputOwner = DEBUG_TOOL::NONE;
		if (m_eDebugWindowFocusPending == eCanonicalTool)
			m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
		if (DEBUG_TOOL::MAP == eCanonicalTool && nullptr != m_pMapTool)
			m_pMapTool->SetOpen(false);
		else if (DEBUG_TOOL::WORLD_OBJECT == eCanonicalTool && m_pWorldObjectTool)
			m_pWorldObjectTool->Deactivate();
		else if (DEBUG_TOOL::CAMERA == eCanonicalTool && nullptr != m_pCameraTool)
			m_pCameraTool->Deactivate();
        else if (DEBUG_TOOL::EFFECT == eCanonicalTool && m_pEffectTool)
            m_pEffectTool->Deactivate_AuthoringWorkspace();
        else if (DEBUG_TOOL::EFFECT_V2 == eCanonicalTool && m_pEffectToolV2)
            m_pEffectToolV2->Deactivate();
	}
}

void CMainApp::ClaimCompositionPreviewOwner(const DEBUG_TOOL owner)
{
	if (m_eCompositionPreviewOwner == owner) return;
	auto* previousWorkbench = m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER ?
		m_pKoukuSaydonActionWorkbench.get() :
		(m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK ? m_pSequenceActionWorkbench.get() : nullptr);
	auto* previousShell = m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER ? m_pSequencerTool.get() :
		(m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK ? m_pSequencerTool.get() : nullptr);
    if (m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
        if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
            arena->Notify_SequencePlaybackEnded();
	if (previousWorkbench)
	{
		previousWorkbench->Cancel_CompleteSequencePlay();
		auto state = previousWorkbench->Get_PreviewState();
		state.bPlaying = false;
		state.bPaused = true;
		previousWorkbench->Set_PreviewState(state);
	}
	if (previousShell)
	{
		auto state = previousShell->Get_AnimationPreviewState();
		state.bPlaying = false;
		state.bPaused = true;
		previousShell->Set_AnimationPreviewState(std::move(state));
	}
	m_eCompositionPreviewOwner = owner;
	m_eColliderAuthoringOwner = DEBUG_TOOL::NONE;
	m_iColliderAuthoringDraftGeneration = UINT64_MAX;
}

void CMainApp::StopCompositionPreview(const DEBUG_TOOL owner)
{
    if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_IsServerClock()) return;
	if (owner == DEBUG_TOOL::NONE || m_eCompositionPreviewOwner != owner) return;
	if (owner == DEBUG_TOOL::SEQUENCER_BENCHMARK) ClearKoukuSequenceArrivals();
	if (m_pAnimationTool) { std::string stoppedAnimationStatus; (void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus); }
	if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Stop_Preview();
	if (auto* arena = CLevel_KakulSaydonArena::Get_Active()) arena->Debug_StopCompositionWorldPreview();
	ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
}

void CMainApp::CloseAllDebugTools()
{
	for (size_t iTool = static_cast<size_t>(DEBUG_TOOL::NONE) + 1u;
		iTool < static_cast<size_t>(DEBUG_TOOL::COUNT); ++iTool)
	{
		const DEBUG_TOOL eTool = static_cast<DEBUG_TOOL>(iTool);
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
			m_pCharacterPreviewPanel, m_pBalanceTool.get(), m_pValtanBossTool.get(),
			m_pDevice, m_pContext);
	return S_OK;
}

HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Open");
	/* Keep the old enum value as an internal compatibility route only. */
	if (DEBUG_TOOL::EFFECT_COMPOSITION == eTool)
		return EnsureDebugTool(DEBUG_TOOL::EFFECT);
	if (eTool == DEBUG_TOOL::WORLD_OBJECT || eTool == DEBUG_TOOL::SEQUENCER_BENCHMARK)
	{
		if (FAILED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER))) return E_FAIL;
		m_pSequencerTool->Open(eTool == DEBUG_TOOL::WORLD_OBJECT ?
			COMPOSITION_WORKBENCH_TARGET::OBJECT : COMPOSITION_WORKBENCH_TARGET::SEQUENCE);
		m_eDebugInputOwner = eTool;
		return S_OK;
	}
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
	case DEBUG_TOOL::WORLD_LEVEL:
		if (!m_pWorldLevelTool) m_pWorldLevelTool = make_unique<CWorldLevelTool>();
		m_pWorldLevelTool->Open(GetWorldLevelAreaId());
		break;
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
        m_pEffectTool->Configure_AuthoringWorkspace(m_pKoukuPresentationPlayer.get());
        m_pEffectTool->Set_KoukuPatternPreviewProvider(
            [this](const std::string& effectId, EFFECT_TOOL_KOUKU_PATTERN_PREVIEW& context, std::string& status)
            {
                status.clear();
                CKoukuSaydonActionWorkbench* workbench = nullptr;
                if (m_pSequencerTool && m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE)
                    workbench = m_pSequenceActionWorkbench.get();
                else if (m_pSequencerTool && m_pSequencerTool->Is_BossSelected())
                    workbench = m_pKoukuSaydonActionWorkbench.get();
                if (!workbench || !workbench->Has_Composition()) return false;
                return CEffect_Tool::Build_KoukuPatternPreviewContext(workbench->Get_Composition(),
                    workbench->Get_SelectedPatternId(), workbench->Get_SelectedPresentationOccurrenceId(),
                    effectId, context, status);
            });
        break;
	}
    case DEBUG_TOOL::EFFECT_V2:
        if (!m_pCharacterPreviewPanel)
            m_pCharacterPreviewPanel = make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
        if (!m_pEffectToolV2)
            m_pEffectToolV2 = make_unique<CEffect_Tool_V2>(m_pDevice, m_pContext);
        m_pEffectToolV2->Configure_AuthoringWorkspace(m_pCharacterPreviewPanel, m_pKoukuPresentationPlayer.get());
        m_pEffectToolV2->Activate();
        break;
	case DEBUG_TOOL::RENDERING:
		if (nullptr == m_RenderingProfiles.Get_ActiveProfile()) return E_FAIL;
		m_bLightResourcesWindowVisible = true;
		m_bLightDetailWindowVisible = true;
		m_bLightSequencerWindowVisible = true;
		m_bRenderingQualityWindowVisible = true;
		m_bRenderQualityDraftInitialized = false;
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
		{
			m_pKoukuSaydonActionWorkbench = make_unique<CKoukuSaydonActionWorkbench>();
			m_pWorldObjectCatalogSource = nullptr;
			m_iWorldObjectCatalogGeneration = UINT64_MAX;
			m_pKoukuSaydonActionWorkbench->Set_WorldPlacementResolver(
				[](KOUKU_SAYDON_WORLD_PLACEMENT& placement, std::string& status)
				{
					auto* arena = CLevel_KakulSaydonArena::Get_Active();
					float3_t position{};
					if (!arena) { status = "WORLD placement requires the KoukuSaydon arena."; return false; }
					if (!arena->Try_Get_AuthoringForwardPlacement(position, status)) return false;
					KOUKU_SAYDON_WORLD_PLACEMENT staged;
					staged.Position = {position.x, position.y, position.z};
					placement = std::move(staged);
					return true;
				});
		}
		if (!m_pWorldObjectTool) m_pWorldObjectTool = make_unique<CWorldObjectTool>();
		m_pWorldObjectTool->Set_LinkedSaveCallbacks(
			[this](std::string& status) {
				for (auto* editor : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
				{
					if (!editor) continue;
					const std::string owner = editor == m_pKoukuSaydonActionWorkbench.get() ? "Pattern" : "Sequence";
					if (editor->Is_PublishRunning())
					{ status = owner + " Publish is already running. Wait for it to finish, then retry Object Save. All edits are preserved."; return false; }
					if (editor->Is_Dirty())
					{ status = "Save " + owner + " edits first, including newly created Logic definitions, then retry Object Save. Starting Publish is not required. All edits are preserved."; return false; }
				}
				return true;
			},
			[this](bool publishPatterns, std::string& status) {
				for (auto* editor : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
				{
					if (!editor) continue;
					const std::string owner = editor == m_pKoukuSaydonActionWorkbench.get() ? "Pattern" : "Sequence";
					if (editor->Is_PublishRunning())
					{ status = "Object is saved; linked reload waits for the running " + owner + " Publish. Retry Object Save after it finishes."; return false; }
					if (editor->Is_Dirty())
					{ status = "Object is saved; new " + owner + " edits are preserved. Save them first, then retry Object Save. Starting Publish is not required."; return false; }
				}
				if (publishPatterns && !m_pKoukuSaydonActionWorkbench)
				{
					m_pKoukuSaydonActionWorkbench = make_unique<CKoukuSaydonActionWorkbench>();
					m_pWorldObjectCatalogSource = nullptr;
					m_iWorldObjectCatalogGeneration = UINT64_MAX;
					m_pKoukuSaydonActionWorkbench->Set_WorldPlacementResolver(
						[](KOUKU_SAYDON_WORLD_PLACEMENT& placement, std::string& status)
						{
							auto* arena = CLevel_KakulSaydonArena::Get_Active();
							float3_t position{};
							if (!arena) { status = "WORLD placement requires the KoukuSaydon arena."; return false; }
							if (!arena->Try_Get_AuthoringForwardPlacement(position, status)) return false;
							KOUKU_SAYDON_WORLD_PLACEMENT staged;
							staged.Position = {position.x, position.y, position.z};
							placement = std::move(staged);
							return true;
						});
				}
				for (auto* editor : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
					if (editor && (editor->Has_Composition() || (publishPatterns && editor == m_pKoukuSaydonActionWorkbench.get())))
						if (!editor->Reload(status)) return false;
				RefreshWorldObjectResources();
				if (publishPatterns)
				{
					if (!m_pKoukuSaydonActionWorkbench->Publish_AllPatterns(status)) return false;
					status = "Map applied; linked Pattern publication started. Its status is shown in Composition.";
				}
				else status = "Map applied and the linked Sequence source reloaded for the next play.";
				return true;
			});
		if (!m_pSequenceActionWorkbench)
		{
			m_pSequenceActionWorkbench = make_unique<CKoukuSaydonActionWorkbench>(true);
			m_pSequenceActionWorkbench->Set_CompleteSequenceAdmission(
				[this](std::string_view gate, std::string& status) { return StartKoukuGateCompletePlay(gate, status); });
			m_pSequenceActionWorkbench->Set_WorldPlacementResolver(
				[](KOUKU_SAYDON_WORLD_PLACEMENT& placement, std::string& status)
				{
					auto* arena = CLevel_KakulSaydonArena::Get_Active();
					float3_t position{};
					if (!arena) { status = "WORLD placement requires the KoukuSaydon arena."; return false; }
					if (!arena->Try_Get_AuthoringForwardPlacement(position, status)) return false;
					placement = {};
					placement.Position = {position.x, position.y, position.z};
					return true;
				});
			m_pWorldObjectCatalogSource = nullptr;
			m_iWorldObjectCatalogGeneration = UINT64_MAX;
		}
		if (!m_pCharacterActionWorkbench)
		{
			if (!m_pEffectTool)
			{
				m_pEffectTool = make_unique<CEffect_Tool>(m_pDevice, m_pContext,
					m_pCharacterPreviewPanel, m_pBalanceTool.get());
				m_pEffectTool->Configure_AuthoringWorkspace(m_pKoukuPresentationPlayer.get());
			}
			m_pCharacterActionWorkbench = make_unique<CCharacterActionWorkbench>(m_pCharacterPreviewPanel,
				m_pEffectTool->Create_CompositionSequencer("character.actions"), m_pAnimationTool.get());
			m_pCharacterActionWorkbench->Set_OpenEffectResourceCallback([this](const std::string& id) {
				std::string error;
				if (!CEffectResourceCatalog::Get().Reload_Valtan(error)) { m_strToolStatus = error; return; }
				auto catalog = CEffectResourceCatalog::Get().Get_Snapshot();
				const auto* entry = catalog ? catalog->Find(id) : nullptr;
				if (!entry) { m_strToolStatus = "Effect resource is not in the active catalog: " + id; return; }
				if (entry->Key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
				{
					if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)))
						(void)m_pEffectTool->Open_AuthoringResource(entry->Key);
				}
				else if (entry->Key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_LEAF ||
					entry->Key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_GROUP)
				{
					if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT_V2)))
						(void)m_pEffectToolV2->Open_Resource(entry->Key);
				}
				else m_strToolStatus = "Effect resource has no supported authoring owner: " + id;
			});
		}
		if (nullptr == m_pSequencerTool)
		{
			m_pSequencerTool = make_unique<CSequencerTool>(
				m_pValtanActionWorkbench.get(), m_pKoukuSaydonActionWorkbench.get());
			m_pSequencerTool->Set_ActionSessions(m_pCharacterActionWorkbench.get(),
				m_pWorldObjectTool.get(), m_pSequenceActionWorkbench.get());
			/* The Map Tool hosts the same Sequence session for its integrated
			   cutscene view. One owner and one draft, borrowed per frame. */
			if (m_pMapTool)
			{
				m_pMapTool->Set_SequenceCompositionSession(m_pSequenceActionWorkbench.get());
				/* Valtan's Area hosts its own workbench: the Sequence session
				   reads the KoukuSaydon composition and has no Valtan data. */
				m_pMapTool->Set_ValtanCompositionSession(m_pValtanActionWorkbench.get());
			}
			if (m_pMapTool)
				m_pMapTool->Set_ObjectCompositionSession(m_pWorldObjectTool.get());
			m_pSequencerTool->Set_TargetChangedCallback([this](COMPOSITION_WORKBENCH_TARGET target) {
				StopCompositionPreview(m_eCompositionPreviewOwner);
				if (m_pValtanActionWorkbench) m_pValtanActionWorkbench->Set_PreviewOwnerActive(false);
				m_eDebugInputOwner = target == COMPOSITION_WORKBENCH_TARGET::OBJECT ? DEBUG_TOOL::WORLD_OBJECT :
					(target == COMPOSITION_WORKBENCH_TARGET::SEQUENCE ? DEBUG_TOOL::SEQUENCER_BENCHMARK : DEBUG_TOOL::SEQUENCER);
			});
			m_pSequencerTool->Open(ETOUI(LEVEL::KAKULSAYDON_ARENA) ==
				CGameInstance::Get().Get_CurrentLevelID() ?
				COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON : COMPOSITION_WORKBENCH_BOSS::VALTAN);
		}
		else
		{
			const auto level = CGameInstance::Get().Get_CurrentLevelID();
			const bool selectedValtan = m_pSequencerTool->Get_SelectedBoss() == COMPOSITION_WORKBENCH_BOSS::VALTAN;
			if (m_pSequencerTool->Is_BossSelected() && level == ETOUI(LEVEL::KAKULSAYDON_ARENA) && selectedValtan)
				m_pSequencerTool->Open(COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON);
			else if (m_pSequencerTool->Is_BossSelected() && level == ETOUI(LEVEL::VALTAN_ARENA) && !selectedValtan)
				m_pSequencerTool->Open(COMPOSITION_WORKBENCH_BOSS::VALTAN);
			else m_pSequencerTool->Open();
		}
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
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.CameraAndPlayer");
	const LEVEL level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
	CLevel_ValtanArena* valtan = LEVEL::VALTAN_ARENA == level ?
		CLevel_ValtanArena::Get_Active() : nullptr;
	CLevel_KakulSaydonArena* kouku = LEVEL::KAKULSAYDON_ARENA == level ?
		CLevel_KakulSaydonArena::Get_Active() : nullptr;
	CLevel_CharacterSelect* characterSelect = LEVEL::CHARACTER_SELECT == level ?
		CLevel_CharacterSelect::Get_Active() : nullptr;
	CLevel_Bern* bern = LEVEL::BERN == level ? CLevel_Bern::Get_Active() : nullptr;
	CLevel_Development* development = LEVEL::DEVELOPMENT == level || LEVEL::MAHARAKA == level ?
		CLevel_Development::Get_Active(level) : nullptr;
	shared_ptr<CCamera_Free> camera;
	CPlayerController* controller = nullptr;
	const char* mapName = "Unavailable";
	if (nullptr != valtan)
	{
		camera = valtan->Get_DebugCamera();
		controller = &valtan->Get_DebugPlayerController();
		mapName = "Valtan";
	}
	else if (nullptr != kouku)
	{
		camera = kouku->Get_DebugCamera();
		controller = &kouku->Get_DebugPlayerController();
		mapName = "KoukuSaydon";
	}
	else if (nullptr != characterSelect)
	{
		camera = characterSelect->Get_DebugCamera();
		mapName = "Character Select";
	}
	else if (nullptr != bern)
	{
		camera = bern->Get_DebugCamera();
		controller = &bern->Get_PlayerController();
		mapName = "Bern";
	}
	else if (nullptr != development)
	{
		camera = development->Get_DebugCamera();
		mapName = LEVEL::MAHARAKA == level ? "Maharaka" : "Development / Training";
	}
	if (camera && ImGui::CollapsingHeader("Map Camera / Player", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const auto setSpeed = [valtan, kouku, camera](const f32_t speed)
		{
			if (nullptr != valtan) valtan->Set_DebugCameraSpeed(speed);
			else if (nullptr != kouku) kouku->Set_DebugCameraSpeed(speed);
			else (void)camera->Set_FreeMoveSpeed(speed);
		};
		ImGui::Text("Current map: %s", mapName);
		/* Where the local player actually stands and which interact box the Server is offering
		there, so an authored trigger can be placed against real coordinates instead of guesses. */
		{
			const std::shared_ptr<CCharacter> pDebugLocal =
				nullptr != valtan ? valtan->Get_LocalCharacter() :
				(nullptr != kouku ? kouku->Get_LocalCharacter() :
					(nullptr != bern ? bern->Get_LocalCharacter() : nullptr));
			if (nullptr != pDebugLocal && nullptr != pDebugLocal->Get_Transform())
			{
				float3_t vPlayer{};
				XMStoreFloat3(&vPlayer,
					pDebugLocal->Get_Transform()->Get_State(STATE::POSITION));
				ImGui::Text("Player world: (%.1f, %.1f, %.1f)",
					vPlayer.x, vPlayer.y, vPlayer.z);
			}
			const std::string& offered =
				CCombatHUDViewModel::Get().Get_InteractPromptTriggerId();
			ImGui::Text("Interact offer: %s", offered.empty() ? "(none)" : offered.c_str());
		}
		f32_t speed = camera->Get_FreeMoveSpeed();
		if (ImGui::DragFloat("Free camera speed (m/s)", &speed, 0.5f,
			CCamera_Free::MIN_FREE_MOVE_SPEED, CCamera_Free::MAX_FREE_MOVE_SPEED,
			"%.1f", ImGuiSliderFlags_AlwaysClamp))
			setSpeed(speed);
		if (ImGui::Button("Reset speed to 20 m/s"))
			setSpeed(CCamera_Free::DEFAULT_ARENA_MOVE_SPEED);
		ImGui::TextDisabled("Shift: x%.0f (%.1f m/s).",
			CCamera_Free::FREE_MOVE_SPRINT_MULTIPLIER,
			camera->Get_FreeMoveSpeed() * CCamera_Free::FREE_MOVE_SPRINT_MULTIPLIER);
		ImGui::TextDisabled(valtan || kouku ? "Free-camera speed is saved per arena for this session." :
			"Free-camera speed lasts for this map visit.");

		// The Server projects every request onto this active world's authored navigation.
		// Bern needs this same Debug-only path to inspect the separate Bern3 deck.
		if (controller)
		{
			const bool_t freeCamera = !camera->Is_FollowRequested() &&
				!camera->Is_PresentationOverrideActive();
			ImGui::BeginDisabled(!freeCamera || controller->Is_DebugPlayerPlacementPending() ||
				controller->Is_DebugPlayerPlacementArmed());
			if (ImGui::Button("Move Player"))
			{
				const auto world = nullptr != valtan ? LostArk::Shared::WORLD_ID::VALTAN_ARENA :
					(nullptr != bern ? LostArk::Shared::WORLD_ID::BERN :
						LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA);
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
		}
		else
			ImGui::TextDisabled("F6 toggles follow / free camera when a player is available. Tab toggles mouse-look.");
	}
	if (m_pLevelNavigationDebug) m_pLevelNavigationDebug->Render_Controls();
	if (nullptr != characterSelect)
		RenderCharacterSelectFloorSwapControls();
	if (characterSelect || kouku || bern || valtan)
		RenderArenaFollowCameraSettings();
	if (nullptr != kouku)
		RenderKoukuUiPreviewControls();
}

void CMainApp::RenderCharacterSelectFloorSwapControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.CharacterSelectFloorSwap");
	if (!ImGui::CollapsingHeader("Character Select Floor Swap", ImGuiTreeNodeFlags_DefaultOpen))
		return;
	CLevel_CharacterSelect* characterSelect = CLevel_CharacterSelect::Get_Active();
	if (nullptr == characterSelect)
	{
		ImGui::TextDisabled("Enter Character Select to change its center floor.");
		return;
	}
	ImGui::PushID("CharacterSelectFloorSwap");
	if (ImGui::Button("Reload floor options"))
		(void)characterSelect->Debug_ReloadFloorSwapOptions();
	const auto& options = characterSelect->Debug_GetFloorSwapOptions();
	const auto findOption = [&options](const std::string& id)
	{
		return std::find_if(options.begin(), options.end(), [&id](const auto& option)
			{ return option.id == id; });
	};
	if (m_strCharacterSelectFloorDraftId.empty() && !options.empty())
		m_strCharacterSelectFloorDraftId = options.front().id;
	const auto selected = findOption(m_strCharacterSelectFloorDraftId);
	const char* preview = selected != options.end() ? selected->label.c_str() : "Choose a floor";
	if (ImGui::BeginCombo("Floor", preview))
	{
		for (const auto& option : options)
		{
			ImGui::PushID(option.id.c_str());
			const bool isSelected = option.id == m_strCharacterSelectFloorDraftId;
			if (ImGui::Selectable(option.label.c_str(), isSelected))
				m_strCharacterSelectFloorDraftId = option.id;
			if (isSelected) ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	if (options.empty())
		ImGui::TextDisabled("Reload floor options to load the available presets.");
	ImGui::DragFloat3("Additional offset XYZ (m)", &m_vCharacterSelectFloorOffsetMeters.x,
		0.01f, -50.f, 50.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Additional yaw (deg)", &m_fCharacterSelectFloorYawDegrees,
		0.25f, -180.f, 180.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	constexpr const char* environmentNames[] = { "Source stage environment", "Center floor environment" };
	ImGui::Combo("Environment", &m_iCharacterSelectFloorEnvironment,
		environmentNames, IM_ARRAYSIZE(environmentNames));
	ImGui::Checkbox("Compare shader default brightness (1)", &m_bCharacterSelectFloorShaderDefaultBrightness);
	ImGui::TextDisabled("Off keeps authored brightness. On sets floor and star brightness to 1; use Apply floor.");
	ImGui::TextWrapped("The preset keeps the floor at its original layer and raises only the star to clear the bridge.");
	ImGui::TextDisabled("Additional XYZ offset moves both pieces from their preset positions.");
	ImGui::BeginDisabled(findOption(m_strCharacterSelectFloorDraftId) == options.end());
	if (ImGui::Button("Apply floor"))
	{
		CHARACTER_SELECT_FLOOR_SWAP_SETTINGS settings;
		settings.offsetMeters = m_vCharacterSelectFloorOffsetMeters;
		settings.yawDegrees = m_fCharacterSelectFloorYawDegrees;
		settings.environment = static_cast<CHARACTER_SELECT_FLOOR_ENVIRONMENT>(m_iCharacterSelectFloorEnvironment);
		settings.useShaderDefaultBrightness = m_bCharacterSelectFloorShaderDefaultBrightness;
		(void)characterSelect->Debug_ApplyFloorSwap(m_strCharacterSelectFloorDraftId, settings);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Restore original floor"))
		(void)characterSelect->Debug_ResetFloorSwap();
	ImGui::SameLine();
	if (ImGui::Button("Reset adjustments"))
	{
		m_vCharacterSelectFloorOffsetMeters = {};
		m_fCharacterSelectFloorYawDegrees = 0.f;
		m_iCharacterSelectFloorEnvironment = 1;
		m_bCharacterSelectFloorShaderDefaultBrightness = false;
	}
	const auto& appliedId = characterSelect->Debug_GetFloorSwapSelectedId();
	const auto applied = findOption(appliedId);
	ImGui::TextWrapped("Applied floor: %s", appliedId.empty() ? "Original layout" :
		(applied != options.end() ? applied->label.c_str() : appliedId.c_str()));
	if (!appliedId.empty())
	{
		const auto& appliedSettings = characterSelect->Debug_GetFloorSwapSettings();
		const char* appliedEnvironment = "Unknown environment";
		switch (appliedSettings.environment)
		{
		case CHARACTER_SELECT_FLOOR_ENVIRONMENT::SOURCE_STAGE:
			appliedEnvironment = "Source stage environment";
			break;
		case CHARACTER_SELECT_FLOOR_ENVIRONMENT::CENTER_FLOOR:
			appliedEnvironment = "Center floor environment";
			break;
		}
		ImGui::Text("Applied additional offset: (%.3f, %.3f, %.3f) m | yaw: %.2f deg",
			appliedSettings.offsetMeters.x, appliedSettings.offsetMeters.y,
			appliedSettings.offsetMeters.z, appliedSettings.yawDegrees);
		ImGui::Text("Applied environment: %s", appliedEnvironment);
		ImGui::Text("Applied brightness: %s", appliedSettings.useShaderDefaultBrightness ?
			"Shader default 1 (comparison)" : "Authored material setting (star brightness 1)");
	}
	ImGui::TextDisabled("Use Apply floor after changing the draft settings.");
	ImGui::TextWrapped("After updating presets, Reset adjustments then Apply floor.");
	ImGui::TextDisabled("Session only. Leaving this map restores the original layout.");
	ImGui::TextDisabled("Close character customization before applying or restoring a floor.");
	ImGui::TextWrapped("Baked lighting stays with the source floor; it has not been rebaked for the center position.");
	const auto& status = characterSelect->Debug_GetFloorSwapStatus();
	if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
	ImGui::PopID();
}

void CMainApp::RenderArenaFollowCameraSettings()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.FollowCamera");
	const uint32_t level = CGameInstance::Get().Get_CurrentLevelID();
	auto* characterSelect = level == ETOUI(LEVEL::CHARACTER_SELECT) ? CLevel_CharacterSelect::Get_Active() : nullptr;
	auto* kouku = level == ETOUI(LEVEL::KAKULSAYDON_ARENA) ? CLevel_KakulSaydonArena::Get_Active() : nullptr;
	auto* bern = level == ETOUI(LEVEL::BERN) ? CLevel_Bern::Get_Active() : nullptr;
	auto* valtan = level == ETOUI(LEVEL::VALTAN_ARENA) ? CLevel_ValtanArena::Get_Active() : nullptr;
	if (m_iArenaCameraLastLevel != level)
	{
		m_iArenaCameraLastLevel = level;
		if (characterSelect) m_iArenaCameraSelectedMap = 0;
		else if (kouku) m_iArenaCameraSelectedMap = 1;
		else if (bern) m_iArenaCameraSelectedMap = 2;
		else if (valtan) m_iArenaCameraSelectedMap = 3;
	}
	if (!ImGui::CollapsingHeader("Player Follow Camera", ImGuiTreeNodeFlags_DefaultOpen))
		return;
	ImGui::PushID("ArenaFollowCameraSettings");
	const char* names[] = { "Character Select", "KoukuSaydon", "Bern", "Valtan" };
	const ARENA_CAMERA_MAP maps[] = { ARENA_CAMERA_MAP::CHARACTER_SELECT,
		ARENA_CAMERA_MAP::KOUKU_SAYDON, ARENA_CAMERA_MAP::BERN, ARENA_CAMERA_MAP::VALTAN };
	ImGui::Combo("Camera map", &m_iArenaCameraSelectedMap, names, IM_ARRAYSIZE(names));
	const size_t index = static_cast<size_t>(m_iArenaCameraSelectedMap);
	const ARENA_CAMERA_MAP map = maps[index];
	auto& draft = m_ArenaCameraDrafts[index];
	auto& status = m_ArenaCameraDraftStatus[index];
	auto& baseline = m_ArenaCameraSourceBaselines[index];
	const bool active = (index == 0u && characterSelect) || (index == 1u && kouku) ||
		(index == 2u && bern) || (index == 3u && valtan);
	shared_ptr<CCamera_Free> camera;
	if (index == 0u && characterSelect) camera = characterSelect->Get_DebugCamera();
	else if (index == 1u && kouku) camera = kouku->Get_DebugCamera();
	else if (index == 2u && bern) camera = bern->Get_DebugCamera();
	else if (index == 3u && valtan) camera = valtan->Get_DebugCamera();
	const auto useCurrent = [&]()
	{
		if (index == 0u && characterSelect)
		{
			draft = characterSelect->Get_FollowCameraProfile();
			status = characterSelect->Get_FollowCameraProfileStatus();
		}
		else if (index == 1u && kouku)
		{
			draft = kouku->Get_FollowCameraProfile();
			status = kouku->Get_FollowCameraProfileStatus();
		}
		else if (index == 2u && bern)
		{
			draft = bern->Get_FollowCameraProfile();
			status = bern->Get_FollowCameraProfileStatus();
		}
		else if (index == 3u && valtan)
		{
			draft = valtan->Get_FollowCameraProfile();
			status = valtan->Get_FollowCameraProfileStatus();
		}
	};
	if (!m_ArenaCameraDraftLoaded[index])
	{
		draft = CArenaCameraProfile::Default(map);
		(void)CArenaCameraProfile::Load(map, draft, status, &baseline);
		m_ArenaCameraDraftLoaded[index] = true;
	}
	const bool canPreview = active && camera && !camera->Is_PresentationOverrideActive();
	const auto applyProfile = [&]()
	{
		if (!active || !camera) return false;
		if (index == 0u) return characterSelect->Set_FollowCameraProfile(draft, status);
		if (index == 1u) return kouku->Set_FollowCameraProfile(draft, status);
		if (index == 2u) return bern->Set_FollowCameraProfile(draft, status);
		return valtan->Set_FollowCameraProfile(draft, status);
	};
	const auto preview = [&]()
	{
		if (canPreview && applyProfile()) camera->Set_FollowEnabled(true);
	};
	const auto save = [&]()
	{
		if (!CArenaCameraProfile::Save(map, draft, status, &baseline)) return;
		const std::string savedStatus = status;
		if (!active)
			status += " Saved for the next entry.";
		else if (applyProfile())
			status = savedStatus + " Character sizes applied to the current map.";
		else
			status = savedStatus + " Current map could not apply the profile: " + status;
	};
	const auto reload = [&]()
	{
		if (!CArenaCameraProfile::Load(map, draft, status, &baseline)) return;
		const std::string loadedStatus = status;
		if (!active)
			status += " Loaded for the selected map; enter it to apply these settings.";
		else if (applyProfile())
			status = loadedStatus + " Character sizes applied to the current map.";
		else
			status = loadedStatus + " Current map could not apply the profile: " + status;
	};
	bool edited = false;
	if (map == ARENA_CAMERA_MAP::KOUKU_SAYDON)
	{
		if (ImGui::Checkbox("Use source camera regions", &draft.useSourceCameraRegions))
		{
			if (draft.useSourceCameraRegions)
			{
				const auto source = CArenaCameraProfile::Default(map);
				draft.positionOffset = source.positionOffset;
				draft.rotationDegrees = source.rotationDegrees;
				draft.focusDistance = source.focusDistance;
				draft.fovYDegrees = source.fovYDegrees;
				draft.followResponse = source.followResponse;
			}
			edited = true;
		}
		if (kouku)
		{
			const auto& effective = kouku->Get_EffectiveFollowCameraProfile();
			ImGui::Text("Effective distance: %.3f m | Source regions: %s",
				effective.focusDistance, draft.useSourceCameraRegions ? "On" : "Off");
		}
	}
	const auto horizontalFov = [](f32_t vertical, f32_t aspect)
	{
		return XMConvertToDegrees(2.f * std::atan(std::tan(XMConvertToRadians(vertical) * 0.5f) * aspect));
	};
	const f32_t referenceAspect = 16.f / 9.f;
	f32_t referenceFov = horizontalFov(draft.fovYDegrees, referenceAspect);
	ImGui::TextWrapped("Smaller FOV shows a closer view; larger FOV shows more of the scene.");
	if (ImGui::SliderFloat("FOV X at 16:9 (deg)", &referenceFov,
		horizontalFov(10.f, referenceAspect), horizontalFov(150.f, referenceAspect),
		"%.2f", ImGuiSliderFlags_AlwaysClamp))
	{
		draft.fovYDegrees = XMConvertToDegrees(2.f * std::atan(
			std::tan(XMConvertToRadians(referenceFov) * 0.5f) / referenceAspect));
		draft.fovYDegrees = std::clamp(draft.fovYDegrees, 10.f, 150.f);
		draft.useSourceCameraRegions = false;
		edited = true;
	}
	f32_t orbitDistance = draft.focusDistance;
	f32_t orbitPitch = draft.rotationDegrees.x;
	f32_t orbitYaw = draft.rotationDegrees.y;
	bool orbitEdited = ImGui::DragFloat("Camera distance (m)", &orbitDistance, 0.1f, 0.1f, 1000.f,
		"%.2f", ImGuiSliderFlags_AlwaysClamp);
	orbitEdited |= ImGui::DragFloat("Camera pitch (deg)", &orbitPitch, 0.25f, -89.f, 89.f,
		"%.2f", ImGuiSliderFlags_AlwaysClamp);
	orbitEdited |= ImGui::DragFloat("Camera yaw (deg)", &orbitYaw, 0.25f, -180.f, 180.f,
		"%.2f", ImGuiSliderFlags_AlwaysClamp);
	if (orbitEdited && CArenaCameraProfile::Set_OrbitAroundFocus(draft, orbitDistance, orbitPitch, orbitYaw, status))
	{
		draft.useSourceCameraRegions = false;
		edited = true;
	}
	ImGui::TextDisabled("Distance moves the camera toward or away from the same focus. Pitch changes the ground angle; yaw circles the focus.");
	if (ImGui::TreeNodeEx("Character Size", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool sizeEdited = false;
		sizeEdited |= ImGui::SliderFloat("All characters", &draft.characterSizeMultiplier,
			0.25f, 4.f, "%.3f x", ImGuiSliderFlags_AlwaysClamp);
		const char* classNames[] = { "Lance Master", "Gunslinger", "Slayer", "Artist", nullptr, "DimensionMaster", "Warlord", "Guardian Knight" };
		for (size_t i = 0u; i < draft.classSizeMultipliers.size(); ++i)
			if (classNames[i]) sizeEdited |= ImGui::SliderFloat(classNames[i], &draft.classSizeMultipliers[i],
				0.25f, 4.f, "%.3f x", ImGuiSliderFlags_AlwaysClamp);
		sizeEdited |= ImGui::SliderFloat("Madness clown", &draft.clownSizeMultiplier,
			0.25f, 4.f, "%.3f x", ImGuiSliderFlags_AlwaysClamp);
		sizeEdited |= ImGui::SliderFloat("Mario clown", &draft.marioSizeMultiplier,
			0.25f, 4.f, "%.3f x", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::Button("Requested size defaults"))
		{
			const ARENA_CAMERA_PROFILE defaults;
			draft.characterSizeMultiplier = defaults.characterSizeMultiplier;
			draft.classSizeMultipliers = defaults.classSizeMultipliers;
			draft.clownSizeMultiplier = defaults.clownSizeMultiplier;
			draft.marioSizeMultiplier = defaults.marioSizeMultiplier;
			sizeEdited = true;
		}
		// Size tuning does not transfer camera ownership or enable F6 follow.
		if (sizeEdited && applyProfile())
			status = "Character sizes applied live. Save to keep the selected map's settings.";
		if (ImGui::Button("Save##CharacterSize")) save();
		ImGui::SameLine();
		if (ImGui::Button("Reload saved##CharacterSize")) reload();
		ImGui::TextWrapped("Class values multiply the current catalog model. Artist 1.6x, DimensionMaster 0.7x and madness clown 0.7x are the requested defaults. Mario 1x keeps its 1.5m admission height.");
		ImGui::Text("Save / Reload target: %s", names[index]);
		ImGui::TextDisabled("Sizes apply during camera sequences too. Save keeps this map's settings for local and remote characters.");
		ImGui::TreePop();
	}
	if (ImGui::TreeNodeEx("Card Maze Player Hammer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool hammerEdited = ImGui::DragFloat3("Hammer position (cm)", &draft.mazeHammerPositionCm.x, .1f, -1000.f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		hammerEdited |= ImGui::DragFloat3("Hammer rotation (deg)", &draft.mazeHammerRotationDegrees.x, .25f, -3600.f, 3600.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		hammerEdited |= ImGui::DragFloat3("Hammer size", &draft.mazeHammerScale.x, .01f, .05f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::Button("Reset player hammer"))
		{
			draft.mazeHammerPositionCm = {}; draft.mazeHammerRotationDegrees = {};
			draft.mazeHammerScale = { 1.f, 1.f, 1.f }; hammerEdited = true;
		}
		// The maze owns an override camera. Its camera preview gate must not
		// suppress the profile consumed by the player's attached hammer.
		if (hammerEdited && applyProfile())
			status = "Player hammer changes applied live. Save to keep this map's settings.";
		ImGui::SameLine();
		if (ImGui::Button("Save player hammer")) save();
		ImGui::TextWrapped("Offsets from each class's right hand. Changes apply live while holding the Card Maze hammer, including during the maze camera. Size 1 keeps the 1.120 m source hammer before character size.");
		ImGui::TextDisabled("Save keeps these values in the selected map's profile for the next entry.");
		ImGui::TreePop();
	}
	shared_ptr<CCharacter> character;
	if (index == 0u && characterSelect) character = characterSelect->Get_LocalCharacter();
	else if (index == 1u && kouku) character = kouku->Get_LocalCharacter();
	else if (index == 2u && bern) character = bern->Get_LocalCharacter();
	else if (index == 3u && valtan) character = valtan->Get_LocalCharacter();
	if (character)
		ImGui::Text("Catalog scale: %.3f | Current visual scale: %.3f",
			character->Get_CatalogPresentationScale(), character->Get_PresentationScale());
	ImGui::TextDisabled("Size 1 uses the admitted class model. Body, equipment and sockets share the visual scale.");
	const auto viewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t aspect = viewport.x > 0.f && viewport.y > 0.f ? viewport.x / viewport.y : referenceAspect;
	ImGui::Text("Vertical: %.3f deg | Horizontal at %.3f: %.3f deg",
		draft.fovYDegrees, aspect, horizontalFov(draft.fovYDegrees, aspect));
	ImGui::Text("Eye-to-focus: %.3f m | Pitch: %.2f deg | Yaw: %.2f deg",
		draft.focusDistance, draft.rotationDegrees.x, draft.rotationDegrees.y);
	ImGui::TextDisabled("Camera distance and pitch also affect screen size. Character size does not change combat ranges.");
	if (ImGui::Button("Source baseline"))
	{
		const ARENA_CAMERA_PROFILE sizeSettings = draft;
		draft = CArenaCameraProfile::Default(map);
		draft.characterSizeMultiplier = sizeSettings.characterSizeMultiplier;
		draft.classSizeMultipliers = sizeSettings.classSizeMultipliers;
		draft.clownSizeMultiplier = sizeSettings.clownSizeMultiplier;
		draft.marioSizeMultiplier = sizeSettings.marioSizeMultiplier;
		draft.mazeHammerPositionCm = sizeSettings.mazeHammerPositionCm;
		draft.mazeHammerRotationDegrees = sizeSettings.mazeHammerRotationDegrees;
		draft.mazeHammerScale = sizeSettings.mazeHammerScale;
		edited = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Before restoration"))
	{
		const ARENA_CAMERA_PROFILE sizeSettings = draft;
		draft = CArenaCameraProfile::BeforeRestoration(map);
		draft.characterSizeMultiplier = sizeSettings.characterSizeMultiplier;
		draft.classSizeMultipliers = sizeSettings.classSizeMultipliers;
		draft.clownSizeMultiplier = sizeSettings.clownSizeMultiplier;
		draft.marioSizeMultiplier = sizeSettings.marioSizeMultiplier;
		draft.mazeHammerPositionCm = sizeSettings.mazeHammerPositionCm;
		draft.mazeHammerRotationDegrees = sizeSettings.mazeHammerRotationDegrees;
		draft.mazeHammerScale = sizeSettings.mazeHammerScale;
		edited = true;
	}
	ImGui::TextDisabled("Presets replace camera pose and lens, preserving character size. Save persists this map's settings.");
	if (map == ARENA_CAMERA_MAP::KOUKU_SAYDON)
		ImGui::TextWrapped("Source baseline: 50 deg / 16 m outside the verified entrance volume, 19 m inside it. Gate 1 battle floor is outside that volume. Final retail framing remains unverified. Manual camera edits disable region selection; Mario, maze and cinematic shots retain their cameras.");
	if (ImGui::TreeNode("Advanced camera pose"))
	{
		bool poseEdited = false;
		ImGui::TextDisabled("World axes relative to player. Pitch +: down; Yaw 0: +Z.");
		poseEdited |= ImGui::DragFloat3("Position offset XYZ (m)", &draft.positionOffset.x, 0.05f, -1000.f, 1000.f,
			"%.3f", ImGuiSliderFlags_AlwaysClamp);
		poseEdited |= ImGui::DragFloat("Pitch (deg)", &draft.rotationDegrees.x, 0.25f, -89.f, 89.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		poseEdited |= ImGui::DragFloat("Yaw (deg)", &draft.rotationDegrees.y, 0.25f, -180.f, 180.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		poseEdited |= ImGui::DragFloat("Roll (deg)", &draft.rotationDegrees.z, 0.25f, -180.f, 180.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		poseEdited |= ImGui::DragFloat("Focus distance (m)", &draft.focusDistance, 0.05f, 0.1f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		poseEdited |= ImGui::DragFloat("FOV Y (deg)", &draft.fovYDegrees, 0.25f, 10.f, 150.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		poseEdited |= ImGui::DragFloat("Follow response", &draft.followResponse, 0.1f, 0.f, 60.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (poseEdited) { draft.useSourceCameraRegions = false; edited = true; }
		ImGui::TreePop();
	}
	if (edited) preview();
	ImGui::TextDisabled("Changes preview live. Save keeps them for the next entry. Response 0: immediate follow.");
	ImGui::BeginDisabled(!canPreview);
	if (ImGui::Button("Follow current map")) preview();
	ImGui::SameLine();
	if (ImGui::Button("Read current camera")) useCurrent();
	ImGui::EndDisabled();
	if (!active) ImGui::TextDisabled("Save this map, then enter it to apply its settings.");
	else if (camera && camera->Is_PresentationOverrideActive())
		ImGui::TextDisabled("Live follow-camera preview is unavailable during a camera sequence.");
	if (ImGui::Button("Save camera settings")) save();
	ImGui::SameLine();
	if (ImGui::Button("Reload saved")) reload();
	ImGui::TextWrapped("%s", CArenaCameraProfile::Path(map).generic_string().c_str());
	if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
	ImGui::PopID();
}

void CMainApp::RenderKoukuUiPreviewControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.KoukuUIPreview");
	if (!ImGui::CollapsingHeader("Kouku UI Preview (Debug)", ImGuiTreeNodeFlags_DefaultOpen))
		return;
	CCombatHUDViewModel& viewModel = CCombatHUDViewModel::Get();
	// Replication reset clears the override when the arena session ends.
	m_bKoukuUiPreview = viewModel.Is_KoukuGimmickPreviewEnabled();
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
		{ "None", "Clown", "Mario", "Dance", "Card maze" };
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
	/* Fires the floating status word over the local character in the KoukuSaydon
	arena, so the retail damagetext motion can be looked at without waiting for the
	Server to apply FEAR. The word and colour come from the same retail tables the
	live path uses (EFTable_GameMsg tip.name.skillbuffdmgfont_*, and
	EFTable_SkillBuff.FontColor = 0x8041D9 for fear). */
	if (ImGui::Button("Fire fear status word##Kouku"))
		viewModel.Debug_Fire_StatusEffectTextPreview();
	ImGui::SameLine();
	ImGui::TextDisabled("KoukuSaydon arena only");
	/* Corner minigame time limit (retail dungeontimer.gfx, titleImageType
	   KOUKUSATON). Drawn on the real screen by m_pDungeonTimerView, not on the
	   HUD Layout Tool canvas -- that tool only positions the emblem. No Server
	   deadline exists for the card maze or the Mario stage, so the countdown runs
	   in CCombatHUDViewModel. Never touches Server truth. */
	ImGui::SeparatorText("Dungeon timer (Debug)");
	{
		HUD_DUNGEON_TIMER_STATE timer = viewModel.Get_DungeonTimer();
		bool_t bRunning = viewModel.Is_DungeonTimerRunning();
		bool_t bTimerChanged = false;
		if (ImGui::Checkbox("Show timer##Kouku", &m_bDungeonTimerPreview))
		{
			timer.isVisible = m_bDungeonTimerPreview;
			timer.fSeconds = m_fDungeonTimerStartSeconds;
			if (!m_bDungeonTimerPreview)
				bRunning = false;
			bTimerChanged = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.f);
		if (ImGui::InputFloat("Start s##Kouku", &m_fDungeonTimerStartSeconds, 0.f, 0.f, "%.0f"))
		{
			m_fDungeonTimerStartSeconds = (std::clamp)(m_fDungeonTimerStartSeconds, 0.f, 3599.f);
			timer.fSeconds = m_fDungeonTimerStartSeconds;
			bTimerChanged = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.f);
		if (ImGui::InputFloat("Warn s##Kouku", &m_fDungeonTimerWarningSeconds, 0.f, 0.f, "%.0f"))
		{
			m_fDungeonTimerWarningSeconds = (std::clamp)(m_fDungeonTimerWarningSeconds, 0.f, 600.f);
			bTimerChanged = true;
		}
		if (ImGui::Button(bRunning ? "Pause##KoukuTimer" : "Run##KoukuTimer"))
		{
			bRunning = !bRunning;
			if (bRunning)
			{
				m_bDungeonTimerPreview = true;
				timer.isVisible = true;
				/* A stopped clock sitting at zero is a fresh run, not a resume. Without
				   this the countdown starts at 0, which is below the warning threshold,
				   so it comes up in the warning colour and the split readout. */
				if (timer.fSeconds <= 0.f)
					timer.fSeconds = m_fDungeonTimerStartSeconds;
			}
			bTimerChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##KoukuTimer"))
		{
			timer.fSeconds = m_fDungeonTimerStartSeconds;
			bRunning = false;
			bTimerChanged = true;
		}
		ImGui::SameLine();
		ImGui::TextDisabled("%02d:%02d", static_cast<int32_t>(timer.fSeconds) / 60,
			static_cast<int32_t>(timer.fSeconds) % 60);
		if (bTimerChanged)
		{
			timer.fWarningSeconds = m_fDungeonTimerWarningSeconds;
			viewModel.Debug_Set_DungeonTimer(timer, bRunning);
		}
	}
	if (bChanged)
	{
		m_KoukuUiPreview.isValid = m_bKoukuUiPreview;
		m_KoukuUiPreview.iMadnessMaximum = 100u;
		viewModel.Debug_Set_KoukuGimmickPreview(m_KoukuUiPreview);
	}
	ImGui::TextDisabled("Preview only (no Server truth). Modes: %zu loaded.", m_KoukuHudModes.size());
}

void CMainApp::RenderDebugLevelNavigation()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.LevelNavigation");
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
			"Data/Effects/V2", DEBUG_TOOL::EFFECT_V2 },
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
			startsWith(path, "Resources/Effect/KoukuSaydon/") ||
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
			"MN_RPCT_00", "MN_RPCT_03", "MN_RPCT_05", "MN_RPCT_06",
			"MN_RPCT_07", "MN_RPCZ_00-1", "MN_RPCZ_00"
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

#ifdef _DEBUG
namespace
{
	bool_t Read_EncoreFile(const std::filesystem::path& path, std::string& text)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return false;
		text.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
		return !input.bad();
	}

	/* Stage both documents before replacing either. Backups survive a failed
	   rollback; stale authoring bytes never get silently overwritten. */
	bool_t Commit_EncoreFiles(
		const std::array<std::filesystem::path, 1>& paths,
		const std::array<std::string, 1>& expected,
		const std::array<std::string, 1>& replacements,
		std::string& status)
	{
		status.clear();
		const std::wstring suffix = L".kouku-encore." +
			std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(GetTickCount64());
		std::array<std::filesystem::path, 1> staged, backups;
		std::size_t promoted = 0u;
		auto cleanup = [&]()
		{
			for (const auto& path : staged)
			{
				std::error_code error;
				if (!path.empty()) std::filesystem::remove(path, error);
			}
		};
		for (std::size_t i = 0u; i < paths.size(); ++i)
		{
			std::string current;
			if (!Read_EncoreFile(paths[i], current) || current != expected[i])
			{
				status = "Encore source changed; Reload Baseline before saving. Nothing was written.";
				cleanup();
				return false;
			}
			staged[i] = paths[i]; staged[i] += suffix + L".tmp";
			backups[i] = paths[i]; backups[i] += suffix + L".rollback";
			const HANDLE file = CreateFileW(staged[i].c_str(), GENERIC_WRITE, 0,
				nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
			DWORD written = 0u;
			const bool_t durable = INVALID_HANDLE_VALUE != file &&
				WriteFile(file, replacements[i].data(),
					static_cast<DWORD>(replacements[i].size()), &written, nullptr) &&
				written == replacements[i].size() && FlushFileBuffers(file);
			if (INVALID_HANDLE_VALUE != file) CloseHandle(file);
			std::string verified;
			if (!durable || !Read_EncoreFile(staged[i], verified) || verified != replacements[i])
			{
				status = "Could not stage encore files; original files are unchanged.";
				cleanup();
				return false;
			}
		}
		for (std::size_t i = 0u; i < paths.size(); ++i)
		{
			std::string current;
			if (!Read_EncoreFile(paths[i], current) || current != expected[i])
			{
				status = "Encore source changed during save; reverting committed files.";
				break;
			}
			if (!ReplaceFileW(paths[i].c_str(), staged[i].c_str(), backups[i].c_str(), 0, nullptr, nullptr))
			{
				const DWORD error = GetLastError();
				status = "Encore file replacement failed (" + std::to_string(error) +
					"); reverting committed files.";
				// ReplaceFile may move the original to the backup before failing.
				// Restore that exact source without replacing a concurrent writer.
				if (ERROR_UNABLE_TO_MOVE_REPLACEMENT_2 == error &&
					(!Read_EncoreFile(backups[i], current) || current != expected[i] ||
					 !MoveFileExW(backups[i].c_str(), paths[i].c_str(), MOVEFILE_WRITE_THROUGH)))
					status += " Recovery copy retained at " + backups[i].string();
				break;
			}
			++promoted;
			// The atomic replace captures the source it actually replaced. A
			// writer between our read and replace must be restored, not lost.
			if (!Read_EncoreFile(backups[i], current) || current != expected[i])
			{
				status = "Encore source changed at replacement; reverting committed files.";
				break;
			}
			if (!Read_EncoreFile(paths[i], current) || current != replacements[i])
			{
				status = "Encore write verification failed; reverting committed files.";
				break;
			}
		}
		if (promoted != paths.size() || !status.empty())
		{
			while (promoted > 0u)
			{
				const std::size_t i = --promoted;
				std::string current;
				if (!Read_EncoreFile(paths[i], current) || current != replacements[i] ||
					!MoveFileExW(backups[i].c_str(), paths[i].c_str(),
						MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
				{
					status += " Recovery copy retained at " + backups[i].string();
				}
			}
			cleanup();
			return false;
		}
		for (const auto& path : backups)
		{
			std::error_code error;
			std::filesystem::remove(path, error);
		}
		cleanup();
		return true;
	}

	bool Patch_EncoreNumber(std::string& text, const std::string_view anchor,
		const std::string_view key, const double value)
	{
		const auto row = text.find(anchor);
		const auto field = row == std::string::npos ? row : text.find(key, row);
		if (field == std::string::npos || !std::isfinite(value)) return false;
		auto begin = text.find(':', field + key.size());
		if (begin == std::string::npos) return false;
		begin = text.find_first_not_of(" \t\r\n", begin + 1u);
		const auto end = text.find_first_not_of("0123456789.eE+-", begin);
		if (begin == std::string::npos || begin == end) return false;
		std::ostringstream number; number << std::setprecision(9) << value;
		text.replace(begin, end - begin, number.str());
		return true;
	}

	void Render_KoukuEncoreRotation(CLevel_KakulSaydonArena& arena)
	{
		static bool loaded = false, preview = false;
		static float baselineYaw = 0.f, targetYaw = 0.f;
		static std::string status;
		constexpr const char* placementId = "boss.kakulsaydon.bingo.saydon";
		const auto path = CProjectDataRoot::Resolve(L"Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json");
		const auto npc = arena.Debug_FindArenaBossNpc("BOSS_KAKULSAYDON_BINGO_SAYDON");
		const auto load = [&]() {
			CWorldGameplayDocument document;
			if (!document.Load(path, "LV_LUT_MIDNIGHTC_ED", status)) return false;
			const auto* row = document.Find(placementId);
			if (!row) { status = "Encore placement is missing."; return false; }
			baselineYaw = targetYaw = row->yawDegrees;
			loaded = true; preview = false;
			if (npc) npc->Set_DebugPresentationYawOffset(0.f);
			status = "Loaded the saved Encore rotation.";
			return true;
		};
		ImGui::SeparatorText("Bingo Encore Rotation");
		if (!loaded) (void)load();
		ImGui::BeginDisabled(!loaded);
		if (ImGui::DragFloat("Encore yaw (degrees)", &targetYaw, .25f, -360.f, 360.f,
			"%.2f", ImGuiSliderFlags_AlwaysClamp)) preview = true;
		if (ImGui::Button("Save Encore Rotation"))
		{
			std::string original;
			CWorldGameplayDocument latest;
			if (Read_EncoreFile(path, original) && latest.Load(path, "LV_LUT_MIDNIGHTC_ED", status))
			{
				const auto* row = latest.Find(placementId);
				if (!row || std::abs(row->yawDegrees - baselineYaw) > .00001f)
					status = "Encore yaw changed on disk. Reload Saved Rotation before saving.";
				else
				{
					std::string replacement = original;
					DATA_JSON_VALUE checked;
					const float wrapped = std::fmod(std::fmod(targetYaw, 360.f) + 360.f, 360.f);
					if (!Patch_EncoreNumber(replacement, "\"" + std::string(placementId) + "\"", "\"yawDegrees\"", wrapped) ||
						!Patch_EncoreNumber(replacement, "\"revision\"", "\"revision\"", latest.Get_Revision() + 1.0) ||
						!CDataJson::Parse(replacement, checked, status))
						status = "Could not validate the Encore rotation patch.";
					else if (Commit_EncoreFiles({path}, {original}, {replacement}, status))
					{
						baselineYaw = targetYaw = wrapped;
						status = "Saved Encore rotation. Publish World Gameplay and reload the Server world to use it for combat.";
					}
				}
			}
			else if (status.empty()) status = "Could not read the latest Encore placement.";
		}
		ImGui::SameLine();
		if (ImGui::Button("Reload Saved Rotation")) (void)load();
		ImGui::SameLine();
		if (ImGui::Button("Reset Rotation Preview"))
		{ preview = false; targetYaw = baselineYaw; if (npc) npc->Set_DebugPresentationYawOffset(0.f); }
		ImGui::EndDisabled();
		if (preview && npc) npc->Set_DebugPresentationYawOffset(targetYaw - npc->Get_DebugUnadjustedYawDegrees());
		ImGui::TextDisabled(npc ? "Live authoring preview; combat keeps the Server rotation." : "Spawn Bingo Saydon to preview rotation.");
		if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
	}

}
#endif

namespace
{
	/* One F1 "Normal Monster 1/2" button. The trigger name, spawn group and maxAlive
	   mirror the Server's WAVE_MONSTER_BUTTON_ROW table and the authored group; the
	   tooltip is their only consumer. A press only asks the Server, which owns the
	   mapping, the removal of the live monsters and the wave itself. */
	struct DEBUG_WAVE_MONSTER_BUTTON final
	{
		LostArk::Shared::WAVE_MONSTER_BUTTON eButton;
		const char* pLabel;
		const char* pTriggerName;
		const char* pSpawnGroupId;
		uint32_t iMaxAlive;
	};

	constexpr DEBUG_WAVE_MONSTER_BUTTON KOUKU_WAVE_MONSTER_BUTTONS[] =
	{
		{ LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1, "Normal Monster 1##KoukuWave", "Book1_Monsters", "spawn.kouku.book1", 22u },
		{ LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_2, "Normal Monster 2##KoukuWave", "Book2_Monsters", "spawn.kouku.book2", 15u },
	};

	constexpr DEBUG_WAVE_MONSTER_BUTTON VALTAN_WAVE_MONSTER_BUTTONS[] =
	{
		{ LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1, "Normal Monster 1##ValtanWave", "Stage_1", "spawn.valtan.stage01", 10u },
		{ LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_2, "Normal Monster 2##ValtanWave", "Stage_2", "spawn.valtan.stage03", 10u },
	};

	template <size_t COUNT>
	void Render_DebugWaveMonsterButtons(
		CPlayerController& controller,
		const DEBUG_WAVE_MONSTER_BUTTON (&buttons)[COUNT])
	{
		for (size_t iButton = 0; iButton < COUNT; ++iButton)
		{
			const DEBUG_WAVE_MONSTER_BUTTON& button = buttons[iButton];
			if (0 != iButton)
				ImGui::SameLine();
			if (ImGui::Button(button.pLabel, ImVec2(160.f, 0.f)))
				(void)controller.Request_DebugResummonWaveMonsters(button.eButton);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Trigger %s -> spawn group %s (max alive %u)\n"
					"Asks the Server to remove that group's live monsters and summon the wave again at its authored anchors.\n"
					"Debug only: stepping into the trigger no longer raises it.",
					button.pTriggerName, button.pSpawnGroupId, button.iMaxAlive);
			}
		}
	}
}

void CMainApp::RenderValtanArenaControls()
{
	/* Hidden outside the arena so the hub does not carry an empty header there. */
	if (ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return;
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.ValtanArena");
	ImGui::SeparatorText("Valtan Arena");
	CLevel_ValtanArena* pArena = CLevel_ValtanArena::Get_Active();
	if (nullptr == pArena)
	{
		ImGui::TextDisabled("Valtan Arena Level instance is unavailable.");
		return;
	}
    auto& controller = pArena->Get_DebugPlayerController();
    ImGui::BeginDisabled(controller.Is_DebugPlayerPlacementPending() || pArena->Is_DebugValtanBossCommandPending());
    if (ImGui::Button("Start Position"))
        (void)controller.Request_DebugTeleportToPosition(LostArk::Shared::WORLD_ID::VALTAN_ARENA, 8.8f, 9.77f, -20.22f);
    ImGui::SameLine();
    if (ImGui::Button("Before Entrance"))
        (void)controller.Request_DebugTeleportToPosition(LostArk::Shared::WORLD_ID::VALTAN_ARENA, 125.9f, 23.0176f, -93.1f);
    ImGui::SameLine();
    if (ImGui::Button("Arena Start"))
        (void)controller.Request_DebugTeleportToPosition(LostArk::Shared::WORLD_ID::VALTAN_ARENA, 147.75f, 23.0176f, -117.25f);
    if (ImGui::Button("Despawn Valtan Boss"))
    {
        std::string status;
        (void)pArena->Debug_DespawnValtanBoss(status);
    }
    ImGui::EndDisabled();
    if (!pArena->Get_DebugValtanBossCommandStatus().empty())
        ImGui::TextWrapped("%s", pArena->Get_DebugValtanBossCommandStatus().c_str());
    if (ImGui::TreeNode("Valtan Presentation Status"))
    {
        ImGui::TextWrapped("%s", pArena->Get_DebugValtanPresentationDiagnostic().c_str());
        ImGui::TextWrapped("%s", pArena->Get_SourceCinematicPreparationStatus().c_str());
        ImGui::TreePop();
    }
	ImGui::SeparatorText("Wave Monsters");
	ImGui::TextDisabled(
		"Debug builds no longer raise the Stage_1 / Stage_2 corridor waves when you step into their trigger; these buttons summon them again.");
	Render_DebugWaveMonsterButtons(pArena->Get_DebugPlayerController(), VALTAN_WAVE_MONSTER_BUTTONS);
}

void CMainApp::RenderKoukuSaydonArenaControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.KoukuArena");
	if (!ImGui::CollapsingHeader("KoukuSaydon Arena", ImGuiTreeNodeFlags_DefaultOpen))
		return;
	ImGui::TextDisabled(
		"Each gate asks the Server to raise its disabled boss placements, moves only your player, and points the boss HUD at that gate's boss.");
	ImGui::TextDisabled(
		"Spawned bosses wait idle. The Boss Tool (Play Isolated / Start Full Pattern) and Complete Play target the raised gate boss; a pattern plays only when the Product lists that boss body.");
	if (ETOUI(LEVEL::KAKULSAYDON_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
	{
		ImGui::TextDisabled(
			"Enter KoukuSaydon Arena through Lobby Server admission first.");
		return;
	}
	CLevel_KakulSaydonArena* pArena = CLevel_KakulSaydonArena::Get_Active();
	if (nullptr == pArena)
	{
		ImGui::TextDisabled("KoukuSaydon Arena Level instance is unavailable.");
		return;
	}
#ifdef _DEBUG
	/* Raid-clear MVP award page. Presentation only: this shows a sample page so
	the layout and the intro timing can be looked at. Nothing decides an MVP yet --
	no Server contribution tracking exists, so the numbers below are made up. */
	ImGui::SeparatorText("MVP Result Page (Debug)");
	ImGui::TextDisabled(
		"Sample page: three contribution rows, three party columns, the default background.");
	ImGui::TextDisabled(
		"Intro runs 135 frames at the source movie's 40fps (3.375s); the medal strip waits 3.5s (mvp.gfx's own Setting component).");
	{
		const bool_t bMvpVisible = pArena->Debug_Is_MvpResultVisible();
		if (ImGui::Button("Play Dungeon Clear -> MVP"))
			pArena->Debug_Play_ClearThenMvp();
		ImGui::SameLine();
		if (ImGui::Button(bMvpVisible ? "Replay MVP Page" : "Show MVP Page"))
			pArena->Debug_Show_MvpResult();
		ImGui::SameLine();
		if (ImGui::Button("Hide"))
			pArena->Debug_Hide_MvpResult();
	}

	/* Bingo board check. Play1 paints cells 0, 1 and 2 white; Play2 paints 3
	and 4, which completes the first row and turns those five red. The Server
	owns both masks, so these only ask. */
	if (ImGui::TreeNode("Kouku Whirlwind Hammer Transform"))
	{
		if ((m_pKoukuSaydonActionWorkbench || SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER))) && m_pKoukuSaydonActionWorkbench)
			m_pKoukuSaydonActionWorkbench->Render_WorldPlacementTuning("KAKULSAYDON_G1_PATTERN_24",
				"KAKULSAYDON_G1_PATTERN_24.world.1");
		ImGui::TreePop();
	}
	ImGui::SeparatorText("Bingo Board");
	ImGui::TextDisabled(
		"Play1 paints cells 0-2, Play2 paints 3-4 and completes row 0. Rows, columns and both diagonals count.");
	{
		CPlayerController& bingoController = pArena->Get_DebugPlayerController();
		if (ImGui::Button("Bingo_Play1", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoFill(0x7u, false);
		ImGui::SameLine();
		if (ImGui::Button("Bingo_Play2", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoFill(0x18u, false);
		ImGui::SameLine();
		if (ImGui::Button("Bingo_Reset", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoFill(0u, true);
		/* Bomb: the Server marks this player, holds it for
		KOUKU_BINGO_BOMB_MARK_MS and then plants the bomb where the player
		was standing. Nothing paints the board yet. */
		if (ImGui::Button("Bingo_Bomb", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoBomb();
		ImGui::SameLine();
		/* Hammer: the Server rolls one of the twenty row/column ends, holds
		it in the sky, drops it and sweeps that line. Diagonals are never
		swept. */
		if (ImGui::Button("Bingo_Hammer", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoHammer();
		/* Wave monsters: Debug builds no longer raise Book1_Monsters / Book2_Monsters
		when a player steps into the trigger, so these two buttons ask the Server to
		summon them again. */
		Render_DebugWaveMonsterButtons(bingoController, KOUKU_WAVE_MONSTER_BUTTONS);
		const auto& board = CCombatHUDViewModel::Get().Get_BingoBoard();
		ImGui::TextDisabled("white 0x%07X   red 0x%07X   bombs %u",
			board.iWhiteMask, board.iRedMask,
			static_cast<std::uint32_t>(board.iBombCount));
		for (std::uint8_t iBomb = 0u; iBomb < board.iBombCount; ++iBomb)
		{
			const auto& bomb = board.Bombs[iBomb];
			ImGui::TextDisabled("  bomb %u  %s  carrier %u  x %.2f  z %.2f",
				static_cast<std::uint32_t>(iBomb),
				LostArk::Shared::BINGO_BOMB_PHASE::MARKED == bomb.ePhase ?
					"MARKED" : "PLANTED",
				static_cast<std::uint32_t>(bomb.iCarrierNetEntityId),
				bomb.fPositionX, bomb.fPositionZ);
		}
		if (LostArk::Shared::BINGO_HAMMER_PHASE::NONE != board.Hammer.ePhase)
		{
			const char* phase =
				LostArk::Shared::BINGO_HAMMER_PHASE::RAISED == board.Hammer.ePhase ? "RAISED" :
				LostArk::Shared::BINGO_HAMMER_PHASE::DESCENDING == board.Hammer.ePhase ? "DESCENDING" :
				"SWEEPING";
			ImGui::TextDisabled("  hammer anchor %d (line %d)  %s  ticks %u..%u",
				board.Hammer.iAnchor, board.Hammer.iAnchor / 2, phase,
				board.Hammer.iPhaseStartTick, board.Hammer.iPhaseEndTick);
		}
	}
	ImGui::Separator();
#endif
	const auto& gates = CLevel_KakulSaydonArena::Get_DebugGates();
	/* No gate may be pressed while the previous player move is unanswered:
	   the Server would replace the bosses but refuse the second move, leaving
	   the player on the old gate. The raised gate cannot be re-pressed. */
	const bool_t placementPending = pArena->Is_DebugGatePending() ||
		pArena->Get_DebugPlayerController().Is_DebugPlayerPlacementPending();
	ImGui::BeginDisabled(placementPending);
	if (ImGui::Button("Return to Start", ImVec2(260.f, 0.f)))
	{
		std::string status;
		(void)pArena->Debug_ReturnToStart(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("Reset this arena's bosses and entry triggers; return your player to start.");
	ImGui::BeginDisabled(placementPending);
	if (ImGui::Button("3\xea\xb4\x80\xeb\xac\xb8 \xec\x9e\x85\xec\x9e\xa5 \xec\xa0\x84 \xea\xb3\xb5\xea\xb0\x84", ImVec2(260.f, 0.f)))
	{
		(void)pArena->Get_DebugPlayerController().Request_DebugTeleportToPosition(
			LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA,
			-17.509552f, 25.600000f, 960.530156f);
	}
	ImGui::EndDisabled();
	for (size_t iGate = 0u; iGate < gates.size(); ++iGate)
	{
		const CLevel_KakulSaydonArena::KAKUL_DEBUG_GATE& gate = gates[iGate];
		ImGui::PushID(static_cast<int32_t>(iGate));
		const bool_t deferred = nullptr != gate.pDeferredReason;
		const bool_t active = iGate == pArena->Get_ActiveDebugGate();
		ImGui::BeginDisabled(deferred || active || placementPending);
		if (ImGui::Button(gate.pLabel, ImVec2(260.f, 0.f)))
		{
			std::string status;
			(void)pArena->Debug_ActivateGate(iGate, status);
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (deferred)
		{
			ImGui::TextDisabled("%s", gate.pDeferredReason);
		}
		else if (active)
		{
			ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.45f, 1.f),
				"active | player (%.2f, %.2f, %.2f) | HUD %s",
				gate.vPlayerPosition.x, gate.vPlayerPosition.y, gate.vPlayerPosition.z,
				nullptr != gate.pHudFocusArchetypeId ?
					gate.pHudFocusArchetypeId : "(none)");
		}
		else
		{
			ImGui::TextDisabled("player (%.2f, %.2f, %.2f) | HUD %s",
				gate.vPlayerPosition.x, gate.vPlayerPosition.y, gate.vPlayerPosition.z,
				nullptr != gate.pHudFocusArchetypeId ?
					gate.pHudFocusArchetypeId : "(none)");
		}
#ifdef _DEBUG
		if (gate.pAuditionPlacementId &&
			std::string_view(gate.pAuditionPlacementId) == "boss.kakulsaydon.bingo.saydon")
			Render_KoukuEncoreRotation(*pArena);
#endif
		ImGui::PopID();
	}
	ImGui::BeginDisabled(placementPending);
	if (ImGui::SmallButton("Despawn Arena Bosses"))
	{
		std::string status;
		(void)pArena->Debug_DespawnArenaBosses(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	const std::string& focus = CCombatHUDViewModel::Get().Get_BossFocusArchetype();
	ImGui::TextDisabled("HUD focus: %s",
		focus.empty() ? "(last primary boss)" : focus.c_str());
	ImGui::BeginDisabled(placementPending);
	if (ImGui::SmallButton("Despawn Fire Object"))
	{
		std::string status;
		(void)pArena->Debug_DespawnFireObjects(status);
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Remove the gate-entry outer fire. Activate Gate 3 again to restore it.");
	ImGui::TextDisabled("Pattern audition target: %s (%s)",
		CKoukuSaydonPatternAuditionService::Get().Get_TargetBossPlacementId().c_str(),
		CKoukuSaydonPatternAuditionService::Get().Get_TargetBossArchetypeId().c_str());
	if (placementPending)
		ImGui::TextDisabled("Gate controls wait for all boss-spawn and player-move replies.");
	ImGui::TextWrapped("%s", pArena->Get_DebugGateStatus().c_str());
	ImGui::TextWrapped("%s",
		pArena->Get_DebugPlayerController().Get_DebugPlayerPlacementStatus().c_str());
	if (ImGui::CollapsingHeader("Mario Controls (Debug Jump)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextWrapped("Mario 1/2/3/4: auto Clown. Left / Right: move along the fixed course line (release to stop). Camera / mouse cannot steer the player. Up: use an offered crossing, otherwise jump along the same line (up to 4 m / 0.6 s). Down / Shift jump: disabled. F6 free camera keeps Shift acceleration.");
		ImGui::TextWrapped("%s", pArena->Get_DebugPlayerController().Get_DebugMarioJumpStatus().c_str());
		ImGui::TextWrapped("0: return from Mario to Gate 3 through the Server-approved exit.");
		ImGui::TextWrapped("%s", pArena->Get_DebugPlayerController().Get_MarioReturnStatus().c_str());
	}

	// Folding keeps the last gaze state owned by the active arena.
	if (ImGui::CollapsingHeader("Find the Real Saydon - Sight Collider", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static CKoukuSaydonCompositionDocument gazeDocument;
		static bool gazeLoadAttempted = false;
		static bool gazeLoaded = false;
		static bool gazeVisible = false;
		static float gazeHalfAngle = 45.f, gazeDistance = 30.f;
		static std::string gazeStatus;
		const bool reloadGaze = ImGui::SmallButton("Reload Sight Settings");
		if (!gazeLoadAttempted || reloadGaze)
		{
			// A rejected document keeps its error until an explicit reload. Retrying
			// the complete composition from this render path stalls every F1 frame.
			gazeLoadAttempted = true;
			gazeLoaded = gazeDocument.Reload(gazeStatus);
			if (gazeLoaded)
				for (const auto& logic : gazeDocument.Get_LastGood().Logics)
					if (logic.strJudgementKind == "GAZE_REAL_BOSS")
					{ gazeHalfAngle = static_cast<float>(logic.fHalfAngleDegrees); gazeDistance = static_cast<float>(logic.fMaxDistanceM); break; }
		}
		ImGui::Checkbox("Debug Render##RealSaydonSight", &gazeVisible);
		float fullAngle = gazeHalfAngle * 2.f;
		if (ImGui::SliderFloat("View angle (degrees)", &fullAngle, 2.f, 360.f)) gazeHalfAngle = fullAngle * .5f;
		ImGui::SliderFloat("View distance (m)", &gazeDistance, .1f, 100.f);
		pArena->Set_DebugGazeView(gazeVisible, gazeHalfAngle, gazeDistance);
		if (gazeLoaded && ImGui::SmallButton("Save Sight Settings"))
		{
			auto candidate = gazeDocument.Get_LastGood();
			for (auto& logic : candidate.Logics)
				if (logic.strJudgementKind == "GAZE_REAL_BOSS")
				{ logic.fHalfAngleDegrees = gazeHalfAngle; logic.fMaxDistanceM = gazeDistance; }
			if (gazeDocument.Save_Atomic(candidate, gazeStatus))
				gazeStatus = "Saved. Publish Gameplay Balance and restart Server to apply judgement settings.";
		}
		if (!gazeStatus.empty()) ImGui::TextWrapped("%s", gazeStatus.c_str());
	}

	/* Madness avatar: the Server owns the form and the snapshot swaps the
	   body, so the buttons only submit intent and follow the replicated form. */
	if (ImGui::CollapsingHeader("Clown", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const HUD_PLAYER_STATE& hudPlayer = CCombatHUDViewModel::Get().Get_Player();
		const bool_t isClown =
			LostArk::Shared::PLAYER_MADNESS_FORM::CLOWN == hudPlayer.eMadnessForm;
		CPlayerController& controller = pArena->Get_DebugPlayerController();
		int mode = static_cast<int>(hudPlayer.eKoukuHudMode);
		if (ImGui::Combo("HUD mode", &mode, "Player\0Clown\0Mario\0Dance\0Card Maze\0"))
			(void)controller.Request_DebugKoukuHudMode(static_cast<LostArk::Shared::KOUKU_HUD_MODE>(mode));
		ImGui::BeginDisabled(isClown || controller.Is_DebugMadnessFormPending());
		if (ImGui::Button("Change to Clown", ImVec2(160.f, 0.f)))
			(void)controller.Request_DebugMadnessForm(LostArk::Shared::PLAYER_MADNESS_FORM::CLOWN);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!isClown || controller.Is_DebugMadnessFormPending());
		if (ImGui::Button("Return to Player", ImVec2(160.f, 0.f)))
			(void)controller.Request_DebugMadnessForm(LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::TextDisabled("Madness %u / %u | form: %s",
			hudPlayer.iCurrentMadness, hudPlayer.iMaximumMadness,
			isClown ? "clown" : "player");
		if (!controller.Get_DebugMadnessFormStatus().empty())
			ImGui::TextWrapped("%s", controller.Get_DebugMadnessFormStatus().c_str());
	}
}

bool_t CMainApp::PrepareKoukuGateCompletePlay(const std::string_view gateId, std::string& status)
{
    auto* arena = CLevel_KakulSaydonArena::Get_Active();
    if (!arena || !arena->Get_PlayerCommandSink() || m_KoukuRaidResourcePreparation ||
        (m_pKoukuSaydonBossTool && m_pKoukuSaydonBossTool->Is_PlayPreparationPending()) ||
        !m_strKoukuCompletePlayFlowGate.empty() || m_iKoukuRaidPendingRequest ||
        CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight() || CKoukuSaydonPatternAuditionService::Get().Get_FlowSnapshot().bActive)
    { status = "Complete Play requires an idle Kouku arena with a Server connection."; return false; }
    if (gateId != "GATE1" && gateId != "GATE2" && gateId != "GATE3" && gateId != "BINGO")
    { status = "Complete raid playback requires a saved Gate or Bingo flow."; return false; }
    if ((m_pKoukuSaydonActionWorkbench && (m_pKoukuSaydonActionWorkbench->Is_Dirty() || m_pKoukuSaydonActionWorkbench->Is_PublishRunning())) ||
        (m_pSequenceActionWorkbench && m_pSequenceActionWorkbench->Is_Dirty()))
    { status = "Save the Action and Sequence documents and publish before Complete Play."; return false; }
    if (!m_pKoukuSaydonBossTool) m_pKoukuSaydonBossTool = make_unique<CKoukuSaydonBossTool>();
    if (!m_pKoukuSaydonBossTool->Reload(status) || !m_pKoukuSaydonBossTool->Validate_PatternFlow(gateId, status)) return false;
    CKoukuSaydonCompositionDocument actions;
    if (!actions.Reload(status)) return false;
    if (actions.Get_LastGood().iRevision != m_pKoukuSaydonBossTool->Get_SourceRevision())
    { status = "Saved Actions differ from the published Product. Publish Saved Patterns, then retry."; return false; }
    return true;
}

bool_t CMainApp::StartKoukuGateCompletePlay(const std::string_view gateId, std::string& status)
{
    using namespace LostArk::Shared;
    if (!PrepareKoukuGateCompletePlay(gateId, status)) return false;
    CKoukuSaydonCompositionDocument sequences(CKoukuSaydonCompositionDocument::Resolve_SequencePath());
    if (!sequences.Reload(status)) return false;
    if (!m_iNextKoukuRaidRequest) { status = "Raid request sequence is exhausted."; return false; }
    auto* arena = CLevel_KakulSaydonArena::Get_Active();
    C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
    request.iRequestSequence = m_iNextKoukuRaidRequest++; request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
    request.ExpectedGameplayRevision = CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
    request.iActionSourceRevision = m_pKoukuSaydonBossTool->Get_SourceRevision();
    request.iSequenceSourceRevision = sequences.Get_LastGood().iRevision; request.strStartGateId = std::string(gateId);
    KOUKU_RAID_RESOURCE_PREPARATION pending;
    pending.request = request;
    pending.patternIds = m_pKoukuSaydonBossTool->Get_PlayAllPatternIds();
    pending.worldGeneration = CNetworkManager::Get().Get_WorldInboundGeneration();
    pending.deadline = std::chrono::steady_clock::now() + std::chrono::minutes(20);
    arena->Debug_ResetCompletePlayPreparation();
    m_KoukuRaidResourcePreparation = std::move(pending);
    m_strKoukuRaidReplyStatus.clear();
    status = m_strKoukuCompletePlayStatus = "Preparing all raid Effect/Sequence/WORLD dependencies before the Server start request.";
    return true;
}

void CMainApp::FinishKoukuGateCompletePlay(const std::string_view, std::string& status)
{
    status = "The Server owns cinematic completion and the next combat flow.";
}

void CMainApp::CancelKoukuGateCompletePlay(const std::string& status)
{
    using namespace LostArk::Shared;
    const std::string reason = status;
    if (m_KoukuRaidResourcePreparation)
    {
        m_KoukuRaidResourcePreparation.reset();
        if (auto* level = CLevel_KakulSaydonArena::Get_Active()) level->Debug_ResetCompletePlayPreparation();
        m_strKoukuCompletePlayStatus = "Complete raid preparation cancelled before Server playback.";
        return;
    }
    auto* arena = CLevel_KakulSaydonArena::Get_Active();
    if (arena && m_iKoukuRaidPendingRequest && m_iKoukuCompletePlayWorldGeneration == CNetworkManager::Get().Get_WorldInboundGeneration())
    {
        m_bKoukuRaidStopAfterAdmission = true;
        m_strKoukuCompletePlayStatus = "Stop is queued until the Server confirms the pending raid request.";
        return;
    }
    if (arena && m_iKoukuCompletePlayWorldGeneration == CNetworkManager::Get().Get_WorldInboundGeneration())
    {
        const auto& state = arena->Get_KoukuRaidState();
        const bool active = state.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING || state.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC || state.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT ||
            state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE || state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME || state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY;
        if (active && state.iOwnerPlayerId == CNetworkManager::Get().Get_LocalPlayerId() && m_iNextKoukuRaidRequest && arena->Get_PlayerCommandSink())
        {
            C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
            request.eWorldId = WORLD_ID::KAKULSAYDON_ARENA; request.iRequestSequence = m_iNextKoukuRaidRequest++;
            request.eOperation = KOUKUSAYDON_RAID_OPERATION::STOP; request.iExpectedRunEpoch = state.iRunEpoch;
            request.ExpectedGameplayRevision = state.PinnedGameplayRevision; request.iActionSourceRevision = state.iActionSourceRevision;
            request.iSequenceSourceRevision = state.iSequenceSourceRevision; request.strStartGateId = state.strGateId;
            if (arena->Get_PlayerCommandSink()->Request_KoukuRaid(request))
            {
                arena->Expect_KoukuRaidReply(request.iRequestSequence);
                m_KoukuRaidRequest = request; m_iKoukuRaidPendingRequest = request.iRequestSequence;
                m_KoukuRaidReplyDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
                m_strKoukuRaidReplyStatus.clear();
                Record_KoukuRaidRequestEvent("kouku.raid.request.sent", request,
                    m_iKoukuCompletePlayWorldGeneration, request.iExpectedRunEpoch, {});
                m_strKoukuCompletePlayStatus = "Waiting for the Server to stop this raid epoch."; return;
            }
        }
        else if (active) { m_strKoukuCompletePlayStatus = "Only the player who started this raid can stop it."; return; }
    }
    if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_IsServerClock()) m_pKoukuPresentationPlayer->Stop_Preview();
    if (arena)
    {
        arena->Debug_SetSequenceCombatPending(false); arena->Debug_ReturnToPlayerCamera();
        std::string restore;
        if (!arena->End_ServerRaidCinematicPresentation(true, restore))
        { m_strKoukuCompletePlayStatus = "Previous gate restore failed: " + restore; return; }
    }
    m_strKoukuRaidPresentationKey.clear(); m_strKoukuRaidFailedKey.clear(); m_pKoukuRaidSequenceDocument.reset(); m_iKoukuRaidDocumentEpoch = 0u;
    if (arena) arena->Expect_KoukuRaidReply(0u);
    m_strKoukuCompletePlayFlowGate.clear(); m_iKoukuRaidPendingRequest = 0u; m_bKoukuRaidStopAfterAdmission = false; m_strKoukuCompletePlayStatus = reason;
    m_KoukuRaidRequest = {}; m_KoukuRaidReplyDeadline = {}; m_strKoukuRaidReplyStatus.clear();
}


void CMainApp::RenderKoukuSaydonCompletePlayControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.KoukuCompletePlay");
	if (!ImGui::CollapsingHeader("KoukuSaydon Complete Play (Server Boss Replay)")) return;
	if (!m_pKoukuSaydonBossTool) m_pKoukuSaydonBossTool = make_unique<CKoukuSaydonBossTool>();
	ImGui::TextWrapped("Complete Play runs the selected Gate and its Saved Pattern Flow. Bingo begins its repeating flow after Server preparation.");
	if (ImGui::SmallButton(m_bKoukuCompletePlayLoadAttempted ? "Reload KoukuSaydon Inventory" : "Load KoukuSaydon Inventory"))
	{ (void)m_pKoukuSaydonBossTool->Reload(m_strKoukuCompletePlayStatus); m_bKoukuCompletePlayLoadAttempted = true; }
	ImGui::SameLine();
	ImGui::BeginDisabled(m_pKoukuSaydonActionWorkbench &&
		(m_pKoukuSaydonActionWorkbench->Is_Dirty() || m_pKoukuSaydonActionWorkbench->Is_PublishRunning()));
	if (ImGui::SmallButton("Publish Saved Patterns##KoukuCompletePlay"))
		(void)m_pKoukuSaydonBossTool->Request_PublishSavedPatterns(m_strKoukuCompletePlayStatus);
	ImGui::EndDisabled();
	if (!m_bKoukuCompletePlayLoadAttempted) { ImGui::TextDisabled("Load the published Boss Patterns tree."); return; }
	static const char* labels[] = { "\x31\xEA\xB4\x80\xEB\xAC\xB8", "\x32\xEA\xB4\x80\xEB\xAC\xB8", "\x33\xEA\xB4\x80\xEB\xAC\xB8", "\xEB\xB9\x99\xEA\xB3\xA0" };
	static const char* gates[] = { "GATE1", "GATE2", "GATE3", "BINGO" };
	m_iKoukuCompletePlayGate = (std::clamp)(m_iKoukuCompletePlayGate, 0, 3);
	if (ImGui::Combo("Gate##KoukuCompletePlayGate", &m_iKoukuCompletePlayGate, labels, 4))
	{ m_iKoukuCompletePlaySelection = 0; m_strKoukuCompletePlayPatternId.clear(); }
	const std::string gate = gates[m_iKoukuCompletePlayGate];
	if (ImGui::Combo("Category##KoukuCompletePlay", &m_iKoukuCompletePlayCategory, "Saved Pattern Flow\0All Patterns\0"))
	{ m_iKoukuCompletePlaySelection = 0; m_strKoukuCompletePlayPatternId.clear(); }
	const auto& patterns = m_pKoukuSaydonBossTool->Get_ProductPatterns();
	const auto& bundles = m_pKoukuSaydonBossTool->Get_ProductBundles();
	const auto findPattern = [&](const std::string& id) -> const CKoukuSaydonBossTool::PRODUCT_PATTERN* {
		auto it=std::find_if(patterns.begin(),patterns.end(),[&](const auto& p){return p.strPatternId==id;}); return it==patterns.end()?nullptr:&*it; };
	if (ImGui::BeginChild("KoukuCompletePlayInventory", ImVec2(0,240), true))
		(void)(m_iKoukuCompletePlayCategory == 0 ?
			m_pKoukuSaydonBossTool->Render_SavedPatternFlow(gate, m_iKoukuCompletePlaySelection, m_strKoukuCompletePlayPatternId) :
			m_pKoukuSaydonBossTool->Render_PatternTree(gate, m_iKoukuCompletePlaySelection, m_strKoukuCompletePlayPatternId));
	ImGui::EndChild();
	const auto* pattern=m_iKoukuCompletePlaySelection==3?findPattern(m_strKoukuCompletePlayPatternId):nullptr;
	const auto selectedBundle=std::find_if(bundles.begin(),bundles.end(),[&](const auto& b){return m_iKoukuCompletePlaySelection==2 && b.strBundleId==m_strKoukuCompletePlayPatternId && b.strGateId==gate;});
	const bool bundleSelected=selectedBundle!=bundles.end();
	if (bundleSelected)
	{
		ImGui::Text("Selected bundle: %s",selectedBundle->strDisplayName.c_str());
		for (const auto& member : selectedBundle->Members)
		{ const auto* p=findPattern(member.strPatternId); ImGui::BulletText("%s -> %s | %u ms",member.strTargetBossPlacementId.c_str(),p?p->strDisplayName.c_str():member.strPatternId.c_str(),member.iStartOffsetMs); }
		if (!selectedBundle->strLoadError.empty()) ImGui::TextWrapped("%s",selectedBundle->strLoadError.c_str());
	}
	else if (pattern)
	{
		ImGui::TextWrapped("Selected Pattern: %s | %s",pattern->strDisplayName.c_str(),pattern->strTargetBossPlacementId.c_str());
		if (!pattern->strLoadError.empty()) ImGui::TextWrapped("%s",pattern->strLoadError.c_str());
	}
	else ImGui::TextDisabled("Select a playback bundle or child Pattern. Parent folders are not executable.");
	auto& service=CKoukuSaydonPatternAuditionService::Get(); const auto audition=service.Get_Snapshot();
	const auto flow = service.Get_FlowSnapshot();
	const bool preparing = m_pKoukuSaydonBossTool->Is_PlayPreparationPending();
	const bool arena=ETOUI(LEVEL::KAKULSAYDON_ARENA)==CGameInstance::Get().Get_CurrentLevelID();
	const bool ready=bundleSelected?selectedBundle->strLoadError.empty() && !selectedBundle->Members.empty():pattern && pattern->strGateId==gate && pattern->strLoadError.empty();
	const bool sequencePlaying = m_KoukuRaidResourcePreparation || m_iKoukuRaidPendingRequest || !m_strKoukuCompletePlayFlowGate.empty() ||
		(m_pSequenceActionWorkbench && m_pSequenceActionWorkbench->Is_CompleteSequencePlaying());
	ImGui::BeginDisabled(!arena || !ready || audition.Is_InFlight() || flow.bActive || sequencePlaying || preparing);
	const auto prepareSavedProduct = [&]() {
		if (m_pKoukuSaydonActionWorkbench && m_pKoukuSaydonActionWorkbench->Has_Composition())
		{
			if (m_pKoukuSaydonActionWorkbench->Is_PublishRunning())
			{ m_strKoukuCompletePlayStatus = "Publish All Patterns is still running. Wait for completion before Complete Play."; return false; }
			if (m_pKoukuSaydonActionWorkbench->Is_Dirty())
			{ m_strKoukuCompletePlayStatus = "Save Composition changes and use Publish All Patterns before Complete Play."; return false; }
		}
		if (!m_pKoukuSaydonBossTool->Reload(m_strKoukuCompletePlayStatus)) return false;
		// A clean editor can still hold a revision from before an external publish.
		// Compare the saved source, preserving any open editor's draft and selection.
		CKoukuSaydonCompositionDocument savedComposition;
		if (!savedComposition.Reload(m_strKoukuCompletePlayStatus)) return false;
		const auto savedRevision = savedComposition.Get_LastGood().iRevision;
		const auto publishedRevision = m_pKoukuSaydonBossTool->Get_SourceRevision();
		if (savedRevision != publishedRevision)
		{
			m_strKoukuCompletePlayStatus = "Saved Composition revision " + std::to_string(savedRevision) +
				" differs from published revision " + std::to_string(publishedRevision) +
				". Use Publish Saved Patterns here, then retry Complete Play.";
			return false;
		}
		return true;
	};
	const std::string playLabel=(bundleSelected?"Complete Play - "+std::to_string(selectedBundle->Members.size())+" actors":"Complete Play - Selected Pattern")+"##KoukuServerPattern";
	if (ImGui::Button(playLabel.c_str()))
	{
		// Reload can replace the inventory backing these UI pointers.
		const std::string selectedId = bundleSelected ? selectedBundle->strBundleId : pattern->strPatternId;
		if (prepareSavedProduct())
		{
			if (bundleSelected) (void)m_pKoukuSaydonBossTool->Play_SavedBundleById(selectedId,m_strKoukuCompletePlayStatus);
			else (void)m_pKoukuSaydonBossTool->Play_SavedPatternById(selectedId,m_strKoukuCompletePlayStatus);
		}
	}
	ImGui::EndDisabled(); ImGui::SameLine();
	ImGui::BeginDisabled(!arena || (!sequencePlaying && !flow.bActive && !audition.Can_Stop() && !preparing));
	if (ImGui::Button("Stop Complete Play"))
	{
		if (preparing)
			(void)m_pKoukuSaydonBossTool->Cancel_PlayPreparation(m_strKoukuCompletePlayStatus);
		else if (sequencePlaying)
		{
			CancelKoukuGateCompletePlay("Complete Play stop requested.");
		}
		else (void)service.Stop(m_strKoukuCompletePlayStatus);
	}
	ImGui::SameLine(); ImGui::BeginDisabled(audition.strBundleId.empty() || sequencePlaying || flow.bActive || !audition.iRoomAuditionEpoch);
	if (ImGui::Button("Restart Bundle")) (void)service.Restart_Bundle(m_strKoukuCompletePlayStatus);
	ImGui::EndDisabled(); ImGui::EndDisabled();
	ImGui::BeginDisabled(!arena || audition.Is_InFlight() || flow.bActive || sequencePlaying || preparing);
	if (ImGui::Button(gate == "BINGO" ? "Complete Play - Bingo Loop" : "Complete Play - Sequences + Pattern Flow"))
		(void)StartKoukuGateCompletePlay(gate, m_strKoukuCompletePlayStatus);
	ImGui::SameLine();
	if (ImGui::Button("Play Saved Pattern Flow"))
		(void)m_pKoukuSaydonBossTool->Play_PatternFlow(gate, m_strKoukuCompletePlayStatus);
	ImGui::SameLine();
	if (ImGui::Button("Composition Play All") && prepareSavedProduct())
		(void)m_pKoukuSaydonBossTool->Play_CompositionAll(gate, m_strKoukuCompletePlayStatus);
	ImGui::EndDisabled();
	if (preparing) ImGui::TextWrapped("%s", m_pKoukuSaydonBossTool->Get_Status().c_str());
	ImGui::Text("Server: %s",Describe_KoukuSaydonPatternAuditionState(audition.eState));
	if (!flow.strStatus.empty()) ImGui::TextWrapped("%s", flow.strStatus.c_str());
	if (!audition.strBundleId.empty()) ImGui::Text("Bundle %s | run %u | common tick %u",audition.strBundleId.c_str(),audition.iRoomAuditionEpoch,audition.iCommonStartTick);
	for (const auto& member : audition.Members) ImGui::BulletText("%s | boss %u | %s | state %u",member.strMemberId.c_str(),member.iBossNetEntityId,member.strPatternId.c_str(),unsigned(member.eState));
	ImGui::TextWrapped("%s",audition.strStatus.c_str());
	// Submission returns a pending message once; the service owns its later verdict.
	if (!m_strKoukuCompletePlayStatus.starts_with("Waiting for the Server to admit") &&
		m_strKoukuCompletePlayStatus != audition.strStatus)
		ImGui::TextWrapped("%s",m_strKoukuCompletePlayStatus.c_str());
    // Keep an unresolved or rejected command visible even while older raid state
    // broadcasts continue to update the presentation status above.
    if (!m_strKoukuRaidReplyStatus.empty() && m_strKoukuRaidReplyStatus != m_strKoukuCompletePlayStatus &&
        m_strKoukuRaidReplyStatus != audition.strStatus)
        ImGui::TextWrapped("%s", m_strKoukuRaidReplyStatus.c_str());
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
    if (m_strCompletePlayPatternId != *found && m_pValtanBossTool)
        m_pValtanBossTool->Cancel_PlayPreparation("The Complete Play selection changed.");
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
		if (nullptr != m_pAnimationTool)
			m_pAnimationTool->Release_ValtanCompositionPreviewForServerPlayback();
		ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
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
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.ValtanCompletePlay");
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
    const bool_t preparing = m_pValtanBossTool && m_pValtanBossTool->Is_PlayPreparationPending();
	ImGui::BeginDisabled(!canCompletePlay || preparing);
	if (ImGui::Button("Complete Play##GlobalServerPattern"))
	{
		(void)Debug_CompletePlaySelected(m_strCompletePlayStatus);
	}
	ImGui::EndDisabled();
    if (preparing && ImGui::Button("Cancel Complete Play Preparation##GlobalServerPattern"))
        m_pValtanBossTool->Cancel_PlayPreparation("Cancelled by user.");
	if (!canCompletePlay)
	{
		ImGui::TextDisabled(
			"Complete Play requires a loaded Pattern selection and Lobby -> Valtan Server admission.");
	}
	else
	{
		ImGui::TextDisabled(
			"Complete Play prepares every selected dependency before automatically submitting. Product and Sound are checked at request time and once more before Server submission.");
	}
	ImGui::TextWrapped("%s", m_strCompletePlayStatus.c_str());
}

void CMainApp::RenderServerArenaActiveControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.ServerArena");
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

const CWorldSequenceDocument* CMainApp::CompositionPreviewWorldSource() const
{
	if (nullptr == m_pWorldObjectTool)
		return nullptr;
	/* Only the Map Tool view that edits Motion, camera and timeline on one
	   screen consumes the draft. Publishing and product playback are
	   unaffected because they never reach this helper. */
	if (nullptr != m_pMapTool && m_pMapTool->Is_IntegratedCutsceneViewOpen())
	{
		if (const CWorldSequenceDocument* draft =
			m_pWorldObjectTool->Get_AuthoringDraftDocument())
		{
			return draft;
		}
	}
	return m_pWorldObjectTool->Get_SavedDocument();
}

void CMainApp::RefreshWorldObjectResources()
{
	if (!m_pKoukuSaydonActionWorkbench && !m_pSequenceActionWorkbench) return;
	const auto* level = CLevel_KakulSaydonArena::Get_Active();
	const CWorldSequenceDocument* document = m_pWorldObjectTool ? m_pWorldObjectTool->Get_SavedDocument() : nullptr;
	const uint64_t generation = document ? m_pWorldObjectTool->Get_SavedGeneration() : 0;
	if (!document && level) document = &level->Get_WorldSequenceDocument();
	const uint32_t revision = document ? document->Get_Revision() : 0;
	if (m_pWorldObjectCatalogSource == document && m_iWorldObjectCatalogGeneration == generation &&
		m_iWorldObjectCatalogRevision == revision) return;
	m_pWorldObjectCatalogSource = document; m_iWorldObjectCatalogGeneration = generation; m_iWorldObjectCatalogRevision = revision;
	std::vector<KOUKU_WORLD_SEQUENCE_RESOURCE> resources;
	if (document)
	{
		// Keep all states for Logic target selection; the Resource pane shows each parent once.
		for (const auto& instance : document->Get_Instances())
		{
			const auto* sequence = document->Find_Template(instance.templateId);
			if (!sequence) continue;
			KOUKU_WORLD_SEQUENCE_RESOURCE row;
			row.strInstanceId = instance.instanceId; row.strDisplayName = sequence->displayName;
			row.bEnabled = instance.enabled; row.strAnchorKind = instance.anchorKind;
			row.SavedPosition = {instance.position.x, instance.position.y, instance.position.z};
			const WORLD_SEQUENCE_OBJECT_RESOURCE* owner = nullptr;
			if (instance.bindings.size() == 1u &&
				instance.bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				owner = document->Find_ObjectResource(instance.bindings.front().targetId);
			if (!owner)
				for (const auto& object : document->Get_ObjectResources())
					if (object.sequenceInstanceId == instance.instanceId) { owner = &object; break; }
			if (owner)
			{
				row.strObjectResourceId = owner->objectId;
				row.strObjectDisplayName = owner->displayName;
				row.strModelAssetId = owner->modelAssetId;
				row.bDefaultMotion = owner->defaultMotionInstanceId == instance.instanceId;
				row.bUntilDestroyed = owner->combatBody && owner->combatBody->lifetimePolicy == "UNTIL_DESTROYED";
				row.iMaximumHp = owner->combatBody ? owner->combatBody->maxHp : 0u;
				row.bSupportsPlacement = owner->sequenceInstanceId.empty() &&
					instance.anchorKind == owner->anchorKind &&
					(instance.anchorKind == "WORLD" || instance.anchorKind == "BOSS" || instance.anchorKind == "PLAYER");
			}
			for (const auto& animation : sequence->animationTracks)
			{
				row.AnimationClips.push_back(animation.clipName);
				std::uint32_t endMs = sequence->durationMs;
				for (const auto& next : sequence->animationTracks)
					if (next.slotId == animation.slotId && next.startMs > animation.startMs)
						endMs = (std::min)(endMs, next.startMs);
				KOUKU_WORLD_ANIMATION_INFO info;
				info.strClipName = animation.clipName; info.strSlotId = animation.slotId;
				info.fStartMs = instance.startDelayMs + animation.startMs / static_cast<double>(instance.playbackSpeed);
				info.fEndMs = instance.startDelayMs + endMs / static_cast<double>(instance.playbackSpeed);
				info.fPlaybackRate = animation.playbackRate * instance.playbackSpeed;
				info.iSourceStartMs = animation.sourceStartMs;
				info.bLoop = animation.loop; info.bHoldLastFrame = animation.holdLastFrame;
				row.AnimationTracks.push_back(std::move(info));
			}
			row.iEmissionCount = sequence->objectMotion.EmissionCount();
			const double span = instance.startDelayMs + static_cast<double>(sequence->durationMs) / instance.playbackSpeed;
			row.iDurationMs = static_cast<uint32_t>((std::clamp)(span, 1.0, static_cast<double>(UINT32_MAX)));
			float3_t placedPosition;
			if (level && level->Try_GetWorldSequencePlacementBaseline(instance, placedPosition))
			{ row.bHasBoundPlacement = true; row.fBoundX = placedPosition.x; row.fBoundZ = placedPosition.z; }
			resources.push_back(std::move(row));
		}
		for (const auto& object : document->Get_ObjectResources())
		{
			const bool hasDefault = std::any_of(resources.begin(), resources.end(), [&](const auto& row) {
				return row.strObjectResourceId == object.objectId && row.bDefaultMotion;
			});
			if (hasDefault) continue;
			KOUKU_WORLD_SEQUENCE_RESOURCE row;
			row.strObjectResourceId = object.objectId;
			row.strObjectDisplayName = object.displayName; row.strDisplayName = object.displayName;
			row.strAnchorKind = object.anchorKind; row.bEnabled = false;
			if (!object.motionInstanceIds.empty())
			{
				row.strInstanceId = object.objectId;
				row.bMotionGroup = true;
				row.bDefaultMotion = true;
				row.bEnabled = true;
				row.bSupportsPlacement = object.anchorKind == "WORLD";
				row.iEmissionCount = 0u;
				for (const auto& memberId : object.motionInstanceIds)
				{
					const auto member = std::find_if(resources.begin(), resources.end(),
						[&](const auto& value) { return value.strInstanceId == memberId && !value.bMotionGroup; });
					const auto* instance = document->Find_Instance(memberId);
					const auto* sequence = instance ? document->Find_Template(instance->templateId) : nullptr;
					const auto* model = instance && instance->bindings.size() == 1u ?
						document->Find_ObjectResource(instance->bindings.front().targetId) : nullptr;
					if (member == resources.end() || !sequence || !model || model->modelAssetId.empty() ||
						!model->motionInstanceIds.empty() || model->combatBody || instance->walkableSurface ||
						!sequence->colliderTracks.empty() || instance->anchorKind != "WORLD" ||
						instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
						(instance->motionEnd != WORLD_SEQUENCE_MOTION_END::STOP && instance->motionEnd != WORLD_SEQUENCE_MOTION_END::LOOP))
					{ row.bEnabled = false; row.bSupportsPlacement = false; continue; }
					if (!member->bEnabled) continue;
					if (row.iEmissionCount == 0u) row.SavedPosition = member->SavedPosition;
					else if (row.SavedPosition != member->SavedPosition) row.bSupportsPlacement = false;
					row.bSupportsPlacement = row.bSupportsPlacement && member->bSupportsPlacement &&
						member->strAnchorKind == object.anchorKind;
					const double span = instance->startDelayMs +
						static_cast<double>(sequence->PresentationSpanMs()) / instance->playbackSpeed;
					row.iDurationMs = (std::max)(row.iDurationMs, static_cast<uint32_t>(
						(std::clamp)(std::ceil(span), 1.0, static_cast<double>(UINT32_MAX))));
					row.iEmissionCount += member->iEmissionCount;
					row.AnimationClips.insert(row.AnimationClips.end(),
						member->AnimationClips.begin(), member->AnimationClips.end());
				}
				row.bEnabled = row.bEnabled && row.iEmissionCount > 0u;
			}
			row.bUntilDestroyed = object.combatBody && object.combatBody->lifetimePolicy == "UNTIL_DESTROYED";
			row.iMaximumHp = object.combatBody ? object.combatBody->maxHp : 0u;
			resources.push_back(std::move(row));
		}
		// Keep donor motions available to Logic/editing, but Object Append must
		// select their unique complete group instead of a same-named g0 default.
		for (auto& donor : resources)
		{
			if (!donor.bDefaultMotion || donor.bMotionGroup) continue;
			std::string ownerId; bool ambiguous = false;
			for (const auto& group : resources)
			{
				if (!group.bMotionGroup || !group.bEnabled) continue;
				const auto* object = document->Find_ObjectResource(group.strObjectResourceId);
				if (!object || std::find(object->motionInstanceIds.begin(), object->motionInstanceIds.end(), donor.strInstanceId) == object->motionInstanceIds.end()) continue;
				if (!ownerId.empty()) { ambiguous = true; break; }
				ownerId = group.strObjectResourceId;
			}
			if (!ambiguous) donor.strAppendGroupObjectId = std::move(ownerId);
		}
	}
	const std::string status = document ?
		"Objects saved in Object Tool. Select an Object to preview or append; edit its animations in Object Tool." :
		"Enter KoukuSaydon or open Object Tool to load saved Objects.";
	if (m_pKoukuSaydonActionWorkbench) m_pKoukuSaydonActionWorkbench->Set_WorldSequenceResources(resources, status);
	if (m_pSequenceActionWorkbench) m_pSequenceActionWorkbench->Set_WorldSequenceResources(std::move(resources), status);
}

void CMainApp::RenderDeveloperTools()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.Build");
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
		(currentLevelId == ETOUI(LEVEL::KAKULSAYDON_ARENA) ?
			"Kouku runtime supports Map, Object, Camera and Sequence authoring through F1." :
			"F1 opens tools. Map authoring is available in Lobby Test or the Kouku runtime."));
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
		toolCell("Effect Tool V1", DEBUG_TOOL::EFFECT);
		toolCell("Effect Tool V2", DEBUG_TOOL::EFFECT_V2);
		toolCell("World Level Tool", DEBUG_TOOL::WORLD_LEVEL);
		toolCell("Map Tool", DEBUG_TOOL::MAP);
		toolCell("Rendering Workbench", DEBUG_TOOL::RENDERING);
		toolCell("Composition Profiler", DEBUG_TOOL::PROFILER);
		toolCell("HUD Layout Tool", DEBUG_TOOL::UI);
		toolCell("Balance Tool", DEBUG_TOOL::BALANCE);
		toolCell("Equipment Authoring Tool", DEBUG_TOOL::EQUIPMENT);
		ImGui::EndTable();
	}
	if (ImGui::Button("Close All Tools"))
		CloseAllDebugTools();
	constexpr std::array<std::pair<DEBUG_TOOL, const char_t*>, 16>
		TOOL_FOCUS_OPTIONS = {{
			{ DEBUG_TOOL::MAP, "Map Tool" },
			{ DEBUG_TOOL::WORLD_OBJECT, "Action Workbench / Object" },
			{ DEBUG_TOOL::WORLD_LEVEL, "Open World Level Tool" },
			{ DEBUG_TOOL::SEQUENCER, "Action Workbench" },
			{ DEBUG_TOOL::SEQUENCER_BENCHMARK, "Action Workbench / Sequence" },
			{ DEBUG_TOOL::ANIMATION, "Animation Clip Tool" },
			{ DEBUG_TOOL::EFFECT, "Effect Tool V1" },
            { DEBUG_TOOL::EFFECT_V2, "Effect Tool V2" },
			{ DEBUG_TOOL::RENDERING, "Rendering Workbench" },
			{ DEBUG_TOOL::PROFILER, "Composition Profiler" },
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
	if (!isMapEditorWorkspace && currentLevelId != ETOUI(LEVEL::KAKULSAYDON_ARENA) &&
		IsDebugToolVisible(DEBUG_TOOL::MAP))
	{
		ImGui::TextDisabled(
			"Map Tool is open in inspect-only mode. Enter Lobby > Test > Map Editor to save map placement/navigation.");
	}

	RenderDebugLevelNavigation();
	RenderArenaCameraAndPlayerControls();
	RenderKoukuSaydonArenaControls();
	RenderValtanArenaControls();
	RenderCompletePlayControls();
	RenderKoukuSaydonCompletePlayControls();
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

	if (ImGui::CollapsingHeader("Vehicle Riding (Debug)"))
	{
		ImGui::TextDisabled(
			"H mounts the selected vehicle. A class without its rider pose uses the first catalog vehicle.");
		const HUD_PLAYER_STATE& ridingPlayer = CCombatHUDViewModel::Get().Get_Player();
		const std::uint32_t preferredVehicleId = CPlayerController::Get_PreferredVehicleId();
		if (ImGui::RadioButton("First available##VehicleRiding", 0u == preferredVehicleId))
			CPlayerController::Set_PreferredVehicleId(0u);
		for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
		{
			const bool_t hasRider = ridingPlayer.isValid &&
				nullptr != vehicle.Find_Rider(ridingPlayer.eCharacterClass);
			const std::string label = vehicle.archetypeId + "  (" +
				std::to_string(vehicle.vehicleId) + ")" + (hasRider ? "" : "  - no rider pose") +
				"##VehicleRiding" + std::to_string(vehicle.vehicleId);
			if (ImGui::RadioButton(label.c_str(), vehicle.vehicleId == preferredVehicleId))
				CPlayerController::Set_PreferredVehicleId(vehicle.vehicleId);
		}
		ImGui::TextDisabled("Riding now: %u", ridingPlayer.iVehicleId);
	}

	RenderSequenceViewer();

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
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Rendering.Build");
    const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
    if (m_iRenderingLastLevel != currentLevel)
    {
        m_iRenderingLastLevel = currentLevel;
        if (const auto* descriptor = CLevelRegistry::Find(static_cast<LEVEL>(currentLevel)))
        {
            m_eRenderingSelectedLevel = descriptor->eLevel;
            m_strRenderingSelectedProfileId = descriptor->pRenderingProfileId;
            m_bRenderQualityDraftInitialized = false;
        }
    }
    if (const auto* descriptor = CLevelRegistry::Find(m_eRenderingSelectedLevel))
        m_strRenderingQualityProfileId = descriptor->pRenderingProfileId;
    if (m_strRenderingSelectedProfileId.empty())
        m_strRenderingSelectedProfileId = m_RenderingProfiles.Get_ActiveProfileId();
    const auto syncDraft = [this]()
    {
        if (const auto* profile = m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
        {
            m_SceneRenderingDraft = *profile;
            m_RenderQualityDraft = m_RenderingProfiles.Get_ProfileQuality(m_strRenderingQualityProfileId);
            m_strRenderingDraftProfileId = profile->strProfileId;
            m_bRenderQualityDraftInitialized = true;
        }
    };
    if (!m_bRenderQualityDraftInitialized || m_strRenderingDraftProfileId != m_strRenderingSelectedProfileId)
        syncDraft();
    const bool resetLayout = m_bResetLightingLayout;
    m_bResetLightingLayout = false;
    const auto beginPane = [this, resetLayout](const char* title, bool_t& visible, const int pane)
    {
        if (!visible) return false;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 origin = viewport->WorkPos;
        const ImVec2 available = viewport->WorkSize;
        constexpr float margin = 8.f;
        constexpr float gap = 8.f;
        const float width = (std::max)(1.f, available.x - margin * 2.f - gap * 2.f);
        const float height = (std::max)(1.f, available.y - margin * 2.f);
        const float leftWidth = width * .24f;
        const float rightWidth = width * .28f;
        const float centerWidth = width - leftWidth - rightWidth;
        const float rightX = origin.x + margin + leftWidth + gap + centerWidth + gap;
        ImVec2 position(origin.x + margin, origin.y + margin);
        ImVec2 size(leftWidth, height);
        if (pane == 1)
        {
            position = ImVec2(rightX, origin.y + margin);
            size = ImVec2(rightWidth, height * .48f);
        }
        else if (pane == 2)
        {
            position = ImVec2(origin.x + margin + leftWidth + gap, origin.y + margin + height * .66f);
            size = ImVec2(centerWidth, height * .34f);
        }
        else if (pane == 3)
        {
            position = ImVec2(rightX, origin.y + margin + height * .48f + gap);
            size = ImVec2(rightWidth, height * .52f - gap);
        }
        const ImGuiCond condition = resetLayout ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
        ImGui::SetNextWindowPos(position, condition);
        ImGui::SetNextWindowSize(size, condition);
        const bool expanded = ImGui::Begin(title, &visible, ImGuiWindowFlags_MenuBar);
        if (expanded && ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Windows"))
            {
                ImGui::MenuItem("Light Resources", nullptr, &m_bLightResourcesWindowVisible);
                ImGui::MenuItem("Light Detail", nullptr, &m_bLightDetailWindowVisible);
                ImGui::MenuItem("Light Sequencer", nullptr, &m_bLightSequencerWindowVisible);
                ImGui::MenuItem("Rendering Workbench", nullptr, &m_bRenderingQualityWindowVisible);
                if (ImGui::MenuItem("Show All"))
                    m_bLightResourcesWindowVisible = m_bLightDetailWindowVisible =
                        m_bLightSequencerWindowVisible = m_bRenderingQualityWindowVisible = true;
                if (ImGui::MenuItem("Reset Layout"))
                {
                    m_bResetLightingLayout = true;
                    m_bLightResourcesWindowVisible = m_bLightDetailWindowVisible =
                        m_bLightSequencerWindowVisible = m_bRenderingQualityWindowVisible = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        if (!expanded) ImGui::End();
        return expanded;
    };
    if (beginPane("Light Resources###LightResourcesWindowV1", m_bLightResourcesWindowVisible, 0))
    {
        static constexpr LEVEL levels[] = { LEVEL::LOBBY, LEVEL::CHARACTER_SELECT, LEVEL::BERN,
            LEVEL::VALTAN_ARENA, LEVEL::KAKULSAYDON_ARENA, LEVEL::DEVELOPMENT };
        static constexpr const char* names[] = { "Lobby", "Character Select", "Bern", "Valtan", "KoukuSaydon", "Development" };
        int selectedLevel = 0;
        for (int i = 0; i < 6; ++i) if (levels[i] == m_eRenderingSelectedLevel) selectedLevel = i;
        if (ImGui::Combo("Level category", &selectedLevel, names, 6))
        {
            m_eRenderingSelectedLevel = levels[selectedLevel];
            if (const auto* descriptor = CLevelRegistry::Find(m_eRenderingSelectedLevel))
                m_strRenderingSelectedProfileId = m_strRenderingQualityProfileId = descriptor->pRenderingProfileId;
            StopLightingPreview();
            syncDraft();
        }
        RenderLightingWorkbench();
        ImGui::End();
    }
    if (beginPane("Light Detail###LightDetailWindowV1", m_bLightDetailWindowVisible, 1))
    {
        RenderLightDetail();
        ImGui::End();
    }
    if (beginPane("Light Sequencer###LightSequencerWindowV1", m_bLightSequencerWindowVisible, 2))
    {
        RenderLightSequencer();
        ImGui::End();
    }
    if (beginPane("Rendering Workbench###RenderingQualityWindowV1", m_bRenderingQualityWindowVisible, 3))
    {
        const auto* pActiveProfile = m_RenderingProfiles.Get_ActiveProfile();
        if (!pActiveProfile || !m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
        { ImGui::TextWrapped("Selected rendering profile is unavailable."); ImGui::End(); return; }
        ImGui::Text("Selected Level quality: %s", m_strRenderingQualityProfileId.c_str());
	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	ImGui::Text("Pipeline: legacy_deferred_v1");
	ImGui::Text("Scene profile: %s", m_strRenderingDraftProfileId.c_str());
	ImGui::Text("Viewport: %.0f x %.0f", viewportSize.x, viewportSize.y);
	ImGui::TextDisabled(
		"FP16 Light -> SceneHDR -> Screen Post -> half-res Bloom -> Hable/FXAA -> UI");
	ImGui::TextDisabled(
		"Quality edits are saved with the selected Level base; pattern scene changes retain Level quality.");
	if (nullptr != m_pRenderingBenchmark)
	{
		const auto& game = CGameInstance::Get();
		const RENDER_QUALITY_SETTINGS Quality = game.Get_RenderQualitySettings();
		const string strQualitySummary = "profile=" + m_RenderingProfiles.Get_ActiveProfileId() +
			" ssao=" + (Quality.bSSAOEnabled ? "on" : "off") +
			" bloom=" + (Quality.bBloomEnabled ? "on" : "off") +
			" fxaa=" + (Quality.bFXAAEnabled ? "on" : "off") +
			" shadow=" + (game.Get_ShadowLightDesc().Settings.bEnabled ? "on" : "off") +
			" fog=" + (game.Get_HeightFogSettings().bEnabled ? "on" : "off") +
			" exposure=" + std::to_string(Quality.fExposure);
		m_pRenderingBenchmark->Render_Section(
			CGameInstance::Get().Get_Profiler(), strQualitySummary, m_RenderingProfiles);
	}

    ImGui::SeparatorText("Live rendering comparison");
    auto comparison = m_RenderingProfiles.Get_ComparisonOptions();
    if (!comparison.bActive)
    {
        const auto& quality = CGameInstance::Get().Get_RenderQualitySettings();
        comparison.bFXAAEnabled = quality.bFXAAEnabled;
        comparison.bBloomEnabled = quality.bBloomEnabled;
    }
    ImGui::BeginDisabled(m_pRenderingBenchmark && m_pRenderingBenchmark->Is_Capturing());
    bool_t comparisonChanged = ImGui::Checkbox("Directional light##LiveCompare", &comparison.bDirectionalEnabled);
    comparisonChanged |= ImGui::SliderFloat("Exposure multiplier##LiveCompare",
        &comparison.fExposureMultiplier, 0.5f, 2.f, "%.3fx", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Updates the current view while dragging. Ctrl+click to enter an exact multiplier.");
    comparisonChanged |= ImGui::Checkbox("LUT grading##LiveCompare", &comparison.bLutEnabled);
    ImGui::SameLine();
    comparisonChanged |= ImGui::Checkbox("FXAA##LiveCompare", &comparison.bFXAAEnabled);
    ImGui::SameLine();
    comparisonChanged |= ImGui::Checkbox("Bloom##LiveCompare", &comparison.bBloomEnabled);
    if (comparisonChanged)
    {
        comparison.bActive = true;
        (void)m_RenderingProfiles.Set_ComparisonOptions(comparison);
    }
    if (ImGui::Button("Reset comparison##LiveCompare")) m_RenderingProfiles.Clear_ComparisonOptions();
    ImGui::EndDisabled();
    ImGui::Text("Effective exposure: %.3f", CGameInstance::Get().Get_RenderQualitySettings().fExposure);
    ImGui::TextWrapped("Exposure scales brightness; LUT grading runs once. Directional toggles diffuse/specular while ambient stays active.");
    ImGui::TextWrapped("Comparison applies to the current view, including Mario. Reset, close, or change Level to return to authored settings. These switches are not saved.");

	ImGui::SeparatorText("Map Materials");
	MATERIAL_RENDER_SETTINGS materialSettings =
		CGameInstance::Get().Get_MaterialRenderSettings();
	bool_t materialChanged = ImGui::Checkbox(
		"Recovered map materials (B)", &materialSettings.bUseSourceMaterials);
	static constexpr const char* materialViews[] = {
		"Final", "Base color", "Normal", "Direct specular", "Reflection delta",
		"Roughness", "Metallic", "Material AO"
	};
	int materialView = static_cast<int>(materialSettings.eDebugView);
	if (ImGui::Combo("Material debug view", &materialView,
		materialViews, static_cast<int>(std::size(materialViews))))
	{
		materialSettings.eDebugView = static_cast<MATERIAL_DEBUG_VIEW>(materialView);
		materialChanged = true;
	}
	if (materialChanged && FAILED(
		CGameInstance::Get().Apply_MaterialRenderSettings(materialSettings)))
	{
		m_strRenderingStatus = "Could not apply material comparison settings.";
	}
	ImGui::TextWrapped(
		"Only materials declared in mapmaterials; restart after data edits. "
		"Reflection view shows absolute base-color change.");
	const auto surfaceBindings = CMapAssetRenderUtils::Get_RecentSurfaceBindings();
	ImGui::TextDisabled("Bindings collected while this pane is open (last second, up to 32).");
	if (surfaceBindings.empty())
		ImGui::TextDisabled("No declared map material was recently bound.");
	else if (ImGui::BeginTable("RecentFloorMaterialBindings", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("Asset");
		ImGui::TableSetupColumn("Material");
		ImGui::TableSetupColumn("Source family");
		ImGui::TableSetupColumn("Active program");
		ImGui::TableHeadersRow();
		for (const auto& row : surfaceBindings)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextWrapped("%s", row.assetId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("%s", row.materialName.c_str());
			ImGui::TableSetColumnIndex(2);
			switch (row.family)
			{
			case MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION:
				ImGui::TextWrapped("Specular texture + reflection");
				break;
			case MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION:
				ImGui::TextWrapped("Diffuse specular + reflection");
				break;
            case MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE:
                ImGui::TextWrapped("Source seamless PBR");
                break;
            case MODEL_SURFACE_FAMILY::PBR_OPAQUE:
                ImGui::TextWrapped("Source PBR");
                break;
            case MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE:
                ImGui::TextWrapped("Source opaque specular");
                break;
            case MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE:
                ImGui::TextWrapped("Source stone overlay + baked lighting");
                break;
			default:
				ImGui::Text("Unknown (%u)", static_cast<uint32_t>(row.family));
				break;
			}
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%s (%u)", row.activeProgram == 0u ? "Legacy A" : "Recovered B", row.activeProgram);
		}
		ImGui::EndTable();
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
		if (const auto* source = m_RenderingProfiles.Find_Profile(m_strRenderingQualityProfileId))
		{
			auto profile = *source;
			profile.bHasQualityOverride = true;
			profile.QualityOverride = m_RenderQualityDraft;
			m_RenderingProfiles.Update_Profile(profile, m_strRenderingStatus);
		}
		m_RenderQualityDraft = m_RenderingProfiles.Get_ProfileQuality(m_strRenderingQualityProfileId);
		if (const auto* profile = m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId)) m_SceneRenderingDraft = *profile;
	};

	bool_t globalChanged = false;
	ImGui::SeparatorText("Selected Level Quality");
	ImGui::Text("Quality owner: %s", m_strRenderingQualityProfileId.c_str());
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
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Scene bloom strength. Full Restore effects use Skill Bloom Intensity in Effect Detail.");
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

	ImGui::SeparatorText("Selected Quality A/B Actions");
	if (ImGui::Button("Reset Selected Quality Defaults"))
	{
		m_RenderQualityDraft = {};
		m_RenderQualityDraft.bSSAOEnabled = false;
		applyGlobal();
	}
	ImGui::SameLine();
	if (ImGui::Button("Selected Reference A/B Start"))
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
	if (ImGui::Button("Reload Selected Draft"))
	{
		m_RenderQualityDraft = m_RenderingProfiles.Get_ProfileQuality(m_strRenderingQualityProfileId);
		if (const SCENE_RENDERING_PROFILE* pProfile =
			m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
		{
			m_SceneRenderingDraft = *pProfile;
			m_strRenderingDraftProfileId = pProfile->strProfileId;
		}
		m_strRenderingStatus = "Selected profile draft reloaded.";
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
			if (m_pRenderingBenchmark)
				m_pRenderingBenchmark->Notify_ProfileReload();
			m_RenderQualityDraft = m_RenderingProfiles.Get_ProfileQuality(m_strRenderingQualityProfileId);
			if (const SCENE_RENDERING_PROFILE* pProfile =
				m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
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
}

void CMainApp::RenderProfilerOverlay()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.ProfilerOverlay");
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
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.ProfilerDetails");
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
	ImGui::BeginDisabled(m_pProfilerTool && m_pProfilerTool->Is_Saving());
	if (ImGui::Button("Save profiler JSON"))
	{
		if (nullptr == pProfiler)
		{
			m_strProfilerCaptureStatus = "Profiler is not available.";
		}
		else
		{
			// Both profiler entry points share the same bounded asynchronous exporter.
			if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::PROFILER)) && m_pProfilerTool)
			{
				m_pProfilerTool->Request_Save(*pProfiler);
				m_strProfilerCaptureStatus.clear();
			}
			else m_strProfilerCaptureStatus = "Composition Profiler could not be opened.";
		}
	}
	ImGui::EndDisabled();
	if (m_pProfilerTool && !m_pProfilerTool->Get_CaptureStatus().empty())
		ImGui::TextWrapped("%s", m_pProfilerTool->Get_CaptureStatus().c_str());
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
#ifdef _DEBUG
	if (m_pEffectTool) m_pEffectTool->Deactivate_AuthoringWorkspace();
    if (m_pEffectToolV2) m_pEffectToolV2->Deactivate();
	if (m_pWorldObjectTool) m_pWorldObjectTool->Deactivate();
	m_pWorldLevelPendingMapRequest.reset();
	m_pMapEffectPlacementRequest.reset();
	m_pWorldLevelTool.reset();
	m_pWorldObjectTool.reset();
#endif
#ifdef _DEBUG
    if (m_pEffectTool) m_pEffectTool->Set_AuthoringPlayer(nullptr);
    if (m_pEffectToolV2) m_pEffectToolV2->Set_AuthoringPlayer(nullptr);
#endif
	if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Reset();
	m_pKoukuPresentationPlayer.reset();
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
	m_pSequenceActionWorkbench.reset();
	m_pProfilerTool.reset();
	m_pRenderingBenchmark.reset();
	m_pKoukuSaydonActionWorkbench.reset();
	m_pValtanActionWorkbench.reset();
	m_pAnimationTool.reset();
	m_pCharacterActionWorkbench.reset();
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
	CEffectV2Runtime::Release_Resources();
	CEffectCatalog::Clear();
}
