#include "Logic_DimensionMaster.h"

#include "Character.h"

namespace
{
	/* The hair this cooked body draws by itself. A worn hairstyle replaces it, so it is
	hidden only while a HEAD set is on -- the in-world look keeps it. Submesh 9 is
	pc_sp_m_hair_55, read off the cooked model's material order. */
	constexpr uint32_t BAKED_HAIR = (1u << 9);

	/* This class's only equipment part. The hairstyle is a part, not something the body
	draws: character creation picks one and swaps this out for it. Style 0 of this class's
	list is what a character that has never been through that screen wears. */
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Hair"),     TEXT("Prototype_Component_Model_DimensionMaster_Hair"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
	};

	/* TownDefaultWeapon 106600901 resolves to WP_WSWP_M_06. Its LookInfo splits
	the weapon into four meshes and gives each one a distinct battle socket. The
	E piece carries the original RelativeRotationYaw=32768 (180 degrees). */
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

	constexpr SKILL_SURFACE_EMISSIVE_SPEC SkillSurfaceEmissives[] =
	{
		{ 2050210u, float4_t(1.f, 1.f, 1.f, 1.f), 4.f },
	};

	/* The robe's lower panels. They hang from the hips, so they swing wide on a turn
	and need the longest travel of the three groups. Eight chains of four: b_skirt_f
	and b_skirt_b1 front and back, l1..l3 and r1..r3 down each side. */
	constexpr f32_t SKIRT_STIFFNESS = 0.1f;
	constexpr f32_t SKIRT_DAMPING = 0.5f;
	constexpr f32_t SKIRT_GRAVITY = 15.f;
	constexpr f32_t SKIRT_MAX_DISPLACEMENT = 0.35f;
	constexpr f32_t SKIRT_WIND_RESPONSE = 4.f;

	/* The shoulder mantle. It rides higher and is the piece a run pushes back, so it
	takes the strongest wind of the three. Its chains start at 00, not 01. */
	constexpr f32_t CAPE_STIFFNESS = 0.08f;
	constexpr f32_t CAPE_DAMPING = 0.55f;
	constexpr f32_t CAPE_GRAVITY = 13.f;
	constexpr f32_t CAPE_MAX_DISPLACEMENT = 0.4f;
	constexpr f32_t CAPE_WIND_RESPONSE = 5.f;

	/* Hair and the belt tail. Both hang off fast-moving parents, so they stay stiffer
	and drier than the cloth -- the same split the other classes use. */
	constexpr f32_t HAIR_STIFFNESS = 0.12f;
	constexpr f32_t HAIR_DAMPING = 0.5f;
	constexpr f32_t HAIR_GRAVITY = 15.f;
	constexpr f32_t HAIR_MAX_DISPLACEMENT = 0.25f;
	constexpr f32_t HAIR_WIND_RESPONSE = 3.f;

	/* Link counts are this body's own: read off DimensionMaster_Character.wmodel's
	225-bone palette, not assumed from the other classes. */
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

	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<Client::CLogic_DimensionMaster>();
	}
}

NS_BEGIN(Client)

void CLogic_DimensionMaster::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

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

	/* The combined body has no separate down loop, so DOWN_LOOP stays null and
	the land clip's final lying pose holds until the standup. */
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
		/* Attached CustomizingAnimSet clip; the retail character-creation pose. */
		"idle_charactercustomizing_1",
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
