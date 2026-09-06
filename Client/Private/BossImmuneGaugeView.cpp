#include "BossImmuneGaugeView.h"

#include "GameInstance.h"
#include "UILayoutRuntime.h"
#include "WorldPlayerNameplateView.h"

#include <algorithm>

namespace
{
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	const char* const ANCHOR_SLOT = "Immune_Anchor";
	const char* const TRACK_SLOT = "Immune_Track";
	const char* const MOVING_SLOTS[] = { "Immune_Frame", TRACK_SLOT };
}

Client::CBossImmuneGaugeView::CBossImmuneGaugeView(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iGameObjectLevelIndex)
{
	m_pView = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iGameObjectLevelIndex,
		TEXT("Layer_UI"), L"UI/BossUI/ImmuneGauge_Layout.json");
	m_pView->Set_AllSlotsVisible(false);
	Capture_SlotOffsets();
}

Client::CBossImmuneGaugeView::~CBossImmuneGaugeView()
{
}

void Client::CBossImmuneGaugeView::Capture_SlotOffsets()
{
	f32_t fW = 0.f, fH = 0.f;
	if (!m_pView->Get_SlotRect(ANCHOR_SLOT, m_fAnchorX, m_fAnchorY, fW, fH))
		return;
	for (const char* pSlotId : MOVING_SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f;
		if (!m_pView->Get_SlotRect(pSlotId, fX, fY, fW, fH))
			continue;
		SLOT_OFFSET Offset{};
		Offset.strId = pSlotId;
		Offset.fDx = fX - m_fAnchorX;
		Offset.fDy = fY - m_fAnchorY;
		m_SlotOffsets.push_back(std::move(Offset));
	}
}

void Client::CBossImmuneGaugeView::Hide()
{
	if (!m_bVisible)
		return;
	m_pView->Set_AllSlotsVisible(false);
	m_bVisible = false;
}

void Client::CBossImmuneGaugeView::Update(
	const f32_t fTimeDelta,
	const HUD_BOSS_STATE& Boss,
	const bool_t bAllowed)
{
	if (!bAllowed || !Boss.isValid || 0u == Boss.iResponseThreshold || m_SlotOffsets.empty())
	{
		Hide();
		return;
	}

	/* Anchor: the boss's replicated ground position projected to the screen; the Debug
	boss preview carries no position and keeps the authored anchor so the HUD Layout Tool
	can still place the slots. */
	f32_t fAnchorX = m_fAnchorX;
	f32_t fAnchorY = m_fAnchorY;
	if (Boss.hasPosition)
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		const float4x4_t* const pView = GameInstance.Get_Transform(D3DTS::VIEW);
		const float4x4_t* const pProj = GameInstance.Get_Transform(D3DTS::PROJ);
		const float2_t vViewport = GameInstance.Get_ViewportSize();
		float2_t vScreen{};
		if (nullptr == pView || nullptr == pProj ||
			vViewport.x <= 0.f || vViewport.y <= 0.f ||
			!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
				float3_t(Boss.fPositionX, Boss.fPositionY, Boss.fPositionZ),
				*pView, *pProj, vViewport, vScreen))
		{
			Hide();
			return;
		}
		fAnchorX = vScreen.x * (REF_WIDTH / vViewport.x);
		fAnchorY = vScreen.y * (REF_HEIGHT / vViewport.y);
	}
	for (const SLOT_OFFSET& Offset : m_SlotOffsets)
		m_pView->Set_SlotPosition(Offset.strId, fAnchorX + Offset.fDx, fAnchorY + Offset.fDy);

	/* The bar shows what is still left to deal: full at pattern start, empty when the
	threshold is met. */
	const uint32_t iProgress = (std::min)(Boss.iResponseProgress, Boss.iResponseThreshold);
	const f32_t fRemaining = 1.f -
		static_cast<f32_t>(iProgress) / static_cast<f32_t>(Boss.iResponseThreshold);

	m_pView->Set_AllSlotsVisible(true);
	m_pView->Set_SlotVisible(ANCHOR_SLOT, false);
	m_pView->Set_SlotFillRatio(TRACK_SLOT, fRemaining);
	m_pView->Set_SlotVisible(TRACK_SLOT, fRemaining > 0.f);
	m_bVisible = true;
	m_pView->Update(fTimeDelta);
}
