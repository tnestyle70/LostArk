#pragma once

#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

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

struct CHARACTER_WORLD_STATE;

class CCharacterSelectionState final
{
public:
	static bool_t Select(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static bool_t Has_Selection();
	static bool_t Try_Get_SelectedClass(
		LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass);
	/* A character created just now: once Bern is entered it is added to the session roster with
	the look it was made with (CCustomizingView::Serialize_Appearance, empty for none). */
	static bool_t Select_CreationSlot(size_t slot);
	static bool_t Stage_Creation(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickname, std::string_view appearanceJson);
	/* A character picked from the session roster: the same identity handoff, nothing new to save. */
	static bool_t Stage_ExistingEntry(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickname, std::string_view appearanceJson, std::string_view characterId);
	/* The look of the character that entered the world, or empty. Set when Bern is entered. */
	static std::string Get_ActiveAppearanceJson();
	/* Writes what the character that entered the world is carrying (inventory, purse, honor
	title, as the Client last received them) into its roster entry. Leaves the session state alone
	when nothing was received yet, so leaving early cannot wipe it. */
	static void Capture_ActiveWorldState();
	/* The session state of the character that entered the world, if it has one. */
	static bool_t Try_Get_ActiveWorldState(CHARACTER_WORLD_STATE& outState);
	static void Mark_RestoreRequested(std::uint32_t sequence);
	/* False is an unrelated/stale result; rejection keeps the previous session state protected. */
	static bool_t Apply_RestoreResult(const LostArk::Shared::S2C_RESTORE_CHARACTER_RESULT& result);
	static bool_t Has_PendingCreation();
	static bool_t Commit_PendingCreation();
	static void Cancel_PendingCreation();
	static bool_t Try_Resolve_ForWorld(
		LostArk::Shared::WORLD_ID worldId,
		CHARACTER_ENTRY_IDENTITY& outIdentity);
};

NS_END
