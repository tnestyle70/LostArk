#include "RepairWindowView.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "ItemCatalog.h"
#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UITextOcclusion.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cmath>

namespace
{
	/* Retail authors this window on a 1920-wide stage; the layout reference is 1280, so an
	authored point size is this much smaller in reference pixels. Matches the same constant in
	Tools/LpkPipeline/build_repair_ui.py, which converted the rects. */
	constexpr f32_t STAGE_SCALE = 2.f / 3.f;
	/* The authored sizes land at 9-12 px on a 1280-wide client, which is faithful but hard to
	read. CHonorTitleWindowView carries the same constant for the same reason. */
	constexpr f32_t TEXT_BOOST = 1.15f;

	constexpr const char* PANEL_ID = "Repair_PanelBg";
	constexpr const char* TITLE_ID = "Repair_Title";
	constexpr const char* CLOSE_ID = "Repair_CloseBtn";
	constexpr const char* EQUIP_LABEL_ID = "Repair_EquipLabel";
	constexpr const char* INVENTORY_LABEL_ID = "Repair_InventoryLabel";
	constexpr const char* MONEY_BAR_ID = "Repair_MoneyBar";
	constexpr const char* EQUIP_BUTTON_ID = "Repair_EquipButton";
	constexpr const char* ALL_BUTTON_ID = "Repair_AllButton";
	constexpr const char* EQUIP_COST_BAR_ID = "Repair_EquipCostBar";
	constexpr const char* ALL_COST_BAR_ID = "Repair_AllCostBar";
	constexpr uint32_t EQUIP_SLOT_COUNT = 16;
	constexpr uint32_t BAG_SLOT_COUNT = 24;

	std::wstring Format_Silver(const uint32_t iValue)
	{
		std::wstring strDigits = std::to_wstring(iValue);
		for (int32_t iAt = static_cast<int32_t>(strDigits.size()) - 3; iAt > 0; iAt -= 3)
			strDigits.insert(static_cast<size_t>(iAt), L",");
		return strDigits;
	}

	/* Text anchors: the document needs a rect for each label, but retail draws no plate behind
	the title or the two section headings, so these slots exist only to be measured. */
	constexpr const char* TEXT_ANCHOR_IDS[] = { TITLE_ID, EQUIP_LABEL_ID, INVENTORY_LABEL_ID };

	/* 아이템 수리 / 착용 장비 / 소지 장비 / 착용 장비 수리 / 모든 장비 수리 / 소지금.
	Source strings are EFTable_GameMsg sys.repair.*; written as escapes because this file is
	compiled as ASCII (a CP949 lead byte in a comment eats the next newline under MSVC). */
	const wchar_t* TITLE_TEXT = L"\xC544\xC774\xD15C \xC218\xB9AC";
	const wchar_t* EQUIP_TEXT = L"\xCC29\xC6A9 \xC7A5\xBE44";
	const wchar_t* INVENTORY_TEXT = L"\xC18C\xC9C0 \xC7A5\xBE44";
	const wchar_t* EQUIP_BUTTON_TEXT = L"\xCC29\xC6A9 \xC7A5\xBE44 \xC218\xB9AC";
	const wchar_t* ALL_BUTTON_TEXT = L"\xBAA8\xB4E0 \xC7A5\xBE44 \xC218\xB9AC";
	const wchar_t* MONEY_TEXT = L"\xC18C\xC9C0\xAE08";
}

Client::CRepairWindowView::CRepairWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pBackgroundView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Repair/RepairUI.json") }
{
	m_pBackgroundView->Set_UISortLayer(UI_TEXT_LAYER::WINDOW_REPAIR);
	/* Authored tint is opaque, and these GameObjects live under LEVEL::STATIC, so without this
	every slot would be on screen from construction until the first Update(). */
	Hide();
}

Client::CRepairWindowView::~CRepairWindowView()
{
}

void Client::CRepairWindowView::Hide()
{
	if (nullptr != m_pBackgroundView)
		m_pBackgroundView->Set_AllSlotsVisible(false);
}

void Client::CRepairWindowView::Refresh_DamagedGear()
{
	m_EquippedGear.clear();
	m_BagGear.clear();
	m_iEquippedCost = 0u;
	m_iAllCost = 0u;
	/* Only gear ever drops below 100, so the percent alone says what is damaged. */
	for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& Item :
		CCombatHUDViewModel::Get().Get_Inventory().Items)
	{
		if (Item.iDurabilityPercent >= 100u)
			continue;
		const uint32_t iCost = LostArk::Shared::Repair_Silver_Cost(Item.iDurabilityPercent);
		m_iAllCost += iCost;
		if (LostArk::Shared::Is_Durable_Slot(Item.eEquippedSlot))
		{
			m_iEquippedCost += iCost;
			m_EquippedGear.push_back({ Item.strItemId, Item.iDurabilityPercent });
		}
		else if (LostArk::Shared::EQUIPMENT_SLOT::NONE == Item.eEquippedSlot)
		{
			m_BagGear.push_back({ Item.strItemId, Item.iDurabilityPercent });
		}
	}
}

void Client::CRepairWindowView::Update()
{
	/* Hit tests below belong to this window; the router refuses a press that lands on the
	window in front and lets only one widget take any one press. */
	CUIPointerScope PointerScope(this);
	if (nullptr == m_pBackgroundView)
		return;
	if (!m_bOpen)
	{
		Hide();
		return;
	}

	m_pBackgroundView->Set_AllSlotsVisible(true);
	for (const char* pAnchorId : TEXT_ANCHOR_IDS)
		m_pBackgroundView->Set_SlotVisible(pAnchorId, false);

	Refresh_DamagedGear();
	/* The damaged pieces fill the slot frames from the first one; the rest stay empty frames. */
	const auto FillIcons = [this](const char* pIconPrefix, const vector<GEAR_ENTRY>& Entries,
		const uint32_t iSlotCount)
	{
		for (uint32_t iSlot = 0; iSlot < iSlotCount; ++iSlot)
		{
			const string strIconId = string(pIconPrefix) + std::to_string(iSlot);
			const ITEM_DEFINITION* pItem = iSlot < Entries.size() ?
				CItemCatalog::Find_ById(Entries[iSlot].strItemId) : nullptr;
			if (nullptr != pItem && !pItem->strIconPath.empty())
			{
				m_pBackgroundView->Set_SlotTexture(strIconId, pItem->strIconPath);
				m_pBackgroundView->Set_SlotVisible(strIconId, true);
			}
			else
			{
				m_pBackgroundView->Set_SlotVisible(strIconId, false);
			}
		}
	};
	FillIcons("Repair_EquipIcon_", m_EquippedGear, EQUIP_SLOT_COUNT);
	FillIcons("Repair_InventoryIcon_", m_BagGear, BAG_SLOT_COUNT);

	Update_Drag();
	Update_Buttons();

	/* Anything over the panel belongs to the panel -- a click on it must not fall through to
	CPlayerController as a move order. */
	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelWidth = 0.f, fPanelHeight = 0.f;
	if (m_pBackgroundView->Get_SlotRect(PANEL_ID, fPanelX, fPanelY, fPanelWidth, fPanelHeight) &&
		CUIInputRouter::Get().Is_Hovered(fPanelX, fPanelY, fPanelWidth, fPanelHeight,
			m_pBackgroundView->Get_ResolutionWidth(), m_pBackgroundView->Get_ResolutionHeight()))
	{
		CUIInputRouter::Get().Claim_Mouse_This_Frame();
	}
}

void Client::CRepairWindowView::Update_Drag()
{
	Clamp_ToScreen();
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();
	CUIInputRouter& Router = CUIInputRouter::Get();

	f32_t fMouseX = 0.f, fMouseY = 0.f;
	if (!Router.Get_MousePosition(fRefWidth, fRefHeight, fMouseX, fMouseY))
		return;

	if (!m_bDraggingPanel)
	{
		/* The close button sits inside the title bar; a press on it is the button's. */
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pBackgroundView->Get_SlotRect(CLOSE_ID, fX, fY, fWidth, fHeight) &&
			Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			return;
		if (!m_pBackgroundView->Get_SlotRect(TITLE_ID, fX, fY, fWidth, fHeight))
			return;
		if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
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

	Move_Panel(fDeltaX, fDeltaY);
}

void Client::CRepairWindowView::Move_Panel(const f32_t fDeltaX, const f32_t fDeltaY)
{
	/* Every slot in this document belongs to the window, so the id list is the document's own
	rather than a second hand-kept copy. */
	for (const string& strId : m_pBackgroundView->Get_SlotIds())
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pBackgroundView->Get_SlotRect(strId, fX, fY, fWidth, fHeight))
			m_pBackgroundView->Set_SlotPosition(strId, fX + fDeltaX, fY + fDeltaY);
	}
}

void Client::CRepairWindowView::Clamp_ToScreen()
{
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pBackgroundView->Get_SlotRect(PANEL_ID, fX, fY, fWidth, fHeight))
		return;
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();
	f32_t fDeltaX = 0.f, fDeltaY = 0.f;
	if (fX < 0.f) fDeltaX = -fX;
	else if (fX + fWidth > fRefWidth) fDeltaX = fRefWidth - (fX + fWidth);
	if (fY < 0.f) fDeltaY = -fY;
	else if (fY + fHeight > fRefHeight) fDeltaY = fRefHeight - (fY + fHeight);
	if (0.f != fDeltaX || 0.f != fDeltaY)
		Move_Panel(fDeltaX, fDeltaY);
}

void Client::CRepairWindowView::Update_Buttons()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();

	struct BUTTON
	{
		const char* pId;
		bool_t bAllSlots;
	};
	const BUTTON Buttons[] = {
		{ EQUIP_BUTTON_ID, false },
		{ ALL_BUTTON_ID, true },
	};

	for (const BUTTON& Button : Buttons)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(Button.pId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered =
			Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		/* Live only with something to repair and the silver to pay for all of it. */
		const uint32_t iCost = Button.bAllSlots ? m_iAllCost : m_iEquippedCost;
		const bool_t bLive = 0u != iCost &&
			CCombatHUDViewModel::Get().Get_Inventory().iSilver >= iCost;
		m_pBackgroundView->Set_SlotTintMultiplier(Button.pId, !bLive ?
			float4_t(0.55f, 0.55f, 0.55f, 1.f) : (bHovered ?
			float4_t(1.25f, 1.25f, 1.25f, 1.f) : float4_t(1.f, 1.f, 1.f, 1.f)));
		if (bLive && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			m_bHasPendingRepair = true;
			m_bPendingRepairAllSlots = Button.bAllSlots;
		}
	}

	f32_t fCloseX = 0.f, fCloseY = 0.f, fCloseWidth = 0.f, fCloseHeight = 0.f;
	if (m_pBackgroundView->Get_SlotRect(CLOSE_ID, fCloseX, fCloseY, fCloseWidth, fCloseHeight))
	{
		const bool_t bHovered = Router.Is_Hovered(
			fCloseX, fCloseY, fCloseWidth, fCloseHeight, fRefWidth, fRefHeight);
		m_pBackgroundView->Set_SlotTintMultiplier(CLOSE_ID, bHovered ?
			float4_t(1.4f, 1.4f, 1.4f, 1.f) : float4_t(1.f, 1.f, 1.f, 1.f));
		if (Router.Is_Clicked(
			fCloseX, fCloseY, fCloseWidth, fCloseHeight, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			Close();
		}
	}
}

bool_t Client::CRepairWindowView::Try_Consume_RepairRequest(bool_t& bAllSlots)
{
	if (!m_bHasPendingRepair)
		return false;
	bAllSlots = m_bPendingRepairAllSlots;
	m_bHasPendingRepair = false;
	return true;
}

void Client::CRepairWindowView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pBackgroundView)
		return;
	/* m_bOpen has no level-change reset of its own, so a window left open into a loading
	transition would otherwise keep drawing its labels over the loading screen. */
	if (!CCombatHUDViewModel::Get().Get_Player().isValid)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewportSize.x / 1280.f;
	const f32_t fScaleY = vViewportSize.y / 720.f;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	CUIInputRouter& Router = CUIInputRouter::Get();

	/* Draws one label centred vertically in its slot's box at the size the source authored,
	through UILabelFont so a 32-42 px sprite font is not bilinearly shrunk to 11 px -- that
	resampling is what makes small window text look broken. fPivotX 0 is left-aligned inside
	the box, 0.5 centred. */
	const auto DrawLabel = [&](const char* pSlotId, const wchar_t* pText,
		const wstring_t& strFamily, const f32_t fAuthoredPx, const f32_t fPivotX)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
			return;
		f32_t fTextScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(
			strFamily, fAuthoredPx * STAGE_SCALE * fUiScale * TEXT_BOOST, fTextScale);
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, pText);
		/* Rounded to whole pixels for the same reason the font is picked by size: a half-pixel
		origin reintroduces the sampling blur the baked variant exists to avoid. */
		const float2_t vPosition(
			std::round((fX + fWidth * fPivotX) * fScaleX - vMeasured.x * fTextScale * fPivotX),
			std::round((fY + fHeight * 0.5f) * fScaleY - vMeasured.y * fTextScale * 0.5f));
		if (Router.Is_UnderTopWindow(vPosition.x, vPosition.y))
			return;
		CGameInstance::Get().Draw_Text(strFont, pText, vPosition,
			Colors::White, 0.f, float2_t(0.f, 0.f), fTextScale);
	};

	/* Sizes are the source's own: the window title is YoonGasiIIM 18, the section labels are
	SimpleLabel size 16 and the money/cost row is ARKMoneyLabel fontSize 14. */
	DrawLabel(TITLE_ID, TITLE_TEXT, TEXT("Font_YoonGasiIIM"), 18.f, 0.5f);
	DrawLabel(EQUIP_LABEL_ID, EQUIP_TEXT, TEXT("Font_YG760"), 16.f, 0.f);
	DrawLabel(INVENTORY_LABEL_ID, INVENTORY_TEXT, TEXT("Font_YG760"), 16.f, 0.f);
	DrawLabel(EQUIP_BUTTON_ID, EQUIP_BUTTON_TEXT, TEXT("Font_YG760"), 16.f, 0.5f);
	DrawLabel(ALL_BUTTON_ID, ALL_BUTTON_TEXT, TEXT("Font_YG760"), 16.f, 0.5f);
	DrawLabel(MONEY_BAR_ID, MONEY_TEXT, TEXT("Font_YG760"), 14.f, 0.f);

	/* The two bills and the purse, right-aligned in their bars. */
	const std::wstring strEquipCost = Format_Silver(m_iEquippedCost);
	const std::wstring strAllCost = Format_Silver(m_iAllCost);
	const std::wstring strSilver = Format_Silver(CCombatHUDViewModel::Get().Get_Inventory().iSilver);
	DrawLabel(EQUIP_COST_BAR_ID, strEquipCost.c_str(), TEXT("Font_YG760"), 14.f, 1.f);
	DrawLabel(ALL_COST_BAR_ID, strAllCost.c_str(), TEXT("Font_YG760"), 14.f, 1.f);
	DrawLabel(MONEY_BAR_ID, strSilver.c_str(), TEXT("Font_YG760"), 14.f, 1.f);
}

bool_t Client::CRepairWindowView::Get_ScreenRect(
	f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const
{
	if (!Is_Open() || nullptr == m_pBackgroundView)
		return false;
	f32_t fRefX = 0.f, fRefY = 0.f, fRefWidth = 0.f, fRefHeight = 0.f;
	if (!m_pBackgroundView->Get_SlotRect(PANEL_ID, fRefX, fRefY, fRefWidth, fRefHeight))
		return false;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pBackgroundView->Get_ResolutionHeight();
	fX = fRefX * fScaleX;
	fY = fRefY * fScaleY;
	fWidth = fRefWidth * fScaleX;
	fHeight = fRefHeight * fScaleY;
	return true;
}
