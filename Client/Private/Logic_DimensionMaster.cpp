#include "Logic_DimensionMaster.h"

// 표현 갱신 인자의 CCharacter 정의를 연결한다. 현재 본문에서는 그 멤버에 접근하지 않는다.
#include "Character.h"

namespace
{
	// 별도 HEAD 표현과 겹치지 않도록 본체의 9번 hair submesh를 숨길 때 쓰는 mask다.
	constexpr uint32_t BAKED_HAIR = (1u << 9);

	// 기본 장비 목록은 별도 머리카락 Part 하나다. 외형 편집은 HEAD 슬롯의 모델을 교체한다.
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Hair"),     TEXT("Prototype_Component_Model_DimensionMaster_Hair"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
	};

	// 무기 네 조각을 서로 다른 본에 붙인다. E 조각에는 180degree yaw를 추가한다.
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_L"),
		  TEXT("Prototype_Component_Model_DimensionMaster_Weapon_L"),
		  "b_wp_swm_m_1" },
		{ TEXT("Part_91_Weapon_S"),
		  TEXT("Prototype_Component_Model_DimensionMaster_Weapon_S"),
		  "b_wp_swm_m_2" },
		{ TEXT("Part_92_Weapon_P"),
		  TEXT("Prototype_Component_Model_DimensionMaster_Weapon_P"),
		  "b_wp_swm_m_3" },
		{ TEXT("Part_93_Weapon_E"),
		  TEXT("Prototype_Component_Model_DimensionMaster_Weapon_E"),
		  "b_wp_swm_m_4_02", 180.f },
	};

	// Server action의 skill ID에 대응해 적용할 표면 발광 색·강도다. 여기서 스킬을 실행하지 않는다.
	constexpr SKILL_SURFACE_EMISSIVE_SPEC SkillSurfaceEmissives[] =
	{
		{ 2050210u, float4_t(1.f, 1.f, 1.f, 1.f), 4.f },
	};

	// 치마 체인의 복원·감쇠·중력·최대 변위·이동풍 계수다.
	constexpr f32_t SKIRT_STIFFNESS = 0.1f;
	constexpr f32_t SKIRT_DAMPING = 0.5f;
	constexpr f32_t SKIRT_GRAVITY = 15.f;
	constexpr f32_t SKIRT_MAX_DISPLACEMENT = 0.35f;
	constexpr f32_t SKIRT_WIND_RESPONSE = 4.f;

	// 어깨 망토 체인에 별도의 계수를 적용한다. 이 체인 이름은 00부터 시작한다.
	constexpr f32_t CAPE_STIFFNESS = 0.08f;
	constexpr f32_t CAPE_DAMPING = 0.55f;
	constexpr f32_t CAPE_GRAVITY = 13.f;
	constexpr f32_t CAPE_MAX_DISPLACEMENT = 0.4f;
	constexpr f32_t CAPE_WIND_RESPONSE = 5.f;

	// 머리카락과 허리 장식은 천과 별도 계수를 사용한다.
	constexpr f32_t HAIR_STIFFNESS = 0.12f;
	constexpr f32_t HAIR_DAMPING = 0.5f;
	constexpr f32_t HAIR_GRAVITY = 15.f;
	constexpr f32_t HAIR_MAX_DISPLACEMENT = 0.25f;
	constexpr f32_t HAIR_WIND_RESPONSE = 3.f;

	// 각 행은 실제 root 본 이름·본 개수와 해당 체인의 solver 계수·각도 제한이다.
	constexpr BONE_CHAIN_SPEC BoneChains[] =
	{
		{ "b_skirt_f_01",  4u, SKIRT_STIFFNESS, SKIRT_DAMPING,
		  SKIRT_GRAVITY, SKIRT_MAX_DISPLACEMENT, SKIRT_WIND_RESPONSE, 40.f },
		{ "b_skirt_b1_01", 4u, SKIRT_STIFFNESS, SKIRT_DAMPING,
		  SKIRT_GRAVITY, SKIRT_MAX_DISPLACEMENT, SKIRT_WIND_RESPONSE, 40.f },
		{ "b_skirt_l1_01", 4u, SKIRT_STIFFNESS, SKIRT_DAMPING,
		  SKIRT_GRAVITY, SKIRT_MAX_DISPLACEMENT, SKIRT_WIND_RESPONSE, 40.f },
		{ "b_skirt_l2_01", 4u, SKIRT_STIFFNESS, SKIRT_DAMPING,
		  SKIRT_GRAVITY, SKIRT_MAX_DISPLACEMENT, SKIRT_WIND_RESPONSE, 40.f },
		{ "b_skirt_l3_01", 4u, SKIRT_STIFFNESS, SKIRT_DAMPING,
		  SKIRT_GRAVITY, SKIRT_MAX_DISPLACEMENT, SKIRT_WIND_RESPONSE, 40.f },
		{ "b_skirt_r1_01", 4u, SKIRT_STIFFNESS, SKIRT_DAMPING,
		  SKIRT_GRAVITY, SKIRT_MAX_DISPLACEMENT, SKIRT_WIND_RESPONSE, 40.f },
		{ "b_skirt_r2_01", 4u, SKIRT_STIFFNESS, SKIRT_DAMPING,
		  SKIRT_GRAVITY, SKIRT_MAX_DISPLACEMENT, SKIRT_WIND_RESPONSE, 40.f },
		{ "b_skirt_r3_01", 4u, SKIRT_STIFFNESS, SKIRT_DAMPING,
		  SKIRT_GRAVITY, SKIRT_MAX_DISPLACEMENT, SKIRT_WIND_RESPONSE, 40.f },

		{ "b_capatcloth_l_00", 6u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.f },
		{ "b_capatcloth_b_00", 6u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.f },
		{ "b_capatcloth_r_00", 6u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE, 40.f },

		{ "b_hair_b_01",  4u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.f },
		{ "b_hair_bl_01", 4u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.f },
		{ "b_hair_br_01", 4u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.f },
		{ "b_hair_fl_01", 3u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.f },
		{ "b_hair_fr_01", 3u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.f },

		{ "b_add_tail_1_01", 3u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.f },
	};

	// 로컬 CCharacter가 spec의 factory를 호출해 logic을 단독 소유한다. Character 정리 시 함께 소멸한다.
	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<Client::CLogic_DimensionMaster>();
	}
}

NS_BEGIN(Client)

// 현재 클래스별 추가 갱신은 없다. 입력·상태를 변경하지 않고 공통 CCharacter 표현을 사용한다.
void CLogic_DimensionMaster::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

// CCharacter가 Part·clip·선택 기능을 조립할 때 읽는 불변 설정이다. 아래 배열·문자열 포인터는 정적 수명이다.
const CHARACTER_SPEC Spec_DimensionMaster =
{
	"DimensionMaster",
	LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,

	TEXT("Prototype_Component_Model_DimensionMaster"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	0u,
	BAKED_HAIR,

	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	// DOWN_LOOP은 지정하지 않았다. 후속 pose 선택은 CCharacter의 공통 애니메이션 경로가 처리한다.
	{
		"pc_sp_m_00_sk_idle_battle_1",
		"pc_sp_m_00_sk_run_battle_1",
		"pc_sp_m_00_sk_dmg_idle_1",
		"pc_sp_m_00_sk_dead_1",
		"pc_sp_m_00_sk_knockdown",
		"pc_sp_m_00_sk_knockdown_land",
		nullptr,
		"pc_sp_m_00_sk_standup_1",
		"pc_sp_m_00_sk_act_estherskill_1",
		// 캐릭터 생성 화면에서 사용할 idle clip이다.
		"idle_charactercustomizing_1",
		"pc_sp_m_00_sk_abn_fear_1",
		// 스퀘어홀 노래 상태에 사용할 body clip이다.
		"pc_sp_m_00_sk_act_music_loop_1",
	},

	&Create_Logic,

	SkillSurfaceEmissives,
	static_cast<uint32_t>(size(SkillSurfaceEmissives)),

	nullptr,
	0u,

	BoneChains,
	static_cast<uint32_t>(size(BoneChains)),

	"SP_M",
};

NS_END
