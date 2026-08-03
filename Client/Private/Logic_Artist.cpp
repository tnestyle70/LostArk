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

	/* Equipment part tags sort after "Part_00_Body" so the body still updates
	first, and before the weapon tags. */
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_Artist_Arm") },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_Artist_Helmet") },
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
};

NS_END
