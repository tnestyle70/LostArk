#include "Logic_LanceMaster.h"

#include "Character.h"

namespace
{
	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ TEXT("Part_90_Weapon_R"),
		  TEXT("Prototype_Component_Model_LanceMaster_Weapon"), "b_weapon_rhand", 0.f,
		  LostArk::Shared::PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR },
		{ TEXT("Part_91_Weapon_R_Short"),
		  TEXT("Prototype_Component_Model_LanceMaster_Weapon_Short"), "b_weapon_rhand", 0.f,
		  LostArk::Shared::PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR },
	};

	/* Equipment part tags sort after "Part_00_Body" so the body still updates
	first, and before "Part_90_Weapon_R". */	
	constexpr EQUIPMENT_PART_SPEC Equipment[] =
	{
		{ TEXT("Part_10_Equip_Arm"),      TEXT("Prototype_Component_Model_LanceMaster_Arm") },
		{ TEXT("Part_10_Equip_Helmet"),   TEXT("Prototype_Component_Model_LanceMaster_Helmet"),
		  0u, false, EQUIPMENT_SLOT_KIND::DEFAULT_HELMET },
		{ TEXT("Part_10_Equip_Lower"),    TEXT("Prototype_Component_Model_LanceMaster_Lower") },
		{ TEXT("Part_10_Equip_Shoulder"), TEXT("Prototype_Component_Model_LanceMaster_Shoulder") },
		{ TEXT("Part_10_Equip_Upper"),    TEXT("Prototype_Component_Model_LanceMaster_Upper") },

		/* Mokoko avatar test slots: presence hides the base parts they cover,
		see CCharacter::Ready_PartObjects. */
		{ TEXT("Part_15_Avatar_Head"),    TEXT("Prototype_Component_Model_LanceMaster_Helmet_Mokoko"),
		  0u, false, EQUIPMENT_SLOT_KIND::AVATAR_HEAD },
		{ TEXT("Part_15_Avatar_Armor"),   TEXT("Prototype_Component_Model_LanceMaster_Upper_Mokoko"),
		  0u, false, EQUIPMENT_SLOT_KIND::AVATAR_ARMOR },
	};

	/* The armour carries its own exposed skin, so the bare arm, torso and legs
	underneath must not be drawn. Submesh order comes from the cook:
	0 arm / 1 upper / 2 lower / 3 face / 4 eyelashes / 5 eye / 6 hair.
	Re-cooking the body with different content invalidates these bits. */
	constexpr uint32_t COVERED_BY_ARMOUR = (1u << 0) | (1u << 1) | (1u << 2);

	/* The rig keys only each chain's first bone and the original game solves
	the rest at runtime -- the same contract as the Warlord chains. The two
	upper_cloth chains are the long front and back battle-skirt drapes, the six
	upper_skirt chains are short two-link flaps around the hip, the four
	b_upper_pelat chains are the Upper armour's own hanging tassets -- by skin
	weight the largest cloth on the piece -- and the b_hair_fr/fl tufts are the
	bangs the helmet mesh itself skins. The long b_hair00/01 chains stay
	unlisted: no shipped mesh references them while the helmet is worn.

	The drapes hang between and behind the legs with no collision yet, so they
	run stiffer than the Warlord cape to keep them out of the thighs. */
	constexpr f32_t DRAPE_STIFFNESS = 0.04f;
	constexpr f32_t DRAPE_DAMPING = 0.6f;
	constexpr f32_t DRAPE_GRAVITY = 16.f;
	constexpr f32_t DRAPE_MAX_DISPLACEMENT = 0.3f;
	constexpr f32_t DRAPE_WIND_RESPONSE = 4.5f;

	constexpr f32_t FLAP_STIFFNESS = 0.12f;
	constexpr f32_t FLAP_DAMPING = 0.5f;
	constexpr f32_t FLAP_GRAVITY = 10.f;
	constexpr f32_t FLAP_MAX_DISPLACEMENT = 0.15f;
	constexpr f32_t FLAP_WIND_RESPONSE = 2.5f;

	constexpr f32_t BANGS_STIFFNESS = 0.15f;
	constexpr f32_t BANGS_DAMPING = 0.5f;
	constexpr f32_t BANGS_GRAVITY = 12.f;
	constexpr f32_t BANGS_MAX_DISPLACEMENT = 0.1f;
	constexpr f32_t BANGS_WIND_RESPONSE = 2.f;

	constexpr BONE_CHAIN_SPEC BoneChains[] =
	{
		{ "b_upper_cloth_f_01", 5u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE },
		{ "b_upper_cloth_b_01", 5u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE },

		{ "b_upper_pelat_br_01", 4u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE },
		{ "b_upper_pelat_bl_01", 4u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE },
		{ "b_upper_pelat_r_01",  5u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE },
		{ "b_upper_pelat_b_01",  4u, DRAPE_STIFFNESS, DRAPE_DAMPING,
		  DRAPE_GRAVITY, DRAPE_MAX_DISPLACEMENT, DRAPE_WIND_RESPONSE },

		{ "b_upper_skirt_fl_01", 2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE },
		{ "b_upper_skirt_fr_01", 2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE },
		{ "b_upper_skirt_l_01",  2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE },
		{ "b_upper_skirt_r_01",  2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE },
		{ "b_upper_skirt_bl_01", 2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE },
		{ "b_upper_skirt_br_01", 2u, FLAP_STIFFNESS, FLAP_DAMPING,
		  FLAP_GRAVITY, FLAP_MAX_DISPLACEMENT, FLAP_WIND_RESPONSE },

		{ "b_hair_fr_01", 2u, BANGS_STIFFNESS, BANGS_DAMPING,
		  BANGS_GRAVITY, BANGS_MAX_DISPLACEMENT, BANGS_WIND_RESPONSE },
		{ "b_hair_fl_01", 2u, BANGS_STIFFNESS, BANGS_DAMPING,
		  BANGS_GRAVITY, BANGS_MAX_DISPLACEMENT, BANGS_WIND_RESPONSE },
	};

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
	LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,

	TEXT("Prototype_Component_Model_LanceMaster"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	COVERED_BY_ARMOUR,

	TEXT("Prototype_Component_Shader_VtxMeshBinary"),
	Weapons,
	static_cast<uint32_t>(size(Weapons)),

	Equipment,
	static_cast<uint32_t>(size(Equipment)),

	/* IDLE, RUN, HIT, DEAD, KNOCKDOWN, KNOCKDOWN_LAND, DOWN_LOOP, STANDUP,
	ESTHER_CAST -- FBX takes are "<armature>_<action>", so the clips carry the
	"flm_" prefix. The cast clip comes from the attached Esther animation set. */
	{
		"flm_idle_battle_1",
		"flm_run_battle_1",
		"flm_dmg_idle_1",
		"flm_dead_1",
		"flm_knockdown",
		"flm_knockdown_land",
		"flm_down",
		"flm_standup_1",
		"flm_act_estherskill_1",
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
