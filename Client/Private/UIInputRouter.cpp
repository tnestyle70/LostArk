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
	/* Whole notches only (WHEEL_DELTA = 120); a remainder from a high-resolution wheel carries
	over to the next frame. */
	m_iWheelNotchesThisFrame = m_iWheelDeltaPending / WHEEL_DELTA;
	m_iWheelDeltaPending -= m_iWheelNotchesThisFrame * WHEEL_DELTA;
}

bool_t Client::CUIInputRouter::Is_Hovered(
	f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
	f32_t fRefWidth, f32_t fRefHeight) const
{
	if (m_bCinematicSuppressed) return false;
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

	return Is_LeftClickEdge();
}

bool_t Client::CUIInputRouter::Is_LeftClickEdge() const
{
	return !m_bCinematicSuppressed && !m_bLeftAwaitRelease && m_bLeftDownThisFrame && !m_bLeftDownLastFrame;
}

bool_t Client::CUIInputRouter::Is_RightClickEdge() const
{
	return !m_bCinematicSuppressed && m_bRightDownThisFrame && !m_bRightDownLastFrame;
}

bool_t Client::CUIInputRouter::Get_MousePosition(
	f32_t fRefWidth, f32_t fRefHeight, f32_t& outX, f32_t& outY) const
{
	if (m_bCinematicSuppressed) return false;
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
	if (m_bCinematicSuppressed) return false;
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

void Client::CUIInputRouter::Set_TopWindowRect(
	f32_t fScreenX, f32_t fScreenY, f32_t fScreenWidth, f32_t fScreenHeight)
{
	m_bHasTopWindow = true;
	m_fTopWindowX = fScreenX;
	m_fTopWindowY = fScreenY;
	m_fTopWindowWidth = fScreenWidth;
	m_fTopWindowHeight = fScreenHeight;
	/* Text under this window is hidden by CUITextOcclusion (CMainApp registers every open
	   window with its layer), not by a clip set here. */
}

bool_t Client::CUIInputRouter::Is_UnderTopWindow(f32_t fScreenX, f32_t fScreenY) const
{
	return m_bHasTopWindow &&
		fScreenX >= m_fTopWindowX && fScreenX < m_fTopWindowX + m_fTopWindowWidth &&
		fScreenY >= m_fTopWindowY && fScreenY < m_fTopWindowY + m_fTopWindowHeight;
}

void Client::CUIInputRouter::Start_TextInput()
{
	m_bTextInputActive = true;
	m_TypedChars.clear();
}

void Client::CUIInputRouter::Stop_TextInput()
{
	m_bTextInputActive = false;
	m_TypedChars.clear();
}

void Client::CUIInputRouter::On_Char(wchar_t ch)
{
	if (!m_bTextInputActive || m_bCinematicSuppressed)
		return;
	/* Bounded so a frame stall can't grow the queue without limit -- a normal frame drains it. */
	if (m_TypedChars.size() < 256)
		m_TypedChars.push_back(ch);
}

void Client::CUIInputRouter::On_MouseWheel(const int32_t iWheelDelta)
{
	m_iWheelDeltaPending += iWheelDelta;
}

wstring_t Client::CUIInputRouter::Take_TypedChars()
{
	wstring_t typed = std::move(m_TypedChars);
	m_TypedChars.clear();
	return typed;
}

void Client::CUIInputRouter::End_Frame()
{
	if (m_bCinematicSuppressed)
		CGameInstance::Get().SetInputBlocked(true, true);
	else if (m_bLeftAwaitRelease || m_bMouseClaimedThisFrame)
		CGameInstance::Get().SetInputBlocked(false, true);

	m_bMouseClaimedLastFrame = m_bMouseClaimedThisFrame;
	m_bHasTopWindow = false;
	CGameInstance::Get().Clear_TextClipOutRect();
	if (!m_bLeftDownThisFrame) m_bLeftAwaitRelease = false;
	m_bLeftDownLastFrame = m_bLeftDownThisFrame;
	m_bRightDownLastFrame = m_bRightDownThisFrame;
}
