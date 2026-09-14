#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Title-bar drag for a CUILayoutRuntime window: a left-click edge inside the handle rect
starts a drag, every slot of the document then follows the mouse until the button is released,
and the union of the slots is kept inside the reference resolution (a drag could otherwise park
a window off screen, where it still toggles but is never seen). One instance per window; the
same rule CInventoryView / CCharacterInfoWindowView / CAvatarBookWindowView apply with their own
Update_PanelDrag, written once here for the windows added after them. */
class CUIWindowDrag final
{
public:
	/* Per frame while the window shows. Handle rect is in the document's reference px; the
	drag does not start while the mouse is over pExcludeSlotId (the close button that sits on
	the title bar). While dragging the mouse is claimed for the window. Returns true while a
	drag is in progress, so a caller can skip its own click handling that frame. */
	bool_t Update(CUILayoutRuntime& View, const vector<string>& SlotIds,
		f32_t fHandleX, f32_t fHandleY, f32_t fHandleWidth, f32_t fHandleHeight,
		const char* pExcludeSlotId = nullptr);
	/* Forget an in-progress drag (window closed / hidden while the button was down). */
	void Reset() { m_bDragging = false; }
	bool_t Is_Dragging() const { return m_bDragging; }

private:
	static void Move_All(CUILayoutRuntime& View, const vector<string>& SlotIds,
		f32_t fDeltaX, f32_t fDeltaY);
	static void Clamp_ToScreen(CUILayoutRuntime& View, const vector<string>& SlotIds);

private:
	bool_t m_bDragging = false;
	f32_t m_fLastMouseX = 0.f;
	f32_t m_fLastMouseY = 0.f;
};

NS_END
