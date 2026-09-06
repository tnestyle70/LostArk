#include "CharacterCatalog.h"

#include "KoukuSaydonPresentationAssetService.h"
#include "Logic_Artist.h"
#include "Logic_DimensionMaster.h"
#include "Logic_GunSlinger.h"
#include "Logic_LanceMaster.h"
#include "Logic_Slayer.h"
#include "Logic_Warlord.h"

namespace
{
	using namespace Client;

	/* The colourless Saydon body (MN_RPCT_03) as a playable avatar. It has no
	weapon or equipment, no class logic and no skill binding document, so a
	skill action keeps the pose it is in; IDLE/RUN come from the rpct00 clip
	set the body embeds. The class stays the wearer's through CHARACTER_DESC. */
	const CHARACTER_SPEC Spec_KoukuSaydonClown =
	{
		"KoukuSaydonClown",
		LostArk::Shared::CHARACTER_CLASS_ID::END,

		KOUKU_CLOWN_BODY_PROTOTYPE_TAG,
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		0u,

		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		nullptr,
		0u,

		nullptr,
		0u,

		/* IDLE, RUN, HIT, DEAD, KNOCKDOWN, KNOCKDOWN_LAND, DOWN_LOOP, STANDUP,
		ESTHER_CAST. Only locomotion is authored for the avatar today. */
		{
			"rpct00_idle_battle_1",
			"rpct00_run_battle_1",
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
		},

		nullptr,

		nullptr,
		0u,

		nullptr,
		0u,

		nullptr,
		0u,
	};
}

const CHARACTER_SPEC* Client::CCharacterCatalog::Find_ClownSpec()
{
	return &Spec_KoukuSaydonClown;
}

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
	case LostArk::Shared::CHARACTER_CLASS_ID::WARLORD:
		return &Spec_Warlord;
	//case LostArk::Shared::CHARACTER_CLASS_ID::DESTROYER:
	//	break;
	case LostArk::Shared::CHARACTER_CLASS_ID::END:
		break;
	default:
		break;
	}

	return nullptr;
}
