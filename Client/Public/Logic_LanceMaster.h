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
	/* The two halves of this class. The game ships them as separate skill blocks
	that share one character: 긴 창 owns 34000..34170 and 짧은 창 owns 34500..34590,
	each with its own basic attack and its own dash, and a skill from the wrong
	block simply has no chain to play. So the key layout is per stance and the
	whole point of the identity is to move between them.

	Skills outside both blocks -- the awakening, the standup, just guard -- belong
	to neither and are bound in both tables. */
	enum class STANCE { LONG, SHORT, END };

public:
	virtual void Update(CCharacter& Character, f32_t fTimeDelta) override;

	STANCE Get_Stance() const {
		return m_eStance;
	}

private:
	/* Shows the stance's spear and hides the other. Both parts are assembled at
	spawn, so this is the only thing a stance change does to the visual. */
	void Apply_Stance(CCharacter& Character);

private:
	STANCE m_eStance = { STANCE::LONG };
	/* The parts exist only once the character is assembled, so the opening stance
	is pushed on the first update rather than in the constructor. */
	bool_t m_isStanceApplied = { false };
	/* The switch skill waiting to finish, 0 when none. The stance flips when its
	clip has played out rather than when it is cast, because that clip is the
	character changing weapons. */
	int32_t m_iSwitchingTo = {};
	/* Previous frame's state of a key, so a held key fires once. Indexed by the
	DirectInput code rather than by bind slot: the two stances put different
	skills on the same slot, and sharing a slot would carry one key's edge over
	onto the other's. */
	bool_t m_bKeyDown[256]{};
};

/* Defined in the .cpp so the tags and clip names stay out of the header. The name
carries the class because every spec shares one namespace. */
extern const CHARACTER_SPEC Spec_LanceMaster;

NS_END
