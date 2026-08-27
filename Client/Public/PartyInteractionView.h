#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class IPlayerCommandSink;
class CHUDRuntimeView;

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
	void Initialize(
		ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
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

private:
	bool_t Update_ContextMenuTrigger(
		const std::vector<REPLICATED_PLAYER_VIEW>& OtherPlayers,
		bool_t worldInteractionAllowed);
	void Render_ContextMenu(
		const std::shared_ptr<IPlayerCommandSink>& pCommandSink);
	void Render_InvitePopup(
		const std::shared_ptr<IPlayerCommandSink>& pCommandSink);

private:
	std::unique_ptr<CHUDRuntimeView> m_pInviteView;
	std::unique_ptr<CHUDRuntimeView> m_pContextMenuView;
	std::uint32_t m_iNextRequestSequence = 1u;
	bool_t m_wasRightMouseDown = false;

	bool_t m_hasContextMenuTarget = false;
	bool_t m_hasContextMenuJustOpened = false;
	f32_t m_fContextMenuScreenX = 0.f;
	f32_t m_fContextMenuScreenY = 0.f;
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
