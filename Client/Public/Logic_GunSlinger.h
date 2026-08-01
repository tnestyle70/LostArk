#pragma once

#include "Client_Defines.h"
#include "CharacterSpec.h"

NS_BEGIN(Client)

/* GunSlinger (gdh_f). Owned by JS.

The game calls this class Devilhunter_Female internally -- EFTable_PC row 512 --
so every asset it needs sits under the gdh_f prefix while the base body comes
from gn_f. See .md/GB/JS/2026-08-01_LOSTARK_GUNSLINGER_ASSET_EXTRACTION_RESULT.md.

Same file-pair shape as CLogic_LanceMaster: the logic plus the spec constant
CCharacter assembles from. */
class CLogic_GunSlinger final : public ICharacterLogic
{
public:
	virtual void Update(CCharacter& Character, f32_t fTimeDelta) override;
};

/* Defined in the .cpp so the tags and clip names stay out of the header. */
extern const CHARACTER_SPEC Spec_GunSlinger;

NS_END
