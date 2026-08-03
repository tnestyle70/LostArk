#pragma once

#include "Client_Defines.h"
#include "CharacterSpec.h"

NS_BEGIN(Client)

/* Slayer (wbk_f). Owned by JS.

The game calls this class Berserker_Female internally -- EFTable_PC row 112,
base class 111 Warrior_Female -- so the class assets sit under the wbk_f prefix
while the body comes from wr_f. Its Action file is
XmlData/Action/BERSERKER_FEMALE.loa and its skills are the 45xxx block
(Skill.LearnClass = 112).

Same file-pair shape as the other classes: the logic plus the spec constant
CCharacter assembles from. */
class CLogic_Slayer final : public ICharacterLogic
{
public:
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) override;
};

/* Defined in the .cpp so the tags and clip names stay out of the header. */
extern const CHARACTER_SPEC Spec_Slayer;

NS_END
