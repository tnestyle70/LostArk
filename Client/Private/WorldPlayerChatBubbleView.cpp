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
	constexpr f32_t BUBBLE_TEXT_SCALE = 0.8f;
	/* Same LOA font pass the nameplate itself uses (Draw_Text, no ImGui). The old ImGui
	drawlist version painted a rounded dark quad behind the text; without a 2D rect primitive
	on the engine path, a 4-way near-black outline keeps the text readable over the world
	instead. */
	constexpr f32_t BUBBLE_OUTLINE_OFFSET = 1.f;
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

		std::wstring bubbleWide;
		if (!CWorldPlayerNameplateView::Try_ConvertUtf8(bubbleText, bubbleWide))
			continue;

		const float2_t OUTLINE_OFFSETS[] = {
			{ -BUBBLE_OUTLINE_OFFSET, 0.f }, { BUBBLE_OUTLINE_OFFSET, 0.f },
			{ 0.f, -BUBBLE_OUTLINE_OFFSET }, { 0.f, BUBBLE_OUTLINE_OFFSET },
		};
		for (const float2_t& vOffset : OUTLINE_OFFSETS)
		{
			gameInstance.Draw_Text(
				TEXT("Font_YG330"),
				bubbleWide.c_str(),
				float2_t(vScreenPosition.x + vOffset.x, vScreenPosition.y + vOffset.y),
				XMVectorSet(20.f / 255.f, 20.f / 255.f, 20.f / 255.f, 0.9f),
				0.f,
				float2_t(0.5f, 0.5f),
				BUBBLE_TEXT_SCALE);
		}
		gameInstance.Draw_Text(
			TEXT("Font_YG330"),
			bubbleWide.c_str(),
			vScreenPosition,
			Colors::White,
			0.f,
			float2_t(0.5f, 0.5f),
			BUBBLE_TEXT_SCALE);
	}
}
