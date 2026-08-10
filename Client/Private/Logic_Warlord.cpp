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
		/* Hidden while the helmet is worn: the loose hairstyle pokes through the
		shell. The part stays created for a later helmet toggle. */
		{ TEXT("Part_10_Equip_Hair"),     TEXT("Prototype_Component_Model_Warlord_Hair"),
		  0u, true },
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

	/* Defence stance is a posture, not a weapon swap: the class crouches behind
	the shield and keeps that pose until it leaves the stance, so idle and run
	both belong to the stance rather than to the class. */
	constexpr STANCE_LOCOMOTION_SPEC StanceLocomotion[] =
	{
		{ LostArk::Shared::PLAYER_STANCE_ID::WARLORD_DEFENSE,
		  "wgl_sk_defence_loop", "wgl_run_defence_1" },
	};

	/* The source rig keys only the first bone of each chain and leaves _02
	through _05 in bind pose, because the original solves them at runtime. These
	are those chains: the animation still swings the panel from the hip, and the
	links below it trail.

	The solver measures every link's rest direction inside its parent's already
	simulated frame, so gravity and the travel wind sag the chain a little more
	at every link and the tip carries the sum. Stiffness is only what carries
	the chain along with the body -- the shape comes from the forces. Damping
	is the fraction of velocity a link keeps per step; past half it starts to
	ring like a spring.

	The skirt stays stiffer and drier than the cape: it has no leg collision
	yet, and the further it strays from the animation the deeper it cuts into
	the thighs. */
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

	/* The helmet plume. The helmet mesh skins to bip001-head plus its own
	b_helmet_b chain -- not to the b_hair01_b chain, which only the hidden
	loose hairstyle uses. It hangs off the fast-moving head, so it stays
	stiffer and drier than the cape. */
	constexpr f32_t PLUME_STIFFNESS = 0.12f;
	constexpr f32_t PLUME_DAMPING = 0.5f;
	constexpr f32_t PLUME_GRAVITY = 15.f;
	constexpr f32_t PLUME_MAX_DISPLACEMENT = 0.25f;
	constexpr f32_t PLUME_WIND_RESPONSE = 3.f;

	constexpr BONE_CHAIN_SPEC BoneChains[] =
	{
		{ "b_skirt_f_01",  5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE },
		{ "b_skirt_fl_01", 5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE },
		{ "b_skirt_fr_01", 5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE },
		{ "b_skirt_l_01",  5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE },
		{ "b_skirt_r_01",  5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE },
		{ "b_skirt_b_01",  5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE },
		{ "b_skirt_bl_01", 5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE },
		{ "b_skirt_br_01", 5u, PLATE_STIFFNESS, PLATE_DAMPING,
		  PLATE_GRAVITY, PLATE_MAX_DISPLACEMENT, PLATE_WIND_RESPONSE },

		{ "b_capatcloth_l_01", 5u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE },
		{ "b_capatcloth_b_01", 5u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE },
		{ "b_capatcloth_r_01", 5u, CAPE_STIFFNESS, CAPE_DAMPING,
		  CAPE_GRAVITY, CAPE_MAX_DISPLACEMENT, CAPE_WIND_RESPONSE },

		{ "b_helmet_b_01", 4u, PLUME_STIFFNESS, PLUME_DAMPING,
		  PLUME_GRAVITY, PLUME_MAX_DISPLACEMENT, PLUME_WIND_RESPONSE },
	};

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

	nullptr,
	0u,

	StanceLocomotion,
	static_cast<uint32_t>(size(StanceLocomotion)),

	BoneChains,
	static_cast<uint32_t>(size(BoneChains)),
};

NS_END
