#pragma once

#include "ClientReplication.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Renders whichever replicated player currently has a live chat bubble
(CClientReplication::Try_Get_ActiveChatBubble) as the retail HeadStatus_Balloon: the
headstatus.gfx normalBG art (72x50, tail bottom-left) stretched as a 9-slice through nine
CUI_Sprite slots per bubble (Data/UI/HeadStatus/ChatBubble_Layout.json, level-owned like the
nameplate gauges) with the text in $YG760 12 px #EEEEEE, wrapped at the retail 200 px, sitting
right above the name plate the way BaseHeadStatus::alignHeadStatus stacks it. Up to four bubbles
a frame; the projection is CWorldPlayerNameplateView::Try_ProjectWorldPosition. */
class CWorldPlayerChatBubbleView final
{
public:
	/* Creates the bubble piece sprites on the owning level's Layer_UI. Without it nothing draws. */
	void Initialize(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);

	void Render(
		const CClientReplication& Replication,
		const std::vector<REPLICATED_PLAYER_VIEW>& Players);

private:
	/* Greedy wrap in screen px: breaks at the last space when there is one, otherwise per
	character (Korean has no word gaps to lean on). */
	void Wrap_Text(const wstring_t& strFont, f32_t fFontScale, const std::wstring& strText,
		f32_t fMaxWidth, std::vector<std::wstring>& outLines) const;
	void Hide_Bubble(size_t iBubble);

private:
	/* One text line already placed in screen px. The nine art slices are CUI_Sprite objects the
	level's UI render group draws, and that group runs before this view is reached, so a rect set
	here only reaches the screen on the next frame. Drawing the text immediately therefore let it
	slide ahead of its own bubble while the speaker moved. The line is kept here and drawn on the
	next call instead, so art and text always show the same frame's anchor. */
	struct PLACED_LINE
	{
		std::wstring strText;
		float2_t vPosition{};
	};

private:
	unique_ptr<CUILayoutRuntime> m_pView;
	std::vector<std::wstring> m_Lines;
	std::vector<PLACED_LINE> m_PlacedLines;
	wstring_t m_strPlacedFont;
	f32_t m_fPlacedFontScale = 1.f;
};

NS_END
