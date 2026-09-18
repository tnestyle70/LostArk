#pragma once

#include "ClientReplication.h"
#include "UserSettingsDocument.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class CCharacter;

/* Projects the Server-replicated players of Bern / Valtan / KoukuSaydon into screen-space
   nameplates. The view owns no player identity, Character, font or gameplay state.

   Retail headstatus.gfx PcHeadStatusMc reduced to what the project shows: the name plate only
   ($YG760 12 px, centred, one line). The worn honor title (snapshot id -> CHonorTitleCatalog name)
   goes ahead of the name on the same line, as BaseHeadStatus.updateTitle does. Guild, the
   functional line and the HP gauge are not drawn: the Server carries no guild data and the
   reference screen shows no gauge over a player. The anchor is the character's head (eye bones
   of the body model plus the crown offset), so the plate sits just above the hair whatever the
   class scale. Sizes are layout-reference px (1280x720) like the rest of the runtime UI. */
class CWorldPlayerNameplateView final
{
public:
	static bool_t Try_ProjectWorldPosition(
		const float3_t& vWorldPosition,
		const float4x4_t& ViewMatrix,
		const float4x4_t& ProjectionMatrix,
		const float2_t& vViewportSize,
		float2_t& vOutScreenPosition);

	/* Shared with CWorldPlayerChatBubbleView, which draws its own text in the same LOA font
	just above these nameplates and needs the identical UTF-8 -> wide conversion. */
	static bool_t Try_ConvertUtf8(
		std::string_view Utf8,
		std::wstring& OutWide);

	/* World position of the head top (eye bones x presentation scale + crown offset, else a
	fixed fallback height). The bubble stacks from the same anchor. */
	static bool_t Try_GetHeadAnchor(
		const CCharacter& Character,
		float3_t& vOutWorldPosition);

	/* Reference px (1280x720) the name plate reaches above the head anchor: the next stacked
	element (the chat bubble) starts there. */
	static f32_t Stack_Top_RefPx();
	/* Own / party (by net entity id in the roster) / other. Shared with the chat bubble. */
	static PLAYER_RELATION Resolve_Relation(
		const REPLICATED_PLAYER_VIEW& Player,
		const LostArk::Shared::S2C_PARTY_ROSTER* pPartyRoster);

	/* The party roster tells own / party / other apart for the system option nametag rows
	(name and honor title per relation); without it every non-local player counts as other. */
	void Render(const std::vector<REPLICATED_PLAYER_VIEW>& Players,
		const LostArk::Shared::S2C_PARTY_ROSTER* pPartyRoster = nullptr);
};

NS_END
