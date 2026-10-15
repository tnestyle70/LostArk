#pragma once

// Client 이름 공간과 생성 대상 Level의 uint32_t 인덱스를 사용한다.
#include "Client_Defines.h"
// DX11 ComPtr와 UI 좌표·시간의 공통 타입을 사용한다.
#include "Engine_Defines.h"
// 표시 여부·남은 초·경고 시간을 담은 HUD_DUNGEON_TIMER_STATE를 받는다.
#include "CombatHUDViewModel.h"

// 레이아웃을 unique_ptr로 단독 소유한다.
#include <memory>

NS_BEGIN(Client)

// 헤더는 포인터만 선언하고 CPP에서 슬롯 생성·조작 함수를 사용한다.
class CUILayoutRuntime;

// MainApp이 소유하는 쿠크 미니게임 제한 시간 표시다. 전달된 초를 읽고 게임 시간을 진행시키지는 않는다.
// DungeonTimer_Layout.json의 문양 슬롯은 Layer_UI가, 남은 시간 글자는 Render가 그린다.
class CDungeonTimerView final
{
public:
	// MainApp 초기화에서 생성해 문양의 위치를 읽고 첫 상태 수신 전에는 숨긴다.
	CDungeonTimerView(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex);
	// 멤버 레이아웃과 그 보유 참조를 정리한다. Layer의 sprite 참조는 별도다.
	~CDungeonTimerView();

public:
	// MainApp::Update가 전달한 유효한 초를 보관한다. delta는 레이아웃 애니메이션에만 쓴다.
	void Update(f32_t fTimeDelta, const HUD_DUNGEON_TIMER_STATE& State);
	// MainApp의 HUD 글자 패스에서 호출한다. 문양 이미지의 렌더는 Layer에 등록된 sprite가 맡는다.
	void Render() const;
	// 이미지와 뒤따르는 글자 출력을 함께 숨긴다.
	void Hide();

private:
	// 문양 이미지와 슬롯 정보를 소유하고 외부의 게임 타이머는 참조하지 않는다.
	unique_ptr<CUILayoutRuntime>	m_pView;
	// 문서 기준 문양 사각형이다. 글자 위치·크기도 이 폭에 비례한다.
	f32_t							m_fEmblemX = 0.f;
	f32_t							m_fEmblemY = 0.f;
	f32_t							m_fEmblemWidth = 0.f;
	f32_t							m_fEmblemHeight = 0.f;
	bool_t							m_bSlotFound = false;
	bool_t							m_bVisible = false;
	// Render가 읽는 최근 입력 초와 경고 전환 기준이다.
	f32_t							m_fSeconds = 0.f;
	f32_t							m_fWarningSeconds = 10.f;
};

NS_END
