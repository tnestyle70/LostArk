#include "UIInputRouter.h"

#include "GameInstance.h"

Client::CUIInputRouter& Client::CUIInputRouter::Get()
{
	static CUIInputRouter instance;
	return instance;
}

void Client::CUIInputRouter::Begin_Frame()
{
	m_bMouseClaimedThisFrame = false;
	m_bLeftDownThisFrame =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::LB) & 0x80);
	m_bRightDownThisFrame =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80);
}

bool_t Client::CUIInputRouter::Is_Hovered(
	f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
	f32_t fRefWidth, f32_t fRefHeight) const
{
	/* Same GetCursorPos + ScreenToClient pattern CPlayerController::Try_PickWorldRay already
	uses for its own world-ray picking, not ImGui::GetMousePos -- the whole point of this router
	is for the runtime UI path not to depend on ImGui. */
	::POINT cursor{};
	if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
		return false;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	if (vViewportSize.x <= 0.f || vViewportSize.y <= 0.f ||
		fRefWidth <= 0.f || fRefHeight <= 0.f)
	{
		return false;
	}

	const f32_t fScaleX = vViewportSize.x / fRefWidth;
	const f32_t fScaleY = vViewportSize.y / fRefHeight;
	const f32_t fLeft = fX * fScaleX;
	const f32_t fTop = fY * fScaleY;
	const f32_t fRight = fLeft + fWidth * fScaleX;
	const f32_t fBottom = fTop + fHeight * fScaleY;

	const f32_t fCursorX = static_cast<f32_t>(cursor.x);
	const f32_t fCursorY = static_cast<f32_t>(cursor.y);

	return fCursorX >= fLeft && fCursorX < fRight &&
		fCursorY >= fTop && fCursorY < fBottom;
}

bool_t Client::CUIInputRouter::Is_Clicked(
	f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
	f32_t fRefWidth, f32_t fRefHeight)
{
	if (!Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return false;

	Claim_Mouse_This_Frame();

	return m_bLeftDownThisFrame && !m_bLeftDownLastFrame;
}

bool_t Client::CUIInputRouter::Is_LeftClickEdge() const
{
	return m_bLeftDownThisFrame && !m_bLeftDownLastFrame;
}

bool_t Client::CUIInputRouter::Is_RightClickEdge() const
{
	return m_bRightDownThisFrame && !m_bRightDownLastFrame;
}

bool_t Client::CUIInputRouter::Get_MousePosition(
	f32_t fRefWidth, f32_t fRefHeight, f32_t& outX, f32_t& outY) const
{
	::POINT cursor{};
	if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
		return false;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	if (vViewportSize.x <= 0.f || vViewportSize.y <= 0.f ||
		fRefWidth <= 0.f || fRefHeight <= 0.f)
	{
		return false;
	}

	outX = static_cast<f32_t>(cursor.x) * fRefWidth / vViewportSize.x;
	outY = static_cast<f32_t>(cursor.y) * fRefHeight / vViewportSize.y;
	return true;
}

bool_t Client::CUIInputRouter::Get_ClientCursorPosition(f32_t& outX, f32_t& outY) const
{
	::POINT cursor{};
	if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
		return false;

	outX = static_cast<f32_t>(cursor.x);
	outY = static_cast<f32_t>(cursor.y);
	return true;
}

void Client::CUIInputRouter::Claim_Mouse_This_Frame()
{
	m_bMouseClaimedThisFrame = true;
}

void Client::CUIInputRouter::End_Frame()
{
	if (m_bMouseClaimedThisFrame)
		CGameInstance::Get().SetInputBlocked(false, true);

	m_bLeftDownLastFrame = m_bLeftDownThisFrame;
	m_bRightDownLastFrame = m_bRightDownThisFrame;
}
