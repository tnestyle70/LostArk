#include <WinSock2.h>
#include "CharacterRoster.h"

#include "DataJson.h"
#include "Network/PacketMessages.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace
{
	constexpr int32_t ROSTER_FORMAT_VERSION = 2;

	/* Card order: the roster stands Warlord, Lance Master, Artist and Guardian Knight from left
	to right whatever order they were created in; any other class follows by class id. */
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

	bool_t Is_Before_In_Card_Order(const Client::CHARACTER_ROSTER_ENTRY& lhs,
		const Client::CHARACTER_ROSTER_ENTRY& rhs)
	{
		return Class_Rank(lhs.eCharacterClass) < Class_Rank(rhs.eCharacterClass);
	}

	std::filesystem::path Get_RosterPath()
	{
		const DWORD count = GetEnvironmentVariableW(L"LOCALAPPDATA", nullptr, 0);
		if (0u == count)
			return {};
		std::wstring base(count, L'\0');
		const DWORD written = GetEnvironmentVariableW(L"LOCALAPPDATA", base.data(), count);
		if (0u == written || written >= count)
			return {};
		base.resize(written);
		return std::filesystem::path(base) / L"LostArk" / L"CharacterRoster.json";
	}

	/* JSON string escaping: quote, backslash and the control characters an appearance document
	(several lines of text) carries. A nickname never has control characters -- they are refused
	by Is_Valid_PlayerNickname. */
	std::string Escape_Json(const std::string& text)
	{
		std::string escaped;
		for (const char character : text)
		{
			if ('"' == character || '\\' == character)
			{
				escaped.push_back('\\');
				escaped.push_back(character);
			}
			else if ('\n' == character)
				escaped += "\\n";
			else if ('\r' == character)
				escaped += "\\r";
			else if ('\t' == character)
				escaped += "\\t";
			else
				escaped.push_back(character);
		}
		return escaped;
	}

	uint32_t Read_Unsigned(const DATA_JSON_VALUE* pValue)
	{
		if (nullptr == pValue || !pValue->Is_Number() || pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() > 4294967295.0)
			return 0u;
		return static_cast<uint32_t>(pValue->Get_Number());
	}

	/* The optional "world" object of a roster entry. An unreadable one is no saved state; a bad
	item inside a readable one is skipped. */
	void Load_WorldState(const DATA_JSON_VALUE* pWorld, Client::CHARACTER_WORLD_STATE& outState)
	{
		outState = {};
		if (nullptr == pWorld || !pWorld->Is_Object())
			return;
		const DATA_JSON_VALUE* pItems = pWorld->Find("items");
		if (nullptr == pItems || DATA_JSON_TYPE::ARRAY != pItems->Get_Type())
			return;
		for (const DATA_JSON_VALUE& item : pItems->Get_Array())
		{
			const DATA_JSON_VALUE* pId = item.Is_Object() ? item.Find("id") : nullptr;
			if (nullptr == pId || DATA_JSON_TYPE::STRING != pId->Get_Type() || pId->Get_String().empty())
				continue;
			LostArk::Shared::INVENTORY_ITEM_SNAPSHOT entry{};
			entry.strItemId = pId->Get_String();
			entry.iQuantity = Read_Unsigned(item.Find("qty"));
			entry.eEquippedSlot = static_cast<LostArk::Shared::EQUIPMENT_SLOT>(
				static_cast<uint8_t>(Read_Unsigned(item.Find("slot"))));
			if (0u == entry.iQuantity)
				continue;
			outState.Items.push_back(std::move(entry));
		}
		outState.iSilver = Read_Unsigned(pWorld->Find("silver"));
		outState.iGold = Read_Unsigned(pWorld->Find("gold"));
		outState.iHonorTitleId = Read_Unsigned(pWorld->Find("honorTitle"));
		outState.bValid = true;
	}

	/* A missing, unreadable or older-format (formatVersion 1 kept only nicknames of four fixed
	cards) file is an empty roster. A bad entry inside a current file is skipped. */
	std::vector<Client::CHARACTER_ROSTER_ENTRY> Load_Roster()
	{
		std::vector<Client::CHARACTER_ROSTER_ENTRY> entries;
		const std::filesystem::path path = Get_RosterPath();
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
			return entries;
		const std::string text{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
		DATA_JSON_VALUE root;
		std::string error;
		if (!CDataJson::Parse(text, root, error) || !root.Is_Object())
			return entries;
		const DATA_JSON_VALUE* version = root.Find("formatVersion");
		if (nullptr == version || !version->Is_Number() ||
			static_cast<int32_t>(version->Get_Number()) != ROSTER_FORMAT_VERSION)
			return entries;
		const DATA_JSON_VALUE* characters = root.Find("characters");
		if (nullptr == characters || DATA_JSON_TYPE::ARRAY != characters->Get_Type())
			return entries;
		for (const DATA_JSON_VALUE& value : characters->Get_Array())
		{
			if (entries.size() >= Client::CCharacterRoster::MAX_CHARACTERS)
				break;
			if (!value.Is_Object())
				continue;
			const DATA_JSON_VALUE* classValue = value.Find("class");
			const DATA_JSON_VALUE* nicknameValue = value.Find("nickname");
			if (nullptr == classValue || !classValue->Is_Number() ||
				nullptr == nicknameValue || DATA_JSON_TYPE::STRING != nicknameValue->Get_Type())
				continue;
			const LostArk::Shared::CHARACTER_CLASS_ID eClass =
				static_cast<LostArk::Shared::CHARACTER_CLASS_ID>(
					static_cast<uint8_t>(classValue->Get_Number()));
			if (!LostArk::Shared::Is_Supported_Playable_Character_Class(eClass) ||
				!LostArk::Shared::Is_Valid_PlayerNickname(nicknameValue->Get_String()))
				continue;
			Client::CHARACTER_ROSTER_ENTRY entry{ eClass, nicknameValue->Get_String(), {} };
			const DATA_JSON_VALUE* appearanceValue = value.Find("appearance");
			if (nullptr != appearanceValue && DATA_JSON_TYPE::STRING == appearanceValue->Get_Type())
				entry.strAppearanceJson = appearanceValue->Get_String();
			Load_WorldState(value.Find("world"), entry.World);
			entries.push_back(std::move(entry));
		}
		std::stable_sort(entries.begin(), entries.end(), Is_Before_In_Card_Order);
		return entries;
	}

	std::vector<Client::CHARACTER_ROSTER_ENTRY>& Roster()
	{
		static std::vector<Client::CHARACTER_ROSTER_ENTRY> entries = Load_Roster();
		return entries;
	}

	/* Written next to the target, flushed, then moved over it in one step, so a failed save
	leaves the previous roster file untouched. */
	bool_t Save_Roster(const std::vector<Client::CHARACTER_ROSTER_ENTRY>& entries, std::string& outStatus)
	{
		std::string document = "{\n  \"schema\": \"lostark.character-roster\",\n  \"formatVersion\": " +
			std::to_string(ROSTER_FORMAT_VERSION) + ",\n  \"characters\": [";
		for (size_t index = 0; index < entries.size(); ++index)
		{
			document += (0u == index ? "\n" : ",\n");
			document += "    { \"class\": " +
				std::to_string(static_cast<uint32_t>(entries[index].eCharacterClass)) +
				", \"nickname\": \"" + Escape_Json(entries[index].strNickname) + "\"";
			if (!entries[index].strAppearanceJson.empty())
				document += ", \"appearance\": \"" + Escape_Json(entries[index].strAppearanceJson) + "\"";
			if (entries[index].World.bValid)
			{
				const Client::CHARACTER_WORLD_STATE& World = entries[index].World;
				document += ", \"world\": { \"silver\": " + std::to_string(World.iSilver) +
					", \"gold\": " + std::to_string(World.iGold) +
					", \"honorTitle\": " + std::to_string(World.iHonorTitleId) + ", \"items\": [";
				for (size_t item = 0; item < World.Items.size(); ++item)
				{
					document += (0u == item ? " " : ", ");
					document += "{ \"id\": \"" + Escape_Json(World.Items[item].strItemId) +
						"\", \"qty\": " + std::to_string(World.Items[item].iQuantity) +
						", \"slot\": " + std::to_string(static_cast<uint32_t>(World.Items[item].eEquippedSlot)) + " }";
				}
				document += " ] }";
			}
			document += " }";
		}
		document += entries.empty() ? " ]\n}\n" : "\n  ]\n}\n";

		const std::filesystem::path path = Get_RosterPath();
		std::error_code ec;
		if (path.empty() || (std::filesystem::create_directories(path.parent_path(), ec), ec))
		{
			outStatus = "Cannot create the roster folder.";
			return false;
		}
		const std::filesystem::path temporary = path.wstring() + L".tmp";
		{
			std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
			output.write(document.data(), static_cast<std::streamsize>(document.size()));
			output.flush();
			if (!output)
			{
				outStatus = "Cannot write the roster file.";
				return false;
			}
		}
		if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			DeleteFileW(temporary.c_str());
			outStatus = "Cannot replace the roster file.";
			return false;
		}
		return true;
	}
}

const std::vector<Client::CHARACTER_ROSTER_ENTRY>& Client::CCharacterRoster::Get_Entries()
{
	return Roster();
}

bool_t Client::CCharacterRoster::Rename(
	const size_t iIndex, const std::string& strNickname, std::string& outStatus)
{
	std::vector<CHARACTER_ROSTER_ENTRY>& entries = Roster();
	if (iIndex >= entries.size())
	{
		outStatus = "No character in that slot.";
		return false;
	}
	if (!LostArk::Shared::Is_Valid_PlayerNickname(strNickname))
	{
		outStatus = "Use 1-32 UTF-8 bytes with no control or edge whitespace.";
		return false;
	}

	std::vector<CHARACTER_ROSTER_ENTRY> staged = entries;
	staged[iIndex].strNickname = strNickname;
	if (!Save_Roster(staged, outStatus))
		return false;
	entries = std::move(staged);
	outStatus = "Name changed.";
	return true;
}

bool_t Client::CCharacterRoster::Update_WorldState(
	const LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass, const std::string& strNickname,
	const CHARACTER_WORLD_STATE& State, std::string& outStatus)
{
	std::vector<CHARACTER_ROSTER_ENTRY>& entries = Roster();
	const auto found = std::find_if(entries.begin(), entries.end(),
		[&](const CHARACTER_ROSTER_ENTRY& entry)
		{
			return entry.eCharacterClass == eCharacterClass && entry.strNickname == strNickname;
		});
	if (found == entries.end())
	{
		outStatus = "No saved character matches.";
		return false;
	}
	std::vector<CHARACTER_ROSTER_ENTRY> staged = entries;
	staged[static_cast<size_t>(found - entries.begin())].World = State;
	staged[static_cast<size_t>(found - entries.begin())].World.bValid = true;
	if (!Save_Roster(staged, outStatus))
		return false;
	entries = std::move(staged);
	outStatus = "World state saved.";
	return true;
}

bool_t Client::CCharacterRoster::Try_Get_WorldState(
	const LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass, const std::string& strNickname,
	CHARACTER_WORLD_STATE& outState)
{
	for (const CHARACTER_ROSTER_ENTRY& entry : Roster())
	{
		if (entry.eCharacterClass == eCharacterClass && entry.strNickname == strNickname &&
			entry.World.bValid)
		{
			outState = entry.World;
			return true;
		}
	}
	return false;
}

bool_t Client::CCharacterRoster::Add(
	const LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass, const std::string& strNickname,
	const std::string& strAppearanceJson, size_t& outIndex, std::string& outStatus)
{
	std::vector<CHARACTER_ROSTER_ENTRY>& entries = Roster();
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(eCharacterClass))
	{
		outStatus = "That class cannot be saved.";
		return false;
	}
	if (!LostArk::Shared::Is_Valid_PlayerNickname(strNickname))
	{
		outStatus = "Use 1-32 UTF-8 bytes with no control or edge whitespace.";
		return false;
	}
	if (entries.size() >= MAX_CHARACTERS)
	{
		outStatus = "The roster is full.";
		return false;
	}

	/* The new card goes where its class belongs in the card order, after any card of the same
	class already there. */
	const CHARACTER_ROSTER_ENTRY created{ eCharacterClass, strNickname, strAppearanceJson };
	std::vector<CHARACTER_ROSTER_ENTRY> staged = entries;
	const auto position = std::upper_bound(staged.begin(), staged.end(), created, Is_Before_In_Card_Order);
	const size_t iInsertedIndex = static_cast<size_t>(position - staged.begin());
	staged.insert(position, created);
	if (!Save_Roster(staged, outStatus))
		return false;
	entries = std::move(staged);
	outIndex = iInsertedIndex;
	outStatus = "Character saved.";
	return true;
}
