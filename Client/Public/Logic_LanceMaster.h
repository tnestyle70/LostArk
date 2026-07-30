#pragma once

#include "Client_Defines.h"
#include "CharacterSpec.h"

NS_BEGIN(Client)

/* LanceMaster (flm). Owned by JS.

Everything that makes this class different lives in this file pair: the logic that
drives it, and the spec constant CCharacter assembles from. Adding another class
means adding a file pair like this one, not another GameObject. */
class CLogic_LanceMaster final : public ICharacterLogic
{
public:
	virtual void Update(CCharacter& Character, f32_t fTimeDelta) override;
};

/* Defined in the .cpp so the tags and clip names stay out of the header. The name
carries the class because every spec shares one namespace. */
extern const CHARACTER_SPEC Spec_LanceMaster;

NS_END
