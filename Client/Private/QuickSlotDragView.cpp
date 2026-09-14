/* WinSock2 then dinput, ahead of everything else -- the order PlayerController.cpp and
VehicleWindowView.cpp already use for the DIK_* constants. */
#include <WinSock2.h>
#include <dinput.h>

#include "QuickSlotDragView.h"

#include "GameInstance.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

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
	/* Same reason as the other LEVEL::STATIC documents: sprites are visible from construction. */
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

void Client::CQuickSlotDragView::Cancel()
{
	m_ePayload = PAYLOAD::NONE;
	m_strItemId.clear();
	m_iVehicleId = 0u;
	m_bBeganThisFrame = false;
	Hide();
}

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
	if (Router.Get_MousePosition(fRefWidth, fRefHeight, fMouseX, fMouseY) &&
		m_pView->Get_SlotRect(ICON_SLOT, fX, fY, fWidth, fHeight))
	{
		m_pView->Set_SlotPosition(ICON_SLOT, fMouseX - fWidth * 0.5f, fMouseY - fHeight * 0.5f);
		m_pView->Set_SlotVisible(ICON_SLOT, true);
	}
	/* A click while carrying is for the carry, never a gameplay move command. */
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
		/* The pick-up click itself. */
		m_bBeganThisFrame = false;
		return false;
	}
	return bClicked;
}

void Client::CQuickSlotDragView::Hide()
{
	m_pView->Set_SlotVisible(ICON_SLOT, false);
}
