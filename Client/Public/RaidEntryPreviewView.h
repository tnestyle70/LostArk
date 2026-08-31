#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Shared "군단장 레이드 입장" full-screen popup. CLevel_Bern owns one for the
   real NPC-driven flow (Entrance submits Request_ConfirmNpcEntry through the
   caller's own command sink); CLevel_CharacterSelect owns a second, _DEBUG-only
   instance purely so its O-key visual-only preview doesn't duplicate this
   ~230-line draw path for a screen with no real entry there (see
   .md/TJ/08-30/2026-08-30_레이드입장창_PLAN.md). Both owners point at the same
   Data/UI/Bern/ValtanRaidEntry_Layout.json and share this one runtime path --
   this is not a second runtime of the same role, only what happens on Entrance
   differs, and that decision stays with the caller (Render() only reports
   whether Entrance was clicked, never sends a command itself).

   Both documents are real CUI_Sprite GameObjects on the owning Level's own
   "Layer_UI" (CUILayoutRuntime), so the popup renders through the normal engine
   pipeline instead of an ImGui popup/foreground drawlist. */
class CRaidEntryPreviewView
{
public:
	/* iOwnerLevelIndex is the Level whose Layer_UI holds this popup's sprites --
	   Bern for the real flow, Character Select for the Debug O-key preview. */
	CRaidEntryPreviewView(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);
	~CRaidEntryPreviewView();

public:
	void Open();
	bool_t Is_Open() const { return m_isOpen; }

	/* Drives the popup's own CUI_Sprite visibility/hover state and hit-testing
	   for one frame (the sprites themselves draw in CObject_Manager's normal
	   render cycle, so there is no ordering requirement against the combat HUD
	   anymore -- that is what the ImGui foreground drawlist version needed).
	   Clicking Entrance does not report back immediately -- it opens a second,
	   small confirm step (the original simple 수락/거절 dialog, reusing
	   Data/UI/Bern/BernValtanEntry_Layout.json) on top; this function only
	   returns true on the frame that inner dialog's own 수락 is clicked
	   (already closed internally by then). Decline on either step always
	   closes internally and never reports back. */
	bool_t Render();

	/* CGameInstance::Draw_Text submits immediately (SpriteBatch), so this must
	   run after CImGuiLayer::EndFrame() -- same reasoning as every other
	   Render_XModalText() split in this codebase. */
	void RenderText();

private:
	bool_t Render_ConfirmStep();
	void RenderText_ConfirmStep();
	/* Hides every slot that carries real art in either document. The
	   position-only marker slots (authored tint alpha 0 -- boss portrait, text
	   boxes, tier badges, ...) are deliberately never touched by either this or
	   the show path: Set_SlotVisible(true) resets a slot's tint to opaque white,
	   which would turn each invisible marker into a white box. */
	void Hide_AllSlots();
	void Hide_ConfirmSlots();

private:
	unique_ptr<CUILayoutRuntime> m_pView;
	bool_t m_isOpen = false;
	bool_t m_hasJustOpened = false;
	/* ImGui's own modal consumed Escape; with no popup left, this reproduces that close
	   gesture (the screen itself is labelled "닫기 (Esc)") with its own down-edge state. */
	bool_t m_wasEscapeDown = false;

	/* Second-step simple 수락/거절 dialog, opened by the main screen's own
	   Entrance button -- see the PLAN follow-up on this two-step flow. */
	unique_ptr<CUILayoutRuntime> m_pConfirmView;
	bool_t m_isConfirmStepOpen = false;
};

NS_END
