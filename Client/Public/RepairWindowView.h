#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Retail's NPC item repair window (interactionrepair.gfx, "아이템 수리"), drawn as real
CUI_Sprite GameObjects from Data/UI/Repair/RepairUI.json through CUILayoutRuntime, the same
path CInventoryView uses.

This slice owns the window only: chrome, the two section labels, the equipped 8x2 and bag 8x3
slot frames, the two cost rows with their guild-discount icons, the two repair buttons and the
money row. Durability values, repair cost and the Server command are a later vertical slice, so
every slot draws its empty frame and both cost figures read 0 -- nothing here invents a number
the Server has not sent.

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

	/* One-shot: true exactly once the frame a repair button was released over itself.
	bAllSlots distinguishes 모든 장비 수리 from 착용 장비 수리. No consumer yet -- the Server
	contract is a later slice -- so CMainApp only consumes it so the button cannot latch. */
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

private:
	unique_ptr<CUILayoutRuntime> m_pBackgroundView;

	bool_t m_bOpen = false;

	bool_t m_bDraggingPanel = false;
	f32_t m_fLastDragMouseX = 0.f;
	f32_t m_fLastDragMouseY = 0.f;

	bool_t m_bHasPendingRepair = false;
	bool_t m_bPendingRepairAllSlots = false;
};

NS_END
