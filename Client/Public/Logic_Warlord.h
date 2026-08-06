#pragma once

#include "Client_Defines.h"
#include "CharacterSpec.h"

NS_BEGIN(Client)

/* Warlord (wgl). Owned by JS.

The game calls this class Gunlancer internally -- EFTable_PC row 104, base class
101 Warrior -- so the class assets sit under the wgl prefix while the body comes
from wr. Its Action file is XmlData/Action/GUNLANCER.loa and its skills are the
17xxx block (Skill.LearnClass = 104).

Same file-pair shape as the other classes: the logic plus the spec constant
CCharacter assembles from. */
class CLogic_Warlord final : public ICharacterLogic
{
public:
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) override;
};

/* Defined in the .cpp so the tags and clip names stay out of the header. */
extern const CHARACTER_SPEC Spec_Warlord;

NS_END
