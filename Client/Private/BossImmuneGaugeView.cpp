// 보스 패턴의 남은 목표량과 월드 위치를 UI 슬롯에 반영한다.
#include "BossImmuneGaugeView.h"

// 현재 카메라 행렬과 viewport를 조회한다.
#include "GameInstance.h"
// JSON 슬롯을 만들고 위치·표시·잔량 비율을 바꾼다.
#include "UILayoutRuntime.h"
// Try_ProjectWorldPosition으로 보스의 월드 위치를 화면에 투영한다.
#include "WorldPlayerNameplateView.h"

// 서버 진행도를 목표량 이하로 제한하는 min을 사용한다.
#include <algorithm>

namespace
{
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	const char* const ANCHOR_SLOT = "Immune_Anchor";
	const char* const TRACK_SLOT = "Immune_Track";
	const char* const MOVING_SLOTS[] = { "Immune_Frame", TRACK_SLOT };
}

// 지정 Level의 Layer_UI에 레이아웃 sprite를 준비하고 첫 Update 전에는 숨긴다.
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

// 기준 슬롯이 없으면 목록을 채우지 않아 Update에서 표시하지 않게 한다.
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

// 표시 조건·보스 상태·투영이 유효할 때만 슬롯을 갱신한다. 실패하면 이전 게이지를 숨긴다.
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

	// 보스 월드 위치를 Client 픽셀로 투영한 뒤 1280×720 기준 좌표로 바꾼다.
	// 위치가 없는 미리보기는 저작 기준점을 그대로 쓴다.
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

	// 진행도를 목표량까지 제한하고 남은 비율을 표시한다. 목표에 도달하면 잔량 이미지를 숨긴다.
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
