#pragma once

// Client namespace와 Engine의 bool_t·f32_t를 사용한다.
#include "Client_Defines.h"
#include "Engine_Defines.h"

// 이동할 슬롯 ID 문자열 목록을 받는다. 창과 슬롯 객체를 소유하지 않는다.
#include <string>
#include <vector>

NS_BEGIN(Client)

// 참조로만 받으므로 여기서는 이름만 선언하고 CPP에서 슬롯 API를 읽는다.
class CUILayoutRuntime;

// Chat·Party 등 각 창이 하나씩 보유하는 드래그 상태다. 제목 영역을 잡으면 지정 슬롯들을 함께 옮긴다.
class CUIWindowDrag final
{
public:
	// 창이 열려 있는 동안 호출한다. 제목 영역과 제외 버튼은 문서 기준 좌표다.
	// 드래그 중 true를 돌려 호출자가 다른 클릭 처리를 건너뛸 수 있다.
	bool_t Update(CUILayoutRuntime& View, const vector<string>& SlotIds,
		f32_t fHandleX, f32_t fHandleY, f32_t fHandleWidth, f32_t fHandleHeight,
		const char* pExcludeSlotId = nullptr);
	// 창을 닫거나 숨길 때 진행 중인 드래그를 취소한다.
	void Reset() { m_bDragging = false; }
	bool_t Is_Dragging() const { return m_bDragging; }

private:
	// 지정한 슬롯들의 문서 좌표에 같은 커서 이동량을 더한다.
	static void Move_All(CUILayoutRuntime& View, const vector<string>& SlotIds,
		f32_t fDeltaX, f32_t fDeltaY);
	// 현재 슬롯 전체 경계가 화면 밖이면 함께 옮긴다. Update의 이번 마우스 이동 전에 수행한다.
	static void Clamp_ToScreen(CUILayoutRuntime& View, const vector<string>& SlotIds);

private:
	// 각 창의 드래그 진행 여부와 문서 기준 이전 커서 좌표다.
	bool_t m_bDragging = false;
	f32_t m_fLastMouseX = 0.f;
	f32_t m_fLastMouseY = 0.f;
};

NS_END
