#include "AvatarBookWindowView.h"

#include "Character.h"
#include "CharacterInfoWindowView.h"
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
#include <cstring>
#include <fstream>

namespace
{
	/* avatarbook.gfx is authored at 1920x1080; the layout scales it by 2/3 onto 1280x720. */
	constexpr f32_t RETAIL_SCALE = 2.f / 3.f;
	constexpr f32_t WINDOW_WIDTH = 1532.f;
	constexpr f32_t TITLE_BAR_HEIGHT = 36.f;
	constexpr f32_t PORTRAIT_DRAG_DEGREES_PER_PX = 0.6f;
	constexpr f32_t TEXT_BOOST = 1.15f;
	constexpr f32_t DOUBLE_CLICK_SECONDS = 0.4f;
	constexpr int32_t TAB_COUNT = 10;
	constexpr int32_t GRID_CELLS = 72;
	constexpr int32_t HAS_CELLS = 10;
	constexpr int32_t PREVIEW_SLOTS[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14 };
	constexpr int32_t SHOW_CHECKS = 7;
	/* Category tabs that narrow the grid to one avatar kind (AvatarBookMainTabGroup order:
	all, weapon, head, face1, face2, top, pants, ...). Other tabs have no catalog content yet. */
	constexpr int32_t TAB_ALL = 0;
	constexpr int32_t TAB_HEAD = 2;
	constexpr int32_t TAB_OUTFIT = 5;
	constexpr uint32_t AVATAR_KINDS_MASK =
		(1u << ETOUI(EQUIPMENT_SLOT_KIND::AVATAR_HEAD)) |
		(1u << ETOUI(EQUIPMENT_SLOT_KIND::AVATAR_ARMOR));

	const wstring_t FONT_YOON = TEXT("Font_YoonGasiIIM");
	const wstring_t FONT_YG760 = TEXT("Font_YG760");

	/* Retail strings (UTF-16 escapes keep this file ASCII). */
	const wstring TEXT_TITLE = L"\xC544\xBC14\xD0C0 \xB3C4\xAC10";                 // avatar book
	const wstring TEXT_AUTO_EQUIP = L"\xC790\xB3D9 \xC7A5\xCC29";                 // auto equip
	const wstring TEXT_REVERT = L"\xB418\xB3CC\xB9AC\xAE30";                      // revert
	const wstring TEXT_SAVE = L"\xC800\xC7A5";                                    // save
	const wstring TEXT_DETAIL_OPTION = L"\xC0C1\xC138 \xC635\xC158";              // detail option
	const wstring TEXT_ROTATE = L"\xB9C8\xC6B0\xC2A4 \xB4DC\xB798\xADF8\xB85C \xD68C\xC804"; // drag to rotate

	const fvector_t COLOR_WHITE = Colors::White;
	const fvector_t COLOR_MUTED = XMVectorSet(145.f / 255.f, 156.f / 255.f, 166.f / 255.f, 1.f);
	const fvector_t COLOR_CREAM = XMVectorSet(1.f, 247.f / 255.f, 226.f / 255.f, 1.f);

	bool_t ConvertUtf8ToWide(const string& strUtf8, wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return true;
		const int32_t iLength = ::MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLength <= 1)
			return false;
		outWide.assign(static_cast<size_t>(iLength - 1), L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, outWide.data(), iLength);
		return true;
	}

	const char* Class_CatalogKey(const LostArk::Shared::CHARACTER_CLASS_ID eClass)
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

	uint32_t Kind_Bit(const EQUIPMENT_SLOT_KIND eKind)
	{
		return 1u << ETOUI(eKind);
	}
}

Client::CAvatarBookWindowView::CAvatarBookWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/AvatarBook/AvatarBook_Layout.json") }
	, m_pPortrait{ std::make_unique<CCharacterPortraitRenderer>(pDevice, pContext) }
{
	m_SlotIds = m_pView->Get_SlotIds();
	Load_DisplayData();
	m_fPortraitYawDegrees = m_Display.fPortraitYawDegrees;
	Hide();
}

Client::CAvatarBookWindowView::~CAvatarBookWindowView()
{
}

void Client::CAvatarBookWindowView::Load_DisplayData()
{
	m_Display = {};
	const filesystem::path DataPath = CProjectDataRoot::Resolve(L"UI/AvatarBook/AvatarBookDisplay.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
	{
		OutputDebugStringA("[AvatarBook] AvatarBookDisplay.json missing -- labels stay empty.\n");
		return;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		OutputDebugStringA(("[AvatarBook] AvatarBookDisplay.json parse failed: " + Error + "\n").c_str());
		return;
	}
	const auto ReadText = [](const DATA_JSON_VALUE& Object, const char* pKey, wstring& outText)
	{
		const DATA_JSON_VALUE* pText = Object.Find(pKey);
		if (nullptr != pText && pText->Is_String())
			(void)ConvertUtf8ToWide(pText->Get_String(), outText);
	};
	const auto ReadNumber = [](const DATA_JSON_VALUE& Object, const char* pKey, f32_t& outValue)
	{
		const DATA_JSON_VALUE* pNumber = Object.Find(pKey);
		if (nullptr != pNumber && pNumber->Is_Number())
			outValue = static_cast<f32_t>(pNumber->Get_Number());
	};
	if (const DATA_JSON_VALUE* pTabs = Root.Find("tabNames"); nullptr != pTabs && pTabs->Is_Array())
	{
		for (const DATA_JSON_VALUE& Value : pTabs->Get_Array())
		{
			wstring strName;
			if (Value.Is_String())
				(void)ConvertUtf8ToWide(Value.Get_String(), strName);
			m_Display.TabNames.push_back(std::move(strName));
		}
	}
	ReadText(Root, "categoryComboText", m_Display.strCategoryCombo);
	ReadText(Root, "alignComboText", m_Display.strAlignCombo);
	ReadText(Root, "searchPlaceholder", m_Display.strSearchPlaceholder);
	ReadText(Root, "favoriteCount", m_Display.strFavoriteCount);
	ReadText(Root, "avatarPoint", m_Display.strAvatarPoint);
	if (const DATA_JSON_VALUE* pPortrait = Root.Find("portrait"); nullptr != pPortrait && pPortrait->Is_Object())
	{
		ReadNumber(*pPortrait, "distance", m_Display.fPortraitDistance);
		ReadNumber(*pPortrait, "eyeHeight", m_Display.fPortraitEyeHeight);
		ReadNumber(*pPortrait, "lookHeight", m_Display.fPortraitLookHeight);
		ReadNumber(*pPortrait, "fovDegrees", m_Display.fPortraitFovDegrees);
		ReadNumber(*pPortrait, "yawDegrees", m_Display.fPortraitYawDegrees);
	}
	const auto ReadIndex = [&ReadNumber](const DATA_JSON_VALUE& Object, const char* pKey, int32_t& outIndex)
	{
		f32_t fValue = -1.f;
		ReadNumber(Object, pKey, fValue);
		outIndex = static_cast<int32_t>(fValue);
	};
	if (const DATA_JSON_VALUE* pSlots = Root.Find("previewSlotIndices"); nullptr != pSlots && pSlots->Is_Object())
	{
		ReadIndex(*pSlots, "head", m_Display.iSlotHead);
		ReadIndex(*pSlots, "outfit", m_Display.iSlotOutfit);
	}
	if (const DATA_JSON_VALUE* pChecks = Root.Find("previewShowCheckIndices"); nullptr != pChecks && pChecks->Is_Object())
	{
		ReadIndex(*pChecks, "head", m_Display.iShowCheckHead);
		ReadIndex(*pChecks, "outfit", m_Display.iShowCheckOutfit);
	}
}

bool_t Client::CAvatarBookWindowView::Register_Click(const string& strId)
{
	const auto Now = std::chrono::steady_clock::now();
	const bool_t bDouble = strId == m_strLastClickId &&
		std::chrono::duration<f32_t>(Now - m_LastClickTime).count() <= DOUBLE_CLICK_SECONDS;
	m_LastClickTime = bDouble ? std::chrono::steady_clock::time_point{} : Now;
	m_strLastClickId = bDouble ? string{} : strId;
	return bDouble;
}

void Client::CAvatarBookWindowView::Open()
{
	m_bOpen = true;
	m_bJustOpened = true;
	m_iSelectedCell = -1;
	m_iSelectedTab = TAB_ALL;
	m_iSelectedCamera = 0;
	m_fPortraitYawDegrees = m_Display.fPortraitYawDegrees;
}

void Client::CAvatarBookWindowView::Hide()
{
	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, false);
	m_bPortraitValid = false;
	m_bDraggingPanel = false;
	m_bDraggingPortrait = false;
}

bool_t Client::CAvatarBookWindowView::Get_WindowOrigin(f32_t& fX, f32_t& fY) const
{
	f32_t fWidth = 0.f, fHeight = 0.f;
	return m_pView->Get_SlotRect("AB_WinBg", fX, fY, fWidth, fHeight);
}

f32_t Client::CAvatarBookWindowView::Ref_X(const f32_t fRetailX) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fX + fRetailX * RETAIL_SCALE;
}

f32_t Client::CAvatarBookWindowView::Ref_Y(const f32_t fRetailY) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fY + fRetailY * RETAIL_SCALE;
}

void Client::CAvatarBookWindowView::Rebuild_Entries(const std::shared_ptr<CCharacter>& pLocalCharacter,
	const LostArk::Shared::CHARACTER_CLASS_ID eClass, const CCharacterInfoWindowView& InfoView)
{
	m_Entries.clear();
	const char* pClassKey = Class_CatalogKey(eClass);
	for (const ITEM_DEFINITION& Item : CItemCatalog::Get_Items())
	{
		if (Item.strGrade != "avatar" || Item.strIconPath.empty())
			continue;
		if (!Item.strCharacterClass.empty() && (nullptr == pClassKey || Item.strCharacterClass != pClassKey))
			continue;
		AVATAR_ENTRY Entry{};
		Entry.pItem = &Item;
		if (Item.strEquipSlot == "avatarHead") Entry.eKind = EQUIPMENT_SLOT_KIND::AVATAR_HEAD;
		else if (Item.strEquipSlot == "avatarOutfit") Entry.eKind = EQUIPMENT_SLOT_KIND::AVATAR_ARMOR;
		else continue;
		/* Only an avatar the character actually carries as a part can go on the mannequin. */
		if (nullptr == pLocalCharacter || !pLocalCharacter->Has_AvatarPart(Entry.eKind))
			continue;
		const bool_t bHead = EQUIPMENT_SLOT_KIND::AVATAR_HEAD == Entry.eKind;
		Entry.iPreviewSlot = bHead ? m_Display.iSlotHead : m_Display.iSlotOutfit;
		Entry.iShowCheck = bHead ? m_Display.iShowCheckHead : m_Display.iShowCheckOutfit;
		(void)InfoView;
		m_Entries.push_back(Entry);
	}
}

void Client::CAvatarBookWindowView::Reset_PreviewToReal(const std::shared_ptr<CCharacter>& pLocalCharacter)
{
	m_iPreviewHiddenAvatarKinds = 0u;
	if (nullptr == pLocalCharacter)
		return;
	for (const AVATAR_ENTRY& Entry : m_Entries)
	{
		if (!pLocalCharacter->Is_AvatarPartVisible(Entry.eKind))
			m_iPreviewHiddenAvatarKinds |= Kind_Bit(Entry.eKind);
	}
}

void Client::CAvatarBookWindowView::Update(const f32_t fTimeDelta,
	const std::shared_ptr<CCharacter>& pLocalCharacter, const HUD_PLAYER_STATE& Player,
	const CCharacterInfoWindowView& InfoView)
{
	(void)fTimeDelta;
	if (!m_bOpen)
	{
		Hide();
		m_pPortraitCharacter.reset();
		return;
	}
	const bool_t bCharacterChanged = m_pPortraitCharacter.lock() != pLocalCharacter;
	m_pPortraitCharacter = pLocalCharacter;
	if (nullptr != pLocalCharacter)
		(void)ConvertUtf8ToWide(pLocalCharacter->Get_NickName(), m_strNickName);
	else
		m_strNickName.clear();
	if (m_bJustOpened || bCharacterChanged || m_eClass != Player.eCharacterClass)
	{
		m_eClass = Player.eCharacterClass;
		m_strClassName = InfoView.Get_ClassDisplayName(m_eClass);
		Rebuild_Entries(pLocalCharacter, m_eClass, InfoView);
		Reset_PreviewToReal(pLocalCharacter);
		m_iSelectedCell = -1;
		m_bJustOpened = false;
	}

	Update_Visibility();
	Update_PanelDrag();
	Update_Chrome();
	Update_Tabs();
	Update_Grid();
	Update_PreviewSlots(pLocalCharacter);
	Update_Buttons(pLocalCharacter);
	Update_PortraitDrag();

	/* Anything over the window belongs to the window; this window is the topmost runtime UI
	while open (CMainApp constructs it last), so the others' text passes skip its rect. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_pView->Get_SlotRect("AB_WinBg", fX, fY, fWidth, fHeight))
	{
		if (Router.Is_Hovered(fX, fY, fWidth, fHeight,
			m_pView->Get_ResolutionWidth(), m_pView->Get_ResolutionHeight()))
			Router.Claim_Mouse_This_Frame();
		const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
		if (vViewport.x > 0.f && vViewport.y > 0.f)
		{
			const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
			const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
			Router.Set_TopWindowRect(fX * fScaleX, fY * fScaleY, fWidth * fScaleX, fHeight * fScaleY);
		}
	}
}

void Client::CAvatarBookWindowView::Update_Visibility()
{
	for (const string& strId : m_SlotIds)
	{
		/* Grid/preview-slot contents, tab highlights and the render target are decided by
		their own passes below; everything else is static chrome. */
		if (Starts_With(strId, "AB_Cell") || Starts_With(strId, "AB_SlotIcon_") ||
			Starts_With(strId, "AB_HasCellIcon_") || Starts_With(strId, "AB_TabHi_") ||
			Starts_With(strId, "AB_Preview"))
			continue;
		m_pView->Set_SlotVisible(strId, true);
	}
	m_pView->Set_SlotVisible("AB_PreviewBg", true);
}

void Client::CAvatarBookWindowView::Clamp_ToScreen()
{
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("AB_WinBg", fX, fY, fWidth, fHeight))
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

void Client::CAvatarBookWindowView::Update_PanelDrag()
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
		if (m_pView->Get_SlotRect("AB_Close", fCloseX, fCloseY, fCloseWidth, fCloseHeight) &&
			Router.Is_Hovered(fCloseX, fCloseY, fCloseWidth, fCloseHeight, fRefWidth, fRefHeight))
			return;
		if (Router.Is_Clicked(Ref_X(0.f), Ref_Y(0.f), WINDOW_WIDTH * RETAIL_SCALE,
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

void Client::CAvatarBookWindowView::Update_Chrome()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("AB_Close", fX, fY, fWidth, fHeight))
		return;
	const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
	m_pView->Set_SlotTexture("AB_Close", bHovered ?
		"UI/CharacterInfo/win_close_over.png" : "UI/CharacterInfo/win_close_normal.png");
	if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
	{
		CMainApp::Play_UIButtonClickSound();
		Close();
	}
}

void Client::CAvatarBookWindowView::Update_Tabs()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	for (int32_t k = 0; k < TAB_COUNT; ++k)
	{
		const string strHi = "AB_TabHi_" + std::to_string(k);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect(strHi, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight) &&
			k != m_iSelectedTab)
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedTab = k;
			m_iSelectedCell = -1;
		}
		m_pView->Set_SlotVisible(strHi, k == m_iSelectedTab || bHovered);
	}
}

void Client::CAvatarBookWindowView::Update_Grid()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();

	/* Entries shown on the current tab, in catalog order. */
	vector<size_t> Shown;
	for (size_t i = 0; i < m_Entries.size(); ++i)
	{
		const EQUIPMENT_SLOT_KIND eKind = m_Entries[i].eKind;
		if (TAB_ALL == m_iSelectedTab ||
			(TAB_HEAD == m_iSelectedTab && EQUIPMENT_SLOT_KIND::AVATAR_HEAD == eKind) ||
			(TAB_OUTFIT == m_iSelectedTab && EQUIPMENT_SLOT_KIND::AVATAR_ARMOR == eKind))
			Shown.push_back(i);
	}

	for (int32_t k = 0; k < GRID_CELLS; ++k)
	{
		const string strSuffix = std::to_string(k);
		const bool_t bHasEntry = static_cast<size_t>(k) < Shown.size();
		const AVATAR_ENTRY* pEntry = bHasEntry ? &m_Entries[Shown[k]] : nullptr;
		if (nullptr != pEntry)
			m_pView->Set_SlotTexture("AB_CellIcon_" + strSuffix, pEntry->pItem->strIconPath);
		m_pView->Set_SlotVisible("AB_CellBg_" + strSuffix, true);
		m_pView->Set_SlotVisible("AB_CellGrade_" + strSuffix, bHasEntry);
		m_pView->Set_SlotVisible("AB_CellIcon_" + strSuffix, bHasEntry);
		const bool_t bWorn = nullptr != pEntry &&
			0u == (m_iPreviewHiddenAvatarKinds & Kind_Bit(pEntry->eKind));
		m_pView->Set_SlotVisible("AB_CellEquipped_" + strSuffix, bWorn);
		m_pView->Set_SlotVisible("AB_CellSelected_" + strSuffix, bHasEntry && k == m_iSelectedCell);

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!bHasEntry || !m_pView->Get_SlotRect("AB_CellBg_" + strSuffix, fX, fY, fWidth, fHeight))
			continue;
		if (Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight) &&
			Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			/* Click: try-on, the avatar goes onto the mannequin (preview only). Double click: the
			real equip on the world character (retail). */
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedCell = k;
			m_iPreviewHiddenAvatarKinds &= ~Kind_Bit(pEntry->eKind);
			if (Register_Click("AB_CellBg_" + strSuffix))
			{
				const std::shared_ptr<CCharacter> pCharacter = m_pPortraitCharacter.lock();
				if (nullptr != pCharacter)
					pCharacter->Set_AvatarPartVisible(pEntry->eKind, true);
			}
		}
	}

	/* Owned-avatar strip at the bottom: every entry, regardless of tab. */
	for (int32_t k = 0; k < HAS_CELLS; ++k)
	{
		const string strIconId = "AB_HasCellIcon_" + std::to_string(k);
		const bool_t bHasEntry = static_cast<size_t>(k) < m_Entries.size();
		if (bHasEntry)
			m_pView->Set_SlotTexture(strIconId, m_Entries[k].pItem->strIconPath);
		m_pView->Set_SlotVisible(strIconId, bHasEntry);
	}
}

void Client::CAvatarBookWindowView::Update_PreviewSlots(const std::shared_ptr<CCharacter>& pLocalCharacter)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();

	for (const int32_t iSlot : PREVIEW_SLOTS)
	{
		const string strIconId = "AB_SlotIcon_" + std::to_string(iSlot);
		const AVATAR_ENTRY* pEntry = nullptr;
		for (const AVATAR_ENTRY& Entry : m_Entries)
			if (Entry.iPreviewSlot == iSlot) { pEntry = &Entry; break; }
		const bool_t bWorn = nullptr != pEntry &&
			0u == (m_iPreviewHiddenAvatarKinds & Kind_Bit(pEntry->eKind));
		if (bWorn)
			m_pView->Set_SlotTexture(strIconId, pEntry->pItem->strIconPath);
		m_pView->Set_SlotVisible(strIconId, bWorn);
		if (!bWorn)
			continue;
		/* Double click on a worn slot: the real unequip on the world character (retail). */
		const string strBgId = "AB_SlotBg_" + std::to_string(iSlot);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pView->Get_SlotRect(strBgId, fX, fY, fWidth, fHeight) &&
			Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight) &&
			Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			if (Register_Click(strBgId) && nullptr != pLocalCharacter)
			{
				pLocalCharacter->Set_AvatarPartVisible(pEntry->eKind, false);
				m_iPreviewHiddenAvatarKinds |= Kind_Bit(pEntry->eKind);
			}
		}
	}

	/* Eye toggles (showItem_k): the one beside a preview slot with an avatar takes it off / puts
	it back on the mannequin; the others are display only. */
	for (int32_t k = 0; k < SHOW_CHECKS; ++k)
	{
		const string strCheckId = "AB_ShowCheck_" + std::to_string(k);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect(strCheckId, fX, fY, fWidth, fHeight))
			continue;
		const AVATAR_ENTRY* pEntry = nullptr;
		for (const AVATAR_ENTRY& Entry : m_Entries)
			if (Entry.iShowCheck == k) { pEntry = &Entry; break; }
		if (nullptr == pEntry)
		{
			m_pView->Set_SlotTexture(strCheckId, "UI/AvatarBook/show_check_normal.png");
			continue;
		}
		const uint32_t iBit = Kind_Bit(pEntry->eKind);
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iPreviewHiddenAvatarKinds ^= iBit;
		}
		const bool_t bWorn = 0u == (m_iPreviewHiddenAvatarKinds & iBit);
		m_pView->Set_SlotTexture(strCheckId, bWorn ? "UI/AvatarBook/show_check_selected.png" :
			(bHovered ? "UI/AvatarBook/show_check_over.png" : "UI/AvatarBook/show_check_normal.png"));
	}
}

void Client::CAvatarBookWindowView::Update_Buttons(const std::shared_ptr<CCharacter>& pLocalCharacter)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	struct BUTTON { const char* pSlotId; const char* pNormal; const char* pOver; };
	constexpr BUTTON BUTTONS[] = {
		{ "AB_EditBtn", "UI/AvatarBook/edit_btn.png", "UI/AvatarBook/edit_btn_over.png" },
		{ "AB_PresetLoadBtn", "UI/AvatarBook/preset_btn.png", "UI/AvatarBook/preset_btn_over.png" },
		{ "AB_PresetTab_0", "UI/CharacterInfo/preset_tab_1.png", "UI/CharacterInfo/preset_tab_1_over.png" },
		{ "AB_SearchBtn", "UI/AvatarBook/find_btn.png", "UI/AvatarBook/find_btn_over.png" },
		{ "AB_StepPrev", "UI/AvatarBook/stepper_arrow.png", "UI/AvatarBook/stepper_arrow_over.png" },
		{ "AB_StepNext", "UI/AvatarBook/stepper_arrow.png", "UI/AvatarBook/stepper_arrow_over.png" },
		{ "AB_AutoEquipBtn", "UI/AvatarBook/yellow_btn.png", "UI/AvatarBook/yellow_btn_over.png" },
		{ "AB_RevertBtn", "UI/AvatarBook/btn_normal.png", "UI/AvatarBook/btn_over.png" },
		{ "AB_SaveBtn", "UI/AvatarBook/btn_normal.png", "UI/AvatarBook/btn_over.png" },
	};
	for (const BUTTON& Button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		m_pView->Set_SlotTexture(Button.pSlotId, bHovered ? Button.pOver : Button.pNormal);
		if (!bHovered || !Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			continue;
		CMainApp::Play_UIButtonClickSound();
		if (0 == strcmp(Button.pSlotId, "AB_AutoEquipBtn"))
		{
			m_iPreviewHiddenAvatarKinds = 0u;
		}
		else if (0 == strcmp(Button.pSlotId, "AB_RevertBtn"))
		{
			Reset_PreviewToReal(pLocalCharacter);
		}
		else if (0 == strcmp(Button.pSlotId, "AB_SaveBtn") && nullptr != pLocalCharacter)
		{
			/* The one real equip/unequip: the mannequin state becomes the world character's. */
			for (const AVATAR_ENTRY& Entry : m_Entries)
				pLocalCharacter->Set_AvatarPartVisible(Entry.eKind,
					0u == (m_iPreviewHiddenAvatarKinds & Kind_Bit(Entry.eKind)));
		}
		/* Edit/preset/search/stepper: press feedback only, no collection data behind them yet. */
	}

	/* Camera presets 1/2/3: full body, upper body, face. */
	for (int32_t k = 0; k < 3; ++k)
	{
		const string strTabId = "AB_CameraTab_" + std::to_string(k);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect(strTabId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight) &&
			k != m_iSelectedCamera)
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedCamera = k;
		}
		m_pView->Set_SlotTexture(strTabId, (k == m_iSelectedCamera) ? "UI/AvatarBook/camera_tab_selected.png" :
			(bHovered ? "UI/AvatarBook/camera_tab_over.png" : "UI/AvatarBook/camera_tab.png"));
	}
}

void Client::CAvatarBookWindowView::Update_PortraitDrag()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	if (!Router.Get_MousePosition(fRefWidth, fRefHeight, fMouseX, fMouseY))
		return;
	if (!m_bDraggingPortrait)
	{
		/* The mannequin area minus the slot columns on its right (previewMouseAreaMc). */
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect("AB_PreviewBg", fX, fY, fWidth, fHeight))
			return;
		if (Router.Is_Clicked(fX, fY, fWidth * 0.78f, fHeight, fRefWidth, fRefHeight))
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

HRESULT Client::CAvatarBookWindowView::Render_Portrait()
{
	m_bPortraitValid = false;
	const std::shared_ptr<CCharacter> pCharacter = m_pPortraitCharacter.lock();
	if (!m_bOpen || nullptr == pCharacter)
	{
		m_pView->Set_SlotVisible("AB_Preview", false);
		return S_FALSE;
	}
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("AB_Preview", fX, fY, fWidth, fHeight))
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
	if (1 == m_iSelectedCamera)
	{
		Camera.fDistance *= 0.6f;
		Camera.fEyeHeight += 0.25f;
		Camera.fLookHeight += 0.3f;
	}
	else if (2 == m_iSelectedCamera)
	{
		Camera.fDistance *= 0.35f;
		Camera.fEyeHeight += 0.5f;
		Camera.fLookHeight += 0.55f;
	}
	const HRESULT hResult = m_pPortrait->Render(pCharacter, iTargetWidth, iTargetHeight, Camera,
		AVATAR_KINDS_MASK, m_iPreviewHiddenAvatarKinds);
	m_bPortraitValid = S_OK == hResult;
	m_pView->Set_SlotTextureSRV("AB_Preview", m_bPortraitValid ? m_pPortrait->Get_SRV() : nullptr);
	m_pView->Set_SlotVisible("AB_Preview", m_bPortraitValid);
	return m_bPortraitValid ? S_OK : (S_FALSE == hResult ? S_FALSE : E_FAIL);
}

void Client::CAvatarBookWindowView::Draw_Label(const wstring_t& strFont, const wstring& strText,
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
	/* Whole-pixel top-left anchor + zero origin: SpriteFont's fractional origin offset would
	push a 1:1 baked glyph off the pixel grid (see CCharacterInfoWindowView::Draw_Label). */
	const float2_t vMeasured = CGameInstance::Get().Measure_Text(strUseFont, strText.c_str());
	const float2_t vPosition(
		std::round(Ref_X(fRetailX) * fScaleX - vMeasured.x * fScale * vOrigin.x),
		std::round(Ref_Y(fRetailY) * fScaleY - vMeasured.y * fScale * vOrigin.y));
	const float2_t vTopLeft(0.f, 0.f);
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(),
		float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 0.75f), 0.f, vTopLeft, fScale);
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(), vPosition, vColor, 0.f, vTopLeft, fScale);
}

void Client::CAvatarBookWindowView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pView)
		return;
	if (!CCombatHUDViewModel::Get().Get_Player().isValid)
		return;
	const float2_t CENTER(0.5f, 0.5f);
	const float2_t LEFT(0.f, 0.5f);

	Draw_Label(FONT_YOON, TEXT_TITLE, WINDOW_WIDTH * 0.5f, 18.f, 18.f, COLOR_CREAM, CENTER);
	/* Category name over the list (tabName_lb), favourites count, filter row. */
	if (m_iSelectedTab >= 0 && static_cast<size_t>(m_iSelectedTab) < m_Display.TabNames.size())
		Draw_Label(FONT_YOON, m_Display.TabNames[m_iSelectedTab], 950.f, 110.f, 16.f, COLOR_WHITE, CENTER);
	Draw_Label(FONT_YG760, m_Display.strFavoriteCount, 656.f, 114.f, 14.f, COLOR_WHITE, LEFT);
	Draw_Label(FONT_YG760, m_strClassName, 1366.f, 116.f, 14.f, COLOR_WHITE, LEFT);
	Draw_Label(FONT_YG760, m_Display.strCategoryCombo, 636.f, 156.f, 14.f, COLOR_WHITE, LEFT);
	Draw_Label(FONT_YG760, m_Display.strSearchPlaceholder, 771.f, 155.f, 13.f, COLOR_MUTED, LEFT);
	Draw_Label(FONT_YG760, TEXT_DETAIL_OPTION, 1252.f, 150.f, 14.f, COLOR_WHITE, LEFT);
	Draw_Label(FONT_YG760, m_Display.strAlignCombo, 1375.f, 157.f, 14.f, COLOR_WHITE, LEFT);
	/* Selected avatar name, owner name, avatar points. */
	if (m_iSelectedCell >= 0)
	{
		vector<size_t> Shown;
		for (size_t i = 0; i < m_Entries.size(); ++i)
		{
			const EQUIPMENT_SLOT_KIND eKind = m_Entries[i].eKind;
			if (TAB_ALL == m_iSelectedTab ||
				(TAB_HEAD == m_iSelectedTab && EQUIPMENT_SLOT_KIND::AVATAR_HEAD == eKind) ||
				(TAB_OUTFIT == m_iSelectedTab && EQUIPMENT_SLOT_KIND::AVATAR_ARMOR == eKind))
				Shown.push_back(i);
		}
		if (static_cast<size_t>(m_iSelectedCell) < Shown.size())
		{
			wstring strName;
			(void)ConvertUtf8ToWide(m_Entries[Shown[m_iSelectedCell]].pItem->strDisplayName, strName);
			Draw_Label(FONT_YOON, strName, 637.f, 645.f, 14.f, COLOR_WHITE, LEFT);
		}
	}
	Draw_Label(FONT_YG760, m_strNickName, 637.f, 770.f, 14.f, COLOR_MUTED, LEFT);
	Draw_Label(FONT_YG760, m_Display.strAvatarPoint, 1221.f, 770.f, 14.f, COLOR_WHITE, LEFT);
	/* Mannequin side: camera numbers, rotate hint, bottom buttons. */
	for (int32_t k = 0; k < 3; ++k)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pView->Get_SlotRect("AB_CameraTab_" + std::to_string(k), fX, fY, fWidth, fHeight))
			continue;
		f32_t fOriginX = 0.f, fOriginY = 0.f;
		(void)Get_WindowOrigin(fOriginX, fOriginY);
		Draw_Label(FONT_YG760, std::to_wstring(k + 1),
			(fX + fWidth * 0.5f - fOriginX) / RETAIL_SCALE,
			(fY + fHeight * 0.5f - fOriginY) / RETAIL_SCALE - 1.f, 14.f,
			(k == m_iSelectedCamera) ? COLOR_WHITE : COLOR_MUTED, CENTER);
	}
	Draw_Label(FONT_YG760, TEXT_ROTATE, 64.f, 703.f, 12.f, COLOR_WHITE, LEFT);
	Draw_Label(FONT_YG760, TEXT_AUTO_EQUIP, 76.f + 103.f * 1.5145f * 0.5f, 753.f + 18.f, 14.f, COLOR_WHITE, CENTER);
	Draw_Label(FONT_YG760, TEXT_REVERT, 234.f + 103.f * 1.5145f * 0.5f, 753.f + 18.f, 14.f, COLOR_WHITE, CENTER);
	Draw_Label(FONT_YG760, TEXT_SAVE, 392.f + 103.f * 1.5145f * 0.5f, 753.f + 18.f, 14.f, COLOR_WHITE, CENTER);
}
