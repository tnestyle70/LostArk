#include <WinSock2.h>
#include "CharacterSelectionState.h"

#include "CharacterRoster.h"
#include "Client_Defines.h"
#include "GameInstance.h"
#include "CombatHUDViewModel.h"
#include "Network/PacketMessages.h"

#include <Windows.h>

#include <algorithm>
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
		std::string strCharacterId;
		size_t iSlot = Client::CCharacterRoster::MAX_CHARACTERS;
		bool_t bNewCharacter = false;
	};

	std::mutex g_SelectionMutex;
	std::optional<LostArk::Shared::CHARACTER_CLASS_ID> g_SelectedClass;
	std::optional<std::string> g_CreatedNickname;
	std::optional<PENDING_CHARACTER_CREATION> g_PendingCreation;
	std::string g_ActiveAppearanceJson;
	std::string g_ActiveCharacterId;
	std::optional<size_t> g_CreationSlot;
	bool_t g_RestoreRequired = false;
	std::uint32_t g_RestoreRequestSequence = 0u;

	bool_t Stage_Pending(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		const std::string_view nickname, const std::string_view appearanceJson,
		const bool_t bNewCharacter, const std::string_view characterId = {})
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
		staged.strCharacterId.assign(characterId);
		staged.bNewCharacter = bNewCharacter;

		std::scoped_lock lock{ g_SelectionMutex };
		if (bNewCharacter)
		{
			if (!g_CreationSlot.has_value())
				for (size_t slot = 0; slot < Client::CCharacterRoster::MAX_CHARACTERS; ++slot)
					if (!Client::CCharacterRoster::Is_Occupied(slot)) { g_CreationSlot = slot; break; }
			if (!g_CreationSlot.has_value() || Client::CCharacterRoster::Is_Occupied(*g_CreationSlot)) return false;
			staged.iSlot = *g_CreationSlot;
		}
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

bool_t Client::CCharacterSelectionState::Select_CreationSlot(const size_t slot)
{
	if (slot >= CCharacterRoster::MAX_CHARACTERS || CCharacterRoster::Is_Occupied(slot)) return false;
	std::scoped_lock lock{ g_SelectionMutex };
	g_CreationSlot = slot;
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
	const std::string_view nickname, const std::string_view appearanceJson, const std::string_view characterId)
{
	const auto& entries = CCharacterRoster::Get_Entries();
	const auto entry = std::find_if(entries.begin(), entries.end(), [&](const auto& candidate)
		{ return candidate.strCharacterId == characterId && candidate.eCharacterClass == characterClass &&
			candidate.strNickname == nickname; });
	if (entry == entries.end()) return false;
	return Stage_Pending(characterClass, nickname, appearanceJson, false, characterId);
}

std::string Client::CCharacterSelectionState::Get_ActiveAppearanceJson()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_ActiveAppearanceJson;
}

void Client::CCharacterSelectionState::Capture_ActiveWorldState()
{
	std::string characterId;
	{
		std::scoped_lock lock{ g_SelectionMutex };
		if (g_ActiveCharacterId.empty() || g_RestoreRequired) return;
		characterId = g_ActiveCharacterId;
	}
	const auto level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
	if (level != LEVEL::BERN && level != LEVEL::VALTAN_ARENA &&
		level != LEVEL::KAKULSAYDON_ARENA && level != LEVEL::MAHARAKA &&
		level != LEVEL::COLOSSEUM) return;
	const CCombatHUDViewModel& ViewModel = CCombatHUDViewModel::Get();
	const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& Inventory = ViewModel.Get_Inventory();
	const auto& Player = ViewModel.Get_Player();
	/* No inventory answer yet (left before it arrived, or a direct entry) is not a state to save. */
	if (!Player.isValid || Player.isPreview || !ViewModel.Has_Inventory())
		return;
	CHARACTER_WORLD_STATE State{};
	State.bValid = true;
	State.Items = Inventory.Items;
	State.iSilver = Inventory.iSilver;
	State.iGold = Inventory.iGold;
	State.iHonorTitleId = Player.iHonorTitleId;
	std::string strStatus;
	if (!CCharacterRoster::Update_WorldState(characterId, State, strStatus))
		OutputDebugStringA(("[CharacterSelection] World state capture failed: " + strStatus + "\n").c_str());
}

bool_t Client::CCharacterSelectionState::Try_Get_ActiveWorldState(CHARACTER_WORLD_STATE& outState)
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_RestoreRequired && g_RestoreRequestSequence == 0u && !g_ActiveCharacterId.empty() &&
		CCharacterRoster::Try_Get_WorldState(g_ActiveCharacterId, outState);
}

void Client::CCharacterSelectionState::Mark_RestoreRequested(const std::uint32_t sequence)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (g_RestoreRequired) g_RestoreRequestSequence = sequence;
}

bool_t Client::CCharacterSelectionState::Apply_RestoreResult(
	const LostArk::Shared::S2C_RESTORE_CHARACTER_RESULT& result)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!g_RestoreRequired || !g_RestoreRequestSequence || result.iRequestSequence != g_RestoreRequestSequence)
		return false;
	/* Keep the rejected request latched: returning from another world cannot retry or save defaults. */
	if (result.eResult == LostArk::Shared::CHARACTER_RESTORE_RESULT::APPLIED)
		g_RestoreRequired = false;
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
	g_ActiveCharacterId = g_PendingCreation->strCharacterId;
	/* A new character joins this process roster only once Bern is really entered. */
	if (g_PendingCreation->bNewCharacter)
	{
		size_t iNewIndex = 0;
		std::string strRosterStatus;
		if (!CCharacterRoster::Add(g_PendingCreation->iSlot, g_PendingCreation->eCharacterClass,
			g_PendingCreation->strNickname, g_PendingCreation->strAppearanceJson,
			iNewIndex, strRosterStatus))
			OutputDebugStringA(("[CharacterSelection] Roster creation failed: " + strRosterStatus + "\n").c_str());
		else
			g_ActiveCharacterId = CCharacterRoster::Get_Entries()[iNewIndex].strCharacterId;
	}
	CHARACTER_WORLD_STATE saved;
	g_RestoreRequired = !g_ActiveCharacterId.empty() && CCharacterRoster::Try_Get_WorldState(g_ActiveCharacterId, saved);
	g_RestoreRequestSequence = 0u;
	g_CreatedNickname = std::move(g_PendingCreation->strNickname);
	g_ActiveAppearanceJson = std::move(g_PendingCreation->strAppearanceJson);
	g_CreationSlot.reset();
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
	case WORLD_ID::COLOSSEUM:
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
