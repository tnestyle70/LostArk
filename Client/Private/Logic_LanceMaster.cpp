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

	/* Skill names are the game's own, straight out of the clipmap, so that a bind
	can be checked against LanceMaster.clipseq by eye. That needs Korean, so this
	file pair is saved as UTF-8 with a BOM -- without the BOM MSVC reads it as
	CP949 like the rest of Client/ and warns C4819 on every build.

	Which skill sits on which key is a design call, not something the data
	decides -- edit these two tables, nothing else. */
	constexpr SKILL_BIND LongBinds[] =
	{
		{ DIK_Q, 34040 },   /* 이연격    */
		{ DIK_W, 34060 },   /* 열공참    */
		{ DIK_E, 34070 },   /* 회선창    */
		{ DIK_R, 34110 },   /* 반월섬    */
		{ DIK_A, 34120 },   /* 연환섬    */
		{ DIK_S, 34130 },   /* 질풍참    */
		{ DIK_D, 34140 },   /* 선풍참혼  */
		{ DIK_F, 34150 },   /* 맹룡열파  */
		{ DIK_C, 34100 },   /* 청룡출수  */
		{ DIK_X, 34010 },   /* basic attack */
		{ DIK_1, 34630 },   /* awakening, plays out */
	};

	constexpr SKILL_BIND ShortBinds[] =
	{
		{ DIK_Q, 34540 },   /* 나선창    */
		{ DIK_W, 34550 },   /* 사두룡격  */
		{ DIK_E, 34560 },   /* 굉열파    */
		{ DIK_R, 34570 },   /* 유성강천  */
		{ DIK_A, 34580 },   /* 절룡세    */
		{ DIK_S, 34590 },   /* 적룡포    */
		{ DIK_D, 34520 },   /* 돌파, the dash */
		{ DIK_X, 34510 },   /* basic attack */
		{ DIK_1, 34630 },   /* awakening, plays out */
	};

	/* The identity itself. Each stance owns the skill that leaves it. */
	constexpr uint8_t STANCE_KEY = DIK_Z;
	constexpr int32_t SWITCH_TO_SHORT = 34000;
	constexpr int32_t SWITCH_TO_LONG = 34500;

	/* Both spears ride the same socket; the stance decides which one is drawn.
	Part tags are what Set_PartVisible is called with. */
	constexpr tchar_t WEAPON_PART_LONG[] = TEXT("Part_90_Weapon_R");
	constexpr tchar_t WEAPON_PART_SHORT[] = TEXT("Part_91_Weapon_R_Short");

	constexpr WEAPON_PART_SPEC Weapons[] =
	{
		{ WEAPON_PART_LONG,
		  TEXT("Prototype_Component_Model_LanceMaster_Weapon"), "b_weapon_rhand" },
		{ WEAPON_PART_SHORT,
		  TEXT("Prototype_Component_Model_LanceMaster_Weapon_Short"), "b_weapon_rhand" },
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

void CLogic_LanceMaster::Apply_Stance(CCharacter& Character)
{
	const bool_t isLong = STANCE::LONG == m_eStance;
	Character.Set_PartVisible(WEAPON_PART_LONG, isLong);
	Character.Set_PartVisible(WEAPON_PART_SHORT, !isLong);
}

void CLogic_LanceMaster::Update(CCharacter& Character, f32_t fTimeDelta)
{
	/* Skills and state transitions land here. The body already advances the clock,
	so this only has to pick the clip. */

	if (!m_isStanceApplied)
	{
		Apply_Stance(Character);
		m_isStanceApplied = true;
	}

	/* A chain owns the character until it ends. Cancel windows are extracted but
	not wired yet, so a cast cannot be interrupted. */
	if (Character.Is_PlayingSkill())
		return;

	const bool_t isLong = STANCE::LONG == m_eStance;
	const SKILL_BIND* pBinds = isLong ? LongBinds : ShortBinds;
	const size_t iCount = isLong ? size(LongBinds) : size(ShortBinds);

	/* The stance key first: in either stance it casts that stance's own exit
	skill, so one key drives the whole identity. */
	{
		const bool_t bDown =
			0 != (CGameInstance::Get().Get_DIKeyState(STANCE_KEY) & 0x80);
		const bool_t bWasDown = m_bKeyDown[STANCE_KEY];
		m_bKeyDown[STANCE_KEY] = bDown;

		if (bDown && !bWasDown)
		{
			const int32_t iSwitch = isLong ? SWITCH_TO_SHORT : SWITCH_TO_LONG;
			if (Character.Play_Skill(iSwitch))
			{
				/* The weapon and the skill block change on the press, not when
				the transition clip ends -- that is what the original does, and
				the clip reads as the character already holding the other spear.
				Waiting for it to finish left the old weapon in hand through the
				whole swap. */
				m_eStance = isLong ? STANCE::SHORT : STANCE::LONG;
				Apply_Stance(Character);
				return;
			}
		}
	}

	for (size_t i = 0; i < iCount; ++i)
	{
		const SKILL_BIND& bind = pBinds[i];
		const bool_t bDown =
			0 != (CGameInstance::Get().Get_DIKeyState(bind.byKey) & 0x80);

		/* DirectInput is polled as a level, so the press edge is tracked here --
		holding a key must not restart the skill every frame. */
		const bool_t bWasDown = m_bKeyDown[bind.byKey];
		m_bKeyDown[bind.byKey] = bDown;

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
