#pragma once

#include "Network/PacketType.h"

//Server의 character class id를 client의 실제 생성 설정으로 변환한다.
//LanceMaster-> Spec LanceMaster
//GUNSLINGER -> Spec Gunslinger
//server가 알면 안되고 client catalog의 책임이다.

namespace Client
{
	struct CHARACTER_SPEC;

	class CCharacterCatalog final
	{
	public:
		//class enum을 받아 해당 CHARACTER_SPEC을 반환한다.
		static const CHARACTER_SPEC* Find_Spec(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass);
		/* The KoukuSaydon clown avatar: the colourless Saydon body every class
		wears while PLAYER_MADNESS_FORM::CLOWN. It carries no class of its own;
		the Character desc supplies the wearer's class. */
		static const CHARACTER_SPEC* Find_ClownSpec();
	};
}