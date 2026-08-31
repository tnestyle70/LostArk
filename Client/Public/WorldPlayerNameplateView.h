#pragma once

#include "ClientReplication.h"

#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

// Bern/Valtan의 Server-replicated player를 화면 공간 이름표로 투영한다.
// 이 view는 player identity, Character, font 또는 gameplay state를 소유하지 않는다.
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

	void Render(
		const std::vector<REPLICATED_PLAYER_VIEW>& Players) const;
};

NS_END
