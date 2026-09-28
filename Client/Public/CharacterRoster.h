#pragma once

#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

/* The characters the character-select window seats on its cards. There is no account
system, so the roster is local to this PC: four fixed classes with default nicknames,
and a rename is saved to %LOCALAPPDATA%/LostArk/CharacterRoster.json so it survives a
restart. The class of each card is fixed; only its nickname changes. The nickname is still
checked by the same rule the Server applies on entry (Is_Valid_PlayerNickname). */
struct CHARACTER_ROSTER_ENTRY
{
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string strNickname;
};

class CCharacterRoster final
{
public:
	/* Defaults first, then the saved nicknames on top. A missing file is not an error; an
	unreadable one keeps the defaults and reports why. */
	static const std::vector<CHARACTER_ROSTER_ENTRY>& Get_Entries();
	/* Validates, saves the whole roster atomically, then updates memory. False keeps the old
	nickname and leaves the reason in outStatus. */
	static bool_t Rename(size_t iIndex, const std::string& strNickname, std::string& outStatus);
};

NS_END
