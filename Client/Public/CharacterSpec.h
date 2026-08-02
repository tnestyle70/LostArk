#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CCharacter;

/* Animation states every class has. Skill clips are not listed here: their count
and ordering differ per class, so the logic addresses those by name itself. */
enum class CHARACTER_ANIM { IDLE, RUN, HIT, DEAD, END };

/* Per-class behaviour. One implementation per class, each in its own file so the
four of us never edit the same source. */
class ICharacterLogic
{
public:
	virtual ~ICharacterLogic() = default;

public:
	virtual void Update(CCharacter& Character, f32_t fTimeDelta) = 0;

	/* Which clip one of the shared states should use right now, or nullptr to
	take the spec's. The spec table is one row per class, so a class whose
	identity changes its stance cannot express itself there: LanceMaster idles and
	runs on flm_*_identity1 holding the long spear and identity2 holding the short
	one. Classes without that answer nullptr and keep the table. */
	virtual const char_t* Resolve_AnimationClip(CHARACTER_ANIM eAnim) const {
		return nullptr;
	}
};

struct EQUIPMENT_PART_SPEC
{
	const tchar_t* pPartTag;      /* sorts after "Part_Body", see CCharacter */
	const tchar_t* pModelTag;
};

/* A piece that rides one bone instead of the whole palette. Classes differ in how
many they carry and what the bone is called -- LanceMaster holds one lance at
b_weapon_rhand, GunSlinger dual-wields at b_wp_1 and b_wp_2 -- so this is a list,
not a fixed pair of fields. A socket bone the rig does not have fails silently:
Get_BoneMatrix returns identity and the piece renders unparented. */
struct WEAPON_PART_SPEC
{
	const tchar_t* pPartTag;      /* sorts after equipment, e.g. "Part_90_Weapon_R" */
	const tchar_t* pModelTag;
	const char_t* pSocketBone;
};

/* Everything that makes one class different, as data. Held as a constant next to
that class's logic; CCharacter itself stays class-agnostic. */
struct CHARACTER_SPEC
{
	/* Names the data files that belong to this class, e.g. "LanceMaster" finds
	../Bin/DataFiles/Anim/LanceMaster.clipseq. */
	const char_t* pAssetName;

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
};

NS_END
