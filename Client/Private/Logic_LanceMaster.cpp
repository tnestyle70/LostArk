#include "Logic_LanceMaster.h"

#include "Character.h"
#include "GameInstance.h"

namespace
{
	/* Key -> skill, for the skills that have a clip chain in LanceMaster.clipseq.
	Skill ids are the game's own (EFTable_Skill), so the id is what ties this to
	the extracted timing and notify data. */
	struct SKILL_BIND
	{
		uint8_t byKey;
		int32_t iSkillId;
	};

	/* Names are in the clipmap; this file stays ASCII so it keeps building under
	the CP949 code page the rest of Client/ is compiled with. */
	constexpr SKILL_BIND Binds[] =
	{
		{ DIK_Q, 34040 },   /* SharpSwing     */
		{ DIK_W, 34060 },   /* StrongRotational */
		{ DIK_E, 34070 },   /* Assault        */
		{ DIK_R, 34110 },   /* CrescentSweep  */
		{ DIK_A, 34120 },   /* ThreeTalonStrike */
		{ DIK_S, 34130 },   /* Ruffle         */
		{ DIK_D, 34140 },   /* ChestDestruction */
		{ DIK_F, 34150 },   /* CrushingBlow   */
		{ DIK_Z, 34100 },   /* CycloinLance   */
		{ DIK_X, 34010 },   /* basic attack   */
		{ DIK_1, 34630 },   /* awakening, plays out */
	};

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

void CLogic_LanceMaster::Update(CCharacter& Character, f32_t fTimeDelta)
{
	static_assert(size(Binds) <= size(CLogic_LanceMaster::m_bKeyDown),
		"m_bKeyDown must cover every bind");

	/* Skills and state transitions land here. The body already advances the clock,
	so this only has to pick the clip. */

	/* A chain owns the character until it ends. Cancel windows are extracted but
	not wired yet, so a cast cannot be interrupted. */
	if (Character.Is_PlayingSkill())
		return;

	for (const SKILL_BIND& bind : Binds)
	{
		const bool_t bDown =
			0 != (CGameInstance::Get().Get_DIKeyState(bind.byKey) & 0x80);

		/* DirectInput is polled as a level, so the press edge is tracked here --
		holding a key must not restart the skill every frame. */
		const bool_t bWasDown = m_bKeyDown[&bind - Binds];
		m_bKeyDown[&bind - Binds] = bDown;

		if (bDown && !bWasDown && Character.Play_Skill(bind.iSkillId))
			return;
	}
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
