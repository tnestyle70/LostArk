// Windows 헤더보다 WinSock2를 먼저 포함하고 Esc 검사에 필요한 DIK_ESCAPE를 가져온다.
#include <WinSock2.h>
#include <dinput.h>

// ID를 든 상태와 마우스 추적 아이콘을 갱신한다.
#include "QuickSlotDragView.h"

// DIK_ESCAPE의 현재 키 상태를 조회한다.
#include "GameInstance.h"
// 문서 기준 커서 위치·왼쪽 눌림을 읽고 게임 마우스를 점유한다.
#include "UIInputRouter.h"
// 운반 아이콘의 이미지·위치·표시를 바꾼다.
#include "UILayoutRuntime.h"
// 일반 창 앞에 그릴 MODAL 정렬값을 사용한다.
#include "UITextOcclusion.h"

namespace
{
	const char* ICON_SLOT = "QuickSlotDrag_Icon";
}

Client::CQuickSlotDragView::CQuickSlotDragView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Common/QuickSlotDrag_Layout.json") }
{
	// 운반 아이콘을 일반 창보다 앞인 MODAL layer에 배치한다.
	m_pView->Set_UISortLayer(UI_TEXT_LAYER::MODAL);
	// STATIC Level의 아이콘은 생성 직후 숨겨 운반 전 노출을 막는다.
	Hide();
}

Client::CQuickSlotDragView::~CQuickSlotDragView() = default;

void Client::CQuickSlotDragView::Begin_ItemCarry(const string& strItemId, const string& strIconPath)
{
	m_ePayload = PAYLOAD::ITEM;
	m_strItemId = strItemId;
	m_iVehicleId = 0u;
	m_bBeganThisFrame = true;
	if (!strIconPath.empty())
		m_pView->Set_SlotTexture(ICON_SLOT, strIconPath);
}

void Client::CQuickSlotDragView::Begin_VehicleCarry(const uint32_t iVehicleId, const string& strIconPath)
{
	m_ePayload = PAYLOAD::VEHICLE;
	m_strItemId.clear();
	m_iVehicleId = iVehicleId;
	m_bBeganThisFrame = true;
	if (!strIconPath.empty())
		m_pView->Set_SlotTexture(ICON_SLOT, strIconPath);
}

// MainApp의 놓기 처리·레벨 전환 또는 Esc 입력이 운반 데이터를 초기화한다.
void Client::CQuickSlotDragView::Cancel()
{
	m_ePayload = PAYLOAD::NONE;
	m_strItemId.clear();
	m_iVehicleId = 0u;
	m_bBeganThisFrame = false;
	Hide();
}

// MainApp::Update_QuickSlotDrag가 호출해 커서 추적 뒤 취소·놓기 입력을 받는다.
bool_t Client::CQuickSlotDragView::Update()
{
	if (!Is_Carrying())
	{
		Hide();
		m_bEscapeDownLastFrame = false;
		return false;
	}

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	// 커서와 슬롯 조회가 모두 성공한 때 아이콘 중심을 문서 기준 커서 위치에 맞춘다.
	if (Router.Get_MousePosition(fRefWidth, fRefHeight, fMouseX, fMouseY) &&
		m_pView->Get_SlotRect(ICON_SLOT, fX, fY, fWidth, fHeight))
	{
		m_pView->Set_SlotPosition(ICON_SLOT, fMouseX - fWidth * 0.5f, fMouseY - fHeight * 0.5f);
		m_pView->Set_SlotVisible(ICON_SLOT, true);
	}
	// 운반 중 마우스는 UI가 점유해 gameplay 이동 명령을 막는다.
	Router.Claim_Mouse_This_Frame();

	const bool_t bEscapeDown = 0 != (CGameInstance::Get().Get_DIKeyState(DIK_ESCAPE) & 0x80);
	if (bEscapeDown && !m_bEscapeDownLastFrame)
	{
		m_bEscapeDownLastFrame = bEscapeDown;
		Cancel();
		return false;
	}
	m_bEscapeDownLastFrame = bEscapeDown;

	const bool_t bClicked = Router.Is_LeftClickEdge();
	if (m_bBeganThisFrame)
	{
		// 집어 든 직후의 첫 Update는 놓기 클릭으로 반환하지 않는다.
		m_bBeganThisFrame = false;
		return false;
	}
	return bClicked;
}

void Client::CQuickSlotDragView::Hide()
{
	m_pView->Set_SlotVisible(ICON_SLOT, false);
}
