#include "Logic_Slayer.h"

// 표현 갱신 인자의 CCharacter 정의를 연결한다. 현재 본문에서는 그 멤버에 접근하지 않는다.
#include "Character.h"

namespace
{
	// 대검을 본체의 b_weapon_rhand에 붙인다. 다른 직업의 소켓 이름을 공통으로 가정하지 않는다.
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_Slayer_Weapon"), "b_weapon_rhand" },
	};

	// Part tag 순서로 본체→장비→무기 갱신 순서를 유지한다.
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_Slayer_Arm"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HANDS },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_Slayer_Helmet"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_Slayer_Lower"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::LOWER },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_Slayer_Shoulder"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::SHOULDER },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_Slayer_Upper"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::UPPER },
	};

	// 장비가 덮는 본체 submesh 4·5·6·7을 숨긴다. 이 mask는 해당 Model의 mesh 순서에 의존한다.
	constexpr uint32_t COVERED_BY_ARMOUR =
		(1u << 4) | (1u << 5) | (1u << 6) | (1u << 7);

	// 본체 머리카락은 0번 submesh지만 별도의 hair 숨김 비트는 지정하지 않는다.
	// BAKED_HAIR가 0이므로 이 spec의 hair mask로 본체 머리를 숨기지 않는다.
	constexpr uint32_t BAKED_HAIR = 0u;

	// 로컬 CCharacter가 spec의 factory를 호출해 logic을 단독 소유한다. Character 정리 시 함께 소멸한다.
	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_Slayer>();
	}
}

NS_BEGIN(Client)

// 현재 클래스별 추가 갱신은 없다. 입력·상태를 변경하지 않고 공통 CCharacter 표현을 사용한다.
void CLogic_Slayer::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

// catalog가 extern으로 참조하는 프로세스 수명의 직업 설정이다.
// CCharacter가 Part·clip·선택 기능을 조립할 때 읽는 불변 설정이다. 아래 배열·문자열 포인터는 정적 수명이다.
const CHARACTER_SPEC Spec_Slayer =
{
	"Slayer",
	LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,

	TEXT("Prototype_Component_Model_Slayer"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,
	BAKED_HAIR,

	// 대검 Part는 static mesh 셰이더를 사용하고 본체의 소켓 행렬로 배치된다.
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	// CHARACTER_ANIM 순서에 wbk 계열 clip 이름을 대응시킨다.
	{
		"wbk_idle_battle_1",
		"wbk_run_battle_1",
		"wbk_dmg_idle_1",
		"wbk_dead_1",
		"wbk_knockdown",
		"wbk_knockdown_land",
		"wbk_down",
		"wbk_standup_1",
		// 이 spec에는 ESTHER_CAST clip을 지정하지 않았다.
		nullptr,
		nullptr, // CUSTOMIZING_IDLE
		"wbk_abn_fear_1",
		nullptr, // SQUAREHOLE_SONG
	},

	&Create_Logic,
};

NS_END
