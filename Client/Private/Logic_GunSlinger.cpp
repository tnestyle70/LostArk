#include "Logic_GunSlinger.h"

// 표현 갱신 인자의 CCharacter 정의를 연결한다. 현재 본문에서는 그 멤버에 접근하지 않는다.
#include "Character.h"

namespace
{
	// 같은 handgun 모델을 오른손 b_wp_1과 왼손 b_wp_2에 각각 붙인다. 소켓 존재는 Part 초기화에서 검사한다.
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_GunSlinger_Weapon"), "b_wp_1" },
		{ TEXT("Part_91_Weapon_L"),
		  TEXT("Prototype_Component_Model_GunSlinger_Weapon"), "b_wp_2" },
	};

	// Part tag 순서로 본체가 먼저 갱신되고 장비 다음에 무기가 갱신된다.
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_GunSlinger_Arm"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HANDS },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_GunSlinger_Helmet"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_GunSlinger_Lower"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::LOWER },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_GunSlinger_Shoulder"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::SHOULDER },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_GunSlinger_Upper"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::UPPER },
	};

	// 장비가 덮는 본체 upper(3)·arm(5)·lower(6)를 숨긴다. 모델 submesh 순서가 바뀌면 함께 확인해야 한다.
	constexpr uint32_t COVERED_BY_ARMOUR = (1u << 3) | (1u << 5) | (1u << 6);

	// 본체 hair submesh는 4번이지만 이 설정은 그 비트를 사용하지 않는다.
	// 현재 BAKED_HAIR는 0이므로 이 spec의 머리카락 mask로 본체 머리를 숨기지 않는다.
	constexpr uint32_t BAKED_HAIR = 0u;

	// 로컬 CCharacter가 spec의 factory를 호출해 logic을 단독 소유한다. Character 정리 시 함께 소멸한다.
	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_GunSlinger>();
	}
}

NS_BEGIN(Client)

// 현재 클래스별 추가 갱신은 없다. 입력·상태를 변경하지 않고 공통 CCharacter 표현을 사용한다.
void CLogic_GunSlinger::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

// 헤더의 extern으로 catalog가 참조하는 프로세스 수명의 직업 설정이다.
// CCharacter가 Part·clip·선택 기능을 조립할 때 읽는 불변 설정이다. 아래 배열·문자열 포인터는 정적 수명이다.
const CHARACTER_SPEC Spec_GunSlinger =
{
	"GunSlinger",
	LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,

	TEXT("Prototype_Component_Model_GunSlinger"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,
	BAKED_HAIR,

	// 쌍권총을 단일 본에 붙이는 static mesh 셰이더를 사용한다.
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	// CHARACTER_ANIM 순서에 gdh 계열 clip 이름을 대응시킨다.
	{
		"gdh_idle_battle_1",
		"gdh_run_battle_1",
		"gdh_dmg_idle_1",
		"gdh_dead_1",
		"gdh_knockdown",
		"gdh_knockdown_land",
		"gdh_down",
		"gdh_standup_1",
		// 이 spec에는 ESTHER_CAST clip을 지정하지 않았다.
		nullptr,
		nullptr, // CUSTOMIZING_IDLE
		"gdh_abn_fear_1",
		nullptr, // SQUAREHOLE_SONG
	},

	&Create_Logic,
};

NS_END
