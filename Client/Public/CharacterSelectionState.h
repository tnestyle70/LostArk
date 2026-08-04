#pragma once

#include "Engine_Defines.h"
#include "Network/PacketType.h"

NS_BEGIN(Client)

enum class CHARACTER_TEST_ENTRY_MODE
{
	NONE,
	SERVER_GAMEPLAY
};

class CCharacterSelectionState final
{
public:
	static bool_t Select(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static bool_t Has_Selection();
	static bool_t Try_Get_SelectedClass(
		LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass);
	static bool_t Stage_TestEntryMode(
		CHARACTER_TEST_ENTRY_MODE entryMode);
	static bool_t Try_Consume_TestEntryMode(
		CHARACTER_TEST_ENTRY_MODE& outEntryMode);
	static void Clear_TestEntryMode();
};

NS_END
