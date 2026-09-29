#include "CharacterSelectWindowView.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "ActorCatalog.h"
#include "Character.h"
#include "CustomizingView.h"
#include "CharacterCatalog.h"
#include "CharacterPortraitRenderer.h"
#include "CharacterRoster.h"
#include "GameInstance.h"
#include "ImGuiLayer.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "Network/PacketMessages.h"
#include "PlayableCharacterAssetService.h"
#include "PlayerSkillCatalog.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>

namespace
{
	/* Every slot in CharacterSelectWindow_Layout.json that carries real art, in document
	   order. The *TextBox / *LabelBox markers (authored tint alpha 0) stay out -- RenderText
	   only reads their rects. */
	constexpr const char* MAIN_ART_SLOTS[] =
	{
		"CharSel_Wallpaper",
		"CharSel_FooterBg",
		"CharSel_CardSlot_0",
		"CharSel_CardSlot_1",
		"CharSel_CardSlot_2",
		"CharSel_CardSlot_3",
		"CharSel_CardSlot_4",
		"CharSel_CardSlot_5",
		"CharSel_StartButton",
		"CharSel_ServerBackIcon",
		"CharSel_RenameIcon",
		"CharSel_OptionIcon",
	};
	constexpr const char* CARD_ICON_SLOTS[] =
	{
		"CharSel_CardIcon_0", "CharSel_CardIcon_1", "CharSel_CardIcon_2",
		"CharSel_CardIcon_3", "CharSel_CardIcon_4", "CharSel_CardIcon_5",
	};
	constexpr const char* RENAME_DIALOG_SLOTS[] =
	{
		"CharSel_RenamePanel", "CharSel_RenameTextBox", "CharSel_RenameConfirm", "CharSel_RenameCancel",
	};

	constexpr int32_t CARD_COUNT = 6;
	constexpr f32_t STAGE_SCALE = 2.f / 3.f;

	/* The seated characters, one portrait slot per card (CharSel_StagePortrait_N sit between the
	   wallpaper and the footer, so the footer gradient fades their feet into the card bar). */
	constexpr const char* STAGE_PORTRAIT_SLOTS[] =
	{
		"CharSel_StagePortrait_0", "CharSel_StagePortrait_1",
		"CharSel_StagePortrait_2", "CharSel_StagePortrait_3",
		"CharSel_StagePortrait_4", "CharSel_StagePortrait_5",
	};
	const wstring_t STAGE_LAYER_TAG = TEXT("Layer_CharacterSelectStage");
	/* Full-body framing: a level camera at chest height, 4 m out with a 30 degree FOV, covers
	   about 2.15 m of height, so the feet land on the slot's bottom edge. */
	constexpr f32_t STAGE_CAMERA_METRES = 4.f;
	constexpr f32_t STAGE_CAMERA_HEIGHT = 1.f;
	constexpr f32_t STAGE_FOV_DEGREES = 30.f;
	/* Once one card is picked the others step back into the shade. */
	const float4_t STAGE_UNSELECTED_TINT{ 0.55f, 0.55f, 0.55f, 1.f };

	/* Stage events go to Client/Default/CharacterSelectStage.user.log beside the startup log, so a
	   card that never shows its character says why without a debugger attached. */
	void Write_StageLog(const std::string& strLine)
	{
		OutputDebugStringA(("[CharacterSelectWindow] " + strLine + "\n").c_str());
		wchar_t szModule[MAX_PATH] = {};
		const DWORD iLength = GetModuleFileNameW(nullptr, szModule, MAX_PATH);
		if (0u == iLength || iLength >= MAX_PATH)
			return;
		const std::filesystem::path Path = std::filesystem::path(szModule).parent_path()
			.parent_path().parent_path() / L"Default" / L"CharacterSelectStage.user.log";
		std::ofstream Output(Path, std::ios::binary | std::ios::app);
		if (!Output)
			return;
		SYSTEMTIME Time{};
		GetLocalTime(&Time);
		char_t szStamp[32] = {};
		(void)sprintf_s(szStamp, "%02u:%02u:%02u ", Time.wHour, Time.wMinute, Time.wSecond);
		Output << szStamp << strLine << "\n";
	}

	std::string Hr_Text(const HRESULT hResult)
	{
		char_t szText[16] = {};
		(void)sprintf_s(szText, "0x%08lX", static_cast<unsigned long>(hResult));
		return szText;
	}

	/* The stance a freshly admitted character stands in, as Model View resolves it: without it
	   the stance classes keep no idle of their own. */
	LostArk::Shared::PLAYER_STANCE_ID Stage_Stance(const LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		using LostArk::Shared::PLAYER_STANCE_ID;
		switch (eClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		case CHARACTER_CLASS_ID::WARLORD: return PLAYER_STANCE_ID::WARLORD_NORMAL;
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT: return PLAYER_STANCE_ID::GUARDIANKNIGHT_HUMAN;
		default: return PLAYER_STANCE_ID::NONE;
		}
	}

	/* "game start" with no seated card selected: the source's disabled state is a darker plate
	   of the same art. */
	const float4_t START_DISABLED_TINT{ 0.45f, 0.45f, 0.45f, 1.f };
	const float4_t TINT_NORMAL{ 1.f, 1.f, 1.f, 1.f };

	/* Gold class symbol per card class (the retail card's ClassIcon_gold, same family). */
	const char* Class_IconPath(const LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (eClass)
		{
		case CHARACTER_CLASS_ID::WARLORD: return "UI/CharacterInfo/class_warlord.png";
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "UI/CharacterInfo/class_lancemaster.png";
		case CHARACTER_CLASS_ID::ARTIST: return "UI/CharacterInfo/class_artist.png";
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT: return "UI/ClassSelect/GuardianKnight/IdentitySymbol.png";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "UI/CharacterInfo/class_dimensionmaster.png";
		case CHARACTER_CLASS_ID::SLAYER: return "UI/CharacterInfo/class_slayer.png";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "UI/CharacterInfo/class_gunslinger.png";
		default: return "UI/Common/White1x1.png";
		}
	}

	/* Class names as the card's class_txt shows them (EFTable_GameMsg class names). */
	const wchar_t* Class_Name(const LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (eClass)
		{
		case CHARACTER_CLASS_ID::WARLORD: return L"\xC6CC\xB85C\xB4DC";
		case CHARACTER_CLASS_ID::LANCE_MASTER: return L"\xCC3D\xC220\xC0AC";
		case CHARACTER_CLASS_ID::ARTIST: return L"\xB3C4\xD654\xAC00";
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT: return L"\xAC00\xB514\xC5B8\xB098\xC774\xD2B8";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return L"\xCC28\xC6D0\xC220\xC0AC";
		case CHARACTER_CLASS_ID::SLAYER: return L"\xC2AC\xB808\xC774\xC5B4";
		case CHARACTER_CLASS_ID::GUNSLINGER: return L"\xAC74\xC2AC\xB9C1\xC5B4";
		default: return L"";
		}
	}

	std::wstring Utf8_ToWide(const std::string& strUtf8)
	{
		if (strUtf8.empty())
			return {};
		const int iLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), nullptr, 0);
		if (iLength <= 0)
			return {};
		std::wstring strWide(static_cast<size_t>(iLength), L' ');
		if (iLength != MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), strWide.data(), iLength))
			return {};
		return strWide;
	}

	std::string Wide_ToUtf8(const std::wstring& strWide)
	{
		if (strWide.empty())
			return {};
		const int iLength = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
			strWide.data(), static_cast<int>(strWide.size()), nullptr, 0, nullptr, nullptr);
		if (iLength <= 0)
			return {};
		std::string strUtf8(static_cast<size_t>(iLength), ' ');
		if (iLength != WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
			strWide.data(), static_cast<int>(strWide.size()), strUtf8.data(), iLength, nullptr, nullptr))
			return {};
		return strUtf8;
	}

	std::string Card_SlotId(const int32_t iCard)
	{
		char_t szSlot[64] = {};
		(void)sprintf_s(szSlot, "CharSel_CardSlot_%d", iCard);
		return szSlot;
	}
}

CCharacterSelectWindowView::CCharacterSelectWindowView(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	uint32_t iOwnerLevelIndex)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
	, m_pView(std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, iOwnerLevelIndex, TEXT("Layer_UI"),
		L"UI/CharacterSelect/CharacterSelectWindow_Layout.json"))
{
	/* A CUI_Sprite is visible from construction; this window starts closed. */
	Hide_AllSlots();
}

CCharacterSelectWindowView::~CCharacterSelectWindowView() = default;

void CCharacterSelectWindowView::Hide_AllSlots()
{
	if (nullptr == m_pView)
		return;
	for (const char* pSlotId : MAIN_ART_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, false);
	for (const char* pSlotId : CARD_ICON_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, false);
	for (const char* pSlotId : RENAME_DIALOG_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, false);
	for (const char* pSlotId : STAGE_PORTRAIT_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, false);
}

void CCharacterSelectWindowView::Open()
{
	m_isOpen = true;
	m_bStageRequested = true;
	m_hasJustOpened = true;
	m_eIntent = INTENT::NONE;
}

void CCharacterSelectWindowView::Close()
{
	Close_RenameDialog();
	m_isOpen = false;
	m_iHoveredCard = -1;
	Hide_AllSlots();
}

CCharacterSelectWindowView::INTENT CCharacterSelectWindowView::Consume_Intent()
{
	const INTENT eIntent = m_eIntent;
	m_eIntent = INTENT::NONE;
	return eIntent;
}

void CCharacterSelectWindowView::Get_StartCharacter(
	LostArk::Shared::CHARACTER_CLASS_ID& outClass, std::string& outNickname,
	std::string& outAppearanceJson, std::string& outCharacterId) const
{
	const std::vector<CHARACTER_ROSTER_ENTRY>& Roster = CCharacterRoster::Get_Entries();
	if (m_iSelectedCard < 0 || !CCharacterRoster::Is_Occupied(static_cast<size_t>(m_iSelectedCard)))
	{
		outClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
		outNickname.clear();
		outAppearanceJson.clear();
		outCharacterId.clear();
		return;
	}
	outClass = Roster[static_cast<size_t>(m_iSelectedCard)].eCharacterClass;
	outNickname = Roster[static_cast<size_t>(m_iSelectedCard)].strNickname;
	outAppearanceJson = Roster[static_cast<size_t>(m_iSelectedCard)].strAppearanceJson;
	outCharacterId = Roster[static_cast<size_t>(m_iSelectedCard)].strCharacterId;
}

void CCharacterSelectWindowView::Update(f32_t fTimeDelta, const bool_t bInputBlocked)
{
	(void)fTimeDelta;
	if (nullptr == m_pView)
		return;

	/* Preparation starts as soon as the Lobby is up, while the player is still on the server
	   select screen, so the characters are usually standing by the time the window opens. It also
	   keeps going while the window is closed, so going back and returning does not restart a
	   class that was halfway loaded. */
	m_bStageRequested = true;
	Update_Stage();

	if (!m_isOpen)
	{
		Hide_AllSlots();
		m_hasJustOpened = false;
		return;
	}

	/* Modal semantics: the pointer belongs to this window for the whole frame it is
	   open (its wallpaper covers the screen), so the Lobby buttons underneath never
	   see hover or click. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	Router.Claim_Mouse_This_Frame();

	const bool_t wasJustOpened = m_hasJustOpened;
	m_hasJustOpened = false;

	/* ESC returns to the Lobby (the server-select screen), mirroring the source's own exit
	   gesture. The rename dialog reads its own ESC from the typed-character stream. */
	const bool_t isEscapeDown =
		GetForegroundWindow() == g_hWnd &&
		0 != (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
	const bool_t escapePressed = isEscapeDown && !m_wasEscapeDown;
	m_wasEscapeDown = isEscapeDown;
	/* The options window updates first, so the press that closes it arrives here with the
	block already lifted; last frame's block keeps that same press from closing this too. */
	const bool_t bBlockedRecently = bInputBlocked || m_wasInputBlocked;
	m_wasInputBlocked = bInputBlocked;
	if (escapePressed && !bBlockedRecently && !m_isRenameOpen)
	{
		m_eIntent = INTENT::CLOSE;
		return;
	}

	for (const char* pSlotId : MAIN_ART_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, true);

	const bool_t bAcceptClicks = !bInputBlocked && !wasJustOpened && !m_isRenameOpen;
	Update_Cards(bAcceptClicks);
	Update_IconButtons(bAcceptClicks);
	Update_RenameDialog();

	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();

	/* "game start": live once a seated card is selected; dimmed and inert otherwise. */
	{
		const bool_t bEnabled = m_iSelectedCard >= 0;
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		const bool_t isHovered = bEnabled && bAcceptClicks &&
			m_pView->Get_SlotRect("CharSel_StartButton", fX, fY, fW, fH) &&
			Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pView->Set_SlotTintMultiplier("CharSel_StartButton", bEnabled ? TINT_NORMAL : START_DISABLED_TINT);
		m_pView->Set_SlotTexture("CharSel_StartButton", isHovered ?
			"UI/CharacterSelect/charsel_start_btn_hover.png" : "");
		if (isHovered && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_eIntent = INTENT::START_CHARACTER;
		}
	}

	/* Server select (back). */
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (m_pView->Get_SlotRect("CharSel_ServerBackIcon", fX, fY, fW, fH))
		{
			/* The label under the icon clicks too -- one merged hit rect covering both. */
			f32_t fLX = fX, fLY = fY, fLW = fW, fLH = fH;
			f32_t fBX = 0.f, fBY = 0.f, fBW = 0.f, fBH = 0.f;
			if (m_pView->Get_SlotRect("CharSel_ServerBackLabelBox", fBX, fBY, fBW, fBH))
			{
				fLX = (std::min)(fX, fBX);
				fLY = (std::min)(fY, fBY);
				fLW = (std::max)(fX + fW, fBX + fBW) - fLX;
				fLH = (std::max)(fY + fH, fBY + fBH) - fLY;
			}
			const bool_t isHovered = bAcceptClicks &&
				Router.Is_Hovered(fLX, fLY, fLW, fLH, fRefWidth, fRefHeight);
			m_pView->Set_SlotTexture("CharSel_ServerBackIcon", isHovered ?
				"UI/CharacterSelect/charsel_server_back_icon_hover.png" : "");
			if (isHovered && Router.Is_Clicked(fLX, fLY, fLW, fLH, fRefWidth, fRefHeight))
			{
				CMainApp::Play_UIButtonClickSound();
				m_eIntent = INTENT::CLOSE;
			}
		}
	}
}

void CCharacterSelectWindowView::Update_Cards(const bool_t bAcceptClicks)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	const std::vector<CHARACTER_ROSTER_ENTRY>& Roster = CCharacterRoster::Get_Entries();

	/* Seated cards: the renderer's filled card (bgMc frame 2) with its over glow on hover and
	   the selected frame while picked; a click selects. Empty cards: the addSlotMc plate with
	   its over glow, and a click is the original's RequestNewCharacter. */
	m_iHoveredCard = -1;
	for (int32_t i = 0; i < CARD_COUNT; ++i)
	{
		const std::string strSlot = Card_SlotId(i);
		const bool_t bSeated = CCharacterRoster::Is_Occupied(static_cast<size_t>(i));
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(strSlot, fX, fY, fW, fH))
			continue;
		const bool_t isHovered = bAcceptClicks &&
			Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		if (bSeated)
		{
			const bool_t bSelected = i == m_iSelectedCard;
			m_pView->Set_SlotTexture(strSlot, bSelected ?
				"UI/CharacterSelect/charsel_card_filled_selected.png" : (isHovered ?
				"UI/CharacterSelect/charsel_card_filled_hover.png" :
				"UI/CharacterSelect/charsel_card_filled.png"));
			m_pView->Set_SlotVisible(CARD_ICON_SLOTS[i], true);
			m_pView->Set_SlotTexture(CARD_ICON_SLOTS[i],
				Class_IconPath(Roster[static_cast<size_t>(i)].eCharacterClass));
		}
		else
		{
			m_pView->Set_SlotTexture(strSlot, isHovered ?
				"UI/CharacterSelect/charsel_card_new_hover.png" : "");
			m_pView->Set_SlotVisible(CARD_ICON_SLOTS[i], false);
		}
		if (!isHovered)
			continue;
		m_iHoveredCard = i;
		if (Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			if (bSeated)
				m_iSelectedCard = i;
			else
			{
				m_iCreationSlot = i;
				m_eIntent = INTENT::NEW_CHARACTER;
			}
		}
	}
}

void CCharacterSelectWindowView::Update_IconButtons(const bool_t bAcceptClicks)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();

	/* The button is the icon plus its caption underneath, like the retail 110x80 button. */
	const auto HitRect = [this](const char* pIconSlot, const char* pLabelSlot,
		f32_t& fX, f32_t& fY, f32_t& fW, f32_t& fH)
	{
		if (!m_pView->Get_SlotRect(pIconSlot, fX, fY, fW, fH))
			return false;
		f32_t fLX = 0.f, fLY = 0.f, fLW = 0.f, fLH = 0.f;
		if (m_pView->Get_SlotRect(pLabelSlot, fLX, fLY, fLW, fLH))
		{
			const f32_t fRight = (std::max)(fX + fW, fLX + fLW);
			const f32_t fBottom = (std::max)(fY + fH, fLY + fLH);
			fX = (std::min)(fX, fLX);
			fY = (std::min)(fY, fLY);
			fW = fRight - fX;
			fH = fBottom - fY;
		}
		return true;
	};

	/* Rename works on the selected seated card; with none selected it shows its disabled
	   state (the grey icon's tint) and ignores clicks. */
	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	const bool_t bRenameEnabled = m_iSelectedCard >= 0;
	m_bRenameHovered = bRenameEnabled && bAcceptClicks &&
		HitRect("CharSel_RenameIcon", "CharSel_RenameLabelBox", fX, fY, fW, fH) &&
		Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
	m_pView->Set_SlotTexture("CharSel_RenameIcon", m_bRenameHovered ?
		"UI/CharacterSelect/charsel_icon_rename_hover.png" : "");
	m_pView->Set_SlotTintMultiplier("CharSel_RenameIcon",
		bRenameEnabled ? TINT_NORMAL : START_DISABLED_TINT);
	if (m_bRenameHovered && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
	{
		CMainApp::Play_UIButtonClickSound();
		Open_RenameDialog();
	}

	m_bOptionHovered = bAcceptClicks &&
		HitRect("CharSel_OptionIcon", "CharSel_OptionLabelBox", fX, fY, fW, fH) &&
		Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
	m_pView->Set_SlotTexture("CharSel_OptionIcon", m_bOptionHovered ?
		"UI/CharacterSelect/charsel_icon_option_hover.png" : "");
	if (m_bOptionHovered && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
	{
		CMainApp::Play_UIButtonClickSound();
		m_eIntent = INTENT::OPEN_OPTIONS;
	}
}

void CCharacterSelectWindowView::Open_RenameDialog()
{
	const std::vector<CHARACTER_ROSTER_ENTRY>& Roster = CCharacterRoster::Get_Entries();
	if (m_iSelectedCard < 0 || !CCharacterRoster::Is_Occupied(static_cast<size_t>(m_iSelectedCard)))
		return;
	m_strRenameDraft = Utf8_ToWide(Roster[static_cast<size_t>(m_iSelectedCard)].strNickname);
	m_strRenameStatus.clear();
	m_isRenameOpen = true;
	CUIInputRouter::Get().Start_TextInput();
}

void CCharacterSelectWindowView::Close_RenameDialog()
{
	if (m_isRenameOpen)
		CUIInputRouter::Get().Stop_TextInput();
	m_isRenameOpen = false;
	m_strRenameDraft.clear();
	m_strRenameStatus.clear();
}

void CCharacterSelectWindowView::Update_RenameDialog()
{
	for (const char* pSlotId : RENAME_DIALOG_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, m_isRenameOpen);
	if (!m_isRenameOpen)
		return;

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	bool_t bConfirm = false;
	bool_t bCancel = false;
	struct DIALOG_BUTTON { const char* pSlotId; bool_t* pOut; };
	const DIALOG_BUTTON Buttons[] = {
		{ "CharSel_RenameConfirm", &bConfirm }, { "CharSel_RenameCancel", &bCancel } };
	for (const DIALOG_BUTTON& Button : Buttons)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(Button.pSlotId, fX, fY, fW, fH))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pView->Set_SlotTexture(Button.pSlotId, bHovered ?
			"UI/ClassSelect/Common/NormalButtonHover.png" : "");
		if (bHovered && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			*Button.pOut = true;
		}
	}

	/* Same WM_CHAR editing loop as the creation modal: committed Hangul arrives as ordinary
	   characters, the composing syllable is drawn from the IME separately, and Backspace /
	   Enter / Escape ride the same stream. The draft is capped at the nickname's byte limit. */
	for (const wchar_t ch : Router.Take_TypedChars())
	{
		if (L'\r' == ch || L'\n' == ch)
			bConfirm = true;
		else if (L'\x1b' == ch)
			bCancel = true;
		else if (L'\b' == ch)
		{
			if (!m_strRenameDraft.empty())
			{
				const bool_t bPair = m_strRenameDraft.size() >= 2 &&
					m_strRenameDraft.back() >= 0xDC00 && m_strRenameDraft.back() <= 0xDFFF;
				m_strRenameDraft.resize(m_strRenameDraft.size() - (bPair ? 2u : 1u));
			}
		}
		else if (ch >= L' ' && L'\x7f' != ch)
		{
			m_strRenameDraft.push_back(ch);
			if (Wide_ToUtf8(m_strRenameDraft).size() > LostArk::Shared::MAX_NICKNAME_BYTES)
				m_strRenameDraft.pop_back();
		}
	}

	if (bCancel)
	{
		Close_RenameDialog();
		return;
	}
	if (bConfirm)
	{
		std::string strStatus;
		if (CCharacterRoster::Rename(static_cast<size_t>(m_iSelectedCard),
			Wide_ToUtf8(m_strRenameDraft), strStatus))
			Close_RenameDialog();
		else
			m_strRenameStatus = std::move(strStatus);
	}
}

void CCharacterSelectWindowView::Update_Stage()
{
	const uint32_t iLobby = ETOUI(LEVEL::LOBBY);

	/* A preparation in flight: its commit lands in the Lobby and the character stands up. One
	   let go by Release_Stage (index -1) is only drained, never committed. */
	if (nullptr != m_pStageAssets && m_pStageAssets->Is_Preparing())
	{
		HRESULT hResult = S_OK;
		std::string strStatus;
		if (!m_pStageAssets->Poll_AsyncPreparation(m_iStagePreparingIndex >= 0, hResult, strStatus))
			return;
		const int32_t iIndex = m_iStagePreparingIndex;
		m_iStagePreparingIndex = -1;
		if (iIndex < 0)
			return;
		if (FAILED(hResult))
		{
			m_bStageFailed[iIndex] = true;
			Write_StageLog("card " + std::to_string(iIndex) + " preparation failed hr=" +
				Hr_Text(hResult) + " " + strStatus);
			return;
		}
		Write_StageLog("card " + std::to_string(iIndex) + " preparation done hr=" + Hr_Text(hResult));
		Spawn_StageCharacter(iIndex);
		return;
	}
	if (!m_bStageRequested || CLevelTransitionService::Is_Pending())
		return;

	/* The next seated card without a character, one class per step. */
	const std::vector<CHARACTER_ROSTER_ENTRY>& Roster = CCharacterRoster::Get_Entries();
	for (int32_t i = 0; i < STAGE_COUNT && static_cast<size_t>(i) < Roster.size(); ++i)
	{
		if (!CCharacterRoster::Is_Occupied(static_cast<size_t>(i)) || m_bStageFailed[i] || !m_StageCharacters[i].expired())
			continue;

		/* The Lobby loader skips the actor and skill catalogs every other level loads before
		   admitting a class; the preparation reads both. */
		if (!m_bStageCatalogsReady)
		{
			std::string strStatus;
			m_bStageCatalogsReady = CActorCatalog::Initialize() &&
				(!CPlayerSkillCatalog::Get_Skills().empty() || CPlayerSkillCatalog::Load(strStatus));
			if (!m_bStageCatalogsReady)
			{
				for (bool_t& bFailed : m_bStageFailed)
					bFailed = true;
				Write_StageLog("catalogs unavailable: " + CActorCatalog::Get_Status() + " " + strStatus);
				return;
			}
		}

		const LostArk::Shared::CHARACTER_CLASS_ID eClass = Roster[static_cast<size_t>(i)].eCharacterClass;
		if (CPlayableCharacterAssetService::Is_Ready(iLobby, eClass))
		{
			Spawn_StageCharacter(i);
			return;
		}
		if (nullptr == m_pStageAssets)
			m_pStageAssets = std::make_unique<CPlayableCharacterAssetService>();
		const HRESULT hStarted = m_pStageAssets->Begin_AsyncPreparation(
			m_pDevice, m_pContext, iLobby, eClass);
		Write_StageLog("card " + std::to_string(i) + " preparation start hr=" + Hr_Text(hStarted));
		if (S_OK == hStarted)
			m_iStagePreparingIndex = i;
		else if (S_FALSE == hStarted)
			Spawn_StageCharacter(i);
		else
		{
			m_bStageFailed[i] = true;
		}
		return;
	}
}

void CCharacterSelectWindowView::Spawn_StageCharacter(const int32_t iIndex)
{
	const std::vector<CHARACTER_ROSTER_ENTRY>& Roster = CCharacterRoster::Get_Entries();
	const CHARACTER_SPEC* pSpec = static_cast<size_t>(iIndex) < Roster.size() ?
		CCharacterCatalog::Find_Spec(Roster[static_cast<size_t>(iIndex)].eCharacterClass) : nullptr;
	if (nullptr == pSpec)
	{
		m_bStageFailed[iIndex] = true;
		return;
	}

	CCharacter::CHARACTER_DESC Desc{};
	Desc.iPrototypeLevelIndex = ETOUI(LEVEL::LOBBY);
	Desc.pSpec = pSpec;
	Desc.eCharacterClass = Roster[static_cast<size_t>(iIndex)].eCharacterClass;
	Desc.fSpeedPerSec = 6.f;
	Desc.fRotationPerSec = 180.f;
	/* Parked 100 m below the origin: the Lobby has no world camera, so whatever view its world pass
	   uses never reaches them there, while each portrait camera follows its own subject. They stay
	   world-visible because the presentation-hidden flag also skips the portrait's own draw. */
	Desc.vPosition = float3_t(static_cast<f32_t>(iIndex) * 5.f, -100.f, 0.f);
	Desc.strNickName = Roster[static_cast<size_t>(iIndex)].strNickname;
	Desc.isLocallyControlled = false;

	shared_ptr<CGameObject> pObject;
	const HRESULT hCreated = CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::LOBBY),
		TEXT("Prototype_GameObject_Character"), ETOUI(LEVEL::LOBBY), STAGE_LAYER_TAG, &Desc, &pObject);
	if (FAILED(hCreated))
	{
		m_bStageFailed[iIndex] = true;
		Write_StageLog("card " + std::to_string(iIndex) + " character create failed hr=" + Hr_Text(hCreated));
		return;
	}
	const shared_ptr<CCharacter> pCharacter = std::dynamic_pointer_cast<CCharacter>(pObject);
	if (nullptr == pCharacter)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(ETOUI(LEVEL::LOBBY), STAGE_LAYER_TAG, pObject);
		m_bStageFailed[iIndex] = true;
		return;
	}

	pCharacter->Apply_NetworkStance(Stage_Stance(Desc.eCharacterClass));
	pCharacter->Set_Animation(CHARACTER_ANIM::IDLE, true);
	/* The card wears the look its character was made with. A document that does not fit (another
	class, unreadable) leaves the class default. */
	if (!Roster[static_cast<size_t>(iIndex)].strAppearanceJson.empty())
		(void)CCustomizingView::Apply_SavedLook(
			pCharacter, Roster[static_cast<size_t>(iIndex)].strAppearanceJson, m_pDevice, m_pContext);
	m_StageCharacters[iIndex] = pCharacter;
	Write_StageLog("card " + std::to_string(iIndex) + " character standing");
}

void CCharacterSelectWindowView::Render_Portraits()
{
	if (!m_isOpen || nullptr == m_pView)
		return;

	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	for (int32_t i = 0; i < STAGE_COUNT; ++i)
	{
		const char* pSlotId = STAGE_PORTRAIT_SLOTS[i];
		const shared_ptr<CCharacter> pCharacter = m_StageCharacters[i].lock();
		STAGE_RECT& Authored = m_StageSlotRects[i];
		if (!Authored.bValid)
			Authored.bValid = m_pView->Get_SlotRect(pSlotId, Authored.fX, Authored.fY, Authored.fW, Authored.fH);
		if (nullptr == pCharacter || !Authored.bValid)
		{
			m_pView->Set_SlotVisible(pSlotId, false);
			continue;
		}
		const f32_t fX = Authored.fX, fY = Authored.fY, fW = Authored.fW, fH = Authored.fH;

		/* The Guardian Knight's halberd reaches far to the left and would cover the card
		neighbour's character. Its portrait is cropped at its own card's left panel edge (halfway
		across the gap to the previous card); the right side is left as it is. The texture keeps
		its full width so the character does not move: only the drawn rect and UV window shrink. */
		f32_t fDrawX = fX, fDrawW = fW, fCropU = 0.f;
		const std::vector<CHARACTER_ROSTER_ENTRY>& Roster = CCharacterRoster::Get_Entries();
		f32_t fCardX = 0.f, fCardY = 0.f, fCardW = 0.f, fCardH = 0.f;
		f32_t fPrevX = 0.f, fPrevY = 0.f, fPrevW = 0.f, fPrevH = 0.f;
		if (i > 0 && static_cast<size_t>(i) < Roster.size() &&
			LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT == Roster[static_cast<size_t>(i)].eCharacterClass &&
			m_pView->Get_SlotRect(Card_SlotId(i), fCardX, fCardY, fCardW, fCardH) &&
			m_pView->Get_SlotRect(Card_SlotId(i - 1), fPrevX, fPrevY, fPrevW, fPrevH))
		{
			const f32_t fBoundary = fCardX - (fCardX - (fPrevX + fPrevW)) * 0.5f;
			if (fBoundary > fX && fBoundary < fX + fW)
			{
				fCropU = (fBoundary - fX) / fW;
				fDrawX = fBoundary;
				fDrawW = fX + fW - fBoundary;
			}
		}

		/* The target is the slot in real pixels, so nothing is stretched. */
		const uint32_t iWidth = static_cast<uint32_t>(fW * vViewport.x / fRefWidth);
		const uint32_t iHeight = static_cast<uint32_t>(fH * vViewport.y / fRefHeight);
		if (nullptr == m_pStagePortraits[i])
			m_pStagePortraits[i] = std::make_unique<CCharacterPortraitRenderer>(m_pDevice, m_pContext);
		CCharacterPortraitRenderer::CAMERA Camera{};
		Camera.fDistance = STAGE_CAMERA_METRES;
		Camera.fEyeHeight = STAGE_CAMERA_HEIGHT;
		Camera.fLookHeight = STAGE_CAMERA_HEIGHT;
		Camera.fFovDegrees = STAGE_FOV_DEGREES;
		if (S_OK != m_pStagePortraits[i]->Render(pCharacter, iWidth, iHeight, Camera, 0u, 0u))
		{
			m_pView->Set_SlotVisible(pSlotId, false);
			continue;
		}

		m_pView->Set_SlotRect(pSlotId, fDrawX, fY, fDrawW, fH);
		m_pView->Set_SlotUVWindow(pSlotId, fCropU, 0.f, 1.f - fCropU, 1.f);
		m_pView->Set_SlotTextureSRV(pSlotId, m_pStagePortraits[i]->Get_SRV());
		m_pView->Set_SlotTintMultiplier(pSlotId,
			(m_iSelectedCard < 0 || m_iSelectedCard == i) ? TINT_NORMAL : STAGE_UNSELECTED_TINT);
		m_pView->Set_SlotVisible(pSlotId, true);
	}
}

void CCharacterSelectWindowView::Release_Stage()
{
	if (nullptr != m_pStageAssets && m_pStageAssets->Is_Preparing())
	{
		m_pStageAssets->Cancel_AsyncPreparation();
		HRESULT hResult = S_OK;
		std::string strStatus;
		(void)m_pStageAssets->Poll_AsyncPreparation(false, hResult, strStatus);
	}
	m_iStagePreparingIndex = -1;
	m_bStageRequested = false;
	for (int32_t i = 0; i < STAGE_COUNT; ++i)
	{
		m_StageCharacters[i].reset();
		m_bStageFailed[i] = false;
	}
}

void CCharacterSelectWindowView::RenderText()
{
	if (!m_isOpen || nullptr == m_pView)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewportSize.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewportSize.y / m_pView->Get_ResolutionHeight();
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);

	const auto DrawCentered = [&](const wchar_t* pText, f32_t fX, f32_t fY,
		f32_t fW, f32_t fH, f32_t fTextHeightRatio, FXMVECTOR vColor)
	{
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(
			TEXT("Font_YoonGasiIIM"), pText);
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ?
			(fH * fTextHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ?
			(fW * 0.92f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pText,
			float2_t((fX + fW * 0.5f) * fScaleX, (fY + fH * 0.5f) * fScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale * fUiScale);
	};
	/* Retail px on the 1920 stage, through the baked label font so small text stays sharp.
	   fPivotX 0 draws from fX, 0.5 centres on it; fY is the text's top. */
	const auto DrawLabel = [&](const wchar_t* pText, const wstring_t& strFamily,
		const f32_t fRetailPx, const f32_t fX, const f32_t fY, const f32_t fPivotX, FXMVECTOR vColor)
	{
		f32_t fTextScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(
			strFamily, fRetailPx * STAGE_SCALE * fUiScale, fTextScale);
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, pText);
		CGameInstance::Get().Draw_Text(strFont, pText,
			float2_t(std::round(fX * fScaleX - vMeasured.x * fTextScale * fPivotX),
				std::round(fY * fScaleY)),
			vColor, 0.f, float2_t(0.f, 0.f), fTextScale);
	};

	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	const std::vector<CHARACTER_ROSTER_ENTRY>& Roster = CCharacterRoster::Get_Entries();

	for (int32_t i = 0; i < CARD_COUNT; ++i)
	{
		if (!m_pView->Get_SlotRect(Card_SlotId(i), fX, fY, fW, fH))
			continue;
		if (CCharacterRoster::Is_Occupied(static_cast<size_t>(i)))
		{
			/* CharacterSelectListRenderer: class_txt $YG760 12 #969696 at (55,0), name_txt
			   $YoonGasiIIM 16 #ffffff at (55,19), in card stage px. */
			const CHARACTER_ROSTER_ENTRY& Entry = Roster[static_cast<size_t>(i)];
			DrawLabel(Class_Name(Entry.eCharacterClass), TEXT("Font_YG760"), 12.f,
				fX + 55.f * STAGE_SCALE, fY + 3.f * STAGE_SCALE, 0.f,
				XMVectorSet(150.f / 255.f, 150.f / 255.f, 150.f / 255.f, 1.f));
			const std::wstring strName = Utf8_ToWide(Entry.strNickname);
			DrawLabel(strName.c_str(), TEXT("Font_YoonGasiIIM"), 16.f,
				fX + 55.f * STAGE_SCALE, fY + 21.f * STAGE_SCALE, 0.f, Colors::White);
			continue;
		}
		// "new character" on every empty card -- new_txt is plain white ($YoonGasiIIM 18px) in
		// every renderer state; only the plate glows on hover.
		const bool_t isHovered = (i == m_iHoveredCard);
		DrawCentered(L"\xC2E0\xADDC \xCE90\xB9AD\xD130 \xC0DD\xC131",
			fX, fY, fW, fH, 0.30f,
			isHovered ?
				XMVectorSet(1.f, 235.f / 255.f, 170.f / 255.f, 1.f) :
				XMVectorSet(200.f / 255.f, 200.f / 255.f, 205.f / 255.f, 1.f));
	}

	// "game start" -- dimmed with its disabled plate until a seated card is selected.
	if (m_pView->Get_SlotRect("CharSel_StartButton", fX, fY, fW, fH))
	{
		DrawCentered(L"\xAC8C\xC784 \xC2DC\xC791", fX, fY, fW, fH, 0.42f,
			m_iSelectedCard >= 0 ? XMVectorSet(1.f, 1.f, 1.f, 1.f) :
				XMVectorSet(150.f / 255.f, 140.f / 255.f, 120.f / 255.f, 1.f));
	}

	// "server select" under the back arrow.
	if (m_pView->Get_SlotRect("CharSel_ServerBackLabelBox", fX, fY, fW, fH))
	{
		DrawCentered(L"\xC11C\xBC84 \xC120\xD0DD", fX, fY, fW, fH, 0.75f,
			XMVectorSet(220.f / 255.f, 220.f / 255.f, 225.f / 255.f, 1.f));
	}

	// Seated count at the card bar's right edge, like the reference's "5 / 6".
	if (m_pView->Get_SlotRect("CharSel_SlotCountTextBox", fX, fY, fW, fH))
	{
		const std::wstring strCount = std::to_wstring(CCharacterRoster::Get_CharacterCount()) + L" / 6";
		DrawCentered(strCount.c_str(), fX, fY, fW, fH, 0.75f,
			XMVectorSet(190.f / 255.f, 190.f / 255.f, 195.f / 255.f, 1.f));
	}

	/* Icon captions: $YG760 14 centred in the button's 100 px caption box; #7b7b7b while
	   disabled (pcselect.button_change_name, characterselect.fla_preference). */
	if (m_pView->Get_SlotRect("CharSel_RenameLabelBox", fX, fY, fW, fH))
		DrawLabel(L"\xCE90\xB9AD\xD130\xBA85 \xBCC0\xACBD", TEXT("Font_YG760"), 14.f,
			fX + fW * 0.5f, fY, 0.5f, m_iSelectedCard >= 0 ? (m_bRenameHovered ?
				XMVectorSet(1.f, 235.f / 255.f, 170.f / 255.f, 1.f) : XMVectorSet(1.f, 1.f, 1.f, 1.f)) :
				XMVectorSet(123.f / 255.f, 123.f / 255.f, 123.f / 255.f, 1.f));
	if (m_pView->Get_SlotRect("CharSel_OptionLabelBox", fX, fY, fW, fH))
		DrawLabel(L"\xD658\xACBD\xC124\xC815", TEXT("Font_YG760"), 14.f, fX + fW * 0.5f, fY, 0.5f,
			m_bOptionHovered ? XMVectorSet(1.f, 235.f / 255.f, 170.f / 255.f, 1.f) :
				XMVectorSet(1.f, 1.f, 1.f, 1.f));

	if (m_isRenameOpen)
		Render_RenameDialogText(fScaleX, fScaleY, fUiScale);
}

void CCharacterSelectWindowView::Render_RenameDialogText(
	const f32_t fScaleX, const f32_t fScaleY, const f32_t fUiScale)
{
	const auto DrawCenteredAt = [&](const f32_t fCenterX, const f32_t fCenterY,
		const wchar_t* pText, const f32_t fTargetHeight, FXMVECTOR vColor)
	{
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pText);
		const f32_t fScale = vMeasured.y > 0.f ? fTargetHeight / vMeasured.y : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pText,
			float2_t(fCenterX * fScaleX, fCenterY * fScaleY), vColor, 0.f,
			float2_t(0.5f, 0.5f), fScale * fUiScale);
	};

	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (m_pView->Get_SlotRect("CharSel_RenamePanel", fX, fY, fW, fH))
	{
		/* pcselect.character_name_change; the same divider line position as the creation
		   modal (43/131 of the panel art). */
		const f32_t fCenterX = fX + fW * 0.5f;
		const f32_t fLineY = fY + fH * (43.f / 131.f);
		DrawCenteredAt(fCenterX, fY - 20.f, L"\xCE90\xB9AD\xD130\xBA85 \xBCC0\xACBD", 22.f, Colors::White);
		const std::wstring strStatus = m_strRenameStatus.empty() ?
			std::wstring(L"\xD55C\xAE00, \xC601\xBB38, \xC22B\xC790 12\xC790\xAE4C\xC9C0 \xC785\xB825 \xAC00\xB2A5") :
			Utf8_ToWide(m_strRenameStatus);
		DrawCenteredAt(fCenterX, fLineY - 15.f, strStatus.c_str(), 15.f,
			m_strRenameStatus.empty() ? Colors::Gold : Colors::OrangeRed);
	}
	const struct { const char* pSlotId; const wchar_t* pLabel; } Labels[] = {
		{ "CharSel_RenameConfirm", L"\xD655\xC778" }, { "CharSel_RenameCancel", L"\xCDE8\xC18C" } };
	for (const auto& Label : Labels)
		if (m_pView->Get_SlotRect(Label.pSlotId, fX, fY, fW, fH))
			DrawCenteredAt(fX + fW * 0.5f, fY + fH * 0.5f, Label.pLabel, fH * 0.32f, Colors::White);

	if (!m_pView->Get_SlotRect("CharSel_RenameTextBox", fX, fY, fW, fH))
		return;
	constexpr f32_t TEXT_HEIGHT = 18.f;
	const f32_t fCenterScreenY = (fY + fH * 0.5f) * fScaleY;
	const auto DrawLeft = [&](const f32_t fScreenX, const wchar_t* pText, FXMVECTOR vColor) -> f32_t
	{
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YG330"), pText);
		if (vMeasured.y <= 0.f)
			return 0.f;
		const f32_t fScale = (TEXT_HEIGHT / vMeasured.y) * fUiScale;
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), pText,
			float2_t(fScreenX, fCenterScreenY), vColor, 0.f, float2_t(0.f, 0.5f), fScale);
		return vMeasured.x * fScale;
	};
	f32_t fCursorScreenX = (fX + 12.f) * fScaleX;
	if (!m_strRenameDraft.empty())
		fCursorScreenX += DrawLeft(fCursorScreenX, m_strRenameDraft.c_str(), Colors::White);
	const wchar_t* pComposition = Engine::CImGuiLayer::Get_ImeCompositionString();
	if (nullptr != pComposition && L'\0' != pComposition[0])
		fCursorScreenX += DrawLeft(fCursorScreenX, pComposition, Colors::Gold);
	const int64_t iHalfSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count() / 500;
	if (0 == (iHalfSeconds % 2))
		DrawLeft(fCursorScreenX + 1.f, L"|", Colors::White);
}
