#include "Logic_GunSlinger.h"

#include "Character.h"

namespace
{
	/* Dual pistols. The gn_f rig has no "b_weapon_*" bone at all -- it carries
	b_wp_1 under bip001-r-hand and b_wp_2 under bip001-l-hand -- so LanceMaster's
	socket name would have attached nothing and rendered the gun unparented at
	thousands of times its size. Both hands share one model; WP_WGDH_02 is a
	single handgun mesh and the sockets orient it. */
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_GunSlinger_Weapon"), "b_wp_1" },
		{ TEXT("Part_91_Weapon_L"),
		  TEXT("Prototype_Component_Model_GunSlinger_Weapon"), "b_wp_2" },
	};

	/* Equipment part tags sort after "Part_00_Body" so the body still updates
	first, and before the weapon tags. */
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_GunSlinger_Arm"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HANDS },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_GunSlinger_Helmet"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::HEAD },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_GunSlinger_Lower"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::LOWER },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_GunSlinger_Shoulder"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::SHOULDER },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_GunSlinger_Upper"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT,
		  EQUIPMENT_PRESENTATION_SLOT::UPPER },
	};

	/* The armour carries its own exposed skin, so the bare torso, arms and legs
	underneath must not be drawn. Submesh order is NOT the same as LanceMaster's:
	the gn_f body bundles the head, so the cook lays it out
	0 face / 1 eye_ao / 2 eye / 3 upper / 4 hair / 5 arm / 6 lower.
	Re-cooking the body with different content invalidates these bits. */
	constexpr uint32_t COVERED_BY_ARMOUR = (1u << 3) | (1u << 5) | (1u << 6);

	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_GunSlinger>();
	}
}

NS_BEGIN(Client)

void CLogic_GunSlinger::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

/* External linkage comes from the extern declaration in the header. */
const CHARACTER_SPEC Spec_GunSlinger =
{
	"GunSlinger",
	LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,

	TEXT("Prototype_Component_Model_GunSlinger"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,

	/* Item 1056002 is this class's default weapon and names WP_WGDH_02, the
	handgun of that set. The rifle and shotgun are cooked and waiting for a
	swap path. */
	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	/* IDLE, RUN, HIT, DEAD, KNOCKDOWN, KNOCKDOWN_LAND, DOWN_LOOP, STANDUP --
	FBX takes are "<armature>_<action>" and the armature is named "gdh", so
	every clip carries that prefix. */
	{
		"gdh_idle_battle_1",
		"gdh_run_battle_1",
		"gdh_dmg_idle_1",
		"gdh_dead_1",
		"gdh_knockdown",
		"gdh_knockdown_land",
		"gdh_down",
		"gdh_standup_1",
		/* No Esther animation set is attached for this class yet. */
		nullptr,
	},

	&Create_Logic,
};

NS_END
