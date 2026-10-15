#pragma once

// 공유 CHARACTER_CLASS_ID를 Client 표현 spec에 대응시킨다.
#include "Network/PacketType.h"

// 선택 가능한 일곱 직업의 고정 순서를 보관한다.
#include <array>

//Server의 character class id를 client의 실제 생성 설정으로 변환한다.
//LanceMaster-> Spec LanceMaster
//GUNSLINGER -> Spec Gunslinger
//server가 알면 안되고 client catalog의 책임이다.

namespace Client
{
	struct CHARACTER_SPEC;

	// 정적 표현 설정의 주소만 반환하는 조회 경계다. Character나 모델을 생성·소유하지 않는다.
	class CCharacterCatalog final
	{
	public:
		// 선택 순서와 Loader의 진입 자원 준비가 공유하는 직업 목록이다.
		static constexpr std::array<LostArk::Shared::CHARACTER_CLASS_ID, 7> CHARACTER_SELECT_CLASSES =
		{
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD,
			LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT
		};

		//class enum을 받아 해당 CHARACTER_SPEC을 반환한다.
		// 성공 결과는 프로세스 수명의 불변 spec 차용 포인터다. 미지원 직업과 END는 nullptr다.
		static const CHARACTER_SPEC* Find_Spec(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass);
		// 광기 변신의 clown 표현 설정이다. 고유 직업은 없으며 Character descriptor가 원래 직업을 제공한다.
		static const CHARACTER_SPEC* Find_ClownSpec();
	};
}