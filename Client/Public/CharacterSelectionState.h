#pragma once

#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <string>
#include <string_view>

NS_BEGIN(Client)

enum class CHARACTER_ENTRY_IDENTITY_SOURCE
{
	AUDITION,
	CREATED,
	PENDING_CREATION,
	END
};

struct CHARACTER_ENTRY_IDENTITY final
{
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string strNickname;
	CHARACTER_ENTRY_IDENTITY_SOURCE eSource =
		CHARACTER_ENTRY_IDENTITY_SOURCE::END;
};

class CCharacterSelectionState final
{
public:
	static bool_t Select(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static bool_t Has_Selection();
	static bool_t Try_Get_SelectedClass(
		LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass);
	static bool_t Stage_Creation(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickname);
	static bool_t Has_PendingCreation();
	static bool_t Commit_PendingCreation();
	static void Cancel_PendingCreation();
	static bool_t Try_Resolve_ForWorld(
		LostArk::Shared::WORLD_ID worldId,
		CHARACTER_ENTRY_IDENTITY& outIdentity);
};

NS_END
