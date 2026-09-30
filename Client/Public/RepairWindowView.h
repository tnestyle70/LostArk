#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Retail's NPC item repair window (interactionrepair.gfx, "아이템 수리"), drawn as real
CUI_Sprite GameObjects from Data/UI/Repair/RepairUI.json through CUILayoutRuntime, the same
path CInventoryView uses.

The equipped 8x2 and bag 8x3 slot frames list the damaged gear only: each slot shows the
piece's icon (a listed piece is a damaged one; no percent is drawn). The two cost rows show what the Server
will bill for repairing the worn pieces and for every damaged piece, and the silver row shows the
purse. A repair button is live only when there is something to repair and the purse covers it.
All of it reads the Server's inventory snapshot; the window invents no number.

There is no toggle key: retail opens this from a repair NPC and which NPC is not decided yet.
CMainApp drives it through Open/Close, and Escape closes it like every other runtime window. */
class CRepairWindowView final
{
public:
	CRepairWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CRepairWindowView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	void Open() { m_bOpen = true; }
	void Close() { m_bOpen = false; }
	void Toggle() { m_bOpen = !m_bOpen; }
	/* Cancel transient gestures without closing or moving the window. */
	void Cancel_Interaction() { m_bDraggingPanel = false; m_bHasPendingRepair = false; }

	/* Screen-pixel rect of the panel while open, for CMainApp's text clip-out. */
	bool_t Get_ScreenRect(f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const;

	/* No-op while closed, except that it hides every owned sprite. */
	void Update();
	/* LOA-font labels; call after CImGuiLayer::EndFrame() like the other window text. */
	void Render_Text();
	/* Forces every owned sprite invisible without touching m_bOpen -- these live under
	LEVEL::STATIC and keep their last state across a level change unless told otherwise. */
	void Hide();

	/* One-shot: true exactly once the frame a live repair button was clicked. bAllSlots tells
	the all-gear button from the worn-gear button; CMainApp sends the typed repair request. */
	bool_t Try_Consume_RepairRequest(bool_t& bAllSlots);

private:
	/* Press on the title bar moves the whole window. */
	void Update_Drag();
	/* Shifts every slot this view owns. */
	void Move_Panel(f32_t fDeltaX, f32_t fDeltaY);
	/* Keeps the window inside the reference resolution after a drag. */
	void Clamp_ToScreen();
	/* Close button and the two repair buttons: hover tint and the release edge. */
	void Update_Buttons();
	/* Rebuilds the worn and bag lists of damaged gear and the two bills from the snapshot. */
	void Refresh_DamagedGear();

private:
	struct GEAR_ENTRY
	{
		string strItemId;
		uint8_t iPercent = 100;
	};

	unique_ptr<CUILayoutRuntime> m_pBackgroundView;
	vector<GEAR_ENTRY> m_EquippedGear;
	vector<GEAR_ENTRY> m_BagGear;
	uint32_t m_iEquippedCost = 0u;
	uint32_t m_iAllCost = 0u;

	bool_t m_bOpen = false;

	bool_t m_bDraggingPanel = false;
	f32_t m_fLastDragMouseX = 0.f;
	f32_t m_fLastDragMouseY = 0.f;

	bool_t m_bHasPendingRepair = false;
	bool_t m_bPendingRepairAllSlots = false;
};

NS_END
