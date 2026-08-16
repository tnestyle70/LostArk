#include "WorldPlayerNameplateView.h"

#include "Character.h"
#include "GameInstance.h"
#include "Transform.h"

#include <Windows.h>

#include <cmath>
#include <limits>
#include <utility>

namespace
{
	constexpr f32_t NAMEPLATE_HEAD_OFFSET = 2.2f;
	constexpr f32_t NAMEPLATE_TEXT_SCALE = 0.75f;

	bool_t Is_Finite(const float2_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y);
	}

	bool_t Is_Finite(const float3_t& value)
	{
		return std::isfinite(value.x) &&
			std::isfinite(value.y) &&
			std::isfinite(value.z);
	}
}

bool_t Client::CWorldPlayerNameplateView::Try_ProjectWorldPosition(
	const float3_t& vWorldPosition,
	const float4x4_t& ViewMatrix,
	const float4x4_t& ProjectionMatrix,
	const float2_t& vViewportSize,
	float2_t& vOutScreenPosition)
{
	vOutScreenPosition = {};
	if (!Is_Finite(vWorldPosition) ||
		!Is_Finite(vViewportSize) ||
		vViewportSize.x <= 0.f || vViewportSize.y <= 0.f)
	{
		return false;
	}

	const vector_t vViewPosition = XMVector3TransformCoord(
		XMLoadFloat3(&vWorldPosition),
		XMLoadFloat4x4(&ViewMatrix));
	float3_t viewPosition{};
	XMStoreFloat3(&viewPosition, vViewPosition);
	if (!Is_Finite(viewPosition) || viewPosition.z <= 0.f)
		return false;

	const vector_t vProjected = XMVector3TransformCoord(
		vViewPosition,
		XMLoadFloat4x4(&ProjectionMatrix));
	float3_t ndc{};
	XMStoreFloat3(&ndc, vProjected);
	if (!Is_Finite(ndc) ||
		ndc.x < -1.f || ndc.x > 1.f ||
		ndc.y < -1.f || ndc.y > 1.f ||
		ndc.z < 0.f || ndc.z > 1.f)
	{
		return false;
	}

	vOutScreenPosition = float2_t(
		(ndc.x * 0.5f + 0.5f) * vViewportSize.x,
		(0.5f - ndc.y * 0.5f) * vViewportSize.y);
	return Is_Finite(vOutScreenPosition);
}

bool_t Client::CWorldPlayerNameplateView::Try_ConvertUtf8(
	const std::string_view Utf8,
	std::wstring& OutWide)
{
	OutWide.clear();
	if (Utf8.empty() ||
		Utf8.size() > static_cast<std::size_t>(
			(std::numeric_limits<int>::max)()))
	{
		return false;
	}

	const int sourceLength = static_cast<int>(Utf8.size());
	const int requiredLength = MultiByteToWideChar(
		CP_UTF8,
		MB_ERR_INVALID_CHARS,
		Utf8.data(),
		sourceLength,
		nullptr,
		0);
	if (requiredLength <= 0)
		return false;

	std::wstring staged(
		static_cast<std::size_t>(requiredLength),
		L'\0');
	if (requiredLength != MultiByteToWideChar(
		CP_UTF8,
		MB_ERR_INVALID_CHARS,
		Utf8.data(),
		sourceLength,
		staged.data(),
		requiredLength))
	{
		return false;
	}

	OutWide = std::move(staged);
	return true;
}

void Client::CWorldPlayerNameplateView::Render(
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
		if (LostArk::Shared::INVALID_PLAYER_ID == player.iPlayerId ||
			LostArk::Shared::INVALID_NET_ENTITY_ID == player.iNetEntityId ||
			player.strNickname.empty())
		{
			continue;
		}

		const std::shared_ptr<CCharacter> pCharacter =
			player.pCharacter.lock();
		if (nullptr == pCharacter)
			continue;
		const std::shared_ptr<CTransform> pTransform =
			pCharacter->Get_Transform();
		if (nullptr == pTransform)
			continue;

		float3_t vHeadPosition{};
		XMStoreFloat3(
			&vHeadPosition,
			pTransform->Get_State(STATE::POSITION));
		vHeadPosition.y += NAMEPLATE_HEAD_OFFSET;

		float2_t vScreenPosition{};
		if (!Try_ProjectWorldPosition(
			vHeadPosition,
			*pViewMatrix,
			*pProjectionMatrix,
			vViewportSize,
			vScreenPosition))
		{
			continue;
		}

		std::wstring nickname;
		if (!Try_ConvertUtf8(player.strNickname, nickname))
			continue;

		gameInstance.Draw_Text(
			TEXT("Font_YG330"),
			nickname.c_str(),
			vScreenPosition,
			Colors::White,
			0.f,
			float2_t(0.5f, 0.5f),
			NAMEPLATE_TEXT_SCALE);
	}
}
