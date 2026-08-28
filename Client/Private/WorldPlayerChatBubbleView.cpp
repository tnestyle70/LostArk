#include "imgui.h"

#include "WorldPlayerChatBubbleView.h"

#include "Character.h"
#include "GameInstance.h"
#include "Transform.h"
#include "WorldPlayerNameplateView.h"

namespace
{
	// Above CWorldPlayerNameplateView's own NAMEPLATE_HEAD_OFFSET (2.2f), so
	// the bubble never overlaps the nickname it sits above.
	constexpr f32_t BUBBLE_HEAD_OFFSET = 2.7f;
	constexpr f32_t BUBBLE_PAD_X = 8.f;
	constexpr f32_t BUBBLE_PAD_Y = 4.f;
	constexpr f32_t BUBBLE_ROUNDING = 6.f;
}

void Client::CWorldPlayerChatBubbleView::Render(
	const CClientReplication& Replication,
	const std::vector<REPLICATED_PLAYER_VIEW>& Players) const
{
	CGameInstance& gameInstance = CGameInstance::Get();
	const float4x4_t* const pViewMatrix =
		gameInstance.Get_Transform(D3DTS::VIEW);
	const float4x4_t* const pProjectionMatrix =
		gameInstance.Get_Transform(D3DTS::PROJ);
	if (nullptr == pViewMatrix || nullptr == pProjectionMatrix)
		return;

	const float2_t vViewportSize = gameInstance.Get_ViewportSize();
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(ImGui::GetMainViewport());

	for (const REPLICATED_PLAYER_VIEW& player : Players)
	{
		std::string bubbleText;
		if (!Replication.Try_Get_ActiveChatBubble(
			player.iNetEntityId, bubbleText))
		{
			continue;
		}

		const std::shared_ptr<CCharacter> pCharacter = player.pCharacter.lock();
		if (nullptr == pCharacter)
			continue;
		const std::shared_ptr<Engine::CTransform> pTransform =
			pCharacter->Get_Transform();
		if (nullptr == pTransform)
			continue;

		float3_t vHeadPosition{};
		XMStoreFloat3(&vHeadPosition, pTransform->Get_State(STATE::POSITION));
		vHeadPosition.y += BUBBLE_HEAD_OFFSET;

		float2_t vScreenPosition{};
		if (!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
			vHeadPosition,
			*pViewMatrix,
			*pProjectionMatrix,
			vViewportSize,
			vScreenPosition))
		{
			continue;
		}

		const ImVec2 vTextSize = ImGui::CalcTextSize(bubbleText.c_str());
		const ImVec2 vRectMin(
			vScreenPosition.x - vTextSize.x * 0.5f - BUBBLE_PAD_X,
			vScreenPosition.y - vTextSize.y * 0.5f - BUBBLE_PAD_Y);
		const ImVec2 vRectMax(
			vScreenPosition.x + vTextSize.x * 0.5f + BUBBLE_PAD_X,
			vScreenPosition.y + vTextSize.y * 0.5f + BUBBLE_PAD_Y);

		pDrawList->AddRectFilled(
			vRectMin, vRectMax, IM_COL32(20, 20, 20, 200), BUBBLE_ROUNDING);
		pDrawList->AddRect(
			vRectMin, vRectMax, IM_COL32(255, 255, 255, 90), BUBBLE_ROUNDING);
		pDrawList->AddText(
			ImVec2(
				vScreenPosition.x - vTextSize.x * 0.5f,
				vScreenPosition.y - vTextSize.y * 0.5f),
			IM_COL32(255, 255, 255, 255),
			bubbleText.c_str());
	}
}
