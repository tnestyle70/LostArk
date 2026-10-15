#include "Logic_GuardianKnight.h"

// 표현 갱신 인자의 CCharacter 정의를 연결한다. 현재 본문에서는 그 멤버에 접근하지 않는다.
#include "Character.h"

namespace
{
	// 오른손의 b_wp_1에 무기를 붙인다. 소켓 누락은 현재 Part 초기화가 거부한다.
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_GuardianKnight_Weapon"), "b_wp_1" },
	};

	// 본체 뒤에 장비를 갱신한다. Wing은 옷 교체 슬롯이 없는 IDENTITY Part이며 dragon stance를 요구한다.
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_GuardianKnight_Arm"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HANDS },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_GuardianKnight_Helmet"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_GuardianKnight_Lower"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::LOWER },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_GuardianKnight_Shoulder"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::SHOULDER },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_GuardianKnight_Upper"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::UPPER },
		{ TEXT("Part_10_Equip_Wing"),     TEXT("Prototype_Component_Model_GuardianKnight_Wing"),
		  0u, false, EQUIPMENT_SLOT_KIND::IDENTITY,
		  EQUIPMENT_PRESENTATION_SLOT::END,
		  LostArk::Shared::PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON },
	};

	// 장비와 겹치는 본체 outfit submesh 1·2를 숨긴다. mask는 해당 Model의 mesh 순서에 의존한다.
	constexpr uint32_t COVERED_BY_ARMOUR = (1u << 1) | (1u << 2);

	// 본체 hair 0번은 별도 HEAD 교체가 성공하기 전까지 fallback으로 유지한다.
	constexpr uint32_t BAKED_HAIR = (1u << 0);

	// dragon stance에서는 공통 idle/run 대신 identity 자세 clip을 선택한다.
	constexpr STANCE_LOCOMOTION_SPEC StanceLocomotion[] =
	{
		{ LostArk::Shared::PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON,
		  "ddk_idle_identity1_1", "ddk_run_identity1_1" },
	};

	// 치마·망토·머리카락 체인에 적용할 solver 계수다. 실제 root와 개수는 아래 목록에 정의한다.
	constexpr f32_t PLATE_STIFFNESS = 0.12f;
	constexpr f32_t PLATE_DAMPING = 0.45f;
	constexpr f32_t PLATE_GRAVITY = 12.f;
	constexpr f32_t PLATE_MAX_DISPLACEMENT = 0.15f;
	constexpr f32_t PLATE_WIND_RESPONSE = 2.f;

	constexpr f32_t CAPE_STIFFNESS = 0.06f;
	constexpr f32_t CAPE_DAMPING = 0.55f;
	constexpr f32_t CAPE_GRAVITY = 20.f;
	constexpr f32_t CAPE_MAX_DISPLACEMENT = 0.35f;
	constexpr f32_t CAPE_WIND_RESPONSE = 5.f;

	constexpr f32_t HAIR_STIFFNESS = 0.12f;
	constexpr f32_t HAIR_DAMPING = 0.5f;
	constexpr f32_t HAIR_GRAVITY = 15.f;
	constexpr f32_t HAIR_MAX_DISPLACEMENT = 0.25f;
	constexpr f32_t HAIR_WIND_RESPONSE = 3.f;

	constexpr BONE_CHAIN_SPEC BoneChains[] =
	{
		{ "b_skirt_f_01",  4u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_fl_01", 4u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_fr_01", 4u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_l_01",  4u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_r_01",  4u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_b_01",  4u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_bl_01", 4u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_br_01", 4u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },

		{ "b_capatcloth_l_01", 5u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.0f },
		{ "b_capatcloth_b_01", 5u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.0f },
		{ "b_capatcloth_r_01", 5u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.0f },
		{ "b_cape_01", 4u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.0f },

		{ "b_hair00_b_01",  4u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.0f },
		{ "b_hair01_b_01",  3u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.0f },
		{ "b_hair02_b_01",  4u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.0f },
		{ "b_hair03_fl_01", 3u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.0f },
		{ "b_hair03_fr_01", 3u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.0f },
		{ "b_hair04_fl_01", 3u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.0f },
		{ "b_hair04_fr_01", 3u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.0f },

		// 허리 장식 체인도 animation 뒤 solver에서 처리한다.
		{ "b_add_tail_1_01", 4u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.0f },
	};

	// 로컬 CCharacter가 spec의 factory를 호출해 logic을 단독 소유한다. Character 정리 시 함께 소멸한다.
	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_GuardianKnight>();
	}
}

NS_BEGIN(Client)

// 현재 클래스별 추가 갱신은 없다. 입력·상태를 변경하지 않고 공통 CCharacter 표현을 사용한다.
void CLogic_GuardianKnight::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

// catalog가 extern으로 참조하는 프로세스 수명의 직업 설정이다.
// CCharacter가 Part·clip·선택 기능을 조립할 때 읽는 불변 설정이다. 아래 배열·문자열 포인터는 정적 수명이다.
const CHARACTER_SPEC Spec_GuardianKnight =
{
	"GuardianKnight",
	LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT,

	TEXT("Prototype_Component_Model_GuardianKnight"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,
	BAKED_HAIR,

	// 무기를 단일 본에 부착하는 static mesh 셰이더를 사용한다.
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	// CHARACTER_ANIM 순서에 ddk 계열 기본·에스더·외형 편집·노래 clip을 대응시킨다.
	{
		"ddk_idle_battle_1",
		"ddk_run_battle_1",
		"ddk_dmg_idle_1",
		"ddk_dead_1",
		"ddk_knockdown",
		"ddk_knockdown_land",
		"ddk_down",
		"ddk_standup_1",
		"ddk_act_estherskill_1",
		"idle_charactercustomizing_1",
		"ddk_abn_fear_1",
		"ddk_act_music_loop_1",
	},

	&Create_Logic,

	nullptr,
	0u,

	StanceLocomotion,
	static_cast<uint32_t>(size(StanceLocomotion)),

	BoneChains,
	static_cast<uint32_t>(size(BoneChains)),

	"DK",
	true, // HEAD 교체 commit 전까지 본체 머리카락을 유지한다.
};

NS_END
