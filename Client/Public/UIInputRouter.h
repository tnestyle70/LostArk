#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

/* Mouse hit-testing and gameplay-mouse gating for CUILayoutRuntime-driven screens (the real
CUIObject runtime replacing the ImGui interim rendering), replacing what ImGui::BeginPopupModal
+ ImGui::IsMouseClicked/GetMousePos gave those screens for free. Uses the same native sources
CPlayerController's own world-ray picking already uses (GetCursorPos/ScreenToClient,
CGameInstance::Get_DIMouseStateRaw) instead of ImGui, since the whole point of this migration is
for the runtime UI path to not depend on ImGui at all.

Scope: one modal-style screen owning input for the whole frame while it's open (matching how
BeginPopupModal already behaved) -- not a z-ordered arbitration system across several
simultaneously-interactive screens. No currently planned migration needs that; add it if one
does. */
class CUIInputRouter final
{
public:
	static CUIInputRouter& Get();

public:
	/* Call once per frame before any screen checks its own widgets. */
	void Begin_Frame();
	/* rect is in the caller's own document reference-resolution units; converted to real
	viewport pixels here so callers don't duplicate that scale math. */
	bool_t Is_Hovered(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
		f32_t fRefWidth, f32_t fRefHeight) const;
	/* True once on the real left-click down-edge while hovered; also claims the mouse for this
	frame (see Claim_Mouse_This_Frame) whether or not the caller acts on it. */
	bool_t Is_Clicked(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
		f32_t fRefWidth, f32_t fRefHeight);
	/* Real left-click down-edge this frame, independent of any rect. */
	bool_t Is_LeftClickEdge() const;
	/* Real right-click down-edge this frame, independent of any rect -- for a menu that closes
	   on "right-clicked anywhere outside my own art" (checked as !Is_Hovered(wholeMenuRect) &&
	   Is_RightClickEdge()), the real dismiss gesture for the Party context menu -- not a left
	   click. */
	bool_t Is_RightClickEdge() const;
	/* Left button currently held, independent of any rect -- for a multi-frame drag gesture
	(press on frame N, keep moving through frame N+k, release on frame N+k) that Is_Clicked's
	single-frame edge can't express by itself. */
	bool_t Is_LeftDown() const { return m_bLeftDownThisFrame; }
	/* Real left-click up-edge this frame, independent of any rect -- the drag-release
	counterpart to Is_LeftClickEdge. */
	bool_t Is_LeftReleaseEdge() const { return !m_bLeftDownThisFrame && m_bLeftDownLastFrame; }
	/* Current cursor position converted into the caller's own document reference-resolution
	units (the inverse of the scaling Is_Hovered applies) -- for a drag gesture that needs the
	real mouse delta/position between frames, not just a hit-test bool. False (position
	unmodified) if the cursor or viewport size can't be read this frame. */
	bool_t Get_MousePosition(f32_t fRefWidth, f32_t fRefHeight, f32_t& outX, f32_t& outY) const;
	/* Raw client-area pixel position, with no reference-resolution conversion -- the same space
	ImGui::GetMousePos() reads in this single-viewport game (WorkPos is always (0,0) here), for a
	caller that has to hand a drop position to an ImGui screen not yet migrated (e.g.
	CInventoryView::Try_Consume_ItemDrop's contract with CMainApp::Render_ItemQuickSlots). False
	if the cursor can't be read this frame. */
	bool_t Get_ClientCursorPosition(f32_t& outX, f32_t& outY) const;
	/* A modal/full-screen UI screen claims the mouse for the whole frame regardless of which
	specific widget (if any) is hovered -- its own dim backdrop swallowing clicks, matching
	BeginPopupModal's own behavior. */
	void Claim_Mouse_This_Frame();
	/* Text-input capture for a runtime UI text field (the Create Character nickname box) -- the
	WM_CHAR half of what ImGui::InputText provided. While active, WndProc (Client.cpp) feeds every
	committed WM_CHAR UTF-16 unit into a queue the owning screen drains once per frame via
	Take_TypedChars, and every io.WantTextInput-style keybind/gameplay gate must also treat
	Is_TextInputActive as "someone is typing". The still-composing (uncommitted) Hangul string
	stays readable via Engine::CImGuiLayer::Get_ImeCompositionString() -- composition happens at
	the window level, not the widget, so it keeps working without InputText. */
	void Start_TextInput();
	void Stop_TextInput();
	bool_t Is_TextInputActive() const { return m_bTextInputActive; }
	/* WndProc only. Queues a committed WM_CHAR UTF-16 unit (control chars like '\b'/'\r'/escape
	included -- the field interprets them) while text input is active; dropped otherwise. */
	void On_Char(wchar_t ch);
	/* Returns and clears everything typed since the last call. */
	wstring_t Take_TypedChars();
	/* Applies CGameInstance::Get().SetInputBlocked(false, true) if anything claimed the mouse
	this frame. Call once per frame, after every screen has had a chance to render. */
	void End_Frame();

private:
	CUIInputRouter() = default;

private:
	bool_t	m_bMouseClaimedThisFrame = false;
	/* Left-button down-edge is computed once in Begin_Frame (not per Is_Clicked call, since a
	caller may test several candidate widgets in one frame) and rolled forward in End_Frame. */
	bool_t	m_bLeftDownThisFrame = false;
	bool_t	m_bLeftDownLastFrame = false;
	bool_t	m_bRightDownThisFrame = false;
	bool_t	m_bRightDownLastFrame = false;
	bool_t	m_bTextInputActive = false;
	wstring_t	m_TypedChars;
};

NS_END
