#include "CharacterCatalog.h"

#include "Logic_Artist.h"
#include "Logic_DimensionMaster.h"
#include "Logic_GunSlinger.h"
#include "Logic_LanceMaster.h"
#include "Logic_Slayer.h"

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
	case LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER:
		return &Spec_GunSlinger;
	case LostArk::Shared::CHARACTER_CLASS_ID::SLAYER:
		return &Spec_Slayer;
	case LostArk::Shared::CHARACTER_CLASS_ID::ARTIST:
		return &Spec_Artist;
	case LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER:
		return &Spec_DimensionMaster;
	//case LostArk::Shared::CHARACTER_CLASS_ID::DESTROYER:
	//	break;
	case LostArk::Shared::CHARACTER_CLASS_ID::END:
		break;
	default:
		break;
	}

	return nullptr;
}
