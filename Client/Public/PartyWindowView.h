#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
class CReplicatedPlayerHealth;

/* Release-safe party roster overlay matching the in-game reference: a title bar, then one row
per member (class symbol, HP bar with the join-order number and nickname drawn on top, and a
crown mark on whoever is currently leader). Sync_From_Roster() feeds it the real Server roster
(CClientReplication::Get_PartyRoster(), populated by S2C_PARTY_ROSTER) each frame -- an empty
roster (no party yet) draws nothing. HP joins the accepted world snapshot by NetEntityId;
before that snapshot or outside the replicated world the HP fill stays hidden. Server roster
order puts the leader first and is preserved for both the crown and member numbers. */
class CPartyWindowView final
{
public:
	struct PARTY_MEMBER
	{
		string strNickname;
		/* Resources-relative path to that member's class symbol (ClassSelect's own
		IdentitySymbol.png per class), or empty for a class with no symbol art. */
		string strClassSymbolPath;
		f32_t fHpRatio = 0.f;
		bool_t hasHealthSnapshot = false;
		bool_t isLeader = false;
	};

public:
	/* Real CUI_Sprite GameObjects on LEVEL::STATIC's own "Layer_UI" (Data/UI/Party/
	   PartyWindow_Layout.json), same as the combat HUD/inventory -- this roster is drawn in
	   Bern and Valtan alike, so it does not belong to either Level's own layer. */
	CPartyWindowView(
		ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CPartyWindowView();

public:
	void Sync_From_Roster(const LostArk::Shared::S2C_PARTY_ROSTER& Roster,
		const CReplicatedPlayerHealth& Health);
	/* Drives the roster sprites' visibility/texture/fill for one frame; the sprites themselves
	   draw through CObject_Manager's normal render cycle. An empty roster hides every row. */
	void Render();
	/* Title and per-member nicknames. Split out for the same reason every other LOA-font label
	   in this codebase is: CGameInstance::Draw_Text submits its SpriteBatch immediately, so it
	   runs in the post-EndFrame text pass, not alongside the sprite state above. */
	void RenderText();

private:
	void Hide_AllRows();

private:
	unique_ptr<CUILayoutRuntime> m_pView;
	string m_strPartyTitle;
	vector<PARTY_MEMBER> m_Members;
	/* Max rows the layout document authors (PartyWindow_*_0..3) -- the same 4-player party cap
	   the Server room enforces. */
	static constexpr size_t MAX_ROWS = 4u;
};

NS_END
