#pragma once

#include "ClientReplication.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
class CReplicatedPlayerHealth;

/* Bern/Valtan의 Server-replicated player를 화면 공간 이름표로 투영한다.
   이 view는 player identity, Character, font 또는 gameplay state를 소유하지 않는다.

   Retail headstatus.gfx PcHeadStatusMc, reduced to what the project replicates: the
   HeadStatusHPGauge (82x9 frame + 78x5 fill, one art pair per relation -- self / party member /
   other player -- straight from the gfx's progress_*HP_PickingHead sprites) sitting on the head
   anchor, and the name plate above it ($YG760 12 px, #ECECEC, centred). The worn honor title (snapshot
   id -> CHonorTitleCatalog name) goes ahead of the name on the same line, as BaseHeadStatus.updateTitle
   does; guild and the functional line are not drawn: the Server carries no such data. The gauges are CUI_Sprite
   slots of a level-owned document (Data/UI/HeadStatus/HeadStatus_Layout.json, up to 8 players)
   moved under each projected head every frame; the name is a LOA-font text pass. */
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

	/* Creates the HP gauge sprites on the owning level's Layer_UI (same construction as
	CPartyInteractionView). Without it Render still draws the names, just no gauges. */
	void Initialize(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);

	/* Roster decides party colouring, Health fills the gauge (a player without a snapshot
	yet shows no gauge). */
	void Render(
		const std::vector<REPLICATED_PLAYER_VIEW>& Players,
		const LostArk::Shared::S2C_PARTY_ROSTER& Roster,
		const CReplicatedPlayerHealth& Health);

private:
	enum class RELATION { PLAYER, PARTY, FRIEND };
	static const char* Frame_Art(RELATION eRelation);
	static const char* Fill_Art(RELATION eRelation);

private:
	unique_ptr<CUILayoutRuntime> m_pView;
};

NS_END
