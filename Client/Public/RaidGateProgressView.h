#pragma once

// Client 이름 공간과 공통 Level 선언을 사용한다.
#include "Client_Defines.h"
// 장치 ComPtr와 UI 수치·문자열 타입을 사용한다.
#include "Engine_Defines.h"

// 관문 번호와 완료 mask의 고정 크기 정수 타입을 사용한다.
#include <cstdint>
// 패널과 확인창 레이아웃을 각각 단독 소유한다.
#include <memory>
// Level에서 받은 레이드·제안자·알림 문구를 복사한다.
#include <string>

NS_BEGIN(Client)

class CUILayoutRuntime;

// Level이 전달한 레이드 이름·관문 상태·투표 창을 표시한다.
// 입력 결과는 INTENT로 돌려주며 Level이 권한을 검사해 Server 명령으로 제출한다.
class CRaidGateProgressView final
{
public:
	// LEAVE는 퇴장 투표 확인창, EXIT는 최종 관문 뒤 직접 복귀 의도다.
	enum class BUTTON { NONE, RESTART, LEAVE, PROGRESS, EXIT, ENTER_GATE3, ENTER_BINGO };
	enum class PROMPT { NONE, CONFIRM_ADVANCE, CONFIRM_RESTART, CONFIRM_EXIT, VOTE_ADVANCE, VOTE_RESTART, VOTE_EXIT, CONFIRM_ENTER_GATE3, VOTE_ENTER_GATE3 };
	enum class INTENT { NONE, PROPOSE_ADVANCE, PROPOSE_RESTART, PROPOSE_EXIT, EXIT, ACCEPT, DECLINE, PROPOSE_ENTER_GATE3 };

public:
	// Level 초기화에서 패널·확인창 레이아웃을 만들고 버튼·확인창을 숨긴다.
	void Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);
	// 패널 이름·난이도와 관문 수를 복사한다. 관문 수는 최대 3으로 제한한다.
	void Set_Raid(const wstring_t& strName, const wstring_t& strDifficulty, uint8_t iGateCount);
	// 현재 관문은 1부터 시작하며 0은 미지정이다. cleared mask의 각 비트는 완료 관문이다.
	void Set_Progress(uint8_t iCurrentGate, uint8_t iClearedMask);
	// 버튼 종류·활성 상태를 적용한다. 비활성 버튼도 보이지만 클릭은 처리하지 않는다.
	void Set_Button(BUTTON eButton, bool_t bEnabled);
	// Level의 투표 갱신 또는 버튼 입력이 호출한다. 확인창 종류와 제안자 문구를 보관한다.
	void Open_Prompt(PROMPT eKind, const wstring_t& strProposerName);
	// 확인창 상태를 NONE으로 바꾸고 이미지를 숨긴다. 투표 취소 명령을 보내지는 않는다.
	void Close_Prompt();
	PROMPT Get_Prompt() const { return m_ePrompt; }
	// 전달한 문구를 지정한 초 동안 화면 위쪽에 표시한다.
	void Show_Notice(const wstring_t& strText, f32_t fSeconds);
	// 버튼 위 또는 열린 확인창의 포인터를 소비하고 이번 프레임의 의도를 반환한다. EXIT 외 버튼은 확인창을 먼저 연다.
	INTENT Update(f32_t fTimeDelta);
	// Level의 Render에서 호출한다. 패널 글자는 HUD, 확인창 글자는 MODAL 계층이다.
	void Render_Text() const;
	// 집결 시간(초)을 올림한 카운트와 공통 이동 안내를 HUD에 표시한다.
	static void Render_AssemblyCountdown(f32_t secondsLeft, const wchar_t* message = nullptr);

private:
	// 확인창의 세 이미지 슬롯에 같은 표시 상태를 적용한다.
	void Set_PromptVisible(bool_t bVisible);
	// 확인/취소 입력을 제안·수락·거절 의도로 변환하고 눌린 창을 닫는다.
	INTENT Update_Prompt();
	// 활성 패널 버튼을 검사해 확인창을 열거나 EXIT 의도를 반환한다.
	INTENT Update_Button();
	const wchar_t* Button_Text() const;

private:
	// 두 레이아웃은 단독 소유한다. 소멸은 보유 참조만 정리하며 Layer의 sprite 참조는 별도다.
	unique_ptr<CUILayoutRuntime>	m_pWidget;
	unique_ptr<CUILayoutRuntime>	m_pPrompt;
	// Level이 공급한 표시값과 이 창의 hover·prompt 상태를 보관한다. Server 판정의 정본은 아니다.
	wstring_t						m_strName;
	wstring_t						m_strDifficulty;
	wstring_t						m_strProposer;
	wstring_t						m_strNotice;
	f32_t							m_fNoticeSeconds = 0.f;
	uint8_t							m_iGateCount = 0u;
	uint8_t							m_iCurrentGate = 0u;
	uint8_t							m_iClearedMask = 0u;
	BUTTON							m_eButton = BUTTON::NONE;
	bool_t							m_bButtonEnabled = false;
	bool_t							m_bButtonHovered = false;
	PROMPT							m_ePrompt = PROMPT::NONE;
};

NS_END
