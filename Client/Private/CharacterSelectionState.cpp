#include "CharacterSelectionState.h"

#include "CharacterRoster.h"
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
		std::string strAppearanceJson;
		bool_t bNewCharacter = false;
	};

	std::mutex g_SelectionMutex;
	std::optional<LostArk::Shared::CHARACTER_CLASS_ID> g_SelectedClass;
	std::optional<std::string> g_CreatedNickname;
	std::optional<PENDING_CHARACTER_CREATION> g_PendingCreation;
	std::string g_ActiveAppearanceJson;

	bool_t Stage_Pending(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		const std::string_view nickname, const std::string_view appearanceJson,
		const bool_t bNewCharacter)
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
		staged.strAppearanceJson.assign(appearanceJson);
		staged.bNewCharacter = bNewCharacter;

		std::scoped_lock lock{ g_SelectionMutex };
		g_PendingCreation = std::move(staged);
		return true;
	}

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
	const std::string_view nickname, const std::string_view appearanceJson)
{
	return Stage_Pending(characterClass, nickname, appearanceJson, true);
}

bool_t Client::CCharacterSelectionState::Stage_ExistingEntry(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::string_view nickname, const std::string_view appearanceJson)
{
	return Stage_Pending(characterClass, nickname, appearanceJson, false);
}

std::string Client::CCharacterSelectionState::Get_ActiveAppearanceJson()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_ActiveAppearanceJson;
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
	/* A new character joins the saved roster only once Bern is really entered. Failing to save
	it never fails the entry: the character plays, it just is not on a card next time. */
	if (g_PendingCreation->bNewCharacter)
	{
		size_t iNewIndex = 0;
		std::string strRosterStatus;
		if (!CCharacterRoster::Add(g_PendingCreation->eCharacterClass,
			g_PendingCreation->strNickname, g_PendingCreation->strAppearanceJson,
			iNewIndex, strRosterStatus))
			OutputDebugStringA(("[CharacterSelection] Roster add failed: " + strRosterStatus + "\n").c_str());
	}
	g_CreatedNickname = std::move(g_PendingCreation->strNickname);
	g_ActiveAppearanceJson = std::move(g_PendingCreation->strAppearanceJson);
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
			break;
		}
		// Direct entry shares the existing created-or-audition identity policy.
		// Only an explicit pending creation is committed after Bern activation.
		[[fallthrough]];

	case WORLD_ID::VALTAN_ARENA:
	case WORLD_ID::KAKULSAYDON_ARENA:
	case WORLD_ID::MAHARAKA:
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
