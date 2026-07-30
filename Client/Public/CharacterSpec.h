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
};

struct EQUIPMENT_PART_SPEC
{
	const tchar_t* pPartTag;      /* sorts after "Part_Body", see CCharacter */
	const tchar_t* pModelTag;
};

/* Everything that makes one class different, as data. Held as a constant next to
that class's logic; CCharacter itself stays class-agnostic. */
struct CHARACTER_SPEC
{
	const tchar_t* pBodyModelTag;
	const tchar_t* pShaderTag;

	/* Bit i hides body submesh i, for the skin the equipment already carries. */
	uint32_t iBodyHiddenMeshMask;

	const tchar_t* pWeaponModelTag;
	const tchar_t* pWeaponShaderTag;
	const char_t* pWeaponSocketBone;

	const EQUIPMENT_PART_SPEC* pEquipment;
	uint32_t iNumEquipment;

	/* Clip names differ per class ("flm_idle_battle_1", "wbk_..."), so the shared
	states are looked up through this table instead of by index. */
	const char_t* AnimationClips[ETOUI(CHARACTER_ANIM::END)];

	unique_ptr<ICharacterLogic> (*pCreateLogic)();
};

NS_END
