#include "UIWindowDrag.h"

#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <algorithm>

bool_t Client::CUIWindowDrag::Update(CUILayoutRuntime& View, const vector<string>& SlotIds,
	const f32_t fHandleX, const f32_t fHandleY, const f32_t fHandleWidth, const f32_t fHandleHeight,
	const char* pExcludeSlotId)
{
	Clamp_ToScreen(View, SlotIds);
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = View.Get_ResolutionWidth();
	const f32_t fRefHeight = View.Get_ResolutionHeight();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	if (!Router.Get_MousePosition(fRefWidth, fRefHeight, fMouseX, fMouseY))
		return m_bDragging;

	if (!m_bDragging)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (nullptr != pExcludeSlotId && View.Get_SlotRect(pExcludeSlotId, fX, fY, fWidth, fHeight) &&
			Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
			return false;
		if (Router.Is_Clicked(fHandleX, fHandleY, fHandleWidth, fHandleHeight, fRefWidth, fRefHeight))
		{
			m_bDragging = true;
			m_fLastMouseX = fMouseX;
			m_fLastMouseY = fMouseY;
		}
		return m_bDragging;
	}

	Router.Claim_Mouse_This_Frame();
	if (!Router.Is_LeftDown())
	{
		m_bDragging = false;
		return false;
	}
	const f32_t fDeltaX = fMouseX - m_fLastMouseX;
	const f32_t fDeltaY = fMouseY - m_fLastMouseY;
	m_fLastMouseX = fMouseX;
	m_fLastMouseY = fMouseY;
	if (0.f != fDeltaX || 0.f != fDeltaY)
		Move_All(View, SlotIds, fDeltaX, fDeltaY);
	return true;
}

void Client::CUIWindowDrag::Move_All(CUILayoutRuntime& View, const vector<string>& SlotIds,
	const f32_t fDeltaX, const f32_t fDeltaY)
{
	for (const string& strId : SlotIds)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (View.Get_SlotRect(strId, fX, fY, fWidth, fHeight))
			View.Set_SlotPosition(strId, fX + fDeltaX, fY + fDeltaY);
	}
}

void Client::CUIWindowDrag::Clamp_ToScreen(CUILayoutRuntime& View, const vector<string>& SlotIds)
{
	/* Bounds of the whole document (hidden slots included -- a hidden list row still belongs to
	the panel's footprint), so no part of the window can leave the reference resolution. */
	f32_t fMinX = 0.f, fMinY = 0.f, fMaxX = 0.f, fMaxY = 0.f;
	bool_t bAny = false;
	for (const string& strId : SlotIds)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!View.Get_SlotRect(strId, fX, fY, fWidth, fHeight))
			continue;
		if (!bAny)
		{
			fMinX = fX; fMinY = fY; fMaxX = fX + fWidth; fMaxY = fY + fHeight;
			bAny = true;
			continue;
		}
		fMinX = (std::min)(fMinX, fX);
		fMinY = (std::min)(fMinY, fY);
		fMaxX = (std::max)(fMaxX, fX + fWidth);
		fMaxY = (std::max)(fMaxY, fY + fHeight);
	}
	if (!bAny)
		return;
	const f32_t fRefWidth = View.Get_ResolutionWidth();
	const f32_t fRefHeight = View.Get_ResolutionHeight();
	f32_t fDeltaX = 0.f, fDeltaY = 0.f;
	if (fMinX < 0.f) fDeltaX = -fMinX;
	else if (fMaxX > fRefWidth) fDeltaX = fRefWidth - fMaxX;
	if (fMinY < 0.f) fDeltaY = -fMinY;
	else if (fMaxY > fRefHeight) fDeltaY = fRefHeight - fMaxY;
	if (0.f != fDeltaX || 0.f != fDeltaY)
		Move_All(View, SlotIds, fDeltaX, fDeltaY);
}
