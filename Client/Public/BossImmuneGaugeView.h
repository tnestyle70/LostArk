#pragma once

// Client 이름 공간과 Level 인덱스의 uint32_t를 사용한다.
#include "Client_Defines.h"
// DX11 ComPtr와 게이지 좌표·시간의 공통 타입을 사용한다.
#include "Engine_Defines.h"
// 입력으로 받는 HUD_BOSS_STATE의 위치·패턴 진행 상태를 사용한다.
#include "CombatHUDViewModel.h"

// 레이아웃은 unique_ptr로 단독 소유하고 슬롯 ID·상대 위치는 string과 vector로 보관한다.
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

// 레이아웃 포인터만 선언하며 생성·조작은 CPP의 전체 정의를 사용한다.
class CUILayoutRuntime;

// 전달받은 보스의 패턴 진행도·목표량을 남은 게이지로 표시한다. 목표량이 0이면 숨긴다.
// 복제된 보스 위치를 화면에 투영하며, 위치가 없는 미리보기는 JSON의 기준점을 쓴다.
class CBossImmuneGaugeView final
{
public:
	// 생성 시 레이아웃을 숨기고 슬롯의 기준점 대비 간격을 저장한다.
	CBossImmuneGaugeView(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex);
	// 소멸 시 레이아웃과 그 보유 참조를 정리한다. Layer의 sprite 참조는 별도다.
	~CBossImmuneGaugeView();

public:
	// 호출자가 정한 표시 허용 여부와 보스 상태로 위치·잔량을 갱신한다.
	void Update(f32_t fTimeDelta, const HUD_BOSS_STATE& Boss, bool_t bAllowed);
	// 게이지를 숨기되 슬롯 위치와 진행도 입력은 새로 계산하지 않는다.
	void Hide();

private:
	// Immune_Anchor에서 각 이미지까지의 문서 좌표 간격을 한 번 저장한다.
	void Capture_SlotOffsets();

private:
	struct SLOT_OFFSET
	{
		string	strId;
		f32_t	fDx = 0.f;
		f32_t	fDy = 0.f;
	};

	// 레이아웃과 슬롯 간격을 소유한다. 보스 객체나 Server 전투 상태는 소유하지 않는다.
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<SLOT_OFFSET>				m_SlotOffsets;
	f32_t							m_fAnchorX = 0.f;
	f32_t							m_fAnchorY = 0.f;
	bool_t							m_bVisible = false;
};

NS_END
