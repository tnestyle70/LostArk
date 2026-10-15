#include "Logic_Warlord.h"

// 표현 갱신 인자의 CCharacter 정의를 연결한다. 현재 본문에서는 그 멤버에 접근하지 않는다.
#include "Character.h"

namespace
{
	// 건랜스는 b_weapon_rhand, 방패는 b_weapon_lhand에 붙인다. 소켓 누락은 현재 Part 초기화가 거부한다.
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_Warlord_Weapon"), "b_weapon_rhand" },
		{ TEXT("Part_90_Weapon_L"),
		  TEXT("Prototype_Component_Model_Warlord_Shield"), "b_weapon_lhand" },
	};

	// 본체→장비→무기 순서로 갱신하도록 tag를 정한다. 머리카락도 별도 장비 Part다.
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_Warlord_Arm"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HANDS },
		// 머리카락 Part는 생성하되 기본 helmet과 겹치지 않도록 처음에는 숨긴다.
		{ TEXT("Part_10_Equip_Hair"),     TEXT("Prototype_Component_Model_Warlord_Hair"),
		  0u, true, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_Warlord_Helmet"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_Warlord_Lower"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::LOWER },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_Warlord_Shoulder"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::SHOULDER },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_Warlord_Upper"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::UPPER },
	};

	// 장비와 겹치는 본체 submesh 0·1·2·3을 숨기고 face·eye는 남긴다.
	constexpr uint32_t COVERED_BY_ARMOUR =
		(1u << 0) | (1u << 1) | (1u << 2) | (1u << 3);

	// 이 본체에는 별도 hair submesh 숨김 비트를 지정하지 않는다.
	constexpr uint32_t BAKED_HAIR = 0u;

	// 방어 stance의 idle/run은 방패 방어용 clip으로 교체한다. 무기 교체를 의미하지 않는다.
	constexpr STANCE_LOCOMOTION_SPEC StanceLocomotion[] =
	{
		{ LostArk::Shared::PLAYER_STANCE_ID::WARLORD_DEFENSE,
		  "wgl_sk_defence_loop", "wgl_run_defence_1" },
	};

	// 치마와 망토 체인의 복원·감쇠·중력·최대 변위·이동풍 계수를 따로 정한다.
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

	// 투구 장식에는 망토와 다른 solver 계수를 사용한다.
	constexpr f32_t PLUME_STIFFNESS = 0.12f;
	constexpr f32_t PLUME_DAMPING = 0.5f;
	constexpr f32_t PLUME_GRAVITY = 15.f;
	constexpr f32_t PLUME_MAX_DISPLACEMENT = 0.25f;
	constexpr f32_t PLUME_WIND_RESPONSE = 3.f;

	// 마지막 값은 degree 단위 반각 제한이다. 실제 적용 대상은 아래 본 이름과 개수로 정한다.
	constexpr BONE_CHAIN_SPEC BoneChains[] =
	{
		{ "b_skirt_f_01",  5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_fl_01", 5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_fr_01", 5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_l_01",  5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_r_01",  5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_b_01",  5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_bl_01", 5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_br_01", 5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE, 40.0f },

		{ "b_capatcloth_l_01", 5u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.0f },
		{ "b_capatcloth_b_01", 5u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.0f },
		{ "b_capatcloth_r_01", 5u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.0f },

		{ "b_helmet_b_01", 4u, PLUME_STIFFNESS, PLUME_DAMPING,
		  PLUME_GRAVITY, PLUME_MAX_DISPLACEMENT, PLUME_WIND_RESPONSE, 30.0f },

		// 별도 머리카락과 허리 장식도 동일 solver 목록에 포함한다.
		{ "b_hair01_b_01", 4u, PLUME_STIFFNESS, PLUME_DAMPING,
		  PLUME_GRAVITY, PLUME_MAX_DISPLACEMENT, PLUME_WIND_RESPONSE, 40.0f },
		{ "b_add_tail_1_01", 4u, PLUME_STIFFNESS, PLUME_DAMPING,
		  PLUME_GRAVITY, PLUME_MAX_DISPLACEMENT, PLUME_WIND_RESPONSE, 40.0f },
	};

	// 로컬 CCharacter가 spec의 factory를 호출해 logic을 단독 소유한다. Character 정리 시 함께 소멸한다.
	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_Warlord>();
	}
}

NS_BEGIN(Client)

// 현재 클래스별 추가 갱신은 없다. 입력·상태를 변경하지 않고 공통 CCharacter 표현을 사용한다.
void CLogic_Warlord::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

// catalog가 extern으로 참조할 프로세스 수명의 직업 설정이다.
// CCharacter가 Part·clip·선택 기능을 조립할 때 읽는 불변 설정이다. 아래 배열·문자열 포인터는 정적 수명이다.
const CHARACTER_SPEC Spec_Warlord =
{
	"Warlord",
	LostArk::Shared::CHARACTER_CLASS_ID::WARLORD,

	TEXT("Prototype_Component_Model_Warlord"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,
	BAKED_HAIR,

	// 건랜스와 방패는 static mesh 셰이더로 그리는 두 무기 Part다.
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	// CHARACTER_ANIM 순서에 wgl 계열 기본·에스더 clip을 대응시킨다.
	{
		"wgl_idle_battle_1",
		"wgl_run_battle_1",
		"wgl_dmg_idle_1",
		"wgl_dead_1",
		"wgl_knockdown",
		"wgl_knockdown_land",
		"wgl_down",
		"wgl_standup_1",
		"wgl_act_estherskill_1",
		// 캐릭터 생성 화면에서 사용할 idle clip이다.
		"idle_charactercustomizing_1",
		"wgl_abn_fear_1",
		// 스퀘어홀 노래 상태에 사용할 body clip이다.
		"wgl_act_music_loop_1",
	},

	&Create_Logic,

	nullptr,
	0u,

	StanceLocomotion,
	static_cast<uint32_t>(size(StanceLocomotion)),

	BoneChains,
	static_cast<uint32_t>(size(BoneChains)),

	"WR",
};

NS_END
