#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Runtime inventory panel, toggled with I. Background/title/buttons/category tabs/slot grid
draw from Data/UI/Inventory/InventoryUI.json (CHUDLayoutTool's "Inventory UI" document) as real
CUI_Sprite GameObjects (CUILayoutRuntime); item icons/quantities placed into the
Inventory_ItemIcon_N grid come from CClientReplication's real S2C_INVENTORY_SNAPSHOT, passed
into Update() each frame. Inventory_ItemIcon_N is a second CUI_Sprite per grid cell layered over
its Inventory_Slot_N background (same rect, authored alongside it) -- CUILayoutRuntime's
Set_SlotTexture replaces a slot's own single sprite outright, so the always-visible empty-slot
frame and the conditionally-visible item icon on top of it need separate slots, the same
foreground-draw-list layering the old ImGui path used.

Category tab clicks toggle a persistent "selected" visual (hover art stays on until a different
tab is clicked, matching the reference's filter-tab feel) and also filter Update_Items by each
item's Data/Items/ItemCatalog.json "category" field ("combat"/"use" today; Cloth/Gem/Card/Etc
have no items yet, so those tabs render an empty grid). All (and both tabs deselected) shows
the full unfiltered inventory.

Drag-and-drop swaps two slots' on-screen order in a local m_DisplayOrder index list; it does not
write anything back to the Server (no C2S move-item message exists yet), so a reconnect or the
next real snapshot with a different item count resets the order. Quantity numbers and the item
tooltip stay ImGui-drawn (CGameInstance has no per-frame dynamic small-text draw of its own yet),
positioned off the same Get_SlotRect rects as everything else here. */
class CInventoryView final
{
public:
	CInventoryView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CInventoryView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	void Toggle() { m_bOpen = !m_bOpen; }

	/* No-op while closed. */
	void Update(const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& items);
	/* Draws the LOA-font title text; call after CImGuiLayer::EndFrame() the same way
	RenderBossHealthBarText()/RenderCombatHUDText() do, since Draw_Text's SpriteBatch submits
	immediately and would otherwise land underneath ImGui's own foreground draw list. No-op
	while closed. */
	void Render_Text();

	/* One-shot: true exactly once, the frame an item drag released outside every
	Inventory_Slot_N (i.e. off the panel entirely -- a candidate Combat HUD quick-slot drop).
	outMouseX/Y are the screen-space release position, for the caller to hit-test against
	Item_1..4's own rects; this class has no idea those slots exist. */
	bool_t Try_Consume_ItemDrop(string& outItemId, float& outMouseX, float& outMouseY);

	/* Forces every owned CUI_Sprite invisible without touching m_bOpen -- the panel reappears on
	its own, still at whatever m_bOpen/category/position state it had, the next time Update()
	sees a level+player combination RenderCombatHUD actually calls it from. Unlike the old ImGui
	pass (which simply stopped being called and drew nothing), these slots live under
	LEVEL::STATIC and keep showing their last state across a level change unless told otherwise,
	so the caller must call this whenever it stops calling Update() for a frame (leaving a
	supported Level, the player going invalid) instead of just skipping the call. */
	void Hide();

private:
	void Update_CategoryTabs();
	void Update_Items(const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& items);
	void Sync_DisplayOrder(size_t itemCount);
	/* Indices into `items` that the selected category tab keeps, in the catalog's own order.
	Pure function of (items, m_strSelectedCategoryId) so Update_Items and Render_Text resolve
	the same slot to the same item -- a filtered view must never label a slot with an item it
	is not showing. */
	vector<size_t> Build_FilteredIndices(
		const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& items) const;
	/* Every slot this view owns, moved together while dragging the title bar. */
	void Update_Drag();

private:
	unique_ptr<CUILayoutRuntime> m_pBackgroundView;

	bool_t m_bOpen = false;
	/* "All" starts selected -- it shows the whole unfiltered inventory, which is also all
	Update_Items ever shows today (see class comment), so this is the only category whose
	selected-by-default state matches what's actually on screen. */
	string m_strSelectedCategoryId = "Inventory_Category_All";

	/* Index into the current frame's items vector, one entry per drawn slot; reordered by
	drag-and-drop. Rebuilt to identity whenever the item count changes underneath it. */
	vector<size_t> m_DisplayOrder;
	int32_t m_iDragFromSlot = -1;

	/* Panel drag (press on the title bar, move the whole window). Screen-space mouse position
	last frame, only meaningful while m_bDraggingPanel is true. */
	bool_t m_bDraggingPanel = false;
	float m_fLastDragMouseX = 0.f;
	float m_fLastDragMouseY = 0.f;

	/* Set by Update_Items on the release frame, consumed by Try_Consume_ItemDrop. */
	bool_t m_bHasPendingItemDrop = false;
	string m_strPendingDropItemId;
	float m_fPendingDropMouseX = 0.f;
	float m_fPendingDropMouseY = 0.f;
};

NS_END
