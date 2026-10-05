#include <WinSock2.h>
#include "CharacterSelectionState.h"

#include "CharacterRoster.h"
#include "Client_Defines.h"
#include "GameInstance.h"
#include "CombatHUDViewModel.h"
#include "Network/PacketMessages.h"
#include "CustomizingView.h"

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
	std::uint32_t g_NextStateSequence = 0u;
	Client::CHARACTER_CAPTURE_STATUS g_CaptureStatus = Client::CHARACTER_CAPTURE_STATUS::NONE;
	std::uint32_t g_CaptureSequence = 0u;
	LostArk::Shared::WORLD_ID g_CaptureWorld = LostArk::Shared::WORLD_ID::BERN;
	LostArk::Shared::PLAYER_ID g_CapturePlayer = 0u;
	LostArk::Shared::NET_ENTITY_ID g_CaptureEntity = 0u;
	std::uint64_t g_CaptureGeneration = 0u;
	std::string g_CaptureCharacterId;
	bool_t g_HasFinalCapture = false;

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
	if (characterId.empty() || entry == entries.end() || entry->strAppearanceJson != appearanceJson) return false;
	return Stage_Pending(characterClass, entry->strNickname, entry->strAppearanceJson, false, entry->strCharacterId);
}

std::string Client::CCharacterSelectionState::Get_ActiveAppearanceJson()
{
	std::scoped_lock lock{ g_SelectionMutex };
	/* The entering character spawns before Bern's identity commit, so its look is the pending
	   one; without this the first entry showed the previous character's look, or none. */
	if (g_PendingCreation.has_value())
		return g_PendingCreation->strAppearanceJson;
	return g_ActiveAppearanceJson;
}

void Client::CCharacterSelectionState::Capture_ActiveWorldState()
{
	std::string characterId;
	{
		std::scoped_lock lock{ g_SelectionMutex };
		if (g_ActiveCharacterId.empty() || g_RestoreRequired || g_HasFinalCapture ||
			g_CaptureStatus != CHARACTER_CAPTURE_STATUS::NONE) return;
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

bool_t Client::CCharacterSelectionState::Has_ActiveCharacter()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return !g_ActiveCharacterId.empty();
}

std::string Client::CCharacterSelectionState::Get_ActiveCharacterId()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_ActiveCharacterId;
}

bool_t Client::CCharacterSelectionState::Is_RestorePending()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_RestoreRequired;
}

bool_t Client::CCharacterSelectionState::Is_WorldStateSyncPending()
{
	std::scoped_lock lock{ g_SelectionMutex };
	// The protected saved state survives recovery, but a creation arena is a new audition.
	const bool_t restoringBern = g_RestoreRequired &&
		CGameInstance::Get().Get_CurrentLevelID() == static_cast<uint32_t>(LEVEL::BERN);
	return restoringBern || g_CaptureStatus != CHARACTER_CAPTURE_STATUS::NONE;
}

std::uint32_t Client::CCharacterSelectionState::Next_StateRequestSequence()
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (++g_NextStateSequence == 0u) ++g_NextStateSequence;
	return g_NextStateSequence;
}

bool_t Client::CCharacterSelectionState::Begin_WorldStateCapture(const std::uint32_t sequence,
	const LostArk::Shared::WORLD_ID world, const LostArk::Shared::PLAYER_ID player,
	const LostArk::Shared::NET_ENTITY_ID entity, const std::uint64_t generation)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!sequence || !player || !entity || !generation || g_ActiveCharacterId.empty() ||
		g_RestoreRequired || g_CaptureStatus != CHARACTER_CAPTURE_STATUS::NONE) return false;
	g_CaptureSequence = sequence;
	g_CaptureWorld = world;
	g_CapturePlayer = player;
	g_CaptureEntity = entity;
	g_CaptureGeneration = generation;
	g_CaptureCharacterId = g_ActiveCharacterId;
	g_CaptureStatus = CHARACTER_CAPTURE_STATUS::WAITING;
	return true;
}

bool_t Client::CCharacterSelectionState::Apply_CaptureResult(
	const LostArk::Shared::S2C_CAPTURE_CHARACTER_RESULT& result, const std::uint64_t generation)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (g_CaptureStatus != CHARACTER_CAPTURE_STATUS::WAITING ||
		result.iRequestSequence != g_CaptureSequence || generation != g_CaptureGeneration)
		return false;
	g_CaptureStatus = CHARACTER_CAPTURE_STATUS::FAILED;
	const auto& entries = CCharacterRoster::Get_Entries();
	const auto entry = std::find_if(entries.begin(), entries.end(), [&](const auto& candidate)
		{ return candidate.strCharacterId == g_CaptureCharacterId; });
	if (result.eResult != LostArk::Shared::CHARACTER_CAPTURE_RESULT::CAPTURED ||
		result.eWorldId != g_CaptureWorld || result.iPlayerId != g_CapturePlayer ||
		result.iNetEntityId != g_CaptureEntity || g_ActiveCharacterId != g_CaptureCharacterId ||
		entry == entries.end() || result.eCharacterClass != entry->eCharacterClass)
		return true;
	CHARACTER_WORLD_STATE state{};
	state.bValid = true;
	state.Items = result.Items;
	state.iSilver = result.iSilver;
	state.iGold = result.iGold;
	state.iHonorTitleId = result.iHonorTitleId;
	std::string status;
	if (CCharacterRoster::Update_WorldState(g_CaptureCharacterId, state, status))
	{
		g_CaptureStatus = CHARACTER_CAPTURE_STATUS::CAPTURED;
		g_HasFinalCapture = true;
	}
	return true;
}

Client::CHARACTER_CAPTURE_STATUS Client::CCharacterSelectionState::Get_WorldStateCaptureStatus()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_CaptureStatus;
}

void Client::CCharacterSelectionState::Finish_WorldStateCapture()
{
	std::scoped_lock lock{ g_SelectionMutex };
	// A rejected or timed-out barrier must not be replaced by the older HUD snapshot.
	if (g_CaptureStatus != CHARACTER_CAPTURE_STATUS::NONE) g_HasFinalCapture = true;
	g_CaptureStatus = CHARACTER_CAPTURE_STATUS::NONE;
	g_CaptureSequence = 0u;
	g_CaptureCharacterId.clear();
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
	{
		g_RestoreRequired = false;
		g_HasFinalCapture = false;
	}
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

	std::string activeCharacterId = g_PendingCreation->strCharacterId;
	/* A new character joins this process roster only once Bern is really entered. */
	if (g_PendingCreation->bNewCharacter)
	{
		size_t iNewIndex = 0;
		std::string strRosterStatus;
		if (!CCharacterRoster::Add(g_PendingCreation->iSlot, g_PendingCreation->eCharacterClass,
			g_PendingCreation->strNickname, g_PendingCreation->strAppearanceJson,
			iNewIndex, strRosterStatus))
		{
			OutputDebugStringA(("[CharacterSelection] Roster creation failed: " + strRosterStatus + "\n").c_str());
			return false;
		}
		activeCharacterId = CCharacterRoster::Get_Entries()[iNewIndex].strCharacterId;
	}
	g_SelectedClass = g_PendingCreation->eCharacterClass;
	g_ActiveCharacterId = std::move(activeCharacterId);
	g_HasFinalCapture = false;
	g_CaptureStatus = CHARACTER_CAPTURE_STATUS::NONE;
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
			staged.iVoiceType = CCustomizingView::Read_SavedVoiceType(
				g_PendingCreation->strAppearanceJson);
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
			staged.iVoiceType = CCustomizingView::Read_SavedVoiceType(g_ActiveAppearanceJson);
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
