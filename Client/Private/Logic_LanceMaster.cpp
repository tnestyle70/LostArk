#include "Logic_LanceMaster.h"

#include "Character.h"

namespace
{
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_LanceMaster_Weapon"), "b_weapon_rhand" },
	};

	/* Equipment part tags sort after "Part_00_Body" so the body still updates
	first, and before "Part_90_Weapon_R". */
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_LanceMaster_Arm") },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_LanceMaster_Helmet") },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_LanceMaster_Lower") },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_LanceMaster_Shoulder") },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_LanceMaster_Upper") },
	};

	/* The armour carries its own exposed skin, so the bare arm, torso and legs
	underneath must not be drawn. Submesh order comes from the cook:
	0 arm / 1 upper / 2 lower / 3 face / 4 eyelashes / 5 eye / 6 hair.
	Re-cooking the body with different content invalidates these bits. */
	constexpr uint32_t COVERED_BY_ARMOUR = (1u << 0) | (1u << 1) | (1u << 2);

	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<CLogic_LanceMaster>();
	}
}

NS_BEGIN(Client)

void CLogic_LanceMaster::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

/* External linkage comes from the extern declaration in the header. */
const CHARACTER_SPEC Spec_LanceMaster =
{
	"LanceMaster",

	TEXT("Prototype_Component_Model_LanceMaster"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,

	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	/* IDLE, RUN, HIT, DEAD -- FBX takes are "<armature>_<action>", so the clips
	carry the "flm_" prefix. */
	{
		"flm_idle_battle_1",
		"flm_run_battle_1",
		"flm_dmg_idle_1",
		"flm_dead_1",
	},

	&Create_Logic,
};

NS_END
