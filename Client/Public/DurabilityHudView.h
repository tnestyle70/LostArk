#pragma once

// Client 이름 공간과 부위 enum의 uint8_t를 사용한다.
#include "Client_Defines.h"
// DX11 ComPtr, unique_ptr와 ETOUI·f32_t 등 엔진 공통 선언을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Client)

// 헤더에는 소유 포인터만 선언하고 CPP에서 슬롯 API를 사용한다.
class CUILayoutRuntime;

// MainApp이 장비 내구도 snapshot을 부위별 상태로 바꿔 전달하는 HUD다.
// DurabilityUI.json의 sprite로 실루엣과 손상·파괴 이미지를 표시한다.
class CDurabilityHudView final
{
public:
	enum class PART : uint8_t
	{
		WEAPON, HELMET, TOP, GLOVES, BOTTOMS, SHOULDER, LIFE_TOOL, BRACER, END
	};
	enum class PART_STATE : uint8_t { NORMAL, DAMAGED, DESTROYED };

	// MainApp 초기화에서 생성하고 STATIC Level의 Layer_UI에 이미지를 준비한다.
	CDurabilityHudView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	// 소유 레이아웃과 그 보유 참조를 정리한다. Layer의 sprite 참조는 별도다.
	~CDurabilityHudView();

public:
	// 실루엣을 보이고 각 부위의 최신 상태를 적용한다. 전체 표시 여부는 MainApp이 결정한다.
	void Update();
	// STATIC Level에 남는 모든 sprite를 숨긴다. 저장한 부위 상태는 유지한다.
	void Hide();

	// MainApp이 장착 장비의 0%를 파괴, 30% 미만을 손상으로 분류해 전달한다.
	void Set_PartState(PART ePart, PART_STATE eState);
	// 팔찌 사용 여부가 바뀌면 실루엣과 장갑 이미지를 함께 바꾼다.
	void Set_BracerEnabled(bool_t bEnabled);

private:
	// 정상 부위는 숨기고 손상·파괴 부위는 해당 이미지와 원본 비율을 적용한다.
	void Apply_Part(PART ePart);

private:
	// 레이아웃은 단독 소유하고 부위 상태는 값으로 보관한다. 장비 객체를 소유하지 않는다.
	unique_ptr<CUILayoutRuntime> m_pView;
	PART_STATE m_PartStates[ETOUI(PART::END)] = {};
	// 처음 읽은 저작 사각형의 배율이다. 0이면 아직 유효한 크기를 얻지 못했다.
	f32_t m_PartAuthoredScales[ETOUI(PART::END)] = {};
	bool_t m_bBracerEnabled = true;
};

NS_END
