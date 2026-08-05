#include "CharacterSelectionState.h"

#include <mutex>
#include <optional>

namespace
{
	std::mutex g_SelectionMutex;
	std::optional<LostArk::Shared::CHARACTER_CLASS_ID> g_SelectedClass;
	Client::CHARACTER_TEST_ENTRY_MODE g_TestEntryMode =
		Client::CHARACTER_TEST_ENTRY_MODE::NONE;
}

bool_t Client::CCharacterSelectionState::Select(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
		characterClass))
	{
		return false;
	}

	std::scoped_lock lock{ g_SelectionMutex };
	g_SelectedClass = characterClass;
	g_TestEntryMode = CHARACTER_TEST_ENTRY_MODE::NONE;
	return true;
}

bool_t Client::CCharacterSelectionState::Has_Selection()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_SelectedClass.has_value();
}

bool_t Client::CCharacterSelectionState::Try_Get_SelectedClass(
	LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!g_SelectedClass.has_value())
		return false;

	outCharacterClass = *g_SelectedClass;
	return true;
}

bool_t Client::CCharacterSelectionState::Stage_TestEntryMode(
	const CHARACTER_TEST_ENTRY_MODE entryMode)
{
	if (CHARACTER_TEST_ENTRY_MODE::NONE == entryMode)
		return false;

	std::scoped_lock lock{ g_SelectionMutex };
	if (!g_SelectedClass.has_value())
		return false;

	g_TestEntryMode = entryMode;
	return true;
}

bool_t Client::CCharacterSelectionState::Try_Consume_TestEntryMode(
	CHARACTER_TEST_ENTRY_MODE& outEntryMode)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (CHARACTER_TEST_ENTRY_MODE::NONE == g_TestEntryMode)
		return false;
	outEntryMode = g_TestEntryMode;
	g_TestEntryMode = CHARACTER_TEST_ENTRY_MODE::NONE;
	return true;
}

void Client::CCharacterSelectionState::Clear_TestEntryMode()
{
	std::scoped_lock lock{ g_SelectionMutex };
	g_TestEntryMode = CHARACTER_TEST_ENTRY_MODE::NONE;
}
