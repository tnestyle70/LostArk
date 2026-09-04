#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <memory>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Shared "군단장 레이드 입장" full-screen popup. CLevel_Bern owns one for the
   real NPC-driven flow (Entrance submits Request_ConfirmNpcEntry through the
   caller's own command sink); CLevel_CharacterSelect owns a second, _DEBUG-only
   instance purely so its O-key visual-only preview doesn't duplicate this
   ~230-line draw path for a screen with no real entry there (see
   .md/TJ/08-30/2026-08-30_레이드입장창_PLAN.md). Both owners point at the same
   Data/UI/RaidEntry/ValtanRaidEntry_Layout.json and share this one runtime path --
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

	/* "나의 아이템 레벨 N". There is no item-level system yet -- equipment upgrade is meant to
	   feed this -- so the value lives here as one number the owner sets rather than baked into
	   the drawn string, which is what made it a fixed 1556. */
	void Set_MyItemLevel(int32_t iItemLevel) { m_iMyItemLevel = iItemLevel; }
	int32_t Get_MyItemLevel() const { return m_iMyItemLevel; }

	/* Drives the popup's own CUI_Sprite visibility/hover state and hit-testing
	   for one frame (the sprites themselves draw in CObject_Manager's normal
	   render cycle, so there is no ordering requirement against the combat HUD
	   anymore -- that is what the ImGui foreground drawlist version needed).
	   Clicking Entrance does not report back immediately -- it opens a second,
	   small confirm step (the original simple 수락/거절 dialog, reusing
	   Data/UI/RaidEntry/BernValtanEntry_Layout.json) on top; this function only
	   returns true on the frame that inner dialog's own 수락 is clicked
	   (already closed internally by then). Decline on either step always
	   closes internally and never reports back. */
	bool_t Render();

	/* CGameInstance::Draw_Text submits immediately (SpriteBatch), so this must
	   run after CImGuiLayer::EndFrame() -- same reasoning as every other
	   Render_XModalText() split in this codebase. */
	void RenderText();

	/* 파티 레이드 입장 투표에서 이번 프레임에 발생한 사용자 의도. Level_Bern이 Render 뒤
	   Consume_Intent로 한 번 가져가 command sink로 제출한다(UI는 socket을 모른다).
	   입장하기 = PROPOSE(선택 탭의 target), 수락/거절 창의 수락·거절 = RESPOND. */
	struct RAID_ENTRY_INTENT
	{
		enum KIND { NONE, PROPOSE, RESPOND } eKind = NONE;
		LostArk::Shared::RAID_ENTRY_TARGET eTarget =
			LostArk::Shared::RAID_ENTRY_TARGET::VALTAN;
		std::uint32_t iProposalId = 0u;
		bool_t bAccepted = false;
	};
	RAID_ENTRY_INTENT Consume_Intent();

	/* 서버 프롬프트 수신 시 수락/거절 창만 연다 -- 파티원은 입장 UI를 안 열었을 수 있으므로
	   메인 화면 없이 confirm step만 표시한다. iProposalId는 응답에 그대로 되돌린다. */
	void Open_VoteConfirm(std::uint32_t iProposalId,
		LostArk::Shared::RAID_ENTRY_TARGET target);
	/* 투표가 거절/타임아웃/취소로 종료되면 수락/거절 창을 닫고 Bern에 남는다. */
	void Close_VoteConfirm();

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
	/* Applies the selected raid to every slot that differs per raid (boss art, left panel,
	   esther portraits, reward icons and their grade colours) and drives the tab strip's own
	   selected look. Called on open and whenever the selection changes, not per frame. */
	void Apply_RaidSelection();
	/* Tab strip hit-test plus the selected tab's grow/lit pass. Separate from Apply because the
	   grow is eased over a few frames while the raid swap is a one-shot. */
	void Update_TabStrip(f32_t fTimeDelta);

private:
	unique_ptr<CUILayoutRuntime> m_pView;
	bool_t m_isOpen = false;
	bool_t m_hasJustOpened = false;
	/* Index into RAID_DEFS, not a tab slot number -- only the raids with real content are
	   selectable, and the tab a raid sits on is the raid's own property. */
	int32_t m_iSelectedRaid = 0;
	/* 0..1 ease driving the selected tab's grow. Reset on selection change so the new tab
	   grows in rather than snapping. */
	f32_t m_fTabGrow = 1.f;
	/* Free-running clock for the selected tab's gold pulse -- independent of the grow, which
	   settles, so the lit look keeps breathing after the tab has finished rising. */
	f32_t m_fGlowPhase = 0.f;
	int32_t m_iMyItemLevel = 1556;
	/* Authored tab rects, captured once: the grow rewrites them through Set_SlotRect, so the
	   document's own values have to survive somewhere to grow from. */
	bool_t m_hasTabBaseRects = false;
	f32_t m_TabBaseRect[8][4] = {};
	/* ImGui's own modal consumed Escape; with no popup left, this reproduces that close
	   gesture (the screen itself is labelled "닫기 (Esc)") with its own down-edge state. */
	bool_t m_wasEscapeDown = false;

	/* Second-step simple 수락/거절 dialog, opened by the main screen's own
	   Entrance button -- see the PLAN follow-up on this two-step flow. */
	unique_ptr<CUILayoutRuntime> m_pConfirmView;
	bool_t m_isConfirmStepOpen = false;
	/* 이번 프레임의 투표 의도. Render/Render_ConfirmStep가 설정하고 Consume_Intent가
	   가져가며 비운다. */
	RAID_ENTRY_INTENT m_Intent{};
	/* 열린 투표의 proposalId(프롬프트가 준 값). 수락/거절 응답에 되돌린다. */
	std::uint32_t m_iVoteProposalId = 0u;
};

NS_END
