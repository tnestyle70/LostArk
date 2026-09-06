#include "CharacterInfoWindowView.h"

#include "Character.h"
#include "CharacterPortraitRenderer.h"
#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "ItemCatalog.h"
#include "MainApp.h"
#include "ProjectDataRoot.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"

#include <cmath>
#include <fstream>

namespace
{
	/* Retail characterinfo.gfx is authored at 1920x1080; the layout document scales it by 2/3
	onto the 1280x720 reference, so every retail px offset below goes through the same factor. */
	constexpr f32_t RETAIL_SCALE = 2.f / 3.f;
	constexpr f32_t WINDOW_WIDTH = 1060.f;
	constexpr f32_t TITLE_BAR_HEIGHT = 36.f;
	constexpr f32_t PORTRAIT_DRAG_DEGREES_PER_PX = 0.6f;
	/* Bottom-left ARKButton (weapon effect / avatar book), enlarged from retail (13,648) 133x30 to
	fill the strip between the ark passive pills and the window bottom (user request: the avatar
	book entry was too easy to miss). Icon and label scale with BOTTOM_LEFT_BUTTON_SCALE. */
	constexpr f32_t BOTTOM_LEFT_BUTTON_SCALE = 1.4f;
	constexpr f32_t BOTTOM_LEFT_BUTTON_X = 13.f;
	constexpr f32_t BOTTOM_LEFT_BUTTON_Y = 637.f;
	constexpr f32_t BOTTOM_LEFT_BUTTON_W = 210.f;
	constexpr f32_t BOTTOM_LEFT_BUTTON_H = 42.f;
	/* The LOA sprite fonts are baked at ~32-40 px; downscaled to the retail 13-16 px labels they
	read small and soft, so every label is drawn a little larger and on whole screen pixels. */
	constexpr f32_t TEXT_BOOST = 1.15f;

	const wstring_t FONT_YOON = TEXT("Font_YoonGasiIIM");
	const wstring_t FONT_YG760 = TEXT("Font_YG760");

	/* Retail strings (UTF-16 escapes keep this file ASCII). */
	const wstring TEXT_TITLE = L"\xCE90\xB9AD\xD130 \xC815\xBCF4";              // character info
	const wstring TEXT_TAB[5] = {
		L"\xC7A5\xBE44",                                                     // equipment
		L"\xAC01\xC778",                                                     // engravings
		L"\xBCF4\xC11D",                                                     // gems
		L"\xCE74\xB4DC",                                                     // cards
		L"\xC544\xD06C \xADF8\xB9AC\xB4DC",                                  // ark grid
	};
	const wstring TEXT_ITEM_LEVEL = L"\xC7A5\xCC29 \xC544\xC774\xD15C \xB808\xBCA8"; // item level
	const wstring TEXT_COMBAT_POWER = L"\xC804\xD22C\xB825";                  // combat power
	const wstring TEXT_BASIC_STATS = L"\xAE30\xBCF8 \xD2B9\xC131";            // basic stats
	const wstring TEXT_COMBAT_STATS = L"\xC804\xD22C \xD2B9\xC131";           // combat stats
	const wstring TEXT_ATTACK = L"\xACF5\xACA9\xB825";                        // attack
	const wstring TEXT_MAX_HP = L"\xCD5C\xB300 \xC0DD\xBA85\xB825";           // max hp
	const wstring TEXT_SPECIAL_GEAR = L"\xD2B9\xC218\xC7A5\xBE44";            // special gear
	const wstring TEXT_WEAPON_EFFECT = L"\xBB34\xAE30 \xC774\xD399\xD2B8";    // weapon effect
	const wstring TEXT_AVATAR_BOOK = L"\xC544\xBC14\xD0C0 \xB3C4\xAC10";      // avatar book
	const wstring TEXT_DETAIL = L"\xC0C1\xC138\xC815\xBCF4";                  // details
	const wstring TEXT_EQUIP = L"\xC7A5\xBE44";                               // equipment
	const wstring TEXT_AVATAR = L"\xC544\xBC14\xD0C0";                        // avatar
	const wstring TEXT_HONOR = L"\xBA85\xC608";                               // honor

	const fvector_t COLOR_WHITE = Colors::White;
	const fvector_t COLOR_MUTED = XMVectorSet(145.f / 255.f, 156.f / 255.f, 166.f / 255.f, 1.f);   // #919ca6
	const fvector_t COLOR_GOLD = XMVectorSet(1.f, 210.f / 255.f, 0.f, 1.f);                       // #ffd200
	const fvector_t COLOR_CREAM = XMVectorSet(1.f, 247.f / 255.f, 226.f / 255.f, 1.f);            // #fff7e2
	const fvector_t COLOR_DIM = XMVectorSet(120.f / 255.f, 120.f / 255.f, 120.f / 255.f, 1.f);    // #787878
	const fvector_t COLOR_TITLE_BLUE = XMVectorSet(74.f / 255.f, 163.f / 255.f, 1.f, 1.f);
	const fvector_t COLOR_LEVEL_GOLD = XMVectorSet(1.f, 204.f / 255.f, 0.f, 1.f);                 // #ffcc00
	const fvector_t COLOR_NAME_YELLOW = XMVectorSet(1.f, 240.f / 255.f, 176.f / 255.f, 1.f);      // #fff0b0

	bool_t ConvertUtf8ToWide(const string& strUtf8, wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return true;
		const int32_t iLength = ::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLength <= 1)
			return false;
		outWide.assign(static_cast<size_t>(iLength - 1), L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, outWide.data(), iLength);
		return true;
	}

	const char* Class_DisplayKey(const LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (eClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST: return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
		case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
		default: return nullptr;
		}
	}

	bool_t Starts_With(const string& strId, const char* pPrefix)
	{
		return 0 == strId.compare(0, strlen(pPrefix), pPrefix);
	}

	/* Layout slot column/index -> equipment slot name in CharacterInfoDisplay.json. */
	constexpr const char* LEFT_SLOT_NAMES[6] = { "helmet", "shoulder", "top", "pants", "gloves", "weapon" };
	constexpr const char* RIGHT_SLOT_NAMES[7] = {
		"necklace", "earring1", "earring2", "ring1", "ring2", "stone", "bracelet" };

	/* ItemCatalog "grade" -> V2gradeIconBG art (unknown/empty = legend, the project's default). */
	const char* Grade_BackgroundPath(const string& strGrade)
	{
		if (strGrade == "relic") return "UI/CharacterInfo/grade_relic.png";
		if (strGrade == "ancient") return "UI/CharacterInfo/grade_ancient.png";
		if (strGrade == "epic") return "UI/CharacterInfo/grade_epic.png";
		if (strGrade == "avatar") return "UI/CharacterInfo/grade_avatar.png";
		return "UI/CharacterInfo/grade_legend.png";
	}
}

Client::CCharacterInfoWindowView::CCharacterInfoWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/CharacterInfo/CharacterInfo_Layout.json") }
	, m_pPortrait{ std::make_unique<CCharacterPortraitRenderer>(pDevice, pContext) }
{
	m_SlotIds = m_pView->Get_SlotIds();
	Load_DisplayData();
	m_fPortraitYawDegrees = m_Display.fPortraitYawDegrees;
	/* Same reason as CInventoryView: LEVEL::STATIC sprites are visible from construction. */
	Hide();
}

Client::CCharacterInfoWindowView::~CCharacterInfoWindowView()
{
}

void Client::CCharacterInfoWindowView::Load_DisplayData()
{
	m_Display = {};
	const filesystem::path DataPath =
		CProjectDataRoot::Resolve(L"UI/CharacterInfo/CharacterInfoDisplay.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
	{
		OutputDebugStringA("[CharacterInfo] CharacterInfoDisplay.json missing -- labels stay empty.\n");
		return;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		OutputDebugStringA(("[CharacterInfo] CharacterInfoDisplay.json parse failed: " + Error + "\n").c_str());
		return;
	}

	const auto ReadText = [](const DATA_JSON_VALUE& Object, const char* pKey, wstring& outText)
	{
		const DATA_JSON_VALUE* pText = Object.Find(pKey);
		if (nullptr != pText && pText->Is_String())
			(void)ConvertUtf8ToWide(pText->Get_String(), outText);
	};
	const auto ReadString = [](const DATA_JSON_VALUE& Object, const char* pKey, string& outText)
	{
		const DATA_JSON_VALUE* pText = Object.Find(pKey);
		if (nullptr != pText && pText->Is_String())
			outText = pText->Get_String();
	};
	const auto ReadNumber = [](const DATA_JSON_VALUE& Object, const char* pKey, f32_t& outValue)
	{
		const DATA_JSON_VALUE* pNumber = Object.Find(pKey);
		if (nullptr != pNumber && pNumber->Is_Number())
			outValue = static_cast<f32_t>(pNumber->Get_Number());
	};

	ReadText(Root, "itemLevel", m_Display.strItemLevel);
	ReadText(Root, "combatPower", m_Display.strCombatPower);
	ReadText(Root, "characterLevel", m_Display.strCharacterLevel);
	ReadText(Root, "expeditionLevel", m_Display.strExpeditionLevel);
	ReadText(Root, "title", m_Display.strTitle);
	ReadText(Root, "guild", m_Display.strGuild);
	ReadText(Root, "estate", m_Display.strEstate);
	ReadText(Root, "presetName", m_Display.strPresetName);
	ReadText(Root, "honor", m_Display.strHonor);

	if (const DATA_JSON_VALUE* pStats = Root.Find("combatStats"); nullptr != pStats && pStats->Is_Array())
	{
		for (const DATA_JSON_VALUE& Value : pStats->Get_Array())
		{
			if (!Value.Is_Object())
				continue;
			STAT_ROW Row{};
			ReadText(Value, "name", Row.strName);
			ReadText(Value, "value", Row.strValue);
			m_Display.CombatStats.push_back(std::move(Row));
		}
	}
	if (const DATA_JSON_VALUE* pPoints = Root.Find("arkPassivePoints"); nullptr != pPoints && pPoints->Is_Array())
	{
		for (const DATA_JSON_VALUE& Value : pPoints->Get_Array())
		{
			wstring strPoint;
			if (Value.Is_String())
				(void)ConvertUtf8ToWide(Value.Get_String(), strPoint);
			m_Display.ArkPassivePoints.push_back(std::move(strPoint));
		}
	}
	if (const DATA_JSON_VALUE* pPortrait = Root.Find("portrait"); nullptr != pPortrait && pPortrait->Is_Object())
	{
		ReadNumber(*pPortrait, "distance", m_Display.fPortraitDistance);
		ReadNumber(*pPortrait, "eyeHeight", m_Display.fPortraitEyeHeight);
		ReadNumber(*pPortrait, "lookHeight", m_Display.fPortraitLookHeight);
		ReadNumber(*pPortrait, "fovDegrees", m_Display.fPortraitFovDegrees);
		ReadNumber(*pPortrait, "yawDegrees", m_Display.fPortraitYawDegrees);
	}
	if (const DATA_JSON_VALUE* pSlots = Root.Find("avatarSlotIndices"); nullptr != pSlots && pSlots->Is_Object())
	{
		f32_t fHead = -1.f, fOutfit = -1.f;
		ReadNumber(*pSlots, "head", fHead);
		ReadNumber(*pSlots, "outfit", fOutfit);
		m_Display.iAvatarSlotHead = static_cast<int32_t>(fHead);
		m_Display.iAvatarSlotOutfit = static_cast<int32_t>(fOutfit);
	}
	if (const DATA_JSON_VALUE* pClasses = Root.Find("classes"); nullptr != pClasses && pClasses->Is_Object())
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		constexpr CHARACTER_CLASS_ID CLASSES[] = {
			CHARACTER_CLASS_ID::LANCE_MASTER, CHARACTER_CLASS_ID::GUNSLINGER,
			CHARACTER_CLASS_ID::SLAYER, CHARACTER_CLASS_ID::ARTIST,
			CHARACTER_CLASS_ID::DIMENSIONMASTER, CHARACTER_CLASS_ID::WARLORD };
		for (const CHARACTER_CLASS_ID eClass : CLASSES)
		{
			const DATA_JSON_VALUE* pClass = pClasses->Find(Class_DisplayKey(eClass));
			if (nullptr == pClass || !pClass->Is_Object())
				continue;
			CLASS_DISPLAY Display{};
			ReadText(*pClass, "displayName", Display.strDisplayName);
			ReadString(*pClass, "symbol", Display.strSymbolPath);
			if (const DATA_JSON_VALUE* pIds = pClass->Find("equippedItemIds"); nullptr != pIds && pIds->Is_Object())
			{
				for (const char* pSlot : { "weapon", "helmet", "shoulder", "top", "pants", "gloves",
					"necklace", "earring1", "earring2", "ring1", "ring2", "stone", "bracelet" })
				{
					string strItemId;
					ReadString(*pIds, pSlot, strItemId);
					Display.EquippedItemIds[pSlot] = strItemId;
				}
			}
			if (const DATA_JSON_VALUE* pAvatarIds = pClass->Find("equippedAvatarItemIds");
				nullptr != pAvatarIds && pAvatarIds->Is_Object())
			{
				for (const char* pSlot : { "head", "outfit" })
				{
					string strItemId;
					ReadString(*pAvatarIds, pSlot, strItemId);
					Display.EquippedAvatarItemIds[pSlot] = strItemId;
				}
			}
			m_Display.Classes[eClass] = std::move(Display);
		}
	}
	m_Display.isLoaded = true;
}

void Client::CCharacterInfoWindowView::Hide()
{
	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, false);
	m_bPortraitValid = false;
	m_bDraggingPanel = false;
	m_bDraggingPortrait = false;
	m_iPreviewHiddenAvatarKinds = 0u;
}

bool_t Client::CCharacterInfoWindowView::Get_WindowOrigin(f32_t& fX, f32_t& fY) const
{
	f32_t fWidth = 0.f, fHeight = 0.f;
	return m_pView->Get_SlotRect("CI_WinBg", fX, fY, fWidth, fHeight);
}

f32_t Client::CCharacterInfoWindowView::Ref_X(const f32_t fRetailX) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fX + fRetailX * RETAIL_SCALE;
}

f32_t Client::CCharacterInfoWindowView::Ref_Y(const f32_t fRetailY) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fY + fRetailY * RETAIL_SCALE;
}

void Client::CCharacterInfoWindowView::Set_HoverArt(CUILayoutRuntime& View, const string& strId,
	const bool_t bHovered, const char* pNormalPath, const char* pOverPath)
{
	View.Set_SlotTexture(strId, bHovered ? pOverPath : pNormalPath);
}

void Client::CCharacterInfoWindowView::Update(const f32_t fTimeDelta,
	const std::shared_ptr<CCharacter>& pLocalCharacter, const HUD_PLAYER_STATE& Player)
{
	(void)fTimeDelta;
	if (!m_bOpen)
	{
		Hide();
		m_pPortraitCharacter.reset();
		return;
	}

	m_pPortraitCharacter = pLocalCharacter;
	m_ePortraitClass = Player.eCharacterClass;
	m_iMaximumHp = Player.iMaximumHp;
	m_bHasAttackPower = CCombatHUDViewModel::Get().Try_Get_ProfileAttackPower(
		Player.eCharacterClass, m_iAttackPower);
	if (nullptr != pLocalCharacter)
		(void)ConvertUtf8ToWide(pLocalCharacter->Get_NickName(), m_strNickName);
	else
		m_strNickName.clear();

	Update_Visibility();
	Update_PanelDrag();
	Update_Chrome();
	Update_Tabs();
	if (0 == m_iSelectedTab)
	{
		Update_EquipToggle();
		Update_Buttons();
		Update_EquipmentIcons(Player.eCharacterClass);
		Update_AvatarSlots(pLocalCharacter, Player.eCharacterClass);
		Update_PortraitDrag();
	}

	/* Anything over the window belongs to the window -- keeps a click on the panel from
	turning into a gameplay move/skill command underneath. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_pView->Get_SlotRect("CI_WinBg", fX, fY, fWidth, fHeight))
	{
		if (Router.Is_Hovered(fX, fY, fWidth, fHeight,
			m_pView->Get_ResolutionWidth(), m_pView->Get_ResolutionHeight()))
		{
			Router.Claim_Mouse_This_Frame();
		}
		/* This window's sprites are the topmost runtime UI unless the avatar book covers it; the
		other windows' text passes skip whatever lands under this rect. */
		const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
		if (!m_bCovered && vViewport.x > 0.f && vViewport.y > 0.f)
		{
			const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
			const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
			Router.Set_TopWindowRect(fX * fScaleX, fY * fScaleY, fWidth * fScaleX, fHeight * fScaleY);
		}
	}
}

void Client::CCharacterInfoWindowView::Update_Visibility()
{
	const bool_t bEquipTab = 0 == m_iSelectedTab;
	for (const string& strId : m_SlotIds)
	{
		bool_t bVisible = bEquipTab;
		if (Starts_With(strId, "CI_WinBg") || Starts_With(strId, "CI_PanelBg") ||
			Starts_With(strId, "CI_TopDeco") || Starts_With(strId, "CI_Close") ||
			Starts_With(strId, "CI_Tab_"))
		{
			bVisible = true;
		}
		else if (Starts_With(strId, "CI_Avatar"))
		{
			bVisible = bEquipTab && m_bAvatarMode;
		}
		else if (Starts_With(strId, "CI_Slot") || Starts_With(strId, "CI_HelmCheck") ||
			Starts_With(strId, "CI_EnhanceCheck") || Starts_With(strId, "CI_AccCheck") ||
			Starts_With(strId, "CI_AccFind") || Starts_With(strId, "CI_CombatTitleBg") ||
			Starts_With(strId, "CI_CombatHelp"))
		{
			bVisible = bEquipTab && !m_bAvatarMode;
		}
		else if (Starts_With(strId, "CI_Preview"))
		{
			/* Render_Portrait decides this one after it has (or hasn't) drawn. */
			continue;
		}
		m_pView->Set_SlotVisible(strId, bVisible);
	}
}

void Client::CCharacterInfoWindowView::Clamp_ToScreen()
{
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("CI_WinBg", fX, fY, fWidth, fHeight))
		return;
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fDeltaX = 0.f, fDeltaY = 0.f;
	if (fX < 0.f) fDeltaX = -fX;
	else if (fX + fWidth > fRefWidth) fDeltaX = fRefWidth - (fX + fWidth);
	if (fY < 0.f) fDeltaY = -fY;
	else if (fY + fHeight > fRefHeight) fDeltaY = fRefHeight - (fY + fHeight);
	if (0.f == fDeltaX && 0.f == fDeltaY)
		return;
	for (const string& strId : m_SlotIds)
	{
		f32_t fSlotX = 0.f, fSlotY = 0.f, fSlotWidth = 0.f, fSlotHeight = 0.f;
		if (m_pView->Get_SlotRect(strId, fSlotX, fSlotY, fSlotWidth, fSlotHeight))
			m_pView->Set_SlotPosition(strId, fSlotX + fDeltaX, fSlotY + fDeltaY);
	}
}

void Client::CCharacterInfoWindowView::Update_PanelDrag()
{
	Clamp_ToScreen();
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	if (!Router.Get_MousePosition(fRefWidth, fRefHeight, fMouseX, fMouseY))
		return;

	if (!m_bDraggingPanel)
	{
		f32_t fCloseX = 0.f, fCloseY = 0.f, fCloseWidth = 0.f, fCloseHeight = 0.f;
		if (m_pView->Get_SlotRect("CI_Close", fCloseX, fCloseY, fCloseWidth, fCloseHeight) &&
			Router.Is_Hovered(fCloseX, fCloseY, fCloseWidth, fCloseHeight, fRefWidth, fRefHeight))
			return;
		/* The retail title bar: the strip above the tabs, full window width. */
		const f32_t fBarX = Ref_X(0.f);
		const f32_t fBarY = Ref_Y(0.f);
		if (Router.Is_Clicked(fBarX, fBarY, WINDOW_WIDTH * RETAIL_SCALE,
			TITLE_BAR_HEIGHT * RETAIL_SCALE, fRefWidth, fRefHeight))
		{
			m_bDraggingPanel = true;
			m_fLastDragMouseX = fMouseX;
			m_fLastDragMouseY = fMouseY;
		}
		return;
	}

	Router.Claim_Mouse_This_Frame();
	if (!Router.Is_LeftDown())
	{
		m_bDraggingPanel = false;
		return;
	}
	const f32_t fDeltaX = fMouseX - m_fLastDragMouseX;
	const f32_t fDeltaY = fMouseY - m_fLastDragMouseY;
	m_fLastDragMouseX = fMouseX;
	m_fLastDragMouseY = fMouseY;
	if (0.f == fDeltaX && 0.f == fDeltaY)
		return;
	for (const string& strId : m_SlotIds)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pView->Get_SlotRect(strId, fX, fY, fWidth, fHeight))
			m_pView->Set_SlotPosition(strId, fX + fDeltaX, fY + fDeltaY);
	}
}

void Client::CCharacterInfoWindowView::Update_Chrome()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("CI_Close", fX, fY, fWidth, fHeight))
		return;
	const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
	Set_HoverArt(*m_pView, "CI_Close", bHovered,
		"UI/CharacterInfo/win_close_normal.png", "UI/CharacterInfo/win_close_over.png");
	if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
	{
		CMainApp::Play_UIButtonClickSound();
		Close();
	}
}

void Client::CCharacterInfoWindowView::Update_Tabs()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	for (int32_t i = 0; i < 5; ++i)
	{
		const string strId = "CI_Tab_" + std::to_string(i);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect(strId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight) &&
			i != m_iSelectedTab)
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedTab = i;
		}
		const char* pPath = (i == m_iSelectedTab) ? "UI/CharacterInfo/tab_selected.png" :
			(bHovered ? "UI/CharacterInfo/tab_over.png" : "UI/CharacterInfo/tab_normal.png");
		m_pView->Set_SlotTexture(strId, pPath);
	}
}

void Client::CCharacterInfoWindowView::Update_EquipToggle()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	struct TOGGLE { const char* pSlotId; const char* pSymbolId; bool_t bAvatar;
		const char* pSymbolNormal; const char* pSymbolSelected; };
	constexpr TOGGLE TOGGLES[2] = {
		{ "CI_ToggleEquip", "CI_ToggleEquipSym", false,
			"UI/CharacterInfo/toggle_equip_sym.png", "UI/CharacterInfo/toggle_equip_sym_sel.png" },
		{ "CI_ToggleAvatar", "CI_ToggleAvatarSym", true,
			"UI/CharacterInfo/toggle_avatar_sym.png", "UI/CharacterInfo/toggle_avatar_sym_sel.png" },
	};
	for (const TOGGLE& Toggle : TOGGLES)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect(Toggle.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight) &&
			m_bAvatarMode != Toggle.bAvatar)
		{
			CMainApp::Play_UIButtonClickSound();
			m_bAvatarMode = Toggle.bAvatar;
			Update_Visibility();
		}
		const bool_t bSelected = m_bAvatarMode == Toggle.bAvatar;
		m_pView->Set_SlotTexture(Toggle.pSlotId, bSelected ? "UI/CharacterInfo/toggle_selected.png" :
			(bHovered ? "UI/CharacterInfo/toggle_over.png" : "UI/CharacterInfo/toggle_normal.png"));
		m_pView->Set_SlotTexture(Toggle.pSymbolId,
			bSelected ? Toggle.pSymbolSelected : Toggle.pSymbolNormal);
	}
	/* The original swaps the bottom-left ARKButton with the page: weapon effect on the equipment
	page, avatar book on the avatar page (equipAvatarBookButton). */
	m_pView->Set_SlotTexture("CI_BottomLeftIcon", m_bAvatarMode ?
		"UI/CharacterInfo/avatarbook_icon.png" : "UI/CharacterInfo/enhance_sym.png");
	/* The button is drawn at 2x the retail size (user request: the avatar book entry was too
	easy to miss); the icon keeps its aspect and sits vertically centred in it. */
	f32_t fIconX = 0.f, fIconY = 0.f, fIconWidth = 0.f, fIconHeight = 0.f;
	if (m_pView->Get_SlotRect("CI_BottomLeftIcon", fIconX, fIconY, fIconWidth, fIconHeight))
	{
		const f32_t fIconH = (m_bAvatarMode ? 20.f : 26.f) * BOTTOM_LEFT_BUTTON_SCALE;
		const f32_t fIconW = (m_bAvatarMode ? 22.f : 18.f) * BOTTOM_LEFT_BUTTON_SCALE;
		m_pView->Set_SlotRect("CI_BottomLeftIcon", fIconX,
			Ref_Y(BOTTOM_LEFT_BUTTON_Y + (BOTTOM_LEFT_BUTTON_H - fIconH) * 0.5f),
			fIconW * RETAIL_SCALE, fIconH * RETAIL_SCALE);
	}
}

void Client::CCharacterInfoWindowView::Update_Buttons()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	struct BUTTON { const char* pSlotId; const char* pNormal; const char* pOver; };
	constexpr BUTTON BUTTONS[] = {
		{ "CI_BottomLeftBtn", "UI/CharacterInfo/btn_normal.png", "UI/CharacterInfo/btn_over.png" },
		{ "CI_DetailBtn", "UI/CharacterInfo/btn_normal.png", "UI/CharacterInfo/btn_over.png" },
		{ "CI_PresetEditBtn", "UI/CharacterInfo/btn_normal.png", "UI/CharacterInfo/btn_over.png" },
		{ "CI_PresetLoadBtn", "UI/CharacterInfo/btn_normal.png", "UI/CharacterInfo/btn_over.png" },
		{ "CI_HonorBg", "UI/CharacterInfo/honor_bg.png", "UI/CharacterInfo/honor_over.png" },
		{ "CI_PresetTab_0", "UI/CharacterInfo/preset_tab_1.png", "UI/CharacterInfo/preset_tab_1_over.png" },
		{ "CI_ItemLevelInfo", "UI/CharacterInfo/info_btn_normal.png", "UI/CharacterInfo/info_btn_over.png" },
		{ "CI_HonorInfo", "UI/CharacterInfo/info_btn_normal.png", "UI/CharacterInfo/info_btn_over.png" },
		{ "CI_GearHelp", "UI/CharacterInfo/help_btn.png", "UI/CharacterInfo/help_btn_over.png" },
		{ "CI_BasicHelp", "UI/CharacterInfo/help_btn.png", "UI/CharacterInfo/help_btn_over.png" },
		{ "CI_CombatHelp", "UI/CharacterInfo/help_btn.png", "UI/CharacterInfo/help_btn_over.png" },
	};
	for (const BUTTON& Button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		Set_HoverArt(*m_pView, Button.pSlotId, bHovered, Button.pNormal, Button.pOver);
		/* The bottom-left button is the avatar book on the avatar page (equipAvatarBookButton);
		everything else is press feedback only until it has Server data behind it. */
		if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			if (m_bAvatarMode && 0 == strcmp(Button.pSlotId, "CI_BottomLeftBtn"))
				m_bAvatarBookRequested = true;
		}
	}
}

void Client::CCharacterInfoWindowView::Update_EquipmentIcons(
	const LostArk::Shared::CHARACTER_CLASS_ID eClass)
{
	const auto it = m_Display.Classes.find(eClass);
	const CLASS_DISPLAY* pClass = (it != m_Display.Classes.end()) ? &it->second : nullptr;
	if (nullptr != pClass && !pClass->strSymbolPath.empty())
		m_pView->Set_SlotTexture("CI_ClassSym", pClass->strSymbolPath);
	m_pView->Set_SlotVisible("CI_ClassSym", nullptr != pClass && !pClass->strSymbolPath.empty());

	const bool_t bShowSlots = 0 == m_iSelectedTab && !m_bAvatarMode;
	const auto Apply = [&](const string& strSlotSuffix, const char* pSlotName)
	{
		const ITEM_DEFINITION* pItem = nullptr;
		if (nullptr != pClass)
		{
			const auto found = pClass->EquippedItemIds.find(pSlotName);
			if (found != pClass->EquippedItemIds.end() && !found->second.empty())
				pItem = CItemCatalog::Find_ById(found->second);
		}
		const bool_t bHasItem = nullptr != pItem && !pItem->strIconPath.empty();
		if (bHasItem)
		{
			m_pView->Set_SlotTexture("CI_SlotIcon_" + strSlotSuffix, pItem->strIconPath);
			m_pView->Set_SlotTexture("CI_SlotGrade_" + strSlotSuffix, Grade_BackgroundPath(pItem->strGrade));
		}
		m_pView->Set_SlotVisible("CI_SlotIcon_" + strSlotSuffix, bShowSlots && bHasItem);
		m_pView->Set_SlotVisible("CI_SlotGrade_" + strSlotSuffix, bShowSlots && bHasItem);
		m_pView->Set_SlotVisible("CI_SlotSil_" + strSlotSuffix, bShowSlots && !bHasItem);
	};
	for (int32_t i = 0; i < 6; ++i)
		Apply("L" + std::to_string(i), LEFT_SLOT_NAMES[i]);
	for (int32_t j = 0; j < 7; ++j)
		Apply("R" + std::to_string(j), RIGHT_SLOT_NAMES[j]);
}

void Client::CCharacterInfoWindowView::Update_AvatarSlots(
	const std::shared_ptr<CCharacter>& pLocalCharacter,
	const LostArk::Shared::CHARACTER_CLASS_ID eClass)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	const bool_t bShow = 0 == m_iSelectedTab && m_bAvatarMode;
	const auto it = m_Display.Classes.find(eClass);
	const CLASS_DISPLAY* pClass = (it != m_Display.Classes.end()) ? &it->second : nullptr;

	for (int32_t k = 0; k < 8; ++k)
	{
		const string strSuffix = std::to_string(k);
		const string strIconId = "CI_AvatarIcon_" + strSuffix;
		const string strCheckId = "CI_AvatarCheck_" + strSuffix;

		/* Slot -> avatar part kind (CharacterInfoDisplay.json avatarSlotIndices). */
		EQUIPMENT_SLOT_KIND eKind = EQUIPMENT_SLOT_KIND::DEFAULT;
		const char* pItemKey = nullptr;
		if (k == m_Display.iAvatarSlotHead) { eKind = EQUIPMENT_SLOT_KIND::AVATAR_HEAD; pItemKey = "head"; }
		else if (k == m_Display.iAvatarSlotOutfit) { eKind = EQUIPMENT_SLOT_KIND::AVATAR_ARMOR; pItemKey = "outfit"; }
		/* Equipped = the character really wears it now (an avatar taken off in the avatar book
		leaves an empty slot here too); the eye toggle below only previews on top of that. */
		const bool_t bHasPart = nullptr != pLocalCharacter && nullptr != pItemKey &&
			pLocalCharacter->Has_AvatarPart(eKind) && pLocalCharacter->Is_AvatarPartVisible(eKind);

		const ITEM_DEFINITION* pItem = nullptr;
		if (bHasPart && nullptr != pClass)
		{
			const auto found = pClass->EquippedAvatarItemIds.find(pItemKey);
			if (found != pClass->EquippedAvatarItemIds.end() && !found->second.empty())
				pItem = CItemCatalog::Find_ById(found->second);
		}
		const bool_t bHasIcon = nullptr != pItem && !pItem->strIconPath.empty();
		if (bHasIcon)
			m_pView->Set_SlotTexture(strIconId, pItem->strIconPath);
		m_pView->Set_SlotVisible(strIconId, bShow && bHasIcon);

		/* Eye toggle: On = avatar shown, Off = hidden, disabled when this slot has no avatar. */
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect(strCheckId, fX, fY, fWidth, fHeight))
			continue;
		if (!bHasPart)
		{
			m_pView->Set_SlotTexture(strCheckId, "UI/CharacterInfo/avatar_eye_off_dis.png");
			continue;
		}
		const bool_t bHovered = bShow && Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		/* Preview-only: the portrait draws without this part, the world character keeps it. */
		const uint32_t iKindBit = 1u << ETOUI(eKind);
		bool_t bVisible = 0u == (m_iPreviewHiddenAvatarKinds & iKindBit);
		if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			bVisible = !bVisible;
			m_iPreviewHiddenAvatarKinds ^= iKindBit;
		}
		m_pView->Set_SlotTexture(strCheckId, bVisible ?
			(bHovered ? "UI/CharacterInfo/avatar_eye_on_over.png" : "UI/CharacterInfo/avatar_eye_on.png") :
			(bHovered ? "UI/CharacterInfo/avatar_eye_off_over.png" : "UI/CharacterInfo/helm_check_off.png"));
	}
}

void Client::CCharacterInfoWindowView::Update_PortraitDrag()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	if (!Router.Get_MousePosition(fRefWidth, fRefHeight, fMouseX, fMouseY))
		return;
	if (!m_bDraggingPortrait)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect("CI_Preview", fX, fY, fWidth, fHeight))
			return;
		if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			m_bDraggingPortrait = true;
			m_fLastDragMouseX = fMouseX;
			m_fLastDragMouseY = fMouseY;
		}
		return;
	}
	Router.Claim_Mouse_This_Frame();
	if (!Router.Is_LeftDown())
	{
		m_bDraggingPortrait = false;
		return;
	}
	m_fPortraitYawDegrees += (fMouseX - m_fLastDragMouseX) * PORTRAIT_DRAG_DEGREES_PER_PX;
	m_fLastDragMouseX = fMouseX;
	m_fLastDragMouseY = fMouseY;
}

HRESULT Client::CCharacterInfoWindowView::Render_Portrait()
{
	m_bPortraitValid = false;
	const std::shared_ptr<CCharacter> pCharacter = m_pPortraitCharacter.lock();
	if (!m_bOpen || 0 != m_iSelectedTab || nullptr == pCharacter)
	{
		m_pView->Set_SlotVisible("CI_Preview", false);
		return S_FALSE;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("CI_Preview", fX, fY, fWidth, fHeight))
		return S_FALSE;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f)
		return S_FALSE;
	const uint32_t iTargetWidth = static_cast<uint32_t>(
		std::lround(fWidth * vViewport.x / m_pView->Get_ResolutionWidth()));
	const uint32_t iTargetHeight = static_cast<uint32_t>(
		std::lround(fHeight * vViewport.y / m_pView->Get_ResolutionHeight()));

	CCharacterPortraitRenderer::CAMERA Camera{};
	Camera.fDistance = m_Display.fPortraitDistance;
	Camera.fEyeHeight = m_Display.fPortraitEyeHeight;
	Camera.fLookHeight = m_Display.fPortraitLookHeight;
	Camera.fFovDegrees = m_Display.fPortraitFovDegrees;
	Camera.fYawDegrees = m_fPortraitYawDegrees;
	const HRESULT hResult = m_pPortrait->Render(pCharacter, iTargetWidth, iTargetHeight, Camera,
		m_iPreviewHiddenAvatarKinds, m_iPreviewHiddenAvatarKinds);
	m_bPortraitValid = S_OK == hResult;
	m_pView->Set_SlotTextureSRV("CI_Preview", m_bPortraitValid ? m_pPortrait->Get_SRV() : nullptr);
	m_pView->Set_SlotVisible("CI_Preview", m_bPortraitValid);
	return m_bPortraitValid ? S_OK : (S_FALSE == hResult ? S_FALSE : E_FAIL);
}

bool_t Client::CCharacterInfoWindowView::Take_AvatarBookRequest()
{
	const bool_t bRequested = m_bAvatarBookRequested;
	m_bAvatarBookRequested = false;
	return bRequested;
}

const wstring& Client::CCharacterInfoWindowView::Get_ClassDisplayName(
	const LostArk::Shared::CHARACTER_CLASS_ID eClass) const
{
	static const wstring EMPTY;
	const auto it = m_Display.Classes.find(eClass);
	return (it != m_Display.Classes.end()) ? it->second.strDisplayName : EMPTY;
}

int32_t Client::CCharacterInfoWindowView::Get_AvatarSlotIndex(const EQUIPMENT_SLOT_KIND eKind) const
{
	if (EQUIPMENT_SLOT_KIND::AVATAR_HEAD == eKind) return m_Display.iAvatarSlotHead;
	if (EQUIPMENT_SLOT_KIND::AVATAR_ARMOR == eKind) return m_Display.iAvatarSlotOutfit;
	return -1;
}

const string& Client::CCharacterInfoWindowView::Get_EquippedAvatarItemId(
	const LostArk::Shared::CHARACTER_CLASS_ID eClass, const EQUIPMENT_SLOT_KIND eKind) const
{
	static const string EMPTY;
	const auto it = m_Display.Classes.find(eClass);
	if (it == m_Display.Classes.end())
		return EMPTY;
	const char* pKey = (EQUIPMENT_SLOT_KIND::AVATAR_HEAD == eKind) ? "head" :
		(EQUIPMENT_SLOT_KIND::AVATAR_ARMOR == eKind) ? "outfit" : nullptr;
	if (nullptr == pKey)
		return EMPTY;
	const auto found = it->second.EquippedAvatarItemIds.find(pKey);
	return (found != it->second.EquippedAvatarItemIds.end()) ? found->second : EMPTY;
}

void Client::CCharacterInfoWindowView::Draw_Label(const wstring_t& strFont, const wstring& strText,
	const f32_t fRetailX, const f32_t fRetailY, const f32_t fRetailPx, const fvector_t vColor,
	const float2_t& vOrigin)
{
	if (strText.empty())
		return;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	/* Baked size nearest the on-screen line height, drawn 1:1 when it matches (no blur). */
	f32_t fScale = 1.f;
	const wstring_t strUseFont = UILabelFont::Resolve(strFont,
		fRetailPx * RETAIL_SCALE * fUiScale * TEXT_BOOST, fScale);
	/* SpriteFont applies the origin as a fractional pixel offset (measured * 0.5 for a centred
	label), which would push a 1:1 baked glyph off the pixel grid; anchor the whole string at a
	whole-pixel top-left instead and draw with a zero origin. */
	const float2_t vMeasured = CGameInstance::Get().Measure_Text(strUseFont, strText.c_str());
	const float2_t vPosition(
		std::round(Ref_X(fRetailX) * fScaleX - vMeasured.x * fScale * vOrigin.x),
		std::round(Ref_Y(fRetailY) * fScaleY - vMeasured.y * fScale * vOrigin.y));
	const float2_t vTopLeft(0.f, 0.f);
	if (m_bCovered && CUIInputRouter::Get().Is_UnderTopWindow(
		vPosition.x + vMeasured.x * fScale * 0.5f, vPosition.y + vMeasured.y * fScale * 0.5f))
		return;
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(),
		float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 0.75f), 0.f, vTopLeft, fScale);
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(), vPosition, vColor, 0.f, vTopLeft, fScale);
}

void Client::CCharacterInfoWindowView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pView)
		return;
	if (!CCombatHUDViewModel::Get().Get_Player().isValid)
		return;

	const float2_t CENTER(0.5f, 0.5f);
	const float2_t LEFT(0.f, 0.5f);
	const float2_t RIGHT(1.f, 0.5f);

	/* Window chrome: s_titleButton_V2_onlyUIWindow (YoonGasiIIM 18, #fff7e2, centred). */
	Draw_Label(FONT_YOON, TEXT_TITLE, WINDOW_WIDTH * 0.5f, 18.f, 18.f, COLOR_CREAM, CENTER);
	/* EFToggleButton_Tab_V2 labels: 16px, white when selected, #919ca6 otherwise. */
	constexpr f32_t TAB_X[5] = { 0.f, 172.f, 344.f, 516.f, 689.f };
	constexpr f32_t TAB_W[5] = { 168.4f, 168.4f, 168.4f, 169.4f, 169.4f };
	for (int32_t i = 0; i < 5; ++i)
	{
		Draw_Label(FONT_YOON, TEXT_TAB[i], 16.f + TAB_X[i] + TAB_W[i] * 0.5f, 57.f, 16.f,
			(i == m_iSelectedTab) ? COLOR_WHITE : COLOR_MUTED, CENTER);
	}
	if (0 != m_iSelectedTab)
		return;

	/* Ability page (843,106): item level box, combat power, basic stats. */
	Draw_Label(FONT_YOON, TEXT_ITEM_LEVEL, 951.f, 138.f, 16.f, COLOR_MUTED, CENTER);
	Draw_Label(FONT_YOON, m_Display.strItemLevel, 951.f, 190.f, 32.f, COLOR_WHITE, CENTER);
	Draw_Label(FONT_YOON, TEXT_COMBAT_POWER, 958.f, 235.f, 18.f, COLOR_WHITE, CENTER);
	Draw_Label(FONT_YOON, m_Display.strCombatPower, 951.f, 262.f, 17.6f, COLOR_WHITE, CENTER);
	Draw_Label(FONT_YOON, TEXT_BASIC_STATS, 951.f, 301.f, 16.f, COLOR_MUTED, CENTER);
	{
		wstring strAttack = m_bHasAttackPower ? std::to_wstring(m_iAttackPower) : L"-";
		wstring strMaxHp = (0u != m_iMaximumHp) ? std::to_wstring(m_iMaximumHp) : L"-";
		const STAT_ROW BasicRows[2] = { { TEXT_ATTACK, strAttack }, { TEXT_MAX_HP, strMaxHp } };
		for (int32_t r = 0; r < 2; ++r)
		{
			const f32_t fRowY = 324.f + 33.f * static_cast<f32_t>(r) + 14.f;
			Draw_Label(FONT_YG760, BasicRows[r].strName, 869.f, fRowY, 14.f, COLOR_GOLD, LEFT);
			Draw_Label(FONT_YG760, BasicRows[r].strValue, 1040.f, fRowY, 14.f, COLOR_WHITE, RIGHT);
		}
	}
	if (!m_bAvatarMode)
	{
		Draw_Label(FONT_YOON, TEXT_COMBAT_STATS, 951.f, 411.f, 16.f, COLOR_MUTED, CENTER);
		for (size_t r = 0; r < m_Display.CombatStats.size() && r < 6; ++r)
		{
			const f32_t fRowY = 436.f + 32.3f * static_cast<f32_t>(r) + 14.f;
			Draw_Label(FONT_YG760, m_Display.CombatStats[r].strName, 869.f, fRowY, 14.f, COLOR_GOLD, LEFT);
			Draw_Label(FONT_YG760, m_Display.CombatStats[r].strValue, 1040.f, fRowY, 14.f, COLOR_WHITE, RIGHT);
		}
	}

	/* Info page (13,77): class, name, level rows under the emblem. */
	{
		wstring strClassName;
		const auto it = m_Display.Classes.find(m_ePortraitClass);
		if (it != m_Display.Classes.end())
			strClassName = it->second.strDisplayName;
		Draw_Label(FONT_YOON, strClassName, 190.f, 183.f, 14.f, COLOR_MUTED, CENTER);
		Draw_Label(FONT_YOON, m_strNickName, 190.f, 213.f, 18.f, COLOR_NAME_YELLOW, CENTER);
		Draw_Label(FONT_YOON, m_Display.strCharacterLevel, 186.f, 234.f, 14.f, COLOR_LEVEL_GOLD, CENTER);
		Draw_Label(FONT_YOON, m_Display.strExpeditionLevel, 55.f, 236.f, 15.f, COLOR_LEVEL_GOLD, CENTER);
		const wstring* pRows[3] = { &m_Display.strTitle, &m_Display.strGuild, &m_Display.strEstate };
		const fvector_t RowColors[3] = { COLOR_TITLE_BLUE, COLOR_MUTED, COLOR_WHITE };
		for (int32_t r = 0; r < 3; ++r)
			Draw_Label(FONT_YOON, *pRows[r], 183.f, 314.f + 35.f * static_cast<f32_t>(r) + 18.f,
				14.f, RowColors[r], CENTER);
		Draw_Label(FONT_YOON, TEXT_SPECIAL_GEAR, 190.f, 466.f, 14.f, COLOR_WHITE, CENTER);
		for (size_t k = 0; k < m_Display.ArkPassivePoints.size() && k < 3; ++k)
			Draw_Label(FONT_YG760, m_Display.ArkPassivePoints[k], 36.f + 112.f * static_cast<f32_t>(k) + 46.f,
				605.f, 14.f, COLOR_WHITE, CENTER);
	}

	/* Preset strip, bottom bar. Shared_PresetBtn: the two unlocked tabs carry "1"/"2"
	(YG760 16, white); the locked ones show the lock art only. */
	{
		f32_t fTabX = 0.f, fTabY = 0.f, fTabWidth = 0.f, fTabHeight = 0.f;
		for (int32_t p = 0; p < 2; ++p)
		{
			if (!m_pView->Get_SlotRect("CI_PresetTab_" + std::to_string(p), fTabX, fTabY, fTabWidth, fTabHeight))
				continue;
			f32_t fOriginX = 0.f, fOriginY = 0.f;
			(void)Get_WindowOrigin(fOriginX, fOriginY);
			Draw_Label(FONT_YG760, std::to_wstring(p + 1),
				(fTabX + fTabWidth * 0.5f - fOriginX) / RETAIL_SCALE,
				(fTabY + fTabHeight * 0.5f - fOriginY) / RETAIL_SCALE - 1.f,
				16.f, COLOR_WHITE, CENTER);
		}
	}
	Draw_Label(FONT_YOON, m_Display.strPresetName, 360.f, 142.f, 13.f, COLOR_WHITE, LEFT);
	Draw_Label(FONT_YG760, m_bAvatarMode ? TEXT_AVATAR_BOOK : TEXT_WEAPON_EFFECT,
		BOTTOM_LEFT_BUTTON_X + 34.f * BOTTOM_LEFT_BUTTON_SCALE + (BOTTOM_LEFT_BUTTON_W - 34.f * BOTTOM_LEFT_BUTTON_SCALE) * 0.5f,
		BOTTOM_LEFT_BUTTON_Y + BOTTOM_LEFT_BUTTON_H * 0.5f, 13.f * BOTTOM_LEFT_BUTTON_SCALE,
		COLOR_WHITE, CENTER);
	Draw_Label(FONT_YG760, TEXT_DETAIL, 954.f, 661.f, 14.f, COLOR_WHITE, CENTER);
	Draw_Label(FONT_YG760, TEXT_EQUIP, 558.f, 620.f, 14.f, m_bAvatarMode ? COLOR_DIM : COLOR_WHITE, CENTER);
	Draw_Label(FONT_YG760, TEXT_AVATAR, 692.f, 620.f, 14.f, m_bAvatarMode ? COLOR_WHITE : COLOR_DIM, CENTER);
	Draw_Label(FONT_YG760, TEXT_HONOR + L" " + m_Display.strHonor, 614.f, 663.f, 14.f, COLOR_WHITE, CENTER);
}
