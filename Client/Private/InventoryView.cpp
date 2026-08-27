#include "imgui.h"

#include "InventoryView.h"

#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "ItemCatalog.h"
#include "MainApp.h"

#include <utility>

Client::CInventoryView::CInventoryView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pBackgroundView{ std::make_unique<CHUDRuntimeView>(
		pDevice, pContext, L"UI/Inventory/InventoryUI.json") }
{
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

void Client::CInventoryView::Render(
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& items)
{
	if (!m_bOpen || nullptr == m_pBackgroundView)
		return;

	Update_Drag();

	/* Category slots are ordinary type-0 layout slots (so they show up in the Tool for
	placement), but their real on/off-hover choice is decided here, not by the generic
	pass's built-in mouse-hover swap -- force them hidden there first. */
	constexpr const char* CATEGORY_SLOT_IDS[] = {
		"Inventory_Category_All", "Inventory_Category_Combat", "Inventory_Category_Cloth",
		"Inventory_Category_Use", "Inventory_Category_Gem", "Inventory_Category_Card",
		"Inventory_Category_Etc",
	};
	for (const char* pId : CATEGORY_SLOT_IDS)
		m_pBackgroundView->Set_SlotVisible(pId, false);

	m_pBackgroundView->Render("Default", 0);

	Render_CategoryTabs();
	Render_Items(items);
}

void Client::CInventoryView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pBackgroundView)
		return;
	/* m_bOpen has no level-change reset, so without this a window left open into a loading
	transition (or Character Select) would keep drawing its title text over the loading
	screen -- same bug Render()'s caller (RenderCombatHUD) already avoids by only calling
	Render() while the player is valid. */
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
	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr == pViewport)
		return;
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;
	const ImVec2 vMouse = ImGui::GetMousePos();

	if (!m_bDraggingPanel)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect("Inventory_Title", fX, fY, fWidth, fHeight))
			return;
		const ImVec2 vMin(
			pViewport->WorkPos.x + fX * scaleX, pViewport->WorkPos.y + fY * scaleY);
		const ImVec2 vMax(
			vMin.x + fWidth * scaleX, vMin.y + fHeight * scaleY);
		const bool_t bHovered = vMouse.x >= vMin.x && vMouse.x < vMax.x &&
			vMouse.y >= vMin.y && vMouse.y < vMax.y;
		if (bHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			m_bDraggingPanel = true;
			m_fLastDragMouseX = vMouse.x;
			m_fLastDragMouseY = vMouse.y;
		}
		return;
	}

	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		m_bDraggingPanel = false;
		return;
	}

	const float fDeltaX = (vMouse.x - m_fLastDragMouseX) / scaleX;
	const float fDeltaY = (vMouse.y - m_fLastDragMouseY) / scaleY;
	m_fLastDragMouseX = vMouse.x;
	m_fLastDragMouseY = vMouse.y;
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
		const string strSlotId = "Inventory_Slot_" + std::to_string(iSlotIndex);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(strSlotId, fX, fY, fWidth, fHeight))
			break;
		m_pBackgroundView->Set_SlotPosition(strSlotId, fX + fDeltaX, fY + fDeltaY);
	}
}

void Client::CInventoryView::Render_CategoryTabs()
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

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);
	const ImVec2 vMouse = ImGui::GetMousePos();
	const bool_t bClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

	for (const CATEGORY_ART& Category : CATEGORIES)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(Category.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		const ImVec2 vMin(
			pViewport->WorkPos.x + fX * scaleX, pViewport->WorkPos.y + fY * scaleY);
		const ImVec2 vMax(
			vMin.x + fWidth * scaleX, vMin.y + fHeight * scaleY);
		const bool_t bHovered = vMouse.x >= vMin.x && vMouse.x < vMax.x &&
			vMouse.y >= vMin.y && vMouse.y < vMax.y;
		const bool_t bSelected = m_strSelectedCategoryId == Category.pSlotId;

		if (bHovered && bClicked)
		{
			CMainApp::Play_UIButtonClickSound();
			m_strSelectedCategoryId = bSelected ? string{} : Category.pSlotId;
		}

		const char* pPath = (bHovered || bSelected) ? Category.pHoverPath : Category.pNormalPath;
		if (ID3D11ShaderResourceView* pSRV = m_pBackgroundView->Load_Texture(pPath))
			pDrawList->AddImage(pSRV, vMin, vMax);
	}
}

void Client::CInventoryView::Render_Items(
	const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& items)
{
	Sync_DisplayOrder(items.size());

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const float scaleX = pViewport->WorkSize.x / 1280.f;
	const float scaleY = pViewport->WorkSize.y / 720.f;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);
	const ImVec2 vMouse = ImGui::GetMousePos();
	const bool_t bMouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
	const bool_t bMouseReleased = ImGui::IsMouseReleased(ImGuiMouseButton_Left);

	int32_t iSlotIndex = 0;
	int32_t iHoveredSlot = -1;
	for (;; ++iSlotIndex)
	{
		const string strSlotId = "Inventory_Slot_" + std::to_string(iSlotIndex);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pBackgroundView->Get_SlotRect(strSlotId, fX, fY, fWidth, fHeight))
			break;

		const ImVec2 vMin(
			pViewport->WorkPos.x + fX * scaleX, pViewport->WorkPos.y + fY * scaleY);
		const ImVec2 vMax(
			vMin.x + fWidth * scaleX, vMin.y + fHeight * scaleY);
		const bool_t bHovered = vMouse.x >= vMin.x && vMouse.x < vMax.x &&
			vMouse.y >= vMin.y && vMouse.y < vMax.y;
		if (bHovered)
			iHoveredSlot = iSlotIndex;

		if (static_cast<size_t>(iSlotIndex) >= m_DisplayOrder.size())
			continue;
		const size_t iItemIndex = m_DisplayOrder[iSlotIndex];
		if (iItemIndex >= items.size())
			continue;
		const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& Item = items[iItemIndex];
		const ITEM_DEFINITION* pDefinition = CItemCatalog::Find_ById(Item.strItemId);

		if (nullptr != pDefinition && !pDefinition->strIconPath.empty())
		{
			if (ID3D11ShaderResourceView* pIconSRV =
				m_pBackgroundView->Load_Texture(pDefinition->strIconPath))
			{
				pDrawList->AddImage(pIconSRV, vMin, vMax);
			}
		}

		const string strQuantity = std::to_string(Item.iQuantity);
		const ImVec2 vTextSize = ImGui::CalcTextSize(strQuantity.c_str());
		const ImVec2 vTextPos(
			vMax.x - vTextSize.x - 2.f, vMax.y - vTextSize.y - 2.f);
		pDrawList->AddText(ImVec2(vTextPos.x + 1.f, vTextPos.y + 1.f), IM_COL32(0, 0, 0, 220), strQuantity.c_str());
		pDrawList->AddText(vTextPos, IM_COL32(255, 255, 255, 255), strQuantity.c_str());

		if (nullptr != pDefinition && bHovered)
			ImGui::SetTooltip("%s x%u", pDefinition->strDisplayName.c_str(), Item.iQuantity);
	}

	/* Drag-and-drop: press-and-hold over a filled slot starts a drag. Releasing over a
	different inventory slot swaps the two display-order entries (local-only, see class
	comment). Releasing outside every inventory slot instead stages a Try_Consume_ItemDrop
	report -- the caller decides whether that landed on a Combat HUD quick slot. */
	if (bMouseDown && -1 == m_iDragFromSlot && iHoveredSlot >= 0 &&
		static_cast<size_t>(iHoveredSlot) < m_DisplayOrder.size() &&
		m_DisplayOrder[iHoveredSlot] < items.size())
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
			m_DisplayOrder[m_iDragFromSlot] < items.size())
		{
			m_bHasPendingItemDrop = true;
			m_strPendingDropItemId = items[m_DisplayOrder[m_iDragFromSlot]].strItemId;
			m_fPendingDropMouseX = vMouse.x;
			m_fPendingDropMouseY = vMouse.y;
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
