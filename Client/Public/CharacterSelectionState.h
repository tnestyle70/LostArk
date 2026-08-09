#pragma once

#include "Engine_Defines.h"
#include "Network/PacketType.h"

NS_BEGIN(Client)

class CCharacterSelectionState final
{
public:
	static bool_t Select(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static bool_t Has_Selection();
	static bool_t Try_Get_SelectedClass(
		LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass);
};

NS_END
