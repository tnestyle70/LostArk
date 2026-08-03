#pragma once

#include "Client_Defines.h"
#include "CharacterSpec.h"

NS_BEGIN(Client)

/* Artist (sdm). Owned by JS.

The game calls this class YinYangShi internally -- EFTable_PC row 602, base
class 601 Specialist -- so the class assets sit under the sdm prefix while the
body comes from sp. Its Action file is XmlData/Action/YINYANGSHI.loa and its
skills are the 31xxx block (Skill.LearnClass = 602).

Same file-pair shape as CLogic_LanceMaster and CLogic_GunSlinger: the logic plus
the spec constant CCharacter assembles from. */
class CLogic_Artist final : public ICharacterLogic
{
public:
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) override;
};

/* Defined in the .cpp so the tags and clip names stay out of the header. */
extern const CHARACTER_SPEC Spec_Artist;

NS_END
