#include "Logic_Artist.h"

#include "Character.h"

namespace
{
	/* One brush. The sp rig has no "b_weapon_*" bone -- it carries b_wp_1 under
	bip001-r-hand, b_wp_2 under bip001-l-hand and b_wp_3 on the spine -- the same
	family gn_f uses. A socket name the rig lacks fails silently: Get_BoneMatrix
	returns identity, the body's 0.0001 preTransform never reaches the weapon and
	it renders thousands of times too large. */
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_Artist_Weapon"), "b_wp_1" },
	};

	/* The helmet piece carries the hat (0) and the helmet-fitted hair (1,
	pc_sp_28). The body already draws pc_sp_06 as its own submesh 3, so leaving
	both on renders two heads of hair. The hat is what this piece is here for. */
	constexpr uint32_t HELMET_HAIR = 1u << 1;

	/* Equipment part tags sort after "Part_00_Body" so the body still updates
	first, and before the weapon tags. */
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_Artist_Arm") },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_Artist_Helmet"), HELMET_HAIR },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_Artist_Lower") },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_Artist_Shoulder") },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_Artist_Upper") },
	};

	/* The cook lays the body out 0 face / 1 eye_ao / 2 eye / 3 hair /
	4 upper / 5 base_body / 6 arm / 7 lower, so everything below the head is
	hidden and the armour draws it instead.

	Unlike gn_f, this body carries a bare-skin submesh of its own (5,
	pc_sp_av_base_body_mi) on top of its default outfit, and every armour part
	ships that same material for the skin it leaves exposed. Keeping both would
	z-fight, so 5 is hidden with the rest. If that turns out to leave a gap the
	armour does not cover, 5 is the bit to put back.
	Re-cooking the body with different content invalidates these bits. */
	constexpr uint32_t COVERED_BY_ARMOUR =
		(1u << 4) | (1u << 5) | (1u << 6) | (1u << 7);

	/* Chains picked by parsing what the shipped meshes actually skin, not by
	what the rig offers: the capatcloth chains and the long b_hair chains have
	no weights on any product mesh and stay unlisted. The Upper coat hangs
	eight b_skirt panels and one b_add_tail tassel off the pelvis, the
	Shoulder piece is almost entirely the two b_armwing sleeve wings, and the
	hat's b_hair_b_11 tassel rides the head. */
	constexpr f32_t ROBE_STIFFNESS = 0.05f;
	constexpr f32_t ROBE_DAMPING = 0.6f;
	constexpr f32_t ROBE_GRAVITY = 16.f;
	constexpr f32_t ROBE_MAX_DISPLACEMENT = 0.3f;
	constexpr f32_t ROBE_WIND_RESPONSE = 4.5f;

	/* The wings hang from the upper arms, which move faster than the hips, so
	they run stiffer to keep them from wrapping around the arm mid-swing. */
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

	constexpr BONE_CHAIN_SPEC BoneChains[] =
	{
		{ "b_skirt_f_01",  4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE },
		{ "b_skirt_fl_01", 4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE },
		{ "b_skirt_fr_01", 4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE },
		{ "b_skirt_l_01",  4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE },
		{ "b_skirt_r_01",  4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE },
		{ "b_skirt_b_01",  4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE },
		{ "b_skirt_bl_01", 4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE },
		{ "b_skirt_br_01", 4u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE },

		{ "b_add_tail_1_01", 3u, ROBE_STIFFNESS, ROBE_DAMPING,
		  ROBE_GRAVITY, ROBE_MAX_DISPLACEMENT, ROBE_WIND_RESPONSE },

		{ "b_armwing_l_01", 5u, WING_STIFFNESS, WING_DAMPING,
		  WING_GRAVITY, WING_MAX_DISPLACEMENT, WING_WIND_RESPONSE },
		{ "b_armwing_r_01", 5u, WING_STIFFNESS, WING_DAMPING,
		  WING_GRAVITY, WING_MAX_DISPLACEMENT, WING_WIND_RESPONSE },

		{ "b_hair_b_11", 4u, TASSEL_STIFFNESS, TASSEL_DAMPING,
		  TASSEL_GRAVITY, TASSEL_MAX_DISPLACEMENT, TASSEL_WIND_RESPONSE },
	};

	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_Artist>();
	}
}

NS_BEGIN(Client)

void CLogic_Artist::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

/* External linkage comes from the extern declaration in the header. */
const CHARACTER_SPEC Spec_Artist =
{
	"Artist",
	LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,

	TEXT("Prototype_Component_Model_Artist"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,

	/* Item 1061002 is this class's default weapon and names WP_WSDM_09-1, which
	is a material variant of the one mesh in WP_WSDM_09 -- the -1..-9 instances
	all name the same textures. */
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	/* IDLE, RUN, HIT, DEAD -- FBX takes are "<armature>_<action>" and the
	armature is named "sdm", so every clip carries that prefix. */
	{
		"sdm_idle_battle_1",
		"sdm_run_battle_1",
		"sdm_dmg_idle_1",
		"sdm_dead_1",
	},

	&Create_Logic,

	nullptr,
	0u,

	nullptr,
	0u,

	BoneChains,
	static_cast<uint32_t>(size(BoneChains)),
};

NS_END
