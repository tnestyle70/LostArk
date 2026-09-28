#include "ShopWindowView.h"
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
#include <cstdio>

namespace
{
	/* Retail authors this window on a 1920-wide stage; the layout reference is 1280. Matches
	Tools/LpkPipeline/build_shop_ui.py, which converted the rects. */
	constexpr f32_t STAGE_SCALE = 2.f / 3.f;
	/* Same readability boost CRepairWindowView and CHonorTitleWindowView carry. */
	constexpr f32_t TEXT_BOOST = 1.15f;

	constexpr const char* PANEL_ID = "Shop_PanelBg";
	constexpr const char* TITLE_ID = "Shop_Title";
	constexpr const char* CLOSE_ID = "Shop_CloseBtn";
	constexpr const char* BUY_AMOUNT_ID = "Shop_BuyAmount";
	constexpr const char* BALANCE_ID = "Shop_Balance";
	constexpr const char* JUNK_BUTTON_ID = "Shop_JunkBtn";
	constexpr const char* BUY_BUTTON_ID = "Shop_BuyBtn";
	constexpr const char* EMPTY_BUTTON_ID = "Shop_EmptyBtn";

	constexpr uint32_t CELL_COUNT = 10;
	constexpr uint32_t BASKET_COUNT = 10;

	/* Text anchors: retail draws no plate behind the title or the two money lines. */
	constexpr const char* TEXT_ANCHOR_IDS[] = { TITLE_ID, BUY_AMOUNT_ID, BALANCE_ID };

	/* Strings are EFTable_GameMsg sys.store.*; escapes because this file is compiled as ASCII:
	title, fla_button_junk/buy/empty_basket, fla_buy_amount, fla_buy_amount_balance. */
	const wchar_t* TITLE_TEXT = L"\xC0C1\xC810";
	const wchar_t* JUNK_TEXT = L"\xC7A1\xB3D9\xC0AC\xB2C8";
	const wchar_t* BUY_TEXT = L"\xAD6C\xB9E4";
	const wchar_t* EMPTY_TEXT = L"\xBE44\xC6B0\xAE30";
	const wchar_t* BUY_AMOUNT_TEXT = L"\xAD6C\xB9E4 \xAE08\xC561";
	const wchar_t* BALANCE_TEXT = L"\xAD6C\xB9E4 \xD6C4 \xC794\xC561";

	/* Item grade colours as GameMsg writes them (same table as Level_ValtanArena's
	Item_GradeRgb): rare #00B5FF, uncommon #91FE02 and so on; white for no grade. */
	float4_t Grade_Color(const string& strGrade)
	{
		uint32_t iRgb = 0xffffffu;
		if ("ancient" == strGrade) iRgb = 0xe3c7a1u;
		else if ("relic" == strGrade) iRgb = 0xff6000u;
		else if ("legend" == strGrade) iRgb = 0xfe9600u;
		else if ("epic" == strGrade) iRgb = 0xce43fcu;
		else if ("rare" == strGrade) iRgb = 0x00b5ffu;
		else if ("uncommon" == strGrade) iRgb = 0x91fe02u;
		return float4_t(((iRgb >> 16) & 0xffu) / 255.f, ((iRgb >> 8) & 0xffu) / 255.f,
			(iRgb & 0xffu) / 255.f, 1.f);
	}

	/* Retail's shortage red (#C24B46, sys.store.buy_count_over). */
	const float4_t COLOR_SHORTAGE(194.f / 255.f, 75.f / 255.f, 70.f / 255.f, 1.f);
	const float4_t COLOR_WHITE(1.f, 1.f, 1.f, 1.f);

	string Make_Id(const char* pPrefix, const uint32_t iIndex)
	{
		char szId[48] = {};
		std::snprintf(szId, sizeof(szId), "%s%u", pPrefix, iIndex);
		return szId;
	}

	/* Same conversion MainApp's ConvertUtf8ToWide does for catalog display names. */
	std::wstring Utf8_ToWide(const string& strUtf8)
	{
		if (strUtf8.empty())
			return {};
		const int iLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), nullptr, 0);
		if (iLength <= 0)
			return {};
		std::wstring strWide(static_cast<size_t>(iLength), L'\0');
		if (iLength != MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), strWide.data(), iLength))
			return {};
		return strWide;
	}

	/* 1234567 -> "1,234,567", the way ARKMoneyLabel groups figures. */
	std::wstring Format_Money(const int64_t iValue)
	{
		const uint64_t iMagnitude = iValue < 0 ? static_cast<uint64_t>(-iValue) : static_cast<uint64_t>(iValue);
		std::wstring strDigits = std::to_wstring(iMagnitude);
		for (int32_t iAt = static_cast<int32_t>(strDigits.size()) - 3; iAt > 0; iAt -= 3)
			strDigits.insert(static_cast<size_t>(iAt), L",");
		return iValue < 0 ? L"-" + strDigits : strDigits;
	}
}

Client::CShopWindowView::CShopWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pBackgroundView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Shop/ShopUI.json") }
{
	m_pBackgroundView->Set_UISortLayer(UI_TEXT_LAYER::WINDOW_SHOP);
	Hide();
}

Client::CShopWindowView::~CShopWindowView()
{
}

void Client::CShopWindowView::Open(const string& strNpcPlacementId)
{
	const SHOP_DEFINITION* pShop = CItemCatalog::Find_ShopByNpc(strNpcPlacementId);
	if (nullptr == pShop)
		return;
	if (pShop != m_pShop)
		m_Basket.clear();
	m_pShop = pShop;
	m_strNpcPlacementId = strNpcPlacementId;
	m_bIconsDirty = true;
	m_bOpen = true;
}

void Client::CShopWindowView::Close()
{
	m_bOpen = false;
	m_bDraggingPanel = false;
	m_Basket.clear();
	m_bIconsDirty = true;
}

void Client::CShopWindowView::Hide()
{
	if (nullptr != m_pBackgroundView)
		m_pBackgroundView->Set_AllSlotsVisible(false);
}

void Client::CShopWindowView::Update()
{
	CUIPointerScope PointerScope(this);
	if (nullptr == m_pBackgroundView)
		return;
	if (!m_bOpen || nullptr == m_pShop)
	{
		Hide();
		return;
	}

	Update_Drag();
	Update_Buttons();
	if (!m_bOpen)
	{
		Hide();
		return;
	}
	Update_Stock();
	Update_Basket();
	Refresh_Icons();
	Apply_Visibility();

	f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelWidth = 0.f, fPanelHeight = 0.f;
	if (m_pBackgroundView->Get_SlotRect(PANEL_ID, fPanelX, fPanelY, fPanelWidth, fPanelHeight) &&
		CUIInputRouter::Get().Is_Hovered(fPanelX, fPanelY, fPanelWidth, fPanelHeight,
			m_pBackgroundView->Get_ResolutionWidth(), m_pBackgroundView->Get_ResolutionHeight()))
	{
		CUIInputRouter::Get().Claim_Mouse_This_Frame();
	}
}

bool_t Client::CShopWindowView::Is_CloseHovered() const
{
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	return m_pBackgroundView->Get_SlotRect(CLOSE_ID, fX, fY, fWidth, fHeight) &&
		CUIInputRouter::Get().Is_Hovered(fX, fY, fWidth, fHeight,
			m_pBackgroundView->Get_ResolutionWidth(), m_pBackgroundView->Get_ResolutionHeight());
}

void Client::CShopWindowView::Update_Drag()
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
		if (Is_CloseHovered())
			return;
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
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

void Client::CShopWindowView::Move_Panel(const f32_t fDeltaX, const f32_t fDeltaY)
{
	for (const string& strId : m_pBackgroundView->Get_SlotIds())
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pBackgroundView->Get_SlotRect(strId, fX, fY, fWidth, fHeight))
			m_pBackgroundView->Set_SlotPosition(strId, fX + fDeltaX, fY + fDeltaY);
	}
}

void Client::CShopWindowView::Clamp_ToScreen()
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

void Client::CShopWindowView::Update_Buttons()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();

	const auto HitTest = [&](const string& strId, bool_t& bHovered) -> bool_t
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		bHovered = false;
		if (!m_pBackgroundView->Get_SlotRect(strId, fX, fY, fWidth, fHeight))
			return false;
		bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		return Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
	};
	const float4_t vNormal(1.f, 1.f, 1.f, 1.f);
	const float4_t vHover(1.25f, 1.25f, 1.25f, 1.f);
	const float4_t vDimmed(0.6f, 0.6f, 0.6f, 1.f);

	bool_t bCloseHovered = false;
	const bool_t bCloseClicked = HitTest(CLOSE_ID, bCloseHovered);
	m_pBackgroundView->Set_SlotTintMultiplier(CLOSE_ID, bCloseHovered ?
		float4_t(1.4f, 1.4f, 1.4f, 1.f) : vNormal);
	if (bCloseClicked)
	{
		CMainApp::Play_UIButtonClickSound();
		Close();
		return;
	}

	/* Buy is live only for a basket the purse covers; the Server re-checks everything. */
	const bool_t bCanBuy = !m_Basket.empty() && Get_BasketTotal() <= Get_PurseAmount();
	bool_t bHovered = false;
	const bool_t bBuyClicked = HitTest(BUY_BUTTON_ID, bHovered);
	m_pBackgroundView->Set_SlotTintMultiplier(BUY_BUTTON_ID,
		!bCanBuy ? vDimmed : (bHovered ? vHover : vNormal));
	if (bBuyClicked && bCanBuy)
	{
		CMainApp::Play_UIButtonClickSound();
		m_PendingEntries.clear();
		for (const BASKET_LINE& Line : m_Basket)
			m_PendingEntries.push_back({ Line.strItemId, Line.iQuantity });
		m_bHasPendingBuy = true;
		m_Basket.clear();
		m_bIconsDirty = true;
	}

	const bool_t bEmptyClicked = HitTest(EMPTY_BUTTON_ID, bHovered);
	m_pBackgroundView->Set_SlotTintMultiplier(EMPTY_BUTTON_ID, bHovered ? vHover : vNormal);
	if (bEmptyClicked && !m_Basket.empty())
	{
		CMainApp::Play_UIButtonClickSound();
		m_Basket.clear();
		m_bIconsDirty = true;
	}

	/* Junk sale is not implemented, so the button only shows it is not live. */
	(void)HitTest(JUNK_BUTTON_ID, bHovered);
	m_pBackgroundView->Set_SlotTintMultiplier(JUNK_BUTTON_ID, vDimmed);
}

void Client::CShopWindowView::Update_Stock()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();
	const size_t iStockCount = (std::min)(m_pShop->Items.size(), static_cast<size_t>(CELL_COUNT));

	for (uint32_t iCell = 0; iCell < iStockCount; ++iCell)
	{
		const string strCellId = Make_Id("Shop_Cell_", iCell);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(strCellId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		m_pBackgroundView->Set_SlotTintMultiplier(strCellId, bHovered ?
			float4_t(1.25f, 1.25f, 1.25f, 1.f) : float4_t(1.f, 1.f, 1.f, 1.f));
		if (!Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			continue;

		/* One more of this item: an existing line grows, otherwise a free slot takes it. */
		const SHOP_ITEM_DEFINITION& Stock = m_pShop->Items[iCell];
		auto Line = std::find_if(m_Basket.begin(), m_Basket.end(),
			[&Stock](const BASKET_LINE& Candidate) { return Candidate.strItemId == Stock.strItemId; });
		if (m_Basket.end() != Line)
		{
			if (Line->iQuantity < LostArk::Shared::MAX_SHOP_BASKET_QUANTITY)
				++Line->iQuantity;
		}
		else if (m_Basket.size() < BASKET_COUNT)
		{
			m_Basket.push_back({ Stock.strItemId, 1u, Stock.iPrice });
			m_bIconsDirty = true;
		}
		CMainApp::Play_UIButtonClickSound();
	}
}

void Client::CShopWindowView::Update_Basket()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();
	for (uint32_t iSlot = 0; iSlot < m_Basket.size(); ++iSlot)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(Make_Id("Shop_BasketSlot_", iSlot), fX, fY, fWidth, fHeight) ||
			!Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			continue;
		m_Basket.erase(m_Basket.begin() + iSlot);
		m_bIconsDirty = true;
		CMainApp::Play_UIButtonClickSound();
		break;
	}
}

void Client::CShopWindowView::Refresh_Icons()
{
	if (!m_bIconsDirty)
		return;
	m_bIconsDirty = false;
	const size_t iStockCount = (std::min)(m_pShop->Items.size(), static_cast<size_t>(CELL_COUNT));
	for (uint32_t iCell = 0; iCell < iStockCount; ++iCell)
	{
		const ITEM_DEFINITION* pItem = CItemCatalog::Find_ById(m_pShop->Items[iCell].strItemId);
		if (nullptr != pItem && !pItem->strIconPath.empty())
			m_pBackgroundView->Set_SlotTexture(Make_Id("Shop_CellIcon_", iCell), pItem->strIconPath);
	}
	for (uint32_t iSlot = 0; iSlot < m_Basket.size(); ++iSlot)
	{
		const ITEM_DEFINITION* pItem = CItemCatalog::Find_ById(m_Basket[iSlot].strItemId);
		if (nullptr != pItem && !pItem->strIconPath.empty())
			m_pBackgroundView->Set_SlotTexture(Make_Id("Shop_BasketIcon_", iSlot), pItem->strIconPath);
	}
}

void Client::CShopWindowView::Apply_Visibility()
{
	m_pBackgroundView->Set_AllSlotsVisible(true);
	for (const char* pAnchorId : TEXT_ANCHOR_IDS)
		m_pBackgroundView->Set_SlotVisible(pAnchorId, false);
	const size_t iShownStock = (std::min)(m_pShop->Items.size(), static_cast<size_t>(CELL_COUNT));
	for (uint32_t iCell = static_cast<uint32_t>(iShownStock); iCell < CELL_COUNT; ++iCell)
	{
		m_pBackgroundView->Set_SlotVisible(Make_Id("Shop_CellIcon_", iCell), false);
		m_pBackgroundView->Set_SlotVisible(Make_Id("Shop_CellCoin_", iCell), false);
	}
	for (uint32_t iSlot = static_cast<uint32_t>(m_Basket.size()); iSlot < BASKET_COUNT; ++iSlot)
		m_pBackgroundView->Set_SlotVisible(Make_Id("Shop_BasketIcon_", iSlot), false);
}

uint64_t Client::CShopWindowView::Get_BasketTotal() const
{
	uint64_t iTotal = 0;
	for (const BASKET_LINE& Line : m_Basket)
		iTotal += static_cast<uint64_t>(Line.iPrice) * Line.iQuantity;
	return iTotal;
}

uint64_t Client::CShopWindowView::Get_PurseAmount() const
{
	if (nullptr == m_pShop || m_pShop->Items.empty())
		return 0;
	/* Every line of the current shops is priced in the same currency. */
	const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& Purse = CCombatHUDViewModel::Get().Get_Inventory();
	return "GOLD" == m_pShop->Items.front().strCurrencyId ? Purse.iGold : Purse.iSilver;
}

bool_t Client::CShopWindowView::Try_Consume_BuyRequest(string& outNpcPlacementId,
	std::vector<LostArk::Shared::SHOP_BASKET_ENTRY>& outEntries)
{
	if (!m_bHasPendingBuy)
		return false;
	m_bHasPendingBuy = false;
	outNpcPlacementId = m_strNpcPlacementId;
	outEntries = std::move(m_PendingEntries);
	m_PendingEntries.clear();
	return true;
}

void Client::CShopWindowView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pBackgroundView || nullptr == m_pShop)
		return;
	if (!CCombatHUDViewModel::Get().Get_Player().isValid)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewportSize.x / 1280.f;
	const f32_t fScaleY = vViewportSize.y / 720.f;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	CUIInputRouter& Router = CUIInputRouter::Get();

	/* Draws one label inside a reference-space box: fPivotX 0 left, 0.5 centre, 1 right;
	vertically centred. Rounded to whole pixels so the baked UILabelFont stays sharp. */
	const auto DrawInBox = [&](const f32_t fX, const f32_t fY, const f32_t fWidth,
		const f32_t fHeight, const wchar_t* pText, const wstring_t& strFamily,
		const f32_t fAuthoredPx, const f32_t fPivotX, const float4_t& vColor)
	{
		f32_t fTextScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(
			strFamily, fAuthoredPx * STAGE_SCALE * fUiScale * TEXT_BOOST, fTextScale);
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, pText);
		const float2_t vPosition(
			std::round((fX + fWidth * fPivotX) * fScaleX - vMeasured.x * fTextScale * fPivotX),
			std::round((fY + fHeight * 0.5f) * fScaleY - vMeasured.y * fTextScale * 0.5f));
		if (Router.Is_UnderTopWindow(vPosition.x, vPosition.y))
			return;
		CGameInstance::Get().Draw_Text(strFont, pText, vPosition,
			XMLoadFloat4(&vColor), 0.f, float2_t(0.f, 0.f), fTextScale);
	};
	const auto DrawLabel = [&](const string& strSlotId, const wchar_t* pText,
		const wstring_t& strFamily, const f32_t fAuthoredPx, const f32_t fPivotX,
		const float4_t& vColor)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pBackgroundView->Get_SlotRect(strSlotId, fX, fY, fWidth, fHeight))
			DrawInBox(fX, fY, fWidth, fHeight, pText, strFamily, fAuthoredPx, fPivotX, vColor);
	};

	/* Sizes are the source's own: title YoonGasiIIM 18, buttons 16, list name and
	price YG760 14, money lines YG760 14. */
	DrawLabel(TITLE_ID, TITLE_TEXT, TEXT("Font_YoonGasiIIM"), 18.f, 0.5f, COLOR_WHITE);

	/* MarketListItem: nameTF at (56,5) 194x23, moneyTF right-aligned at (179,54) 140x22 so
	the figure ends against the coin at x 316. */
	const size_t iShownStock = (std::min)(m_pShop->Items.size(), static_cast<size_t>(CELL_COUNT));
	for (uint32_t iCell = 0; iCell < iShownStock; ++iCell)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(Make_Id("Shop_Cell_", iCell), fX, fY, fWidth, fHeight))
			continue;
		const SHOP_ITEM_DEFINITION& Stock = m_pShop->Items[iCell];
		const ITEM_DEFINITION* pItem = CItemCatalog::Find_ById(Stock.strItemId);
		if (nullptr != pItem)
		{
			const std::wstring strName = Utf8_ToWide(pItem->strDisplayName);
			DrawInBox(fX + 56.f * STAGE_SCALE, fY + 5.f * STAGE_SCALE, 194.f * STAGE_SCALE,
				23.f * STAGE_SCALE, strName.c_str(), TEXT("Font_YG760"), 14.f, 0.f,
				Grade_Color(pItem->strGrade));
		}
		const std::wstring strPrice = Format_Money(Stock.iPrice);
		DrawInBox(fX + 179.f * STAGE_SCALE, fY + 54.f * STAGE_SCALE, 134.f * STAGE_SCALE,
			22.f * STAGE_SCALE, strPrice.c_str(), TEXT("Font_YG760"), 14.f, 1.f, COLOR_WHITE);
	}

	/* Basket counts in the slot's lower right, like the inventory's stack count. */
	for (uint32_t iSlot = 0; iSlot < m_Basket.size(); ++iSlot)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(Make_Id("Shop_BasketSlot_", iSlot), fX, fY, fWidth, fHeight))
			continue;
		const std::wstring strCount = std::to_wstring(m_Basket[iSlot].iQuantity);
		DrawInBox(fX, fY + fHeight * 0.55f, fWidth - 3.f, fHeight * 0.45f, strCount.c_str(),
			TEXT("Font_YG760"), 12.f, 1.f, COLOR_WHITE);
	}

	/* Money lines: the label on the left, the figure right-aligned against its coin. */
	const uint64_t iTotal = Get_BasketTotal();
	const int64_t iBalance = static_cast<int64_t>(Get_PurseAmount()) - static_cast<int64_t>(iTotal);
	DrawLabel(BUY_AMOUNT_ID, BUY_AMOUNT_TEXT, TEXT("Font_YG760"), 14.f, 0.f, COLOR_WHITE);
	DrawLabel(BALANCE_ID, BALANCE_TEXT, TEXT("Font_YG760"), 14.f, 0.f, COLOR_WHITE);
	const std::wstring strTotal = Format_Money(static_cast<int64_t>(iTotal));
	const std::wstring strBalance = Format_Money(iBalance);
	DrawLabel(BUY_AMOUNT_ID, strTotal.c_str(), TEXT("Font_YG760"), 14.f, 1.f, COLOR_WHITE);
	DrawLabel(BALANCE_ID, strBalance.c_str(), TEXT("Font_YG760"), 14.f, 1.f,
		iBalance < 0 ? COLOR_SHORTAGE : COLOR_WHITE);

	DrawLabel(JUNK_BUTTON_ID, JUNK_TEXT, TEXT("Font_YG760"), 16.f, 0.5f, float4_t(0.6f, 0.6f, 0.6f, 1.f));
	DrawLabel(BUY_BUTTON_ID, BUY_TEXT, TEXT("Font_YG760"), 16.f, 0.5f, COLOR_WHITE);
	DrawLabel(EMPTY_BUTTON_ID, EMPTY_TEXT, TEXT("Font_YG760"), 16.f, 0.5f, COLOR_WHITE);
}

bool_t Client::CShopWindowView::Get_ScreenRect(
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
