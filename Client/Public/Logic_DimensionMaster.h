#pragma once

// Client 이름 공간과 공통 수치 타입을 사용한다.
#include "Client_Defines.h"
// ICharacterLogic 표시 갱신 인터페이스를 상속하고 CHARACTER_SPEC을 외부에 선언한다.
#include "CharacterSpec.h"

NS_BEGIN(Client)

// 차원술사의 로컬 표현 갱신 경계다. 실제 조립 정보는 Spec_DimensionMaster이 제공한다.
// 현재 Update_Presentation은 비어 있고 gameplay 판정은 Server 경로가 담당한다.
class CLogic_DimensionMaster final : public ICharacterLogic
{
public:
	// CCharacter가 로컬 제어 객체에서 호출한다. 별도 멤버 상태·렌더 자원을 소유하지 않는다.
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) override;
};

extern const CHARACTER_SPEC Spec_DimensionMaster;

NS_END
