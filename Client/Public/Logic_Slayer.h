#pragma once

// Client 이름 공간과 공통 수치 타입을 사용한다.
#include "Client_Defines.h"
// ICharacterLogic 표시 갱신 인터페이스를 상속하고 CHARACTER_SPEC을 외부에 선언한다.
#include "CharacterSpec.h"

NS_BEGIN(Client)

// Slayer의 로컬 표현 갱신 경계와 직업 조립 설정이다. 담당 JS.
// 현재 logic 본문은 비어 있으며 Part·clip·stance 설정은 CCharacter 공통 경로가 소비한다.
class CLogic_Slayer final : public ICharacterLogic
{
public:
	// CCharacter가 로컬 제어 객체에서 호출한다. 별도 멤버 상태·렌더 자원을 소유하지 않는다.
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) override;
};

// 모델 tag·clip·Part 배열은 CPP의 상수 정의에 두고 catalog에는 extern으로 제공한다.
extern const CHARACTER_SPEC Spec_Slayer;

NS_END
