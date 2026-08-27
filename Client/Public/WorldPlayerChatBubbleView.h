#pragma once

#include "ClientReplication.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

/* Renders whichever replicated player currently has a live chat bubble
(CClientReplication::Try_Get_ActiveChatBubble) as a small rounded rect above
their nameplate -- the same world-to-screen projection
CWorldPlayerNameplateView::Try_ProjectWorldPosition already gives nameplates,
just offset further up. There is no extracted speech-bubble art yet, so the
background is a plain ImGui-drawn rounded rect rather than authored asset. */
class CWorldPlayerChatBubbleView final
{
public:
	void Render(
		const CClientReplication& Replication,
		const std::vector<REPLICATED_PLAYER_VIEW>& Players) const;
};

NS_END
