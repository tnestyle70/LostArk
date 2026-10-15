#pragma once

// Client 이름 공간과 Level 공통 선언을 사용한다.
#include "Client_Defines.h"
// 장치 ComPtr와 기본 UI 타입을 사용한다.
#include "Engine_Defines.h"

// 메뉴 레이아웃을 단독 소유한다.
#include <memory>

NS_BEGIN(Client)

class CUILayoutRuntime;

// 월드 화면 아래의 환경설정·캐릭터 선택 버튼을 표시하고 클릭 의도를 반환한다.
// Level이 의도를 실행하며 이 view는 네트워크 송신이나 Level 전환을 수행하지 않는다.
class CSystemMenuButtonsView final
{
public:
	enum class INTENT { NONE, OPEN_OPTIONS, RETURN_TO_CHARACTER_SELECT };

public:
	// Level 초기화에서 메뉴 레이아웃을 해당 Level의 Layer_UI에 만든다.
	void Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);
	// 표시 상태를 갱신하고 이번 프레임의 클릭 의도를 반환한다. 숨겨졌으면 입력을 읽지 않는다.
	INTENT Update(bool_t bVisible);
	// 이번 Update에서 다른 UI에 선점되지 않은 아이콘 위에 포인터가 있었는지 반환한다. Level이 gameplay 입력 차단에 쓴다.
	bool_t Is_PointerOver() const { return m_bPointerOver; }
	// Level의 텍스트 pass에서 각 아이콘 아래 문구를 그린다.
	void Render_Text() const;

private:
	// 레이아웃 소멸은 보유 참조만 정리한다. Layer의 sprite 참조는 별도다.
	unique_ptr<CUILayoutRuntime> m_pView;
	bool_t m_bShown = true;
	bool_t m_bPointerOver = false;
};

NS_END
