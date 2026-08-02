#include "Logic_Slayer.h"

#include "Character.h"
#include "GameInstance.h"

namespace
{
	/* One greatsword. Unlike gn_f and sp, the wr_f rig uses the LanceMaster
	family of socket names -- b_weapon_rhand / b_weapon_lhand, with no b_wp_*
	at all. The socket name has to be read off each class's own rig: one the rig
	lacks fails silently, since Get_BoneMatrix returns identity and the body's
	0.0001 preTransform never reaches the weapon. */
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_Slayer_Weapon"), "b_weapon_rhand" },
	};

	/* Equipment part tags sort after "Part_00_Body" so the body still updates
	first, and before the weapon tags. */
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_Slayer_Arm") },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_Slayer_Helmet") },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_Slayer_Lower") },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_Slayer_Shoulder") },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_Slayer_Upper") },
	};

	/* The cook lays this body out 0 hair / 1 face / 2 eye_ao / 3 eye /
	4 upper / 5 base_body / 6 arm / 7 lower, so everything below the head is
	hidden and the armour draws it instead.

	Note the order is NOT the same as the other classes -- hair comes first here
	and face second, where Artist runs face, eye_ao, eye, hair. These bits are
	read off the cooked model every time, never copied from another class, and
	re-cooking the body with different content invalidates them. */
	constexpr uint32_t COVERED_BY_ARMOUR =
		(1u << 4) | (1u << 5) | (1u << 6) | (1u << 7);

	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_Slayer>();
	}
}

NS_BEGIN(Client)

void CLogic_Slayer::Update(CCharacter& Character, f32_t fTimeDelta)
{
	/* Skill binds go here. Slayer.clipseq carries 99 chains over 35 skills, so
	Play_Skill accepts ids; which key gets which skill is still a design call and
	is left to whoever owns the input map. */
}

/* External linkage comes from the extern declaration in the header. */
const CHARACTER_SPEC Spec_Slayer =
{
	"Slayer",

	TEXT("Prototype_Component_Model_Slayer"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,

	/* Item 1016002 is this class's default weapon and names WP_WWBK_03. That
	package also holds wp_wwbk_03_sk_bk, an 11-bone variant with its own
	4-sequence animset; the plain single-bone mesh is the one the table points
	at, so it is cooked static like the other weapons. */
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	/* IDLE, RUN, HIT, DEAD -- FBX takes are "<armature>_<action>" and the
	armature is named "wbk", so every clip carries that prefix. */
	{
		"wbk_idle_battle_1",
		"wbk_run_battle_1",
		"wbk_dmg_idle_1",
		"wbk_dead_1",
	},

	&Create_Logic,
};

NS_END
