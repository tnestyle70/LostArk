#include "CharacterSelectionState.h"

#include "Network/PacketMessages.h"

#include <Windows.h>

#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace
{
	struct PENDING_CHARACTER_CREATION final
	{
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strNickname;
	};

	std::mutex g_SelectionMutex;
	std::optional<LostArk::Shared::CHARACTER_CLASS_ID> g_SelectedClass;
	std::optional<std::string> g_CreatedNickname;
	std::optional<PENDING_CHARACTER_CREATION> g_PendingCreation;

	const std::string& Get_AuditionNickname()
	{
		static const std::string nickname =
			"Test-" + std::to_string(GetCurrentProcessId());
		return nickname;
	}
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

bool_t Client::CCharacterSelectionState::Stage_Creation(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::string_view nickname)
{
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
			characterClass) ||
		!LostArk::Shared::Is_Valid_PlayerNickname(nickname))
	{
		return false;
	}

	PENDING_CHARACTER_CREATION staged{};
	staged.eCharacterClass = characterClass;
	staged.strNickname.assign(nickname);

	std::scoped_lock lock{ g_SelectionMutex };
	g_PendingCreation = std::move(staged);
	return true;
}

bool_t Client::CCharacterSelectionState::Has_PendingCreation()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_PendingCreation.has_value();
}

bool_t Client::CCharacterSelectionState::Commit_PendingCreation()
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!g_PendingCreation.has_value())
		return false;

	g_SelectedClass = g_PendingCreation->eCharacterClass;
	g_CreatedNickname = std::move(g_PendingCreation->strNickname);
	g_PendingCreation.reset();
	return true;
}

void Client::CCharacterSelectionState::Cancel_PendingCreation()
{
	std::scoped_lock lock{ g_SelectionMutex };
	g_PendingCreation.reset();
}

bool_t Client::CCharacterSelectionState::Try_Resolve_ForWorld(
	const LostArk::Shared::WORLD_ID worldId,
	CHARACTER_ENTRY_IDENTITY& outIdentity)
{
	using namespace LostArk::Shared;

	std::scoped_lock lock{ g_SelectionMutex };
	CHARACTER_ENTRY_IDENTITY staged{};

	switch (worldId)
	{
	case WORLD_ID::CHARACTER_SELECT_ARENA:
	case WORLD_ID::TRAINING_GROUND:
		staged.eCharacterClass = g_SelectedClass.value_or(
			CHARACTER_CLASS_ID::LANCE_MASTER);
		staged.strNickname = Get_AuditionNickname();
		staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::AUDITION;
		break;

	case WORLD_ID::BERN:
		if (g_PendingCreation.has_value())
		{
			staged.eCharacterClass = g_PendingCreation->eCharacterClass;
			staged.strNickname = g_PendingCreation->strNickname;
			staged.eSource =
				CHARACTER_ENTRY_IDENTITY_SOURCE::PENDING_CREATION;
		}
		else if (g_SelectedClass.has_value() &&
			g_CreatedNickname.has_value())
		{
			staged.eCharacterClass = *g_SelectedClass;
			staged.strNickname = *g_CreatedNickname;
			staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::CREATED;
		}
		else
		{
			return false;
		}
		break;

	case WORLD_ID::VALTAN_ARENA:
	case WORLD_ID::KAKULSAYDON_ARENA:
		staged.eCharacterClass = g_SelectedClass.value_or(
			CHARACTER_CLASS_ID::LANCE_MASTER);
		if (g_CreatedNickname.has_value())
		{
			staged.strNickname = *g_CreatedNickname;
			staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::CREATED;
		}
		else
		{
			staged.strNickname = Get_AuditionNickname();
			staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::AUDITION;
		}
		break;

	default:
		return false;
	}

	if (!Is_Supported_Playable_Character_Class(
			staged.eCharacterClass) ||
		!Is_Valid_PlayerNickname(staged.strNickname))
	{
		return false;
	}

	outIdentity = std::move(staged);
	return true;
}
