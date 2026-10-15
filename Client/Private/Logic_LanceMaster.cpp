#include "Logic_LanceMaster.h"

// 표현 갱신 인자의 CCharacter 정의를 연결한다. 현재 본문에서는 그 멤버에 접근하지 않는다.
#include "Character.h"

namespace
{
	// 같은 오른손 소켓에 장창/단창 Part를 두고 각각 요구 stance를 지정한다.
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_LanceMaster_Weapon"), "b_weapon_rhand", 0.f,
		  LostArk::Shared::PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR },
		{ TEXT("Part_91_Weapon_R_Short"),
		  TEXT("Prototype_Component_Model_LanceMaster_Weapon_Short"), "b_weapon_rhand", 0.f,
		  LostArk::Shared::PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR },
	};

	// tag 순서로 Part_00_Body 다음에 장비, 그 뒤에 무기가 갱신된다.
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_LanceMaster_Arm"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HANDS },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_LanceMaster_Helmet"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT_HELMET,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_LanceMaster_Lower"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::LOWER },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_LanceMaster_Shoulder"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::SHOULDER },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_LanceMaster_Upper"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::UPPER },
		// 머리카락은 별도 HEAD Part로 제공한다. 외형 설정의 저장 index를 이 목록에서 재정렬하지 않는다.
		{ TEXT("Part_10_Equip_Hair"),     TEXT("Prototype_Component_Model_LanceMaster_Hair"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
	};

	// 장비와 겹치는 arm·upper·lower인 본체 submesh 0·1·2를 숨긴다.
	constexpr uint32_t COVERED_BY_ARMOUR = (1u << 0) | (1u << 1) | (1u << 2);

	// 눈 mesh에서 분리한 scalp cap은 6번이다. 이 비트만 숨겨 눈 자체는 유지한다.
	constexpr uint32_t EYE_SUBMESH_SCALP_CAP = (1u << 6);

	// 별도 HEAD 표현과 겹치는 본체 7번 hair mesh를 숨길 때 쓰는 mask다.
	constexpr uint32_t BAKED_HAIR = (1u << 7);

	// 긴 천·짧은 치마·앞머리 체인에 서로 다른 solver 계수를 전달한다. 대상은 아래 BoneChains 목록이다.
	constexpr f32_t DRAPE_STIFFNESS = 0.04f;
	constexpr f32_t DRAPE_DAMPING = 0.6f;
	constexpr f32_t DRAPE_GRAVITY = 16.f;
	constexpr f32_t DRAPE_MAX_DISPLACEMENT = 0.3f;
	constexpr f32_t DRAPE_WIND_RESPONSE = 4.5f;

	constexpr f32_t FLAP_STIFFNESS = 0.12f;
	constexpr f32_t FLAP_DAMPING = 0.5f;
	constexpr f32_t FLAP_GRAVITY = 10.f;
	constexpr f32_t FLAP_MAX_DISPLACEMENT = 0.15f;
	constexpr f32_t FLAP_WIND_RESPONSE = 2.5f;

	constexpr f32_t BANGS_STIFFNESS = 0.15f;
	constexpr f32_t BANGS_DAMPING = 0.5f;
	constexpr f32_t BANGS_GRAVITY = 12.f;
	constexpr f32_t BANGS_MAX_DISPLACEMENT = 0.1f;
	constexpr f32_t BANGS_WIND_RESPONSE = 2.f;

	constexpr BONE_CHAIN_SPEC BoneChains[] =
	{
		{ "b_upper_cloth_f_01", 5u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE, 60.0f },
		{ "b_upper_cloth_b_01", 5u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE, 60.0f },

		{ "b_upper_pelat_br_01", 4u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE, 40.0f },
		{ "b_upper_pelat_bl_01", 4u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE, 40.0f },
		{ "b_upper_pelat_r_01",  5u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE, 60.0f },
		{ "b_upper_pelat_b_01",  4u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE, 40.0f },

		{ "b_upper_skirt_fl_01", 2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE, 50.0f },
		{ "b_upper_skirt_fr_01", 2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE, 50.0f },
		{ "b_upper_skirt_l_01",  2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE, 50.0f },
		{ "b_upper_skirt_r_01",  2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE, 50.0f },
		{ "b_upper_skirt_bl_01", 2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE, 50.0f },
		{ "b_upper_skirt_br_01", 2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE, 50.0f },

		{ "b_hair_fr_01", 2u, BANGS_STIFFNESS, BANGS_DAMPING,
		  BANGS_GRAVITY, BANGS_MAX_DISPLACEMENT, BANGS_WIND_RESPONSE, 40.0f },
		{ "b_hair_fl_01", 2u, BANGS_STIFFNESS, BANGS_DAMPING,
		  BANGS_GRAVITY, BANGS_MAX_DISPLACEMENT, BANGS_WIND_RESPONSE, 40.0f },

		// 뒷머리와 허리 장식도 solver에 포함한다. b_add_hair01_b는 골격 이름대로 11부터 시작한다.
		{ "b_hair00_b_01", 5u, BANGS_STIFFNESS, BANGS_DAMPING,
		  BANGS_GRAVITY, BANGS_MAX_DISPLACEMENT, BANGS_WIND_RESPONSE, 40.0f },
		{ "b_hair01_b_01", 4u, BANGS_STIFFNESS, BANGS_DAMPING,
		  BANGS_GRAVITY, BANGS_MAX_DISPLACEMENT, BANGS_WIND_RESPONSE, 40.0f },
		{ "b_add_hair01_b_11", 4u, BANGS_STIFFNESS, BANGS_DAMPING,
		  BANGS_GRAVITY, BANGS_MAX_DISPLACEMENT, BANGS_WIND_RESPONSE, 30.0f },
		{ "b_add_tail_1_01", 4u, BANGS_STIFFNESS, BANGS_DAMPING,
		  BANGS_GRAVITY, BANGS_MAX_DISPLACEMENT, BANGS_WIND_RESPONSE, 30.0f },
	};

	// 로컬 CCharacter가 spec의 factory를 호출해 logic을 단독 소유한다. Character 정리 시 함께 소멸한다.
	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_LanceMaster>();
	}
}

NS_BEGIN(Client)

// 현재 클래스별 추가 갱신은 없다. 입력·상태를 변경하지 않고 공통 CCharacter 표현을 사용한다.
void CLogic_LanceMaster::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

// catalog가 extern으로 참조할 프로세스 수명의 직업 설정이다.
// CCharacter가 Part·clip·선택 기능을 조립할 때 읽는 불변 설정이다. 아래 배열·문자열 포인터는 정적 수명이다.
const CHARACTER_SPEC Spec_LanceMaster =
{
	"LanceMaster",
	LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,

	TEXT("Prototype_Component_Model_LanceMaster"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR | EYE_SUBMESH_SCALP_CAP,
	BAKED_HAIR,

	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	// CHARACTER_ANIM 순서에 flm 계열 기본·에스더 clip을 대응시킨다.
	{
		"flm_idle_battle_1",
		"flm_run_battle_1",
		"flm_dmg_idle_1",
		"flm_dead_1",
		"flm_knockdown",
		"flm_knockdown_land",
		"flm_down",
		"flm_standup_1",
		"flm_act_estherskill_1",
		// 캐릭터 생성 화면에서 사용할 idle clip이다.
		"idle_charactercustomizing_1",
		"flm_abn_fear_1",
		// 스퀘어홀 노래 상태에 사용할 body clip이다.
		"flm_act_music_loop_1",
	},

	&Create_Logic,

	nullptr,
	0u,

	nullptr,
	0u,

	BoneChains,
	static_cast<uint32_t>(size(BoneChains)),

	"FT",
};

NS_END
