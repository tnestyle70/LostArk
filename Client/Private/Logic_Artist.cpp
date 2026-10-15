#include "Logic_Artist.h"

// 표현 갱신 인자의 CCharacter 정의를 연결한다. 현재 본문에서는 그 멤버에 접근하지 않는다.
#include "Character.h"

namespace
{
	// 붓을 오른손의 b_wp_1에 붙인다. 소켓 누락은 현재 Part 초기화가 거부한다.
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_Artist_Weapon"), "b_wp_1" },
	};

	// helmet의 1번 머리카락 mesh를 제외해 별도 hair 표현과 겹치지 않게 한다.
	constexpr uint32_t HELMET_HAIR = 1u << 1;

	// tag 순서로 본체가 먼저 갱신되고 장비 다음에 무기가 갱신된다.
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_Artist_Arm"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HANDS },
		// helmet Part는 생성하지만 기본 숨김 상태로 두어 별도 머리카락과 겹침을 막는다.
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_Artist_Helmet"),
		  HELMET_HAIR, true, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_Artist_Lower"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::LOWER },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_Artist_Shoulder"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::SHOULDER },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_Artist_Upper"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::UPPER },
		// HEAD 슬롯의 별도 머리카락 Part다. 캐릭터 외형 편집은 이 표현을 교체한다.
		{ TEXT("Part_10_Equip_Hair"),     TEXT("Prototype_Component_Model_Artist_Hair"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
	};

	// 장비와 겹치는 upper·base_body·arm·lower인 본체 submesh 4·5·6·7을 숨긴다.
	constexpr uint32_t COVERED_BY_ARMOUR =
		(1u << 4) | (1u << 5) | (1u << 6) | (1u << 7);

	// 별도 HEAD 표현에 가려지는 본체 3번 hair mesh의 mask다.
	constexpr uint32_t BAKED_HAIR = (1u << 3);

	// 치마·허리 장식 solver에 전달할 계수다. 실제 적용 대상은 아래 BoneChains 목록이다.
	constexpr f32_t ROBE_STIFFNESS = 0.05f;
	constexpr f32_t ROBE_DAMPING = 0.6f;
	constexpr f32_t ROBE_GRAVITY = 16.f;
	constexpr f32_t ROBE_MAX_DISPLACEMENT = 0.3f;
	constexpr f32_t ROBE_WIND_RESPONSE = 4.5f;

	// 팔에 붙은 소매 체인은 몸통 천과 다른 복원·감쇠·풍응답 계수를 사용한다.
	constexpr f32_t WING_STIFFNESS = 0.08f;
	constexpr f32_t WING_DAMPING = 0.55f;
	constexpr f32_t WING_GRAVITY = 12.f;
	constexpr f32_t WING_MAX_DISPLACEMENT = 0.25f;
	constexpr f32_t WING_WIND_RESPONSE = 3.f;

	constexpr f32_t TASSEL_STIFFNESS = 0.12f;
	constexpr f32_t TASSEL_DAMPING = 0.5f;
	constexpr f32_t TASSEL_GRAVITY = 12.f;
	constexpr f32_t TASSEL_MAX_DISPLACEMENT = 0.15f;
	constexpr f32_t TASSEL_WIND_RESPONSE = 2.f;

	// 마지막 값은 체인별 degree 반각 제한이다. root 이름과 길이는 직업 골격에 맞춘다.
	constexpr BONE_CHAIN_SPEC BoneChains[] =
	{
		{ "b_skirt_f_01",  4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_fl_01", 4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_fr_01", 4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_l_01",  4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_r_01",  4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_b_01",  4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_bl_01", 4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE, 40.0f },
		{ "b_skirt_br_01", 4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE, 40.0f },

		{ "b_add_tail_1_01", 3u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE, 30.0f },

		{ "b_armwing_l_01", 5u, WING_STIFFNESS, WING_DAMPING,
		  WING_GRAVITY, WING_MAX_DISPLACEMENT, WING_WIND_RESPONSE, 40.0f },
		{ "b_armwing_r_01", 5u, WING_STIFFNESS, WING_DAMPING,
		  WING_GRAVITY, WING_MAX_DISPLACEMENT, WING_WIND_RESPONSE, 40.0f },

		{ "b_hair_b_11", 4u, TASSEL_STIFFNESS, TASSEL_DAMPING,
		  TASSEL_GRAVITY, TASSEL_MAX_DISPLACEMENT, TASSEL_WIND_RESPONSE, 30.0f },
	};

	// 로컬 CCharacter가 spec의 factory를 호출해 logic을 단독 소유한다. Character 정리 시 함께 소멸한다.
	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_Artist>();
	}
}

NS_BEGIN(Client)

// 현재 클래스별 추가 갱신은 없다. 입력·상태를 변경하지 않고 공통 CCharacter 표현을 사용한다.
void CLogic_Artist::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

// catalog가 extern으로 참조할 직업 설정이며 배열과 문자열은 프로세스 동안 유지된다.
// CCharacter가 Part·clip·선택 기능을 조립할 때 읽는 불변 설정이다. 아래 배열·문자열 포인터는 정적 수명이다.
const CHARACTER_SPEC Spec_Artist =
{
	"Artist",
	LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,

	TEXT("Prototype_Component_Model_Artist"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,
	BAKED_HAIR,

	// 붓 Part는 static mesh 셰이더를 사용한다.
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	// CHARACTER_ANIM 순서에 sdm 계열 기본·에스더 clip을 대응시킨다.
	{
		"sdm_idle_battle_1",
		"sdm_run_battle_1",
		"sdm_dmg_idle_1",
		"sdm_dead_1",
		"sdm_knockdown",
		"sdm_knockdown_land",
		"sdm_down",
		"sdm_standup_1",
		"sdm_act_estherskill_1",
		// 캐릭터 생성 화면의 idle clip이다.
		"idle_charactercustomizing_1",
		"sdm_abn_fear_1",
		// 스퀘어홀 노래 상태에 사용할 body clip이다.
		"sdm_act_music_loop_1",
	},

	&Create_Logic,

	nullptr,
	0u,

	nullptr,
	0u,

	BoneChains,
	static_cast<uint32_t>(size(BoneChains)),

	"SP",
};

NS_END
