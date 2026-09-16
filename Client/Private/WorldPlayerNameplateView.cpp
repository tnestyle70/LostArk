#include "WorldPlayerNameplateView.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "Character.h"
#include "GameInstance.h"
#include "HonorTitleCatalog.h"
#include "ReplicatedPlayerHealth.h"
#include "Transform.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"

#include <Windows.h>

#include <cmath>
#include <limits>
#include <utility>

namespace
{
	constexpr f32_t NAMEPLATE_HEAD_OFFSET = 2.2f;
	/* headstatus.gfx PcHeadStatusMc, retail px around the head anchor (1920x1080 stage):
	hpGauge at (-39,-8) whose frame art starts 2 px outside the 78x5 fill; namePlate at
	(-29,-37) with its 12 px text at y+5 -> text centre 26 px above the anchor. */
	constexpr f32_t RETAIL_STAGE_HEIGHT = 1080.f;
	constexpr f32_t GAUGE_FILL_X = -39.f;
	constexpr f32_t GAUGE_FILL_Y = -8.f;
	constexpr f32_t GAUGE_FRAME_PAD = 2.f;
	constexpr f32_t NAME_CENTER_Y = -26.f;
	constexpr f32_t NAME_FONT_PX = 12.f;
	constexpr size_t GAUGE_SLOTS = 8;
	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	const fvector_t COLOR_NAME = XMVectorSet(236.f / 255.f, 236.f / 255.f, 236.f / 255.f, 1.f);   // #ECECEC

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

const char* Client::CWorldPlayerNameplateView::Frame_Art(const RELATION eRelation)
{
	switch (eRelation)
	{
	case RELATION::PLAYER: return "UI/HeadStatus/HS_Frame_Player.png";
	case RELATION::PARTY: return "UI/HeadStatus/HS_Frame_Party.png";
	default: return "UI/HeadStatus/HS_Frame_Friend.png";
	}
}

const char* Client::CWorldPlayerNameplateView::Fill_Art(const RELATION eRelation)
{
	switch (eRelation)
	{
	case RELATION::PLAYER: return "UI/HeadStatus/HS_Fill_Player.png";
	case RELATION::PARTY: return "UI/HeadStatus/HS_Fill_Party.png";
	default: return "UI/HeadStatus/HS_Fill_Friend.png";
	}
}

void Client::CWorldPlayerNameplateView::Initialize(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iOwnerLevelIndex)
{
	m_pView = std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, iOwnerLevelIndex, TEXT("Layer_UI"),
		L"UI/HeadStatus/HeadStatus_Layout.json");
	for (const string& strId : m_pView->Get_SlotIds())
		m_pView->Set_SlotVisible(strId, false);
}

void Client::CWorldPlayerNameplateView::Render(
	const std::vector<REPLICATED_PLAYER_VIEW>& Players,
	const LostArk::Shared::S2C_PARTY_ROSTER& Roster,
	const CReplicatedPlayerHealth& Health)
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
	/* Retail px -> screen px follows the viewport height (the gfx stage is 1080 tall); slots
	live in the layout's 1280x720 reference, so screen -> reference is a second factor. */
	const f32_t fRetailToScreen = vViewportSize.y / RETAIL_STAGE_HEIGHT;
	const f32_t fRefWidth = nullptr != m_pView ? m_pView->Get_ResolutionWidth() : 1280.f;
	const f32_t fRefHeight = nullptr != m_pView ? m_pView->Get_ResolutionHeight() : 720.f;
	const f32_t fScreenToRefX = fRefWidth / vViewportSize.x;
	const f32_t fScreenToRefY = fRefHeight / vViewportSize.y;

	size_t iGaugeSlot = 0;
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

		RELATION eRelation = RELATION::FRIEND;
		if (player.isLocal)
			eRelation = RELATION::PLAYER;
		else
		{
			for (const LostArk::Shared::PARTY_ROSTER_MEMBER& member : Roster.Members)
			{
				if (member.iNetEntityId == player.iNetEntityId)
				{
					eRelation = RELATION::PARTY;
					break;
				}
			}
		}

		/* HP gauge: frame + fill slots parked under this head, relation art, Server ratio. */
		const REPLICATED_PLAYER_HEALTH health = Health.Find(player.iNetEntityId);
		if (nullptr != m_pView && iGaugeSlot < GAUGE_SLOTS && health.hasSnapshot)
		{
			const string strFrame = "HS_" + std::to_string(iGaugeSlot) + "_HpFrame";
			const string strFill = "HS_" + std::to_string(iGaugeSlot) + "_HpFill";
			const f32_t fFillX = (vScreenPosition.x + GAUGE_FILL_X * fRetailToScreen) * fScreenToRefX;
			const f32_t fFillY = (vScreenPosition.y + GAUGE_FILL_Y * fRetailToScreen) * fScreenToRefY;
			const f32_t fPadX = GAUGE_FRAME_PAD * fRetailToScreen * fScreenToRefX;
			const f32_t fPadY = GAUGE_FRAME_PAD * fRetailToScreen * fScreenToRefY;
			m_pView->Set_SlotTexture(strFrame, Frame_Art(eRelation));
			m_pView->Set_SlotTexture(strFill, Fill_Art(eRelation));
			m_pView->Set_SlotPosition(strFrame, fFillX - fPadX, fFillY - fPadY);
			m_pView->Set_SlotPosition(strFill, fFillX, fFillY);
			m_pView->Set_SlotFillRatio(strFill, health.Get_Ratio());
			m_pView->Set_SlotVisible(strFrame, true);
			m_pView->Set_SlotVisible(strFill, true);
			++iGaugeSlot;
		}

		std::wstring nickname;
		if (!Try_ConvertUtf8(player.strNickname, nickname))
			continue;
		/* BaseHeadStatus.updateTitle: title + " " + name on the one line. */
		if (const wstring* pTitle = CHonorTitleCatalog::Find_Name(player.iHonorTitleId))
			nickname = *pTitle + L" " + nickname;
		f32_t fScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(
			FONT_YG760, NAME_FONT_PX * fRetailToScreen, fScale);
		const float2_t vMeasured = gameInstance.Measure_Text(strFont, nickname.c_str());
		const float2_t vPosition(
			std::round(vScreenPosition.x - vMeasured.x * fScale * 0.5f),
			std::round(vScreenPosition.y + NAME_CENTER_Y * fRetailToScreen - vMeasured.y * fScale * 0.5f));
		gameInstance.Draw_Text(strFont, nickname.c_str(),
			float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 0.75f), 0.f, float2_t(0.f, 0.f), fScale);
		gameInstance.Draw_Text(strFont, nickname.c_str(),
			vPosition, COLOR_NAME, 0.f, float2_t(0.f, 0.f), fScale);
	}

	/* Gauges past the players seen this frame (left / despawned) go dark. */
	if (nullptr != m_pView)
	{
		for (size_t i = iGaugeSlot; i < GAUGE_SLOTS; ++i)
		{
			m_pView->Set_SlotVisible("HS_" + std::to_string(i) + "_HpFrame", false);
			m_pView->Set_SlotVisible("HS_" + std::to_string(i) + "_HpFill", false);
		}
	}
}
