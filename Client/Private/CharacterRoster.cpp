#include <WinSock2.h>
#include "CharacterRoster.h"

#include "Network/PacketMessages.h"

#include <algorithm>

namespace
{
	std::vector<Client::CHARACTER_ROSTER_ENTRY>& Roster()
	{
		/* Each EXE starts empty. No personal file is read, written or removed. */
		static std::vector<Client::CHARACTER_ROSTER_ENTRY> entries(Client::CCharacterRoster::MAX_CHARACTERS);
		return entries;
	}

	int32_t Class_Rank(const LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (eClass)
		{
		case CHARACTER_CLASS_ID::WARLORD: return 0;
		case CHARACTER_CLASS_ID::LANCE_MASTER: return 1;
		case CHARACTER_CLASS_ID::ARTIST: return 2;
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT: return 3;
		default: return 4 + static_cast<int32_t>(eClass);
		}
	}
}

const std::vector<Client::CHARACTER_ROSTER_ENTRY>& Client::CCharacterRoster::Get_Entries()
{
	return Roster();
}

size_t Client::CCharacterRoster::Get_CharacterCount()
{
	return static_cast<size_t>(std::count_if(Roster().begin(), Roster().end(),
		[](const auto& entry) { return !entry.strCharacterId.empty(); }));
}

bool_t Client::CCharacterRoster::Is_Occupied(const size_t slot)
{
	return slot < Roster().size() && !Roster()[slot].strCharacterId.empty();
}

bool_t Client::CCharacterRoster::Rename(
	const size_t iIndex, const std::string& strNickname, std::string& outStatus)
{
	auto& entries = Roster();
	if (!Is_Occupied(iIndex))
	{
		outStatus = "No character in that slot.";
		return false;
	}
	if (!LostArk::Shared::Is_Valid_PlayerNickname(strNickname))
	{
		outStatus = "Use 1-32 UTF-8 bytes with no control or edge whitespace.";
		return false;
	}
	entries[iIndex].strNickname = strNickname;
	outStatus = "Name changed for this session.";
	return true;
}

bool_t Client::CCharacterRoster::Update_WorldState(
	const std::string& strCharacterId, const CHARACTER_WORLD_STATE& State, std::string& outStatus)
{
	auto& entries = Roster();
	const auto found = std::find_if(entries.begin(), entries.end(),
		[&](const auto& entry) { return entry.strCharacterId == strCharacterId; });
	if (found == entries.end())
	{
		outStatus = "No session character matches.";
		return false;
	}
	found->World = State;
	found->World.bValid = true;
	outStatus = "World state kept for this session.";
	return true;
}

bool_t Client::CCharacterRoster::Try_Get_WorldState(
	const std::string& strCharacterId, CHARACTER_WORLD_STATE& outState)
{
	for (const auto& entry : Roster())
	{
		if (entry.strCharacterId == strCharacterId && entry.World.bValid)
		{
			outState = entry.World;
			return true;
		}
	}
	return false;
}

bool_t Client::CCharacterRoster::Add(
	const size_t slot, const LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass, const std::string& strNickname,
	const std::string& strAppearanceJson, size_t& outIndex, std::string& outStatus)
{
	auto& entries = Roster();
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(eCharacterClass))
	{
		outStatus = "That class cannot be created.";
		return false;
	}
	if (!LostArk::Shared::Is_Valid_PlayerNickname(strNickname))
	{
		outStatus = "Use 1-32 UTF-8 bytes with no control or edge whitespace.";
		return false;
	}
	if (slot >= MAX_CHARACTERS || Is_Occupied(slot))
	{
		outStatus = "The selected character slot is unavailable.";
		return false;
	}
	/* Monotonic session identity is independent of nickname, sorting and card index. */
	static uint64_t nextIdentity = 0u;
	CHARACTER_ROSTER_ENTRY created{ eCharacterClass, strNickname, strAppearanceJson };
	created.strCharacterId = "session.character." + std::to_string(++nextIdentity);
	const std::string strCreatedId = created.strCharacterId;
	entries[slot] = std::move(created);
	/* Card order: Warlord, Lance Master, Artist and Guardian Knight from the left, any other
	   class after by class id, empty cards last. Warlord must stand leftmost or the next card
	   covers its weapons. Characters keep a stable id, so moving a card changes no state. */
	std::stable_sort(entries.begin(), entries.end(),
		[](const CHARACTER_ROSTER_ENTRY& lhs, const CHARACTER_ROSTER_ENTRY& rhs)
		{
			const bool_t bLhsEmpty = lhs.strCharacterId.empty();
			const bool_t bRhsEmpty = rhs.strCharacterId.empty();
			if (bLhsEmpty || bRhsEmpty)
				return !bLhsEmpty && bRhsEmpty;
			return Class_Rank(lhs.eCharacterClass) < Class_Rank(rhs.eCharacterClass);
		});
	outIndex = static_cast<size_t>(std::find_if(entries.begin(), entries.end(),
		[&strCreatedId](const CHARACTER_ROSTER_ENTRY& entry) { return entry.strCharacterId == strCreatedId; }) -
		entries.begin());
	outStatus = "Character created for this session.";
	return true;
}
