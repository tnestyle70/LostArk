// 각 창이 보유한 드래그 상태로 지정한 레이아웃 슬롯을 함께 이동한다.
#include "UIWindowDrag.h"

// 버튼·커서 상태를 읽고 UILayoutRuntime으로 슬롯들의 위치를 바꾼다.
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

// 슬롯 사각형의 전체 최소·최대 경계를 계산한다.
#include <algorithm>

// Chat·Party 등 창의 프레임 처리에서 호출한다. 커서 조회 실패는 기존 드래그 상태를 유지한다.
bool_t Client::CUIWindowDrag::Update(CUILayoutRuntime& View, const vector<string>& SlotIds,
	const f32_t fHandleX, const f32_t fHandleY, const f32_t fHandleWidth, const f32_t fHandleHeight,
	const char* pExcludeSlotId)
{
	// 이번 이동 전에 창 영역을 화면 쪽으로 보정한다.
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
		// 닫기 버튼 같은 제외 슬롯 위에서는 제목 영역 드래그를 시작하지 않는다.
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
	// 문서 기준 이전 커서와의 차이만 적용한다. 화면 밖 보정은 다음 Update 시작에서 다시 수행한다.
	const f32_t fDeltaX = fMouseX - m_fLastMouseX;
	const f32_t fDeltaY = fMouseY - m_fLastMouseY;
	m_fLastMouseX = fMouseX;
	m_fLastMouseY = fMouseY;
	if (0.f != fDeltaX || 0.f != fDeltaY)
		Move_All(View, SlotIds, fDeltaX, fDeltaY);
	return true;
}

// 지정 ID마다 같은 문서 좌표 증분을 더한다. 없는 슬롯은 건너뛴다.
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

// 모든 지정 슬롯의 경계를 모아 화면 밖으로 벗어난 축을 함께 옮긴다.
void Client::CUIWindowDrag::Clamp_ToScreen(CUILayoutRuntime& View, const vector<string>& SlotIds)
{
	// 숨긴 슬롯도 창 영역에 포함해 벗어난 변을 찾는다. 화면보다 큰 창 전체가 들어가는 것은 보장하지 않는다.
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
