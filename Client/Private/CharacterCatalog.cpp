#include "CharacterCatalog.h"

// clown 본체·망치의 prototype tag와 소켓 상수를 사용한다.
#include "KoukuSaydonPresentationAssetService.h"
// 도화가의 정적 CHARACTER_SPEC 주소를 반환한다.
#include "Logic_Artist.h"
// 차원술사의 정적 CHARACTER_SPEC 주소를 반환한다.
#include "Logic_DimensionMaster.h"
// 건슬링어의 정적 CHARACTER_SPEC 주소를 반환한다.
#include "Logic_GunSlinger.h"
// 창술사의 정적 CHARACTER_SPEC 주소를 반환한다.
#include "Logic_LanceMaster.h"
// 슬레이어의 정적 CHARACTER_SPEC 주소를 반환한다.
#include "Logic_Slayer.h"
// 워로드의 정적 CHARACTER_SPEC 주소를 반환한다.
#include "Logic_Warlord.h"
// 가디언나이트의 정적 CHARACTER_SPEC 주소를 반환한다.
#include "Logic_GuardianKnight.h"

namespace
{
	using namespace Client;

	// 광기 clown 변신용 본체·재질·clip 설정이다. 원래 플레이어 직업은 Character descriptor에 남는다.
	// 기존 무기 Part 경로로 clown의 오른손 소켓에 망치를 붙인다.
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  KOUKU_CLOWN_HAMMER_PROTOTYPE_TAG, KOUKU_CLOWN_HAMMER_SOCKET_BONE },
	};

	const CHARACTER_SPEC Spec_KoukuSaydonClown =
	{
		"KoukuSaydonClown",
		LostArk::Shared::CHARACTER_CLASS_ID::END,

		KOUKU_CLOWN_BODY_PROTOTYPE_TAG,
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		0u,
		// 이 설정은 본체 hair 숨김 mask를 사용하지 않는다.
		0u,

		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		Weapons,
		static_cast<uint32_t>(size(Weapons)),

		nullptr,
		0u,

		// idle/run만 지정하고 나머지 공통 상태 clip은 nullptr로 둔다.
		{
			"rpcz00p_idle_battle_1",
			"rpcz00p_run_battle_1",
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

// 공통 clown 표현 spec의 주소를 반환한다. 모델 생성이나 원래 직업 변경은 하지 않는다.
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
	// 현재 switch는 일곱 직업을 지원한다. 위의 네 직업 메모는 원문으로 보존한다.
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
	case LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT:
		return &Spec_GuardianKnight;
	//case LostArk::Shared::CHARACTER_CLASS_ID::DESTROYER:
	//	break;
	case LostArk::Shared::CHARACTER_CLASS_ID::END:
		break;
	default:
		break;
	}

	return nullptr;
}
