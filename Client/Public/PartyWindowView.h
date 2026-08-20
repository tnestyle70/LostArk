#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

class CUITextureCache;

/* Release-safe party roster overlay matching the in-game reference: a title bar, then one row
per member (class symbol, HP bar with the join-order number and nickname drawn on top, and a
crown mark on whoever is currently leader). UI-only for now -- there is no party Shared
protocol yet (no C2S/S2C party messages, no Server roster), so Render() draws whatever
SEED_MEMBERS below is populated with instead of a real roster. Swapping that for live data is
a separate follow-up once the team has a party system to read from. */
class CPartyWindowView final
{
public:
	struct PARTY_MEMBER
	{
		string strNickname;
		/* Resources-relative path to that member's class symbol (ClassSelect's own
		IdentitySymbol.png per class), or empty for a class with no symbol art. */
		string strClassSymbolPath;
		f32_t fHpRatio = 1.f;
		bool_t isLeader = false;
	};

public:
	explicit CPartyWindowView(ComPtr<ID3D11Device> pDevice);
	~CPartyWindowView();

public:
	void Render();

private:
	unique_ptr<CUITextureCache> m_pTextureCache;
	string m_strPartyTitle;
	vector<PARTY_MEMBER> m_Members;
};

NS_END
