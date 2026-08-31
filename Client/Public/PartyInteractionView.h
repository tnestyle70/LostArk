#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class IPlayerCommandSink;
class CUILayoutRuntime;

/* Right-click another replicated player -> "파티초대" context menu -> Server
   invite -> target sees an accept/decline popup -> Server roster sync.
   Shared by every world that replicates other players (Bern, Valtan Arena),
   the same reasoning CWorldPlayerNameplateView already uses -- just with its
   own input/popup state instead of being a pure stateless render helper.
   Same-room-only: targeting is by NetEntityId, the same identity
   Try_PickGroundPlane/CWorldPlayerNameplateView already key off of. There is
   no cross-room player identity (nickname is display text only). */
class CPartyInteractionView final
{
public:
	/* iOwnerLevelIndex: the calling Level's own ETOUI(LEVEL::...) -- this view's context menu
	slots become real CUI_Sprite GameObjects on that Level's own "Layer_UI", so they get cleaned
	up the same way the rest of that Level's objects do on Change_Level. Bern and Valtan Arena
	each own their own CPartyInteractionView instance (see this class's own doc comment), so
	each passes its own real level index rather than this guessing from global state. */
	void Initialize(
		ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);
	/* True while either popup owns the mouse, including its opening frame.
	   Callers add LB/RB blocks before gameplay polling; Engine holds each
	   consumed physical press until release, even after the popup closes. */
	bool_t Update(
		CClientReplication& Replication,
		const std::shared_ptr<IPlayerCommandSink>& pCommandSink,
		const std::vector<REPLICATED_PLAYER_VIEW>& OtherPlayers,
		bool_t worldInteractionAllowed);
	void Render(const std::shared_ptr<IPlayerCommandSink>& pCommandSink);
	/* CGameInstance::Draw_Text submits immediately (SpriteBatch), but
	   Render_InvitePopup's popup art composites later inside
	   CImGuiLayer::EndFrame() -- same reason Level_Bern's Valtan-entry modal
	   splits its text pass into Render_ValtanEntryModalText(). Callers reach
	   this the same way, through their level's Get_Active() after EndFrame. */
	void Render_InvitePopupText();
	/* Context menu's own nickname + "파티초대" label, same post-EndFrame() text-pass reasoning
	   as Render_InvitePopupText -- called from the same call site, right alongside it. */
	void Render_ContextMenuText();

private:
	bool_t Update_ContextMenuTrigger(
		const std::vector<REPLICATED_PLAYER_VIEW>& OtherPlayers,
		bool_t worldInteractionAllowed);
	void Render_ContextMenu(
		const std::shared_ptr<IPlayerCommandSink>& pCommandSink);
	void Render_InvitePopup(
		const std::shared_ptr<IPlayerCommandSink>& pCommandSink);

private:
	std::unique_ptr<CUILayoutRuntime> m_pInviteView;
	std::unique_ptr<CUILayoutRuntime> m_pContextMenuView;
	std::uint32_t m_iNextRequestSequence = 1u;
	bool_t m_wasRightMouseDown = false;

	bool_t m_hasContextMenuTarget = false;
	bool_t m_hasContextMenuJustOpened = false;
	/* Reference-resolution position the menu's PartyContextMenu_Panel slot was last moved to
	   (Set_SlotPosition), not a raw screen pixel -- Get_SlotRect/Set_SlotPosition both work in
	   that unit. */
	f32_t m_fContextMenuScreenX = 0.f;
	f32_t m_fContextMenuScreenY = 0.f;
	/* PartyContextMenu_HoverHighlight's authored offset from PartyContextMenu_Panel, captured
	   once at Initialize() before either slot is ever moved -- reused on every open instead of
	   re-reading Get_SlotRect at open time, since Set_SlotPosition overwrites the Panel slot's
	   own rect with wherever it was moved to last, which would corrupt this delta after the
	   first open. */
	f32_t m_fContextMenuHighlightOffsetX = 0.f;
	f32_t m_fContextMenuHighlightOffsetY = 0.f;
	LostArk::Shared::NET_ENTITY_ID m_iContextMenuTargetNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	std::string m_strContextMenuTargetNickname;

	bool_t m_isInvitePopupOpen = false;
	bool_t m_hasInvitePopupJustOpened = false;
	LostArk::Shared::S2C_PARTY_INVITE_RECEIVED m_PendingInvite{};
	std::wstring m_strTransferFailureNotice;
	std::chrono::steady_clock::time_point m_TransferNoticeExpiresAt{};
};

NS_END
