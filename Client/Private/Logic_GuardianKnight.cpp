#include "Logic_GuardianKnight.h"

#include "Character.h"

namespace
{
	/* One halberd in the right hand. The dk rig carries the GunSlinger/Artist
	family of socket names -- b_wp_1 / b_wp_2 / b_wp_3, no b_weapon_* -- and the
	source LookInfo binds the battle socket wp_ddk_r_battle to b_wp_1. The socket
	name has to be read off each class's own rig: one the rig lacks fails
	silently, since Get_BoneMatrix returns identity and the body's 0.0001
	preTransform never reaches the weapon. */
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_GuardianKnight_Weapon"), "b_wp_1" },
	};

	/* Equipment part tags sort after "Part_00_Body" so the body still updates
	first, and before the weapon tags. The wings are the identity part
	SK_DDK_WIN_00: skinned to the body's own b_wing_* bones, never covered by an
	avatar set, and drawn only while the dragon stance is active. */
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

	/* The cook lays this body out 0 hair / 1 avatar upper1 / 2 avatar upper /
	3 face / 4 eye ao / 5 eye. pc_dk_00_sk ships no skin under its clothes: the
	two pc_dl_av_018a pieces are the avatar outfit the base body wears, and the
	armour parts carry their own pc_dk_av_base skin, so those two submeshes hide
	while the armour draws. These bits are read off the cooked model every
	time, never copied from another class, and re-cooking the body with
	different content invalidates them. */
	constexpr uint32_t COVERED_BY_ARMOUR = (1u << 1) | (1u << 2);

	/* The hair this cooked body draws by itself (pc_ft_15_hair, submesh 0). Kept at
	zero on purpose, like GunSlinger: this class has no cooked hair sets, so nothing can
	take over from the body's own hair and hiding it leaves the head bare. Put the bit
	back once its hairstyle sets exist and it gets a hair part like Warlord. */
	constexpr uint32_t BAKED_HAIR = 0u;

	/* The dragon form is a posture, not a weapon swap: the class stands and runs
	on its identity clips until it leaves the stance. */
	constexpr STANCE_LOCOMOTION_SPEC StanceLocomotion[] =
	{
		{ LostArk::Shared::PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON,
		  "ddk_idle_identity1_1", "ddk_run_identity1_1" },
	};

	/* The source rig keys only the first bone of each chain and leaves the
	links below it in bind pose, because the original solves them at runtime.
	Link counts are this body's own, read off the 282-bone armour master the
	body was rebound to. */
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

		/* The belt tail lives only on the armour master; the body's 278-bone psa
		never keys it, so it rides the solver like the Warlord one. */
		{ "b_add_tail_1_01", 4u, HAIR_STIFFNESS, HAIR_DAMPING,
		  HAIR_GRAVITY, HAIR_MAX_DISPLACEMENT, HAIR_WIND_RESPONSE, 40.0f },
	};

	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_GuardianKnight>();
	}
}

NS_BEGIN(Client)

void CLogic_GuardianKnight::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

/* External linkage comes from the extern declaration in the header. */
const CHARACTER_SPEC Spec_GuardianKnight =
{
	"GuardianKnight",
	LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT,

	TEXT("Prototype_Component_Model_GuardianKnight"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,
	BAKED_HAIR,

	/* Item 1071002 is this class's default weapon and names WP_WDDK_04, a
	single halberd mesh cooked static like the other weapons. */
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	/* IDLE, RUN, HIT, DEAD, KNOCKDOWN, KNOCKDOWN_LAND, DOWN_LOOP, STANDUP,
	ESTHER_CAST -- FBX takes are "<armature>_<action>" and the armature is
	named "ddk", so every clip carries that prefix. The cast, customizing and
	music clips come from attached animation sets. */
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
};

NS_END
