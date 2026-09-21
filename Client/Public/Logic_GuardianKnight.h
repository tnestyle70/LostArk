#pragma once

#include "Client_Defines.h"
#include "CharacterSpec.h"

NS_BEGIN(Client)

/* Guardian Knight (ddk). Owned by JS.

The game calls this class DragonKnight internally -- EFTable_PC row 702, base
class 701 DragonHuman -- so the class assets sit under the ddk prefix while the
body comes from dk. Its Action file is XmlData/Action/DRAGONKNIGHT.loa and its
skills are the 49xxx block (Skill.LearnClass = 702).

Same file-pair shape as the other classes: the logic plus the spec constant
CCharacter assembles from. */
class CLogic_GuardianKnight final : public ICharacterLogic
{
public:
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) override;
};

/* Defined in the .cpp so the tags and clip names stay out of the header. */
extern const CHARACTER_SPEC Spec_GuardianKnight;

NS_END
