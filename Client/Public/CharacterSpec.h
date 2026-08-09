#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

NS_BEGIN(Client)

class CCharacter;

/* Animation states every class has. Skill clips are not listed here: their count
and ordering differ per class, so the logic addresses those by name itself. */
enum class CHARACTER_ANIM { IDLE, RUN, HIT, DEAD, END };

/* Per-class presentation behaviour. Input, network transport and gameplay truth
stay outside this interface. One implementation per class keeps team ownership
separate without allowing class code to issue commands. */
class ICharacterLogic
{
public:
	virtual ~ICharacterLogic() = default;

public:
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) = 0;
};

struct EQUIPMENT_PART_SPEC
{
	const tchar_t* pPartTag;      /* sorts after "Part_Body", see CCharacter */
	const tchar_t* pModelTag;

	/* Bit i hides submesh i of this piece, for content the body already draws. */
	uint32_t iHiddenMeshMask;
};

/* A piece that rides one bone instead of the whole palette. Classes differ in how
many they carry and what the bone is called -- LanceMaster holds one lance at
b_weapon_rhand, GunSlinger dual-wields at b_wp_1 and b_wp_2 -- so this is a list,
not a fixed pair of fields. Character creation must reject a missing socket; an
unparented weapon is invalid content, not a runtime fallback. */
struct WEAPON_PART_SPEC
{
	const tchar_t* pPartTag;      /* sorts after equipment, e.g. "Part_90_Weapon_R" */
	const tchar_t* pModelTag;
	const char_t* pSocketBone;
	f32_t fSocketYawDegrees = 0.f;
	LostArk::Shared::PLAYER_STANCE_ID eRequiredStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
};

/* Optional per-skill presentation authored beside the owning class.  This is
visual state only; Server action snapshots still decide when it is active. */
struct SKILL_SURFACE_EMISSIVE_SPEC
{
	LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
	float4_t vColor = float4_t(1.f, 1.f, 1.f, 1.f);
	f32_t fIntensity = 0.f;
};

/* A stance the class stands and runs differently in. Only the stances a class
lists here replace the shared IDLE and RUN clips; every other stance keeps them,
so a class that merely swaps weapons needs no entry. */
struct STANCE_LOCOMOTION_SPEC
{
	LostArk::Shared::PLAYER_STANCE_ID eStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	const char_t* pIdleClip = nullptr;
	const char_t* pRunClip = nullptr;
};

/* Everything that makes one class different, as data. Held as a constant next to
that class's logic; CCharacter itself stays class-agnostic. */
struct CHARACTER_SPEC
{
	/* Stable data ID. For example LanceMaster resolves under
	Data/Animation/{Authored,Reference}/LanceMaster. */
	const char_t* pAssetName;

	/* Which class this spec is. Carried here rather than plumbed at spawn so a
	character knows its own class in local preview too, where no snapshot ever
	arrives to tell it. Input binding resolves quick slots through this. */
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass;

	const tchar_t* pBodyModelTag;
	const tchar_t* pShaderTag;

	/* Bit i hides body submesh i, for the skin the equipment already carries. */
	uint32_t iBodyHiddenMeshMask;

	const tchar_t* pWeaponShaderTag;
	const WEAPON_PART_SPEC* pWeapons;
	uint32_t iNumWeapons;

	const EQUIPMENT_PART_SPEC* pEquipment;
	uint32_t iNumEquipment;

	/* Clip names differ per class ("flm_idle_battle_1", "wbk_..."), so the shared
	states are looked up through this table instead of by index. */
	const char_t* AnimationClips[ETOUI(CHARACTER_ANIM::END)];

	unique_ptr<ICharacterLogic> (*pCreateLogic)();

	const SKILL_SURFACE_EMISSIVE_SPEC* pSkillSurfaceEmissives = nullptr;
	uint32_t iNumSkillSurfaceEmissives = 0u;

	const STANCE_LOCOMOTION_SPEC* pStanceLocomotion = nullptr;
	uint32_t iNumStanceLocomotion = 0u;
};

NS_END
