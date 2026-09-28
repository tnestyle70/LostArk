#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include "Network/PacketMessages.h"

#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
struct SHOP_DEFINITION;

/* Retail's NPC shop window (interactionmarket.gfx, sys.store.title), drawn as real CUI_Sprite
GameObjects from Data/UI/Shop/ShopUI.json through CUILayoutRuntime, the same path
CRepairWindowView uses.

A shop NPC opens it with its placement ID; the stock is that NPC's shop in
Data/Items/ItemCatalog.json "shops". Clicking a stock cell adds one to the basket, clicking a
basket slot takes that line out, Empty clears it and Buy hands the basket to CMainApp, which
sends C2S_BUY_ITEMS. The Server prices and applies the purchase; the purse figures shown here
are read from the replicated purse. Only buying exists: retail's buy / repurchase / sell tabs are
not built, and junk sale is not implemented. */
class CShopWindowView final
{
public:
	CShopWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CShopWindowView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	/* Opens the shop the NPC placement runs. No-op for an NPC without a shop. */
	void Open(const string& strNpcPlacementId);
	void Close();
	/* Cancel transient gestures without closing or moving the window. */
	void Cancel_Interaction() { m_bDraggingPanel = false; m_bHasPendingBuy = false; }

	/* Screen-pixel rect of the panel while open, for CMainApp's text clip-out. */
	bool_t Get_ScreenRect(f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const;

	/* No-op while closed, except that it hides every owned sprite. */
	void Update();
	/* LOA-font labels; call after CImGuiLayer::EndFrame() like the other window text. */
	void Render_Text();
	/* Forces every owned sprite invisible without touching m_bOpen. */
	void Hide();

	/* One-shot: true once after Buy was pressed on an affordable basket. The basket is
	emptied as it is handed over; the Server's inventory answer shows the result. */
	bool_t Try_Consume_BuyRequest(string& outNpcPlacementId,
		std::vector<LostArk::Shared::SHOP_BASKET_ENTRY>& outEntries);

private:
	struct BASKET_LINE
	{
		string strItemId;
		uint32_t iQuantity = 0;
		uint32_t iPrice = 0;
	};

private:
	/* Press on the title bar moves the whole window. */
	void Update_Drag();
	/* Shifts every slot this view owns. */
	void Move_Panel(f32_t fDeltaX, f32_t fDeltaY);
	/* Keeps the window inside the reference resolution after a drag. */
	void Clamp_ToScreen();
	/* Close button and the bottom buttons. */
	void Update_Buttons();
	/* Stock cells add to the basket; basket slots take a line out. */
	void Update_Stock();
	void Update_Basket();
	/* Pushes the stock and basket icons to their slots; only when either changes. */
	void Refresh_Icons();
	/* Shown and hidden per frame: empty cells and basket slots draw no icon or coin. */
	void Apply_Visibility();

	uint64_t Get_BasketTotal() const;
	/* The bag's stack of the shop's currency item. */
	uint64_t Get_PurseAmount() const;
	bool_t Is_CloseHovered() const;

private:
	unique_ptr<CUILayoutRuntime> m_pBackgroundView;

	bool_t m_bOpen = false;
	string m_strNpcPlacementId;
	const SHOP_DEFINITION* m_pShop = nullptr;
	std::vector<BASKET_LINE> m_Basket;
	bool_t m_bIconsDirty = true;

	bool_t m_bDraggingPanel = false;
	f32_t m_fLastDragMouseX = 0.f;
	f32_t m_fLastDragMouseY = 0.f;

	bool_t m_bHasPendingBuy = false;
	std::vector<LostArk::Shared::SHOP_BASKET_ENTRY> m_PendingEntries;
};

NS_END
