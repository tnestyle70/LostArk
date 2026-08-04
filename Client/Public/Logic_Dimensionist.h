#pragma once

#include "Client_Defines.h"
#include "CharacterSpec.h"

NS_BEGIN(Client)

// Dimensionist uses one combined animated body WModel. Gameplay authority
// remains in the server; this class only owns local presentation updates.
class CLogic_Dimensionist final : public ICharacterLogic
{
public:
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) override;
};

extern const CHARACTER_SPEC Spec_Dimensionist;

NS_END
