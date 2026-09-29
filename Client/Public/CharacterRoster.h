#pragma once

#include "Engine_Defines.h"
#include "Network/PacketMessages.h"
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
/* What the Server had this character carrying when it last left the world: the whole inventory
(equipment is the items with an equipped slot), the purse and the worn honor title. The Client
keeps it and offers it back on the next entry; bValid is false until a first save. */
struct CHARACTER_WORLD_STATE
{
	bool_t bValid = false;
	std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT> Items;
	uint32_t iSilver = 0;
	uint32_t iGold = 0;
	uint32_t iHonorTitleId = 0;
};

struct CHARACTER_ROSTER_ENTRY
{
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string strNickname;
	/* The look made on the customizing screen (CCustomizingView::Serialize_Appearance), kept as
	that document's own text. Empty when the character was made without one. */
	std::string strAppearanceJson;
	CHARACTER_WORLD_STATE World;
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
	/* Replaces the world state of the character with this class and nickname and saves the whole
	roster atomically. False (no such character, or the save failed) leaves the roster as it was. */
	static bool_t Update_WorldState(LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass,
		const std::string& strNickname, const CHARACTER_WORLD_STATE& State, std::string& outStatus);
	static bool_t Try_Get_WorldState(LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass,
		const std::string& strNickname, CHARACTER_WORLD_STATE& outState);
	static bool_t Add(LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass,
		const std::string& strNickname, const std::string& strAppearanceJson,
		size_t& outIndex, std::string& outStatus);
};

NS_END
