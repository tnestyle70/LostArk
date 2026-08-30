#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>

NS_BEGIN(Client)

class CHUDRuntimeView;

/* Shared "군단장 레이드 입장" full-screen popup. CLevel_Bern owns one for the
   real NPC-driven flow (Entrance submits Request_ConfirmNpcEntry through the
   caller's own command sink); CLevel_CharacterSelect owns a second, _DEBUG-only
   instance purely so its O-key visual-only preview doesn't duplicate this
   ~230-line draw path for a screen with no real entry there (see
   .md/TJ/08-30/2026-08-30_레이드입장창_PLAN.md). Both owners point at the same
   Data/UI/Bern/ValtanRaidEntry_Layout.json and share this one runtime path --
   this is not a second runtime of the same role, only what happens on Entrance
   differs, and that decision stays with the caller (Render() only reports
   whether Entrance was clicked, never sends a command itself). */
class CRaidEntryPreviewView
{
public:
	CRaidEntryPreviewView(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CRaidEntryPreviewView();

public:
	void Open();
	bool_t Is_Open() const { return m_isOpen; }

	/* Draws the panel/button art to ImGui::GetForegroundDrawList() -- must run
	   after the combat HUD renders (RenderCombatHUD/RenderBossHealthBar/
	   RenderSkillIcons/RenderQuickSlot in MainApp.cpp all draw to that same
	   shared list, which composites in real submission order regardless of
	   window Z-order) or the always-on HUD paints over this full-screen popup.
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

private:
	unique_ptr<CHUDRuntimeView> m_pView;
	bool_t m_isOpen = false;
	bool_t m_hasJustOpened = false;

	/* Second-step simple 수락/거절 dialog, opened by the main screen's own
	   Entrance button -- see the PLAN follow-up on this two-step flow. */
	unique_ptr<CHUDRuntimeView> m_pConfirmView;
	bool_t m_isConfirmStepOpen = false;
};

NS_END
