#pragma once

#include "Engine_Macro.h"

NS_BEGIN(Client)

//제품 UI가 요청하는 소리의 의미. 저장 ID나 Sound Manager의 자원키가 아니다.
enum class UI_SOUND_CUE
{
	BUTTON_CLICK,
	WINDOW_OPEN,
	WINDOW_CLOSE,
	POPUP_REQUEST
};

//Engine 초기화 뒤 UI 주 스레드에서 호출. WAV 선택만 맡고 재생 자원은 Engine이
//소유한다. 재생 HRESULT로 버튼, 창 상태를 바꾸지 않으며 기존 경로 처리의 
//예외 계약을 유지한다. 
void Play_UISound(UI_SOUND_CUE eCue);
void Prepare_UISounds();

NS_END