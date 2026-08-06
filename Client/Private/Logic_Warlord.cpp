#include "Logic_Warlord.h"

#include "Character.h"

namespace
{
	/* A gunlance in the right hand and a shield in the left. The wr rig carries
	the LanceMaster family of socket names -- b_weapon_rhand / b_weapon_lhand,
	with no b_wp_* at all -- plus a third, b_weapon_hold_01, that the earlier
	classes do not have. The socket name has to be read off each class's own rig:
	one the rig lacks fails silently, since Get_BoneMatrix returns identity and
	the body's 0.0001 preTransform never reaches the weapon. */
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_Warlord_Weapon"), "b_weapon_rhand" },
		{ TEXT("Part_90_Weapon_L"),
		  TEXT("Prototype_Component_Model_Warlord_Shield"), "b_weapon_lhand" },
	};

	/* Equipment part tags sort after "Part_00_Body" so the body still updates
	first, and before the weapon tags. Hair is an equipment part here because the
	body does not draw it -- see the mask note below. */
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_Warlord_Arm") },
		{ TEXT("Part_10_Equip_Hair"),     TEXT("Prototype_Component_Model_Warlord_Hair") },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_Warlord_Helmet") },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_Warlord_Lower") },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_Warlord_Shoulder") },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_Warlord_Upper") },
	};

	/* The cook lays this body out 0 lower / 1 upper / 2 base_upper / 3 arm /
	4 face / 5 eye, so the four skin meshes below the head are hidden and the
	armour draws them instead.

	This class splits the head differently from every earlier one: pc_wr_00_sk
	lists face and eye materials but no hair material, so the face and eyes stay
	visible on the body while the hair ships as its own part. Slayer and Artist
	draw all three from the body; LanceMaster draws none of them. These bits are
	read off the cooked model every time, never copied from another class, and
	re-cooking the body with different content invalidates them. */
	constexpr uint32_t COVERED_BY_ARMOUR =
		(1u << 0) | (1u << 1) | (1u << 2) | (1u << 3);

	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_Warlord>();
	}
}

NS_BEGIN(Client)

void CLogic_Warlord::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

/* External linkage comes from the extern declaration in the header. */
const CHARACTER_SPEC Spec_Warlord =
{
	"Warlord",
	LostArk::Shared::CHARACTER_CLASS_ID::WARLORD,

	TEXT("Prototype_Component_Model_Warlord"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,

	/* Item 1013002 is this class's default weapon and names WP_WWGL_04. That
	package holds two meshes, the gunlance and the shield the class carries in
	its off hand, and both are cooked static like the other weapons. */
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	/* IDLE, RUN, HIT, DEAD -- FBX takes are "<armature>_<action>" and the
	armature is named "wgl", so every clip carries that prefix. */
	{
		"wgl_idle_battle_1",
		"wgl_run_battle_1",
		"wgl_dmg_idle_1",
		"wgl_dead_1",
	},

	&Create_Logic,
};

NS_END
