#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

class CUITextureCache;
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
	explicit CPartyWindowView(ComPtr<ID3D11Device> pDevice);
	~CPartyWindowView();

public:
	void Sync_From_Roster(const LostArk::Shared::S2C_PARTY_ROSTER& Roster,
		const CReplicatedPlayerHealth& Health);
	void Render();

private:
	unique_ptr<CUITextureCache> m_pTextureCache;
	string m_strPartyTitle;
	vector<PARTY_MEMBER> m_Members;
};

NS_END
