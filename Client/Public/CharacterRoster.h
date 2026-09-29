#pragma once

#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

/* The characters the character-select window seats on its cards. There is no account
system, so the roster is local to this PC and starts empty: a card is added when a character
is created (Add), and the whole roster is saved to %LOCALAPPDATA%/LostArk/CharacterRoster.json
so it survives a restart. Cards stand in class order (Warlord, Lance Master, Artist, Guardian
Knight from the left) whatever order they were created in. The class of a card is fixed at
creation; only its nickname changes. The nickname is checked by the same rule the Server applies
on entry (Is_Valid_PlayerNickname). */
struct CHARACTER_ROSTER_ENTRY
{
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string strNickname;
};

class CCharacterRoster final
{
public:
	/* The character-select window has six cards. */
	static constexpr size_t MAX_CHARACTERS = 6;

	/* The saved characters, in card order. A missing file is an empty roster; an unreadable or
	older-format one is treated the same way. */
	static const std::vector<CHARACTER_ROSTER_ENTRY>& Get_Entries();
	/* Validates, saves the whole roster atomically, then updates memory. False keeps the old
	nickname and leaves the reason in outStatus. */
	static bool_t Rename(size_t iIndex, const std::string& strNickname, std::string& outStatus);
	/* Validates, appends a newly created character, saves the whole roster atomically, then
	updates memory. False leaves the roster as it was and the reason in outStatus. */
	static bool_t Add(LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass,
		const std::string& strNickname, size_t& outIndex, std::string& outStatus);
};

NS_END
