#include <WinSock2.h>
#include "CharacterRoster.h"

#include "DataJson.h"
#include "Network/PacketMessages.h"

#include <filesystem>
#include <fstream>
#include <iterator>

namespace
{
	using LostArk::Shared::CHARACTER_CLASS_ID;

	/* Card order and default nicknames, as the team set them. */
	const Client::CHARACTER_ROSTER_ENTRY DEFAULT_ENTRIES[] = {
		{ CHARACTER_CLASS_ID::WARLORD, "TJ" },
		{ CHARACTER_CLASS_ID::LANCE_MASTER, "JS" },
		{ CHARACTER_CLASS_ID::ARTIST, "CY" },
		{ CHARACTER_CLASS_ID::GUARDIANKNIGHT, "GB" },
	};

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

	/* JSON string escaping for the two characters a nickname could need (quote, backslash);
	control characters are already refused by Is_Valid_PlayerNickname. */
	std::string Escape_Json(const std::string& text)
	{
		std::string escaped;
		for (const char character : text)
		{
			if ('"' == character || '\\' == character)
				escaped.push_back('\\');
			escaped.push_back(character);
		}
		return escaped;
	}

	std::vector<Client::CHARACTER_ROSTER_ENTRY> Load_Roster()
	{
		std::vector<Client::CHARACTER_ROSTER_ENTRY> entries(
			std::begin(DEFAULT_ENTRIES), std::end(DEFAULT_ENTRIES));
		const std::filesystem::path path = Get_RosterPath();
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
			return entries;
		const std::string text{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
		DATA_JSON_VALUE root;
		std::string error;
		if (!CDataJson::Parse(text, root, error) || !root.Is_Object())
			return entries;
		const DATA_JSON_VALUE* nicknames = root.Find("nicknames");
		if (nullptr == nicknames || DATA_JSON_TYPE::ARRAY != nicknames->Get_Type())
			return entries;
		/* The saved file carries one nickname per card in card order; a bad or missing one keeps
		that card's default. */
		size_t index = 0;
		for (const DATA_JSON_VALUE& value : nicknames->Get_Array())
		{
			if (index >= entries.size())
				break;
			if (DATA_JSON_TYPE::STRING == value.Get_Type() &&
				LostArk::Shared::Is_Valid_PlayerNickname(value.Get_String()))
				entries[index].strNickname = value.Get_String();
			++index;
		}
		return entries;
	}

	std::vector<Client::CHARACTER_ROSTER_ENTRY>& Roster()
	{
		static std::vector<Client::CHARACTER_ROSTER_ENTRY> entries = Load_Roster();
		return entries;
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
	std::string document = "{\n  \"schema\": \"lostark.character-roster\",\n  \"formatVersion\": 1,\n  \"nicknames\": [";
	for (size_t index = 0; index < staged.size(); ++index)
		document += (0u == index ? " \"" : ", \"") + Escape_Json(staged[index].strNickname) + "\"";
	document += " ]\n}\n";

	/* Written next to the target, flushed, then moved over it in one step, so a failed save
	leaves the previous roster file untouched. */
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
	entries = std::move(staged);
	outStatus = "Name changed.";
	return true;
}
