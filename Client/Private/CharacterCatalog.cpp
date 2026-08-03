#include "CharacterCatalog.h"

#include "Logic_LanceMaster.h"

const CHARACTER_SPEC* Client::CCharacterCatalog::Find_Spec(
	LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	using LostArk::Shared::CHARACTER_CLASS_ID;
	//지금 lance master, gunslinger, slayer, artist 4명 띄울 수 있음
	//NPC들도 있으니 전부 동기화 시키는 것을 목표로 간다.
	switch (characterClass)
	{
	case LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER:
		return &Spec_LanceMaster;
	//case LostArk::Shared::CHARACTER_CLASS_ID::DESTROYER:
	//	break;
	case LostArk::Shared::CHARACTER_CLASS_ID::END:
		break;
	default:
		break;
	}

	return nullptr;
}
