#pragma once

#include "Engine_Defines.h"
#include "Network/PacketMessages.h"
#include "Network/PacketType.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

/* One EXE owns an initially empty roster of up to six characters. Names, appearance and the
last authoritative inventory/purse/title stay in memory while switching characters. Exiting
ends the roster; existing personal files are never read, written or removed. */
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
	/* Stable process-session identity; nickname and display order are not lookup keys. */
	std::string strCharacterId;
};

class CCharacterRoster final
{
public:
	/* The character-select window has six cards. */
	static constexpr size_t MAX_CHARACTERS = 6;

	/* Six stable card slots; an empty slot has no character ID. Slots are never sorted. */
	static const std::vector<CHARACTER_ROSTER_ENTRY>& Get_Entries();
	static size_t Get_CharacterCount();
	static bool_t Is_Occupied(size_t slot);
	static bool_t Rename(size_t iIndex, const std::string& strNickname, std::string& outStatus);
	/* Replace only the matching session character; failures preserve the previous state. */
	static bool_t Update_WorldState(const std::string& strCharacterId, const CHARACTER_WORLD_STATE& State, std::string& outStatus);
	static bool_t Try_Get_WorldState(const std::string& strCharacterId, CHARACTER_WORLD_STATE& outState);
	static bool_t Add(size_t slot, LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass,
		const std::string& strNickname, const std::string& strAppearanceJson,
		size_t& outIndex, std::string& outStatus);
};

NS_END
