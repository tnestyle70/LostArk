#include "WorldPlayerNameplateView.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "Character.h"
#include "GameInstance.h"
#include "HonorTitleCatalog.h"
#include "Model.h"
#include "Transform.h"
#include "UILabelFont.h"
#include "UserSettingsDocument.h"

#include <Windows.h>

#include <cmath>
#include <limits>
#include <utility>

namespace
{
	/* Body-model eye bones (model space); the plate anchors on the head top, EYE_TO_CROWN metres
	above them in world space. Characters without these bones use the fallback height. The bone
	goes through the presentation root (class scale, facing, vehicle seat lift), so a mounted
	character keeps the plate over the rider's head. */
	constexpr const char_t* EYE_BONE_NAMES[] = { "b_fc_l_eye_ani", "b_fc_r_eye_ani" };
	constexpr f32_t EYE_TO_CROWN = 0.16f;
	constexpr f32_t HEAD_HEIGHT_FALLBACK = 1.75f;
	/* Layout-reference px (1280x720): retail $YG760 12 px scaled 1.2x on the user's call, one
	line whose bottom sits NAME_BOTTOM_GAP above the head anchor. */
	constexpr f32_t REFERENCE_HEIGHT = 720.f;
	constexpr f32_t NAME_FONT_PX = 12.f * 1.2f;
	constexpr f32_t NAME_BOTTOM_GAP = 3.f;
	constexpr f32_t NAME_STACK_TOP = NAME_BOTTOM_GAP + NAME_FONT_PX + 2.f;
	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	/* Retail capture: the honor title reads as a saturated sky blue, the nickname after it as a
	pale yellow. Both are estimates from the capture's glyph pixels; the host colour presets are
	not in the extracted data. */
	const fvector_t COLOR_TITLE = XMVectorSet(58.f / 255.f, 175.f / 255.f, 255.f / 255.f, 1.f);   // #3AAFFF
	const fvector_t COLOR_NAME = XMVectorSet(235.f / 255.f, 232.f / 255.f, 200.f / 255.f, 1.f);   // #EBE8C8
	const fvector_t COLOR_SHADOW = XMVectorSet(0.f, 0.f, 0.f, 0.75f);

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

	void Draw_Shadowed(CGameInstance& gameInstance, const wstring_t& strFont, const wstring& strText,
		const float2_t& vPosition, const fvector_t vColor, const f32_t fScale)
	{
		gameInstance.Draw_Text(strFont, strText.c_str(),
			float2_t(vPosition.x + 1.f, vPosition.y + 1.f), COLOR_SHADOW, 0.f, float2_t(0.f, 0.f), fScale);
		gameInstance.Draw_Text(strFont, strText.c_str(), vPosition, vColor, 0.f, float2_t(0.f, 0.f), fScale);
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

bool_t Client::CWorldPlayerNameplateView::Try_GetHeadAnchor(
	const CCharacter& Character,
	float3_t& vOutWorldPosition)
{
	vOutWorldPosition = {};
	float4x4_t rootMatrix{};
	if (!Character.Try_Get_PresentationRootMatrix(&rootMatrix))
		return false;
	const matrix_t root = XMLoadFloat4x4(&rootMatrix);

	float3_t vLocal(0.f, HEAD_HEIGHT_FALLBACK, 0.f);
	f32_t fCrown = 0.f;
	if (const std::shared_ptr<Engine::CModel> pModel = Character.Get_BodyModel())
	{
		bool_t bFound = false;
		float3_t vEye{};
		for (const char_t* pBoneName : EYE_BONE_NAMES)
		{
			if (!pModel->Has_Bone(pBoneName))
				continue;
			float3_t vBone{};
			XMStoreFloat3(&vBone, pModel->Get_BoneMatrix(pBoneName).r[3]);
			if (!Is_Finite(vBone) || vBone.y <= 0.f)
				continue;
			if (!bFound || vBone.y > vEye.y)
				vEye = vBone;
			bFound = true;
		}
		if (bFound)
		{
			vLocal = vEye;
			fCrown = EYE_TO_CROWN;
		}
	}
	float3_t vWorld{};
	XMStoreFloat3(&vWorld, XMVector3TransformCoord(XMLoadFloat3(&vLocal), root));
	if (!Is_Finite(vWorld))
		return false;
	vOutWorldPosition = float3_t(vWorld.x, vWorld.y + fCrown, vWorld.z);
	return true;
}

f32_t Client::CWorldPlayerNameplateView::Stack_Top_RefPx()
{
	return NAME_STACK_TOP;
}

Client::PLAYER_RELATION Client::CWorldPlayerNameplateView::Resolve_Relation(
	const REPLICATED_PLAYER_VIEW& Player,
	const LostArk::Shared::S2C_PARTY_ROSTER* pPartyRoster)
{
	if (Player.isLocal)
		return PLAYER_RELATION::LOCAL;
	if (nullptr != pPartyRoster)
		for (const LostArk::Shared::PARTY_ROSTER_MEMBER& Member : pPartyRoster->Members)
			if (Member.iNetEntityId == Player.iNetEntityId)
				return PLAYER_RELATION::PARTY;
	return PLAYER_RELATION::OTHER;
}

void Client::CWorldPlayerNameplateView::Render(
	const std::vector<REPLICATED_PLAYER_VIEW>& Players,
	const LostArk::Shared::S2C_PARTY_ROSTER* pPartyRoster)
{
	CGameInstance& gameInstance = CGameInstance::Get();
	const float4x4_t* const pViewMatrix =
		gameInstance.Get_Transform(D3DTS::VIEW);
	const float4x4_t* const pProjectionMatrix =
		gameInstance.Get_Transform(D3DTS::PROJ);
	const float2_t vViewportSize = gameInstance.Get_ViewportSize();
	if (nullptr == pViewMatrix || nullptr == pProjectionMatrix ||
		vViewportSize.x <= 0.f || vViewportSize.y <= 0.f)
	{
		return;
	}
	/* Reference px -> screen px follows the viewport height, like the layout runtime. */
	const f32_t fRefToScreen = vViewportSize.y / REFERENCE_HEIGHT;

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

		float3_t vHeadPosition{};
		if (!Try_GetHeadAnchor(*pCharacter, vHeadPosition))
			continue;
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

		/* System option nametag rows, per relation: no name, no plate at all. */
		const PLAYER_RELATION eRelation = Resolve_Relation(player, pPartyRoster);
		if (!CUserSettings::Get().Is_NametagShown(eRelation, false))
			continue;
		std::wstring nickname;
		if (!Try_ConvertUtf8(player.strNickname, nickname))
			continue;
		/* BaseHeadStatus.updateTitle: title + " " + name on the one line; the title keeps its
		own colour, so the two halves are measured together and drawn apart. */
		std::wstring titleWithSpace;
		if (CUserSettings::Get().Is_NametagShown(eRelation, true))
			if (const wstring* pTitle = CHonorTitleCatalog::Find_Name(player.iHonorTitleId))
				titleWithSpace = *pTitle + L" ";
		f32_t fScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(
			FONT_YG760, NAME_FONT_PX * fRefToScreen, fScale);
		const float2_t vNameSize = gameInstance.Measure_Text(strFont, nickname.c_str());
		const f32_t fTitleWidth = titleWithSpace.empty() ? 0.f :
			gameInstance.Measure_Text(strFont, titleWithSpace.c_str()).x * fScale;
		const f32_t fTotalWidth = fTitleWidth + vNameSize.x * fScale;
		const float2_t vPosition(
			std::round(vScreenPosition.x - fTotalWidth * 0.5f),
			std::round(vScreenPosition.y - NAME_BOTTOM_GAP * fRefToScreen - vNameSize.y * fScale));
		if (!titleWithSpace.empty())
			Draw_Shadowed(gameInstance, strFont, titleWithSpace, vPosition, COLOR_TITLE, fScale);
		Draw_Shadowed(gameInstance, strFont, nickname,
			float2_t(std::round(vPosition.x + fTitleWidth), vPosition.y), COLOR_NAME, fScale);
	}
}
