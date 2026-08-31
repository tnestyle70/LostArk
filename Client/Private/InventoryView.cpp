#include "imgui.h"

#include "InventoryView.h"

#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "ItemCatalog.h"
#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <utility>

Client::CInventoryView::CInventoryView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pBackgroundView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Inventory/InventoryUI.json") }
{
	/* Authored layer tint is opaque ([1,1,1,1]) -- every slot would otherwise sit fully visible
	on screen (LEVEL::STATIC's GameObjects persist across every Level, including Lobby/Loading)
	from construction until whatever first calls Update() while m_bOpen is still false. */
	Hide();
}

Client::CInventoryView::~CInventoryView()
{
}

void Client::CInventoryView::Sync_DisplayOrder(const size_t itemCount)
{
	if (m_DisplayOrder.size() == itemCount)
		return;
	m_DisplayOrder.resize(itemCount);
	for (size_t i = 0; i < itemCount; ++i)
		m_DisplayOrder[i] = i;
}

void Client::CInventoryView::Hide()
{
	m_pBackgroundView->Set_SlotVisible("Inventory_PanelBg", false);
	m_pBackgroundView->Set_SlotVisible("Inventory_Title", false);
	m_pBackgroundView->Set_SlotVisible("Inventory_Button1", false);
	m_pBackgroundView->Set_SlotVisible("Inventory_CraftingButton", false);
	m_pBackgroundView->Set_SlotVisible("Inventory_GemButton", false);
	m_pBackgroundView->Set_SlotVisible("Inventory_BottomBars", false);
	constexpr const char* CATEGORY_SLOT_IDS[] = {
		"Inventory_Category_All", "Inventory_Category_Combat", "Inventory_Category_Cloth",
		"Inventory_Category_Use", "Inventory_Category_Gem", "Inventory_Category_Card",
		"Inventory_Category_Etc",
	};
	for (const char* pId : CATEGORY_SLOT_IDS)
		m_pBackgroundView->Set_SlotVisible(pId, false);
	for (int32_t iSlotIndex = 0;; ++iSlotIndex)
	{
		const string strBgId = "Inventory_Slot_" + std::to_string(iSlotIndex);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(strBgId, fX, fY, fWidth, fHeight))
			break;
		m_pBackgroundView->Set_SlotVisible(strBgId, false);
		m_pBackgroundView->Set_SlotVisible(
			"Inventory_ItemIcon_" + std::to_string(iSlotIndex), false);
	}
}

void Client::CInventoryView::Update(
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& items)
{
	if (nullptr == m_pBackgroundView)
		return;
	if (!m_bOpen)
	{
		/* Every slot self-renders now (no generic Render(class, revision) gate to fall back on
		like the old ImGui pass had) -- closing the panel has to hide it explicitly instead of
		simply not being drawn this frame. */
		Hide();
		return;
	}

	m_pBackgroundView->Set_SlotVisible("Inventory_PanelBg", true);
	m_pBackgroundView->Set_SlotVisible("Inventory_Title", true);
	m_pBackgroundView->Set_SlotVisible("Inventory_Button1", true);
	m_pBackgroundView->Set_SlotVisible("Inventory_CraftingButton", true);
	m_pBackgroundView->Set_SlotVisible("Inventory_GemButton", true);
	m_pBackgroundView->Set_SlotVisible("Inventory_BottomBars", true);
	for (int32_t iSlotIndex = 0;; ++iSlotIndex)
	{
		const string strBgId = "Inventory_Slot_" + std::to_string(iSlotIndex);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(strBgId, fX, fY, fWidth, fHeight))
			break;
		m_pBackgroundView->Set_SlotVisible(strBgId, true);
	}

	Update_Drag();
	Update_CategoryTabs();
	Update_Items(items);
}

void Client::CInventoryView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pBackgroundView)
		return;
	/* m_bOpen has no level-change reset, so without this a window left open into a loading
	transition (or Character Select) would keep drawing its title text over the loading
	screen -- same bug Update()'s caller (RenderCombatHUD) already avoids by only calling
	Update() while the player is valid. */
	if (!CCombatHUDViewModel::Get().Get_Player().isValid)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleWidth = 0.f, fTitleHeight = 0.f;
	if (!m_pBackgroundView->Get_SlotRect(
		"Inventory_Title", fTitleX, fTitleY, fTitleWidth, fTitleHeight))
	{
		return;
	}

	const wstring strTitle = L"\xC18C\xC9C0\xD488"; // "소지품"
	/* Auto-scaled from the slot's own box height (measured at scale=1) -- the reference shows
	the title text at roughly 45% of the title bar's own height, not filling it, so that's the
	fraction here instead of 100%. */
	const float2_t vMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), strTitle.c_str());
	constexpr f32_t TITLE_HEIGHT_FRACTION = 0.45f;
	const f32_t fScale = (vMeasured.y > 0.f) ?
		(fTitleHeight * TITLE_HEIGHT_FRACTION / vMeasured.y) : 1.f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), strTitle.c_str(),
		float2_t(
			(fTitleX + fTitleWidth * 0.5f) * textScaleX,
			(fTitleY + fTitleHeight * 0.5f) * textScaleY),
		Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
}

void Client::CInventoryView::Update_Drag()
{
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();
	CUIInputRouter& Router = CUIInputRouter::Get();

	f32_t fMouseX = 0.f, fMouseY = 0.f;
	if (!Router.Get_MousePosition(fRefWidth, fRefHeight, fMouseX, fMouseY))
		return;

	if (!m_bDraggingPanel)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect("Inventory_Title", fX, fY, fWidth, fHeight))
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

	constexpr const char* STATIC_SLOT_IDS[] = {
		"Inventory_PanelBg", "Inventory_Title", "Inventory_Button1",
		"Inventory_CraftingButton", "Inventory_GemButton", "Inventory_BottomBars",
		"Inventory_Category_All", "Inventory_Category_Combat", "Inventory_Category_Cloth",
		"Inventory_Category_Use", "Inventory_Category_Gem", "Inventory_Category_Card",
		"Inventory_Category_Etc",
	};
	for (const char* pId : STATIC_SLOT_IDS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pBackgroundView->Get_SlotRect(pId, fX, fY, fWidth, fHeight))
			m_pBackgroundView->Set_SlotPosition(pId, fX + fDeltaX, fY + fDeltaY);
	}
	for (int32_t iSlotIndex = 0;; ++iSlotIndex)
	{
		const string strBgId = "Inventory_Slot_" + std::to_string(iSlotIndex);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(strBgId, fX, fY, fWidth, fHeight))
			break;
		m_pBackgroundView->Set_SlotPosition(strBgId, fX + fDeltaX, fY + fDeltaY);

		const string strIconId = "Inventory_ItemIcon_" + std::to_string(iSlotIndex);
		f32_t fIconX = 0.f, fIconY = 0.f, fIconWidth = 0.f, fIconHeight = 0.f;
		if (m_pBackgroundView->Get_SlotRect(strIconId, fIconX, fIconY, fIconWidth, fIconHeight))
			m_pBackgroundView->Set_SlotPosition(strIconId, fIconX + fDeltaX, fIconY + fDeltaY);
	}
}

void Client::CInventoryView::Update_CategoryTabs()
{
	struct CATEGORY_ART { const char* pSlotId; const char* pNormalPath; const char* pHoverPath; };
	constexpr CATEGORY_ART CATEGORIES[] = {
		{ "Inventory_Category_All", "UI/Inventory/Category All.png", "UI/Inventory/Category all_hover.png" },
		{ "Inventory_Category_Combat", "UI/Inventory/Category combat.png", "UI/Inventory/Category combat_hover.png" },
		{ "Inventory_Category_Cloth", "UI/Inventory/Category Cloth.png", "UI/Inventory/Category Cloth_hover .png" },
		{ "Inventory_Category_Use", "UI/Inventory/Category use.png", "UI/Inventory/Category use_hover.png" },
		{ "Inventory_Category_Gem", "UI/Inventory/Category Gem.png", "UI/Inventory/Category Gem_hover.png" },
		{ "Inventory_Category_Card", "UI/Inventory/Category Card.png", "UI/Inventory/Category card_hover.png" },
		{ "Inventory_Category_Etc", "UI/Inventory/Category etc.png", "UI/Inventory/Category etc_hover.png" },
	};

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();

	for (const CATEGORY_ART& Category : CATEGORIES)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(Category.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		const bool_t bSelected = m_strSelectedCategoryId == Category.pSlotId;

		if (bHovered)
		{
			Router.Claim_Mouse_This_Frame();
			if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			{
				CMainApp::Play_UIButtonClickSound();
				m_strSelectedCategoryId = bSelected ? string{} : Category.pSlotId;
			}
		}

		const char* pPath = (bHovered || bSelected) ? Category.pHoverPath : Category.pNormalPath;
		m_pBackgroundView->Set_SlotTexture(Category.pSlotId, pPath);
	}
}

void Client::CInventoryView::Update_Items(
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& items)
{
	/* "" (both tabs just deselected) and Inventory_Category_All show everything. Every other
	tab keeps only items whose catalog category maps to it; a tab with no matching items yet
	(Cloth/Gem/Card/Etc) simply renders an empty grid rather than falling back to unfiltered. */
	const bool_t bShowAll = m_strSelectedCategoryId.empty() ||
		m_strSelectedCategoryId == "Inventory_Category_All";
	string strCategoryFilter;
	if (!bShowAll)
	{
		static const std::pair<const char*, const char*> CATEGORY_BY_SLOT[] = {
			{ "Inventory_Category_Combat", "combat" }, { "Inventory_Category_Cloth", "cloth" },
			{ "Inventory_Category_Use", "use" }, { "Inventory_Category_Gem", "gem" },
			{ "Inventory_Category_Card", "card" }, { "Inventory_Category_Etc", "etc" },
		};
		for (const auto& [pSlotId, pCategory] : CATEGORY_BY_SLOT)
		{
			if (m_strSelectedCategoryId == pSlotId)
			{
				strCategoryFilter = pCategory;
				break;
			}
		}
	}

	vector<size_t> filteredIndices;
	filteredIndices.reserve(items.size());
	for (size_t i = 0; i < items.size(); ++i)
	{
		if (bShowAll)
		{
			filteredIndices.push_back(i);
			continue;
		}
		const ITEM_DEFINITION* pFilterDefinition = CItemCatalog::Find_ById(items[i].strItemId);
		if (nullptr != pFilterDefinition && pFilterDefinition->strCategory == strCategoryFilter)
			filteredIndices.push_back(i);
	}

	Sync_DisplayOrder(filteredIndices.size());

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pBackgroundView->Get_ResolutionHeight();

	/* Quantity numbers and the hover tooltip stay on ImGui's own foreground draw list (see class
	comment) -- both need real screen-pixel rects, unlike the CUI_Sprite slots driven above. */
	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	const bool_t bMouseDown = Router.Is_LeftDown();
	const bool_t bMouseReleased = Router.Is_LeftReleaseEdge();

	int32_t iSlotIndex = 0;
	int32_t iHoveredSlot = -1;
	for (;; ++iSlotIndex)
	{
		const string strBgId = "Inventory_Slot_" + std::to_string(iSlotIndex);
		const string strIconId = "Inventory_ItemIcon_" + std::to_string(iSlotIndex);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(strBgId, fX, fY, fWidth, fHeight))
			break;

		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		if (bHovered)
			iHoveredSlot = iSlotIndex;

		const bool_t bHasDisplayEntry = static_cast<size_t>(iSlotIndex) < m_DisplayOrder.size();
		const size_t iFilteredIndex = bHasDisplayEntry ? m_DisplayOrder[iSlotIndex] : filteredIndices.size();
		const bool_t bHasItem = bHasDisplayEntry && iFilteredIndex < filteredIndices.size() &&
			filteredIndices[iFilteredIndex] < items.size();
		if (!bHasItem)
		{
			m_pBackgroundView->Set_SlotVisible(strIconId, false);
			continue;
		}

		const size_t iItemIndex = filteredIndices[iFilteredIndex];
		const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& Item = items[iItemIndex];
		const ITEM_DEFINITION* pDefinition = CItemCatalog::Find_ById(Item.strItemId);

		if (nullptr != pDefinition && !pDefinition->strIconPath.empty())
		{
			m_pBackgroundView->Set_SlotTexture(strIconId, pDefinition->strIconPath);
			m_pBackgroundView->Set_SlotVisible(strIconId, true);
		}
		else
		{
			m_pBackgroundView->Set_SlotVisible(strIconId, false);
		}

		const ImVec2 vMin(
			pViewport->WorkPos.x + fX * scaleX, pViewport->WorkPos.y + fY * scaleY);
		const ImVec2 vMax(
			vMin.x + fWidth * scaleX, vMin.y + fHeight * scaleY);

		/* Equipment ("combat") never stacks past 1 -- showing a quantity number on it just reads
		as clutter, not information, so only non-equipment (stackable) items draw one. */
		if (nullptr == pDefinition || "combat" != pDefinition->strCategory)
		{
			const string strQuantity = std::to_string(Item.iQuantity);
			const ImVec2 vTextSize = ImGui::CalcTextSize(strQuantity.c_str());
			const ImVec2 vTextPos(
				vMax.x - vTextSize.x - 2.f, vMax.y - vTextSize.y - 2.f);
			pDrawList->AddText(ImVec2(vTextPos.x + 1.f, vTextPos.y + 1.f), IM_COL32(0, 0, 0, 220), strQuantity.c_str());
			pDrawList->AddText(vTextPos, IM_COL32(255, 255, 255, 255), strQuantity.c_str());
		}

		if (nullptr != pDefinition && bHovered)
			ImGui::SetTooltip("%s x%u", pDefinition->strDisplayName.c_str(), Item.iQuantity);
	}

	/* Drag-and-drop: press-and-hold over a filled slot starts a drag. Releasing over a
	different inventory slot swaps the two display-order entries (local-only, see class
	comment). Releasing outside every inventory slot instead stages a Try_Consume_ItemDrop
	report -- the caller decides whether that landed on a Combat HUD quick slot. */
	if (bMouseDown && -1 == m_iDragFromSlot && iHoveredSlot >= 0 &&
		static_cast<size_t>(iHoveredSlot) < m_DisplayOrder.size() &&
		m_DisplayOrder[iHoveredSlot] < filteredIndices.size())
	{
		m_iDragFromSlot = iHoveredSlot;
	}
	if (bMouseReleased && m_iDragFromSlot >= 0)
	{
		if (iHoveredSlot >= 0 && iHoveredSlot != m_iDragFromSlot &&
			static_cast<size_t>(iHoveredSlot) < m_DisplayOrder.size() &&
			static_cast<size_t>(m_iDragFromSlot) < m_DisplayOrder.size())
		{
			std::swap(m_DisplayOrder[iHoveredSlot], m_DisplayOrder[m_iDragFromSlot]);
		}
		else if (-1 == iHoveredSlot &&
			static_cast<size_t>(m_iDragFromSlot) < m_DisplayOrder.size() &&
			m_DisplayOrder[m_iDragFromSlot] < filteredIndices.size() &&
			filteredIndices[m_DisplayOrder[m_iDragFromSlot]] < items.size())
		{
			f32_t fDropScreenX = 0.f, fDropScreenY = 0.f;
			if (Router.Get_ClientCursorPosition(fDropScreenX, fDropScreenY))
			{
				m_bHasPendingItemDrop = true;
				m_strPendingDropItemId =
					items[filteredIndices[m_DisplayOrder[m_iDragFromSlot]]].strItemId;
				m_fPendingDropMouseX = fDropScreenX;
				m_fPendingDropMouseY = fDropScreenY;
			}
		}
		m_iDragFromSlot = -1;
	}
}

bool_t Client::CInventoryView::Try_Consume_ItemDrop(
	string& outItemId, float& outMouseX, float& outMouseY)
{
	if (!m_bHasPendingItemDrop)
		return false;
	outItemId = m_strPendingDropItemId;
	outMouseX = m_fPendingDropMouseX;
	outMouseY = m_fPendingDropMouseY;
	m_bHasPendingItemDrop = false;
	m_strPendingDropItemId.clear();
	return true;
}
